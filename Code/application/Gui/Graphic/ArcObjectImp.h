/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARCOBJECTIMP_H
#define ARCOBJECTIMP_H

#include "FilledObjectImp.h"
#include "GraphicObjectFactory.h"
#include "GraphicObjectImp.h"
#include "GraphicProperty.h"
#include "EllipseObjectImp.h"
#include "TypesFile.h"

class GraphicLayer;

class ArcObjectImp : public FilledObjectImp
{
public:
   ArcObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   void draw(double zoomFactor) const;
   bool setProperty(const GraphicProperty* pProperty);
   GraphicProperty* getProperty(const std::string& name) const;
   void moveHandle(int handle, LocationType point, bool bMaintainAspect = false);
   void updateHandles();
   bool hit(LocationType pixelCoord) const;

   bool processMousePress(LocationType screenCoord, Qt::MouseButton button,Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   LocationType getLocation(double dAngle) const;
   double getAngle(LocationType point) const;
   LocationType getCenter() const;
   double getXRadius() const;
   double getYRadius() const;

   bool setEllipseCorners(LocationType llCorner, LocationType urCorner);
   LocationType getEllipseLlCorner() const;
   LocationType getEllipseUrCorner() const;

   bool replicateObject(const GraphicObject* pObject);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   void updateBoundingBox();
   void updateLayout();
   double normalizeAngle(double dAngle) const;

private:
   GraphicResource<GraphicObjectImp> mpEllipse;

   bool mbNeedsLayout;
   LocationType mLlCorner;
   LocationType mUrCorner;
};

#define ARCOBJECTADAPTEREXTENSION_CLASSES \
   FILLEDOBJECTADAPTEREXTENSION_CLASSES

#define ARCOBJECTADAPTER_METHODS(impClass) \
   FILLEDOBJECTADAPTER_METHODS(impClass)

#endif   // ARCOBJECT_H
