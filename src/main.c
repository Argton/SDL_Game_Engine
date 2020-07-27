#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#define SDL_MAIN_HANDLED /*To prevent conflict with the main() in SDL2*/
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
#define VSYNC 1

#define DBG 1

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

//The window we'll be rendering to
SDL_Window *gWindow = NULL;

//The window renderer
SDL_Renderer *gRenderer = NULL;

//The surface contained by the window
SDL_Surface *gScreenSurface = NULL;

SDL_Rect *gDisplayBounds = NULL;

int textureCounter = 0;
bool show_fps = false;
bool start = false;
bool show_menu = false;
int score[2] = {0, 0};

/*Forward declarations*/
void close_texture(SDL_Texture* texture);

/************************************
*
*
* Structs
*
*
*************************************/

// Used for rendering a texture from an image
struct texture_core {
    char *path;
    int mWidth;
    int mHeight;
    int mPosX;
    int mPosY;
    SDL_Texture* mTexture;
};

// A way of keeping track of the status of the window
struct window {
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
struct ttf {
    char *text;
    SDL_Color color;
    TTF_Font* font;
    struct texture_core texture_core;
};

struct timer {
    Uint32 mStartTicks;
    Uint32 mPausedTicks;
    bool mPaused;
    bool mStarted;
};

struct particle {
    int mPosX;
    int mPosY;
    int mFrame;
    SDL_Texture* mTexture;
};

struct dot {
    int DOT_VEL;
    int mVelX;
    int mVelY;
    SDL_Color color;
    SDL_Rect mCollider;
    struct particle *particle[TOTAL_PARTICLES];
    struct texture_core texture_core;
};

struct player {
    int PLAYER_VEL;
    int mVelY;
    SDL_Color color;
    SDL_Rect mCollider;
    struct texture_core texture_core;
    struct particle *particle[TOTAL_PARTICLES];
};

struct LinkedList{
    struct texture_core *core;
    struct LinkedList *next;
 };

/************************************
*
*
* Declaration of structs
*
*
*************************************/

struct texture_core gDotTexture;
struct texture_core gRedTexture;
struct texture_core gGreenTexture;
struct texture_core gBlueTexture;
struct texture_core gShimmerTexture;

struct player player1;
struct player player2;
struct ttf player1_score;
struct ttf player2_score;
struct dot dot;

/************************************
*
*
* Functions
*
*
*************************************/

bool check_collision( SDL_Rect a, SDL_Rect b )
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

double bounce_factor( SDL_Rect a, SDL_Rect b )
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
    /*Cap the bounce factor to 0.75 to prevent overly slow speed in x direction*/
    return bounce > 0.75 ? 0.75 : bounce;
}

/*Init functions
************************************************/

struct LinkedList *slist_init()
{
    struct LinkedList *temp;
    temp = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    temp->next = NULL;
    return temp;
}

void init_dot(struct dot *input)
{
    input->texture_core.mWidth = 20;
    input->texture_core.mHeight = 20;
    input->mCollider.w = input->texture_core.mWidth;
    input->mCollider.h = input->texture_core.mHeight;
    input->DOT_VEL = 20;
    input->texture_core.mPosX = SCREEN_WIDTH / 2;
    input->texture_core.mPosY = SCREEN_HEIGHT / 2;
    int granularity = 20;
    srand((unsigned int)time(NULL));
    double xfactor = (double) (40 + rand() % granularity) / 100;
    double yfactor = VEL_X_MAX_FACTOR - xfactor;
    input->mVelX = (int) input->DOT_VEL * xfactor;
    input->mVelY = (int) input->DOT_VEL * yfactor;
    if(rand() % 2 == 0) {
        input->mVelX = input->mVelX * (-1);
    }
    if(rand() % 2 == 0) {
        input->mVelY = input->mVelY * (-1);
    }
}

void init_player(struct player *input)
{
    input->mCollider.w = input->texture_core.mWidth;
    input->mCollider.h = input->texture_core.mHeight;
    input->PLAYER_VEL = 10;
    input->mVelY = 0;
}

