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
 *    GEOCENTRIC, GEOREF, UTM, MGRS
 *
 * ERROR HANDLING
 *
 *    This component checks for error codes returned by the DT&CC modules.
 *    If an error code is returned, it is combined with the current
 *    error code using the bitwise or.  This combining allows multiple error
 *    codes to be returned. The possible error codes are listed below.
 *
 *
 * REFERENCES
 *
 *    Further information on GEOTRANS ENGINE can be found in the GEOTRANS ENGINE 
 *    Reuse Manual.
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


/***************************************************************************/
/*
 *                               INCLUDES
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Mgrs.h"
#include "MgrsDatum.h"
#include "MgrsEngine.h"
/*
 *  ctype.h     - standard C character handling library
 *  math.h      - standard C math library
 *  stdio.h     - standard C input/output library
 *  stdlib.h    - standard C general utility library 
 *  string.h    - standard C string handling library
 *
 *  datum.h     - access to datum parameters and datum transformation functions
 *  ellipse.h   - access to ellipsoid parameters
 *  geoid.h     - geoid-ellipsoid separation
 *  georef.h    - conversion between Geodetic and GEOREF coordinates
 *  geocent.h   - conversion between Geocentric and Geodetic coordinates
 *  mgrs.h      - conversion between Geodetic and MGRS coordinates
 *  utm.h       - conversion between Geodetic and UTM coordinates
 *  tranmerc.h  - conversion between Geodetic and Transverse Mercator coordinates
 *
 *  engine.h    - type definitions and function prototype error checking
 */


/****************************************************************************/
/*
 *                                DEFINES
 */

#define FALSE 0
#define TRUE  1

/***************************************************************************/
/*
 *                              DECLARATIONS
 */

   /* CS_State[x][y] is set up as follows:
      x = Number of interaction states (File, Interactive, etc.)
      y = Number of IO states (Input, Output, etc.) */
//   MgrsEngine::Coordinate_State_Row MgrsEngine::CS_State[2][2];

   /* Local State Variables */
MgrsEngine::Coordinate_System_Row MgrsEngine::Coordinate_System_Table[NUMBER_COORD_SYS];
MgrsEngine::Parameter_Tuple Default_Parameters[NUMBER_COORD_SYS];    /* Default Parameters */
MgrsEngine::Coordinate_Tuple Default_Coordinates[NUMBER_COORD_SYS];  /* Default Coordinates */
int MgrsEngine::Engine_Initialized = FALSE;
int MgrsEngine::Number_of_Datums = 0;
MgrsEngine::Precision MgrsEngine::Engine_Precision = Tenth_of_Second; /* Default Precision Level */
double MgrsEngine::ce90 = -1.0;  /* 90% circular (horizontal) error */
double MgrsEngine::le90 = -1.0;  /* 90% linear (vertical) error */
double MgrsEngine::se90 = -1.0;  /* 90% spherical error */
MgrsEngine::Parameter_Tuple MgrsEngine::Default_Parameters[NUMBER_COORD_SYS];    /* Default Parameters */
MgrsEngine::Coordinate_Tuple MgrsEngine::Default_Coordinates[NUMBER_COORD_SYS];  /* Default Coordinates */
MgrsEngine::Coordinate_State_Row MgrsEngine::CS_State[2][2];




MgrsEngine *MgrsEngine::mpSingleton = NULL;
MgrsEngine *MgrsEngine::Instance()
{
   if(mpSingleton == NULL)
   {
      mpSingleton = new MgrsEngine();
   }
   return mpSingleton;
}


MgrsEngine::MgrsEngine()
{
}
/***************************************************************************/
/*
 *                       LOCAL FUNCTIONS
 */


int Valid_Datum_Index(const int Index)
/*
 *  The function Valid_Datum_Index returns TRUE if the specified index is a
 *  valid datum index and FALSE otherwise.
 *  Index      : Index of a particular datum                           (input)
 */
{ /* Valid_Datum_Index */
   return ( (Index > 0) && (Index <= MgrsEngine::Number_of_Datums) );
} /* Valid_Datum_Index */


int Valid_State(MgrsEngine::File_or_Interactive State)
/*
 *  The function Valid_State returns TRUE if the specified state is a
 *  valid state and FALSE otherwise.
 *  State      : Sepcified state                                       (input)
 */
{ /* Valid_State */
  return ( (State == MgrsEngine::File) || (State == MgrsEngine::Interactive) );
} /* Valid_State */


int Valid_Direction(const MgrsEngine::Input_or_Output Direction)
/*
 *  The function Valid_Direction returns TRUE if the specified direction is a
 *  valid direction and FALSE otherwise.
 *  Direction : Sepcified direction                                    (input)
 */
{ /* Valid_Direction */
  return ( (Direction == MgrsEngine::Input) || (Direction == MgrsEngine::Output) );
} /* Valid_Direction */


void MgrsEngine::Set_Defaults()
/*
 *  The function Set_Defaults sets the coordinate system parameters and coordinate
 *  values for all coordinate systems to default values.
 */
{ /* Set_Defaults */
//  double a;
//  double b;
//  double origin_latitude;
//  double std_parallel;
//  double std_parallel_1;
//  double std_parallel_2;
//  double latitude_of_true_scale;
//  double origin_longitude;
//  double central_meridian;
//  double longitude_down_from_pole;
//  double false_easting;
//  double false_northing;
//  double scale_factor;
//  double orientation;
//  double origin_height;

  Default_Parameters[MgrsEngine::Geodetic].Geodetic.height_type = MgrsEngine::Ellipsoid_Height;

  Default_Coordinates[MgrsEngine::Geodetic].Geodetic.longitude = 0.0;
  Default_Coordinates[MgrsEngine::Geodetic].Geodetic.latitude = 0.0;
  Default_Coordinates[MgrsEngine::Geodetic].Geodetic.height = 0.0;

  Default_Parameters[MgrsEngine::UTM].UTM.override = 0;
  Default_Parameters[MgrsEngine::UTM].UTM.zone = 30;
  Default_Coordinates[MgrsEngine::UTM].UTM.zone = 31;
  Default_Coordinates[MgrsEngine::UTM].UTM.hemisphere = 'N';
  Default_Coordinates[MgrsEngine::UTM].UTM.easting = 500000.0;
  Default_Coordinates[MgrsEngine::UTM].UTM.northing = 0.0;

} /* Set_Defaults */


void Initialize_Coordinate_System
    ( const MgrsEngine::Coordinate_Type    System,
      MgrsEngine::Parameter_Tuple   *Parameters,
      MgrsEngine::Coordinate_Tuple  *Coordinates )
/*
 *  The function Initialize_Coordinate_System set the coordinate system parameters and
 *  coordinate values for the specified coordinate system to the default values.
 *  System      : Coordinate system type                                (input)
 *  Parameters  : Parameters of specified coordinate system type        (output)
 *  Coordinates : Coordinates of specified coordinate system type       (output)
 */
{ /* Initialize_Coordinate_System */
  *Parameters = Default_Parameters[System];
  *Coordinates = Default_Coordinates[System];
} /* Initialize_Coordinate_System */


/***************************************************************************/
/*
 *                              FUNCTIONS
 */


