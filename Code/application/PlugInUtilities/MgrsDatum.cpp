/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 
#include "AppConfig.h"

/***************************************************************************/
/* RSC IDENTIFIER: Datum
 *
 * ABSTRACT
 *
 *    This component provides datum shifts for a large collection of local
 *    datums, WGS72, and WGS84.  A particular datum can be accessed by using its 
 *    standard 5-letter code to find its index in the datum table.  The index 
 *    can then be used to retrieve the name, type, ellipsoid code, and datum 
 *    shift parameters, and to perform shifts to or from that datum.
 *    
 *    By sequentially retrieving all of the datum codes and/or names, a menu
 *    of the available datums can be constructed.  The index values resulting
 *    from selections from this menu can then be used to access the parameters
 *    of the selected datum, or to perform datum shifts involving that datum.
 *
 *    This component supports both 3-parameter local datums, for which only X,
 *    Y, and Z translations relative to WGS 84 have been defined, and 
 *    7-parameter local datums, for which X, Y, and Z rotations, and a scale 
 *    factor, are also defined.  It also includes entries for WGS 84 (with an
 *    index of 0), and WGS 72 (with an index of 1), but no shift parameter 
 *    values are defined for these.
 *
 *    This component provides datum shift functions for both geocentric and
 *    geodetic coordinates.  WGS84 is used as an intermediate state when
 *    shifting from one local datum to another.  When geodetic coordinates are
 *    given Molodensky's method is used, except near the poles where the 3-step
 *    step method is used instead.  Specific algorithms are used for shifting 
 *    between WGS72 and WGS84.
 *
 *    This component depends on two data files, named 3_param.dat and 
 *    7_param.dat, which contain the datum parameter values.  Copies of these
 *    files must be located in the directory specified by the value of the 
 *    environment variable "DATUM_DATA", if defined, or else in the current 
 *    directory whenever a program containing this component is executed. 
 *
 *    Additional datums can be added to these files, either manually or using 
 *    the Create_Datum function.  However, if a large number of datums are 
 *    added, the datum table array sizes in this component will have to be 
 *    increased.
 *
 *    This component depends on two other components: the Ellipsoid component
 *    for access to ellipsoid parameters; and the Geocentric component for 
 *    conversions between geodetic and geocentric coordinates.
 *
 * ERROR HANDLING
 *
 *    This component checks for input file errors and input parameter errors.
 *    If an invalid value is found, the error code is combined with the current
 *    error code using the bitwise or.  This combining allows multiple error
 *    codes to be returned. The possible error codes are:
 *
 *  DATUM_NO_ERROR                  : No errors occurred in function
 *  DATUM_NOT_INITIALIZED_ERROR     : Datum module has not been initialized
 *  DATUM_7PARAM_FILE_OPEN_ERROR    : 7 parameter file opening error
 *  DATUM_7PARAM_FILE_PARSING_ERROR : 7 parameter file structure error
 *  DATUM_7PARAM_OVERFLOW_ERROR     : 7 parameter table overflow
 *  DATUM_3PARAM_FILE_OPEN_ERROR    : 3 parameter file opening error
 *  DATUM_3PARAM_FILE_PARSING_ERROR : 3 parameter file structure error
 *  DATUM_3PARAM_OVERFLOW_ERROR     : 3 parameter table overflow
 *  DATUM_INVALID_INDEX_ERROR       : Index out of valid range (less than one
 *                                      or more than Number_of_Datums)
 *  DATUM_INVALID_SRC_INDEX_ERROR   : Source datum index invalid
 *  DATUM_INVALID_DEST_INDEX_ERROR  : Destination datum index invalid
 *  DATUM_INVALID_CODE_ERROR        : Datum code not found in table
 *  DATUM_LAT_ERROR                 : Latitude out of valid range (-90 to 90)
 *  DATUM_LON_ERROR                 : Longitude out of valid range (-180 to
 *                                    360)
 *  DATUM_SIGMA_ERROR               : Standard error values must be positive
 *                                    (or -1 if unknown)
 *  DATUM_DOMAIN_ERROR              : Domain of validity not well defined
 *  DATUM_ELLIPSE_ERROR             : Error in ellipsoid module
 *
 *
 * REUSE NOTES
 *
 *    Datum is intended for reuse by any application that needs access to 
 *    datum shift parameters relative to WGS 84.
 *
 *    
 * REFERENCES
 *
 *    Further information on Datum can be found in the Reuse Manual.
 *
 *    Datum originated from :  U.S. Army Topographic Engineering Center (USATEC)
 *                             Geospatial Information Division (GID)
 *                             7701 Telegraph Road
 *                             Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    Datum has no restrictions.
 *
 * ENVIRONMENT
 *
 *    Datum was tested and certified in the following environments:
 *
 *    1. Solaris 2.5 with GCC 2.8.1
 *    2. MS Windows 95 with MS Visual C++ 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    03/30/97          Original Code
 *    05/28/99          Added user-definable datums (for JMTK)
 *                      Added datum domain of validity checking (for JMTK)
 *                      Added datum shift accuracy calculation (for JMTK) 
 */


/***************************************************************************/
/*
 *                               INCLUDES
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "MgrsDatum.h"
/* 
 *    stdio.h    - standard C input/output library
 *    stdlib.h   - standard C general utilities library 
 *    string.h   - standard C string handling library
 *    ctype.h    - standard C character handling library
 *    math.h     - standard C mathematics library
 *    ellipse.h  - used to get ellipsoid parameters
 *    geocent.h  - used to convert between geodetic and geocentric coordinates
 *    datum.h    - for prototype error ehecking and error codes
 */


/***************************************************************************/
/*
 *                               DEFINES
 */

#define SECONDS_PER_RADIAN     206264.8062471;   /* Seconds in a radian */
#define MIN_LAT                  (-PI/2.0)
#define MAX_LAT                  (+PI/2.0)
#define MIN_LON                   -PI
#define MAX_LON                  (2.0 * PI)
#define DATUM_CODE_LENGTH           7
#define DATUM_NAME_LENGTH          32
#define ELLIPSOID_CODE_LENGTH       3  // Length of ellipsoid code (including null) 
#define MAX_7PARAM                 25
#define MAX_3PARAM                250
#define MOLODENSKY_MAX            (89.75 * PI / 180.0) /* Polar limit */
#define FILENAME_LENGTH           128


/***************************************************************************/
/*
 *                          GLOBAL DECLARATIONS
 */

typedef struct Datum_Table_Row
{
  Datum_Type Type;
  char Code[DATUM_CODE_LENGTH];
  char Name[DATUM_NAME_LENGTH];
  char Ellipsoid_Code[ELLIPSOID_CODE_LENGTH];
  double Parameters[7];  /* interface for 3 and 7 Parameters */
  double Sigma_X;        /* standard error along X axis */
  double Sigma_Y;        /* standard error along Y axis */
  double Sigma_Z;        /* standard error along Z axis */
  double West_longitude; /* western boundary of validity rectangle */
  double East_longitude; /* eastern boundary of validity rectangle */
  double South_latitude; /* southern boundary of validity rectangle */
  double North_latitude; /* northern boundary of validity rectangle */
} Datum_Row; /* defines a single entry in the datum table */

