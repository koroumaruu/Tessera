#include "grid.h"
#include "particle.h"
#include "sound.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

Cell grid[ROWS][COLS] = {0};
Piece piece = {0};

bool soft_drop = false;

#define BAG_SIZE 7
#define POWER 1.0f
#define NEXT_COUNT BAG_SIZE

#define DAS 0.100
#define ARR 0.001

#define MAX_LEVEL 10

static const double piece_hz_table[] = { 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 8.0, 10.0, 12.0, 15.0 };
static const double sim_hz_table[]   = { 7.0, 8.0, 9.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 22.0 };
static float recency[BAG_SIZE];
static int next_queue[NEXT_COUNT];

static double das_left = 0, arr_left = 0;
static double das_right = 0, arr_right = 0;

static int hold_tier = -1;
static bool can_hold = true;

static int score = 0;

static Font font = {0};

typedef enum {
    FALL_NONE,
    FALL_DOWN,
    FALL_LEFT,
    FALL_RIGHT,
} FallDir;

static FallDir fall_dir(int r, int c) {
    if (r + 1 >= ROWS) return FALL_NONE;
    if (grid[r+1][c].type == EMPTY) return FALL_DOWN;
    
    bool can_left = c > 0 && grid[r+1][c-1].type == EMPTY && grid[r][c-1].type == EMPTY;
    bool can_right = c < COLS-1 && grid[r+1][c+1].type == EMPTY && grid[r][c+1].type == EMPTY;
    if (can_left && !can_right) return FALL_LEFT;
    if (!can_left && can_right) return FALL_RIGHT;
    if (can_left && can_right) return FALL_LEFT;
    return FALL_NONE;
}

static inline float cell_sx(int c) {
    return (GetScreenWidth() - GRID_PIXEL_W) / 2.0f + c * CELL + CELL / 2.0f;
}

static inline float cell_sy(int c) {
    return (GetScreenHeight() - GRID_PIXEL_H) / 2.0f + c * CELL + CELL / 2.0f;
}

static int gen_next(void) {
    float weights[BAG_SIZE];
    float total = 0.0f;
    for (int i = 0; i < BAG_SIZE; i++) {
        weights[i] = powf(1.0f / recency[i], POWER);
        total += weights[i];
    }
    float roll = (float)GetRandomValue(0, 10000) / 10000.0f * total;
    int pick = BAG_SIZE - 1;
    float acc = 0.0f;
    for (int i = 0; i < BAG_SIZE; i++) {
        acc += weights[i];
        if (roll < acc) { pick = i; break; }
    }
    recency[pick] += 1.0f;
    for (int i = 0; i < BAG_SIZE; i++) {
        if (i != pick) recency[i] /= 2.0f;
    }
    return pick;
}

Color tier_color(int tier) {
    float hue = (tier / 10.0f) * 270.0f;
    return ColorFromHSV(hue, 1.0f, 1.0f);
} 

static bool grid_settled(void) {
    for (int r = 0; r < ROWS -1; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c].type == SAND && fall_dir(r, c) != FALL_NONE) {
                return false;
            }
        }
    }
    return true;
}

static void try_move(int dc) {
    int nc = piece.c + dc;
    if (nc >= 0 && nc < COLS && grid[piece.r][nc].type == EMPTY) {
        piece.c = nc;
    }
}

static int next_from_bag(void) {
    int pick = next_queue[0];
    for (int i = 0; i < NEXT_COUNT - 1; i++) {
        next_queue[i] = next_queue[i+1];
    }
    next_queue[NEXT_COUNT - 1] = gen_next();
    return pick;
}

void piece_spawn(void) {
    piece.c      = COLS / 2;
    piece.r      = 0;
    piece.cell   = (Cell){ SAND, next_from_bag(), true};
    piece.active = true;
}

