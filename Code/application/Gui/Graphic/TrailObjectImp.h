/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRAILOBJECTIMP_H
#define TRAILOBJECTIMP_H

#include "LocationType.h"
#include "RectangleObjectImp.h"

class TrailObjectImp : public RectangleObjectImp
{
public:
   TrailObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~TrailObjectImp();

   void setStencilBufferSize(int rows, int columns);
   void addToStencil(LocationType lowerLeft, LocationType lowerRight, LocationType upperLeft, LocationType upperRight);
   void clearStencil();

   void drawVector(double zoomFactor) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   void updateStencil();

private:
   char* mpBuffer;
   int mRows;
   int mColumns;
};

#define TRAILOBJECTADAPTEREXTENSION_CLASSES \
   RECTANGLEOBJECTADAPTEREXTENSION_CLASSES

#define TRAILOBJECTADAPTER_METHODS(impClass) \
   RECTANGLEOBJECTADAPTER_METHODS(impClass)

#endif
