/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 

#ifndef DATUM_H
#define DATUM_H

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
 *                                      or more than Datum_Count)
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
 *                              DEFINES
 */

#define DATUM_NO_ERROR                          0x0000
#define DATUM_NOT_INITIALIZED_ERROR             0x0001
#define DATUM_7PARAM_FILE_OPEN_ERROR            0x0002
#define DATUM_7PARAM_FILE_PARSING_ERROR         0x0004
#define DATUM_7PARAM_OVERFLOW_ERROR             0x0008
#define DATUM_3PARAM_FILE_OPEN_ERROR            0x0010
#define DATUM_3PARAM_FILE_PARSING_ERROR         0x0020
#define DATUM_3PARAM_OVERFLOW_ERROR             0x0040
#define DATUM_INVALID_INDEX_ERROR               0x0080
#define DATUM_INVALID_SRC_INDEX_ERROR           0x0100
#define DATUM_INVALID_DEST_INDEX_ERROR          0x0200
#define DATUM_INVALID_CODE_ERROR                0x0400
#define DATUM_LAT_ERROR                         0x0800
#define DATUM_LON_ERROR                         0x1000
#define DATUM_SIGMA_ERROR                       0x2000
#define DATUM_DOMAIN_ERROR                      0x4000
#define DATUM_ELLIPSE_ERROR                     0x8000


/***************************************************************************/
/*
 *                          GLOBAL DECLARATIONS
 */
typedef enum Datum_Types
{
  Three_Param_Datum,
  Seven_Param_Datum,
  WGS84_Datum,
  WGS72_Datum
} Datum_Type; /* different types of datums */



/***************************************************************************/
/*
 *                            FUNCTION PROTOTYPES
 */

  int Initialize_Datums(void);
/*
 * The function Initialize_Datums creates the datum table from two external
 * files.  If an error occurs, the initialization stops and an error code is
 * returned.  This function must be called before any of the other functions
 * in this component.
 */

/*
  int Create_Datum ( const char *Code,
                      const char *Name,
                      const char *Ellipsoid_Code,
                      double Delta_X,
                      double Delta_Y,
                      double Delta_Z,
                      double Sigma_X,
                      double Sigma_Y,
                      double Sigma_Z,
                      double South_latitude,
                      double North_latitude,
                      double West_longitude,
                      double East_longitude);
/*
 *   Code           : 5-letter new datum code.                      (input)
 *   Name           : Name of the new datum                         (input)
 *   Ellipsoid_Code : 2-letter code for the associated ellipsoid    (input)
 *   Delta_X        : X translation to WGS84 in meters              (input)
 *   Delta_Y        : Y translation to WGS84 in meters              (input)
 *   Delta_Z        : Z translation to WGS84 in meters              (input)
 *   Sigma_X        : Standard error in X in meters                 (input)
 *   Sigma_Y        : Standard error in Y in meters                 (input)
 *   Sigma_Z        : Standard error in Z in meters                 (input)
 *   South_latitude : Southern edge of validity rectangle in radians(input)
 *   North_latitude : Northern edge of validity rectangle in radians(input)
 *   West_longitude : Western edge of validity rectangle in radians (input)
 *   East_longitude : Eastern edge of validity rectangle in radians (input)
 *
 * The function Create_Datum creates a new local (3-parameter) datum with the 
 * specified code, name, shift values, and standard error values.  If the 
 * datum table has not been initialized, the specified code is already in use, 
 * or a new version of the 3-param.dat file cannot be created, an error code 
 * is returned, otherwise DATUM_NO_ERROR is returned.  Note that the indexes 
 * of all datums in the datum table may be changed by this function.
 */


  int Datum_Count ( int *Count );
/*
 *  The function Datum_Count returns the number of Datums in the table
 *  if the table was initialized without error.
 *
 *  Count   : number of datums in the datum table                   (output)
 */


  int Datum_Index ( const char *Code, 
                     int *Index );
/*
 *  The function Datum_Index returns the index of the datum with the 
 *  specified code.
 *
 *  Code    : The datum code being searched for                     (input)
 *  Index   : The index of the datum in the table with the          (output)
 *              specified code
 */


  int Datum_Code ( const int Index,
                    char *Code );
