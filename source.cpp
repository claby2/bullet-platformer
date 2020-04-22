#include "C:/MinGW/include/SDL2/SDL.h"
#include "C:/MinGW/include/SDL2/SDL_image.h"
#include "C:/MinGW/include/SDL2/SDL_ttf.h"
#include <string>
#include <math.h>
#include <chrono>
#include <map>
#include <iostream>
#include "./playerAnimations.cpp"
#include "./playerClips.cpp"
#include "./tilesClips.cpp"
#include "./levels.cpp"
#include "./constants.cpp"

anim playerAnimations;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;

int currentLevel = 1;

class LTexture {
    public:
		LTexture() {
            mTexture = NULL;
            mWidth = 0;
            mHeight = 0;
        }
		~LTexture() {
            free();
        }
		bool loadFromFile( std::string path ) {
            free();
            SDL_Texture* newTexture = NULL;
            SDL_Surface* loadedSurface = IMG_Load(path.c_str());
            SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));
            newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
            SDL_FreeSurface(loadedSurface);
            mTexture = newTexture;
            return mTexture != NULL;
        }
        bool loadFromRenderedText(std::string textureText, SDL_Color textColor) {
            free();
            SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
            mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
            mWidth = textSurface->w;
            mHeight = textSurface->h;
            SDL_FreeSurface(textSurface);
            return mTexture != NULL;
        }
		void free() {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
            if(mTexture != NULL){
                SDL_DestroyTexture(mTexture);
                mTexture = NULL;
                mWidth = 0;
                mHeight = 0;
            }
        }
		void setColor( Uint8 red, Uint8 green, Uint8 blue ) {
            SDL_SetTextureColorMod(mTexture, red, green, blue);
        }
		void setBlendMode( SDL_BlendMode blending ) {
            SDL_SetTextureBlendMode(mTexture, blending);
        }
		void setAlpha( Uint8 alpha ) {
            SDL_SetTextureAlphaMod(mTexture, alpha);
        }
		void render( int x, int y, const SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE, float widthMultiplier = 1, float heightMultiplier = 1){
            SDL_Rect renderQuad = { x, y, mWidth, mHeight };

            if( clip != NULL )
            {
                renderQuad.w = clip->w * widthMultiplier;
                renderQuad.h = clip->h * heightMultiplier;
            }

            SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
        }
		int getWidth(){
            return mWidth;
        }
		int getHeight(){
            return mHeight;
        }

	private:
		SDL_Texture* mTexture;
		int mWidth;
		int mHeight;
};


constexpr player_clip_container playerClips;
constexpr tiles_clip_container tilesClips;
LTexture gSpriteSheetTexture;
LTexture gTilesSpriteSheetTexture;
LTexture gTextTexture;

class Player {
    public:
        Player() {
            velocity = 2.0;
            x = SCREEN_WIDTH/2;
            y = SCREEN_HEIGHT/2;
            speedX = 0;
            speedY = 0;
            acceleration = 0.1;
            playerState = "idle";
            frame = 0;
            clip = playerAnimations.idle;
            direction = true;
            isFalling = false;
            isAttacking = false;
            isContactWall = false;
            jumpMultiplier = 3.0;
        }

        void setClip() {
            std::pair<int, int> initialClip = clip;

            if(isContactWall) {
                playerState = "wallSlide";
            } else if(isAttacking) {
                playerState = "attack1";
            } else if(speedY < 0) {
                playerState = "jump";
            } else if(isFalling) {
                playerState = "fall";
            } else if(speedX) {
                playerState = "run";
            } else {
                playerState = "idle";
            }

            if(playerState == "idle") clip = playerAnimations.idle;
            else if(playerState == "run") clip = playerAnimations.run; 
            else if(playerState == "attack1") clip = playerAnimations.attack1; 
            else if(playerState == "attack2") clip = playerAnimations.attack2; 
            else if(playerState == "attack3") clip = playerAnimations.attack3;
            else if(playerState == "jump") clip = playerAnimations.jump; 
            else if(playerState == "fall") clip = playerAnimations.fall; 
            else if(playerState == "hurt") clip = playerAnimations.hurt;
            else if(playerState == "death") clip = playerAnimations.death; 
            else if(playerState == "blockIdle") clip = playerAnimations.blockIdle; 
            else if(playerState == "block") clip = playerAnimations.block;
            else if(playerState == "roll") clip = playerAnimations.roll;
            else if(playerState == "ledgeGrab") clip = playerAnimations.ledgeGrab;
            else if(playerState == "wallSlide") clip = playerAnimations.wallSlide;
 
            if(initialClip != clip) {
                frame = clip.first*ANIMATION_FRAME_RATE;
            }
        }

        void setDirection() {
            if(speedX > 0) {
                direction = false;
            } else if(speedX < 0){
                direction = true;
            }
        }
        
