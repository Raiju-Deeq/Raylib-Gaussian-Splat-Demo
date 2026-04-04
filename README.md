# Raylib Gaussian Splat Demo — Stanford Bunny

An educational Gaussian splat renderer built with raylib and C++.

## Scene

~50,000 back-to-front sorted, screen-aligned splat quads forming a Stanford Bunny. The scene is built from overlapping ellipsoidal Gaussian lobes covering:

- Body, haunches, chest
- Neck and head
- Snout and nose tip
- Eyes with glint highlights
- Tall ears with pink inner-ear lobes
- Front and back legs, paws/feet
- Fluffy white tail
- Warm subsurface-scatter rim glow on ears and cheeks

## Key techniques

| Technique | Detail |
|---|---|
| Screen-aligned quads | Built via `rlgl` using view matrix right/up vectors — true billboard geometry |
| Gaussian fragment shader | UV → [-1,1], `exp(-r²/0.5)` falloff, hard `discard` outside unit circle |
| Back-to-front sort | Per-frame distance sort for correct alpha compositing |
| Volumetric lobes | Each anatomical feature is an independent ellipsoidal density lobe |

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
