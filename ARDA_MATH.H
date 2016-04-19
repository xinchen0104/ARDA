#ifndef ARDA_MATH
#define ARDA_MATH

// DEFINES & CONSTANTS /////////////////////////////////////

// pi defines
#define PI         ((float)3.141592654f)
#define PI2        ((float)6.283185307f)
#define PI_DIV_2   ((float)1.570796327f)
#define PI_DIV_4   ((float)0.785398163f) 
#define PI_INV     ((float)0.318309886f) 

// fixed point mathematics constants
#define FIXP16_SHIFT     16
#define FIXP16_MAG       65536
#define FIXP16_DP_MASK   0x0000ffff
#define FIXP16_WP_MASK   0xffff0000
#define FIXP16_ROUND_UP  0x00008000

// defines for small numbers
#define EPSILON_E3 (float)(1E-3) 
#define EPSILON_E4 (float)(1E-4) 
#define EPSILON_E5 (float)(1E-5)
#define EPSILON_E6 (float)(1E-6)

// defines for parametric line intersections
#define PARM_LINE_NO_INTERSECT          0
#define PARM_LINE_INTERSECT_IN_SEGMENT  1
#define PARM_LINE_INTERSECT_OUT_SEGMENT 2
#define PARM_LINE_INTERSECT_EVERYWHERE  3

// deg-rad convert
#define DEG_TO_RAD(ang) ((ang)*PI/180.0)
#define RAD_TO_DEG(rads) ((rads)*180.0/PI)

// random number in a range
#define RAND_RANGE(x,y) ( (x) + (rand()%((y)-(x)+1)))

// storage for our lookup tables
extern float cos_look[361]; // 1 extra so we can store 0-360 inclusive
extern float sin_look[361]; // 1 extra so we can store 0-360 inclusive


// vector types ///////////////////////////////////////////
// 2D vector, point without the w ////////////////////////
typedef struct VECTOR2D_TYP
{
union
    {
    float M[2]; // array indexed storage

    // explicit names
    struct
         {
         float x,y;
         }; // end struct

    }; // end union

} VECTOR2D, POINT2D, *VECTOR2D_PTR, *POINT2D_PTR;

// 3D vector, point without the w ////////////////////////
typedef struct VECTOR3D_TYP
{
union
    {
    float M[3]; // array indexed storage

    // explicit names
    struct
         {
         float x,y,z;
         }; // end struct

    }; // end union

} VECTOR3D, POINT3D, *VECTOR3D_PTR, *POINT3D_PTR;

// 4D homogenous vector, point with w ////////////////////
typedef struct VECTOR4D_TYP
{
union
    {
    float M[4]; // array indexed storage

    // explicit names
    struct
         {
         float x,y,z,w;
         }; // end struct
    }; // end union

} VECTOR4D, POINT4D, *VECTOR4D_PTR, *POINT4D_PTR;
// 3x3 matrix /////////////////////////////////////////////
typedef struct MATRIX3X3_TYP
        {
        union
        {
        float M[3][3]; // array indexed data storage

        // storage in row major form with explicit names
        struct
             {
             float M00, M01, M02;
             float M10, M11, M12;
             float M20, M21, M22;
             }; // end explicit names

        }; // end union
        } MATRIX3X3, *MATRIX3X3_PTR;

// 1x3 matrix /////////////////////////////////////////////
typedef struct MATRIX1X3_TYP
        {
        union
        {
        float M[3]; // array indexed data storage

        // storage in row major form with explicit names
        struct
             {
             float M00, M01, M02;

             }; // end explicit names
        }; // end union
        } MATRIX1X3, *MATRIX1X3_PTR;

