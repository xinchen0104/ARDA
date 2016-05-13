#ifndef ARDA_IMPORTER
#define ARDA_IMPORTER
//DEFINES
#define PLX_RGB_MASK          0x80000000   // mask to extract RGB or indexed color
#define PLX_SHADE_MODE_MASK   0x60000000   // mask to extract shading mode
#define PLX_2SIDED_MASK       0x10000000   // mask for double sided
#define PLX_COLOR_MASK        0x00ffffff   // xxxxrrrrggggbbbb, 4-bits per channel RGB
                                       // xxxxxxxxiiiiiiii, indexed mode 8-bit index

// these are the comparision flags after masking
#define PLX_COLOR_MODE_RGB_FLAG     0x80000000   // this poly uses RGB color
#define PLX_COLOR_MODE_INDEXED_FLAG 0x00000000   // this poly uses an indexed 8-bit color

// double sided flag
#define PLX_2SIDED_FLAG              0x10000000   // this poly is double sided
#define PLX_1SIDED_FLAG              0x00000000   // this poly is single sided

// shading mode of polygon
#define PLX_SHADE_MODE_PURE_FLAG      0x00000000 // this poly is a constant color
#define PLX_SHADE_MODE_CONSTANT_FLAG  0x00000000  // alias
#define PLX_SHADE_MODE_FLAT_FLAG      0x20000000  // this poly uses flat shading
#define PLX_SHADE_MODE_GOURAUD_FLAG   0x40000000  // this poly used gouraud shading
#define PLX_SHADE_MODE_PHONG_FLAG     0x60000000  // this poly uses phong shading
#define PLX_SHADE_MODE_FASTPHONG_FLAG 0x60000000  // this poly uses phong shading (alias)

//FUNCTIONS

char *Get_Line_PLG(char *buffer, int maxlength, FILE *fp);

float Compute_OBJECT4DV1_Radius(OBJECT4DV1_PTR obj);

int Load_OBJECT4DV1_PLG(OBJECT4DV1_PTR obj, char *filename, VECTOR4D_PTR scale,
                    VECTOR4D_PTR pos,  VECTOR4D_PTR rot);

#else
#endif
