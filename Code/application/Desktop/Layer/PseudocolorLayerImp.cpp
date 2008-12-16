/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "DrawUtil.h"
#include "Icons.h"
#include "Image.h"
#include "ModelServices.h"
#include "PropertiesPseudocolorLayer.h"
#include "PseudocolorLayer.h"
#include "PseudocolorLayerImp.h"
#include "PseudocolorLayerUndo.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SessionManager.h"
#include "switchOnEncoding.h"
#include "SymbolRegionDrawer.h"
#include "Undo.h"
#include "View.h"
#include "xmlreader.h"

#include <algorithm>
#include <limits>

#if defined(CG_SUPPORTED)
#include "CgContext.h"
#include "GpuImage.h"
#endif

using namespace std;
XERCES_CPP_NAMESPACE_USE

template<class T>
class PixelOper
{
public:
   PixelOper(const T* pData, int rows, int cols, T value) :
      mpData(pData), mRows(rows), mCols(cols), mValue(value) {}

   inline bool operator()(int row, int col) const
   {
      int value = static_cast<int>(ModelServices::getDataValue(static_cast<T>(mpData[row * mCols + col]),
         COMPLEX_MAGNITUDE));
      return (value == static_cast<int>(mValue));
   }

private:
   const T* mpData;
   int mRows;
   int mCols;
   T mValue;
};

PseudocolorLayerImp::PseudocolorLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mNextID(0),
   mpImage(NULL)
{
   mpElement.addSignal(SIGNAL_NAME(RasterElement, DataModified),
      Slot(this, &PseudocolorLayerImp::rasterElementDataModified));

   setSymbol(PseudocolorLayer::getSettingMarkerSymbol());
   addPropertiesPage(PropertiesPseudocolorLayer::getName());

   // Setting up the icon.
   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mPseudocolorLayer);
   }

   VERIFYNR(connect(this, SIGNAL(modified()), this, SLOT(invalidateImage())));
}

PseudocolorLayerImp::~PseudocolorLayerImp()
{
   clear();
}

const string& PseudocolorLayerImp::getObjectType() const
{
   static string sType("PseudocolorLayerImp");
   return sType;
}

bool PseudocolorLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PseudocolorLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

void PseudocolorLayerImp::rasterElementDataModified(Subject& subject, const string& signal, const boost::any& v)
{
   invalidateImage();
}

PseudocolorLayerImp& PseudocolorLayerImp::operator= (const PseudocolorLayerImp& pseudocolorLayer)
{
   if (this != &pseudocolorLayer)
   {
      LayerImp::operator =(pseudocolorLayer);
      clear();

      QMap<int, PseudocolorClass*>::ConstIterator iter = pseudocolorLayer.mClasses.begin();
      while (iter != pseudocolorLayer.mClasses.end())
      {
         PseudocolorClass* pClass = iter.value();
         if (pClass != NULL)
         {
            QString strClassName = pClass->getName();
            int iValue = pClass->getValue();
            QColor clrClass = pClass->getColor();
            bool bDisplayed = pClass->isDisplayed();

            addClass(strClassName, iValue, clrClass, bDisplayed);
         }

         ++iter;
      }

      mSymbol = pseudocolorLayer.mSymbol;
   }

   return *this;
}

LayerType PseudocolorLayerImp::getLayerType() const
{
   return PSEUDOCOLOR;
}

vector<ColorType> PseudocolorLayerImp::getColors() const
{
   vector<ColorType> colors;

   vector<PseudocolorClass*> classes = getAllClasses();
   for (unsigned int i = 0; i < classes.size(); ++i)
   {
      PseudocolorClass* pClass = classes[i];
      if (pClass != NULL)
      {
         QColor currentColor = pClass->getColor();
         if (currentColor.isValid() == true)
         {
            ColorType color(currentColor.red(), currentColor.green(), currentColor.blue());
            colors.push_back(color);
         }
      }
   }

   return colors;
}

