/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "BitMask.h"
#include "DataAccessorImpl.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "MathUtil.h"
#include "ModelServices.h"
#include "PropertiesThresholdLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "SymbolRegionDrawer.h"
#include "ThresholdLayer.h"
#include "ThresholdLayerImp.h"
#include "ThresholdLayerUndo.h"
#include "UtilityServices.h"
#include "View.h"

#include <vector>
#include <algorithm>
using namespace std;
XERCES_CPP_NAMESPACE_USE

unsigned int ThresholdLayerImp::msThresholdLayers = 0;

template<class T>
class ThresholdPixelOper
{
public:
   ThresholdPixelOper(const T* pData, int rows, int cols, double lower, double upper, PassArea passArea,
             const vector<int>& badValues = vector<int>()) :
      mpData(pData),
      mRows(rows),
      mCols(cols),
      mLower(lower),
      mUpper(upper),
      mPassArea(passArea),
      mBadValues(badValues)
   {}

   inline bool operator()(int row, int col) const
   {
      double value = ModelServices::getDataValue(*const_cast<T*>(mpData + (row * mCols + col)), COMPLEX_MAGNITUDE);

      bool passed = false;
      switch (mPassArea)
      {
      case LOWER:
         if (value <= mLower)
         {
            passed = true;
         }
         break;
      case UPPER:
         if (value >= mLower)
         {
            passed = true;
         }
         break;
      case MIDDLE:
         if ((value >= mLower) && (value <= mUpper))
         {
            passed = true;
         }
         break;
      case OUTSIDE:
         if ((value <= mLower) || (value >= mUpper))
         {
            passed = true;
         }
         break;
      default:
         break;
      }

      if (passed)
      {
         if (find(mBadValues.begin(), mBadValues.end(), roundDouble(value)) != mBadValues.end())
         {
            return false;
         }
      }
      return passed;
   }

private:
   const T* mpData;
   int mRows;
   int mCols;
   double mLower;
   double mUpper;
   PassArea mPassArea;
   vector<int> mBadValues;
};

