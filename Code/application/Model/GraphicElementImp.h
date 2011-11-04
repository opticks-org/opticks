/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICELEMENTIMP_H
#define GRAPHICELEMENTIMP_H

#include "AttachmentPtr.h"
#include "DataElementImp.h"
#include "GraphicGroup.h"
#include "GraphicObjectFactory.h"
#include "RasterElement.h"
#include "SpatialDataView.h"

class GraphicElementImp : public DataElementImp
{
public:
   GraphicElementImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~GraphicElementImp();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getElementTypes(std::vector<std::string>& classList);
   static bool isKindOfElement(const std::string& className);

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   GraphicGroup *getGroup();
   const GraphicGroup *getGroup() const;

   virtual void groupModified(Subject &subject, const std::string &signal, const boost::any &data);

   void setInteractive(bool interactive);
   bool getInteractive() const;

   virtual bool setGeocentric(bool geocentric);
   virtual bool getGeocentric() const;

   const RasterElement *getGeoreferenceElement() const;

   void georeferenceModified(Subject &subject, const std::string &signal, const boost::any &data);

private:
   GraphicElementImp(const GraphicElementImp& rhs);
   GraphicElementImp& operator=(const GraphicElementImp& rhs);
   GraphicResource<GraphicGroup> mpGroup;
   bool mInteractive;
   AttachmentPtr<RasterElement> mpGeocentricSource;
   AttachmentPtr<SpatialDataView> mpView;
};

#define GRAPHICELEMENTADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define GRAPHICELEMENTADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   GraphicGroup *getGroup() \
   { \
      return impClass::getGroup(); \
   } \
   const GraphicGroup *getGroup() const \
   { \
      return impClass::getGroup(); \
   } \
   void setInteractive(bool interactive) \
   { \
      return impClass::setInteractive(interactive); \
   } \
   bool getInteractive() const \
   { \
      return impClass::getInteractive(); \
   } \
   bool setGeocentric(bool geocentric) \
   { \
      return impClass::setGeocentric(geocentric); \
   } \
   bool getGeocentric() const \
   { \
      return impClass::getGeocentric(); \
   }


#endif
