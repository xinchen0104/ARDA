#include <SDL2/SDL.h>
#include <stdio.h> /* for fprintf() */
#include <stdlib.h> /* for atexit() */
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"
#include "ARDA_2D.H"
#include "ARDA_3DV1.H"
#include "ARDA_3D_LIGHTV1.H"
#include "ARDA_IMPORTER.H"

extern SDL_Renderer*		mainRenderer;
extern SDL_Texture*		    defaultTexture;
uint32_t*			        defaultBuffer = NULL;
int				            defaultLpitchByte;
int32_t                     lpitch;
extern int32_t			    mainWindowHeight;
extern int32_t			    mainWindowWidth;
extern LIGHTV1 gLights[MAX_LIGHTS];

const int NUM_TOWERS        = 64,
          NUM_TANKS         = 32,
          TANK_SPEED        = 15,
          UNIVERSE_RADIUS   = 4000,
          POINT_SIZE        = 200;

const float NUM_POINTS_X = (2*UNIVERSE_RADIUS/POINT_SIZE),
            NUM_POINTS_Z = (2*UNIVERSE_RADIUS/POINT_SIZE),
            NUM_POINTS   = (NUM_POINTS_X*NUM_POINTS_Z);

const int AMBIENT_LIGHT_INDEX   = 0, // ambient light index
          INFINITE_LIGHT_INDEX  = 1, // infinite light index
          POINT_LIGHT_INDEX     = 2, // point light index
          SPOT_LIGHT_INDEX      = 3; // spot light index

RGBAV1 white, gray, black, red, green, blue;
CAM4DV1 cam;
RENDERLIST4DV1 rend_list;
OBJECT4DV1 obj_tank, obj_tower, obj_marker, obj_player;
POINT4D        towers[NUM_TOWERS],
               tanks[NUM_TANKS];


ARDA_RESULT GameInit(void);
ARDA_RESULT GameMain(void);
ARDA_RESULT GameShutdown(void);
void CalcuFPS();
void RenderMain();

