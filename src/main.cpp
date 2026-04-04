#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include <cmath>
#include <cstdlib>

struct Splat {
    Vector3 position;
    Color color;
    float radius;
    float alpha;
};

static float Frand(float minV, float maxV) {
    return minV + (maxV - minV) * ((float)rand() / (float)RAND_MAX);
}

static std::vector<Splat> GenerateSplats(int count) {
    std::vector<Splat> splats;
    splats.reserve(count);

    for (int i = 0; i < count; ++i) {
        float t = (float)i / (float)count;
        float a = t * 18.0f;
        float r = 0.25f + t * 6.0f + Frand(-0.15f, 0.15f);
        float x = cosf(a) * r;
        float z = sinf(a) * r;
        float y = (t - 0.5f) * 6.0f + sinf(a * 1.7f) * 0.3f;

        unsigned char red   = (unsigned char)(110 + 120 * (0.5f + 0.5f * sinf(a * 0.7f)));
        unsigned char green = (unsigned char)(100 + 120 * (0.5f + 0.5f * sinf(a * 1.1f + 1.4f)));
        unsigned char blue  = (unsigned char)(140 + 100 * (0.5f + 0.5f * sinf(a * 0.9f + 2.2f)));

        splats.push_back({
            {x, y, z},
            {red, green, blue, 255},
            Frand(0.06f, 0.16f),
            Frand(0.35f, 0.85f)
        });
    }

    return splats;
}

int main() {
    const int screenWidth  = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Raylib Gaussian Splat Demo");
    SetTargetFPS(144);

    Camera3D camera = { 0 };
    camera.position   = { 10.0f, 6.0f, 10.0f };
    camera.target     = {  0.0f, 0.0f,  0.0f };
    camera.up         = {  0.0f, 1.0f,  0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadShader("shaders/splat.vs", "shaders/splat.fs");
    int tintLoc     = GetShaderLocation(shader, "uTintStrength");
    int softnessLoc = GetShaderLocation(shader, "uSoftness");
    float tintStrength = 1.0f;
    float softness     = 2.2f;
    SetShaderValue(shader, tintLoc,     &tintStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, softnessLoc, &softness,     SHADER_UNIFORM_FLOAT);

    Texture2D white = LoadTextureFromImage(GenImageColor(8, 8, WHITE));
    std::vector<Splat> splats = GenerateSplats(7000);

    float baseSize  = 0.45f;
    bool additive   = false;
    bool autoRotate = true;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) additive   = !additive;
        if (IsKeyPressed(KEY_R))     autoRotate = !autoRotate;
        if (IsKeyPressed(KEY_UP))    baseSize  += 0.05f;
        if (IsKeyPressed(KEY_DOWN))  baseSize   = fmaxf(0.05f, baseSize - 0.05f);

        UpdateCamera(&camera, CAMERA_ORBITAL);

        if (autoRotate) {
            float time = GetTime();
            camera.position.x = cosf(time * 0.35f) * 11.0f;
            camera.position.z = sinf(time * 0.35f) * 11.0f;
            camera.position.y = 5.5f + sinf(time * 0.22f) * 2.0f;
        }

        BeginDrawing();
        ClearBackground((Color){10, 12, 18, 255});

        BeginMode3D(camera);
        DrawGrid(20, 1.0f);

        BeginBlendMode(additive ? BLEND_ADDITIVE : BLEND_ALPHA);
        BeginShaderMode(shader);

        for (const auto &s : splats) {
            Color c = s.color;
            c.a = (unsigned char)(255.0f * s.alpha);
            DrawBillboard(camera, white, s.position, baseSize * s.radius * 6.0f, c);
        }

        EndShaderMode();
        EndBlendMode();
        EndMode3D();

        DrawRectangle(16, 16, 430, 120, Fade(BLACK, 0.45f));
        DrawText("Gaussian Splat Demo (educational approximation)", 28, 28, 24, RAYWHITE);
        DrawText(TextFormat("Splats: %i", (int)splats.size()), 28, 62, 20, LIGHTGRAY);
        DrawText(TextFormat("Blend: %s  |  Size: %.2f", additive ? "Additive" : "Alpha", baseSize), 28, 86, 20, LIGHTGRAY);
        DrawText("SPACE toggle blend  |  R toggle auto-rotate  |  Up/Down change size", 28, 110, 18, GRAY);
        DrawFPS(GetScreenWidth() - 100, 20);

        EndDrawing();
    }

    UnloadTexture(white);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
