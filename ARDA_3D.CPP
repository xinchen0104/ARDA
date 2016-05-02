// INCLUDES ///////////////////////////////////////////////////
#define DEBUG_ON

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "ARDA_CORE.H"
#include "ARDA_MATH.H"
#include "ARDA_2D.H"
#include "ARDA_3D.H"

// GLOBALS ////////////////////////////////////////////////////

// FUNCTIONS //////////////////////////////////////////////////

/* OBJECT4DV1 FUNCTIONS *************************************************************/

ARDA_RESULT ARDA_Reset_OBJECT4DV1(OBJECT4DV1_PTR obj)
{
    // this function resets the sent object and redies it for
    // transformations, basically just resets the culled, clipped and
    // backface flags, but here's where you would add stuff
    // to ready any object for the pipeline
    // the object is valid, let's rip it apart polygon by polygon

    // reset object's culled flag
    RESET_BIT(obj->state, OBJECT4DV1_STATE_CULLED);

    // now the clipped and backface flags for the polygons
    for (int poly = 0; poly < obj->num_polys; poly++)
        {
        // acquire polygon
        POLY4DV1_PTR curr_poly = &obj->plist[poly];

        // first is this polygon even visible?
        if (!(curr_poly->state & POLY4DV1_STATE_ACTIVE))
           continue; // move onto next poly

        // reset clipped and backface flags
        RESET_BIT(curr_poly->state, POLY4DV1_STATE_CLIPPED);
        RESET_BIT(curr_poly->state, POLY4DV1_STATE_BACKFACE);

        } // end for poly
    return 0;

} // end Reset_OBJECT4DV1


///////////////////////////////////////////////////////////

ARDA_RESULT ARDA_Transform_OBJECT4DV1(OBJECT4DV1_PTR obj, // object to transform
                          MATRIX4X4_PTR mt,   // transformation matrix
                          int coord_select,   // selects coords to transform
                          int transform_basis) // flags if vector orientation
                                               // should be transformed too
{
    // this function simply transforms all of the vertices in the local or trans
    // array by the sent matrix

    // what coordinates should be transformed?
    switch(coord_select)
          {
          case TRANSFORM_LOCAL_ONLY:
          {
          // transform each local/model vertex of the object mesh in place
          for (int vertex=0; vertex < obj->num_vertices; vertex++)
              {
              POINT4D presult; // hold result of each transformation

              // transform point
              Mat_Mul_VECTOR4D_4X4(&obj->vlist_local[vertex], mt, &presult);

              // store result back
              VECTOR4D_COPY(&obj->vlist_local[vertex], &presult);
              } // end for index
          } break;

          case TRANSFORM_TRANS_ONLY:
          {
          // transform each "transformed" vertex of the object mesh in place
          // remember, the idea of the vlist_trans[] array is to accumulate
          // transformations
          for (int vertex=0; vertex < obj->num_vertices; vertex++)
              {
              POINT4D presult; // hold result of each transformation

              // transform point
              Mat_Mul_VECTOR4D_4X4(&obj->vlist_trans[vertex], mt, &presult);

              // store result back
              VECTOR4D_COPY(&obj->vlist_trans[vertex], &presult);
              } // end for index

          } break;

          case TRANSFORM_LOCAL_TO_TRANS:
          {
          // transform each local/model vertex of the object mesh and store result
          // in "transformed" vertex list
          for (int vertex=0; vertex < obj->num_vertices; vertex++)
              {
              // transform point
              Mat_Mul_VECTOR4D_4X4(&obj->vlist_local[vertex], mt, &obj->vlist_trans[vertex]);

              } // end for index
          } break;

          default: break;

    } // end switch

    // finally, test if transform should be applied to orientation basis
    // hopefully this is a rotation, otherwise the basis will get corrupted
    if (transform_basis)
       {
           // now rotate orientation basis for object
           VECTOR4D vresult; // use to rotate each orientation vector axis

           // rotate ux of basis
           Mat_Mul_VECTOR4D_4X4(&obj->ux, mt, &vresult);
           VECTOR4D_COPY(&obj->ux, &vresult);

           // rotate uy of basis
           Mat_Mul_VECTOR4D_4X4(&obj->uy, mt, &vresult);
           VECTOR4D_COPY(&obj->uy, &vresult);

           // rotate uz of basis
           Mat_Mul_VECTOR4D_4X4(&obj->uz, mt, &vresult);
           VECTOR4D_COPY(&obj->uz, &vresult);
       } // end if
    return 0;

} // end Transform_OBJECT4DV1

//////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_Translate_OBJECT4DV1(OBJECT4DV1_PTR obj, VECTOR4D_PTR vt)
{
    // NOTE: Not matrix based
    // this function translates an object without matrices,
    // simply updates the world_pos
    VECTOR4D_Add(&obj->world_pos, vt, &obj->world_pos);
    return 0;

} // end Translate_OBJECT4DV1

/////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_Scale_OBJECT4DV1(OBJECT4DV1_PTR obj, VECTOR4D_PTR vs)
{
    // NOTE: Not matrix based
    // this function scales and object without matrices
    // modifies the object's local vertex list
    // additionally the radii is updated for the object

    // for each vertex in the mesh scale the local coordinates by
    // vs on a componentwise basis, that is, sx, sy, sz
    for (int vertex=0; vertex < obj->num_vertices; vertex++)
        {
        obj->vlist_local[vertex].x*=vs->x;
        obj->vlist_local[vertex].y*=vs->y;
        obj->vlist_local[vertex].z*=vs->z;
        // leave w unchanged, always equal to 1

        } // end for vertex

    // now since the object is scaled we have to do something with
    // the radii calculation, but we don't know how the scaling
    // factors relate to the original major axis of the object,
    // therefore for scaling factors all ==1 we will simple multiply
    // which is correct, but for scaling factors not equal to 1, we
    // must take the largest scaling factor and use it to scale the
    // radii with since it's the worst case scenario of the new max and
    // average radii

    // find max scaling factor
    float scale = MAX(vs->x, vs->y);
    scale = MAX(scale, vs->z);

    // now scale
    obj->max_radius*=scale;
    obj->avg_radius*=scale;
    return 0;

} // end Scale_OBJECT4DV1

