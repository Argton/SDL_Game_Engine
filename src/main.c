#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

/************************************
*
*
* Defines and macros
*
*
*************************************/
#define VSYNC

// Used to create particle effects
#define TOTAL_PARTICLES 20

#define DBG 0
/************************************
*
*
* Constants
*
*
*************************************/

const int SCREEN_TICKS_PER_FRAME = 1000 / 60;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const double VEL_X_MAX_FACTOR = (double) 3/5;
const double VEL_Y_MAX_FACTOR = (double) 2/5;

/************************************
*
*
* Variables
*
*
*************************************/

int gTotalDisplays = 0;

// Textures to render
SDL_Texture* gTexture = NULL;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Globally used font
TTF_Font *gFont = NULL;

SDL_Rect *gDisplayBounds = NULL;

SDL_Color textColor = { 0, 0, 0, 0xFF };

int textureCounter = 0;

bool showFps = false;
bool start = false;
bool show_menu = false;
int score[2] = {0, 0};
/************************************
*
*
* Structs
*
*
*************************************/

// Used for rendering a texture from an image
struct textureStruct
{
    char *imagePath;
    int mWidth;
    int mHeight;
    int mPosX;
    int mPosY;
    SDL_Texture* mTexture;
};

// A way of keeping track of the status of the window
struct LWindow
{
    SDL_Renderer* mRenderer;
    int mWindowID;
    int mWindowDisplayID;
    SDL_Window* mWindow;
    int mWidth;
    int mHeight;
    bool mMouseFocus;
    bool mKeyboardFocus;
    bool mFullScreen;
    bool mMinimized;
    bool mShown;
};

// Used for rendering a texture from a text string
struct ttfStruct
{
    char *textureText;
    SDL_Color textColor;
    struct textureStruct texture_struct;
};

struct timerStruct
{
    Uint32 mStartTicks;
    Uint32 mPausedTicks;
    bool mPaused;
    bool mStarted;
};

struct particleStruct
{
    int mPosX;
    int mPosY;
    int mFrame;
    SDL_Texture* mTexture;
};

struct dotStruct
{
    int DOT_VEL;
    int mVelX;
    int mVelY;
    SDL_Color textColor;
    SDL_Rect mCollider;
    struct particleStruct *particle[TOTAL_PARTICLES];
    struct textureStruct texture_struct;
};

struct playerStruct
{
    int PLAYER_WIDTH;
    int PLAYER_HEIGHT;
    int PLAYER_VEL;
    int mPosX;
    int mPosY;
    int mVelY;
    char *imagePath;
    SDL_Color textColor;
    SDL_Rect mCollider;
    SDL_Texture* mTexture;
    struct particleStruct *particle[TOTAL_PARTICLES];
};

/************************************
*
*
* Initialize structs
*
*
*************************************/

struct textureStruct gDotTexture;
struct textureStruct gRedTexture;
struct textureStruct gGreenTexture;
struct textureStruct gBlueTexture;
struct textureStruct gShimmerTexture;

struct playerStruct player1;
struct playerStruct player2;
struct ttfStruct player1_score;
struct ttfStruct player2_score;
struct dotStruct dot;

/************************************
*
*
* Functions
*
*
*************************************/

void sleep(int delay) {
  #ifdef _WIN32
  Sleep(delay);
  #else
  usleep(delay*1000);
  #endif
}

bool checkCollision( SDL_Rect a, SDL_Rect b )
{
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    return true;
}

double bounceFactor( SDL_Rect a, SDL_Rect b )
{
    int topA, topB;
    int bottomA, bottomB;

    topA = a.y;
    bottomA = a.y + a.h;

    topB = b.y;
    bottomB = b.y + b.h;

    int midHeightA = topA + (bottomA - topA) / 2;
    int midHeightB = topB + (bottomB - topB) / 2;

    /* midHeightA > midHeightB: bounce downward on screen (towards higher y)
       midHeightA < midHeightB: bounce upward on screen (towards lower y) */
    int max_bounce = (bottomB - topB) / 2 + (bottomA - topA) / 2;
    double bounce = fabs((double) (midHeightA - midHeightB) / max_bounce);
    return bounce > 0.75 ? 0.75 : bounce;
}