void piece_input(double dt) {
    if (!piece.active) return;
    soft_drop = IsKeyDown(KEY_DOWN);
    if (IsKeyPressed(KEY_SPACE) && grid_settled()) {
        while (piece.r + 1 < ROWS && grid[piece.r+1][piece.c].type == EMPTY) {
            piece.r++;
        }
        grid[piece.r][piece.c] = piece.cell;
        grid[piece.r][piece.c].fresh = true;
        piece.active = false;
        particles_spawn(cell_sx(piece.c), cell_sy(piece.r) + CELL / 2.0f, tier_color(piece.cell.tier), EFFECT_LAND);
        sound_play_land();
        piece_spawn();
        can_hold = true;
        return;
    }

    if (IsKeyPressed(KEY_C) && can_hold) {
        if (hold_tier == -1) {
            hold_tier = piece.cell.tier;
            piece_spawn();
        } else {
            int tmp = hold_tier;
            hold_tier = piece.cell.tier;
            piece.cell.tier = tmp;
            piece.r = 0;
            piece.c = COLS / 2;
        }
        can_hold = false;
    }

    if (IsKeyPressed(KEY_LEFT)) {
        try_move(-1);
        das_left = arr_left = 0;
    } else if (IsKeyDown(KEY_LEFT)) {
        das_left += dt;
        if (das_left >= DAS) {
            arr_left += dt;
            if (arr_left >= ARR) { try_move (-1); arr_left = 0; }
        }
    } else {
        das_left = arr_left = 0;
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        try_move(1);
        das_right = arr_right = 0;
    } else if (IsKeyDown(KEY_RIGHT)) {
        das_right += dt;
        if (das_right >= DAS) {
            arr_right += dt;
            if (arr_right >= ARR) { try_move (1); arr_right = 0; }
        }
    } else {
        das_right = arr_right = 0;
    }
}

static void fall_once(void) {
    for (int r = ROWS - 2; r >= 0; r--) {
        int left = (r % 2 == 0);
        for (int i = 0; i < COLS; i++) {
            int c = left ? i : (COLS - 1 - i);
            if (grid[r][c].type != SAND) continue;
            switch (fall_dir(r, c)) {
                case FALL_DOWN:
                    grid[r+1][c] = grid[r][c];
                    grid[r+1][c].fresh = true;
                    grid[r][c] = (Cell){ EMPTY, 0, false};
                    break;
                case FALL_LEFT:
                    grid[r+1][c-1] = grid[r][c];
                    grid[r+1][c-1].fresh = true;
                    grid[r][c] = (Cell){ EMPTY, 0, false};
                    break;
                case FALL_RIGHT:
                    grid[r+1][c+1] = grid[r][c];
                    grid[r+1][c+1].fresh = true;
                    grid[r][c] = (Cell){ EMPTY, 0, false};
                    break;
                case FALL_NONE:
                    break;
            }
        }
    }
}

static bool try_merge(int r, int c, bool used[ROWS][COLS]) {
    int tier = grid[r][c].tier;
    bool is_fresh = grid[r][c].fresh;
    int nr[] = { is_fresh ? r+1 : r-1, is_fresh ? r-1 : r+1, r, r };
    int nc[] = { c, c, c-1, c+1 };
    for (int i = 0; i < 4; i++) {
        int rr = nr[i], cc = nc[i];
        if (rr < 0 || rr >= ROWS || cc < 0 || cc >= COLS) continue;
        if (used[rr][cc] || grid[rr][cc].type != SAND) continue;
        if (grid[rr][cc].tier != tier) continue;
        float px = (cell_sx(c) + cell_sx(cc)) / 2.0f;
        float py = (cell_sy(r) + cell_sy(rr)) / 2.0f;
        
        if (tier == 10) {
            score += 10000;
            int radius = 3;
            for (int er = r - radius; er <= r + radius; er++) {
                for (int ec = c - radius; ec <= c + radius; ec++) {
                    if (er < 0 || er >= ROWS || ec < 0 || ec >= COLS) continue;
                    int dr = er - r, dc = ec - c;
                    if (dr*dr + dc*dc <= radius*radius) {
                        grid[er][ec] = (Cell){ EMPTY, 0, false};
                    }
                }
            }
            particles_spawn(cell_sx(c), cell_sy(r), tier_color(grid[r][c].tier), EFFECT_EXPLODE);
            sound_play_explode();
        } else {
            bool lower_wins = rr == r+1 && cc == c;
            int wr = lower_wins? rr : r, wc = lower_wins ? cc : c;
            int lr = lower_wins ? r : rr, lc = lower_wins ? c : cc;
            grid[wr][wc].tier++;
            grid[wr][wc].fresh = true;
            score += (1 << tier);
            particles_spawn(px, py, tier_color(grid[wr][wc].tier), EFFECT_MERGE);
            sound_play_merge(tier);
            grid[lr][lc] = (Cell){ EMPTY, 0, false };
        } 
        
        used[r][c] = used[rr][cc] = true;
        return true;
    }
    return false;
}

