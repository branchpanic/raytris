#include <raylib.h>

#include "tetromino.h"

void rotate(struct tetromino *src, struct tetromino *dst, enum direction dir) {
    int size = src->size;
    dst->size = size;

    for (int i = 0; i < size / 2; i++) {
        for (int j = i; j < size - 1 - i; j++) {
            int temp = src->shape[i][j]; // In case src is dst

            if (dir == CLOCKWISE) {
                dst->shape[i][j] = src->shape[size - 1 - j][i];
                dst->shape[size - 1 - j][i] =
                    src->shape[size - 1 - i][size - 1 - j];
                dst->shape[size - 1 - i][size - 1 - j] =
                    src->shape[j][size - 1 - i];
                dst->shape[j][size - 1 - i] = temp;
            } else if (dir == COUNTERCLOCKWISE) {
                dst->shape[i][j] = src->shape[j][size - 1 - i];
                dst->shape[j][size - 1 - i] =
                    src->shape[size - 1 - i][size - 1 - j];
                dst->shape[size - 1 - i][size - 1 - j] =
                    src->shape[size - 1 - j][i];
                dst->shape[size - 1 - j][i] = temp;
            }
        }
    }

    // Copy single center block if src is dst
    if (dst != src && size % 2 == 1) {
        dst->shape[size / 2][size / 2] = src->shape[size / 2][size / 2];
    }
}

void choose_sequence(struct tetromino bag[TM_COUNT]) {
    int indices[TM_COUNT];
    for (int i = 0; i < TM_COUNT; i++) {
        indices[i] = i;
    }

    if (TM_COUNT > 1) {
        for (int i = TM_COUNT - 1; i > 0; i--) {
            int j = GetRandomValue(0, i);
            int t = indices[j];
            indices[j] = indices[i];
            indices[i] = t;
        }
    }

    for (int i = 0; i < TM_COUNT; i++) {
        bag[i] = TETROMINOES[i];
    }
}

// clang-format off
const struct tetromino TETROMINOES[TM_COUNT] = {
    { // I
        .shape = {
            {0, 0, 0, 0,},
            {1, 1, 1, 1,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 4
    },
    { // J
        .shape = {
            {2, 0, 0, 0,},
            {2, 2, 2, 0,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 3
    },
    { // L
        .shape = {
            {0, 0, 3, 0,},
            {3, 3, 3, 0,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 3
    },
    { // O
        .shape = {
            {4, 4, 0, 0,},
            {4, 4, 0, 0,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 2
    },
    { // S
        .shape = {
            {0, 5, 5, 0,},
            {5, 5, 0, 0,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 3
    },
    { // T
        .shape = {
            {0, 6, 0, 0,},
            {6, 6, 6, 0,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 3
    },
    { // Z
        .shape = {
            {7, 7, 0, 0,},
            {0, 7, 7, 0,},
            {0, 0, 0, 0,},
            {0, 0, 0, 0,},
        },
        .size = 3
    },
};
// clang-format on