/////////////////////////////////////////////////////////////


ARDA_RESULT ARDA_RotateXYZ_OBJECT4DV1(OBJECT4DV1_PTR obj, // object to rotate
                          float theta_x,      // euler angles
                          float theta_y,
                          float theta_z)
{
    // this function rotates and object parallel to the
    // XYZ axes in that order or a subset thereof, without
    // matrices (at least externally sent)
    // modifies the object's local vertex list
    // additionally it rotates the unit directional vectors
    // that track the objects orientation, also note that each
    // time this function is called it calls the rotation generation
    // function, this is wastefull if a number of object are being rotated
    // by the same matrix, therefore, if that's the case, then generate the
    // rotation matrix, store it, and call the general Transform_OBJECT4DV1()
    // with the matrix

    MATRIX4X4 mrot; // used to store generated rotation matrix

    // generate rotation matrix, no way to avoid rotation with a matrix
    // too much math to do manually!
    ARDA_BuildXYZRotation_MATRIX4X4(theta_x, theta_y, theta_z, &mrot);

    // now simply rotate each point of the mesh in local/model coordinates
    for (int vertex=0; vertex < obj->num_vertices; vertex++)
        {
        POINT4D presult; // hold result of each transformation

        // transform point
        Mat_Mul_VECTOR4D_4X4(&obj->vlist_local[vertex], &mrot, &presult);

        // store result back
        VECTOR4D_COPY(&obj->vlist_local[vertex], &presult);

        } // end for index

    // now rotate orientation basis for object
    VECTOR4D vresult; // use to rotate each orientation vector axis

    // rotate ux of basis
    Mat_Mul_VECTOR4D_4X4(&obj->ux, &mrot, &vresult);
    VECTOR4D_COPY(&obj->ux, &vresult);

    // rotate uy of basis
    Mat_Mul_VECTOR4D_4X4(&obj->uy, &mrot, &vresult);
    VECTOR4D_COPY(&obj->uy, &vresult);

    // rotate uz of basis
    Mat_Mul_VECTOR4D_4X4(&obj->uz, &mrot, &vresult);
    VECTOR4D_COPY(&obj->uz, &vresult);
    return 0;

} // end Rotate_XYZ_OBJECT4DV1


////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_ModelToWorld_OBJECT4DV1(OBJECT4DV1_PTR obj, int coord_select)
{
    // NOTE: Not matrix based
    // this function converts the local model coordinates of the
    // sent object into world coordinates, the results are stored
    // in the transformed vertex list (vlist_trans) within the object

    // interate thru vertex list and transform all the model/local
    // coords to world coords by translating the vertex list by
    // the amount world_pos and storing the results in vlist_trans[]

    if (coord_select == TRANSFORM_LOCAL_TO_TRANS)
    {
        for (int vertex=0; vertex < obj->num_vertices; vertex++)
            {
            // translate vertex
            VECTOR4D_Add(&obj->vlist_local[vertex], &obj->world_pos, &obj->vlist_trans[vertex]);
            } // end for vertex
    } // end if local
    else
    { // TRANSFORM_TRANS_ONLY
        for (int vertex=0; vertex < obj->num_vertices; vertex++)
            {
            // translate vertex
            VECTOR4D_Add(&obj->vlist_trans[vertex], &obj->world_pos, &obj->vlist_trans[vertex]);
            } // end for vertex
    } // end else trans
    return 0;

} // end Model_To_World_OBJECT4DV1

////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_WorldToCamera_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
    // NOTE: this is a matrix based function
    // this function transforms the world coordinates of an object
    // into camera coordinates, based on the sent camera matrix
    // but it totally disregards the polygons themselves,
    // it only works on the vertices in the vlist_trans[] list
    // this is one way to do it, you might instead transform
    // the global list of polygons in the render list since you
    // are guaranteed that those polys represent geometry that
    // has passed thru backfaces culling (if any)

    // transform each vertex in the object to camera coordinates
    // assumes the object has already been transformed to world
    // coordinates and the result is in vlist_trans[]
    for (int vertex = 0; vertex < obj->num_vertices; vertex++)
        {
        // transform the vertex by the mcam matrix within the camera
        // it better be valid!
        POINT4D presult; // hold result of each transformation

        // transform point
        Mat_Mul_VECTOR4D_4X4(&obj->vlist_trans[vertex], &cam->mcam, &presult);

        // store result back
        VECTOR4D_COPY(&obj->vlist_trans[vertex], &presult);
        } // end for vertex
    return 0;

} // end World_To_Camera_OBJECT4DV1

