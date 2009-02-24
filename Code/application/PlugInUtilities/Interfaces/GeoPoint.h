/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOPOINT_H
#define GEOPOINT_H

#include "LocationType.h"
#include "EnumWrapper.h"
#include "TypesFile.h"

#include <string>

/**
 *  Adds text formatting for a latitude or longitude value.
 *
 *  This class provides common formatting to a single latutude or longitude value.
 *  When the data value is set, it can be retrieved either in the raw data value
 *  format, or in one of several text formats, as defined by the DmsType enum.
 *
 *  @see        LatLonPoint
 */
class DmsPoint
{
public:
   /**
    *  Specifies the text formatting type for the data value.
    *
    *  @see     DmsPoint::getValueText()
    */
   enum DmsTypeEnum
   {
      DMS_DECIMAL,      /**< The value is interpreted as neither latitude or longitude,
                             and a negative sign or the absence of one indicates the
                             hemisphere in the text. */
      DMS_LATITUDE,     /**< The value is interpreted as latitude, and the hemisphere
                             is designated by 'N' or 'S' in the text. */
      DMS_LONGITUDE     /**< The value is interpreted as longitude, and the hemisphere
                             is designated by 'E' or 'W' in the text. */
   };

   /**
    * @EnumWrapper DmsPoint::DmsTypeEnum.
    */
   typedef EnumWrapper<DmsTypeEnum> DmsType;

   /**
    *  Creates a DMS point with an initial raw value.
    *
    *  @param   eType
    *           The DMS point type.
    *  @param   dValue
    *           The initial value for the DMS point.
    *
    *  @see     DmsType
    */
   DmsPoint(DmsType eType, double dValue = 0.0);

   /**
    *  Creates a DMS point with an initial string value.
    *
    *  @param   eType
    *           The DMS point type.
    *  @param   valueText
    *           A string representing the initial value that can be converted to a
    *           numeric raw value.
    */
   DmsPoint(DmsType eType, const std::string& valueText);

   /**
    *  Destroys the DMS point.
    */
   ~DmsPoint();

   /**
    *  Returns the DMS point type.
    *
    *  @return  The DMS type of the DMS point.
    *
    *  @see     DmsType
    */
   DmsType getType() const;

   /**
    *  Sets the value as a raw value.
    *
    *  @param   dValue
    *           The new value for the DMS point.
    */
   void setValue(double dValue);

   /**
    *  Sets the value as a string.
    *
    *  @param   valueText
    *           A string representing the new value that can be converted to a numeric
    *           raw value.
    */
   void setValue(const std::string& valueText);

   /**
    *  Returns the raw value of the DMS point.
    *
    *  @return  The DMS point raw value.
    *
    *  @see     getValueText()
    */
   double getValue() const;

   /**
    *  Returns the value of the DMS point in a given text format.
    *
    *  @param   format
    *           The text format for the string.
    *  @param   precision
    *           The number of digits after decimal point. Defaults to three for
    *           decimal degrees and seconds and two for decimal minutes.
    *
    *  @return  The value of the DMS point as an formatted string.
    *
    *  @see     DmsFormatType, getValue()
    */
   std::string getValueText(DmsFormatType format = DMS_FULL, int precision = -1) const;

   /**
    *  Sets the type and value to that of another DMS point.
    *
    *  @param   original
    *           The point from which to set this point's type and value.
    *
    *  @return  A reference to this DMS point, whose type and value have changed.
    */
   DmsPoint& operator =(const DmsPoint& original);

   /**
    *  Compares two DMS points.
    *
    *  @param   rhs
    *           The point to compare the type and value against this point.
    *
    *  @return  Returns true if the type and value of the given point are identical
    *           to the type and value of this point; otherwise returns false.
    */
   bool operator ==(const DmsPoint& rhs) const;

private:
   DmsType mType;
   double mValue;
};

/**
 *  Stores a latitude/longitude coordinate pair.
 *
 *  This class stores a latitude/longitude pair as DmsPoint objects to provide
 *  common text formatting when retrieving the value.  In addition to the text
 *  formatting, the raw value can also be retrieved.
 *
 *  @see        DmsPoint
 */
class LatLonPoint
{
public:
   /**
    *  Creates a latitude/longitude point with an initial value of 0° N and 0° E.
    */
   LatLonPoint();

   /**
    *  Creates a latitude/longitude point with initial raw values.
    *
    *  @param   latLon
    *           The initial latitude and longitude coordinate.  The latitude value
    *           must correspond to the x-coordinate of the LocationType, and the
    *           longitude value must correspond to the y-coordinate.
    */
   LatLonPoint(LocationType latLon);

   /**
    *  Creates a latitude/longitude point with a initial string values for both
    *  latitude and longitude.
    *
    *  @param   latitudeText
    *           A string representing the initial latitude value that can be
    *           converted to a numeric raw value.
    *  @param   longitudeText
    *           A string representing the initial longitude value that can be
    *           converted to a numeric raw value.
    */
   LatLonPoint(const std::string& latitudeText, const std::string& longitudeText);

