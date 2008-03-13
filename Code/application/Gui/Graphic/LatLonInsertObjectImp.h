/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONINSERTOBJECTIMP_H
#define LATLONINSERTOBJECTIMP_H

#include "AttachmentPtr.h"
#include "GraphicProperty.h"
#include "GraphicGroup.h"
#include "GraphicGroupImp.h"
#include "GraphicObjectFactory.h"
#include "GraphicObjectImp.h"
#include "GeoPoint.h"
#include "RasterElement.h"
#include "TypesFile.h"

class GraphicLayer;
class ArrowObjectImp;
class TextObjectImp;

class LatLonInsertObjectImp : public GraphicObjectImp
{
   Q_OBJECT

public:
   LatLonInsertObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~LatLonInsertObjectImp();

   void setLayer(GraphicLayer* pLayer);
   void draw(double zoomFactor) const;

   void updateHandles();

   bool setProperty(const GraphicProperty* pProp);
   GraphicProperty* getProperty(const std::string &name) const;

   void move(LocationType delta);

   void moveHandle (int handle, LocationType pixel, bool bMaintainAspect = false);
   bool hit(LocationType pixelCoord) const;

   bool replicateObject(const GraphicObject* pObject);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   LocationType getLatLonLoc() const;

   const GraphicGroup& getGroup() const { return *dynamic_cast<const GraphicGroup*>(mpGroup.get()); }

   /**
    * Intended insertion behavior:
    *  * Clicking left mouse will insert object with default bounding box.
    *  * No further insertion behavior
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

   bool hasCornerHandles() const { return false; }

   void updateGeo();

protected:
   void updateLatLon();
   void updateLatLonText();

   void updateGeoreferenceAttachment();
   void georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v);

   TextObjectImp* getTextObject() const;
   ArrowObjectImp* getArrowObject() const;

private:
   GraphicResource<GraphicGroupImp> mpGroup;
   AttachmentPtr<RasterElement> mpGeoreference;
   GeocoordType mGeocoordType;
   DmsFormatType mFormatType;
};

#define LATLONINSERTOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
