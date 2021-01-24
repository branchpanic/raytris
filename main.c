#include <raylib.h>

#include "graphics.h"
#include "raytris.h"

int main(int argc, char const *argv[]) {
    InitWindow(600, 800, "raytris");
    SetTargetFPS(60);

    game_t game = {0};

    settings_t settings = SETTINGS_DEFAULT;
    settings_load_colors(&settings, "resources/colors.txt");

    game_reset(&game, settings);

    while (!WindowShouldClose()) {
        double time = GetTime();

        if (!game_update(&game, time)) {
            if (IsKeyPressed(settings.bindings.key_reset)) {
                game_reset(&game, settings);
            }
        }

        BeginDrawing();
        ClearBackground(game.settings.palette.bg_color);
        draw_game(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
