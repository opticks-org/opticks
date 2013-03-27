/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MGRS_H
#define MGRS_H

#include "AppConfig.h"

class Mgrs
{
public:

   static Mgrs *Instance();
private:

   static Mgrs *mpSingleton;
   /**
    * Constructor for Mgrs
    * 
    * Constructor for Military Grid Reference System (MGRS)
    *
    * @param   n/a
    *          
   **/
   Mgrs();

   //destructor
   ~Mgrs(){};

   

/***************************************************************************/
/*
 *                        MGRS GLOBAL DECLARATIONS
 */
#define DEGRAD       0.017453292519943295 /* PI/180                          */
#define R3           0.052359877559829890 /* RADIANS FOR  3 DEGREES          */ 
#define R8           0.139626340159546400 /* RADIANS FOR  8 DEGREES          */
#define R9           0.157079632679489700 /* RADIANS FOR  9 DEGREES          */
#define R21          0.366519142918809200 /* RADIANS FOR  21 DEGREES         */
#define R33          0.575958653158128800 /* RADIANS FOR  33 DEGREES         */
#define R56          0.977384381116824600 /* RADIANS FOR  56 DEGREES         */
#define R64          1.117010721276371000 /* RADIANS FOR  64 DEGREES         */
#define R72          1.256637061435917000 /* RADIANS FOR  72 DEGREES         */
#define R80          1.396263401595464000 /* RADIANS FOR  80 DEGREES         */
#define UPS_SOUTH              3    /* UPS COORDINATE IN SOUTHERN HEMISPHERE */
#define UPS_NORTH              2    /* UPS COORDINATE IN NORTHERN HEMISPHERE */ 
//#define UTM                    1    /* UTM COORDINATE                        */ 
#define ALBET                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ" /* ALPHABET       */
#define LETTER_A               0   /* ARRAY INDEX FOR LETTER A               */
#define LETTER_B               1   /* ARRAY INDEX FOR LETTER B               */
#define LETTER_C               2   /* ARRAY INDEX FOR LETTER C               */
#define LETTER_D               3   /* ARRAY INDEX FOR LETTER D               */
#define LETTER_E               4   /* ARRAY INDEX FOR LETTER E               */
#define LETTER_H               7   /* ARRAY INDEX FOR LETTER H               */
#define LETTER_I               8   /* ARRAY INDEX FOR LETTER I               */
#define LETTER_J               9   /* ARRAY INDEX FOR LETTER J               */
#define LETTER_L              11   /* ARRAY INDEX FOR LETTER L               */
#define LETTER_M              12   /* ARRAY INDEX FOR LETTER M               */
#define LETTER_N              13   /* ARRAY INDEX FOR LETTER N               */
#define LETTER_O              14   /* ARRAY INDEX FOR LETTER O               */
#define LETTER_P              15   /* ARRAY INDEX FOR LETTER P               */
#define LETTER_Q              16   /* ARRAY INDEX FOR LETTER Q               */
#define LETTER_R              17   /* ARRAY INDEX FOR LETTER R               */
#define LETTER_S              18   /* ARRAY INDEX FOR LETTER S               */
#define LETTER_U              20   /* ARRAY INDEX FOR LETTER U               */
#define LETTER_V              21   /* ARRAY INDEX FOR LETTER V               */
#define LETTER_W              22   /* ARRAY INDEX FOR LETTER W               */
#define LETTER_X              23   /* ARRAY INDEX FOR LETTER X               */
#define LETTER_Y              24   /* ARRAY INDEX FOR LETTER Y               */
#define LETTER_Z              25   /* ARRAY INDEX FOR LETTER Z               */
#define RND1                  0.1e0  /* ROUND TO NEAREST .1            */
#define RND5                  0.5e0  /* ROUND TO NEAREST .5            */
#define EOLN                   '\0'  /* END OF STRING CHARACTER        */
#define BLANK                   ' '  /* BLANK CHARACTER                */
#define MGRS_LETTERS            3  /* NUMBER OF LETTERS IN MGRS              */
#define NUM_OFFSET             48  /* USED TO CONVERT NUMBERS TO LETTERS     */
#define ONEHT          100000.e0    /* ONE HUNDRED THOUSAND                  */
#define TWOMIL        2000000.e0    /* TWO MILLION                           */
#define EIGHT          800000.e0    /* EIGHT HUNDRED THOUSAND                */
#define ONE3HT        1300000.e0    /* ONE MILLION THREE HUNDRED THOUSAND    */
#define ZERO                   0.e0  /* ZERO                           */
#define TEN                10.e0    /* TEN                                   */
#define TRUE                      1  /* CONSTANT VALUE FOR TRUE VALUE  */
#define FALSE                     0  /* CONSTANT VALUE FOR FALSE VALUE */
#define PI_OVER_2  (PI / 2.0e0)
#define NUM                   "01234567890"                /* NUMBERS        */
#define MAXALBET              25   /* LAST INDEX IN ALPHABET ARRAY(0-25)     */
#define MAXNUMBER             10   /* LAST INDEX IN NUMBER ARRAY(0-9)        */
#define MGRS_ZONE_AND_LETTERS   5  /* NUM. OF CHARS. IN ZONE AND LETTERS     */
#define MGRS_MINIMUM            9  /* MINIMUM NUMBER OF CHARS FOR MGRS       */
#define MGRS_MAXIMUM           15  /* MAXIMUM NUMBER OF CHARS FOR MGRS       */

#define MIN_EASTING  100000
#define MAX_EASTING  900000
#define MIN_NORTHING 0
#define MAX_NORTHING 10000000
#define MAX_PRECISION           5   /* Maximum precision of easting & northing */
#define MIN_UTM_LAT      ( (-80 * PI) / 180.0 ) /* -80 degrees in radians    */
#define MAX_UTM_LAT      ( (84 * PI) / 180.0 )  /* 84 degrees in radians     */

#define MIN_EAST_NORTH 0
#define MAX_EAST_NORTH 4000000

/***************************************************************************/
/*
 *                        UTM GLOBAL DECLARATIONS
 */
#define MIN_LAT      ( (-80.5 * PI) / 180.0 ) /* -80.5 degrees in radians    */
#define U_MAX_LAT      ( (84.5 * PI) / 180.0 )  /* 84.5 degrees in radians     */
#define MIN_EASTING  100000
#define MAX_EASTING  900000
#define MIN_NORTHING 0
#define MAX_NORTHING 10000000

/***************************************************************************/
/*                      TRANMERC DEFINES 
 *
 */
#define PI_OVER         (PI/2.0e0)            /* PI over 2 */
#define T_MAX_LAT         ((PI * 90)/180.0)    /* 90 degrees in radians */
#define MAX_DELTA_LONG  ((PI * 90)/180.0)    /* 90 degrees in radians */
#define MIN_SCALE_FACTOR  0.3
#define MAX_SCALE_FACTOR  3.0

#define SPHTMD(Latitude) ((double) (TranMerc_ap * Latitude \
      - TranMerc_bp * sin(2.e0 * Latitude) + TranMerc_cp * sin(4.e0 * Latitude) \
      - TranMerc_dp * sin(6.e0 * Latitude) + TranMerc_ep * sin(8.e0 * Latitude) ) )

