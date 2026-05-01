# Electromagnetic Field

Physics sandbox built on the **Wind** engine (git submodule `external/engine`, from
[wismh/wind-engine](https://github.com/wismh/wind-engine)).

## Build (Windows / Visual Studio)

```bash
git submodule update --init --recursive
cmake --preset vs
cmake --build --preset game --config Debug
./build/bin/Debug/electromagnetic-field.exe
```

Tests:

```bash
cmake --build build --target electromagnetic_field_tests --config Debug
ctest --test-dir build -C Debug -R SmokeTest --output-on-failure
```

Web (Emscripten) and Android presets (`web`, `android-arm64`) follow the engine's README:
`external/engine/README.md`.

## Updating the engine

```bash
git submodule update --remote external/engine
git add external/engine
git commit -m "chore: bump engine"
```
