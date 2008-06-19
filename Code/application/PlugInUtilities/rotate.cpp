/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 

/****************************************************************
Current Version: 3/20/1992

  rotate.c:
    See rotate.h for documentation
    
  Created by Todd A. Johnson, BSED

****************************************************************/

#include "AppConfig.h"
#include "rotate.h"
#include <stdio.h>
#include <math.h>

#define DEG_TO_RAD(x)      ((x) / 57.29577951310823)
#define RAD_TO_DEG(x)      ((x) * 57.29577951310823)
/* note (double)(180.0/PI) = 57.29578018... */

#define DATA_ORIGIN   0
#define VIEWER_ORIGIN   1

static double VectorMagnitude (double vect[3]);
static void MatrixCopy (double source[4][4], 
                        double dest[4][4]);
static void MatrixMult (double m1[4][4],
                        double m2[4][4],
                        double dest[4][4]);


void SetViewPointFocalPoint (double ViewMatrix[4][4],
                             double vp[3],
                             double fp[3],
                             double twist)
{
  double b[3], r[3], h[3];
  double mat[4][4];
  double mag_b, mag_h;
  double phi;
  double roll;
  int i, j;
  
  for (i=0; i<3; i++)
  {
    h[i] = fp[i] - vp[i];
    b[i] = h[i];
  }
  mag_h = VectorMagnitude (h);
  
  if (mag_h == 0.0) 
  {
    fprintf (stderr, "Error: ViewPoint = FocalPoint\n");
    return;
  }
    
//  phi = acosf (-h[2] / mag_h);
  phi = acos(-h[2] / mag_h);
  
//  mag_b = sqrtf (b[0] * b[0] + b[1] * b[1]);
  mag_b = sqrt(b[0] * b[0] + b[1] * b[1]);
  
  if (mag_b == 0.0)
  {
    r[1] = r[2] = 0.0;
    r[0] = 1.0;
    phi = 0.0;
    if (b[2] > 0.0)
    {
      phi = PI;
    }
  }
  else
  {
  
    r[0] = -b[1] / mag_b;
    r[1] = b[0] / mag_b;
    r[2] = 0.0;
  }
  
  MakeRotationMatrix (mat, r, phi);
  
  MatrixCopy (mat, ViewMatrix);
  
  for (i=0; i<3; i++)
  {
    ViewMatrix[3][i] = 0.0;
    for (j=0; j<3; j++)
    {
      ViewMatrix[3][i] += vp[j] * ViewMatrix[j][i];
    }
    ViewMatrix[3][i] = -ViewMatrix[3][i];
  }
  
  if (GetRoll (ViewMatrix, &roll) != -1)
  {
    Rotation (ViewMatrix, LOS, DEG_TO_RAD(twist-roll), vp, fp);
  }
}


void SetHome (double ViewMatrix[4][4], 
              double HomeMatrix[4][4],
              double vp[3],
              double Homevp[3],
              double fp[3],
              double Homefp[3])
{
//  int i, j;
  int i = 0;
  
  for (i=0; i<3; i++)
  {
    Homevp[i] = vp[i];
    Homefp[i] = fp[i];
  }
  
  MatrixCopy (ViewMatrix, HomeMatrix);
}


void Home (double ViewMatrix[4][4], 
           double HomeMatrix[4][4],
           double vp[3],
           double Homevp[3],
           double fp[3],
           double Homefp[3])
{
//  int i, j;
   int i = 0;
  
  for (i=0; i<3; i++)
  {
    vp[i] = Homevp[i];
    fp[i] = Homefp[i];
  }
  
  MatrixCopy (HomeMatrix, ViewMatrix);
}


void Rotation (double ViewMatrix[4][4],
               int axis,
               double phi,
               double vp[3],
               double fp[3])
{
  double a[3];
  int coords;
#ifndef _NO_PROTO
  double vec[3] = {0.0, 0.0, 0.0};
#else
  double vec[3];
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;
#endif
  
  coords = MakeAxis (ViewMatrix, axis, a);
  
  AnyRotation (ViewMatrix, coords, a, VIEWER_COORDS, vec,
    phi, vp, fp);
}


