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

static unsigned int rngState = 12345;
static float frand01() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return (float)(rngState & 0x00FFFFFF) / (float)0x01000000;
}
static float frand(float lo, float hi) { return lo + (hi - lo) * frand01(); }

// Uniform point inside ellipsoid (a,b,c) semi-axes
static Vector3 randEllipsoid(float a, float b, float c) {
    float u, v, w, len;
    do {
        u = frand(-1,1); v = frand(-1,1); w = frand(-1,1);
        len = sqrtf(u*u+v*v+w*w);
    } while (len < 1e-5f || len > 1.0f);
    float t = cbrtf(frand01());
    return {u/len*a*t, v/len*b*t, w/len*c*t};
}

static void addLobe(std::vector<Splat>& s, Vector3 centre,
                    float ax, float ay, float az, int n,
                    float r, float g, float b, float aMin, float aMax,
                    float szMin, float szMax,
                    float rVar=0.04f, float gVar=0.04f, float bVar=0.04f) {
    for (int i = 0; i < n; ++i) {
        Vector3 p = randEllipsoid(ax, ay, az);
        float dist = sqrtf((p.x/ax)*(p.x/ax)+(p.y/ay)*(p.y/ay)+(p.z/az)*(p.z/az));
        float alpha = (1.0f - dist*0.5f) * frand(aMin, aMax);
        s.push_back({
            {centre.x+p.x, centre.y+p.y, centre.z+p.z},
            Clamp(r+frand(-rVar,rVar),0,1),
            Clamp(g+frand(-gVar,gVar),0,1),
            Clamp(b+frand(-bVar,bVar),0,1),
            alpha,
            frand(szMin, szMax)
        });
    }
}