int MgrsEngine::Initialize_Engine()
/*
 *  The function Initialize_Engine sets the initial state of the engine in
 *  in preparation for coordinate conversion and/or datum transformation
 *  operations.
 */
{ /* Initialize_Engine */
  int error_code = ENGINE_NO_ERROR;
  int i, j;
  if (Engine_Initialized)
    return (error_code);
  if (Initialize_Ellipsoids())
    error_code |= ENGINE_ELLIPSOID_ERROR;
  if (Initialize_Datums())
    error_code |= ENGINE_DATUM_ERROR;
//  if (Initialize_Geoid())
//    error_code |= ENGINE_GEOID_ERROR;
  /* Initialize Coordinate System Table */
  strcpy(Coordinate_System_Table[MgrsEngine::Geodetic].Name, "Geodetic");
  strcpy(Coordinate_System_Table[MgrsEngine::Geodetic].Code, "GD");
  strcpy(Coordinate_System_Table[MgrsEngine::MGRS].Name, "MGRS");
  strcpy(Coordinate_System_Table[MgrsEngine::MGRS].Code, "MG");
  strcpy(Coordinate_System_Table[MgrsEngine::UTM].Name, "UTM");
  strcpy(Coordinate_System_Table[MgrsEngine::UTM].Code, "UT");
  if (!error_code)
  {
    Engine_Initialized = TRUE;
    Set_Defaults();
    Datum_Count (&Number_of_Datums);
    Set_Coordinate_System(File,Input,Geodetic);
    Set_Coordinate_System(File,Output,Geodetic);
    Set_Coordinate_System(Interactive,Input,Geodetic);
    Set_Coordinate_System(Interactive,Output,Geodetic);
    for (i=0;i<2;i++)
      for (j=0;j<2;j++)
      {
        CS_State[i][j].datum_Index = 1;
        CS_State[i][j].status = ENGINE_NO_ERROR;
      }
    Engine_Precision = Tenth_of_Second;
  }
  return (error_code);
} /* Initialize_Engine */


int MgrsEngine::Get_Coordinate_System_Count ( int *Count )
/*
 *   Count    : The number of coordinate systems supported. (output) 
 *
 * The function Get_Coordinate_System_Count returns the number of coordinate
 * systems (including projections and grids) that are supported.
 */
{ /* Get_Coordinate_System_Count */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  *Count = NUMBER_COORD_SYS;
  return (error_code);
} /* Get_Coordinate_System_Count */


int MgrsEngine::Get_Coordinate_System_Index ( const char *Code,
                                   int *Index )
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
{ /* Get_Coordinate_System_Index */
  char temp_code[COORD_SYS_CODE_LENGTH];
  int error_code = ENGINE_NO_ERROR;
  int i = 0;        /* index for coordinate system table */
  int j = 0;
  *Index = 0;
  if (MgrsEngine::Engine_Initialized)
  {
    while (j < COORD_SYS_CODE_LENGTH)
    {
      temp_code[j] = toupper(Code[j]);
      j++;
    }
    temp_code[COORD_SYS_CODE_LENGTH - 1] = 0;
    while ((i < NUMBER_COORD_SYS)
           && strcmp(temp_code, MgrsEngine::Coordinate_System_Table[i].Code))
    {
      i++;
    }
    if (strcmp(temp_code, MgrsEngine::Coordinate_System_Table[i].Code))
      error_code |= ENGINE_INVALID_CODE_ERROR;
    else
      *Index = i+1;
  }
  else
  {
    error_code |= ENGINE_NOT_INITIALIZED;
  }
  return (error_code);
} /* Get_Coordinate_System_Index */


int MgrsEngine::Get_Coordinate_System_Type ( const int Index,
                                  MgrsEngine::Coordinate_Type *System ) 
/*
 *    Index   : Index of a specific coordinate system            (input)
 *    System  : Type of the coordinate system referencd by index (output)
 *
 *  The Function Get_Coordinate_System_Type returns the type of the coordinate 
 *  system with the specified index.  If theh index is valid, ENGINE_NO_ERROR is 
 *  returned, otherwise ENGINE_INVALID_INDEX_ERROR is returned.
 */
{ /* Get_Coordinate_System_Type */
  int error_code = ENGINE_NO_ERROR;
  if (MgrsEngine::Engine_Initialized)
  {
    if ((Index < 1) || (Index > NUMBER_COORD_SYS))
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else
      *System = ((MgrsEngine::Coordinate_Type)(Index-1));
  }
  else
    error_code |= ENGINE_NOT_INITIALIZED;
  return (error_code);
} /* Get_Coordinate_System_Type */


int MgrsEngine::Get_Coordinate_System_Name ( const int Index,
                                  char *Name ) 
/*
 *    Index   : Index of a specific coordinate system            (input)
 *    Name    : Name of the coordinate system referencd by index (output)
 *
 *  The Function Get_Coordinate_System_Name returns the name of the coordinate 
 *  system with the specified index.  If theh index is valid, ENGINE_NO_ERROR is 
 *  returned, otherwise ENGINE_INVALID_INDEX_ERROR is returned.
 */
{ /* Get_Coordinate_System_Name */
  int error_code = ENGINE_NO_ERROR;
  strcpy(Name,"");
  if (MgrsEngine::Engine_Initialized)
  {
    if ((Index < 1) || (Index > NUMBER_COORD_SYS))
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else
      strcpy(Name, MgrsEngine::Coordinate_System_Table[Index-1].Name);
  }
  else
    error_code |= ENGINE_NOT_INITIALIZED;
  return (error_code);
} /* Get_Coordinate_System_Name */


int MgrsEngine::Get_Coordinate_System_Code ( const int Index,
                                  char *Code ) 
/*
 *    Index   : Index of a specific coordinate system            (input)
 *    Code    : 2-letter coordinate system code.                 (output)
 *
 *  The Function Get_Coordinate_System_Code returns the 2-letter code for the 
 *  coordinate system with the specified index.  If index is valid, ENGINE_NO_ERROR  
 *  is returned, otherwise ENGINE_INVALID_INDEX_ERROR is returned.
 */
{ /* Begin Get_Coordinate_System_Code */
  int error_code = ENGINE_NO_ERROR;
  strcpy(Code,"");
  if (MgrsEngine::Engine_Initialized)
  {
    if ((Index < 1) || (Index > NUMBER_COORD_SYS))
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else
      strcpy(Code, MgrsEngine::Coordinate_System_Table[Index-1].Code);
  }
  else
    error_code |= ENGINE_NOT_INITIALIZED;
  return (error_code);
} /* Get_Coordinate_System_Code */


int MgrsEngine::Set_Coordinate_System
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::Coordinate_Type     System )
/*
 *  The function Set_Coordinate_System sets the coordinate system for the
 *  specified state to the specified coordinate system type.
 *  State      : Indicates whether the coordinate system is to be used for
 *               interactive or file processing                        (input)
 *  Direction  : Indicates whether the coordinate system is to be used for
 *               input or output                                       (input)
 *  System     : Identifies the coordinate system to be used           (input)
 */
{ /* Set_Coordinate_System */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    MgrsEngine::CS_State[State][Direction].type = System;
    MgrsEngine::CS_State[State][Direction].status = ENGINE_NO_ERROR;
    Initialize_Coordinate_System ( System,
                                   &(MgrsEngine::CS_State[State][Direction].parameters),
                                   &(MgrsEngine::CS_State[State][Direction].coordinates));
  }
  return (error_code);
} /* Set_Coordinate_System */


int MgrsEngine::Get_Coordinate_System
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      MgrsEngine::Coordinate_Type           *System )
/*
 *  The function Get_Coordinate_System returns the current coordinate system
 *  type for the specified state.
 *  State      : Indicates whether the coordinate system is to be used for
 *               interactive or file processing                        (input)
 *  Direction  : Indicates whether the coordinate system is to be used for
 *               input or output                                       (input)
 *  System     : Identifies current coordinate system type             (output)
 */
{ /* Get_Coordinate_System */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    *System = MgrsEngine::CS_State[State][Direction].type;
  }
  return (error_code);
} /* Get_Coordinate_System */


