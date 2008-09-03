/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 

#ifndef MGRSENGINE_H
#define MGRSENGINE_H
/***************************************************************************/
/* RSC IDENTIFIER: GEOTRANS ENGINE                 
 *
 * ABSTRACT
 *
 *    This component is the coordinate transformation engine for the GEOTRANS
 *    application.  It provides an external input interface that supports the
 *    GEOTRANS GUIs (Motif and Windows) and the GEOTRANS file processing
 *    component.
 *
 *    This component depends on the DT&CC modules:  DATUM, ELLIPSOID,
 *    UTM, MGRS
 *
 * ERROR HANDLING
 *
 *    This component checks for error codes returned by the DT&CC modules.
 *    If an error code is returned, it is combined with the current
 *    error code using the bitwise or.  This combining allows multiple error
 *    codes to be returned. The possible error codes are listed below.
 *
 *
 *    GEOTRANS ENGINE originated from :  U.S. Army Topographic Engineering Center
 *                             Geospatial Information Division
 *                             7701 Telegraph Road
 *                             Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    GEOTRANS ENGINE has no restrictions.
 *
 * ENVIRONMENT
 *
 *    GEOTRANS ENGINE was tested and certified in the following environments:
 *
 *    1. Solaris 2.5 with GCC, version 2.8.1
 *    2. Windows 95 with MS Visual C++, version 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    04-22-97          Original Code
 *    09-30-99          Added support for 15 new projections
 */

class MgrsEngine
{
public:

   static MgrsEngine *Instance();



private:

   static MgrsEngine *mpSingleton;
   /**
    * Constructor for MgrsEngine
    * 
    * Constructor for Military Grid Reference System (MGRS) Engine
    *
    * @param   n/a
    *          
   **/
   MgrsEngine();

