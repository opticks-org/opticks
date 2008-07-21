/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEASUREMENTLAYERIMP_H
#define MEASUREMENTLAYERIMP_H

#include "AnnotationLayerImp.h"
#include "TypesFile.h"

class MeasurementLayerImp : public AnnotationLayerImp
{
   Q_OBJECT

public:
   MeasurementLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~MeasurementLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   MeasurementLayerImp &operator =(const MeasurementLayerImp &measurementLayer);

   void setDisplayDistance(bool bDisplay);
   bool getDisplayDistance() const;

   void setDisplayBearing(bool bDisplay);
   bool getDisplayBearing() const;

   void setDisplayEndPoints(bool bDisplay);
   bool getDisplayEndPoints() const;

   DistanceUnits getDistanceUnits() const;
   void setDistanceUnits(DistanceUnits unit);

   void getGeocoordTypes(GeocoordType &geocoord, DmsFormatType &dms) const;
   void setGeocoordTypes(GeocoordType geocoord, DmsFormatType dms);
   GeocoordType getGeocoordType() const;
   DmsFormatType getGeoFormat() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

signals:
   void distanceDisplayChanged(bool bDisplay);
   void distancePrecisionChanged(int precision);
   void bearingDisplayChanged(bool bDisplay);
   void bearingPrecisionChanged(int precision);
   void endPointsDisplayChanged(bool bDisplay);
   void endPointsPrecisionChanged(int precision);

private:
   bool mbDisplayDistanceText;
   bool mbDisplayBearingText;
   bool mbDisplayEndPointsText;
   DistanceUnits mDistanceUnits;
   GeocoordType mGeocoordType;
   DmsFormatType mGeoFormat;
};

#define MEASUREMENTLAYERADAPTER_METHODS(impClass) \
   ANNOTATIONLAYERADAPTER_METHODS(impClass) \
   void setDisplayDistance(bool bDisplay) \
   { \
      impClass::setDisplayDistance(bDisplay); \
   } \
   bool getDisplayDistance() const \
   { \
      return impClass::getDisplayDistance(); \
   } \
   void setDisplayBearing(bool bDisplay) \
   { \
      impClass::setDisplayBearing(bDisplay); \
   } \
   bool getDisplayBearing() const \
   { \
      return impClass::getDisplayBearing(); \
   } \
   void setDisplayEndPoints(bool bDisplay) \
   { \
      impClass::setDisplayEndPoints(bDisplay); \
   } \
   bool getDisplayEndPoints() const \
   { \
      return impClass::getDisplayEndPoints(); \
   } \
   DistanceUnits getDistanceUnits() const \
   { \
      return impClass::getDistanceUnits(); \
   } \
   void setDistanceUnits(DistanceUnits unit)\
   { \
      impClass::setDistanceUnits(unit); \
   } \
   void getGeocoordTypes(GeocoordType &geocoord, DmsFormatType &dms) const \
   { \
      impClass::getGeocoordTypes(geocoord, dms); \
   } \
   void setGeocoordTypes(GeocoordType geocoord, DmsFormatType dms)\
   { \
      impClass::setGeocoordTypes(geocoord, dms); \
   } 

#endif