int MakeAxis (double ViewMatrix[4][4],
               int axis,
               double a[3])
{
  int coords;
  
  a[0] = 0.0;
  a[1] = 0.0;
  a[2] = 0.0;
  
  if (axis < VXAXIS)
  {
    a[axis - DXAXIS] = 1.0;
    coords = DATA_COORDS;
  }
  else
  {
    coords = VIEWER_COORDS;
    if (axis < LOS)
    {
      a[axis - VXAXIS] = 1.0;
    }
    else
    {
      if (axis == LOS)
      {
        a[2] = -1.0;
      }
      else
      {
        double mag;
        
        a[2] = -1.0;
        ViewerToDataMinusOrigin (ViewMatrix, a, a);
        a[2] = 0.0;
        
        mag = VectorMagnitude (a);
        if (mag == 0.0) mag = 1.0;
        
        a[0] /= mag;
        a[1] /= mag;
        
        coords = DATA_COORDS;
      }
    }
  }
  
  return coords;
}


void RotationA (double ViewMatrix[4][4],
                int coords,
                double axis[3],
                double phi,
                double vp[3],
                double fp[3])
{
  double a[3];
  double mag;
#ifndef _NO_PROTO
  double vec[3] = {0.0, 0.0, 0.0};
#else
  double vec[3];
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;
#endif
  
  mag = VectorMagnitude (axis);
  if (mag == 0.0) return;
  
  a[0] = axis[0] / mag;
  a[1] = axis[1] / mag;
  a[2] = axis[2] / mag;
  
  AnyRotation (ViewMatrix, coords, a, VIEWER_COORDS, vec,
    phi, vp, fp);
}


void Motion (double ViewMatrix[4][4],
             int axis,
             double dx,
             double vp[3],
             double fp[3])
{
  double a[3];
  int coords;
  
  coords = MakeAxis (ViewMatrix, axis, a);
  MotionA (ViewMatrix, coords, a, dx, vp, fp);
}

//translate
void MotionA (double ViewMatrix[4][4],
              int coords,
              double axis[3],
              double dx,
              double vp[3],
              double fp[3])
{
  int i;
  double a[3];
  
  if (coords == DATA_COORDS)
  {
    DataToViewer (ViewMatrix, axis, a);
    for (i=0; i<3; i++)
    {
      a[i] -= ViewMatrix[3][i];
    }
  }
  else
  {
    a[0] = axis[0];
    a[1] = axis[1];
    a[2] = axis[2];
  }
  
  for (i=0; i<3; i++)
  {
    a[i] *= -dx;
  }
  
  for (i=0; i<3; i++)
  {
    ViewMatrix[3][i] += a[i];
    vp[i] += a[i];
    fp[i] += a[i];
  }
}


void DataToViewer (double ViewMatrix[4][4],
                   double axis[3],
                   double a[3])
{
  int i, j;
  double temp[3];
  
  for (i=0; i<3; i++)
  {
    temp[i] = axis[i];
  }
  
  for (i=0; i<3; i++)
  {
    a[i] = 0.0;
    for (j=0; j<3; j++)
    {
      a[i] += ViewMatrix[j][i] * temp[j];
    }
    a[i] += ViewMatrix[3][i];
  }
}


void ViewerToData (double ViewMatrix[4][4],
                   double axis[3],
                   double a[3])
{
  double Inverse[4][4];
  
  InvertViewMatrix (ViewMatrix, Inverse);
  
  DataToViewer (Inverse, axis, a);
}


void DataToViewerMinusOrigin (double ViewMatrix[4][4],
                              double axis[3],
                              double a[3])
{
#ifndef _NO_PROTO
  double origin[3] = {0.0, 0.0, 0.0};
#else
  double origin[3];
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 0.0;
#endif
  
  DataToViewer (ViewMatrix, axis, a);
  DataToViewer (ViewMatrix, origin, origin);
  
  a[0] -= origin[0];
  a[1] -= origin[1];
  a[2] -= origin[2];
}


void ViewerToDataMinusOrigin (double ViewMatrix[4][4],
                              double axis[3],
                              double a[3])
{
  double Inverse[4][4];
  
  InvertViewMatrix (ViewMatrix, Inverse);
  
  DataToViewerMinusOrigin (Inverse, axis, a);
}


