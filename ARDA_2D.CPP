#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "ARDA_2D.H"

static uint32_t gColor;
static SDL_Rect gClipRect;
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
        return(0);

    // test for totally visible, if so leave points untouched
    if (p1_code==0 && p2_code==0)
        return(1);

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
        return(0);
        } // end if

    // store vars back
    x1 = xc1;
    y1 = yc1;
    x2 = xc2;
    y2 = yc2;

    return(1);

} // end Clip_Line