/*
 *  The function Datum_Code returns the 5-letter code of the datum
 *  referenced by index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Code    : The datum code of the datum referenced by index       (output)
 */


  int Datum_Name ( const int Index,
                    char *Name );
/*
 *  The function Datum_Name returns the name of the datum referenced by
 *  index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Name    : The datum name of the datum referenced by index       (output)
 */


  int Datum_Ellipsoid_Code ( const int Index,
                              char *Code );
/*
 *  The function Datum_Ellipsoid_Code returns the 2-letter ellipsoid code 
 *  for the ellipsoid associated with the datum referenced by index.
 *
 *  Index   : The index of a given datum in the datum table           (input)
 *  Code    : The ellisoid code for the ellipsoid associated with the (output)
 *               datum referenced by index 
 */


  int Get_Datum_Type ( const int Index,
                        Datum_Type *Type );
/*
 *  The function Get_Datum_Type returns the type of the datum referenced by
 *  index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Type    : The type of the datum referenced by index             (output)
 */


  int Datum_Seven_Parameters ( const int Index, 
                                double *Delta_X,
                                double *Delta_Y,
                                double *Delta_Z,
                                double *Rx,
                                double *Ry,
                                double *Rz,
                                double *Scale_Factor );
/*
 *   The function Datum_Seven_Parameters returns the seven parameters
 *   for the datum referenced by index.
 *
 *    Index      : The index of a given datum in the datum table   (input)
 *    Delta_X    : X translation in meters                         (output)
 *    Delta_Y    : Y translation in meters                         (output)
 *    Delta_Z    : Z translation in meters                         (output)
 *    Rx         : X rotation in radians                           (output)
 *    Rx         : Y rotation in radians                           (output)
 *    Ry         : Z rotation in radians                           (output)
 *    Scale_Factor : Scale factor                                  (output)
 */


  int Datum_Three_Parameters ( const int Index, 
                                double *Delta_X,
                                double *Delta_Y,
                                double *Delta_Z);
/*
 *   The function Datum_Three_Parameters returns the three parameters 
 *   for the datum referenced by index.
 *
 *    Index      : The index of a given datum in the datum table   (input)
 *    Delta_X    : X translation in meters                         (output)
 *    Delta_Y    : Y translation in meters                         (output)
 *    Delta_Z    : Z translation in meters                         (output)
 */


  int Datum_Errors ( const int Index,
                      double *Sigma_X,
                      double *Sigma_Y,
                      double *Sigma_Z);
/*
 *   The function Datum_Errors returns the standard errors in X,Y, & Z 
 *   for the datum referenced by index.
 *
 *    Index      : The index of a given datum in the datum table   (input)
 *    Sigma_X    : Standard error in X in meters                   (output)
 *    Sigma_Y    : Standard error in Y in meters                   (output)
 *    Sigma_Z    : Standard error in Z in meters                   (output)
 */


  int Datum_Valid_Rectangle ( const int Index,
                               double *South_latitude,
                               double *North_latitude,
                               double *West_longitude,
                               double *East_longitude);
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


  int Valid_Datum ( const int Index,
                     double latitude,
                     double longitude,
                     int *result);
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


  int Geocentric_Shift_To_WGS84 (const int Index,
                                  const double X,
                                  const double Y,
                                  const double Z,
                                  double *X_WGS84,
                                  double *Y_WGS84,
                                  double *Z_WGS84);
/*
 *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
 *  to the datum referenced by index to a geocentric coordinate (X, Y, Z in
 *  meters) relative to WGS84.
 *
 *  Index   : Index of local datum                       (input)
 *  X       : X coordinate relative to the source datum  (input)
 *  Y       : Y coordinate relative to the source datum  (input)
 *  Z       : Z coordinate relative to the source datum  (input)
 *  X_WGS84 : X coordinate relative to WGS84             (output)
 *  Y_WGS84 : Y coordinate relative to WGS84             (output)
 *  Z_WGS84 : Z coordinate relative to WGS84             (output)
 */


  int Geocentric_Shift_From_WGS84 (const double X_WGS84,
                                    const double Y_WGS84,
                                    const double Z_WGS84,
                                    const int Index,
                                    double *X,
                                    double *Y,
                                    double *Z);
