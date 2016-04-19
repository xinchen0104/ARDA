#include <SDL2/SDL.h>
#include <stdio.h> /* for fprintf() */
#include <stdlib.h> /* for atexit() */
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"
#include "ARDA_2D.H"

extern SDL_Renderer*		mainRenderer;
extern SDL_Texture*		    defaultTexture;
uint32_t*			        defaultBuffer = NULL;
int				            defaultLpitchByte;
extern int32_t			            mainWindowHeight;
extern int32_t			            mainWindowWidth;
ARDA_RESULT GameInit(void);
ARDA_RESULT GameMain(void);
ARDA_RESULT GameShutdown(void);
POLYGON2D triangle;
int main(int argc, char** argv) {
    GameInit();
    bool quit = false;
    SDL_Event e;
    while(!quit){
	if(SDL_PollEvent(&e) != 0){
	    if(e.type == SDL_QUIT){
		quit = true;
	    }
	    else if(e.type == SDL_KEYDOWN){
	    	switch(e.key.keysym.sym){
		    case SDLK_ESCAPE :
			quit = true;
			break;
		    default:
			break;
		}
	    }
	}
	GameMain();
    }
    GameShutdown();
    return 0;
}
ARDA_RESULT GameInit(void){
    ARDA_RESULT flag = ARDA_Init();
    SDL_Rect cr = {0,0,mainWindowWidth,mainWindowHeight};
    ARDA_SetClipRect(cr);
    triangle = {1, 3, 300, 300, 0, 0, _RGB32(255, 0, 127, 127), NULL};
    triangle.vlist = (VERTEX2DF*)malloc(triangle.num_verts*sizeof(VERTEX2DF));
    triangle.vlist[0] = {-120, 20};
    triangle.vlist[1] = {20, 400};
    triangle.vlist[2] = {10, 40};
    return flag;
}
ARDA_RESULT GameShutdown(void){
    return ARDA_Shutdown();
}
ARDA_RESULT GameMain(void){
    static int32_t lpitch;
    SDL_LockTexture(defaultTexture, NULL, (void**)&defaultBuffer, &defaultLpitchByte);
    lpitch = defaultLpitchByte >> 2;
    memset((void*)defaultBuffer, 0 , mainWindowHeight * defaultLpitchByte);
    ARDA_VLine(-10, 2000, 500, _RGB32(255, 127, 127, 0), defaultBuffer, lpitch);
    ARDA_HLine(-10, 2000, 500, _RGB32(255, 127, 127, 0), defaultBuffer, lpitch);
    ARDA_TranslatePolygon2D(&triangle, 1, 1);
    ARDA_RotatePolygon2D(&triangle, -362);
    ARDA_ScalePolygon2D(&triangle, 0.98, 1.02);
    ARDA_DrawPolygon2D(&triangle, defaultBuffer, lpitch);
    SDL_UnlockTexture(defaultTexture);
    SDL_RenderClear(mainRenderer);
    SDL_RenderCopy(mainRenderer, defaultTexture, NULL, NULL);
    SDL_RenderPresent(mainRenderer);
    return 0;
}


