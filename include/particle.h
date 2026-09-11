#pragma once
#include "raylib.h"

typedef enum {
    EFFECT_LAND,
    EFFECT_MERGE,
    EFFECT_EXPLODE,
} EffectType;

void particles_spawn(float x, float y, Color color, EffectType type);
void particles_update(double dt);
void particles_draw(void);
