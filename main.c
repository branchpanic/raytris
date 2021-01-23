#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tetromino.h"

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 40;

// BOARD_VISIBLE is the first visible line of the board. The has an extra 20
// blocks of headroom above the play area.
const int BOARD_VISIBLE = 20;

const int BLOCK_SIZE = 32;

const Color COLORS[] = {RAYWHITE, RED,  ORANGE,   YELLOW,
                        GREEN,    BLUE, DARKBLUE, VIOLET};

const Color GRID_COLOR = {0, 0, 0, 128};

// Collides returns true iff the piece defined by `piece` and `size` cannot be
// placed on `board` at position `x`, `y` without going out-of-bounds or
// colliding with an existing square.
bool collides(int board[BOARD_HEIGHT][BOARD_WIDTH], int x, int y,
              struct tetromino *piece) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;

            if (x + i < 0 || x + i >= BOARD_WIDTH || y + j < 0 ||
                y + j >= BOARD_HEIGHT || board[y + j][x + i] != 0) {
                return true;
            }
        }
    }

    return false;
}

// Place stamps the piece defined by `piece` and `size` onto `board` at `x`,
// `y`.
void place(int board[BOARD_HEIGHT][BOARD_WIDTH], int x, int y,
           struct tetromino *piece) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;
            board[y + j][x + i] = piece->shape[i][j];
        }
    }
}

// Clear removes rows [y0, y1) from `board`. All of the cells above the removed
// rows are shifted down.
void clear(int board[BOARD_HEIGHT][BOARD_WIDTH], int y0, int y1) {
    memmove((int *)board + (y1 - y0) * BOARD_WIDTH, board,
            y0 * BOARD_WIDTH * sizeof(int));
    memset((int *)board, 0, (y1 - y0) * BOARD_WIDTH * sizeof(int));
}

// Read_colors reads 7 colors into `colors` from lines of the file at `file`.
// TODO: replace with INI parser
void read_colors(Color colors[7], const char *file) {
    char *text = LoadFileText(file);
    char *token = strtok(text, "\n");

    int i = 0;
    while (token != NULL) {
        colors[i] = GetColor((int)strtol(token, NULL, 16));
        colors[i].a = 255;
        token = strtok(NULL, "\n");
        i++;
        if (i >= 7)
            break;
    }

    UnloadFileText(text);
}

void draw_piece(struct tetromino *piece, Color colors[8], int x, int y) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;

            DrawRectangle(BLOCK_SIZE * (i + x),
                          BLOCK_SIZE * (j + y - BOARD_VISIBLE), BLOCK_SIZE,
                          BLOCK_SIZE, colors[piece->shape[i][j]]);
        }
    }
}

// Settings contains configurable game settings.
// TODO: load/save from INI
struct settings {
    double fast_fall_rate;
    double das_delay;
    double das_rate;
};

int main(int argc, char const *argv[]) {
    InitWindow(600, 800, "raytris");
    SetTargetFPS(60);

    struct settings settings = {
        .fast_fall_rate = 0.1,
        .das_delay = 0.12,
        .das_rate = 0.01,
    };

    int board[BOARD_HEIGHT][BOARD_WIDTH];
    memset(board, 0, BOARD_WIDTH * BOARD_HEIGHT * sizeof(int));

    Color colors[8];
    memcpy(colors, COLORS, 8 * sizeof(Color));
    read_colors(colors + 1, "colors.txt");

    struct tetromino bag[TM_COUNT];
    int bag_current = 0;
    choose_sequence(bag);

    struct tetromino falling;
    struct tetromino rotated;
    falling = bag[bag_current];

    int piece_y = BOARD_VISIBLE;
    int piece_x = 0;

    double last_fall = 0.0;
    double fall_rate = 0.5;
    double move_start = 0.0;
    double last_das = 0.0;

    while (!WindowShouldClose()) {
        double t = GetTime();
        double actual_fall_rate = fall_rate;

        if (IsKeyDown(KEY_DOWN) && settings.fast_fall_rate < fall_rate) {
            actual_fall_rate = settings.fast_fall_rate;
        }

        bool can_place = false;

        if (t - last_fall >= actual_fall_rate) {
            last_fall = t;
            if (collides(board, piece_x, piece_y + 1, &falling)) {
                can_place = true;
            } else {
                piece_y += 1;
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            while (!collides(board, piece_x, piece_y + 1, &falling)) {
                piece_y++;
            }

            can_place = true;
        }

        if (can_place) {
            place(board, piece_x, piece_y, &falling);

            bag_current++;
            if (bag_current >= 7) {
                bag_current = 0;
                choose_sequence(bag);
            }

            falling = bag[bag_current];

            for (int j = piece_y; j < piece_y + 4; j++) {
                for (int i = 0; i < BOARD_WIDTH; i++) {
                    if (board[j][i] == 0)
                        goto next_row;
                }

                clear(board, j, j + 1);
            next_row:;
            }

            piece_y = BOARD_VISIBLE;
            piece_x = 0;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            move_start = t;
            if (!collides(board, piece_x + 1, piece_y, &falling)) {
                piece_x += 1;
            }
        } else if (IsKeyDown(KEY_RIGHT) &&
                   t - move_start >= settings.das_delay) {
            if (t - last_das >= settings.das_rate) {
                if (!collides(board, piece_x + 1, piece_y, &falling)) {
                    piece_x += 1;
                }
                last_das = t;
            }
        }

        if (IsKeyPressed(KEY_LEFT)) {
            move_start = t;
            if (!collides(board, piece_x - 1, piece_y, &falling)) {
                piece_x -= 1;
            }
        } else if (IsKeyDown(KEY_LEFT) &&
                   t - move_start >= settings.das_delay) {
            if (t - last_das >= settings.das_rate) {
                if (!collides(board, piece_x - 1, piece_y, &falling)) {
                    piece_x -= 1;
                }
                last_das = t;
            }
        }

        bool cw = IsKeyPressed(KEY_X);
        bool ccw = IsKeyPressed(KEY_Z);

        if (cw || ccw) {
            rotate(&falling, &rotated, cw ? CLOCKWISE : COUNTERCLOCKWISE);

            bool fits = !collides(board, piece_x, piece_y, &rotated);

            if (!fits && !collides(board, piece_x - 1, piece_y, &rotated)) {
                piece_x -= 1;
                fits = true;
            } else if (!fits &&
                       !collides(board, piece_x + 1, piece_y, &rotated)) {
                piece_x += 1;
                fits = true;
            }

            if (fits) {
                falling = rotated;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int j = 0; j < BOARD_VISIBLE; j++) {
            DrawLine(0, j * BLOCK_SIZE, BOARD_WIDTH * BLOCK_SIZE,
                     j * BLOCK_SIZE, GRID_COLOR);
        }

        for (int j = 0; j < BOARD_WIDTH; j++) {
            DrawLine(j * BLOCK_SIZE, 0, j * BLOCK_SIZE,
                     BOARD_VISIBLE * BLOCK_SIZE, GRID_COLOR);
        }

        for (int j = BOARD_VISIBLE; j < BOARD_HEIGHT; j++) {
            for (int i = 0; i < BOARD_WIDTH; i++) {
                if (board[j][i] == 0)
                    continue;

                DrawRectangle(BLOCK_SIZE * i, BLOCK_SIZE * (j - BOARD_VISIBLE),
                              BLOCK_SIZE, BLOCK_SIZE, colors[board[j][i]]);
            }
        }

        draw_piece(&falling, colors, piece_x, piece_y);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