   //destructor
   ~MgrsEngine(){};


/***************************************************************************/
/*
 *                              DEFINES
 */
/* Engine return status codes */
   #define ENGINE_INPUT_WARNING        0x00000001    /* Warning returned by 1st conversion */
   #define ENGINE_INPUT_ERROR          0x00000002    /* Error returned by 1st conversion */
   #define ENGINE_OUTPUT_WARNING       0x00000004    /* Warning returned by 2nd conversion */
   #define ENGINE_OUTPUT_ERROR         0x00000008    /* Error returned by 2nd conversion */
/* Initialization errors */
   #define ENGINE_NOT_INITIALIZED      0x00000010    /* Initialize_Engine has not been called */ 
   #define ENGINE_ELLIPSOID_ERROR      0x00000020    /* Error returned by Ellipsoid module */
   #define ENGINE_DATUM_ERROR          0x00000040    /* Error returned by Datum module */
   #define ENGINE_GEOID_ERROR          0x00000080    /* Error returned by Geoid module*/
/* Interface parameter errors (should never occur) */
   #define ENGINE_INVALID_TYPE         0x00000100    /* Invalid coordinate system type */
   #define ENGINE_INVALID_DIRECTION    0x00000200    /* Invalid direction (input or output) */
   #define ENGINE_INVALID_STATE        0x00000400    /* Invalid state (interactive or file) */
/* Lookup table errors */
   #define ENGINE_INVALID_INDEX_ERROR  0x00001000    /* Index is an invalid value */
   #define ENGINE_INVALID_CODE_ERROR   0x00002000    /* Code was not found in table */
   #define ENGINE_ELLIPSOID_OVERFLOW   0x00004000    /* Ellipsoid table overflow */
   #define ENGINE_DATUM_OVERFLOW       0x00008000    /* 3-parameter datum table overflow */
/* Datum definition errors */
   #define ENGINE_DATUM_SIGMA_ERROR    0x00010000    /* Standard error values must be positive
                                                        (or -1 if unknown) */
   #define ENGINE_DATUM_DOMAIN_ERROR   0x00020000    /* Invalid local datum domain of validity */
/* Input/Output conversion status codes */
   #define ENGINE_ORIGIN_LAT_ERROR     0x00000001    /* Invalid Origin Latitude */
   #define ENGINE_CENT_MER_ERROR       0x00000002    /* Invalid Central Meridian */
   #define ENGINE_EASTING_ERROR        0x00000004    /* Invalid Easting */
   #define ENGINE_NORTHING_ERROR       0x00000008    /* Invalid Northing */
   #define ENGINE_RADIUS_ERROR         0x00000010    /* Point too far from center of projection */
   #define ENGINE_HEMISPHERE_ERROR     0x00000020    /* Invalid Hemisphere */                                           
   #define ENGINE_SCALE_FACTOR_ERROR   0x00000040    /* Invalid Scale Factor */
   #define ENGINE_LON_WARNING          0x00000080    /* Longitude too far from Central Meridian */
   #define ENGINE_ORIGIN_LON_ERROR     0x00000100    /* Invalid Origin Longitude */
   #define ENGINE_DATUM_WARNING        0x00000200    /* Point outside datum validity rectangle */
/* Lambert/Albers error conditions */
   #define ENGINE_FIRST_STDP_ERROR     0x00000400    /* Invalid 1st Standard Parallel */
   #define ENGINE_SECOND_STDP_ERROR    0x00000800    /* Invalid 2nd Standard Parallel */
   #define ENGINE_FIRST_SECOND_ERROR   0x00001000    /* 1st & 2nd Standard Parallel cannot be same */
/* UTM/UPS/MGRS specific error conditions */
   #define ENGINE_ZONE_ERROR           0x00002000    /* Invalid UTM Zone */
   #define ENGINE_ZONE_OVERRIDE_ERROR  0x00004000    /* Invalid UTM zone Override */
   #define ENGINE_MGRS_STR_ERROR       0x00008000    /* Invalid MGRS String */
/* GEOREF specific error conditions */
   #define ENGINE_GEOREF_STR_ERROR     0x00010000    /* Invalid GEOREF String */
   #define ENGINE_STR_LON_MIN_ERROR    0x00020000    /* GEOREF string int. min. error */
   #define ENGINE_STR_LAT_MIN_ERROR    0x00040000    /* GEOREF string lat. min. error */
   #define ENGINE_STDP_ERROR           0x00080000    /* Standard parallel error */
     
/* Common status codes */
   #define ENGINE_NO_ERROR             0x00000000    /* no error */ 
   #define ENGINE_LAT_ERROR            0x01000000    /* Invalid Latitude */
   #define ENGINE_LON_ERROR            0x02000000    /* Invalid Longitude */
   #define ENGINE_A_ERROR              0x10000000    /* Invalid Ellipsoid Semi-Major Axis */
   #define ENGINE_B_ERROR              0x20000000    /* Invalid Ellipsoid Semi-Minor Axis */
   #define ENGINE_A_LESS_B_ERROR       0x40000000    /* Semi-Major Axis less than Semi_Minor Axis */

/* Symbolic constants */   
   #define NUMBER_COORD_SYS             3  //25  /* Number of coordinate systems        */
   #define COORD_SYS_CODE_LENGTH        3  /* Length of coordinate system codes (including null) */
   #define COORD_SYS_NAME_LENGTH       50  /* Max length of coordinate system names (including null) */
   #define DATUM_CODE_LENGTH            7  /* Length of datum codes (including null) */
   #define DATUM_NAME_LENGTH           32  /* Max length of datum names (including null) */
   #define ELLIPSOID_CODE_LENGTH        3  /* Length of ellipsoid codes (including null) */
   #define ELLIPSOID_NAME_LENGTH       30  /* Max length of ellipsoid names (including null) */
   #define CONVERT_MSG_LENGTH        2048  /* Max length of coordinate conversion status message */
   #define RETURN_MSG_LENGTH          256  /* Max length of return code status message */
   

public:

/***************************************************************************/
/*
 *                          GLOBAL DECLARATIONS
 */

/* State Enumerations */
typedef enum Input_Output
{
  Input = 0,
  Output = 1
} Input_or_Output;

typedef enum File_Interactive
{
  File = 0,
  Interactive = 1
} File_or_Interactive;

/* Coordinate Type Enumeration */
typedef enum Coordinate_Types
{
  Geodetic,
  MGRS,
  UTM
} Coordinate_Type;

/* Precision Enumeration */
typedef enum Precisions
{
  Degree,                /* 100000m */
  Ten_Minutes,           /* 10000m */
  Minute,                /* 1000m */
  Ten_Seconds,           /* 100m */
  Second,                /* 10m */
  Tenth_of_Second,       /* 1m */
  Hundredth_of_Second,   /* 0.1m */
  Thousandth_of_Second,  /* 0.01 */
  Ten_Thousandth_of_Second  /* 0.001m */
} Precision;

/* Heights */
typedef enum Height_Types
{
  No_Height,
  Ellipsoid_Height,
  Geoid_or_MSL_Height
} Height_Type;


/* Geodetic Coordinate System Definition */
typedef struct Geodetic_Structure
{
  Height_Type  height_type;
} Geodetic_Parameters;

/* Geodetic Coordinate Tuple Definition */
typedef struct Geodetic_Tuple_Structure
{
  double  longitude;   /* radians */
  double  latitude;    /* radians */
  double  height;      /* meters */
} Geodetic_Tuple;

/* MGRS Coordinate Tuple Definition */
typedef struct MGRS_Tuple_Structure
{
  char    string[21];
} MGRS_Tuple;

/* UTM Coordinate System Definition */
typedef struct UTM_Structure
{
  int     zone;     /* 1..60 */
  int     override; /* 0 or 1 */
} UTM_Parameters;

/* UTM Coordinate Tuple Definition */
typedef struct UTM_Tuple_Structure
{
  double  easting;     /* meters */
  double  northing;    /* meters */
  int    zone;        /* 1..60 */
  char    hemisphere;  /* "N" or "S" */
} UTM_Tuple;

