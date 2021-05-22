#ifndef RAYTRIS_GRAPHICS_H_
#define RAYTRIS_GRAPHICS_H_

#include "raytris.h"
#include "settings.h"

// Draw_piece_s draws the given tetromino at (x, y) on-screen using the given
// `block_size` and corresponding color(s) from `palette`.
void draw_piece_s(tetromino_t *piece, palette_t *palette, int x, int y, int block_size);

// Draw_piece draws the given tetromino at (x, y) on-screen with the default
// block size and corresponding color(s) from `palette`.
void draw_piece(tetromino_t *piece, palette_t *palette, int x, int y);

// Draw_board draws the given board at (x, y) using the given `palette`.
void draw_board(board_t *board, palette_t *palette, int x, int y);

// Draw_bag draws the queue of upcoming pieces at (x, y).
void draw_bag(game_t *game, int x, int y);

// Draw_game draws the entire game to the screen.
void draw_game(game_t *game, double time);

#endif
