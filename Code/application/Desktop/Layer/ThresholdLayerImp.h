/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef THRESHOLDLAYERIMP_H
#define THRESHOLDLAYERIMP_H

#include "LayerImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"

#include <QtGui/QColor>

class BitMask;
class Statistics;

class ThresholdLayerImp: public LayerImp
{
   Q_OBJECT

public:
   ThresholdLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~ThresholdLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ThresholdLayerImp& operator= (const ThresholdLayerImp& thresholdLayer);

   LayerType getLayerType() const;
   using LayerImp::setName;

   std::vector<ColorType> getColors() const;
   void draw();
   bool getExtents(double& x1, double& y1, double& x4, double& y4);

   RegionUnits getRegionUnits() const;
   PassArea getPassArea() const;
   double getFirstThreshold() const;
   double getSecondThreshold() const;
   QString getRegionUnitsAsString() const;

   double convertThreshold(const RegionUnits& eUnits, double dThreshold,
      const RegionUnits& eNewUnits) const;
   double convertThreshold(double dThreshold, const RegionUnits& eNewUnits) const;

   virtual void getBoundingBox(int &x1, int &y1, int &x2, int &y2) const;
   
   /**
    * Get the pixels within the threshold as a bitmask.
    *
    * The Layer owns the returned BitMask.
    */
   virtual const BitMask *getSelectedPixels() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   QColor getColor() const;
   SymbolType getSymbol() const;

public slots:
   void setRegionUnits(const RegionUnits& eUnits);
   void setPassArea(const PassArea& eArea);
   void setFirstThreshold(double dRawValue);
   void setSecondThreshold(double dRawValue);
   void setColor(const QColor &color);
   void setSymbol(SymbolType symbol);
   void reset();

signals:
   void regionUnitsChanged(const RegionUnits& eUnits);
   void passAreaChanged(const PassArea& eArea);
   void firstThresholdChanged(double dThreshold);
   void secondThresholdChanged(double dThreshold);
   void colorChanged(const QColor& color);
   void symbolChanged(SymbolType symbol);

protected:
   Statistics* getStatistics(RasterChannelType eColor) const;
   double percentileToRaw(double value, const double* pdPercentiles) const;
   double rawToPercentile(double value, const double* pdPercentiles) const;

private:
   RegionUnits meRegionUnits;
   PassArea mePassArea;
   double mdFirstThreshold;
   double mdSecondThreshold;
   SymbolType mSymbol;
   QColor mColor;

   mutable bool mbModified;
   mutable FactoryResource<BitMask> mpMask;

   static unsigned int msThresholdLayers;
};

#define THRESHOLDLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define THRESHOLDLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   void setFirstThreshold(double dThreshold) \
   { \
      return impClass::setFirstThreshold(dThreshold); \
   } \
   double getFirstThreshold() const \
   { \
      return impClass::getFirstThreshold(); \
   } \
   void setSecondThreshold(double dThreshold) \
   { \
      return impClass::setSecondThreshold(dThreshold); \
   } \
   double getSecondThreshold() const \
   { \
      return impClass::getSecondThreshold(); \
   } \
   void setPassArea(const PassArea& eArea) \
   { \
      return impClass::setPassArea(eArea); \
   } \
   PassArea getPassArea() const \
   { \
      return impClass::getPassArea(); \
   } \
   void setRegionUnits(const RegionUnits& eUnits) \
   { \
      return impClass::setRegionUnits(eUnits); \
   } \
   RegionUnits getRegionUnits() const \
   { \
      return impClass::getRegionUnits(); \
   } \
   double convertThreshold(const RegionUnits& eUnits, double dThreshold, const RegionUnits& eNewUnits) \
   { \
      return impClass::convertThreshold(eUnits, dThreshold, eNewUnits); \
   } \
   double convertThreshold(double dThreshold, const RegionUnits& eNewUnits) \
   { \
      return impClass::convertThreshold(dThreshold, eNewUnits); \
   } \
   SymbolType getSymbol() const \
   { \
      return impClass::getSymbol(); \
   } \
   void setSymbol(SymbolType symbol) \
   { \
      return impClass::setSymbol(symbol); \
   } \
   ColorType getColor() const \
   { \
      QColor color = impClass::getColor(); \
      return ColorType(color.red(), color.green(), color.blue()); \
   } \
   void setColor(const ColorType &color) \
   { \
      QColor qcolor(color.mRed, color.mGreen, color.mBlue); \
      return impClass::setColor(qcolor); \
   }

#endif