////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_CameraToPerspective_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
    // NOTE: this is not a matrix based function
    // this function transforms the camera coordinates of an object
    // into perspective coordinates, based on the
    // sent camera object, but it totally disregards the polygons themselves,
    // it only works on the vertices in the vlist_trans[] list
    // this is one way to do it, you might instead transform
    // the global list of polygons in the render list since you
    // are guaranteed that those polys represent geometry that
    // has passed thru backfaces culling (if any)
    // finally this function is really for experimental reasons only
    // you would probably never let an object stay intact this far down
    // the pipeline, since it's probably that there's only a single polygon
    // that is visible! But this function has to transform the whole mesh!

    // transform each vertex in the object to perspective coordinates
    // assumes the object has already been transformed to camera
    // coordinates and the result is in vlist_trans[]
    for (int vertex = 0; vertex < obj->num_vertices; vertex++)
        {
        float z = obj->vlist_trans[vertex].z;

        // transform the vertex by the view parameters in the camera
        obj->vlist_trans[vertex].x = cam->view_dist*obj->vlist_trans[vertex].x/z;
        obj->vlist_trans[vertex].y = cam->view_dist*obj->vlist_trans[vertex].y*cam->aspect_ratio/z;
        // z = z, so no change

        // not that we are NOT dividing by the homogenous w coordinate since
        // we are not using a matrix operation for this version of the function

        } // end for vertex
    return 0;

} // end Camera_To_Perspective_OBJECT4DV1

//////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_CameraToPerspectiveScreen_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
    // NOTE: this is not a matrix based function
    // this function transforms the camera coordinates of an object
    // into Screen scaled perspective coordinates, based on the
    // sent camera object, that is, view_dist_h and view_dist_v
    // should be set to cause the desired (width X height)
    // projection of the vertices, but the function totally
    // disregards the polygons themselves,
    // it only works on the vertices in the vlist_trans[] list
    // this is one way to do it, you might instead transform
    // the global list of polygons in the render list since you
    // are guaranteed that those polys represent geometry that
    // has passed thru backfaces culling (if any)
    // finally this function is really for experimental reasons only
    // you would probably never let an object stay intact this far down
    // the pipeline, since it's probably that there's only a single polygon
    // that is visible! But this function has to transform the whole mesh!
    // finally, the function also inverts the y axis, so the coordinates
    // generated from this function ARE screen coordinates and ready for
    // rendering

    float alpha = (0.5*cam->viewport_width-0.5);
    float beta  = (0.5*cam->viewport_height-0.5);

    // transform each vertex in the object to perspective screen coordinates
    // assumes the object has already been transformed to camera
    // coordinates and the result is in vlist_trans[]
    for (int vertex = 0; vertex < obj->num_vertices; vertex++)
        {
        float z = obj->vlist_trans[vertex].z;

        // transform the vertex by the view parameters in the camera
        obj->vlist_trans[vertex].x = cam->view_dist*obj->vlist_trans[vertex].x/z;
        obj->vlist_trans[vertex].y = cam->view_dist*obj->vlist_trans[vertex].y/z;
        // z = z, so no change

        // not that we are NOT dividing by the homogenous w coordinate since
        // we are not using a matrix operation for this version of the function

        // now the coordinates are in the range x:(-viewport_width/2 to viewport_width/2)
        // and y:(-viewport_height/2 to viewport_height/2), thus we need a translation and
        // since the y-axis is inverted, we need to invert y to complete the screen
        // transform:
        obj->vlist_trans[vertex].x =  obj->vlist_trans[vertex].x + alpha;
        obj->vlist_trans[vertex].y = -obj->vlist_trans[vertex].y + beta;

        } // end for vertex
    return 0;

} // end Camera_To_Perspective_Screen_OBJECT4DV1

//////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_PerspectiveToScreen_OBJECT4DV1(OBJECT4DV1_PTR obj, CAM4DV1_PTR cam)
{
    // NOTE: this is not a matrix based function
    // this function transforms the perspective coordinates of an object
    // into screen coordinates, based on the sent viewport info
    // but it totally disregards the polygons themselves,
    // it only works on the vertices in the vlist_trans[] list
    // this is one way to do it, you might instead transform
    // the global list of polygons in the render list since you
    // are guaranteed that those polys represent geometry that
    // has passed thru backfaces culling (if any)
    // finally this function is really for experimental reasons only
    // you would probably never let an object stay intact this far down
    // the pipeline, since it's probably that there's only a single polygon
    // that is visible! But this function has to transform the whole mesh!
    // this function would be called after a perspective
    // projection was performed on the object

    // transform each vertex in the object to screen coordinates
    // assumes the object has already been transformed to perspective
    // coordinates and the result is in vlist_trans[]

    float alpha = (0.5*cam->viewport_width-0.5);
    float beta  = (0.5*cam->viewport_height-0.5);

    for (int vertex = 0; vertex < obj->num_vertices; vertex++)
        {
        // assumes the vertex is in perspective normalized coords from -1 to 1
        // on each axis, simple scale them to viewport and invert y axis and project
        // to screen

        // transform the vertex by the view parameters in the camera
        obj->vlist_trans[vertex].x = alpha + alpha*obj->vlist_trans[vertex].x;
        obj->vlist_trans[vertex].y = beta  - beta *obj->vlist_trans[vertex].y;

        } // end for vertex
    return 0;

} // end Perspective_To_Screen_OBJECT4DV1

/////////////////////////////////////////////////////////////




