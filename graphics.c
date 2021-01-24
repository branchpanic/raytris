#include "graphics.h"

#include <raylib.h>

void draw_piece_s(tetromino_t *piece, palette_t *palette, int x, int y,
                  int block_size) {
    for (int i = 0; i < piece->size; i++) {
        for (int j = 0; j < piece->size; j++) {
            if (piece->shape[i][j] == 0)
                continue;

            DrawRectangle(block_size * i + x, block_size * j + y, block_size,
                          block_size,
                          palette->block_colors[piece->shape[i][j] - 1]);
        }
    }
}

void draw_piece(struct tetromino *piece, palette_t *palette, int x, int y) {
    draw_piece_s(piece, palette, x, y, BLOCK_SIZE);
}

void draw_board(board_t *board, palette_t *palette, int x, int y) {
    for (int j = 0; j < BOARD_VISIBLE; j++) {
        DrawLine(x, y + j * BLOCK_SIZE, x + BOARD_WIDTH * BLOCK_SIZE,
                 y + j * BLOCK_SIZE, palette->grid_color);
    }

    for (int j = 0; j < BOARD_WIDTH; j++) {
        DrawLine(x + j * BLOCK_SIZE, y, x + j * BLOCK_SIZE,
                 y + BOARD_VISIBLE * BLOCK_SIZE, palette->grid_color);
    }

    for (int j = BOARD_VISIBLE; j < BOARD_HEIGHT; j++) {
        for (int i = 0; i < BOARD_WIDTH; i++) {
            if (board->data[j][i] == 0)
                continue;

            DrawRectangle(x + BLOCK_SIZE * i,
                          y + BLOCK_SIZE * (j - BOARD_VISIBLE), BLOCK_SIZE,
                          BLOCK_SIZE,
                          palette->block_colors[board->data[j][i] - 1]);
        }
    }
}

void draw_bag(struct game *game, int x, int y) {
    int j = 0;
    for (int i = game->bag_current + 1; i < TM_COUNT; i++) {
        draw_piece_s(game->bag + i, &game->settings.palette, x,
                     y + (j++ * TM_MAX_SIZE * (2 + BLOCK_SIZE / 2)),
                     BLOCK_SIZE / 2);
    }

    for (int i = 0; i < game->bag_current; i++) {
        draw_piece_s(game->next_bag + i, &game->settings.palette, x,
                     y + (j++ * TM_MAX_SIZE * (2 + BLOCK_SIZE / 2)),
                     BLOCK_SIZE / 2);
    }
}

void draw_game(game_t *game) {
    int board_width_px = BLOCK_SIZE * BOARD_WIDTH;
    int x = (600 - board_width_px) / 2;
    int y = (800 - BLOCK_SIZE * BOARD_VISIBLE) / 2;

    DrawRectangle(x, y, board_width_px, BLOCK_SIZE * BOARD_VISIBLE,
                  GetColor(0x00000055));
    draw_board(&game->board, &game->settings.palette, x, y);
    draw_piece(&game->falling, &game->settings.palette,
               x + BLOCK_SIZE * game->falling_x,
               y + BLOCK_SIZE * (game->falling_y - BOARD_VISIBLE));

    draw_bag(game, x + board_width_px + 16, y + 16);

    if (game->has_held) {
        draw_piece_s(&game->held, &game->settings.palette,
                     x - (2 * BLOCK_SIZE) - 16, y + 16, BLOCK_SIZE / 2);
    }
}