#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define TARGET_FPS 100
#define W 320
#define H 200

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

Texture2D gpu_data;
Color cpu_data[W * H];
Image noise_image;
Color *noise_data;

extern inline double dist(double x1, double y1, double x2, double y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void draw_noise (double time)
{
    double value = 0;
    double t = 100 + time * 20.0;

    for(int y = 0; y < H; y++) {
        for(int x = 0; x < W; x++) {

            value = 0;
            double d1 = dist(x + t, y, W, H) / 17.0;
            double d2 = dist(x, y + t * 2.0, W / 2.0, H / 2.0) / 14.0;
            double d3 = dist(x, y + t * 1.0, W * 2, H * 2) / 13.0;
            double d4 = dist(x + t, y, 0, 0) / 12.0;

            value += (sin(d1) + sin(d2) + sin(d3) + sin(d4));

            Color noise_color = noise_data[y * W + x];
            float xoffs = (sin(t / 100.0f + sin(t / 177.0))*W*2.0);
            Color c = ColorFromHSV(CLAMP(value, 90.0, 180.0), 1.0, 1.0);
            c.r += dist(x + xoffs, c.r, noise_color.r, noise_color.r * 3.0);
            c.g += dist(x - xoffs, y + (sin(t / 100.0f)*H), noise_color.g, noise_color.g * 4.0);
            c.b += dist(x + xoffs, c.b, noise_color.b, noise_color.b * 8.0);

            c.r += (c.r + noise_color.r) / 2.0;
            c.g += (c.g + noise_color.g) / 2.0;
            c.b += (c.b + noise_color.b) / 2.0;

            int a = 155 + (sin(t / 300.0)*.5+.5) * 100.0;

            cpu_data[y * W + x] = (Color) { c.r, c.g, c.b, a};
        }
    }
}

void main_loop_body()
{
    double time;
    time = GetTime();

    if (IsKeyPressed(KEY_F) || IsGestureDetected(GESTURE_TAP)) {
        if (IsWindowFullscreen()) {
            RestoreWindow();
        } else {
            ToggleBorderlessWindowed();
        }
    }

    draw_noise(time);

    BeginDrawing();
        ClearBackground(WHITE);
        UpdateTexture(gpu_data, cpu_data);
        Rectangle source = {0, 0, W, H};
        Rectangle dest = {0, 0, GetRenderWidth(), GetRenderHeight()};
        Vector2 origin = {0, 0};
        DrawTexturePro(gpu_data, source, dest, origin, 0.0, WHITE);
        //DrawFPS(10, 10);
    EndDrawing();
}

int main(int argc, char * argv[])
{
    InitWindow(W, H, "software plasma");

    Image img = {
        .width = W,
        .height = H,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    img.data = (Color *)malloc(W * H * sizeof(Color));
    memcpy(img.data, cpu_data, (W * H * sizeof(Color)));
    gpu_data = LoadTextureFromImage(img);
    UnloadImage(img);

    noise_image = GenImagePerlinNoise(W, H, 0, 0, 1.0);
    noise_data = LoadImageColors(noise_image);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(main_loop_body, 120, 1);
#else

    SetTargetFPS(TARGET_FPS);
    DisableCursor();
    ToggleBorderlessWindowed();
    while (!WindowShouldClose()) {
        main_loop_body();
    }
    EnableCursor();
#endif
    UnloadImageColors(noise_data);
    UnloadImage(noise_image);

    CloseWindow();
    return 0;
}
