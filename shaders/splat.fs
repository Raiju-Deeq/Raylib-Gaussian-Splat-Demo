#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform float uTintStrength;
uniform float uSoftness;

out vec4 finalColor;

void main() {
    vec2  uv       = fragTexCoord * 2.0 - 1.0;
    float r2       = dot(uv, uv);
    float gaussian = exp(-uSoftness * r2 * 2.0);
    float edgeMask = smoothstep(1.0, 0.0, r2);
    float alpha    = gaussian * edgeMask * fragColor.a;
    vec3  color    = fragColor.rgb * mix(0.85, 1.15, gaussian * uTintStrength);
    finalColor     = vec4(color, alpha);
}