ThresholdLayerImp::ThresholdLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement)
{
   mbModified = true;

   bool autoColorOn = ThresholdLayer::getSettingAutoColor();
   if (autoColorOn == true)
   {
      Service<UtilityServices> pUtilities;
      ColorType color = pUtilities->getAutoColor(msThresholdLayers);
      mColor = COLORTYPE_TO_QCOLOR(color);
   }
   else
   {
      ColorType color = ThresholdLayer::getSettingMarkerColor();
      mColor = COLORTYPE_TO_QCOLOR(color);
   }

   mSymbol = ThresholdLayer::getSettingMarkerSymbol();
   meRegionUnits = ThresholdLayer::getSettingRegionUnits();
   mePassArea = ThresholdLayer::getSettingPassArea();
   mdFirstThreshold = convertThreshold(meRegionUnits, ThresholdLayer::getSettingFirstValue(), RAW_VALUE);
   mdSecondThreshold = convertThreshold(meRegionUnits, ThresholdLayer::getSettingSecondValue(), RAW_VALUE);

   addPropertiesPage(PropertiesThresholdLayer::getName());

   // Setting up the icon.
   setIcon(QIcon(":/icons/ThresholdLayer"));

   VERIFYNR(connect(this, SIGNAL(firstThresholdChanged(double)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(secondThresholdChanged(double)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(passAreaChanged(const PassArea&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(regionUnitsChanged(const RegionUnits&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(symbolChanged(SymbolType)), this, SIGNAL(modified())));

   msThresholdLayers++;
}

ThresholdLayerImp::~ThresholdLayerImp()
{}

const string& ThresholdLayerImp::getObjectType() const
{
   static string sType("ThresholdLayerImp");
   return sType;
}

bool ThresholdLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ThresholdLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

bool ThresholdLayerImp::isKindOfLayer(const string& className)
{
   if ((className == "ThresholdLayerImp") || (className == "ThresholdLayer"))
   {
      return true;
   }

   return LayerImp::isKindOfLayer(className);
}

void ThresholdLayerImp::getLayerTypes(vector<string>& classList)
{
   classList.push_back("ThresholdLayer");
   LayerImp::getLayerTypes(classList);
}

ThresholdLayerImp& ThresholdLayerImp::operator=(const ThresholdLayerImp& thresholdLayer)
{
   if (this != &thresholdLayer)
   {
      LayerImp::operator =(thresholdLayer);

      meRegionUnits = thresholdLayer.meRegionUnits;
      mePassArea = thresholdLayer.mePassArea;
      mdFirstThreshold = thresholdLayer.mdFirstThreshold;
      mdSecondThreshold = thresholdLayer.mdSecondThreshold;
      mColor = thresholdLayer.mColor;
      mSymbol = thresholdLayer.mSymbol;
   }

   return *this;
}

LayerType ThresholdLayerImp::getLayerType() const
{
   return THRESHOLD;
}

vector<ColorType> ThresholdLayerImp::getColors() const
{
   vector<ColorType> colors;

   QColor color = getColor();
   if (color.isValid())
   {
      colors.push_back(ColorType(color.red(), color.green(), color.blue()));
   }

   return colors;
}

template<class T>
void drawMarkers(T* pData, int stopColumn, int stopRow, int visStartColumn, int visStartRow, int visEndColumn,
                 int visEndRow, SymbolType eSymbol, QColor clrMarker, double lower, double upper, PassArea passArea,
                 const vector<int>& badValues, int row = -1)
{
   if (row < 0) // in memory so process all rows
   {
      ThresholdPixelOper<T> oper(pData, stopRow + 1, stopColumn + 1, lower, upper, passArea, badValues);
      SymbolRegionDrawer::drawMarkers(0, 0, stopColumn, stopRow, visStartColumn, visStartRow, visEndColumn,
         visEndRow, eSymbol, clrMarker, oper);
   }
   else // on disk so being processed one row at a time
   {
      ThresholdPixelOper<T> oper(pData, 1, 0, lower, upper, passArea, badValues);
      SymbolRegionDrawer::drawMarkers(0, row, stopColumn, row, visStartColumn, visStartRow, visEndColumn,
         visEndRow, eSymbol, clrMarker, oper);
   }
}

void ThresholdLayerImp::draw()
{
   DataAccessor accessor(NULL, NULL);

   DataElement* pElement = getDataElement();
   if (pElement != NULL)
   {
      int columns = 0;
      int rows = 0;
      void* pData = NULL;
      EncodingType eType;
      vector<int> badValues;

      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         columns = static_cast<int>(pDescriptor->getColumnCount());
         rows = static_cast<int>(pDescriptor->getRowCount());
         eType = pDescriptor->getDataType();
      }

      RasterElement* pRasterElement = dynamic_cast<RasterElement*>(pElement);
      if (pRasterElement != NULL)
      {
         Statistics* pStatistics = pRasterElement->getStatistics();
         if (pStatistics != NULL)
         {
            badValues = pStatistics->getBadValues();
         }

         if (pDescriptor->getBandCount() == 1 || pDescriptor->getInterleaveFormat() == BSQ)
         {
            pData = pRasterElement->getRawData();
         }
         if (pData == NULL)
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setInterleaveFormat(BSQ);
            accessor = pRasterElement->getDataAccessor(pRequest.release());
         }
      }

      SymbolType eSymbol = getSymbol();
      QColor clrMarker = mColor;

      int visStartColumn = 0;
      int visEndColumn = columns - 1;
      int visStartRow = 0;
      int visEndRow = rows - 1;

      DrawUtil::restrictToViewport(visStartColumn, visStartRow, visEndColumn, visEndRow);

      double firstThreshold = mdFirstThreshold;
      double secondThreshold = mdSecondThreshold;

      if (pData == NULL)
      {
         for (int row = 0; row < rows; ++row)
         {
            if (!accessor.isValid())
            {
               break;
            }

            pData = accessor->getColumn();
            switchOnEncoding(eType, drawMarkers, pData, columns - 1, row, visStartColumn, row, visEndColumn,
               row, eSymbol, clrMarker, firstThreshold, secondThreshold, mePassArea, badValues, row);
            accessor->nextRow();
         }
      }
      else
      {
         switchOnEncoding(eType, drawMarkers, pData, columns - 1, rows - 1, visStartColumn, visStartRow,
            visEndColumn, visEndRow, eSymbol, clrMarker, firstThreshold, secondThreshold, mePassArea, badValues);
      }
   }
}

bool ThresholdLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   DataElement* pElement = getDataElement();
   VERIFY(pElement != NULL);

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   translateDataToWorld(0, 0, x1, y1);
   translateDataToWorld(pDescriptor->getColumnCount(), pDescriptor->getRowCount(),
      x4, y4);

   return true;
}

RegionUnits ThresholdLayerImp::getRegionUnits() const
{
   return meRegionUnits;
}

PassArea ThresholdLayerImp::getPassArea() const
{
   return mePassArea;
}

double ThresholdLayerImp::getFirstThreshold() const
{
   return mdFirstThreshold;
}

double ThresholdLayerImp::getSecondThreshold() const
{
   return mdSecondThreshold;
}

QString ThresholdLayerImp::getRegionUnitsAsString() const
{
   switch (meRegionUnits)
   {
      case RAW_VALUE:
         break;

      case PERCENTAGE:
         return "%age";

      case PERCENTILE:
         return "%ile";

      case STD_DEV:
      {
         QChar tmpchar = QChar(0x03C3);    // Unicode sigma
         QString tmpstr;
         tmpstr.setUnicode(&tmpchar, 1);
         return tmpstr;
      }

      default:
         break;
   }

   return QString();
}

double ThresholdLayerImp::convertThreshold(const RegionUnits& eUnits, double dThreshold,
                                           const RegionUnits& eNewUnits) const
{
   double dNewThreshold = 0.0;

   Statistics* pStatistics = getStatistics(GRAY);
   if (pStatistics == NULL)
   {
      return dNewThreshold;
   }

   double dMin = pStatistics->getMin();
   double dMax = pStatistics->getMax();
   double dAverage = pStatistics->getAverage();
   double dStdDev = pStatistics->getStandardDeviation();
   const double* pdPercentiles = pStatistics->getPercentiles();

   double dRawValue = 0.0;

   // Convert the threshold value to a raw threshold value
   switch (eUnits)
   {
      case RAW_VALUE:
         dRawValue = dThreshold;
         break;

      case PERCENTAGE:
         dRawValue = (((dMax - dMin) * dThreshold) / 100) + dMin;
         break;

      case PERCENTILE:
         dRawValue = percentileToRaw(dThreshold, pdPercentiles);
         break;

      case STD_DEV:
         dRawValue = (dThreshold * dStdDev) + dAverage;
         break;

      default:
         break;
   }

   // Convert the raw threshold value to the new value
   switch (eNewUnits)
   {
      case RAW_VALUE:
         dNewThreshold = dRawValue;
         break;

      case PERCENTAGE:
         dNewThreshold = (((dRawValue - dMin) / (dMax - dMin)) * 100);
         break;

      case PERCENTILE:
         dNewThreshold = rawToPercentile(dRawValue, pdPercentiles);
         break;

      case STD_DEV:
         if (dStdDev != 0.0)
         {
            dNewThreshold = (dRawValue - dAverage) / dStdDev;
         }
         break;

      default:
         break;
   }

   return dNewThreshold;
}

double ThresholdLayerImp::convertThreshold(double dThreshold, const RegionUnits& eNewUnits) const
{
   return convertThreshold(meRegionUnits, dThreshold, eNewUnits);
}

void ThresholdLayerImp::setRegionUnits(const RegionUnits& eUnits)
{
   if (eUnits == meRegionUnits)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetThresholdUnits(dynamic_cast<ThresholdLayer*>(this), meRegionUnits, eUnits));
      }

      meRegionUnits = eUnits;
      mbModified = true;
      emit regionUnitsChanged(meRegionUnits);
      notify(SIGNAL_NAME(ThresholdLayer, UnitsChanged), boost::any(meRegionUnits));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setRegionUnits(eUnits);
         }
      }

      mbLinking = false;
   }
}

