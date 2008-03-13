/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MULTIPOINTOBJECTIMP_H
#define MULTIPOINTOBJECTIMP_H

#include "LocationType.h"
#include "PixelObjectImp.h"
#include "TypesFile.h"

#include <vector>

class MultipointObjectImp : public PixelObjectImp
{
public:
   MultipointObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~MultipointObjectImp();

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;

   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);
   void updateHandles();
   void updateGeo();
   void enableGeo();
   bool hit(LocationType pixelCoord) const;
   bool setProperty(const GraphicProperty* pProperty);
   bool addVertices(const std::vector<LocationType> &vertices, 
      const std::vector<LocationType> &geoVertices);
   virtual bool addVertex(LocationType endPoint);
   virtual bool addVertices(const std::vector<LocationType> &vertices);
   virtual bool addGeoVertices(const std::vector<LocationType> &geoVertices);
   virtual const std::vector<LocationType> &getVertices() const;
   void removeVertex(unsigned int index);
   void clearVertices();

   using GraphicObjectImp::getPixels;
   const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   bool replicateObject(const GraphicObject* pObject);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   /**
    * Desired insertion behavior:
    *  * left click and release to define a point, moving the point until release
    *  * continue left clicking and releasing for as many points as desired
    *  * double-click to end
    */
   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

protected:
   void updateBoundingBox();
   void scaleAndTranslateAllPoints(LocationType fixedPoint, LocationType startPoint,
      LocationType endPoint, bool bMaintainAspect, LocationType translateFactor = LocationType());

private:
   void getVertices(const DOMNode* pVertex, const std::string& nodeName, std::vector<LocationType>& vertices);
   std::vector<LocationType> mVertices;
   std::vector<LocationType> mGeoVertices;
   
   bool mFlipX;
   bool mFlipY;
   // bounding box is a derived value, is it current?
   bool mBoxMatchesVertices;
   bool mUseHitTolerance;
   bool mUpdating;
};

#define MULTIPOINTOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass) \
   const std::vector<LocationType> &getVertices() const \
   { \
      return impClass::getVertices(); \
   }

#endif // MULTIPOINTOBJECT_H
