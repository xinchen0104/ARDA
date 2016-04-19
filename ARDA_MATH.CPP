#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"


// storage for our lookup tables
float cos_look[361]; // 1 extra so we can store 0-360 inclusive
float sin_look[361]; // 1 extra so we can store 0-360 inclusive

ARDA_RESULT ARDA_BuildSinCosTables(void)
{

// create sin/cos lookup table
// note the creation of one extra element; 360
// this helps with logic in using the tables

// generate the tables 0 - 360 inclusive
    for (int ang = 0; ang <= 360; ang++)
        {
        // convert ang to radians
        float theta = (float)ang*PI/(float)180;

        // insert next entry into table
        cos_look[ang] = cos(theta);
        sin_look[ang] = sin(theta);

        } // end for ang
    return 0;


} // end Build_Sin_Cos_Tables


ARDA_RESULT ARDA_MathInit(void){
    ARDA_BuildSinCosTables();
    return 0;
}
