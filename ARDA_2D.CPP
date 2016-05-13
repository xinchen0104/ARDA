#include <SDL2/SDL.h>
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"
#include "ARDA_2D.H"

static uint32_t gColor;
static SDL_Rect gClipRect;
// storage for our lookup tables
extern float cos_look[361]; // 1 extra so we can store 0-360 inclusive
extern float sin_look[361]; // 1 extra so we can store 0-360 inclusive

ARDA_RESULT ARDA_SetColor(uint32_t c){
    gColor = c;
    return 0;
}
uint32_t ARDA_GetColor(void){
    return gColor;
}
ARDA_RESULT ARDA_SetClipRect(SDL_Rect cr){
    gClipRect = cr;
    return 0;
}
SDL_Rect ARDA_GetClipRect(void){
    return gClipRect;
}

//function ARDA_DrawLine
ARDA_RESULT ARDA_DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, uint32_t *buffer, uint32_t lpitch){
    int dx, dy, dx2, dy2, x_inc, y_inc, error, index;
    uint32_t* vb_start = buffer + x0 + y0 * lpitch;
    dx = x1 - x0;
    dy = y1 - y0;
    if(dx > 0){
 	x_inc = 1;
    } else {
	x_inc = -1;
	dx = -dx;
    }
    if(dy > 0){
 	y_inc = lpitch;
    } else {
	y_inc = -lpitch;
	dy = -dy;
    }
    dx2 = dx << 1;
    dy2 = dy << 1;
    if(dx >dy){
	error = dy2 - dx;
	for(index = 0; index <= dx; index++){
	    *vb_start = color;
	    if(error >= 0){
		vb_start +=  y_inc;
		error -= dx2;
	    }
	    vb_start += x_inc;
	    error += dy2;
	}
    }else{
	error = dx2 - dy;
	for(index = 0; index <= dy; index++){
	    *vb_start = color;
	    if(error >= 0){
		vb_start +=  x_inc;
		error -= dy2;
	    }
	    vb_start += y_inc;
	    error += dx2;
	}

    }
    return 0;
}

//function ARDA_ClipLine

ARDA_RESULT ARDA_HLine(int32_t x1, int32_t x2, int32_t y, uint32_t color, uint32_t* vbuffer, int lpitch)
{
    // draw a horizontal line using the memset function

    int32_t temp; // used for temporary storage during endpoint swap
    uint32_t* start_offset;
    int min_clip_x = gClipRect.x,
        min_clip_y = gClipRect.y,
        max_clip_x = gClipRect.x + gClipRect.w - 1,
        max_clip_y = gClipRect.y + gClipRect.h - 1;

    // perform trivial rejections
    if (y > max_clip_y || y < min_clip_y)
       return -1;

    // sort x1 and x2, so that x2 > x1
    if (x1>x2)
       {
       temp = x1;
       x1   = x2;
       x2   = temp;
       } // end swap

    // perform trivial rejections
    if (x1 > max_clip_x || x2 < min_clip_x)
       return -1;

    // now clip
    x1 = ((x1 < min_clip_x) ? min_clip_x : x1);
    x2 = ((x2 > max_clip_x) ? max_clip_x : x2);

    start_offset = vbuffer + (y*lpitch) + x1;

    /*for (int index=0; index<=x2-x1; index++)
    {
        // set the pixel
        *start_offset = color;

        // move downward to next line
        start_offset ++;

    }*/ // end for index
    MemSetDWord((void*)start_offset, color, x2 - x1 + 1);
    return 0;

} // end HLine

//////////////////////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_VLine(int32_t y1, int32_t y2, int32_t x, uint32_t color, uint32_t* vbuffer, int lpitch)
{
    // draw a vertical line, note that a memset function can no longer be
    // used since the pixel addresses are no longer contiguous in memory
    // note that the end points of the line must be on the screen

    uint32_t *start_offset; // starting memory offset of line
    int32_t temp;  // used for temporary storage during swap
    int min_clip_x = gClipRect.x,
        min_clip_y = gClipRect.y,
        max_clip_x = gClipRect.x + gClipRect.w - 1,
        max_clip_y = gClipRect.y + gClipRect.h - 1;


    // perform trivial rejections
    if (x > max_clip_x || x < min_clip_x)
       return -1;

    // make sure y2 > y1
    if (y1>y2)
       {
       temp = y1;
       y1   = y2;
       y2   = temp;
       } // end swap

    // perform trivial rejections
    if (y1 > max_clip_y || y2 < min_clip_y)
       return -1;

    // now clip
    y1 = ((y1 < min_clip_y) ? min_clip_y : y1);
    y2 = ((y2 > max_clip_y) ? max_clip_y : y2);

    // compute starting position
    start_offset = vbuffer + (y1*lpitch) + x;

    // draw line one pixel at a time
    for (int index=0; index<=y2-y1; index++)
        {
        // set the pixel
        *start_offset = color;

        // move downward to next line
        start_offset+=lpitch;

        } // end for index
    return 0;

} // end VLine


