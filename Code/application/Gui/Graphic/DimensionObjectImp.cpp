/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "DimensionObjectImp.h"
#include "GraphicLayer.h"
#include "MathUtil.h"
#include "View.h"

#include <algorithm>
using namespace std;

XERCES_CPP_NAMESPACE_USE

DimensionObjectImp::DimensionSwitcher::DimensionSwitcher(GraphicObjectType type) : mType(type)
{
}

LocationType DimensionObjectImp::DimensionSwitcher::location(double mainDim, double secondDim) const
{
   if (mType == ROW_OBJECT)
   {
      return LocationType(secondDim, mainDim);
   }
   else
   {
      return LocationType(mainDim, secondDim);
   }
}

int DimensionObjectImp::DimensionSwitcher::mainDimension(const LocationType &location) const
{
   if (mType == ROW_OBJECT)
   {
      return floor(location.mY);
   }
   else
   {
      return floor(location.mX);
   }
}

int DimensionObjectImp::DimensionSwitcher::secondDimension(const LocationType &location) const
{
   if (mType == ROW_OBJECT)
   {
      return floor(location.mX);
   }
   else
   {
      return floor(location.mY);
   }
}

int DimensionObjectImp::DimensionSwitcher::mainDimension(int x, int y) const
{
   if (mType == ROW_OBJECT)
   {
      return y;
   }
   else
   {
      return x;
   }
}

int DimensionObjectImp::DimensionSwitcher::secondDimension(int x, int y) const
{
   if (mType == ROW_OBJECT)
   {
      return x;
   }
   else
   {
      return y;
   }
}
int DimensionObjectImp::DimensionSwitcher::x(int mainDim, int secondDim) const
{
   if (mType == ROW_OBJECT)
   {
      return secondDim;
   }
   else
   {
      return mainDim;
   }
}
int DimensionObjectImp::DimensionSwitcher::y(int mainDim, int secondDim) const
{
   if (mType == ROW_OBJECT)
   {
      return mainDim;
   }
   else
   {
      return secondDim;
   }
}


namespace
{
   class AddHandle
   {
   public:
      AddHandle(const DimensionObjectImp::DimensionSwitcher &switcher,
         std::vector<LocationType> &handles, double midSec) : 
         mSwitcher(switcher), mHandles(handles), mMidSec(midSec)
      {
      }

      void operator()(int main)
      {
         mHandles.push_back(mSwitcher.location(main+0.5, mMidSec));
      }
   private:
      const DimensionObjectImp::DimensionSwitcher& mSwitcher;
      std::vector<LocationType>& mHandles;
      double mMidSec;
   };

   class DrawDimVector
   {
   public:
      DrawDimVector(const DimensionObjectImp::DimensionSwitcher &switcher,
         int minSec, int maxSec) : mSwitcher(switcher), mMinSec(minSec), mMaxSec(maxSec)
      {
         glBegin(GL_LINES);
      }

      ~DrawDimVector()
      {
         glEnd();
      }

      void operator()(int main)
      {
         glVertex2i(mSwitcher.x(main, mMinSec), mSwitcher.y(main, mMinSec));
         glVertex2i(mSwitcher.x(main, mMaxSec), mSwitcher.y(main, mMaxSec));
      }
   private:
      const DimensionObjectImp::DimensionSwitcher& mSwitcher;
      int mMinSec;
      int mMaxSec;
   };

   class DrawDimPixels
   {
   public:
      DrawDimPixels(const DimensionObjectImp::DimensionSwitcher &switcher,
         SymbolType symbol, int minSec, int maxSec) : mSwitcher(switcher),
         mDrawer(symbol), mMinSec(minSec), mMaxSec(maxSec)
      {
      }

      void operator()(int main)
      {
         DrawUtil::drawPixelLine(mSwitcher.location(main, mMinSec), 
            mSwitcher.location(main, mMaxSec), mDrawer);
      }

   private:
      const DimensionObjectImp::DimensionSwitcher& mSwitcher;
      DrawUtil::PixelDrawer mDrawer;
      int mMinSec;
      int mMaxSec;
   };
}