static std::vector<Splat> BuildBust() {
    std::vector<Splat> s;
    s.reserve(15000);

    // --- NECK / BUST base ---
    addLobe(s, {0,-2.1f,0},      0.55f,0.55f,0.45f, 800,  0.80f,0.62f,0.50f, 0.4f,0.7f, 0.06f,0.12f);
    addLobe(s, {0,-3.0f,0},      1.1f, 0.45f,0.85f, 1200, 0.78f,0.60f,0.48f, 0.4f,0.7f, 0.07f,0.14f);

    // --- SKULL / HEAD volume ---
    addLobe(s, {0, 0.1f, 0},     1.15f,1.35f,1.05f, 3000, 0.82f,0.64f,0.51f, 0.45f,0.75f, 0.055f,0.11f);

    // --- FOREHEAD slightly narrower ---
    addLobe(s, {0, 1.1f,-0.05f}, 1.0f, 0.55f,0.88f, 800,  0.84f,0.66f,0.52f, 0.4f,0.65f, 0.05f,0.095f);

    // --- CHEEKS ---
    addLobe(s, {-0.72f,-0.2f, 0.55f}, 0.42f,0.38f,0.32f, 600, 0.88f,0.63f,0.51f, 0.4f,0.7f, 0.045f,0.09f);
    addLobe(s, { 0.72f,-0.2f, 0.55f}, 0.42f,0.38f,0.32f, 600, 0.88f,0.63f,0.51f, 0.4f,0.7f, 0.045f,0.09f);

    // --- NOSE bridge ---
    addLobe(s, {0, 0.0f, 0.95f},      0.12f,0.35f,0.14f, 350, 0.83f,0.63f,0.50f, 0.5f,0.8f, 0.035f,0.07f);
    // Nose tip
    addLobe(s, {0,-0.28f, 1.08f},     0.16f,0.12f,0.14f, 250, 0.85f,0.60f,0.49f, 0.5f,0.8f, 0.03f,0.065f);
    // Nostrils
    addLobe(s, {-0.14f,-0.35f,1.02f}, 0.08f,0.06f,0.07f, 120, 0.70f,0.46f,0.38f, 0.6f,0.85f,0.025f,0.05f);
    addLobe(s, { 0.14f,-0.35f,1.02f}, 0.08f,0.06f,0.07f, 120, 0.70f,0.46f,0.38f, 0.6f,0.85f,0.025f,0.05f);

    // --- LIPS ---
    // Upper lip
    addLobe(s, {0,-0.65f, 0.96f},     0.30f,0.09f,0.10f, 280, 0.82f,0.45f,0.42f, 0.55f,0.85f,0.03f,0.06f);
    // Lower lip (fuller)
    addLobe(s, {0,-0.82f, 0.93f},     0.28f,0.10f,0.10f, 300, 0.80f,0.42f,0.40f, 0.55f,0.85f,0.032f,0.065f);
    // Mouth corners
    addLobe(s, {-0.30f,-0.72f,0.88f}, 0.07f,0.07f,0.07f, 80,  0.68f,0.38f,0.36f, 0.6f,0.85f,0.025f,0.05f);
    addLobe(s, { 0.30f,-0.72f,0.88f}, 0.07f,0.07f,0.07f, 80,  0.68f,0.38f,0.36f, 0.6f,0.85f,0.025f,0.05f);

    // --- EYE SOCKETS (dark recesses) ---
    addLobe(s, {-0.38f, 0.28f, 0.88f}, 0.22f,0.14f,0.10f, 300, 0.25f,0.20f,0.18f, 0.55f,0.80f,0.028f,0.055f);
    addLobe(s, { 0.38f, 0.28f, 0.88f}, 0.22f,0.14f,0.10f, 300, 0.25f,0.20f,0.18f, 0.55f,0.80f,0.028f,0.055f);
    // Irises (blue-grey)
    addLobe(s, {-0.38f, 0.28f, 0.96f}, 0.10f,0.10f,0.04f, 180, 0.28f,0.45f,0.72f, 0.6f,0.9f, 0.022f,0.045f);
    addLobe(s, { 0.38f, 0.28f, 0.96f}, 0.10f,0.10f,0.04f, 180, 0.28f,0.45f,0.72f, 0.6f,0.9f, 0.022f,0.045f);
    // Pupils
    addLobe(s, {-0.38f, 0.28f, 0.98f}, 0.055f,0.055f,0.02f, 80, 0.05f,0.05f,0.06f, 0.7f,0.95f,0.018f,0.035f);
    addLobe(s, { 0.38f, 0.28f, 0.98f}, 0.055f,0.055f,0.02f, 80, 0.05f,0.05f,0.06f, 0.7f,0.95f,0.018f,0.035f);
    // Eye whites
    addLobe(s, {-0.38f, 0.28f, 0.94f}, 0.14f,0.09f,0.03f, 150, 0.92f,0.91f,0.89f, 0.5f,0.8f, 0.02f,0.042f);
    addLobe(s, { 0.38f, 0.28f, 0.94f}, 0.14f,0.09f,0.03f, 150, 0.92f,0.91f,0.89f, 0.5f,0.8f, 0.02f,0.042f);

    // --- EYEBROWS ---
    addLobe(s, {-0.38f, 0.52f, 0.85f}, 0.25f,0.06f,0.07f, 160, 0.22f,0.17f,0.13f, 0.65f,0.9f, 0.022f,0.045f);
    addLobe(s, { 0.38f, 0.52f, 0.85f}, 0.25f,0.06f,0.07f, 160, 0.22f,0.17f,0.13f, 0.65f,0.9f, 0.022f,0.045f);

    // --- EARS ---
    addLobe(s, {-1.18f, 0.0f, 0.0f},  0.12f,0.22f,0.10f, 350, 0.80f,0.56f,0.44f, 0.4f,0.7f, 0.035f,0.07f);
    addLobe(s, { 1.18f, 0.0f, 0.0f},  0.12f,0.22f,0.10f, 350, 0.80f,0.56f,0.44f, 0.4f,0.7f, 0.035f,0.07f);

    // --- HAIR: top and back of skull ---
    addLobe(s, {0, 1.55f, -0.1f},     1.10f,0.5f, 0.95f, 1200, 0.14f,0.10f,0.08f, 0.4f,0.75f,0.06f,0.13f);
    addLobe(s, {0, 0.9f, -0.95f},     0.9f, 0.7f, 0.35f, 800,  0.13f,0.09f,0.07f, 0.4f,0.7f, 0.055f,0.11f);
    // Hair wisps / highlights
    addLobe(s, {0.3f, 1.7f, 0.2f},    0.35f,0.18f,0.3f,  200,  0.38f,0.28f,0.20f, 0.3f,0.55f,0.04f,0.085f);
    addLobe(s, {-0.3f,1.7f, 0.2f},    0.35f,0.18f,0.3f,  200,  0.38f,0.28f,0.20f, 0.3f,0.55f,0.04f,0.085f);

    // --- SUBSURFACE SCATTER rim (warm translucent glow on ear / jaw edges) ---
    addLobe(s, {-1.05f, -0.1f, 0.2f}, 0.18f,0.35f,0.15f, 250, 0.96f,0.52f,0.32f, 0.15f,0.35f,0.07f,0.14f);
    addLobe(s, { 1.05f, -0.1f, 0.2f}, 0.18f,0.35f,0.15f, 250, 0.96f,0.52f,0.32f, 0.15f,0.35f,0.07f,0.14f);

    return s;
}

