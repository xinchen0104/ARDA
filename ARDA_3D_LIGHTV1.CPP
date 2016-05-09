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


ARDA_RESULT ARDA_DrawSolid_OBJECT4DV1(OBJECT4DV1_PTR obj,
                          uint32_t *video_buffer, int lpitch)

{
    for (int poly=0; poly < obj->num_polys; poly++)
        {
        // render this polygon if and only if it's not clipped, not culled,
        // active, and visible, note however the concecpt of "backface" is
        // irrelevant in a wire frame engine though
        if (!(obj->plist[poly].state & POLY4DV1_STATE_ACTIVE) ||
             (obj->plist[poly].state & POLY4DV1_STATE_CLIPPED ) ||
             (obj->plist[poly].state & POLY4DV1_STATE_BACKFACE) )
           continue; // move onto next poly

        // extract vertex indices into master list, rember the polygons are
        // NOT self contained, but based on the vertex list stored in the object
        // itself
        int vindex_0 = obj->plist[poly].vert[0];
        int vindex_1 = obj->plist[poly].vert[1];
        int vindex_2 = obj->plist[poly].vert[2];

        // draw the triangle
        ARDA_FilledTriangle2D(obj->vlist_trans[ vindex_0 ].x, obj->vlist_trans[ vindex_0 ].y,
                         obj->vlist_trans[ vindex_1 ].x, obj->vlist_trans[ vindex_1 ].y,
                         obj->vlist_trans[ vindex_2 ].x, obj->vlist_trans[ vindex_2 ].y,
                         obj->plist[poly].lit_color, video_buffer, lpitch);

        } // end for poly
    return 0;

} // end Draw_OBJECT4DV1_Solid

///////////////////////////////////////////////////////////////

ARDA_RESULT DrawSolid_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,
                              uint32_t *video_buffer, int lpitch)
{

    for (int poly=0; poly < rend_list->num_polys; poly++)
        {
        // render this polygon if and only if it's not clipped, not culled,
        // active, and visible, note however the concecpt of "backface" is
        // irrelevant in a wire frame engine though
        if (!(rend_list->poly_ptrs[poly]->state & POLY4DV1_STATE_ACTIVE) ||
             (rend_list->poly_ptrs[poly]->state & POLY4DV1_STATE_CLIPPED ) ||
             (rend_list->poly_ptrs[poly]->state & POLY4DV1_STATE_BACKFACE) )
           continue; // move onto next poly

        // draw the triangle
        ARDA_FilledTriangle2D(rend_list->poly_ptrs[poly]->tvlist[0].x, rend_list->poly_ptrs[poly]->tvlist[0].y,
                         rend_list->poly_ptrs[poly]->tvlist[1].x, rend_list->poly_ptrs[poly]->tvlist[1].y,
                         rend_list->poly_ptrs[poly]->tvlist[2].x, rend_list->poly_ptrs[poly]->tvlist[2].y,
                         rend_list->poly_ptrs[poly]->lit_color, video_buffer, lpitch);

        } // end for poly
    return 0;

} // end Draw_RENDERLIST4DV1_Solid

//////////////////////////////////////////////////////////////////////////////