   /* Coordinate System Definition with Multiple Variants */
   typedef union Parameters
   {
     Geodetic_Parameters               Geodetic;
     UTM_Parameters                    UTM;
   } Parameter_Tuple;

   /* Coordinate Tuple Definition with Multiple Variants */
   typedef union Coordinate_Tuples
   {
     Geodetic_Tuple                    Geodetic;
     MGRS_Tuple                        MGRS;
     UTM_Tuple                         UTM;
   } Coordinate_Tuple;

   typedef struct Coordinate_System_Table_Row
   {
     char Name[COORD_SYS_NAME_LENGTH];
     char Code[COORD_SYS_CODE_LENGTH];
   } Coordinate_System_Row;

   /* Engine State Definition */
   typedef struct coordinate_State_Row
   {
     int                 datum_Index;  /* currently specified datum */
     int                 status;       /* current error status */
     Parameter_Tuple      parameters;   /* current coordinate system parameters */
     Coordinate_Tuple     coordinates;  /* current coordinates */
     Coordinate_Type      type;         /* current coordinate system type */
   } Coordinate_State_Row;

public:
/***************************************************************************/
/*
 *                              DECLARATIONS
 */

   /* CS_State[x][y] is set up as follows:
      x = Number of interaction states (File, Interactive, etc.)
      y = Number of IO states (Input, Output, etc.) */
   static Coordinate_State_Row CS_State[2][2];

   /* Local State Variables */
   static Coordinate_System_Row Coordinate_System_Table[NUMBER_COORD_SYS];
   static Parameter_Tuple Default_Parameters[NUMBER_COORD_SYS];    /* Default Parameters */
   static Coordinate_Tuple Default_Coordinates[NUMBER_COORD_SYS];  /* Default Coordinates */
   static int Engine_Initialized; // = FALSE;
   static int Number_of_Datums; // = 0;
   static Precision Engine_Precision; // = Tenth_of_Second; /* Default Precision Level */
   static double ce90; // = -1.0;  /* 90% circular (horizontal) error */
   static double le90; // = -1.0;  /* 90% linear (vertical) error */
   static double se90; // = -1.0;  /* 90% spherical error */

/***************************************************************************/
/*
 *                              FUNCTION PROTOTYPES
 *                                for ENGINE.C
 */