static Datum_Row Datum_Table_3Param[MAX_3PARAM] = {
Three_Param_Datum,"ADI-M",  "ADINDAN, Mean"                  ,"CD", -166,    -15,   204, 0, 0, 0, 1,   5,   5,      3,   -5,   31,   15,   55, 
Three_Param_Datum,"ADI-A",  "ADINDAN, Ethiopia"              ,"CD", -165,    -11,   206, 0, 0, 0, 1,   3,   3,      3,   -3,   25,   26,   50, 
Three_Param_Datum,"ADI-B",  "ADINDAN, Sudan"                 ,"CD", -161,    -14,   205, 0, 0, 0, 1,   3,   5,      3,   -3,   31,   15,   45, 
Three_Param_Datum,"ADI-C",  "ADINDAN, Mali"                  ,"CD", -123,    -20,   220, 0, 0, 0, 1,  25,  25,     25,    3,   31,  -20,   11, 
Three_Param_Datum,"ADI-D",  "ADINDAN, Senegal"               ,"CD", -128,    -18,   224, 0, 0, 0, 1,  25,  25,     25,    5,   23,  -24,   -5, 
Three_Param_Datum,"ADI-E",  "ADINDAN, Burkina Faso"          ,"CD", -118,    -14,   218, 0, 0, 0, 1,  25,  25,     25,    4,   22,   -5,    8, 
Three_Param_Datum,"ADI-F",  "ADINDAN, Cameroon"              ,"CD", -134,     -2,   210, 0, 0, 0, 1,  25,  25,     25,   -4,   19,    3,   23, 
Three_Param_Datum,"AFG"  ,    "AFGOOYE, Somalia"             ,"KA",  -43,   -163,    45, 0, 0, 0, 1,  25,  25,     25,   -8,   19,   35,   60, 
Three_Param_Datum,"AIA"  ,    "ANTIGUA ISLAND ASTRO 1943"    ,"CD", -270,     13,    62, 0, 0, 0, 1,  25,  25,     25,   16,   20,  -65,  -61, 
Three_Param_Datum,"AIN-A",  "AIN EL ABD 1970, Bahrain"       ,"IN", -150,   -250,    -1, 0, 0, 0, 1,  25,  25,     25,   24,   28,   49,   53, 
Three_Param_Datum,"AIN-B",  "AIN EL ABD 1970, Saudi Arabia"  ,"IN", -143,   -236,     7, 0, 0, 0, 1,  10,  10,     10,    8,   38,   28,   62, 
Three_Param_Datum,"AMA"  ,    "AMERICAN SAMOA 1962"          ,"CC", -115,    118,   426, 0, 0, 0, 1,  25,  25,     25,  -19,   -9, -174, -165, 
Three_Param_Datum,"ANO"  ,  "ANNA 1 ASTRO 1965, Cocos Is."   ,"AN", -491,    -22,   435, 0, 0, 0, 1,  25,  25,     25,  -14,  -10,   94,   99, 
Three_Param_Datum,"ARF-M",  "ARC 1950, Mean"                 ,"CD", -143,    -90,  -294, 0, 0, 0, 1,  20,  33,     20,  -36,   10,    4,   42, 
Three_Param_Datum,"ARF-A",  "ARC 1950, Botswana"             ,"CD", -138,   -105,  -289, 0, 0, 0, 1,   3,   5,      3,  -33,  -13,   13,   36, 
Three_Param_Datum,"ARF-B",  "ARC 1950, Lesotho"              ,"CD", -125,   -108,  -295, 0, 0, 0, 1,   3,   3,      8,  -36,  -23,   21,   35, 
Three_Param_Datum,"ARF-C",  "ARC 1950, Malawi"               ,"CD", -161,    -73,  -317, 0, 0, 0, 1,   9,  24,      8,  -21,   -3,   26,   42, 
Three_Param_Datum,"ARF-D",  "ARC 1950, Swaziland"            ,"CD", -134,   -105,  -295, 0, 0, 0, 1,  15,  15,     15,  -33,  -20,   25,   40, 
Three_Param_Datum,"ARF-E",  "ARC 1950, Zaire"                ,"CD", -169,    -19,  -278, 0, 0, 0, 1,  25,  25,     25,  -21,   10,    4,   38, 
Three_Param_Datum,"ARF-F",  "ARC 1950, Zambia"               ,"CD", -147,    -74,  -283, 0, 0, 0, 1,  21,  21,     27,  -24,   -1,   15,   40, 
Three_Param_Datum,"ARF-G",  "ARC 1950, Zimbabwe"             ,"CD", -142,    -96,  -293, 0, 0, 0, 1,   5,   8,     11,  -29,   -9,   19,   39, 
Three_Param_Datum,"ARF-H",  "ARC 1950, Burundi"              ,"CD", -153,     -5,  -292, 0, 0, 0, 1,  20,  20,     20,  -11,    4,   21,   37, 
Three_Param_Datum,"ARS-M",  "ARC 1960, Kenya & Tanzania"     ,"CD", -160,     -6,  -302, 0, 0, 0, 1,  20,  20,     20,  -18,    8,   23,   47, 
Three_Param_Datum,"ARS-A",  "ARC 1960, Kenya"                ,"CD", -157,     -2,  -299, 0, 0, 0, 1,   4,   3,      3,  -11,    8,   28,   47, 
Three_Param_Datum,"ARS-B",  "ARC 1960, Tanzania"             ,"CD", -175,    -23,  -303, 0, 0, 0, 1,   6,   9,     10,  -18,    5,   23,   47, 
Three_Param_Datum,"ASC"  ,  "ASCENSION ISLAND 1958"          ,"IN", -205,    107,    53, 0, 0, 0, 1,  25,  25,     25,   -9,   -6,  -16,  -13, 
Three_Param_Datum,"ASM"  ,  "MONTSERRAT ISLAND ASTRO 1958"   ,"CD",  174,    359,   365, 0, 0, 0, 1,  25,  25,     25,   15,   18,  -64,  -61, 
Three_Param_Datum,"ASQ"  ,  "ASTRO STATION 1952, Marcus Is." ,"IN",  124,   -234,   -25, 0, 0, 0, 1,  25,  25,     25,   22,   26,  152,  156, 
Three_Param_Datum,"ATF"  ,  "ASTRO BEACON E 1845, Iwo Jima"  ,"IN",  145,     75,  -272, 0, 0, 0, 1,  25,  25,     25,   22,   26,  140,  144, 
Three_Param_Datum,"AUA"  ,  "AUSTRALIAN GEODETIC 1966"       ,"AN", -133,    -48,   148, 0, 0, 0, 1,   3,   3,      3,  -46,   -4,  109,  161, 
Three_Param_Datum,"AUG"  ,  "AUSTRALIAN GEODETIC 1984"       ,"AN", -134,    -48,   149, 0, 0, 0, 1,   2,   2,      2,  -46,   -4,  109,  161, 
Three_Param_Datum,"BAT"  ,  "DJAKARTA, INDONESIA"            ,"BR", -377,    681,   -50, 0, 0, 0, 1,   3,   3,      3,  -16,   11,   89,  146, 
Three_Param_Datum,"BID"  ,  "BISSAU, Guinea-Bissau"          ,"IN", -173,    253,    27, 0, 0, 0, 1,  25,  25,     25,    5,   19,  -23,   -7, 
Three_Param_Datum,"BER"  ,  "BERMUDA 1957, Bermuda Islands"  ,"CC",  -73,    213,   296, 0, 0, 0, 1,  20,  20,     20,   31,   34,  -66,  -63, 
Three_Param_Datum,"BOO"  ,  "BOGOTA OBSERVATORY, Columbia"   ,"IN",  307,    304,  -318, 0, 0, 0, 1,   6,   5,      6,  -10,   16,  -85,  -61, 
Three_Param_Datum,"BUR"  ,  "BUKIT RIMPAH, Banka & Belitung" ,"BR", -384,    664,   -48, 0, 0, 0, 1,  -1,  -1,     -1,   -6,    0,  103,  110, 
Three_Param_Datum,"CAC"  ,  "CAPE CANAVERAL, Fla & Bahamas"  ,"CC",   -2,    151,   181, 0, 0, 0, 1,   3,   3,      3,   15,   38,  -94,  -12, 
Three_Param_Datum,"CAI"  ,  "CAMPO INCHAUSPE 1969, Arg."     ,"IN", -148,    136,    90, 0, 0, 0, 1,   5,   5,      5,  -58,  -27,  -72,  -51, 
Three_Param_Datum,"CAO"  ,  "CANTON ASTRO 1966, Phoenix Is." ,"IN",  298,   -304,  -375, 0, 0, 0, 1,  15,  15,     15,  -13,    3, -180, -165, 
Three_Param_Datum,"CAP"  ,  "CAPE, South Africa"             ,"CD", -136,   -108,  -292, 0, 0, 0, 1,   3,   6,      6,  -43,  -15,   10,   40, 
Three_Param_Datum,"CAZ"  ,  "CAMP AREA ASTRO, Camp McMurdo"  ,"IN", -104,   -129,   239, 0, 0, 0, 1,  -1,  -1,     -1,  -85,  -70,  135,  180, 
Three_Param_Datum,"CCD"  ,  "S-JTSK, Czech Republic"         ,"BR",  589,     76,   480, 0, 0, 0, 1,   4,   2,      3,   43,   56,    6,   28, 
Three_Param_Datum,"CGE"  ,  "CARTHAGE, Tunisia"              ,"CD", -263,      6,   431, 0, 0, 0, 1,   6,   9,      8,   24,   43,    2,   18, 
Three_Param_Datum,"CHI"  ,  "CHATHAM ISLAND ASTRO 1971, NZ"  ,"IN",  175,    -38,   113, 0, 0, 0, 1,  15,  15,     15,  -46,  -42, -180, -174, 
Three_Param_Datum,"CHU"  ,  "CHUA ASTRO, Paraguay"           ,"IN", -134,    229,   -29, 0, 0, 0, 1,   6,   9,      5,  -33,  -14,  -69,  -49, 
Three_Param_Datum,"COA"  ,  "CORREGO ALEGRE, Brazil"         ,"IN", -206,    172,    -6, 0, 0, 0, 1,   5,   3,      5,  -39,   -2,  -80,  -29, 
Three_Param_Datum,"DAL"  ,  "DABOLA, Guinea"                 ,"CD",  -83,     37,   124, 0, 0, 0, 1,  15,  15,     15,    1,   19,   12,   11, 
Three_Param_Datum,"DID"  ,  "DECEPTION ISLAND"               ,"CD",  260,     12,  -147, 0, 0, 0, 1,  20,  20,     20,  -65,  -62,   58,   62, 
Three_Param_Datum,"DOB"  ,  "GUX 1 ASTRO, Guadalcanal Is."   ,"IN",  252,   -209,  -751, 0, 0, 0, 1,  25,  25,     25,  -12,   -8,  158,  163, 
Three_Param_Datum,"EAS"  ,  "EASTER ISLAND 1967"             ,"IN",  211,    147,   111, 0, 0, 0, 1,  25,  25,     25,  -29,  -26, -111, -108, 
Three_Param_Datum,"ENW"  ,  "WAKE-ENIWETOK 1960"             ,"HO",  102,     52,   -38, 0, 0, 0, 1,   3,   3,      3,    1,   16,  159,  175, 
Three_Param_Datum,"EST"  ,  "ESTONIA, 1937"                  ,"BR",  374,    150,   588, 0, 0, 0, 1,   2,   3,      3,   52,   65,   16,   34, 
Three_Param_Datum,"EUR-M",  "EUROPEAN 1950, Mean (3 Param)"  ,"IN",  -87,    -98,  -121, 0, 0, 0, 1,   3,   8,      5,   30,   80,    5,   33, 
Three_Param_Datum,"EUR-A",  "EUROPEAN 1950, Western Europe"  ,"IN",  -87,    -96,  -120, 0, 0, 0, 1,   3,   3,      3,   30,   78,  -15,   25, 
Three_Param_Datum,"EUR-B",  "EUROPEAN 1950, Greece"          ,"IN",  -84,    -95,  -130, 0, 0, 0, 1,  25,  25,     25,   30,   48,   14,   34, 
Three_Param_Datum,"EUR-C",  "EUROPEAN 1950, Norway & Finland","IN",  -87,    -95,  -120, 0, 0, 0, 1,   3,   5,      3,   52,   80,   -2,   38, 
Three_Param_Datum,"EUR-D",  "EUROPEAN 1950, Portugal & Spain","IN",  -84,   -107,  -120, 0, 0, 0, 1,   5,   6,      3,   30,   49,  -15,   10, 
Three_Param_Datum,"EUR-E",  "EUROPEAN 1950, Cyprus"          ,"IN", -104,   -101,  -140, 0, 0, 0, 1,  15,  15,     15,   33,   37,   31,   36, 
Three_Param_Datum,"EUR-F",  "EUROPEAN 1950, Egypt"           ,"IN", -130,   -117,  -151, 0, 0, 0, 1,   6,   8,      8,   16,   38,   19,   42, 
Three_Param_Datum,"EUR-G",  "EUROPEAN 1950, England, Channel","IN",  -86,    -96,  -120, 0, 0, 0, 1,   3,   3,      3,   48,   62,  -10,    3, 
Three_Param_Datum,"EUR-H",  "EUROPEAN 1950, Iran"            ,"IN", -117,   -132,  -164, 0, 0, 0, 1,   9,  12,     11,   19,   47,   37,   69, 
Three_Param_Datum,"EUR-I",  "EUROPEAN 1950, Sardinia(Italy)" ,"IN",  -97,   -103,  -120, 0, 0, 0, 1,  25,  25,     25,   37,   43,    6,   12, 
Three_Param_Datum,"EUR-J",  "EUROPEAN 1950, Sicily(Italy)"   ,"IN",  -97,    -88,  -135, 0, 0, 0, 1,  20,  20,     20,   35,   40,   10,   17, 
Three_Param_Datum,"EUR-K",  "EUROPEAN 1950, England, Ireland","IN",  -86,    -96,  -120, 0, 0, 0, 1,   3,   3,      3,   48,   62,  -12,    3, 
Three_Param_Datum,"EUR-L",  "EUROPEAN 1950, Malta"           ,"IN", -107,    -88,  -149, 0, 0, 0, 1,  25,  25,     25,   34,   38,   12,   16, 
Three_Param_Datum,"EUR-S",  "EUROPEAN 1950, Iraq, Israel"    ,"IN", -103,   -106,  -141, 0, 0, 0, 1,  -1,  -1,     -1,  -38,   -4,   36,   57, 
Three_Param_Datum,"EUR-T",  "EUROPEAN 1950, Tunisia"         ,"IN", -112,    -77,  -145, 0, 0, 0, 1,  25,  25,     25,   24,   43,    2,   18, 
Three_Param_Datum,"EUS"  ,  "EUROPEAN 1979"                  ,"IN",  -86,    -98,  -119, 0, 0, 0, 1,   3,   3,      3,   30,   80,  -15,   24, 
Three_Param_Datum,"FAH"  ,  "OMAN"                           ,"CD", -346,     -1,   224, 0, 0, 0, 1,   3,   3,      9,   10,   32,   46,   65, 
Three_Param_Datum,"FLO"  ,  "OBSERVATORIO MET. 1939, Flores" ,"IN", -425,   -169,    81, 0, 0, 0, 1,  20,  20,     20,   38,   41,  -33,  -30, 
Three_Param_Datum,"FOT"  ,  "FORT THOMAS 1955, Leeward Is."  ,"CD",   -7,    215,   225, 0, 0, 0, 1,  25,  25,     25,   16,   19,  -64,  -61, 
Three_Param_Datum,"GAA"  ,  "GAN 1970, Rep. of Maldives"     ,"IN", -133,   -321,    50, 0, 0, 0, 1,  25,  25,     25,   -2,    9,   71,   75, 
Three_Param_Datum,"GEO"  ,  "GEODETIC DATUM 1949, NZ"        ,"IN",   84,    -22,   209, 0, 0, 0, 1,   5,   3,      5,  -48,  -33,  165,  180, 
Three_Param_Datum,"GIZ"  ,  "DOS 1968, Gizo Island"          ,"IN",  230,   -199,  -752, 0, 0, 0, 1,  25,  25,     25,  -10,   -7,  155,  158, 
Three_Param_Datum,"GRA"  ,  "GRACIOSA BASE SW 1948, Azores"  ,"IN", -104,    167,   -38, 0, 0, 0, 1,   3,   3,      3,   37,   41,  -30,  -26, 
Three_Param_Datum,"GUA"  ,  "GUAM 1963"                      ,"CC", -100,   -248,   259, 0, 0, 0, 1,   3,   3,      3,   12,   15,  143,  146, 
Three_Param_Datum,"GSE"  ,  "GUNUNG SEGARA, Indonesia"       ,"BR", -403,    684,    41, 0, 0, 0, 1,  -1,  -1,     -1,   -6,    9,  106,  121, 
Three_Param_Datum,"HEN"  ,  "HERAT NORTH, Afghanistan"       ,"IN", -333,   -222,   114, 0, 0, 0, 1,  -1,  -1,     -1,   23,   44,   55,   81, 
Three_Param_Datum,"HER"  ,  "HERMANNSKOGEL, old Yugoslavia"  ,"BR",  682,   -203,   480, 0, 0, 0, 1,  -1,  -1,     -1,   35,   52,    7,   29, 
Three_Param_Datum,"HIT"  ,  "PROVISIONAL SOUTH CHILEAN 1963" ,"IN",   16,    196,    93, 0, 0, 0, 1,  25,  25,     25,  -64,  -25,  -83,  -60, 
Three_Param_Datum,"HJO"  ,  "HJORSEY 1955, Iceland"          ,"IN",  -73,     46,   -86, 0, 0, 0, 1,   3,   3,      6,   61,   69,  -24,  -11, 
Three_Param_Datum,"HKD"  ,  "HONG KONG 1963"                 ,"IN", -156,   -271,  -189, 0, 0, 0, 1,  25,  25,     25,   21,   24,  112,  116, 
Three_Param_Datum,"HTN"  ,  "HU-TZU-SHAN, Taiwan"            ,"IN", -637,   -549,  -203, 0, 0, 0, 1,  15,  15,     15,   20,   28,  117,  124, 
Three_Param_Datum,"IBE"  ,  "BELLEVUE (IGN), Efate Is."      ,"IN", -127,   -769,   472, 0, 0, 0, 1,  20,  20,     20,  -20,  -16,  167,  171, 
Three_Param_Datum,"IDN"  ,  "INDONESIAN 1974"                ,"ID",  -24,    -15,     5, 0, 0, 0, 1,  25,  25,     25,  -16,   11,   89,  146, 
Three_Param_Datum,"IND-B",  "INDIAN, Bangladesh"             ,"EA",  282,    726,   254, 0, 0, 0, 1,  10,   8,     12,   15,   33,   80,  100, 
Three_Param_Datum,"IND-I",  "INDIAN, India & Nepal"          ,"EC",  295,    736,   257, 0, 0, 0, 1,  12,  10,     15,    2,   44,   62,  105, 
Three_Param_Datum,"IND-P",  "INDIAN, Pakistan"               ,"EA",  283,    682,   231, 0, 0, 0, 1,  -1,  -1,     -1,   17,   44,   55,   81, 
Three_Param_Datum,"INF-A",  "INDIAN 1954, Thailand"          ,"EA",  217,    823,   299, 0, 0, 0, 1,  15,   6,     12,    0,   27,   91,  111, 
Three_Param_Datum,"ING-A",  "INDIAN 1960, Vietnam 16N"       ,"EA",  198,    881,   317, 0, 0, 0, 1,  25,  25,     25,   11,   23,  101,  115, 
Three_Param_Datum,"ING-B",  "INDIAN 1960, Con Son Island"    ,"EA",  182,    915,   344, 0, 0, 0, 1,  25,  25,     25,    6,   11,  104,  109, 
Three_Param_Datum,"INH-A",  "INDIAN 1975, Thailand"          ,"EA",  209,    818,   290, 0, 0, 0, 1,  12,  10,     12,    0,   27,   91,  111, 
Three_Param_Datum,"INH-A1", "INDIAN 1975, Thailand"          ,"EA",  210,    814,   289, 0, 0, 0, 1,   3,   2,      3,    0,   27,   91,  111, 
Three_Param_Datum,"IRL"  ,  "IRELAND 1965"                   ,"AM",  506,   -122,   611, 0, 0, 0, 1,   3,   3,      3,   50,   57,  -12,   -4, 
Three_Param_Datum,"ISG"  ,  "ISTS 061 ASTRO 1968, S Georgia" ,"IN", -794,    119,  -298, 0, 0, 0, 1,  25,  25,     25,  -56,  -52,  -38,  -34, 
Three_Param_Datum,"IST"  ,  "ISTS 073 ASTRO 1969, Diego Garc","IN",  208,   -435,  -229, 0, 0, 0, 1,  25,  25,     25,  -10,   -4,   69,   75, 
Three_Param_Datum,"JOH"  ,  "JOHNSTON ISLAND 1961"           ,"IN",  189,    -79,  -202, 0, 0, 0, 1,  25,  25,     25,  -46,  -43,  -76,  -73, 
Three_Param_Datum,"KAN"  ,  "KANDAWALA, Sri Lanka"           ,"EA",  -97,    787,    86, 0, 0, 0, 1,  20,  20,     20,    4,   12,   77,   85, 
Three_Param_Datum,"KEG"  ,  "KERGUELEN ISLAND 1949"          ,"IN",  145,   -187,   103, 0, 0, 0, 1,  25,  25,     25,  -81,  -74,  139,  180, 
Three_Param_Datum,"KEA"  ,  "KERTAU 1948, W Malaysia & Sing.","EE",  -11,    851,     5, 0, 0, 0, 1,  10,   8,      6,   -5,   12,   94,  112, 
Three_Param_Datum,"KUS"  ,  "KUSAIE ASTRO 1951, Caroline Is.","IN",  647,   1777, -1124, 0, 0, 0, 1,  25,  25,     25,   -1,   12,  134,  167, 
Three_Param_Datum,"LCF"  ,  "L.C. 5 ASTRO 1961, Cayman Brac" ,"CC",   42,    124,   147, 0, 0, 0, 1,  25,  25,     25,   18,   21,  -81,  -78, 
Three_Param_Datum,"LEH"  ,  "LEIGON, Ghana"                  ,"CD", -130,     29,   364, 0, 0, 0, 1,   2,   3,      2,   -1,   17,   -9,    7, 
Three_Param_Datum,"LIB"  ,  "LIBERIA 1964"                   ,"CD",  -90,     40,    88, 0, 0, 0, 1,  15,  15,     15,   -1,   14,  -17,   -1, 
Three_Param_Datum,"LUZ-A",  "LUZON, Phillipines"             ,"CC", -133,    -77,   -51, 0, 0, 0, 1,   8,  11,      9,    3,   23,  115,  128, 
Three_Param_Datum,"LUZ-B",  "LUZON, Mindanao Island"         ,"CC", -133,    -79,   -72, 0, 0, 0, 1,  25,  25,     25,    4,   12,  120,  128, 
Three_Param_Datum,"MAS"  ,  "MASSAWA, Ethiopia"              ,"BR",  639,    405,    60, 0, 0, 0, 1,  25,  25,     25,    7,   25,   37,   53, 
Three_Param_Datum,"MER"  ,  "MERCHICH, Morocco"              ,"CD",   31,    146,    47, 0, 0, 0, 1,   5,   3,      3,   22,   42,  -19,    5, 
Three_Param_Datum,"MID"  ,  "MIDWAY ASTRO 1961, Midway Is."  ,"IN",  912,    -58,  1227, 0, 0, 0, 1,  25,  25,     25,   25,   30, -180, -169, 
Three_Param_Datum,"MIK"  ,  "MAHE 1971, Mahe Is."            ,"CD",   41,   -220,  -134, 0, 0, 0, 1,  25,  25,     25,   -6,   -3,   54,   57, 
Three_Param_Datum,"MIN-A",  "MINNA, Cameroon"                ,"CD",  -81,    -84,   115, 0, 0, 0, 1,  25,  25,     25,   -4,   19,    3,   23, 
Three_Param_Datum,"MIN-B",  "MINNA, Nigeria"                 ,"CD",  -92,    -93,   122, 0, 0, 0, 1,   3,   6,      5,   -1,   21,   -4,   20, 
Three_Param_Datum,"MOD"  ,  "ROME 1940, Sardinia"            ,"IN", -225,    -65,     9, 0, 0, 0, 1,  25,  25,     25,   37,   43,    6,   12, 
Three_Param_Datum,"MPO"  ,  "M'PORALOKO, Gabon"              ,"CD",  -74,   -130,    42, 0, 0, 0, 1,  25,  25,     25,  -10,    8,    3,   20, 
Three_Param_Datum,"MVS"  ,  "VITI LEVU 1916, Viti Levu Is."  ,"CD",   51,    391,   -36, 0, 0, 0, 1,  25,  25,     25,  -20,  -16,  176,  180, 
Three_Param_Datum,"NAH-A",  "NAHRWAN, Masirah Island (Oman)" ,"CD", -247,   -148,   369, 0, 0, 0, 1,  25,  25,     25,   19,   22,   57,   60, 
Three_Param_Datum,"NAH-B",  "NAHRWAN, United Arab Emirates"  ,"CD", -249,   -156,   381, 0, 0, 0, 1,  25,  25,     25,   17,   32,   45,   62, 
Three_Param_Datum,"NAH-C",  "NAHRWAN, Saudi Arabia"          ,"CD", -243,   -192,   477, 0, 0, 0, 1,  25,  25,     20,    8,   38,   28,   62, 
Three_Param_Datum,"NAP"  ,  "NAPARIMA, Trinidad & Tobago"    ,"IN",  -10,    375,   165, 0, 0, 0, 1,  15,  15,     15,    8,   13,  -64,  -59, 
Three_Param_Datum,"NAS-A",  "NORTH AMERICAN 1927, Eastern US","CC",   -9,    161,   179, 0, 0, 0, 1,   5,   5,      8,   18,   55, -102,  -60, 
Three_Param_Datum,"NAS-B",  "NORTH AMERICAN 1927, Western US","CC",   -8,    159,   175, 0, 0, 0, 1,   5,   3,      3,   19,   55, -132,  -87, 
Three_Param_Datum,"NAS-C",  "NORTH AMERICAN 1927, CONUS"     ,"CC",   -8,    160,   176, 0, 0, 0, 1,   5,   5,      6,   15,   60, -135,  -60, 
Three_Param_Datum,"NAS-D",  "NORTH AMERICAN 1927, Alaska"    ,"CC",   -5,    135,   172, 0, 0, 0, 1,   5,   9,      5,   47,   78, -175, -157, 
Three_Param_Datum,"NAS-E",  "NORTH AMERICAN 1927, Canada"    ,"CC",  -10,    158,   187, 0, 0, 0, 1,  15,  11,      6,   36,   90, -150,  -50, 
Three_Param_Datum,"NAS-F",  "NORTH AMERICAN 1927, Alberta/BC","CC",   -7,    162,   188, 0, 0, 0, 1,   8,   8,      6,   43,   65, -145, -105, 
Three_Param_Datum,"NAS-G",  "NORTH AMERICAN 1927, E. Canada" ,"CC",  -22,    160,   190, 0, 0, 0, 1,   6,   6,      3,   38,   68,  -85,  -45, 
Three_Param_Datum,"NAS-H",  "NORTH AMERICAN 1927, Man/Ont"   ,"CC",   -9,    157,   184, 0, 0, 0, 1,   9,   5,      5,   36,   63, -108,  -69, 
Three_Param_Datum,"NAS-I",  "NORTH AMERICAN 1927, NW Terr."  ,"CC",    4,    159,   188, 0, 0, 0, 1,   5,   5,      3,   43,   90, -144,  -55, 
Three_Param_Datum,"NAS-J",  "NORTH AMERICAN 1927, Yukon"     ,"CC",   -7,    139,   181, 0, 0, 0, 1,   5,   8,      3,   53,   75, -147, -117, 
Three_Param_Datum,"NAS-L",  "NORTH AMERICAN 1927, Mexico"    ,"CC",  -12,    130,   190, 0, 0, 0, 1,   8,   6,      6,   10,   38, -122,  -80, 
Three_Param_Datum,"NAS-N",  "NORTH AMERICAN 1927, C. America","CC",    0,    125,   194, 0, 0, 0, 1,   8,   3,      5,    3,   25,  -98,  -77, 
Three_Param_Datum,"NAS-O",  "NORTH AMERICAN 1927, Canal Zone","CC",    0,    125,   201, 0, 0, 0, 1,  20,  20,     20,    3,   15,  -86,  -74, 
Three_Param_Datum,"NAS-P",  "NORTH AMERICAN 1927, Caribbean" ,"CC",   -3,    142,   183, 0, 0, 0, 1,   3,   9,     12,    8,   29,  -87,  -58, 
Three_Param_Datum,"NAS-Q",  "NORTH AMERICAN 1927, Bahamas"   ,"CC",   -4,    154,   178, 0, 0, 0, 1,   5,   3,      5,   19,   29,  -83,  -71, 
Three_Param_Datum,"NAS-R",  "NORTH AMERICAN 1927, San Salv." ,"CC",    1,    140,   165, 0, 0, 0, 1,  25,  25,     25,   23,   26,  -75,  -74, 
Three_Param_Datum,"NAS-T",  "NORTH AMERICAN 1927, Cuba"      ,"CC",   -9,    152,   178, 0, 0, 0, 1,  25,  25,     25,   18,   25,  -87,  -78, 
Three_Param_Datum,"NAS-U",  "NORTH AMERICAN 1927, Greenland" ,"CC",   11,    114,   195, 0, 0, 0, 1,  25,  25,     25,   74,   81,   74,   56, 
Three_Param_Datum,"NAS-V",  "NORTH AMERICAN 1927, Aleutian E","CC",   -2,    152,   149, 0, 0, 0, 1,   6,   8,     10,   50,   58, -180, -161, 
Three_Param_Datum,"NAS-W",  "NORTH AMERICAN 1927, Aleutian W","CC",    2,    204,   105, 0, 0, 0, 1,  10,  10,     10,   50,   58,  169,  180, 
Three_Param_Datum,"NAR-A",  "NORTH AMERICAN 1983, Alaska"    ,"RF",    0,      0,     0, 0, 0, 0, 1,   2,   2,      2,   48,   78, -175, -135, 
Three_Param_Datum,"NAR-B",  "NORTH AMERICAN 1983, Canada"    ,"RF",    0,      0,     0, 0, 0, 0, 1,   2,   2,      2,   36,   90, -150,  -50, 
Three_Param_Datum,"NAR-C",  "NORTH AMERICAN 1983, CONUS"     ,"RF",    0,      0,     0, 0, 0, 0, 1,   2,   2,      2,   15,   60, -135,  -60, 
Three_Param_Datum,"NAR-D",  "NORTH AMERICAN 1983, Mexico"    ,"RF",    0,      0,     0, 0, 0, 0, 1,   2,   2,      2,   28,   11, -122,  -72, 
Three_Param_Datum,"NAR-E",  "NORTH AMERICAN 1983, Aleutian"  ,"RF",   -2,      0,     4, 0, 0, 0, 1,   5,   2,      5,   51,   74, -180,  180, 
Three_Param_Datum,"NAR-H",  "NORTH AMERICAN 1983, Hawai'i"   ,"RF",    1,      1,    -1, 0, 0, 0, 1,   2,   2,      2,   17,   24, -164, -153, 
Three_Param_Datum,"NSD"  ,  "NORTH SAHARA 1959, Algeria"     ,"CD", -186,    -93,   310, 0, 0, 0, 1,  25,  25,     25,   13,   43,  -15,   11, 
Three_Param_Datum,"OEG"  ,  "OLD EGYPTIAN 1907"              ,"HE", -130,    110,   -13, 0, 0, 0, 1,   3,   6,      8,   16,   38,   19,   42, 
Three_Param_Datum,"OGB-M",  "ORDNANCE GB 1936, Mean (3 Para)","AA",  375,   -111,   431, 0, 0, 0, 1,  10,  10,     15,   44,   66,  -14,    7, 
Three_Param_Datum,"OGB-A",  "ORDNANCE GB 1936, England"      ,"AA",  371,   -112,   434, 0, 0, 0, 1,   5,   5,      6,   44,   61,  -12,    7, 
Three_Param_Datum,"OGB-B",  "ORDNANCE GB 1936, Eng., Wales"  ,"AA",  371,   -111,   434, 0, 0, 0, 1,  10,  10,     15,   44,   61,  -12,    7, 
Three_Param_Datum,"OGB-C",  "ORDNANCE GB 1936, Scotland"     ,"AA",  384,   -111,   425, 0, 0, 0, 1,  10,  10,     10,   49,   66,  -14,    4, 
Three_Param_Datum,"OGB-D",  "ORDNANCE GB 1936, Wales"        ,"AA",  370,   -108,   434, 0, 0, 0, 1,  20,  20,     20,   46,   59,  -11,    3, 
Three_Param_Datum,"OHA-M",  "OLD HAWAI'IAN (CC), Mean"       ,"CC",   61,   -285,  -181, 0, 0, 0, 1,  25,  20,     20,   17,   24, -164, -153, 
Three_Param_Datum,"OHA-A",  "OLD HAWAI'IAN (CC), Hawai'i"    ,"CC",   89,   -279,  -183, 0, 0, 0, 1,  25,  25,     25,   17,   22, -158, -153, 
Three_Param_Datum,"OHA-B",  "OLD HAWAI'IAN (CC), Kauai"      ,"CC",   45,   -290,  -172, 0, 0, 0, 1,  20,  20,     20,   20,   24, -161, -158, 
Three_Param_Datum,"OHA-C",  "OLD HAWAI'IAN (CC), Maui"       ,"CC",   65,   -290,  -190, 0, 0, 0, 1,  25,  25,     25,   19,   23, -158, -154, 
Three_Param_Datum,"OHA-D",  "OLD HAWAI'IAN (CC), Oahu"       ,"CC",   58,   -283,  -182, 0, 0, 0, 1,  10,   6,      6,   20,   23, -160, -156, 
Three_Param_Datum,"OHI-M",  "OLD HAWAI'IAN (IN), Mean"       ,"IN",  201,   -228,  -346, 0, 0, 0, 1,  25,  20,     20,   17,   24, -164, -153, 
Three_Param_Datum,"OHI-A",  "OLD HAWAI'IAN (IN), Hawai'i"    ,"IN",  229,   -222,  -348, 0, 0, 0, 1,  25,  25,     25,   17,   22, -158, -153, 
Three_Param_Datum,"OHI-B",  "OLD HAWAI'IAN (IN), Kauai"      ,"IN",  185,   -233,  -337, 0, 0, 0, 1,  20,  20,     20,   20,   24, -161, -158, 
Three_Param_Datum,"OHI-C",  "OLD HAWAI'IAN (IN), Maui"       ,"IN",  205,   -233,  -355, 0, 0, 0, 1,  25,  25,     25,   19,   23, -158, -154, 
Three_Param_Datum,"OHI-D",  "OLD HAWAI'IAN (IN), Oahu"       ,"IN",  198,   -226,  -347, 0, 0, 0, 1,  10,   6,      6,   20,   23, -160, -156, 
Three_Param_Datum,"PHA"  ,  "AYABELLA LIGHTHOUSE, Bjibouti"  ,"CD",  -79,   -129,   145, 0, 0, 0, 1,  25,  25,     25,    5,   20,   36,   49, 
Three_Param_Datum,"PIT"  ,  "PITCAIRN ASTRO 1967"            ,"IN",  185,    165,    42, 0, 0, 0, 1,  25,  25,     25,  -27,  -21, -134, -119, 
Three_Param_Datum,"PLN"  ,  "PICO DE LAS NIEVES, Canary Is." ,"IN", -307,    -92,   127, 0, 0, 0, 1,  25,  25,     25,   26,   31,  -20,  -12, 
Three_Param_Datum,"POS"  ,  "PORTO SANTO 1936, Madeira Is."  ,"IN", -499,   -249,   314, 0, 0, 0, 1,  25,  25,     25,   31,   35,  -18,  -15, 
Three_Param_Datum,"PRP-A",  "PROV. S AMERICAN 1956, Bolivia" ,"IN", -270,    188,  -388, 0, 0, 0, 1,   5,  11,     14,  -28,   -4,  -75,  -51, 
Three_Param_Datum,"PRP-B",  "PROV. S AMERICAN 1956, N Chile" ,"IN", -270,    183,  -390, 0, 0, 0, 1,  25,  25,     25,  -45,  -12,  -83,  -60, 
Three_Param_Datum,"PRP-C",  "PROV. S AMERICAN 1956, S Chile" ,"IN", -305,    243,  -442, 0, 0, 0, 1,  20,  20,     20,  -64,  -20,  -83,  -60, 
Three_Param_Datum,"PRP-D",  "PROV. S AMERICAN 1956, Colombia","IN", -282,    169,  -371, 0, 0, 0, 1,  15,  15,     15,  -10,   16,  -85,  -61, 
Three_Param_Datum,"PRP-E",  "PROV. S AMERICAN 1956, Ecuador" ,"IN", -278,    171,  -367, 0, 0, 0, 1,   3,   5,      3,  -11,    7,  -85,  -70, 
Three_Param_Datum,"PRP-F",  "PROV. S AMERICAN 1956, Guyana"  ,"IN", -298,    159,  -369, 0, 0, 0, 1,   6,  14,      5,   -4,   14,  -67,  -51, 
Three_Param_Datum,"PRP-G",  "PROV. S AMERICAN 1956, Peru"    ,"IN", -279,    175,  -379, 0, 0, 0, 1,   6,   8,     12,  -24,    5,  -87,  -63, 
Three_Param_Datum,"PRP-H",  "PROV. S AMERICAN 1956, Venez"   ,"IN", -295,    173,  -371, 0, 0, 0, 1,   9,  14,     15,   -5,   18,  -79,  -54, 
Three_Param_Datum,"PRP-M",  "PROV. S AMERICAN 1956, Mean"    ,"IN", -288,    175,  -376, 0, 0, 0, 1,  17,  27,     27,  -64,   18,  -87,  -51, 
Three_Param_Datum,"PTB"  ,  "POINT 58, Burkina Faso & Niger" ,"CD", -106,   -129,   165, 0, 0, 0, 1,  25,  25,     25,    0,   10,  -15,   25, 
Three_Param_Datum,"PTN"  ,  "POINT NOIRE 1948"               ,"CD", -148,     51,  -291, 0, 0, 0, 1,  25,  25,     25,  -11,   10,    5,   25, 
Three_Param_Datum,"PUK"  ,  "PULKOVO 1942, Russia"           ,"KA",   28,   -130,   -95, 0, 0, 0, 1,  -1,  -1,     -1,   36,   89, -180,  180, 
Three_Param_Datum,"PUR"  ,  "PUERTO RICO & Virgin Is."       ,"CC",   11,     72,  -101, 0, 0, 0, 1,   3,   3,      3,   16,   20,  -69,  -63, 
Three_Param_Datum,"QAT"  ,  "QATAR NATIONAL"                 ,"IN", -128,   -283,    22, 0, 0, 0, 1,  20,  20,     20,   19,   32,   45,   57, 
Three_Param_Datum,"QUO"  ,  "QORNOQ, South Greenland"        ,"IN",  164,    138,  -189, 0, 0, 0, 1,  25,  25,     32,   57,   85,  -77,   -7, 
Three_Param_Datum,"REU"  ,  "REUNION, Mascarene Is."         ,"IN",   94,   -948, -1262, 0, 0, 0, 1,  25,  25,     25,  -27,  -12,   47,   65, 
Three_Param_Datum,"SAE"  ,  "SANTO (DOS) 1965"               ,"IN",  170,     42,    84, 0, 0, 0, 1,  22,  25,     25,  -17,  -13,  160,  169, 
Three_Param_Datum,"SAO"  ,  "SAO BRAZ, Santa Maria Is."      ,"IN", -203,    141,    53, 0, 0, 0, 1,  25,  25,     25,   35,   39,  -27,  -23, 
Three_Param_Datum,"SAP"  ,  "SAPPER HILL 1943, E Falkland Is","IN", -355,     21,    72, 0, 0, 0, 1,   1,   1,      1,  -54,  -50,  -61,  -56, 
Three_Param_Datum,"SAN-M",  "SOUTH AMERICAN 1969, Mean"      ,"SA",  -57,      1,   -41, 0, 0, 0, 1,  15,   6,      9,  -65,  -50,  -90,  -25, 
Three_Param_Datum,"SAN-A",  "SOUTH AMERICAN 1969, Argentina" ,"SA",  -62,     -1,   -37, 0, 0, 0, 1,   5,   5,      5,  -62,  -23,  -76,  -47, 
Three_Param_Datum,"SAN-B",  "SOUTH AMERICAN 1969, Bolivia"   ,"SA",  -61,      2,   -48, 0, 0, 0, 1,  15,  15,     15,  -28,   -4,  -75,  -51, 
Three_Param_Datum,"SAN-C",  "SOUTH AMERICAN 1969, Brazil"    ,"SA",  -60,     -2,   -41, 0, 0, 0, 1,   3,   5,      5,  -39,   -2,  -80,  -29, 
Three_Param_Datum,"SAN-D",  "SOUTH AMERICAN 1969, Chile"     ,"SA",  -75,     -1,   -44, 0, 0, 0, 1,  15,   8,     11,  -64,  -12,  -83,  -60, 
Three_Param_Datum,"SAN-E",  "SOUTH AMERICAN 1969, Colombia"  ,"SA",  -44,      6,   -36, 0, 0, 0, 1,   6,   6,      5,  -10,   16,  -85,  -61, 
Three_Param_Datum,"SAN-F",  "SOUTH AMERICAN 1969, Ecuador"   ,"SA",  -48,      3,   -44, 0, 0, 0, 1,   3,   3,      3,  -11,    7,  -85,  -70, 
Three_Param_Datum,"SAN-G",  "SOUTH AMERICAN 1969, Guyana"    ,"SA",  -53,      3,   -47, 0, 0, 0, 1,   9,   5,      5,   -4,   14,  -67,  -51, 
Three_Param_Datum,"SAN-H",  "SOUTH AMERICAN 1969, Paraguay"  ,"SA",  -61,      2,   -33, 0, 0, 0, 1,  15,  15,     15,  -33,  -14,  -69,  -49, 
Three_Param_Datum,"SAN-I",  "SOUTH AMERICAN 1969, Peru"      ,"SA",  -58,      0,   -44, 0, 0, 0, 1,   5,   5,      5,  -24,    5,  -87,  -63, 
Three_Param_Datum,"SAN-J",  "SOUTH AMERICAN 1969, Baltra"    ,"SA",  -47,     26,   -42, 0, 0, 0, 1,  25,  25,     25,   -2,    1,  -92,  -89, 
Three_Param_Datum,"SAN-K",  "SOUTH AMERICAN 1969, Trinidad"  ,"SA",  -45,     12,   -33, 0, 0, 0, 1,  25,  25,     25,    4,   17,  -68,  -55, 
Three_Param_Datum,"SAN-L",  "SOUTH AMERICAN 1969, Venezuela" ,"SA",  -45,      8,   -33, 0, 0, 0, 1,   3,   6,      3,   -5,   18,  -79,  -54, 
Three_Param_Datum,"SCK"  ,  "SCHWARZECK, Namibia"            ,"BN",  616,     97,  -251, 0, 0, 0, 1,  20,  20,     20,  -35,  -11,    5,   31, 
Three_Param_Datum,"SGM"  ,  "SELVAGEM GRADE 1938, Salvage Is","IN", -289,   -124,    60, 0, 0, 0, 1,  25,  25,     25,   28,   32,  -18,  -14, 
Three_Param_Datum,"SHB"  ,  "ASTRO DOS 71/4, St. Helena Is." ,"IN", -320,    550,  -494, 0, 0, 0, 1,  25,  25,     25,  -18,  -14,   -7,   -4, 
Three_Param_Datum,"SOA"  ,  "SOUTH ASIA, Singapore"          ,"FA",    7,    -10,   -26, 0, 0, 0, 1,  25,  25,     25,    0,    3,  102,  106, 
Three_Param_Datum,"SPK-A",  "S-42 (PULKOVO 1942), Hungary"   ,"KA",   28,   -121,   -77, 0, 0, 0, 1,   2,   2,      2,   40,   54,   11,   29, 
Three_Param_Datum,"SPK-B",  "S-42 (PULKOVO 1942), Poland"    ,"KA",   23,   -124,   -82, 0, 0, 0, 1,   4,   2,      4,   43,   60,    8,   30, 
Three_Param_Datum,"SPK-C",  "S-42 (PK42) Former Czechoslov." ,"KA",   26,   -121,   -78, 0, 0, 0, 1,   3,   3,      2,   42,   57,    6,   28, 
Three_Param_Datum,"SPK-D",  "S-42 (PULKOVO 1942), Latvia"    ,"KA",   24,   -124,   -82, 0, 0, 0, 1,   2,   2,      2,   50,   64,   15,   34, 
Three_Param_Datum,"SPK-E",  "S-42 (PK 1942), Kazakhstan"     ,"KA",   15,   -130,   -84, 0, 0, 0, 1,  25,  25,     25,   35,   62,   41,   93, 
Three_Param_Datum,"SPK-F",  "S-42 (PULKOVO 1942), Albania"   ,"KA",   24,   -130,   -92, 0, 0, 0, 1,   3,   3,      3,   34,   48,   14,   26, 
Three_Param_Datum,"SPK-G",  "S-42 (PULKOVO 1942), Romania"   ,"KA",   28,   -121,   -77, 0, 0, 0, 1,   3,   5,      3,   38,   54,   15,   35, 
Three_Param_Datum,"SRL"  ,  "SIERRA LEONE 1960"              ,"CD",  -88,      4,   101, 0, 0, 0, 1,  15,  15,     15,    1,   16,  -19,   -4, 
Three_Param_Datum,"TAN"  ,  "TANANARIVE OBSERVATORY 1925"    ,"IN", -189,   -242,   -91, 0, 0, 0, 1,  -1,  -1,     -1,  -34,   -8,   40,   53, 
Three_Param_Datum,"TDC"  ,  "TRISTAN ASTRO 1968"             ,"IN", -632,    438,  -609, 0, 0, 0, 1,  25,  25,     25,  -39,  -36,  -14,  -11, 
Three_Param_Datum,"TIL"  ,  "TIMBALAI 1948, Brunei & E Malay","EA", -679,    669,   -48, 0, 0, 0, 1,  10,  10,     12,   -5,   15,  101,  125, 
Three_Param_Datum,"TOY-A",  "TOKYO, Japan"                   ,"BR", -148,    507,   685, 0, 0, 0, 1,   8,   5,      8,   19,   51,  119,  156, 
Three_Param_Datum,"TOY-B",  "TOKYO, South Korea"             ,"BR", -146,    507,   687, 0, 0, 0, 1,   8,   5,      8,   27,   45,  120,  139, 
Three_Param_Datum,"TOY-B1", "TOKYO, South Korea"             ,"BR", -147,    506,   687, 0, 0, 0, 1,   2,   2,      2,   27,   45,  120,  139, 
Three_Param_Datum,"TOY-C",  "TOKYO, Okinawa"                 ,"BR", -158,    507,   676, 0, 0, 0, 1,  20,   5,     20,   19,   31,  119,  134, 
Three_Param_Datum,"TOY-M",  "TOKYO, Mean"                    ,"BR", -148,    507,   685, 0, 0, 0, 1,  20,   5,     20,   23,   53,  120,  155, 
Three_Param_Datum,"TRN"  ,  "ASTRO TERN ISLAND (FRIG) 1961"  ,"IN",  114,   -116,  -333, 0, 0, 0, 1,  25,  25,     25,   22,   26, -166, -166, 
Three_Param_Datum,"VOI"  ,  "VOIROL 1874, Algeria"           ,"CD",  -73,   -247,   227, 0, 0, 0, 1,  -1,  -1,     -1,   13,   43,  -15,   11, 
Three_Param_Datum,"VOR"  ,  "VOIROL 1960, Algeria"           ,"CD", -123,   -206,   219, 0, 0, 0, 1,  25,  25,     25,   13,   43,  -15,   11, 
Three_Param_Datum,"WAK"  ,  "WAKE ISLAND ASTRO 1952"         ,"IN",  276,    -57,   149, 0, 0, 0, 1,  25,  25,     25,   17,   21, -176, -171, 
Three_Param_Datum,"YAC"  ,  "YACARE, Uruguay"                ,"IN", -155,    171,    37, 0, 0, 0, 1,  -1,  -1,     -1,  -40,  -25,  -65,  -47, 
Three_Param_Datum,"ZAN"  ,  "ZANDERIJ, Suriname"             ,"IN", -265,    120,  -358, 0, 0, 0, 1,   5,   5,      8,  -10,   20,  -76,  -47 
};// if new items added, be sure to update Datum_3Param_Count