#define SPHSN(Latitude) ((double) (TranMerc_a / sqrt( 1.e0 - TranMerc_es * \
      pow(sin(Latitude), 2))))

#define SPHSR(Latitude) ((double) (TranMerc_a * (1.e0 - TranMerc_es) / \
    pow(DENOM(Latitude), 3)))

#define DENOM(Latitude) ((double) (sqrt(1.e0 - TranMerc_es * pow(sin(Latitude),2))))
   
/***************************************************************************/
/*
 *                       MGRS Error DEFINES
 */

#define MGRS_NO_ERROR                0x0000
#define MGRS_LAT_ERROR               0x0001
#define MGRS_LON_ERROR               0x0002
#define MGRS_STRING_ERROR            0x0004
#define MGRS_PRECISION_ERROR         0x0008
#define MGRS_A_ERROR                 0x0010
#define MGRS_B_ERROR                 0x0020
#define MGRS_A_LESS_B_ERROR          0x0040
#define MGRS_EASTING_ERROR           0x0080
#define MGRS_NORTHING_ERROR          0x0100
#define MGRS_ZONE_ERROR              0x0200
#define MGRS_HEMISPHERE_ERROR        0x0400

/***************************************************************************/
/*
 *                       UTM Error DEFINES
 */
#define UTM_NO_ERROR            0x0000
#define UTM_LAT_ERROR           0x0001
#define UTM_LON_ERROR           0x0002
#define UTM_EASTING_ERROR       0x0004
#define UTM_NORTHING_ERROR      0x0008
#define UTM_ZONE_ERROR          0x0010
#define UTM_HEMISPHERE_ERROR    0x0020
#define UTM_ZONE_OVERRIDE_ERROR 0x0040
#define UTM_A_ERROR             0x0080
#define UTM_B_ERROR             0x0100
#define UTM_A_LESS_B_ERROR      0x0200

