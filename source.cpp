#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <math.h>
#include <chrono>
#include <map>
#include <iostream>
#include <vector>
#include <string.h>
#include "./playerAnimations.cpp"
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
        bool loadFromFile(std::string path) {
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
constexpr projectiles_clip_container projectilesClips;
LTexture gSpriteSheetTexture;
LTexture gTilesSpriteSheetTexture;
LTexture gProjectilesSpriteSheetTexture;
LTexture gTextTexture;

class Player {
    public:
        float x;
        float y;

        Player() {
            velocity = 0.2;
            x = SCREEN_WIDTH/2;
            y = SCREEN_HEIGHT/2;
            speedX = 0;
            speedY = 0;
            acceleration = 0.001;
            playerState = "idle";
            frame = 0;
            clip = playerAnimations.idle;
            direction = true;
            isFalling = false;
            isAttacking = false;
            isJumping = false;
            jumpMultiplier = 3.7;
            isAPressed = isDPressed = false;
            animationFPS = ANIMATION_FRAME_RATE;
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
                frame = clip.first*animationFPS;
            }
        }

        void setDirection() {
            if(speedX > 0) {
                direction = false;
            } else if(speedX < 0){
                direction = true;
            }
        }

        bool willIntersectTile(float speedX, float speedY, float delta, bool debug) {
            SDL_Rect recty, actual;

            if(debug) SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);

            bool isIntersect = false;

            std::pair<float, float> playerCoords = {x + (speedX * delta)+ ((SPRITE_WIDTH - HITBOX_WIDTH)/2), y + (speedY * delta) };
            std::pair<int, int> topLeft = {playerCoords.first / LEVEL_WIDTH, playerCoords.second / LEVEL_HEIGHT};
            std::pair<int, int> bottomRight = {(playerCoords.first + HITBOX_WIDTH) / LEVEL_WIDTH, (playerCoords.second + HITBOX_HEIGHT) / LEVEL_HEIGHT};

            for(int i = topLeft.first; i <= bottomRight.first; i++) {
                for(int j = topLeft.second; j <= bottomRight.second; j++) {
                    if(levels[currentLevel][j*LEVEL_WIDTH + i] != -1) {
                        isIntersect = true;
                    }

                    if(debug){
                        recty = {i*TILE_HITBOX_WIDTH, j*TILE_HITBOX_HEIGHT, TILE_HITBOX_WIDTH, TILE_HITBOX_HEIGHT};
                        SDL_RenderFillRect(gRenderer, &recty);
                        SDL_RenderDrawRect(gRenderer, &recty);
                        SDL_RenderDrawPoint(gRenderer, i*TILE_HITBOX_WIDTH, j*TILE_HITBOX_HEIGHT);
                        SDL_RenderDrawPoint(gRenderer, i*TILE_HITBOX_WIDTH + TILE_HITBOX_WIDTH, j*TILE_HITBOX_HEIGHT);
                        SDL_RenderDrawPoint(gRenderer, i*TILE_HITBOX_WIDTH, j*TILE_HITBOX_HEIGHT + TILE_HITBOX_HEIGHT);
                        SDL_RenderDrawPoint(gRenderer, i*TILE_HITBOX_WIDTH + TILE_HITBOX_WIDTH, j*TILE_HITBOX_HEIGHT + TILE_HITBOX_HEIGHT);
                    }
                }
            }

            return isIntersect;
        }

        void move(float delta) {
            float initialY = y;
            speedY += acceleration*delta;
            if(!willIntersectTile(speedX, speedY, delta, false)){
                x += speedX*delta;
                y += speedY*delta;
            } else if(!willIntersectTile(0, speedY, delta, false)){
                y += speedY * delta;
                speedX = 0;
            } else if(!willIntersectTile(speedX, 0, delta, false)){
                isJumping = false;
                x += speedX * delta;
                speedY = 0;
            } else {
                speedX = 0;
                speedY = 0;
            }
            isFalling = initialY < y ? true : false;
        }

        void handleEvents(SDL_Event& event) {
            if(event.type == SDL_KEYDOWN && event.key.repeat == 0){
                switch(event.key.keysym.sym){
                    case SDLK_w: if(!isJumping){speedY = -jumpMultiplier*velocity; isJumping = true;} break;
                    case SDLK_SPACE: if(!isJumping){speedY = -jumpMultiplier*velocity; isJumping = true;} break;
                    case SDLK_a: isAPressed = true; break;
                    case SDLK_d: isDPressed = true; break;
                }
            } else if(event.type == SDL_KEYUP && event.key.repeat == 0){
                switch(event.key.keysym.sym){
                    case SDLK_a: if(isAPressed){ speedX = 0; isAPressed = false; } break;
                    case SDLK_d: if(isDPressed){ speedX = 0; isDPressed = false; } break;
                }
            }
            if(isAPressed && isDPressed) speedX = 0;
            else if(isAPressed) speedX = -velocity;
            else if(isDPressed) speedX = velocity;
            if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                isAttacking = true;
            }
        }

        void render(int refreshRate) {
            animationFPS = floor(0.06 * refreshRate);
            if(playerState == "jump" || playerState == "fall") {
                frame = frame / animationFPS >= clip.second ? clip.second*animationFPS : frame + 1;
            } else {
                if((playerState == "attack1" || playerState == "attack2" || playerState == "attack3") && (frame / animationFPS >= clip.second)) {
                    isAttacking = false;
                }
                frame = frame / animationFPS >= clip.second ? clip.first*animationFPS : frame + 1;
            }
            res = direction  ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            gSpriteSheetTexture.render(x, y, &playerClips.gHeroKnightClips[frame / animationFPS], 0.0, NULL, res);
        }

    private:
        SDL_RendererFlip res;
        float velocity;
        float speedX;
        float speedY;
        float acceleration;
        int frame;
        std::pair<int, int> clip;
        bool direction;
        std::string playerState;
        bool isFalling;
        bool isAttacking;
        bool isAPressed, isDPressed;
        bool isJumping;
        float jumpMultiplier;
        int animationFPS;
};