   int Initialize_Engine();
  /*
   *  The function Initialize_Engine sets the initial state of the engine in
   *  in preparation for coordinate conversion and/or datum transformation
   *  operations.
   */


  int Get_Coordinate_System_Count ( int *Count );
  /*
   *   Count    : The number of coordinate systems supported. (output) 
   *
   * The function Get_Coordinate_System_Count returns the number of coordinate
   * systems (including projections and grids) that are supported.
   */


  int Get_Coordinate_System_Index ( const char *Code,
                                     int *Index );
  /*
   *    Code     : 2-letter coordinate system code.              (input)
   *    Index    : Index of the coordinate system with the specified code
   *                                                             (output)
   *
   *  The function Get_Coordinate_System_Index returns the index of the coordinate
   *  system with the specified code.  If the specified Code is found, 
   *  ENGINE_NO_ERROR is returned, otherwise ENGINE_INVALID_CODE_ERROR is 
   *  returned.
   */


  int Get_Coordinate_System_Type ( const int Index,
                                    Coordinate_Type *System ); 
/*
 *    Index   : Index of a specific coordinate system            (input)
 *    System  : Type of the coordinate system referencd by index (output)
 *
 *  The Function Get_Coordinate_System_Type returns the type of the coordinate 
 *  system with the specified index.  If theh index is valid, ENGINE_NO_ERROR is 
 *  returned, otherwise ENGINE_INVALID_INDEX_ERROR is returned.
 */


  int Get_Coordinate_System_Name ( const int Index, 
                                    char *Name );
  /*
   *    Index   : Index of a specific coordinate system            (input)
   *    Name    : Name of the coordinate system referencd by index (output)
   *
   *  The Function Get_Coordinate_System_Name returns the name of the coordinate 
   *  system with the specified index.  If theh index is valid, ENGINE_NO_ERROR is 
   *  returned, otherwise ENGINE_INVALID_INDEX_ERROR is returned.
   */


  int Get_Coordinate_System_Code ( const int Index,
                                    char *Code );
  /*
   *    Index   : Index of a specific coordinate system            (input)
   *    Code    : 2-letter coordinate system code.                 (output)
   *
   *  The Function Get_Coordinate_System_Code returns the 2-letter code for the 
   *  coordinate system with the specified index.  If index is valid, ENGINE_NO_ERROR  
   *  is returned, otherwise ENGINE_INVALID_INDEX_ERROR is returned.
   */


  int Set_Coordinate_System
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const Coordinate_Type     System );
  /*
   *  The function Set_Coordinate_System sets the coordinate system for the 
   *  specified state to the specified coordinate system type.
   *  State      : Indicates whether the coordinate system is to be used for 
   *               interactive or file processing                        (input)
   *  Direction  : Indicates whether the coordinate system is to be used for 
   *               input or output                                       (input)
   *  System     : Identifies the coordinate system to be used           (input)
   */


  int Get_Coordinate_System
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        Coordinate_Type           *System );
  /*
   *  The function Get_Coordinate_System returns the current coordinate system  
   *  type for the specified state.
   *  State      : Indicates whether the coordinate system is to be used for 
   *               interactive or file processing                        (input)
   *  Direction  : Indicates whether the coordinate system is to be used for 
   *               input or output                                       (input)
   *  System     : Identifies current coordinate system type             (output)
   */

  int Get_Datum_Count ( int *Count );
/*
 *  The function Get_Datum_Count returns the number of Datums in the table
 *  if the table was initialized without error.
 *
 *  Count   : number of datums in the datum table                   (output)
 */


  int Get_Datum_Index ( const char *Code, 
                         int *Index );
/*
 *  The function Get_Datum_Index returns the index of the datum with the 
 *  specified code.
 *
 *  Code    : The datum code being searched for                     (input)
 *  Index   : The index of the datum in the table with the          (output)
 *              specified code
 */


  int Get_Datum_Name ( const int Index,
                        char *Name );
/*
 *  The function Get_Datum_Name returns the name of the datum referenced by
 *  index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Name    : The datum name of the datum referenced by index       (output)
 */


  int Get_Datum_Code ( const int Index,
                        char *Code );