ARDA_RESULT LightWorld_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,  // list to process
                                 CAM4DV1_PTR cam,     // camera position
                                 LIGHTV1_PTR lights,  // light list (might have more than one)
                                 int max_lights)      // maximum lights in list
{

    uint32_t r_base, g_base, b_base,  // base color being lit
             r_sum,  g_sum,  b_sum;   // sum of lighting process over all lights

    float dp,     // dot product
          dist,   // distance from light to surface
          i,      // general intensities
          nl,     // length of normal
          atten;  // attenuation computations


    // for each valid poly, light it...
    for (int poly=0; poly < rend_list->num_polys; poly++)
        {
        // acquire polygon
        POLYF4DV1_PTR curr_poly = rend_list->poly_ptrs[poly];

        // light this polygon if and only if it's not clipped, not culled,
        // active, and visible
        if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE) ||
             (curr_poly->state & POLY4DV1_STATE_CLIPPED ) ||
             (curr_poly->state & POLY4DV1_STATE_BACKFACE) )
           continue; // move onto next poly


        // we will use the transformed polygon vertex list since the backface removal
        // only makes sense at the world coord stage further of the pipeline

        // test the lighting mode of the polygon (use flat for flat, gouraud))
        if (curr_poly->attr & POLY4DV1_ATTR_SHADE_MODE_FLAT || curr_poly->attr & POLY4DV1_ATTR_SHADE_MODE_GOURAUD)
           {
           // step 1: extract the base color out in RGB mode

           _RGBAFROM32BIT(curr_poly->color, &r_base, &g_base, &b_base);

           // initialize color sum
           r_sum  = 0;
           g_sum  = 0;
           b_sum  = 0;

           // loop thru lights
           for (int curr_light = 0; curr_light < max_lights; curr_light++)
               {
               // is this light active
               if (lights[curr_light].state==LIGHTV1_STATE_OFF)
                  continue;

               // what kind of light are we dealing with
               if (lights[curr_light].attr & LIGHTV1_ATTR_AMBIENT)
                  {
                  // simply multiply each channel against the color of the
                  // polygon then divide by 256 to scale back to 0..255
                  // use a shift in real life!!! >> 8
                  r_sum+= ((lights[curr_light].c_ambient.r * r_base) / 256);
                  g_sum+= ((lights[curr_light].c_ambient.g * g_base) / 256);
                  b_sum+= ((lights[curr_light].c_ambient.b * b_base) / 256);

                  // there better only be one ambient light!

                  } // end if
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_INFINITE)
                  {
                  // infinite lighting, we need the surface normal, and the direction
                  // of the light source

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n;

                  // build u, v
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[1], &u);
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[2], &v);

                  // compute cross product
                  VECTOR4D_Cross(&u, &v, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // ok, recalling the lighting model for infinite lights
                  // I(d)dir = I0dir * Cldir
                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles

                  dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     i = dp/nl;
                     r_sum+= (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                     g_sum+= (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                     b_sum+= (lights[curr_light].c_diffuse.b * b_base * i) / 256;
                     } // end if

                  } // end if infinite light
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_POINT)
                  {
                  // perform point light computations
                  // light model for point light is once again:
                  //              I0point * Clpoint
                  //  I(d)point = ___________________
                  //              kc +  kl*d + kq*d2
                  //
                  //  Where d = |p - s|
                  // thus it's almost identical to the infinite light, but attenuates as a function
                  // of distance from the point source to the surface point being lit

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n, l;

                  // build u, v
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[1], &u);
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[2], &v);

                  // compute cross product
                  VECTOR4D_Cross(&u, &v, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // compute vector from surface to light
                  VECTOR4D_Build(&curr_poly->tvlist[0], &lights[curr_light].pos, &l);

                  // compute distance and attenuation
                  dist = VECTOR4D_Length(&l);

                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles
                  dp = VECTOR4D_Dot(&n, &l);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     atten =  (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

                     i = dp / (nl * dist * atten );

                     r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                     g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                     b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / 256;
                     } // end if

                  } // end if point
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_SPOTLIGHT1)
                  {
                  // perform spotlight/point computations simplified model that uses
                  // point light WITH a direction to simulate a spotlight
                  // light model for point light is once again:
                  //              I0point * Clpoint
                  //  I(d)point = ___________________
                  //              kc +  kl*d + kq*d2
                  //
                  //  Where d = |p - s|
                  // thus it's almost identical to the infinite light, but attenuates as a function
                  // of distance from the point source to the surface point being lit

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n, l;

                  // build u, v
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[1], &u);
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[2], &v);

                  // compute cross product (we need -n, so do vxu)
                  VECTOR4D_Cross(&v, &u, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // compute vector from surface to light
                  VECTOR4D_Build(&curr_poly->tvlist[0], &lights[curr_light].pos, &l);

                  // compute distance and attenuation
                  dist = VECTOR4D_Length(&l);

                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles

                  // note that I use the direction of the light here rather than a the vector to the light
                  // thus we are taking orientation into account which is similar to the spotlight model
                  dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     atten =  (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

                     i = dp / (nl * atten );

                     r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                     g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                     b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / 256;
                     } // end if

                  } // end if spotlight1
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_SPOTLIGHT2) // simple version
                  {
                  // perform spot light computations
                  // light model for spot light simple version is once again:
                  //         	     I0spotlight * Clspotlight * MAX( (l . s), 0)^pf
                  // I(d)spotlight = __________________________________________
                  //               		 kc + kl*d + kq*d2
                  // Where d = |p - s|, and pf = power factor

                  // thus it's almost identical to the point, but has the extra term in the numerator
                  // relating the angle between the light source and the point on the surface

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n, s;

                  // build u, v
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[1], &u);
                  VECTOR4D_Build(&curr_poly->tvlist[0], &curr_poly->tvlist[2], &v);

                  // compute cross product (v x u, to invert n)
                  VECTOR4D_Cross(&v, &u, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles
                  dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     // compute vector from light to surface (different from l which IS the light dir)
                     VECTOR4D_Build( &lights[curr_light].pos, &curr_poly->tvlist[0], &s);

                     // compute length of s (distance to light source) to normalize s for lighting calc
                     dist = VECTOR4D_Length(&s);

                     // compute spot light term (s . l)
                     float dpsl = VECTOR4D_Dot(&s, &lights[curr_light].dir)/dist;

                     // proceed only if term is positive
                     if (dpsl > 0)
                        {
                        // compute attenuation
                        atten = (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

                        // for speed reasons, pf exponents that are less that 1.0 are out of the question, and exponents
                        // must be integral
                        float dpsl_exp = dpsl;

                        // exponentiate for positive integral powers
                        for (int e_index = 1; e_index < (int)lights[curr_light].pf; e_index++)
                             dpsl_exp*=dpsl;

                        // now dpsl_exp holds (dpsl)^pf power which is of course (s . l)^pf

                        i = dp * dpsl_exp / (nl * atten );

                        r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                        g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                        b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / 256;

                        } // end if

                     } // end if

                  } // end if spot light

               } // end for light

           // make sure colors aren't out of range
           if (r_sum  > 255) r_sum = 255;
           if (g_sum  > 255) g_sum = 255;
           if (b_sum  > 255) b_sum = 255;

           // write the color over current color
           curr_poly->lit_color = _RGBA32(r_sum, g_sum, b_sum, 255);

           } // end if
        else // assume POLY4DV1_ATTR_SHADE_MODE_CONSTANT
           {
           // emmisive shading only, do nothing
           // ...
           } // end if

        } // end for poly

    // return success
    return 0;

} // end Light_RENDERLIST4DV1_World16

