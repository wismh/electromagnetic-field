# Electromagnetic Field

Physics sandbox built on the **Wind** engine (git submodule `external/engine`, from
[wismh/wind-engine](https://github.com/wismh/wind-engine)).

## Build (Windows / Visual Studio)

```bash
git submodule update --init --recursive
cmake --preset vs
cmake --build --preset game --config RelWithDebInfo
./build/bin/RelWithDebInfo/electromagnetic-field.exe
```

Run an optimized config (`RelWithDebInfo` or `Release`) for normal use. `Debug` is roughly 80× slower in the
field-line tracer (glm is not inlined), so scenes with moving charges drop to ~15 FPS there.

Tests:

```bash
cmake --build build --target electromagnetic_field_tests --config Debug
ctest --test-dir build -C Debug -R "ElectrostaticsTest|SimulationTest|SceneTest|CameraControlTest|FieldVizTest|DynamicsTest|TrailsTest" --output-on-failure
```

Web (Emscripten) and Android presets (`web`, `android-arm64`) follow the engine's README:
`external/engine/README.md`.

## Controls

| Input | Action |
|---|---|
| LMB on empty space | add a `+1` charge |
| RMB on empty space | add a `-1` charge |
| LMB drag on a charge | move it |
| RMB on a charge / `Delete` | remove it |
| Wheel over a charge | change `|q|` by 0.25 (0.25 … 5) |
| Wheel elsewhere | zoom about the cursor |
| MMB drag | pan the camera |
| `Home` | reset the camera |
| `F` over a charge | flip the sign |
| `L` over a charge | pin / unpin (pinned charges have a white ring) |
| `Space` | pause / resume |
| `Right` (while paused) | single step |
| `Up` / `Down` | time scale ×2 / ÷2 (1/8 … 8) |
| `1`–`7` | presets: dipole, like pair, quadrupole, capacitor, orbit, Rutherford scattering, swarm |
| `R` / `C` | reload current preset / clear all |
| `F1`–`F6` | toggle layers: potential, field lines, vector grid, flow tracers, cursor probe, trails |
| `K` | toggle collisions |

## Updating the engine

```bash
git submodule update --remote external/engine
git add external/engine
git commit -m "chore: bump engine"
```