static bool merge_once(void) {
    bool used[ROWS][COLS] = {0};
    bool any = false;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!grid[r][c].fresh || used[r][c] || grid[r][c].type != SAND) continue;
            if (try_merge(r, c, used)) any = true;
        }
    }
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c].fresh || used[r][c] || grid[r][c].type != SAND) continue;
            if (try_merge(r, c, used)) any = true;
        }
    }
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c].fresh = false;   
        }
    }
    return any;
}

int grid_get_level(void) {
    int level = score / 10000;
    return level >= MAX_LEVEL ? MAX_LEVEL - 1 : level;
}

void grid_update(double dt) {
    particles_update(dt);
    static double sim_acc = 0.0;
    static double piece_acc = 0.0;
    double SIM_HZ = sim_hz_table[grid_get_level()];
    double PIECE_HZ = piece_hz_table[grid_get_level()];

    double piece_step = soft_drop ? (1.0 / (PIECE_HZ * 2)) : (1.0 / PIECE_HZ);
    piece_acc += dt;
    
    while (piece_acc >=  piece_step) {
        piece_acc -= piece_step;
        bool settled = grid_settled();

        if (!piece.active && settled) {
            piece_spawn();
        }

        if (piece.active && settled) {
            if (piece.r + 1 < ROWS && grid[piece.r+1][piece.c].type == EMPTY) {
                piece.r++;
            } else {
                grid[piece.r][piece.c] = piece.cell;
                grid[piece.r][piece.c].fresh = true;
                piece.active = false;
                can_hold = true;
                particles_spawn(cell_sx(piece.c), cell_sy(piece.r) + CELL / 2.0f, tier_color(piece.cell.tier), EFFECT_LAND);
                sound_play_land();
            }
        }
    }

    sim_acc += dt;
    int max_ticks = 1;
    while (sim_acc >= 1.0 / SIM_HZ && max_ticks-- > 0) {
        sim_acc -= 1.0 / SIM_HZ;
        merge_once();
        fall_once();
    }
}

static void draw_star(int cx, int cy, int points, float outer_r, float inner_r, Color color) {
    float angle_step = PI / points;
    Vector2 verts[points * 2];
    for (int i = 0; i < points * 2; i++) {
        float angle = i * angle_step - PI / 2.0f;
        float r = (i % 2 == 0) ? outer_r : inner_r;
        verts[i] = (Vector2){ cx + cosf(angle) * r, cy + sinf(angle) * r };
    }
    for (int i = 0; i < points * 2; i++) {
        Vector2 a = verts[i];
        Vector2 b = verts[(i+1) % (points * 2)];
        DrawLineV(a, b, color);
    }
}

static void draw_tier_shape(int cx, int cy, int tier, Color color) {
    float r = CELL * 0.4f;
    Vector2 center = { cx, cy };

    int sides[] =  { 0, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0 };
    int points[] = { 0, 0, 0, 0, 0, 3, 4, 5, 6, 7, 8 };
    float angles[] = { 0, 0, 45, 0, 0, 0, 0, 0, 0, 0, 0 };

    if (tier == 0) {
        DrawCircleLines(cx, cy, r, color);
    } else if (tier >= 5) {
        draw_star(cx, cy, points[tier], r, r * 0.30f, color);
    } else {
        DrawPolyLines(center, sides[tier], r, angles[tier], color);
    }
}