static Datum_Row Datum_Table_7Param[MAX_7PARAM] = {
Seven_Param_Datum,"EUR-7", "EUROPEAN 1950, Mean (7 Param)"  ,"IN",  -102,  -102,  -129,  0.413, -0.184,  0.385,    0.0000024664, 0, 0,0, MIN_LAT, MAX_LAT, -PI, PI,
Seven_Param_Datum,"OGB-7", "ORDNANCE GB 1936, Mean (7 Para)","AA",   446,   -99,   544, -0.945, -0.261, -0.435,   -0.0000208927, 0, 0,0, MIN_LAT, MAX_LAT, -PI, PI
};// if new items added, be sure to update Datum_7Param_Count


/* World Geodetic Systems */
static Datum_Row WGS84;
static Datum_Row WGS72;
const char *WGS84_Datum_Code = "WGE";
const char *WGS72_Datum_Code = "WGD";
static int Datum_WGS84_Index = 0; /* Index of WGS84 in datum table */
static int Datum_WGS72_Index = 0; /* Index of WGS72 in datum table */
static const int Datum_3Param_Count = 224;
static const int Datum_7Param_Count = 2;
static Datum_Row *Datum_Table[2 + MAX_3PARAM + MAX_7PARAM]; /* Datum pointer array, for sorting */
static int Number_of_Datums = 0; /* Count for datum table */
static int Datum_Initialized = 0; /* Indicates successful initialization */


