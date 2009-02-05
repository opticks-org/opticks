/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "MeasurementLayer.h"
#include "MeasurementLayerImp.h"
#include "PropertiesAnnotationLayer.h"
#include "PropertiesMeasurementLayer.h"
#include "SessionManager.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

MeasurementLayerImp::MeasurementLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   AnnotationLayerImp(id, layerName, pElement)
{
   removePropertiesPage(PropertiesAnnotationLayer::getName());
   addPropertiesPage(PropertiesMeasurementLayer::getName());
   clearAcceptableGraphicTypes();
   addAcceptableGraphicType(MOVE_OBJECT);
   addAcceptableGraphicType(MEASUREMENT_OBJECT);

   mbDisplayDistanceText = MeasurementLayer::getSettingDisplayDistanceLabel();
   mbDisplayBearingText = MeasurementLayer::getSettingDisplayBearingLabel();
   mbDisplayEndPointsText = MeasurementLayer::getSettingDisplayEndPointsLabel();
   mDistanceUnits = MeasurementLayer::getSettingDistanceUnits();
   mGeocoordType = MeasurementLayer::getSettingGeocoordType();
   mGeoFormat = MeasurementLayer::getSettingGeoFormat();

   VERIFYNR(connect(this, SIGNAL(distanceDisplayChanged(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(bearingDisplayChanged(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(endPointsDisplayChanged(bool)), this, SIGNAL(modified())));
}

MeasurementLayerImp::~MeasurementLayerImp()
{
}

const string& MeasurementLayerImp::getObjectType() const
{
   static string type("MeasurementLayerImp");
   return type;
}

bool MeasurementLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MeasurementLayer"))
   {
      return true;
   }

   return AnnotationLayerImp::isKindOf(className);
}

MeasurementLayerImp &MeasurementLayerImp::operator =(
   const MeasurementLayerImp &measurementLayer)
{
   if (this != &measurementLayer)
   {
      AnnotationLayerImp::operator =(measurementLayer);

      mbDisplayBearingText = measurementLayer.mbDisplayBearingText;
      mbDisplayDistanceText = measurementLayer.mbDisplayDistanceText;
      mbDisplayEndPointsText = measurementLayer.mbDisplayEndPointsText;
      mDistanceUnits = measurementLayer.mDistanceUnits;
      mGeocoordType = measurementLayer.mGeocoordType;
      mGeoFormat = measurementLayer.mGeoFormat;
   }

   return *this;
}

void MeasurementLayerImp::setDisplayDistance(bool bDisplay)
{
   mbDisplayDistanceText = bDisplay;
}

bool MeasurementLayerImp::getDisplayDistance() const
{
   return mbDisplayDistanceText;
}

void MeasurementLayerImp::setDisplayBearing(bool bDisplay)
{
   mbDisplayBearingText = bDisplay;
}

bool MeasurementLayerImp::getDisplayBearing() const
{
   return mbDisplayBearingText;
}

void MeasurementLayerImp::setDisplayEndPoints(bool bDisplay)
{
   mbDisplayEndPointsText = bDisplay;
}

bool MeasurementLayerImp::getDisplayEndPoints() const
{
   return mbDisplayEndPointsText;
}

DistanceUnits MeasurementLayerImp::getDistanceUnits() const
{
   return mDistanceUnits;
}

void MeasurementLayerImp::setDistanceUnits(DistanceUnits unit)
{
   mDistanceUnits = unit;
}

void MeasurementLayerImp::getGeocoordTypes(GeocoordType &geocoord, DmsFormatType &dms) const
{
   geocoord = mGeocoordType;
   dms = mGeoFormat;
}

void MeasurementLayerImp::setGeocoordTypes(GeocoordType geocoord, DmsFormatType dms)
{
   mGeocoordType = geocoord;
   mGeoFormat = dms;
}

GeocoordType MeasurementLayerImp::getGeocoordType() const
{
   return mGeocoordType;
}

DmsFormatType MeasurementLayerImp::getGeoFormat() const
{
   return mGeoFormat;
}

bool MeasurementLayerImp::toXml(XMLWriter* pXml) const
{
   if (Service<SessionManager>()->isSessionSaving())
   {
      if (pXml == NULL || !AnnotationLayerImp::toXml(pXml))
      {
         return false;
      }
      pXml->addAttr("displayBearing", mbDisplayBearingText);
      pXml->addAttr("displayDistance", mbDisplayDistanceText);
      pXml->addAttr("displayEndPoints", mbDisplayEndPointsText);
      pXml->addAttr("distanceUnit", mDistanceUnits);
      pXml->addAttr("geoCoordType", mGeocoordType);
      pXml->addAttr("geoFormat", mGeoFormat);
   }

   return true;
}

bool MeasurementLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (Service<SessionManager>()->isSessionLoading())
   {
      if (pDocument == NULL || !AnnotationLayerImp::fromXml(pDocument, version))
      {
         return false;
      }
      mbDisplayBearingText = StringUtilities::fromXmlString<bool>(
         A(static_cast<DOMElement*>(pDocument)->getAttribute(X("displayBearing"))));
      mbDisplayDistanceText = StringUtilities::fromXmlString<bool>(
         A(static_cast<DOMElement*>(pDocument)->getAttribute(X("displayDistance"))));
      mbDisplayEndPointsText = StringUtilities::fromXmlString<bool>(
         A(static_cast<DOMElement*>(pDocument)->getAttribute(X("displayEndPoints"))));
      mDistanceUnits = StringUtilities::fromXmlString<DistanceUnits>(
         A(static_cast<DOMElement*>(pDocument)->getAttribute(X("distanceUnit"))));
      mGeocoordType = StringUtilities::fromXmlString<GeocoordType>(
         A(static_cast<DOMElement*>(pDocument)->getAttribute(X("geoCoordType"))));
      mGeoFormat = StringUtilities::fromXmlString<DmsFormatType>(
         A(static_cast<DOMElement*>(pDocument)->getAttribute(X("geoFormat"))));
   }

   return true;
}
