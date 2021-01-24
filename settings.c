#include <stdlib.h>
#include <string.h>

#include "settings.h"

void settings_load_colors(settings_t *s, const char *colors_txt_path) {
    char *text = LoadFileText(colors_txt_path);
    char *token = strtok(text, "\n");

    int i = 0;
    while (token != NULL) {
        s->palette.block_colors[i] = GetColor(strtoul(token, NULL, 16));
        token = strtok(NULL, "\n");
        i++;
        if (i >= TM_COUNT)
            break;
    }

    for (; i < TM_COUNT; i++) {
        s->palette.block_colors[i] = RED;
    }

    UnloadFileText(text);
}

const settings_t SETTINGS_DEFAULT = {
    .fast_fall_rate = 0.1,
    .das_delay = 0.12,
    .das_rate = 0.01,

    .palette =
        {
            .block_colors = {WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE},
            .bg_color = {.r = 0, .g = 0, .b = 0, .a = 255},
            .grid_color = {.r = 255, .g = 255, .b = 255, .a = 128},
        },

    .bindings =
        {
            .key_soft_drop = KEY_DOWN,
            .key_hard_drop = KEY_SPACE,
            .key_left = KEY_LEFT,
            .key_right = KEY_RIGHT,
            .key_rotate_cw = KEY_X,
            .key_rotate_ccw = KEY_Z,
            .key_hold = KEY_C,
            .key_reset = KEY_Q,
        },
};