template<class T>
void drawPseudocolorMarkers(T* pData, int stopColumn, int stopRow, int visStartColumn, int visStartRow,
                            int visEndColumn, int visEndRow, SymbolType eSymbol, QColor clrMarker, 
                            double value, int row = -1)
{
   if (row < 0) // in memory so process all rows
   {
      PixelOper<T> oper(pData, stopRow + 1, stopColumn + 1, static_cast<T>(value));
      SymbolRegionDrawer::drawMarkers(0, 0, stopColumn, stopRow, visStartColumn, visStartRow, visEndColumn,
         visEndRow, eSymbol, clrMarker, oper);
   }
   else // on disk so being processed one row at a time
   {
      PixelOper<T> oper(pData, 1, 0, static_cast<T>(value));
      SymbolRegionDrawer::drawMarkers(0, row, stopColumn, row, visStartColumn, visStartRow, visEndColumn,
         visEndRow, eSymbol, clrMarker, oper);
   }
}

void PseudocolorLayerImp::draw()
{
   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   if (pRasterElement != NULL)
   {
      if (canRenderAsImage())
      {
         generateImage();
         VERIFYNRV(mpImage != NULL);

         mpImage->draw(GL_NEAREST);
      }
      else
      {
         DataAccessor accessor(NULL, NULL);
         bool usingRawData = false;

         int columns = 0;
         int rows = 0;
         EncodingType eType;
         void* pData = NULL;

         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            columns = static_cast<int>(pDescriptor->getColumnCount());
            rows = static_cast<int>(pDescriptor->getRowCount());
            eType = pDescriptor->getDataType();
         }

         // There is an optimization when the full scene can be processed at once.
         // Check for if it can be done
         if (pDescriptor->getBandCount() == 1 || pDescriptor->getInterleaveFormat() == BSQ)
         {
            usingRawData = true;
            pData = pRasterElement->getRawData();
         }
         if (pData == NULL)
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setInterleaveFormat(BSQ);
            accessor = pRasterElement->getDataAccessor(pRequest.release());
         }

         SymbolType eSymbol = getSymbol();

         int visStartColumn = 0;
         int visEndColumn = columns - 1;
         int visStartRow = 0;
         int visEndRow = rows - 1;
         DrawUtil::restrictToViewport(visStartColumn, visStartRow, visEndColumn, visEndRow);

         QMap<int, PseudocolorClass*>::Iterator iter = mClasses.begin();
         while (iter != mClasses.end())
         {
            PseudocolorClass* pClass = iter.value();
            if (pClass != NULL)
            {
               if (pClass->isDisplayed())
               {
                  QColor clrMarker = pClass->getColor();

                  if (usingRawData) // all data in memory
                  {
                     switchOnEncoding(eType, drawPseudocolorMarkers, pData, columns - 1, rows - 1, visStartColumn,
                        visStartRow, visEndColumn, visEndRow, eSymbol, clrMarker, pClass->getValue());
                  }
                  else
                  {
                     for (int row = 0; row < rows; ++row)
                     {
                        if (!accessor.isValid())
                        {
                           break;
                        }

                        pData = accessor->getColumn();
                        switchOnEncoding(eType, drawPseudocolorMarkers, pData, columns - 1, row, visStartColumn,
                           row, visEndColumn, row, eSymbol, clrMarker, pClass->getValue(), row);
                        accessor->nextRow();
                     }
                     accessor->toPixel(0, 0);
                  }
               }
            }

            iter++;
         }
      }
   }
}

bool PseudocolorLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   VERIFY(pRasterElement != NULL);

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   translateDataToWorld(0, 0, x1, y1);
   translateDataToWorld(pDescriptor->getColumnCount(), pDescriptor->getRowCount(), x4, y4);

   return true;
}

