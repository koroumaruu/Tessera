#include "raylib.h"
#include "grid.h"
#include "sound.h"

#ifdef PLATFORM_WEB
    #include <emscripten/emscripten.h>
#endif

static void main_loop(void) {
    double dt = GetFrameTime();
    piece_input(dt);
    grid_update(dt);
    grid_draw();
    // sound_update_music();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    #ifndef PLATFORM_WEB
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    #endif

    InitWindow(1280, 720, "Tessera");
    
    // set fps (anything above 30 makes the inputs too sensitive and this is why i forked this project lmao) 
    #ifndef PLATFORM_WEB
        SetTargetFPS(30); 
    #endif

    sound_init();
    grid_init();

#ifdef PLATFORM_WEB
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!WindowShouldClose()) {
        main_loop();
    }  
#endif

    sound_close();
    CloseWindow();
    return 0;
}
