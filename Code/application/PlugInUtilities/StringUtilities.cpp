/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "StringUtilities.h"
#include "ApplicationServices.h"
#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "DateTime.h"
#include "Filename.h"
#include "GeoPoint.h"
#include "Int64.h"
#include "Locator.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "Point.h"
#include "SpatialDataView.h"
#include "TypesFile.h"
#include "UInt64.h"
#include "xmlbase.h"

#include <iomanip>
#include <sstream>
#include <limits>

#include "StringUtilitiesMacros.h"

using namespace std;

namespace StringUtilities
{

/* this is a handy-dandy function that reads READSIZE bytes into the target string in APPEND MODE
by default. This way, you can actually OVERWRITE a string if you'd like!
*/
bool readSTLString(FILE* pInputFile, size_t readSize, string &target, bool append)
{
   bool success = true;
   char c;
   
   if (!append)
   {
      // if we're not appending anyway, let's do it faster and more efficiently - with a VECTOR
      vector<char> buf(readSize + 1);

      buf[readSize] = '\0';
      char* pBuffer = &buf.at(0);

      if (0 == fread(pBuffer, 1, readSize, pInputFile))
      {
         return false; // bad read -> kick out
      }

      target = pBuffer;
   }
   else
   {
      for (size_t i = 0; i < readSize; i++)
      {
         if (success)
         {
            success = 0 < fread(&c, 1, 1, pInputFile); // read it
         }

         if (success)
         {
            target.append(1, c);
         }
      }
   }

   return success;
}

string stripRightWhitespace(const string& source)
{
   unsigned int right = 0;
   for (right = source.length() - 1; right < source.length(); right--)
   {
      if (source[right] < 0)
      {
         break;
      }

      if (isspace(source[right]) == 0)
      {
         break;
      }
   }

   return source.substr(0, right + 1);
}

string stripLeftWhitespace(const string& source)
{
   unsigned int left = 0;
   for (left = 0; left < source.length(); left++)
   {
      if (source[left] < 0)
      {
         break;
      }

      if (isspace(source[left]) == 0)
      {
         break;
      }
   }

   return source.substr(left, source.length() - left);
}

string stripWhitespace(const string& source)
{
   string copy = stripRightWhitespace(source);
   copy = stripLeftWhitespace(copy);
   return copy;
}

bool isAllBlank(const string &source)
{
   static const basic_string <char>::size_type npos = -1;
   basic_string <char>::size_type findIndex = source.find_first_not_of(" ");
   if (findIndex == npos)
   {
      return true;      // Found an all blank string
   }

   return false;
}

string toLower(const string& source)
{
   string lowerString = source;
   for (unsigned int i = 0; i < lowerString.size(); i++)
   {
      lowerString.at(i) = tolower(lowerString.at(i));
   }

   return lowerString;
}

string toUpper(const string& source)
{
   string upperString = source;
   for (unsigned int i = 0; i < upperString.size(); i++)
   {
      upperString.at(i) = toupper(upperString.at(i));
   }

   return upperString;
}

vector<string> split(const string &source, char separator)
{
   vector<char> sep(1, separator);
   string::const_iterator start = source.begin();
   string::const_iterator stop = find_first_of(start, source.end(), sep.begin(), sep.end());
   vector<string> components;
   while (stop != source.end())
   {
      components.push_back(source.substr(start-source.begin(), stop-start));
      start = stop+1;
      stop = find_first_of(start, source.end(), sep.begin(), sep.end());
   }
   if (stop!=start)
   {
      components.push_back(source.substr(start-source.begin(), stop-start));
   }
   return components;
}

string join(const vector<string> &source, const string &separator)
{
   string rval;
   for(vector<string>::const_iterator it = source.begin(); it != source.end(); ++it)
   {
      if (it != source.begin())
      {
         rval += separator;
      }
      rval += *it;
   }
   return rval;
}

string latLonToText(LocationType latLonCoords)
{
   LatLonPoint latLon(latLonCoords);
   return latLon.getText().c_str();
}

string expandVariables(const string& originalString,
   const vector<string>& ignoredExpansions)
{
   bool allowV = (find(ignoredExpansions.begin(), ignoredExpansions.end(), "V") == ignoredExpansions.end());
   bool allowC = (find(ignoredExpansions.begin(), ignoredExpansions.end(), "C") == ignoredExpansions.end());
   bool allowE = (find(ignoredExpansions.begin(), ignoredExpansions.end(), "E") == ignoredExpansions.end());
   Service<ConfigurationSettings> pSettings;
   string newString = originalString;
   for (int i = 0; i < 50; i++)
   {
      //each pass does replacement of $V(a) currently in the string.
      //do 50 passes to perform sub-expansion at most fifty times, ie. prevent infinite loop
      //for non-terminating recursive expansion
      string::size_type pos = newString.find("$");
      while (pos != string::npos)
      {
         if (pos + 1 >= newString.size())
         {
            break;
         }
         string type = newString.substr(pos+1, 1);
         if (type != "$") //ie. not $$, the escape sequence so continue
         {
            bool replaced = false;
            if (pos+4 < newString.size()) //ie. $V(a)
            {
               if (newString[pos+2] == '(')
               {
                  string::size_type closeParen = newString.find(')', pos+2);
                  string variableName = newString.substr(pos+3, closeParen-(pos+2)-1);
                  string replacementString;
                  if (type == "V" && allowV)
                  {
                     if (variableName == "APP_HOME")
                     {
                        replacementString = pSettings->getHome();
                        replaced = true;
                     }
                     else if (variableName == "USER_DOCS")
                     {
                        replacementString = pSettings->getUserDocs();
                        replaced = true;
                     }
                     else if (variableName == "APP_VERSION")
                     {
                        replacementString = pSettings->getVersion();
                        replaced = true;
                     }
                  }
                  else if (type == "E" && allowE)
                  {
                     char* value = getenv(variableName.c_str());
                     if (value != NULL)
                     {
                        replacementString = value;
                        if (replacementString.size() > 2 && replacementString[0] == '"' &&
                           replacementString[replacementString.size() - 1] == '"')
                        {
                           replacementString = replacementString.substr(1, replacementString.size() - 2);
                        }
                        replaced = true;
                     }
                  }
                  else if (type == "C" && allowC)
                  {
                     const DataVariant& value = pSettings->getSetting(variableName);
                     if (value.isValid())
                     {
                        if (value.getTypeName() == "Filename")
                        {
                           const Filename* pFilename = dv_cast<Filename>(&value);
                           if (pFilename != NULL)
                           {
                              replacementString = pFilename->getFullPathAndName();
                              replaced = true;
                           }
                        }
                        else
                        {
                           replacementString = value.toXmlString();
                           replaced = true;
                        }
                     }
                  }
                  if (replaced)
                  {
                     newString.replace(pos, closeParen-pos+1, replacementString);
                     pos = newString.find("$", pos+replacementString.size());
                  }
               }
            }
            if (!replaced)
            {
               pos = newString.find("$", pos+1);
            }
         }
         else
         {
            pos = newString.find("$", pos+2);
         }
      }
   }
   string::size_type pos = newString.find("$$");
   while (pos != string::npos)
   {
      newString.replace(pos, 2, "$");
      pos = newString.find("$$");
   }

   return newString;
}

const char* escapedToken(const string &textIn)
{
   static string text;
   static string retval;
   static unsigned int pos;
   retval.clear();
   if (textIn.empty() == false)
   {
      text = textIn;
      pos = 0;
   }
   bool done = false;
   bool escaped = false;
   for (; pos < text.size() && !done; ++pos)
   {
      char ch = text[pos];
      if (!escaped)
      {
         if (ch == '\\')
         {
            escaped = true;
         }
         else if (isspace(ch))
         {
            done = true;
         }
         else
         {
            retval.push_back(ch);
         }
      }
      else
      {
         escaped = false;
         retval.push_back(ch);
      }
   }
   if (retval.empty())
   {
      return NULL;
   }
   retval = stripWhitespace(retval);

   return retval.c_str();
}

//toString and fromString implementations

template<>
string toDisplayString(const bool& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   return (val ? "True" : "False");
}

template<>
string toXmlString(const bool& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   return (val ? "true" : "false");
}

template<>
bool fromDisplayString<bool>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   string lowerValue = toLower(value);
   if ( (lowerValue == "true") || (lowerValue == "1") ||
        (lowerValue == "t") || (lowerValue == "y") ||
        (lowerValue == "yes") )
   {
      return true;
   }
   else if ( (lowerValue == "false") || (lowerValue == "0") ||
        (lowerValue == "f") || (lowerValue == "n") ||
        (lowerValue == "no") )
   {
      return false;
   }
   else
   {
      if (pError != NULL)
      {
         *pError = true;
      }
      return false;
   }
}