PseudocolorClass* PseudocolorLayerImp::addClass()
{
   PseudocolorLayer* pLayer = dynamic_cast<PseudocolorLayer*>(this);
   VERIFYRV(pLayer != NULL, NULL);

   PseudocolorClass* pClass = new PseudocolorClass(pLayer);
   if (pClass != NULL)
   {
      insertClass(pClass);
   }

   return pClass;
}

PseudocolorClass* PseudocolorLayerImp::addClass(const QString& strClass, int iValue, const QColor& clrClass,
                                                bool bDisplayed)
{
   UndoGroup group(getView(), "Add Pseudocolor Class");

   PseudocolorClass* pClass = addClass();
   if (pClass != NULL)
   {
      pClass->setClassName(strClass);
      pClass->setValue(iValue);
      pClass->setColor(clrClass);
      pClass->setDisplayed(bDisplayed);
   }

   return pClass;
}

void PseudocolorLayerImp::insertClass(PseudocolorClass* pClass, int iID)
{
   if (pClass != NULL)
   {
      connect(pClass, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));
      connect(pClass, SIGNAL(nameChanged(const QString&)), this, SIGNAL(modified()));
      connect(pClass, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
      connect(pClass, SIGNAL(displayStateChanged(bool)), this, SIGNAL(modified()));

      if (iID == -1)
      {
         iID = mNextID;
         mNextID++;
      }

      mClasses.insert(iID, pClass);

      // Add the undo action after inserting the class into the map to ensure the action has the correct class ID
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new AddPseudocolorClass(dynamic_cast<PseudocolorLayer*>(this), pClass));
      }

      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

PseudocolorClass* PseudocolorLayerImp::getClass(int iValue) const
{
   QMap<int, PseudocolorClass*>::ConstIterator iter;
   iter = mClasses.begin();
   while (iter != mClasses.end())
   {
      PseudocolorClass* pClass = iter.value();
      if (pClass != NULL)
      {
         int iCurrentValue = -1;
         iCurrentValue = pClass->getValue();
         if (iCurrentValue == iValue)
         {
            return pClass;
         }
      }

      iter++;
   }

   return NULL;
}

PseudocolorClass* PseudocolorLayerImp::getClassById(int iID) const
{
   PseudocolorClass* pClass = NULL;

   QMap<int, PseudocolorClass*>::ConstIterator iter;
   iter = mClasses.find(iID);
   if (iter != mClasses.end())
   {
      pClass = iter.value();
   }

   return pClass;
}

int PseudocolorLayerImp::getClassID(PseudocolorClass* pClass) const
{
   if (pClass == NULL)
   {
      return -1;
   }

   QMap<int, PseudocolorClass*>::ConstIterator iter;
   iter = mClasses.begin();
   while (iter != mClasses.end())
   {
      PseudocolorClass* pCurrentClass = iter.value();
      if (pCurrentClass == pClass)
      {
         int iID = -1;
         iID = iter.key();

         return iID;
      }

      iter++;
   }

   return -1;
}

vector<PseudocolorClass*> PseudocolorLayerImp::getAllClasses() const
{
   vector<PseudocolorClass*> classList;

   QMap<int, PseudocolorClass*>::const_iterator iter = mClasses.constBegin();
   while (iter != mClasses.constEnd())
   {
      PseudocolorClass* pClass = iter.value();
      if (pClass != NULL)
      {
         classList.push_back(pClass);
      }

      iter++;
   }

   return classList;
}

unsigned int PseudocolorLayerImp::getClassCount() const
{
   return mClasses.count();
}

