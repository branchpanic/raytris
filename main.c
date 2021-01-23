#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int BLOCK_SIZE = 32;

// clang-format off
const int TETROMINOES[7][4][4] = {
	{
		{0, 0, 0, 0,},
	 	{1, 1, 1, 1,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
	{
		{2, 0, 0, 0,},
	 	{2, 2, 2, 0,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
	{
		{0, 0, 3, 0,},
	 	{3, 3, 3, 0,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
	{
		{4, 4, 0, 0,},
	 	{4, 4, 0, 0,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
	{
		{0, 5, 5, 0,},
	 	{5, 5, 0, 0,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
	{
		{0, 6, 0, 0,},
	 	{6, 6, 6, 0,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
	{
		{7, 7, 0, 0,},
	 	{0, 7, 7, 0,},
	 	{0, 0, 0, 0,},
	 	{0, 0, 0, 0,},
	},
};
// clang-format on

const int SIZES[7] = {4, 3, 3, 2, 3, 3, 3};
const Color COLORS[] = {RAYWHITE, RED,  ORANGE,   YELLOW,
                        GREEN,    BLUE, DARKBLUE, VIOLET};

const Color GRID_COLOR = {0, 0, 0, 128};

enum direction { CLOCKWISE, COUNTERCLOCKWISE };

// Rotate rotates the given piece from src to dst. To rotate in-place, src and
// dst can be the same array.
void rotate(int src[4][4], int dst[4][4], int size, enum direction rotation) {
    for (int i = 0; i < size / 2; i++) {
        for (int j = i; j < size - 1 - i; j++) {
            int temp = src[i][j]; // In case src is dst

            if (rotation == CLOCKWISE) {
                dst[i][j] = src[size - 1 - j][i];
                dst[size - 1 - j][i] = src[size - 1 - i][size - 1 - j];
                dst[size - 1 - i][size - 1 - j] = src[j][size - 1 - i];
                dst[j][size - 1 - i] = temp;
            } else if (rotation == COUNTERCLOCKWISE) {
                dst[i][j] = src[j][size - 1 - i];
                dst[j][size - 1 - i] = src[size - 1 - i][size - 1 - j];
                dst[size - 1 - i][size - 1 - j] = src[size - 1 - j][i];
                dst[size - 1 - j][i] = temp;
            }
        }
    }

    // Copy single center block if src is dst
    if (dst != src && size % 2 == 1) {
        dst[size / 2][size / 2] = src[size / 2][size / 2];
    }
}

// Collides returns true iff the piece defined by `piece` and `size` cannot be
// placed on `board` at position `x`, `y` without going out-of-bounds or
// colliding with an existing square.
bool collides(int board[BOARD_HEIGHT][BOARD_WIDTH], int x, int y,
              int piece[4][4], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (piece[i][j] == 0)
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
void place(int board[BOARD_HEIGHT][BOARD_WIDTH], int x, int y, int piece[4][4],
           int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (piece[i][j] == 0)
                continue;
            board[y + j][x + i] = piece[i][j];
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

// Select_piece puts a random piece in `piece` and its size in `size`.
// TODO: implement "bag" piece selection
void select_piece(int piece[4][4], int *size) {
    int i = GetRandomValue(0, 6);
    memcpy(piece, TETROMINOES[i], 16 * sizeof(int));
    *size = SIZES[i];
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

    int falling_piece[4][4];
    int piece_size;
    int rotated_piece[4][4];

    select_piece(falling_piece, &piece_size);

    int piece_y = 0;
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
            if (collides(board, piece_x, piece_y + 1, falling_piece,
                         piece_size)) {
                can_place = true;
            } else {
                piece_y += 1;
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            while (!collides(board, piece_x, piece_y + 1, falling_piece,
                             piece_size)) {
                piece_y++;
            }

            can_place = true;
        }

        if (can_place) {
            place(board, piece_x, piece_y, falling_piece, piece_size);
            select_piece(falling_piece, &piece_size);

            for (int j = piece_y; j < piece_y + 4; j++) {
                for (int i = 0; i < BOARD_WIDTH; i++) {
                    if (board[j][i] == 0)
                        goto next_row;
                }

                clear(board, j, j + 1);
            next_row:;
            }

            piece_y = 0;
            piece_x = 0;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            move_start = t;
            if (!collides(board, piece_x + 1, piece_y, falling_piece,
                          piece_size)) {
                piece_x += 1;
            }
        } else if (IsKeyDown(KEY_RIGHT) &&
                   t - move_start >= settings.das_delay) {
            if (t - last_das >= settings.das_rate) {
                if (!collides(board, piece_x + 1, piece_y, falling_piece,
                              piece_size)) {
                    piece_x += 1;
                }
                last_das = t;
            }
        }

        if (IsKeyPressed(KEY_LEFT)) {
            move_start = t;
            if (!collides(board, piece_x - 1, piece_y, falling_piece,
                          piece_size)) {
                piece_x -= 1;
            }
        } else if (IsKeyDown(KEY_LEFT) &&
                   t - move_start >= settings.das_delay) {
            if (t - last_das >= settings.das_rate) {
                if (!collides(board, piece_x - 1, piece_y, falling_piece,
                              piece_size)) {
                    piece_x -= 1;
                }
                last_das = t;
            }
        }

        bool cw = IsKeyPressed(KEY_X);
        bool ccw = IsKeyPressed(KEY_Z);

        if (cw || ccw) {
            rotate(falling_piece, rotated_piece, piece_size,
                   cw ? CLOCKWISE : COUNTERCLOCKWISE);

            bool fits =
                !collides(board, piece_x, piece_y, rotated_piece, piece_size);

            if (!fits && !collides(board, piece_x - 1, piece_y, rotated_piece,
                                   piece_size)) {
                piece_x -= 1;
                fits = true;
            } else if (!fits && !collides(board, piece_x + 1, piece_y,
                                          rotated_piece, piece_size)) {
                piece_x += 1;
                fits = true;
            }

            if (fits) {
                memcpy(falling_piece, rotated_piece, 16 * sizeof(int));
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int j = 0; j < BOARD_HEIGHT; j++) {
            DrawLine(0, j * BLOCK_SIZE, BOARD_WIDTH * BLOCK_SIZE,
                     j * BLOCK_SIZE, GRID_COLOR);
        }

        for (int j = 0; j < BOARD_WIDTH; j++) {
            DrawLine(j * BLOCK_SIZE, 0, j * BLOCK_SIZE,
                     BOARD_HEIGHT * BLOCK_SIZE, GRID_COLOR);
        }

        for (int j = 0; j < BOARD_HEIGHT; j++) {
            for (int i = 0; i < BOARD_WIDTH; i++) {
                if (board[j][i] == 0)
                    continue;

                DrawRectangle(BLOCK_SIZE * i, BLOCK_SIZE * j, BLOCK_SIZE,
                              BLOCK_SIZE, colors[board[j][i]]);
            }
        }

        for (int i = 0; i < piece_size; i++) {
            for (int j = 0; j < piece_size; j++) {
                if (falling_piece[i][j] == 0)
                    continue;

                DrawRectangle(BLOCK_SIZE * (i + piece_x),
                              BLOCK_SIZE * (j + piece_y), BLOCK_SIZE,
                              BLOCK_SIZE, colors[falling_piece[i][j]]);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
