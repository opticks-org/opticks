/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include "Mgrs.h"
#include "TypesFile.h"

#include <stdio.h>
#include <string>
#if defined(UNIX_API)
#include <strings.h>
#include <string.h>
#endif
#include <math.h>

double Mgrs::MGRS_a = 6378137.0;    // Semi-major axis of ellipsoid in meters
double Mgrs::MGRS_b = 6356752.3142; // Semi-minor axis of ellipsoid           
double Mgrs::MGRS_recpf = 1 / ((6378137.0 - 6356752.3142) / 6378137.0);
//char Mgrs::MGRS_Ellipsoid_Code[0] = 'W';
//char Mgrs::MGRS_Ellipsoid_Code[1] = 'E';
//char Mgrs::MGRS_Ellipsoid_Code[2] = 0;
char Mgrs::MGRS_Ellipsoid_Code[3] = {'W','E',0};
char* Mgrs::CLARKE_1866 = "CC";
char* Mgrs::CLARKE_1880 = "CD";
char* Mgrs::BESSEL_1841 = "BR";

//                  UTM    GLOBAL DECLARATIONS
double Mgrs::UTM_a = 6378137.0;    // Semi-major axis of ellipsoid in meters  
double Mgrs::UTM_b = 6356752.3142; // Semi-minor axis of ellipsoid            
int Mgrs::UTM_Override = 0;     // Zone override flag                      

//                  Tranmerc    GLOBAL DECLARATIONS

// Ellipsoid Parameters, default to WGS 84  
double Mgrs::TranMerc_a = 6378137.0;              // Semi-major axis of ellipsoid i meters 
double Mgrs::TranMerc_b = 6356752.3142;           // Eccentricity of ellipsoid  
double Mgrs::TranMerc_es = 0.0066943799901413800; // Eccentricity (0.08181919084262188000) squared 
double Mgrs::TranMerc_ebs = 0.0067394967565869;   // Second Eccentricity squared 

// Transverse_Mercator projection Parameters 
double Mgrs::TranMerc_Origin_Lat = 0.0;           // Latitude of origin in radians 
double Mgrs::TranMerc_Origin_Long = 0.0;          // Longitude of origin in radians 
double Mgrs::TranMerc_False_Northing = 0.0;       // False northing in meters 
double Mgrs::TranMerc_False_Easting = 0.0;        // False easting in meters 
double Mgrs::TranMerc_Scale_Factor = 1.0;         // Scale factor  

// Isometeric to geodetic latitude parameters, default to WGS 84 
double Mgrs::TranMerc_ap = 6367449.1458008;
double Mgrs::TranMerc_bp = 16038.508696861;
double Mgrs::TranMerc_cp = 16.832613334334;
double Mgrs::TranMerc_dp = 0.021984404273757;
double Mgrs::TranMerc_ep = 3.1148371319283e-005;

// Maximum variance for easting and northing values for WGS 84. 
double Mgrs::TranMerc_Delta_Easting = 40000000.0;
double Mgrs::TranMerc_Delta_Northing = 40000000.0;

// These state variables are for optimization purposes. The only function
// that should modify them is Set_Tranverse_Mercator_Parameters.         


Mgrs *Mgrs::mpSingleton = NULL;
Mgrs *Mgrs::Instance()
{
   if(mpSingleton == NULL)
   {
      mpSingleton = new Mgrs();
   }
   return mpSingleton;
}
//*****************************************************************
Mgrs::Mgrs()
{
}

/***************************************************************************/
/*
 *                     MGRS FUNCTIONS     
 */


void Mgrs::UTMSET (int izone, 
             int* ltrlow, 
             int* ltrhi, 
             double *fnltr)
{ /* BEGIN UTMSET */
  /*
   *    izone  : Zone number
   *    ltrlow : 2nd letter low number
   *    ltrhi  : 2nd letter high number
   *    fnltr  : False northing
   */
  int iset;      /* Set number (1-6) based on UTM zone number           */
  int igroup;    /* Group number (1-2) based on ellipsoid code and zone */
  iset = 1;
  while (((izone-iset) / 6) * 6 + iset != izone)
  {
    iset = iset +1;
    if (iset > 6)
    {
      return;
    }
  } 
  igroup = 1;
  if (!strcmp(MGRS_Ellipsoid_Code,CLARKE_1866) || !strcmp(MGRS_Ellipsoid_Code, CLARKE_1880) || !strcmp(MGRS_Ellipsoid_Code,BESSEL_1841))
  {
    igroup = 2;
  }
  else if (!strcmp(MGRS_Ellipsoid_Code, CLARKE_1866) && (izone >= 47) && (izone <= 55 ))
  {
    igroup = 1;
  }
  if ((iset == 1) || (iset == 4))
  {
    *ltrlow = LETTER_A;
    *ltrhi = LETTER_H;
  }
  else if ((iset == 2) || (iset == 5))
  {
    *ltrlow = LETTER_J;
    *ltrhi = LETTER_R;
  }
  else if ((iset == 3) || (iset == 6))
  {
    *ltrlow = LETTER_S;
    *ltrhi = LETTER_Z;
  }
  if (igroup == 1)
  {
    *fnltr = ZERO;
    if ((iset % 2) ==  0)
    {
      *fnltr = 1500000.e0;
    }
  }
  else if (igroup == 2)
  {
    *fnltr = 1000000.0e0;
    if ((iset % 2) == 0)
    {
      *fnltr =  500000.e0;
    }
  }
  return;
} /* END OF UTMSET */


void Mgrs::UTMLIM (int* n, 
             double sphi, 
             int izone, 
             double *spsou, 
             double *spnor,
             double *sleast, 
             double *slwest)
{                          /* BEGIN UTMLIM */
  /*
   *    n      : 1st letter number for MGRS
   *    sphi   : Latitude in radians
   *    izone  : Zone number 
   *    spsou  : South latitude limit
   *    spnor  : North latitude limit
   *    sleast : East longitude limit 
   *    slwest : West longitude limit 
   */
  double slcm;     /* Central meridian - Longitude of origin              */
  double temp;     /* Temporary variable                                  */
  int icm;        /* Central meridian                                    */
  int isphi;      /* South latitude limit                                */
  if (*n <= LETTER_A)
  {
    temp = ((sphi + R80) / (R8)) + 2;
    temp = temp + .00000001;
    *n = (int)temp;
    if (*n > LETTER_H)
    {
      *n = *n + 1;
    }
    if (*n > LETTER_N)
    {
      *n = *n + 1;
    }
    if (*n >= LETTER_Y)
    {
      *n = LETTER_X;
    }
    if ((*n  ==  LETTER_M) && (sphi  ==  ZERO ))
    {
      *n = LETTER_N;
    }
    isphi = (*n - 3) * 8 - 80;
  }
  else
  {
    isphi = (*n - 3) * 8 - 80;
    *n = *n - 1;    
  }
  if (*n > LETTER_H)
  {
    isphi = isphi - 8;
  }
  if (*n > LETTER_N)
  {
    isphi = isphi - 8;
  }
  *spsou = (double)(isphi) * DEGRAD;
  *spnor = *spsou + R8;
  if (*n == LETTER_X)
  {
    *spnor = *spsou + 12.e0 * DEGRAD;
  }
  icm = izone * 6 - 183;
  slcm = (double) icm * DEGRAD;
  *sleast = slcm + R3;
  *slwest = slcm - R3;
  if ((izone < 31) || (izone > 37))
  {
    return;
  }
  if (*n < LETTER_V)
  {
    return;
  }
  if ((*n == LETTER_V) && (izone == 31))
  {
    *sleast = R3;
  }
  if ((*n == LETTER_V) && (izone == 32))
  {
    *slwest = R3;
  }
  if (*n < LETTER_X)
  {
    return;
  }
  if (izone == 31)
  {
    *sleast = R9;
  }
  if (izone == 33)
  {
    *slwest = R9;
    *sleast = R21;
  }
  if (izone == 35)
  {
    *slwest = R21;
    *sleast = R33;
  }
  if (izone == 37)
  {
    *slwest = R33;
  }
  return;
} /* END OF UTMLIM */