DimensionObjectImp::DimensionObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer, 
                                       LocationType pixelCoord) :
   PixelObjectImp(id, type, pLayer, pixelCoord),
   mInsertingDim(0),
   mInserting(false),
   mSwitcher(type)
{
   addProperty("LineColor");
   addProperty("PixelSymbol");
}

DimensionObjectImp::~DimensionObjectImp()
{
}

void DimensionObjectImp::drawVector(double zoomFactor) const
{
   ColorType color = getLineColor();
   glColor3ub(color.mRed, color.mGreen, color.mBlue);

   int minSec;
   int maxSec;
   getSecExtents(minSec, maxSec);
   ::DrawDimVector drawer(mSwitcher, minSec, maxSec); 
   for_each(mSelectedDims.begin(), mSelectedDims.end(), drawer);
   if (mInserting)
   {
      drawer(mInsertingDim);
   }

   return;
}

void DimensionObjectImp::drawPixels(double zoomFactor) const
{
   ColorType color = getLineColor();
   glColor3ub(color.mRed, color.mGreen, color.mBlue);

   int minSec;
   int maxSec;
   getSecExtents(minSec, maxSec);

   ::DrawDimPixels drawer(mSwitcher, getPixelSymbol(), minSec, maxSec);
   for_each(mSelectedDims.begin(), mSelectedDims.end(), drawer);
   if (mInserting)
   {
      drawer(mInsertingDim);
   }
}

void DimensionObjectImp::getSecExtents(int &minSec, int &maxSec) const
{
   GraphicLayer* pLayer = getLayer();
   if (pLayer == NULL)
   {
      // if there is no layer associated with this object, the handles
      // will be incorrect. this is ok as there is no layer to draw the
      // handles. this means that if a layer is attached to an existing
      // element, the handles should be updated
      return;
   }

   View* pView = pLayer->getView();
   VERIFYNRV(pView != NULL);

   double dMinX;
   double dMinY;
   double dMaxX;
   double dMaxY;
   pView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

   minSec = mSwitcher.secondDimension(dMinX, dMinY);
   maxSec = mSwitcher.secondDimension(roundDouble(dMaxX), roundDouble(dMaxY));
}

void DimensionObjectImp::getMainExtents(int &minMain, int &maxMain) const
{
   GraphicLayer* pLayer = getLayer();
   VERIFYNRV(pLayer != NULL);

   View* pView = pLayer->getView();
   VERIFYNRV(pView != NULL);

   double dMinX;
   double dMinY;
   double dMaxX;
   double dMaxY;
   pView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

   minMain = mSwitcher.mainDimension(dMinX, dMinY);
   // ensure that the main extents encompass at least one pixel
   maxMain = mSwitcher.mainDimension(dMaxX+1, dMaxY+1);
}

bool DimensionObjectImp::hit(LocationType coord) const
{
   int dim = mSwitcher.mainDimension(coord);
   return (find(mSelectedDims.begin(), mSelectedDims.end(), dim) != mSelectedDims.end());
}

bool DimensionObjectImp::processMousePress(LocationType sceneCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{

   if (button == Qt::LeftButton)
   {
      mInserting = true;
      mInsertingDim = mSwitcher.mainDimension(sceneCoord);
      emit modified();
   }
   return true;
}

bool DimensionObjectImp::processMouseMove(LocationType sceneCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   if (buttons & Qt::LeftButton)
   {
      mInserting = true;
      mInsertingDim = mSwitcher.mainDimension(sceneCoord);
      emit modified();
   }
   return true;
}

bool DimensionObjectImp::processMouseRelease(LocationType sceneCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      processMouseMove(sceneCoord, button, buttons, modifiers);
      if (find(mSelectedDims.begin(), mSelectedDims.end(), mInsertingDim) == mSelectedDims.end())
      {
         int minMain;
         int maxMain;
         getMainExtents(minMain, maxMain);
         if (mInsertingDim >= minMain && mInsertingDim < maxMain)
         {
            // only insert if it's in the view
            mSelectedDims.push_back(mInsertingDim);
         }
      }
      mInserting = false;
      updateHandles();
      emit modified();
   }
   return true;
}