// Initialize a random particle effect
void init_particle(struct particle *input, int x, int y)
{
    input->mPosX = x - 5 + ( rand() ) % 25;
    input->mPosY = y - 5 + ( rand() ) % 25;
    input->mFrame = rand() % 5;
    switch( rand() % 3 )
    {
        case 0: input->mTexture = gRedTexture.mTexture; break;
        case 1: input->mTexture = gGreenTexture.mTexture; break;
        case 2: input->mTexture = gBlueTexture.mTexture; break;
    }
}

// Init all SDL related stuff
bool init_renderer()
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
bool init_window(struct window *inputStruct)
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

// Used instead of init_renderer if several monitors are used
bool init_window_renderer(struct window *inputStruct)
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

/*Misc functions*/

struct LinkedList *slist_add(struct LinkedList *head, struct texture_core *value)
{
    struct LinkedList *temp, *p;
    temp = slist_init();
    temp->core = value;
    if(head == NULL){
        head = temp;
    }
    else{
        p = head;
        while(p->next != NULL){
            p = p->next;
        }
        p->next = temp;
    }
    return head;
}

/*Timer functions
*************************************************/

void sleep_function(int amount)
{
  #ifdef _WIN32
  Sleep(amount);
  #else
  usleep(amount*1000);
  #endif
}

void timer_init(struct timer *input)
{
    input->mStarted = false;
    input->mPaused = false;
    input->mStartTicks = 0;
    input->mPausedTicks = 0;
}

void timer_start(struct timer *input)
{
    input->mStarted = true;
    input->mPaused = false;
    input->mStartTicks = SDL_GetTicks();
    input->mPausedTicks = 0;
}

void timer_stop(struct timer *input)
{
    input->mStarted = false;
    input->mPaused = false;
    input->mStartTicks = 0;
    input->mPausedTicks = 0;
}

void timer_pause(struct timer *input)
{
    if( input->mStarted && !input->mPaused )
    {
        input->mPaused = true;
        input->mPausedTicks = SDL_GetTicks() - input->mStartTicks;
        input->mStartTicks = 0;
    }
}

void timer_unpause(struct timer *input)
{
    if( input->mStarted && input->mPaused )
    {
        input->mPaused = false;
        input->mPausedTicks = SDL_GetTicks() - input->mPausedTicks;
        input->mPausedTicks = 0;
    }
}

Uint32 get_ticks(struct timer *input)
{
    Uint32 time = 0;
    if( input->mStarted )
    {
        if( input->mPaused)
        {
            time = input->mPausedTicks;
        }
        else
            {
                time = SDL_GetTicks() - input->mStartTicks;
            }
    }
    return time;
}

void set_fps_text(struct ttf *input, float fps)
{
    char timeText[100] = "";
    char timeBuffer[100] = "";
    strcpy(timeText, "FPS: ");
    sprintf(timeBuffer, "%.0f", fps  );
    strcat(timeText, timeBuffer);
    input->texture_core.mPosX = (SCREEN_WIDTH - 1.5*input->texture_core.mWidth ) ;
    input->texture_core.mPosY = input->texture_core.mHeight;
    input->text = malloc(sizeof(char) * 16);
    strcpy(input->text, timeText);
}

/*Loading functions
*************************************************/

