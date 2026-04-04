# Raylib Gaussian Splat Demo

An educational Gaussian splat style viewer built with raylib and C++.

## What it is

This demo renders ~20,000 back-to-front sorted, screen-aligned quad splats using a custom GLSL fragment shader that applies a true Gaussian falloff. It is not a full 3D Gaussian Splatting implementation but gives you the core visual intuition of how splat-based rendering works before building a lower-level Vulkan renderer.

## Key techniques

- **Screen-aligned quads via `rlgl`** — each splat is a camera-facing quad, not a sprite/billboard texture
- **Gaussian fragment shader** — UV remapped to [-1,1], `exp(-r²/2σ²)` falloff, hard discard outside unit circle
- **Back-to-front sorting** — splats sorted by camera distance every frame for correct alpha blending
- **Dense point cloud scene** — multi-lobe procedural cloud with a warm body, cool rim highlights, and ground scatter

## Controls

| Key | Action |
|-----|--------|
| R | Toggle auto-rotate |
| Up / Down | Increase / decrease splat size |
| H | Toggle HUD |

## Build

Requires raylib (install via vcpkg: `vcpkg install raylib`).

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/raylib_gaussian_splat_demo
```

## Why this is useful

This demo teaches you:

- How splats are represented as oriented ellipses projected onto screen space
- Why back-to-front sorting matters for alpha compositing
- How the Gaussian function shapes a soft radial falloff in the fragment shader
- How to drive raw GPU quads via `rlgl` in raylib bypassing the sprite pipeline

A natural next step is replacing the procedural cloud with loaded `.ply` splat data and adding a depth-sorted GPU buffer.