/***************************************************************************/
/*
 *                   TRANMERC Error DEFINES
 */
#define TRANMERC_NO_ERROR           0x0000
#define TRANMERC_LAT_ERROR          0x0001
#define TRANMERC_LON_ERROR          0x0002
#define TRANMERC_EASTING_ERROR      0x0004
#define TRANMERC_NORTHING_ERROR     0x0008
#define TRANMERC_ORIGIN_LAT_ERROR   0x0010
#define TRANMERC_CENT_MER_ERROR     0x0020
#define TRANMERC_A_ERROR            0x0040
#define TRANMERC_B_ERROR            0x0080
#define TRANMERC_A_LESS_B_ERROR     0x0100
#define TRANMERC_SCALE_FACTOR_ERROR 0x0200
#define TRANMERC_LON_WARNING        0x0400

   
/* Ellipsoid parameters, default to WGS 84 */
/*
double MGRS_a = 6378137.0;    // Semi-major axis of ellipsoid in meters 
double MGRS_b = 6356752.3142; // Semi-minor axis of ellipsoid           
double MGRS_recpf = 1 / ((6378137.0 - 6356752.3142) / 6378137.0);
char   MGRS_Ellipsoid_Code[3] = {'W','E',0};

const char* CLARKE_1866 = "CC";
const char* CLARKE_1880 = "CD";
const char* BESSEL_1841 = "BR";
*/

static double MGRS_a;    // Semi-major axis of ellipsoid in meters 
static double MGRS_b; // Semi-minor axis of ellipsoid           
static double MGRS_recpf;
static char   MGRS_Ellipsoid_Code[3]; //= {'W','E',0}; 

static const char* CLARKE_1866;
static const char* CLARKE_1880;
static const char* BESSEL_1841;


public:
/***************************************************************************/
/*
 *                  MGRS   FUNCTION PROTOTYPES
 */

/* ensure proper linkage to c++ programs */
  
  int Set_MGRS_Parameters(double a,
                           double b,
                           char   *Ellipsoid_Code);
/*
 * The function Set_MGRS_Parameters receives the ellipsoid parameters and sets
 * the corresponding state variables. If any errors occur, the error code(s)
 * are returned by the function, otherwise MGRS_NO_ERROR is returned.
 *
 *   a                : Semi-major axis of ellipsoid in meters (input)
 *   b                : Semi-minor axis of ellipsoid in meters (input)
 *   Ellipsoid_Code   : 2-letter code for ellipsoid            (input)
 */


  void Get_MGRS_Parameters(double *a,
                           double *b,
                           char   *Ellipsoid_Code);
/*
 * The function Get_MGRS_Parameters returns the current ellipsoid
 * parameters.
 *
 *  a                :Semi-major axis of ellipsoid, in meters (output)
 *  b                :Semi_minor axis of ellipsoid, in meters (output)
 *  Ellipsoid_Code   : 2-letter code for ellipsoid            (output)
 */


  int Convert_Geodetic_To_MGRS(double Latitude,
                                 double Longitude,
                                 int   Precision,
                                 char *MGRS);
