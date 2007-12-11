/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMIMP_H
#define HISTOGRAMIMP_H

#include <QtGui/QPixmap>

#include "LocationType.h"
#include "PlotObjectImp.h"
#include "TypesFile.h"

#include <vector>

class RegionObjectAdapter;

class HistogramImp : public PlotObjectImp
{
   Q_OBJECT

public:
   HistogramImp(PlotViewImp* pPlot, bool bPrimary);
   ~HistogramImp();

   HistogramImp& operator= (const HistogramImp& object);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual PlotObjectType getType() const;
   virtual void draw();

   bool setHistogramData(unsigned int binCount, const double* pBinCenters, const double* pValues,
      const double* pBinWidths = NULL, bool bAbove = true);
   void getHistogramData(std::vector<double>& binCenters, std::vector<double>& binValues,
      std::vector<double>& binWidths) const;
   unsigned int getNumBins() const;
   QColor getColor() const;

   virtual bool hit(LocationType point) const;
   virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   virtual const QPixmap& getLegendPixmap(bool bSelected) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setColor(const QColor& clrBins);

signals:
   void colorChanged(const QColor& clrBins);
   void histogramChanged();

protected:
   double calculateBinWidth(unsigned int binCount, const double* pBinCenters);

private:
   std::vector<RegionObjectAdapter*> mBins;
   QColor mColor;
};

#define HISTOGRAMADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   virtual bool setHistogramData(unsigned int binCount, const double* pBinCenters, \
      const double* pValues, const double* pBinWidths = NULL, bool bAbove = true) \
   { \
      return impClass::setHistogramData(binCount, pBinCenters, pValues, pBinWidths, bAbove); \
   } \
   void getHistogramData(std::vector<double>& binCenters, std::vector<double>& binValues, \
      std::vector<double>& binWidths) const \
   { \
      return impClass::getHistogramData(binCenters, binValues, binWidths); \
   } \
   unsigned int getNumBins() const \
   { \
      return impClass::getNumBins(); \
   } \
   void setColor(const ColorType& histogramColor) \
   { \
      return impClass::setColor(QColor(histogramColor.mRed, histogramColor.mGreen, histogramColor.mBlue)); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   }

#endif
