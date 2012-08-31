/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QColor>
#include <QtGui/QPainter>

#include "Histogram.h"
#include "HistogramImp.h"
#include "PlotView.h"
#include "RegionObjectAdapter.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
using XERCES_CPP_NAMESPACE_QUALIFIER XMLString;

HistogramImp::HistogramImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mColor(Qt::black)
{
   VERIFYNR(connect(this, SIGNAL(histogramChanged()), this, SIGNAL(extentsChanged())));
   VERIFYNR(connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(legendPixmapChanged())));
}

HistogramImp::~HistogramImp()
{
   clearBins();
}

HistogramImp& HistogramImp::operator= (const HistogramImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      clearBins();

      vector<RegionObjectAdapter*>::const_iterator iter = object.mBins.begin();
      while (iter != object.mBins.end())
      {
         RegionObjectAdapter* pRegion = NULL;
         pRegion = *iter;
         if (pRegion != NULL)
         {
            PlotViewImp* pPlot = getPlot();
            bool bPrimary = isPrimary();

            RegionObjectAdapter* pNewRegion = NULL;
            pNewRegion = new RegionObjectAdapter(pPlot, bPrimary);
            if (pNewRegion != NULL)
            {
               *pNewRegion = *pRegion;
               mBins.push_back(pNewRegion);
            }
         }

         ++iter;
      }

      mColor = object.mColor;
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& HistogramImp::getObjectType() const
{
   static string type("HistogramImp");
   return type;
}

bool HistogramImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Histogram"))
   {
      return true;
   }

   return PlotObjectImp::isKindOf(className);
}

PlotObjectType HistogramImp::getType() const
{
   return HISTOGRAM;
}

void HistogramImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   for (unsigned int i = 0; i < mBins.size(); i++)
   {
      RegionObjectAdapter* pBin = NULL;
      pBin = mBins.at(i);
      if (pBin != NULL)
      {
         pBin->draw();
      }
   }
}

bool HistogramImp::setHistogramData(unsigned int binCount, const double* pBinCenters, const double* pValues,
   const double* pBinWidths, bool bAbove)
{
   PlotViewImp* pPlot = getPlot();
   if (pPlot == NULL)
   {
      return false;
   }

   clearBins();

   if (binCount == 0)
   {
      return true;
   }

   double dBinWidth = 0.0;
   if (pBinWidths == NULL)
   {
      dBinWidth = calculateBinWidth(binCount, pBinCenters);
      if (dBinWidth <= 0.0)
      {
         return false;
      }
   }

   bool bPrimary = false;
   bPrimary = isPrimary();

   for (unsigned int i = 0; i < binCount; i++)
   {
      if (pValues[i] > 0)
      {
         RegionObjectAdapter* pBin = NULL;
         pBin = new RegionObjectAdapter(pPlot, bPrimary);
         if (pBin != NULL)
         {
            if (pBinWidths != NULL)
            {
               dBinWidth = pBinWidths[i];
            }

            double dValue = pValues[i];
            if (bAbove == false)
            {
               dValue *= -1.0;
            }

            pBin->setRegion(pBinCenters[i] - (dBinWidth * 0.45), 0.0,
               pBinCenters[i] + (dBinWidth * 0.45), dValue);
            pBin->RegionObjectImp::setColor(mColor);

            mBins.push_back(pBin);
         }
      }
   }

   emit histogramChanged();
   notify(SIGNAL_NAME(Histogram, HistogramChanged));
   return true;
}

void HistogramImp::getHistogramData(vector<double>& binCenters, vector<double>& binValues,
   vector<double>& binWidths) const
{
   binCenters.clear();
   binValues.clear();
   binWidths.clear();

   for (unsigned int i = 0; i < mBins.size(); i++)
   {
      RegionObjectAdapter* pBin = NULL;
      pBin = mBins.at(i);
      if (pBin != NULL)
      {
         double dCenter = 0.0;
         double dValue = 0.0;
         double dWidth = 0.0;

         double dMinX = 0.0;
         double dMinY = 0.0;
         double dMaxX = 0.0;
         double dMaxY = 0.0;

         bool bSuccess = false;
         bSuccess = pBin->getExtents(dMinX, dMinY, dMaxX, dMaxY);
         if (bSuccess == true)
         {
            dCenter = (dMaxX + dMinX) * 0.5;
            dValue = dMaxY - dMinY;
            dWidth = dMaxX - dMinX;
         }

         binCenters.push_back(dCenter);
         binValues.push_back(dValue);
         binWidths.push_back(dWidth);
      }
   }
}

unsigned int HistogramImp::getNumBins() const
{
   return mBins.size();
}

QColor HistogramImp::getColor() const
{
   return mColor;
}

bool HistogramImp::hit(LocationType point) const
{
   for (unsigned int i = 0; i < mBins.size(); i++)
   {
      RegionObjectAdapter* pBin = NULL;
      pBin = mBins.at(i);
      if (pBin != NULL)
      {
         bool bHit = false;
         bHit = pBin->hit(point);
         if (bHit == true)
         {
            return true;
         }
      }
   }

   return false;
}

