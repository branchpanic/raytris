#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tetromino.h"

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 40

// BOARD_VISIBLE is the first visible line of the board. The has an extra 20
// blocks of headroom above the play area.
const int BOARD_VISIBLE = 20;

const int BLOCK_SIZE = 32;

// Settings contains configurable game settings.
// TODO: load/save from INI
struct settings {
    double fast_fall_rate;
    double das_delay;
    double das_rate;

    Color block_colors[TM_COUNT];
    Color bg_color;
    Color grid_color;
};

struct board {
    int data[BOARD_HEIGHT][BOARD_WIDTH];
};

struct game {
    struct settings settings;
    struct board board;

    struct tetromino bag[TM_COUNT];
    struct tetromino next_bag[TM_COUNT];
    int bag_current;

    struct tetromino falling;
    int falling_x;
    int falling_y;

    struct tetromino held;
    bool has_held;
    bool used_hold;

    double last_fall;
    double fall_rate;
    double move_start;
    double last_das;

    bool over;
};

// Collides returns true iff the piece defined by `piece` and `size` cannot be
// placed on `board` at position `x`, `y` without going out-of-bounds or
// colliding with an existing square.
bool collides(struct board *board, int x, int y, struct tetromino *piece) {
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

// Place stamps the piece defined by `piece` and `size` onto `board` at `x`,
// `y`.
void place(struct board *board, int x, int y, struct tetromino *piece) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;
            board->data[y + j][x + i] = piece->shape[i][j];
        }
    }
}

// Clear removes rows [y0, y1) from `board`. All of the cells above the removed
// rows are shifted down.
void clear(struct board *board, int y0, int y1) {
    int *data = (int *)(board->data);
    memmove(data + (y1 - y0) * BOARD_WIDTH, data,
            y0 * BOARD_WIDTH * sizeof(int));
    memset(data, 0, (y1 - y0) * BOARD_WIDTH * sizeof(int));
}

// Read_colors reads 7 colors into `colors` from lines of the file at `file`.
// TODO: replace with INI parser
void read_colors(Color colors[TM_COUNT], const char *file) {
    char *text = LoadFileText(file);
    char *token = strtok(text, "\n");

    int i = 0;
    while (token != NULL) {
        colors[i] = GetColor(strtoul(token, NULL, 16));
        token = strtok(NULL, "\n");
        i++;
        if (i >= TM_COUNT)
            break;
    }

    for (; i < TM_COUNT; i++) {
        colors[i] = RED;
    }

    UnloadFileText(text);
}

void draw_piece_sized(struct tetromino *piece, Color colors[TM_COUNT], int x,
                      int y, int block_size) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;

            DrawRectangle(block_size * i + x, block_size * j + y, block_size,
                          block_size, colors[piece->shape[i][j] - 1]);
        }
    }
}

// Draw_piece draws the given tetromino at (x, y) on-screen.
void draw_piece(struct tetromino *piece, Color colors[TM_COUNT], int x, int y) {
    draw_piece_sized(piece, colors, x, y, BLOCK_SIZE);
}

void draw_board(struct game *game, int x, int y) {
    for (int j = 0; j < BOARD_VISIBLE; j++) {
        DrawLine(x, y + j * BLOCK_SIZE, x + BOARD_WIDTH * BLOCK_SIZE, y + j * BLOCK_SIZE,
                 game->settings.grid_color);
    }

    for (int j = 0; j < BOARD_WIDTH; j++) {
        DrawLine(x + j * BLOCK_SIZE, y, x + j * BLOCK_SIZE, y + BOARD_VISIBLE * BLOCK_SIZE,
                 game->settings.grid_color);
    }

    for (int j = BOARD_VISIBLE; j < BOARD_HEIGHT; j++) {
        for (int i = 0; i < BOARD_WIDTH; i++) {
            if (game->board.data[j][i] == 0)
                continue;

            DrawRectangle(
                x + BLOCK_SIZE * i, y + BLOCK_SIZE * (j - BOARD_VISIBLE), BLOCK_SIZE,
                BLOCK_SIZE,
                game->settings.block_colors[game->board.data[j][i] - 1]);
        }
    }
}

void draw_bag(struct game *game, int x, int y) {
    int j = 0;
    for (int i = game->bag_current + 1; i < TM_COUNT; i++) {
        draw_piece_sized(game->bag + i, game->settings.block_colors, x,
                         y + (j++ * TM_MAX_SIZE * (2 + BLOCK_SIZE / 2)),
                         BLOCK_SIZE / 2);
    }

    for (int i = 0; i < game->bag_current; i++) {
        draw_piece_sized(game->next_bag + i, game->settings.block_colors, x,
                         y + (j++ * TM_MAX_SIZE * (2 + BLOCK_SIZE / 2)),
                         BLOCK_SIZE / 2);
    }
}

void draw_game(struct game *game) {
    int board_width_px = BLOCK_SIZE * BOARD_WIDTH;
    int x = (600 - board_width_px)/2;
    int y = (800 - BLOCK_SIZE * BOARD_VISIBLE)/2;

    draw_board(game, x, y);
    draw_piece(&game->falling, game->settings.block_colors,
               x + BLOCK_SIZE * game->falling_x,
               y + BLOCK_SIZE * (game->falling_y - BOARD_VISIBLE));
    draw_bag(game, x + board_width_px + 16, y + 16);

    if (game->has_held) {
        draw_piece_sized(&game->held, game->settings.block_colors,
                   x - (2 * BLOCK_SIZE) - 16, y + 16, BLOCK_SIZE/2);
    }
}