   /**
    *  Creates a latitude/longitude point with an initial string value.
    *
    *  @param   latLonText
    *           A string representing the initial latitude and longitude value that
    *           can be converted to numeric raw values.
    */
   LatLonPoint(const std::string& latLonText);

   /**
    *  Destroys the latitude/longitude point.
    */
   ~LatLonPoint();

   /**
    *  Returns the raw value of the latitude/longitude point.
    *
    *  @return  The latitude/longitude coordinate.  The latitude value is stored in
    *           the LocationType x-coordinate, and the longitude value is stored in
    *           the y-coordinate.
    *
    *  @see     getLatitude(), getLongitude(), getText()
    */
   LocationType getCoordinates() const;

   /**
    *  Retrieves the latitude value.
    *
    *  @return  A reference to the DMS point containing the latitude value.
    *
    *  @see     DmsPoint
    */
   const DmsPoint& getLatitude() const;

   /**
    *  Retrieves the longitude value.
    *
    *  @return  A reference to the DMS point containing the longitude value.
    *
    *  @see     DmsPoint
    */
   const DmsPoint& getLongitude() const;

   /**
    *  Returns the latitude/longitude coordinate in a given text format.
    *
    *  @param   format
    *           The text format for the string.
    *  @param   precision
    *           The number of digits after decimal point. Defaults to three for
    *           decimal degrees and seconds and two for decimal minutes.
    *
    *  @return  The value of the latitude/longitude coordinate as a formatted string.
    *
    *  @see     DmsFormatType, getCoordinates(), getLatitudeText(), getLongitudeText()
    */
   std::string getText(DmsFormatType format = DMS_FULL, int precision = -1) const;

   /**
    *  Returns the latitude value in a given text format.
    *
    *  @param   format
    *           The text format for the string.
    *  @param   precision
    *           The number of digits after decimal point. Defaults to three for
    *           decimal degrees and seconds and two for decimal minutes.
    *
    *  @return  The latitude value as a formatted string.
    *
    *  @see     DmsFormatType, getText(), getLongitudeText()
    */
   std::string getLatitudeText(DmsFormatType format = DMS_FULL, int precision = -1) const;

   /**
    *  Returns the longitude value in a given text format.
    *
    *  @param   format
    *           The text format for the string.
    *  @param   precision
    *           The number of digits after decimal point. Defaults to three for
    *           decimal degrees and seconds and two for decimal minutes.
    *
    *  @return  The longitude value as a formatted string.
    *
    *  @see     DmsFormatType, getText(), getLatitudeText()
    */
   std::string getLongitudeText(DmsFormatType format = DMS_FULL, int precision = -1) const;

   /**
    *  Sets the latitude and longitude values to that of another latitude/longitude
    *  point.
    *
    *  @param   original
    *           The point from which to set this point's values.
    *
    *  @return  A reference to this latitude/longitude point, whose values have
    *           changed.
    */
   LatLonPoint& operator =(const LatLonPoint& original);

   /**
    *  Compares two latitude/longitude points.
    *
    *  @param   rhs
    *           The latitude/longitude point to compare the values against this
    *           point.
    *
    *  @return  Returns true if the values of the given point are identical to the
    *           values of this point; otherwise returns false.
    */
   bool operator ==(const LatLonPoint& rhs) const;

private:
   DmsPoint mLatitude;
   DmsPoint mLongitude;
};

/**
 *  Stores a Universal Transverse Mercator (UTM) coordinate.
 *
 *  This class stores a UTM coordinate and provides common text formatting when
 *  retrieving the value.  In addition to the text formatting, the raw easting,
 *  northing, zone, and hemisphere values can also be retrieved.
 */
class UtmPoint
{
public:
   /**
    *  Creates a UTM point based on a given latitude/longitude point.
    *
    *  This constructor automatically converts a latitude/longitude coordinate
    *  pair into a UTM coordinate.
    * 
    *  @param   latLon
    *           The latitude/longitude coordinate to convert to a UTM point.
    */
   UtmPoint(LatLonPoint latLon);

   /**
    *  Creates a UTM point based on initial easting, northing, zone, and
    *  hemisphere values.
    *
    *  @param   dEasting
    *           The easting value.
    *  @param   dNorthing
    *           The northing value.
    *  @param   iZone
    *           The integer zone value.
    *  @param   hemisphere
    *           The hemisphere, as a character, which should be either 'N'
    *           or 'S'.
    */
   UtmPoint(double dEasting, double dNorthing, int iZone, char hemisphere);

   /**
    *  Destroys the UTM point.
    */
   ~UtmPoint();

   /**
    *  Retrieves the easting and northing values.
    *
    *  @return  The easting and northing values in a LocationType.  The easting
    *           value is stored in the LocationType x-coordinate, and the
    *           northing value is stored in the y-coordinate.
    *
    *  @see     getEasting(), getNorthing(), getText()
    */
   LocationType getCoordinates() const;