/* MATRIX FUNCTIONS *************************************************************/
ARDA_RESULT ARDA_BuildXYZRotation_MATRIX4X4(float theta_x, // euler angles
                                  float theta_y,
                                  float theta_z,
                                  MATRIX4X4_PTR mrot) // output
{
    // this helper function takes a set if euler angles and computes
    // a rotation matrix from them, usefull for object and camera
    // work, also  we will do a little testing in the function to determine
    // the rotations that need to be performed, since there's no
    // reason to perform extra matrix multiplies if the angles are
    // zero!

    MATRIX4X4 mx, my, mz, mtmp;       // working matrices
    float sin_theta=0, cos_theta=0;   // used to initialize matrices
    int rot_seq = 0;                  // 1 for x, 2 for y, 4 for z

    // step 0: fill in with identity matrix
    MAT_IDENTITY_4X4(mrot);

    // step 1: based on zero and non-zero rotation angles, determine
    // rotation sequence
    if (fabs(theta_x) > EPSILON_E5) // x
       rot_seq = rot_seq | 1;

    if (fabs(theta_y) > EPSILON_E5) // y
       rot_seq = rot_seq | 2;

    if (fabs(theta_z) > EPSILON_E5) // z
       rot_seq = rot_seq | 4;

    // now case on sequence
    switch(rot_seq)
          {
          case 0: // no rotation
          {
          // what a waste!
          return 0;
          } break;

          case 1: // x rotation
          {
          // compute the sine and cosine of the angle
          cos_theta = Fast_Cos(theta_x);
          sin_theta = Fast_Sin(theta_x);

          // set the matrix up
          Mat_Init_4X4(&mx, 1,    0,          0,         0,
                            0,    cos_theta,  sin_theta, 0,
                            0,   -sin_theta, cos_theta, 0,
                            0,    0,          0,         1);

          // that's it, copy to output matrix
          MAT_COPY_4X4(&mx, mrot);
          return 0;

          } break;

          case 2: // y rotation
          {
          // compute the sine and cosine of the angle
          cos_theta = Fast_Cos(theta_y);
          sin_theta = Fast_Sin(theta_y);

          // set the matrix up
          Mat_Init_4X4(&my,cos_theta, 0, -sin_theta, 0,
                           0,         1,  0,         0,
                           sin_theta, 0, cos_theta,  0,
                           0,         0, 0,          1);


          // that's it, copy to output matrix
          MAT_COPY_4X4(&my, mrot);
          return 0;

          } break;

          case 3: // xy rotation
          {
          // compute the sine and cosine of the angle for x
          cos_theta = Fast_Cos(theta_x);
          sin_theta = Fast_Sin(theta_x);

          // set the matrix up
          Mat_Init_4X4(&mx, 1,    0,          0,         0,
                            0,    cos_theta,  sin_theta, 0,
                            0,   -sin_theta, cos_theta, 0,
                            0,    0,          0,         1);

          // compute the sine and cosine of the angle for y
          cos_theta = Fast_Cos(theta_y);
          sin_theta = Fast_Sin(theta_y);

          // set the matrix up
          Mat_Init_4X4(&my,cos_theta, 0, -sin_theta, 0,
                           0,         1,  0,         0,
                           sin_theta, 0, cos_theta,  0,
                           0,         0, 0,          1);

          // concatenate matrices
          Mat_Mul_4X4(&mx, &my, mrot);
          return 0;

          } break;

          case 4: // z rotation
          {
          // compute the sine and cosine of the angle
          cos_theta = Fast_Cos(theta_z);
          sin_theta = Fast_Sin(theta_z);

          // set the matrix up
          Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
                           -sin_theta, cos_theta, 0, 0,
                            0,         0,         1, 0,
                            0,         0,         0, 1);


          // that's it, copy to output matrix
          MAT_COPY_4X4(&mz, mrot);
          return 0;

          } break;

          case 5: // xz rotation
          {
          // compute the sine and cosine of the angle x
          cos_theta = Fast_Cos(theta_x);
          sin_theta = Fast_Sin(theta_x);

          // set the matrix up
          Mat_Init_4X4(&mx, 1,    0,          0,         0,
                            0,    cos_theta,  sin_theta, 0,
                            0,   -sin_theta, cos_theta, 0,
                            0,    0,          0,         1);

          // compute the sine and cosine of the angle z
          cos_theta = Fast_Cos(theta_z);
          sin_theta = Fast_Sin(theta_z);

          // set the matrix up
          Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
                           -sin_theta, cos_theta, 0, 0,
                            0,         0,         1, 0,
                            0,         0,         0, 1);

          // concatenate matrices
          Mat_Mul_4X4(&mx, &mz, mrot);
          return 0;

          } break;

          case 6: // yz rotation
          {
          // compute the sine and cosine of the angle y
          cos_theta = Fast_Cos(theta_y);
          sin_theta = Fast_Sin(theta_y);

          // set the matrix up
          Mat_Init_4X4(&my,cos_theta, 0, -sin_theta, 0,
                           0,         1,  0,         0,
                           sin_theta, 0, cos_theta,  0,
                           0,         0, 0,          1);

          // compute the sine and cosine of the angle z
          cos_theta = Fast_Cos(theta_z);
          sin_theta = Fast_Sin(theta_z);

          // set the matrix up
          Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
                           -sin_theta, cos_theta, 0, 0,
                            0,         0,         1, 0,
                            0,         0,         0, 1);

          // concatenate matrices
          Mat_Mul_4X4(&my, &mz, mrot);
          return 0;

          } break;

          case 7: // xyz rotation
          {
          // compute the sine and cosine of the angle x
          cos_theta = Fast_Cos(theta_x);
          sin_theta = Fast_Sin(theta_x);

          // set the matrix up
          Mat_Init_4X4(&mx, 1,    0,         0,         0,
                            0,    cos_theta, sin_theta, 0,
                            0,   -sin_theta, cos_theta, 0,
                            0,    0,         0,         1);

          // compute the sine and cosine of the angle y
          cos_theta = Fast_Cos(theta_y);
          sin_theta = Fast_Sin(theta_y);

          // set the matrix up
          Mat_Init_4X4(&my,cos_theta, 0, -sin_theta, 0,
                           0,         1,  0,         0,
                           sin_theta, 0,  cos_theta,  0,
                           0,         0,  0,          1);

          // compute the sine and cosine of the angle z
          cos_theta = Fast_Cos(theta_z);
          sin_theta = Fast_Sin(theta_z);

          // set the matrix up
          Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
                           -sin_theta, cos_theta, 0, 0,
                            0,         0,         1, 0,
                            0,         0,         0, 1);

          // concatenate matrices, watch order!
          Mat_Mul_4X4(&mx, &my, &mtmp);
          Mat_Mul_4X4(&mtmp, &mz, mrot);

          } break;

          default: break;

          } // end switch
    return 0;

} // end Build_XYZ_Rotation_MATRIX4X4

