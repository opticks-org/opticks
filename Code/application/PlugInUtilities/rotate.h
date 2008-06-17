/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 

/****************************************************************
*****************************************************************
Current Version: 3/20/1992

   rotate.h:   
     A library of routines to manipulate 4x4 transformation 
     matrices based on user specified parameters. The
     routines are defined in rotate.c
     
     The functions are:
     
*****************************************************************
     
void SetViewPointFocalPoint (double ViewMatrix[4][4],
                             double vp[3],
                             double fp[3],
                             double twist);
  -Takes a viewpoint (vp) and a focalpoint (fp) and determines
  -the corresponding view matrix. Twist is the clockwise angle 
  -of rotation around the LOS of the viewer's X-axis away from 
  -horizontal.
  
*****************************************************************
  
void Rotation (double ViewMatrix[4][4],
               int axis,
               double phi,
               double vp[3],
               double fp[3]);
  -Takes an existing view matrix (ViewMatrix), a rotation axis 
  -(0-6 corresponding to Data X, Y, Z, Viewer X, Y, Z and LOS 
  -respectively: LOS is another name for -ViewerZ), and an angle
  -of rotation as arguments, generates a rotation matrix assuming
  -that the rotation is around the viewer's origin, and then
  -postmultiplies the view matrix. The resulting matrix is the 
  -ViewMatrix after the specified rotation. The viewpoint (vp) 
  -and focalpoint (fp) are updated for the new orientation 
  -(if you are not interested in them, discard them).
  
*****************************************************************

void RotationA (double ViewMatrix[4][4],
                int coords,
                double axis[3],
                double phi,
                double vp[3],
                double fp[3]);
  -As Rotation above, but it rotates around the axis specified. The
  -possible values for coords are DATA_COORDS and VIEWER_COORDS,
  -the meanings being obvious. (DATA_COORDS and VIEWER_COORDS
  -are defined as constants below). The center of rotation is
  -still the viewer's origin (for other CORs see DataRotation
  -DataRotationA and AnyRotation below).
  
*****************************************************************

void Motion (double ViewMatrix[4][4],
             int axis,
             double dx,
             double vp[3],
             double fp[3]);
  -As for Rotation, but rather than applying a rotation to the
  -existing matrix, Motion applies a translation to it. It also
  -applies the translation to the viewpoint and focalpoint. The
  -possible axes are as for Rotation, plus HLOS, which is the
  -viewer's LOS projected onto the Z=0 plane. Motion along this
  -axis will change only the X and Y position of the viewer in
  -the data coords, it will not change Z. This is useful for
  -looking down on a surface while flying past it. (In truth,
  -you can rotate around HLOS also, if this is useful. Note that
  -if LOS = 00Z, the projection will be 000. In this case, no
  -Motion or Rotation will take place).
  
*****************************************************************

void MotionA (double ViewMatrix[4][4],
              int coords,
              double axis[3],
              double dx,
              double vp[3],
              double fp[3]);
  -As for Motion, but moves along the axis specified. coords is
  -treated as in RotationA.
  
*****************************************************************

void MakeRotationMatrix (double m[4][4], 
                         double a[3], 
                         double phi);
  -Takes a normalized axis of rotation (a) and a clockwise angle
  -of rotation (phi) and generates a rotation matrix based on
  -those parameters. The matrix is 4x4 with the fourth row and
  -column set to 0, except for the bottom right element which is
  -set to 1. I.e. the matrix is of the form:
                 a b c 0
                 e f g 0
                 i j k 0
                 0 0 0 1
                 
*****************************************************************

void SetHome (double ViewMatrix[4][4], 
              double HomeMatrix[4][4],
              double vp[3],
              double Homevp[3],
              double fp[3],
              double Homefp[3]);
  -Makes a copy of the current state (ViewMatrix -> HomeMatrix,
  -vp -> Homevp, and fp -> Homefp). Again, if you aren't 
  -concerned with maintaining vp and fp, simply give it unused
  -vectors.
  
*****************************************************************

void Home (double ViewMatrix[4][4], 
           double HomeMatrix[4][4],
           double vp[3],
           double Homevp[3],
           double fp[3],
           double Homefp[3]);
  -The inverse of SetHome - copies the saved state back to the
  -current one.
  
*****************************************************************

void GetVPFP (double ViewMatrix[4][4],
              double vp[3],
              double fp[3]);
  -Computes the viewpoint and focal point from the view matrix.
  -It maintains the magnitude of |fp-vp| if this isn't zero, 
  -otherwise it sets |fp-vp| to 1.
  
*****************************************************************

int MakeAxis (double ViewMatrix[4][4],
               int axis,
               double a[3]);
  -Takes an axis ID, specified as for Rotation, and creates a
  -vector matching that axis, and returns the coordinate system.  
  -I.e., if axis = DXAXIS, a = (1,0,0) and the return value is 
  -DATA_COORDS. If axis = LOS, a = (0,0,-1) and the return value 
  -is VIEWER_COORDS;
  
*****************************************************************

void DataToViewer (double ViewMatrix[4][4],
                   double axis[3],
                   double a[3]);
  -Takes a vector specified in data coordinates (axis) and 
  -places in vector a, the equivalent vector in viewer coords.
  -Note: axis and a may safely point to the same vector.

*****************************************************************

void ViewerToData (double ViewMatrix[4][4],
                   double axis[3],
                   double a[3])
  -The inverse of DataToViewer. Takes a viewer coord vector (axis) and
  -converts it into a data coord vector (a).

*****************************************************************

void DataToViewerMinusOrigin (double ViewMatrix[4][4],
                              double axis[3],
                              double a[3]);

void ViewerToDataMinusOrigin (double ViewMatrix[4][4],
                              double axis[3],
                              double a[3]);
  -As DataToViewer and ViewerToData, except that it subtracts
  -the position of the origin from the resulting vector.  This
  -is useful if all you care about is the direction of the
  -vector and not its position.

*****************************************************************

void InvertViewMatrix (double ViewMatrix[4][4],
                       double Inverse[4][4])
  -Computes the inverse of the ViewMatrix. This is not a general
  -matrix inversion routine. It only works on Viewing Matrices.
  
*****************************************************************

int GetRoll (double ViewMatrix[4][4],
              double *roll)
  -Computes the clockwise roll angle of the viewer around the LOS.
  -When the viewer's Z and the data's Z are parallel (or anti-parallel)
  -roll has no meaning, and the function returns a -1. Otherwise, it
  -updates the roll parameter and returns a 1.
  
*****************************************************************

void DataRotation (double ViewMatrix[4][4],
                   int axis,
                   double phi,
                   double vp[3],
                   double fp[3]);
                   
void DataRotationA (double ViewMatrix[4][4],
                    int coords,
                    double axis[3],
                    double phi,
                    double vp[3],
                    double fp[3]);
  -These two functions are similar to Rotation and RotationA, except
  -that they rotate around the data's origin rather than the viewer's
  -origin, and they reverse the sign of phi.  This has the effect of
  -rotating the data around its origin, rather than rotating the
  -viewer around his origin.
  
*****************************************************************
                     
void AnyRotation (double ViewMatrix[4][4],
                  int axis_coords,
                  double axis[3],
                  int COR_coords,
                  double COR[3],
                  double phi,
                  double vp[3],
                  double fp[3]);
  -A general rotation function. All of the other rotation functions
  -call this function. It allows the user to specify a center of
  -rotation in addition to a rotation axis.  The COR may be specified
  -in either viewer or data coords, just as the axis may. Also, the
  -axis and COR don't have to be specified in the same coords. (I.e.
  -one may be specified in viewer coords and the other in data coords
  -or both in either coordinate system). All rotation is of the 
  -viewer around the specified axis and COR. To rotate the data
  -around the specified axis and COR instead, simply change the
  -sign of phi.
    
*****************************************************************

int NormalizeViewMatrix (double Source[4][4],
                          double Dest[4][4]);
  -Attempts to normalize the rotation portion of the Source matrix.
  -It first normalizes the rows, and then normalizes the columns
  -by setting the third item in each column to sqrt (1 - a*a - b*b)
  -where a and b are the other two items in the column. If a*a + b*b
  -is greater than 1 or if any row has a magnitude of 0, it prints
  -an error message and leaves Dest unmodified, returning -1. If it
  -succeeds, it returns +1. Note: Soure and Dest may be the same 
  -matrix - the function makes an internal copy of Source which 
  -it uses, and if it succeeds in normalization, it copies its 
  -internal matrix to Dest. It leaves the fourth row untouched 
  -and sets the fourth column to 0001.

*****************************************************************

All of these routines were created by Todd A. Johnson, BSED.

*****************************************************************
****************************************************************/