void InvertViewMatrix (double ViewMatrix[4][4],
                       double Inverse[4][4])
{
  int i, j;
  
  for (i=0; i<3; i++)
  {
    for (j=0; j<3; j++)
    {
      Inverse[i][j] = ViewMatrix[j][i];
    }
  }
  
  for (i=0; i<3; i++)
  {
    Inverse[3][i] = 0.0;
    for (j=0; j<3; j++)
    {
      Inverse[3][i] += ViewMatrix[3][j] * ViewMatrix[i][j];
    }
    Inverse[3][i] = -Inverse[3][i];
  }
}


void GetVPFP (double ViewMatrix[4][4],
              double vp[3],
              double fp[3])
{
  int i, j;
  double mag_h;
  double h[3];
  
  for (i=0; i<3; i++)
  {
    h[i] = vp[i] - fp[i];
  }
  
  mag_h = VectorMagnitude (h);
  if (mag_h == 0.0)
  {
    mag_h = 1.0;
  }
  
  for (i=0; i<3; i++)
  {
    vp[i] = 0.0;
    for (j=0; j<3; j++)
    {
      vp[i] += ViewMatrix[3][j] * ViewMatrix[i][j];
    }
    vp[i] = -vp[i];
  }
  
  h[0] = -ViewMatrix[0][2] * mag_h;
  h[1] = -ViewMatrix[1][2] * mag_h;
  h[2] = -ViewMatrix[2][2] * mag_h;
  
  fp[0] = h[0] + vp[0];
  fp[1] = h[1] + vp[1];
  fp[2] = h[2] + vp[2];
}


static double VectorMagnitude (double vect[3])
{
  double dist;
  
  dist = vect[0] * vect[0];
  dist += vect[1] * vect[1];
  dist += vect[2] * vect[2];
  
//  dist = sqrtf (dist);
  dist = sqrt(dist);
  
  return dist;
}


void MakeRotationMatrix (double m[4][4], 
                         double a[3], 
                         double phi)
{
  double sine, cosine;
  double c;
  double b[3];
  int i;
  
//  sine = sinf (phi / 2.0);
  sine = sin(phi / 2.0);
//  cosine = cosf (phi / 2.0);
  cosine = cos(phi / 2.0);
  c = cosine;
  
  for (i=0; i<3; i++)
  {
    b[i] = a[i] * sine;
  }
  
  m[0][0] = 1.0 - 2.0 * (b[1] * b[1] + b[2] * b[2]);
  m[1][0] = 2.0 * (b[0] * b[1] + b[2] * c);
  m[2][0] = 2.0 * (b[0] * b[2] - b[1] * c);
  m[3][0] = 0.0;
  
  m[0][1] = 2.0 * (b[0] * b[1] - b[2] * c);
  m[1][1] = 1.0 - 2.0 * (b[0] * b[0] + b[2] * b[2]);
  m[2][1] = 2.0 * (b[1] * b[2] + b[0] * c);
  m[3][1] = 0.0;
  
  m[0][2] = 2.0 * (b[0] * b[2] + b[1] * c);
  m[1][2] = 2.0 * (b[1] * b[2] - b[0] * c);
  m[2][2] = 1.0 - 2.0 * (b[1] * b[1] + b[0] * b[0]);
  m[3][2] = 0.0;
  
  m[0][3] = 0.0;
  m[1][3] = 0.0;
  m[2][3] = 0.0;
  m[3][3] = 1.0;
}