/*
 *  This function shifts a geocentric coordinate (X, Y, Z in meters) relative
 *  to WGS84 to a geocentric coordinate (X, Y, Z in meters) relative to the
 *  local datum referenced by index.
 *
 *  X_WGS84 : X coordinate relative to WGS84                 (input)
 *  Y_WGS84 : Y coordinate relative to WGS84                 (input)
 *  Z_WGS84 : Z coordinate relative to WGS84                 (input)
 *  Index   : Index of destination datum                     (input)
 *  X       : X coordinate relative to the destination datum (output)
 *  Y       : Y coordinate relative to the destination datum (output)
 *  Z       : Z coordinate relative to the destination datum (output)
 */


  int Geocentric_Datum_Shift ( const int Index_in,
                                const double X_in,
                                const double Y_in,
                                const double Z_in,
                                const int Index_out,
                                double *X_out,
                                double *Y_out,
                                double *Z_out);
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


  int Geodetic_Shift_To_WGS84 (const int Index,
                                const double Lat_in,
                                const double Lon_in,
                                const double Hgt_in,
                                double *WGS84_Lat,
                                double *WGS84_Lon,
                                double *WGS84_Hgt);
/*
 *  This function shifts geodetic coordinates relative to a given source datum
 *  to geodetic coordinates relative to WGS84.
 *
 *  Index     : Index of source datum                                 (input)
 *  Lat_in    : Latitude in radians relative to source datum          (input)
 *  Lon_in    : Longitude in radians relative to source datum         (input)
 *  Hgt_in    : Height in meters relative to source datum's ellipsoid (input)
 *  WGS84_Lat : Latitude in radians relative to WGS84                 (output)
 *  WGS84_Lon : Longitude in radians relative to WGS84                (output)
 *  WGS84_Hgt : Height in meters relative to WGS84 ellipsoid          (output)
 */


  int Geodetic_Shift_From_WGS84( const double WGS84_Lat,
                                  const double WGS84_Lon,
                                  const double WGS84_Hgt,
                                  const int Index,
                                  double *Lat_out,
                                  double *Lon_out,
                                  double *Hgt_out);
/*
 *  This function shifts geodetic coordinates relative to a WGS84 
 *  to geodetic coordinates relative to a given local datum.
 *                                                                   
 *  WGS84_Lat : Latitude in radians relative to WGS84                      (input)
 *  WGS84_Lon : Longitude in radians relative to WGS84                     (input)
 *  WGS84_Hgt : Height in meters relative to WGS84 ellipsoid               (input)
 *  Index     : Index of destination datum                                 (input)
 *  Lat_in    : Latitude in radians relative to destination datum          (output)
 *  Lon_in    : Longitude in radians relative to destination datum         (output)
 *  Hgt_in    : Height in meters relative to destination datum's ellipsoid (output)
 */


  int Geodetic_Datum_Shift ( const int Index_in,
                              const double Lat_in,
                              const double Lon_in,
                              const double Hgt_in,
                              const int Index_out,
                              double *Lat_out,
                              double *Lon_out,
                              double *Hgt_out);
/*
 *  This function shifts geodetic coordinates (latitude, longitude in radians
 *  and height in meters) relative to the source datum to geodetic coordinates
 *  (latitude, longitude in radians and height in meters) relative to the
 *  destination datum.
 *
 *  Index_in  : Index of source datum                                      (input)
 *  Lat_in    : Latitude in radians relative to source datum               (input)
 *  Lon_in    : Longitude in radians relative to source datum              (input)
 *  Hgt_in    : Height in meters relative to source datum's ellipsoid      (input)
 *  Index_out : Index of destination datum                                 (input)
 *  Lat_out   : Latitude in radians relative to destination datum          (output)
 *  Lon_out   : Longitude in radians relative to destination datum         (output)
 *  Hgt_out   : Height in meters relative to destination datum's ellipsoid (output)
 */


  int Datum_Shift_Error (const int Index_in,
                          const int Index_out,
                          double latitude,
                          double longitude,
                          double *ce90,
                          double *le90,
                          double *se90);
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
 *  ELLIPSE_INITIALIZE_ERROR     : Ellipsoid database can not initialize
 *  ELLIPSE_TABLE_OVERFLOW_ERROR : Ellipsoid table overflow
 *  ELLIPSE_NOT_INITIALIZED_ERROR: Ellipsoid database not initialized properly
 *  ELLIPSE_INVALID_INDEX_ERROR  : Index is an invalid value
 *  ELLIPSE_INVALID_CODE_ERROR   : Code was not found in database
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
 *                            ELLIPSE GLOBALS
 */