void Mgrs::UTMMGRS (int izone,
              int* ltrnum,
              double sphi,
              double x,
              double y)
{ /* BEGIN UTMMGRS */
  /*
   *    izone      : Zone number.
   *    ltrnum     : Values of letters in the MGRS coordinate.
   *    sphi       : Latitude in radians.
   *    x          : Easting.
   *    y          : Northing.
   *
   *    UTMMGRS CALLS THE FOLLOWING ROUTINES:
   *
   *    GPTUTM
   *    UTMLIM
   *    UTMSET
   */
  double fnltr;       /* False northing for 3rd letter                     */
  double slcm;        /* Central meridian - longitude of origin            */
  double sleast;      /* Longitude east limit - UTM                        */
  double slwest;      /* Longitude west limit -UTM                         */
  double spnor;       /* MGRS north latitude limits based on 1st letter    */
  double spsou;       /* MGRS south latitude limits based on 1st letter    */
  double xltr;        /* Easting used to derive 2nd letter of MGRS         */
  double yltr;        /* Northing used to derive 3rd letter of MGRS        */
  int ltrlow;        /* 2nd letter range - low number                     */
  int ltrhi;         /* 2nd letter range - high number                    */
  char hemisphere;

  UTMSET(izone, &ltrlow, &ltrhi, &fnltr);
  ltrnum[0] = LETTER_A;
  UTMLIM(&ltrnum[0], sphi, izone, &spsou, &spnor, &sleast, &slwest);
  slcm = (double)(izone * 6 - 183) * DEGRAD;
  /*
    GPTUTM(a, recf, spsou, slcm, &izone, &yltr, &xltr, (int)1);
  */
  Set_UTM_Parameters(MGRS_a,MGRS_b,izone);
  Convert_Geodetic_To_UTM(spsou,slcm,&izone,&hemisphere,&xltr,&yltr);

  yltr = (double)((int)(y + RND5));
  if (((double)((int)(yltr + RND5))) == ((double)((int)(1.e7 + RND5))))
  {
    yltr = (double)((int)(yltr - 1.e0 + RND5)); 
  }
  while (yltr >= TWOMIL)
  {
    yltr = yltr - TWOMIL; 
  }
  yltr = yltr - fnltr;
  if (yltr < ZERO)
  {
    yltr = yltr + TWOMIL;
  }
  ltrnum[2] = (int)((yltr + RND1) / ONEHT); 
  if (ltrnum[2] > LETTER_H)
  {
    ltrnum[2] = ltrnum[2] + 1;
  }
  if (ltrnum[2] > LETTER_N)
  {
    ltrnum[2] = ltrnum[2] + 1;
  }
  xltr = (double)((int)(x));
  if (((ltrnum[0] == LETTER_V) && (izone == 31)) && 
      (((double)((int)(xltr + RND5))) == ((double)((int)(5.e5 + RND5)))))
  {
    xltr = (double)((int)(xltr - 1.e0 + RND5)); /* SUBTRACT 1 METER */
  }
  ltrnum[1] = ltrlow + ((int)((xltr + RND1) / ONEHT) -1); 
  if ((ltrlow == LETTER_J) && (ltrnum[1] > LETTER_N))
  {
    ltrnum[1] = ltrnum[1] + 1;
  }
  return;
} /* END UTMMGRS */


void Mgrs::UPSSET (int n, 
             int* ltrlow, 
             int* ltrhi, 
             double *feltr,
             double *fnltr, 
             int* ltrhy)
{ /* BEGIN UPSSET */
  /*
   *   n      : Value of 1st letter in MGRS coordinate.
   *   ltrlow : Low number for 2nd letter.
   *   ltrhi  : High number for 2nd letter.
   *   feltr  : False easting.
   *   fnltr  : False northing.
   *   ltrhy  : High number for 3rd letter.
   */
  if (n == LETTER_Z) /* EASTERN HEMISPHERE-NORTH ZONE */
  {
    *ltrlow = LETTER_A;
    *ltrhi  = LETTER_J;
    *feltr = TWOMIL;
    *fnltr = ONE3HT;
    *ltrhy = LETTER_P;
  }
  else if (n == LETTER_Y) /* WESTERN HEMISPHERE-NORTH ZONE */
  {
    *ltrlow = LETTER_J;
    *ltrhi  = LETTER_Z;
    *feltr = EIGHT;
    *fnltr = ONE3HT;
    *ltrhy = LETTER_P;
  }
  else if (n == LETTER_B) /* ** EASTERN HEMISPHERE - SOUTH ZONE */
  {
    *ltrlow = LETTER_A;
    *ltrhi  = LETTER_R;
    *feltr = TWOMIL;
    *fnltr = EIGHT;
    *ltrhy = LETTER_Z;
  }
  else if (n == LETTER_A) /* ** WESTERN HEMISPHERE - SOUTH ZONE */
  {
    *ltrlow = LETTER_J;
    *ltrhi  = LETTER_Z;
    *feltr = EIGHT;
    *fnltr = EIGHT;
    *ltrhy = LETTER_Z;
  }
  return;
} /* END OF UPSSET */


void Mgrs::UPS(char* mgrs,
          int* ltrnum,
          double x,
          double y,
          int iarea)
{ /* BEGIN UPS */
  /*
   *    mgrs   : MGRS coordinate.
   *    ltrnum : Values of the letters in the MGRS coordinate.
   *    x      : Easting.
   *    y      : Northing.
   *    iarea  : Set to UPS_NORTH or UPS_SOUTH.
   *
   *    UPS CALLS THE FOLLOWING ROUTINES:
   *
   *    UPSSET
   */
  double feltr;       /* False easting for 2nd letter                      */
  double fnltr;       /* False northing for 3rd letter                     */
  double xltr;        /* Easting used to derive 2nd letter of MGRS         */
  double yltr;        /* Northing used to derive 3rd letter of MGRS        */
  int ltrlow;        /* 2nd letter range - low number                     */
  int ltrhi;         /* 2nd letter range - high number                    */
  int ltrhy;         /* 3rd letter range - high number (UPS)              */
  if (iarea == UPS_NORTH)
  {
    ltrnum[0] = LETTER_Y;
    if (((double)((int)(x + RND5))) >= TWOMIL)
    {
      ltrnum[0] = LETTER_Z; 
    }
  }
  else if (iarea == UPS_SOUTH)
  {
    ltrnum[0] = LETTER_A;
    if (((double)((int)(x + RND5))) >= TWOMIL)
    {
      ltrnum[0] = LETTER_B;
    }
  }
  UPSSET(ltrnum[0], &ltrlow, &ltrhi, &feltr, &fnltr, &ltrhy);
  mgrs[0] = BLANK;
  mgrs[1] = BLANK;
  yltr = (double)((int)(y + RND5));
  yltr = yltr - fnltr;
  ltrnum[2] = (int)((yltr + RND1) / ONEHT);
  if (ltrnum[2] > LETTER_H)
  {
    ltrnum[2] = ltrnum[2] + 1;
  }
  if (ltrnum[2] > LETTER_N)
  {
    ltrnum[2] = ltrnum[2] + 1;
  }
  xltr = (double)((int)(x + RND5));
  xltr = xltr - feltr;
  ltrnum[1] = ltrlow + ((int)((xltr + RND1) / ONEHT)); 
  if (x < TWOMIL)
  {
    if (ltrnum[1] > LETTER_L)
    {
      ltrnum[1] = ltrnum[1] + 3; 
    }
    if (ltrnum[1] > LETTER_U)
    {
      ltrnum[1] = ltrnum[1] + 2; 
    }
  }
  if (x >= TWOMIL)
  {
    if (ltrnum[1] > LETTER_C)
    {
      ltrnum[1] = ltrnum[1] + 2; 
    }
    if (ltrnum[1] > LETTER_H)
    {
      ltrnum[1] = ltrnum[1] + 1; 
    }
    if (ltrnum[1] > LETTER_L)
    {
      ltrnum[1] = ltrnum[1] + 3; 
    }
  }
  return;
} /* END OF UPS */