/*Init functions*/

void initDot(struct dotStruct *inputStruct)
{
    inputStruct->texture_struct.mWidth = 20;
    inputStruct->texture_struct.mHeight = 20;
    inputStruct->mCollider.w = inputStruct->texture_struct.mWidth;
    inputStruct->mCollider.h = inputStruct->texture_struct.mHeight;
    inputStruct->DOT_VEL = 20;
    inputStruct->texture_struct.mPosX = SCREEN_WIDTH / 2;
    inputStruct->texture_struct.mPosY = SCREEN_HEIGHT / 2;
    int granularity = 20;
    srand((unsigned int)time(NULL));
    double xfactor = (double) (40 + rand() % granularity) / 100;
    double yfactor = VEL_X_MAX_FACTOR - xfactor;
    inputStruct->mVelX = (int) inputStruct->DOT_VEL * xfactor;
    inputStruct->mVelY = (int) inputStruct->DOT_VEL * yfactor;
    if(rand() % 2 == 0) {
        inputStruct->mVelX = inputStruct->mVelX * (-1);
    }
    if(rand() % 2 == 0) {
        inputStruct->mVelY = inputStruct->mVelY * (-1);
    }
}

void initPlayer(struct playerStruct *inputStruct)
{
    inputStruct->PLAYER_VEL = 10;
    inputStruct->mVelY = 0;
}

// Initialize a random particle effect
void initParticle(struct particleStruct *inputStruct, int x, int y)
{
    inputStruct->mPosX = x - 5 + ( rand() ) % 25;
    inputStruct->mPosY = y - 5 + ( rand() ) % 25;
    inputStruct->mFrame = rand() % 5;
    switch( rand() % 3 )
    {
        case 0: inputStruct->mTexture = gRedTexture.mTexture; break;
        case 1: inputStruct->mTexture = gGreenTexture.mTexture; break;
        case 2: inputStruct->mTexture = gBlueTexture.mTexture; break;
    }
}

// Init all SDL related stuff
bool initRenderer()
{
    bool success = true;
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        gWindow = SDL_CreateWindow( "Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            #ifdef VSYNC
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
            #else
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED);
            #endif

            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                else
                {
                    gScreenSurface = SDL_GetWindowSurface( gWindow );
                }
            }
        }
    }
    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    return success;
}

// Used if several monitors are needed
bool initLWindow(struct LWindow *inputStruct)
{
    inputStruct->mWindow = NULL;
    inputStruct->mRenderer = NULL;
    inputStruct->mWidth = SCREEN_WIDTH;
    inputStruct->mHeight = SCREEN_HEIGHT;
    inputStruct->mMouseFocus = false;
    inputStruct->mKeyboardFocus = false;
    inputStruct->mFullScreen = false;
    inputStruct->mMinimized = false;
    inputStruct->mWindowID = -1;
    return inputStruct->mWindow == NULL;
}