bool PseudocolorLayerImp::removeClass(PseudocolorClass* pClass)
{
   if (pClass == NULL)
   {
      return false;
   }

   bool bRemoved = false;

   QMap<int, PseudocolorClass*>::Iterator iter;
   iter = mClasses.begin();
   while (iter != mClasses.end())
   {
      PseudocolorClass* pCurrentClass = iter.value();
      if (pCurrentClass == pClass)
      {
         disconnect(pClass, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));
         disconnect(pClass, SIGNAL(nameChanged(const QString&)), this, SIGNAL(modified()));
         disconnect(pClass, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
         disconnect(pClass, SIGNAL(displayStateChanged(bool)), this, SIGNAL(modified()));

         // Add the undo action before removing the class from the map to ensure the action has the correct class ID
         View* pView = getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new DeletePseudocolorClass(dynamic_cast<PseudocolorLayer*>(this), pClass));
         }

         mClasses.erase(iter);
         bRemoved = true;
         break;
      }

      iter++;
   }

   if (bRemoved == true)
   {
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }

   delete pClass;
   return bRemoved;
}

void PseudocolorLayerImp::clear()
{
   unsigned int uiCount = 0;
   uiCount = mClasses.count();
   if (uiCount == 0)
   {
      return;
   }

   View* pView = getView();
   UndoGroup group(pView, "Clear Pseudocolor Classes");

   QMap<int, PseudocolorClass*>::Iterator iter;
   iter = mClasses.begin();
   while (iter != mClasses.end())
   {
      PseudocolorClass* pClass = iter.value();
      if (pClass != NULL)
      {
         if (pView != NULL)
         {
            pView->addUndoAction(new DeletePseudocolorClass(dynamic_cast<PseudocolorLayer*>(this), pClass));
         }

         delete pClass;
      }

      iter++;
   }

   mClasses.clear();

   invalidateImage();

   emit modified();
   notify(SIGNAL_NAME(PseudocolorLayer, Cleared));
}

bool PseudocolorLayerImp::toXml(XMLWriter* pXml) const
{
   if (!LayerImp::toXml(pXml))
   {
      return false;
   }

   if (Service<SessionManager>()->isSessionSaving())
   {
      pXml->addAttr("symbol", mSymbol);
   }

   if (mClasses.size() > 0)
   {
      QMap<int, PseudocolorClass*>::ConstIterator it;
      for (it = mClasses.begin(); it != mClasses.end(); ++it)
      {
         PseudocolorClass* pClass = it.value();
         if (pClass != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("PseudocolorClass"));
            pXml->addAttr("color", QCOLOR_TO_COLORTYPE(pClass->getColor()));
            pXml->addAttr("displayed", pClass->isDisplayed());
            pXml->addAttr("name", pClass->getName().toStdString());
            pXml->addAttr("value", pClass->getValue());
            pXml->popAddPoint();
         }
      }
   }

   return true;
}

bool PseudocolorLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!LayerImp::fromXml(pDocument, version))
   {
      return false;
   }
   mClasses.clear();
 
   DOMElement* pTop = static_cast<DOMElement*>(pDocument);
   if (Service<SessionManager>()->isSessionLoading())
   {
      mSymbol = StringUtilities::fromXmlString<SymbolType>(A(pTop->getAttribute(X("symbol"))));
   }
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("PseudocolorClass")))
      {
         DOMElement* pElmnt = static_cast<DOMElement*>(pChld);
         int value = StringUtilities::fromXmlString<int>(A(pElmnt->getAttribute(X("value"))));
         QString name(A(pElmnt->getAttribute(X("name"))));
         bool displayed = StringUtilities::fromXmlString<bool>(A(pElmnt->getAttribute(X("displayed"))));
         ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElmnt->getAttribute(X("color"))));
         QColor clrClass = COLORTYPE_TO_QCOLOR(color);

         if (addClass(name, value, clrClass, displayed) == NULL)
         {
            return false;
         }
      }
   }

   return true;
}

void PseudocolorLayerImp::getBoundingBox(int& x1, int& y1, int& x2, int& y2) const
{
   x1 = 0;
   y1 = 0;
   x2 = 0;
   y2 = 0;

   const RasterElement* pRasterElement = dynamic_cast<const RasterElement*>(getDataElement());
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         x2 = pDescriptor->getColumnCount() - 1;
         y2 = pDescriptor->getRowCount() - 1;
      }
   }
}

