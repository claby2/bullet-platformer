#ifndef CONSTANTS_CPP
#define CONSTANTS_CPP
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 900;

const int SPRITE_ROW = 9;
const int SPRITE_COLUMN = 10;
const int SPRITE_WIDTH = 100;
const int SPRITE_HEIGHT = 55;

const int HITBOX_WIDTH = 30;
const int HITBOX_HEIGHT = 55;

const int LEVEL_WIDTH = 30;
const int LEVEL_HEIGHT = 30;

const int TILE_ROW = 16;
const int TILE_COLUMN = 12;
const float TILE_WIDTH = 16;
const float TILE_HEIGHT = 16;

const float TILE_HITBOX_WIDTH = 30;
const float TILE_HITBOX_HEIGHT = 30;

const int PROJECTILE_ROW = 6;
const int PROJECTILE_COLUMN = 13;
const float PROJECTILE_WIDTH = 16;
const float PROJECTILE_HEIGHT = 16;

const float SPAWNER_HITBOX_WIDTH = 30;
const float SPAWNER_HITBOX_HEIGHT = 30;

const float PROJECTILE_HITBOX_WIDTH = 15;
const float PROJECTILE_HITBOX_HEIGHT = 15;

const int ANIMATION_FRAME_RATE = 8;

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

struct tiles_clip_container {
    SDL_Rect gTilesClips[TILE_ROW*TILE_COLUMN];
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

#endif