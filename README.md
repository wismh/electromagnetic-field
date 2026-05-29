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
ctest --test-dir build -C Debug -R "ElectrostaticsTest|SimulationTest|SceneTest|CameraControlTest|FieldVizTest|DynamicsTest|TrailsTest|SliderTest|MagnetismTest" --output-on-failure
```

Web (Emscripten) and Android presets (`web`, `android-arm64`) follow the engine's README:
`external/engine/README.md`.

## Controls

The right-hand panel mirrors most shortcuts (time, scenes, layers) and adds sliders for the Coulomb constant `k`,
the softening `ε`, the `|q|` of newly placed charges and a signed uniform external `B_z`, plus live energy totals.
Each scene sets the magnetic part itself: scenes 1–7 are electrostatic (Lorentz force off, no external field), 8 and 9 switch the Lorentz force on with their uniform `B_z`, and 0 places a current loop (coil) in the plane whose strongly non-uniform `B_z` makes charges drift around it (grad-B drift). `M` and the panel then toggle it freely. The speed of light `c` (panel slider, default 20) fixes the magnetic constant as `μ₀/4π = k/c²`, so magnetic forces between moving charges are `(v/c)²` of the Coulomb force, as in nature; speeds are capped at `0.9c`, and the panel shows the fastest `v/c`. The readout next to the cursor shows
`|E|`, the components of `E` and the potential `φ`; with the magnetic layer on it also shows `B_z`.

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
| `1`–`9`, `0` | presets: dipole, like pair, quadrupole, capacitor, orbit, Rutherford scattering, swarm, cyclotron, E×B drift, coil |
| `R` / `C` | reload current preset / clear all |
| `F1`–`F6` | toggle layers: potential, field lines, vector grid, flow tracers, cursor probe, trails |
| `F7` | toggle the magnetic-field marks |
| `M` | toggle the Lorentz force |
| `K` | toggle collisions |
| `Tab` | show / hide the control panel |

## Updating the engine

```bash
git submodule update --remote external/engine
git add external/engine
git commit -m "chore: bump engine"
```