/* Only include it once */
#ifndef ROTATE_HEADER_INCLUDED
#define ROTATE_HEADER_INCLUDED


/* Axis definitions */
#define DXAXIS      0   /* Data's X-Axis */
#define DYAXIS      1   /* Data's Y-Axis */
#define DZAXIS      2   /* Data's Z-Axis */
#define VXAXIS      3   /* Viewer's X-Axis */
#define VYAXIS      4   /* Viewer's Y-Axis */
#define VZAXIS      5   /* Viewer's Z-Axis */
#define LOS      6   /* Viewer's Line-Of-Sight */
#define HLOS      7   /* LOS projected onto (x,y,0) */

#define DATA_COORDS   0
#define VIEWER_COORDS   1

/* Function prototypes */
void SetViewPointFocalPoint (double ViewMatrix[4][4],
                             double vp[3],
                             double fp[3],
                             double twist);
                             
void Rotation (double ViewMatrix[4][4],
               int axis,
               double phi,
               double vp[3],
               double fp[3]);
               
void Motion (double ViewMatrix[4][4],
             int axis,
             double dx,
             double vp[3],
             double fp[3]);
             
void MakeRotationMatrix (double m[4][4], 
                         double a[3], 
                         double phi);

void SetHome (double ViewMatrix[4][4], 
              double HomeMatrix[4][4],
              double vp[3],
              double Homevp[3],
              double fp[3],
              double Homefp[3]);
              