/***************************************************************************/
/*
 *                              FUNCTIONS     
 */
/*Forward function declarations */
void Geodetic_Shift_WGS84_To_WGS72( const double WGS84_Lat,
                                    const double WGS84_Lon,
                                    const double WGS84_Hgt,
                                    double *WGS72_Lat,
                                    double *WGS72_Lon,
                                    double *WGS72_Hgt);

void Geodetic_Shift_WGS72_To_WGS84( const double WGS72_Lat,
                                    const double WGS72_Lon,
                                    const double WGS72_Hgt,
                                    double *WGS84_Lat,
                                    double *WGS84_Lon,
                                    double *WGS84_Hgt);

void Assign_Datum_Row(Datum_Row *destination, Datum_Row *source)
{ /* Begin Assign_Datum_Row */
  /*
   * The function Assign_Datum_Row copies the data from source into destination.
   *
   * destination      : Pointer to the destination datum container.   (output)
   * source           : Pointer to the source datum container.        (input)
   */
  int i = 0;

  destination->Type = source->Type;
  strcpy(destination->Code, source->Code);
  strcpy(destination->Name, source->Name);
  strcpy(destination->Ellipsoid_Code, source->Ellipsoid_Code);

  for (i = 0; i < 7; i++)
  {
    destination->Parameters[i] = source->Parameters[i];
  }

  destination->Sigma_X = source->Sigma_X;
  destination->Sigma_Y = source->Sigma_Y;
  destination->Sigma_Z = source->Sigma_Z;
} /* End Assign_Datum_Row */



int Initialize_Datums(void)
{ /* Begin Initialize_Datums */
/*
 * The function Initialize_Datums creates the datum table from two external
 * files.  If an error occurs, the initialization stops and an error code is
 * returned.  This function must be called before any of the other functions
 * in this component.
 */
  int index = 0, i = 0;
  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    return (error_code);
  }

  /*  Check the environment for a user provided path, else current directory;   */
  /*  Build a File Name, including specified or default path:                   */


    /* Initialize array of pointers to datums */
    if (!error_code)
    {
      /* set total number of datums available */
      Number_of_Datums = Datum_3Param_Count + Datum_7Param_Count + 2;

      /* build WGS84 and WGS72 datum table entries */
      WGS84.Type = WGS84_Datum;
      strcpy(WGS84.Name,"World Geodetic System 1984");
      strcpy(WGS84.Code,"WGE");
      strcpy(WGS84.Ellipsoid_Code,"WE");
      WGS72.Type = WGS72_Datum;
      strcpy(WGS72.Name,"World Geodetic System 1972");
      strcpy(WGS72.Code,"WGD");
      strcpy(WGS72.Ellipsoid_Code,"WD");
      for (i=0; i<6; i++)
      {
        WGS84.Parameters[i] = 0.0;
        WGS72.Parameters[i] = 0.0;
      }
      WGS84.Parameters[6] = 1.0;
      WGS72.Parameters[6] = 1.0;

      WGS84.Sigma_X = 0.0;
      WGS84.Sigma_Y = 0.0;
      WGS84.Sigma_Z = 0.0;
      WGS84.South_latitude = -PI / 2.0;
      WGS84.North_latitude = +PI / 2.0;
      WGS84.West_longitude = -PI;
      WGS84.East_longitude = +PI;

      WGS72.Sigma_X = 0.0;
      WGS72.Sigma_Y = 0.0;
      WGS72.Sigma_Z = 0.0;
      WGS72.South_latitude = -PI / 2.0;
      WGS72.North_latitude = +PI / 2.0;
      WGS72.West_longitude = -PI;
      WGS72.East_longitude = +PI;

      Datum_WGS84_Index = 1;
      Datum_Table[Datum_WGS84_Index - 1] = &WGS84;
      Datum_WGS72_Index = 2;
      Datum_Table[Datum_WGS72_Index - 1] = &WGS72;
      index = 2;
      for (i = 0; i < Datum_7Param_Count; i++)
      {
        Datum_Table[index++] = &(Datum_Table_7Param[i]);
      }
      for (i = 0; i < Datum_3Param_Count; i++)
      {
        Datum_Table[index++] = &(Datum_Table_3Param[i]);
      }

      if (error_code)
      {
        error_code |= DATUM_ELLIPSE_ERROR;
        Datum_Initialized = 0;
        Number_of_Datums = 0;
      }
      else
        Datum_Initialized = 1;      
    }
  return (error_code);
}


int Datum_Count (int *Count)