#define ELLIPSE_NO_ERROR              0x0000
#define ELLIPSE_FILE_OPEN_ERROR       0x0001
#define ELLIPSE_INITIALIZE_ERROR      0x0002
#define ELLIPSE_TABLE_OVERFLOW_ERROR  0x0004
#define ELLIPSE_NOT_INITIALIZED_ERROR 0x0008
#define ELLIPSE_INVALID_INDEX_ERROR   0x0010
#define ELLIPSE_INVALID_CODE_ERROR    0x0020
#define ELLIPSE_A_ERROR               0x0040
#define ELLIPSE_B_ERROR               0x0080
#define ELLIPSE_A_LESS_B_ERROR        0x0100

/***************************************************************************/
/*
 *                          FUNCTION PROTOTYPES
 *                             for ellipse.c
 */

  int Initialize_Ellipsoids ();
/*
 * The function Initialize_Ellipsoids reads ellipsoid data from ellips.dat in
 * the current directory and builds the ellipsoid table from it.  If an error
 * occurs, the error code is returned, otherwise ELLIPSE_NO_ERROR is returned.
 */


  int Create_Ellipsoid (const char* Code,
                         const char* Name,
                         double a,
                         double b);
/*
 *   Code     : 2-letter ellipsoid code.                      (input)
 *   Name     : Name of the new ellipsoid                     (input)
 *   a        : Semi-major axis, in meters, of new ellipsoid  (input)
 *   b        : Semi-minor axis, in meters, of new ellipsoid. (input)
 *
 * The function Create_Ellipsoid creates a new ellipsoid with the specified
 * code, name, and axes.  If the ellipsoid table has not been initialized,
 * the specified code is already in use, or a new version of the ellips.dat 
 * file cannot be created, an error code is returned, otherwise ELLIPSE_NO_ERROR 
 * is returned.  Note that the indexes of all ellipsoids in the ellipsoid
 * table may be changed by this function.
 */


  int Ellipsoid_Count ( int *Count );
/*
 *   Count    : The number of ellipsoids in the ellipsoid table. (output)
 *
 * The function Ellipsoid_Count returns the number of ellipsoids in the
 * ellipsoid table.  If the ellipsoid table has been initialized without error,
 * ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR
 * is returned.
 */


  int Ellipsoid_Index ( const char *Code,
                         int *Index );
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


  int Ellipsoid_Name ( const int Index, 
                        char *Name );
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


  int Ellipsoid_Code ( const int Index,
                        char *Code );
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    Code     : 2-letter ellipsoid code.                          (output)
 *
 *  The Function Ellipsoid_Code returns the 2-letter code for the 
 *  ellipsoid in the ellipsoid table with the specified index.  If index is 
 *  valid, ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR 
 *  is returned.
 */


  int Ellipsoid_Axes ( const int Index,
                        double *a,
                        double *b );
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    a        : Semi-major axis, in meters, of ellipsoid          (output)
 *    b        : Semi-minor axis, in meters, of ellipsoid.         (output)
 *
 *  The function Ellipsoid_Axes returns the semi-major and semi-minor axes
 *  for the ellipsoid with the specified index.  If index is valid,
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR is 
 *  returned.
 */


  int Ellipsoid_Eccentricity2 ( const int Index,
                                 double *e2 );
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    e2       : Square of eccentricity of ellipsoid               (output)
 *
 *  The function Ellipsoid_Eccentricity2 returns the square of the 
 *  eccentricity for the ellipsoid with the specified index.  If index is 
 *  valid, ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR 
 *  is returned.
 */


  int Ellipsoid_Flattening ( const int Index,
                              double *f );
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    f        : Flattening of ellipsoid.                          (output)
 *
 *  The function Ellipsoid_Flattening returns the flattening of the 
 *  ellipsoid with the specified index.  If index is valid ELLIPSE_NO_ERROR is 
 *  returned, otherwise ELLIPSE_INVALID_INDEX_ERROR is returned.
 */


  int WGS84_Axes ( double *a,
                    double *b );
