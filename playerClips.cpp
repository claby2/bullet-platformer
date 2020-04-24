#include <SDL2/SDL.h>
#include "./constants.cpp"

struct player_clip_container {
    SDL_Rect gHeroKnightClips[SPRITE_ROW*SPRITE_COLUMN];
    constexpr player_clip_container() : gHeroKnightClips() {
        for(int i = 0; i < SPRITE_ROW; i++) {
            for(int j = 0; j < SPRITE_COLUMN; j++) {
                gHeroKnightClips[i*SPRITE_COLUMN + j].x = j * SPRITE_WIDTH;
                gHeroKnightClips[i*SPRITE_COLUMN + j].y = i * SPRITE_HEIGHT;
                gHeroKnightClips[i*SPRITE_COLUMN + j].w = SPRITE_WIDTH;
                gHeroKnightClips[i*SPRITE_COLUMN + j].h = SPRITE_HEIGHT;
            }
        }
    }
};