# Raylib Gaussian Splat Demo

An educational Gaussian splat renderer built with raylib and C++.

## Scene

~15,000 back-to-front sorted, screen-aligned splat quads forming a human head bust. The scene is built entirely from overlapping ellipsoidal Gaussian lobes covering:

- Skull and neck/bust volume
- Cheeks, forehead, jaw
- Nose (bridge, tip, nostrils)
- Lips (upper, lower, corners)
- Eye sockets, irises, pupils, whites
- Eyebrows
- Ears with warm subsurface-scatter rim glow
- Dark hair with lighter highlight wisps

## Key techniques

| Technique | Detail |
|---|---|
| Screen-aligned quads | Built via `rlgl` using the view matrix right/up vectors — true billboard geometry, no sprite textures |
| Gaussian fragment shader | UV remapped to [-1,1], `exp(-r²/0.5)` falloff, hard `discard` outside unit circle |
| Back-to-front sort | Per-frame distance sort for correct alpha compositing |
| Volumetric lobes | Each facial feature is an independent ellipsoidal density, mirroring how real 3DGS captures organic surfaces |

## Controls

| Key | Action |
|---|---|
| R | Toggle auto-rotate |
| Up / Down | Splat size |
| H | Toggle HUD |

## Build

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/raylib_gaussian_splat_demo
```
