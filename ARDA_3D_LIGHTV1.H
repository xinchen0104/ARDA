#ifndef ARDA_3D_LIGHTV1
#define ARDA_3D_LIGHTV1

// DEFINES //////////////////////////////////////////////////////////////////

#define POLY4DV1_ATTR_SHADE_MODE_PURE       0x0020
#define POLY4DV1_ATTR_SHADE_MODE_CONSTANT   0x0020 // (alias)
#define POLY4DV1_ATTR_SHADE_MODE_FLAT       0x0040
#define POLY4DV1_ATTR_SHADE_MODE_GOURAUD    0x0080
#define POLY4DV1_ATTR_SHADE_MODE_PHONG      0x0100
#define POLY4DV1_ATTR_SHADE_MODE_FASTPHONG  0x0100 // (alias)
#define POLY4DV1_ATTR_SHADE_MODE_TEXTURE    0x0200

#define VERTEX_FLAGS_INVERT_X               0x0001   // inverts the Z-coordinates
#define VERTEX_FLAGS_INVERT_Y               0x0002   // inverts the Z-coordinates
#define VERTEX_FLAGS_INVERT_Z               0x0004   // inverts the Z-coordinates
#define VERTEX_FLAGS_SWAP_YZ                0x0008   // transforms a RHS model to a LHS model
#define VERTEX_FLAGS_SWAP_XZ                0x0010
#define VERTEX_FLAGS_SWAP_XY                0x0020
#define VERTEX_FLAGS_INVERT_WINDING_ORDER   0x0040   // invert winding order from cw to ccw or ccw to cc


#define VERTEX_FLAGS_TRANSFORM_LOCAL        0x0200   // if file format has local transform then do it!
#define VERTEX_FLAGS_TRANSFORM_LOCAL_WORLD  0x0400  // if file format has local to world then do it!


// defines for materials, follow our polygon attributes as much as possible
#define MATERIALV1_ATTR_2SIDED                 0x0001
#define MATERIALV1_ATTR_TRANSPARENT            0x0002
#define MATERIALV1_ATTR_8BITCOLOR              0x0004
#define MATERIALV1_ATTR_RGB16                  0x0008
#define MATERIALV1_ATTR_RGB24                  0x0010

#define MATERIALV1_ATTR_SHADE_MODE_CONSTANT    0x0020
#define MATERIALV1_ATTR_SHADE_MODE_EMMISIVE    0x0020 // alias
#define MATERIALV1_ATTR_SHADE_MODE_FLAT        0x0040
#define MATERIALV1_ATTR_SHADE_MODE_GOURAUD     0x0080
#define MATERIALV1_ATTR_SHADE_MODE_FASTPHONG   0x0100
#define MATERIALV1_ATTR_SHADE_MODE_TEXTURE     0x0200

// defines for material system
#define MAX_MATERIALS                     256

// states of materials
#define MATERIALV1_STATE_ACTIVE                0x0001
#define MATERIALV1_STATE_INACTIVE              0x0001

// defines for light types
#define LIGHTV1_ATTR_AMBIENT      0x0001    // basic ambient light
#define LIGHTV1_ATTR_INFINITE     0x0002    // infinite light source
#define LIGHTV1_ATTR_DIRECTIONAL  0x0002    // infinite light source (alias)
#define LIGHTV1_ATTR_POINT        0x0004    // point light source
#define LIGHTV1_ATTR_SPOTLIGHT1   0x0008    // spotlight type 1 (simple)
#define LIGHTV1_ATTR_SPOTLIGHT2   0x0010    // spotlight type 2 (complex)

#define LIGHTV1_STATE_ON          1         // light on
#define LIGHTV1_STATE_OFF         0         // light off

#define MAX_LIGHTS                8         // good luck with 1!

// polygon sorting, and painters algorithm defines

// flags for sorting algorithm
#define SORT_POLYLIST_AVGZ  0  // sorts on average of all vertices
#define SORT_POLYLIST_NEARZ 1  // sorts on closest z vertex of each poly
#define SORT_POLYLIST_FARZ  2  // sorts on farthest z vertex of each poly