int MgrsEngine::Get_Datum_Count ( int *Count )
/*
 *  The function Get_Datum_Count returns the number of Datums in the table
 *  if the table was initialized without error.
 *
 *  Count   : number of datums in the datum table                   (output)
 */
{ /* Get_Datum_Count */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Datum_Count (Count);
    if (temp_error != DATUM_NO_ERROR)
      error_code |= ENGINE_DATUM_ERROR;
  }
  return (error_code);
} /* Get_Datum_Count */


int MgrsEngine::Get_Datum_Index ( const char *Code, 
                       int *Index )
/*
 *  The function Get_Datum_Index returns the index of the datum with the 
 *  specified code.
 *
 *  Code    : The datum code being searched for                     (input)
 *  Index   : The index of the datum in the table with the          (output)
 *              specified code
 */
{ /* Get_Datum_Index */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Datum_Index (Code, Index);
    if (temp_error == DATUM_INVALID_CODE_ERROR)
      error_code |= ENGINE_INVALID_CODE_ERROR;
    else if (temp_error != DATUM_NO_ERROR)
      error_code |= ENGINE_DATUM_ERROR;
  }
  return (error_code);
} /* Get_Datum_Index */


int MgrsEngine::Get_Datum_Name ( const int Index,
                      char *Name )
/*
 *  The function Get_Datum_Name returns the name of the datum referenced by
 *  index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Name    : The datum name of the datum referenced by index       (output)
 */
{ /* Get_Datum_Name */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Datum_Name (Index, Name);
    if (temp_error == DATUM_INVALID_INDEX_ERROR)
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else if (temp_error != DATUM_NO_ERROR)
      error_code |= ENGINE_DATUM_ERROR;
  }
  return (error_code);
} /* Get_Datum_Name */


int MgrsEngine::Get_Datum_Code ( const int Index,
                      char *Code )
/*
 *  The function Get_Datum_Code returns the 5-letter code of the datum
 *  referenced by index.
 *
 *  Index   : The index of a given datum in the datum table         (input)
 *  Code    : The datum code of the datum referenced by index       (output)
 */
{ /* Get_Datum_Code */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Datum_Code (Index, Code);
    if (temp_error == DATUM_INVALID_INDEX_ERROR)
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else if (temp_error != DATUM_NO_ERROR)
      error_code |= ENGINE_DATUM_ERROR;
  }
  return (error_code);
} /* Get_Datum_Code */


int MgrsEngine::Get_Datum_Ellipsoid_Code ( const int Index,
                                char *Code )
/*
 *  The function Get_Datum_Ellipsoid_Code returns the 2-letter ellipsoid code 
 *  for the ellipsoid associated with the datum referenced by index.
 *
 *  Index   : The index of a given datum in the datum table           (input)
 *  Code    : The ellisoid code for the ellipsoid associated with the (output)
 *               datum referenced by index 
 */
{ /* Get_Datum_Ellipssoid_Code */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Datum_Ellipsoid_Code (Index, Code);
    if (temp_error == DATUM_INVALID_INDEX_ERROR)
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else if (temp_error != DATUM_NO_ERROR)
      error_code |= ENGINE_DATUM_ERROR;
  }
  return (error_code);
} /* Get_Datum_Ellipsoid_Code */


int MgrsEngine::Set_Datum
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const int                Index )
/*
 *  The function Set_Datum sets the datum for the specified state to the
 *  datum corresponding to the specified index.
 *  State      : Indicates whether the datum is to be used for interactive
 *               or file processing                                    (input)
 *  Direction  : Indicates whether the datum is to be used for input or
 *               output                                                (input)
 *  Index      : Identifies the index of the datum to be used          (input)
 */
{ /* Set_Datum */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!Valid_Datum_Index(Index))
    error_code |= ENGINE_INVALID_INDEX_ERROR;
  if (!error_code)
  {
    MgrsEngine::CS_State[State][Direction].datum_Index = Index;
  }
  return (error_code);
} /* Set_Datum */


int MgrsEngine::Get_Datum
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      int                      *Index )
/*
 *  The function Get_Datum returns the index of the current datum for the
 *  specified state.
 *  State      : Indicates whether the datum is to be used for interactive
 *               or file processing                                    (input)
 *  Direction  : Indicates whether the datum is to be used for input or
 *               output                                                (input)
 *  Index      : Identifies the index of the current datum             (input)
 */
{ /* Get_Datum */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    *Index = MgrsEngine::CS_State[State][Direction].datum_Index;
  }
  return (error_code);
} /* Get_Datum */

/*
int MgrsEngine::Define_Datum ( const char *Code,
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
                    double East_longitude)
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
//{ /* Define_Datum */

//  int error_code = ENGINE_NO_ERROR;
  /*
  int temp_error;
  if (!Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Create_Datum (Code, Name, Ellipsoid_Code,
                               Delta_X, Delta_Y, Delta_Z, Sigma_X, Sigma_Y, Sigma_Z,
                               South_latitude, North_latitude, West_longitude, East_longitude);
    if (temp_error & DATUM_INVALID_CODE_ERROR)
      error_code |= ENGINE_INVALID_CODE_ERROR;
    if (temp_error & DATUM_SIGMA_ERROR)
      error_code |= ENGINE_DATUM_SIGMA_ERROR;
    if (temp_error & DATUM_DOMAIN_ERROR)
      error_code |= ENGINE_DATUM_DOMAIN_ERROR;
    if (temp_error & DATUM_LAT_ERROR)
      error_code |= ENGINE_LAT_ERROR;
    if (temp_error & DATUM_LON_ERROR)
      error_code |= ENGINE_LON_ERROR;
    if (temp_error & DATUM_3PARAM_OVERFLOW_ERROR)
      error_code |= ENGINE_DATUM_OVERFLOW;
    if (temp_error & DATUM_ELLIPSE_ERROR)
      error_code |= ENGINE_ELLIPSOID_ERROR;
    if (temp_error & DATUM_3PARAM_FILE_OPEN_ERROR)
      error_code |= ENGINE_DATUM_ERROR;
    if (temp_error == DATUM_NO_ERROR)
      Datum_Count (&Number_of_Datums);
  }
  */
//  return (error_code);
//} /* Define_Datum */



int MgrsEngine::Get_Ellipsoid_Count ( int *Count )
/*
 *   Count    : The number of ellipsoids in the ellipsoid table. (output)
 *
 * The function Get_Ellipsoid_Count returns the number of ellipsoids in the
 * ellipsoid table.  If the ellipsoid table has been initialized without error,
 * ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_NOT_INITIALIZED_ERROR
 * is returned.
 */
{ /* Get_Ellipsoid_Count */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Ellipsoid_Count (Count);
    if (temp_error != ELLIPSE_NO_ERROR)
      error_code |= ENGINE_ELLIPSOID_ERROR;
  }
  return (error_code);
} /* Get_Ellipsoid_Count */


int MgrsEngine::Get_Ellipsoid_Index ( const char *Code,
                           int *Index )
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
{ /* Get_Ellipsoid_Index */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Ellipsoid_Index (Code, Index);
    if (temp_error == ELLIPSE_INVALID_CODE_ERROR)
      error_code |= ENGINE_INVALID_CODE_ERROR;
    else if (temp_error != ELLIPSE_NO_ERROR)
      error_code |= ENGINE_ELLIPSOID_ERROR;
  }
  return (error_code);
} /* Get_Ellipsoid_Index */


