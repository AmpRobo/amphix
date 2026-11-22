#!/usr/bin/env bash
#
# Installs the external dependencies needed to build and run Amphix:
#   - amphix_core:        OpenGL, SDL2, Freetype, GLM
#   - amphix_ros_wrapper: ROS 2 (rclcpp, msgs, tf2, image_transport, PCL, ...)
#   - docs (optional):    Doxygen and a Python venv with Sphinx
#
# Usage: ./setup-dev-env.sh [--no-ros] [--docs] [--dry-run]
#
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROS_DISTRO="${ROS_DISTRO:-jazzy}"

WITH_ROS=1
WITH_DOCS=0
DRY_RUN=0

# ---------------------------------------------------------------------------
# Dependency lists
# ---------------------------------------------------------------------------

BUILD_TOOLS=(
  build-essential
  cmake
  git
  curl
  pkg-config
)

# amphix_core (see amphix_core/CMakeLists.txt and amphix_core/package.xml)
CORE_DEPS=(
  libgl1-mesa-dev    # OpenGL
  libglu1-mesa-dev   # OpenGL
  libsdl2-dev        # SDL2
  libfreetype-dev    # Freetype
  libglm-dev         # OpenGL Mathematics (header-only, >= 0.9.9.0)
)

# amphix_ros_wrapper (see amphix_ros_wrapper/CMakeLists.txt and package.xml)
ROS_DEPS=(
  "ros-${ROS_DISTRO}-ros-base"
  "ros-${ROS_DISTRO}-ament-cmake"
  "ros-${ROS_DISTRO}-rclcpp"
  "ros-${ROS_DISTRO}-ament-index-cpp"
  "ros-${ROS_DISTRO}-std-msgs"
  "ros-${ROS_DISTRO}-std-srvs"
  "ros-${ROS_DISTRO}-geometry-msgs"
  "ros-${ROS_DISTRO}-nav-msgs"
  "ros-${ROS_DISTRO}-sensor-msgs"
  "ros-${ROS_DISTRO}-visualization-msgs"
  "ros-${ROS_DISTRO}-rosidl-default-generators"
  "ros-${ROS_DISTRO}-rosidl-default-runtime"
  "ros-${ROS_DISTRO}-pcl-conversions"
  "ros-${ROS_DISTRO}-image-transport"
  "ros-${ROS_DISTRO}-tf2"
  "ros-${ROS_DISTRO}-tf2-ros"
  "ros-${ROS_DISTRO}-ros2launch"
  "ros-${ROS_DISTRO}-ament-lint-auto"
  "ros-${ROS_DISTRO}-ament-lint-common"
  libpcl-dev
  python3-colcon-common-extensions
  python3-rosdep
)

DOCS_DEPS=(
  doxygen
  python3-venv
  python3-pip
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Installs the system packages needed to develop Amphix on Ubuntu 24.04.

Options:
  --no-ros    Only install amphix_core dependencies (skip ROS 2)
  --docs      Also install Doxygen and a Sphinx venv in ${REPO_DIR}/.venv
  --dry-run   Print the commands instead of running them
  -h, --help  Show this help

Environment:
  ROS_DISTRO  ROS 2 distribution to use (default: jazzy)
EOF
}

log() { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
warn() { printf '\033[1;33mWarning: %s\033[0m\n' "$*" >&2; }
die() { printf '\033[1;31mError: %s\033[0m\n' "$*" >&2; exit 1; }

run() {
  if (( DRY_RUN )); then
    printf '+ %s\n' "$*"
  else
    "$@"
  fi
}

SUDO=()
if (( EUID != 0 )); then
  SUDO=(sudo)
fi

apt_install() {
  run "${SUDO[@]}" env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "$@"
}

# ---------------------------------------------------------------------------
# Steps
# ---------------------------------------------------------------------------

check_os() {
  [[ -r /etc/os-release ]] || die "Cannot detect the OS (/etc/os-release missing)."
  # shellcheck source=/dev/null
  . /etc/os-release
  if [[ "${ID:-}" != "ubuntu" ]]; then
    die "Only Ubuntu is supported (found: ${ID:-unknown})."
  fi
  if [[ "${VERSION_CODENAME:-}" != "noble" ]]; then
    warn "Tested on Ubuntu 24.04 (noble); found ${VERSION_CODENAME:-unknown}. ROS 2 ${ROS_DISTRO} may not be available."
  fi
}

install_base() {
  log "Installing build tools"
  run "${SUDO[@]}" apt-get update
  apt_install "${BUILD_TOOLS[@]}"

  log "Installing amphix_core dependencies"
  apt_install "${CORE_DEPS[@]}"
}

setup_ros_apt_source() {
  if apt-cache show "ros-${ROS_DISTRO}-ros-base" >/dev/null 2>&1; then
    return
  fi

  log "Adding the ROS 2 apt repository"
  apt_install software-properties-common
  run "${SUDO[@]}" add-apt-repository -y universe

  local version codename deb
  if (( DRY_RUN )); then
    printf '+ %s\n' "download and install ros2-apt-source from github.com/ros-infrastructure/ros-apt-source"
    return
  fi
  version="$(curl -fsSL https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest \
    | grep -F '"tag_name"' | awk -F'"' '{print $4}')"
  [[ -n "${version}" ]] || die "Could not determine the latest ros-apt-source release."
  codename="$(. /etc/os-release && echo "${UBUNTU_CODENAME:-${VERSION_CODENAME}}")"
  deb="$(mktemp --suffix=.deb)"

  run curl -fsSL -o "${deb}" \
    "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${version}/ros2-apt-source_${version}.${codename}_all.deb"
  run "${SUDO[@]}" dpkg -i "${deb}"
  run rm -f "${deb}"
  run "${SUDO[@]}" apt-get update
}

install_ros() {
  setup_ros_apt_source

  log "Installing ROS 2 ${ROS_DISTRO} and amphix_ros_wrapper dependencies"
  apt_install "${ROS_DEPS[@]}"

  log "Initializing rosdep"
  if [[ ! -f /etc/ros/rosdep/sources.list.d/20-default.list ]]; then
    run "${SUDO[@]}" rosdep init
  fi
  run rosdep update --rosdistro "${ROS_DISTRO}"

  # package.xml files are the source of truth; this catches anything missing above.
  log "Checking package.xml dependencies with rosdep"
  run rosdep install --from-paths "${REPO_DIR}" --ignore-src --rosdistro "${ROS_DISTRO}" -y
}

install_docs() {
  log "Installing documentation tools"
  apt_install "${DOCS_DEPS[@]}"

  log "Creating Sphinx venv in ${REPO_DIR}/.venv"
  run python3 -m venv "${REPO_DIR}/.venv"
  run "${REPO_DIR}/.venv/bin/pip" install --upgrade pip
  run "${REPO_DIR}/.venv/bin/pip" install -r "${REPO_DIR}/docs/requirements.txt"
}

print_next_steps() {
  log "Done"
  if (( WITH_ROS )); then
    cat <<EOF
Build the workspace (from the folder that contains this repository under src/):
  source /opt/ros/${ROS_DISTRO}/setup.bash
  colcon build
  source install/setup.bash
EOF
  else
    cat <<EOF
Build amphix_core:
  cmake -S ${REPO_DIR}/amphix_core -B build
  cmake --build build -j"\$(nproc)"
EOF
  fi
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

while (( $# )); do
  case "$1" in
    --no-ros)  WITH_ROS=0 ;;
    --docs)    WITH_DOCS=1 ;;
    --dry-run) DRY_RUN=1 ;;
    -h|--help) usage; exit 0 ;;
    *) usage >&2; die "Unknown option: $1" ;;
  esac
  shift
done

check_os
install_base
if (( WITH_ROS )); then
  install_ros
fi
if (( WITH_DOCS )); then
  install_docs
fi
print_next_steps
