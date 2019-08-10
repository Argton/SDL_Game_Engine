#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <string.h>

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
* Constants
*
*
*************************************/

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

/************************************
*
*
* Variables
*
*
*************************************/

int gTotalDisplays = 0;

// Textures to render
SDL_Texture* newTexture = NULL;
SDL_Texture* mTexture;
SDL_Texture* gTexture = NULL;

SDL_Rect *gDisplayBounds = NULL;

SDL_Color textColor = { 0, 0, 0, 0xFF };

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Globally used font
TTF_Font *gFont = NULL;

SDL_Surface* gImage = NULL;
/************************************
*
*
* Structs
*
*
*************************************/

struct textureStruct
{
    char *imagePath;
    int mWidth;
    int mHeight;
    int xPos;
    int yPos;
    SDL_Texture* mTexture;
};

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

struct ttfStruct
{
    char *imagePath;
    int mWidth;
    int mHeight;
    int xPos;
    int yPos;
    SDL_Texture* mTexture;
    char *textureText;
    SDL_Color textColor;
};

/************************************
*
*
* Functions
*
*
*************************************/

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO  ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Initialize PNG loading
            int imgFlags = IMG_INIT_PNG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) )
            {
                printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                success = false;
            }
            else
            {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );
            }
        }
    }
        //Initialize SDL_mixer
        if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
        {
            printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
            success = false;
        }
    return success;
}


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

bool initRenderer()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED);
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                else
                {
                    //Get window surface
                    gScreenSurface = SDL_GetWindowSurface( gWindow );
                }
            }
        }
    }
    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    //Initialize SDL_mixer
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    return success;
}

bool initLWindowRenderer(struct LWindow *inputStruct)
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
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

        //Create window
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
            //Create renderer for window
            inputStruct->mRenderer = SDL_CreateRenderer( inputStruct->mWindow, -1, SDL_RENDERER_ACCELERATED);
            if( inputStruct->mRenderer  == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( inputStruct->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                else
                {
                    //Get window surface
                    gScreenSurface = SDL_GetWindowSurface( inputStruct->mWindow );
                    //Grab window identifier
                    inputStruct->mWindowID = SDL_GetWindowID( inputStruct->mWindow );
                    inputStruct->mWindowDisplayID = SDL_GetWindowDisplayIndex( inputStruct->mWindow );
                    //Flag as opened
                    inputStruct->mShown = true;
                }
            }
        }
    }
    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    //Initialize SDL_mixer
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    return inputStruct->mWindow != NULL && inputStruct->mRenderer != NULL && success;
}


SDL_Surface* loadSurface( char *path )
{
    //The final optimized image
	SDL_Surface* optimizedSurface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", path, IMG_GetError() );
    }
    else
    {
       //Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface( loadedSurface, gScreenSurface->format, 0 );
		if( optimizedSurface == NULL )
		{
			printf( "Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return optimizedSurface;
}

SDL_Texture* loadTexture( char *path )
{
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
        }
        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }
    return newTexture;
}


bool LTexture(struct textureStruct *structinput)
{
    mTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load( structinput->imagePath );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", structinput->imagePath, IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", structinput->imagePath, SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            structinput->mWidth = loadedSurface->w;
            structinput->mHeight = loadedSurface->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }
    //Return success
    structinput->mTexture = newTexture;
    return newTexture != NULL;
}


void textureRender(struct textureStruct *structinput, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip, int posX, int posY)
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { structinput->xPos+ posX, structinput->yPos+ posY, structinput->mWidth, structinput->mHeight };

    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    //Render to screen
    //SDL_RenderCopy( gRenderer, structinput->mTexture, clip, &renderQuad );
    SDL_RenderCopyEx( gRenderer, structinput->mTexture, clip, &renderQuad, angle, center, flip );
}

bool loadFromRenderedText(struct ttfStruct *structinput, SDL_Color textColor )
{
    //Get rid of preexisting texture
    //closeTexture();
    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, structinput->textureText, textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            structinput->mWidth = textSurface->w;
            structinput->mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }

    //Return success
    structinput->mTexture = mTexture;
    return mTexture != NULL;
}

void close()
{
    //Deallocate surface
    SDL_FreeSurface( gImage );
    gImage= NULL;

    //Free global font
    TTF_CloseFont( gFont );
    gFont = NULL;

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Free the music

    //Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
}

void closeTexture()
{
    //Free loaded image
    SDL_DestroyTexture( gTexture );
    gTexture = NULL;

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* args[])
{
    SDL_Color highlightColor = { 0xFF, 0, 0, 0xFF };
    bool quit = false;
    SDL_Event e;
    double degrees = 0;
    SDL_RendererFlip flipType = SDL_FLIP_NONE;
    SDL_Rect* gDisplayBounds = NULL;
    SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    struct LWindow gWindow;

    //Start up SDL and create window
    if( !initRenderer() )
    {
        printf( "Failed to initialize!\n" );
    }
    struct ttfStruct gTimeTexture;
    struct textureStruct gSceneTexture;

    SDL_Color textColor = { 0, 0, 0, 255 };

    //gBGTexture.imagePath = "31_scrolling_backgrounds/bg.png";
    char *inputText = "lmao";

    gSceneTexture.imagePath = "texture.png";
    if( !LTexture(&gSceneTexture) )
    {
        printf( "Failed to load media! \n" );
    }
    gSceneTexture.xPos = ( 0 ) ;
    gSceneTexture.yPos = ( 0 ) ;
    while( !quit )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }

        //Handle window events
     //   handleLWindowEvent(&gWindow ,&e);
        }


        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

       /* if( !loadMedia(&gfpsTexture, textColor) )
        {
            printf( "Failed to load media! \n" );
        }*/

        textureRender(&gSceneTexture, &camera, degrees, NULL, flipType, 0 , 0);
}
        //Free resources and close SDL
        close();

        return 0;
}