void Mgrs::LTR2UPS (int* ltrnum, 
              int ltrlow, 
              int ltrhi, 
              int ltrhy, 
              int* ierr, 
              double *xltr, 
              double *yltr, 
              double fnltr, 
              double feltr, 
              double *x, 
              double *y, 
              double sign)
{ /* BEGIN LTR2UPS */
  /*    
   *    ltrnum : Values of the letters in the MGRS coordinate
   *    ltrlow : Low number
   *    ltrhi  : High number-2nd letter
   *    ltrhy  : High number-3rd letter
   *    ierr   : Error code
   *    xltr   : Easting for 100,000 meter grid square
   *    yltr   : Northing for 100,000 meter grid square
   *    fnltr  : False northing for 3rd letter
   *    feltr  : False easting for 2nd letter
   *    x      : Easting
   *    y      : Northing
   *    sign   : Set to either positive or negative
   */
  if (ltrnum[1] < ltrlow)
  {
    *ierr = TRUE;
    return;
  }
  if (ltrnum[1] > ltrhi)
  {
    *ierr = TRUE;
    return;
  }
  if (ltrnum[2] > ltrhy)
  {
    *ierr = TRUE;
    return;
  }
  if ((ltrnum[1] == LETTER_D) || (ltrnum[1] == LETTER_E) ||
      (ltrnum[1] == LETTER_M) || (ltrnum[1] == LETTER_N) ||
      (ltrnum[1] == LETTER_V) || (ltrnum[1] == LETTER_W))
  {
    *ierr = TRUE;
    return;
  }
  *yltr = (double)ltrnum[2] * ONEHT + fnltr; 
  if (ltrnum[2] > LETTER_I)
  {
    *yltr = *yltr - ONEHT;
  }
  if (ltrnum[2] > LETTER_O)
  {
    *yltr = *yltr - ONEHT;
  }
  *xltr = (double)((ltrnum[1]) - ltrlow) * ONEHT + feltr; 
  if (ltrlow != LETTER_A)
  {
    if (ltrnum[1] > LETTER_L)
    {
      *xltr = *xltr - 3.e0 * ONEHT;
    }
    if (ltrnum[1] > LETTER_U)
    {
      *xltr = *xltr - 2.e0 * ONEHT;
    }
  }
  else if (ltrlow == LETTER_A)
  {
    if (ltrnum[1] > LETTER_C)
    {
      *xltr = *xltr - 2.e0 * ONEHT;
    }
    if (ltrnum[1] > LETTER_I)
    {
      *xltr = *xltr - ONEHT;
    }
    if (ltrnum[1] > LETTER_M)
    {
      *xltr = *xltr - ONEHT;
    }
  }
  *x = *xltr;
  *y = *yltr * sign;
  return;
} /* END OF LTR2UPS */

/*
void Mgrs::GRID_UPS (int   *Letters,
               char   *Hemisphere,
               double *Easting,
               double *Northing,
               int   *Error)
{
  // BEGIN GRID_UPS 
  double feltr;               // False easting for 2nd letter               
  double fnltr;               // False northing for 3rd letter              
  double sleast;              // Longitude east limit - UTM                 
  double slwest;              // Longitude west limit -UTM                  
  double spnor;               // North latitude limits based on 1st letter  
  double spsou;               // South latitude limits based on 1st letter  
  double x;                   // easting                                    
  double xltr;                // easting for 100,000 meter grid square      
  double xnum;                // easting part of MGRS                       
  double y;                   // northing                                   
  double yltr;                // northing for 100,000 meter grid square     
  double ynum;                // northing part of MGRS                      
  int izone;                 // Zone number                                
  int ltrhi;                 // 2nd letter range - high number             
  int ltrhy;                 // 3rd letter range - high number (UPS)       
  int ltrlow;                // 2nd letter range - low number              
  int sign;
  double sphi;
  double slam;
  if ((Letters[0] == LETTER_Y) || (Letters[0] == LETTER_Z))
  {
    spsou = MAX_UTM_LAT;
    sign = 1;
  }
  else
  {
    spsou = MIN_UTM_LAT;
    sign = -1;
  }
  slam = PI / 2.e0;
  if ((Letters[0] == LETTER_Y) || (Letters[0] == LETTER_A))
  {
    slam = -slam;
  }
  izone = 0;
  sphi = spsou;
  Set_UPS_Parameters(MGRS_a,MGRS_b);
  Convert_Geodetic_To_UPS(sphi,slam,Hemisphere,&x,&y);
  spnor = sphi;
  sleast = slam;
  slwest = slam;
  UPSSET(Letters[0], &ltrlow, &ltrhi, &feltr, &fnltr, &ltrhy);
  LTR2UPS(Letters, ltrlow, ltrhi, ltrhy, Error, &xltr, &yltr, fnltr, feltr, 
          &x, &y, sign);
  xnum = *Easting;
  ynum = *Northing;
  y = (yltr + ynum);
  x = xltr + xnum;
  *Easting = x;
  *Northing = y;
  return;
} // END OF GRID_UPS 
*/

void Mgrs::LTR2UTM (int* ltrnum, 
              int ltrlow, 
              int ltrhi, 
              int* ierr, 
              double *xltr, 
              double *yltr, 
              double fnltr, 
              double yslow, 
              double ylow) 
{ /* BEGIN LTR2UTM */
  /*    
   *    xltr   : Easting for 100,000 meter grid square.
   *    yltr   : Northing for 100,000 meter grid square.
   *    ierr   : Error code.
   *    ltrnum : Values of the letters in the MGRS coordinate.
   *    ltrlow : Low number.
   *    ltrhi  : High number.
   *    fnltr  : False northing for 3rd letter.
   *    yslow  : Northing scaled down to less than 2 million.
   *    ylow   : Lowest northing of area to nearest 100,000.
   */
  if (ltrnum[1] < ltrlow)
  {
    *ierr = TRUE;
    return;
  }
  if (ltrnum[1] > ltrhi)
  {
    *ierr = TRUE;
    return;
  }
  if (ltrnum[2] > LETTER_V)
  {
    *ierr = TRUE;
    return;
  }
  *yltr = (double)(ltrnum[2]) * ONEHT + fnltr;
  *xltr = (double)((ltrnum[1]) - ltrlow + 1) * ONEHT;
  if ((ltrlow == LETTER_J) && (ltrnum[1] > LETTER_O))
  {
    *xltr = *xltr - ONEHT;
  }
  if (ltrnum[2] > LETTER_O)
  {
    *yltr = *yltr - ONEHT;
  }
  if (ltrnum[2] > LETTER_I)
  {
    *yltr = *yltr - ONEHT; 
  }
  if (((double)((int)(*yltr + RND5))) >= ((double)((int)(TWOMIL + RND5))))
  {
    *yltr = *yltr - TWOMIL;
  }
  *yltr = ((double)((int)(*yltr + RND5)));
  *yltr = *yltr - yslow;
  if (*yltr < ZERO)
  {
    *yltr = *yltr + TWOMIL;
  }
  *yltr = ((double)((int)(ylow + *yltr + RND5)));
  return;
} /* END OF LTR2UTM */


void Mgrs::GRID_UTM (int   *Zone,
               int   *Letters,
               char   *Hemisphere,
               double *Easting,
               double *Northing,
               int   In_Precision,
               int   *Error)
{ /* BEGIN GRID_UTM */
  double fnltr;               /* False northing for 3rd letter              */
  int ltrhi;                 /* 2nd letter range - High number             */
  int ltrlow;                /* 2nd letter range - Low number              */
  int number;                /* Value of ltrnum[0] + 1                     */
//  double slam;
  double slcm;                /* Central meridian                           */
  double sleast;              /* Longitude east limit - UTM                 */
  double slwest;              /* Longitude west limit -UTM                  */
  double sphi;                /* Latitude (needed by UTMLIM)                */
  double spnor;               /* North latitude limits based on 1st letter  */
  double spsou;               /* South latitude limits based on 1st letter  */
  double xltr;                /* Easting for 100,000 meter grid square      */
  double ylow;                /* Lowest northing of area to nearest 100,000 */
  double yltr;                /* Northing for 100,000 meter grid square     */
  double yslow;               /* Northing scaled down to less than 2 million*/
  double Latitude = 0.0;
  double Longitude = 0.0;
  double divisor = 1.0;
  if ((*Zone == 32) && (Letters[0] == LETTER_X))
  {
    *Error = TRUE;
    return;
  }
  if ((*Zone == 34) && (Letters[0] == LETTER_X))
  {
    *Error = TRUE;
    return;
  }
  if ((*Zone == 36) && (Letters[0] == LETTER_X))
  {
    *Error = TRUE;
    return;
  }
  number = Letters[0] + 1;
  sphi = 0.0;
  UTMLIM(&number,sphi,*Zone,&spsou,&spnor,&sleast,&slwest);
  Set_UTM_Parameters(MGRS_a,MGRS_b,*Zone);
  slcm = (double)(*Zone * 6 - 183) * DEGRAD; 
  Convert_Geodetic_To_UTM(spsou,slcm,Zone,Hemisphere,&xltr,&yltr);
  ylow = ((double)((int)((double)((int)(yltr / ONEHT)) * ONEHT)));
  yslow = ylow;
  while (yslow >= TWOMIL)
  {
    yslow = yslow - TWOMIL;
  }
  yslow = ((double)((int)(yslow)));
  UTMSET(*Zone, &ltrlow, &ltrhi, &fnltr);
  LTR2UTM(Letters, ltrlow, ltrhi, Error, &xltr, &yltr, fnltr, yslow, ylow);
  *Easting = xltr + *Easting;
  *Northing = yltr + *Northing;
  /* check that point is within Zone Letter bounds */
  Convert_UTM_To_Geodetic(*Zone,*Hemisphere,*Easting,*Northing,&Latitude,&Longitude);
  divisor = pow (10.0, In_Precision);
  if (((spsou - DEGRAD/divisor) <= Latitude) && (Latitude <= (spnor + DEGRAD/divisor)))
    return;
  else
    *Error = TRUE;
  return;
}/* END OF GRID_UTM */