int MgrsEngine::Get_Ellipsoid_Name ( const int Index, 
                          char *Name )
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
{ /* Get_Ellipsoid_Name */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Ellipsoid_Name (Index, Name);
    if (temp_error == ELLIPSE_INVALID_INDEX_ERROR)
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else if (temp_error != ELLIPSE_NO_ERROR)
      error_code |= ENGINE_ELLIPSOID_ERROR;
  }
  return (error_code);
} /* Get_Ellipsoid_Name */


int MgrsEngine::Get_Ellipsoid_Code ( const int Index,
                          char *Code )
/*
 *    Index    : Index of a given ellipsoid in the ellipsoid table (input)
 *    Code     : 2-letter ellipsoid code.                          (output)
 *
 *  The Function Get_Ellipsoid_Code returns the 2-letter code for the 
 *  ellipsoid in the ellipsoid table with the specified index.  If index is 
 *  valid, ELLIPSE_NO_ERROR is returned, otherwise ELLIPSE_INVALID_INDEX_ERROR 
 *  is returned.
 */
{ /* Get_Ellipsoid_Code */
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = Ellipsoid_Code (Index, Code);
    if (temp_error == ELLIPSE_INVALID_INDEX_ERROR)
      error_code |= ENGINE_INVALID_INDEX_ERROR;
    else if (temp_error != ELLIPSE_NO_ERROR)
      error_code |= ENGINE_ELLIPSOID_ERROR;
  }
  return (error_code);
} /* Get_Ellipsoid_Code */

/*
int MgrsEngine::Define_Ellipsoid (const char* Code,
                       const char* Name,
                       double a,
                       double b)
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
//{ // Define_Ellipsoid 
/*
  int error_code = ENGINE_NO_ERROR;
  int temp_error;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  else
  {
    temp_error = MgrsEngine::Create_Ellipsoid (Code, Name, a, b);
    if (temp_error & ELLIPSE_TABLE_OVERFLOW_ERROR)
      error_code |= ENGINE_ELLIPSOID_OVERFLOW;
    if (temp_error & ELLIPSE_INVALID_CODE_ERROR)
      error_code |= ENGINE_INVALID_CODE_ERROR;
    if (temp_error & ELLIPSE_A_ERROR)
      error_code |= ENGINE_A_ERROR;
    if (temp_error & ELLIPSE_B_ERROR)
      error_code |= ENGINE_B_ERROR;
    if (temp_error & ELLIPSE_A_LESS_B_ERROR)
      error_code |= ENGINE_A_LESS_B_ERROR;
    if (temp_error & ELLIPSE_FILE_OPEN_ERROR)
      error_code |= ENGINE_ELLIPSOID_ERROR;
  }
  return (error_code);
} /* Define_Ellipsoid */


void MgrsEngine::Set_Precision(MgrsEngine::Precision Precis)
/*
 *  The function Set_Precision sets the output precision to the specified level.
 *  Precis     : Indicates the desired level of precision              (input)
 */
{ /* Set_Precision */
  Engine_Precision = Precis;
} /* Set_Precision */


void MgrsEngine::Get_Precision(MgrsEngine::Precision *Precis)
/*
 *  The function Get_Precision returns the current level of precision.
 *  Precis     : Indicates the current level of precision              (output)
 */
{ /* Get_Precision */
  *Precis = Engine_Precision;
} /* Get_Precision */

int MgrsEngine::Set_Geodetic_Params
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::Geodetic_Parameters parameters)
/*
 *  The function Set_Geodetic_Params sets the Geodetic coordinate system  
 *  parameters for the specified state. 
 *  State        : Indicates whether the parameters are to be set for  
 *                 interactive or file processing                      (input)
 *  Direction    : Indicates whether the parameters are to be set for  
 *                 input or output                                     (input)
 *  Geodetic_Parameters : Geodetic parmaeters to be set                (input)
 */
{ /* Set_Geodetic_Parameters */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::Geodetic)
      error_code |= ENGINE_INVALID_TYPE;
    else
      MgrsEngine::CS_State[State][Direction].parameters.Geodetic = parameters;
  }
  return (error_code);
} /* END Set_Geodetic_Parameters */


int MgrsEngine::Get_Geodetic_Params
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      MgrsEngine::Geodetic_Parameters       *parameters)
/*
 *  The function Get_Geodetic_Params returns the Geodetic coordinate system
 *  parameters for the specified state. 
 *  State        : Indicates whether the parameters are to be returned for
 *                 interactive or file processing                      (input)
 *  Direction    : Indicates whether the parameters are to be returned for  
 *                 input or output                                     (input)
 *  Geodetic_Parameters : Geodetic parameters to be returned           (output)
 */
{ /* Get_Geodetic_Parameters */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::Geodetic)
      error_code |= ENGINE_INVALID_TYPE;
    else
      *parameters = MgrsEngine::CS_State[State][Direction].parameters.Geodetic;
  }
  return ( error_code );
} /* END Get_Geodetic_Parameters */


int MgrsEngine::Set_Geodetic_Coordinates
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::Geodetic_Tuple      coordinates)
/*
 *  The function Set_Geodetic_Coordinates sets the Geodetic coordinates
 *  for the specified state..
 *  State          : Indicates whether the parameters are to be set for
 *                   interactive or file processing                    (input)
 *  Direction      : Indicates whether the parameters are to be set for
 *                   input or output                                   (input)
 *  Geodetic_Tuple : Geodetic coordinate values to be set              (input)
 */
{ /* Set_Geodetic_Coordinates */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::Geodetic)
      error_code |= ENGINE_INVALID_TYPE;
    else
      MgrsEngine::CS_State[State][Direction].coordinates.Geodetic = coordinates;
  }
  return ( error_code );
} /* Set_Geodetic_Coordinates */


int MgrsEngine::Get_Geodetic_Coordinates
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      MgrsEngine::Geodetic_Tuple            *coordinates)
/*
 *  The function Get_Geodetic_Coordinates returns the Geodetic coordinates
 *  for the specified state..
 *  State          : Indicates whether the coordinates are to be returned for
 *                   interactive or file processing                    (input)
 *  Direction      : Indicates whether the coordinates are to be returned for
 *                   input or output                                   (input)
 *  Geodetic_Tuple : Geodetic coordinate values to be returned         (output)
 */
{ /* Get_Geodetic_Coordinates */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::Geodetic)
      error_code |= ENGINE_INVALID_TYPE;
    else
      *coordinates = MgrsEngine::CS_State[State][Direction].coordinates.Geodetic;
  }
  return ( error_code );
} /* Get_Geodetic_Coordinates */

int MgrsEngine::Set_MGRS_Coordinates
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::MGRS_Tuple          coordinates)
/*
 *  The function Set_MGRS_Coordinates sets the MGRS projection coordinates
 *  for the specified state.
 *  State        : Indicates whether the coordinates are to be set for
 *                 interactive or file processing                      (input)
 *  Direction    : Indicates whether the coordinates are to be set for
 *                 input or output                                     (input)
 *  MGRS_Tuple   : MGRS projection coordinates to be set
 *                                                                     (input)
 */
{ /* Set_MGRS_Coordinates */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::MGRS)
      error_code |= ENGINE_INVALID_TYPE;
    else
      MgrsEngine::CS_State[State][Direction].coordinates.MGRS = coordinates;
  }
  return ( error_code );
} /* Set_MGRS_Coordinates */