//////////////////////////////////////////////////////////////////////////////


ARDA_RESULT ARDA_LightWorld_OBJECT4DV1(OBJECT4DV1_PTR obj,  // object to process
                             CAM4DV1_PTR cam,     // camera position
                             LIGHTV1_PTR lights,  // light list (might have more than one)
                             int max_lights)      // maximum lights in list
{

    unsigned int r_base, g_base, b_base,  // base color being lit
                 r_sum,  g_sum,  b_sum;   // sum of lighting process over all lights

    float dp,     // dot product
          dist,   // distance from light to surface
          i,      // general intensities
          nl,     // length of normal
          atten;  // attenuation computations

    // test if the object is culled
    if (!(obj->state & OBJECT4DV1_STATE_ACTIVE) ||
         (obj->state & OBJECT4DV1_STATE_CULLED) ||
         !(obj->state & OBJECT4DV1_STATE_VISIBLE))
       return(0);

    // process each poly in mesh
    for (int poly=0; poly < obj->num_polys; poly++)
        {
        // acquire polygon
        POLY4DV1_PTR curr_poly = &obj->plist[poly];

        // is this polygon valid?
        // test this polygon if and only if it's not clipped, not culled,
        // active, and visible. Note we test for backface in the event that
        // a previous call might have already determined this, so why work
        // harder!
        if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE) ||
             (curr_poly->state & POLY4DV1_STATE_CLIPPED ) ||
             (curr_poly->state & POLY4DV1_STATE_BACKFACE) )
           continue; // move onto next poly

        // extract vertex indices into master list, rember the polygons are
        // NOT self contained, but based on the vertex list stored in the object
        // itself
        int vindex_0 = curr_poly->vert[0];
        int vindex_1 = curr_poly->vert[1];
        int vindex_2 = curr_poly->vert[2];

        // we will use the transformed polygon vertex list since the backface removal
        // only makes sense at the world coord stage further of the pipeline

        // test the lighting mode of the polygon (use flat for flat, gouraud))
        if (curr_poly->attr & POLY4DV1_ATTR_SHADE_MODE_FLAT || curr_poly->attr & POLY4DV1_ATTR_SHADE_MODE_GOURAUD)
           {
           // step 1: extract the base color out in RGB mode
            _RGBAFROM32BIT(curr_poly->color, &r_base, &g_base, &b_base);

           // initialize color sum
           r_sum  = 0;
           g_sum  = 0;
           b_sum  = 0;

           // loop thru lights
           for (int curr_light = 0; curr_light < max_lights; curr_light++)
               {
               // is this light active
               if (lights[curr_light].state==LIGHTV1_STATE_OFF)
                  continue;

               // what kind of light are we dealing with
               if (lights[curr_light].attr & LIGHTV1_ATTR_AMBIENT)
                  {
                  // simply multiply each channel against the color of the
                  // polygon then divide by 256 to scale back to 0..255
                  // use a shift in real life!!! >> 8
                  r_sum+= ((lights[curr_light].c_ambient.r * r_base) / 256);
                  g_sum+= ((lights[curr_light].c_ambient.g * g_base) / 256);
                  b_sum+= ((lights[curr_light].c_ambient.b * b_base) / 256);

                  // there better only be one ambient light!

                  } // end if
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_INFINITE)
                  {
                  // infinite lighting, we need the surface normal, and the direction
                  // of the light source

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n;

                  // build u, v
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_1 ], &u);
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_2 ], &v);

                  // compute cross product
                  VECTOR4D_Cross(&u, &v, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // ok, recalling the lighting model for infinite lights
                  // I(d)dir = I0dir * Cldir
                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles

                  dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     i = dp/nl;
                     r_sum+= (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                     g_sum+= (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                     b_sum+= (lights[curr_light].c_diffuse.b * b_base * i) / 256;
                     } // end if

                  } // end if infinite light
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_POINT)
                  {
                  // perform point light computations
                  // light model for point light is once again:
                  //              I0point * Clpoint
                  //  I(d)point = ___________________
                  //              kc +  kl*d + kq*d2
                  //
                  //  Where d = |p - s|
                  // thus it's almost identical to the infinite light, but attenuates as a function
                  // of distance from the point source to the surface point being lit

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n, l;

                  // build u, v
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_1 ], &u);
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_2 ], &v);

                  // compute cross product
                  VECTOR4D_Cross(&u, &v, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // compute vector from surface to light
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &lights[curr_light].pos, &l);

                  // compute distance and attenuation
                  dist = VECTOR4D_Length(&l);

                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles
                  dp = VECTOR4D_Dot(&n, &l);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     atten =  (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

                     i = dp / (nl * dist * atten );

                     r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                     g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                     b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / 256;
                     } // end if

                  } // end if point
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_SPOTLIGHT1)
                  {
                  // perform spotlight/point computations simplified model that uses
                  // point light WITH a direction to simulate a spotlight
                  // light model for point light is once again:
                  //              I0point * Clpoint
                  //  I(d)point = ___________________
                  //              kc +  kl*d + kq*d2
                  //
                  //  Where d = |p - s|
                  // thus it's almost identical to the infinite light, but attenuates as a function
                  // of distance from the point source to the surface point being lit

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n, l;

                  // build u, v
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_1 ], &u);
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_2 ], &v);

                  // compute cross product (we need -n, so do vxu)
                  VECTOR4D_Cross(&v, &u, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // compute vector from surface to light
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &lights[curr_light].pos, &l);

                  // compute distance and attenuation
                  dist = VECTOR4D_Length(&l);

                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles

                  // note that I use the direction of the light here rather than a the vector to the light
                  // thus we are taking orientation into account which is similar to the spotlight model
                  dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     atten =  (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

                     i = dp / (nl * atten );

                     r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                     g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                     b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / 256;
                     } // end if

                  } // end if spotlight1
               else
               if (lights[curr_light].attr & LIGHTV1_ATTR_SPOTLIGHT2) // simple version
                  {
                  // perform spot light computations
                  // light model for spot light simple version is once again:
                  //         	     I0spotlight * Clspotlight * MAX( (l . s), 0)^pf
                  // I(d)spotlight = __________________________________________
                  //               		 kc + kl*d + kq*d2
                  // Where d = |p - s|, and pf = power factor

                  // thus it's almost identical to the point, but has the extra term in the numerator
                  // relating the angle between the light source and the point on the surface

                  // we need to compute the normal of this polygon face, and recall
                  // that the vertices are in cw order, u=p0->p1, v=p0->p2, n=uxv
                  VECTOR4D u, v, n, s;

                  // build u, v
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_1 ], &u);
                  VECTOR4D_Build(&obj->vlist_trans[ vindex_0 ], &obj->vlist_trans[ vindex_2 ], &v);

                  // compute cross product (v x u, to invert n)
                  VECTOR4D_Cross(&v, &u, &n);

                  // at this point, we are almost ready, but we have to normalize the normal vector!
                  // this is a key optimization we can make later, we can pre-compute the length of all polygon
                  // normals, so this step can be optimized
                  // compute length of normal
                  nl = VECTOR4D_Length(&n);

                  // and for the diffuse model
                  // Itotald =   Rsdiffuse*Idiffuse * (n . l)
                  // so we basically need to multiple it all together
                  // notice the scaling by 128, I want to avoid floating point calculations, not because they
                  // are slower, but the conversion to and from cost cycles
                  dp = VECTOR4D_Dot(&n, &lights[curr_light].dir);

                  // only add light if dp > 0
                  if (dp > 0)
                     {
                     // compute vector from light to surface (different from l which IS the light dir)
                     VECTOR4D_Build( &lights[curr_light].pos, &obj->vlist_trans[ vindex_0 ], &s);

                     // compute length of s (distance to light source) to normalize s for lighting calc
                     dist = VECTOR4D_Length(&s);

                     // compute spot light term (s . l)
                     float dpsl = VECTOR4D_Dot(&s, &lights[curr_light].dir)/dist;

                     // proceed only if term is positive
                     if (dpsl > 0)
                        {
                        // compute attenuation
                        atten = (lights[curr_light].kc + lights[curr_light].kl*dist + lights[curr_light].kq*dist*dist);

                        // for speed reasons, pf exponents that are less that 1.0 are out of the question, and exponents
                        // must be integral
                        float dpsl_exp = dpsl;

                        // exponentiate for positive integral powers
                        for (int e_index = 1; e_index < (int)lights[curr_light].pf; e_index++)
                             dpsl_exp*=dpsl;

                        // now dpsl_exp holds (dpsl)^pf power which is of course (s . l)^pf

                        i = dp * dpsl_exp / (nl * atten );

                        r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / 256;
                        g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / 256;
                        b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / 256;

                        } // end if

                     } // end if

                  } // end if spot light

               } // end for light

           // make sure colors aren't out of range
           if (r_sum  > 255) r_sum = 255;
           if (g_sum  > 255) g_sum = 255;
           if (b_sum  > 255) b_sum = 255;

           // write the color
           curr_poly->lit_color = _RGBA32(r_sum, g_sum, b_sum, 255);

           } // end if
        else // assume POLY4DV1_ATTR_SHADE_MODE_CONSTANT
           {
           // emmisive shading only, copy base color into upper 16-bits
           // without any change
           curr_poly->lit_color = curr_poly->color;
           } // end if

        } // end for poly

    // return success
    return 0;

} // end Light_OBJECT4DV1_World16


