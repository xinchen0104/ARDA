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

const int NUM_TOWERS        = 64,
          NUM_TANKS         = 32,
          TANK_SPEED        = 15,
          UNIVERSE_RADIUS   = 4000,
          POINT_SIZE        = 200;

const float NUM_POINTS_X = (2*UNIVERSE_RADIUS/POINT_SIZE),
            NUM_POINTS_Z = (2*UNIVERSE_RADIUS/POINT_SIZE),
            NUM_POINTS   = (NUM_POINTS_X*NUM_POINTS_Z);


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

    // initialize camera position and direction
    POINT4D  cam_pos    = {0,40,0,1};
    POINT4D  cam_target = {0,0,0,1};
    VECTOR4D cam_dir    = {0,0,0,1};

    // all your initialization code goes here...
    VECTOR4D vscale={1.0,1.0,1.0,1},
             vpos = {0,0,0,1},
             vrot = {0,0,0,1};
    /*ARDA_RESULT ARDA_Init_CAM4DV1(CAM4DV1_PTR cam, int attr, POINT4D_PTR cam_pos,
                  VECTOR4D_PTR cam_dir, VECTOR4D_PTR cam_target,
                  float near_clip_z, float far_clip_z, float fov,
                  float viewport_width,  float viewport_height)*/

    // initialize the camera with 90 FOV, normalized coordinates
    ARDA_Init_CAM4DV1(&cam,      // the camera object
                 CAM_MODEL_EULER, // the euler model
                 &cam_pos,  // initial camera position
                 &cam_dir,  // initial camera angles
                 &cam_target,      // no target
                 200.0,      // near and far clipping planes
                 12000.0,
                 120.0,      // field of view in degrees
                 mainWindowWidth,   // size of final screen viewport
                 mainWindowHeight);

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
    static const uint8_t* keyboard_state;


    static float sun_angle = 0;

    // reset the render list
    ARDA_Reset_RENDERLIST4DV1(&rend_list);

    // allow user to move camera

    // turbo
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
    ARDA_InsertOBJECT4DV1_RENDERLIST4DV1(&rend_list, &obj_player,0);

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
    ARDA_RemoveBackfaces_RENDERLIST4DV1(&rend_list, &cam);

    // apply world to camera transform
    ARDA_WorldToCamera_RENDERLIST4DV1(&rend_list, &cam);

    // apply camera to perspective transformation
    ARDA_CameraToPerspective_RENDERLIST4DV1(&rend_list, &cam);

    // apply screen transform
    ARDA_PerspectiveToScreen_RENDERLIST4DV1(&rend_list, &cam);
    SDL_LockTexture(defaultTexture, NULL, (void**)&defaultBuffer, &defaultLpitchByte);
    lpitch = defaultLpitchByte >> 2;
    memset((void*)defaultBuffer, 0 , mainWindowHeight * defaultLpitchByte);
    ARDA_DrawWire_RENDERLIST4DV1(&rend_list, defaultBuffer, lpitch);
    //ARDA_DrawSolid_RENDERLIST4DV1(&rend_list, defaultBuffer, lpitch, 0);
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