int MgrsEngine::Get_MGRS_Coordinates
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      MgrsEngine::MGRS_Tuple                *coordinates)
/*
 *  The function Get_MGRS_Coordinates returns the MGRS projection coordinates
 *  for the specified state.
 *  State          : Indicates whether the coordinates are to be returned for
 *                   interactive or file processing                    (input)
 *  Direction      : Indicates whether the coordinates are to be returned for
 *                   input or output                                   (input)
 *  MGRS_Tuple     : MGRS projection coordinates to be returned        (output)
 */
{ /* Get_MGRS_Coordinates */
  int error_code = ENGINE_NO_ERROR;

  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::MGRS)
      error_code |= ENGINE_INVALID_TYPE;
    else
      *coordinates = MgrsEngine::CS_State[State][Direction].coordinates.MGRS;
  }
  return ( error_code );
} /* Get_MGRS_Coordinates */

int MgrsEngine::Set_UTM_Params
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::UTM_Parameters      parameters)
/*
 *  The function Set_UTM_Params sets the UTM projection parameters
 *  for the specified state.
 *  State        : Indicates whether the parameters are to be set for
 *                 interactive or file processing                      (input)
 *  Direction    : Indicates whether the parameters are to be set for
 *                 input or output                                     (input)
 *  UTM_Parameters : UTM projection parameters to be set               (input)
 */
{ /* Set_UTM_Params */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::UTM)
      error_code |= ENGINE_INVALID_TYPE;
    else
      MgrsEngine::CS_State[State][Direction].parameters.UTM = parameters;
  }
  return ( error_code );
} /* Set_UTM_Params */

int MgrsEngine::Get_UTM_Params
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      MgrsEngine::UTM_Parameters            *parameters)
/*
 *  The function Get_UTM_Params returns the UTM projection parameters
 *  for the specified state.
 *  State        : Indicates whether the parameters are to be returned for
 *                 interactive or file processing                      (input)
 *  Direction    : Indicates whether the parameters are to be returned for
 *                 input or output                                     (input)
 *  UTM_Parameters : UTM projection parameters to be returned          (output)
 */
{ /* Get_UTM_Params */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::UTM)
      error_code |= ENGINE_INVALID_TYPE;
    else
      *parameters = MgrsEngine::CS_State[State][Direction].parameters.UTM;
  }
  return ( error_code );
} /* Get_UTM_Params */

int MgrsEngine::Set_UTM_Coordinates
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      const MgrsEngine::UTM_Tuple           coordinates)
/*
 *  The function Set_UTM_Coordinates sets the UTM projection coordinates
 *  for the specified state.
 *  State        : Indicates whether the coordinates are to be set for
 *                 interactive or file processing                      (input)
 *  Direction    : Indicates whether the coordinates are to be set for
 *                 input or output                                     (input)
 *  UTM_Tuple    : UTM projection coordinates to be set                (input)
 */
{ /* Set_UTM_Coordinates */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::UTM)
      error_code |= ENGINE_INVALID_TYPE;
    else
      MgrsEngine::CS_State[State][Direction].coordinates.UTM = coordinates;
  }
  return ( error_code );
} /* Set_UTM_Coordinates */

int MgrsEngine::Get_UTM_Coordinates
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      MgrsEngine::UTM_Tuple                 *coordinates)
/*
 *  The function Get_UTM_Coordinates returns the UTM projection coordinates
 *  for the specified state.
 *  State          : Indicates whether the coordinates are to be returned for
 *                   interactive or file processing                    (input)
 *  Direction      : Indicates whether the coordinates are to be returned for
 *                   input or output                                   (input)
 *  UTM_Tuple      : UTM projection coordinates to be returned         (output)
 */
{ /* Get_UTM_Coordinates */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (MgrsEngine::CS_State[State][Direction].type != MgrsEngine::UTM)
      error_code |= ENGINE_INVALID_TYPE;
    else
      *coordinates = MgrsEngine::CS_State[State][Direction].coordinates.UTM;
  }
  return ( error_code );
} /* Get_UTM_Coordinates */

