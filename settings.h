#ifndef RAYTRIS_SETTINGS_H_
#define RAYTRIS_SETTINGS_H_

#include "tetromino.h" // TM_COUNT

#include <raylib.h>

typedef struct bindings {
    int key_soft_drop;
    int key_hard_drop;
    int key_left;
    int key_right;
    int key_rotate_cw;
    int key_rotate_ccw;
    int key_hold;
    int key_reset;
} bindings_t;

typedef struct palette {
    Color block_colors[TM_COUNT];
    Color bg_color;
    Color grid_color;
} palette_t;

// Settings contains configurable game settings.
// TODO: load/save from INI
typedef struct settings {
    double fast_fall_rate;
    double das_delay;
    double das_rate;

    palette_t palette;
    bindings_t bindings;
} settings_t;

const settings_t SETTINGS_DEFAULT;

void settings_load_colors(settings_t* s, const char* colors_txt_path);

#endif