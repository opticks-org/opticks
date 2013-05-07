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

   using PixelObjectImp::getPixels;

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;

   bool hit(LocationType pixelCoord) const;
   bool getExtents(std::vector<LocationType>& dataCoords) const;

   const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);

   bool setProperty(const GraphicProperty* pProp);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   bool getUseHitTolerance() const;
   bool setUseHitTolerance(bool bUse);

   double getHitToleranceFactor() const;
   bool setHitToleranceFactor(double hitFactor);

private:
   LineObjectImp(const LineObjectImp& rhs);
   LineObjectImp& operator=(const LineObjectImp& rhs);
   double mToleranceFactor;
   bool mUseHitTolerance;
};

#define LINEOBJECTADAPTEREXTENSION_CLASSES \
   PIXELOBJECTADAPTEREXTENSION_CLASSES

#define LINEOBJECTADAPTER_METHODS(impClass) \
   PIXELOBJECTADAPTER_METHODS(impClass)

#endif
