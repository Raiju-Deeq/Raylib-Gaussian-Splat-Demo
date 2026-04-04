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

static unsigned int rngState = 98765;
static float frand01() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return (float)(rngState & 0x00FFFFFF) / (float)0x01000000;
}
static float frand(float lo, float hi) { return lo + (hi - lo) * frand01(); }

static void addLobe(std::vector<Splat>& s, Vector3 centre,
                    float ax, float ay, float az, int n,
                    float r, float g, float b,
                    float aMin, float aMax,
                    float szMin, float szMax,
                    float rV=0.04f, float gV=0.04f, float bV=0.04f) {
    for (int i = 0; i < n; ++i) {
        float u, v, w, len;
        do {
            u = frand(-1,1); v = frand(-1,1); w = frand(-1,1);
            len = sqrtf(u*u+v*v+w*w);
        } while (len < 1e-5f || len > 1.0f);
        float t  = cbrtf(frand01());
        float px = centre.x + (u/len)*ax*t;
        float py = centre.y + (v/len)*ay*t;
        float pz = centre.z + (w/len)*az*t;
        float d  = sqrtf((px-centre.x)*(px-centre.x)/(ax*ax) +
                         (py-centre.y)*(py-centre.y)/(ay*ay) +
                         (pz-centre.z)*(pz-centre.z)/(az*az));
        float alpha = (1.0f - d*0.55f) * frand(aMin, aMax);
        s.push_back({
            {px, py, pz},
            Clamp(r + frand(-rV, rV), 0.f, 1.f),
            Clamp(g + frand(-gV, gV), 0.f, 1.f),
            Clamp(b + frand(-bV, bV), 0.f, 1.f),
            alpha,
            frand(szMin, szMax)
        });
    }
}

