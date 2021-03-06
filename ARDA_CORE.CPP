#include <SDL2/SDL.h>
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"

SDL_Window*             mainWindow = NULL;
SDL_Renderer*		    mainRenderer = NULL;
SDL_Texture*		    defaultTexture = NULL;
int32_t			            mainWindowWidth;
int32_t			            mainWindowHeight;
uint32_t                start_clock_count;

ARDA_RESULT ARDA_Init(void){

    ARDA_RESULT flag = 0;

    //SDL INIT
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        flag = ERROR_CUSTOM;
    }

    if (ARDA_MathInit() != 0) {
        SDL_Log("Unable to initialize math module: %s", SDL_GetError());
        flag = ERROR_CUSTOM;
    }
    //CREATE MAIN WINDOW
    mainWindow = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
    if(mainWindow == NULL){
        SDL_Log("Unable to create the mainWindow: %s", SDL_GetError());
        flag = ERROR_CUSTOM;
    }
    SDL_GetWindowSize(mainWindow, &mainWindowWidth, &mainWindowHeight);
    //SDL_Log("%i %i", mainWindowWidth, mainWindowHeight);
    //GET PRIMARY SURFACE
    /*Could not be used together with renderer
    primarySurface = SDL_GetWindowSurface(mainWindow);
    if(primarySurface == NULL){
        SDL_Log("Unable to get the primary surface: %s", SDL_GetError());
        flag = ERROR_CUSTOM;
    }
    */
    //CREATE MAIN RENDERER
    mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    //mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
    if(mainRenderer == NULL){
	SDL_Log("Unable to Create the main renderer: %s", SDL_GetError());
        flag = ERROR_CUSTOM;
    }
    //CREATE DEFAULT TEXTURE
    //defaultTexture = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, mainWindowWidth, mainWindowHeight);
    defaultTexture = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, mainWindowWidth, mainWindowHeight);
    if(defaultTexture == NULL){
	SDL_Log("Unable to Create the default texture: %s", SDL_GetError());
        flag = ERROR_CUSTOM;
    }

    return flag;
}
ARDA_RESULT ARDA_Shutdown(){
    SDL_DestroyTexture(defaultTexture);
    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
    return(0);
}

uint32_t Get_Clock(void)
{
    // this function returns the current tick count

    // return time
    return SDL_GetTicks();

} // end Get_Clock

///////////////////////////////////////////////////////////

uint32_t Start_Clock(void)
{
    // this function starts the clock, that is, saves the current
    // count, use in conjunction with Wait_Clock()

    return (start_clock_count = SDL_GetTicks());

} // end Start_Clock

////////////////////////////////////////////////////////////

uint32_t Wait_Clock(uint32_t count)
{
    // this function is used to wait for a specific number of clicks
    // since the call to Start_Clock

    while((Get_Clock() - start_clock_count) < count);
    return SDL_GetTicks();

} // end Wait_Clock