template<class T, class Drawer>
void fillRegion(T* pData, DataAccessor& da, Drawer drawer, int classValue, unsigned int numRows,
                unsigned int numColumns)
{
   for (unsigned int uiRow = 0; uiRow < numRows; ++uiRow)
   {
      for (unsigned int uiColumn = 0; uiColumn < numColumns; ++uiColumn)
      {
         VERIFYNRV(da.isValid());
         int iValue = ModelServices::getDataValue(*((T*)da->getColumn()), COMPLEX_MAGNITUDE);
         if (classValue == iValue)
         {
            drawer(uiColumn, uiRow);
         }

         da->nextColumn();
      }

      da->nextRow();
   }
}

const BitMask* PseudocolorLayerImp::getSelectedPixels() const
{
   const RasterElement* pRasterElement = dynamic_cast<const RasterElement*>(getDataElement());
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         unsigned int uiNumColumns = pDescriptor->getColumnCount();
         unsigned int uiNumRows = pDescriptor->getRowCount();
         EncodingType eEncoding = pDescriptor->getDataType();

         DataAccessor da = pRasterElement->getDataAccessor();
         if (da.isValid() == false)
         {
            return NULL;
         }

         mpMask->clear();
         DrawUtil::BitMaskPixelDrawer drawer(mpMask.get());

         QMap<int, PseudocolorClass*>::const_iterator iter = mClasses.begin();
         while (iter != mClasses.end())
         {
            PseudocolorClass* pClass = iter.value();
            if (pClass != NULL)
            {
               if (pClass->isDisplayed())
               {
                  int classValue = pClass->getValue();
                  da->toPixel(0, 0);
                  switchOnEncoding(eEncoding, fillRegion, NULL, da, drawer, classValue, uiNumRows, uiNumColumns);
               }
            }
            ++iter;
         }
      }
   }

   return mpMask.get();
}

void PseudocolorLayerImp::reset()
{
   setSymbol(PseudocolorLayer::getSettingMarkerSymbol());
}

void PseudocolorLayerImp::setSymbol(SymbolType symbol)
{
   if (symbol != mSymbol)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetPseudocolorSymbol(dynamic_cast<PseudocolorLayer*>(this), mSymbol, symbol));
      }

      mSymbol = symbol;
      emit modified();
      notify(SIGNAL_NAME(PseudocolorLayer, SymbolChanged), boost::any(symbol));
   }
}

SymbolType PseudocolorLayerImp::getSymbol() const
{
   return mSymbol;
}

/**
 *  Gets the range of values for which classes are defined.
 *
 *  Gets the range of values for which classes are defined, possibly limiting
 *  its scan to displayed classes. If there are no classes (or no displayed
 *  classes) it returns (0,-1). Otherwise it returns (min,max).
 *
 * @param onlyDisplayed
 *          If true, only displayed classes will be considered
 *
 *  @return  The range of class values or (0,-1) if there are no classes
 */
pair<int,int> PseudocolorLayerImp::getValueRange(bool onlyDisplayed) const
{
   int minValue = numeric_limits<int>::max();
   int maxValue = numeric_limits<int>::min();

   QMap<int, PseudocolorClass*>::ConstIterator iter = mClasses.constBegin();
   while (iter != mClasses.constEnd())
   {
      PseudocolorClass* pClass = iter.value();
      if (pClass != NULL)
      {
         if (!onlyDisplayed || pClass->isDisplayed())
         {
            int value = pClass->getValue();
            minValue = min(minValue, value);
            maxValue = max(maxValue, value);
         }
      }
      iter++;
   }

   if (minValue == numeric_limits<int>::max() && maxValue == numeric_limits<int>::min())
   {
      return pair<int, int>(0, -1);
   }

   return pair<int, int>(minValue, maxValue);
}