   /**
    *  Converts the UTM point to a latitude/longitude point.
    *
    *  @return  The UTM point converted to a latitude/longitude coordinate.  The
    *           latitude value is stored in the LocationType x-coordinate, and
    *           the longitude value is stored in the y-coordinate.
    *
    *  @see     LatLonPoint, getCoordinates()
    */
   LatLonPoint getLatLonCoordinates() const;

   /**
    *  Returns the easting value.
    *
    *  @return  The easting value of the UTM point.
    *
    *  @see     getNorthing()
    */
   double getEasting() const;

   /**
    *  Returns the northing value.
    *
    *  @return  The northing value of the UTM point.
    *
    *  @see     getEasting()
    */
   double getNorthing() const;

   /**
    *  Returns the zone value.
    *
    *  @return  The zone value of the UTM point as an integer.
    */
   int getZone() const;

   /**
    *  Returns the hemisphere value.
    *
    *  @return  The hemisphere value of the UTM point as a character.  Will be
    *           either 'N' or 'S'.
    */
   const char getHemisphere() const;

   /**
    *  Returns UTM point in a formatted string.
    *
    *  @return  A string representing the UTM point in the following format:
    *           easting text + " " + zone text + ", " + northing text + " " +
    *           hemisphere.
    *
    *  @see     getEastingText(), getNorthingText(), getZoneText(), getHemisphere()
    */
   std::string getText() const;

   /**
    *  Returns the easting value as a formatted string.
    *
    *  @return  A string representing the easting value of the UTM point in the
    *           following format used by sprintf(): "E%.0f".
    *
    *  @see     getText()
    */
   std::string getEastingText() const;

   /**
    *  Returns the northing value as a formatted string.
    *
    *  @return  A string representing the northing value of the UTM point in the
    *           following format used by sprintf(): "N%.0f".
    *
    *  @see     getText()
    */
   std::string getNorthingText() const;

   /**
    *  Returns the zone value as a formatted string.
    *
    *  @return  A string representing the zone value of the UTM point in the
    *           following format used by sprintf(): "Zone %d".
    *
    *  @see     getText()
    */
   std::string getZoneText() const;

private:
   double mEasting;
   double mNorthing;
   int mZone;
   char mHemisphere;
};

/**
 *  Stores a Military Grid Reference System (MGRS) coordinate.
 *
 *  This class stores an MGRS coordinate and provides common text formatting when
 *  retrieving the value.  In addition to the text formatting, the raw component
 *  values can also be retrieved.
 */
class MgrsPoint
{
public:
   /**
    *  Creates an MGRS point based on a given latitude/longitude point.
    *
    *  This constructor automatically converts a latitude/longitude coordinate
    *  pair into an MGRS coordinate.
    * 
    *  @param   latLon
    *           The latitude/longitude coordinate to convert to an MGRS point.
    */
   MgrsPoint(LatLonPoint latLon);

   /**
    *  Creates an MGRS point based on its string representation.
    *
    *  @param   mgrsText
    *           A string representation of the MGRS point.
    */
   MgrsPoint(const std::string& mgrsText);

   /**
    *  Destroys the MGRS point.
    */
   ~MgrsPoint();

   /**
    *  Retrieves the easting and northing components of the MGRS point.
    *
    *  @return  The easting and northing component values in a LocationType.  The
    *           easting value is stored in the LocationType x-coordinate, and the
    *           northing value is stored in the y-coordinate.
    *
    *  @see     getEasting(), getNorthing(), getText()
    */
   LocationType getCoordinates() const;

   /**
    *  Converts the MGRS point to a latitude/longitude point.
    *
    *  @return  The MGRS point converted to a latitude/longitude coordinate.  The
    *           latitude value is stored in the LocationType x-coordinate, and
    *           the longitude value is stored in the y-coordinate.
    *
    *  @see     LatLonPoint, getCoordinates()
    */
   LatLonPoint getLatLonCoordinates() const;

   /**
    *  Returns the easting component value.
    *
    *  @return  The easting value of the MGRS point.
    *
    *  @see     getNorthing()
    */
   double getEasting() const;

   /**
    *  Returns the northing component value.
    *
    *  @return  The northing value of the MGRS point.
    *
    *  @see     getEasting()
    */
   double getNorthing() const;

   /**
    *  Returns the zone component value.
    *
    *  @return  The zone value of the MGRS point as an integer.
    */
   int getZone() const;

   /**
    *  Returns the text representation of the MGRS point.
    *
    *  @return  A formatted string representing the MGRS point, including the
    *           easting, northing, zone, and SCR code.
    */
   std::string getText() const;

   /**
    *  Returns the zone component value as a formatted string.
    *
    *  @return  A string representing the zone value of the MGRS point in the
    *           format used in the string representation of the entire coordinate.
    *
    *  @see     getText()
    */
   std::string getZoneText() const;

   /**
    *  Returns the MGRS code representing a region in which the point resides.
    *
    *  @return  The alphabetic character string representing a 100 km MGRS region
    *           in which the MGRS point resides.
    *
    *  @see     getText()
    */
   std::string getScrCodeText() const;

private:
   std::string mText;
};

#endif