/*
 *  The function Get_Datum_Code returns the 5-letter code of the datum
 *  referenced by index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Code    : The datum code of the datum referenced by index       (output)
 */


  int Get_Datum_Ellipsoid_Code ( const int Index,
                                  char *Code );
/*
 *  The function Get_Datum_Ellipsoid_Code returns the 2-letter ellipsoid code 
 *  for the ellipsoid associated with the datum referenced by index.
 *
 *  Index   : The index of a given datum in the datum table           (input)
 *  Code    : The ellisoid code for the ellipsoid associated with the (output)
 *               datum referenced by index 
 */


  int Set_Datum
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const int                Index );
  /*
   *  The function Set_Datum sets the datum for the specified state to the 
   *  datum corresponding to the specified index.
   *  State      : Indicates whether the datum is to be used for interactive 
   *               or file processing                                    (input)
   *  Direction  : Indicates whether the datum is to be used for input or 
   *               output                                                (input)
   *  Index      : Identifies the index of the datum to be used          (input)
   */


  int Get_Datum
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        int                      *Index );
  /*
   *  The function Get_Datum returns the index of the current datum for the 
   *  specified state. 
   *  State      : Indicates whether the datum is to be used for interactive 
   *               or file processing                                    (input)
   *  Direction  : Indicates whether the datum is to be used for input or 
   *               output                                                (input)
   *  Index      : Identifies the index of the current datum             (input)
   */

  int Define_Datum ( const char *Code,
                      const char* Name,
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
 * The function Define_Datum creates a new local (3-parameter) datum with the 
 * specified code, name, shift values, and standard error values.  If the 
 * datum table has not been initialized, the specified code is already in use, 
 * or a new version of the 3-param.dat file cannot be created, an error code 
 * is returned, otherwise DATUM_NO_ERROR is returned.  Note that the indexes 
 * of all datums in the datum table may be changed by this function.
 */


  int Get_Ellipsoid_Count ( int *Count );
/*
 *   Count    : The number of ellipsoids in the ellipsoid table. (output)
 *
 * The function Get_Ellipsoid_Count returns the number of ellipsoids in the
 * ellipsoid table.  If the ellipsoid table has been initialized without error,
 * ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR
 * is returned.
 */


  int Get_Ellipsoid_Index ( const char *Code,
                             int *Index );
/*
 *    Code     : 2-letter ellipsoid code.                      (input)
 *    Index    : Index of the ellipsoid in the ellipsoid table with the 
 *                  specified code                             (output)
 *
 *  The function Get_Ellipsoid_Index returns the index of the ellipsoid in 
 *  the ellipsoid table with the specified code.  If ellipsoid_Code is found, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_CODE_ERROR is 
 *  returned.
 */


  int Get_Ellipsoid_Name ( const int Index, 
                            char *Name );
/*
 *    Index   : Index of a given ellipsoid.in the ellipsoid table with the
 *                 specified index                             (input)
 *    Name    : Name of the ellipsoid referencd by index       (output)
 *
 *  The Function Get_Ellipsoid_Name returns the name of the ellipsoid in 
 *  the ellipsoid table with the specified index.  If index is valid, 
 *  ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR is
 *  returned.
 */


  int Get_Ellipsoid_Code ( const int Index,
                            char *Code );
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    Code     : 2-letter ellipsoid code.                          (output)
 *
 *  The Function Get_Ellipsoid_Code returns the 2-letter code for the 
 *  ellipsoid in the ellipsoid table with the specified index.  If index is 
 *  valid, ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR 
 *  is returned.
 */


  int Define_Ellipsoid (const char* Code,
                         const char* Name,
                         double a,
                         double b);
/*
 *   Code     : 2-letter ellipsoid code.                      (input)
 *   Name     : Name of the new ellipsoid                     (input)
 *   a        : Semi-major axis, in meters, of new ellipsoid  (input)
 *   b        : Semi-minor axis, in meters, of new ellipsoid. (input)
 *
 * The function Define_Ellipsoid creates a new ellipsoid with the specified
 * code, name, and axes.  If the ellipsoid table has not been initialized,
 * the specified code is already in use, or a new version of the ellips.dat 
 * file cannot be created, an error code is returned, otherwise ELLIPSE_NO_ERROR 
 * is returned.  Note that the indexes of all ellipsoids in the ellipsoid
 * table may be changed by this function.
 */


  void Set_Precision(Precision Precis);
  /*
   *  The function Set_Precision sets the output precision to the specified level. 
   *  Precis     : Indicates the desired level of precision              (input)
   */


  void Get_Precision(Precision *Precis);
  /*
   *  The function Get_Precision returns the current level of precision. 
   *  Precis     : Indicates the current level of precision              (output)
   */

/*
  int Set_Geocentric_Coordinates
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const Geocentric_Tuple    coordinates);
*/
  /*
   *  The function Set_Geocentric_Coordinates sets the Geocentric coordinates
   *  for the specified state.. 
   *  State            : Indicates whether the coordinates are to be set for  
   *                     interactive or file processing                  (input)
   *  Direction        : Indicates whether the coordinates are to be set for  
   *                     input or output                                 (input)
   *  Geocentric_Tuple : Geocentric coordinate values to be set          (input)
   */



  int Set_Geodetic_Params
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const Geodetic_Parameters parameters);
  /*
   *  The function Set_Geodetic_Params sets the Geodetic coordinate system  
   *  parameters for the specified state. 
   *  State        : Indicates whether the parameters are to be set for  
   *                 interactive or file processing                      (input)
   *  Direction    : Indicates whether the parameters are to be set for  
   *                 input or output                                     (input)
   *  Geodetic_Parameters : Geodetic parmaeters to be set                (input)
   */


  int Get_Geodetic_Params
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        Geodetic_Parameters       *parameters);
  /*
   *  The function Get_Geodetic_Params returns the Geodetic coordinate system
   *  parameters for the specified state. 
   *  State        : Indicates whether the parameters are to be returned for
   *                 interactive or file processing                      (input)
   *  Direction    : Indicates whether the parameters are to be returned for  
   *                 input or output                                     (input)
   *  Geodeticr_Parameters : Geodetic parameters to be returned          (output)
   */


  int Set_Geodetic_Coordinates
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const Geodetic_Tuple      coordinates);
  /*
   *  The function Set_Geodetic_Coordinates sets the Geodetic coordinates
   *  for the specified state.. 
   *  State          : Indicates whether the parameters are to be set for  
   *                   interactive or file processing                    (input)
   *  Direction      : Indicates whether the parameters are to be set for  
   *                   input or output                                   (input)
   *  Geodetic_Tuple : Geodetic coordinate values to be set              (input)
   */


  int Get_Geodetic_Coordinates
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        Geodetic_Tuple            *coordinates);
  /*
   *  The function Get_Geodetic_Coordinates returns the Geodetic coordinates
   *  for the specified state.. 
   *  State          : Indicates whether the coordinates are to be returned for  
   *                   interactive or file processing                    (input)
   *  Direction      : Indicates whether the coordinates are to be returned for  
   *                   input or output                                   (input)
   *  Geodetic_Tuple : Geodetic coordinate values to be returned         (output)
   */

  int Get_MGRS_Coordinates
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        MGRS_Tuple                *coordinates);
  /*
   *  The function Get_MGRS_Coordinates returns the MGRS projection coordinates 
   *  for the specified state. 
   *  State          : Indicates whether the coordinates are to be returned for  
   *                   interactive or file processing                    (input)
   *  Direction      : Indicates whether the coordinates are to be returned for  
   *                   input or output                                   (input)
   *  MGRS_Tuple     : MGRS projection coordinates to be returned        (output)
   */

  int Set_UTM_Params
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const UTM_Parameters      parameters);
  /*
   *  The function Set_UTM_Params sets the UTM projection parameters   
   *  for the specified state. 
   *  State        : Indicates whether the parameters are to be set for  
   *                 interactive or file processing                      (input)
   *  Direction    : Indicates whether the parameters are to be set for  
   *                 input or output                                     (input)
   *  UTM_Parameters : UTM projection parameters to be set               (input)
   */


  int Get_UTM_Params
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        UTM_Parameters            *parameters);
  /*
   *  The function Get_UTM_Params returns the UTM projection parameters 
   *  for the specified state. 
   *  State        : Indicates whether the parameters are to be returned for
   *                 interactive or file processing                      (input)
   *  Direction    : Indicates whether the parameters are to be returned for  
   *                 input or output                                     (input)
   *  UTM_Parameters : UTM projection parameters to be returned          (output)
   */


  int Set_UTM_Coordinates
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        const UTM_Tuple           coordinates); 
  /*
  *  The function Set_UTM_Coordinates sets the UTM projection coordinates  
  *  for the specified state. 
  *  State        : Indicates whether the coordinates are to be set for  
  *                 interactive or file processing                      (input)
  *  Direction    : Indicates whether the coordinates are to be set for  
  *                 input or output                                     (input)
  *  UTM_Tuple    : UTM projection coordinates to be set                (input)
  */


  int Get_UTM_Coordinates
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        UTM_Tuple                 *coordinates);
  /*
  *  The function Get_UTM_Coordinates returns the UTM projection coordinates
  *  for the specified state. 
  *  State          : Indicates whether the coordinates are to be returned for  
  *                   interactive or file processing                    (input)
  *  Direction      : Indicates whether the coordinates are to be returned for  
  *                   input or output                                   (input)
  *  UTM_Tuple      : UTM projection coordinates to be returned         (output)
  */



  int Convert();