bool PseudocolorLayerImp::canRenderAsImage() const
{
   bool canBeImage = true;
   if (getSymbol() != SOLID)
   {
      canBeImage = false;
   }

   // there must be displayed classes and the range of values must
   // be low enough to prevent an unreasonably large colormap.
   pair<int, int> range = getValueRange(true);
   if (range.first == 0 && range.second == -1 ||   // no displayed classes
      range.second - range.first > 1000000)        // too many colors for a colormap
   {
      canBeImage = false;
   }

   return canBeImage;
}

void PseudocolorLayerImp::generateImage()
{
   RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      if (!VERIFYNR(mpImage == NULL))
      {
         invalidateImage();
      }
      return;
   }

   if (mpImage == NULL)
   {
#if defined(WIN_API)
      bool useGpuImage = RasterLayer::getSettingGpuImage();
      if (useGpuImage)
      {
         mpImage = new GpuImage();
      }
#endif

      if (mpImage == NULL) // not using gpu image
      {
         mpImage = new Image();
      }

      VERIFYNRV (mpImage != NULL);

      vector<int> badValues; // empty - the classes already determine which values to display
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      VERIFYNRV(pDescriptor != NULL);
      unsigned int uiRows = pDescriptor->getRowCount();
      unsigned int uiColumns = pDescriptor->getColumnCount();
      EncodingType eEncoding = pDescriptor->getDataType();
      vector<double> lstStretchValues;
      pair<int, int> valueRange = getValueRange(true); // first = min, second = max

      if (valueRange.first == -1 && valueRange.second == 0) // no classes displayed!
      {
         invalidateImage();
         return;
      }

      // +1 to be inclusive, +2 to put a transparent entry at each end
      vector<ColorType> colorMap(valueRange.second-valueRange.first+1+2); 
      for (unsigned int i = 0; i < colorMap.size(); ++i)
      {
         colorMap[i].mAlpha = 0;
         colorMap[i].mRed = 0;
         colorMap[i].mGreen = 0;
         colorMap[i].mBlue = 0;
      }

      // set the displayed classes into the colormap
      QMap<int, PseudocolorClass*>::Iterator iter = mClasses.begin();
      while (iter != mClasses.end())
      {
         PseudocolorClass* pClass = NULL;
         pClass = iter.value();
         if (pClass != NULL)
         {
            if (pClass->isDisplayed())
            {
               int value = pClass->getValue();
               QColor clrMarker = pClass->getColor();
               ColorType color(clrMarker.red(), clrMarker.green(), clrMarker.blue());
               colorMap[value-valueRange.first+1] = color;
            }
         }
         iter++;
      }

      // +/-1 for the transparent entry, +/-0.5 for half the width of an entry
      lstStretchValues.push_back(valueRange.first-1.0-0.5); 
      lstStretchValues.push_back(valueRange.second+1+0.5);
      GLenum format = GL_RGBA;
      DimensionDescriptor bandDim = pDescriptor->getActiveBand(0);
      mpImage->initialize(512, 512, bandDim, uiColumns, uiRows, 1, format, eEncoding, COMPLEX_MAGNITUDE, NULL,
         LINEAR, lstStretchValues, pRaster, colorMap, badValues);
   }
}

void PseudocolorLayerImp::invalidateImage()
{
   if (mpImage != NULL)
   {
      delete mpImage;
      mpImage = NULL;
   }
}

bool PseudocolorLayerImp::isGpuImageSupported() const
{
#ifdef WIN_API
   // Check if the hardware supports the GPU image
   if (CgContext::instance() == NULL)
   {
      return false;
   }

   // The GPU image does not currently support complex data
   const DataElement* pElement = getDataElement();
   if (pElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      VERIFY(pDescriptor != NULL);

      EncodingType eEncoding = pDescriptor->getDataType();
      if ((eEncoding != INT4SCOMPLEX) || (eEncoding != FLT8COMPLEX))
      {
         return true;
      }
   }
#endif

   return false;
}
