#ifndef RAYTRIS_RAYTRIS_H_
#define RAYTRIS_RAYTRIS_H_

#include "settings.h"
#include "tetromino.h"

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 40

// BOARD_VISIBLE is the first visible line of the board. The has an extra 20
// blocks of headroom above the play area.
#define BOARD_VISIBLE 20

// BLOCK_SIZE defines how big one square block of the game board is in pixels.
#define BLOCK_SIZE 32

// Board is a game board of size BOARD_HEIGHT x BOARD_WIDTH. It is indexed
// row-first.
typedef struct board {
    int data[BOARD_HEIGHT][BOARD_WIDTH];
} board_t;

// Board_clear deletes lines in the range [y0, y1) and shifts down all lines
// above them.
void board_clear(board_t *board, int y0, int y1);

// Board_place stamps the piece defined by `piece` and `size` onto `board` at
// `x`, `y`.
void board_place(board_t *board, tetromino_t *piece, int x, int y);

// Board_collides returns true if the given `piece`, positioned at `x`, `y`,
// either collides with filled cells on the board or exceeds the board's
// boundaries.
bool board_collides(board_t *board, tetromino_t *piece, int x, int y);

// Game is the main game data structure.
typedef struct game {
    settings_t settings;
    board_t board;

    tetromino_t bag[TM_COUNT];
    tetromino_t next_bag[TM_COUNT];
    int bag_current;

    tetromino_t falling;
    int falling_x;
    int falling_y;

    tetromino_t held;
    bool has_held;
    bool used_hold;

    double last_fall;
    double fall_rate;
    double move_start;
    double last_das;

    bool over;

    Shader bg_shader;
    struct shader_info {
        int time_loc;
        int over_time_loc;
        int resolution_loc;
        int height_loc;
        int block_size_loc;

        int approx_height;
        float over_time;
    } shader_info;
} game_t;

// Game_advance_piece updates `falling` with the next piece in the queue. If
// `bag` is exhausted, it is replaced with the contents of `next_bag`. When this
// happens, `bag_current` is set to 0 and `next_bag` is randomly regenerated.
void game_advance_piece(game_t *game);

// Game_swap_held_piece updates `held` with the current piece. If `held` had
// a prior value, `falling` is set to it. Otherwise, `game_advance_piece` is
// used to select the next piece.
void game_swap_held_piece(game_t *game);

// Game_init sets the state of the given `game` to reasonable defaults. It also
// loads the background shader if one is specified.
void game_init(game_t *game);

// Game_reset resets a game to its default state.
void game_reset(game_t *game);

// Game_reload_shaders reloads game shaders.
void game_reload_shaders(game_t *game);

// Game_update handles input and updates the current game. It returns false if
// the game has ended.
bool game_update(game_t *game, double time);

void game_free(game_t *game);

#endif