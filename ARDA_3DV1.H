#ifndef ARDA_3D
#define ARDA_3D

// DEFINES ////////////////////////////////////////////////////

// defines for polygons and faces version 1

// attributes of polygons and polygon faces
#define POLY4DV1_ATTR_2SIDED              0x0001
#define POLY4DV1_ATTR_TRANSPARENT         0x0002


// states of polygons and faces
#define POLY4DV1_STATE_ACTIVE             0x0001
#define POLY4DV1_STATE_CLIPPED            0x0002
#define POLY4DV1_STATE_BACKFACE           0x0004

// defines for objects version 1
#define OBJECT4DV1_MAX_VERTICES           1024  // 64
#define OBJECT4DV1_MAX_POLYS              1024 // 128

// states for objects
#define OBJECT4DV1_STATE_ACTIVE           0x0001
#define OBJECT4DV1_STATE_VISIBLE          0x0002
#define OBJECT4DV1_STATE_CULLED           0x0004

// attributes for objects

// render list defines
#define RENDERLIST4DV1_MAX_POLYS          32768// 16384

// transformation control flags
#define TRANSFORM_LOCAL_ONLY       0  // perform the transformation in place on the
                                      // local/world vertex list
#define TRANSFORM_TRANS_ONLY       1  // perfrom the transformation in place on the
                                      // "transformed" vertex list
#define TRANSFORM_LOCAL_TO_TRANS   2  // perform the transformation to the local
                                      // vertex list, but store the results in the
                                      // transformed vertex list

// general culling flags
#define CULL_OBJECT_X_PLANE           0x0001 // cull on the x clipping planes
#define CULL_OBJECT_Y_PLANE           0x0002 // cull on the y clipping planes
#define CULL_OBJECT_Z_PLANE           0x0004 // cull on the z clipping planes
#define CULL_OBJECT_XYZ_PLANES        (CULL_OBJECT_X_PLANE | CULL_OBJECT_Y_PLANE | CULL_OBJECT_Z_PLANE)

// defines for camera rotation sequences
#define CAM_ROT_SEQ_XYZ  0
#define CAM_ROT_SEQ_YXZ  1
#define CAM_ROT_SEQ_XZY  2
#define CAM_ROT_SEQ_YZX  3
#define CAM_ROT_SEQ_ZYX  4
#define CAM_ROT_SEQ_ZXY  5

// defines for special types of camera projections
#define CAM_PROJ_NORMALIZED        0x0001
#define CAM_PROJ_SCREEN            0x0002
#define CAM_PROJ_FOV90             0x0004

#define CAM_MODEL_EULER            0x0008
#define CAM_MODEL_UVN              0x0010

#define UVN_MODE_SIMPLE            0
#define UVN_MODE_SPHERICAL         1

// TYPES //////////////////////////////////////////////////////

// a polygon based on an external vertex list
typedef struct POLY4DV1_TYP
{
int state;    // state information
int attr;     // physical attributes of polygon
uint32_t color;
uint32_t lit_color;    // color of polygon

POINT4D_PTR vlist; // the vertex list itself
int vert[3];       // the indices into the vertex list

} POLY4DV1, *POLY4DV1_PTR;

// a self contained polygon used for the render list
typedef struct POLYF4DV1_TYP
{
int state;    // state information
int attr;     // physical attributes of polygon
uint32_t color;
uint32_t lit_color;

POINT4D vlist[3];  // the vertices of this triangle
POINT4D tvlist[3]; // the vertices after transformation if needed

POLYF4DV1_TYP *next; // pointer to next polygon in list??
POLYF4DV1_TYP *prev; // pointer to previous polygon in list??

} POLYF4DV1, *POLYF4DV1_PTR;

