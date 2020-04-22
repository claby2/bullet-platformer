#include <SDL2/SDL.h>
#include "./constants.cpp"

struct tiles_clip_container {
    SDL_Rect gTilesClips[192];
    constexpr tiles_clip_container() : gTilesClips() {
        for(int i = 0; i < TILE_ROW; i++) {
            for(int j = 0; j < TILE_COLUMN; j++) {
                gTilesClips[i*TILE_COLUMN + j].x = j * TILE_WIDTH;
                gTilesClips[i*TILE_COLUMN + j].y = i * TILE_HEIGHT;
                gTilesClips[i*TILE_COLUMN + j].w = TILE_WIDTH;
                gTilesClips[i*TILE_COLUMN + j].h = TILE_HEIGHT;
            }
        }
    }
};