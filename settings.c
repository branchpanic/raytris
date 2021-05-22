#include <errno.h>
#include <ini.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"

const char *BLOCK_ORDER = "IJLOSTZ";

#define SECTION_IS(S) (strcmp(section, (S)) == 0)
#define KEY_IS(K) (strcmp(name, (K)) == 0)
#define USE_VALUE_AS_COLOR(K)                                                  \
    {                                                                          \
        errno = 0;                                                             \
        (K) = GetColor((int)strtoul(value, NULL, 16));                         \
        if (errno != 0)                                                        \
            return errno;                                                      \
    }

static int ini_callback(void *user, const char *section, const char *name,
                        const char *value) {
    settings_t *settings = (settings_t *)user;

    if (SECTION_IS("palette")) {
        if (strlen(name) != 1) {
            if (KEY_IS("background"))
                USE_VALUE_AS_COLOR(settings->palette.bg_color);
            if (KEY_IS("grid"))
                USE_VALUE_AS_COLOR(settings->palette.grid_color);
        } else {
            for (int i = 0; i < TM_COUNT; i++) {
                if (name[0] == BLOCK_ORDER[i]) {
                    USE_VALUE_AS_COLOR(settings->palette.block_colors[i]);
                }
            }
        }
    }

    if (SECTION_IS("shaders")) {
        if (KEY_IS("background")) {
            settings->bg_shader_name = _strdup(value);
        }
    }

    return 0;
}

#undef USE_VALUE_AS_COLOR
#undef USE_VALUE_AS_STRING
#undef KEY_IS
#undef SECTION_IS

int settings_load(settings_t *s, const char *path) {
    return ini_parse(path, &ini_callback, (void *)s);
}

const settings_t SETTINGS_DEFAULT = {
    .fast_fall_rate = 0.1,
    .das_delay = 0.12,
    .das_rate = 0.01,

    .palette =
        {
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

    .bg_shader_name = NULL  // Raylib will interpret this as "no shader"
};
