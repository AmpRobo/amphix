# amphix_core

The AmphiX simulation library: a C++ physics engine based on Bullet Physics, extended with geometry-based hydrodynamics, plus an OpenGL 4.3 rendering pipeline for atmosphere, ocean and underwater scenes.

- CMake package: `AmphixCore` (target `AmphixCore::AmphixCore`)
- ROS 2 package: `amphix_core` (build type `cmake`)
- Headers: `#include <AmphixCore/...>`, namespace `amphix`

Contents:

| Path | Contents |
|---|---|
| `Library/` | Library headers, sources and shaders |
| `3rdparty/` | Bundled third-party code (Bullet, TinyXML-2, TinySpline, TinyExpr, stb, rapidobj, GLAD) |
| `ResourceManager/` | Tool that embeds shaders into the binary (`EMBED_RESOURCES=ON`) |
| `Tests/` | Example simulators and their data, built with `BUILD_TESTS=ON` |
| `doxygen` | Doxygen configuration for the API reference |

Build, installation and citation instructions are in the [repository README](../README.md).

Licensed under the GNU General Public License v3.0.
