/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCALEBAROBJECTIMP_H
#define SCALEBAROBJECTIMP_H

#include "AttachmentPtr.h"
#include "FilledObjectImp.h"
#include "GraphicGroupImp.h"
#include "GraphicProperty.h"
#include "GraphicObjectFactory.h"
#include "PerspectiveView.h"
#include "RasterElement.h"

#include <string>

class GraphicLayer;
class GraphicGroup;

class ScaleBarObjectImp : public FilledObjectImp
{
public:
   ScaleBarObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~ScaleBarObjectImp();

   void setLayer(GraphicLayer* pLayer);
   void draw(double zoomFactor) const;
   bool setProperty(const GraphicProperty* pProp);
   GraphicProperty* getProperty(const std::string& name) const;
   void moveHandle (int handle, LocationType pixel, bool bMaintainAspect = false);
   bool hit(LocationType pixelCoord) const;
   const GraphicGroup &getGroup() const;

   bool replicateObject(const GraphicObject* pObject);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void updateGeo();

protected:
   void updateLayout();
   void updateGeoreferenceAttachment();
   void viewModified(Subject& subject, const std::string& signal, const boost::any& value);
   void georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v);

private:
   double mXgsd, mYgsd;   // variables for the georeference GSDs
   bool mNeedsLayout;
   AttachmentPtr<PerspectiveView> mpView;
   AttachmentPtr<RasterElement> mpGeoreference;

   GraphicResource<GraphicGroupImp> mpGroup;
};

#define SCALEBAROBJECTADAPTEREXTENSION_CLASSES \
   FILLEDOBJECTADAPTEREXTENSION_CLASSES

#define SCALEBAROBJECTADAPTER_METHODS(impClass) \
   FILLEDOBJECTADAPTER_METHODS(impClass)

#endif   // SCALEBAROBJECTImp_H
