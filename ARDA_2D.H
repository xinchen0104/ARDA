#ifndef ARDA_2D
#define ARDA_2D
// structs
// vertex types ///////////////////////////////////////////
// 2D vertex
typedef struct VERTEX2DI_TYP
        {
        int32_t x,y; // the vertex
        } VERTEX2DI, *VERTEX2DI_PTR;

typedef struct VERTEX2DF_TYP
        {
        float x,y; // the vertex
        } VERTEX2DF, *VERTEX2DF_PTR;
// 2D polygon
typedef struct POLYGON2D_TYP
        {
        uint32_t state;        // state of polygon
        uint32_t num_verts;    // number of vertices
        int32_t x0,y0;        // position of center of polygon
        int32_t xv,yv;        // initial velocity
        uint32_t color;      // could be index or PALETTENTRY
        VERTEX2DF *vlist; // pointer to vertex list

        } POLYGON2D, *POLYGON2D_PTR;

uint32_t ARDA_GetColor(void);
ARDA_RESULT ARDA_SetColor(uint32_t);
ARDA_RESULT ARDA_SetClipRect(SDL_Rect);
SDL_Rect ARDA_GetClipRect(void);

ARDA_RESULT ARDA_DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, uint32_t *buffer, uint32_t lpitch);
ARDA_RESULT ARDA_ClipLine(int32_t& x0, int32_t& y0, int32_t& x1, int32_t& y1, SDL_Rect* clipRect);
ARDA_RESULT ARDA_DrawClipLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, SDL_Rect* clipRect, uint32_t color, uint32_t* buffer, uint32_t lpitch);
ARDA_RESULT ARDA_HLine(int32_t x1, int32_t x2, int32_t y, uint32_t color, uint32_t* vbuffer, int lpitch);
ARDA_RESULT ARDA_VLine(int32_t y1, int32_t y2, int32_t x, uint32_t color, uint32_t* vbuffer, int lpitch);
ARDA_RESULT ARDA_DrawPolygon2D(POLYGON2D_PTR polygon, uint32_t* buffer, uint32_t liptch);

ARDA_RESULT ARDA_TranslatePolygon2D(POLYGON2D_PTR poly, int32_t dx, int32_t dy);
ARDA_RESULT ARDA_RotatePolygon2D(POLYGON2D_PTR poly, int32_t theta);
ARDA_RESULT ARDA_ScalePolygon2D(POLYGON2D_PTR poly, float sx, float sy);

ARDA_RESULT ARDA_FillTopTri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color, uint32_t* vbuffer, uint32_t lpitch);
ARDA_RESULT ARDA_FillBottomTri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color, uint32_t* vbuffer, uint32_t lpitch);
ARDA_RESULT ARDA_FillTriangle2D(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color, uint32_t* vbuffer, uint32_t lpitch);
#else
#endif
