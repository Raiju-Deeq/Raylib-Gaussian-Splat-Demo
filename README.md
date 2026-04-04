# Raylib Gaussian Splat Demo

A small educational Gaussian splat style viewer built with raylib and C++.

## What it is

This is not a full 3D Gaussian Splatting implementation. It is a lightweight demo that renders thousands of camera-facing billboards with a Gaussian-like fragment shader so you can explore the look and blending behaviour of splats before building a lower-level renderer.

## Features

- Orbiting 3D camera
- Thousands of generated splats
- Custom shader with Gaussian falloff
- Alpha and additive blending toggle
- Simple CMake build

## Controls

- `SPACE`: toggle alpha/additive blending
- `R`: toggle auto rotate
- `UP` / `DOWN`: increase or decrease splat size

## Build

You need raylib installed and discoverable by CMake.

```bash
cmake -S . -B build
cmake --build build
./build/raylib_gaussian_splat_demo
```

## Learning value

This demo helps you experiment with:

- point based scene representations
- shader driven soft splat edges
- billboard rendering
- transparency and blending tradeoffs

A future version could replace generated splats with loaded `.ply` or `.txt` data and sort splats back-to-front for improved transparency behaviour.