ARDA_RESULT ARDA_ClipLine(int32_t &x1,int32_t &y1,int32_t &x2, int32_t &y2, SDL_Rect* clipRect)
{
    // this function clips the sent line using the globally defined clipping
    // region

    // internal clipping codes
    #define CLIP_CODE_C  0x0000
    #define CLIP_CODE_N  0x0008
    #define CLIP_CODE_S  0x0004
    #define CLIP_CODE_E  0x0002
    #define CLIP_CODE_W  0x0001

    #define CLIP_CODE_NE 0x000a
    #define CLIP_CODE_SE 0x0006
    #define CLIP_CODE_NW 0x0009
    #define CLIP_CODE_SW 0x0005
    if(clipRect == NULL){
        clipRect = &gClipRect;
    }

    int32_t xc1 = x1,
        yc1 = y1,
        xc2 = x2,
        yc2 = y2;
    int min_clip_x = clipRect->x,
        min_clip_y = clipRect->y,
        max_clip_x = clipRect->x + clipRect->w - 1,
        max_clip_y = clipRect->y + clipRect->h - 1;
    clipRect = NULL;
    int p1_code=0,
        p2_code=0;

    // determine codes for p1 and p2
    if (y1 < min_clip_y)
        p1_code|=CLIP_CODE_N;
    else
    if (y1 > max_clip_y)
        p1_code|=CLIP_CODE_S;

    if (x1 < min_clip_x)
        p1_code|=CLIP_CODE_W;
    else
    if (x1 > max_clip_x)
        p1_code|=CLIP_CODE_E;

    if (y2 < min_clip_y)
        p2_code|=CLIP_CODE_N;
    else
    if (y2 > max_clip_y)
        p2_code|=CLIP_CODE_S;

    if (x2 < min_clip_x)
        p2_code|=CLIP_CODE_W;
    else
    if (x2 > max_clip_x)
        p2_code|=CLIP_CODE_E;

    // try and trivially reject
    if ((p1_code & p2_code))
        return -1;

    // test for totally visible, if so leave points untouched
    if (p1_code==0 && p2_code==0)
        return 0;

    // determine end clip point for p1
    switch(p1_code)
          {
          case CLIP_CODE_C: break;

          case CLIP_CODE_N:
               {
               yc1 = min_clip_y;
               xc1 = x1 + 0.5+(float)(min_clip_y-y1)*(x2-x1)/(y2-y1);
               } break;
          case CLIP_CODE_S:
               {
               yc1 = max_clip_y;
               xc1 = x1 + 0.5+(float)(max_clip_y-y1)*(x2-x1)/(y2-y1);
               } break;

          case CLIP_CODE_W:
               {
               xc1 = min_clip_x;
               yc1 = y1 + 0.5+(float)(min_clip_x-x1)*(y2-y1)/(x2-x1);
               } break;

          case CLIP_CODE_E:
               {
               xc1 = max_clip_x;
               yc1 = y1 + 0.5+(float)(max_clip_x-x1)*(y2-y1)/(x2-x1);
               } break;

        // these cases are more complex, must compute 2 intersections
          case CLIP_CODE_NE:
               {
               // north hline intersection
               yc1 = min_clip_y;
               xc1 = x1 + 0.5+(float)(min_clip_y-y1)*(x2-x1)/(y2-y1);

               // test if intersection is valid, of so then done, else compute next
                if (xc1 < min_clip_x || xc1 > max_clip_x)
                    {
                    // east vline intersection
                    xc1 = max_clip_x;
                    yc1 = y1 + 0.5+(float)(max_clip_x-x1)*(y2-y1)/(x2-x1);
                    } // end if

               } break;

          case CLIP_CODE_SE:
               {
               // south hline intersection
               yc1 = max_clip_y;
               xc1 = x1 + 0.5+(float)(max_clip_y-y1)*(x2-x1)/(y2-y1);

               // test if intersection is valid, of so then done, else compute next
               if (xc1 < min_clip_x || xc1 > max_clip_x)
                  {
                  // east vline intersection
                  xc1 = max_clip_x;
                  yc1 = y1 + 0.5+(float)(max_clip_x-x1)*(y2-y1)/(x2-x1);
                  } // end if

               } break;

          case CLIP_CODE_NW:
               {
               // north hline intersection
               yc1 = min_clip_y;
               xc1 = x1 + 0.5+(float)(min_clip_y-y1)*(x2-x1)/(y2-y1);

               // test if intersection is valid, of so then done, else compute next
               if (xc1 < min_clip_x || xc1 > max_clip_x)
                  {
                  xc1 = min_clip_x;
                  yc1 = y1 + 0.5+(float)(min_clip_x-x1)*(y2-y1)/(x2-x1);
                  } // end if

               } break;

          case CLIP_CODE_SW:
               {
               // south hline intersection
               yc1 = max_clip_y;
               xc1 = x1 + 0.5+(float)(max_clip_y-y1)*(x2-x1)/(y2-y1);

               // test if intersection is valid, of so then done, else compute next
               if (xc1 < min_clip_x || xc1 > max_clip_x)
                  {
                  xc1 = min_clip_x;
                  yc1 = y1 + 0.5+(float)(min_clip_x-x1)*(y2-y1)/(x2-x1);
                  } // end if

               } break;

          default:break;

          } // end switch

    // determine clip point for p2
    switch(p2_code)
          {
          case CLIP_CODE_C: break;

          case CLIP_CODE_N:
               {
               yc2 = min_clip_y;
               xc2 = x2 + 0.5+(float)(min_clip_y-y2)*(x1-x2)/(y1-y2);
               } break;

          case CLIP_CODE_S:
               {
               yc2 = max_clip_y;
               xc2 = x2 + 0.5+(float)(max_clip_y-y2)*(x1-x2)/(y1-y2);
               } break;

          case CLIP_CODE_W:
               {
               xc2 = min_clip_x;
               yc2 = y2 + 0.5+(float)(min_clip_x-x2)*(y1-y2)/(x1-x2);
               } break;

          case CLIP_CODE_E:
               {
               xc2 = max_clip_x;
               yc2 = y2 + 0.5+(float)(max_clip_x-x2)*(y1-y2)/(x1-x2);
               } break;

            // these cases are more complex, must compute 2 intersections
          case CLIP_CODE_NE:
               {
               // north hline intersection
               yc2 = min_clip_y;
               xc2 = x2 + 0.5+(float)(min_clip_y-y2)*(x1-x2)/(y1-y2);

               // test if intersection is valid, of so then done, else compute next
                if (xc2 < min_clip_x || xc2 > max_clip_x)
                    {
                    // east vline intersection
                    xc2 = max_clip_x;
                    yc2 = y2 + 0.5+(float)(max_clip_x-x2)*(y1-y2)/(x1-x2);
                    } // end if

               } break;

          case CLIP_CODE_SE:
               {
               // south hline intersection
               yc2 = max_clip_y;
               xc2 = x2 + 0.5+(float)(max_clip_y-y2)*(x1-x2)/(y1-y2);

               // test if intersection is valid, of so then done, else compute next
               if (xc2 < min_clip_x || xc2 > max_clip_x)
                  {
                  // east vline intersection
                  xc2 = max_clip_x;
                  yc2 = y2 + 0.5+(float)(max_clip_x-x2)*(y1-y2)/(x1-x2);
                  } // end if

               } break;

          case CLIP_CODE_NW:
               {
               // north hline intersection
               yc2 = min_clip_y;
               xc2 = x2 + 0.5+(float)(min_clip_y-y2)*(x1-x2)/(y1-y2);

               // test if intersection is valid, of so then done, else compute next
               if (xc2 < min_clip_x || xc2 > max_clip_x)
                  {
                  xc2 = min_clip_x;
                  yc2 = y2 + 0.5+(float)(min_clip_x-x2)*(y1-y2)/(x1-x2);
                  } // end if

               } break;

          case CLIP_CODE_SW:
               {
               // south hline intersection
               yc2 = max_clip_y;
               xc2 = x2 + 0.5+(float)(max_clip_y-y2)*(x1-x2)/(y1-y2);

               // test if intersection is valid, of so then done, else compute next
               if (xc2 < min_clip_x || xc2 > max_clip_x)
                  {
                  xc2 = min_clip_x;
                  yc2 = y2 + 0.5+(float)(min_clip_x-x2)*(y1-y2)/(x1-x2);
                  } // end if

               } break;

          default:break;

          } // end switch

    // do bounds check
    if ((xc1 < min_clip_x) || (xc1 > max_clip_x) ||
        (yc1 < min_clip_y) || (yc1 > max_clip_y) ||
        (xc2 < min_clip_x) || (xc2 > max_clip_x) ||
        (yc2 < min_clip_y) || (yc2 > max_clip_y) )
        {
        return -1;
        } // end if

    // store vars back
    x1 = xc1;
    y1 = yc1;
    x2 = xc2;
    y2 = yc2;

    return 0;

} // end Clip_Line