int Mgrs::Round_MGRS (double value)
/* Round value to nearest integer, using standard engineering rule */
{ /* Round_MGRS */
  double ivalue;
  int ival;
  double fraction = modf (value, &ivalue);
  ival = (int)(ivalue);
  if ((fraction > 0.5) || ((fraction == 0.5) && (ival%2 == 1)))
    ival++;
  return (ival);
} /* Round_MGRS */


int Mgrs::Make_MGRS_String (char* MGRS, 
                       int Zone, 
                       int ltrnum[MGRS_LETTERS], 
                       double Easting, 
                       double Northing,
                       int Precision)
/* Construct an MGRS string from its component parts */
{ /* Make_MGRS_String */
  double divisor;
  int east;
  int north;
  int i;
  int j;
  int error_code = MGRS_NO_ERROR;
  i = 0;
  if (Zone)
    i = sprintf (MGRS+i,"%2.2ld",Zone);
  for (j=0;j<3;j++)
    MGRS[i++] = ALBET[ltrnum[j]];
  divisor = pow (10.0, (5 - Precision));
  Easting = fmod (Easting, 100000.0);
  if (Easting >= 99999.5)
    Easting = 0.0;
  east = Round_MGRS (Easting/divisor);
  i += sprintf (MGRS+i, "%*.*ld", Precision, Precision, east);
  Northing = fmod (Northing, 100000.0);
  if (Northing >= 99999.5)
    Northing = 0.0;
  north = Round_MGRS (Northing/divisor);
  i += sprintf (MGRS+i, "%*.*ld", Precision, Precision, north);
  return (error_code);
} /* Make_MGRS_String */


int Mgrs::Break_MGRS_String (char* MGRS,
                        int* Zone,
                        int Letters[MGRS_LETTERS],
                        double* Easting,
                        double* Northing,
                        int* Precision)
/* Break down an MGRS string into its component parts */
{ /* Break_MGRS_String */
  int error_code = MGRS_NO_ERROR;
  int i = 0;
  int j;
  int num_digits;
  int num_letters;
  while (MGRS[i] == ' ')
    i++;  /* skip any leading blanks */
  j = i;
  while (isdigit(MGRS[i]))
    i++;
  num_digits = i - j;
  if (num_digits <= 2)
    if (num_digits > 0)
    {
      char zone_string[3];
      /* get zone */
      strncpy (zone_string, MGRS+j, 2);
      zone_string[2] = 0;
      sscanf (zone_string, "%d", Zone);  
      if ((*Zone < 1) || (*Zone > 60))
        error_code |= MGRS_STRING_ERROR;
    }
    else
      *Zone = 0;
  else
    error_code |= MGRS_STRING_ERROR;
  j = i;
  while (isalpha(MGRS[i]))
    i++;
  num_letters = i - j;
  if (num_letters == 3)
  {
    /* get letters */
    Letters[0] = (toupper(MGRS[j]) - (int)'A');
    if ((Letters[0] == LETTER_I) || (Letters[0] == LETTER_O))
      error_code |= MGRS_STRING_ERROR;
    Letters[1] = (toupper(MGRS[j+1]) - (int)'A');
    if ((Letters[1] == LETTER_I) || (Letters[1] == LETTER_O))
      error_code |= MGRS_STRING_ERROR;
    Letters[2] = (toupper(MGRS[j+2]) - (int)'A');
    if ((Letters[2] == LETTER_I) || (Letters[2] == LETTER_O))
      error_code |= MGRS_STRING_ERROR;
  }
  else
    error_code |= MGRS_STRING_ERROR;
  j = i;
  while (isdigit(MGRS[i]))
    i++;
  num_digits = i - j;
  if ((num_digits <= 10) && (num_digits%2 == 0))
  {
    int n;
    char east_string[6];
    char north_string[6];
    int east;
    int north;
    double multiplier;
    /* get easting & northing */
    n = num_digits/2;
    *Precision = n;
    if (n > 0)
    {
      strncpy (east_string, MGRS+j, n);
      east_string[n] = 0;
      sscanf (east_string, "%d", &east);
      strncpy (north_string, MGRS+j+n, n);
      north_string[n] = 0;
      sscanf (north_string, "%d", &north);
      multiplier = pow (10.0, 5 - n);
      *Easting = east * multiplier;
      *Northing = north * multiplier;
    }
    else
    {
      *Easting = 0.0;
      *Northing = 0.0;
    }
  }
  else
    error_code |= MGRS_STRING_ERROR;
  return (error_code);
} /* Break_MGRS_String */


int Mgrs::Set_MGRS_Parameters (double a,
                          double b,
                          char   *Ellipsoid_Code)
/*
 * The function SET_MGRS_PARAMETERS receives the ellipsoid parameters and sets
 * the corresponding state variables. If any errors occur, the error code(s)
 * are returned by the function, otherwise MGRS_NO_ERROR is returned.
 *
 *   a                :Semi-major axis of ellipsoid in meters  (input)
 *   b                :Semi-minor axis of ellipsoid in meters  (input)
 *   Ellipsoid_Code   : 2-letter code for ellipsoid            (input)
 */
{ /* Set_MGRS_Parameters  */
  int Error_Code = MGRS_NO_ERROR;
  if (a <= 0.0)
  { /* Semi-major axis must be greater than zero */
    Error_Code |= MGRS_A_ERROR;
  }
  if (b <= 0.0)
  { /* Semi-minor must be greater than zero */
    Error_Code |= MGRS_B_ERROR;
  }
  if (a < b)
  { /* Semi-major axis can not be less than Semi-minor axis */
    Error_Code |= MGRS_A_LESS_B_ERROR;
  }
  if (!Error_Code)
  { /* no errors */
    MGRS_a = a;
    MGRS_b = b;
    MGRS_recpf = 1 / ((a - b) / a);
    strcpy (MGRS_Ellipsoid_Code, Ellipsoid_Code);
  }
  return (Error_Code);
}  /* Set_MGRS_Parameters  */



void Mgrs::Get_MGRS_Parameters (double *a,
                          double *b,
                          char* Ellipsoid_Code)
/*
 * The function Get_MGRS_Parameters returns the current ellipsoid
 * parameters.
 *
 *  a                :Semi-major axis of ellipsoid, in meters (output)
 *  b                :Semi_minor axis of ellipsoid, in meters (output)
 *  Ellipsoid_Code   : 2-letter code for ellipsoid            (output)
 */
{ /* Get_MGRS_Parameters */
  *a = MGRS_a;
  *b = MGRS_b;
  strcpy (Ellipsoid_Code, MGRS_Ellipsoid_Code);
  return;
} /* Get_MGRS_Parameters */


int Mgrs::Convert_Geodetic_To_MGRS (double Latitude,
                               double Longitude,
                               int Precision,
                               char* MGRS)
/*
 *    Latitude   : Latitude in radians              (input)
 *    Longitude  : Longitude in radians             (input)
 *    MGRS       : MGRS coordinate string           (output)
 *  
 */
{ /* Convert_Geodetic_To_MGRS */
  int error_code = MGRS_NO_ERROR;
  int zone;
  char hemisphere;
  double easting;
  double northing;
  if ((Latitude < -PI_OVER_2) || (Latitude > PI_OVER_2))
  { /* Latitude out of range */
    error_code |= MGRS_LAT_ERROR;
  }
  if ((Longitude < -PI) || (Longitude > (2*PI)))
  { /* Longitude out of range */
    error_code |= MGRS_LON_ERROR;
  }
  if ((Precision < 0) || (Precision > MAX_PRECISION))
    error_code |= MGRS_PRECISION_ERROR;
  if (!error_code)
  {
    if ((Latitude < MIN_UTM_LAT) || (Latitude > MAX_UTM_LAT))
    {
/*
      Set_UPS_Parameters (MGRS_a, MGRS_b);
      error_code |= Convert_Geodetic_To_UPS (Latitude, Longitude, &hemisphere, &easting, &northing);
      error_code |= Convert_UPS_To_MGRS (hemisphere, easting, northing, Precision, MGRS);
*/
  }
    else
    {
      Set_UTM_Parameters (MGRS_a, MGRS_b, 0);
      error_code |= Convert_Geodetic_To_UTM (Latitude, Longitude, &zone, &hemisphere, &easting, &northing);
      error_code |= Convert_UTM_To_MGRS (zone, hemisphere, easting, northing, Precision, MGRS);
    }
  }
  return (error_code);
} /* Convert_Geodetic_To_MGRS */


