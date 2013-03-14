/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceDescriptorImp.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "SignalBlocker.h"
#include "StringUtilities.h"

XERCES_CPP_NAMESPACE_USE

GeoreferenceDescriptorImp::GeoreferenceDescriptorImp() :
   mGeoreferenceOnImport(GeoreferenceDescriptor::getSettingAutoGeoreference()),
   mCreateLayer(GeoreferenceDescriptor::getSettingCreateLayer()),
   mLayerName("GEO_RESULTS"),
   mDisplayLayer(GeoreferenceDescriptor::getSettingDisplayLayer()),
   mGeocoordType(GeoreferenceDescriptor::getSettingGeocoordType()),
   mLatLonFormat(GeoreferenceDescriptor::getSettingLatLonFormat())
{}

GeoreferenceDescriptorImp::~GeoreferenceDescriptorImp()
{}

void GeoreferenceDescriptorImp::setGeoreferenceOnImport(bool georeference)
{
   if (georeference != mGeoreferenceOnImport)
   {
      mGeoreferenceOnImport = georeference;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferenceOnImportChanged), boost::any(mGeoreferenceOnImport));
   }
}

bool GeoreferenceDescriptorImp::getGeoreferenceOnImport() const
{
   return mGeoreferenceOnImport;
}

void GeoreferenceDescriptorImp::setGeoreferencePlugInName(const std::string& plugInName)
{
   // Ensure that the plug-in is a Georeference plug-in
   if (plugInName.empty() == false)    // Allow setting an empty string for the plug-in
   {
      PlugInDescriptor* pPlugInDescriptor = Service<PlugInManagerServices>()->getPlugInDescriptor(plugInName);
      if ((pPlugInDescriptor == NULL) || (pPlugInDescriptor->getType() != PlugInManagerServices::GeoreferenceType()))
      {
         return;
      }
   }

   if (plugInName != mPlugInName)
   {
      mPlugInName = plugInName;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferencePlugInNameChanged), boost::any(mPlugInName));
   }
}

const std::string& GeoreferenceDescriptorImp::getGeoreferencePlugInName() const
{
   return mPlugInName;
}

void GeoreferenceDescriptorImp::setCreateLayer(bool createLayer)
{
   if (createLayer != mCreateLayer)
   {
      mCreateLayer = createLayer;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, CreateLayerChanged), boost::any(mCreateLayer));
   }
}

bool GeoreferenceDescriptorImp::getCreateLayer() const
{
   return mCreateLayer;
}

void GeoreferenceDescriptorImp::setLayerName(const std::string& layerName)
{
   if (layerName != mLayerName)
   {
      mLayerName = layerName;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, LayerNameChanged), boost::any(mLayerName));
   }
}

const std::string& GeoreferenceDescriptorImp::getLayerName() const
{
   return mLayerName;
}

void GeoreferenceDescriptorImp::setDisplayLayer(bool displayLayer)
{
   if (displayLayer != mDisplayLayer)
   {
      mDisplayLayer = displayLayer;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, DisplayLayerChanged), boost::any(mDisplayLayer));
   }
}

bool GeoreferenceDescriptorImp::getDisplayLayer() const
{
   return mDisplayLayer;
}

void GeoreferenceDescriptorImp::setGeocoordType(GeocoordType geocoordType)
{
   if (geocoordType != mGeocoordType)
   {
      mGeocoordType = geocoordType;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, GeocoordTypeChanged), boost::any(mGeocoordType));
   }
}

GeocoordType GeoreferenceDescriptorImp::getGeocoordType() const
{
   return mGeocoordType;
}

void GeoreferenceDescriptorImp::setLatLonFormat(DmsFormatType latLonFormat)
{
   if (latLonFormat != mLatLonFormat)
   {
      mLatLonFormat = latLonFormat;
      notify(SIGNAL_NAME(GeoreferenceDescriptor, LatLonFormatChanged), boost::any(mLatLonFormat));
   }
}

DmsFormatType GeoreferenceDescriptorImp::getLatLonFormat() const
{
   return mLatLonFormat;
}

