#ifndef RAYTRIS_TETROMINO_H_
#define RAYTRIS_TETROMINO_H_

// TM_COUNT defines how many tetrominoes there are.
#define TM_COUNT 7

// TM_MAX_SIZE defines how big of a grid is needed to represent the largest
// tetromino.
#define TM_MAX_SIZE 4

typedef struct tetromino {
    int shape[TM_MAX_SIZE][TM_MAX_SIZE];
    int size;
} tetromino_t;

extern const tetromino_t TETROMINOES[TM_COUNT];

// Direction represents a rotation direction: either clockwise or
// counterclockwise.
typedef enum direction { CLOCKWISE, COUNTERCLOCKWISE } direction_t;

// Tetromino_rotate rotates the given piece from src to dst. To rotate in-place,
// src and dst can be the same array.
void tetromino_rotate(tetromino_t *src, tetromino_t *dst, direction_t dir);

// Choose_sequence places all pieces into bag in a random order.
void choose_sequence(tetromino_t bag[TM_COUNT]);

#endif