int Mgrs::Convert_MGRS_To_Geodetic (char* MGRS, 
                               double *Latitude, 
                               double *Longitude)
/*
 *    MGRS       : MGRS coordinate string           (output)
 *    Latitude   : Latitude in radians              (input)
 *    Longitude  : Longitude in radians             (input)
 *  
 */
{ /* Convert_MGRS_To_Geodetic */
  int error_code = MGRS_NO_ERROR;
  int Zone;
  int Letters[MGRS_LETTERS];
  char Hemisphere;
  double Easting;
  double Northing;
  int In_Precision;
  error_code = Break_MGRS_String (MGRS, &Zone, Letters, &Easting, &Northing, &In_Precision);
  if (!error_code)
    if (Zone)
    {
      error_code |= Convert_MGRS_To_UTM (MGRS, &Zone, &Hemisphere, &Easting, &Northing);
      Set_UTM_Parameters (MGRS_a, MGRS_b, 0);
      error_code |= Convert_UTM_To_Geodetic (Zone, Hemisphere, Easting, Northing, Latitude, Longitude);
    }
    /*
    else
    {
      error_code |= Convert_MGRS_To_UPS (MGRS, &Hemisphere, &Easting, &Northing);
      Set_UPS_Parameters (MGRS_a, MGRS_b);
      error_code |= Convert_UPS_To_Geodetic (Hemisphere, Easting, Northing, Latitude, Longitude);
    }
    */
  return (error_code);
} /* END OF Convert_MGRS_To_Geodetic */



int Mgrs::Convert_UTM_To_MGRS (int Zone,
                          char Hemisphere,
                          double Easting,
                          double Northing,
                          int Precision,
                          char* MGRS)
/*
 * The function Convert_UTM_To_MGRS converts UTM (zone, easting, and
 * northing) coordinates to an MGRS coordinate string, according to the 
 * current ellipsoid parameters.  If any errors occur, the error code(s) 
 * are returned by the  function, otherwise MGRS_NO_ERROR is returned.
 *
 *    Zone       : UTM zone                         (input)
 *    Hemisphere : North or South hemisphere        (input)
 *    Easting    : Easting (X) in meters            (input)
 *    Northing   : Northing (Y) in meters           (input)
 *    Precision  : Precision level of MGRS string   (input)
 *    MGRS       : MGRS coordinate string           (output)
 */
{ /* Convert_UTM_To_MGRS */
  int error_code = MGRS_NO_ERROR;
  int Letters[MGRS_LETTERS]; /* Number location of 3 letters in alphabet   */
  double Latitude;           /* Latitude of UTM point */
  double Longitude;          /* Longitude of UTM point */
  if ((Zone < 1) || (Zone > 60))
    error_code |= MGRS_ZONE_ERROR;
  if ((Hemisphere != 'S') && (Hemisphere != 'N'))
    error_code |= MGRS_HEMISPHERE_ERROR;
  if ((Easting < MIN_EASTING) || (Easting > MAX_EASTING))
    error_code |= MGRS_EASTING_ERROR;
  if ((Northing < MIN_NORTHING) || (Northing > MAX_NORTHING))
    error_code |= MGRS_NORTHING_ERROR;
  if ((Precision < 0) || (Precision > MAX_PRECISION))
    error_code |= MGRS_PRECISION_ERROR;
  if (!error_code)
  {
    Set_UTM_Parameters (MGRS_a, MGRS_b,0);
    Convert_UTM_To_Geodetic (Zone, Hemisphere, Easting, Northing, &Latitude, &Longitude);
    UTMMGRS (Zone, Letters, Latitude, Easting, Northing);
    /* UTM checks - these should be done in UTMMGRS */
    if ((Zone == 31) && (Letters[0] == LETTER_V))
      if (Easting > 500000)
        Easting = 500000;
    if (Northing > 10000000)
      Northing = 10000000;
    Make_MGRS_String (MGRS, Zone, Letters, Easting, Northing, Precision);
  }
  return (error_code);
} /* Convert_UTM_To_MGRS */


int Mgrs::Convert_MGRS_To_UTM (char   *MGRS,
                          int   *Zone,
                          char   *Hemisphere,
                          double *Easting,
                          double *Northing)
/*
 * The function Convert_MGRS_To_UTM converts an MGRS coordinate string
 * to UTM projection (zone, hemisphere, easting and northing) coordinates 
 * according to the current ellipsoid parameters.  If any errors occur, 
 * the error code(s) are returned by the function, otherwise UTM_NO_ERROR 
 * is returned.
 *
 *    MGRS       : MGRS coordinate string           (input)
 *    Zone       : UTM zone                         (output)
 *    Hemisphere : North or South hemisphere        (output)
 *    Easting    : Easting (X) in meters            (output)
 *    Northing   : Northing (Y) in meters           (output)
 */
{ /* Convert_MGRS_To_UTM */
  int error_code = MGRS_NO_ERROR;
  int Letters[MGRS_LETTERS];
  int In_Precision;
  int Error = 0;
  error_code = Break_MGRS_String (MGRS, Zone, Letters, Easting, Northing, &In_Precision);
  if (!*Zone)
    error_code |= MGRS_STRING_ERROR;
  if (!error_code)
  {
    GRID_UTM (Zone, Letters, Hemisphere, Easting, Northing, In_Precision, &Error);
    if (Error)
      error_code = MGRS_STRING_ERROR;
  }
  return (error_code);
} /* Convert_MGRS_To_UTM */


//**************************************************************************/
//
//                       UTM   FUNCTIONS

int Mgrs::Set_UTM_Parameters(double a,      
                        double b,
                        int   override)
{
/*
 * The function Set_UTM_Parameters receives the ellipsoid parameters and
 * UTM zone override parameter as inputs, and sets the corresponding state
 * variables.  If any errors occur, the error code(s) are returned by the 
 * function, otherwise UTM_NO_ERROR is returned.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters       (input)
 *    b                 : Semi-minor axis of ellipsoid, in meters       (input)
 *    override          : UTM override zone, zero indicates no override (input)
 */

  int Error_Code = UTM_NO_ERROR;

  if (a <= 0.0)
  { /* Semi-major axis must be greater than zero */
    Error_Code |= UTM_A_ERROR;
  }
  if (b <= 0.0)
  { /* Semi-minor axis must be greater than zero */
    Error_Code |= UTM_B_ERROR;
  }
  if (a < b)
  { /* Semi-major axis can not be less than Semi-minor axis */
    Error_Code |= UTM_A_LESS_B_ERROR;
  }
  if ((override < 0) || (override > 60))
  {
    Error_Code |= UTM_ZONE_OVERRIDE_ERROR;
  }
  if (!Error_Code)
  { /* no errors */
    UTM_a = a;
    UTM_b = b;
    UTM_Override = override;
  }
  return (Error_Code);
} /* END OF Set_UTM_Parameters */


void Mgrs::Get_UTM_Parameters(double *a,
                        double *b,
                        int   *override)
{
/*
 * The function Get_UTM_Parameters returns the current ellipsoid
 * parameters and UTM zone override parameter.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters       (output)
 *    b                 : Semi-minor axis of ellipsoid, in meters       (output)
 *    override          : UTM override zone, zero indicates no override (output)
 */

  *a = UTM_a;
  *b = UTM_b;
  *override = UTM_Override;
} /* END OF Get_UTM_Parameters */


