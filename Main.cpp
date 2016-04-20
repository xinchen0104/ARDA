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
uint32_t                    frames;
uint32_t                    lastTime, currentTime;
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
    triangle.vlist[0] = {-20, 400};
    triangle.vlist[1] = {700, 900};
    triangle.vlist[2] = {300, 160};
    return flag;
}
ARDA_RESULT GameShutdown(void){
    return ARDA_Shutdown();
}
ARDA_RESULT GameMain(void){
    static int32_t lpitch;
    currentTime = SDL_GetTicks();
    frames ++;
    if(currentTime - lastTime > 1000){
        SDL_Log("%f frames", (float) frames / (currentTime - lastTime) * 1000);
        lastTime = currentTime;
        frames = 0;
    }
    SDL_LockTexture(defaultTexture, NULL, (void**)&defaultBuffer, &defaultLpitchByte);
    lpitch = defaultLpitchByte >> 2;
    memset((void*)defaultBuffer, 0 , mainWindowHeight * defaultLpitchByte);
    ARDA_TranslatePolygon2D(&triangle, 1, 1);
    ARDA_RotatePolygon2D(&triangle, -362);
    ARDA_ScalePolygon2D(&triangle, 1.01, 1.01);
    ARDA_DrawPolygon2D(&triangle, defaultBuffer, lpitch);
    ARDA_FilledTriangle2D(triangle.vlist[0].x + triangle.x0, triangle.vlist[0].y + triangle.y0, triangle.vlist[1].x + triangle.x0, triangle.vlist[1].y + triangle.y0, triangle.vlist[2].x + triangle.x0, triangle.vlist[2].y + triangle.y0, _RGB32(255,127,0,0),defaultBuffer,lpitch);
    SDL_UnlockTexture(defaultTexture);
    SDL_RenderClear(mainRenderer);
    SDL_RenderCopy(mainRenderer, defaultTexture, NULL, NULL);
    SDL_RenderPresent(mainRenderer);
    return 0;
}