////////////////////////////////////////////////////////////////////////////////

int Compare_AvgZ_POLYF4DV1(const void *arg1, const void *arg2)
{
// this function comapares the average z's of two polygons and is used by the
// depth sort surface ordering algorithm

float z1, z2;

POLYF4DV1_PTR poly_1, poly_2;

// dereference the poly pointers
poly_1 = *((POLYF4DV1_PTR *)(arg1));
poly_2 = *((POLYF4DV1_PTR *)(arg2));

// compute z average of each polygon
z1 = (float)0.33333*(poly_1->tvlist[0].z + poly_1->tvlist[1].z + poly_1->tvlist[2].z);

// now polygon 2
z2 = (float)0.33333*(poly_2->tvlist[0].z + poly_2->tvlist[1].z + poly_2->tvlist[2].z);

// compare z1 and z2, such that polys' will be sorted in descending Z order
if (z1 > z2)
   return(-1);
else
if (z1 < z2)
   return(1);
else
   return(0);

} // end Compare_AvgZ_POLYF4DV1

////////////////////////////////////////////////////////////////////////////////

int Compare_NearZ_POLYF4DV1(const void *arg1, const void *arg2)
{
// this function comapares the closest z's of two polygons and is used by the
// depth sort surface ordering algorithm

float z1, z2;

POLYF4DV1_PTR poly_1, poly_2;

// dereference the poly pointers
poly_1 = *((POLYF4DV1_PTR *)(arg1));
poly_2 = *((POLYF4DV1_PTR *)(arg2));

// compute the near z of each polygon
z1 = MIN(poly_1->tvlist[0].z, poly_1->tvlist[1].z);
z1 = MIN(z1, poly_1->tvlist[2].z);

z2 = MIN(poly_2->tvlist[0].z, poly_2->tvlist[1].z);
z2 = MIN(z2, poly_2->tvlist[2].z);

// compare z1 and z2, such that polys' will be sorted in descending Z order
if (z1 > z2)
   return(-1);
else
if (z1 < z2)
   return(1);
else
   return(0);

} // end Compare_NearZ_POLYF4DV1