bool HistogramImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   if (mBins.size() == 0)
   {
      return false;
   }

   dMinX = 1e38;
   dMinY = 1e38;
   dMaxX = -1e38;
   dMaxY = -1e38;

   for (unsigned int i = 0; i < mBins.size(); i++)
   {
      RegionObjectAdapter* pBin = NULL;
      pBin = mBins.at(i);
      if (pBin != NULL)
      {
         double dCurrentMinX = 0.0;
         double dCurrentMinY = 0.0;
         double dCurrentMaxX = 0.0;
         double dCurrentMaxY = 0.0;

         bool bSuccess = false;
         bSuccess = pBin->getExtents(dCurrentMinX, dCurrentMinY, dCurrentMaxX, dCurrentMaxY);
         if (bSuccess == false)
         {
            dMinX = -1.0;
            dMinY = -1.0;
            dMaxX = 1.0;
            dMaxY = 1.0;

            return false;
         }

         if (dCurrentMinX < dMinX)
         {
            dMinX = dCurrentMinX;
         }

         if (dCurrentMinY < dMinY)
         {
            dMinY = dCurrentMinY;
         }

         if (dCurrentMaxX > dMaxX)
         {
            dMaxX = dCurrentMaxX;
         }

         if (dCurrentMaxY > dMaxY)
         {
            dMaxY = dCurrentMaxY;
         }
      }
   }

   // The bin regions cannot be less than zero
   if (dMinY < 0.0)
   {
      dMinY = 0.0;
   }

   return true;
}

const QPixmap& HistogramImp::getLegendPixmap(bool bSelected) const
{
   // QPixmap must be destroyed before QApplication. This can't be guaranteed with
   // a static object. A heap object will leak but since the lifespan of this object
   // is the life of the application this is ok.
   static QPixmap* spPix(NULL);
   if (!spPix)
   {
      spPix = new QPixmap(25, 15);
   }
   static QColor pixColor;

   if (spPix->isNull() == false)
   {
      if (pixColor != mColor)
      {
         pixColor = mColor;
         spPix->fill(Qt::transparent);

         QRect rcPixmap = spPix->rect();

         QPainter p(spPix);
         p.setPen(QPen(mColor, 2));
         p.drawLine(3, rcPixmap.bottom(), 3, 7);
         p.drawLine(6, rcPixmap.bottom(), 6, 9);
         p.drawLine(9, rcPixmap.bottom(), 9, 0);
         p.drawLine(12, rcPixmap.bottom(), 12, 5);
         p.drawLine(15, rcPixmap.bottom(), 15, 10);
         p.drawLine(18, rcPixmap.bottom(), 18, 3);
         p.drawLine(21, rcPixmap.bottom(), 21, 8);
         p.end();
      }

      return *spPix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void HistogramImp::setColor(const QColor& clrBins)
{
   if (clrBins.isValid() == false)
   {
      return;
   }

   if (clrBins != mColor)
   {
      mColor = clrBins;

      for (unsigned int i = 0; i < mBins.size(); i++)
      {
         RegionObjectAdapter* pBin = NULL;
         pBin = mBins.at(i);
         if (pBin != NULL)
         {
            pBin->RegionObjectImp::setColor(clrBins);
         }
      }

      emit colorChanged(clrBins);
      notify(SIGNAL_NAME(Histogram, ColorChanged), boost::any(
         ColorType(clrBins.red(), clrBins.green(), clrBins.blue())));
   }
}

double HistogramImp::calculateBinWidth(unsigned int binCount, const double* pBinCenters)
{
   if (pBinCenters == NULL)
   {
      return -1.0;
   }

   if (binCount == 1)
   {
      return 1.0;
   }

   double dWidth = 1e38;
   double dPreviousCenter = pBinCenters[0];

   for (unsigned int i = 1; i < binCount; i++)
   {
      double dCurrentWidth = 0.0;
      dCurrentWidth = fabs(pBinCenters[i] - dPreviousCenter);
      if (dCurrentWidth < dWidth)
      {
         dWidth = dCurrentWidth;
      }

      dPreviousCenter = pBinCenters[i];
   }

   return dWidth;
}

bool HistogramImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   // Color
   pXml->addAttr("color", QCOLOR_TO_COLORTYPE(mColor));

   // Bins
   for (vector<RegionObjectAdapter*>::const_iterator iter = mBins.begin(); iter != mBins.end(); ++iter)
   {
      RegionObjectAdapter* pBin = *iter;
      if (pBin != NULL)
      {
         pXml->pushAddPoint(pXml->addElement("bin"));
         if (pBin->toXml(pXml) == false)
         {
            return false;
         }

         pXml->popAddPoint();
      }
   }

   return true;
}

bool HistogramImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);

   // Color
   ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElem->getAttribute(X("color"))));
   mColor = COLORTYPE_TO_QCOLOR(color);

   // Bins
   PlotViewImp* pPlot = getPlot();
   bool primary = isPrimary();

   clearBins();
   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("bin")))
      {
         RegionObjectAdapter* pRegion = new RegionObjectAdapter(pPlot, primary);
         if (pRegion->fromXml(pChild, version) == false)
         {
            clearBins();
            delete pRegion;
            return false;
         }

         mBins.push_back(pRegion);
      }
   }

   return true;
}

void HistogramImp::clearBins()
{
   for (vector<RegionObjectAdapter*>::iterator iter = mBins.begin(); iter != mBins.end(); ++iter)
   {
      RegionObjectAdapter* pBin = *iter;
      if (pBin != NULL)
      {
         delete pBin;
      }
   }

   mBins.clear();
}