/*
 * The function Convert_Geodetic_To_MGRS converts geodetic (latitude and
 * longitude) coordinates to an MGRS coordinate string, according to the 
 * current ellipsoid parameters.  If any errors occur, the error code(s) 
 * are returned by the  function, otherwise MGRS_NO_ERROR is returned.
 *
 *    Latitude   : Latitude in radians              (input)
 *    Longitude  : Longitude in radians             (input)
 *    Precision  : Precision level of MGRS string   (input)
 *    MGRS       : MGRS coordinate string           (output)
 *  
 */


  int Convert_MGRS_To_Geodetic(char *MGRS,
                                 double *Latitude,
                                 double *Longitude);
/*
 * This function converts an MGRS coordinate string to Geodetic (latitude
 * and longitude in radians) coordinates.  If any errors occur, the error 
 * code(s) are returned by the  function, otherwise MGRS_NO_ERROR is returned.  
 *
 *    MGRS       : MGRS coordinate string           (input)
 *    Latitude   : Latitude in radians              (output)
 *    Longitude  : Longitude in radians             (output)
 *  
 */


  int Convert_UTM_To_MGRS(int Zone,
                            char Hemisphere,
                            double Easting,
                            double Northing,
                            int Precision,
                            char *MGRS);
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


  int Convert_MGRS_To_UTM(char   *MGRS,
                            int   *Zone,
                            char   *Hemisphere,
                            double *Easting,
                            double *Northing); 
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



  int Convert_UPS_To_MGRS( char   Hemisphere,
                             double Easting,
                             double Northing,
                             int Precision,
                             char *MGRS);


   void UTMSET(int izone, 
             int* ltrlow, 
             int* ltrhi, 
             double *fnltr);
   void UTMLIM(int* n, 
             double sphi, 
             int izone, 
             double *spsou, 
             double *spnor,
             double *sleast, 
             double *slwest);
   void UTMMGRS(int izone,
              int* ltrnum,
              double sphi,
              double x,
              double y);
   void UPS(char* mgrs,
          int* ltrnum,
          double x,
          double y,
          int iarea);
   void UPSSET(int n, 
             int* ltrlow, 
             int* ltrhi, 
             double *feltr,
             double *fnltr, 
             int* ltrhy);
   void LTR2UPS(int* ltrnum, 
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
              double sign);
   void GRID_UPS(int   *Letters,
               char   *Hemisphere,
               double *Easting,
               double *Northing,
               int   *Error);
   void LTR2UTM(int* ltrnum, 
              int ltrlow, 
              int ltrhi, 
              int* ierr, 
              double *xltr, 
              double *yltr, 
              double fnltr, 
              double yslow, 
              double ylow);
   void GRID_UTM(int   *Zone,
               int   *Letters,
               char   *Hemisphere,
               double *Easting,
               double *Northing,
               int   In_Precision,
               int   *Error);
   int Round_MGRS(double value);
   int Make_MGRS_String(char* MGRS, 
                       int Zone, 
                       int ltrnum[MGRS_LETTERS], 
                       double Easting, 
                       double Northing,
                       int Precision);
   int Break_MGRS_String (char* MGRS,
                        int* Zone,
                        int Letters[MGRS_LETTERS],
                        double* Easting,
                        double* Northing,
                        int* Precision);


private:
/***************************************************************************/
/*
 *                         UTM  GLOBAL DECLARATIONS
 */

static double  UTM_a;// = 6378137.0;    // Semi-major axis of ellipsoid in meters  
static double  UTM_b;// = 6356752.3142; // Semi-minor axis of ellipsoid            
static int   UTM_Override;// = 0;     // Zone override flag                      