///////////////////////////////////////////////////////////


ARDA_RESULT ARDA_BuildModelToWorld_MATRIX4X4(VECTOR4D_PTR vpos, MATRIX4X4_PTR m)
{
    // this function builds up a general local to world
    // transformation matrix that is really nothing more than a translation
    // of the origin by the amount specified in vpos

    Mat_Init_4X4(m, 1,       0,       0,       0,
                    0,       1,       0,       0,
                    0,       0,       1,       0,
                    vpos->x, vpos->y, vpos->z, 1 );
    return 0;

} // end Build_Model_To_World_MATRIX4X4

//////////////////////////////////////////////////////////

ARDA_RESULT ARDA_BuildCameraToPerspective_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
    // this function builds up a camera to perspective transformation
    // matrix, in most cases the camera would have a 2x2 normalized
    // view plane with a 90 degree FOV, since the point of the having
    // this matrix must be to also have a perspective to screen (viewport)
    // matrix that scales the normalized coordinates, also the matrix
    // assumes that you are working in 4D homogenous coordinates and at
    // some point there will be a 4D->3D conversion, it might be immediately
    // after this transform is applied to vertices, or after the perspective
    // to screen transform

    Mat_Init_4X4(m, cam->view_dist, 0,                                0, 0,
                    0,              cam->view_dist*cam->aspect_ratio, 0, 0,
                    0,              0,                                1, 1,
                    0,              0,                                0, 0);

} // end Build_Camera_To_Perspective_MATRIX4X4

///////////////////////////////////////////////////////////

ARDA_RESULT ARDA_BuildPerspectiveToScreen4D_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
    // this function builds up a perspective to screen transformation
    // matrix, the function assumes that you want to perform the
    // transform in homogeneous coordinates and at raster time there will be
    // a 4D->3D homogenous conversion and of course only the x,y points
    // will be considered for the 2D rendering, thus you would use this
    // function's matrix is your perspective coordinates were still
    // in homgeneous form whene this matrix was applied, additionally
    // the point of this matrix to to scale and translate the perspective
    // coordinates to screen coordinates, thus the matrix is built up
    // assuming that the perspective coordinates are in normalized form for
    // a (2x2)/aspect_ratio viewplane, that is, x: -1 to 1, y:-1/aspect_ratio to 1/aspect_ratio

    float alpha = (0.5*cam->viewport_width-0.5);
    float beta  = (0.5*cam->viewport_height-0.5);

    Mat_Init_4X4(m, alpha,   0,     0,    0,
                    0,      -beta,  0,    0,
                    alpha,   beta,  1,    0,
                    0,       0,     0,    1);

} // end Build_Perspective_To_Screen_4D_MATRIX4X4()

//////////////////////////////////////////////////////////

ARDA_RESULT ARDA_BuildPerspectiveToScreen_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
    // this function builds up a perspective to screen transformation
    // matrix, the function assumes that you want to perform the
    // transform in 2D/3D coordinates, that is, you have already converted
    // the perspective coordinates from homogenous 4D to 3D before applying
    // this matrix, additionally
    // the point of this matrix to to scale and translate the perspective
    // coordinates to screen coordinates, thus the matrix is built up
    // assuming that the perspective coordinates are in normalized form for
    // a 2x2 viewplane, that is, x: -1 to 1, y:-1 to 1
    // the only difference between this function and the version that
    // assumes the coordinates are still in homogenous format is the
    // last column doesn't force w=z, in fact the z, and w results
    // are irrelevent since we assume that BEFORE this matrix is applied
    // all points are already converted from 4D->3D

    float alpha = (0.5*cam->viewport_width-0.5);
    float beta  = (0.5*cam->viewport_height-0.5);

    Mat_Init_4X4(m, alpha,   0,     0,    0,
                    0,      -beta,  0,    0,
                    alpha,   beta,  1,    0,
                    0,       0,     0,    1);

} // end Build_Perspective_To_Screen_MATRIX4X4()

///////////////////////////////////////////////////////////

ARDA_RESULT ARDA_BuildCameraToScreen_MATRIX4X4(CAM4DV1_PTR cam, MATRIX4X4_PTR m)
{
    // this function creates a single matrix that performs the
    // entire camera->perspective->screen transform, the only
    // important thing is that the camera must be created with
    // a viewplane specified to be the size of the viewport
    // furthermore, after this transform is applied the the vertex
    // must be converted from 4D homogeneous to 3D, technically
    // the z is irrelevant since the data would be used for the
    // screen, but still the division by w is needed no matter
    // what

    float alpha = (0.5*cam->viewport_width-0.5);
    float beta  = (0.5*cam->viewport_height-0.5);

    Mat_Init_4X4(m, cam->view_dist,    0,               0,    0,
                    0,                -cam->view_dist,  0,    0,
                    alpha,             beta,            1,    1,
                    0,                 0,               0,    0);

} // end Build_Camera_To_Screen_MATRIX4X4()