int GetRoll (double ViewMatrix[4][4],
              double *roll)
{
#ifndef _NO_PROTO
  double zaxis[3] = {0.0,0.0,1.0}, xaxis[3] = {1.0,0.0,0.0};
  double origin[3]= {0.0,0.0,0.0};
#else
  double zaxis[3], xaxis[3];
  double origin[3];
#endif
  double A, B, C, a, b, c;
  double sine_squared;
  double square_root;
  double phi;
  double mag_z, mag_x;
  int i;
#ifdef _NO_PROTO
  zaxis[0] = xaxis[0] = origin[0] = 0.0;
  zaxis[1] = xaxis[1] = origin[1] = 0.0;
  zaxis[2] = xaxis[2] = origin[2] = 0.0;
#endif
  
  if ((fabs (ViewMatrix[0][2]) < .01) && (fabs (ViewMatrix[1][2]) < .01))
  {
    return -1;
  }
  
  ViewerToData (ViewMatrix, zaxis, zaxis);
  ViewerToData (ViewMatrix, xaxis, xaxis);
  ViewerToData (ViewMatrix, origin, origin);
  
  for (i=0; i<3; i++)
  {
    zaxis[i] -= origin[i];
    xaxis[i] -= origin[i];
  }
  
  mag_z = VectorMagnitude (zaxis);
  mag_x = VectorMagnitude (xaxis);
  
  for (i=0; i<3; i++)
  {
    zaxis[i] /= mag_z;
    xaxis[i] /= mag_x;
  }
  
  A = (2.0*xaxis[0]*zaxis[0]*zaxis[2]) + (2.0*xaxis[1]*zaxis[1]*zaxis[2])
    - (2.0*xaxis[2]*zaxis[0]*zaxis[0]) - (2.0*xaxis[2]*zaxis[1]*zaxis[1]);
  
  B = (2.0*xaxis[1]*zaxis[0]) - (2.0*xaxis[0]*zaxis[1]);
  
  C = xaxis[2];
  
  a = A*A + B*B;
  
  b = 2.0*A*C - B*B;
  
  c = C*C;
  
//  square_root = sqrtf (b*b - 4.0*a*c);
  square_root = sqrt(b*b - 4.0*a*c);
  
  if (a != 0.0)
  {
    sine_squared = (-b - square_root) / (2.0*a);
  
    if (sine_squared < 0.0)
    {
      sine_squared = (-b + square_root) / (2.0*a);
    }
  
//    phi = 2.0 * asinf (sqrtf (sine_squared));
    phi = 2.0 * asin(sqrt(sine_squared));
  }
  else
  {
    phi = PI / 2.0;
  }
  
  if (xaxis[2] > 0.0)
  {
    phi = -phi;
  }
  
  if (ViewMatrix[2][1] < 0.0)
  {
    if (phi > 0.0)
    {
      phi = PI - phi;
    }
    else
    {
      phi = -PI - phi;
    }
  }
  
  *roll = RAD_TO_DEG(-phi);
  
  return 1;
}


void GetPitch (double mat[4][4], 
               double *Pitch)
{
#define sign(x) ((x)>0?1:-1)
  double h[3], ph[3];
  double hmag, phmag;
  double RtoD = 180.0 / PI;
  
  h[0] = mat[0][2];
  h[1] = mat[1][2];
  h[2] = mat[2][2];
//  hmag = fsqrt (h[0]*h[0] + h[1]*h[1] + h[2]*h[2]);
  hmag = sqrt(h[0]*h[0] + h[1]*h[1] + h[2]*h[2]);
  
  ph[0] = h[0];
  ph[1] = h[1];
  ph[2] = 0.0;
//  phmag = fsqrt (ph[0]*h[0] + ph[1]*h[1]);
  phmag = sqrt(ph[0]*h[0] + ph[1]*h[1]);
  if (phmag < 0.0001) 
  {
    *Pitch =  (90.0 * sign (h[2]));
    return;
  }
  
  *Pitch = h[0] * h[0] + h[1] * h[1];
  *Pitch /= hmag;
  *Pitch /= phmag;
  *Pitch = acos (*Pitch);
  *Pitch *= RtoD;
  *Pitch *= sign (h[2]);
  
  return;
#undef sign
}


static void MatrixCopy (double source[4][4], 
                        double dest[4][4])
{
  int i, j;
  
  for (i=0; i<4; i++)
  {
    for (j=0; j<4; j++)
    {
      dest[i][j] = source[i][j];
    }
  }
}


static void MatrixMult (double m1[4][4], 
                        double m2[4][4],
                        double dest[4][4])
{
  int i, j, k;
  
  for (i=0; i<4; i++)
  {
    for (j=0; j<4; j++)
    {
      dest[i][j] = 0.0;
      for (k=0; k<4; k++)
      {
        dest[i][j] += m1[k][j] * m2[i][k];
      }
    }
  }
}


void DataRotation (double ViewMatrix[4][4],
                   int axis,
                   double phi,
                   double vp[3],
                   double fp[3])
{
  double a[3];
  int coords;
#ifndef _NO_PROTO
  double vec[3] = {0.0, 0.0, 0.0};
#else
  double vec[3];
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;
#endif

  coords = MakeAxis (ViewMatrix, axis, a);
  
  AnyRotation (ViewMatrix, coords, a, DATA_COORDS, vec,
    -phi, vp, fp);
}