// Used instead of initRenderer if several monitors are used
bool initLWindowRenderer(struct LWindow *inputStruct)
{
    bool success = true;
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        gTotalDisplays = SDL_GetNumVideoDisplays();
        if( gTotalDisplays < 2 )
        {
            printf( "Warning: Only one display connected!" );
        }

        gDisplayBounds = malloc (gTotalDisplays * sizeof(SDL_Rect) );

        for( int i = 0; i < gTotalDisplays; ++i )
        {
            SDL_GetDisplayBounds( i, &gDisplayBounds[ i ] );
        }
        inputStruct->mWindow = NULL;
        inputStruct->mWindow = SDL_CreateWindow( "Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( inputStruct->mWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            inputStruct->mMouseFocus = true;
            inputStruct->mKeyboardFocus = true;
            inputStruct->mWidth = SCREEN_WIDTH;
            inputStruct->mHeight = SCREEN_HEIGHT;
            inputStruct->mRenderer = SDL_CreateRenderer( inputStruct->mWindow, -1, SDL_RENDERER_ACCELERATED);
            if( inputStruct->mRenderer  == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor( inputStruct->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                else
                {
                    gScreenSurface = SDL_GetWindowSurface( inputStruct->mWindow );
                    inputStruct->mWindowID = SDL_GetWindowID( inputStruct->mWindow );
                    inputStruct->mWindowDisplayID = SDL_GetWindowDisplayIndex( inputStruct->mWindow );
                    inputStruct->mShown = true;
                }
            }
        }
    }
    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    return inputStruct->mWindow != NULL && inputStruct->mRenderer != NULL && success;
}

/*Loading functions*/

bool LPlayerTexture(struct playerStruct *structinput)
{
    structinput->mTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load( structinput->imagePath );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", structinput->imagePath, IMG_GetError() );
    }
    else
    {
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );
        structinput->mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( structinput->mTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", structinput->imagePath, SDL_GetError() );
        }
        else
        {
            structinput->PLAYER_WIDTH = loadedSurface->w;
            structinput->PLAYER_HEIGHT = loadedSurface->h;
            structinput->mCollider.w = structinput->PLAYER_WIDTH;
            structinput->mCollider.h = structinput->PLAYER_HEIGHT;
            textureCounter++;
            if(DBG) {
                printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
            }
        }
        SDL_FreeSurface( loadedSurface );
    }
    loadedSurface = NULL;
    return structinput->mTexture != NULL;
}

// Create texture from image for a struct
bool LTexture(struct textureStruct *structinput)
{
    structinput->mTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load( structinput->imagePath );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", structinput->imagePath, IMG_GetError() );
    }
    else
    {
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );
        structinput->mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( structinput->mTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", structinput->imagePath, SDL_GetError() );
        }
        else
        {
            structinput->mWidth = loadedSurface->w;
            structinput->mHeight = loadedSurface->h;
        }

        SDL_FreeSurface( loadedSurface );
        loadedSurface = NULL;
    }
    textureCounter++;
    if(DBG) {
        if(structinput->mTexture != NULL)
        {
            printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
        }
    }
    return structinput->mTexture != NULL;
}

