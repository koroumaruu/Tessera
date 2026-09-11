#include "particle.h"
#include "raylib.h"
#include <math.h>

#define MAX_PARTICLES 512

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float size;
    Color color;
} Particle;

static Particle pool[MAX_PARTICLES];

static Particle make_particle(float x, float y, float vx, float vy, float life, float size, Color color) {
    return (Particle){ x, y, vx, vy, life, size, color };
}

static void spawn(float x, float y, float vx, float vy, float life, float size, Color color) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (pool[i].life <= 0.0f) {
            pool[i] = make_particle(x, y, vx, vy, life, size, color);
            return;
        }
    }
}

void particles_spawn(float x, float y, Color color, EffectType type) {
    switch (type) {
        case EFFECT_LAND: {
            int count = 10;
            for (int i = 0; i < count; i++) {
                float angle = ((float)i / count) * 2.0 * PI;
                float speed = (float)GetRandomValue(20, 50);
                float vx = cosf(angle) * speed;
                float vy = sinf(angle) * speed * 0.5f - 30.0f;
                Color c = GRAY;
                c.a = 200;
                spawn(x, y, vx, vy, 0.4f, 2.0f, c);
            }
            break;       
        }
        case EFFECT_MERGE: {
            int count = 16;
            for (int i = 0; i < count; i++) {
                float angle = ((float)i / count) * 2.0 * PI;
                float speed = (float)GetRandomValue(40, 100);
                float vx = cosf(angle) * speed;
                float vy = sinf(angle) * speed;
                Color c = color;
                c.a = 255;
                spawn(x, y, vx, vy, 0.6f, 3.0f, c);
            }
            break;
        }
        case EFFECT_EXPLODE: {
            int count = 48;
            for (int i = 0; i < count; i++) {
                float angle = ((float)i / count) * 2.0 * PI;
                float speed = (float)GetRandomValue(200, 300);
                float vx = cosf(angle) * speed;
                float vy = sinf(angle) * speed;
                Color c = ColorFromHSV((float)GetRandomValue(255, 360), 1.0f, 1.0f);
                c.a = 255;
                spawn(x, y, vx, vy, 1.0f, 4.0f, c);
            }
            break;
        }
    } 
}

void particles_update(double dt) {
    float gravity = 200.0f;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (pool[i].life <= 0.0f) continue;
        pool[i].life -= dt * 2.0f;
        pool[i].x += pool[i].vx * dt;
        pool[i].y += pool[i].vy * dt;
        pool[i].vy += gravity * dt;
    }
}

void particles_draw(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (pool[i].life <= 0.0f) continue;
        Color c = pool[i].color;
        c.a = (unsigned char)(pool[i].life * 255);
        DrawCircleV((Vector2){ pool[i].x, pool[i].y }, pool[i].size, c);
    }
}