/***************************************************************************/
/* RSC IDENTIFIER: UTM
 *
 * ABSTRACT
 *
 *    This component provides conversions between geodetic coordinates 
 *    (latitude and longitudes) and Universal Transverse Mercator (UTM)
 *    projection (zone, hemisphere, easting, and northing) coordinates.
 *
 * ERROR HANDLING
 *
 *    This component checks parameters for valid values.  If an invalid value
 *    is found, the error code is combined with the current error code using 
 *    the bitwise or.  This combining allows multiple error codes to be
 *    returned. The possible error codes are:
 *
 *          UTM_NO_ERROR           : No errors occurred in function
 *          UTM_LAT_ERROR          : Latitude outside of valid range
 *                                    (-80.5 to 84.5 degrees)
 *          UTM_LON_ERROR          : Longitude outside of valid range
 *                                    (-180 to 360 degrees)
 *          UTM_EASTING_ERROR      : Easting outside of valid range
 *                                    (100,000 to 900,000 meters)
 *          UTM_NORTHING_ERROR     : Northing outside of valid range
 *                                    (0 to 10,000,000 meters)
 *          UTM_ZONE_ERROR         : Zone outside of valid range (1 to 60)
 *          UTM_HEMISPHERE_ERROR   : Invalid hemisphere ('N' or 'S')
 *          UTM_ZONE_OVERRIDE_ERROR: Zone outside of valid range
 *                                    (1 to 60) and within 1 of 'natural' zone
 *          UTM_A_ERROR            : Semi-major axis less than or equal to zero
 *          UTM_B_ERROR            : Semi-minor axis less than or equal to zero
 *          UTM_A_LESS_B_ERROR     : Semi-major axis less than semi-minor axis
 *
 * REUSE NOTES
 *
 *    UTM is intended for reuse by any application that performs a Universal
 *    Transverse Mercator (UTM) projection or its inverse.
 *    
 * REFERENCES
 *
 *    Further information on UTM can be found in the Reuse Manual.
 *
 *    UTM originated from :  U.S. Army Topographic Engineering Center
 *                           Geospatial Information Division
 *                           7701 Telegraph Road
 *                           Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    UTM has no restrictions.
 *
 * ENVIRONMENT
 *
 *    UTM was tested and certified in the following environments:
 *
 *    1. Solaris 2.5 with GCC, version 2.8.1
 *    2. MSDOS with MS Visual C++, version 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    10-02-97          Original Code
 *
 */


public:
/***************************************************************************/
/*
 *                              FUNCTION PROTOTYPES
 *                                for UTM.C
 */

/* ensure proper linkage to c++ programs */
  int Set_UTM_Parameters(double a,      
                          double b,
                          int   override);
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


  void Get_UTM_Parameters(double *a,
                          double *b,
                          int   *override);
/*
 * The function Get_UTM_Parameters returns the current ellipsoid
 * parameters and UTM zone override parameter.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters       (output)
 *    b                 : Semi-minor axis of ellipsoid, in meters       (output)
 *    override          : UTM override zone, zero indicates no override (output)
 */


  int Convert_Geodetic_To_UTM (double Latitude,
                                double Longitude,
                                int   *Zone,
                                char   *Hemisphere,
                                double *Easting,
                                double *Northing); 
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


  int Convert_UTM_To_Geodetic(int   Zone,
                               char   Hemisphere,
                               double Easting,
                               double Northing,
                               double *Latitude,
                               double *Longitude);
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


//Tranmeric declarations

/***************************************************************************/
/* RSC IDENTIFIER: TRANSVERSE MERCATOR
 *
 * ABSTRACT
 *
 *    This component provides conversions between Geodetic coordinates 
 *    (latitude and longitude) and Transverse Mercator projection coordinates
 *    (easting and northing).
 *
 * ERROR HANDLING
 *
 *    This component checks parameters for valid values.  If an invalid value
 *    is found the error code is combined with the current error code using 
 *    the bitwise or.  This combining allows multiple error codes to be
 *    returned. The possible error codes are:
 *
 *       TRANMERC_NO_ERROR           : No errors occurred in function
 *       TRANMERC_LAT_ERROR          : Latitude outside of valid range
 *                                      (-90 to 90 degrees)
 *       TRANMERC_LON_ERROR          : Longitude outside of valid range
 *                                      (-180 to 360 degrees, and within
 *                                        +/-90 of Central Meridian)
 *       TRANMERC_EASTING_ERROR      : Easting outside of valid range
 *                                      (depending on ellipsoid and
 *                                       projection parameters)
 *       TRANMERC_NORTHING_ERROR     : Northing outside of valid range
 *                                      (depending on ellipsoid and
 *                                       projection parameters)
 *       TRANMERC_ORIGIN_LAT_ERROR   : Origin latitude outside of valid range
 *                                      (-90 to 90 degrees)
 *       TRANMERC_CENT_MER_ERROR     : Central meridian outside of valid range
 *                                      (-180 to 360 degrees)
 *       TRANMERC_A_ERROR            : Semi-major axis less than or equal to zero
 *       TRANMERC_B_ERROR            : Semi-minor axis less than or equal to zero
 *       TRANMERC_A_LESS_B_ERROR     : Semi-major axis less than semi-minor axis
 *       TRANMERC_SCALE_FACTOR_ERROR : Scale factor outside of valid
 *                                     range (0.3 to 3.0)
 *         TM_LON_WARNING              : Distortion will result if longitude is more
 *                                       than 9 degrees from the Central Meridian
 *
 * REUSE NOTES
 *
 *    TRANSVERSE MERCATOR is intended for reuse by any application that 
 *    performs a Transverse Mercator projection or its inverse.
 *    
 * REFERENCES
 *
 *    Further information on TRANSVERSE MERCATOR can be found in the 
 *    Reuse Manual.
 *
 *    TRANSVERSE MERCATOR originated from :  
 *                      U.S. Army Topographic Engineering Center
 *                      Geospatial Information Division
 *                      7701 Telegraph Road
 *                      Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    TRANSVERSE MERCATOR has no restrictions.
 *
 * ENVIRONMENT
 *
 *    TRANSVERSE MERCATOR was tested and certified in the following 
 *    environments:
 *
 *    1. Solaris 2.5 with GCC, version 2.8.1
 *    2. Windows 95 with MS Visual C++, version 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    10-02-97          Original Code
 *    03-02-97          Re-engineered Code
 *
 */



