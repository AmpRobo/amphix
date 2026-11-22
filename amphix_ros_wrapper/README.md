# amphix_ros_wrapper

ROS 2 interface for the AmphiX simulation library (`amphix_core`).

It provides:

- `amphix_simulator`: graphical simulator node that loads a scenario from an XML file
- `amphix_simulator_nogpu`: console simulator node for machines without a capable GPU
- An XML parser that extends the `amphix_core` parser with ROS 2 publishers, subscribers, services, parameter resolution and package file lookup
- Messages (`msg/`) and services (`srv/`) for sensors and simulator control
- Launch files `amphix_simulator.launch.py` and `amphix_simulator_nogpu.launch.py`

Usage:

```bash
ros2 run amphix_ros_wrapper amphix_simulator <data_path> <scenario_file> <rate> <window_res_x> <window_res_y> <rendering_quality>
```

Build instructions are in the [repository README](../README.md). Scenario syntax and node details are in [`docs/parser.rst`](../docs/parser.rst) and [`docs/running.rst`](../docs/running.rst).

Licensed under the GNU General Public License v3.0.
