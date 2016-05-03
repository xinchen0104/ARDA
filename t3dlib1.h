// T3DLIB1.H - Header file for T3DLIB1.CPP game engine library

// watch for multiple inclusions
#ifndef T3DLIB1
#define T3DLIB1

// DEFINES ////////////////////////////////////////////////

// bitmap defines
#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define BITMAP_STATE_DEAD    0
#define BITMAP_STATE_ALIVE   1
#define BITMAP_STATE_DYING   2
#define BITMAP_ATTR_LOADED   128

#define BITMAP_EXTRACT_MODE_CELL  0
#define BITMAP_EXTRACT_MODE_ABS   1


// bit manipulation macros
#define SET_BIT(word,bit_flag)   ((word)=((word) | (bit_flag)))
#define RESET_BIT(word,bit_flag) ((word)=((word) & (~bit_flag)))



// container structure for bitmaps .BMP file
typedef struct BITMAP_FILE_TAG
        {
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info including the palette
        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data

        } BITMAP_FILE, *BITMAP_FILE_PTR;

// the blitter object structure BOB
// the simple bitmap image
typedef struct BITMAP_IMAGE_TYP
        {
        int state;          // state of bitmap
        int attr;           // attributes of bitmap
        int x,y;            // position of bitmap
        int width, height;  // size of bitmap
        int num_bytes;      // total bytes of bitmap
        int bpp;            // bits per pixel
        UCHAR *buffer;      // pixels of bitmap

        } BITMAP_IMAGE, *BITMAP_IMAGE_PTR;


int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client=NULL);

// general utility functions
DWORD Get_Clock(void);
DWORD Start_Clock(void);
DWORD Wait_Clock(DWORD count);

int Collision_Test(int x1, int y1, int w1, int h1,
                   int x2, int y2, int w2, int h2);

int Color_Scan(int x1, int y1, int x2, int y2,
               UCHAR scan_start, UCHAR scan_end,
               UCHAR *scan_buffer, int scan_lpitch);

int Color_Scan16(int x1, int y1, int x2, int y2,
                  USHORT scan_start, USHORT scan_end,
                  UCHAR *scan_buffer, int scan_lpitch);

int Draw_Pixel(int x, int y,int color,UCHAR *video_buffer, int lpitch);
int Draw_Rectangle(int x1, int y1, int x2, int y2, int color,LPDIRECTDRAWSURFACE7 lpdds);

void Screen_Transitions(int effect, UCHAR *vbuffer, int lpitch);
int Draw_Pixel(int x, int y,int color,UCHAR *video_buffer, int lpitch);


// simple bitmap image functions
int Create_Bitmap(BITMAP_IMAGE_PTR image, int x, int y, int width, int height, int bpp=8);
int Destroy_Bitmap(BITMAP_IMAGE_PTR image);
int Draw_Bitmap(BITMAP_IMAGE_PTR source_bitmap,UCHAR *dest_buffer, int lpitch, int transparent);
int Draw_Bitmap16(BITMAP_IMAGE_PTR source_bitmap,UCHAR *dest_buffer, int lpitch, int transparent);
int Load_Image_Bitmap(BITMAP_IMAGE_PTR image,BITMAP_FILE_PTR bitmap,int cx,int cy,int mode);
int Load_Image_Bitmap16(BITMAP_IMAGE_PTR image,BITMAP_FILE_PTR bitmap,int cx,int cy,int mode);
int Scroll_Bitmap(BITMAP_IMAGE_PTR image, int dx, int dy=0);

int Copy_Bitmap(BITMAP_IMAGE_PTR dest_bitmap, int dest_x, int dest_y,
                BITMAP_IMAGE_PTR source_bitmap, int source_x, int source_y,
                int width, int height);

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);

// bitmap file functions
int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename);
int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);

// gdi functions
int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds);
int Draw_Text_GDI(char *text, int x,int y,int color, LPDIRECTDRAWSURFACE7 lpdds);

// error functions
int Open_Error_File(char *filename, FILE *fp_override=NULL);
int Close_Error_File(void);
int Write_Error(char *string, ...);

