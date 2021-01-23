#ifndef RAYTRIS_TETROMINO_H_
#define RAYTRIS_TETROMINO_H_

#define TM_COUNT 7
#define TM_MAX_SIZE 4

struct tetromino {
    int shape[TM_MAX_SIZE][TM_MAX_SIZE];
    int size;
};

const struct tetromino TETROMINOES[TM_COUNT];

// Direction represents a rotation direction: either clockwise or
// counterclockwise.
enum direction { CLOCKWISE, COUNTERCLOCKWISE };

// Rotate rotates the given piece from src to dst. To rotate in-place, src and
// dst can be the same array.
void rotate(struct tetromino *src, struct tetromino *dst, enum direction dir);

// Choose_sequence places all pieces into bag in a random order.
void choose_sequence(struct tetromino bag[TM_COUNT]);

#endif