int grid_get_score(void) { return score; }

void grid_init(void) {
    for (int i = 0; i < BAG_SIZE; i++) recency [i] = 1.0f;
    for (int i = 0; i < NEXT_COUNT; i++) next_queue[i] = gen_next();
    piece_spawn();
    font = LoadFontEx("assets/iosevka.ttf", CELL, NULL, 0);
}

void grid_draw(void) {
    int ox = (GetScreenWidth() - GRID_PIXEL_W) / 2;
    int oy = (GetScreenHeight() - GRID_PIXEL_H) / 2;

    BeginDrawing();
    ClearBackground(BLACK);

    Color grid_color = { 255, 255, 255, 20 };
    for (int r = 0; r <= ROWS; r++) {
        DrawLine(ox, oy + r * CELL, ox + GRID_PIXEL_W, oy + r * CELL, grid_color);
    }
    for (int c = 0; c <= COLS; c++) {
        DrawLine(ox + c * CELL, oy, ox + c * CELL, oy + GRID_PIXEL_H, grid_color);
    }
    
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c].type == SAND) {
                draw_tier_shape(ox + c * CELL + CELL/2, oy + r * CELL + CELL/2, grid[r][c].tier, tier_color(grid[r][c].tier));
            }
        }
    }
    if (piece.active) {
        draw_tier_shape(ox + piece.c * CELL + CELL/2, oy + piece.r * CELL + CELL/2, piece.cell.tier, tier_color(piece.cell.tier));
        int ghost_r = piece.r;
        while (ghost_r + 1 < ROWS && grid[ghost_r+1][piece.c].type == EMPTY) {
            ghost_r++;
        }

        if (ghost_r != piece.r) {
            Color ghost_color = tier_color(piece.cell.tier);
            ghost_color.a = 80;
            draw_tier_shape(ox + piece.c * CELL + CELL/2, oy + ghost_r * CELL + CELL/2, piece.cell.tier, ghost_color);
        }
        for (int lr = piece.r; lr <= ghost_r; lr++) {
            Color line_color = tier_color(piece.cell.tier);
            line_color.a = 25;
            DrawRectangle(ox + piece.c * CELL, oy + lr * CELL, CELL, CELL, line_color);
        }
    }

    int qx = ox + GRID_PIXEL_W + CELL;
    int qy = oy;

    Vector2 q_size = MeasureTextEx(font, "next", CELL, 0);
    DrawTextEx(font, "next", (Vector2){qx +  CELL/2.0f - q_size.x/2, oy - CELL}, CELL, 0, WHITE);
    for (int i = 0; i < BAG_SIZE; i++) {
        int tier = next_queue[i];
        draw_tier_shape(qx + CELL/2, qy + i * CELL + CELL/2, tier, tier_color(tier));
    }

    Vector2 h_size = MeasureTextEx(font, "hold", CELL, 0);
    DrawTextEx(font, "hold", (Vector2){ox - CELL - CELL + CELL/2.0f - h_size.x/2, oy - CELL}, CELL, 0, WHITE);
    if (hold_tier != -1) {
        Color c = can_hold ? tier_color(hold_tier) : (Color){100, 100, 100, 255};
        draw_tier_shape(ox - CELL - CELL/2, oy + CELL/2, hold_tier, c);
    }

    char score_buf[32];
    snprintf(score_buf, sizeof(score_buf), "%d", grid_get_score());
    Vector2 score_size = MeasureTextEx(font, score_buf, CELL, 0);
    DrawTextEx(font, score_buf, (Vector2){ox + GRID_PIXEL_W/2.0f - score_size.x/2, oy - CELL}, CELL, 0, WHITE);   

    int score_in_level = score % 10000;
    float progress = score_in_level / 10000.0f;
    DrawRectangle(ox, oy + GRID_PIXEL_H + 4, (int)(GRID_PIXEL_W * progress), 3, WHITE);
    
    particles_draw();
    EndDrawing();
}