// an object based on a vertex list and list of polygons
typedef struct OBJECT4DV1_TYP
{
int  id;           // numeric id of this object
char name[64];     // ASCII name of object just for kicks
int  state;        // state of object
int  attr;         // attributes of object
float avg_radius;  // average radius of object used for collision detection
float max_radius;  // maximum radius of object

POINT4D world_pos;  // position of object in world

VECTOR4D dir;       // rotation angles of object in local
                    // cords or unit direction vector user defined???

VECTOR4D ux,uy,uz;  // local axes to track full orientation
                    // this is updated automatically during
                    // rotation calls

int num_vertices;   // number of vertices of this object

POINT4D vlist_local[OBJECT4DV1_MAX_VERTICES]; // array of local vertices
POINT4D vlist_trans[OBJECT4DV1_MAX_VERTICES]; // array of transformed vertices

int num_polys;        // number of polygons in object mesh
POLY4DV1 plist[OBJECT4DV1_MAX_POLYS];  // array of polygons

} OBJECT4DV1, *OBJECT4DV1_PTR;

// camera version 1
typedef struct CAM4DV1_TYP
{
int state;      // state of camera
int attr;       // camera attributes

POINT4D pos;    // world position of camera used by both camera models

VECTOR4D dir;   // angles or look at direction of camera for simple
                // euler camera models, elevation and heading for
                // uvn model

VECTOR4D u;     // extra vectors to track the camera orientation
VECTOR4D v;     // for more complex UVN camera model
VECTOR4D n;

VECTOR4D target; // look at target

float view_dist;  // focal length

float fov;          // field of view for both horizontal and vertical axes

// 3d clipping planes
// if view volume is NOT 90 degree then general 3d clipping
// must be employed
float near_clip_z;     // near z=constant clipping plane
float far_clip_z;      // far z=constant clipping plane

PLANE3D rt_clip_plane;  // the right clipping plane
PLANE3D lt_clip_plane;  // the left clipping plane
PLANE3D tp_clip_plane;  // the top clipping plane
PLANE3D bt_clip_plane;  // the bottom clipping plane

float viewplane_width;     // width and height of view plane to project onto
float viewplane_height;    // usually 2x2 for normalized projection or
                           // the exact same size as the viewport or screen window

// remember screen and viewport are synonomous
float viewport_width;     // size of screen/viewport
float viewport_height;
float viewport_center_x;  // center of view port (final image destination)
float viewport_center_y;

// aspect ratio
float aspect_ratio;

// these matrices are not necessarily needed based on the method of
// transformation, for example, a manual perspective or screen transform
// and or a concatenated perspective/screen, however, having these
// matrices give us more flexibility

MATRIX4X4 mcam;   // storage for the world to camera transform matrix
MATRIX4X4 mper;   // storage for the camera to perspective transform matrix
MATRIX4X4 mscr;   // storage for the perspective to screen transform matrix

} CAM4DV1, *CAM4DV1_PTR;

// object to hold the render list, this way we can have more
// than one render list at a time
typedef struct RENDERLIST4DV1_TYP
{
int state; // state of renderlist ???
int attr;  // attributes of renderlist ???

// the render list is an array of pointers each pointing to
// a self contained "renderable" polygon face POLYF4DV1
POLYF4DV1_PTR poly_ptrs[RENDERLIST4DV1_MAX_POLYS];

// additionally to cut down on allocatation, de-allocation
// of polygons each frame, here's where the actual polygon
// faces will be stored
POLYF4DV1 poly_data[RENDERLIST4DV1_MAX_POLYS];

int num_polys; // number of polys in render list

} RENDERLIST4DV1, *RENDERLIST4DV1_PTR;



// PROTOTYPES /////////////////////////////////////////////////

//FUNCTIONS  //////////////////////////////////////////////////


// OBJECT4DV1

ARDA_RESULT ARDA_Reset_OBJECT4DV1(OBJECT4DV1_PTR obj);

ARDA_RESULT ARDA_Transform_OBJECT4DV1(OBJECT4DV1_PTR obj, MATRIX4X4_PTR mt, int coord_select, int transform_basis);

ARDA_RESULT ARDA_Translate_OBJECT4DV1(OBJECT4DV1_PTR obj, VECTOR4D_PTR vt);

ARDA_RESULT ARDA_Scale_OBJECT4DV1(OBJECT4DV1_PTR obj, VECTOR4D_PTR vs);