void ThresholdLayerImp::setPassArea(const PassArea& eArea)
{
   if (eArea == mePassArea)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetThresholdPassArea(dynamic_cast<ThresholdLayer*>(this), mePassArea, eArea));
      }

      mePassArea = eArea;
      mbModified = true;
      emit passAreaChanged(mePassArea);
      notify(SIGNAL_NAME(ThresholdLayer, PassAreaChanged), boost::any(mePassArea));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setPassArea(eArea);
         }
      }

      mbLinking = false;
   }
}

void ThresholdLayerImp::setFirstThreshold(double dRawValue)
{
   if (dRawValue == mdFirstThreshold)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetThresholdValues(dynamic_cast<ThresholdLayer*>(this), mdFirstThreshold,
            dRawValue, mdSecondThreshold, mdSecondThreshold));
      }

      mdFirstThreshold = dRawValue;
      mbModified = true;
      emit firstThresholdChanged(mdFirstThreshold);
      notify(SIGNAL_NAME(ThresholdLayer, FirstThresholdChanged), boost::any(mdFirstThreshold));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setFirstThreshold(dRawValue);
         }
      }

      mbLinking = false;
   }
}

void ThresholdLayerImp::setSecondThreshold(double dRawValue)
{
   if (dRawValue == mdSecondThreshold)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetThresholdValues(dynamic_cast<ThresholdLayer*>(this), mdFirstThreshold,
            mdFirstThreshold, mdSecondThreshold, dRawValue));
      }

      mdSecondThreshold = dRawValue;
      mbModified = true;
      emit secondThresholdChanged(mdSecondThreshold);
      notify(SIGNAL_NAME(ThresholdLayer, SecondThresholdChanged), boost::any(mdSecondThreshold));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setSecondThreshold(dRawValue);
         }
      }

      mbLinking = false;
   }
}

