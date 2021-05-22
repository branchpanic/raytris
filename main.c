#include <raylib.h>
#include <stdio.h>

#include "graphics.h"
#include "raytris.h"

int main(int argc, char const *argv[]) {
    InitWindow(600, 800, "raytris");
    SetTargetFPS(60);

    game_t game = {0};

    settings_t settings = SETTINGS_DEFAULT;

    printf("Loading\n");
    if (!settings_load(&settings, "resources/raytris.ini")) {
        printf("Failed to read settings\n");
        for (int i = 0; i < TM_COUNT; i++) {
            settings.palette.block_colors[i] = WHITE;
        }
    }

    game.settings = settings;
    game_init(&game);

    while (!WindowShouldClose()) {
        double time = GetTime();

        if (!game_update(&game, time)) {
            if (IsKeyPressed(settings.bindings.key_reset)) {
                game_reset(&game);
            }
        }

        BeginDrawing();
        ClearBackground(game.settings.palette.bg_color);
        draw_game(&game, time);
        EndDrawing();
    }

    game_free(&game);
    CloseWindow();
    return 0;
}