template<>
bool fromXmlString<bool>(string value, bool* pError)
{
   return fromDisplayString<bool>(value, pError);
}

STRINGSTREAM_MAPPING_TO_DISPLAY_STRING_VEC(bool)
STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING_VEC(bool)
STRINGSTREAM_MAPPING_TO_XML_STRING_VEC(bool)
STRINGSTREAM_MAPPING_FROM_XML_STRING_VEC(bool)

STRINGSTREAM_MAPPING_CAST(char, short)
STRINGSTREAM_MAPPING_CAST(unsigned char, unsigned short)
STRINGSTREAM_MAPPING_CAST(signed char, short)
STRINGSTREAM_MAPPING(short)
STRINGSTREAM_MAPPING(unsigned short)
STRINGSTREAM_MAPPING(int)
STRINGSTREAM_MAPPING(unsigned int)
#ifdef WIN_API
STRINGSTREAM_MAPPING_CAST(long, int64_t)
STRINGSTREAM_MAPPING_CAST(unsigned long, uint64_t)
#endif
STRINGSTREAM_MAPPING(int64_t)
STRINGSTREAM_MAPPING(uint64_t)
STRINGSTREAM_MAPPING_PRECISION(float, numeric_limits<float>::digits10)
STRINGSTREAM_MAPPING_PRECISION(double, numeric_limits<double>::digits10)

