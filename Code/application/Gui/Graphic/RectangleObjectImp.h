/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RECTANGLEOBJECTIMP_H
#define RECTANGLEOBJECTIMP_H

#include "PixelObjectImp.h"
#include "TypesFile.h"
#include "GraphicObjectImp.h"

class GraphicLayer;

class RectangleObjectImp : public PixelObjectImp
{
public:
   RectangleObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   using PixelObjectImp::getPixels;

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;

   bool hit(LocationType pixelCoord) const;

   const BitMask *getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   RectangleObjectImp(const RectangleObjectImp& rhs);
   RectangleObjectImp& operator=(const RectangleObjectImp& rhs);
};

#define RECTANGLEOBJECTADAPTEREXTENSION_CLASSES \
   PIXELOBJECTADAPTEREXTENSION_CLASSES

#define RECTANGLEOBJECTADAPTER_METHODS(impClass) \
   PIXELOBJECTADAPTER_METHODS(impClass)

class RoundedRectangleObjectImp : public RectangleObjectImp
{
public:
   RoundedRectangleObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
      LocationType pixelCoord);

   void draw(double zoomFactor) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   RoundedRectangleObjectImp(const RoundedRectangleObjectImp& rhs);
   RoundedRectangleObjectImp& operator=(const RoundedRectangleObjectImp& rhs);
};

#define ROUNDEDRECTANGLEOBJECTADAPTEREXTENSION_CLASSES \
   RECTANGLEOBJECTADAPTEREXTENSION_CLASSES

#define ROUNDEDRECTANGLEOBJECTADAPTER_METHODS(impClass) \
   RECTANGLEOBJECTADAPTER_METHODS(impClass)

#endif
