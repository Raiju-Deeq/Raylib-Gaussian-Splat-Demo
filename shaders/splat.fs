#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    // Map UV [0,1] -> [-1,1]
    vec2  uv    = fragTexCoord * 2.0 - 1.0;
    float r2    = dot(uv, uv);

    // Hard discard outside unit circle
    if (r2 > 1.0) discard;

    // True Gaussian falloff
    float sigma = 0.55;
    float gauss = exp(-r2 / (2.0 * sigma * sigma));

    // Subtle brightness boost at centre
    vec3  col   = fragColor.rgb * (0.88 + 0.22 * gauss);
    float alpha = fragColor.a * gauss;

    finalColor  = vec4(col, alpha);
}
