#include "raytris.h"

#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

void board_clear(board_t *board, int y0, int y1) {
    int *data = (int *)(board->data);
    memmove(data + (y1 - y0) * BOARD_WIDTH, data,
            y0 * BOARD_WIDTH * sizeof(int));
    memset(data, 0, (y1 - y0) * BOARD_WIDTH * sizeof(int));
}

void board_place(board_t *board, tetromino_t *piece, int x, int y) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;
            board->data[y + j][x + i] = piece->shape[i][j];
        }
    }
}

bool board_collides(board_t *board, tetromino_t *piece, int x, int y) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;

            if (x + i < 0 || x + i >= BOARD_WIDTH || y + j < 0 ||
                y + j >= BOARD_HEIGHT || board->data[y + j][x + i] != 0) {
                return true;
            }
        }
    }

    return false;
}

void game_advance_piece(game_t *game) {
    game->bag_current++;
    if (game->bag_current >= TM_COUNT) {
        game->bag_current = 0;
        memcpy(game->bag, game->next_bag, TM_COUNT * sizeof(struct tetromino));
        choose_sequence(game->next_bag);
    }

    game->falling = game->bag[game->bag_current];
    game->falling_y = BOARD_VISIBLE;
    game->falling_x = (BOARD_WIDTH - game->falling.size) / 2;

    if (board_collides(&game->board, &game->falling, game->falling_x,
                       game->falling_y)) {
        game->over = true;
    }
}

void game_swap_held_piece(game_t *game) {
    if (game->has_held) {
        struct tetromino temp = game->falling;
        game->falling = game->held;
        game->held = temp;
        game->falling_y = BOARD_VISIBLE;
        game->falling_x = (BOARD_WIDTH - game->falling.size) / 2;
        if (board_collides(&game->board, &game->falling, game->falling_x,
                           game->falling_y)) {
            game->over = true;
        }
    } else {
        game->held = game->falling;
        game->has_held = true;
        game_advance_piece(game);
    }
}

static void handle_shift(struct game *game, int key, int x_offset,
                         double time) {
    if (IsKeyPressed(key)) {
        game->move_start = time;
        if (!board_collides(&game->board, &game->falling,
                            game->falling_x + x_offset, game->falling_y)) {
            game->falling_x += x_offset;
        }
    } else if (IsKeyDown(key) &&
               time - game->move_start >= game->settings.das_delay) {
        if (time - game->last_das >= game->settings.das_rate) {
            if (!board_collides(&game->board, &game->falling,
                                game->falling_x + x_offset, game->falling_y)) {
                game->falling_x += x_offset;
            }
            game->last_das = time;
        }
    }
}

void game_init(game_t *game) {
    game_reset(game);
    game_reload_shaders(game);
}

void game_reload_shaders(game_t *game) {
    game->bg_shader = LoadShader(0, game->settings.bg_shader_name);

#define FIND_LOC(X)                                                            \
    game->shader_info.X##_loc = GetShaderLocation(game->bg_shader, "u_" #X);

    FIND_LOC(block_size)
    FIND_LOC(height)
    FIND_LOC(resolution)
    FIND_LOC(over_time)
    FIND_LOC(time)

#undef FIND_LOC
}

void game_free(game_t *game) {
    UnloadShader(game->bg_shader);
}

void game_reset(game_t *game) {
    memset(&game->board, 0, sizeof(board_t));

    choose_sequence(game->bag);
    choose_sequence(game->next_bag);

    game->falling = game->bag[game->bag_current];
    game->falling_y = BOARD_VISIBLE;
    game->falling_x = (BOARD_WIDTH - game->falling.size) / 2;

    game->held = (tetromino_t){0};
    game->has_held = false;
    game->used_hold = false;

    game->last_fall = 0.0;
    game->fall_rate = 0.5;
    game->move_start = 0.0;
    game->last_das = 0.0;

    game->over = false;

    game->shader_info.approx_height = 0;
    game->shader_info.over_time = 0;
}

bool game_update(game_t *game, double time) {
    if (game->over)
        return false;

    bindings_t bindings = game->settings.bindings;
    double actual_fall_rate = game->fall_rate;

    if (IsKeyDown(bindings.key_soft_drop) &&
        game->settings.fast_fall_rate < game->fall_rate) {
        actual_fall_rate = game->settings.fast_fall_rate;
    }

    if (!game->used_hold && IsKeyPressed(bindings.key_hold)) {
        game_swap_held_piece(game);
        game->used_hold = true;
    }

    bool can_place = false;

    if (time - game->last_fall >= actual_fall_rate) {
        game->last_fall = time;
        if (board_collides(&game->board, &game->falling, game->falling_x,
                           game->falling_y + 1)) {
            can_place = true;
        } else {
            game->falling_y++;
        }
    }

    if (IsKeyPressed(bindings.key_hard_drop)) {
        while (!board_collides(&game->board, &game->falling, game->falling_x,
                               game->falling_y + 1)) {
            game->falling_y++;
        }

        can_place = true;
    }

    if (can_place) {
        board_place(&game->board, &game->falling, game->falling_x,
                    game->falling_y);
        game->shader_info.approx_height = max(BOARD_HEIGHT - game->falling_y,
                                              game->shader_info.approx_height);

        for (int j = game->falling_y; j < game->falling_y + TM_MAX_SIZE; j++) {
            if (j < 0)
                continue;
            if (j >= BOARD_HEIGHT)
                break;

            for (int i = 0; i < BOARD_WIDTH; i++) {
                if (game->board.data[j][i] == 0)
                    goto next_row;
            }

            board_clear(&game->board, j, j + 1);
            game->shader_info.approx_height--;
        next_row:;
        }

        game_advance_piece(game);
        game->used_hold = false;
    }

    handle_shift(game, bindings.key_right, +1, time);
    handle_shift(game, bindings.key_left, -1, time);

    bool cw = IsKeyPressed(bindings.key_rotate_cw);
    bool ccw = IsKeyPressed(bindings.key_rotate_ccw);

    if (cw || ccw) {
        tetromino_t rotation_buf;
        tetromino_rotate(&game->falling, &rotation_buf,
                         cw ? CLOCKWISE : COUNTERCLOCKWISE);

        bool fits = !board_collides(&game->board, &rotation_buf,
                                    game->falling_x, game->falling_y);

        if (!fits) {
            if (!board_collides(&game->board, &rotation_buf,
                                game->falling_x - 1, game->falling_y)) {
                game->falling_x -= 1;
                fits = true;
            } else if (!board_collides(&game->board, &rotation_buf,
                                       game->falling_x + 1, game->falling_y)) {
                game->falling_x += 1;
                fits = true;
            }
        }

        if (fits) {
            game->falling = rotation_buf;
        }
    }

    if (game->over) {
        game->shader_info.over_time = (float)time;
    }

    return true;
}