static void SortSplats(std::vector<Splat>& splats, Vector3 camPos) {
    std::sort(splats.begin(), splats.end(), [&](const Splat& a, const Splat& b) {
        return Vector3DistanceSqr(a.position, camPos) > Vector3DistanceSqr(b.position, camPos);
    });
}

static void DrawSplatQuad(const Splat& s, float scale, Matrix& view) {
    Vector3 right = {view.m0, view.m4, view.m8};
    Vector3 up    = {view.m1, view.m5, view.m9};
    float   sz    = s.size * scale;
    Vector3 pos   = s.position;

    Vector3 v0 = Vector3Add(pos, Vector3Add(Vector3Scale(right,-sz), Vector3Scale(up,-sz)));
    Vector3 v1 = Vector3Add(pos, Vector3Add(Vector3Scale(right, sz), Vector3Scale(up,-sz)));
    Vector3 v2 = Vector3Add(pos, Vector3Add(Vector3Scale(right, sz), Vector3Scale(up, sz)));
    Vector3 v3 = Vector3Add(pos, Vector3Add(Vector3Scale(right,-sz), Vector3Scale(up, sz)));

    rlColor4f(s.r, s.g, s.b, s.a);
    rlTexCoord2f(0,0); rlVertex3f(v0.x,v0.y,v0.z);
    rlTexCoord2f(1,0); rlVertex3f(v1.x,v1.y,v1.z);
    rlTexCoord2f(1,1); rlVertex3f(v2.x,v2.y,v2.z);
    rlTexCoord2f(0,1); rlVertex3f(v3.x,v3.y,v3.z);
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Gaussian Splat Demo — Human Bust");
    SetTargetFPS(144);

    Camera3D camera = {};
    camera.position   = {0.0f, 0.5f, 6.5f};
    camera.target     = {0.0f, 0.0f, 0.0f};
    camera.up         = {0.0f, 1.0f, 0.0f};
    camera.fovy       = 38.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadShader("shaders/splat.vs", "shaders/splat.fs");

    std::vector<Splat> splats = BuildBust();

    bool  autoRotate = true;
    bool  showHelp   = true;
    float orbitAngle = 0.0f;
    float sizeScale  = 1.0f;
    float orbitR     = 6.5f;
    float orbitH     = 0.5f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (IsKeyPressed(KEY_R))    autoRotate = !autoRotate;
        if (IsKeyPressed(KEY_H))    showHelp   = !showHelp;
        if (IsKeyPressed(KEY_UP))   sizeScale  = fminf(sizeScale+0.1f, 3.0f);
        if (IsKeyPressed(KEY_DOWN)) sizeScale  = fmaxf(sizeScale-0.1f, 0.2f);

        if (autoRotate) orbitAngle += dt * 0.25f;

        camera.position = {
            cosf(orbitAngle) * orbitR,
            orbitH,
            sinf(orbitAngle) * orbitR
        };

        SortSplats(splats, camera.position);
        Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

        BeginDrawing();
        ClearBackground({10, 10, 14, 255});

        BeginMode3D(camera);

        rlPushMatrix();
        rlTranslatef(0, -3.2f, 0);
        DrawGrid(30, 0.4f);
        rlPopMatrix();

        BeginShaderMode(shader);
        rlSetTexture(rlGetTextureIdDefault());
        BeginBlendMode(BLEND_ALPHA);
        rlBegin(RL_QUADS);
        for (const auto& s : splats)
            DrawSplatQuad(s, sizeScale, view);
        rlEnd();
        EndBlendMode();
        EndShaderMode();

        EndMode3D();

        if (showHelp) {
            DrawRectangleRounded({14,14,460,110}, 0.12f, 6, Fade({0,0,0,200}, 0.72f));
            DrawText("Gaussian Splat Demo — Human Bust", 28, 22, 20, WHITE);
            DrawText(TextFormat("Splats: %i   Size: %.1fx", (int)splats.size(), sizeScale), 28, 50, 18, LIGHTGRAY);
            DrawText("R  auto-rotate   Up/Down  splat size   H  hide", 28, 74, 16, GRAY);
            DrawText("Alpha-blended Gaussian quads, back-to-front sorted", 28, 96, 14, {100,160,255,200});
        }
        DrawFPS(GetScreenWidth()-90, 18);
        EndDrawing();
    }

    UnloadShader(shader);
    CloseWindow();
    return 0;
}