void ThresholdLayerImp::reset()
{
   ColorType color = ThresholdLayer::getSettingMarkerColor();
   QColor clrThreshold = COLORTYPE_TO_QCOLOR(color);

   bool autoColorOn = ThresholdLayer::getSettingAutoColor();
   if (autoColorOn == true)
   {
      Service<UtilityServices> pUtilities;
      ColorType autoColor = pUtilities->getAutoColor(msThresholdLayers);
      clrThreshold = COLORTYPE_TO_QCOLOR(autoColor);
   }

   setColor(clrThreshold);
   setSymbol(ThresholdLayer::getSettingMarkerSymbol());
   setRegionUnits(ThresholdLayer::getSettingRegionUnits());
   setPassArea(ThresholdLayer::getSettingPassArea());
   setFirstThreshold(convertThreshold(ThresholdLayer::getSettingRegionUnits(),
      ThresholdLayer::getSettingFirstValue(), RAW_VALUE));
   setSecondThreshold(convertThreshold(ThresholdLayer::getSettingRegionUnits(),
      ThresholdLayer::getSettingSecondValue(), RAW_VALUE));
}

Statistics* ThresholdLayerImp::getStatistics(RasterChannelType eColor) const
{
   Statistics* pStatistics = NULL;

   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   if (pRasterElement != NULL)
   {
      pStatistics = pRasterElement->getStatistics();
   }

   return pStatistics;
}

double ThresholdLayerImp::percentileToRaw(double value, const double* pdPercentiles) const
{
   if (pdPercentiles == NULL)
   {
      return 0.0;
   }

   if (value < 0.0 || value > 100.0)
   {
      return pdPercentiles[0] + value * (pdPercentiles[1000] - pdPercentiles[0]) / 100.0;
   }

   int lower = static_cast<int>(10.0 * value);
   if (lower < 0)
   {
      return pdPercentiles[0];
   }
   if (lower > 999)
   {
      return pdPercentiles[1000];
   }

   return pdPercentiles[lower] + (pdPercentiles[lower + 1] - pdPercentiles[lower]) *
      (10.0 * value - static_cast<double>(lower));
}

double ThresholdLayerImp::rawToPercentile(double value, const double* pdPercentiles) const
{
   if (pdPercentiles == NULL)
   {
      return -1.0;
   }

   if (value < pdPercentiles[0] || value > pdPercentiles[1000])
   {
      return 100.0 * (value - pdPercentiles[0]) / (pdPercentiles[1000] - pdPercentiles[0]);
   }

   int i = 0;
   for (i = 0; i < 1000; ++i)
   {
      if (pdPercentiles[i] >= value)
      {
         break;
      }
   }

   int lower = i - 1;
   if (lower < 0)
   {
      return 0.0;
   }
   if (lower > 999)
   {
      return 100.0;
   }

   return (lower + (value - pdPercentiles[lower]) / (pdPercentiles[lower + 1] - pdPercentiles[lower])) / 10.0;
}

void ThresholdLayerImp::getBoundingBox(int& x1, int& y1, int& x2, int& y2) const
{
   x1 = 0;
   y1 = 0;
   x2 = 0;
   y2 = 0;

   DataElement* pElement = getDataElement();
   if (pElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         x2 = pDescriptor->getColumnCount() - 1;
         y2 = pDescriptor->getRowCount() - 1;
      }
   }
}

template<class T, class Drawer>
void fillRegion(T* pData, DataAccessor& da, Drawer drawer, double firstThreshold, double secondThreshold,
                unsigned int numRows, unsigned int numColumns, PassArea passArea, const vector<int>& badValues)
{
   for (unsigned int uiRow = 0; uiRow < numRows; ++uiRow)
   {
      for (unsigned int uiColumn = 0; uiColumn < numColumns; ++uiColumn)
      {
         double value = ModelServices::getDataValue(*(reinterpret_cast<T*>(da->getColumn())), COMPLEX_MAGNITUDE);

         bool passed = false;
         switch (passArea)
         {
         case LOWER:
            if (value <= firstThreshold)
            {
               passed = true;
            }
            break;
         case UPPER:
            if (value >= firstThreshold)
            {
               passed = true;
            }
            break;
         case MIDDLE:
            if ((value >= firstThreshold) && (value <= secondThreshold))
            {
               passed = true;
            }
            break;
         case OUTSIDE:
            if ((value <= firstThreshold) || (value >= secondThreshold))
            {
               passed = true;
            }
            break;
         default:
            break;
         }

         if (passed)
         {
            if (find(badValues.begin(), badValues.end(), static_cast<int>(value + 0.5)) == badValues.end())
            {
               drawer(uiColumn, uiRow);
            }
         }

         da->nextColumn();
      }

      da->nextRow();
   }
}