static std::vector<Splat> BuildBunny() {
    std::vector<Splat> s;
    s.reserve(50000);

    // BODY
    addLobe(s, {0.0f, 0.0f, 0.0f},     1.05f,1.30f,0.90f, 12000, 0.82f,0.76f,0.68f, 0.40f,0.72f, 0.030f,0.060f);
    // HAUNCHES
    addLobe(s, {0.0f,-0.55f,-0.45f},   1.0f, 0.80f,0.75f,  5000, 0.80f,0.74f,0.66f, 0.38f,0.68f, 0.028f,0.055f);
    // CHEST
    addLobe(s, {0.0f, 0.30f, 0.60f},   0.68f,0.70f,0.45f,  3500, 0.84f,0.78f,0.70f, 0.38f,0.65f, 0.025f,0.050f);
    // NECK
    addLobe(s, {0.0f, 1.35f, 0.25f},   0.38f,0.42f,0.32f,  2500, 0.83f,0.77f,0.69f, 0.40f,0.68f, 0.022f,0.045f);
    // HEAD
    addLobe(s, {0.0f, 1.95f, 0.20f},   0.72f,0.68f,0.62f,  6000, 0.85f,0.79f,0.71f, 0.42f,0.72f, 0.022f,0.045f);
    // SNOUT
    addLobe(s, {0.0f, 1.72f, 0.72f},   0.28f,0.20f,0.24f,  2000, 0.87f,0.80f,0.72f, 0.42f,0.72f, 0.018f,0.036f);
    // NOSE TIP
    addLobe(s, {0.0f, 1.65f, 0.90f},   0.09f,0.07f,0.08f,   600, 0.90f,0.58f,0.56f, 0.50f,0.80f, 0.014f,0.026f);
    // EYES
    addLobe(s, {-0.28f,2.02f, 0.58f},  0.10f,0.08f,0.05f,   500, 0.12f,0.08f,0.07f, 0.60f,0.88f, 0.012f,0.022f);
    addLobe(s, { 0.28f,2.02f, 0.58f},  0.10f,0.08f,0.05f,   500, 0.12f,0.08f,0.07f, 0.60f,0.88f, 0.012f,0.022f);
    // EYE GLINTS
    addLobe(s, {-0.26f,2.04f,0.62f},   0.04f,0.04f,0.02f,   120, 0.95f,0.95f,0.95f, 0.65f,0.90f, 0.008f,0.016f);
    addLobe(s, { 0.26f,2.04f,0.62f},   0.04f,0.04f,0.02f,   120, 0.95f,0.95f,0.95f, 0.65f,0.90f, 0.008f,0.016f);
    // EARS
    addLobe(s, {-0.30f,2.90f, 0.0f},   0.14f,0.80f,0.10f,  3000, 0.82f,0.72f,0.64f, 0.35f,0.65f, 0.018f,0.038f);
    addLobe(s, { 0.30f,2.90f, 0.0f},   0.14f,0.80f,0.10f,  3000, 0.82f,0.72f,0.64f, 0.35f,0.65f, 0.018f,0.038f);
    // INNER EAR (pink)
    addLobe(s, {-0.30f,2.90f,0.04f},   0.07f,0.68f,0.05f,  1000, 0.92f,0.60f,0.58f, 0.25f,0.50f, 0.014f,0.028f);
    addLobe(s, { 0.30f,2.90f,0.04f},   0.07f,0.68f,0.05f,  1000, 0.92f,0.60f,0.58f, 0.25f,0.50f, 0.014f,0.028f);
    // FRONT LEGS
    addLobe(s, {-0.42f,-0.60f,0.55f},  0.20f,0.68f,0.20f,  1800, 0.80f,0.74f,0.66f, 0.36f,0.64f, 0.022f,0.042f);
    addLobe(s, { 0.42f,-0.60f,0.55f},  0.20f,0.68f,0.20f,  1800, 0.80f,0.74f,0.66f, 0.36f,0.64f, 0.022f,0.042f);
    // FRONT PAWS
    addLobe(s, {-0.44f,-1.25f,0.60f},  0.18f,0.12f,0.20f,   600, 0.78f,0.72f,0.65f, 0.40f,0.68f, 0.018f,0.034f);
    addLobe(s, { 0.44f,-1.25f,0.60f},  0.18f,0.12f,0.20f,   600, 0.78f,0.72f,0.65f, 0.40f,0.68f, 0.018f,0.034f);
    // BACK LEGS
    addLobe(s, {-0.58f,-0.20f,-0.50f}, 0.32f,0.60f,0.30f,  1800, 0.79f,0.73f,0.65f, 0.36f,0.64f, 0.024f,0.046f);
    addLobe(s, { 0.58f,-0.20f,-0.50f}, 0.32f,0.60f,0.30f,  1800, 0.79f,0.73f,0.65f, 0.36f,0.64f, 0.024f,0.046f);
    // BACK FEET
    addLobe(s, {-0.55f,-1.22f,-0.10f}, 0.22f,0.12f,0.38f,   700, 0.77f,0.71f,0.63f, 0.38f,0.65f, 0.018f,0.035f);
    addLobe(s, { 0.55f,-1.22f,-0.10f}, 0.22f,0.12f,0.38f,   700, 0.77f,0.71f,0.63f, 0.38f,0.65f, 0.018f,0.035f);
    // TAIL
    addLobe(s, {0.0f,-0.30f,-1.05f},   0.22f,0.20f,0.18f,   800, 0.96f,0.95f,0.94f, 0.35f,0.60f, 0.018f,0.036f);
    // SUBSURFACE SCATTER — warm rim on ears
    addLobe(s, {-0.30f,2.88f,0.05f},   0.16f,0.75f,0.06f,   500, 0.98f,0.55f,0.30f, 0.08f,0.22f, 0.030f,0.060f);
    addLobe(s, { 0.30f,2.88f,0.05f},   0.16f,0.75f,0.06f,   500, 0.98f,0.55f,0.30f, 0.08f,0.22f, 0.030f,0.060f);
    // SUBSURFACE SCATTER — cheek glow
    addLobe(s, {-0.60f,1.85f,0.30f},   0.22f,0.28f,0.16f,   350, 0.95f,0.52f,0.28f, 0.06f,0.18f, 0.030f,0.055f);
    addLobe(s, { 0.60f,1.85f,0.30f},   0.22f,0.28f,0.16f,   350, 0.95f,0.52f,0.28f, 0.06f,0.18f, 0.030f,0.055f);

    return s;
}

static void SortSplats(std::vector<Splat>& splats, Vector3 cam) {
    std::sort(splats.begin(), splats.end(), [&](const Splat& a, const Splat& b) {
        return Vector3DistanceSqr(a.position, cam) > Vector3DistanceSqr(b.position, cam);
    });
}

