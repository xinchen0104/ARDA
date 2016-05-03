#include <SDL2/SDL.h>
#include <stdio.h> /* for fprintf() */
#include <stdlib.h> /* for atexit() */
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"
#include "ARDA_2D.H"
#include "ARDA_3DV1.H"

extern SDL_Renderer*		mainRenderer;
extern SDL_Texture*		    defaultTexture;
uint32_t*			        defaultBuffer = NULL;
int				            defaultLpitchByte;
int32_t                     lpitch;
extern int32_t			    mainWindowHeight;
extern int32_t			    mainWindowWidth;

CAM4DV1 cam;
RENDERLIST4DV1 rend_list;
POLYF4DV1 poly1;


ARDA_RESULT GameInit(void);
ARDA_RESULT GameMain(void);
ARDA_RESULT GameShutdown(void);
void CalcuFPS();
void RenderMain();
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
    POINT4D cam_pos = {0, 0, 0, 1};
    VECTOR4D cam_dir = {0, 0, 0, 1};
    VECTOR4D cam_target = {0 ,0, 1, 1};
    /*ARDA_RESULT ARDA_Init_CAM4DV1(CAM4DV1_PTR cam, int attr, POINT4D_PTR cam_pos,
                  VECTOR4D_PTR cam_dir, VECTOR4D_PTR cam_target,
                  float near_clip_z, float far_clip_z, float fov,
                  float viewport_width,  float viewport_height)*/
    ARDA_Init_CAM4DV1(&cam, CAM_MODEL_UVN, &cam_pos, &cam_dir, &cam_target, 200, 2000, 10, mainWindowWidth, mainWindowHeight);
    ARDA_Reset_RENDERLIST4DV1(&rend_list);
    poly1.state  = POLY4DV1_STATE_ACTIVE;
    poly1.attr   =  0;
    poly1.color  = _RGB32(0,255,0,0);

    poly1.vlist[0].x = 0;
    poly1.vlist[0].y = 50;
    poly1.vlist[0].z = 0;
    poly1.vlist[0].w = 1;

    poly1.vlist[1].x = 50;
    poly1.vlist[1].y = -50;
    poly1.vlist[1].z = 0;
    poly1.vlist[1].w = 1;

    poly1.vlist[2].x = -50;
    poly1.vlist[2].y = -50;
    poly1.vlist[2].z = 0;
    poly1.vlist[2].w = 1;

    poly1.next = poly1.prev = NULL;

    return flag;
}
ARDA_RESULT GameShutdown(void){
    return ARDA_Shutdown();
}
ARDA_RESULT GameMain(void){
    CalcuFPS();
    RenderMain();
    return 0;
}
void CalcuFPS(){
    static uint32_t lastTime = 0, currentTime = 0;
    static uint32_t frames = 0;
    currentTime = SDL_GetTicks();
    frames ++;
    if(currentTime - lastTime > 1000){
        SDL_Log("%f frames", (float) frames / (currentTime - lastTime) * 1000);
        lastTime = currentTime;
        frames = 0;
    }
}

void RenderMain(){
    static float ang_y;
    static MATRIX4X4 mrot;
    static POINT4D poly1_pos = {0,0,400,1};

    ARDA_Reset_RENDERLIST4DV1(&rend_list);

    ARDA_InsertPOLYF4DV1_RENDERLIST4DV1(&rend_list, &poly1);

    ARDA_BuildXYZRotation_MATRIX4X4(0, ang_y, 0, &mrot);

    if (++ang_y >= 360.0) ang_y = 0;

    ARDA_Transform_RENDERLIST4DV1(&rend_list, &mrot, TRANSFORM_LOCAL_ONLY);

    ARDA_ModelToWorld_RENDERLIST4DV1(&rend_list, &poly1_pos);

    ARDA_RemoveBackfaces_RENDERLIST4DV1(&rend_list, &cam);

//    ARDA_BuildMatrixEuler_CAM4DV1(&cam, CAM_ROT_SEQ_ZYX);

    ARDA_BuildMatrixUVN_CAM4DV1(&cam, CAM_ROT_SEQ_ZYX);

    ARDA_WorldToCamera_RENDERLIST4DV1(&rend_list, &cam);

    ARDA_CameraToPerspective_RENDERLIST4DV1(&rend_list, &cam);

    ARDA_PerspectiveToScreen_RENDERLIST4DV1(&rend_list, &cam);

    SDL_LockTexture(defaultTexture, NULL, (void**)&defaultBuffer, &defaultLpitchByte);
    lpitch = defaultLpitchByte >> 2;
    memset((void*)defaultBuffer, 0 , mainWindowHeight * defaultLpitchByte);
    ARDA_DrawWire_RENDERLIST4DV1(&rend_list, defaultBuffer, lpitch);
    SDL_UnlockTexture(defaultTexture);
    SDL_RenderClear(mainRenderer);
    SDL_RenderCopy(mainRenderer, defaultTexture, NULL, NULL);
    SDL_RenderPresent(mainRenderer);
}