        bool willIntersectTile() {

            bool isIntersect = false;
 
            std::pair<float, float> playerCoords = {x + speedX + ((SPRITE_WIDTH - HITBOX_WIDTH)/2), y + speedY};
            std::pair<int, int> topLeft = {playerCoords.first / LEVEL_WIDTH, playerCoords.second / LEVEL_HEIGHT};
            std::pair<int, int> bottomRight = {(playerCoords.first + HITBOX_WIDTH) / LEVEL_WIDTH, (playerCoords.second + HITBOX_HEIGHT) / LEVEL_HEIGHT};

            for(int i = topLeft.first; i <= bottomRight.first; i++) {
                for(int j = topLeft.second; j <= bottomRight.second; j++) {
                    if(levels[currentLevel][j*LEVEL_WIDTH + i] != -1) {
                        isIntersect = true;
                    }
                }
            }

            return isIntersect;
        }

        void move() {
            bool isIntersect = willIntersectTile();

            x += speedX;
            float initialY = y;
            y += speedY;

            if(isIntersect) {
                isContactWall = true;
                x -= speedX;
                y -= speedY;
                speedX = 0;
                speedY = 0;
            }

            speedY += acceleration;

            if(willIntersectTile()) {
                speedY = 0;
            }

            isFalling = initialY < y ? true : false;

            std::cout << isContactWall << "\n";
        }

        void handleEvents(SDL_Event& event) {
            if(event.type == SDL_KEYDOWN && event.key.repeat == 0){
                switch(event.key.keysym.sym){
                    case SDLK_w: speedY = -jumpMultiplier*velocity; break;
                    case SDLK_SPACE: speedY = -jumpMultiplier*velocity; break;
                    case SDLK_a: if(!isContactWall) speedX += -velocity; else isContactWall = false; break;
                    case SDLK_d: if(!isContactWall) speedX += velocity; else isContactWall = false; break;
                }
            } else if(event.type == SDL_KEYUP && event.key.repeat == 0){
                switch(event.key.keysym.sym){
                    case SDLK_a: if(!isContactWall) speedX += velocity; else isContactWall = false; break;
                    case SDLK_d: if(!isContactWall) speedX += -velocity; else isContactWall = false; break;
                }
            }
            if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                isAttacking = true;
            }
        }

        void render() {
            if(playerState == "jump" || playerState == "fall") {
                frame = frame / ANIMATION_FRAME_RATE >= clip.second ? clip.second*ANIMATION_FRAME_RATE : frame + 1;
            } else {
                if((playerState == "attack1" || playerState == "attack2" || playerState == "attack3") && (frame / ANIMATION_FRAME_RATE >= clip.second)) {
                    isAttacking = false;
                }
                frame = frame / ANIMATION_FRAME_RATE >= clip.second ? clip.first*ANIMATION_FRAME_RATE : frame + 1;
            }
            res = direction  ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            gSpriteSheetTexture.render(x, y, &playerClips.gHeroKnightClips[frame / ANIMATION_FRAME_RATE], 0.0, NULL, res);
        }

    private:
        SDL_RendererFlip res;
        float velocity;
        float x;
        float y;
        float speedX;
        float speedY;
        float acceleration;
        int frame;
        std::pair<int, int> clip;
        bool direction;
        std::string playerState;
        bool isFalling;
        bool isAttacking;
        bool isContactWall;
        float jumpMultiplier;
};

class Level {
    public:
        Level() {
            widthMultiplier = TILE_HITBOX_WIDTH / TILE_WIDTH;
            heightMultiplier = TILE_HITBOX_HEIGHT / TILE_HEIGHT;
        }

        void render() {
            for(int i = 0; i < LEVEL_HEIGHT; i++) {
                for(int j = 0; j < LEVEL_WIDTH; j++) {
                    int tile = levels[currentLevel][i*LEVEL_WIDTH + j];
                    if(tile != -1) {
                        int x = j*TILE_HITBOX_WIDTH;
                        int y = i*TILE_HITBOX_HEIGHT;
                        gTilesSpriteSheetTexture.render(x, y, &tilesClips.gTilesClips[tile], 0.0, NULL, SDL_FLIP_NONE, widthMultiplier, heightMultiplier);
                    }
                }
            }
        }
    private:                        
        float widthMultiplier;
        float heightMultiplier;
};

bool loadMedia() {
    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 28);
    gSpriteSheetTexture.loadFromFile("assets/hero_knight_sprite_sheet.png");
    gTilesSpriteSheetTexture.loadFromFile("assets/tiles_sprite_sheet.png");
}

void close() {
    gSpriteSheetTexture.free();
    gTilesSpriteSheetTexture.free();
    gTextTexture.free();
    TTF_CloseFont(gFont);
    gFont = NULL;
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    gWindow = SDL_CreateWindow("Bullet Platformer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
}

int main(int argc, char* args[]) {
    init();
    loadMedia();

    Player player;
    Level level;

    bool quit = false;
    SDL_Event event;

    while(!quit) {

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }

            player.handleEvents(event);
        }

        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(gRenderer);

        level.render();

        player.move();
        player.setDirection();
        player.setClip();
        player.render();
        SDL_RenderPresent(gRenderer);

    }

    close();
    return 0;
}