// 2d 8-bit, 16-bit triangle rendering
void Draw_Top_Tri(int x1,int y1,int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);
void Draw_Bottom_Tri(int x1,int y1, int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Top_Tri16(int x1,int y1,int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);
void Draw_Bottom_Tri16(int x1,int y1, int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Top_TriFP(int x1,int y1,int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);
void Draw_Bottom_TriFP(int x1,int y1, int x2,int y2, int x3,int y3,int color,UCHAR *dest_buffer, int mempitch);

void Draw_Triangle_2D(int x1,int y1,int x2,int y2,int x3,int y3,
                      int color,UCHAR *dest_buffer, int mempitch);

void Draw_Triangle_2D16(int x1,int y1,int x2,int y2,int x3,int y3,
                        int color,UCHAR *dest_buffer, int mempitch);

void Draw_TriangleFP_2D(int x1,int y1,int x2,int y2,int x3,int y3,
                        int color,UCHAR *dest_buffer, int mempitch);





inline void Draw_QuadFP_2D(int x0,int y0,int x1,int y1,
                           int x2,int y2,int x3, int y3,
                           int color,UCHAR *dest_buffer, int mempitch);


// general 2D 8-bit, 16-bit polygon rendering and transforming functions
void Draw_Filled_Polygon2D(POLYGON2D_PTR poly, UCHAR *vbuffer, int mempitch);
void Draw_Filled_Polygon2D16(POLYGON2D_PTR poly, UCHAR *vbuffer, int mempitch);
int Translate_Polygon2D(POLYGON2D_PTR poly, int dx, int dy);
int Rotate_Polygon2D(POLYGON2D_PTR poly, int theta);
int Scale_Polygon2D(POLYGON2D_PTR poly, float sx, float sy);
void Build_Sin_Cos_Tables(void);
int Translate_Polygon2D_Mat(POLYGON2D_PTR poly, int dx, int dy);
int Rotate_Polygon2D_Mat(POLYGON2D_PTR poly, int theta);
int Scale_Polygon2D_Mat(POLYGON2D_PTR poly, float sx, float sy);
int Draw_Polygon2D(POLYGON2D_PTR poly, UCHAR *vbuffer, int lpitch);
int Draw_Polygon2D16(POLYGON2D_PTR poly, UCHAR *vbuffer, int lpitch);

// math functions
int Fast_Distance_2D(int x, int y);
float Fast_Distance_3D(float x, float y, float z);

// collision detection functions
int Find_Bounding_Box_Poly2D(POLYGON2D_PTR poly,
                             float &min_x, float &max_x,
                             float &min_y, float &max_y);

int Mat_Mul_1X2_3X2(MATRIX1X2_PTR ma,
                   MATRIX3X2_PTR mb,
                   MATRIX1X2_PTR mprod);

int Mat_Mul_1X3_3X3(MATRIX1X3_PTR ma,
                   MATRIX3X3_PTR mb,
                   MATRIX1X3_PTR mprod);

int Mat_Mul_3X3(MATRIX3X3_PTR ma,
               MATRIX3X3_PTR mb,
               MATRIX3X3_PTR mprod);

inline int Mat_Init_3X2(MATRIX3X2_PTR ma,
                        float m00, float m01,
                        float m10, float m11,
                        float m20, float m21);


// memory manipulation functions
inline void Mem_Set_WORD(void *dest, USHORT data, int count);
inline void Mem_Set_QUAD(void *dest, UINT   data, int count);

// INLINE FUNCTIONS //////////////////////////////////////////

inline void Mem_Set_WORD(void *dest, USHORT data, int count)
{
// this function fills or sets unsigned 16-bit aligned memory
// count is number of words

//Write_Error("{");

_asm
    {
    mov edi, dest   ; edi points to destination memory
    mov ecx, count  ; number of 16-bit words to move
    mov ax,  data   ; 16-bit data
    rep stosw       ; move data
    } // end asm

//Write_Error("}");

} // end Mem_Set_WORD

///////////////////////////////////////////////////////////

inline void Mem_Set_QUAD(void *dest, UINT data, int count)
{
// this function fills or sets unsigned 32-bit aligned memory
// count is number of quads

_asm
    {
    mov edi, dest   ; edi points to destination memory
    mov ecx, count  ; number of 32-bit words to move
    mov eax, data   ; 32-bit data
    rep stosd       ; move data
    } // end asm

} // end Mem_Set_QUAD

//////////////////////////////////////////////////////////

inline void Draw_QuadFP_2D(int x0,int y0,
                    int x1,int y1,
                    int x2,int y2,
                    int x3, int y3,
                    int color,
                    UCHAR *dest_buffer, int mempitch)
{
// this function draws a 2D quadrilateral

// simply call the triangle function 2x, let it do all the work
Draw_TriangleFP_2D(x0,y0,x1,y1,x3,y3,color,dest_buffer,mempitch);
Draw_TriangleFP_2D(x1,y1,x2,y2,x3,y3,color,dest_buffer,mempitch);

} // end Draw_QuadFP_2D

////////////////////////////////////////////////////////////

inline int Mat_Init_3X2(MATRIX3X2_PTR ma,
                        float m00, float m01,
                        float m10, float m11,
                        float m20, float m21)
{
// this function fills a 3x2 matrix with the sent data in row major form
ma->M[0][0] = m00; ma->M[0][1] = m01;
ma->M[1][0] = m10; ma->M[1][1] = m11;
ma->M[2][0] = m20; ma->M[2][1] = m21;

// return success
return(1);

} // end Mat_Init_3X2

// GLOBALS ////////////////////////////////////////////////

extern FILE *fp_error;                           // general error file
extern char error_filename[80];                  // error file name

// notice that interface 4.0 is used on a number of interfaces
extern LPDIRECTDRAW7        lpdd;                 // dd object
extern LPDIRECTDRAWSURFACE7 lpddsprimary;         // dd primary surface
extern LPDIRECTDRAWSURFACE7 lpddsback;            // dd back surface
extern LPDIRECTDRAWPALETTE  lpddpal;              // a pointer to the created dd palette
extern LPDIRECTDRAWCLIPPER  lpddclipper;          // dd clipper for back surface
extern LPDIRECTDRAWCLIPPER  lpddclipperwin;       // dd clipper for window
extern PALETTEENTRY         palette[256];         // color palette
extern PALETTEENTRY         save_palette[256];    // used to save palettes
extern DDSURFACEDESC2       ddsd;                 // a direct draw surface description struct
extern DDBLTFX              ddbltfx;              // used to fill
extern DDSCAPS2             ddscaps;              // a direct draw surface capabilities struct
extern HRESULT              ddrval;               // result back from dd calls
extern UCHAR                *primary_buffer;      // primary video buffer
extern UCHAR                *back_buffer;         // secondary back buffer
extern int                  primary_lpitch;       // memory line pitch
extern int                  back_lpitch;          // memory line pitch
extern BITMAP_FILE          bitmap8bit;           // a 8 bit bitmap file
extern BITMAP_FILE          bitmap16bit;          // a 16 bit bitmap file
extern BITMAP_FILE          bitmap24bit;          // a 24 bit bitmap file


extern DWORD                start_clock_count;    // used for timing
extern int                  windowed_mode;        // tracks if dd is windowed or not

// these defined the general clipping rectangle for software clipping
extern int min_clip_x,                             // clipping rectangle
           max_clip_x,
           min_clip_y,
           max_clip_y;

// these are overwritten globally by DD_Init()
extern int screen_width,                            // width of screen
           screen_height,                           // height of screen
           screen_bpp,                              // bits per pixel
           screen_windowed;                         // is this a windowed app?


extern int dd_pixel_format;                         // default pixel format

extern int window_client_x0;   // used to track the starting (x,y) client area for
extern int window_client_y0;   // for windowed mode directdraw operations

// storage for our lookup tables
extern float cos_look[361]; // 1 extra so we can store 0-360 inclusive
extern float sin_look[361]; // 1 extra so we can store 0-360 inclusive

// function ptr to RGB16 builder
extern USHORT (*RGB16Bit)(int r, int g, int b);

// root functions
extern USHORT RGB16Bit565(int r, int g, int b);
extern USHORT RGB16Bit555(int r, int g, int b);

// conditionally compilation for engine stats
extern int debug_total_polys_per_frame;
extern int debug_polys_lit_per_frame;
extern int debug_polys_clipped_per_frame;
extern int debug_polys_rendered_per_frame;
extern int debug_backfaces_removed_per_frame;
extern int debug_objects_removed_per_frame;
extern int debug_total_transformations_per_frame;

#endif