class Spawner {
    public:
        float lifetime;
        float fireState;
        int type;
        float x;
        float y;

        Spawner() {
            widthMultiplier = SPAWNER_HITBOX_WIDTH / PROJECTILE_WIDTH;
            heightMultiplier = SPAWNER_HITBOX_HEIGHT / PROJECTILE_HEIGHT;
            type = 20;
            lifetime = 10000.0;
            fireRate = 1000.0;
            fireState = fireRate;
            yOffset = 0;
            yOffsetFactor = -1;
            yOffsetShake = 10;
        }

        void spawn(int spawnerMap[LEVEL_HEIGHT * LEVEL_WIDTH]) {
            std::vector<std::pair<int, int> > possibleLocations;
            for(int i = 0; i < LEVEL_HEIGHT; i++) {
                for(int j = 0; j < LEVEL_WIDTH; j++) {
                    if(spawnerMap[i*LEVEL_WIDTH + j] == 0 && levels[currentLevel][i*LEVEL_WIDTH + j] == -1) {
                        possibleLocations.push_back(std::make_pair(j, i));
                    }
                }
            }
            std::pair<int, int> randomLocation = possibleLocations[rand()%possibleLocations.size()];
            spawnerMap[randomLocation.second * LEVEL_WIDTH + randomLocation.first] = 1;
            x = randomLocation.first * TILE_HITBOX_WIDTH;
            y = randomLocation.second * TILE_HITBOX_HEIGHT;
        }

        void resetFireRate() {
            fireState = fireRate;
        }

        void render(float delta) {
            lifetime -= delta;
            fireState -= delta;

            yOffset += yOffsetFactor * (delta / yOffsetShake);
            if(yOffset >= 2*(delta / yOffsetShake) || yOffset <= -2*(delta / yOffsetShake)) {
                yOffsetFactor = yOffsetFactor == -1 ? yOffsetFactor = 1 : yOffsetFactor = -1;
                yOffset = 0;
            }

            gProjectilesSpriteSheetTexture.render(x, y + yOffset, &projectilesClips.gProjectilesClips[type], 0.0, NULL, SDL_FLIP_NONE, widthMultiplier, heightMultiplier);
        }

    private:
        float widthMultiplier;
        float heightMultiplier;
        float fireRate;
        float yOffset;
        float yOffsetFactor;
        float yOffsetShake;
};

class Projectile {
    public:
        Projectile() {
            widthMultiplier = PROJECTILE_HITBOX_WIDTH / PROJECTILE_WIDTH;
            heightMultiplier = PROJECTILE_HITBOX_HEIGHT / PROJECTILE_HEIGHT;
        }

        void setProperties(float spawnerX, float spawnerY, float spawnerType, float setSpeedX = 0, float setSpeedY = 0) {
            x = spawnerX + ((SPAWNER_HITBOX_WIDTH - PROJECTILE_HITBOX_WIDTH) / 2);
            y = spawnerY + ((SPAWNER_HITBOX_HEIGHT - PROJECTILE_HITBOX_HEIGHT) / 2);
            type = spawnerType;
            speedX = setSpeedX;
            speedY = setSpeedY;
        }