//      ( const File_or_Interactive State);
/*
 *  The function Convert converts the current input coordinates in the coordinate
 *  system defined by the current input coordinate system parameters and input datum,
 *  into output coordinates in the coordinate system defined by the output coordinate
 *  system parameters and output datum.
 *  State      : Indicates whether the interactive or file processing state data
 *               is to be used                                             (input)
 */


  int Get_Conversion_Errors
      ( const File_or_Interactive State,
        double *CE90,
        double *LE90,
        double *SE90);
  /*
   *  The function Get_Conversion_Errors returns the estimated errors in the location
   *  the most recently converted location
   *  State  : Indicates whether the conversion error information is for interactive 
   *           or file processing                                    (input)
   *  CE90   : 90% circular (horizontal) error, in meters            (output)
   *  LE90   : 90% linear (vertical) error, in meters                (output)
   *  SE90   : 90% spherical error, in meters                        (output)
   */


  int Get_Conversion_Status
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        int                      *Conversion_Status);
  /*
   *  The function Get_Conversion_Status returns the current status for the specified state.. 
   *  State      : Indicates whether the status returned is for interactive 
   *               or file processing                                    (input)
   *  Direction  : Indicates whether the status returned is for input or output
   *                                                                     (input)
   *  Conversion_Status : The current status for the specified state     (output)
   */


  int Get_Conversion_Status_String
      ( const File_or_Interactive State,
        const Input_or_Output     Direction,
        char                      *Separator,
        char                      *String);
  /*
   *  The function Get_Conversion_Status_String returns a string describing the 
   *  current status for the specified state. 
   *  State      : Indicates whether the status string returned is for interactive 
   *               or file processing                                    (input)
   *  Direction  : Indicates whether the status string returned is for input or 
   *               output                                                (input)
   *  Separator  : String to be used to separate individual status messages
   *                                                                     (input)
   *  String     : String describing the current status for the specified state     
   *                                                                     (output)
   */


  void Get_Return_Code_String
      ( int  Error_Code,
        char  *Separator,
        char  *String);
/*
 *  The function Get_Return_Code_String returns a string describing the specified 
 *  engine return code.
 *  Error_Code : Status code returned by engine function               (input) 
 *  Separator  : String to be used to separate individual status messages
 *                                                                     (input)
 *  String     : String describing the current status for the specified state     
 *                                                                     (output)
 */
   int Set_MGRS_Coordinates
      ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::MGRS_Tuple          coordinates);

   void Set_Defaults();


};

#endif /* ENGINE_H */

 