// TYPES ///////////////////////////////////////////////////////////////////

// RGB+alpha color
typedef struct RGBAV1_TYP
{
union
    {
    int rgba;                    // compressed format
    uint8_t rgba_M[4];             // array format
    struct {  uint8_t a,b,g,r;  }; // explict name format
    }; // end union

} RGBAV1, *RGBAV1_PTR;


// a first version of a "material"
typedef struct MATERIALV1_TYP
{
int state;           // state of material
int id;              // id of this material, index into material array
char name[64];       // name of material
int  attr;           // attributes, the modes for shading, constant, flat,
                     // gouraud, fast phong, environment, textured etc.
                     // and other special flags...

RGBAV1 color;            // color of material
float ka, kd, ks, power;

RGBAV1 ra, rd, rs;
char texture_file[80];   // file location of texture
SDL_Texture* texture;    // actual texture map (if any)

int   iaux1, iaux2;      // auxiliary vars for future expansion
float faux1, faux2;
void *ptr;

} MATERIALV1, *MATERIALV1_PTR;


// first light structure
typedef struct LIGHTV1_TYP
{
int state; // state of light
int id;    // id of light
int attr;  // type of light, and extra qualifiers

RGBAV1 c_ambient;   // ambient light intensity
RGBAV1 c_diffuse;   // diffuse light intensity
RGBAV1 c_specular;  // specular light intensity

POINT4D  pos;       // position of light
VECTOR4D dir;       // direction of light
float kc, kl, kq;   // attenuation factors
float spot_inner;   // inner angle for spot light
float spot_outer;   // outer angle for spot light
float pf;           // power factor/falloff for spot lights

int   iaux1, iaux2; // auxiliary vars for future expansion
float faux1, faux2;
void *ptr;

} LIGHTV1, *LIGHTV1_PTR;




// CLASSES /////////////////////////////////////////////////////////////////

// MACROS ///////////////////////////////////////////////////////////////////

// TYPES ///////////////////////////////////////////////////////////////////

// PROTOTYPES //////////////////////////////////////////////////////////////


// lighting system
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
                       float        _pf);        // power factor/falloff for spot lights

ARDA_RESULT ARDA_ResetLights_LIGHTV1(void);

// material system
ARDA_RESULT ARDA_ResetMaterials_MATERIALV1(void);


ARDA_RESULT ARDA_DrawSolid_OBJECT4DV1(OBJECT4DV1_PTR obj, uint32_t *video_buffer, int lpitch);

ARDA_RESULT ARDA_DrawSolid_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, uint32_t *video_buffer, int lpitch);

// inserts an object into renderlist with shaded color override
ARDA_RESULT ARDA_InsertOBJECT4DV1_RENDERLIST4DV1_V2(RENDERLIST4DV1_PTR rend_list,
                                      OBJECT4DV1_PTR obj,
                                      int insert_local=0,
                                      int lighting_on=0);
// light an object


ARDA_RESULT ARDA_LightWorld_OBJECT4DV1(OBJECT4DV1_PTR obj,  // object to process
                           CAM4DV1_PTR cam,     // camera position
                           LIGHTV1_PTR lights,  // light list (might have more than one)
                           int max_lights);      // maximum lights in list

// light the entire rendering list
ARDA_RESULT ARDA_LightWorld_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,  // list to process
                                 CAM4DV1_PTR cam,     // camera position
                                 LIGHTV1_PTR lights,  // light list (might have more than one)
                                 int max_lights);     // maximum lights in list


// z-sort algorithm (simple painters algorithm)
ARDA_RESULT ARDA_Sort_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, int sort_method=SORT_POLYLIST_AVGZ);

// avg z-compare
int Compare_AvgZ_POLYF4DV1(const void *arg1, const void *arg2);

// near z-compare
int Compare_NearZ_POLYF4DV1(const void *arg1, const void *arg2);

// far z-compare
int Compare_FarZ_POLYF4DV1(const void *arg1, const void *arg2);

// GLOBALS ///////////////////////////////////////////////////////////////////



#endif
