#include <SDL2/SDL.h>
#include "./constants.cpp"

struct projectiles_clip_container {
    SDL_Rect gProjectilesClips[PROJECTILE_ROW*PROJECTILE_COLUMN];
    constexpr projectiles_clip_container() : gProjectilesClips() {
        for(int i = 0; i < PROJECTILE_ROW; i++) {
            for(int j = 0; j < PROJECTILE_COLUMN; j++) {
                gProjectilesClips[i*PROJECTILE_COLUMN + j].x = j * PROJECTILE_WIDTH;
                gProjectilesClips[i*PROJECTILE_COLUMN + j].y = i * PROJECTILE_HEIGHT;
                gProjectilesClips[i*PROJECTILE_COLUMN + j].w = PROJECTILE_WIDTH;
                gProjectilesClips[i*PROJECTILE_COLUMN + j].h = PROJECTILE_HEIGHT;
            }
        }
    }
};