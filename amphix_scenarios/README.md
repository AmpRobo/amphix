# amphix_scenarios

Example AmphiX scenarios with their meshes and textures, taken from [bvibhav/stonefish_bluerov2](https://github.com/bvibhav/stonefish_bluerov2) by Vibhav Bharti (Apache-2.0).

| Scenario | Contents |
|---|---|
| `bluerov2_tank.scn` | BlueROV2 in a wave tank (default) |
| `bluerov2_test.scn` | BlueROV2 test scene |
| `bluerov2_farm.scn` | BlueROV2 in a wind farm (uses `turbine1.scn`) |
| `bluerov2_turbine.scn` | BlueROV2 next to a wind turbine |
| `bluesim.scn` | BlueROV2 and BlueBoat together |
| `blueboat_sea.scn` | Sonobot surface vehicle at sea |

`bluerov2.scn`, `blueboat.scn`, `sonobot.scn`, `turbine1.scn` and `turbine2.scn` are vehicle and object definitions included by the scenes above; they cannot be launched on their own.

## Usage

```bash
ros2 launch amphix_scenarios scenario.launch.py scenario:=bluerov2_tank.scn
```

Launch arguments: `scenario`, `simulation_rate`, `window_res_x`, `window_res_y`, `rendering_quality`.

Paths in the scenarios are resolved with `$(find amphix_scenarios)`, and mesh paths are relative to `data/`.

## Changes from the original

- `stonefish_bluerov2` package references were renamed to `amphix_scenarios`.
- The original launch files and the ArduSub SITL bridge (`ardusim_patch.py`) were not included; `scenario.launch.py` starts the AmphiX simulator only.
- Ported from the Stonefish v1.3 syntax to AmphiX (Stonefish 1.6):
  - Thrusters: `<specs thrust_coeff torque_coeff max_rpm inverted>` became `<specs max_setpoint inverted_setpoint normalized_setpoint>` with `max_setpoint` in rad/s, plus `<rotor_dynamics type="zero_order"/>` and a `fluid_dynamics` thrust model using the original coefficients. Setpoints stay normalized to [-1, 1].
  - The `ins` sensor in `bluerov2.scn` is commented out, because the 1.6 parser has no INS sensor. The `/bluerov2/INS` topic is no longer published.
  - `mtllib`/`usemtl` lines that pointed to missing `.mtl` files were commented out in 7 OBJ files under `data/`, because the 1.6 OBJ loader aborts on a missing material library.

All six top-level scenes load and run in `amphix_simulator`. The `*_nogpu` simulator cannot load them, because they contain cameras.