int MgrsEngine::Convert()
//    ( const File_or_Interactive State)
/*
 *  The function Convert converts the current input coordinates in the coordinate
 *  system defined by the current input coordinate system parameters and input datum,
 *  into output coordinates in the coordinate system defined by the output coordinate
 *  system parameters and output datum.
 *  State      : Indicates whether the interactive or file processing state data
 *               is to be used                                             (input)
 */
{ /* Convert */
  MgrsEngine::Coordinate_State_Row *input;
  MgrsEngine::Coordinate_State_Row *output;
  MgrsEngine::Geodetic_Tuple Converted_Geodetic;
  MgrsEngine::Geodetic_Tuple WGS84_Geodetic;
  MgrsEngine::Geodetic_Tuple Shifted_Geodetic;
  double input_a;
  double input_b;
  double input_f;
  double output_a;
  double output_b;
  double output_f;
  char Input_Ellipsoid_Code[3];
  int Input_Ellipsoid_Index;
  char Output_Ellipsoid_Code[3];
  int Output_Ellipsoid_Index;
  int error_code = ENGINE_NO_ERROR;
  int temp_error = ENGINE_NO_ERROR;
  int input_valid = 0;
  int output_valid = 0;
  int special = 0;
  Mgrs *pMgrs = Mgrs::Instance();

  Converted_Geodetic.latitude = 0.0;
  Converted_Geodetic.longitude = 0.0;
  Converted_Geodetic.height = 0.0;
  Shifted_Geodetic.latitude = 0.0;
  Shifted_Geodetic.longitude = 0.0;
  Shifted_Geodetic.height = 0.0;
  if (!Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
//  if (!Valid_State(State))
//    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
//    input = &(MgrsEngine::CS_State[State][MgrsEngine::Input]);
//    output = &(MgrsEngine::CS_State[State][MgrsEngine::Output]);
    input = &(MgrsEngine::CS_State[MgrsEngine::Interactive][MgrsEngine::Input]);
    output = &(MgrsEngine::CS_State[MgrsEngine::Interactive][MgrsEngine::Output]);
    input->status = ENGINE_NO_ERROR;
    output->status = ENGINE_NO_ERROR;
    Datum_Ellipsoid_Code(input->datum_Index, Input_Ellipsoid_Code);
    Datum_Ellipsoid_Code(output->datum_Index, Output_Ellipsoid_Code);
    Ellipsoid_Index(Input_Ellipsoid_Code, &Input_Ellipsoid_Index);
    Ellipsoid_Index(Output_Ellipsoid_Code, &Output_Ellipsoid_Index);
    Ellipsoid_Axes(Input_Ellipsoid_Index,&input_a,&input_b);
    Ellipsoid_Axes(Output_Ellipsoid_Index,&output_a,&output_b);
    Ellipsoid_Flattening(Input_Ellipsoid_Index,&input_f);
    Ellipsoid_Flattening(Output_Ellipsoid_Index,&output_f);
    /********************************************************/
    /* Check for special cases when there is no datum shift */
    /********************************************************/
    if (input->datum_Index == output->datum_Index)
    {
      if ((input->type == MGRS) && 
               (output->type == UTM) && (output->parameters.UTM.override == 0))
      {
         temp_error = pMgrs->Set_MGRS_Parameters(input_a,input_b,
                                         Input_Ellipsoid_Code);
        if (temp_error)
        {
          if (temp_error & MGRS_A_ERROR)
            input->status |= ENGINE_A_ERROR;
          if (temp_error & MGRS_B_ERROR)
            input->status |= ENGINE_B_ERROR;
          if (temp_error & MGRS_A_LESS_B_ERROR)
            input->status |= ENGINE_A_LESS_B_ERROR;
        }
        else
        {
          temp_error = pMgrs->Convert_MGRS_To_UTM(input->coordinates.MGRS.string,
                                           &(output->coordinates.UTM.zone),
                                           &(output->coordinates.UTM.hemisphere),
                                           &(output->coordinates.UTM.easting),
                                           &(output->coordinates.UTM.northing));
          if (temp_error & MGRS_STRING_ERROR)
            input->status = ENGINE_MGRS_STR_ERROR;
        }
        special = 1;
      }
      else if ((input->type == UTM) && (output->type == MGRS))
      {
        temp_error = ENGINE_NO_ERROR;
        temp_error = pMgrs->Set_MGRS_Parameters(output_a,output_b,
                                         Output_Ellipsoid_Code);
        if (temp_error)
        {
          if (temp_error & MGRS_A_ERROR)
            output->status |= ENGINE_A_ERROR;
          if (temp_error & MGRS_B_ERROR)
            output->status |= ENGINE_B_ERROR;
          if (temp_error & MGRS_A_LESS_B_ERROR)
            output->status |= ENGINE_A_LESS_B_ERROR;
        }
        else
        {
          UTM_Tuple coord = input->coordinates.UTM;
          Precision temp_precision = Engine_Precision;
          if (temp_precision < Degree)
            temp_precision = Degree;
          if (temp_precision > Tenth_of_Second)
            temp_precision = Tenth_of_Second;
          temp_error = pMgrs->Convert_UTM_To_MGRS(coord.zone,
                                           coord.hemisphere,
                                           coord.easting,
                                           coord.northing,
                                           temp_precision,
                                           output->coordinates.MGRS.string);
          if (temp_error & MGRS_ZONE_ERROR)
            input->status |= ENGINE_ZONE_ERROR;
          if (temp_error & MGRS_HEMISPHERE_ERROR)
            input->status |= ENGINE_HEMISPHERE_ERROR;
          if (temp_error & MGRS_EASTING_ERROR)
            input->status |= ENGINE_EASTING_ERROR;
          if (temp_error & MGRS_NORTHING_ERROR)
            input->status |= ENGINE_NORTHING_ERROR;
        }
        special = 1;
      }
    }
    if (special)
    {  /* Special case error reporting */
      if (input->status)
        error_code = ENGINE_INPUT_ERROR;
      if (output->status)
        error_code = ENGINE_OUTPUT_ERROR;
      ce90 = 1.0;
      le90 = 1.0;
      se90 = 1.0;
    }
    else
    {
      /**********************************************************/
      /* First coordinate conversion stage, convert to Geodetic */
      /**********************************************************/
      switch (input->type)
      {
      case Geodetic:
        {
          Converted_Geodetic.latitude = input->coordinates.Geodetic.latitude;
          Converted_Geodetic.longitude = input->coordinates.Geodetic.longitude;
          Converted_Geodetic.height = input->coordinates.Geodetic.height;
          break;
        }
      case MGRS:
        {
          temp_error = pMgrs->Set_MGRS_Parameters(input_a,input_b,
                                           Input_Ellipsoid_Code);
          if (temp_error)
          {
            if (temp_error & MGRS_A_ERROR)
              input->status |= ENGINE_A_ERROR;
            if (temp_error & MGRS_B_ERROR)
              input->status |= ENGINE_B_ERROR;
            if (temp_error & MGRS_A_LESS_B_ERROR)
              input->status |= ENGINE_A_LESS_B_ERROR;
          }
          else
          {
            temp_error = pMgrs->Convert_MGRS_To_Geodetic(input->coordinates.MGRS.string,
                                                  &(Converted_Geodetic.latitude),
                                                  &(Converted_Geodetic.longitude));
            if (temp_error & MGRS_STRING_ERROR)
              input->status = ENGINE_MGRS_STR_ERROR;
            break;
          }
        }
      case UTM:
        {
          temp_error = pMgrs->Set_UTM_Parameters(input_a,input_b,0);
          if (temp_error)
          {
            if (temp_error & UTM_A_ERROR)
              input->status |= ENGINE_A_ERROR;
            if (temp_error & UTM_B_ERROR)
              input->status |= ENGINE_B_ERROR;
            if (temp_error & UTM_A_LESS_B_ERROR)
              input->status |= ENGINE_A_LESS_B_ERROR;
            if (temp_error & UTM_ZONE_OVERRIDE_ERROR)
              input->status |= ENGINE_ZONE_OVERRIDE_ERROR;
          }
          else
          {
            UTM_Tuple coord = input->coordinates.UTM;
            temp_error = pMgrs->Convert_UTM_To_Geodetic(coord.zone,coord.hemisphere,
                                                 coord.easting,coord.northing,
                                                 &(Converted_Geodetic.latitude),
                                                 &(Converted_Geodetic.longitude));
            if (temp_error & UTM_ZONE_ERROR)
              input->status |= ENGINE_ZONE_ERROR;
            if (temp_error & UTM_HEMISPHERE_ERROR)
              input->status |= ENGINE_HEMISPHERE_ERROR;
            if (temp_error & UTM_EASTING_ERROR)
              input->status |= ENGINE_EASTING_ERROR;
            if (temp_error & UTM_NORTHING_ERROR)
              input->status |= ENGINE_NORTHING_ERROR;
          }
          break;
        }
          break;
        }   
      } /* switch (input->type) */
      if ((input->status) && 
          (input->status != ENGINE_DATUM_WARNING) &&
          (input->status != ENGINE_LON_WARNING))
        error_code |= ENGINE_INPUT_ERROR;
      if (input->status == ENGINE_LON_WARNING)
        error_code |= ENGINE_INPUT_WARNING;
      /******************************/
      /* Datum Transformation Stage */
      /******************************/
      if (!(input->status && ENGINE_INPUT_ERROR))
      { /* Datum transformation */
        int WGS84_datum_index;
        Height_Type input_height_type;
        Height_Type output_height_type;
        if (input->type == Geodetic)
          input_height_type = input->parameters.Geodetic.height_type;
//        else if ((input->type == Geocentric) || (input->type == Local_Cartesian))
//          input_height_type = Ellipsoid_Height;
        else
          input_height_type = No_Height;
        if (output->type == Geodetic)
          output_height_type = output->parameters.Geodetic.height_type;
//        else if ((output->type == Geocentric) || (output->type == Local_Cartesian))
//          output_height_type = Ellipsoid_Height;
        else
          output_height_type = No_Height;
        Datum_Index ("WGE", &WGS84_datum_index);
        if ((input->datum_Index == output->datum_Index)
            && ((input_height_type == output_height_type)
                || (input_height_type == No_Height)
                || (output_height_type == No_Height)))
        { /* Copy coordinate tuple */
          WGS84_Geodetic = Converted_Geodetic;
          Shifted_Geodetic = Converted_Geodetic;
        }
        else
        { /* Shift to WGS84, apply geoid correction, shift to output datum */
          if (input->datum_Index != WGS84_datum_index)
          {
            Geodetic_Shift_To_WGS84(input->datum_Index,
                                    Converted_Geodetic.latitude,
                                    Converted_Geodetic.longitude,
                                    Converted_Geodetic.height,
                                    &(WGS84_Geodetic.latitude),
                                    &(WGS84_Geodetic.longitude),
                                    &(WGS84_Geodetic.height));
            if (input_height_type == Geoid_or_MSL_Height)
              WGS84_Geodetic.height = Converted_Geodetic.height;
            else if (input_height_type == No_Height)
              WGS84_Geodetic.height = 0.0;
            /* check input datum validity */
            temp_error = Valid_Datum (input->datum_Index, 
                                      WGS84_Geodetic.latitude, WGS84_Geodetic.longitude, &input_valid);
            if (!input_valid)
            {
              input->status |= ENGINE_DATUM_WARNING;
              error_code |= ENGINE_INPUT_WARNING;
            }
          }
          else
          { /* Copy coordinate tuple */
            WGS84_Geodetic = Converted_Geodetic;
          }
/*
          if ((input_height_type == Geoid_or_MSL_Height) &&
              (output_height_type == Ellipsoid_Height))
          {
            double Corrected_Height;
            Convert_Geoid_To_Ellipsoid_Height ( WGS84_Geodetic.latitude,
                                                WGS84_Geodetic.longitude,
                                                WGS84_Geodetic.height,
                                                &(Corrected_Height));
            WGS84_Geodetic.height = Corrected_Height;
          }

          if ((input_height_type == Ellipsoid_Height) &&
              (output_height_type == Geoid_or_MSL_Height))
          {
            double Corrected_Height;
            Convert_Ellipsoid_To_Geoid_Height ( WGS84_Geodetic.latitude,
                                                WGS84_Geodetic.longitude,
                                                WGS84_Geodetic.height,
                                                &(Corrected_Height));
            WGS84_Geodetic.height = Corrected_Height;
          }
*/
          if (output->datum_Index != WGS84_datum_index)
          {
            Geodetic_Shift_From_WGS84(WGS84_Geodetic.latitude,
                                      WGS84_Geodetic.longitude,
                                      WGS84_Geodetic.height,
                                      output->datum_Index,
                                      &(Shifted_Geodetic.latitude),
                                      &(Shifted_Geodetic.longitude),
                                      &(Shifted_Geodetic.height));
            if (output_height_type == Geoid_or_MSL_Height)
              Shifted_Geodetic.height = WGS84_Geodetic.height;
            else if (output_height_type == No_Height)
              Shifted_Geodetic.height = 0.0;
            /* check output datum validity */
            temp_error = Valid_Datum (output->datum_Index, 
                                      WGS84_Geodetic.latitude, WGS84_Geodetic.longitude, &output_valid);
            if (!output_valid)
            {
              output->status |= ENGINE_DATUM_WARNING;
              error_code |= ENGINE_OUTPUT_WARNING;
            }
          }
          else
          { /* Copy coordinate tuple */
            Shifted_Geodetic = WGS84_Geodetic;
          }
        }
      }
      /* calculate conversion errors */
      if ((input->status && ENGINE_DATUM_WARNING) || (output->status && ENGINE_DATUM_WARNING))
      {
        MgrsEngine::ce90 = -1.0;
        MgrsEngine::le90 = -1.0;
        MgrsEngine::se90 = -1.0;
      }
      else
        temp_error = Datum_Shift_Error (input->datum_Index, output->datum_Index, 
                                        WGS84_Geodetic.latitude, WGS84_Geodetic.longitude, &ce90, &le90, &se90);
      /*************************************************************/
      /* Second coordinate conversion stage, convert from Geodetic */
      /*************************************************************/
      if (error_code != ENGINE_INPUT_ERROR)
      {
        switch (output->type)
        {
        case Geodetic:
          {
            output->coordinates.Geodetic.latitude = Shifted_Geodetic.latitude;
            output->coordinates.Geodetic.longitude = Shifted_Geodetic.longitude;
            output->coordinates.Geodetic.height = Shifted_Geodetic.height;
            break;
          }
        case MGRS:
          {
            temp_error = pMgrs->Set_MGRS_Parameters(output_a,output_b,
                                             Output_Ellipsoid_Code);
            if (temp_error)
            {
              if (temp_error & MGRS_A_ERROR)
                output->status |= ENGINE_A_ERROR;
              if (temp_error & MGRS_B_ERROR)
                output->status |= ENGINE_B_ERROR;
              if (temp_error & MGRS_A_LESS_B_ERROR)
                output->status |= ENGINE_A_LESS_B_ERROR;
            }
            else
            {
              Precision temp_precision = Engine_Precision;
              if (temp_precision < Degree)
                temp_precision = Degree;
              if (temp_precision > Tenth_of_Second)
                temp_precision = Tenth_of_Second;
              pMgrs->Convert_Geodetic_To_MGRS(Shifted_Geodetic.latitude,
                                       Shifted_Geodetic.longitude,
                                       temp_precision,
                                       output->coordinates.MGRS.string);
              break;
            }
          }
        case UTM:
          {
            UTM_Parameters param = output->parameters.UTM;
            if (param.override && !param.zone)
              param.zone = output->coordinates.UTM.zone;
            else
            {
              if (!param.override)
                param.zone = 0;
            }
            temp_error = pMgrs->Set_UTM_Parameters(output_a,output_b, 
                                            param.zone);
            if (temp_error)
            {
              if (temp_error & UTM_A_ERROR)
                output->status |= ENGINE_A_ERROR;
              if (temp_error & UTM_B_ERROR)
                output->status |= ENGINE_B_ERROR;
              if (temp_error & UTM_A_LESS_B_ERROR)
                output->status |= ENGINE_A_LESS_B_ERROR;
              if (temp_error & UTM_ZONE_OVERRIDE_ERROR)
                output->status |= ENGINE_ZONE_OVERRIDE_ERROR;
            }
            else
            {
              UTM_Tuple coord;
              temp_error = pMgrs->Convert_Geodetic_To_UTM(Shifted_Geodetic.latitude,
                                                   Shifted_Geodetic.longitude,
                                                   &(coord.zone),
                                                   &(coord.hemisphere),
                                                   &(coord.easting),
                                                   &(coord.northing));
              output->coordinates.UTM = coord;
              if (temp_error & UTM_LAT_ERROR)
                output->status |= ENGINE_LAT_ERROR;
              if (temp_error & UTM_LON_ERROR)
                output->status |= ENGINE_LON_ERROR;
              if (temp_error & UTM_ZONE_OVERRIDE_ERROR)
                output->status |= ENGINE_ZONE_ERROR;
              if (temp_error & UTM_EASTING_ERROR)
                output->status |= ENGINE_EASTING_ERROR;
              if (temp_error & UTM_NORTHING_ERROR)
                output->status |= ENGINE_NORTHING_ERROR;
            }
            break;
          }
        }           /* switch (output->type) */
        if ((output->status) && 
            (output->status != ENGINE_DATUM_WARNING) &&
            (output->status != ENGINE_LON_WARNING))
          error_code |= ENGINE_OUTPUT_ERROR;
        if (output->status == ENGINE_LON_WARNING)
          error_code |= ENGINE_OUTPUT_WARNING;
        if ((input->status && ENGINE_LON_WARNING) || (output->status && ENGINE_LON_WARNING))
        { /* if a distortion warning occurs, standard errors are unknown */
          ce90 = -1.0;
          le90 = -1.0;
          se90 = -1.0;
        }
      }
//    } /* if (!special) */
  } /* if (!error_code) */
  return (error_code);
} /* Convert */


