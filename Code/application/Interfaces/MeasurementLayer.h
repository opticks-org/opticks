/*
* The information in this file is
* Copyright(c) 2007 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/



#ifndef MEASUREMENTLAYER_H
#define MEASUREMENTLAYER_H

#include "AnnotationLayer.h"

/**
 *  Adjusts the properties of a MeasurementLayer.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in AnnotationLayer.
 *
 *  @see     AnnotationLayer
 */
class MeasurementLayer : public AnnotationLayer
{
public:
   SETTING(DisplayDistanceLabel, MeasurementLayer, bool, true)
   SETTING(DisplayBearingLabel, MeasurementLayer, bool, true)
   SETTING(DisplayEndPointsLabel, MeasurementLayer, bool, true)
   SETTING(DistancePrecision, MeasurementLayer, int, 1)
   SETTING(BearingPrecision, MeasurementLayer, int, 1)
   SETTING(EndPointsPrecision, MeasurementLayer, int, 1)
   SETTING(LineColor, MeasurementLayer, ColorType, ColorType())
   SETTING(LineStyle, MeasurementLayer, LineStyle, SOLID_LINE)
   SETTING(LineWidth, MeasurementLayer, unsigned int, 0)
   SETTING(TextColor, MeasurementLayer, ColorType, ColorType())
   SETTING(TextFont, MeasurementLayer, std::string, "")
   SETTING(TextFontSize, MeasurementLayer, unsigned int, 12)
   SETTING(TextBold, MeasurementLayer, bool, false)
   SETTING(TextItalics, MeasurementLayer, bool, false)
   SETTING(TextUnderline, MeasurementLayer, bool, false)
   SETTING(DistanceUnits, MeasurementLayer, DistanceUnits, NO_DISTANCE_UNIT)
   SETTING(GeocoordType, MeasurementLayer, GeocoordType, GEOCOORD_GENERAL)
   SETTING(GeoFormat, MeasurementLayer, DmsFormatType, DMS_FULL_DECIMAL)

   /**
   *  Sets the status of displaying the distance text.
   *
   *  @param   bDisplay
   *           The new display status for the distance text.
   */
   virtual void setDisplayDistance(bool bDisplay) = 0;

   /**
   *  Returns the display status of the distance text.
   *
   *  @return  The distance display status.
   */
   virtual bool getDisplayDistance() const = 0;

   /**
   *  Sets the status of displaying the bearing text.
   *
   *  @param   bDisplay
   *           The new display status for the bearing text.
   */
   virtual void setDisplayBearing(bool bDisplay) = 0;

   /**
   *  Returns the display status of the bearing text.
   *
   *  @return  The bearing display status.
   */
   virtual bool getDisplayBearing() const = 0;

   /**
   *  Sets the status of displaying the end points text.
   *
   *  @param   bDisplay
   *           The new display status for the end points text.
   */
   virtual void setDisplayEndPoints(bool bDisplay) = 0;

   /**
   *  Returns the display status of the end points text.
   *
   *  @return  The end points display status.
   */
   virtual bool getDisplayEndPoints() const = 0;

   /**
   *  Returns the units used in displaying the distance.
   *
   *  @return  The distance units.
   */
   virtual DistanceUnits getDistanceUnits() const = 0;

   /**
   *  Sets the units to use in displaying the distance.
   *
   *  @param  unit
   *          The units to use in displaying the distance.
   */
   virtual void setDistanceUnits(DistanceUnits unit) = 0;

   /**
   *  Returns the geocoordinate type and data format used in displaying the 
   *  end point locations.
   *
   *  @param  geocoord
   *          The geocoordinate type.
   *  @param  dms
   *          The data format
   */
   virtual void getGeocoordTypes(GeocoordType &geocoord, DmsFormatType &dms) const = 0;

   /**
   *  Sets the geocoordinate type and data format to use in displaying the 
   *  end points locations.
   *
   *  @param  geocoord
   *          The geocoordinate type.
   *  @param  dms
   *          The data format.
   */
   virtual void setGeocoordTypes(GeocoordType geocoord, DmsFormatType dms) = 0;

protected:
   /**
   * The MeasurementLayer is deleted by SpatialDataView when it is deleted.
   */
   virtual ~MeasurementLayer() {}
};

#endif