{ /* Begin Datum_Count */
  /*
   *  The function Datum_Count returns the number of Datums in the table
   *  if the table was initialized without error.
   *
   *  Count                : number of datums in the datum table     (output)
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
    *Count = Number_of_Datums;
  else
  {
    *Count = 0;
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End of Datum_Count */


int Datum_Index( const char *Code,
                  int *Index )
{ /* Begin Datum_Index */
  /*
   *  The function Datum_Index returns the index of the datum with the 
   *  specified code.
   *
   *  Code    : The datum code being searched for.                    (input)
   *  Index   : The index of the datum in the table with the          (output)
   *              specified code.
   */

  char temp_code[DATUM_CODE_LENGTH];
  int error_code = DATUM_NO_ERROR;
  int length;
  int pos = 0;
  int i = 0;

  *Index = 0;
  if (Datum_Initialized)
  {
    length = strlen(Code);
    if (length > (DATUM_CODE_LENGTH-1))
      error_code |= DATUM_INVALID_CODE_ERROR;
    else
    {
      strcpy(temp_code,Code);

      /* Convert to upper case */
      for (i=0;i<length;i++)
        temp_code[i] = toupper(temp_code[i]);

      /* Strip blank spaces */
      while (pos < length)
      {
        if (isspace(temp_code[pos]))
        {
          for (i=pos;i<=length;i++)
            temp_code[i] = temp_code[i+1];
          length -= 1;
        }
        else
          pos += 1;
      }
      /* Search for code */
      i = 0;
      while (i < Number_of_Datums && strcmp(temp_code, Datum_Table[i]->Code))
      {
        i++;
      }
      if (i == Number_of_Datums || strcmp(temp_code, Datum_Table[i]->Code))
        error_code |= DATUM_INVALID_CODE_ERROR;
      else
        *Index = i+1;
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Index */


int Datum_Code (const int Index,
                 char *Code)
{ /* Begin Datum_Code */
  /*
   *  The function Datum_Code returns the 5-letter code of the datum
   *  referenced by index.
   *
   *  Index   : The index of a given datum in the datum table.        (input)
   *  Code    : The datum Code of the datum referenced by Index.      (output)
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if (Index > 0 && Index <= Number_of_Datums)
      strcpy(Code, Datum_Table[Index-1]->Code);
    else
      error_code |= DATUM_INVALID_INDEX_ERROR;
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Code */


int Datum_Name (const int Index,
                 char *Name)
{ /* Begin Datum_Name */
  /*
   *  The function Datum_Name returns the name of the datum referenced by
   *  index.
   *
   *  Index   : The index of a given datum in the datum table.        (input)
   *  Name    : The datum Name of the datum referenced by Index.      (output)
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if ((Index > 0) && (Index <= Number_of_Datums))
      strcpy(Name, Datum_Table[Index-1]->Name);
    else
      error_code |= DATUM_INVALID_INDEX_ERROR;
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Name */


int Datum_Ellipsoid_Code (const int Index,
                           char *Code)
{ /* Begin Datum_Ellipsoid_Code */
  /*
   *  The function Datum_Ellipsoid_Code returns the 2-letter ellipsoid code 
   *  for the ellipsoid associated with the datum referenced by index.
   *
   *  Index   : The index of a given datum in the datum table.          (input)
   *  Code    : The ellipsoid code for the ellipsoid associated with    (output)
   *               the datum referenced by index.
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Datums))
      error_code |= DATUM_INVALID_INDEX_ERROR;
    else
      strcpy(Code, Datum_Table[Index-1]->Ellipsoid_Code);
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Ellipsoid_Code */


int Get_Datum_Type (const int Index,
                     Datum_Type *Type)
{ /* Begin Datum_Type */
  /*
   *  The function Datum_Type returns the type of the datum referenced by
   *  index.
   *
   *  Index   : The index of a given datum in the datum table.        (input)
   *  Type    : The type of datum referenced by index.                (output)
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Datums))
      error_code |= DATUM_INVALID_INDEX_ERROR;
    else
      *Type = Datum_Table[Index-1]->Type;
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Type */


int Datum_Seven_Parameters (const int Index, 
                             double *Delta_X,                             
                             double *Delta_Y,
                             double *Delta_Z,
                             double *Rx, 
                             double *Ry, 
                             double *Rz, 
                             double *Scale_Factor)

{ /* Begin Datum_Seven_Parameters */
  /*
   *   The function Datum_Seven_Parameters returns the seven parameters 
   *   for the datum referenced by index.
   *
   *    Index      : The index of a given datum in the datum table.  (input)
   *    Delta_X    : X translation in meters                         (output)
   *    Delta_Y    : Y translation in meters                         (output)
   *    Delta_Z    : Z translation in meters                         (output)
   *    Rx         : X rotation in radians                           (output)
   *    Rx         : Y rotation in radians                           (output)
   *    Ry         : Z rotation in radians                           (output)
   *    Scale_Factor : Scale factor                                  (output)
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if (Index > 0 && Index <= Number_of_Datums)
    {
      *Delta_X = Datum_Table[Index-1]->Parameters[0];
      *Delta_Y = Datum_Table[Index-1]->Parameters[1];
      *Delta_Z = Datum_Table[Index-1]->Parameters[2];
      *Rx = Datum_Table[Index-1]->Parameters[3];
      *Ry = Datum_Table[Index-1]->Parameters[4];
      *Rz = Datum_Table[Index-1]->Parameters[5];
      *Scale_Factor = Datum_Table[Index-1]->Parameters[6];
    }
    else
    {
      error_code |= DATUM_INVALID_INDEX_ERROR;
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Seven_Parameters */


int Datum_Three_Parameters (const int Index, 
                             double *Delta_X,
                             double *Delta_Y,
                             double *Delta_Z)
{ /* Begin Datum_Three_Parameters */
  /*
   *   The function Datum_Three_Parameters returns the three parameters
   *   for the datum referenced by index.
   *
   *    Index      : The index of a given datum in the datum table.  (input)
   *    Delta_X    : X translation in meters                         (output)
   *    Delta_Y    : Y translation in meters                         (output)
   *    Delta_Z    : Z translation in meters                         (output)
   */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if (Index > 0 && Index <= Number_of_Datums)
    {
      *Delta_X = Datum_Table[Index-1]->Parameters[0];
      *Delta_Y = Datum_Table[Index-1]->Parameters[1];
      *Delta_Z = Datum_Table[Index-1]->Parameters[2];
    }
    else
    {
      error_code |= DATUM_INVALID_INDEX_ERROR;
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Three_Parameters */

int Datum_Errors (const int Index, 
                   double *Sigma_X,
                   double *Sigma_Y,
                   double *Sigma_Z)
{ /* Begin Datum_Errors */
/*
 *   The function Datum_Errors returns the standard errors in X,Y, & Z 
 *   for the datum referenced by index.
 *
 *    Index      : The index of a given datum in the datum table   (input)
 *    Sigma_X    : Standard error in X in meters                   (output)
 *    Sigma_Y    : Standard error in Y in meters                   (output)
 *    Sigma_Z    : Standard error in Z in meters                   (output)
 */

  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if (Index > 0 && Index <= Number_of_Datums)
    {
      *Sigma_X = Datum_Table[Index-1]->Sigma_X;
      *Sigma_Y = Datum_Table[Index-1]->Sigma_Y;
      *Sigma_Z = Datum_Table[Index-1]->Sigma_Z;
    }
    else
    {
      error_code |= DATUM_INVALID_INDEX_ERROR;
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Errors */


int Datum_Valid_Rectangle ( const int Index,
                             double *South_latitude,
                             double *North_latitude,
                             double *West_longitude,
                             double *East_longitude)
{ /* Begin Datum_Valid_Rectangle */
  /*
   *   The function Datum_Valid_Rectangle returns the edges of the validity 
   *   rectangle for the datum referenced by index.
   *
   *   Index          : The index of a given datum in the datum table   (input)
   *   South_latitude : Southern edge of validity rectangle in radians  (input)
   *   North_latitude : Northern edge of validity rectangle in radians  (input)
   *   West_longitude : Western edge of validity rectangle in radians   (input)
   *   East_longitude : Eastern edge of validity rectangle in radians   (input)
   */
  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if (Index > 0 && Index <= Number_of_Datums)
    {
      *South_latitude = Datum_Table[Index-1]->South_latitude;
      *North_latitude = Datum_Table[Index-1]->North_latitude;
      *West_longitude = Datum_Table[Index-1]->West_longitude;
      *East_longitude = Datum_Table[Index-1]->East_longitude;
    }
    else
    {
      error_code |= DATUM_INVALID_INDEX_ERROR;
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Valid_Rectangle */


int Valid_Datum(const int Index,
                 double latitude,
                 double longitude,
                 int *result)
{ /* Begin Valid_Datum */
  /*
   *  This function checks whether or not the specified location is within the 
   *  validity rectangle for the specified datum.  It returns zero if the specified
   *  location is NOT within the validity rectangle, and returns 1 otherwise.
   *
   *   Index     : The index of a given datum in the datum table      (input)
   *   latitude  : Latitude of the location to be checked in radians  (input)
   *   longitude : Longitude of the location to be checked in radians (input)
   *   result    : Indicates whether location is inside (1) or outside (0)
   *               of the validity rectangle of the specified datum   (output)
   */
  int error_code = DATUM_NO_ERROR;
  *result = 0;
  if (Datum_Initialized)
  {
    if (Index <= 0 && Index >= Number_of_Datums)
      error_code |= DATUM_INVALID_INDEX_ERROR;
    if ((latitude < MIN_LAT) || (latitude > MAX_LAT))
      error_code |= DATUM_LAT_ERROR;
    if ((longitude < MIN_LON) || (longitude > MAX_LON))
      error_code |= DATUM_LON_ERROR;
    if (!error_code)
    {
      if ((Datum_Table[Index-1]->South_latitude <= latitude) &&
          (latitude <= Datum_Table[Index-1]->North_latitude) &&
          (Datum_Table[Index-1]->West_longitude <= longitude) &&
          (longitude <= Datum_Table[Index-1]->East_longitude))
      {
        *result = 1;
      }
      else
      {
        *result = 0;
      }
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Valid_Datum */


void Geocentric_Shift_WGS72_To_WGS84(const double X,
                                     const double Y,
                                     const double Z,
                                     double *X_WGS84,
                                     double *Y_WGS84,
                                     double *Z_WGS84)

{ /* Begin Geocentric_Shift_WGS72_To_WGS84 */
  /*
   *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
   *  to WGS72 to a geocentric coordinate (X, Y, Z in meters) relative to WGS84.
   *
   *  X       : X coordinate relative to WGS72            (input)
   *  Y       : Y coordinate relative to WGS72            (input)
   *  Z       : Z coordinate relative to WGS72            (input)
   *  X_WGS84 : X coordinate relative to WGS84            (output)
   *  Y_WGS84 : Y coordinate relative to WGS84            (output)
   *  Z_WGS84 : Z coordinate relative to WGS84            (output)
   */
  double Lat_72; /* Latitude relative to WGS72                   */
  double Lon_72; /* Longitude relative to WGS72                  */
  double Hgt_72; /* Height relative to WGS72                     */
  double Lat_84; /* Latitude relative to WGS84                   */
  double Lon_84; /* Longitude relative to WGS84                  */
  double Hgt_84; /* Heightt relative to WGS84                    */
  double a_72;   /* Semi-major axis in meters of WGS72 ellipsoid */
  double b_72;   /* Semi-minor axis in meters of WGS72 ellipsoid */
  double a_84;   /* Semi-major axis in meters of WGS84 ellipsoid */
  double b_84;   /* Semi-minor axis in meters of WGS84 ellipsoid */

  /* Set WGS72 ellipsoid params */
  WGS72_Axes(&a_72, &b_72);
  Set_Geocentric_Parameters(a_72, b_72);
  Convert_Geocentric_To_Geodetic(X, Y, Z, &Lat_72, &Lon_72, &Hgt_72);
  Geodetic_Shift_WGS72_To_WGS84(Lat_72, Lon_72, Hgt_72, &Lat_84, &Lon_84,
                                &Hgt_84);
  /* Set WGS84 ellipsoid params */
  WGS84_Axes(&a_84, &b_84);
  Set_Geocentric_Parameters(a_84, b_84);
  Convert_Geodetic_To_Geocentric(Lat_84, Lon_84, Hgt_84, X_WGS84, Y_WGS84,
                                 Z_WGS84);
} /* End Geocentric_Shift_WGS72_To_WGS84 */


void Geocentric_Shift_WGS84_To_WGS72(const double X_WGS84,
                                     const double Y_WGS84,
                                     const double Z_WGS84,
                                     double *X,
                                     double *Y,
                                     double *Z)

{ /* Begin Geocentric_Shift_WGS84_To_WGS72 */
  /*
   *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
   *  to WGS84 to a geocentric coordinate (X, Y, Z in meters) relative to WGS72.
   *
   *  X_WGS84 : X coordinate relative to WGS84            (input)
   *  Y_WGS84 : Y coordinate relative to WGS84            (input)
   *  Z_WGS84 : Z coordinate relative to WGS84            (input)
   *  X       : X coordinate relative to WGS72            (output)
   *  Y       : Y coordinate relative to WGS72            (output)
   *  Z       : Z coordinate relative to WGS72            (output)
   */
  double Lat_72; /* Latitude relative to WGS72                   */
  double Lon_72; /* Longitude relative to WGS72                  */
  double Hgt_72; /* Height relative to WGS72                     */
  double Lat_84; /* Latitude relative to WGS84                   */
  double Lon_84; /* Longitude relative to WGS84                  */
  double Hgt_84; /* Heightt relative to WGS84                    */
  double a_72;   /* Semi-major axis in meters of WGS72 ellipsoid */
  double b_72;   /* Semi-minor axis in meters of WGS72 ellipsoid */
  double a_84;   /* Semi-major axis in meters of WGS84 ellipsoid */
  double b_84;   /* Semi-minor axis in meters of WGS84 ellipsoid */

  /* Set WGS84 ellipsoid params */
  WGS84_Axes(&a_84, &b_84);
  Set_Geocentric_Parameters(a_84, b_84);
  Convert_Geocentric_To_Geodetic(X_WGS84, Y_WGS84, Z_WGS84, &Lat_84, &Lon_84, &Hgt_84);
  Geodetic_Shift_WGS84_To_WGS72(Lat_84, Lon_84, Hgt_84, &Lat_72, &Lon_72,
                                &Hgt_72);
  /* Set WGS72 ellipsoid params */
  WGS72_Axes(&a_72, &b_72);
  Set_Geocentric_Parameters(a_72, b_72);
  Convert_Geodetic_To_Geocentric(Lat_72, Lon_72, Hgt_72, X, Y, Z);
} /* End Geocentric_Shift_WGS84_To_WGS72 */


int Geocentric_Shift_To_WGS84(const int Index,
                               const double X,
                               const double Y,
                               const double Z,
                               double *X_WGS84,
                               double *Y_WGS84,
                               double *Z_WGS84)

{ /* Begin Geocentric_Shift_To_WGS84 */
  /*
   *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
   *  to the datum referenced by index to a geocentric coordinate (X, Y, Z in
   *  meters) relative to WGS84.
   *
   *  Index   : Index of source datum                         (input)
   *  X       : X coordinate relative to the source datum     (input)
   *  Y       : Y coordinate relative to the source datum     (input)
   *  Z       : Z coordinate relative to the source datum     (input)
   *  X_WGS84 : X coordinate relative to WGS84                (output)
   *  Y_WGS84 : Y coordinate relative to WGS84                (output)
   *  Z_WGS84 : Z coordinate relative to WGS84                (output)
   */
  Datum_Row *local;
  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Datums))
      error_code |= DATUM_INVALID_SRC_INDEX_ERROR;
    if (!error_code)
    {
      local = Datum_Table[Index-1];
      switch (local->Type)
      {
      case WGS72_Datum:
        {
          Geocentric_Shift_WGS72_To_WGS84(X, Y, Z, X_WGS84, Y_WGS84, Z_WGS84);
          break;
        }
      case WGS84_Datum:
        {          
          *X_WGS84 = X;
          *Y_WGS84 = Y;
          *Z_WGS84 = Z;
          break;
        }
      case Seven_Param_Datum:
        {
          *X_WGS84 = X + local->Parameters[0] + local->Parameters[5] * Y
                     - local->Parameters[4] * Z + local->Parameters[6] * X;
          *Y_WGS84 = Y + local->Parameters[1] - local->Parameters[5] * X
                     + local->Parameters[3] * Z + local->Parameters[6] * Y;
          *Z_WGS84 = Z + local->Parameters[2] + local->Parameters[4] * X
                     - local->Parameters[3] * Y + local->Parameters[6] * Z;
          break;
        }
      case Three_Param_Datum:
        {
          *X_WGS84 = X + local->Parameters[0];
          *Y_WGS84 = Y + local->Parameters[1];
          *Z_WGS84 = Z + local->Parameters[2];
          break;
        }
      } /* End switch */
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Geocentric_Shift_To_WGS84 */


int Geocentric_Shift_From_WGS84(const double X_WGS84,
                                 const double Y_WGS84,
                                 const double Z_WGS84,
                                 const int Index,
                                 double *X,
                                 double *Y,
                                 double *Z)

{ /* Begin Geocentric_Shift_From_WGS84 */
  /*
   *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
   *  to WGS84 to a geocentric coordinate (X, Y, Z in meters) relative to the
   *  local datum referenced by index.
   *
   *  X_WGS84 : X coordinate relative to WGS84                      (input)
   *  Y_WGS84 : Y coordinate relative to WGS84                      (input)
   *  Z_WGS84 : Z coordinate relative to WGS84                      (input)
   *  Index   : Index of destination datum                          (input)
   *  X       : X coordinate relative to the destination datum      (output)
   *  Y       : Y coordinate relative to the destination datum      (output)
   *  Z       : Z coordinate relative to the destination datum      (output)
   */
  Datum_Row *local;
  int error_code = DATUM_NO_ERROR;

  if (Datum_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Datums))
      error_code |= DATUM_INVALID_DEST_INDEX_ERROR;
    if (!error_code)
    {
      local = Datum_Table[Index-1];
      switch (local->Type)
      {
      case WGS72_Datum:
        {
          Geocentric_Shift_WGS84_To_WGS72(X_WGS84, Y_WGS84, Z_WGS84, X, Y, Z);
          break;
        }
      case WGS84_Datum:
        {
          *X = X_WGS84;
          *Y = Y_WGS84;
          *Z = Z_WGS84;    
          break;
        }
      case Seven_Param_Datum:
        {
          *X = X_WGS84 - local->Parameters[0] - local->Parameters[5] * Y_WGS84
               + local->Parameters[4] * Z_WGS84 - local->Parameters[6] * X_WGS84;
          *Y = Y_WGS84 - local->Parameters[1] + local->Parameters[5] * X_WGS84
               - local->Parameters[3] * Z_WGS84 - local->Parameters[6] * Y_WGS84;
          *Z = Z_WGS84 - local->Parameters[2] - local->Parameters[4] * X_WGS84
               + local->Parameters[3] * Y_WGS84 - local->Parameters[6] * Z_WGS84;
          break;
        }
      case Three_Param_Datum:
        {
          *X = X_WGS84 - local->Parameters[0];
          *Y = Y_WGS84 - local->Parameters[1];
          *Z = Z_WGS84 - local->Parameters[2];
          break;
        }
      } /* End switch */
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Geocentric_Shift_From_WGS84 */


int Geocentric_Datum_Shift (const int Index_in,
                             const double X_in,
                             const double Y_in,
                             const double Z_in,
                             const int Index_out,
                             double *X_out,
                             double *Y_out,
                             double *Z_out)

{ /* Begin Geocentric_Datum_Shift */
  /*
   *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
   *  to the source datum to geocentric coordinate (X, Y, Z in meters) relative
   *  to the destination datum.
   *
   *  Index_in  : Index of source datum                      (input)
   *  X_in      : X coordinate relative to source datum      (input)
   *  Y_in      : Y coordinate relative to source datum      (input)
   *  Z_in      : Z coordinate relative to source datum      (input)
   *  Index_out : Index of destination datum                 (input)
   *  X_out     : X coordinate relative to destination datum (output)
   *  Y_out     : Y coordinate relative to destination datum (output)
   *  Z_out     : Z coordinate relative to destination datum (output)
   */
  int error_code = DATUM_NO_ERROR;
  double X_WGS84;
  double Y_WGS84;
  double Z_WGS84;

  if (Datum_Initialized)
  {
    if ((Index_in < 1) || (Index_in > Number_of_Datums))
      error_code |= DATUM_INVALID_SRC_INDEX_ERROR;
    if ((Index_out < 1) || (Index_out > Number_of_Datums))
      error_code |= DATUM_INVALID_DEST_INDEX_ERROR;
    if (!error_code)
    {
      if (Index_in == Index_out)
      {
        *X_out = X_in;
        *Y_out = Y_in;
        *Z_out = Z_in;  
      }
      else
      {
        Geocentric_Shift_To_WGS84(Index_in, X_in, Y_in, Z_in, &X_WGS84,
                                  &Y_WGS84,&Z_WGS84);
        Geocentric_Shift_From_WGS84(X_WGS84, Y_WGS84, Z_WGS84, Index_out,
                                    X_out, Y_out, Z_out);      
      }
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Geocentric_Datum_Shift */


void Geodetic_Shift_WGS72_To_WGS84( const double WGS72_Lat,
                                    const double WGS72_Lon,
                                    const double WGS72_Hgt,
                                    double *WGS84_Lat,
                                    double *WGS84_Lon,
                                    double *WGS84_Hgt)

{ /* Begin Geodetic_Shift_WGS72_To_WGS84 */
  /*
   *  This function shifts a geodetic coordinate (latitude, longitude in radians
   *  and height in meters) relative to WGS72 to a geodetic coordinate 
   *  (latitude, longitude in radians and height in meters) relative to WGS84.
   *
   *  WGS72_Lat : Latitude in radians relative to WGS72     (input)
   *  WGS72_Lon : Longitude in radians relative to WGS72    (input)
   *  WGS72_Hgt : Height in meters relative to WGS72        (input)
   *  WGS84_Lat : Latitude in radians relative to WGS84     (output)
   *  WGS84_Lon : Longitude in radians relative to WGS84    (output)
   *  WGS84_Hgt : Height in meters  relative to WGS84       (output)
   */
  double Delta_Lat;
  double Delta_Lon;
  double Delta_Hgt;
  double WGS84_a;       /* Semi-major axis of WGS84 ellipsoid               */
  double WGS84_b;       /* Semi-minor axiz of WGS84 ellipsoid               */
  double WGS84_f;       /* Flattening of WGS84 ellipsoid                    */
  double WGS72_a;       /* Semi-major axis of WGS72 ellipsoid               */
  double WGS72_b;       /* Semi-minor axiz of WGS72 ellipsoid               */
  double WGS72_f;       /* Flattening of WGS72 ellipsoid                    */
  double da;            /* WGS84_a - WGS72_a                                */
  double df;            /* WGS84_f - WGS72_f                                */
  double Q;
  double sin_Lat;
  double sin2_Lat;

  WGS84_Axes( &WGS84_a, &WGS84_b);  
  WGS84_Flattening( &WGS84_f);
  WGS72_Axes( &WGS72_a, &WGS72_b);  
  WGS72_Flattening( &WGS72_f);
  da = WGS84_a - WGS72_a;
  df = WGS84_f - WGS72_f;
  Q = PI /  648000;
  sin_Lat = sin(WGS72_Lat);
  sin2_Lat = sin_Lat * sin_Lat;

  Delta_Lat = (4.5 * cos(WGS72_Lat)) / (WGS72_a*Q) + (df * sin(2*WGS72_Lat)) / Q;
  Delta_Lat /= SECONDS_PER_RADIAN;
  Delta_Lon = 0.554 / SECONDS_PER_RADIAN;
  Delta_Hgt = 4.5 * sin_Lat + WGS72_a * df * sin2_Lat - da + 1.4;

  *WGS84_Lat = WGS72_Lat + Delta_Lat;
  *WGS84_Lon = WGS72_Lon + Delta_Lon;
  *WGS84_Hgt = WGS72_Hgt + Delta_Hgt;
} /* End Geodetic_Shift_WGS72_To_WGS84 */


void Geodetic_Shift_WGS84_To_WGS72( const double WGS84_Lat,
                                    const double WGS84_Lon,
                                    const double WGS84_Hgt,
                                    double *WGS72_Lat,
                                    double *WGS72_Lon,
                                    double *WGS72_Hgt)

{ /* Begin Geodetic_Shift_WGS84_To_WGS72 */
  /*
   *  This function shifts a geodetic coordinate (latitude, longitude in radians
   *  and height in meters) relative to WGS84 to a geodetic coordinate 
   *  (latitude, longitude in radians and height in meters) relative to WGS72.
   *
   *  WGS84_Lat : Latitude in radians relative to WGS84     (input)
   *  WGS84_Lon : Longitude in radians relative to WGS84    (input)
   *  WGS84_Hgt : Height in meters  relative to WGS84       (input)
   *  WGS72_Lat : Latitude in radians relative to WGS72     (output)
   *  WGS72_Lon : Longitude in radians relative to WGS72    (output)
   *  WGS72_Hgt : Height in meters relative to WGS72        (output)
   */
  double Delta_Lat;
  double Delta_Lon;
  double Delta_Hgt;
  double WGS84_a;       /* Semi-major axis of WGS84 ellipsoid               */
  double WGS84_b;       /* Semi-minor axis of WGS84 ellipsoid               */
  double WGS84_f;       /* Flattening of WGS84 ellipsoid                    */
  double WGS72_a;       /* Semi-major axis of WGS72 ellipsoid               */
  double WGS72_b;       /* Semi-minor axis of WGS72 ellipsoid               */
  double WGS72_f;       /* Flattening of WGS72 ellipsoid                    */
  double da;            /* WGS72_a - WGS84_a                                */
  double df;            /* WGS72_f - WGS84_f                                */
  double Q;
  double sin_Lat;
  double sin2_Lat;

  WGS84_Axes( &WGS84_a, &WGS84_b);  
  WGS84_Flattening( &WGS84_f);
  WGS72_Axes( &WGS72_a, &WGS72_b);  
  WGS72_Flattening( &WGS72_f);
  da = WGS72_a - WGS84_a;
  df = WGS72_f - WGS84_f;
  Q = PI / 648000;
  sin_Lat = sin(WGS84_Lat);
  sin2_Lat = sin_Lat * sin_Lat;

  Delta_Lat = (-4.5 * cos(WGS84_Lat)) / (WGS84_a*Q)
              + (df * sin(2*WGS84_Lat)) / Q;
  Delta_Lat /= SECONDS_PER_RADIAN;
  Delta_Lon = -0.554 / SECONDS_PER_RADIAN;
  Delta_Hgt = -4.5 * sin_Lat + WGS84_a * df * sin2_Lat - da - 1.4;

  *WGS72_Lat = WGS84_Lat + Delta_Lat;
  *WGS72_Lon = WGS84_Lon + Delta_Lon;
  *WGS72_Hgt = WGS84_Hgt + Delta_Hgt;
} /* End Geodetic_Shift_WGS84_To_WGS72 */


void Molodensky_Shift( const double a,
                       const double da,
                       const double f,
                       const double df,
                       const double dx,
                       const double dy,
                       const double dz,
                       const double Lat_in,
                       const double Lon_in,
                       const double Hgt_in,
                       double *Lat_out,
                       double *Lon_out,
                       double *Hgt_out)

{ /* Begin Molodensky_Shift */
  /*
   *  This function shifts geodetic coordinates using the Molodensky method.
   *
   *    a         : Semi-major axis of source ellipsoid in meters  (input)
   *    da        : Destination a minus source a                   (input)
   *    f         : Flattening of source ellipsoid                 (input)
   *    df        : Destination f minus source f                   (input)
   *    dx        : X coordinate shift in meters                   (input)
   *    dy        : Y coordinate shift in meters                   (input)
   *    dz        : Z coordinate shift in meters                   (input)
   *    Lat_in    : Latitude in radians.                           (input)
   *    Lon_in    : Longitude in radians.                          (input)
   *    Hgt_in    : Height in meters.                              (input)
   *    Lat_out   : Calculated latitude in radians.                (output)
   *    Lon_out   : Calculated longitude in radians.               (output)
   *    Hgt_out   : Calculated height in meters.                   (output)
   */
  double tLon_in;   /* temp longitude                                   */
  double e2;        /* Intermediate calculations for dp, dl               */
  double ep2;       /* Intermediate calculations for dp, dl               */
  double sin_Lat;   /* sin(Latitude_1)                                    */
  double sin2_Lat;  /* (sin(Latitude_1))^2                                */
  double sin_Lon;   /* sin(Longitude_1)                                   */
  double cos_Lat;   /* cos(Latitude_1)                                    */
  double cos_Lon;   /* cos(Longitude_1)                                   */
  double w2;        /* Intermediate calculations for dp, dl               */
  double w;         /* Intermediate calculations for dp, dl               */
  double w3;        /* Intermediate calculations for dp, dl               */
  double m;         /* Intermediate calculations for dp, dl               */
  double n;         /* Intermediate calculations for dp, dl               */
  double dp;        /* Delta phi                                          */
  double dp1;       /* Delta phi calculations                             */
  double dp2;       /* Delta phi calculations                             */
  double dp3;       /* Delta phi calculations                             */
  double dl;        /* Delta lambda                                       */
  double dh;        /* Delta height                                       */
  double dh1;       /* Delta height calculations                          */
  double dh2;       /* Delta height calculations                          */

  if (Lon_in > PI)
    tLon_in = Lon_in - (2*PI);
  else
    tLon_in = Lon_in;
  e2 = 2 * f - f * f;
  ep2 = e2 / (1 - e2);
  sin_Lat = sin(Lat_in);
  cos_Lat = cos(Lat_in);
  sin_Lon = sin(tLon_in);
  cos_Lon = cos(tLon_in);
  sin2_Lat = sin_Lat * sin_Lat;
  w2 = 1.0 - e2 * sin2_Lat;
  w = sqrt(w2);
  w3 = w * w2;
  m = (a * (1.0 - e2)) / w3;
  n = a / w;
  dp1 = cos_Lat * dz - sin_Lat * cos_Lon * dx - sin_Lat * sin_Lon * dy;
  dp2 = ((e2 * sin_Lat * cos_Lat) / w) * da;
  dp3 = sin_Lat * cos_Lat * (2.0 * n + ep2 * m * sin2_Lat) * (1.0 - f) * df;
  dp = (dp1 + dp2 + dp3) / (m + Hgt_in);
  dl = (-sin_Lon * dx + cos_Lon * dy) / ((n + Hgt_in) * cos_Lat);
  dh1 = (cos_Lat * cos_Lon * dx) + (cos_Lat * sin_Lon * dy) + (sin_Lat * dz);
  dh2 = -(w * da) + ((a * (1 - f)) / w) * sin2_Lat * df;
  dh = dh1 + dh2;
  *Lat_out = Lat_in + dp;
  *Lon_out = Lon_in + dl;
  *Hgt_out = Hgt_in + dh;
  if (*Lon_out > (PI * 2))
    *Lon_out -= 2*PI;
  if (*Lon_out < (- PI))
    *Lon_out += 2*PI;
} /* End Molodensky_Shift */


int Geodetic_Shift_To_WGS84( const int Index,
                              const double Lat_in,
                              const double Lon_in,
                              const double Hgt_in,
                              double *WGS84_Lat,
                              double *WGS84_Lon,
                              double *WGS84_Hgt)

{ /* Begin Geodetic_Shift_To_WGS84 */
  /*
   *  This function shifts geodetic coordinates relative to a given source datum
   *  to geodetic coordinates relative to WGS84.
   *
   *    Index     : Index of source datum                         (input)
   *    Lat_in    : Latitude in radians relative to source datum  (input)
   *    Lon_in    : Longitude in radians relative to source datum (input)
   *    Hgt_in    : Height in meters relative to source datum     (input)
   *    WGS84_Lat : Latitude in radians relative to WGS84         (output)
   *    WGS84_Lon : Longitude in radians relative to WGS84        (output)
   *    WGS84_Hgt : Height in meters relative to WGS84            (output)
   */
  double WGS84_a;   /* Semi-major axis of WGS84 ellipsoid in meters */
  double WGS84_b;   /* Semi-minor axis of WGS84 ellipsoid in meters */
  double WGS84_f;   /* Flattening of WGS84 ellisoid                 */
  double a;         /* Semi-major axis of ellipsoid in meters       */
  double b;         /* Semi-minor axis of ellipsoid in meters       */
  double da;        /* Difference in semi-major axes                */
  double f;         /* Flattening of ellipsoid                      */
  double df;        /* Difference in flattening                     */
  double dx;
  double dy;
  double dz;
  int E_Index;
  int error_code = DATUM_NO_ERROR;
  Datum_Row *local;

  if (Datum_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Datums))
      error_code |= DATUM_INVALID_SRC_INDEX_ERROR;
    if ((Lat_in < (-90*PI/180)) || (Lat_in > (90*PI/180)))
      error_code |= DATUM_LAT_ERROR;
    if ((Lon_in < (-PI)) || (Lon_in > (2*PI)))
      error_code |= DATUM_LON_ERROR;
    if (!error_code)
    {
      local = Datum_Table[Index-1];
      switch (local->Type)
      {
      case WGS72_Datum:
        { /* Special case for WGS72 */
          Geodetic_Shift_WGS72_To_WGS84(Lat_in, Lon_in, Hgt_in, WGS84_Lat,
                                        WGS84_Lon, WGS84_Hgt);
          break;
        }
      case WGS84_Datum:
        {        /* Just copy */
          *WGS84_Lat = Lat_in;
          *WGS84_Lon = Lon_in;
          *WGS84_Hgt = Hgt_in;
          break;
        }
      case Seven_Param_Datum:
      case Three_Param_Datum:
        {
          if (Ellipsoid_Index(local->Ellipsoid_Code, &E_Index))
            error_code |= DATUM_ELLIPSE_ERROR;
          if (Ellipsoid_Axes( E_Index, &a, &b))
            error_code |= DATUM_ELLIPSE_ERROR;
          if (!error_code)
          {
            if ((local->Type == Seven_Param_Datum) ||
                (Lat_in < (-MOLODENSKY_MAX)) || 
                (Lat_in > MOLODENSKY_MAX))
            { /* Use 3-step method */
              double local_X;
              double local_Y;
              double local_Z;
              double WGS84_X;
              double WGS84_Y;
              double WGS84_Z;
              Set_Geocentric_Parameters(a,b);
              Convert_Geodetic_To_Geocentric(Lat_in, Lon_in, Hgt_in,
                                             &local_X, &local_Y, &local_Z);
              Geocentric_Shift_To_WGS84(Index,
                                        local_X, local_Y, local_Z,
                                        &WGS84_X, &WGS84_Y, &WGS84_Z);
              WGS84_Axes( &WGS84_a, &WGS84_b);
              Set_Geocentric_Parameters(WGS84_a, WGS84_b);
              Convert_Geocentric_To_Geodetic(WGS84_X, WGS84_Y, WGS84_Z,
                                             WGS84_Lat, WGS84_Lon, WGS84_Hgt);
            }
            else
            { /* Use Molodensky's method */
              WGS84_Axes( &WGS84_a, &WGS84_b);
              WGS84_Flattening( &WGS84_f);
              da = WGS84_a - a;
              f = (a - b) / a;
              df = WGS84_f - f;
              dx = local->Parameters[0];
              dy = local->Parameters[1];
              dz = local->Parameters[2];
              Molodensky_Shift(a, da, f, df, dx, dy, dz, Lat_in, Lon_in, 
                               Hgt_in, WGS84_Lat, WGS84_Lon, WGS84_Hgt);
            }
          }
          break;
        }
      } /* End switch */
    }
  } /* End if (Datum_Initialized) */
  return (error_code);
} /* End Geodetic_Shift_To_WGS84 */


int Geodetic_Shift_From_WGS84( const double WGS84_Lat,
                                const double WGS84_Lon,
                                const double WGS84_Hgt,
                                const int Index,
                                double *Lat_out,
                                double *Lon_out,
                                double *Hgt_out)

{ /* Begin Geodetic_Shift_From_WGS84 */
  /*
   *    WGS84_Lat : Latitude in radians relative to WGS84              (input)
   *    WGS84_Lon : Longitude in radians relative to WGS84             (input)
   *    WGS84_Hgt : Height in meters  relative to WGS84                (input)
   *    Index     : Index of destination datum                         (input)
   *    Lat_out   : Latitude in radians relative to destination datum  (output)
   *    Lon_out   : Longitude in radians relative to destination datum (output)
   *    Hgt_out   : Height in meters relative to destination datum     (output)
   *
   *  This function shifts geodetic coordinates relative to a WGS84 
   *  to geodetic coordinates relative to a given local datum.
   */
  double WGS84_a;   /* Semi-major axis of WGS84 ellipsoid in meters */
  double WGS84_b;   /* Semi-minor axis of WGS84 ellipsoid in meters */
  double WGS84_f;   /* Flattening of WGS84 ellisoid                 */
  double a;         /* Semi-major axis of ellipsoid in meters       */
  double b;         /* Semi-minor axis of ellipsoid in meters       */
  double da;        /* Difference in semi-major axes                */
  double f;         /* Flattening of ellipsoid                      */
  double df;        /* Difference in flattening                     */
  double dx;
  double dy;
  double dz;
  int E_Index;
  int error_code = DATUM_NO_ERROR;
  Datum_Row *local;

  if (Datum_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Datums))
      error_code |= DATUM_INVALID_DEST_INDEX_ERROR;
    if ((WGS84_Lat < (-90*PI/180)) || (WGS84_Lat > (90*PI/180)))
      error_code |= DATUM_LAT_ERROR;
    if ((WGS84_Lon < (-PI)) || (WGS84_Lon > (2*PI)))
      error_code |= DATUM_LON_ERROR;
    if (!error_code)
    {
      local = Datum_Table[Index-1];
      switch (local->Type)
      {
      case WGS72_Datum:
        {
          Geodetic_Shift_WGS84_To_WGS72( WGS84_Lat, WGS84_Lon, WGS84_Hgt,
                                         Lat_out, Lon_out, Hgt_out);
          break;
        }
      case WGS84_Datum:
        {          
          *Lat_out = WGS84_Lat;
          *Lon_out = WGS84_Lon;
          *Hgt_out = WGS84_Hgt;
          break;
        }
      case Seven_Param_Datum:
      case Three_Param_Datum:
        {
          if (Ellipsoid_Index(local->Ellipsoid_Code, &E_Index))
            error_code |= DATUM_ELLIPSE_ERROR;
          if (Ellipsoid_Axes(E_Index, &a, &b))
            error_code |= DATUM_ELLIPSE_ERROR;
          if (!error_code)
          {
            if ((local->Type == Seven_Param_Datum) ||
                (WGS84_Lat < (-MOLODENSKY_MAX)) || 
                (WGS84_Lat > MOLODENSKY_MAX))
            { /* Use 3-step method */
              double local_X;
              double local_Y;
              double local_Z;
              double WGS84_X;
              double WGS84_Y;
              double WGS84_Z;
              WGS84_Axes(&WGS84_a, &WGS84_b);
              Set_Geocentric_Parameters(WGS84_a, WGS84_b);
              Convert_Geodetic_To_Geocentric(WGS84_Lat, WGS84_Lon, WGS84_Hgt,
                                             &WGS84_X, &WGS84_Y, &WGS84_Z);
              Geocentric_Shift_From_WGS84(WGS84_X, WGS84_Y, WGS84_Z,
                                          Index, &local_X, &local_Y, &local_Z);
              Set_Geocentric_Parameters(a, b);
              Convert_Geocentric_To_Geodetic(local_X, local_Y, local_Z,
                                             Lat_out, Lon_out, Hgt_out);
            }
            else
            { /* Use Molodensky's method */
              WGS84_Axes(&WGS84_a, &WGS84_b);
              WGS84_Flattening(&WGS84_f);
              da = a - WGS84_a;
              f = (a - b) / a;
              df = f - WGS84_f;
              dx = -(local->Parameters[0]);
              dy = -(local->Parameters[1]);
              dz = -(local->Parameters[2]);
              Molodensky_Shift(WGS84_a, da, WGS84_f, df, dx, dy, dz, 
                               WGS84_Lat, WGS84_Lon, WGS84_Hgt, Lat_out, Lon_out, Hgt_out);
            }
          }
          break;
        }
      } /* End switch */
    }
  } /* End if (Datum_Initialized) */
  return (error_code);
} /* End Geodetic_Shift_From_WGS84 */


int Geodetic_Datum_Shift ( const int Index_in,
                            const double Lat_in,
                            const double Lon_in,
                            const double Hgt_in,
                            const int Index_out,
                            double *Lat_out,
                            double *Lon_out,
                            double *Hgt_out)

{ /* Begin Geodetic_Datum_Shift */
  /*
   *  This function shifts geodetic coordinates (latitude, longitude in radians
   *  and height in meters) relative to the source datum to geodetic coordinates
   *  (latitude, longitude in radians and height in meters) relative to the
   *  destination datum.
   *
   *  Index_in  : Index of source datum                               (input)
   *  Lat_in    : Latitude in radians relative to source datum        (input)
   *  Lon_in    : Longitude in radians relative to source datum       (input)
   *  Hgt_in    : Height in meters relative to source datum           (input)
   *  Index_out : Index of destination datum                          (input)
   *  Lat_out   : Latitude in radians relative to destination datum   (output)
   *  Lon_out   : Longitude in radians relative to destination datum  (output)
   *  Hgt_out   : Height in meters relative to destination datum      (output)
   */
  int error_code = DATUM_NO_ERROR;
  double WGS84_Lat; /* Latitude in radians relative to WGS84   */
  double WGS84_Lon; /* Longitude in radians relative to WGS84  */
  double WGS84_Hgt; /* Height in meters relative to WGS84      */
  Datum_Row *In_Datum;
  Datum_Row *Out_Datum;
  int E_Index;
  double a;
  double b;
  double X1;
  double X2;
  double Y1;
  double Y2;
  double Z1;
  double Z2;

  if (Datum_Initialized)
  {
    if ((Index_in < 1) || (Index_in > Number_of_Datums))
      error_code |= DATUM_INVALID_SRC_INDEX_ERROR;
    if ((Index_out < 1) || (Index_out > Number_of_Datums))
      error_code |= DATUM_INVALID_DEST_INDEX_ERROR;
    if ((Lat_in < (-90*PI/180)) || (Lat_in > (90*PI/180)))
      error_code |= DATUM_LAT_ERROR;
    if ((Lon_in < (-PI)) || (Lon_in > (2*PI)))
      error_code |= DATUM_LON_ERROR;
    if (!error_code)
    {
      In_Datum = Datum_Table[Index_in-1];
      Out_Datum = Datum_Table[Index_out-1];
      if (Index_in == Index_out)
      { /* Just copy */
        *Lat_out = Lat_in;
        *Lon_out = Lon_in;
        *Hgt_out = Hgt_in;  
      }
      else if (In_Datum->Type == Seven_Param_Datum)
      {
        if (Ellipsoid_Index(In_Datum->Ellipsoid_Code, &E_Index))
          error_code |= DATUM_ELLIPSE_ERROR;
        if (Ellipsoid_Axes(E_Index, &a, &b))
          error_code |= DATUM_ELLIPSE_ERROR;
        Set_Geocentric_Parameters(a, b);
        Convert_Geodetic_To_Geocentric(Lat_in, Lon_in, Hgt_in, &X1, &Y1, &Z1);
        if (Out_Datum->Type == Seven_Param_Datum)
        { /* Use 3-step method for both stages */
          Geocentric_Datum_Shift(Index_in, X1, Y1, Z1, Index_out, &X2, &Y2, &Z2);
          if (Ellipsoid_Index(Out_Datum->Ellipsoid_Code, &E_Index))
            error_code |= DATUM_ELLIPSE_ERROR;
          if (Ellipsoid_Axes(E_Index, &a, &b))
            error_code |= DATUM_ELLIPSE_ERROR;
          Set_Geocentric_Parameters(a,b);
          Convert_Geocentric_To_Geodetic(X2, Y2, Z2, Lat_out, Lon_out, Hgt_out);
        }
        else
        { /* Use 3-step method for 1st stage, Molodensky if possible for 2nd stage */
          Geocentric_Shift_To_WGS84(Index_in, X1, Y1, Z1, &X2, &Y2, &Z2);
          WGS84_Axes( &a, &b);
          Set_Geocentric_Parameters(a, b);
          Convert_Geocentric_To_Geodetic(X2, Y2, Z2, &WGS84_Lat, &WGS84_Lon, &WGS84_Hgt);
          Geodetic_Shift_From_WGS84(WGS84_Lat, WGS84_Lon, WGS84_Hgt, Index_out,
                                    Lat_out, Lon_out, Hgt_out);

        }
      }
      else if (Out_Datum->Type == Seven_Param_Datum)
      { /* Use Molodensky if possible for 1st stage, 3-step method for 2nd stage */
        Geodetic_Shift_To_WGS84(Index_in, Lat_in, Lon_in,
                                Hgt_in, &WGS84_Lat, &WGS84_Lon, &WGS84_Hgt);
        WGS84_Axes( &a, &b);
        Set_Geocentric_Parameters(a, b);
        Convert_Geodetic_To_Geocentric(WGS84_Lat, WGS84_Lon, WGS84_Hgt, &X1, &Y1, &Z1);
        Geocentric_Shift_From_WGS84(X1, Y1, Z1, Index_out, &X2, &Y2, &Z2);
        if (Ellipsoid_Index(Out_Datum->Ellipsoid_Code, &E_Index))
          error_code |= DATUM_ELLIPSE_ERROR;
        if (Ellipsoid_Axes(E_Index, &a, &b))
          error_code |= DATUM_ELLIPSE_ERROR;
        Set_Geocentric_Parameters(a,b);
        Convert_Geocentric_To_Geodetic(X2, Y2, Z2, Lat_out, Lon_out, Hgt_out);
      }
      else
      { /* Use Molodensky if possible for both stages */
        error_code |= Geodetic_Shift_To_WGS84(Index_in, Lat_in, Lon_in,
                                              Hgt_in, &WGS84_Lat, &WGS84_Lon, &WGS84_Hgt);
        if (!error_code)
          error_code |= Geodetic_Shift_From_WGS84(WGS84_Lat, WGS84_Lon,
                                                  WGS84_Hgt, Index_out, Lat_out, Lon_out, Hgt_out);
      }
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Geodetic_Datum_Shift */


int Datum_Shift_Error (const int Index_in,
                        const int Index_out,
                        double latitude,
                        double longitude,
                        double *ce90,
                        double *le90,
                        double *se90)
/*
 *  This function returns the 90% horizontal (circular), vertical (linear), and 
 *  spherical errors for a shift from the specified source datum to the 
 *  specified destination datum at the specified location.
 *
 *  Index_in  : Index of source datum                                      (input)
 *  Index_out : Index of destination datum                                 (input)
 *  latitude  : Latitude of point being converted in radians               (input)
 *  longitude : Longitude of point being converted in radians              (input)
 *  ce90      : Combined 90% circular horizontal error in meters           (output)
 *  le90      : Combined 90% linear vertical error in meters               (output)
 *  se90      : Combined 90% spherical error in meters                     (output)
 */
{ /* Begin Datum_Shift_Error */
  int error_code = DATUM_NO_ERROR;
  Datum_Row *In_Datum;
  Datum_Row *Out_Datum;
  double sinlat = sin(latitude);
  double coslat = cos(latitude);
  double sinlon = sin(longitude);
  double coslon = cos(longitude);
  double sigma_delta_lat;
  double sigma_delta_lon;
  double sigma_delta_height;
  double sx, sy, sz;
  double ce90_in = -1.0;
  double le90_in = -1.0;
  double se90_in = -1.0;
  double ce90_out = -1.0;
  double le90_out = -1.0;
  double se90_out = -1.0;

  *ce90 = -1.0;
  *le90 = -1.0;
  *se90 = -1.0;
  if (Datum_Initialized)
  {
    if ((Index_in < 1) || (Index_in > Number_of_Datums))
      error_code |= DATUM_INVALID_SRC_INDEX_ERROR;
    if ((Index_out < 1) || (Index_out > Number_of_Datums))
      error_code |= DATUM_INVALID_DEST_INDEX_ERROR;
    if ((latitude < (-90*PI/180)) || (latitude > (90*PI/180)))
      error_code |= DATUM_LAT_ERROR;
    if ((longitude < (-PI)) || (longitude > (2*PI)))
      error_code |= DATUM_LON_ERROR;
    if (!error_code)
    {
      In_Datum = Datum_Table[Index_in-1];
      Out_Datum = Datum_Table[Index_out-1];

      if (Index_in == Index_out)
      { /* Just copy */
        *ce90 = 1.0;
        *le90 = 1.0;
        *se90 = 1.0;
      }
      else
      {
        /* calculate input datum errors */
        switch (In_Datum->Type)
        {
        case WGS84_Datum:
        case WGS72_Datum:
        case Seven_Param_Datum:
          {
            ce90_in = 0.0;
            le90_in = 0.0;
            se90_in = 0.0;
            break;
          }
        case Three_Param_Datum:
          {
            if ((In_Datum->Sigma_X < 0)
                ||(In_Datum->Sigma_Y < 0)
                ||(In_Datum->Sigma_Z < 0))
            {
              ce90_in = -1.0;
              le90_in = -1.0;
              se90_in = -1.0;
            }
            else
            {
              sx = (In_Datum->Sigma_X * sinlat * coslon);
              sy = (In_Datum->Sigma_Y * sinlat * sinlon);
              sz = (In_Datum->Sigma_Z * coslat);
              sigma_delta_lat = sqrt((sx * sx) + (sy * sy) + (sz * sz));
              sx = (In_Datum->Sigma_X * sinlon);
              sy = (In_Datum->Sigma_Y * coslon);
              sigma_delta_lon = sqrt((sx * sx) + (sy * sy));
              sx = (In_Datum->Sigma_X * coslat * coslon);
              sy = (In_Datum->Sigma_Y * coslat * sinlon);
              sz = (In_Datum->Sigma_Z * sinlat);
              sigma_delta_height = sqrt((sx * sx) + (sy * sy) + (sz * sz));
              ce90_in = 2.146 * (sigma_delta_lat + sigma_delta_lon) / 2.0;
              le90_in = 1.6449 * sigma_delta_height;
              se90_in = 2.5003 * (In_Datum->Sigma_X + In_Datum->Sigma_Y + In_Datum->Sigma_Z) / 3.0;
            }
            break;
          }
        } /* End switch */
        /* calculate output datum errors */
        switch (Out_Datum->Type)
        {
        case WGS84_Datum:
        case WGS72_Datum:
        case Seven_Param_Datum:
          {
            ce90_out = 0.0;
            le90_out = 0.0;
            se90_out = 0.0;
            break;
          }
        case Three_Param_Datum:
          {
            if ((Out_Datum->Sigma_X < 0)
                ||(Out_Datum->Sigma_Y < 0)
                ||(Out_Datum->Sigma_Z < 0))
            {
              ce90_out = -1.0;
              le90_out = -1.0;
              se90_out = -1.0;
            }
            else
            {
              sx = (Out_Datum->Sigma_X * sinlat * coslon);
              sy = (Out_Datum->Sigma_Y * sinlat * sinlon);
              sz = (Out_Datum->Sigma_Z * coslat);
              sigma_delta_lat = sqrt((sx * sx) + (sy * sy) + (sz * sz));
              sx = (Out_Datum->Sigma_X * sinlon);
              sy = (Out_Datum->Sigma_Y * coslon);
              sigma_delta_lon = sqrt((sx * sx) + (sy * sy));
              sx = (Out_Datum->Sigma_X * coslat * coslon);
              sy = (Out_Datum->Sigma_Y * coslat * sinlon);
              sz = (Out_Datum->Sigma_Z * sinlat);
              sigma_delta_height = sqrt((sx * sx) + (sy * sy) + (sz * sz));
              ce90_out = 2.146 * (sigma_delta_lat + sigma_delta_lon) / 2.0;
              le90_out = 1.6449 * sigma_delta_height;
              se90_out = 2.5003 * (Out_Datum->Sigma_X + Out_Datum->Sigma_Y + Out_Datum->Sigma_Z) / 3.0;
            }
            break;
          }
        } /* End switch */
        /* combine errors */
        if ((ce90_in < 0.0) || (ce90_out < 0.0))
        {
          *ce90 = -1.0;
          *le90 = -1.0;
          *se90 = -1.0;
        }
        else
        {
          *ce90 = sqrt((ce90_in * ce90_in) + (ce90_out * ce90_out));
          if (*ce90 < 1.0)
          {
            *ce90 = 1.0;
          }
          *le90 = sqrt((le90_in * le90_in) + (le90_out * le90_out));
          if (*le90 < 1.0)
          {
            *le90 = 1.0;
          }
          *se90 = sqrt((se90_in * se90_in) + (se90_out * se90_out));
          if (*se90 < 1.0)
          {
            *se90 = 1.0;
          }
        }
      }/* End else */
    }
  }
  else
  {
    error_code |= DATUM_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Datum_Shift_Error */

/****************************************************************************/
/* RSC IDENTIFIER:  Ellipsoid
 *
* ABSTRACT
 *
 *    The purpose of ELLIPSOID is to provide access to ellipsoid parameters 
 *    for a collection of common ellipsoids.  A particular ellipsoid can be 
 *    accessed by using its standard 2-letter code to find its index in the 
 *    ellipsoid table.  The index can then be used to retrieve the ellipsoid 
 *    name and parameters.
 *
 *    By sequentially retrieving all of the ellipsoid codes and/or names, a 
 *    menu of the available ellipsoids can be constructed.  The index values 
 *    resulting from selections from this menu can then be used to access the 
 *    parameters of the selected ellipsoid.
 *
 *    This component depends on a data file named "ellips.dat", which contains
 *    the ellipsoid parameter values.  A copy of this file must be located in 
 *    the directory specified by the environment variable "ELLIPSOID_DATA", if 
 *    defined, or else in the current directory, whenever a program containing 
 *    this component is executed.
 *
 *    Additional ellipsoids can be added to this file, either manually or using 
 *    the Create_Ellipsoid function.  However, if a large number of ellipsoids 
 *    are added, the ellipsoid table array size in this component will have to 
 *    be increased.
 *
 * ERROR HANDLING
 *
 *    This component checks parameters for valid values.  If an invalid value
 *    is found, the error code is combined with the current error code using 
 *    the bitwise or.  This combining allows multiple error codes to be
 *    returned. The possible error codes are:
 *
 *  ELLIPSE_NO_ERROR             : No errors occured in function
 *  ELLIPSE_FILE_OPEN_ERROR      : Ellipsoid file opening error
 *  ELLIPSE_INITIALIZE_ERROR     : Ellipsoid table can not initialize
 *  ELLIPSE_TABLE_OVERFLOW_ERROR : Ellipsoid table overflow
 *  ELLIPSE_NOT_INITIALIZED_ERROR: Ellipsoid table not initialized properly
 *  ELLIPSE_INVALID_INDEX_ERROR  : Index is an invalid value
 *  ELLIPSE_INVALID_CODE_ERROR   : Code was not found in table
 *  ELLIPSE_A_ERROR              : Semi-major axis less than or equal to zero
 *  ELLIPSE_B_ERROR              : Semi-minor axis less than or equal to zero
 *  ELLIPSE_A_LESS_B_ERROR       : Semi-major axis less than semi-minor axis
 *
 * REUSE NOTES
 *
 *    Ellipsoid is intended for reuse by any application that requires Earth
 *    approximating ellipsoids.
 *     
 * REFERENCES
 *
 *    Further information on Ellipsoid can be found in the Reuse Manual.
 *
 *    Ellipsoid originated from :  U.S. Army Topographic Engineering Center (USATEC)
 *                                 Geospatial Information Division (GID)
 *                                 7701 Telegraph Road
 *                                 Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    Ellipsoid has no restrictions.
 *
 * ENVIRONMENT
 *
 *    Ellipsoid was tested and certified in the following environments
 *
 *    1. Solaris 2.5
 *    2. Windows 95 
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    11-19-95          Original Code
 *    17-Jan-97         Moved local constants out of public interface
 *                      Improved efficiency in algorithms (GEOTRANS)
 *    24-May-99         Added user-defined ellipsoids (GEOTRANS for JMTK)
 *
 */

/***************************************************************************/
/*
 *                             GLOBAL DATA STRUCTURES
 */

#define MAX_ELLIPSOIDS        32  /* Maximum number of ellipsoids in table */
//#define ELLIPSOID_CODE_LENGTH  3  /* Length of ellipsoid code (including null) */
#define ELLIPSOID_NAME_LENGTH 30  /* Max length of ellipsoid name */
//#define ELLIPSOID_BUF         90
#define FILENAME_LENGTH      128

const char *WGS84_Ellipsoid_Code = "WE";
const char *WGS72_Ellipsoid_Code = "WD";

typedef struct Ellipsoid_Table_Row
{
  char Name[ELLIPSOID_NAME_LENGTH];
  char Code[ELLIPSOID_CODE_LENGTH];
  double A;
  double B;
  double Recp_F;
} Ellipsoid_Row;

static Ellipsoid_Row Ellipsoid_Table[MAX_ELLIPSOIDS] = {
"Airy                         "           ,"AA", 6377563.396, 6356256.9090, 299.324964600,
"Modified Airy                "           ,"AM", 6377340.189, 6356034.4480, 299.324964600,
"Australian National          "           ,"AN", 6378160.000, 6356774.7190, 298.250000000,
"Bessel 1841                  "           ,"BR", 6377397.155, 6356078.9630, 299.152812800,
"Bessel 1841(Namibia)         "           ,"BN", 6377483.865, 6356165.3830, 299.152812800,
"Clarke 1866                  "           ,"CC", 6378206.400, 6356583.8000, 294.978698200,
"Clarke 1880                  "           ,"CD", 6378249.145, 6356514.8700, 293.465000000,
"Everest                      "           ,"EA", 6377276.345, 6356075.4130, 300.801700000,
"Everest (E. Malasia, Brunei) "           ,"EB", 6377298.556, 6356097.5500, 300.801700000,
"Everest 1956 (India)         "           ,"EC", 6377301.243, 6356100.2280, 300.801700000,
"Everest 1969 (West Malasia)  "           ,"ED", 6377295.664, 6356094.6680, 300.801700000,
"Everest 1948(W.Mals. & Sing.)"           ,"EE", 6377304.063, 6356103.0390, 300.801700000,
"Everest (Pakistan)           "           ,"EF", 6377309.613, 6356109.5710, 300.801700000,
"Mod. Fischer 1960(South Asia)"           ,"FA", 6378155.000, 6356773.3200, 298.300000000,
"GRS 80                       "           ,"RF", 6378137.000, 6356752.3141, 298.257222101,
"Helmert 1906                 "           ,"HE", 6378200.000, 6356818.1700, 298.300000000,
"Hough                        "           ,"HO", 6378270.000, 6356794.3430, 297.000000000,
"Indonesian 1974              "           ,"ID", 6378160.000, 6356774.5040, 298.247000000,
"International                "           ,"IN", 6378388.000, 6356911.9460, 297.000000000,
"Krassovsky                   "           ,"KA", 6378245.000, 6356863.0190, 298.300000000,
"South American 1969          "           ,"SA", 6378160.000, 6356774.7190, 298.250000000,
"WGS 72                       "           ,"WD", 6378135.000, 6356750.5200, 298.260000000,
"WGS 84                       "           ,"WE", 6378137.000, 6356752.3142, 298.257223563   
};
static int WGS84_Index = 0;           /* Index of WGS84 in ellipsoid table */
static int WGS72_Index = 0;           /* Index of WGS72 in ellipsoid table */
static int Number_of_Ellipsoids = 23;       /* Number of ellipsoids in table */
static int Ellipsoid_Initialized = 1; /* Indicates successful initialization */
                                       //was a 0 in the origional code
/***************************************************************************/
/*                              FUNCTIONS                                  */


void Assign_Ellipsoid_Row (Ellipsoid_Row *destination, 
                           const Ellipsoid_Row *source)
{ /* Begin Assign_Ellipsoid_Row */
/*
 *   destination  : The destination of the copy         (output)
 *   source       : The source for the copy             (input)
 *
 * The function Assign_Ellipsoid_Row copies ellipsoid data.
 */

  strcpy(destination->Name, source->Name);
  strcpy(destination->Code, source->Code);
  destination->A = source->A;
  destination->B = source->B;
  destination->Recp_F = source->Recp_F;
} /* End Assign_Ellipsoid_Row */


int Initialize_Ellipsoids() 
{ /* Begin Initialize_Ellipsoids */
/*
 * The function Initialize_Ellipsoids reads ellipsoid data from ellips.dat in
 * the current directory and builds the ellipsoid table from it.  If an 
 * error occurs, the error code is returned, otherwise ELLIPSE_NO_ERROR is 
 * returned.
 */

//  char buffer[ELLIPSOID_BUF];
  int index = 0;                     /* Array index                         */
  int error_code = ELLIPSE_NO_ERROR;

  if (Ellipsoid_Initialized)
  {
    return error_code;
  }

  /*  Check the environment for a user provided path, else current directory;   */
  /*  Build a File Name, including specified or default path:                   */


  /* Store WGS84 Index*/
  if (Ellipsoid_Index(WGS84_Ellipsoid_Code, &WGS84_Index))
    error_code |= ELLIPSE_INITIALIZE_ERROR;

  /* Store WGS72 Index*/
  if (Ellipsoid_Index(WGS72_Ellipsoid_Code, &WGS72_Index))
    error_code |= ELLIPSE_INITIALIZE_ERROR;

  return (error_code);
} /* End of Initialize_Ellipsoids */


//int Create_Ellipsoid (const char* Code,
//                       const char* Name,
//                       double A,
//                       double B)
//{ /* Begin Create_Ellipsoid */
/*
 *   Code     : 2-letter ellipsoid code.                      (input)
 *   Name     : Name of the new ellipsoid                     (input)
 *   A        : Semi-major axis, in meters, of new ellipsoid  (input)
 *   B        : Semi-minor axis, in meters, of new ellipsoid. (input)
 *
 * The function Create_Ellipsoid creates a new ellipsoid with the specified
 * Code, name, and axes.  If the ellipsoid table has not been initialized,
 * the specified code is already in use, or a new version of the ellips.dat 
 * file cannot be created, an error code is returned, otherwise ELLIPSE_NO_ERROR 
 * is returned.  Note that the indexes of all ellipsoids in the ellipsoid
 * table may be changed by this function.
 */
/*
  int error_code = ELLIPSE_NO_ERROR;
  int index = 0;
  char *PathName;
  char FileName[FILENAME_LENGTH];
  FILE *fp = NULL;                    /* File pointer to file ellips.dat     */
/*
  if (!Ellipsoid_Initialized)
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  else if (!(Number_of_Ellipsoids < MAX_ELLIPSOIDS))
    error_code |= ELLIPSE_TABLE_OVERFLOW_ERROR;
  else
  {
    if (!Ellipsoid_Index(Code, &index))
      error_code |= ELLIPSE_INVALID_CODE_ERROR;
    if (A <= 0.0)
      error_code |= ELLIPSE_A_ERROR;
    if (B <= 0.0)
      error_code |= ELLIPSE_B_ERROR;
    if (A < B)
      error_code |= ELLIPSE_A_LESS_B_ERROR;
    if (!error_code)
    {
      index = Number_of_Ellipsoids;
      strcpy(Ellipsoid_Table[index].Name, Name);
      strcpy(Ellipsoid_Table[index].Code, Code);
      Ellipsoid_Table[index].A = A;
      Ellipsoid_Table[index].B = B;
      Ellipsoid_Table[index].Recp_F = A / (A - B);
      Number_of_Ellipsoids++;

      /*output updated ellipsoid table*/
/*      PathName = getenv( "ELLIPSOID_DATA" );
      if (PathName != NULL)
      {
        strcpy( FileName, PathName );
        strcat( FileName, "/" );
      }
      else
      {
        strcpy( FileName, "./" );
      }
      strcat( FileName, "ellips.dat" );

      if ((fp = fopen(FileName, "w")) == NULL)
      { /* fatal error */
//        return ELLIPSE_FILE_OPEN_ERROR;
//      }

      /* write file */
/*      index = 0;
      while (index < Number_of_Ellipsoids)
      {
        fprintf(fp, "%-29s %-2s %11.3f %12.4f %13.9f \n",
                Ellipsoid_Table[index].Name,
                Ellipsoid_Table[index].Code,
                Ellipsoid_Table[index].A,
                Ellipsoid_Table[index].B,
                Ellipsoid_Table[index].Recp_F);
        index++;
      }
      fclose(fp);

      /* Store WGS84 */
//      Ellipsoid_Index(WGS84_Ellipsoid_Code, &WGS84_Index);

      /* Store WGS72 */
/*      Ellipsoid_Index(WGS72_Ellipsoid_Code, &WGS72_Index);
    }
  }
  return (error_code);
} /* End Create_Ellipsoid */


int Ellipsoid_Count ( int *Count )
{ /* Begin Ellipsoid_Count */
/*
 *   Count    : The number of ellipsoids in the ellipsoid table. (output)
 *
 * The function Ellipsoid_Count returns the number of ellipsoids in the
 * ellipsoid table.  If the ellipsoid table has been initialized without error,
 * ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR
 * is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  if (!Ellipsoid_Initialized)
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  *Count = Number_of_Ellipsoids;
  return (error_code);
} /* End Ellipsoid_Count */


int Ellipsoid_Index ( const char *Code,
                       int *Index )
{ /* Begin Ellipsoid_Index */
/*
 *    Code     : 2-letter ellipsoid code.                      (input)
 *    Index    : Index of the ellipsoid in the ellipsoid table with the 
 *                  specified code                             (output)
 *
 *  The function Ellipsoid_Index returns the index of the ellipsoid in 
 *  the ellipsoid table with the specified code.  If ellipsoid_Code is found, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_CODE_ERROR is 
 *  returned.
 */

  char temp_code[3];
  int error_code = ELLIPSE_NO_ERROR;
  int i = 0;                   /* index for ellipsoid table */
  int j = 0;

  *Index = 0;
  if (Ellipsoid_Initialized)
  {
    while (j < ELLIPSOID_CODE_LENGTH)
    {
      temp_code[j] = toupper(Code[j]);
      j++;
    }
    temp_code[ELLIPSOID_CODE_LENGTH - 1] = 0;
    while ((i < Number_of_Ellipsoids)
           && strcmp(temp_code, Ellipsoid_Table[i].Code))
    {
      i++;
    }
    if (strcmp(temp_code, Ellipsoid_Table[i].Code))
      error_code |= ELLIPSE_INVALID_CODE_ERROR;
    else
      *Index = i+1;
  }
  else
  {
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  }
  return (error_code);
} /* End Ellipsoid_Index */


int Ellipsoid_Name ( const int Index,
                      char *Name ) 
{ /* Begin Ellipsoid_Name */
/*
 *    Index   : Index of a given ellipsoid.in the ellipsoid table with the
 *                 specified index                             (input)
 *    Name    : Name of the ellipsoid referencd by index       (output)
 *
 *  The Function Ellipsoid_Name returns the name of the ellipsoid in 
 *  the ellipsoid table with the specified index.  If index is valid, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR is
 *  returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  strcpy(Name,"");
  if (Ellipsoid_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Ellipsoids))
      error_code |= ELLIPSE_INVALID_INDEX_ERROR;
    else
      strcpy(Name, Ellipsoid_Table[Index-1].Name);
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End Ellipsoid_Name */


int Ellipsoid_Code ( const int Index,
                      char *Code ) 
{ /* Begin Ellipsoid_Code */
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    Code     : 2-letter ellipsoid code.                          (output)
 *
 *  The Function Ellipsoid_Code returns the 2-letter code for the 
 *  ellipsoid in the ellipsoid table with the specified index.  If index is 
 *  valid, ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR 
 *  is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  strcpy(Code,"");
  if (Ellipsoid_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Ellipsoids))
      error_code |= ELLIPSE_INVALID_INDEX_ERROR;
    else
      strcpy(Code, Ellipsoid_Table[Index-1].Code);
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End Ellipsoid_Name */


int Ellipsoid_Axes ( const int Index,
                      double *A,
                      double *B )
{ /* Begin Ellipsoid_Axes */
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    a        : Semi-major axis, in meters, of ellipsoid          (output)
 *    B        : Semi-minor axis, in meters, of ellipsoid.         (output)
 *
 *  The function Ellipsoid_Axes returns the semi-major and semi-minor axes
 *  for the ellipsoid with the specified index.  If index is valid,
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR is 
 *  returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  *A = 0;
  *B = 0;
  if (Ellipsoid_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Ellipsoids))
    {
      error_code |= ELLIPSE_INVALID_INDEX_ERROR;
    }
    else
    {
      *A = Ellipsoid_Table[Index-1].A;
      *B = Ellipsoid_Table[Index-1].B;
    }
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End Ellipsoid_Axes */


int Ellipsoid_Eccentricity2 ( const int Index,
                               double *e2 )
{ /* Begin Ellipsoid_Eccentricity2 */
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    e2       : Square of eccentricity of ellipsoid               (output)
 *
 *  The function Ellipsoid_Eccentricity2 returns the square of the 
 *  eccentricity for the ellipsoid with the specified index.  If index is 
 *  valid, ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR 
 *  is returned.
 */
  double a2,b2;
  int error_code = ELLIPSE_NO_ERROR;

  *e2 = 0;
  if (Ellipsoid_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Ellipsoids))
    {
      error_code |= ELLIPSE_INVALID_INDEX_ERROR;
    }
    else
    {
      a2 = Ellipsoid_Table[Index-1].A * Ellipsoid_Table[Index-1].A;
      b2 = Ellipsoid_Table[Index-1].B * Ellipsoid_Table[Index-1].B;
      *e2 = (a2 - b2) / a2;
    }
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End Ellipsoid_Eccentricity2 */


int Ellipsoid_Flattening ( const int Index,
                            double *f )
{ /* Begin Ellipsoid_Flattening */
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    f        : Flattening of ellipsoid.                          (output)
 *
 *  The function Ellipsoid_Flattening returns the flattening of the 
 *  ellipsoid with the specified index.  If index is valid ELLIPSE_NO_ERROR is 
 *  returned, otherwise ELLIPSE_INVALID_INDEX_ERROR is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  *f = 0;
  if (Ellipsoid_Initialized)
  {
    if ((Index < 1) || (Index > Number_of_Ellipsoids))
      error_code |= ELLIPSE_INVALID_INDEX_ERROR;
    else
      *f = 1 / Ellipsoid_Table[Index-1].Recp_F;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End Ellipsoid_Flattening */


int WGS84_Axes ( double *A,
                  double *B)
{ /* Begin WGS84_Axes */
/*
 *    A      : Semi-major axis, in meters, of ellipsoid       (output)
 *    B      : Semi-minor axis, in meters, of ellipsoid       (output)
 *
 *  The function WGS84_Axes returns the lengths of the semi-major and 
 *  semi-minor axes for the WGS84 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  *A = 0;
  *B = 0;
  if (Ellipsoid_Initialized)
  {
    *A = Ellipsoid_Table[WGS84_Index-1].A;
    *B = Ellipsoid_Table[WGS84_Index-1].B;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End WGS84_Axes */


int WGS84_Eccentricity2 ( double *e2 )
{ /* Begin WGS84_Eccentricity2 */
/*
 *    e2    : Square of eccentricity of WGS84 ellipsoid      (output)
 *
 *  The function WGS84_Eccentricity2 returns the square of the 
 *  eccentricity for the WGS84 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;
  double a2, b2;

  *e2 = 0;
  if (Ellipsoid_Initialized)
  {
    a2 = Ellipsoid_Table[WGS84_Index-1].A * Ellipsoid_Table[WGS84_Index-1].A;
    b2 = Ellipsoid_Table[WGS84_Index-1].B * Ellipsoid_Table[WGS84_Index-1].B;
    *e2 = (a2 - b2) / a2;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End WGS84_Eccentricity2 */


int WGS84_Flattening ( double *f )
{ /* Begin WGS84_Flattening */
/*
 *  f       : Flattening of WGS84 ellipsoid.                 (output)
 *
 *  The function WGS84_Flattening returns the flattening of the WGS84
 *  ellipsoid.  If the ellipsoid table was initialized successfully, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR 
 *  is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  *f = 0;
  if (Ellipsoid_Initialized)
  {
    *f = 1 / Ellipsoid_Table[WGS84_Index-1].Recp_F;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End WGS84_Flattening */


int WGS72_Axes( double *A,
                 double *B)
{ /* Begin WGS72_Axes */
/*
 *    A    : Semi-major axis, in meters, of ellipsoid        (output)
 *    B    : Semi-minor axis, in meters, of ellipsoid        (output)
 *
 *  The function WGS72_Axes returns the lengths of the semi-major and 
 *  semi-minor axes for the WGS72 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;

  *A = 0;
  *B = 0;
  if (Ellipsoid_Initialized)
  {
    *A = Ellipsoid_Table[WGS72_Index-1].A;
    *B = Ellipsoid_Table[WGS72_Index-1].B;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End WGS72_Axes */


int WGS72_Eccentricity2 ( double *e2 )
{ /* Begin WGS72_Eccentricity2 */
/*
 *    e2     : Square of eccentricity of WGS84 ellipsoid     (output)
 *
 *  The function WGS72_Eccentricity2 returns the square of the 
 *  eccentricity for the WGS72 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */

  int error_code = ELLIPSE_NO_ERROR;
  double a2, b2;

  *e2 = 0;
  if (Ellipsoid_Initialized)
  {
    a2 = Ellipsoid_Table[WGS72_Index-1].A * Ellipsoid_Table[WGS72_Index-1].A;
    b2 = Ellipsoid_Table[WGS72_Index-1].B * Ellipsoid_Table[WGS72_Index-1].B;
    *e2 = (a2 - b2) / a2;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End WGS72_Eccentricity2 */


int WGS72_Flattening (double *f )
{ /* Begin WGS72_Flattening */
/*
 *    f      : Flattening of WGS72 ellipsoid.                (output)
 *
 *  The function WGS72_Flattening returns the flattening of the WGS72
 *  ellipsoid .  If the ellipsoid table was initialized successfully, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR 
 *  is returned.
 */
  int error_code = ELLIPSE_NO_ERROR;

  *f = 0;
  if (Ellipsoid_Initialized)
  {
    *f = 1 / Ellipsoid_Table[WGS72_Index-1].Recp_F;
  }
  else
    error_code |= ELLIPSE_NOT_INITIALIZED_ERROR;
  return (error_code);
} /* End WGS72_Flattening */


/***************************************************************************/
/* RSC IDENTIFIER:  GEOCENTRIC
 *
 * ABSTRACT
 *
 *    This component provides conversions between Geodetic coordinates (latitude,
 *    longitude in radians and height in meters) and Geocentric coordinates
 *    (X, Y, Z) in meters.
 *
 * ERROR HANDLING
 *
 *    This component checks parameters for valid values.  If an invalid value
 *    is found, the error code is combined with the current error code using 
 *    the bitwise or.  This combining allows multiple error codes to be
 *    returned. The possible error codes are:
 *
 *      GEOCENT_NO_ERROR        : No errors occurred in function
 *      GEOCENT_LAT_ERROR       : Latitude out of valid range
 *                                 (-90 to 90 degrees)
 *      GEOCENT_LON_ERROR       : Longitude out of valid range
 *                                 (-180 to 360 degrees)
 *      GEOCENT_A_ERROR         : Semi-major axis lessthan or equal to zero
 *      GEOCENT_B_ERROR         : Semi-minor axis lessthan or equal to zero
 *      GEOCENT_A_LESS_B_ERROR  : Semi-major axis less than semi-minor axis
 *
 *
 * REUSE NOTES
 *
 *    GEOCENTRIC is intended for reuse by any application that performs
 *    coordinate conversions between geodetic coordinates and geocentric
 *    coordinates.
 *    
 *
 * REFERENCES
 *    
 *    An Improved Algorithm for Geocentric to Geodetic Coordinate Conversion,
 *    Ralph Toms, February 1996  UCRL-JC-123138.
 *    
 *    Further information on GEOCENTRIC can be found in the Reuse Manual.
 *
 *    GEOCENTRIC originated from : U.S. Army Topographic Engineering Center
 *                                 Geospatial Information Division
 *                                 7701 Telegraph Road
 *                                 Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    GEOCENTRIC has no restrictions.
 *
 * ENVIRONMENT
 *
 *    GEOCENTRIC was tested and certified in the following environments:
 *
 *    1. Solaris 2.5 with GCC version 2.8.1
 *    2. Windows 95 with MS Visual C++ version 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    25-02-97          Original Code
 *
 */

/***************************************************************************/
/*
 *                               DEFINES
 */
#define PI_OVER_2  (PI / 2.0e0)
#define FALSE      0
#define TRUE       1
#define COS_67P5   0.38268343236508977  /* cosine of 67.5 degrees */
#define AD_C       1.0026000            /* Toms region 1 constant */


/***************************************************************************/
/*
 *                              GLOBAL DECLARATIONS
 */
/* Ellipsoid parameters, default to WGS 84 */
double Geocent_a = 6378137.0;     /* Semi-major axis of ellipsoid in meters */
double Geocent_b = 6356752.3142;  /* Semi-minor axis of ellipsoid           */

double Geocent_a2 = 40680631590769.0;        /* Square of semi-major axis */
double Geocent_b2 = 40408299984087.05;       /* Square of semi-minor axis */
double Geocent_e2 = 0.0066943799901413800;   /* Eccentricity squared  */
double Geocent_ep2 = 0.00673949675658690300; /* 2nd eccentricity squared */
/*
 * These state variables are for optimization purposes.  The only function
 * that should modify them is Set_Geocentric_Parameters.
 */


/***************************************************************************/
/*
 *                              FUNCTIONS     
 */


int Set_Geocentric_Parameters (double a, 
                                double b) 
{ /* BEGIN Set_Geocentric_Parameters */
/*
 * The function Set_Geocentric_Parameters receives the ellipsoid parameters
 * as inputs and sets the corresponding state variables.
 *
 *    a  : Semi-major axis, in meters.          (input)
 *    b  : Semi-minor axis, in meters.          (input)
 */
  int Error_Code = GEOCENT_NO_ERROR;

  if (a <= 0.0)
    Error_Code |= GEOCENT_A_ERROR;
  if (b <= 0.0)
    Error_Code |= GEOCENT_B_ERROR;
  if (a < b)
    Error_Code |= GEOCENT_A_LESS_B_ERROR;
  if (!Error_Code)
  {
    Geocent_a = a;
    Geocent_b = b;
    Geocent_a2 = a * a;
    Geocent_b2 = b * b;
    Geocent_e2 = (Geocent_a2 - Geocent_b2) / Geocent_a2;
    Geocent_ep2 = (Geocent_a2 - Geocent_b2) / Geocent_b2;
  }
  return (Error_Code);
} /* END OF Set_Geocentric_Parameters */


void Get_Geocentric_Parameters (double *a, 
                                double *b)
{ /* BEGIN Get_Geocentric_Parameters */
/*
 * The function Get_Geocentric_Parameters returns the ellipsoid parameters
 * to be used in geocentric coordinate conversions.
 *
 *    a  : Semi-major axis, in meters.          (output)
 *    b  : Semi-minor axis, in meters.          (output)
 */

  *a = Geocent_a;
  *b = Geocent_b;
} /* END OF Get_Geocentric_Parameters */


int Convert_Geodetic_To_Geocentric (double Latitude,
                                     double Longitude,
                                     double Height,
                                     double *X,
                                     double *Y,
                                     double *Z) 
{ /* BEGIN Convert_Geodetic_To_Geocentric */
/*
 * The function Convert_Geodetic_To_Geocentric converts geodetic coordinates
 * (latitude, longitude, and height) to geocentric coordinates (X, Y, Z),
 * according to the current ellipsoid parameters.
 *
 *    Latitude  : Geodetic latitude in radians                     (input)
 *    Longitude : Geodetic longitude in radians                    (input)
 *    Height    : Geodetic height, in meters                       (input)
 *    X         : Calculated Geocentric X coordinate, in meters    (output)
 *    Y         : Calculated Geocentric Y coordinate, in meters    (output)
 *    Z         : Calculated Geocentric Z coordinate, in meters    (output)
 *
 */
  int Error_Code = GEOCENT_NO_ERROR;
  double Rn;            /*  Earth radius at location  */
  double Sin_Lat;       /*  sin(Latitude)  */
  double Sin2_Lat;      /*  Square of sin(Latitude)  */
  double Cos_Lat;       /*  cos(Latitude)  */

  if ((Latitude < -PI_OVER_2) || (Latitude > PI_OVER_2))
  { /* Latitude out of range */
    Error_Code |= GEOCENT_LAT_ERROR;
  }
  if ((Longitude < -PI) || (Longitude > (2*PI)))
  { /* Longitude out of range */
    Error_Code |= GEOCENT_LON_ERROR;
  }
  if (!Error_Code)
  { /* no errors */
    if (Longitude > PI)
      Longitude -= (2*PI);
    Sin_Lat = sin(Latitude);
    Cos_Lat = cos(Latitude);
    Sin2_Lat = Sin_Lat * Sin_Lat;
    Rn = Geocent_a / (sqrt(1.0e0 - Geocent_e2 * Sin2_Lat));
    *X = (Rn + Height) * Cos_Lat * cos(Longitude);
    *Y = (Rn + Height) * Cos_Lat * sin(Longitude);
    *Z = ((Rn * (1 - Geocent_e2)) + Height) * Sin_Lat;

  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Geocentric */


void Convert_Geocentric_To_Geodetic (double X,
                                     double Y, 
                                     double Z,
                                     double *Latitude,
                                     double *Longitude,
                                     double *Height)
{ /* BEGIN Convert_Geocentric_To_Geodetic */
/*
 * The function Convert_Geocentric_To_Geodetic converts geocentric
 * coordinates (X, Y, Z) to geodetic coordinates (latitude, longitude, 
 * and height), according to the current ellipsoid parameters.
 *
 *    X         : Geocentric X coordinate, in meters.         (input)
 *    Y         : Geocentric Y coordinate, in meters.         (input)
 *    Z         : Geocentric Z coordinate, in meters.         (input)
 *    Latitude  : Calculated latitude value in radians.       (output)
 *    Longitude : Calculated longitude value in radians.      (output)
 *    Height    : Calculated height value, in meters.         (output)
 *
 * The method used here is derived from 'An Improved Algorithm for
 * Geocentric to Geodetic Coordinate Conversion', by Ralph Toms, Feb 1996
 */

/* Note: Variable names follow the notation used in Toms, Feb 1996 */

  double W;        /* distance from Z axis */
  double W2;       /* square of distance from Z axis */
  double T0;       /* initial estimate of vertical component */
  double T1;       /* corrected estimate of vertical component */
  double S0;       /* initial estimate of horizontal component */
  double S1;       /* corrected estimate of horizontal component */
  double Sin_B0;   /* sin(B0), B0 is estimate of Bowring aux variable */
  double Sin3_B0;  /* cube of sin(B0) */
  double Cos_B0;   /* cos(B0) */
  double Sin_p1;   /* sin(phi1), phi1 is estimated latitude */
  double Cos_p1;   /* cos(phi1) */
  double Rn;       /* Earth radius at location */
  double Sum;      /* numerator of cos(phi1) */
  int At_Pole;     /* indicates location is in polar region */

  At_Pole = FALSE;
  if (X != 0.0)
  {
    *Longitude = atan2(Y,X);
  }
  else
  {
    if (Y > 0)
    {
      *Longitude = PI_OVER_2;
    }
    else if (Y < 0)
    {
      *Longitude = -PI_OVER_2;
    }
    else
    {
      At_Pole = TRUE;
      *Longitude = 0.0;
      if (Z > 0.0)
      {  /* north pole */
        *Latitude = PI_OVER_2;
      }
      else if (Z < 0.0)
      {  /* south pole */
        *Latitude = -PI_OVER_2;
      }
      else
      {  /* center of earth */
        *Latitude = PI_OVER_2;
        *Height = -Geocent_b;
        return;
      } 
    }
  }
  W2 = X*X + Y*Y;
  W = sqrt(W2);
  T0 = Z * AD_C;
  S0 = sqrt(T0 * T0 + W2);
  Sin_B0 = T0 / S0;
  Cos_B0 = W / S0;
  Sin3_B0 = Sin_B0 * Sin_B0 * Sin_B0;
  T1 = Z + Geocent_b * Geocent_ep2 * Sin3_B0;
  Sum = W - Geocent_a * Geocent_e2 * Cos_B0 * Cos_B0 * Cos_B0;
  S1 = sqrt(T1*T1 + Sum * Sum);
  Sin_p1 = T1 / S1;
  Cos_p1 = Sum / S1;
  Rn = Geocent_a / sqrt(1.0 - Geocent_e2 * Sin_p1 * Sin_p1);
  if (Cos_p1 >= COS_67P5)
  {
    *Height = W / Cos_p1 - Rn;
  }
  else if (Cos_p1 <= -COS_67P5)
  {
    *Height = W / -Cos_p1 - Rn;
  }
  else
  {
    *Height = Z / Sin_p1 + Rn * (Geocent_e2 - 1.0);
  }
  if (At_Pole == FALSE)
  {
    *Latitude = atan(Sin_p1 / Cos_p1);
  }
} /* END OF Convert_Geocentric_To_Geodetic */

 