///////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
/* RENDERLIST4DV1 FUNCTIONS *************************************************************/

ARDA_RESULT ARDA_ModelToWorld_RENDERLIST4DV1(RENDERLIST4DV1_PTR rend_list,
                                  POINT4D_PTR world_pos,
                                  int coord_select)
{
    // NOTE: Not matrix based
    // this function converts the local model coordinates of the
    // sent render list into world coordinates, the results are stored
    // in the transformed vertex list (tvlist) within the renderlist

    // interate thru vertex list and transform all the model/local
    // coords to world coords by translating the vertex list by
    // the amount world_pos and storing the results in tvlist[]
    // is this polygon valid?

    if (coord_select == TRANSFORM_LOCAL_TO_TRANS)
       {
       for (int poly = 0; poly < rend_list->num_polys; poly++)
           {
           // acquire current polygon
           POLYF4DV1_PTR curr_poly = rend_list->poly_ptrs[poly];

           // transform this polygon if and only if it's not clipped, not culled,
           // active, and visible, note however the concept of "backface" is
           // irrelevant in a wire frame engine though
           if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) ||
               (curr_poly->state & POLY4DV1_STATE_CLIPPED ) ||
               (curr_poly->state & POLY4DV1_STATE_BACKFACE) )
              continue; // move onto next poly

       // all good, let's transform
       for (int vertex = 0; vertex < 3; vertex++)
           {
           // translate vertex
           VECTOR4D_Add(&curr_poly->vlist[vertex], world_pos, &curr_poly->tvlist[vertex]);
           } // end for vertex

       } // end for poly
    } // end if local
    else // TRANSFORM_TRANS_ONLY
    {
    for (int poly = 0; poly < rend_list->num_polys; poly++)
        {
        // acquire current polygon
        POLYF4DV1_PTR curr_poly = rend_list->poly_ptrs[poly];

       // transform this polygon if and only if it's not clipped, not culled,
       // active, and visible, note however the concept of "backface" is
       // irrelevant in a wire frame engine though
       if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV1_STATE_ACTIVE) ||
           (curr_poly->state & POLY4DV1_STATE_CLIPPED ) ||
           (curr_poly->state & POLY4DV1_STATE_BACKFACE) )
            continue; // move onto next poly

       for (int vertex = 0; vertex < 3; vertex++)
           {
           // translate vertex
           VECTOR4D_Add(&curr_poly->tvlist[vertex], world_pos, &curr_poly->tvlist[vertex]);
           } // end for vertex

        } // end for poly


    } // end else
    return 0;

} // end Model_To_World_RENDERLIST4DV1

/* CAM4DV1 FUNCTIONS ********************************************************************/