BEGIN_ENUM_MAPPING(AoiAddMode)
ADD_ENUM_MAPPING(APPEND_AOI, "Append", "Append")
ADD_ENUM_MAPPING(REPLACE_AOI, "Replace", "Replace")
ADD_ENUM_MAPPING(NEW_AOI, "New", "New")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(AnimationCycle)
ADD_ENUM_MAPPING(PLAY_ONCE, "Play Once", "play_once")
ADD_ENUM_MAPPING(REPEAT, "Repeat", "repeat")
ADD_ENUM_MAPPING(BOUNCE, "Bounce", "bounce")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(AnimationState)
ADD_ENUM_MAPPING(STOP, "Stop", "stop")
ADD_ENUM_MAPPING(PLAY_FORWARD, "Play Forward", "play_forward")
ADD_ENUM_MAPPING(PLAY_BACKWARD, "Play Backward", "play_backward")
ADD_ENUM_MAPPING(PAUSE_FORWARD, "Pause Forward", "pause_forward")
ADD_ENUM_MAPPING(PAUSE_BACKWARD, "Pause Backward", "pause_backward")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ArcRegion)
ADD_ENUM_MAPPING(ARC_CENTER, "Center", "Center")
ADD_ENUM_MAPPING(ARC_CHORD, "Chord", "Chord")
ADD_ENUM_MAPPING(ARC_OPEN, "Open", "Open")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ArrowStyle)
ADD_ENUM_MAPPING(ARROW_NONE, "No Arrow", "none")
ADD_ENUM_MAPPING(ARROW_SMALL, "Small", "small")
ADD_ENUM_MAPPING(ARROW_LARGE, "Large", "large")
ADD_ENUM_MAPPING(ARROW_TRIANGLE_SMALL, "Small Triangle", "triangleSmall")
ADD_ENUM_MAPPING(ARROW_TRIANGLE_LARGE, "Large Triangle", "triangleLarge")
ADD_ENUM_MAPPING(ARROW_TRIANGLE_SMALL_FILL, "Small Filled Triangle", "triangleSmallFill")
ADD_ENUM_MAPPING(ARROW_TRIANGLE_LARGE_FILL, "Large Filled Triangle", "triangleLargeFill")
ADD_ENUM_MAPPING(ARROW_DIAMOND, "Diamond", "diamond")
ADD_ENUM_MAPPING(ARROW_DIAMOND_FILL, "Filled Diamond", "diamondFill")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ComplexComponent)
ADD_ENUM_MAPPING(COMPLEX_MAGNITUDE, "Magnitude", "Magnitude")
ADD_ENUM_MAPPING(COMPLEX_PHASE, "Phase", "Phase")
ADD_ENUM_MAPPING(COMPLEX_INPHASE, "In-Phase", "Inphase")
ADD_ENUM_MAPPING(COMPLEX_QUADRATURE, "Quadrature", "Quadrature")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(DataOrigin)
ADD_ENUM_MAPPING(LOWER_LEFT, "LowerLeft", "LowerLeft")
ADD_ENUM_MAPPING(UPPER_LEFT, "UpperLeft", "UpperLeft")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(DisplayMode)
ADD_ENUM_MAPPING(GRAYSCALE_MODE, "Grayscale", "grayscale")
ADD_ENUM_MAPPING(RGB_MODE, "RGB", "rgb")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(DistanceUnits)
ADD_ENUM_MAPPING(NO_DISTANCE_UNIT, "No Unit", "No Unit")
ADD_ENUM_MAPPING(KILOMETER, "Kilometer", "Kilometer")
ADD_ENUM_MAPPING(MILE, "Statute Mile", "Statute Mile")
ADD_ENUM_MAPPING(NAUTICAL_MILE, "Nautical Mile", "Nautical Mile")
ADD_ENUM_MAPPING(METER, "Meter", "Meter")
ADD_ENUM_MAPPING(YARD, "Yard", "Yard")
ADD_ENUM_MAPPING(FOOT, "Foot", "Foot")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(DmsFormatType)
ADD_ENUM_MAPPING(DMS_FULL, "DD° MM' SS.SS\"", "Full")
ADD_ENUM_MAPPING(DMS_FULL_DECIMAL, "DD.ddd°", "DecimalDegrees")
ADD_ENUM_MAPPING(DMS_MINUTES_DECIMAL, "DD° MM.MM'", "DegreesMinutes")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(EncodingType)
ADD_ENUM_MAPPING(INT1UBYTE, "Unsigned 1 Byte", "INT1UBYTE")
ADD_ENUM_MAPPING(INT1SBYTE, "Signed 1 Byte", "INT1SBYTE")
ADD_ENUM_MAPPING(INT2UBYTES, "Unsigned 2 Bytes", "INT2UBYTES")
ADD_ENUM_MAPPING(INT2SBYTES, "Signed 2 Bytes", "INT2SBYTES")
ADD_ENUM_MAPPING(INT4UBYTES, "Unsigned 4 Bytes", "INT4UBYTES")
ADD_ENUM_MAPPING(INT4SBYTES, "Signed 4 Bytes", "INT4SBYTES")
ADD_ENUM_MAPPING(INT4SCOMPLEX, "Integer Complex", "INT4SCOMPLEX")
ADD_ENUM_MAPPING(FLT4BYTES, "Float 4 Bytes", "FLT4BYTES")
ADD_ENUM_MAPPING(FLT8COMPLEX, "Float Complex", "FLT8COMPLEX")
ADD_ENUM_MAPPING(FLT8BYTES, "Float 8 Bytes", "FLT8BYTES")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(EndianType)
ADD_ENUM_MAPPING(BIG_ENDIAN_ORDER, "Big", "big")
ADD_ENUM_MAPPING(LITTLE_ENDIAN_ORDER, "Little", "little")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(FillStyle)
ADD_ENUM_MAPPING(SOLID_FILL, "Solid", "Solid")
ADD_ENUM_MAPPING(HATCH, "Hatch", "Hatch")
ADD_ENUM_MAPPING(EMPTY_FILL, "Empty", "Empty")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(FrameType)
ADD_ENUM_MAPPING(FRAME_ID, "Id", "id")
ADD_ENUM_MAPPING(FRAME_TIME, "Time", "time")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(GcpSymbol)
ADD_ENUM_MAPPING(GCP_X, "X", "X")
ADD_ENUM_MAPPING(GCP_PLUS, "Plus", "Plus")
ADD_ENUM_MAPPING(GCP_NODRAW, "NoDraw", "NoDraw")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(GeocoordType)
ADD_ENUM_MAPPING(GEOCOORD_GENERAL, "General", "General")
ADD_ENUM_MAPPING(GEOCOORD_LATLON, "Lat/Lon", "Lat/Lon")
ADD_ENUM_MAPPING(GEOCOORD_UTM, "UTM", "UTM")
ADD_ENUM_MAPPING(GEOCOORD_MGRS, "MGRS", "MGRS")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(GraphicObjectType)
ADD_ENUM_MAPPING(ARC_OBJECT, "Arc", "Arc")
ADD_ENUM_MAPPING(ARROW_OBJECT, "Arrow", "Arrow")
ADD_ENUM_MAPPING(BITMASK_OBJECT, "Bitmask", "Bitmask")
ADD_ENUM_MAPPING(CGM_OBJECT, "CGM", "CGM")
ADD_ENUM_MAPPING(COLUMN_OBJECT, "Column", "Column")
ADD_ENUM_MAPPING(EASTARROW_OBJECT, "East Arrow", "East Arrow")
ADD_ENUM_MAPPING(ELLIPSE_OBJECT, "Ellipse", "Ellipse")
ADD_ENUM_MAPPING(FILE_IMAGE_OBJECT, "File Image", "File Image")
ADD_ENUM_MAPPING(FRAME_LABEL_OBJECT, "Frame Label", "Frame Label")
ADD_ENUM_MAPPING(GROUP_OBJECT, "Group", "Group")
ADD_ENUM_MAPPING(HLINE_OBJECT, "Horizontal Line", "Horizontal Line")
ADD_ENUM_MAPPING(LATLONINSERT_OBJECT, "Lat/Lon Insert", "Latitude/Longitude")
ADD_ENUM_MAPPING(LINE_OBJECT, "Line", "Line")
ADD_ENUM_MAPPING(MEASUREMENT_OBJECT, "Measurement", "Measurement")
ADD_ENUM_MAPPING(MOVE_OBJECT, "Move", "Move")
ADD_ENUM_MAPPING(MULTIPOINT_OBJECT, "Point", "Multipoint")
ADD_ENUM_MAPPING(NORTHARROW_OBJECT, "North Arrow", "North Arrow")
ADD_ENUM_MAPPING(POLYGON_OBJECT, "Polygon", "Polygon")
ADD_ENUM_MAPPING(POLYLINE_OBJECT, "Polyline", "Polyline")
ADD_ENUM_MAPPING(RAW_IMAGE_OBJECT, "Image", "Image")
ADD_ENUM_MAPPING(RECTANGLE_OBJECT, "Rectangle", "Rectangle")
ADD_ENUM_MAPPING(ROTATE_OBJECT, "Rotate", "Rotate")
ADD_ENUM_MAPPING(ROUNDEDRECTANGLE_OBJECT, "Rounded Rectangle", "Rounded Rectangle")
ADD_ENUM_MAPPING(ROW_OBJECT, "Row", "Row")
ADD_ENUM_MAPPING(SCALEBAR_OBJECT, "Scale Bar", "Scale Bar")
ADD_ENUM_MAPPING(TEXT_OBJECT, "Text", "Text")
ADD_ENUM_MAPPING(TRAIL_OBJECT, "Trail", "Trail")
ADD_ENUM_MAPPING(TRIANGLE_OBJECT, "Triangle", "Triangle")
ADD_ENUM_MAPPING(VIEW_OBJECT, "View", "View")
ADD_ENUM_MAPPING(VLINE_OBJECT, "Vertical Line", "Vertical Line")
ADD_ENUM_MAPPING(WIDGET_IMAGE_OBJECT, "Plot Image", "Widget Image")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(InsetZoomMode)
ADD_ENUM_MAPPING(ABSOLUTE_MODE, "Absolute", "Absolute")
ADD_ENUM_MAPPING(RELATIVE_MODE, "Relative", "Relative")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(InterleaveFormatType)
ADD_ENUM_MAPPING(BIL, "BIL", "BIL")
ADD_ENUM_MAPPING(BIP, "BIP", "BIP")
ADD_ENUM_MAPPING(BSQ, "BSQ", "BSQ")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(LatLonStyle)
ADD_ENUM_MAPPING(LATLONSTYLE_SOLID, "Solid", "Solid")
ADD_ENUM_MAPPING(LATLONSTYLE_DASHED, "Dashed", "Dashed")
ADD_ENUM_MAPPING(LATLONSTYLE_CROSS, "Crosshair", "Crosshair")
ADD_ENUM_MAPPING(LATLONSTYLE_NONE, "None", "None")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(LayerType)
ADD_ENUM_MAPPING(ANNOTATION, "Annotation", "AnnotationLayer")
ADD_ENUM_MAPPING(AOI_LAYER, "AOI", "AoiLayer")
ADD_ENUM_MAPPING(CONTOUR_MAP, "Contour Map", "ContourMapLayer")
ADD_ENUM_MAPPING(GCP_LAYER, "GCP List", "GcpLayer")
ADD_ENUM_MAPPING(GRAPHIC_LAYER, "Graphic", "GraphicLayer")
ADD_ENUM_MAPPING(LAT_LONG, "Latitude/Longtitude", "LatitudeLongitudeLayer")
ADD_ENUM_MAPPING(PSEUDOCOLOR, "Pseudocolor", "PseudocolorLayer")
ADD_ENUM_MAPPING(RASTER, "Raster", "RasterLayer")
ADD_ENUM_MAPPING(THRESHOLD, "Threshold", "ThresholdLayer")
ADD_ENUM_MAPPING(TIEPOINT_LAYER, "Tie Point List", "TiePointLayer")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(LineStyle)
ADD_ENUM_MAPPING(SOLID_LINE, "Solid", "Solid")
ADD_ENUM_MAPPING(DASHED, "Dashed", "Dashed")
ADD_ENUM_MAPPING(DOT, "Dot", "Dot")
ADD_ENUM_MAPPING(DASH_DOT, "Dash-Dot", "DashDot")
ADD_ENUM_MAPPING(DASH_DOT_DOT, "Dash-Dot-Dot", "DashDotDot")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(LinkType)
ADD_ENUM_MAPPING(NO_LINK, "No Link", "NoLink")
ADD_ENUM_MAPPING(AUTOMATIC_LINK, "Automatic Link", "AutomaticLink")
ADD_ENUM_MAPPING(MIRRORED_LINK, "Mirrored Link", "MirroredLink")
ADD_ENUM_MAPPING(GEOCOORD_LINK, "Geocoord Link", "GeocoordLink")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING_ALIAS(::Locator::LocatorStyle, LocatorLocatorStyle)
ADD_ENUM_MAPPING(::Locator::HORIZONTAL_LOCATOR, "Horizontal", "horizontal")
ADD_ENUM_MAPPING(::Locator::VERTICAL_LOCATOR, "Vertical", "vertical")
ADD_ENUM_MAPPING(::Locator::CROSSHAIR_LOCATOR, "Crosshair", "crosshair")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ModeType)
ADD_ENUM_MAPPING(DRAW, "Draw", "Draw")
ADD_ENUM_MAPPING(ERASE, "Erase", "Erase")
ADD_ENUM_MAPPING(TOGGLE, "Toggle", "Toggle")
ADD_ENUM_MAPPING(AOI_MOVE, "AOI Move", "AOIMove")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(OrientationType)
ADD_ENUM_MAPPING(HORIZONTAL, "Horizontal", "horizontal")
ADD_ENUM_MAPPING(VERTICAL, "Vertical", "vertical")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(PanLimitType)
ADD_ENUM_MAPPING(NO_LIMIT, "No Limit", "NoLimit")
ADD_ENUM_MAPPING(CUBE_EXTENTS, "Limit To Cube Extents", "CubeExtents")
ADD_ENUM_MAPPING(MAX_EXTENTS, "Limit To Extents of All Layers", "MaxExtents")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(PassArea)
ADD_ENUM_MAPPING(LOWER, "Less Than", "Lower")
ADD_ENUM_MAPPING(UPPER, "Greater Than", "Upper")
ADD_ENUM_MAPPING(MIDDLE, "Between", "Middle")
ADD_ENUM_MAPPING(OUTSIDE, "Outside", "Outside")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(PlotObjectType)
ADD_ENUM_MAPPING(CURVE, "Curve", "Curve")
ADD_ENUM_MAPPING(CURVE_COLLECTION, "Curve Collection", "CurveCollection")
ADD_ENUM_MAPPING(HISTOGRAM, "Histogram", "Histogram")
ADD_ENUM_MAPPING(LOCATOR, "Locator", "Locator")
ADD_ENUM_MAPPING(REGION, "Region", "Region")
ADD_ENUM_MAPPING(POINT_OBJECT, "Point", "Point")
ADD_ENUM_MAPPING(POINT_SET, "Point Set", "PointSet")
ADD_ENUM_MAPPING(ARROW, "Arrow", "Arrow")
ADD_ENUM_MAPPING(AXIS, "Axis", "Axis")
ADD_ENUM_MAPPING(CARTESIAN_GRIDLINES, "Cartesian Gridlines", "CartesianGridlines")
ADD_ENUM_MAPPING(TEXT_OBJECT_TYPE, "Text", "Text")
ADD_ENUM_MAPPING(PLOT_GROUP, "Plot Group", "PlotGroup")
ADD_ENUM_MAPPING(POLAR_GRIDLINES, "Polar Gridlines", "PolarGridlines")
ADD_ENUM_MAPPING(POLYGON_OBJECT_TYPE, "Polygon", "Polygon")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(PlotType)
ADD_ENUM_MAPPING(CARTESIAN_PLOT, "Cartesian", "Cartesian")
ADD_ENUM_MAPPING(HISTOGRAM_PLOT, "Histogram", "Histogram")
ADD_ENUM_MAPPING(SIGNATURE_PLOT, "Signature", "Signature")
ADD_ENUM_MAPPING(POLAR_PLOT, "Polar", "Polar")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING_ALIAS(Point::PointSymbolType, PointPointSymbolType)
ADD_ENUM_MAPPING(Point::SOLID, "Solid", "solid")
ADD_ENUM_MAPPING(Point::X, "X", "x")
ADD_ENUM_MAPPING(Point::CROSS_HAIR, "+", "crosshair")
ADD_ENUM_MAPPING(Point::ASTERISK, "*", "asterisk")
ADD_ENUM_MAPPING(Point::HORIZONTAL_LINE, "-", "horizontalLine")
ADD_ENUM_MAPPING(Point::VERTICAL_LINE, "|", "verticalLine")
ADD_ENUM_MAPPING(Point::FORWARD_SLASH, "/", "forwardSlash")
ADD_ENUM_MAPPING(Point::BACK_SLASH, "\\", "backSlash")
ADD_ENUM_MAPPING(Point::BOX, "Border", "box")
ADD_ENUM_MAPPING(Point::BOXED_X, "Bordered X", "boxX")
ADD_ENUM_MAPPING(Point::BOXED_CROSS_HAIR, "Bordered +", "boxCrosshair")
ADD_ENUM_MAPPING(Point::BOXED_ASTERISK, "Bordered *", "boxAsterisk")
ADD_ENUM_MAPPING(Point::BOXED_HORIZONTAL_LINE, "Bordered -", "boxHorizontalLine")
ADD_ENUM_MAPPING(Point::BOXED_VERTICAL_LINE, "Bordered |", "boxVerticalLine")
ADD_ENUM_MAPPING(Point::BOXED_FORWARD_SLASH, "Bordered /", "boxForwardSlash")
ADD_ENUM_MAPPING(Point::BOXED_BACK_SLASH, "Bordered \\", "boxBackSlash")
ADD_ENUM_MAPPING(Point::DIAMOND, "Diamond", "diamond")
ADD_ENUM_MAPPING(Point::DIAMOND_FILLED, "Filled Diamond", "filledDiamond")
ADD_ENUM_MAPPING(Point::DIAMOND_CROSS_HAIR, "Diamond +", "diamondCrosshair")
ADD_ENUM_MAPPING(Point::TRIANGLE, "Triangle", "triangle")
ADD_ENUM_MAPPING(Point::TRIANGLE_FILLED, "Filled Triangle", "filledTriangle")
ADD_ENUM_MAPPING(Point::RIGHT_TRIANGLE, "Right Triangle", "rightTriangle")
ADD_ENUM_MAPPING(Point::RIGHT_TRIANGLE_FILLED, "Filled Right Triangle", "filledRightTriangle")
ADD_ENUM_MAPPING(Point::LEFT_TRIANGLE, "Left Triangle", "leftTriangle")
ADD_ENUM_MAPPING(Point::LEFT_TRIANGLE_FILLED, "Filled Left Triangle", "filledLeftTriangle")
ADD_ENUM_MAPPING(Point::DOWN_TRIANGLE, "Down Triangle", "downTriangle")
ADD_ENUM_MAPPING(Point::DOWN_TRIANGLE_FILLED, "Filled Down Triangle", "filledDownTriangle")
ADD_ENUM_MAPPING(Point::CIRCLE, "Circle", "circle")
ADD_ENUM_MAPPING(Point::CIRCLE_FILLED, "Filled Circle", "filledCircle")
ADD_ENUM_MAPPING(Point::OCTAGON, "Octagon", "octagon")
ADD_ENUM_MAPPING(Point::OCTAGON_FILLED, "Filled Octagon", "filledOctagon")
ADD_ENUM_MAPPING(Point::OCTAGON_CROSS_HAIR, "Octagon +", "octagonCrosshair")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(PositionType)
ADD_ENUM_MAPPING(TOP_LEFT_BOTTOM_LEFT, "Top Left/Bottom Left", "TopLeftBottomLeft")
ADD_ENUM_MAPPING(TOP_LEFT_BOTTOM_RIGHT, "Top Left/Bottom Right", "TopLeftBottomRight")
ADD_ENUM_MAPPING(CENTER, "Top Center/Bottom Center", "Center")
ADD_ENUM_MAPPING(TOP_RIGHT_BOTTOM_LEFT, "Top Right/Bottom Left", "TopRightBottomLeft")
ADD_ENUM_MAPPING(TOP_RIGHT_BOTTOM_RIGHT, "Top Right/Bottom Right", "TopRightBottomRight")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ProcessingLocation)
ADD_ENUM_MAPPING(IN_MEMORY, "In Memory", "inMemory")
ADD_ENUM_MAPPING(ON_DISK_READ_ONLY, "On Disk (Read-Only)", "onDiskReadOnly")
ADD_ENUM_MAPPING(ON_DISK, "On Disk", "onDisk")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(RasterChannelType)
ADD_ENUM_MAPPING(GRAY, "Gray", "gray")
ADD_ENUM_MAPPING(RED, "Red", "red")
ADD_ENUM_MAPPING(GREEN, "Green", "green")
ADD_ENUM_MAPPING(BLUE, "Blue", "blue")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(RegionUnits)
ADD_ENUM_MAPPING(RAW_VALUE, "Raw Value", "Raw")
ADD_ENUM_MAPPING(PERCENTAGE, "Percentage", "Percentage")
ADD_ENUM_MAPPING(PERCENTILE, "Percentile", "Percentile")
ADD_ENUM_MAPPING(STD_DEV, "Standard Deviation", "StdDev")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ScaleType)
ADD_ENUM_MAPPING(SCALE_LINEAR, "Linear", "linear")
ADD_ENUM_MAPPING(SCALE_LOG, "Logarithmic", "logarithmic")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(SessionSaveType)
ADD_ENUM_MAPPING(SESSION_AUTO_SAVE, "Auto", "Auto")
ADD_ENUM_MAPPING(SESSION_DONT_AUTO_SAVE, "Don't Save", "DontSave")
ADD_ENUM_MAPPING(SESSION_QUERY_SAVE, "Query", "Query")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(StretchType)
ADD_ENUM_MAPPING(LINEAR, "Linear", "Linear")
ADD_ENUM_MAPPING(LOGARITHMIC, "Logarithmic", "Logarithmic")
ADD_ENUM_MAPPING(EXPONENTIAL, "Exponential", "Exponential")
ADD_ENUM_MAPPING(EQUALIZATION, "Histogram Equalization", "Equalization")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(SymbolType)
ADD_ENUM_MAPPING(SOLID, "Solid", "solid")
ADD_ENUM_MAPPING(X, "X", "x")
ADD_ENUM_MAPPING(CROSS_HAIR, "+", "crosshair")
ADD_ENUM_MAPPING(ASTERISK, "*", "asterisk")
ADD_ENUM_MAPPING(HORIZONTAL_LINE, "-", "horizontalLine")
ADD_ENUM_MAPPING(VERTICAL_LINE, "|", "verticalLine")
ADD_ENUM_MAPPING(FORWARD_SLASH, "/", "forwardSlash")
ADD_ENUM_MAPPING(BACK_SLASH, "\\", "backSlash")
ADD_ENUM_MAPPING(BOX, "Border", "box")
ADD_ENUM_MAPPING(BOXED_X, "Bordered X", "boxX")
ADD_ENUM_MAPPING(BOXED_CROSS_HAIR, "Bordered +", "boxCrosshair")
ADD_ENUM_MAPPING(BOXED_ASTERISK, "Bordered *", "boxAsterisk")
ADD_ENUM_MAPPING(BOXED_HORIZONTAL_LINE, "Bordered -", "boxHorizontalLine")
ADD_ENUM_MAPPING(BOXED_VERTICAL_LINE, "Bordered |", "boxVerticalLine")
ADD_ENUM_MAPPING(BOXED_FORWARD_SLASH, "Bordered /", "boxForwardSlash")
ADD_ENUM_MAPPING(BOXED_BACK_SLASH, "Bordered \\", "boxBackSlash")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(TextureMode)
ADD_ENUM_MAPPING(TEXTURE_LINEAR, "Linear", "Linear")
ADD_ENUM_MAPPING(TEXTURE_NEAREST_NEIGHBOR, "NearestNeighbor", "NearestNeighbor")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(UnitSystem)
ADD_ENUM_MAPPING(UNIT_KM, "meters/kilometers", "m-km")
ADD_ENUM_MAPPING(UNIT_KFT, "feet/kilofeet", "f-kft")
ADD_ENUM_MAPPING(UNIT_MI, "feet/miles", "f-mi")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(UnitType)
ADD_ENUM_MAPPING(RADIANCE, "Radiance", "Radiance")
ADD_ENUM_MAPPING(REFLECTANCE, "Reflectance", "Reflectance")
ADD_ENUM_MAPPING(EMISSIVITY, "Emissivity", "Emissivity")
ADD_ENUM_MAPPING(DIGITAL_NO, "Digital Number", "Digital Number")
ADD_ENUM_MAPPING(CUSTOM_UNIT, "Custom", "Custom")
ADD_ENUM_MAPPING(REFLECTANCE_FACTOR, "Reflectance Factor", "Reflectance Factor")
ADD_ENUM_MAPPING(TRANSMITTANCE, "Transmittance", "Transmittance")
ADD_ENUM_MAPPING(ABSORPTANCE, "Absorptance", "Absorptance")
ADD_ENUM_MAPPING(ABSORBANCE, "Absorbance", "Absorbance")
ADD_ENUM_MAPPING(DISTANCE, "Distance", "Distance")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(ViewType)
ADD_ENUM_MAPPING(SPATIAL_DATA_VIEW, "Spatial Data View", "Spatial Data View")
ADD_ENUM_MAPPING(PRODUCT_VIEW, "Product View", "Product View")
ADD_ENUM_MAPPING(PLOT_VIEW, "Plot View", "Plot View")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(WavelengthUnitsType)
ADD_ENUM_MAPPING(MICRONS, "Microns", "Microns")
ADD_ENUM_MAPPING(NANOMETERS, "Nanometers", "Nanometers")
ADD_ENUM_MAPPING(INVERSE_CENTIMETERS, "Inverse Centimeters", "InverseCentimeters")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(WindowSizeType)
ADD_ENUM_MAPPING(FIXED_SIZE, "Fixed", "Fixed")
ADD_ENUM_MAPPING(MAXIMIZED, "Maximized", "Maximized")
ADD_ENUM_MAPPING(WORKSPACE_PERCENTAGE, "Percentage", "Percentage")
END_ENUM_MAPPING()