ARDA_RESULT ARDA_RotateXYZ_OBJECT4DV1(OBJECT4DV1_PTR obj,
                          float theta_x,
                          float theta_y,
                          float theta_z);

ARDA_RESULT ARDA_ModelToWorld_OBJECT4DV1(OBJECT4DV1_PTR obj, int coord_select=TRANSFORM_LOCAL_TO_TRANS);

ARDA_RESULT ARDA_WorldToCamera_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam);

ARDA_RESULT ARDA_CameraToPerspective_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam);

ARDA_RESULT ARDA_CameraToPerspectiveScreen_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam);

ARDA_RESULT ARDA_PerspectiveToScreen_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam);

int ARDA_Cull_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam, int cull_flags);

ARDA_RESULT ARDA_RemoveBackfaces_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam);

ARDA_RESULT ARDA_DrawWire_OBJECT4DV1(OBJECT4DV1_PTR obj, uint32_t *video_buffer, int lpitch);

float ARDA_ComputeRadius_OBJECT4DV1(OBJECT4DV1_PTR obj);

ARDA_RESULT ARDA_ConvertFromHomogeneous4D_OBJECT4DV1(OBJECT4DV1_PTR obj);



//MATRIX
ARDA_RESULT ARDA_BuildModelToWorld_MATRIX4X4(VECTOR4D_PTR vpos, MATRIX4X4_PTR m);

ARDA_RESULT ARDA_BuildCameraToPerspective_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m);

ARDA_RESULT ARDA_BuildPerspectiveToScreen4D_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m);

ARDA_RESULT ARDA_BuildPerspectiveToScreen_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m);

ARDA_RESULT ARDA_BuildCameraToScreen_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m);

ARDA_RESULT ARDA_BuildXYZRotation_MATRIX4X4(float theta_x, float theta_y, float theta_z,
                                     MATRIX4X4_PTR mrot);


//RENDERLIST4DV1

ARDA_RESULT ARDA_Reset_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list);

ARDA_RESULT ARDA_InsertPOLY4DV1_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,POLY4DV1_PTR poly);

ARDA_RESULT ARDA_InsertPOLYF4DV1_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, POLYF4DV1_PTR poly);

ARDA_RESULT ARDA_InsertOBJECT4DV1_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, OBJECT4DV1_PTR obj, int insert_local=0);

ARDA_RESULT ARDA_Transform_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, MATRIX4X4_PTR mt,
                              int coord_select);

ARDA_RESULT ARDA_ModelToWorld_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, POINT4D_PTR world_pos, int coord_select=TRANSFORM_LOCAL_TO_TRANS);

ARDA_RESULT ARDA_WorldToCamera_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, CAM4DV1_PTR cam);

ARDA_RESULT ARDA_CameraToPerspective_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,
                                               CAM4DV1_PTR cam);
ARDA_RESULT ARDA_PerspectiveToScreen_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,
                                               CAM4DV1_PTR cam);
ARDA_RESULT ARDA_CameraToPerspectiveScreen_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,
                                                 CAM4DV1_PTR cam);

ARDA_RESULT ARDA_RemoveBackfaces_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, CAM4DV1_PTR cam);

ARDA_RESULT ARDA_DrawWire_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, uint32_t *video_buffer, int lpitch);

ARDA_RESULT ARDA_ConvertFromHomogeneous4D_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list);

//CAM4DV1

ARDA_RESULT ARDA_Init_CAM4DV1(CAM4DV1_PTR cam, int attr, POINT4D_PTR cam_pos,
                  VECTOR4D_PTR cam_dir, VECTOR4D_PTR cam_target,
                  float near_clip_z, float far_clip_z, float fov,
                  float viewport_width,  float viewport_height);
ARDA_RESULT ARDA_BuildMatrixEuler_CAM4DV1(CAM4DV1_PTR cam, int cam_rot_seq);

ARDA_RESULT ARDA_BuildMatrixUVN_CAM4DV1(CAM4DV1_PTR cam, int mode);


#else
#endif