ARDA_RESULT GameInit(void){

    ARDA_RESULT flag = ARDA_Init();
    SDL_Rect cr = {0,0,mainWindowWidth,mainWindowHeight};
    ARDA_SetClipRect(cr);

    white.rgba = _RGBA32(255,255,255,0);
    gray.rgba  = _RGBA32(100,100,100,0);
    black.rgba = _RGBA32(0,0,0,0);
    red.rgba   = _RGBA32(255,0,0,0);
    green.rgba = _RGBA32(0,255,0,0);
    blue.rgba  = _RGBA32(0,0,255,0);


    POINT4D cam_pos     = {0, 40, 0, 1};
    VECTOR4D cam_dir    = {0, 0, 0, 1};
    VECTOR4D cam_target = {0 ,0, 0, 1};
    /*ARDA_RESULT ARDA_Init_CAM4DV1(CAM4DV1_PTR cam, int attr, POINT4D_PTR cam_pos,
                  VECTOR4D_PTR cam_dir, VECTOR4D_PTR cam_target,
                  float near_clip_z, float far_clip_z, float fov,
                  float viewport_width,  float viewport_height)*/
    ARDA_Init_CAM4DV1(&cam, CAM_MODEL_EULER, &cam_pos, &cam_dir, &cam_target, 200, 12000, 120, mainWindowWidth, mainWindowHeight);





    VECTOR4D vscale = {1 ,1, 1, 1};
    VECTOR4D vpos   = {0 ,0, 0, 1};
    VECTOR4D vrot   = {0 ,0, 0, 1};

    // load the master tank object
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV1_PLG(&obj_tank, "tank3.plg",&vscale, &vpos, &vrot);

    // load player object for 3rd person view
    VECTOR4D_INITXYZ(&vscale,0.75,0.75,0.75);
    Load_OBJECT4DV1_PLG(&obj_player, "tank2.plg",&vscale, &vpos, &vrot);


    // load the master tower object
    VECTOR4D_INITXYZ(&vscale,1.0, 2.0, 1.0);
    Load_OBJECT4DV1_PLG(&obj_tower, "tower1.plg",&vscale, &vpos, &vrot);

    // load the master ground marker
    VECTOR4D_INITXYZ(&vscale,3.0,3.0,3.0);
    Load_OBJECT4DV1_PLG(&obj_marker, "marker1.plg",&vscale, &vpos, &vrot);


        // position the tanks
    for (int index = 0; index < NUM_TANKS; index++)
        {
        // randomly position the tanks
        tanks[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].y = 0; // obj_tank.max_radius;
        tanks[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        tanks[index].w = RAND_RANGE(0,360);
        } // end for

    // position the towers
    for (int index = 0; index < NUM_TOWERS; index++)
        {
        // randomly position the tower
        towers[index].x = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        towers[index].y = 0; // obj_tower.max_radius;
        towers[index].z = RAND_RANGE(-UNIVERSE_RADIUS, UNIVERSE_RADIUS);
        } // end for
    ARDA_ResetLights_LIGHTV1();
    // ambient light
    ARDA_InitLight_LIGHTV1(AMBIENT_LIGHT_INDEX,
                       LIGHTV1_STATE_ON,      // turn the light on
                       LIGHTV1_ATTR_AMBIENT,  // ambient light type
                       gray, black, black,    // color for ambient term only
                       NULL, NULL,            // no need for pos or dir
                       0,0,0,                 // no need for attenuation
                       0,0,0);                // spotlight info NA


    VECTOR4D dlight_dir = {-1,0,-1,0};

    // directional light
    ARDA_InitLight_LIGHTV1(INFINITE_LIGHT_INDEX,
                       LIGHTV1_STATE_ON,      // turn the light on
                       LIGHTV1_ATTR_INFINITE, // infinite light type
                       black, gray, black,    // color for diffuse term only
                       NULL, &dlight_dir,     // need direction only
                       0,0,0,                 // no need for attenuation
                       0,0,0);                // spotlight info NA


    VECTOR4D plight_pos = {0,200,0,0};

    // point light
    ARDA_InitLight_LIGHTV1(POINT_LIGHT_INDEX,
                       LIGHTV1_STATE_ON,      // turn the light on
                       LIGHTV1_ATTR_POINT,    // pointlight type
                       black, white, black,   // color for diffuse term only
                       &plight_pos, NULL,     // need pos only
                       0,.001,0,              // linear attenuation only
                       0,0,1);                // spotlight info NA

    VECTOR4D slight_pos = {0,200,0,0};
    VECTOR4D slight_dir = {-1,0,-1,0};

    // spot light
    ARDA_InitLight_LIGHTV1(SPOT_LIGHT_INDEX,
                       LIGHTV1_STATE_ON,         // turn the light on
                       LIGHTV1_ATTR_SPOTLIGHT2,  // spot light type 2
                       black, white, black,      // color for diffuse term only
                       &slight_pos, &slight_dir, // need pos only
                       0,.001,0,                 // linear attenuation only
                       0,0,1);

    return flag;
}

void RenderMain(){

    static MATRIX4X4 mrot;   // general rotation matrix

    // these are used to create a circling camera
    static float view_angle = 0;
    static float camera_distance = 6000;
    static VECTOR4D pos = {0,0,0,0};
    static float tank_speed;
    static float turning = 0;
    static const uint8_t *keyboard_state;

    static int  wireframe_mode = -1,
                backface_mode  = 1,
                lighting_mode  = 1,
                help_mode      = 1,
                zsort_mode     = 1;

    char work_string[256]; // temp string


    static float sun_angle = 0;

    ARDA_Reset_RENDERLIST4DV1(&rend_list);

    keyboard_state = SDL_GetKeyboardState(NULL);

    if (keyboard_state[SDL_SCANCODE_SPACE])
        tank_speed = 5*TANK_SPEED;
    else
        tank_speed = TANK_SPEED;

    // forward/backward
    if (keyboard_state[SDL_SCANCODE_UP])
       {
       // move forward
       cam.pos.x += tank_speed*Fast_Sin(cam.dir.y);
       cam.pos.z += tank_speed*Fast_Cos(cam.dir.y);
       } // end if

    if (keyboard_state[SDL_SCANCODE_DOWN])
       {
       // move backward
       cam.pos.x -= tank_speed*Fast_Sin(cam.dir.y);
       cam.pos.z -= tank_speed*Fast_Cos(cam.dir.y);
       } // end if

    // rotate
    if (keyboard_state[SDL_SCANCODE_RIGHT])
       {
       cam.dir.y+=3;

       // add a little turn to object
       if ((turning+=2) > 15)
          turning=15;
       } // end if

    if (keyboard_state[SDL_SCANCODE_LEFT])
       {
       cam.dir.y-=3;

       // add a little turn to object
       if ((turning-=2) < -15)
          turning=-15;

       } // end if
    else // center heading again
       {
       if (turning > 0)
           turning-=1;
       else
       if (turning < 0)
           turning+=1;

       } // end else
    // wireframe mode
    if (keyboard_state[SDL_SCANCODE_W])
       {
       // toggle wireframe mode
       wireframe_mode = -wireframe_mode;
       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if

    // backface removal
    if (keyboard_state[SDL_SCANCODE_B])
       {
       // toggle backface removal
       backface_mode = -backface_mode;
       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if

    // lighting
    if (keyboard_state[SDL_SCANCODE_L])
       {
       // toggle lighting engine completely
       lighting_mode = -lighting_mode;
       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if

    // toggle ambient light
    if (keyboard_state[SDL_SCANCODE_A])
       {
       // toggle ambient light
       if (gLights[AMBIENT_LIGHT_INDEX].state == LIGHTV1_STATE_ON)
          gLights[AMBIENT_LIGHT_INDEX].state = LIGHTV1_STATE_OFF;
       else
          gLights[AMBIENT_LIGHT_INDEX].state = LIGHTV1_STATE_ON;

       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if

    // toggle infinite light
    if (keyboard_state[SDL_SCANCODE_I])
       {
       // toggle ambient light
       if (gLights[INFINITE_LIGHT_INDEX].state == LIGHTV1_STATE_ON)
          gLights[INFINITE_LIGHT_INDEX].state = LIGHTV1_STATE_OFF;
       else
          gLights[INFINITE_LIGHT_INDEX].state = LIGHTV1_STATE_ON;

       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if

    // toggle point light
    if (keyboard_state[SDL_SCANCODE_P])
       {
       // toggle point light
       if (gLights[POINT_LIGHT_INDEX].state == LIGHTV1_STATE_ON)
          gLights[POINT_LIGHT_INDEX].state = LIGHTV1_STATE_OFF;
       else
          gLights[POINT_LIGHT_INDEX].state = LIGHTV1_STATE_ON;

       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if


    // toggle spot light
    if (keyboard_state[SDL_SCANCODE_S])
       {
       // toggle spot light
       if (gLights[SPOT_LIGHT_INDEX].state == LIGHTV1_STATE_ON)
          gLights[SPOT_LIGHT_INDEX].state = LIGHTV1_STATE_OFF;
       else
          gLights[SPOT_LIGHT_INDEX].state = LIGHTV1_STATE_ON;

       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if


    // help menu
    if (keyboard_state[SDL_SCANCODE_H])
       {
       // toggle help menu
       help_mode = -help_mode;
       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if

    // z-sorting
    if (keyboard_state[SDL_SCANCODE_Z])
       {
       // toggle z sorting
       zsort_mode = -zsort_mode;
       Wait_Clock(100); // wait, so keyboard doesn't bounce
       } // end if
    static float plight_ang = 0, slight_ang = 0; // angles for light motion

    // move point light source in ellipse around game world
    gLights[POINT_LIGHT_INDEX].pos.x = 4000*Fast_Cos(plight_ang);
    gLights[POINT_LIGHT_INDEX].pos.y = 200;
    gLights[POINT_LIGHT_INDEX].pos.z = 4000*Fast_Sin(plight_ang);

    if ((plight_ang+=3) > 360)
        plight_ang = 0;

    // move spot light source in ellipse around game world
    gLights[SPOT_LIGHT_INDEX].pos.x = 2000*Fast_Cos(slight_ang);
    gLights[SPOT_LIGHT_INDEX].pos.y = 200;
    gLights[SPOT_LIGHT_INDEX].pos.z = 2000*Fast_Sin(slight_ang);

    if ((slight_ang-=5) < 0)
        slight_ang = 360;


    // generate camera matrix
    ARDA_BuildMatrixEuler_CAM4DV1(&cam, CAM_ROT_SEQ_ZYX);


    // insert the player into the world
    // reset the object (this only matters for backface and object removal)
    ARDA_Reset_OBJECT4DV1(&obj_player);

    // set position of tank
    obj_player.world_pos.x = cam.pos.x+300*Fast_Sin(cam.dir.y);
    obj_player.world_pos.y = cam.pos.y-70;
    obj_player.world_pos.z = cam.pos.z+300*Fast_Cos(cam.dir.y);

    // generate rotation matrix around y axis
    ARDA_BuildXYZRotation_MATRIX4X4(0, cam.dir.y+turning, 0, &mrot);

    // rotate the local coords of the object
    ARDA_Transform_OBJECT4DV1(&obj_player, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

    // perform world transform
    ARDA_ModelToWorld_OBJECT4DV1(&obj_player, TRANSFORM_TRANS_ONLY);

    // insert the object into render list
    ARDA_InsertOBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_player, 0);

    //////////////////////////////////////////////////////////

    // insert the tanks in the world
    for (int index = 0; index < NUM_TANKS; index++)
        {
        // reset the object (this only matters for backface and object removal)
        ARDA_Reset_OBJECT4DV1(&obj_tank);

        // generate rotation matrix around y axis
        ARDA_BuildXYZRotation_MATRIX4X4(0, tanks[index].w, 0, &mrot);

        // rotate the local coords of the object
        ARDA_Transform_OBJECT4DV1(&obj_tank, &mrot, TRANSFORM_LOCAL_TO_TRANS,1);

        // set position of tank
        obj_tank.world_pos.x = tanks[index].x;
        obj_tank.world_pos.y = tanks[index].y;
        obj_tank.world_pos.z = tanks[index].z;

        // attempt to cull object
        if (!ARDA_Cull_OBJECT4DV1(&obj_tank, &cam, CULL_OBJECT_XYZ_PLANES))
           {
           // if we get here then the object is visible at this world position
           // so we can insert it into the rendering list
           // perform local/model to world transform
           ARDA_ModelToWorld_OBJECT4DV1(&obj_tank, TRANSFORM_TRANS_ONLY);

           // insert the object into render list
           ARDA_InsertOBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_tank,0);
           } // end if

        } // end for

    ////////////////////////////////////////////////////////

    // insert the towers in the world
    for (int index = 0; index < NUM_TOWERS; index++)
        {
        // reset the object (this only matters for backface and object removal)
        ARDA_Reset_OBJECT4DV1(&obj_tower);

        // set position of tower
        obj_tower.world_pos.x = towers[index].x;
        obj_tower.world_pos.y = towers[index].y;
        obj_tower.world_pos.z = towers[index].z;

        // attempt to cull object
        if (!ARDA_Cull_OBJECT4DV1(&obj_tower, &cam, CULL_OBJECT_XYZ_PLANES))
           {
           // if we get here then the object is visible at this world position
           // so we can insert it into the rendering list
           // perform local/model to world transform
           ARDA_ModelToWorld_OBJECT4DV1(&obj_tower);

           // insert the object into render list
           ARDA_InsertOBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_tower,0);

           } // end if

        } // end for

    ///////////////////////////////////////////////////////////////

    // seed number generator so that modulation of markers is always the same
    srand(13);

    // insert the ground markers into the world
    for (int index_x = 0; index_x < NUM_POINTS_X; index_x++)
        for (int index_z = 0; index_z < NUM_POINTS_Z; index_z++)
            {
            // reset the object (this only matters for backface and object removal)
            ARDA_Reset_OBJECT4DV1(&obj_marker);

            // set position of tower
            obj_marker.world_pos.x = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_x*POINT_SIZE;
            obj_marker.world_pos.y = obj_marker.max_radius;
            obj_marker.world_pos.z = RAND_RANGE(-100,100)-UNIVERSE_RADIUS+index_z*POINT_SIZE;

            // attempt to cull object
            if (!ARDA_Cull_OBJECT4DV1(&obj_marker, &cam, CULL_OBJECT_XYZ_PLANES))
               {
               // if we get here then the object is visible at this world position
               // so we can insert it into the rendering list
               // perform local/model to world transform
               ARDA_ModelToWorld_OBJECT4DV1(&obj_marker);

               // insert the object into render list
               ARDA_InsertOBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_marker,0);

               } // end if

            } // end for

    ////////////////////////////////////////////////////////////////////////


// remove backfaces
    if (backface_mode==1)
       ARDA_RemoveBackfaces_RENDERLIST4DV1(&rend_list, &cam);

    // light scene all at once
    if (lighting_mode==1)
       ARDA_LightWorld_RENDERLIST4DV1(&rend_list, &cam, gLights, 4);

    // apply world to camera transform
    ARDA_WorldToCamera_RENDERLIST4DV1(&rend_list, &cam);

    // sort the polygon list (hurry up!)
    if (zsort_mode == 1)
       ARDA_Sort_RENDERLIST4DV1(&rend_list,  SORT_POLYLIST_AVGZ);


    // apply camera to perspective transformation
    ARDA_CameraToPerspective_RENDERLIST4DV1(&rend_list, &cam);

    // apply screen transform
    ARDA_PerspectiveToScreen_RENDERLIST4DV1(&rend_list, &cam);

    SDL_LockTexture(defaultTexture, NULL, (void**)&defaultBuffer, &defaultLpitchByte);
    lpitch = defaultLpitchByte >> 2;
    memset((void*)defaultBuffer, 0 , mainWindowHeight * defaultLpitchByte);
    if (wireframe_mode  == 1)
       ARDA_DrawWire_RENDERLIST4DV1(&rend_list, defaultBuffer, lpitch);
    else
       ARDA_DrawSolid_RENDERLIST4DV1(&rend_list, defaultBuffer, lpitch, 1);
    SDL_UnlockTexture(defaultTexture);
    SDL_RenderClear(mainRenderer);
    SDL_RenderCopy(mainRenderer, defaultTexture, NULL, NULL);
    SDL_RenderPresent(mainRenderer);
}


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
ARDA_RESULT GameShutdown(void){
    return ARDA_Shutdown();
}
ARDA_RESULT GameMain(void){
    Start_Clock();
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