void advance_piece(struct game *game) {
    game->bag_current++;
    if (game->bag_current >= TM_COUNT) {
        game->bag_current = 0;
        memcpy(game->bag, game->next_bag, TM_COUNT * sizeof(struct tetromino));
        choose_sequence(game->next_bag);
    }

    game->falling = game->bag[game->bag_current];
    game->falling_y = BOARD_VISIBLE;
    game->falling_x = (BOARD_WIDTH - game->falling.size) / 2;

    if (collides(&game->board, game->falling_x, game->falling_y,
                 &game->falling)) {
        game->over = true;
    }
}

void swap_held_piece(struct game *game) {
    if (game->has_held) {
        struct tetromino temp = game->falling;
        game->falling = game->held;
        game->held = temp;
        game->falling_y = BOARD_VISIBLE;
        game->falling_x = (BOARD_WIDTH - game->falling.size) / 2;
        if (collides(&game->board, game->falling_x, game->falling_y,
                     &game->falling)) {
            game->over = true;
        }
    } else {
        game->held = game->falling;
        game->has_held = true;
        advance_piece(game);
    }
}

void game_reset(struct game *game, struct settings settings) {
    memset(game, 0, sizeof(struct game));
    game->settings = settings;

    choose_sequence(game->bag);
    choose_sequence(game->next_bag);

    game->falling = game->bag[game->bag_current];
    game->falling_y = BOARD_VISIBLE;
    game->falling_x = (BOARD_WIDTH - game->falling.size) / 2;

    game->last_fall = 0.0;
    game->fall_rate = 0.5;
    game->move_start = 0.0;
    game->last_das = 0.0;
}

void handle_shift(struct game *game, int key, int x_offset, double time) {
    if (IsKeyPressed(key)) {
        game->move_start = time;
        if (!collides(&game->board, game->falling_x + x_offset, game->falling_y,
                      &game->falling)) {
            game->falling_x += x_offset;
        }
    } else if (IsKeyDown(key) &&
               time - game->move_start >= game->settings.das_delay) {
        if (time - game->last_das >= game->settings.das_rate) {
            if (!collides(&game->board, game->falling_x + x_offset,
                          game->falling_y, &game->falling)) {
                game->falling_x += x_offset;
            }
            game->last_das = time;
        }
    }
}

int main(int argc, char const *argv[]) {
    InitWindow(600, 800, "raytris");
    SetTargetFPS(60);

    struct settings defaults = {
        .fast_fall_rate = 0.1,
        .das_delay = 0.12,
        .das_rate = 0.01,
        .bg_color = GetColor(0x000000FF),
        .grid_color = GetColor(0xFFFFFF22),
    };

    read_colors(defaults.block_colors, "colors.txt");

    struct game game = {0};
    game_reset(&game, defaults);

    struct tetromino rotation_buf;

            for (int j = piece_y; j < piece_y + 4; j++) {
                for (int i = 0; i < BOARD_WIDTH; i++) {
                    if (board[j][i] == 0)

    while (!WindowShouldClose()) {
        if (!game.over) {
            double t = GetTime();
            double actual_fall_rate = game.fall_rate;

            if (IsKeyDown(KEY_DOWN) &&
                game.settings.fast_fall_rate < game.fall_rate) {
                actual_fall_rate = game.settings.fast_fall_rate;
            }

            if (!game.used_hold && IsKeyPressed(KEY_C)) {
                swap_held_piece(&game);
                game.used_hold = true;
            }

            bool can_place = false;

            if (t - game.last_fall >= actual_fall_rate) {
                game.last_fall = t;
                if (collides(&game.board, game.falling_x, game.falling_y + 1,
                             &game.falling)) {
                    can_place = true;
                } else {
                    game.falling_y++;
                }
            }

            if (IsKeyPressed(KEY_SPACE)) {
                while (!collides(&game.board, game.falling_x,
                                 game.falling_y + 1, &game.falling)) {
                    game.falling_y++;
                }

                can_place = true;
            }

            if (can_place) {
                place(&game.board, game.falling_x, game.falling_y,
                      &game.falling);

                for (int j = game.falling_y; j < game.falling_y + TM_MAX_SIZE;
                     j++) {
                    if (j < 0)
                        continue;
                    if (j >= BOARD_HEIGHT)
                        break;

                    for (int i = 0; i < BOARD_WIDTH; i++) {
                        if (game.board.data[j][i] == 0)
                            goto next_row;
                    }

                    clear(&game.board, j, j + 1);
                next_row:;
                }

                advance_piece(&game);
                game.used_hold = false;
            }

            handle_shift(&game, KEY_RIGHT, +1, t);
            handle_shift(&game, KEY_LEFT, -1, t);

            bool cw = IsKeyPressed(KEY_X);
            bool ccw = IsKeyPressed(KEY_Z);

            if (cw || ccw) {
                rotate(&game.falling, &rotation_buf,
                       cw ? CLOCKWISE : COUNTERCLOCKWISE);

                bool fits = !collides(&game.board, game.falling_x,
                                      game.falling_y, &rotation_buf);

                if (!fits) {
                    if (!collides(&game.board, game.falling_x - 1,
                                  game.falling_y, &rotation_buf)) {
                        game.falling_x -= 1;
                        fits = true;
                    } else if (!collides(&game.board, game.falling_x + 1,
                                         game.falling_y, &rotation_buf)) {
                        game.falling_x += 1;
                        fits = true;
                    }
                }

                if (fits) {
                    game.falling = rotation_buf;
                }
            }
        } else {
            if (IsKeyPressed(KEY_Q)) {
                game_reset(&game, defaults);
            }
        }

        BeginDrawing();
        ClearBackground(game.settings.bg_color);
        draw_game(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