// 3x2 matrix /////////////////////////////////////////////
typedef struct MATRIX3X2_TYP
        {
        union
        {
        float M[3][2]; // array indexed data storage

        // storage in row major form with explicit names
        struct
             {
             float M00, M01;
             float M10, M11;
             float M20, M21;
             }; // end explicit names

        }; // end union

// 2x2 matrix /////////////////////////////////////////////
typedef struct MATRIX2X2_TYP
{
union
    {
    float M[2][2]; // array indexed data storage

    // storage in row major form with explicit names
    struct
         {
         float M00, M01;
         float M10, M11;
         }; // end explicit names

     }; // end union
} MATRIX2X2, *MATRIX2X2_PTR;

// 1x4 matrix /////////////////////////////////////////////
typedef struct MATRIX1X4_TYP
{
union
    {
    float M[4]; // array indexed data storage

    // storage in row major form with explicit names
    struct
         {
         float M00, M01, M02, M03;
         }; // end explicit names

     }; // end union
} MATRIX1X4, *MATRIX1X4_PTR;

// 4x4 matrix /////////////////////////////////////////////
typedef struct MATRIX4X4_TYP
{
union
    {
    float M[4][4]; // array indexed data storage

    // storage in row major form with explicit names
    struct
         {
         float M00, M01, M02, M03;
         float M10, M11, M12, M13;
         float M20, M21, M22, M23;
         float M30, M31, M32, M33;
         }; // end explicit names

    }; // end union

} MATRIX4X4, *MATRIX4X4_PTR;

// 4x3 matrix /////////////////////////////////////////////
typedef struct MATRIX4X3_TYP
{
union
    {
    float M[4][3]; // array indexed data storage

    // storage in row major form with explicit names
    struct
         {
         float M00, M01, M02;
         float M10, M11, M12;
         float M20, M21, M22;
         float M30, M31, M32;
         }; // end explicit names

    }; // end union

} MATRIX4X3, *MATRIX4X3_PTR;


// 2D parametric line /////////////////////////////////////////
typedef struct PARMLINE2D_TYP
{
POINT2D  p0; // start point of parametric line
POINT2D  p1; // end point of parametric line
VECTOR2D v;  // direction vector of line segment
             // |v|=|p0->p1|
} PARMLINE2D, *PARMLINE2D_PTR;

// 3D parametric line /////////////////////////////////////////
typedef struct PARMLINE3D_TYP
{
POINT3D  p0; // start point of parametric line
POINT3D  p1; // end point of parametric line
VECTOR3D v;  // direction vector of line segment
             // |v|=|p0->p1|
} PARMLINE3D, *PARMLINE3D_PTR;

// 3D plane ///////////////////////////////////////////////////
typedef struct PLANE3D_TYP
{
POINT3D p0; // point on the plane
VECTOR3D n; // normal to the plane (not necessarily a unit vector)
} PLANE3D, *PLANE3D_PTR;

// 2D polar coordinates ///////////////////////////////////////
typedef struct POLAR2D_TYP
{
float r;     // the radi of the point
float theta; // the angle in rads
} POLAR2D, *POLAR2D_PTR;

// 3D cylindrical coordinates ////////////////////////////////
typedef struct CYLINDRICAL3D_TYP
{
float r;     // the radi of the point
float theta; // the angle in degrees about the z axis
float z;     // the z-height of the point
} CYLINDRICAL3D, *CYLINDRICAL3D_PTR;

// 3D spherical coordinates //////////////////////////////////
typedef struct SPHERICAL3D_TYP
{
float p;      // rho, the distance to the point from the origin
float theta;  // the angle from the z-axis and the line segment o->p
float phi;    // the angle from the projection if o->p onto the x-y 
              // plane and the x-axis
} SPHERICAL3D, *SPHERICAL3D_PTR;

// 4d quaternion ////////////////////////////////////////////
// note the union gives us a number of ways to work with
// the components of the quaternion
typedef struct QUAT_TYP
{
union
    {
    float M[4]; // array indexed storage w,x,y,z order

    // vector part, real part format
    struct 
         {
         float    q0;  // the real part
         VECTOR3D qv;  // the imaginary part xi+yj+zk
         };
    struct
         {
         float w,x,y,z;
         }; 
    }; // end union

} QUAT, *QUAT_PTR;









#else
#endif
