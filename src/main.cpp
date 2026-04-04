#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

struct Splat {
    Vector3 position;
    float   r, g, b, a;
    float   size;
};

static float frand(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}

static std::vector<Splat> BuildScene() {
    std::vector<Splat> splats;
    splats.reserve(20000);

    auto addCloud = [&](Vector3 centre, float radius, int count,
                        float r, float g, float b, float sizeScale) {
        for (int i = 0; i < count; ++i) {
            float u   = frand(-1.f, 1.f);
            float v   = frand(-1.f, 1.f);
            float w   = frand(-1.f, 1.f);
            float len = sqrtf(u*u + v*v + w*w);
            if (len < 1e-5f) continue;
            float t   = cbrtf(frand(0.f, 1.f));
            float x   = centre.x + (u / len) * radius * t;
            float y   = centre.y + (v / len) * radius * t;
            float z   = centre.z + (w / len) * radius * t;
            float d   = sqrtf((x-centre.x)*(x-centre.x) +
                               (y-centre.y)*(y-centre.y) +
                               (z-centre.z)*(z-centre.z)) / radius;
            float alpha = (1.f - d * 0.6f) * frand(0.35f, 0.75f);
            float cr = r + frand(-0.08f, 0.08f);
            float cg = g + frand(-0.08f, 0.08f);
            float cb = b + frand(-0.08f, 0.08f);
            splats.push_back({{x,y,z}, cr, cg, cb, alpha,
                              sizeScale * frand(0.04f, 0.09f)});
        }
    };

    // Main body
    addCloud({0, 0, 0},    2.4f, 7000, 0.72f, 0.58f, 0.45f, 1.0f);
    // Head lobe
    addCloud({0, 2.8f, 0}, 1.3f, 3000, 0.76f, 0.61f, 0.48f, 0.85f);
    // Ears
    addCloud({-0.6f, 4.2f, 0.1f}, 0.5f, 1200, 0.80f, 0.55f, 0.50f, 0.6f);
    addCloud({ 0.6f, 4.2f, 0.1f}, 0.5f, 1200, 0.80f, 0.55f, 0.50f, 0.6f);
    // Ground scatter
    addCloud({0, -2.0f, 0}, 2.8f, 2500, 0.55f, 0.42f, 0.32f, 0.7f);
    // Cool rim highlights
    addCloud({ 1.6f, 1.0f,  0.8f}, 1.0f, 1500, 0.55f, 0.72f, 0.90f, 0.65f);
    addCloud({-1.6f, 1.0f, -0.8f}, 1.0f, 1500, 0.40f, 0.55f, 0.80f, 0.65f);

    return splats;
}

static void SortSplats(std::vector<Splat>& splats, Vector3 camPos) {
    std::sort(splats.begin(), splats.end(), [&](const Splat& a, const Splat& b) {
        float da = Vector3DistanceSqr(a.position, camPos);
        float db = Vector3DistanceSqr(b.position, camPos);
        return da > db;
    });
}

static void DrawSplatQuad(Vector3 pos, float size, float r, float g, float b, float a,
                          Matrix viewMatrix) {
    Vector3 right = {viewMatrix.m0, viewMatrix.m4, viewMatrix.m8};
    Vector3 up    = {viewMatrix.m1, viewMatrix.m5, viewMatrix.m9};

    Vector3 v0 = Vector3Add(pos, Vector3Add(Vector3Scale(right, -size), Vector3Scale(up, -size)));
    Vector3 v1 = Vector3Add(pos, Vector3Add(Vector3Scale(right,  size), Vector3Scale(up, -size)));
    Vector3 v2 = Vector3Add(pos, Vector3Add(Vector3Scale(right,  size), Vector3Scale(up,  size)));
    Vector3 v3 = Vector3Add(pos, Vector3Add(Vector3Scale(right, -size), Vector3Scale(up,  size)));

    rlColor4f(r, g, b, a);
    rlTexCoord2f(0, 0); rlVertex3f(v0.x, v0.y, v0.z);
    rlTexCoord2f(1, 0); rlVertex3f(v1.x, v1.y, v1.z);
    rlTexCoord2f(1, 1); rlVertex3f(v2.x, v2.y, v2.z);
    rlTexCoord2f(0, 1); rlVertex3f(v3.x, v3.y, v3.z);
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Raylib Gaussian Splat Demo");
    SetTargetFPS(144);

    Camera3D camera = {};
    camera.position   = {6.0f, 4.0f, 8.0f};
    camera.target     = {0.0f, 1.0f, 0.0f};
    camera.up         = {0.0f, 1.0f, 0.0f};
    camera.fovy       = 42.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadShader("shaders/splat.vs", "shaders/splat.fs");

    std::vector<Splat> splats = BuildScene();

    bool  autoRotate  = true;
    bool  showHelp    = true;
    float orbitAngle  = 0.0f;
    float orbitRadius = 8.5f;
    float orbitHeight = 3.5f;
    float sizeScale   = 1.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_R))    autoRotate = !autoRotate;
        if (IsKeyPressed(KEY_H))    showHelp   = !showHelp;
        if (IsKeyPressed(KEY_UP))   sizeScale  = fminf(sizeScale + 0.1f, 3.0f);
        if (IsKeyPressed(KEY_DOWN)) sizeScale  = fmaxf(sizeScale - 0.1f, 0.2f);

        if (autoRotate) orbitAngle += dt * 0.28f;

        camera.position.x = cosf(orbitAngle) * orbitRadius;
        camera.position.z = sinf(orbitAngle) * orbitRadius;
        camera.position.y = orbitHeight + sinf(orbitAngle * 0.5f) * 1.2f;

        SortSplats(splats, camera.position);

        Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

        BeginDrawing();
        ClearBackground({8, 9, 14, 255});

        BeginMode3D(camera);

        rlPushMatrix();
        rlTranslatef(0, -2.6f, 0);
        DrawGrid(24, 0.5f);
        rlPopMatrix();

        BeginShaderMode(shader);
        rlSetTexture(rlGetTextureIdDefault());
        BeginBlendMode(BLEND_ALPHA);

        rlBegin(RL_QUADS);
        for (const auto& s : splats) {
            DrawSplatQuad(s.position, s.size * sizeScale,
                          s.r, s.g, s.b, s.a, view);
        }
        rlEnd();

        EndBlendMode();
        EndShaderMode();

        EndMode3D();

        if (showHelp) {
            DrawRectangleRounded({14, 14, 420, 108}, 0.12f, 6, Fade({0,0,0,200}, 0.7f));
            DrawText("Gaussian Splat Demo", 28, 24, 22, WHITE);
            DrawText(TextFormat("Splats: %i   Size: %.1fx", (int)splats.size(), sizeScale), 28, 52, 18, LIGHTGRAY);
            DrawText("R  auto-rotate   Up/Down  splat size   H  hide help", 28, 76, 16, GRAY);
            DrawText("Back-to-front sorted alpha blending", 28, 98, 14, (Color){100,160,255,200});
        }
        DrawFPS(GetScreenWidth() - 90, 18);

        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();
    return 0;
}