/////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_Init_CAM4DV1(CAM4DV1_PTR cam,       // the camera object
                  int cam_attr,          // attributes
                  POINT4D_PTR cam_pos,   // initial camera position
                  VECTOR4D_PTR cam_dir,  // initial camera angles
                  POINT4D_PTR cam_target, // UVN target
                  float near_clip_z,     // near and far clipping planes
                  float far_clip_z,
                  float fov,             // field of view in degrees
                  float viewport_width,  // size of final screen viewport
                  float viewport_height)
{
    // this function initializes the camera object cam, the function
    // doesn't do a lot of error checking or sanity checking since
    // I want to allow you to create projections as you wish, also
    // I tried to minimize the number of parameters the functions needs

    // first set up parms that are no brainers
    cam->attr = cam_attr;              // camera attributes

    VECTOR4D_COPY(&cam->pos, cam_pos); // positions
    VECTOR4D_COPY(&cam->dir, cam_dir); // direction vector or angles for
                                       // euler camera
    // for UVN camera
    VECTOR4D_INITXYZ(&cam->u, 1,0,0);  // set to +x
    VECTOR4D_INITXYZ(&cam->v, 0,1,0);  // set to +y
    VECTOR4D_INITXYZ(&cam->n, 0,0,1);  // set to +z

    if (cam_target!=NULL)
       VECTOR4D_COPY(&cam->target, cam_target); // UVN target
    else
       VECTOR4D_ZERO(&cam->target);

    cam->near_clip_z = near_clip_z;     // near z=constant clipping plane
    cam->far_clip_z  = far_clip_z;      // far z=constant clipping plane

    cam->viewport_width  = viewport_width;   // dimensions of viewport
    cam->viewport_height = viewport_height;

    cam->viewport_center_x = (viewport_width-1)/2; // center of viewport
    cam->viewport_center_y = (viewport_height-1)/2;

    cam->aspect_ratio = (float)viewport_width/(float)viewport_height;

    // set all camera matrices to identity matrix
    MAT_IDENTITY_4X4(&cam->mcam);
    MAT_IDENTITY_4X4(&cam->mper);
    MAT_IDENTITY_4X4(&cam->mscr);

    // set independent vars
    cam->fov              = fov;

    // set the viewplane dimensions up, they will be 2 x (2/ar)
    cam->viewplane_width  = 2.0;
    cam->viewplane_height = 2.0/cam->aspect_ratio;

    // now we know fov and we know the viewplane dimensions plug into formula and
    // solve for view distance parameters
    float tan_fov_div2 = tan(DEG_TO_RAD(fov/2));

    cam->view_dist = (0.5)*(cam->viewplane_width)/tan_fov_div2;

    // test for 90 fov first since it's easy :)
    if (fov == 90.0)
        {
          // set up the clipping planes -- easy for 90 degrees!
          POINT3D pt_origin; // point on the plane
          VECTOR3D_INITXYZ(&pt_origin,0,0,0);

          VECTOR3D vn; // normal to plane

          // right clipping plane
          VECTOR3D_INITXYZ(&vn,1,0,-1); // x=z plane
          PLANE3D_Init(&cam->rt_clip_plane, &pt_origin,  &vn, 1);

          // left clipping plane
          VECTOR3D_INITXYZ(&vn,-1,0,-1); // -x=z plane
          PLANE3D_Init(&cam->lt_clip_plane, &pt_origin,  &vn, 1);

          // top clipping plane
          VECTOR3D_INITXYZ(&vn,0,1,-1); // y=z plane
          PLANE3D_Init(&cam->tp_clip_plane, &pt_origin,  &vn, 1);

          // bottom clipping plane
          VECTOR3D_INITXYZ(&vn,0,-1,-1); // -y=z plane
          PLANE3D_Init(&cam->bt_clip_plane, &pt_origin,  &vn, 1);
        } // end if d=1
    else
        {
          // now compute clipping planes yuck!
          POINT3D pt_origin; // point on the plane
          VECTOR3D_INITXYZ(&pt_origin,0,0,0);

          VECTOR3D vn; // normal to plane

          // since we don't have a 90 fov, computing the normals
          // are a bit tricky, there are a number of geometric constructions
          // that solve the problem, but I'm going to solve for the
          // vectors that represent the 2D projections of the frustrum planes
          // on the x-z and y-z planes and then find perpendiculars to them

          // right clipping plane, check the math on graph paper
          VECTOR3D_INITXYZ(&vn,cam->view_dist,0,-cam->viewplane_width/2.0);
          PLANE3D_Init(&cam->rt_clip_plane, &pt_origin,  &vn, 1);

          // left clipping plane, we can simply reflect the right normal about
          // the z axis since the planes are symetric about the z axis
          // thus invert x only
          VECTOR3D_INITXYZ(&vn,-cam->view_dist,0,-cam->viewplane_width/2.0);
          PLANE3D_Init(&cam->lt_clip_plane, &pt_origin,  &vn, 1);

          // top clipping plane, same construction
          VECTOR3D_INITXYZ(&vn,0,cam->view_dist,-cam->viewplane_width/2.0);
          PLANE3D_Init(&cam->tp_clip_plane, &pt_origin,  &vn, 1);

          // bottom clipping plane, same inversion
          VECTOR3D_INITXYZ(&vn,0,-cam->view_dist,-cam->viewplane_width/2.0);
          PLANE3D_Init(&cam->bt_clip_plane, &pt_origin,  &vn, 1);
        } // end else
    return 0;

} // end Init_CAM4DV1

/////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_BuildMatrixEuler_CAM4DV1(CAM4DV1_PTR cam, int cam_rot_seq)
{
    // this creates a camera matrix based on Euler angles
    // and stores it in the sent camera object
    // if you recall from chapter 6 to create the camera matrix
    // we need to create a transformation matrix that looks like:

    // Mcam = mt(-1) * my(-1) * mx(-1) * mz(-1)
    // that is the inverse of the camera translation matrix mutilplied
    // by the inverses of yxz, in that order, however, the order of
    // the rotation matrices is really up to you, so we aren't going
    // to force any order, thus its programmable based on the value
    // of cam_rot_seq which can be any value CAM_ROT_SEQ_XYZ where
    // XYZ can be in any order, YXZ, ZXY, etc.

    MATRIX4X4 mt_inv,  // inverse camera translation matrix
              mx_inv,  // inverse camera x axis rotation matrix
              my_inv,  // inverse camera y axis rotation matrix
              mz_inv,  // inverse camera z axis rotation matrix
              mrot,    // concatenated inverse rotation matrices
              mtmp;    // temporary working matrix


    // step 1: create the inverse translation matrix for the camera
    // position
    Mat_Init_4X4(&mt_inv, 1,    0,     0,     0,
                          0,    1,     0,     0,
                          0,    0,     1,     0,
                          -cam->pos.x, -cam->pos.y, -cam->pos.z, 1);

    // step 2: create the inverse rotation sequence for the camera
    // rember either the transpose of the normal rotation matrix or
    // plugging negative values into each of the rotations will result
    // in an inverse matrix

    // first compute all 3 rotation matrices

    // extract out euler angles
    float theta_x = cam->dir.x;
    float theta_y = cam->dir.y;
    float theta_z = cam->dir.z;

    // compute the sine and cosine of the angle x
    float cos_theta = Fast_Cos(theta_x);  // no change since cos(-x) = cos(x)
    float sin_theta = -Fast_Sin(theta_x); // sin(-x) = -sin(x)

    // set the matrix up
    Mat_Init_4X4(&mx_inv, 1,    0,         0,         0,
                          0,    cos_theta, sin_theta, 0,
                          0,   -sin_theta, cos_theta, 0,
                          0,    0,         0,         1);

    // compute the sine and cosine of the angle y
    cos_theta = Fast_Cos(theta_y);  // no change since cos(-x) = cos(x)
    sin_theta = -Fast_Sin(theta_y); // sin(-x) = -sin(x)

    // set the matrix up
    Mat_Init_4X4(&my_inv,cos_theta, 0, -sin_theta, 0,
                         0,         1,  0,         0,
                         sin_theta, 0,  cos_theta,  0,
                         0,         0,  0,          1);

    // compute the sine and cosine of the angle z
    cos_theta = Fast_Cos(theta_z);  // no change since cos(-x) = cos(x)
    sin_theta = -Fast_Sin(theta_z); // sin(-x) = -sin(x)

    // set the matrix up
    Mat_Init_4X4(&mz_inv, cos_theta, sin_theta, 0, 0,
                         -sin_theta, cos_theta, 0, 0,
                          0,         0,         1, 0,
                          0,         0,         0, 1);

    // now compute inverse camera rotation sequence
    switch(cam_rot_seq)
          {
          case CAM_ROT_SEQ_XYZ:
          {
          Mat_Mul_4X4(&mx_inv, &my_inv, &mtmp);
          Mat_Mul_4X4(&mtmp, &mz_inv, &mrot);
          } break;

          case CAM_ROT_SEQ_YXZ:
          {
          Mat_Mul_4X4(&my_inv, &mx_inv, &mtmp);
          Mat_Mul_4X4(&mtmp, &mz_inv, &mrot);
          } break;

          case CAM_ROT_SEQ_XZY:
          {
          Mat_Mul_4X4(&mx_inv, &mz_inv, &mtmp);
          Mat_Mul_4X4(&mtmp, &my_inv, &mrot);
          } break;

          case CAM_ROT_SEQ_YZX:
          {
          Mat_Mul_4X4(&my_inv, &mz_inv, &mtmp);
          Mat_Mul_4X4(&mtmp, &mx_inv, &mrot);
          } break;

          case CAM_ROT_SEQ_ZYX:
          {
          Mat_Mul_4X4(&mz_inv, &my_inv, &mtmp);
          Mat_Mul_4X4(&mtmp, &mx_inv, &mrot);
          } break;

          case CAM_ROT_SEQ_ZXY:
          {
          Mat_Mul_4X4(&mz_inv, &mx_inv, &mtmp);
          Mat_Mul_4X4(&mtmp, &my_inv, &mrot);

          } break;

          default: break;
          } // end switch

    // now mrot holds the concatenated product of inverse rotation matrices
    // multiply the inverse translation matrix against it and store in the
    // camera objects' camera transform matrix we are done!
    Mat_Mul_4X4(&mt_inv, &mrot, &cam->mcam);
    return 0;

} // end Build_CAM4DV1_Matrix_Euler

