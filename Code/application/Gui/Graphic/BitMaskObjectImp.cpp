/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BitMaskObjectImp.h"
#include "ColorType.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "GraphicLayer.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataView.h"
#include "SymbolRegionDrawer.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtGui/QColor>

using namespace std;
XERCES_CPP_NAMESPACE_USE

namespace
{
   class PixelOper
{
public:
   PixelOper(const BitMask& mask) : mMask(mask) {}
   inline bool operator()(int row, int col) const
   {
      return mMask.getPixel(col, row);
   }
private:
   const BitMask &mMask;
};
}

BitMaskObjectImp::BitMaskObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                   LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord),
   mpUnownedMask(NULL)
{
   addProperty("PixelSymbol");
   addProperty("FillColor");
   addProperty("DrawMode");
}

BitMaskObjectImp::~BitMaskObjectImp()
{
}

void BitMaskObjectImp::draw(double zoomFactor) const
{

   // Draw the pixels as a bitmask
   const BitMask *pMask = getMask();
   if (pMask == NULL)
   {
      return;
   }

   unsigned int ulStartColumn, ulEndColumn, ulStartRow, ulEndRow;
   pMask->getBoundingBox((int&) ulStartColumn, (int&) ulStartRow, (int&) ulEndColumn,
      (int&) ulEndRow);
   
   LocationType textPosition(ulStartColumn, ulStartRow);

   RasterElement* pRaster = NULL;

   GraphicLayer *pLayer = getLayer();
   VERIFYNRV(pLayer != NULL);

   SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*> (pLayer->getView());
   if (pSpatialDataView != NULL)
   {
      LayerList* pLayerList = pSpatialDataView->getLayerList();
      if (pLayerList != NULL)
      {
         pRaster = dynamic_cast<RasterElement*> (pLayerList->getPrimaryRasterElement());
      }
   }

   if (pRaster == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   unsigned int ulRows = pDescriptor->getRowCount();
   unsigned int ulColumns = pDescriptor->getColumnCount();

   bool bOutside = pMask->getPixel(ulStartColumn - 1, ulStartRow - 1);
   if (bOutside == true)
   {
      ulStartColumn = 0;
      ulStartRow = 0;
      ulEndColumn = ulColumns - 1;
      ulEndRow = ulRows - 1;
   }
   else
   {
      int iCount = 0;
      iCount = pMask->getCount();
      if (iCount == 0)
      {
         return;
      }
   }

   if (ulStartColumn > ulColumns)
   {
      ulStartColumn = ulColumns - 1;
   }

   if (ulStartRow > ulRows)
   {
      ulStartRow = ulRows - 1;
   }

   if (ulEndColumn > ulColumns)
   {
      ulEndColumn = ulColumns - 1;
   }
   if (ulEndRow > ulRows)
   {
      ulEndRow = ulRows - 1;
   }

   unsigned int ulVisStartColumn = ulStartColumn;
   unsigned int ulVisEndColumn = ulEndColumn;
   unsigned int ulVisStartRow = ulStartRow;
   unsigned int ulVisEndRow = ulEndRow;
   DrawUtil::restrictToViewport(ulVisStartColumn, ulVisStartRow, ulVisEndColumn, ulVisEndRow);

   PixelOper oper(*pMask);

   ColorType color = getFillColor();
   QColor qcolor(color.mRed, color.mGreen, color.mBlue);
   SymbolType symbol = getPixelSymbol();

   SymbolRegionDrawer::drawMarkers(ulStartColumn, ulStartRow, ulEndColumn, ulEndRow, 
      ulVisStartColumn, ulVisStartRow, ulVisEndColumn, ulVisEndRow, 
      symbol, qcolor, oper);
      
}

bool BitMaskObjectImp::hit(LocationType pixelCoord) const
{
   const BitMask *pMask = getMask();
   VERIFY(pMask != NULL);
   return pMask->getPixel(static_cast<int>(pixelCoord.mX), static_cast<int>(pixelCoord.mY));
}

void BitMaskObjectImp::setBitMask(const BitMask *pMask, bool copy)
{
   if (pMask != NULL)
   {
      if (copy)
      {
         *dynamic_cast<BitMaskImp*>(mpMask.get()) = 
            *dynamic_cast<const BitMaskImp*>(pMask);
      }
      else
      {
         mpUnownedMask = pMask;
      }
      emit modified();
   }
}

const BitMask *BitMaskObjectImp::getMask() const
{
   if (mpUnownedMask != NULL)
   {
      return mpUnownedMask;
   }
   return mpMask.get();
}

bool BitMaskObjectImp::toXml(XMLWriter* pXml) const
{
   VERIFY(pXml != NULL);
   if (!GraphicObjectImp::toXml(pXml))
   {
      return false;
   }
   pXml->pushAddPoint(pXml->addElement("bitmask"));
   BitMaskImp *pMask = const_cast<BitMaskImp*>(dynamic_cast<const BitMaskImp*>(getMask()));
   bool success = false;
   if (pMask != NULL)
   {
      success = pMask->toXml(pXml);
   }
   pXml->popAddPoint();
   return success;

}
 
bool BitMaskObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);
   if (!GraphicObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMNode *pBitmaskNode = NULL;
   for (pBitmaskNode = pDocument->getFirstChild();
      pBitmaskNode != NULL; pBitmaskNode = pBitmaskNode->getNextSibling())
   {
      if (XMLString::equals(pBitmaskNode->getNodeName(), X("bitmask")))
      {
         break;
      }
   }
   BitMaskImp *pMask = dynamic_cast<BitMaskImp*>(mpMask.get());
   if (pMask != NULL)
   {
      if (mpMask->fromXml(pBitmaskNode, version))
      {
         mpUnownedMask = NULL;
         return true;
      }
   }
   return false;
      
}

const string& BitMaskObjectImp::getObjectType() const
{
   static string type("BitMaskObjectImp");
   return type;
}

bool BitMaskObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "BitMaskObject"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}

const BitMask *BitMaskObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   return getMask();
}
