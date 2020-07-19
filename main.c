#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

/* Some reference for myself about coding practices

int * pt; // an uninitialized pointer
*pt = 5; // a terrible error

Always make the pointer point to an adress first before assigning a value

This is OK:
const char * pc = "Behold a string literal!"


If a function’s intent is that it not change the contents of the array, use the
keyword const when declaring the formal parameter in the prototype and in the function defi-
nition. For example, the prototype and definition for sum() should look like this:

int sum(const int ar[], int n); // prototype

Here is what you can
do:
int (* pz)[2]; // pz points to an array of 2 ints
This statement says that pz is a pointer to an array of two int s. Why the parentheses? Well, []
has a higher precedence than * . Therefore, with a declaration such as
int * pax[2]; // pax is an array of two pointers-to-int
you apply the brackets first, making pax an array of two somethings. Next, you apply the * ,
making pax an array of two pointers. Finally, use the int , making pax an array of two pointers
to int .

Malloc:
double * ptd;
ptd = (double *) malloc(30 * sizeof(double));

const float * pf; // pf points to a constant float value
float * const pt; // pt is a const pointer
const float * const ptr; // means both that ptr must always point to the same location and that the value stored at the location must not change.
float const * pfc; // same as const float * pfc; // As the comment indicates, placing const after the type name and before the * means that the
pointer can’t be used to change the pointed-to value

*/

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

bool kill = false;
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
    int xPos;
    int yPos;
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
    char *fontPath;
    int mWidth;
    int mHeight;
    int xPos;
    int yPos;
    SDL_Texture* mTexture;
    char *textureText;
    SDL_Color textColor;
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
    int DOT_WIDTH;
    int DOT_HEIGHT;
    int DOT_VEL;
    int mPosX;
    int mPosY;
    int mVelX;
    int mVelY;
    char *imagePath;
    SDL_Texture* mTexture;
    SDL_Color textColor;
    SDL_Rect mCollider;
    struct particleStruct *particle[TOTAL_PARTICLES];
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

void initDot(struct dotStruct *inputStruct)
{
    inputStruct->DOT_WIDTH = 20;
    inputStruct->DOT_HEIGHT = 20;
    inputStruct->DOT_VEL = 20;
    inputStruct->mPosX = SCREEN_WIDTH / 2;
    inputStruct->mPosY = SCREEN_HEIGHT / 2;
    int granularity = 20;
    srand((unsigned int)time(NULL));
    double xfactor = (double) (40 + rand() % granularity) / 100;
    double yfactor = 0.6 - xfactor;
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
        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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
        inputStruct->mWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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

// Create texture for dotStructs
bool LDotTexture(struct dotStruct *structinput)
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
            structinput->DOT_WIDTH = loadedSurface->w;
            structinput->DOT_HEIGHT = loadedSurface->h;
            structinput->mCollider.w = structinput->DOT_WIDTH;
            structinput->mCollider.h = structinput->DOT_HEIGHT;
            textureCounter++;
            printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
        }
        SDL_FreeSurface( loadedSurface );
    }
    loadedSurface = NULL;
    return structinput->mTexture != NULL;
}

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
            printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
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
    if(structinput->mTexture != NULL)
    {
        printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
    }
    return structinput->mTexture != NULL;
}

bool reloadTexture(struct textureStruct *structinput)
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
    return structinput->mTexture != NULL;
}

// Create texture from a text string for a struct
bool LRenderedText(struct ttfStruct *structinput, SDL_Color textColor )
{
    gFont = TTF_OpenFont( structinput->fontPath, 28 );
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
            structinput->mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
            if( structinput->mTexture == NULL )
            {
                printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                structinput->mWidth = textSurface->w;
                structinput->mHeight = textSurface->h;
            }
        }
        SDL_FreeSurface( textSurface );
        textSurface = NULL;
    }
    if(structinput->mTexture != NULL)
    {
        textureCounter++;
        printf("Texture loaded. Number of loaded textures: %d\n", textureCounter);
    }
    return structinput->mTexture != NULL;
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
        structinput->mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( structinput->mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            structinput->mWidth = textSurface->w;
            structinput->mHeight = textSurface->h;
        }
    }
    SDL_FreeSurface( textSurface );
    textSurface = NULL;
    return structinput->mTexture != NULL && success;
}