// Create texture from image for a struct
bool load_texture(struct texture_core *structinput)
{
    structinput->mTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load( structinput->path );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", structinput->path, IMG_GetError() );
    }
    else
    {
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );
        structinput->mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( structinput->mTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", structinput->path, SDL_GetError() );
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
bool load_rendered_text(struct ttf *input, SDL_Color textColor, int size)
{
    if(input->font == NULL) {
        input->font = TTF_OpenFont( input->texture_core.path, size );
    }
    if( input->font == NULL ) {
        printf( "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else {
        SDL_Surface* textSurface = TTF_RenderText_Solid( input->font, input->text, textColor );
        if( textSurface == NULL ) {
            printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
        }
        else {
            if(input->texture_core.mTexture) {
                close_texture(input->texture_core.mTexture);
            }
            input->texture_core.mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
            if( input->texture_core.mTexture == NULL ) {
                printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            }
            else {
                input->texture_core.mWidth = textSurface->w;
                input->texture_core.mHeight = textSurface->h;
                textureCounter++;
            }
        }
        SDL_FreeSurface( textSurface );
        textSurface = NULL;
    }
    if(DBG) {
        if(input->texture_core.mTexture != NULL) {
            printf("TTF texture loaded. Number of loaded textures: %d\n", textureCounter);
        }
    }
    return input->texture_core.mTexture != NULL;
}

/*Rendering functions
************************************************/

// Render a loaded texture
void texture_render(struct texture_core *input, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip,
        double scale_x, double scale_y)
{
    SDL_Rect renderQuad = { input->mPosX, input->mPosY, input->mWidth * scale_x, input->mHeight * scale_y };

    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx( gRenderer, input->mTexture, clip, &renderQuad, angle, center, flip );
}

/*Event functions
*************************************************/

void handle_event( SDL_Event *e)
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
                show_fps = show_fps == true ? false : true;
            break;
            case SDLK_RETURN:
                start = true;
                score[0] = 0;
                score[1] = 0;
                init_dot(&dot);
            break;
        }
    }
}

void handle_player_event(SDL_Event *e)
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

void move_dot(struct dot *dot, struct player *player1, struct player *player2)
{
    dot->texture_core.mPosX += dot->mVelX;
    dot->mCollider.x = dot->texture_core.mPosX;
    bool player1_collided = false;
    bool player2_collided = false;
    double bounce = 0;
    player1_collided = check_collision(dot->mCollider, player1->mCollider);
    player2_collided = check_collision(dot->mCollider, player2->mCollider);
    if(player1_collided || player2_collided)
    {
        dot->texture_core.mPosX -= dot->mVelX;
        dot->mCollider.x = dot->texture_core.mPosX;
        if(player1_collided) {
            bounce = bounce_factor(dot->mCollider, player1->mCollider);
            dot->mVelX = dot->DOT_VEL * (1 - bounce);
        } else {
            bounce = bounce_factor(dot->mCollider, player2->mCollider);
            dot->mVelX = (-1) * dot->DOT_VEL * (1 - bounce);
        }
        dot->mVelY = dot->DOT_VEL * bounce;
    }
    else if( (dot->texture_core.mPosX < 0 ) ) {
        score[1]++;
        sleep_function(500);
        init_dot(dot);
    } else if (( dot->texture_core.mPosX + dot->texture_core.mWidth > SCREEN_WIDTH )) {
        score[0]++;
        sleep_function(500);
        init_dot(dot);
    }

    dot->texture_core.mPosY += dot->mVelY;
    dot->mCollider.y = dot->texture_core.mPosY;

    if( check_collision(dot->mCollider, player1->mCollider) || check_collision(dot->mCollider, player2->mCollider) )
    {
        dot->mVelY = -dot->mVelY;
        dot->mVelX = -dot->mVelX;
        dot->texture_core.mPosY -= dot->mVelY;
    } else if (( dot->texture_core.mPosY < 0 ) || ( dot->texture_core.mPosY + dot->texture_core.mHeight > SCREEN_HEIGHT )) {
        dot->mVelY = -dot->mVelY;
    }
}

void move_player(struct player *input)
{
    input->mCollider.x = input->texture_core.mPosX;

    input->texture_core.mPosY += input->mVelY;
    input->mCollider.y = input->texture_core.mPosY;

    if( ( input->texture_core.mPosY < 0 ) || ( input->texture_core.mPosY + input->texture_core.mHeight > SCREEN_HEIGHT ) )
    {
        input->texture_core.mPosY -= input->mVelY;
        input->mCollider.y = input->texture_core.mPosY;
    }
}

/*Deconstructors
*************************************************/

