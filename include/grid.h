#pragma once
#include "raylib.h"

#define COLS 7
#define ROWS 14
#define CELL 32

#define GRID_PIXEL_W (COLS * CELL)
#define GRID_PIXEL_H (ROWS * CELL)

typedef enum { EMPTY, SAND } CellType;

typedef struct {
    CellType type;
    int tier;
    bool fresh;
} Cell;

typedef struct {
    int r, c;
    Cell cell;
    bool active;
} Piece;

extern Piece piece;

extern Cell grid[ROWS][COLS];

void piece_input(double dt);
void piece_spawn();
extern bool soft_drop;

int grid_get_score(void);
void grid_init(void);
void grid_update(double dt);
void grid_draw(void);
