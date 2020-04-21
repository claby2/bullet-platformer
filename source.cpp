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
#include "./constants.cpp"

anim playerAnimations;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;

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
		void render( int x, int y, const SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE){
            SDL_Rect renderQuad = { x, y, mWidth, mHeight };

            if( clip != NULL )
            {
                renderQuad.w = clip->w;
                renderQuad.h = clip->h;
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


constexpr clip_container clips;
LTexture gSpriteSheetTexture;
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
        }

        void setClip() {
            std::pair<int, int> initialClip = clip;

            if(isAttacking) {
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

        void move() {
            x += speedX;
            if((x < -HITBOX_WIDTH) || (x + (SPRITE_WIDTH - HITBOX_WIDTH)> SCREEN_WIDTH)){
                x -= speedX;
            }

            float initialY = y;

            y += speedY;
            if((y < 0) || (y +  HITBOX_HEIGHT > SCREEN_HEIGHT)){
                y -= speedY;
            }

            isFalling = initialY < y ? true : false;

            speedY += speedY < HITBOX_HEIGHT ? acceleration : 0;
        }

        void handleEvents(SDL_Event& event) {
            if(event.type == SDL_KEYDOWN && event.key.repeat == 0){
                switch(event.key.keysym.sym){
                    case SDLK_w: speedY = -2*velocity; break;
                    case SDLK_SPACE: speedY = -2*velocity; break;
                    // case SDLK_s: speedY += velocity; break;
                    case SDLK_a: speedX -= velocity; break;
                    case SDLK_d: speedX += velocity; break;
                }
            } else if(event.type == SDL_KEYUP && event.key.repeat == 0){
                switch(event.key.keysym.sym){
                    // case SDLK_w: speedY -= velocity; break;
                    // case SDLK_s: speedY -= velocity; break;
                    case SDLK_a: speedX += velocity; break;
                    case SDLK_d: speedX -= velocity; break;
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
            gSpriteSheetTexture.render(x, y, &clips.gHeroKnightClips[frame / ANIMATION_FRAME_RATE], 0.0, NULL, res);
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
};

bool loadMedia() {
    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 28);
    gSpriteSheetTexture.loadFromFile("assets/hero_knight_sprite_sheet.png");
}

void close() {
    gSpriteSheetTexture.free();
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

    bool quit = false;
    SDL_Event event;

    while(!quit) {

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }

            player.handleEvents(event);
        }

        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x000, 0x00, 0x00);
        SDL_RenderClear(gRenderer);
        player.move();
        player.setDirection();
        player.setClip();
        player.render();
        SDL_RenderPresent(gRenderer);

    }

    close();
    return 0;
}