private:
/**************************************************************************/
/*                               GLOBAL DECLARATIONS
 *
 */

/* Ellipsoid Parameters, default to WGS 84  */
static double  TranMerc_a;// = 6378137.0;              /* Semi-major axis of ellipsoid i meters */
static double  TranMerc_b;// = 6356752.3142;           /* Eccentricity of ellipsoid  */
static double  TranMerc_es;// = 0.0066943799901413800; /* Eccentricity (0.08181919084262188000) squared */
static double  TranMerc_ebs;// = 0.0067394967565869;   /* Second Eccentricity squared */

/* Transverse_Mercator projection Parameters */
static double  TranMerc_Origin_Lat;// = 0.0;           /* Latitude of origin in radians */
static double  TranMerc_Origin_Long;// = 0.0;          /* Longitude of origin in radians */
static double  TranMerc_False_Northing;// = 0.0;       /* False northing in meters */
static double  TranMerc_False_Easting;// = 0.0;        /* False easting in meters */
static double  TranMerc_Scale_Factor;// = 1.0;         /* Scale factor  */

/* Isometeric to geodetic latitude parameters, default to WGS 84 */
static double  TranMerc_ap;// = 6367449.1458008;
static double  TranMerc_bp;// = 16038.508696861;
static double  TranMerc_cp;// = 16.832613334334;
static double  TranMerc_dp;// = 0.021984404273757;
static double  TranMerc_ep;// = 3.1148371319283e-005;

/* Maximum variance for easting and northing values for WGS 84. */
static double  TranMerc_Delta_Easting;// = 40000000.0;
static double  TranMerc_Delta_Northing;// = 40000000.0;

/* These state variables are for optimization purposes. The only function
 * that should modify them is Set_Tranverse_Mercator_Parameters.         */


/***************************************************************************/
/*
 *                              FUNCTION PROTOTYPES
 *                                for TRANMERC.C
 */

/* ensure proper linkage to c++ programs */

  int Set_Transverse_Mercator_Parameters(double a,      
                                          double b,
                                          double Origin_Latitude,
                                          double Central_Meridian,
                                          double False_Easting,
                                          double False_Northing,
                                          double Scale_Factor);
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


  void Get_Transverse_Mercator_Parameters(double *a,
                                          double *b,
                                          double *Origin_Latitude,
                                          double *Central_Meridian,
                                          double *False_Easting,
                                          double *False_Northing,
                                          double *Scale_Factor);
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


  int Convert_Geodetic_To_Transverse_Mercator (double Latitude,
                                                double Longitude,
                                                double *Easting,
                                                double *Northing);

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


  int Convert_Transverse_Mercator_To_Geodetic (double Easting,
                                                double Northing,
                                                double *Latitude,
                                                double *Longitude);

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


};
#endif

 