int MgrsEngine::Get_Conversion_Errors
    ( const MgrsEngine::File_or_Interactive State,
      double *CE90,
      double *LE90,
      double *SE90)
/*
 *  The function Get_Conversion_Errors returns the estimated errors in the location
 *  the most recently converted location
 *  State  : Indicates whether the datum is to be used for interactive 
 *           or file processing                                    (input)
 *  CE90   : 90% circular (horizontal) error, in meters            (output)
 *  LE90   : 90% linear (vertical) error, in meters                (output)
 *  SE90   : 90% spherical error, in meters                        (output)
 */
{ /* Get_Conversion_Errors */
  int error_code = ENGINE_NO_ERROR;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
//    *CE90 = MgrsEngine::ce90;
//    *LE90 = MgrsEngine::le90;
//    *SE90 = MgrsEngine::se90;
  }
  return (error_code);
} /* Get_Conversion_Errors */


int MgrsEngine::Get_Conversion_Status
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output Direction,
      int  *Conversion_Status )
/*
 *  The function Get_Conversion_Status returns the current status for the specified state..
 *  State      : Indicates whether the datum is to be used for interactive
 *               or file processing                                    (input)
 *  Direction  : Indicates whether the datum is to be used for input or
 *               output                                                (input)
 *  Conversion_Status : The current status for the specified state     (output)
 */
{ /* Get_Conversion_Status */
  int error_code = ENGINE_NO_ERROR;

  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    *Conversion_Status = MgrsEngine::CS_State[State][Direction].status;
  }
  return (error_code);
} /* Get_Conversion_Status */



