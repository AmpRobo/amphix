# AmphiX

AmphiX is a marine robotics simulator with a ROS 2 interface. It combines a physics engine based on [Bullet Physics](https://pybullet.org), geometry-based hydrodynamics, and an OpenGL rendering pipeline for atmosphere, ocean and underwater scenes.

AmphiX is based on [Stonefish](https://github.com/patrykcieslak/stonefish) and [stonefish_ros2](https://github.com/patrykcieslak/stonefish_ros2) by Patryk Cieślak.

## Repository layout

| Path | Contents |
|---|---|
| [`amphix_core/`](amphix_core) | The simulation library (C++, CMake package `AmphixCore`, ROS 2 package `amphix_core`). Includes example simulators in `Tests/`. |
| [`amphix_ros_wrapper/`](amphix_ros_wrapper) | ROS 2 package with the standard simulator nodes, XML scenario parser extensions, messages and services. |
| [`amphix_scenarios/`](amphix_scenarios) | ROS 2 package with example scenarios (BlueROV2, BlueBoat, Sonobot, wind farm) and their meshes. |
| [`docs/`](docs) | Sphinx documentation for both packages. |
| [`setup-dev-env.sh`](setup-dev-env.sh) | Installs all system dependencies. |

## Requirements

- Ubuntu 24.04 with ROS 2 Jazzy
- A GPU with OpenGL 4.3 or newer and the manufacturer's drivers (the `*_nogpu` simulator runs without one, with limited features)

## Quick start

Clone the repository into the `src` folder of a ROS 2 workspace, then install the dependencies:

```bash
mkdir -p ~/amphix_ws/src && cd ~/amphix_ws/src
git clone <repository-url> amphix
./amphix/setup-dev-env.sh
```

`setup-dev-env.sh` installs the build tools, the `amphix_core` libraries (OpenGL, SDL2, Freetype, GLM), ROS 2 Jazzy and the `amphix_ros_wrapper` dependencies, then runs `rosdep install` for both packages. Options:

- `--no-ros`: only install what `amphix_core` needs
- `--docs`: also install Doxygen and a Sphinx virtual environment in `.venv/`
- `--dry-run`: print the commands without running them

Build the workspace (`amphix_core` is built first):

```bash
cd ~/amphix_ws
source /opt/ros/jazzy/setup.bash
colcon build
source install/setup.bash
```

## Running the simulator

In every new terminal, load the ROS 2 and workspace environments first (from the workspace root):

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
```

### Example scenarios

Start one of the scenarios bundled in `amphix_scenarios`:

```bash
ros2 launch amphix_scenarios scenario.launch.py scenario:=bluerov2_tank.scn
```

| Scenario | Contents |
|---|---|
| `bluerov2_tank.scn` | BlueROV2 in a wave tank (default) |
| `bluerov2_test.scn` | BlueROV2 test scene |
| `bluerov2_farm.scn` | BlueROV2 in a wind farm |
| `bluerov2_turbine.scn` | BlueROV2 next to a wind turbine |
| `bluesim.scn` | BlueROV2 and BlueBoat together |
| `blueboat_sea.scn` | Sonobot surface vehicle at sea |

Launch arguments (all optional):

| Argument | Default | Meaning |
|---|---|---|
| `scenario` | `bluerov2_tank.scn` | Scenario file in `amphix_scenarios/scenarios` |
| `simulation_rate` | `100.0` | Physics steps per second |
| `window_res_x`, `window_res_y` | `1280`, `720` | Window size in pixels |
| `rendering_quality` | `high` | `low`, `medium` or `high`; lower it if the frame rate is low |

For example:

```bash
ros2 launch amphix_scenarios scenario.launch.py scenario:=bluerov2_farm.scn rendering_quality:=medium window_res_x:=1920 window_res_y:=1080
```

### Your own scenarios

Run the simulator node directly with a data folder (meshes and textures) and a scenario file:

```bash
ros2 run amphix_ros_wrapper amphix_simulator <data_path> <scenario_file> <rate> <window_res_x> <window_res_y> <rendering_quality>
```

For example, running a bundled scenario this way:

```bash
SHARE=$(ros2 pkg prefix amphix_scenarios)/share/amphix_scenarios
ros2 run amphix_ros_wrapper amphix_simulator $SHARE/data $SHARE/scenarios/bluerov2_tank.scn 100.0 1280 720 high
```

Without a capable GPU, use the console simulator instead. It has no window and cannot simulate cameras or other vision sensors, so it does not load the bundled scenarios:

```bash
ros2 run amphix_ros_wrapper amphix_simulator_nogpu <data_path> <scenario_file> <rate>
```

To start the simulator from your own launch file, include `amphix_simulator.launch.py` (or `amphix_simulator_nogpu.launch.py`) from the `amphix_ros_wrapper` package and set its arguments, as [`amphix_scenarios/launch/scenario.launch.py`](amphix_scenarios/launch/scenario.launch.py) does. See [`docs/running.rst`](docs/running.rst) for details.

### Stopping and errors

- Stop the simulator with Ctrl+C or by closing its window.
- If a scenario fails to load, the simulator exits with code 1. The reason is written to `~/.ros/log/amphix_ros_wrapper_parser.log`.

## Building amphix_core without ROS

```bash
cmake -S amphix_core -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
sudo cmake --install build
```

Other projects can then use it with `find_package(AmphixCore REQUIRED)` and link to `AmphixCore::AmphixCore`. Headers are included as `<AmphixCore/...>` and the C++ namespace is `amphix`.

CMake options:

- `BUILD_TESTS=ON`: build the example simulators in `amphix_core/Tests` against a local, non-installable copy of the library
- `EMBED_RESOURCES=ON`: compile the shaders into the library binary

## Documentation

- User guide (Sphinx): install the tools with `./setup-dev-env.sh --no-ros --docs`, then run `make -C docs html SPHINXBUILD=../.venv/bin/sphinx-build` and open `docs/_build/html/index.html`.
- API reference (Doxygen): run `doxygen doxygen` inside `amphix_core/` and open `amphix_core/docs/html/index.html`.

## Credits

AmphiX is derived from Stonefish, written by Patryk Cieślak. Parts of the code based on work by other authors are marked as such, and the third-party libraries bundled in `amphix_core/3rdparty` are listed in [`docs/install.rst`](docs/install.rst).

If you use this software in your research, please cite the original Stonefish papers:

*Patryk Cieślak, "Stonefish: An Advanced Open-Source Simulation Tool Designed for Marine Robotics, With a ROS Interface", In Proceedings of MTS/IEEE OCEANS 2019, June 2019, Marseille, France*

```bibtex
@inproceedings{stonefish,
   author = {Cie{\'s}lak, Patryk},
   booktitle = {Proceedings of MTS/IEEE OCEANS 2019},
   title = {{Stonefish: An Advanced Open-Source Simulation Tool Designed for Marine Robotics, With a ROS Interface}},
   month = jun,
   year = {2019},
   doi = {10.1109/OCEANSE.2019.8867434}}
```

*Michele Grimaldi, Patryk Cieślak, Eduardo Ochoa, Vibhav Bharti, Hayat Rajani, Ignacio Carlucho, Maria Koskinopoulou, Yvan R. Petillot, and Nuno Gracias, "Stonefish: Supporting Machine Learning Research in Marine Robotics", In Proceedings of IEEE ICRA 2025, May 2025, Atlanta, USA*

```bibtex
@inproceedings{stonefish_ml,
   author = {Michele Grimaldi and Patryk Cieslak and Eduardo Ochoa and Vibhav Bharti and Hayat Rajani and Ignacio Carlucho and Maria Koskinopoulou and Yvan R. Petillot and Nuno Gracias},
   title = {Stonefish: Supporting Machine Learning Research in Marine Robotics},
   booktitle = {Proceedings of the IEEE International Conference on Robotics and Automation},
   month = may,
   year = {2025},
   eprint = {2502.11887},
   archivePrefix = {arXiv},
   url = {https://arxiv.org/abs/2502.11887},
   organization = {IEEE}}
```

## License

`amphix_core` and `amphix_ros_wrapper` are licensed under the GNU General Public License v3.0, the license of the Stonefish code they are based on (see the file headers and each `package.xml`).