BEGIN_ENUM_MAPPING(WindowType)
ADD_ENUM_MAPPING(WORKSPACE_WINDOW, "Workspace Window", "WorkspaceWindow")
ADD_ENUM_MAPPING(SPATIAL_DATA_WINDOW, "Spatial Data Window", "SpatialDataWindow")
ADD_ENUM_MAPPING(PRODUCT_WINDOW, "Product Window", "ProductWindow")
ADD_ENUM_MAPPING(DOCK_WINDOW, "Dock Window", "DockWindow")
ADD_ENUM_MAPPING(PLOT_WINDOW, "Plot Window", "PlotWindow")
ADD_ENUM_MAPPING(TOOLBAR, "Toolbar", "ToolbarWindow")
END_ENUM_MAPPING()

ENUM_MAPPING_PRE_DEFINITIONS(ReleaseType, ReleaseType)
ENUM_MAPPING_TO_DISPLAY_STRING_VEC(ReleaseType)
ENUM_MAPPING_TO_XML_STRING(ReleaseType, ReleaseType)
ENUM_MAPPING_TO_XML_STRING_VEC(ReleaseType) \
ENUM_MAPPING_FROM_DISPLAY_STRING_VEC(ReleaseType)
ENUM_MAPPING_FROM_XML_STRING(ReleaseType, ReleaseType)
ENUM_MAPPING_FROM_XML_STRING_VEC(ReleaseType)
ENUM_MAPPING_FUNCTION(ReleaseType)
ADD_ENUM_MAPPING(RT_NORMAL, "", "normal")
ADD_ENUM_MAPPING(RT_DEMO, "", "demo")
ADD_ENUM_MAPPING(RT_TRAINING, "", "training")
ADD_ENUM_MAPPING(RT_TEST, "", "testing")
ADD_ENUM_MAPPING(RT_PROTO, "", "prototype")
ADD_ENUM_MAPPING(RT_RD, "", "rd")
END_ENUM_MAPPING()