// Render texture for a struct containing an image path to render
void textureRender(struct textureStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip, int offsetX, int offsetY, double scaleX, double scaleY)
{
    SDL_Rect renderQuad = { structinput->xPos + offsetX, structinput->yPos + offsetY, structinput->mWidth * scaleX, structinput->mHeight * scaleY };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx( gRenderer, structinput->mTexture, clip, &renderQuad, angle, center, flip );
}

// Render texture for a struct containing a text string to render
void textureRenderttf(struct ttfStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip, int offsetX, int offsetY)
{
    SDL_Rect renderQuad = { structinput->xPos + offsetX, structinput->yPos + offsetY, structinput->mWidth, structinput->mHeight };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx( gRenderer, structinput->mTexture, clip, &renderQuad, angle, center, flip );
}

// Render texture for a dotstruct
void textureDotRender(struct dotStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = { structinput->mPosX, structinput->mPosY, structinput->DOT_WIDTH, structinput->DOT_HEIGHT };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx( gRenderer, structinput->mTexture, clip, &renderQuad, angle, center, flip );
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

// Free textures
void closeTexture(SDL_Texture* texture)
{
    SDL_DestroyTexture( texture );
    texture = NULL;
    textureCounter--;
    printf("Killing texture. Number of loaded textures: %d\n", textureCounter);
}

void handleEvent( SDL_Event *e)
{
//If a key was pressed
    if( e->type == SDL_KEYDOWN && e->key.repeat == 0 )
    {

        switch( e->key.keysym.sym )
        {
            case SDLK_F1:
            if(showFps == true)
            {
                showFps = false;
            }
            else
            {
                showFps = true;
            }
            break;
            case SDLK_F2:
            if(kill == true)
            {
                kill = false;
            }
            else
            {
                kill = true;
            }
            break;
        }
    }
}

void handleDotEvent(struct dotStruct *inputStruct, SDL_Event *e)
{
     if( e->type == SDL_KEYDOWN && e->key.repeat == 0 )
     {
         switch( e->key.keysym.sym )
         {
            case SDLK_UP: inputStruct->mVelY -= inputStruct->DOT_VEL; break;
            case SDLK_DOWN: inputStruct->mVelY += inputStruct->DOT_VEL; break;
            case SDLK_LEFT: inputStruct->mVelX -= inputStruct->DOT_VEL; break;
            case SDLK_RIGHT: inputStruct->mVelX += inputStruct->DOT_VEL; break;
         }
     }
     else if( e->type == SDL_KEYUP && e->key.repeat == 0 )
     {
         switch( e->key.keysym.sym )
         {
            case SDLK_UP: inputStruct->mVelY += inputStruct->DOT_VEL; break;
            case SDLK_DOWN: inputStruct->mVelY -= inputStruct->DOT_VEL; break;
            case SDLK_LEFT: inputStruct->mVelX += inputStruct->DOT_VEL; break;
            case SDLK_RIGHT: inputStruct->mVelX -= inputStruct->DOT_VEL; break;
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

void moveDot(struct dotStruct *inputStruct, struct playerStruct *player1, struct playerStruct *player2)
{
    inputStruct->mPosX += inputStruct->mVelX;
    inputStruct->mCollider.x = inputStruct->mPosX;
    if(
       checkCollision(inputStruct->mCollider, player1->mCollider) || checkCollision(inputStruct->mCollider, player2->mCollider) )
    {
        inputStruct->mPosX -= inputStruct->mVelX;
        inputStruct->mCollider.x = inputStruct->mPosX;
    }
    else if( (inputStruct->mPosX < 0 ) || ( inputStruct->mPosX + inputStruct->DOT_WIDTH > SCREEN_WIDTH ) ) {
        //score point
        sleep(1000);
        initDot(inputStruct);
    }

    inputStruct->mPosY += inputStruct->mVelY;
    inputStruct->mCollider.y = inputStruct->mPosY;

    if( ( inputStruct->mPosY < 0 ) || ( inputStruct->mPosY + inputStruct->DOT_HEIGHT > SCREEN_HEIGHT ) ||
        checkCollision(inputStruct->mCollider, player1->mCollider) || checkCollision(inputStruct->mCollider, player2->mCollider)  )
    {
        inputStruct->mPosY -= inputStruct->mVelY;
        inputStruct->mCollider.y = inputStruct->mPosY;
    }
}

void movePlayer(struct playerStruct *inputStruct)
{
    inputStruct->mCollider.x = inputStruct->mPosX;

    inputStruct->mPosY += inputStruct->mVelY;
    inputStruct->mCollider.y = inputStruct->mPosY;

    //If the player went too far up or down
    if( ( inputStruct->mPosY < 0 ) || ( inputStruct->mPosY + inputStruct->PLAYER_HEIGHT > SCREEN_HEIGHT ) )
    {
        //Move back
        inputStruct->mPosY -= inputStruct->mVelY;
        inputStruct->mCollider.y = inputStruct->mPosY;
    }
}


int main(int argc, char* args[])
{
    SDL_Color highlightColor = { 0xFF, 0, 0, 0xFF };
    bool quit = false;
    SDL_Event e;
    double degrees = 0;
    SDL_RendererFlip flipType = SDL_FLIP_NONE;
    SDL_Rect* gDisplayBounds = NULL;

    // Used to handle scrolling
    SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    //Start up SDL and create window
    if( !initRenderer() )
    {
        printf( "Failed to initialize SDL!\n" );
    }

    struct textureStruct gSceneTexture;
    struct ttfStruct gfpsTexture;
    struct timerStruct fpsTimer;
    struct dotStruct dot;

    SDL_Color textColor = { 0xFF, 0xFF, 0xFF, 0 };

    gSceneTexture.imagePath = "voronoi.png";
    gSceneTexture.xPos = ( 0 ) ;
    gSceneTexture.yPos = ( 0 ) ;

    gfpsTexture.fontPath = "lazy.ttf";
    gfpsTexture.textureText = "LMAO";
    gfpsTexture.xPos = 0;
    gfpsTexture.yPos = 0;

    dot.imagePath = "dot.bmp";
    player1.imagePath = "player_texture.png";
    player2.imagePath = "player_texture.png";

    int countedFrames = 0;

    if( !LTexture(&gSceneTexture) )
    {
        printf( "Failed to load texture from image! \n" );
    }
    if( !LRenderedText(&gfpsTexture, textColor) )
    {
        printf( "Failed to load texture from font! \n" );
    }
    if(!LDotTexture(&dot) )
    {
        printf( "Failed to load gDot texture! \n" );
    }
    if(!LPlayerTexture(&player1) )
    {
        printf( "Failed to load player1 texture! \n" );
    }
    if(!LPlayerTexture(&player2) )
    {
        printf( "Failed to load player2 texture! \n" );
    }

    timerInit(&fpsTimer);
    timerStart(&fpsTimer);
    initDot(&dot);
    initPlayer(&player1);
    initPlayer(&player2);

    player1.mPosX = player1.PLAYER_WIDTH;
    player1.mPosY = 40;

    player2.mPosX = SCREEN_WIDTH - 5 - player2.PLAYER_WIDTH;
    player2.mPosY = 40;

    // Main loop
    while( !quit )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
            //Handle events
            handleEvent(&e);
           // handleDotEvent(&dot, &e);
            handlePlayerEvent(&e);
        }
        moveDot(&dot, &player1, &player2);
        movePlayer(&player1);
        movePlayer(&player2);
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
        gfpsTexture.xPos = (SCREEN_WIDTH - 1.5*gfpsTexture.mWidth ) ;
        gfpsTexture.yPos = ( gfpsTexture.mHeight) ;
        gfpsTexture.textureText = timeText;

        if( !reloadRenderedText(&gfpsTexture, textColor) )
        {
            printf( "Failed to load texture from font! \n" );
        }

        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );

        if(!kill)
        {
           // textureRender(&gSceneTexture, NULL, degrees, NULL, flipType, 0 , 0, (double) SCREEN_WIDTH / gSceneTexture.mWidth, (double) SCREEN_HEIGHT / gSceneTexture.mHeight);
            textureDotRender(&dot, NULL, degrees, NULL, flipType);
            texturePlayerRender(&player1, NULL, degrees, NULL, flipType);
            texturePlayerRender(&player2, NULL, degrees, NULL, flipType);
        }

        if(showFps)
        {
            textureRenderttf(&gfpsTexture, NULL, degrees, NULL, flipType, 0 , 0);
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
    closeTexture(gSceneTexture.mTexture);
    closeTexture(gfpsTexture.mTexture);
    closeTexture(dot.mTexture);
    closeTexture(player1.mTexture);
    closeTexture(player2.mTexture);
    close();

    return 0;
}
