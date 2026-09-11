#pragma once

#include "raylib.h"

void sound_init(void);
void sound_close(void);

void sound_play_land(void);
void sound_play_merge(int tier);
void sound_play_explode(void);

// void sound_update_music(void);