void DimensionObjectImp::updateHandles()
{
   GraphicObjectImp::updateHandles();

   if (mSelectedDims.empty())
   {
      return;
   }
   int minSec = 0;
   int maxSec = 0;
   getSecExtents(minSec, maxSec);
   if (minSec == maxSec)
   {
      return;
   }
   double midSec = (minSec + maxSec) / 2.0;
   for_each(mSelectedDims.begin(), mSelectedDims.end(), ::AddHandle(mSwitcher, mHandles, midSec));

   std::vector<int>::iterator minMain =
      min_element(mSelectedDims.begin(), mSelectedDims.end());
   std::vector<int>::iterator maxMain =
      max_element(mSelectedDims.begin(), mSelectedDims.end());

   setBoundingBox(mSwitcher.location(*minMain, minSec), mSwitcher.location(*maxMain+1, maxSec));
};

bool DimensionObjectImp::hasCornerHandles() const
{
   return false;
}

void DimensionObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   VERIFYNRV(static_cast<size_t>(handle) < mHandles.size());
 
   int dim = mSwitcher.mainDimension(pixel);
   int minMain;
   int maxMain;
   getMainExtents(minMain, maxMain);
   if (dim < minMain || dim > maxMain)
   {
      return;
   }
   
   mSelectedDims[handle] = mSwitcher.mainDimension(pixel);
   updateHandles();

   emit modified();
   
}

const BitMask* DimensionObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   if (mBitMaskDirty)
   {
      int startMain = mSwitcher.mainDimension(iStartColumn, iStartRow);
      int startSec = mSwitcher.secondDimension(iStartColumn, iStartRow);
      int endMain = mSwitcher.mainDimension(iEndColumn, iEndRow);
      int endSec = mSwitcher.secondDimension(iEndColumn, iEndRow);

      mPixelMask.clear();

      for (std::vector<int>::iterator iter = this->mSelectedDims.begin();
         iter != mSelectedDims.end(); ++iter)
      {
         int main = *iter;
         if (main < startMain || main > endMain)
         {
            continue;
         }
         for (int sec = startSec; sec <= endSec; ++sec)
         {
            int col = mSwitcher.x(main, sec);
            int row = mSwitcher.y(main, sec);
            mPixelMask.setPixel(col, row, true);
         }
      }
      mBitMaskDirty = false;
   }
   return &mPixelMask;
}

bool DimensionObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::toXml(pXml);
   if (bSuccess == true)
   {
      if (mSelectedDims.size() > 0)
      {
         DOMElement* pAllVertElement = pXml->addElement("vertices");
         if (pAllVertElement != NULL)
         {
            pXml->pushAddPoint(pAllVertElement);

            std::stringstream buf;
            for (unsigned int i = 0; i < mSelectedDims.size(); ++i)
            {
               buf.str("");

               DOMElement* pPixel = pXml->addElement("pixel", pAllVertElement);
               if (pPixel != NULL)
               {
                  buf << mSelectedDims[i];
                  pXml->addText(buf.str().c_str(), pPixel);
                  pAllVertElement->appendChild(pPixel);
               }
            }

            pXml->popAddPoint();
         }
      }
   }

   return bSuccess;
}

bool DimensionObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::fromXml(pDocument, version);
   if (bSuccess == true)
   {
      DOMNode* pObjectNode = pDocument->getFirstChild();
      while (pObjectNode != NULL)
      {
         if (XMLString::equals(pObjectNode->getNodeName(), X("vertices")))
         {
            for (DOMNode *path = pObjectNode->getFirstChild();
               path != NULL;
               path = path->getNextSibling())
            {
               std::string name(A(path->getNodeName()));
               if (XMLString::equals(path->getNodeName(), X("pixel")))
               {
                  DOMNode* pValue = path->getFirstChild();
                  unsigned int pixel = atoi(A(pValue->getNodeValue()));
                  mSelectedDims.push_back(pixel);
               }
            }
         }

         pObjectNode = pObjectNode->getNextSibling();
      }
   }
   updateHandles();
   emit modified();

   return bSuccess;
}

const string& DimensionObjectImp::getObjectType() const
{
   static string type("DimensionObjectImp");
   return type;
}

bool DimensionObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DimensionObject"))
   {
      return true;
   }

   return PixelObjectImp::isKindOf(className);
}

void DimensionObjectImp::setLayer(GraphicLayer *pLayer)
{
   GraphicObjectImp::setLayer(pLayer);
   updateHandles();
   emit modified();
}