////////////////////////////////////////////////////////////////////////////////

int Compare_FarZ_POLYF4DV1(const void *arg1, const void *arg2)
{
// this function comapares the farthest z's of two polygons and is used by the
// depth sort surface ordering algorithm

float z1, z2;

POLYF4DV1_PTR poly_1, poly_2;

// dereference the poly pointers
poly_1 = *((POLYF4DV1_PTR *)(arg1));
poly_2 = *((POLYF4DV1_PTR *)(arg2));

// compute the near z of each polygon
z1 = MAX(poly_1->tvlist[0].z, poly_1->tvlist[1].z);
z1 = MAX(z1, poly_1->tvlist[2].z);

z2 = MAX(poly_2->tvlist[0].z, poly_2->tvlist[1].z);
z2 = MAX(z2, poly_2->tvlist[2].z);

// compare z1 and z2, such that polys' will be sorted in descending Z order
if (z1 > z2)
   return(-1);
else
if (z1 < z2)
   return(1);
else
   return(0);

} // end Compare_FarZ_POLYF4DV1

///////////////////////////////////////////////////////////////////////////////

void Sort_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list, int sort_method)
{
// this function sorts the rendering list based on the polygon z-values
// the specific sorting method is controlled by sending in control flags
// #define SORT_POLYLIST_AVGZ  0 - sorts on average of all vertices
// #define SORT_POLYLIST_NEARZ 1 - sorts on closest z vertex of each poly
// #define SORT_POLYLIST_FARZ  2 - sorts on farthest z vertex of each poly

switch(sort_method)
      {
      case SORT_POLYLIST_AVGZ:  //  - sorts on average of all vertices
           {
           qsort((void *)rend_list->poly_ptrs, rend_list->num_polys, sizeof(POLYF4DV1_PTR), Compare_AvgZ_POLYF4DV1);
           } break;

      case SORT_POLYLIST_NEARZ: // - sorts on closest z vertex of each poly
           {
           qsort((void *)rend_list->poly_ptrs, rend_list->num_polys, sizeof(POLYF4DV1_PTR), Compare_NearZ_POLYF4DV1);
           } break;

      case SORT_POLYLIST_FARZ:  //  - sorts on farthest z vertex of each poly
           {
           qsort((void *)rend_list->poly_ptrs, rend_list->num_polys, sizeof(POLYF4DV1_PTR), Compare_FarZ_POLYF4DV1);
           } break;

       default: break;
       } // end switch

} // end Sort_RENDERLIST4DV1