const BitMask* ThresholdLayerImp::getSelectedPixels() const
{
   if (mbModified)
   {
      RasterElement* pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
      if (pRasterElement != NULL)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            unsigned int uiNumColumns = pDescriptor->getColumnCount();
            unsigned int uiNumRows = pDescriptor->getRowCount();

            DataAccessor da = pRasterElement->getDataAccessor();
            if (da.isValid() == false)
            {
               return NULL;
            }

            void* pData = NULL;

            EncodingType eType = pDescriptor->getDataType();

            double dFirstThreshold = mdFirstThreshold;
            double dSecondThreshold = mdSecondThreshold;

            vector<int> badValues;

            Statistics* pStatistics = pRasterElement->getStatistics();
            if (pStatistics != NULL)
            {
               badValues = pStatistics->getBadValues();
            }

            DrawUtil::BitMaskPixelDrawer drawer(mpMask.get());
            switchOnEncoding(eType, fillRegion, pData, da, drawer, dFirstThreshold, dSecondThreshold, uiNumRows,
               uiNumColumns, mePassArea, badValues);
         }
      }

      mbModified = false;
   }

   return mpMask.get();
}

bool ThresholdLayerImp::toXml(XMLWriter* pXml) const
{
   if (!LayerImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("regionUnits", static_cast<int>(meRegionUnits));
   pXml->addAttr("passArea", static_cast<int>(mePassArea));
   pXml->addAttr("firstThreshold", mdFirstThreshold);
   pXml->addAttr("secondThreshold", mdSecondThreshold);
   pXml->addAttr("symbolType", static_cast<int>(mSymbol));
   pXml->addAttr("symbolColor", mColor.name().toStdString());

   return true;
}

bool ThresholdLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!LayerImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pElement = dynamic_cast<DOMElement*>(pDocument);
   if (pElement)
   {
      meRegionUnits = static_cast<RegionUnitsEnum>(atoi(A(pElement->getAttribute(X("regionUnits")))));
      mePassArea = static_cast<PassAreaEnum>(atoi(A(pElement->getAttribute(X("passArea")))));
      mdFirstThreshold = atof(A(pElement->getAttribute(X("firstThreshold"))));
      mdSecondThreshold = atof(A(pElement->getAttribute(X("secondThreshold"))));
      mSymbol = static_cast<SymbolTypeEnum>(atoi(A(pElement->getAttribute(X("symbolType")))));
      mColor = QColor(A(pElement->getAttribute(X("symbolColor"))));
      return true;
   }
   return false;
}

QColor ThresholdLayerImp::getColor() const
{
   return mColor;
}

SymbolType ThresholdLayerImp::getSymbol() const
{
   return mSymbol;
}

void ThresholdLayerImp::setColor(const QColor& color)
{
   if (color.isValid() == false)
   {
      return;
   }

   if (color != mColor)
   {
      ColorType newColor(color.red(), color.green(), color.blue(), color.alpha());

      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetThresholdColor(dynamic_cast<ThresholdLayer*>(this),
            QCOLOR_TO_COLORTYPE(mColor), newColor));
      }

      mColor = color;

      emit colorChanged(mColor);
      notify(SIGNAL_NAME(ThresholdLayer, ColorChanged), boost::any(newColor));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setColor(newColor);
         }
      }

      mbLinking = false;
   }
}

void ThresholdLayerImp::setSymbol(SymbolType symbol)
{
   if (symbol.isValid() == false)
   {
      return;
   }

   if (symbol != mSymbol)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetThresholdSymbol(dynamic_cast<ThresholdLayer*>(this), mSymbol, symbol));
      }

      mSymbol = symbol;

      emit symbolChanged(mSymbol);
      notify(SIGNAL_NAME(ThresholdLayer, SymbolChanged), boost::any(mSymbol));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setSymbol(symbol);
         }
      }

      mbLinking = false;
   }
}