// Create texture from a text string for a struct
bool LRenderedText(struct ttfStruct *structinput, SDL_Color textColor, int size )
{
    gFont = TTF_OpenFont( structinput->texture_struct.imagePath, size );
    if( gFont == NULL )
    {
        printf( "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, structinput->textureText, textColor );
        if( textSurface == NULL )
        {
            printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
        }
        else
        {
            structinput->texture_struct.mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
            if( structinput->texture_struct.mTexture == NULL )
            {
                printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                structinput->texture_struct.mWidth = textSurface->w;
                structinput->texture_struct.mHeight = textSurface->h;
            }
        }
        SDL_FreeSurface( textSurface );
        textSurface = NULL;
    }
    if(DBG) {
        if(structinput->texture_struct.mTexture != NULL)
        {
            textureCounter++;
            printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
        }
    }
    return structinput->texture_struct.mTexture != NULL;
}

bool reloadRenderedText(struct ttfStruct *structinput, SDL_Color textColor)
{
    bool success = true;
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, structinput->textureText, textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    {
        structinput->texture_struct.mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( structinput->texture_struct.mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            structinput->texture_struct.mWidth = textSurface->w;
            structinput->texture_struct.mHeight = textSurface->h;
        }
    }
    SDL_FreeSurface( textSurface );
    textSurface = NULL;
    return structinput->texture_struct.mTexture != NULL && success;
}

/*Rendering functions*/

// Render texture for a struct containing an image path to render
void textureRender(struct textureStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip, double scaleX, double scaleY)
{
    SDL_Rect renderQuad = { structinput->mPosX, structinput->mPosY, structinput->mWidth * scaleX, structinput->mHeight * scaleY };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx( gRenderer, structinput->mTexture, clip, &renderQuad, angle, center, flip );
}

// Render texture for a struct containing a text string to render
void textureRenderttf(struct ttfStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = { structinput->texture_struct.mPosX, structinput->texture_struct.mPosY, structinput->texture_struct.mWidth, structinput->texture_struct.mHeight };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx( gRenderer, structinput->texture_struct.mTexture, clip, &renderQuad, angle, center, flip );
}

void texturePlayerRender(struct playerStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = { structinput->mPosX, structinput->mPosY, structinput->PLAYER_WIDTH, structinput->PLAYER_HEIGHT };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx( gRenderer, structinput->mTexture, clip, &renderQuad, angle, center, flip );
}

/*Timer functions*/

void timerInit(struct timerStruct *inputStruct)
{
    inputStruct->mStarted = false;
    inputStruct->mPaused = false;
    inputStruct->mStartTicks = 0;
    inputStruct->mPausedTicks = 0;
}

void timerStart(struct timerStruct *inputStruct)
{
    inputStruct->mStarted = true;
    inputStruct->mPaused = false;
    inputStruct->mStartTicks = SDL_GetTicks();
    inputStruct->mPausedTicks = 0;
}

void timerStop(struct timerStruct *inputStruct)
{
    inputStruct->mStarted = false;
    inputStruct->mPaused = false;
    inputStruct->mStartTicks = 0;
    inputStruct->mPausedTicks = 0;
}

void timerPause(struct timerStruct *inputStruct)
{
    if( inputStruct->mStarted && !inputStruct->mPaused )
    {
        inputStruct->mPaused = true;
        inputStruct->mPausedTicks = SDL_GetTicks() - inputStruct->mStartTicks;
        inputStruct->mStartTicks = 0;
    }
}

void timerUnpause(struct timerStruct *inputStruct)
{
    if( inputStruct->mStarted && inputStruct->mPaused )
    {
        inputStruct->mPaused = false;
        inputStruct->mPausedTicks = SDL_GetTicks() - inputStruct->mPausedTicks;
        inputStruct->mPausedTicks = 0;
    }
}

Uint32 getTicks(struct timerStruct *inputStruct)
{
    Uint32 time = 0;
    if( inputStruct->mStarted )
    {
        if( inputStruct->mPaused)
        {
            time = inputStruct->mPausedTicks;
        }
        else
            {
                time = SDL_GetTicks() - inputStruct->mStartTicks;
            }
    }
    return time;
}

// Free all SDL related stuff from memory, except for textures which are freed separately
void close()
{
    SDL_FreeSurface( gScreenSurface );
    gScreenSurface = NULL;

    TTF_CloseFont( gFont );
    gFont = NULL;

    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    SDL_DestroyRenderer( gRenderer );
    gRenderer = NULL;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
}

void closeTexture(SDL_Texture* texture)
{
    SDL_DestroyTexture( texture );
    texture = NULL;
    textureCounter--;
    if(DBG) {
        printf("Killing texture. Number of loaded textures: %d\n", textureCounter);
    }
}

/*Event functions*/

void handleEvent( SDL_Event *e)
{
//If a key was pressed
    if( e->type == SDL_KEYDOWN && e->key.repeat == 0 )
    {
        switch( e->key.keysym.sym )
        {
            case SDLK_F1:
                show_menu = show_menu == true ? false : true;
            break;
            case SDLK_F3:
                showFps = showFps == true ? false : true;
            break;
            case SDLK_RETURN:
                start = true;
                score[0] = 0;
                score[1] = 0;
                initDot(&dot);
            break;
        }
    }
}

void handlePlayerEvent(SDL_Event *e)
{
    if( e->type == SDL_KEYDOWN && e->key.repeat == 0 )
    {
        switch( e->key.keysym.sym )
        {
            case SDLK_UP: player2.mVelY -= player2.PLAYER_VEL; break;
            case SDLK_DOWN: player2.mVelY += player2.PLAYER_VEL; break;
            case SDLK_w: player1.mVelY -= player1.PLAYER_VEL; break;
            case SDLK_s: player1.mVelY += player1.PLAYER_VEL; break;
        }
    }
    else if( e->type == SDL_KEYUP && e->key.repeat == 0 )
    {
        switch( e->key.keysym.sym )
        {
            case SDLK_UP: player2.mVelY += player2.PLAYER_VEL; break;
            case SDLK_DOWN: player2.mVelY -= player2.PLAYER_VEL; break;
            case SDLK_w: player1.mVelY += player1.PLAYER_VEL; break;
            case SDLK_s: player1.mVelY -= player1.PLAYER_VEL; break;
        }
    }
}

void moveDot(struct dotStruct *dot, struct playerStruct *player1, struct playerStruct *player2)
{
    dot->texture_struct.mPosX += dot->mVelX;
    dot->mCollider.x = dot->texture_struct.mPosX;
    bool player1_collided = false;
    bool player2_collided = false;
    double bounce = 0;
    player1_collided = checkCollision(dot->mCollider, player1->mCollider);
    player2_collided = checkCollision(dot->mCollider, player2->mCollider);
    if(player1_collided || player2_collided)
    {
        dot->texture_struct.mPosX -= dot->mVelX;
        dot->mCollider.x = dot->texture_struct.mPosX;
        if(player1_collided) {
            bounce = bounceFactor(dot->mCollider, player1->mCollider);
            dot->mVelX = dot->DOT_VEL * (1 - bounce);
        } else {
            bounce = bounceFactor(dot->mCollider, player2->mCollider);
            dot->mVelX = (-1) * dot->DOT_VEL * (1 - bounce);
        }
        dot->mVelY = dot->DOT_VEL * bounce;
    }
    else if( (dot->texture_struct.mPosX < 0 ) ) {
        score[1]++;
        sleep(500);
        initDot(dot);
    } else if (( dot->texture_struct.mPosX + dot->texture_struct.mWidth > SCREEN_WIDTH )) {
        score[0]++;
        sleep(500);
        initDot(dot);
    }

    dot->texture_struct.mPosY += dot->mVelY;
    dot->mCollider.y = dot->texture_struct.mPosY;

    if( checkCollision(dot->mCollider, player1->mCollider) || checkCollision(dot->mCollider, player2->mCollider) )
    {
        dot->mVelY = -dot->mVelY;
        dot->mVelX = -dot->mVelX;
        dot->texture_struct.mPosY -= dot->mVelY;
    } else if (( dot->texture_struct.mPosY < 0 ) || ( dot->texture_struct.mPosY + dot->texture_struct.mHeight > SCREEN_HEIGHT )) {
        dot->mVelY = -dot->mVelY;
    }
}

void movePlayer(struct playerStruct *inputStruct)
{
    inputStruct->mCollider.x = inputStruct->mPosX;

    inputStruct->mPosY += inputStruct->mVelY;
    inputStruct->mCollider.y = inputStruct->mPosY;

    if( ( inputStruct->mPosY < 0 ) || ( inputStruct->mPosY + inputStruct->PLAYER_HEIGHT > SCREEN_HEIGHT ) )
    {
        inputStruct->mPosY -= inputStruct->mVelY;
        inputStruct->mCollider.y = inputStruct->mPosY;
    }
}

int main(int argc, char* args[])
{
    bool quit = false;
    SDL_Event e;
    double degrees = 0;
    SDL_RendererFlip flipType = SDL_FLIP_NONE;

    //Start up SDL and create window
    if( !initRenderer() )
    {
        printf( "Failed to initialize SDL!\n" );
    }

    struct ttfStruct gfpsTexture;
    struct ttfStruct player_win;
    struct ttfStruct help_text[4];
    struct timerStruct fpsTimer;

    SDL_Color textColor = { 0xFF, 0xFF, 0xFF, 0 };

    SDL_Rect top;
    top.x = 0;
    top.y = 1;
    top.w = SCREEN_WIDTH;
    top.h = 2;

    SDL_Rect bottom;
    bottom.x = 0;
    bottom.y = SCREEN_HEIGHT - 2;
    bottom.w = SCREEN_WIDTH;
    bottom.h = 2;

    char font_name[32] = "OpenSans-Regular.ttf";

    for(int i = 0; i < 4; i++) {
        help_text[i].texture_struct.imagePath = font_name;
        help_text[i].texture_struct.mPosX = 0;
        help_text[i].texture_struct.mPosY = i * 40;
    }
    help_text[0].textureText = "Press ENTER for new game.";
    help_text[1].textureText = "Player1 controls: W and S";
    help_text[2].textureText = "Player2 controls: UP and DOWN arrow keys";
    help_text[3].textureText = "Press F1 to show this help";

    gfpsTexture.texture_struct.imagePath = font_name;
    gfpsTexture.textureText = "empty";
    gfpsTexture.texture_struct.mPosX = 0;
    gfpsTexture.texture_struct.mPosY = 0;

    player1_score.texture_struct.imagePath = font_name;
    player1_score.textureText = "P1: 0";
    player1_score.texture_struct.mPosX = 40;
    player1_score.texture_struct.mPosY = 0;

    player2_score.texture_struct.imagePath = font_name;
    player2_score.textureText = "P2: 0";
    player2_score.texture_struct.mPosX = SCREEN_WIDTH - 200;
    player2_score.texture_struct.mPosY = 0;

    player_win.texture_struct.imagePath = font_name;
    player_win.textureText = "empty";
    player_win.texture_struct.mPosX = 0;
    player_win.texture_struct.mPosY = 0;

    dot.texture_struct.imagePath = "dot.bmp";
    player1.imagePath = "player_texture.png";
    player2.imagePath = "player_texture.png";

    int countedFrames = 0;

    for(int i = 0; i < 4; i++) {
        if( !LRenderedText(&help_text[i], textColor, 28) )
        {
            printf( "Failed to load texture from font! \n" );
        }
    }
    if( !LRenderedText(&gfpsTexture, textColor, 28) )
    {
        printf( "Failed to load texture from font! \n" );
    }
    if( !LRenderedText(&player1_score, textColor, 72) )
    {
        printf( "Failed to load texture from font! \n" );
    }
    if( !LRenderedText(&player2_score, textColor, 72) )
    {
        printf( "Failed to load texture from font! \n" );
    }
    if( !LRenderedText(&player_win, textColor, 72) )
    {
        printf( "Failed to load texture from font! \n" );
    }
    if(!LTexture(&dot.texture_struct))
    {
        printf( "Failed to load dot texture! \n" );
    }
    if(!LPlayerTexture(&player1) )
    {
        printf( "Failed to load player1 texture! \n" );
    }
    if(!LPlayerTexture(&player2) )
    {
        printf( "Failed to load player2 texture! \n" );
    }

    SDL_Texture *textures[11] = {help_text[0].texture_struct.mTexture, help_text[1].texture_struct.mTexture,
        help_text[2].texture_struct.mTexture, help_text[3].texture_struct.mTexture, gfpsTexture.texture_struct.mTexture,
        player1_score.texture_struct.mTexture, player2_score.texture_struct.mTexture, player_win.texture_struct.mTexture,
        dot.texture_struct.mTexture, player1.mTexture, player2.mTexture};

    timerInit(&fpsTimer);
    timerStart(&fpsTimer);

    initPlayer(&player1);
    initPlayer(&player2);
    initDot(&dot);

    player1.mPosX = player1.PLAYER_WIDTH + 20;
    player1.mPosY = SCREEN_HEIGHT / 2;

    player2.mPosX = SCREEN_WIDTH - 5 - player2.PLAYER_WIDTH - 20;
    player2.mPosY = SCREEN_HEIGHT / 2;

    // Main loop
    while( !quit )
    {
        if(score[0] == 10 || score[1] == 10) {
            start = false;
            player_win.textureText = score[0] == 10 ? "Player1 wins!" : "Player2 wins!";
        }
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
            handleEvent(&e);
            handlePlayerEvent(&e);
        }
        if(start) {
            moveDot(&dot, &player1, &player2);
            movePlayer(&player1);
            movePlayer(&player2);
        }
        float avgFPS = countedFrames / ( getTicks(&fpsTimer) / 1000.f );
        if( avgFPS > 2000000 )
        {
            avgFPS = 0;
        }

        char timeText[100] = "";
        char timeBuffer[100] = "";
        strcpy(timeText, "FPS: ");
        sprintf(timeBuffer, "%.0f", avgFPS  );
        strcat(timeText, timeBuffer);
        gfpsTexture.texture_struct.mPosX = (SCREEN_WIDTH - 1.5*gfpsTexture.texture_struct.mWidth ) ;
        gfpsTexture.texture_struct.mPosY = ( gfpsTexture.texture_struct.mHeight) ;
        gfpsTexture.textureText = timeText;

        char player1_text[100] = "";
        char player1_buffer[100] = "";
        strcpy(player1_text, "P1: ");
        sprintf(player1_buffer, "%d", score[0] );
        strcat(player1_text, player1_buffer);
        player1_score.textureText = player1_text;

        char player2_text[100] = "";
        char player2_buffer[100] = "";
        strcpy(player2_text, "P2: ");
        sprintf(player2_buffer, "%d", score[1] );
        strcat(player2_text, player2_buffer);
        player2_score.textureText = player2_text;

        if( !reloadRenderedText(&gfpsTexture, textColor) )
        {
            printf( "Failed to load texture from font! \n" );
        }
        if( !reloadRenderedText(&player1_score, textColor) )
        {
            printf( "Failed to load texture from font! \n" );
        }
        if( !reloadRenderedText(&player2_score, textColor) )
        {
            printf( "Failed to load texture from font! \n" );
        }
        if( !reloadRenderedText(&player_win, textColor) )
        {
            printf( "Failed to load texture from font! \n" );
        }

        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );

        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        if(show_menu || !start) {
            for(int i = 0; i < 4; i++) {
                textureRender(&help_text[i].texture_struct, NULL, degrees, NULL, flipType, 1, 1);
            }
        }
        else if(strcmp(player_win.textureText, "empty") != 0){
            textureRender(&player_win.texture_struct, NULL, degrees, NULL, flipType, 1, 1);
            player_win.textureText = "empty";
        }
        else if(start) {
            SDL_RenderDrawRect( gRenderer, &top );
            SDL_RenderDrawRect( gRenderer, &bottom );
            textureRender(&dot.texture_struct, NULL, degrees, NULL, flipType, 1, 1);
            texturePlayerRender(&player1, NULL, degrees, NULL, flipType);
            texturePlayerRender(&player2, NULL, degrees, NULL, flipType);
            textureRender(&player1_score.texture_struct, NULL, degrees, NULL, flipType, 1, 1);
            textureRender(&player2_score.texture_struct, NULL, degrees, NULL, flipType, 1, 1);
        }
        if(showFps){
            textureRender(&gfpsTexture.texture_struct, NULL, degrees, NULL, flipType, 1, 1);
        }

        SDL_RenderPresent( gRenderer );
        ++countedFrames;

        int frameTicks = getTicks(&fpsTimer);
        if( frameTicks < SCREEN_TICKS_PER_FRAME )
        {
            //Wait remaining time
            SDL_Delay( SCREEN_TICKS_PER_FRAME - frameTicks );
        }
    }

    //Free resources and close SDL
    int nmbr = textureCounter;
    for(int i = 0; i < nmbr; i++) {
        closeTexture(textures[i]);
    }
    close();

    return 0;
}
