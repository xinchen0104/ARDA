#include <SDL2/SDL.h>
#include <stdio.h> /* for fprintf() */
#include <stdlib.h> /* for atexit() */
#include "ARDA_CORE.H"
#include "ARDA_2D.H"

extern SDL_Renderer*		mainRenderer;
extern SDL_Texture*		    defaultTexture;
uint32_t*			        defaultBuffer = NULL;
int				            defaultLpitchByte;
int GameInit(void);
int GameMain(void);
int GameShutdown(void);
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
int GameInit(void){
    ARDA_Init();
}
int GameShutdown(void){
    ARDA_Shutdown();
}
int GameMain(void){
    uint32_t tempColor;
    SDL_LockTexture(defaultTexture, NULL, (void**)&defaultBuffer, &defaultLpitchByte);
    int lpitch = defaultLpitchByte >> 2;
    SDL_Rect cr = {200,200,400,400};
    ARDA_SetClipRect(cr);
    int x1 = 300,
        y1 = 100,
        x2 = 300,
        y2 = 800;
    if(ARDA_ClipLine(x1, y1, x2, y2, NULL)){
    	ARDA_DrawLine(x1, y1, x2, y2, _RGB32(0, 255, 0, 0), defaultBuffer, lpitch);
    }else{
	SDL_Log("No line segement to draw");
    }
    SDL_UnlockTexture(defaultTexture);
    SDL_RenderClear(mainRenderer);
    SDL_RenderCopy(mainRenderer, defaultTexture, NULL, NULL);
    SDL_RenderPresent(mainRenderer);
}