static void DrawSplatQuad(const Splat& s, float scale, Matrix& view) {
    Vector3 right = {view.m0, view.m4, view.m8};
    Vector3 up    = {view.m1, view.m5, view.m9};
    float   sz    = s.size * scale;
    Vector3 p     = s.position;

    Vector3 v0 = Vector3Add(p, Vector3Add(Vector3Scale(right,-sz), Vector3Scale(up,-sz)));
    Vector3 v1 = Vector3Add(p, Vector3Add(Vector3Scale(right, sz), Vector3Scale(up,-sz)));
    Vector3 v2 = Vector3Add(p, Vector3Add(Vector3Scale(right, sz), Vector3Scale(up, sz)));
    Vector3 v3 = Vector3Add(p, Vector3Add(Vector3Scale(right,-sz), Vector3Scale(up, sz)));

    rlColor4f(s.r, s.g, s.b, s.a);
    rlTexCoord2f(0,0); rlVertex3f(v0.x,v0.y,v0.z);
    rlTexCoord2f(1,0); rlVertex3f(v1.x,v1.y,v1.z);
    rlTexCoord2f(1,1); rlVertex3f(v2.x,v2.y,v2.z);
    rlTexCoord2f(0,1); rlVertex3f(v3.x,v3.y,v3.z);
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Gaussian Splat Demo - Stanford Bunny");
    SetTargetFPS(144);

    Camera3D camera = {};
    camera.position   = {0.0f, 1.0f, 7.5f};
    camera.target     = {0.0f, 0.8f, 0.0f};
    camera.up         = {0.0f, 1.0f, 0.0f};
    camera.fovy       = 36.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadShader("shaders/splat.vs", "shaders/splat.fs");

    std::vector<Splat> splats = BuildBunny();

    bool  autoRotate = true;
    bool  showHelp   = true;
    float orbitAngle = 0.0f;
    float sizeScale  = 1.0f;
    const float orbitR = 7.5f;
    const float orbitH = 1.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (IsKeyPressed(KEY_R))    autoRotate = !autoRotate;
        if (IsKeyPressed(KEY_H))    showHelp   = !showHelp;
        if (IsKeyPressed(KEY_UP))   sizeScale  = fminf(sizeScale+0.1f, 3.0f);
        if (IsKeyPressed(KEY_DOWN)) sizeScale  = fmaxf(sizeScale-0.1f, 0.2f);

        if (autoRotate) orbitAngle += dt * 0.28f;

        camera.position = {
            cosf(orbitAngle) * orbitR,
            orbitH + sinf(orbitAngle * 0.4f) * 0.6f,
            sinf(orbitAngle) * orbitR
        };

        SortSplats(splats, camera.position);
        Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

        BeginDrawing();
        ClearBackground({8, 9, 12, 255});

        BeginMode3D(camera);

        rlPushMatrix();
        rlTranslatef(0, -1.35f, 0);
        DrawGrid(28, 0.4f);
        rlPopMatrix();

        BeginShaderMode(shader);
        rlSetTexture(rlGetTextureIdDefault());
        BeginBlendMode(BLEND_ALPHA);
        rlBegin(RL_QUADS);
        for (const auto& sp : splats)
            DrawSplatQuad(sp, sizeScale, view);
        rlEnd();
        EndBlendMode();
        EndShaderMode();

        EndMode3D();

        if (showHelp) {
            DrawRectangleRounded({14,14,480,112}, 0.12f, 6, Fade({0,0,0,200}, 0.72f));
            DrawText("Gaussian Splat Demo - Stanford Bunny", 28, 22, 20, WHITE);
            DrawText(TextFormat("Splats: %i   Size: %.1fx", (int)splats.size(), sizeScale), 28, 50, 18, LIGHTGRAY);
            DrawText("R  auto-rotate   Up/Down  splat size   H  hide", 28, 74, 16, GRAY);
            DrawText("50k alpha Gaussian quads, back-to-front sorted", 28, 96, 14, (Color){100,160,255,200});
        }
        DrawFPS(GetScreenWidth()-90, 18);
        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();
    return 0;
}