/*
 *    a      : Semi-major axis, in meters, of ellipsoid       (output)
 *    b      : Semi-minor axis, in meters, of ellipsoid       (output)
 *
 *  The function WGS84_Axes returns the lengths of the semi-major and 
 *  semi-minor axes for the WGS84 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */


  int WGS84_Eccentricity2 ( double *e2 );
/*
 *    e2    : Square of eccentricity of WGS84 ellipsoid      (output)
 *
 *  The function WGS84_Eccentricity2 returns the square of the 
 *  eccentricity for the WGS84 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */


  int WGS84_Flattening ( double *f );
/*
 *  f       : Flattening of WGS84 ellipsoid.                 (output)
 *
 *  The function WGS84_Flattening returns the flattening of the WGS84
 *  ellipsoid.  If the ellipsoid table was initialized successfully, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR 
 *  is returned.
 */

  int WGS72_Axes( double *a,
                   double *b);
/*
 *    a    : Semi-major axis, in meters, of ellipsoid        (output)
 *    b    : Semi-minor axis, in meters, of ellipsoid        (output)
 *
 *  The function WGS72_Axes returns the lengths of the semi-major and 
 *  semi-minor axes for the WGS72 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */

  int WGS72_Eccentricity2 ( double *e2 );
/*
 *    e2     : Square of eccentricity of WGS84 ellipsoid     (output)
 *
 *  The function WGS72_Eccentricity2 returns the square of the 
 *  eccentricity for the WGS72 ellipsoid.  If the ellipsoid table was 
 *  initialized successfully, ELLIPSE_NO_ERROR is returned, otherwise 
 *  ELLIPSE_NOT_INITIALIZED_ERROR is returned.
 */

  int WGS72_Flattening (double *f );
/*
 *    f      : Flattening of WGS72 ellipsoid.                (output)
 *
 *  The function WGS72_Flattening returns the flattening of the WGS72
 *  ellipsoid .  If the ellipsoid table was initialized successfully, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR 
 *  is returned.
 */

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
 *      GEOCENT_A_ERROR         : Semi-major axis less than or equal to zero
 *      GEOCENT_B_ERROR         : Semi-minor axis less than or equal to zero
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
 *
 *
 */


/***************************************************************************/
/*
 *                              DEFINES
 */
#define GEOCENT_NO_ERROR        0x0000
#define GEOCENT_LAT_ERROR       0x0001
#define GEOCENT_LON_ERROR       0x0002
#define GEOCENT_A_ERROR         0x0004
#define GEOCENT_B_ERROR         0x0008
#define GEOCENT_A_LESS_B_ERROR  0x0010


/***************************************************************************/
/*
 *                              FUNCTION PROTOTYPES
 */

  int Set_Geocentric_Parameters (double a, 
                                  double b);
/*
 * The function Set_Geocentric_Parameters receives the ellipsoid parameters
 * as inputs and sets the corresponding state variables.
 *
 *    a  : Semi-major axis, in meters.          (input)
 *    b  : Semi-minor axis, in meters.          (input)
 */


  void Get_Geocentric_Parameters (double *a, 
                                  double *b);
/*
 * The function Get_Geocentric_Parameters returns the ellipsoid parameters
 * to be used in geocentric coordinate conversions.
 *
 *    a  : Semi-major axis, in meters.          (output)
 *    b  : Semi-minor axis, in meters.          (output)
 */


  int Convert_Geodetic_To_Geocentric (double Latitude,
                                       double Longitude,
                                       double Height,
                                       double *X,
                                       double *Y,
                                       double *Z);
/*
 * The function Convert_Geodetic_To_Geocentric converts geodetic coordinates
 * (latitude, longitude, and height) to geocentric coordinates (X, Y, Z),
 * according to the current ellipsoid parameters.
 *
 *    Latitude  : Geodetic latitude in radians                     (input)
 *    Longitude : Geodetic longitude in radians                    (input)
 *    Height    : Geodetic height, in meters                       (input)
 *    X         : Calculated Geocentric X coordinate, in meters.   (output)
 *    Y         : Calculated Geocentric Y coordinate, in meters.   (output)
 *    Z         : Calculated Geocentric Z coordinate, in meters.   (output)
 *
 */


  void Convert_Geocentric_To_Geodetic (double X,
                                       double Y, 
                                       double Z,
                                       double *Latitude,
                                       double *Longitude,
                                       double *Height);
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
 */



#endif /* DATUM_H */

 
