/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYLINEOBJECTIMP_H
#define POLYLINEOBJECTIMP_H

#include "MultipointObjectImp.h"
#include "GraphicProperty.h"
#include "GraphicGroup.h"
#include "TypesFile.h"

#include <list>
#include <vector>

class GraphicLayer;

class PolylineObjectImp : public MultipointObjectImp
{
public:
   PolylineObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~PolylineObjectImp();

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;
   bool hit(LocationType pixelCoord) const;

   unsigned int getNumSegments() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   using MultipointObjectImp::getPixels;
   const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   bool replicateObject(const GraphicObject *pObject);

   bool newPath();

   /**
    * Desired insertion behavior:
    *  * left click and release to define a point
    *  * moving mouse between clicks causes rubber-banded drawing of line
    *  * double-click to end
    */
   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

protected:
   /**
    * Starting point of new path in the shape.  
    *
    * This should always have size of at least one, with the first value
    * being zero.
    */
   std::vector<unsigned int> mPaths;

private:
   bool mUseHitTolerance;
   mutable bool mResetSymbolName;
};

#define POLYLINEOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass) \
   const std::vector<LocationType> &getVertices() const \
   { \
      return impClass::getVertices(); \
   }


#endif