/////////////////////////////////////////////////////////////

ARDA_RESULT ARDA_BuildMatrixUVN_CAM4DV1(CAM4DV1_PTR cam, int mode)
{
    // this creates a camera matrix based on a look at vector n,
    // look up vector v, and a look right (or left) u
    // and stores it in the sent camera object, all values are
    // extracted out of the camera object itself
    // mode selects how uvn is computed
    // UVN_MODE_SIMPLE - low level simple model, use the target and view reference point
    // UVN_MODE_SPHERICAL - spherical mode, the x,y components will be used as the
    //     elevation and heading of the view vector respectively
    //     along with the view reference point as the position
    //     as usual

    MATRIX4X4 mt_inv,  // inverse camera translation matrix
              mt_uvn;  // the final uvn matrix

    // step 1: create the inverse translation matrix for the camera
    // position
    Mat_Init_4X4(&mt_inv, 1,    0,     0,     0,
                          0,    1,     0,     0,
                          0,    0,     1,     0,
                          -cam->pos.x, -cam->pos.y, -cam->pos.z, 1);


    // step 2: determine how the target point will be computed
    if (mode == UVN_MODE_SPHERICAL)
       {
       // use spherical construction
       // target needs to be recomputed

       // extract elevation and heading
       float phi   = cam->dir.x; // elevation
       float theta = cam->dir.y; // heading

       // compute trig functions once
       float sin_phi = Fast_Sin(phi);
       float cos_phi = Fast_Cos(phi);

       float sin_theta = Fast_Sin(theta);
       float cos_theta = Fast_Cos(theta);

       // now compute the target point on a unit sphere x,y,z
       cam->target.x = -1*sin_phi*sin_theta;
       cam->target.y =  1*cos_phi;
       cam->target.z =  1*sin_phi*cos_theta;
       } // end else

    // at this point, we have the view reference point, the target and that's
    // all we need to recompute u,v,n
    // Step 1: n = <target position - view reference point>
    VECTOR4D_Build(&cam->pos, &cam->target, &cam->n);

    // Step 2: Let v = <0,1,0>
    VECTOR4D_INITXYZ(&cam->v,0,1,0);

    // Step 3: u = (v x n)
    VECTOR4D_Cross(&cam->v,&cam->n,&cam->u);

    // Step 4: v = (n x u)
    VECTOR4D_Cross(&cam->n,&cam->u,&cam->v);

    // Step 5: normalize all vectors
    VECTOR4D_Normalize(&cam->u);
    VECTOR4D_Normalize(&cam->v);
    VECTOR4D_Normalize(&cam->n);


    // build the UVN matrix by placing u,v,n as the columns of the matrix
    Mat_Init_4X4(&mt_uvn, cam->u.x,    cam->v.x,     cam->n.x,     0,
                          cam->u.y,    cam->v.y,     cam->n.y,     0,
                          cam->u.z,    cam->v.z,     cam->n.z,     0,
                          0,           0,            0,            1);

    // now multiply the translation matrix and the uvn matrix and store in the
    // final camera matrix mcam
    Mat_Mul_4X4(&mt_inv, &mt_uvn, &cam->mcam);
    return 0;

} // end Build_CAM4DV1_Matrix_UVN