ARDA_RESULT ARDA_DrawClipLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, SDL_Rect* clipRect, uint32_t color, uint32_t *buffer, uint32_t lpitch){
    int32_t cxs = x0,
            cys = y0,
            cxe = x1,
            cye = y1;
    if(ARDA_ClipLine(cxs, cys, cxe, cye, clipRect) != 0){
        SDL_Log("The line is out of clip rect\n line: (%i, %i) -> (%i, %i)", x0, y0, x1, y1);
        return -1;
    }
    if(ARDA_DrawLine(cxs, cys, cxe, cye, color, buffer, lpitch) != 0){
        return -1;
    }
    SDL_Log("Draw line: (%i, %i) -> (%i, %i) color %x", x0, y0, x1, y1, color);
    return 0;

}

ARDA_RESULT ARDA_DrawPolygon2D(POLYGON2D_PTR poly, uint32_t* buffer, uint32_t lpitch){

    // test if the polygon is visible
    if (poly->state)
       {
       // loop thru and draw a line from vertices 1 to n
       uint32_t index = 0;
       for (index=0; index < poly->num_verts-1; index++)
            {
            // draw line from ith to ith+1 vertex
            ARDA_DrawClipLine(poly->vlist[index].x+poly->x0,
                           poly->vlist[index].y+poly->y0,
                           poly->vlist[index+1].x+poly->x0,
                           poly->vlist[index+1].y+poly->y0,
                           NULL,
                           poly->color,
                           buffer, lpitch);

            } // end for

           // now close up polygon
           // draw line from last vertex to 0th
           ARDA_DrawClipLine(poly->vlist[0].x+poly->x0,
                          poly->vlist[0].y+poly->y0,
                          poly->vlist[index].x+poly->x0,
                          poly->vlist[index].y+poly->y0,
                          NULL,
                          poly->color,
                          buffer, lpitch);

       // return success
       return 0;
       } // end if
    else
       return -1;

} // end Draw_Polygon2D