        bool willIntersectTile(float delta) {
            bool isIntersect = false;

            std::pair<float, float> playerCoords = {x + (speedX * delta), y + (speedY * delta) };
            std::pair<int, int> topLeft = {playerCoords.first / LEVEL_WIDTH, playerCoords.second / LEVEL_HEIGHT};
            std::pair<int, int> bottomRight = {(playerCoords.first + PROJECTILE_HITBOX_WIDTH) / LEVEL_WIDTH, (playerCoords.second + PROJECTILE_HITBOX_HEIGHT) / LEVEL_HEIGHT};

            for(int i = topLeft.first; i <= bottomRight.first; i++) {
                for(int j = topLeft.second; j <= bottomRight.second; j++) {
                    if(levels[currentLevel][j*LEVEL_WIDTH + i] != -1) {
                        isIntersect = true;
                    }
                }
            }

            return isIntersect;
        }

        bool isIntersectPlayer(float playerX, float playerY) {
            playerX += ((SPRITE_WIDTH - HITBOX_WIDTH)/2);
            if(std::max(playerX, x) >= std::min(playerX + HITBOX_WIDTH, x + PROJECTILE_HITBOX_WIDTH) || (std::max(playerY, y) >= std::min(playerY + HITBOX_HEIGHT, y + PROJECTILE_HITBOX_HEIGHT))) {
                return false;
            }
            return true;
        }

        void move(float delta) {
            if(type == 20) {
                x += speedX * delta;
                y += speedY * delta;
            }
        }

        void render() {
            gProjectilesSpriteSheetTexture.render(x, y, &projectilesClips.gProjectilesClips[type], 0.0, NULL, SDL_FLIP_NONE, widthMultiplier, heightMultiplier);
        }

    private:
        float widthMultiplier;
        float heightMultiplier;
        int type;
        float x;
        float y;
        float speedX;
        float speedY;
        
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

void loadMedia() {
    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 28);
    gSpriteSheetTexture.loadFromFile("assets/hero_knight_sprite_sheet.png");
    gTilesSpriteSheetTexture.loadFromFile("assets/tiles_sprite_sheet.png");
    gProjectilesSpriteSheetTexture.loadFromFile("assets/projectiles_sprite_sheet.png");
}

void close() {
    gSpriteSheetTexture.free();
    gTilesSpriteSheetTexture.free();
    gProjectilesSpriteSheetTexture.free();
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

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    gWindow = SDL_CreateWindow("Bullet Platformer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
}

int main(int argc, char* args[]) {
    srand((unsigned)time(NULL));
    init();
    loadMedia();

    int spawnerMap[LEVEL_HEIGHT * LEVEL_WIDTH];
    memset(spawnerMap, 0, sizeof(spawnerMap));
    int spawnerMaximum = 3;
    std::vector<Spawner> spawners;

    std::vector<Projectile> projectiles;
    std::pair<float, float> cardinalDirections[4] = {{0.0, -1.0}, {1.0, 0.0}, {0.0, 1.0}, {-1.0, 0.0}};

    Player player;
    Level level;

    bool quit = false;
    SDL_Event event;

    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    double delta = 0;

    SDL_DisplayMode display;
    SDL_GetCurrentDisplayMode(0, &display);

    while(!quit) {
        last = now;
        now = SDL_GetPerformanceCounter();
        delta = ((now-last) * 1000 / (double)SDL_GetPerformanceFrequency());
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }

            player.handleEvents(event);
        }

        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(gRenderer);
        level.render();
        player.move(delta);
        player.setDirection();
        player.setClip();
        player.render(display.refresh_rate);
        for(int i = spawners.size(); i < spawnerMaximum; i++) {
            Spawner spawner;
            spawner.spawn(spawnerMap);
            spawners.push_back(spawner);
        }
        for(int i = 0; i < projectiles.size(); i++) {
            projectiles[i].move(delta);
            if(projectiles[i].willIntersectTile(delta)) {
                projectiles.erase(projectiles.begin() + i);
            } else if(projectiles[i].isIntersectPlayer(player.x, player.y)) {
                projectiles.erase(projectiles.begin() + i);
            } else {
                projectiles[i].render();
            }
        }
        for(int i = 0; i < spawners.size(); i++) {
            if(spawners[i].lifetime <= 0) {
                spawners.erase(spawners.begin() + i);
            } else {
                if(spawners[i].fireState <= 0) {
                    spawners[i].resetFireRate();
                    if(spawners[i].type == 20) {
                        for(int j = 0; j < 4; j++) {
                            Projectile projectile;
                            projectile.setProperties(spawners[i].x, spawners[i].y, spawners[i].type, cardinalDirections[j].first, cardinalDirections[j].second);
                            projectiles.push_back(projectile);
                        }
                    }
                }
                spawners[i].render(delta);
            }
        }
        SDL_RenderPresent(gRenderer);
    }

    close();
    return 0;
}