// Free all SDL related stuff from memory, except for textures which are freed separately
void close()
{
    SDL_FreeSurface( gScreenSurface );
    gScreenSurface = NULL;

    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    SDL_DestroyRenderer( gRenderer );
    gRenderer = NULL;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
}

void close_font(TTF_Font* font)
{
    TTF_CloseFont( font );
    font = NULL;
}

void close_texture(SDL_Texture* texture)
{
    SDL_DestroyTexture( texture );
    texture = NULL;
    textureCounter--;
    if(DBG) {
        printf("Killing texture. Number of loaded textures: %d\n", textureCounter);
    }
}

/************************************************/

int main(int argc, char* args[])
{
    bool quit = false;
    SDL_Event e;
    double degrees = 0;
    SDL_RendererFlip flipType = SDL_FLIP_NONE;

    //Start up SDL and create window
    if( !init_renderer() )
    {
        printf( "Failed to initialize SDL!\n" );
    }

    struct ttf fps;
    struct ttf player_win;
    struct ttf help_text[4];
    struct timer fps_timer;

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
        help_text[i].texture_core.path = font_name;
        help_text[i].texture_core.mPosX = 0;
        help_text[i].texture_core.mPosY = i * 40;
        help_text[i].font = NULL;
        help_text[i].texture_core.mTexture = NULL;
    }
    help_text[0].text = "Press ENTER for new game.";
    help_text[1].text = "Player1 controls: W and S";
    help_text[2].text = "Player2 controls: UP and DOWN arrow keys";
    help_text[3].text = "Press F1 to show this help";

    fps.texture_core.path = font_name;
    fps.text = "empty";
    fps.texture_core.mPosX = 0;
    fps.texture_core.mPosY = 0;
    fps.font = NULL;
    fps.texture_core.mTexture = NULL;

    player1_score.texture_core.path = font_name;
    player1_score.text = "P1: 0";
    player1_score.texture_core.mPosX = 40;
    player1_score.texture_core.mPosY = 0;
    player1_score.font = NULL;
    player1_score.texture_core.mTexture = NULL;

    player2_score.texture_core.path = font_name;
    player2_score.text = "P2: 0";
    player2_score.texture_core.mPosX = SCREEN_WIDTH - 200;
    player2_score.texture_core.mPosY = 0;
    player2_score.font = NULL;
    player2_score.texture_core.mTexture = NULL;

    player_win.texture_core.path = font_name;
    player_win.text = "empty";
    player_win.texture_core.mPosX = 0;
    player_win.texture_core.mPosY = 0;
    player_win.font = NULL;
    player_win.texture_core.mTexture = NULL;

    dot.texture_core.path = "dot.bmp";
    player1.texture_core.path = "player_texture.png";
    player2.texture_core.path = "player_texture.png";

    int countedFrames = 0;

    for(int i = 0; i < 4; i++) {
        load_rendered_text(&help_text[i], textColor, 28);
    }
    load_rendered_text(&fps, textColor, 28);
    load_rendered_text(&player1_score, textColor, 72);
    load_rendered_text(&player2_score, textColor, 72);
    load_rendered_text(&player_win, textColor, 72);
    load_texture(&dot.texture_core);
    load_texture(&player1.texture_core);
    load_texture(&player2.texture_core);

    TTF_Font *all_fonts[8] = {help_text[0].font, help_text[1].font, help_text[2].font, help_text[3].font, fps.font,
            player1_score.font, player2_score.font, player_win.font};

    struct LinkedList *ptr_slist = NULL;
    ptr_slist = slist_init();
    ptr_slist->core = &help_text[0].texture_core;
    slist_add(ptr_slist, &help_text[1].texture_core);
    slist_add(ptr_slist, &help_text[2].texture_core);
    slist_add(ptr_slist, &help_text[3].texture_core);
    slist_add(ptr_slist, &fps.texture_core);
    slist_add(ptr_slist, &player1_score.texture_core);
    slist_add(ptr_slist, &player2_score.texture_core);
    slist_add(ptr_slist, &player_win.texture_core);
    slist_add(ptr_slist, &dot.texture_core);
    slist_add(ptr_slist, &player1.texture_core);
    slist_add(ptr_slist, &player2.texture_core);

    timer_init(&fps_timer);
    timer_start(&fps_timer);

    init_player(&player1);
    init_player(&player2);
    init_dot(&dot);

    player1.texture_core.mPosX = player1.texture_core.mWidth + 20;
    player1.texture_core.mPosY = SCREEN_HEIGHT / 2;

    player2.texture_core.mPosX = SCREEN_WIDTH - 5 - player2.texture_core.mWidth - 20;
    player2.texture_core.mPosY = SCREEN_HEIGHT / 2;

    // Main loop
    while( !quit )
    {
        float avg_FPS = countedFrames / ( get_ticks(&fps_timer) / 1000.f );
        if( avg_FPS > 2000000 )
        {
            avg_FPS = 0;
        }

        if(score[0] == 10 || score[1] == 10) {
            start = false;
            player_win.text = score[0] == 10 ? "Player1 wins!" : "Player2 wins!";
        }
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
            handle_event(&e);
            handle_player_event(&e);
        }

        set_fps_text(&fps, avg_FPS);

        if(start) {
            move_dot(&dot, &player1, &player2);
            move_player(&player1);
            move_player(&player2);
        }

        char player1_text[100] = "";
        char player1_buffer[100] = "";
        strcpy(player1_text, "P1: ");
        sprintf(player1_buffer, "%d", score[0] );
        strcat(player1_text, player1_buffer);
        player1_score.text = player1_text;

        char player2_text[100] = "";
        char player2_buffer[100] = "";
        strcpy(player2_text, "P2: ");
        sprintf(player2_buffer, "%d", score[1] );
        strcat(player2_text, player2_buffer);
        player2_score.text = player2_text;

        load_rendered_text(&fps, textColor, 60);
        load_rendered_text(&player1_score, textColor, 60);
        load_rendered_text(&player2_score, textColor, 60);
        load_rendered_text(&player_win, textColor, 60);

        struct texture_core start_textures[5] = {player1_score.texture_core, player2_score.texture_core,
            dot.texture_core, player1.texture_core, player2.texture_core};

        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );

        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        if(strcmp(player_win.text, "empty") != 0){
            texture_render(&player_win.texture_core, NULL, degrees, NULL, flipType, 1, 1);
            player_win.text = "empty";
        }
        else if(show_menu || !start) {
            for(int i = 0; i < 4; i++) {
                texture_render(&help_text[i].texture_core, NULL, degrees, NULL, flipType, 1, 1);
            }
        }
        else if(start) {
            SDL_RenderDrawRect( gRenderer, &top );
            SDL_RenderDrawRect( gRenderer, &bottom );
            int sz = sizeof(start_textures) / sizeof(start_textures[0]);
            for(int i = 0; i < sz; i++) {
                texture_render(&start_textures[i], NULL, degrees, NULL, flipType, 1, 1);
            }
        }
        if(show_fps){
            texture_render(&fps.texture_core, NULL, degrees, NULL, flipType, 1, 1);
        }

        SDL_RenderPresent( gRenderer );
        ++countedFrames;

        int frameTicks = get_ticks(&fps_timer);
        if( frameTicks < SCREEN_TICKS_PER_FRAME )
        {
            //Wait remaining time
            SDL_Delay( SCREEN_TICKS_PER_FRAME - frameTicks );
        }
    }

    while(ptr_slist != NULL){
        struct texture_core *core = ptr_slist->core;
        SDL_Texture* texture = core->mTexture;
        close_texture(texture);
        ptr_slist->core = NULL;
        ptr_slist = ptr_slist->next;
    }

    int sz = sizeof(all_fonts) / sizeof(all_fonts[0]);
    for(int i = 0; i < sz; i++) {
        close_font(all_fonts[i]);
    }
    close();

    return 0;
}