ARDA_RESULT ARDA_TranslatePolygon2D(POLYGON2D_PTR poly, int32_t dx, int32_t dy){
    if(!poly){
        return -1;
    }
    poly->x0 += dx;
    poly->y0 += dy;
    return 0;
}

ARDA_RESULT ARDA_RotatePolygon2D(POLYGON2D_PTR poly, int32_t theta)
{
// this function rotates the local coordinates of the polygon

// test for valid pointer
    if (!poly)
       return -1;

    theta %= 360;

    // test for negative rotation angle
     if(theta < 0)
       theta+=360;

    // loop and rotate each point, very crude, no lookup!!!
    for (uint32_t curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
        {

        // perform rotation
        float xr = (float)poly->vlist[curr_vert].x*cos_look[theta] -
                        (float)poly->vlist[curr_vert].y*sin_look[theta];

        float yr = (float)poly->vlist[curr_vert].x*sin_look[theta] +
                        (float)poly->vlist[curr_vert].y*cos_look[theta];

        // store result back
        poly->vlist[curr_vert].x = xr;
        poly->vlist[curr_vert].y = yr;

        } // end for curr_vert

    // return success
    return 0;

} // end Rotate_Polygon2D

////////////////////////////////////////////////////////

ARDA_RESULT ARDA_ScalePolygon2D(POLYGON2D_PTR poly, float sx, float sy)
{
    // this function scalesthe local coordinates of the polygon

    // test for valid pointer
    if (!poly)
       return -1;

    // loop and scale each point
    for (uint32_t curr_vert = 0; curr_vert < poly->num_verts; curr_vert++)
        {
        // scale and store result back
        poly->vlist[curr_vert].x *= sx;
        poly->vlist[curr_vert].y *= sy;

        } // end for curr_vert

    // return success
    return 0;

} // end Scale_Polygon2D

ARDA_RESULT ARDA_FilledTopTri(int32_t x1,int32_t y1,
                  int32_t x2,int32_t y2,
                  int32_t x3,int32_t y3,
                  uint32_t color,
                  uint32_t *vbuffer, uint32_t lpitch)
{
// this function draws a triangle that has a flat top

    float dx_right,    // the dx/dy ratio of the right edge of line
          dx_left,     // the dx/dy ratio of the left edge of line
          xs,xe,       // the starting and ending points of the edges
          height;      // the height of the triangle

    int temp_x,        // used during sorting as temps
        temp_y,
        right,         // used by clipping
        left;

    // destination address of next scanline
    uint32_t* dest_addr = NULL;

    int min_clip_x = gClipRect.x,
        min_clip_y = gClipRect.y,
        max_clip_x = gClipRect.x + gClipRect.w - 1,
        max_clip_y = gClipRect.y + gClipRect.h - 1;

    // test order of x1 and x2
    if (x2 < x1)
       {
       temp_x = x2;
       x2     = x1;
       x1     = temp_x;
       } // end if swap

    // compute delta's
    height = y3-y1;

    dx_left  = (x3-x1)/height;
    dx_right = (x3-x2)/height;

    // set starting points
    xs = (float)x1;
    xe = (float)x2+(float)0.5;

    // perform y clipping
    if (y1 < min_clip_y)
       {
       // compute new xs and ys
       xs = xs+dx_left*(float)(-y1+min_clip_y);
       xe = xe+dx_right*(float)(-y1+min_clip_y);

       // reset y1
       y1=min_clip_y;

       } // end if top is off screen

    if (y3>max_clip_y)
       y3=max_clip_y;

    // compute starting address in video memory
    dest_addr = vbuffer+y1*lpitch;

    // test if x clipping is needed
    if (x1>=min_clip_x && x1<=max_clip_x &&
        x2>=min_clip_x && x2<=max_clip_x &&
        x3>=min_clip_x && x3<=max_clip_x)
        {
        // draw the triangle
        for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=lpitch)
            {
            MemSetDWord((void*)(dest_addr + (uint32_t)xs) , color, (uint32_t)((int)xe-(int)xs+1));
            // adjust starting point and ending point
            xs+=dx_left;
            xe+=dx_right;

            } // end for

        } // end if no x clipping needed
    else
       {
       // clip x axis with slower version

       // draw the triangle
       for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=lpitch)
           {
           // do x clip
           left  = (int)xs;
           right = (int)xe;

           // clip line
           if (left < min_clip_x)
              {
              left = min_clip_x;

              if (right < min_clip_x)
                 continue;
              }

           if (right > max_clip_x)
              {
              right = max_clip_x;

              if (left > max_clip_x)
                 continue;
              }
            MemSetDWord((void*)(dest_addr + (uint32_t)left), color, (unsigned int)(right-left+1));
            // adjust starting point and ending point
            xs+=dx_left;
            xe+=dx_right;


           } // end for

       } // end else x clipping needed
       return 0;

} // end Draw_Top_Tri