int MgrsEngine::Get_Conversion_Status_String
    ( const MgrsEngine::File_or_Interactive State,
      const MgrsEngine::Input_or_Output     Direction,
      char  *Separator,
      char  *String)
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
{ /* Get_Conversion_Status_String */
  int error_code = ENGINE_NO_ERROR;
  char   *in_out;
  int Error_Code;
  MgrsEngine::Coordinate_Type System;
  if (!MgrsEngine::Engine_Initialized)
    error_code |= ENGINE_NOT_INITIALIZED;
  if (!Valid_Direction(Direction))
    error_code |= ENGINE_INVALID_DIRECTION;
  if (!Valid_State(State))
    error_code |= ENGINE_INVALID_STATE;
  if (!error_code)
  {
    if (Direction == MgrsEngine::Input)
      in_out = "Input";
    else
      in_out = "Output";
    Get_Coordinate_System(State, Direction, &System);
    Get_Conversion_Status(State, Direction, &Error_Code);
    switch (System)
    {
    case MgrsEngine::Geodetic:
      {
        sprintf(String,"%s%s%s%s",in_out," Geodetic Coordinates:",Separator,Separator);
        break;
      }
    case MgrsEngine::MGRS:
      {
        sprintf(String,"%s%s%s%s",in_out," MGRS Coordinates:",Separator,Separator);
        break;
      }
    case MgrsEngine::UTM:
      {
        sprintf(String,"%s%s%s%s",in_out," UTM Coordinates:",Separator,Separator);
        break;
      }
    } /* switch */
    if (Error_Code & ENGINE_DATUM_WARNING)
    {
      strcat(String,"Location is not within valid domain for current datum");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_LON_WARNING)
    {
      strcat(String,"Longitude is too far from Central Meridian, distortion may be significant");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_A_ERROR)
    {
      strcat(String,"Ellipsoid semi-major axis must be greater than zero");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_B_ERROR)
    {
      strcat(String,"Ellipsoid semi-minor axis must be greater than zero");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_A_LESS_B_ERROR)
    {
      strcat(String,"Ellipsoid semi-major axis must be greater than semi-minor axis");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_ORIGIN_LAT_ERROR)
    {
      strcat(String,"Origin Latitude (or Latitude of True Scale) out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_STDP_ERROR)
    {
      strcat(String,"Standard_Parallel out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_ORIGIN_LON_ERROR)
    {
      strcat(String,"Origin Longitude (or Longitude Down from Pole) out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_CENT_MER_ERROR)
    {
      strcat(String,"Central Meridian out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_RADIUS_ERROR)
    {
      strcat(String,"Easting/Northing too far from center of projection");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_HEMISPHERE_ERROR)
    {
      strcat(String,"Standard Parallels cannot be equal and opposite latitudes");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_SCALE_FACTOR_ERROR)
    {
      strcat(String,"Scale Factor out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_LAT_ERROR)
    {
      strcat(String,"Latitude out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_LON_ERROR)
    {
      strcat(String,"Longitude out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_EASTING_ERROR)
    {
      strcat(String,"Easting/X out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_NORTHING_ERROR)
    {
      strcat(String,"Northing/Y out of range");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_SECOND_STDP_ERROR)
    {
      strcat(String,"Invalid 1st Standard Parallel");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_FIRST_STDP_ERROR)
    {
      strcat(String,"Invalid 2nd Standard Parallel");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_FIRST_SECOND_ERROR)
    {
      strcat(String,"1st & 2nd Standard Parallels cannot both be zero");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_ZONE_ERROR)
    {
      strcat(String,"Invalid Zone");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_ZONE_OVERRIDE_ERROR)
    {
      strcat(String,"Invalid Zone Override");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_MGRS_STR_ERROR)
    {
      strcat(String,"Invalid MGRS String");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_STR_LON_MIN_ERROR)
    {
      strcat(String,"The longitude minute part of the GEOREF string is greater than 60");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_STR_LAT_MIN_ERROR)
    {
      strcat(String,"The latitude minute part of the GEOREF string is greater than 60");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_DATUM_SIGMA_ERROR)
    {
      strcat(String,"Invalid standard error value, must be positive of -1 for unknown");
      strcat(String,Separator);
    }
    if (Error_Code & ENGINE_DATUM_DOMAIN_ERROR)
    {
      strcat(String,"Invalid definition for datum domain of validity");
      strcat(String,Separator);
    }
  }
  return (error_code);
} /* Get_Conversion_Status_String */



void MgrsEngine::Get_Return_Code_String
    ( int  Error_Code,
      char  *Separator,
      char  *String)
/*
 *  The function Get_Return_Code_String returns a string describing the specified 
 *  engine return code.
 *  Error_Code : Status code returned by engine function               (input) 
 *  Separator  : String to be used to separate individual status messages
 *                                                                     (input)
 *  String     : String describing the current status for the specified state     
 *                                                                     (output)
 */
{ /* Get_Return_Code_String */
  String[0] = 0;
  if (Error_Code & ENGINE_INPUT_WARNING)
  {
    strcat(String,"Warning returned by 1st conversion");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_OUTPUT_WARNING)
  {
    strcat(String,"Warning returned by 2st conversion");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_INPUT_ERROR)
  {
    strcat(String,"Error returned by 1st conversion");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_OUTPUT_ERROR)
  {
    strcat(String,"Error returned by 2nd conversion");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_INVALID_TYPE)
  {
    strcat(String,"Invalid coordinate system type");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_INVALID_DIRECTION)
  {
    strcat(String,"Invalid direction (input or output)");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_INVALID_STATE)
  {
    strcat(String,"Invalid state (interactive or file)");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_INVALID_INDEX_ERROR)
  {
    strcat(String,"Index value outside of valid range");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_INVALID_CODE_ERROR)
  {
    strcat(String,"Specified code already in use");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_ELLIPSOID_OVERFLOW)
  {
    strcat(String,"Ellipsoid table is full");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_DATUM_OVERFLOW)
  {
    strcat(String,"3-parameter datum table is full");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_DATUM_SIGMA_ERROR)
  {
    strcat(String,"Standard error values must be positive, or -1 if unknown");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_DATUM_DOMAIN_ERROR)
  {
    strcat(String,"Invalid local datum domain of validity");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_LAT_ERROR)
  {
    strcat(String,"Latitude out of range");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_LON_ERROR)
  {
    strcat(String,"Longitude out of range");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_A_ERROR)
  {
    strcat(String,"Ellipsoid semi-major axis must be greater than zero");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_B_ERROR)
  {
    strcat(String,"Ellipsoid semi-minor axis must be greater than zero");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_A_LESS_B_ERROR)
  {
    strcat(String,"Ellipsoid semi-major axis must be greater than semi-minor axis");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_DATUM_ERROR)
  {
    strcat(String,"Error returned by Datum module");
    strcat(String,Separator);
  }
  if (Error_Code & ENGINE_ELLIPSOID_ERROR)
  {
    strcat(String,"Error returned by Ellipsoid module");
    strcat(String,Separator);
  }
} /* Get_Return_Code_String */

 
