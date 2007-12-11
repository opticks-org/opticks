/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINEOBJECTIMP_H
#define LINEOBJECTIMP_H

#include "GraphicObjectImp.h"
#include "PixelObjectImp.h"
#include "TypesFile.h"

class GraphicLayer;

class LineObjectImp : public PixelObjectImp
{
public:
   LineObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;

   bool hit(LocationType pixelCoord) const;

   using GraphicObjectImp::getPixels;
   const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);

   bool setProperty(const GraphicProperty* pProp);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   bool mUseHitTolerance;
};

#define LINEOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass)

#endif
