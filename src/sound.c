#include "sound.h"
#include "raylib.h"

#define MERGE_SOUND_COUNT 10

static Sound s_land;
static Sound s_merge[MERGE_SOUND_COUNT];
static Sound s_explode;
static Music s_music;

void sound_init(void) {
    InitAudioDevice();
    s_land = LoadSound("assets/sfx/land.wav");

    s_merge[0] = LoadSound("assets/sfx/merge1.wav");
    s_merge[1] = LoadSound("assets/sfx/merge2.wav");
    s_merge[2] = LoadSound("assets/sfx/merge3.wav");
    s_merge[3] = LoadSound("assets/sfx/merge4.wav");
    s_merge[4] = LoadSound("assets/sfx/merge5.wav");
    s_merge[5] = LoadSound("assets/sfx/merge6.wav");
    s_merge[6] = LoadSound("assets/sfx/merge7.wav");
    s_merge[7] = LoadSound("assets/sfx/merge8.wav");
    s_merge[8] = LoadSound("assets/sfx/merge9.wav");
    s_merge[9] = LoadSound("assets/sfx/merge10.wav");

    s_explode = LoadSound("assets/sfx/merge_explode.wav");

    s_music = LoadMusicStream("assets/sfx/music0.wav");
    s_music.looping = true;
    SetMusicVolume(s_music, 0.2f);
    PlayMusicStream(s_music);
}

void sound_close(void) {
    UnloadSound(s_land);
    for (int i = 0; i < MERGE_SOUND_COUNT; i++) {
        UnloadSound(s_merge[i]);
    }
    UnloadSound(s_explode);
    UnloadMusicStream(s_music);
    CloseAudioDevice();
}

void sound_play_land(void) {
    PlaySound(s_land);
}

void sound_play_merge(int tier) {
    if (tier < 0 || tier >= MERGE_SOUND_COUNT) return;
    PlaySound(s_merge[tier]);
}

void sound_play_explode(void) {
    PlaySound(s_explode);
}

// void sound_update_music(void) {
//     UpdateMusicStream(s_music);
// }