int Mgrs::Convert_Geodetic_To_UTM (double Latitude,
                              double Longitude,
                              int   *Zone,
                              char   *Hemisphere,
                              double *Easting,
                              double *Northing)
{ 
/*
 * The function Convert_Geodetic_To_UTM converts geodetic (latitude and
 * longitude) coordinates to UTM projection (zone, hemisphere, easting and
 * northing) coordinates according to the current ellipsoid and UTM zone
 * override parameters.  If any errors occur, the error code(s) are returned
 * by the function, otherwise UTM_NO_ERROR is returned.
 *
 *    Latitude          : Latitude in radians                 (input)
 *    Longitude         : Longitude in radians                (input)
 *    Zone              : UTM zone                            (output)
 *    Hemisphere        : North or South hemisphere           (output)
 *    Easting           : Easting (X) in meters               (output)
 *    Northing          : Northing (Y) in meters              (output)
 */

  int Lat_Degrees;
  int Long_Degrees;
  int temp_zone;
  int Error_Code = UTM_NO_ERROR;
  double Origin_Latitude = 0;
  double Central_Meridian = 0;
  double False_Easting = 500000;
  double False_Northing = 0;
  double Scale = 0.9996;

  if ((Latitude < MIN_LAT) || (Latitude > U_MAX_LAT))
  { /* Latitude out of range */
    Error_Code |= UTM_LAT_ERROR;
  }
  if ((Longitude < -PI) || (Longitude > (2*PI)))
  { /* Longitude out of range */
    Error_Code |= UTM_LON_ERROR;
  }

  { /* no errors */
    if (Longitude < 0)
      Longitude += (2*PI);
    Lat_Degrees = (int)(Latitude * 180.0 / PI);
    Long_Degrees = (int)(Longitude * 180.0 / PI);

    if (Longitude < PI)
      temp_zone = (int)(31 + ((Longitude * 180.0 / PI) / 6.0));
    else
      temp_zone = (int)(((Longitude * 180.0 / PI) / 6.0) - 29);
    if (temp_zone > 60)
      temp_zone = 1;
    /* UTM special cases */
    if ((Lat_Degrees > 55) && (Lat_Degrees < 64) && (Long_Degrees > -1)
        && (Long_Degrees < 3))
      temp_zone = 31;
    if ((Lat_Degrees > 55) && (Lat_Degrees < 64) && (Long_Degrees > 2)
        && (Long_Degrees < 12))
      temp_zone = 32;
    if ((Lat_Degrees > 71) && (Long_Degrees > -1) && (Long_Degrees < 9))
      temp_zone = 31;
    if ((Lat_Degrees > 71) && (Long_Degrees > 8) && (Long_Degrees < 21))
      temp_zone = 33;
    if ((Lat_Degrees > 71) && (Long_Degrees > 20) && (Long_Degrees < 33))
      temp_zone = 35;
    if ((Lat_Degrees > 71) && (Long_Degrees > 32) && (Long_Degrees < 42))
      temp_zone = 37;
/*
    if (UTM_Override)
    {
      if ((temp_zone == 1) && (UTM_Override == 60))
        temp_zone = UTM_Override;
      else if ((temp_zone == 60) && (UTM_Override == 1))
        temp_zone = UTM_Override;
      else if (((temp_zone-1) <= UTM_Override) && (UTM_Override <= (temp_zone+1)))
        temp_zone = UTM_Override;
      else
        Error_Code = UTM_ZONE_OVERRIDE_ERROR;
    }
*/
    if (!Error_Code)
    {
      if (temp_zone >= 31)
        Central_Meridian = (6 * temp_zone - 183) * PI / 180.0;
      else
        Central_Meridian = (6 * temp_zone + 177) * PI / 180.0;
      *Zone = temp_zone;
      if (Latitude < 0)
      {
        False_Northing = 10000000;
        *Hemisphere = 'S';
      }
      else
        *Hemisphere = 'N';
      Set_Transverse_Mercator_Parameters(UTM_a, UTM_b, Origin_Latitude,
                                         Central_Meridian, False_Easting, False_Northing, Scale);
      Convert_Geodetic_To_Transverse_Mercator(Latitude, Longitude, Easting,
                                              Northing);
      if ((*Easting < MIN_EASTING) || (*Easting > MAX_EASTING))
        Error_Code = UTM_EASTING_ERROR;
      if ((*Northing < MIN_NORTHING) || (*Northing > MAX_NORTHING))
        Error_Code |= UTM_NORTHING_ERROR;
    }
  } /* END OF if (!Error_Code) */
  return (Error_Code);
} /* END OF Convert_Geodetic_To_UTM */


int Mgrs::Convert_UTM_To_Geodetic(int   Zone,
                             char   Hemisphere,
                             double Easting,
                             double Northing,
                             double *Latitude,
                             double *Longitude)
{
/*
 * The function Convert_UTM_To_Geodetic converts UTM projection (zone, 
 * hemisphere, easting and northing) coordinates to geodetic(latitude
 * and  longitude) coordinates, according to the current ellipsoid
 * parameters.  If any errors occur, the error code(s) are returned
 * by the function, otherwise UTM_NO_ERROR is returned.
 *
 *    Zone              : UTM zone                               (input)
 *    Hemisphere        : North or South hemisphere              (input)
 *    Easting           : Easting (X) in meters                  (input)
 *    Northing          : Northing (Y) in meters                 (input)
 *    Latitude          : Latitude in radians                    (output)
 *    Longitude         : Longitude in radians                   (output)
 */
  int Error_Code = UTM_NO_ERROR;
  double Origin_Latitude = 0;
  double Central_Meridian = 0;
  double False_Easting = 500000;
  double False_Northing = 0;
  double Scale = 0.9996;

  if ((Zone < 1) || (Zone > 60))
    Error_Code |= UTM_ZONE_ERROR;
  if ((Hemisphere != 'S') && (Hemisphere != 'N'))
    Error_Code |= UTM_HEMISPHERE_ERROR;
  if ((Easting < MIN_EASTING) || (Easting > MAX_EASTING))
    Error_Code |= UTM_EASTING_ERROR;
  if ((Northing < MIN_NORTHING) || (Northing > MAX_NORTHING))
    Error_Code |= UTM_NORTHING_ERROR;
  if (!Error_Code)
  { /* no errors */
    if (Zone >= 31)
      Central_Meridian = ((6 * Zone - 183) * PI / 180.0 + 0.00000005);
    else
      Central_Meridian = ((6 * Zone + 177) * PI / 180.0 + 0.00000005);
    if (Hemisphere == 'S')
      False_Northing = 10000000;
    Set_Transverse_Mercator_Parameters(UTM_a, UTM_b, Origin_Latitude,
                                       Central_Meridian, False_Easting, False_Northing, Scale);
    if (Convert_Transverse_Mercator_To_Geodetic(Easting,
                                                Northing,
                                                Latitude, 
                                                Longitude))
      Error_Code |= UTM_NORTHING_ERROR;
    if ((*Latitude < MIN_LAT) || (*Latitude > U_MAX_LAT))
    { /* Latitude out of range */
      Error_Code |= UTM_NORTHING_ERROR;
    }
  }
  return (Error_Code);
} /* END OF Convert_UTM_To_Geodetic */


/************************************************************************/
/*                  Tranmerc  FUNCTIONS     
 *
 */


int Mgrs::Set_Transverse_Mercator_Parameters(double a,
                                        double b,
                                        double Origin_Latitude,
                                        double Central_Meridian,
                                        double False_Easting,
                                        double False_Northing,
                                        double Scale_Factor)