/////////////////////////////////////////////////////////////////////////////
ARDA_RESULT ARDA_FilledBottomTri(int32_t x1,int32_t y1,
                  int32_t x2,int32_t y2,
                  int32_t x3,int32_t y3,
                  uint32_t color,
                  uint32_t *vbuffer, uint32_t lpitch)
{
// this function draws a triangle that has a flat bottom

    float dx_right,    // the dx/dy ratio of the right edge of line
          dx_left,     // the dx/dy ratio of the left edge of line
          xs,xe,       // the starting and ending points of the edges
          height;      // the height of the triangle

    int temp_x,        // used during sorting as temps
        temp_y,
        right,         // used by clipping
        left;


    int min_clip_x = gClipRect.x,
        min_clip_y = gClipRect.y,
        max_clip_x = gClipRect.x + gClipRect.w - 1,
        max_clip_y = gClipRect.y + gClipRect.h - 1;

    // destination address of next scanline
    uint32_t* dest_addr;

    // test order of x1 and x2
    if (x3 < x2)
       {
       temp_x = x2;
       x2     = x3;
       x3     = temp_x;
       } // end if swap

    // compute delta's
    height = y3-y1;

    dx_left  = (x2-x1)/height;
    dx_right = (x3-x1)/height;

    // set starting points
    xs = (float)x1;
    xe = (float)x1; // +(float)0.5;

    // perform y clipping
    if (y1<min_clip_y)
       {
       // compute new xs and ys
       xs = xs+dx_left*(float)(-y1+min_clip_y);
       xe = xe+dx_right*(float)(-y1+min_clip_y);

       // reset y1
       y1=min_clip_y;

       } // end if top is off screen

    if (y3>max_clip_y)
       y3=max_clip_y;

    // compute starting address in video memory
    dest_addr = vbuffer+y1*lpitch;

    // test if x clipping is needed
    if (x1>=min_clip_x && x1<=max_clip_x &&
        x2>=min_clip_x && x2<=max_clip_x &&
        x3>=min_clip_x && x3<=max_clip_x)
        {
        // draw the triangle
        for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=lpitch)
            {
            MemSetDWord((void*)(dest_addr+(uint32_t)xs), color,(uint32_t)((int)xe-(int)xs+1));

            // adjust starting point and ending point
            xs+=dx_left;
            xe+=dx_right;

            } // end for

        } // end if no x clipping needed
    else
       {
       // clip x axis with slower version

       // draw the triangle

       for (temp_y=y1; temp_y<=y3; temp_y++,dest_addr+=lpitch)
           {
           // do x clip
           left  = (int)xs;
           right = (int)xe;

           // adjust starting point and ending point
           xs+=dx_left;
           xe+=dx_right;

           // clip line
           if (left < min_clip_x)
              {
              left = min_clip_x;

              if (right < min_clip_x)
                 continue;
              }

           if (right > max_clip_x)
              {
              right = max_clip_x;

              if (left > max_clip_x)
                 continue;
              }

           MemSetDWord((void*)(dest_addr+(uint32_t)left), color, (uint32_t)(right-left+1));

           } // end for

       } // end else x clipping needed
       return 0;

} // end Draw_Bottom_Tri
ARDA_RESULT ARDA_FilledTriangle2D(int32_t x1, int32_t y1,
                      int32_t x2, int32_t y2,
                      int32_t x3, int32_t y3,
                      uint32_t color,
					  uint32_t* vbuffer, uint32_t lpitch)
{
    // this function draws a triangle on the destination buffer
    // it decomposes all triangles into a pair of flat top, flat bottom

    int temp_x, // used for sorting
        temp_y,
        new_x;

    int min_clip_x = gClipRect.x,
        min_clip_y = gClipRect.y,
        max_clip_x = gClipRect.x + gClipRect.w - 1,
        max_clip_y = gClipRect.y + gClipRect.h - 1;

    #ifdef DEBUG_ON
        // track rendering stats
        debug_polys_rendered_per_frame++;
    #endif


    // test for h lines and v lines
    if ((x1==x2 && x2==x3)  ||  (y1==y2 && y2==y3))
       return -1;

    // sort p1,p2,p3 in ascending y order
    if (y2<y1)
       {
       temp_x = x2;
       temp_y = y2;
       x2     = x1;
       y2     = y1;
       x1     = temp_x;
       y1     = temp_y;
       } // end if

    // now we know that p1 and p2 are in order
    if (y3<y1)
       {
       temp_x = x3;
       temp_y = y3;
       x3     = x1;
       y3     = y1;
       x1     = temp_x;
       y1     = temp_y;
       } // end if

    // finally test y3 against y2
    if (y3<y2)
       {
       temp_x = x3;
       temp_y = y3;
       x3     = x2;
       y3     = y2;
       x2     = temp_x;
       y2     = temp_y;

       } // end if

    // do trivial rejection tests for clipping
    if ( y3<min_clip_y || y1>max_clip_y ||
        (x1<min_clip_x && x2<min_clip_x && x3<min_clip_x) ||
        (x1>max_clip_x && x2>max_clip_x && x3>max_clip_x) )
       return -1;

    // test if top of triangle is flat
    if (y1==y2)
       {
       ARDA_FilledTopTri(x1,y1,x2,y2,x3,y3,color,vbuffer,lpitch);
       } // end if
    else
    if (y2==y3)
       {
       ARDA_FilledBottomTri(x1,y1,x2,y2,x3,y3,color,vbuffer,lpitch);
       } // end if bottom is flat
    else
       {
       // general triangle that's needs to be broken up along long edge
       new_x = x1 + (int)(0.5+(float)(y2-y1)*(float)(x3-x1)/(float)(y3-y1));

       // draw each sub-triangle
       ARDA_FilledBottomTri(x1,y1,new_x,y2,x2,y2,color,vbuffer,lpitch);
       ARDA_FilledTopTri(x2,y2,new_x,y2,x3,y3,color,vbuffer,lpitch);

       } // end else
    return 0;

} // end Draw_Triangle_2D