template<>
string toDisplayString(const ReleaseType& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   switch(val)
   {
   case RT_NORMAL:
      {
         Service<ConfigurationSettings> pConfigSettings;
         if (!pConfigSettings->isProductionRelease())
         {
            return "Developer Release";
         }
         return "";
      }
   case RT_DEMO:     return "Demonstration Mode";
   case RT_TRAINING: return "Training Mode";
   case RT_TEST: return "Testing Mode";
   case RT_PROTO: return "Prototype Product";
   case RT_RD: return "R&D Mode";
   default: break;
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return "";
}

template<>
ReleaseType fromDisplayString<ReleaseType>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if ((value == "Developer Release") || value.empty())
   {
      return RT_NORMAL;
   }
   else if (value == "Demonstration Mode")
   {
      return RT_DEMO;
   }
   else if (value == "Training Mode")
   {
      return RT_TRAINING;
   }
   else if (value == "Testing Mode")
   {
      return RT_TEST;
   }
   else if (value == "Prototype Product")
   {
      return RT_PROTO;
   }
   else if (value == "R&D Mode")
   {
      return RT_RD;
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return ReleaseType();
}

template<>
ColorType fromDisplayString<ColorType>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   //Format should be #aarrggbb
   //where aa - is the alpha in hex format, rr - is the red in hex format, gg - is the green
   //in hex format and bb - is the blue in hex format.
   if (value == "InvalidColor")
   {
      return ColorType();
   }

   if ((value.size() == 9) || (value.size() == 7))
   {
      if (value.substr(0,1) == "#")
      {
         string alphaStr;
         string redStr;
         string greenStr;
         string blueStr;
         if (value.size() == 7)
         {
            redStr = value.substr(1, 2);
            greenStr = value.substr(3, 2);
            blueStr = value.substr(5, 2);
         }
         else
         {
            alphaStr = value.substr(1, 2);
            redStr = value.substr(3, 2);
            greenStr = value.substr(5, 2);
            blueStr = value.substr(7, 2);
         }
         unsigned int alpha = 255;
         unsigned int red = 0;
         unsigned int green = 0;
         unsigned int blue = 0;
         bool parseSuccess = true;
         if (!alphaStr.empty())
         {
            istringstream alphaParser;
            alphaParser.str(alphaStr);
            alphaParser >> hex >> alpha;
            parseSuccess = (!alphaParser.fail());
         }
         if (parseSuccess)
         {
            istringstream redParser;
            redParser.str(redStr);
            redParser >> hex >> red;
            if (!redParser.fail())
            {
               istringstream greenParser;
               greenParser.str(greenStr);
               greenParser >> hex >> green;
               if (!greenParser.fail())
               {
                  istringstream blueParser;     
                  blueParser.str(blueStr);
                  blueParser >> hex >> blue;
                  if (!blueParser.fail())
                  {
                     return ColorType(red, green, blue, alpha);
                  }
               }
            }
         }
      }
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return ColorType();
}

template<>
ColorType fromXmlString<ColorType>(string value, bool* pError)
{
   return fromDisplayString<ColorType>(value, pError);
}

template<>
string toDisplayString(const ColorType& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if (val.isValid())
   {
      ostringstream formatter;
      formatter << "#" << hex;
      formatter << setw(2) << setfill('0') << val.mAlpha;
      formatter << setw(2) << setfill('0') << val.mRed;
      formatter << setw(2) << setfill('0') << val.mGreen;
      formatter << setw(2) << setfill('0') << val.mBlue;
      return formatter.str();
   }
   else
   {
      return "InvalidColor";
   }
}

template<>
string toXmlString(const ColorType& val, bool* pError)
{
   return toDisplayString(val, pError);
}

template<>
string toDisplayString(const LocationType& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   ostringstream formatter;
   formatter << setprecision(numeric_limits<double>::digits10) << "(" << val.mX << "," << val.mY << ")";
   return formatter.str();
}

template<>
string toXmlString(const LocationType& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   ostringstream formatter;
   formatter << setprecision(numeric_limits<double>::digits10) << val.mX << " " << val.mY;
   return formatter.str();
}

template<>
LocationType fromDisplayString<LocationType>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   LocationType parsedValue;
   istringstream parser;
   parser.str(value);
   if (parser.get() == '(')
   {
      parser >> parsedValue.mX;
      if (parser.get() == ',')
      {
         parser >> parsedValue.mY;
         if (parser.get() == ')')
         {
            return parsedValue;
         }
      }
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return LocationType();
}

template<>
LocationType fromXmlString<LocationType>(string value, bool* pError)
{
   LocationType parsedValue;
   if (pError != NULL)
   {
      *pError = false;
   }
   stringstream parser(value);
   parser >> parsedValue.mX;
   if (!parser.fail())
   {
      parser >> parsedValue.mY;
      if (!parser.fail())
      {
         return parsedValue;
      }
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return LocationType();
}

template<>
string toDisplayString(const string& val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   return val;
}

template<>
string toDisplayString(const vector<string>& vec, bool* pError)
{
   return convertVectorToString(vec, pError, ", ", true);
}

template<>
string toXmlString(const string& val, bool* pError)
{
   return toDisplayString(val, pError);
}

template<>
string toXmlString(const vector<string>& vec, bool* pError)
{
   return convertVectorToString(vec, pError, ", ", false);
}

template<>
string fromDisplayString<string>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   return value;
}

template<>
vector<string> fromDisplayString<vector<string> >(string value, bool* pError)
{
   return convertStringToVector<string>(value, pError, ", ", true);
}

template<>
string fromXmlString<string>(string value, bool* pError)
{
   return fromDisplayString<string>(value, pError);
}

template<>
vector<string> fromXmlString<vector<string> >(string value, bool* pError)
{
   return convertStringToVector<string>(value, pError, ", ", false);
}

template<>
string toDisplayString<const Filename*>(const Filename* const & val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if (val != NULL)
   {
      return val->getFullPathAndName();
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return "";
}

template<>
string toDisplayString(const vector<Filename*> & vec, bool* pError)
{
   return convertVectorToString(vec, pError, ", ", true);
}

TO_DISPLAY_POINTER_VARIATIONS(Filename)

template<>
string toXmlString<const Filename*>(const Filename* const & val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if (val != NULL)
   {
      return XmlBase::PathToURL(val->getFullPathAndName());
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return "";
}

template<>
string toXmlString(const vector<Filename*>& vec, bool* pError)
{
   return convertVectorToString(vec, pError, ", ", false);
}

TO_XML_POINTER_VARIATIONS(Filename)

template<>
Filename* fromDisplayString<Filename*>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(value);
   return pFilename.release();
}

template<>
vector<Filename*> fromDisplayString<vector<Filename*> >(string value, bool* pError)
{
   return convertStringToVector<Filename*>(value, pError, ", ", true);
}

template<>
Filename* fromXmlString<Filename*>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(value);
   return pFilename.release();
}

template<>
vector<Filename*> fromXmlString<vector<Filename*> >(string value, bool* pError)
{
   return convertStringToVector<Filename*>(value, pError, ", ", false);
}

template<>
string toDisplayString<const DateTime*>(const DateTime* const & val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if (val != NULL)
   {
      if (val->isValid())
      {
         string format;
         if (val->isTimeValid())
         {
#if defined(WIN_API)
            format = "%B %#d, %#Y, %H:%M:%S";
#else
            format = "%B %d, %Y, %H:%M:%S";
#endif
         }
         else
         {
#if defined(WIN_API)
            format = "%B %#d, %#Y";
#else
            format = "%B %d, %Y";
#endif
         }
         return val->getFormattedUtc(format);
      }
      else
      {
         return "Invalid DateTime";
      }
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return "";
}

TO_DISPLAY_POINTER_VARIATIONS(DateTime)

template<>
string toXmlString<const DateTime*>(const DateTime* const & val, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if (val != NULL)
   {
      if (val->isValid())
      {

         string format;
         if (val->isTimeValid())
         {
            format = "%Y-%m-%dT%H:%M:%SZ";
         }
         else
         {
            format = "%Y-%m-%d";
         }
         return val->getFormattedUtc(format);
      }
      else
      {
         return "InvalidDateTime";
      }
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return "";
}

TO_XML_POINTER_VARIATIONS(DateTime)

template<>
DateTime* fromDisplayString<DateTime*>(string dateText, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   static const char* const monthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

   FactoryResource<DateTime> pDateTime;
   if (dateText == "Invalid DateTime")
   {
      return pDateTime.release();
   }
   char month[128];
   int iMonth = 0;
   int iDay = 0;
   int iYear = 0;
   int iHour = 0;
   int iMinute = 0;
   int iSecond = 0;

   int iValues = 0;
   iValues = sscanf(dateText.c_str(), "%s %d, %d, %d:%d:%d", &month, &iDay, &iYear,
      &iHour, &iMinute, &iSecond);
   if (iValues > 0)
   {
      string monthText = month;
      for (int i = 0; i < 12; i++)
      {
         if (monthText.substr(0, 3) == monthNames[i])
         {
            iMonth = i + 1;
            break;
         }
      }
   }
   bool dateTimeSet = false;
   if (iValues > 3)
   {
      dateTimeSet = pDateTime->set(iYear, iMonth, iDay, iHour, iMinute, iSecond);
      dateTimeSet = dateTimeSet && pDateTime->isTimeValid();
   }
   else
   {
      dateTimeSet = pDateTime->set(iYear, iMonth, iDay);
      dateTimeSet = dateTimeSet && !(pDateTime->isTimeValid());
   }
   if (dateTimeSet)
   {
      return pDateTime.release();
   }
   if (pError != NULL)
   {
      *pError = true;
   }
   return NULL;
}

template<>
DateTime* fromXmlString<DateTime*>(string value, bool* pError)
{
   FactoryResource<DateTime> pDateTime;
   if (pError != NULL)
   {
      *pError = false;
   }
   if (value == "InvalidDateTime")
   {
      return pDateTime.release();
   }
   bool dateTimeSet = pDateTime->set(value);
   if (dateTimeSet && pDateTime->isValid())
   {
      return pDateTime.release();
   }
   else
   {
      if (pError != NULL)
      {
         *pError = true;
      }
      return NULL;
   }
}

template<>
string toDisplayString(const Int64& val, bool* pError)
{
   return toDisplayString<int64_t>(val.get(), pError);
}

template<>
string toXmlString(const Int64& val, bool* pError)
{
   return toXmlString<int64_t>(val.get(), pError);
}

template<>
Int64 fromDisplayString<Int64>(string value, bool* pError)
{
   return Int64(fromDisplayString<int64_t>(value, pError));
}

template<>
Int64 fromXmlString<Int64>(string value, bool* pError)
{
   return Int64(fromXmlString<int64_t>(value, pError));
}

namespace Int64Functors
{
   class CreateInt64Object
   {
   public:
      Int64 operator() (const int64_t& value)
      {
         return Int64(value);
      }
   };
}

namespace UInt64Functors
{
   class CreateUInt64Object
   {
   public:
      UInt64 operator() (const uint64_t& value)
      {
         return UInt64(value);
      }
   };

};

template<>
string toDisplayString(const vector<Int64>& val, bool* pError)
{
   vector<int64_t> tempValues(val.size());
   copy(val.begin(), val.end(), tempValues.begin());
   return toDisplayString<vector<int64_t> >(tempValues, pError);
}

template<>
string toXmlString(const vector<Int64>& val, bool* pError)
{
   vector<int64_t> tempValues(val.size());
   copy(val.begin(), val.end(), tempValues.begin());
   return toXmlString<vector<int64_t> >(tempValues, pError);
}

template<>
vector<Int64> fromDisplayString<vector<Int64> >(string value, bool* pError)
{
   vector<int64_t> tempValues = fromDisplayString<vector<int64_t> >(value, pError);
   vector<Int64> retValues;
   retValues.reserve(tempValues.size());
   transform(tempValues.begin(), tempValues.end(), back_inserter(retValues), Int64Functors::CreateInt64Object());
   return retValues;
}

template<>
vector<Int64> fromXmlString<vector<Int64> >(string value, bool* pError)
{
   vector<int64_t> tempValues = fromXmlString<vector<int64_t> >(value, pError);
   vector<Int64> retValues;
   retValues.reserve(tempValues.size());
   transform(tempValues.begin(), tempValues.end(), back_inserter(retValues), Int64Functors::CreateInt64Object());
   return retValues;
}

template<>
string toDisplayString(const UInt64& val, bool* pError)
{
   return toDisplayString<uint64_t>(val.get(), pError);
}

template<>
string toXmlString(const UInt64& val, bool* pError)
{
   return toXmlString<uint64_t>(val.get(), pError);
}

template<>
UInt64 fromDisplayString<UInt64>(string value, bool* pError)
{
   return UInt64(fromDisplayString<uint64_t>(value, pError));
}

template<>
UInt64 fromXmlString<UInt64>(string value, bool* pError)
{
   return UInt64(fromXmlString<uint64_t>(value, pError));
}

template<>
string toDisplayString(const vector<UInt64>& val, bool* pError)
{
   vector<uint64_t> tempValues(val.size());
   copy(val.begin(), val.end(), tempValues.begin());
   return toDisplayString<vector<uint64_t> >(tempValues, pError);
}

template<>
string toXmlString(const vector<UInt64>& val, bool* pError)
{
   vector<uint64_t> tempValues(val.size());
   copy(val.begin(), val.end(), tempValues.begin());
   return toXmlString<vector<uint64_t> >(tempValues, pError);
}

template<>
vector<UInt64> fromDisplayString<vector<UInt64> >(string value, bool* pError)
{
   vector<uint64_t> tempValues = fromDisplayString<vector<uint64_t> >(value, pError);
   vector<UInt64> retValues;
   retValues.reserve(tempValues.size());
   transform(tempValues.begin(), tempValues.end(), back_inserter(retValues), UInt64Functors::CreateUInt64Object());
   return retValues;
}

template<>
vector<UInt64> fromXmlString<vector<UInt64> >(string value, bool* pError)
{
   vector<uint64_t> tempValues = fromXmlString<vector<uint64_t> >(value, pError);
   vector<UInt64> retValues;
   retValues.reserve(tempValues.size());
   transform(tempValues.begin(), tempValues.end(), back_inserter(retValues), UInt64Functors::CreateUInt64Object());
   return retValues;
}
}
