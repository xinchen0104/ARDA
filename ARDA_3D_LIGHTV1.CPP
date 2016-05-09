// INCLUDES ///////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"
#include "ARDA_2D.H"
#include "ARDA_3DV1.H"
#include "ARDA_3D_LIGHTV1.H"
// GLOBALS ////////////////////////////////////////////////////

MATERIALV1 gMaterials[MAX_MATERIALS]; // materials in system
int gNum_materials;              // current number of materials


LIGHTV1 gLights[MAX_LIGHTS];  // lights in system
int gNum_lights;              // current number of lights


// FUNCTIONS //////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_ResetLights_LIGHTV1(void)
{
    // this function simply resets all lights in the system
    //static int first_time = 1;

    memset(gLights, 0, MAX_LIGHTS*sizeof(LIGHTV1));

    // reset number of lights
    gNum_lights = 0;

    // reset first time
    //first_time = 0;

    // return success
    return 0;

} // end Reset_Lights_LIGHTV1

//////////////////////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_ResetMaterials_MATERIALV1(void)
{
    // this function resets all the materials
    static int first_time = 1;

    // if this is the first time then zero EVERYTHING out
    if (first_time)
       {
       memset(gMaterials, 0, MAX_MATERIALS*sizeof(MATERIALV1));
       first_time = 0;
       } // end if

    // scan thru materials and release all textures, if any?
    for (int curr_matt = 0; curr_matt < MAX_MATERIALS; curr_matt++)
        {
        // regardless if the material is active check to see if there is a
        // dangling texture map
        SDL_DestroyTexture(gMaterials[curr_matt].texture);

        // now it's safe to zero memory out
        memset(&gMaterials[curr_matt], 0, sizeof(MATERIALV1));
        } // end if

    return 0;

} // end Reset_Materials_MATV1

//////////////////////////////////////////////////////////////////////////////

int ARDA_InitLight_LIGHTV1(int           index,      // index of light to create (0..MAX_LIGHTS-1)
                       int          _state,      // state of light
                       int          _attr,       // type of light, and extra qualifiers
                       RGBAV1       _c_ambient,  // ambient light intensity
                       RGBAV1       _c_diffuse,  // diffuse light intensity
                       RGBAV1       _c_specular, // specular light intensity
                       POINT4D_PTR  _pos,        // position of light
                       VECTOR4D_PTR _dir,        // direction of light
                       float        _kc,         // attenuation factors
                       float        _kl,
                       float        _kq,
                       float        _spot_inner, // inner angle for spot light
                       float        _spot_outer, // outer angle for spot light
                       float        _pf)         // power factor/falloff for spot lights
{
    // this function initializes a light based on the flags sent in _attr, values that
    // aren't needed are set to 0 by caller

    // make sure light is in range
    if (index < 0 || index >= MAX_LIGHTS)
        return -1;

    // all good, initialize the light (many fields may be dead)
    gLights[index].state       = _state;      // state of light
    gLights[index].id          = index;       // id of light
    gLights[index].attr        = _attr;       // type of light, and extra qualifiers

    gLights[index].c_ambient   = _c_ambient;  // ambient light intensity
    gLights[index].c_diffuse   = _c_diffuse;  // diffuse light intensity
    gLights[index].c_specular  = _c_specular; // specular light intensity

    gLights[index].kc          = _kc;         // constant, linear, and quadratic attenuation factors
    gLights[index].kl          = _kl;
    gLights[index].kq          = _kq;

    if (_pos)
       VECTOR4D_COPY(&gLights[index].pos, _pos);  // position of light

    if (_dir)
       {
       VECTOR4D_COPY(&gLights[index].dir, _dir);  // direction of light
       // normalize it
       VECTOR4D_Normalize(&gLights[index].dir);

       } // end if

    gLights[index].spot_inner  = _spot_inner; // inner angle for spot light
    gLights[index].spot_outer  = _spot_outer; // outer angle for spot light
    gLights[index].pf          = _pf;         // power factor/falloff for spot lights

    // return light index as success
    return index;

} // end Create_Light_LIGHTV1

//////////////////////////////////////////////////////////////////////////////





