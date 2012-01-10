/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DIMENSIONOBJECTIMP_H
#define DIMENSIONOBJECTIMP_H

#include "BitMask.h"
#include "ObjectFactory.h"
#include "PixelObjectImp.h"

#include <vector>

class DimensionObjectImp : public PixelObjectImp
{
public:
   DimensionObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~DimensionObjectImp();

   using PixelObjectImp::getPixels;

   void drawVector(double zoomFactor) const;
   void drawPixels(double zoomFactor) const;

   void getSecExtents(int& minSec, int& maxSec) const;
   void getMainExtents(int& minMain, int& maxMain) const;


   bool hit(LocationType coord) const;

   bool processMousePress(LocationType sceneCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   bool processMouseMove(LocationType sceneCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(LocationType sceneCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);

   bool hasCornerHandles() const;

   void updateHandles();

   void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);

   const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void setLayer(GraphicLayer *pLayer);

   class DimensionSwitcher
   {
   public:
      DimensionSwitcher(GraphicObjectType type);

      LocationType location(double mainDim, double secondDim) const;

      int mainDimension(const LocationType &location) const;
      int secondDimension(const LocationType &location) const;

      int mainDimension(int x, int y) const;
      int secondDimension(int x, int y) const;

      int x(int mainDim, int secondDim) const;
      int y(int mainDim, int secondDim) const;

   private:
      DimensionSwitcher& operator=(const DimensionSwitcher& rhs);
      GraphicObjectType mType;
   };

private:
   DimensionObjectImp(const DimensionObjectImp& rhs);
   DimensionObjectImp& operator=(const DimensionObjectImp& rhs);
   std::vector <int> mSelectedDims;
   int mInsertingDim;
   bool mInserting;
   DimensionSwitcher mSwitcher;
};

#define DIMENSIONOBJECTADAPTEREXTENSION_CLASSES \
   PIXELOBJECTADAPTEREXTENSION_CLASSES

#define DIMENSIONOBJECTADAPTER_METHODS(impClass) \
   PIXELOBJECTADAPTER_METHODS(impClass)

#endif
