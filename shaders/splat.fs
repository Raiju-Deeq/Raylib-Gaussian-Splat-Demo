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