void Home (double ViewMatrix[4][4], 
           double HomeMatrix[4][4],
           double vp[3],
           double Homevp[3],
           double fp[3],
           double Homefp[3]);
           
void GetVPFP (double ViewMatrix[4][4],
              double vp[3],
              double fp[3]);

int MakeAxis (double ViewMatrix[4][4],
               int axis,
               double a[3]);

void RotationA (double ViewMatrix[4][4],
                int coords,
                double axis[3],
                double phi,
                double vp[3],
                double fp[3]);

void MotionA (double ViewMatrix[4][4],
              int coords,
              double axis[3],
              double dx,
              double vp[3],
              double fp[3]);

void DataToViewer (double ViewMatrix[4][4],
                   double axis[3],
                   double a[3]);

void ViewerToData (double ViewMatrix[4][4],
                   double axis[3],
                   double a[3]);

void ViewerToDataMinusOrigin (double ViewMatrix[4][4],
                              double axis[3],
                              double a[3]);

void InvertViewMatrix (double ViewMatrix[4][4],
                       double Inverse[4][4]);

void DataRotation (double ViewMatrix[4][4],
                   int axis,
                   double phi,
                   double vp[3],
                   double fp[3]);
                   
void DataRotationA (double ViewMatrix[4][4],
                    int coords,
                    double axis[3],
                    double phi,
                    double vp[3],
                    double fp[3]);
                     
void AnyRotation (double ViewMatrix[4][4],
                  int axis_coords,
                  double axis[3],
                  int COR_coords,
                  double COR[3],
                  double phi,
                  double vp[3],
                  double fp[3]);

int GetRoll (double ViewMatrix[4][4],
              double *roll);

int NormalizeViewMatrix (double Source[4][4],
                          double Dest[4][4]);



#endif

 
