/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYGONOBJECTIMP_H
#define POLYGONOBJECTIMP_H

#include "glCommon.h"
#include "PolylineObjectImp.h"

class GraphicLayer;

class PolygonObjectImp : public PolylineObjectImp
{
public:
   PolygonObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   using PolylineObjectImp::getPixels;

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;

   bool addVertex(LocationType endPoint);

   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);

   bool hit(LocationType pixelCoord) const;

   const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   bool newPath();

   /**
    * Desired insertion behavior:
    *  * always display as a closed polygon
    *  * left click and release to define a point, moving the point until release
    *  * moving mouse between clicks causes rubber-banded drawing of line,
    *      but not the fill area.
    *  * double-click to end
    */
   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   PolygonObjectImp(const PolygonObjectImp& rhs);
   PolygonObjectImp& operator=(const PolygonObjectImp& rhs);
   static void CALLBACK combineVertexData(GLdouble coords[3], GLdouble* pVertexData[4],
      GLfloat weight[4], void** pOutData);
};

#define POLYGONOBJECTADAPTEREXTENSION_CLASSES \
   POLYLINEOBJECTADAPTEREXTENSION_CLASSES

#define POLYGONOBJECTADAPTER_METHODS(impClass) \
   POLYLINEOBJECTADAPTER_METHODS(impClass)

#endif