{ /* BEGIN Set_Tranverse_Mercator_Parameters */
  /*
   * The function Set_Tranverse_Mercator_Parameters receives the ellipsoid
   * parameters and Tranverse Mercator projection parameters as inputs, and
   * sets the corresponding state variables. If any errors occur, the error
   * code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    a                 : Semi-major axis of ellipsoid, in meters    (input)
   *    b                 : Semi-minor axis of ellipsoid, in meters    (input)
   *    Origin_Latitude   : Latitude in radians at the origin of the   (input)
   *                         projection
   *    Central_Meridian  : Longitude in radians at the center of the  (input)
   *                         projection
   *    False_Easting     : Easting/X at the center of the projection  (input)
   *    False_Northing    : Northing/Y at the center of the projection (input)
   *    Scale_Factor      : Projection scale factor                    (input) 
   */

  double tn;        /* True Meridional distance constant  */
  double tn2;
  double tn3;
  double tn4;
  double tn5;
  double a2;
  double b2;
  int Error_Code = TRANMERC_NO_ERROR;
  double dummy_northing;

  if (a <= 0.0)
  { /* Semi-major axis must be greater than zero */
    Error_Code |= TRANMERC_A_ERROR;
  }
  if (b <= 0.0)
  { /* Semi-minor must be greater than zero */
    Error_Code |= TRANMERC_B_ERROR;
  }
  if (a < b)
  { /* Semi-major axis can not be less than Semi-minor axis */
    Error_Code |= TRANMERC_A_LESS_B_ERROR;
  }
  if ((Origin_Latitude < -T_MAX_LAT) || (Origin_Latitude > T_MAX_LAT))
  { /* origin latitude out of range */
    Error_Code |= TRANMERC_ORIGIN_LAT_ERROR;
  }
  if ((Central_Meridian < -PI) || (Central_Meridian > (2*PI)))
  { /* origin longitude out of range */
    Error_Code |= TRANMERC_CENT_MER_ERROR;
  }
  if ((Scale_Factor < MIN_SCALE_FACTOR) || (Scale_Factor > MAX_SCALE_FACTOR))
  {
    Error_Code |= TRANMERC_SCALE_FACTOR_ERROR;
  }
  if (!Error_Code)
  { /* no errors */
    TranMerc_a = a;
    TranMerc_b = b;
    TranMerc_Origin_Lat = 0;
    TranMerc_Origin_Long = 0;
    TranMerc_False_Northing = 0;
    TranMerc_False_Easting = 0; 
    TranMerc_Scale_Factor = 1;

    /* Eccentricity Squared */
    a2 = a * a;
    b2 = b * b;
    TranMerc_es = (a2 - b2) / a2;
    /* Second Eccentricity Squared */
    TranMerc_ebs = (a2 - b2) / b2;

    /*True meridional constants  */
    tn = (a - b) / (a + b);
    tn2 = tn * tn;
    tn3 = tn2 * tn;
    tn4 = tn3 * tn;
    tn5 = tn4 * tn;

    TranMerc_ap = a * (1.e0 - tn + 5.e0 * (tn2 - tn3)/4.e0
                       + 81.e0 * (tn4 - tn5)/64.e0 );
    TranMerc_bp = 3.e0 * a * (tn - tn2 + 7.e0 * (tn3 - tn4)
                              /8.e0 + 55.e0 * tn5/64.e0 )/2.e0;
    TranMerc_cp = 15.e0 * a * (tn2 - tn3 + 3.e0 * (tn4 - tn5 )/4.e0) /16.0;
    TranMerc_dp = 35.e0 * a * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
    TranMerc_ep = 315.e0 * a * (tn4 - tn5) / 512.e0;
    Convert_Geodetic_To_Transverse_Mercator(T_MAX_LAT,
                                            MAX_DELTA_LONG,
                                            &TranMerc_Delta_Easting,
                                            &TranMerc_Delta_Northing);
    Convert_Geodetic_To_Transverse_Mercator(0,
                                            MAX_DELTA_LONG,
                                            &TranMerc_Delta_Easting,
                                            &dummy_northing);
    TranMerc_Origin_Lat = Origin_Latitude;
    if (Central_Meridian > PI)
      Central_Meridian -= (2*PI);
    TranMerc_Origin_Long = Central_Meridian;
    TranMerc_False_Northing = False_Northing;
    TranMerc_False_Easting = False_Easting; 
    TranMerc_Scale_Factor = Scale_Factor;
  } /* END OF if(!Error_Code) */
  return (Error_Code);
}  /* END of Set_Transverse_Mercator_Parameters  */


void Mgrs::Get_Transverse_Mercator_Parameters(double *a,
                                        double *b,
                                        double *Origin_Latitude,
                                        double *Central_Meridian,
                                        double *False_Easting,
                                        double *False_Northing,
                                        double *Scale_Factor)

{ /* BEGIN Get_Tranverse_Mercator_Parameters  */
  /*
   * The function Get_Transverse_Mercator_Parameters returns the current
   * ellipsoid and Transverse Mercator projection parameters.
   *
   *    a                 : Semi-major axis of ellipsoid, in meters    (output)
   *    b                 : Semi-minor axis of ellipsoid, in meters    (output)
   *    Origin_Latitude   : Latitude in radians at the origin of the   (output)
   *                         projection
   *    Central_Meridian  : Longitude in radians at the center of the  (output)
   *                         projection
   *    False_Easting     : Easting/X at the center of the projection  (output)
   *    False_Northing    : Northing/Y at the center of the projection (output)
   *    Scale_Factor      : Projection scale factor                    (output) 
   */

  *a = TranMerc_a;
  *b = TranMerc_b;
  *Origin_Latitude = TranMerc_Origin_Lat;
  *Central_Meridian = TranMerc_Origin_Long;
  *False_Easting = TranMerc_False_Easting;
  *False_Northing = TranMerc_False_Northing;
  *Scale_Factor = TranMerc_Scale_Factor;
  return;
} /* END OF Get_Tranverse_Mercator_Parameters */



int Mgrs::Convert_Geodetic_To_Transverse_Mercator (double Latitude,
                                              double Longitude,
                                              double *Easting,
                                              double *Northing)

