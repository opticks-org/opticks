/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DIRECTIONALARROWOBJECTIMP_H
#define DIRECTIONALARROWOBJECTIMP_H

#include "AttachmentPtr.h"
#include "GraphicGroup.h"
#include "GraphicGroupImp.h"
#include "GraphicObjectImp.h"
#include "GraphicObjectFactory.h"
#include "RasterElement.h"
#include "TypesFile.h"

class GraphicLayer;

class DirectionalArrowObjectImp : public GraphicObjectImp
{
   Q_OBJECT

public:
   DirectionalArrowObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
      LocationType pixelCoord);
   virtual ~DirectionalArrowObjectImp();

   void setLayer(GraphicLayer* pLayer);
   void draw(double zoomFactor) const;
   bool setProperty(const GraphicProperty* pProperty);
   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);
   bool hit(LocationType pixelCoord) const;

   bool isOriented() const;

   bool replicateObject(const GraphicObject* pObject);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   const GraphicGroup& getGroup() const { return *(dynamic_cast<const GraphicGroup*>(mpGroup.get())); }

   /**
    * Desired insertion behavior:
    *  * Clicking button created object with default bounding box
    *  * User is warned if orientation is unavailable.
    *  * No further interaction
    */
   virtual bool processMousePress(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   virtual bool processMouseMove(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   virtual bool processMouseRelease(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);

   void updateGeo();

protected:
   virtual std::string getFileIndicator() const = 0;
   bool import(const std::string &filename);
   virtual void orient();

   void georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v);
   void updateGeoreferenceAttachment();

   AttachmentPtr<RasterElement> mpGeoreference;

protected slots:
   virtual void updateOrientation();

private:
   GraphicResource<GraphicGroupImp> mpGroup;
   bool mbOriented;
};

#endif