bool GeoreferenceDescriptorImp::compare(const DynamicObject* pObject) const
{
   const GeoreferenceDescriptor* pGeorefDescriptor = dynamic_cast<const GeoreferenceDescriptor*>(pObject);
   if (pGeorefDescriptor == NULL)
   {
      return false;
   }

   if ((pGeorefDescriptor->getGeoreferenceOnImport() != mGeoreferenceOnImport) ||
      (pGeorefDescriptor->getGeoreferencePlugInName() != mPlugInName) ||
      (pGeorefDescriptor->getCreateLayer() != mCreateLayer) ||
      (pGeorefDescriptor->getLayerName() != mLayerName) ||
      (pGeorefDescriptor->getDisplayLayer() != mDisplayLayer) ||
      (pGeorefDescriptor->getGeocoordType() != mGeocoordType) ||
      (pGeorefDescriptor->getLatLonFormat() != mLatLonFormat))
   {
      return false;
   }

   return DynamicObjectImp::compare(pObject);
}

bool GeoreferenceDescriptorImp::clone(const GeoreferenceDescriptor* pGeorefDescriptor)
{
   if (pGeorefDescriptor == NULL)
   {
      return false;
   }

   const GeoreferenceDescriptorImp* pGeorefDescriptorImp =
      dynamic_cast<const GeoreferenceDescriptorImp*>(pGeorefDescriptor);
   if (pGeorefDescriptorImp != this)
   {
      // Only notify the Modified signal at the end instead of notifying potentially
      // numerous signals as attributes are cleared and added in the base class
      {
         Subject* pSubject = dynamic_cast<Subject*>(this);
         VERIFY(pSubject != NULL);

         SignalBlocker block(*pSubject);
         DynamicObjectImp::operator=(*pGeorefDescriptorImp);
      }

      setGeoreferenceOnImport(pGeorefDescriptor->getGeoreferenceOnImport());
      setGeoreferencePlugInName(pGeorefDescriptor->getGeoreferencePlugInName());
      setCreateLayer(pGeorefDescriptor->getCreateLayer());
      setLayerName(pGeorefDescriptor->getLayerName());
      setDisplayLayer(pGeorefDescriptor->getDisplayLayer());
      setGeocoordType(pGeorefDescriptor->getGeocoordType());
      setLatLonFormat(pGeorefDescriptor->getLatLonFormat());
   }

   return true;
}

const std::string& GeoreferenceDescriptorImp::getObjectType() const
{
   static std::string sType("GeoreferenceDescriptorImp");
   return sType;
}

bool GeoreferenceDescriptorImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "GeoreferenceDescriptor"))
   {
      return true;
   }

   return false;
}

bool GeoreferenceDescriptorImp::toXml(XMLWriter* pXml) const
{
   if ((pXml == NULL) || (DynamicObjectImp::toXml(pXml) == false))
   {
      return false;
   }

   try
   {
      pXml->addAttr("georeferenceOnImport", mGeoreferenceOnImport);
      pXml->addAttr("plugInName", mPlugInName);
      pXml->addAttr("createLayer", mCreateLayer);
      pXml->addAttr("layerName", mLayerName);
      pXml->addAttr("displayLayer", mDisplayLayer);
      pXml->addAttr("geocoordType", mGeocoordType);
      pXml->addAttr("latLonFormat", mLatLonFormat);
   }
   catch (XmlBase::XmlException)
   {
      return false;
   }

   return true;
}

bool GeoreferenceDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if ((pDocument == NULL) || (DynamicObjectImp::fromXml(pDocument, version) == false))
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   VERIFY(pElement != NULL);

   bool error = false;
   mGeoreferenceOnImport = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("georeferenceOnImport"))),
      &error);
   if (error == true)
   {
      return false;
   }

   mPlugInName = A(pElement->getAttribute(X("plugInName")));

   mCreateLayer = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("createLayer"))), &error);
   if (error == true)
   {
      return false;
   }

   mLayerName = A(pElement->getAttribute(X("layerName")));

   mDisplayLayer = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("displayLayer"))), &error);
   if (error == true)
   {
      return false;
   }

   mGeocoordType = StringUtilities::fromXmlString<GeocoordType>(A(pElement->getAttribute(X("geocoordType"))), &error);
   if (error == true)
   {
      return false;
   }

   mLatLonFormat = StringUtilities::fromXmlString<DmsFormatType>(A(pElement->getAttribute(X("latLonFormat"))), &error);
   if (error == true)
   {
      return false;
   }

   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}