{      /* BEGIN Convert_Geodetic_To_Transverse_Mercator */

  /*
   * The function Convert_Geodetic_To_Transverse_Mercator converts geodetic
   * (latitude and longitude) coordinates to Transverse Mercator projection
   * (easting and northing) coordinates, according to the current ellipsoid
   * and Transverse Mercator projection coordinates.  If any errors occur, the
   * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    Latitude      : Latitude in radians                         (input)
   *    Longitude     : Longitude in radians                        (input)
   *    Easting       : Easting/X in meters                         (output)
   *    Northing      : Northing/Y in meters                        (output)
   */

  double c;       /* Cosine of latitude                          */
  double c2;
  double c3;
  double c5;
  double c7;
  double dlam;    /* Delta longitude - Difference in Longitude       */
  double eta;     /* constant - TranMerc_ebs *c *c                   */
  double eta2;
  double eta3;
  double eta4;
  double s;       /* Sine of latitude                        */
  double sn;      /* Radius of curvature in the prime vertical       */
  double t;       /* Tangent of latitude                             */
  double tan2;
  double tan3;
  double tan4;
  double tan5;
  double tan6;
  double t1;      /* Term in coordinate conversion formula - GP to Y */
  double t2;      /* Term in coordinate conversion formula - GP to Y */
  double t3;      /* Term in coordinate conversion formula - GP to Y */
  double t4;      /* Term in coordinate conversion formula - GP to Y */
  double t5;      /* Term in coordinate conversion formula - GP to Y */
  double t6;      /* Term in coordinate conversion formula - GP to Y */
  double t7;      /* Term in coordinate conversion formula - GP to Y */
  double t8;      /* Term in coordinate conversion formula - GP to Y */
  double t9;      /* Term in coordinate conversion formula - GP to Y */
  double tmd;     /* True Meridional distance                        */
  double tmdo;    /* True Meridional distance for latitude of origin */
  int    Error_Code = TRANMERC_NO_ERROR;
  double temp_Origin;
  double temp_Long;

  if ((Latitude < -T_MAX_LAT) || (Latitude > T_MAX_LAT))
  {  /* Latitude out of range */
    Error_Code|= TRANMERC_LAT_ERROR;
  }
  if (Longitude > PI)
    Longitude -= (2 * PI);
  if ((Longitude < (TranMerc_Origin_Long - MAX_DELTA_LONG))
      || (Longitude > (TranMerc_Origin_Long + MAX_DELTA_LONG)))
  {
    if (Longitude < 0)
      temp_Long = Longitude + 2 * PI;
    else
      temp_Long = Longitude;
    if (TranMerc_Origin_Long < 0)
      temp_Origin = TranMerc_Origin_Long + 2 * PI;
    else
      temp_Origin = TranMerc_Origin_Long;
    if ((temp_Long < (temp_Origin - MAX_DELTA_LONG))
        || (temp_Long > (temp_Origin + MAX_DELTA_LONG)))
      Error_Code|= TRANMERC_LON_ERROR;
  }
  if (!Error_Code)
  { /* no errors */

    /* 
     *  Delta Longitude
     */
    dlam = Longitude - TranMerc_Origin_Long;

    if (fabs(dlam) > (9.0 * PI / 180))
    { /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
      Error_Code |= TRANMERC_LON_WARNING;
    }

    if (dlam > PI)
      dlam -= (2 * PI);
    if (dlam < -PI)
      dlam += (2 * PI);
    if (fabs(dlam) < 2.e-10)
      dlam = 0.0;

    s = sin(Latitude);
    c = cos(Latitude);
    c2 = c * c;
    c3 = c2 * c;
    c5 = c3 * c2;
    c7 = c5 * c2;
    t = tan (Latitude);
    tan2 = t * t;
    tan3 = tan2 * t;
    tan4 = tan3 * t;
    tan5 = tan4 * t;
    tan6 = tan5 * t;
    eta = TranMerc_ebs * c2;
    eta2 = eta * eta;
    eta3 = eta2 * eta;
    eta4 = eta3 * eta;

    /* radius of curvature in prime vertical */
    sn = SPHSN(Latitude);

    /* True Meridonal Distances */
    tmd = SPHTMD(Latitude);

    /*  Origin  */
    tmdo = SPHTMD (TranMerc_Origin_Lat);

    /* northing */
    t1 = (tmd - tmdo) * TranMerc_Scale_Factor;
    t2 = sn * s * c * TranMerc_Scale_Factor/ 2.e0;
    t3 = sn * s * c3 * TranMerc_Scale_Factor * (5.e0 - tan2 + 9.e0 * eta 
                                                + 4.e0 * eta2) /24.e0; 

    t4 = sn * s * c5 * TranMerc_Scale_Factor * (61.e0 - 58.e0 * tan2
                                                + tan4 + 270.e0 * eta - 330.e0 * tan2 * eta + 445.e0 * eta2
                                                + 324.e0 * eta3 -680.e0 * tan2 * eta2 + 88.e0 * eta4 
                                                -600.e0 * tan2 * eta3 - 192.e0 * tan2 * eta4) / 720.e0;

    t5 = sn * s * c7 * TranMerc_Scale_Factor * (1385.e0 - 3111.e0 * 
                                                tan2 + 543.e0 * tan4 - tan6) / 40320.e0;

    *Northing = TranMerc_False_Northing + t1 + pow(dlam,2.e0) * t2
                + pow(dlam,4.e0) * t3 + pow(dlam,6.e0) * t4
                + pow(dlam,8.e0) * t5; 

    /* Easting */
    t6 = sn * c * TranMerc_Scale_Factor;
    t7 = sn * c3 * TranMerc_Scale_Factor * (1.e0 - tan2 + eta ) /6.e0;
    t8 = sn * c5 * TranMerc_Scale_Factor * (5.e0 - 18.e0 * tan2 + tan4
                                            + 14.e0 * eta - 58.e0 * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 
                                            - 64.e0 * tan2 * eta2 - 24.e0 * tan2 * eta3 )/ 120.e0;
    t9 = sn * c7 * TranMerc_Scale_Factor * ( 61.e0 - 479.e0 * tan2
                                             + 179.e0 * tan4 - tan6 ) /5040.e0;

    *Easting = TranMerc_False_Easting + dlam * t6 + pow(dlam,3.e0) * t7 
               + pow(dlam,5.e0) * t8 + pow(dlam,7.e0) * t9;
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Transverse_Mercator */


int Mgrs::Convert_Transverse_Mercator_To_Geodetic (
                                             double Easting,
                                             double Northing,
                                             double *Latitude,
                                             double *Longitude)
{      /* BEGIN Convert_Transverse_Mercator_To_Geodetic */

  /*
   * The function Convert_Transverse_Mercator_To_Geodetic converts Transverse
   * Mercator projection (easting and northing) coordinates to geodetic
   * (latitude and longitude) coordinates, according to the current ellipsoid
   * and Transverse Mercator projection parameters.  If any errors occur, the
   * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    Easting       : Easting/X in meters                         (input)
   *    Northing      : Northing/Y in meters                        (input)
   *    Latitude      : Latitude in radians                         (output)
   *    Longitude     : Longitude in radians                        (output)
   */

  double c;       /* Cosine of latitude                          */
  double de;      /* Delta easting - Difference in Easting (Easting-Fe)    */
  double dlam;    /* Delta longitude - Difference in Longitude       */
  double eta;     /* constant - TranMerc_ebs *c *c                   */
  double eta2;
  double eta3;
  double eta4;
  double ftphi;   /* Footpoint latitude                              */
  int    i;       /* Loop iterator                   */
  double s;       /* Sine of latitude                        */
  double sn;      /* Radius of curvature in the prime vertical       */
  double sr;      /* Radius of curvature in the meridian             */
  double t;       /* Tangent of latitude                             */
  double tan2;
  double tan4;
  double t10;     /* Term in coordinate conversion formula - GP to Y */
  double t11;     /* Term in coordinate conversion formula - GP to Y */
  double t12;     /* Term in coordinate conversion formula - GP to Y */
  double t13;     /* Term in coordinate conversion formula - GP to Y */
  double t14;     /* Term in coordinate conversion formula - GP to Y */
  double t15;     /* Term in coordinate conversion formula - GP to Y */
  double t16;     /* Term in coordinate conversion formula - GP to Y */
  double t17;     /* Term in coordinate conversion formula - GP to Y */
  double tmd;     /* True Meridional distance                        */
  double tmdo;    /* True Meridional distance for latitude of origin */
  int Error_Code = TRANMERC_NO_ERROR;

  if ((Easting < (TranMerc_False_Easting - TranMerc_Delta_Easting))
      ||(Easting > (TranMerc_False_Easting + TranMerc_Delta_Easting)))
  { /* Easting out of range  */
    Error_Code |= TRANMERC_EASTING_ERROR;
  }
  if ((Northing < (TranMerc_False_Northing - TranMerc_Delta_Northing))
      || (Northing > (TranMerc_False_Northing + TranMerc_Delta_Northing)))
  { /* Northing out of range */
    Error_Code |= TRANMERC_NORTHING_ERROR;
  }

  if (!Error_Code)
  {
    /* True Meridional Distances for latitude of origin */
    tmdo = SPHTMD(TranMerc_Origin_Lat);

    /*  Origin  */
    tmd = tmdo +  (Northing - TranMerc_False_Northing) / TranMerc_Scale_Factor; 

    /* First Estimate */
    sr = SPHSR(0.e0);
    ftphi = tmd/sr;

    for (i = 0; i < 5 ; i++)
    {
      t10 = SPHTMD (ftphi);
      sr = SPHSR(ftphi);
      ftphi = ftphi + (tmd - t10) / sr;
    }

    /* Radius of Curvature in the meridian */
    sr = SPHSR(ftphi);

    /* Radius of Curvature in the meridian */
    sn = SPHSN(ftphi);

    /* Sine Cosine terms */
    s = sin(ftphi);
    c = cos(ftphi);

    /* Tangent Value  */
    t = tan(ftphi);
    tan2 = t * t;
    tan4 = tan2 * tan2;
    eta = TranMerc_ebs * pow(c,2);
    eta2 = eta * eta;
    eta3 = eta2 * eta;
    eta4 = eta3 * eta;
    de = Easting - TranMerc_False_Easting;
    if (fabs(de) < 0.0001)
      de = 0.0;

    /* Latitude */
    t10 = t / (2.e0 * sr * sn * pow(TranMerc_Scale_Factor, 2));
    t11 = t * (5.e0  + 3.e0 * tan2 + eta - 4.e0 * pow(eta,2)
               - 9.e0 * tan2 * eta) / (24.e0 * sr * pow(sn,3) 
                                       * pow(TranMerc_Scale_Factor,4));
    t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4
               - 252.e0 * tan2 * eta  - 3.e0 * eta2 + 100.e0 
               * eta3 - 66.e0 * tan2 * eta2 - 90.e0 * tan4
               * eta + 88.e0 * eta4 + 225.e0 * tan4 * eta2
               + 84.e0 * tan2* eta3 - 192.e0 * tan2 * eta4)
          / ( 720.e0 * sr * pow(sn,5) * pow(TranMerc_Scale_Factor, 6) );
    t13 = t * ( 1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0 
                * pow(t,6))/ (40320.e0 * sr * pow(sn,7) * pow(TranMerc_Scale_Factor,8));
    *Latitude = ftphi - pow(de,2) * t10 + pow(de,4) * t11 - pow(de,6) * t12 
                + pow(de,8) * t13;

    t14 = 1.e0 / (sn * c * TranMerc_Scale_Factor);

    t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn,3) * c * 
                                        pow(TranMerc_Scale_Factor,3));

    t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2
           + 8.e0 * tan2 * eta + 24.e0 * tan4 - 4.e0 
           * eta3 + 4.e0 * tan2 * eta2 + 24.e0 
           * tan2 * eta3) / (120.e0 * pow(sn,5) * c  
                             * pow(TranMerc_Scale_Factor,5));

    t17 = (61.e0 +  662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 
           * pow(t,6)) / (5040.e0 * pow(sn,7) * c 
                          * pow(TranMerc_Scale_Factor,7));

    /* Difference in Longitude */
    dlam = de * t14 - pow(de,3) * t15 + pow(de,5) * t16 - pow(de,7) * t17;

    /* Longitude */
    (*Longitude) = TranMerc_Origin_Long + dlam;
    while (*Latitude > (90.0 * PI / 180.0))
    {
      *Latitude = PI - *Latitude;
      *Longitude += PI;
      if (*Longitude > PI)
        *Longitude -= (2 * PI);
    }

    while (*Latitude < (-90.0 * PI / 180.0))
    {
      *Latitude = - (*Latitude + PI);
      *Longitude += PI;
      if (*Longitude > PI)
        *Longitude -= (2 * PI);
    }
    if (*Longitude > (2*PI))
      *Longitude -= (2 * PI);
    if (*Longitude < -PI)
      *Longitude += (2 * PI);

    if (fabs(dlam) > (9.0 * PI / 180))
    { /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
      Error_Code |= TRANMERC_LON_WARNING;
    }
  }
  return (Error_Code);
} /* END OF Convert_Transverse_Mercator_To_Geodetic */

 
