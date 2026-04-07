# Gaussian Splatting — Math, Shaders, and This Demo

## Table of Contents

1. [What is a Gaussian?](#1-what-is-a-gaussian)
2. [From Points to Splats](#2-from-points-to-splats)
3. [3D Gaussian Splatting — The Full Picture](#3-3d-gaussian-splatting--the-full-picture)
4. [How This Demo Works](#4-how-this-demo-works)
5. [The Vertex Shader](#5-the-vertex-shader)
6. [The Fragment Shader](#6-the-fragment-shader)
7. [Alpha Compositing and Back-to-Front Sorting](#7-alpha-compositing-and-back-to-front-sorting)
8. [Scene Construction — Building the Bunny](#8-scene-construction--building-the-bunny)
9. [Simplifications vs. Production 3DGS](#9-simplifications-vs-production-3dgs)

---

## 1. What is a Gaussian?

A **Gaussian function** (named after Carl Friedrich Gauss) is a bell-shaped curve that peaks at its centre and decays exponentially with distance. In one dimension:

```
G(x) = exp( -x² / (2σ²) )
```

- `x` — distance from the centre
- `σ` (sigma) — the **standard deviation**, controlling width
- The output ranges from 1.0 at the centre (x = 0) down toward 0 as x grows

The key property: the falloff is **smooth and differentiable everywhere**. There are no hard edges. This makes Gaussians ideal for representing soft, continuous volumes like smoke, skin, fur, or any organic surface.

In **two dimensions** the isotropic (equal spread in all directions) form is:

```
G(x, y) = exp( -(x² + y²) / (2σ²) )
         = exp( -r² / (2σ²) )
```

where `r² = x² + y²` is the squared distance from the centre in the 2D plane.

In the fragment shader, `r²` is called `r2` and `2σ²` is the constant `0.5` (i.e. σ ≈ 0.5).

---

## 2. From Points to Splats

Traditional point clouds store one colour per 3D point — a discrete, infinitely thin sample. Rendered naively they produce a sparse, holey result with no sense of volume.

A **splat** replaces the infinitely thin point with a small **oriented disc** (a billboard quad) whose opacity follows a Gaussian falloff from the centre outward. The idea:

```
Opacity at pixel = base_alpha × G(distance from disc centre)
```

Because the falloff is smooth, overlapping splats blend seamlessly. Stack enough of them and the eye perceives a continuous, volumetric surface — even though the underlying geometry is just a cloud of transparent quads.

This is the core insight behind **3D Gaussian Splatting (3DGS)**, introduced by Kerbl et al. (SIGGRAPH 2023).

---

## 3. 3D Gaussian Splatting — The Full Picture

In production 3DGS, each splat is a **full 3D Gaussian** described by:

| Parameter | Meaning |
|---|---|
| **Position** `μ` | 3D centre in world space |
| **Covariance matrix** `Σ` | 3×3 matrix encoding scale and rotation (the ellipsoid shape) |
| **Opacity** `α` | Base transparency |
| **Spherical harmonics** | View-dependent colour (captures specularity) |

The 3D covariance is decomposed as:

```
Σ = R S S^T R^T
```

where `R` is a rotation matrix (stored as a quaternion) and `S` is a diagonal scale matrix. This factorisation guarantees `Σ` is positive semi-definite and stays physically valid during optimisation.

### Projecting to 2D

To render, each 3D Gaussian is **projected** (splatted) onto the image plane. Given the view transform `W` and the Jacobian of the projective transformation `J`, the projected 2D covariance is:

```
Σ' = J W Σ W^T J^T
```

This gives a 2D ellipse on screen. The fragment shader then evaluates the 2D Gaussian over that ellipse.

### The 2D Gaussian Evaluated per Fragment

For a pixel at normalised offset `(u, v)` from the splat centre, the Gaussian value is:

```
G = exp( -0.5 × [u v] × (Σ')⁻¹ × [u v]^T )
```

The inverse of the 2D covariance scales and rotates the ellipse. For an **isotropic** (circular) splat with `Σ' = σ²I`, this simplifies to:

```
G = exp( -(u² + v²) / (2σ²) )
  = exp( -r² / (2σ²) )
```

which is exactly what this demo uses.

---

## 4. How This Demo Works

This demo implements a **simplified but faithful** version of Gaussian splatting:

```
CPU side                           GPU side
─────────────────────────────      ─────────────────────────────
Each splat → screen-aligned   →    Vertex shader: MVP transform
billboard quad (2 triangles)       Fragment shader: Gaussian falloff
                                   + alpha blend (back-to-front)
```

The pipeline step by step:

1. **Scene generation** — `BuildBunny()` places ~50,000 splats in 3D space, each with a position, base colour, alpha, and size.
2. **Back-to-front sort** — `SortSplats()` orders splats by distance from the camera (farthest first). This is required for correct alpha blending.
3. **Billboard construction** — `DrawSplatQuad()` builds a camera-aligned quad in world space using the view matrix right/up vectors. UV coordinates `[0,1]²` are assigned to the corners.
4. **Vertex shader** — passes position, UV, and colour through; applies the MVP matrix.
5. **Fragment shader** — remaps UV to `[-1,1]²`, computes `r²`, discards pixels outside the unit circle, then applies the Gaussian to modulate alpha and brightness.
6. **Alpha blend** — `BLEND_ALPHA` composites each splat over what is behind it.

---

## 5. The Vertex Shader

```glsl
// shaders/splat.vs
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor    = vertexColor;
    gl_Position  = mvp * vec4(vertexPosition, 1.0);
}
```

### What it does

The vertex shader is minimal by design. All the geometric work (building the billboard quad) was already done on the CPU in `DrawSplatQuad()`.

**Inputs:**
- `vertexPosition` — one corner of the billboard quad in world space
- `vertexTexCoord` — UV coordinate for this corner, one of `(0,0)`, `(1,0)`, `(1,1)`, `(0,1)`
- `vertexColor` — the splat's `(r, g, b, α)` packed as a `vec4`

**Outputs:**
- `fragTexCoord` — passed through unchanged to the fragment shader
- `fragColor` — passed through unchanged
- `gl_Position` — the corner projected to clip space via the **Model-View-Projection** matrix

The MVP transform is the standard pipeline:

```
clip position = Projection × View × Model × world position
```

Because the quad was built in world space (no separate model transform), `M = I` here.

### Why the quad is built on the CPU

The CPU uses the **right** and **up** vectors extracted from the view matrix:

```cpp
Vector3 right = {view.m0, view.m4, view.m8};
Vector3 up    = {view.m1, view.m5, view.m9};
```

These are the first two rows of the view matrix — they describe the camera's horizontal and vertical axes in world space. The four corners of the billboard are then:

```
v0 = centre + (-sz)*right + (-sz)*up   (bottom-left)
v1 = centre + (+sz)*right + (-sz)*up   (bottom-right)
v2 = centre + (+sz)*right + (+sz)*up   (top-right)
v3 = centre + (-sz)*right + (+sz)*up   (top-left)
```

This keeps each quad **always facing the camera**, which is exactly what a screen-aligned billboard needs.

---

## 6. The Fragment Shader

```glsl
// shaders/splat.fs
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    vec2  uv    = fragTexCoord * 2.0 - 1.0;
    float r2    = dot(uv, uv);
    if (r2 > 1.0) discard;

    // Gaussian falloff  σ = 0.5
    float gauss = exp(-r2 / 0.5);

    // Subtle centre brightening for subsurface feel
    vec3  col   = fragColor.rgb * (0.85 + 0.20 * gauss);
    float alpha = fragColor.a * gauss;

    finalColor  = vec4(col, alpha);
}
```

### Step-by-step breakdown

#### Step 1 — Remap UV to `[-1, 1]²`

```glsl
vec2 uv = fragTexCoord * 2.0 - 1.0;
```

The incoming `fragTexCoord` is in `[0, 1]²`. Multiplying by 2 and subtracting 1 remaps this to `[-1, 1]²`. Now the **centre of the quad is at `(0, 0)`** and the corners are at `(±1, ±1)`.

#### Step 2 — Compute squared radius

```glsl
float r2 = dot(uv, uv);
```

`dot(uv, uv) = uv.x² + uv.y²` — this is the squared distance from the centre of the disc. Using squared distance avoids a `sqrt()` call, since the Gaussian and the circle test both only need `r²`.

#### Step 3 — Discard outside the unit circle

```glsl
if (r2 > 1.0) discard;
```

The billboard quad is a square, but we want a circular splat. Any fragment in the corners of the square (where `r² > 1`) is discarded entirely. This makes the splat circular and prevents hard square edges from being visible.

Visually:

```
   ┌───────────┐
   │  ╔═════╗  │   Square quad
   │  ║     ║  │   ═══ kept (r² ≤ 1)
   │  ║     ║  │   ·   discarded (r² > 1)
   │  ╚═════╝  │
   └───────────┘
```

#### Step 4 — Evaluate the Gaussian

```glsl
float gauss = exp(-r2 / 0.5);
```

This is the 2D isotropic Gaussian `G = exp(-r² / (2σ²))` with `2σ² = 0.5`, so `σ = 0.5 / sqrt(2) ≈ 0.354`.

| `r²` | `gauss` | Meaning |
|---|---|---|
| 0.0 | 1.000 | Centre of disc — full intensity |
| 0.25 | 0.607 | At half-radius |
| 0.5 | 0.368 | At `1/√2` of radius |
| 1.0 | 0.135 | Edge of the kept circle |

The falloff is smooth and continuous — no banding or hard boundary.

#### Step 5 — Colour brightening (subsurface scatter approximation)

```glsl
vec3 col = fragColor.rgb * (0.85 + 0.20 * gauss);
```

At the centre (`gauss = 1.0`): multiplier = `0.85 + 0.20 = 1.05` → slightly brighter than the base colour.  
At the edge (`gauss ≈ 0.135`): multiplier = `0.85 + 0.027 = 0.877` → slightly dimmer.

This mimics **subsurface scattering** — the way light penetrates skin and scatters, making the interior of a translucent object appear to glow. The centre is lighter, the periphery darker, giving depth to each splat.

#### Step 6 — Modulate alpha

```glsl
float alpha = fragColor.a * gauss;
```

The per-splat base alpha (set during scene construction) is multiplied by the Gaussian weight. Near the centre, the splat is opaque at its intended alpha. Toward the edges, it fades toward zero — no hard boundary, a smooth transparent halo.

---

## 7. Alpha Compositing and Back-to-Front Sorting

### Why order matters

Standard **over** compositing (Porter-Duff "over") combines a source colour `src` over a destination `dst` as:

```
out.rgb = src.rgb × src.a + dst.rgb × (1 - src.a)
out.a   = src.a + dst.a × (1 - src.a)
```

This is **not commutative** — swapping the order changes the result. Drawing a dark splat in front of a light one produces different output than the reverse.

### The painter's algorithm

The demo sorts all splats every frame by squared distance from the camera:

```cpp
std::sort(splats.begin(), splats.end(), [&](const Splat& a, const Splat& b) {
    return Vector3DistanceSqr(a.position, cam) > Vector3DistanceSqr(b.position, cam);
});
```

Farthest splats are drawn first. Then closer splats are composited on top with alpha blending. This is the **painter's algorithm** — like painting a landscape by starting with the sky and finishing with foreground objects.

### GPU blend mode

```cpp
BeginBlendMode(BLEND_ALPHA);
```

This enables the GPU's alpha blending pipeline. For every fragment, the GPU computes:

```
finalRGB = src.rgb × src.a + framebuffer.rgb × (1 - src.a)
```

The CPU sort ensures the splats arrive in the correct order for this formula to produce an accurate result.

### Cost

Sorting 50,000 splats with `std::sort` is `O(N log N)`. At 50k splats this takes roughly 3–6 ms per frame on a modern CPU — noticeable but acceptable for a demo. Production renderers either use GPU-side radix sort or reduce the sort to coarse depth buckets.

---

## 8. Scene Construction — Building the Bunny

The bunny is assembled from **anatomical lobes** — each lobe is a 3D ellipsoid filled with randomly placed splats. The `addLobe()` function handles this:

### Uniform sampling inside an ellipsoid

```cpp
// Step 1: uniform direction on the unit sphere
do {
    u = frand(-1,1); v = frand(-1,1); w = frand(-1,1);
    len = sqrtf(u*u+v*v+w*w);
} while (len < 1e-5f || len > 1.0f);

// Step 2: uniform radius via cube-root trick
float t = cbrtf(frand01());

// Step 3: scale by ellipsoid axes
float px = centre.x + (u/len)*ax*t;
float py = centre.y + (v/len)*ay*t;
float pz = centre.z + (w/len)*az*t;
```

**Why `cbrtf`?** A common mistake is picking `t = frand01()` uniformly. This over-concentrates splats near the centre because the volume element in spherical coordinates scales as `r²`. The correct fix: if `t ~ Uniform(0,1)`, then `r = t^(1/3)` produces **uniformly distributed radii** inside a ball, since:

```
P(r < R) = R³   →   PDF = 3r²   →   sample r = U^(1/3)
```

Scaling each axis independently (`ax`, `ay`, `az`) stretches the sphere into an ellipsoid.

### Alpha fade from centre

```cpp
float d = sqrt( (px-cx)²/ax² + (py-cy)²/ay² + (pz-cz)²/az² );
float alpha = (1.0f - d*0.55f) * frand(aMin, aMax);
```

`d` is the normalised distance from the lobe centre (0 at centre, ~1 at ellipsoid boundary). The `1 - d×0.55` term makes splats near the centre slightly more opaque, giving each lobe a soft, volumetric interior.

### Anatomical breakdown

| Lobe | Splats | Purpose |
|---|---|---|
| Body | 12,000 | Main torso ellipsoid |
| Haunches | 5,000 | Rear mass |
| Chest | 3,500 | Forward chest swell |
| Head | 6,000 | Cranial volume |
| Snout | 2,000 | Muzzle protrusion |
| Nose tip | 600 | Pink nose, small radius |
| Eyes | 500 × 2 | Dark irises |
| Eye glints | 120 × 2 | Specular highlights |
| Ears | 3,000 × 2 | Tall narrow ellipsoids |
| Inner ear | 1,000 × 2 | Pink membrane |
| Front legs | 1,800 × 2 | Vertical cylinders |
| Front paws | 600 × 2 | Flattened ellipsoids |
| Back legs | 1,800 × 2 | Angled rear haunches |
| Back feet | 700 × 2 | Elongated rear paws |
| Tail | 800 | Small white fluff sphere |
| SSS ear rim | 500 × 2 | Warm orange rim glow |
| SSS cheek | 350 × 2 | Warm subsurface cheek |

**Total: ~50,000 splats.**

The **subsurface scattering (SSS) lobes** are low-alpha splats with warm orange/amber colours layered inside the ears and cheeks. Because they are semi-transparent and blend additively with the skin tone, they produce the warm glow visible when backlit — the same phenomenon seen in real rabbit ears.

---

## 9. Simplifications vs. Production 3DGS

This demo is educational. Here is what it omits compared to the full Kerbl et al. 3DGS pipeline:

| Feature | This Demo | Production 3DGS |
|---|---|---|
| **Splat shape** | Isotropic circle (σ fixed) | Full 3D anisotropic Gaussian (covariance matrix) |
| **Projection** | CPU billboard quad | GPU 2D covariance projection via Jacobian |
| **Colour** | Fixed RGB per splat | Spherical harmonics (view-dependent colour) |
| **Scene origin** | Hand-authored procedural | Optimised from Structure-from-Motion point cloud |
| **Sort** | CPU `std::sort` | GPU radix sort |
| **Shader** | Simple Gaussian α blend | Full EWA (Elliptical Weighted Average) splatting |
| **Scale** | 50k splats | 1–6 million splats typical |
| **Learning** | N/A | Gradient-descent optimisation of all parameters |

Despite these simplifications, the core rendering equation is the same:

```
pixel colour = Σ (colour_i × α_i × G_i(pixel))
```

summed back-to-front over all splats that cover the pixel, where `G_i` is the Gaussian weight of splat `i` at that pixel location. Everything else in production 3DGS is in service of learning better splat parameters from real photographs.

---

*This document was written as a companion to the Raylib Gaussian Splat Demo source code. Read it alongside `src/main.cpp`, `shaders/splat.vs`, and `shaders/splat.fs`.*