void DataRotationA (double ViewMatrix[4][4],
                    int coords,
                    double axis[3],
                    double phi,
                    double vp[3],
                    double fp[3])
{
  double a[3];
  double mag;
#ifndef _NO_PROTO
  double vec[3] = {0.0, 0.0, 0.0};
#else
  double vec[3];
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;
#endif

  mag = VectorMagnitude (axis);
  if (mag == 0.0) return;
  
  a[0] = axis[0] / mag;
  a[1] = axis[1] / mag;
  a[2] = axis[2] / mag;
  
  AnyRotation (ViewMatrix, coords, a, DATA_COORDS, vec,
    -phi, vp, fp);
}


void AnyRotation (double ViewMatrix[4][4],
                  int axis_coords,
                  double axis[3],
                  int COR_coords,
                  double COR[3],
                  double phi,
                  double vp[3],
                  double fp[3])
{
//  double mag_fp;
  double n[4][4], e[4][4], v[4][4];
  int i;
#ifndef _NO_PROTO
  double trans[3] = {0.0, 0.0, 0.0};
  double a[3], b[3] = {0.0,0.0,0.0}, c[3] = {0.0,0.0,1.0};
#else
  double trans[3];
  double a[3], b[3], c[3];
  trans[0] = b[0] = c[0] = 0.0;
  trans[1] = b[1] = c[1] = 0.0;
  trans[2] = b[2] = 0.0;
  c[2] = 1.0;
#endif
  
  if (phi == 0.0) return;

  if (axis_coords == DATA_COORDS)
  {
    DataToViewer (ViewMatrix, axis, a);
    for (i=0; i<3; i++)
    {
      a[i] -= ViewMatrix[3][i];
    }
  }
  else
  {
    a[0] = axis[0];
    a[1] = axis[1];
    a[2] = axis[2];
  }

  if (COR_coords == DATA_COORDS)
  {
    DataToViewer (ViewMatrix, COR, trans);
  }
  else
  {
    trans[0] = COR[0];
    trans[1] = COR[1];
    trans[2] = COR[2];
  }
  
  if (VectorMagnitude (a) == 0.0) return;
  
  MotionA (ViewMatrix, VIEWER_COORDS, trans, 1.0, b, c);
  
  MatrixCopy (ViewMatrix, n);
  
  MakeRotationMatrix (e, a, DEG_TO_RAD (phi));
  
  MatrixMult (e, n, v);

  MatrixCopy (v, ViewMatrix);
  
  MotionA (ViewMatrix, VIEWER_COORDS, trans, -1.0, b, c);
  
  GetVPFP (ViewMatrix, vp, fp);
}


int NormalizeViewMatrix (double Source[4][4],
                          double Dest[4][4])
{
//  int i, j;
  int j;
  double vec[3];
  double mag;
  double temp[4][4];
  #define sgn(x) ((x)>0?1:-1)
  
  MatrixCopy (Source, temp);
  
  for (j=0; j<3; j++)
  {
    vec[0] = temp[j][0];
    vec[1] = temp[j][1];
    vec[2] = temp[j][2];
    
    mag = VectorMagnitude (vec);
    if (mag == 0.0)
    {
      fprintf (stderr, "Illegal ViewMatrix in NormalizeViewMatrix()\n");
      return -1;
    }
    
    vec[0] /= mag;
    vec[1] /= mag;
    vec[2] /= mag;
    
    temp[j][0] = vec[0];
    temp[j][1] = vec[1];
    temp[j][2] = vec[2];
  }
    
  for (j=0; j<3; j++)
  {
    mag = temp[0][j]*temp[0][j] + temp[1][j]*temp[1][j];
    if (mag > 1.0)
    {
      fprintf (stderr, "Illegal ViewMatrix in NormalizeViewMatrix()\n");
      return -1;
    }
//    temp[2][j] = sqrtf (1.0 - mag) * (double)sgn (Source[2][j]);
    temp[2][j] = sqrt(1.0 - mag) * (double)sgn(Source[2][j]);
  }
  
  MatrixCopy (temp, Dest);
  
  Dest[0][3] = Dest[1][3] = Dest[2][3] = 0.0;
  Dest[3][3] = 1.0;
  
  return 1;
}

 
