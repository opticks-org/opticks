/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEDESCRIPTORIMP_H
#define GEOREFERENCEDESCRIPTORIMP_H

#include "DynamicObjectImp.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class DynamicObject;
class GeoreferenceDescriptor;

class GeoreferenceDescriptorImp : public DynamicObjectImp
{
public:
   GeoreferenceDescriptorImp();
   virtual ~GeoreferenceDescriptorImp();

   virtual void setGeoreferenceOnImport(bool georeference);
   virtual bool getGeoreferenceOnImport() const;
   virtual void setGeoreferencePlugInName(const std::string& plugInName);
   virtual const std::string& getGeoreferencePlugInName() const;
   virtual void setValidGeoreferencePlugIns(const std::vector<std::string>& plugInNames);
   virtual bool isValidGeoreferencePlugIn(const std::string& plugInName) const;
   virtual const std::vector<std::string>& getValidGeoreferencePlugIns() const;
   virtual void resetValidGeoreferencePlugIns();
   virtual void setCreateLayer(bool createLayer);
   virtual bool getCreateLayer() const;
   virtual void setLayerName(const std::string& layerName);
   virtual const std::string& getLayerName() const;
   virtual void setDisplayLayer(bool displayLayer);
   virtual bool getDisplayLayer() const;
   virtual void setGeocoordType(GeocoordType geocoordType);
   virtual GeocoordType getGeocoordType() const;
   virtual void setLatLonFormat(DmsFormatType latLonFormat);
   virtual DmsFormatType getLatLonFormat() const;

   virtual bool compare(const DynamicObject* pObject) const;
   virtual bool clone(const GeoreferenceDescriptor* pGeorefDescriptor) = 0;

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   bool mGeoreferenceOnImport;
   std::string mPlugInName;
   std::vector<std::string> mValidPlugIns;
   bool mCreateLayer;
   std::string mLayerName;
   bool mDisplayLayer;
   GeocoordType mGeocoordType;
   DmsFormatType mLatLonFormat;
};

#define GEOREFERENCEDESCRIPTORADAPTEREXTENSION_CLASSES \
   DYNAMICOBJECTADAPTEREXTENSION_CLASSES

#define GEOREFERENCEDESCRIPTORADAPTER_METHODS(impClass) \
   DYNAMICOBJECTADAPTER_METHODS(impClass) \
   void setGeoreferenceOnImport(bool georeference) \
   { \
      impClass::setGeoreferenceOnImport(georeference); \
   } \
   bool getGeoreferenceOnImport() const \
   { \
      return impClass::getGeoreferenceOnImport(); \
   } \
   void setGeoreferencePlugInName(const std::string& plugInName) \
   { \
      impClass::setGeoreferencePlugInName(plugInName); \
   } \
   const std::string& getGeoreferencePlugInName() const \
   { \
      return impClass::getGeoreferencePlugInName(); \
   } \
   void setValidGeoreferencePlugIns(const std::vector<std::string>& plugInNames) \
   { \
      impClass::setValidGeoreferencePlugIns(plugInNames); \
   } \
   bool isValidGeoreferencePlugIn(const std::string& plugInName) const \
   { \
      return impClass::isValidGeoreferencePlugIn(plugInName); \
   } \
   const std::vector<std::string>& getValidGeoreferencePlugIns() const \
   { \
      return impClass::getValidGeoreferencePlugIns(); \
   } \
   void resetValidGeoreferencePlugIns() \
   { \
      impClass::resetValidGeoreferencePlugIns(); \
   } \
   void setCreateLayer(bool createLayer) \
   { \
      impClass::setCreateLayer(createLayer); \
   } \
   bool getCreateLayer() const \
   { \
      return impClass::getCreateLayer(); \
   } \
   void setLayerName(const std::string& layerName) \
   { \
      impClass::setLayerName(layerName); \
   } \
   const std::string& getLayerName() const \
   { \
      return impClass::getLayerName(); \
   } \
   void setDisplayLayer(bool displayLayer)\
   { \
      impClass::setDisplayLayer(displayLayer); \
   } \
   bool getDisplayLayer() const \
   { \
      return impClass::getDisplayLayer(); \
   } \
   void setGeocoordType(GeocoordType geocoordType) \
   { \
      impClass::setGeocoordType(geocoordType); \
   } \
   GeocoordType getGeocoordType() const \
   { \
      return impClass::getGeocoordType(); \
   } \
   void setLatLonFormat(DmsFormatType latLonFormat) \
   { \
      impClass::setLatLonFormat(latLonFormat); \
   } \
   DmsFormatType getLatLonFormat() const \
   { \
      return impClass::getLatLonFormat(); \
   } \
   bool clone(const GeoreferenceDescriptor* pGeorefDescriptor) \
   { \
      return impClass::clone(pGeorefDescriptor); \
   }

#endif
