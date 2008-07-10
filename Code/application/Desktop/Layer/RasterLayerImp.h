/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERLAYERIMP_H
#define RASTERLAYERIMP_H

#include "ColorMap.h"
#include "AppConfig.h"
#include "ComplexData.h"
#include "DimensionDescriptor.h"
#include "LayerImp.h"
#include "RasterElement.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Animation;
class Image;
class ImageFilterDescriptor;
class QAction;
class QMenu;
class Statistics;

class RasterLayerImp : public LayerImp
{
   Q_OBJECT

#ifdef CPPTESTS // allow testing of image rendering
   friend class FilterRedrawTestCase;
#endif

public:
   RasterLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~RasterLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void movieUpdated(Subject &subject, const std::string &signal, const boost::any &v);
   void movieDeleted(Subject &subject, const std::string &signal, const boost::any &v);

   // Since Subject::attach does not reference count attachments, we need 4 
   // separate slots to force attach/detach pairs to function correctly. When 
   // Subject::attach is updated to reference count, these can be removed and
   // elementDeleted can be used directly.
   void elementDeletedGray(Subject &subject, const std::string &signal, const boost::any &v);
   void elementDeletedRed(Subject &subject, const std::string &signal, const boost::any &v);
   void elementDeletedGreen(Subject &subject, const std::string &signal, const boost::any &v);
   void elementDeletedBlue(Subject &subject, const std::string &signal, const boost::any &v);

   // Since Subject::attach does not reference count attachments, we need 4 
   // separate slots to force attach/detach pairs to function correctly. When 
   // Subject::attach is updated to reference count, these can be removed and
   // elementModified can be used directly.
   void elementModifiedGray(Subject &subject, const std::string &signal, const boost::any &v);
   void elementModifiedRed(Subject &subject, const std::string &signal, const boost::any &v);
   void elementModifiedGreen(Subject &subject, const std::string &signal, const boost::any &v);
   void elementModifiedBlue(Subject &subject, const std::string &signal, const boost::any &v);

   RasterLayerImp& operator= (const RasterLayerImp& rasterLayer);

   LayerType getLayerType() const;
   using LayerImp::setName;

   void draw();
   bool getExtents(double& x1, double& y1, double& x4, double& y4);

   std::list<ContextMenuAction> getContextMenuActions() const;

   virtual bool isGpuImageSupported() const;
   bool isGpuImageEnabled() const;

   DisplayMode getDisplayMode() const;

   // Displayed bands
   DimensionDescriptor getDisplayedBand(RasterChannelType eColor) const;
   RasterElement* getDisplayedRasterElement(RasterChannelType eColor) const;
   bool isBandDisplayed(RasterChannelType eColor, DimensionDescriptor pBand,
      const RasterElement* pRasterElement = NULL) const;

   bool setColorMap(const std::string& name, const std::vector<ColorType>& colorTable);
   bool setColorMap(const ColorMap& colorMap);
   const ColorMap& getColorMap() const;

   ComplexComponent getComplexComponent() const;
   StretchType getStretchType(const DisplayMode& eMode) const;
   RegionUnits getStretchUnits(const RasterChannelType& eColor) const;
   void getStretchValues(const RasterChannelType& eColor, double& dLower, double& dUpper) const;
   unsigned int getAlpha() const { return mAlpha; }
   QString getStretchUnitsAsString(const RasterChannelType &eColor) const;

   double convertStretchValue(const RasterChannelType& eColor, const RegionUnits& eUnits,
      double dStretchValue, const RegionUnits& eNewUnits) const;
   double convertStretchValue(const RasterChannelType& eColor, double dStretchValue,
      const RegionUnits& eNewUnits) const;

   bool isFilterSupported(const std::string& filterName) const;
   std::vector<std::string> getSupportedFilters() const;
   void enableFilter(const std::string& filterName);
   void enableFilters(const std::vector<std::string>& filterNames);
   void disableFilter(const std::string& filterName);
   bool isFilterEnabled(const std::string& filterName) const;
   ImageFilterDescriptor* getEnabledFilter(const std::string& filterName) const;
   const std::vector<ImageFilterDescriptor*>& getEnabledFilters() const;
   std::vector<std::string> getEnabledFilterNames() const;
   void resetFilter(const std::string& filterName);
   void freezeFilter(const std::string& filterName, bool toggle = true);

   unsigned int readFilterBuffer(double xCoord, double yCoord, int width, int height, std::vector<float>& values);

   bool generateFullResTexture();
   void generateFullImage();

   static void getDefaultStretchValues(const RasterChannelType& eColor, double& dLower, double& dUpper);
   static unsigned int getDefaultAlpha();

#if defined(WIN_API)
   typedef void (__stdcall *GlBlendSubtractProc)(unsigned int);
#else
   typedef void (*GlBlendSubtractProc)(unsigned int);
#endif
   static GlBlendSubtractProc getGlBlendSubtractProc();

   // Animation
   void setAnimation(Animation* pAnimation);
   Animation* getAnimation() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

public slots:
   void enableGpuImage(bool bEnable);
   void toggleDisplayMode();
   void setDisplayMode(const DisplayMode& eMode);
   void setDisplayedBand(RasterChannelType eColor, DimensionDescriptor pBand);
   void setDisplayedBand(RasterChannelType eColor, DimensionDescriptor pBand, RasterElement* pRasterElement);
   bool canApplyFastContrastStretch() const;
   bool enableFastContrastStretch(bool enable);
   void setComplexComponent(const ComplexComponent& eComponent);
   void setStretchType(const DisplayMode& eMode, const StretchType& eType);
   void setStretchUnits(const DisplayMode& eMode, const RegionUnits& eUnits);
   void setStretchUnits(const RasterChannelType& eColor, const RegionUnits& eUnits);
   void setStretchValues(const RasterChannelType& eColor, double dLower, double dUpper);
   void setAlpha(unsigned int alpha);
   void reset();
   void displayAsTrueColor();

signals:
   void gpuImageEnabled(bool bEnabled);
   void displayedBandChanged(RasterChannelType eColor, DimensionDescriptor band);
   void displayModeChanged(const DisplayMode& eMode);
   void complexComponentChanged(const ComplexComponent& eComponent);
   void stretchTypeChanged(const DisplayMode& eMode, const StretchType& eType);
   void stretchUnitsChanged(const RasterChannelType& eColor, const RegionUnits& eUnits);
   void stretchValuesChanged(const RasterChannelType& eColor, double dLower, double dUpper);
   void alphaChanged(unsigned int);
   void filtersChanged(const std::vector<ImageFilterDescriptor*>& filters);
   void colorMapChanged(const ColorMap& colorMap);

protected:
   void updateFromMovie();
   void setInternalDisplayMode(const DisplayMode& eMode);

   Image* getImage();
   void setImage(Image* pImage);
   void setImageChanged(bool bChanged);
   std::vector<double> getRawStretchValues(const RasterChannelType& eColor) const;
   const std::vector<ColorType>& getColorTable() const { return mColorMap.getTable(); }

   virtual void generateImage();
   virtual void applyFastContrastStretch();
   void applyFastContrastStretch(RasterChannelType element);
   virtual Statistics* getStatistics(RasterChannelType eColor) const;
   double percentileToRaw(double value, const double* pdPercentiles) const;
   double rawToPercentile(double value, const double* pdPercentiles) const;

   bool needToDrawPixelValues() const;
   void drawPixelValues();
   double getNumberThreshold() const;

protected slots:
   void setDisplayMode(QAction* pAction);
   void updateDisplayModeAction(const DisplayMode& displayMode);
   void changeStretch(QAction* pAction);

private:
   Image* mpImage;
   bool mUseGpuImage;
   bool mbRegenerate;

   DisplayMode meDisplayMode;
   ComplexComponent mComplexComponent;
   StretchType meGrayStretchType;
   StretchType meRgbStretchType;
   RegionUnits meGrayStretchUnits;
   RegionUnits meRedStretchUnits;
   RegionUnits meGreenStretchUnits;
   RegionUnits meBlueStretchUnits;

   DimensionDescriptor mGrayBand;
   DimensionDescriptor mRedBand;
   DimensionDescriptor mGreenBand;
   DimensionDescriptor mBlueBand;

   AttachmentPtr<RasterElement> mpGrayRasterElement;
   AttachmentPtr<RasterElement> mpRedRasterElement;
   AttachmentPtr<RasterElement> mpGreenRasterElement;
   AttachmentPtr<RasterElement> mpBlueRasterElement;

   std::vector<double> mlstGrayStretchValues;
   std::vector<double> mlstRedStretchValues;
   std::vector<double> mlstGreenStretchValues;
   std::vector<double> mlstBlueStretchValues;
   ColorMap mColorMap;
   unsigned int mAlpha;
   bool mEnableFastContrastStretch;

   std::vector<ImageFilterDescriptor*> mEnabledFilters;
   Animation* mpAnimation;

   // Context menu items
   QAction* mpSeparatorAction;
   QMenu* mpDisplayModeMenu;
   QAction* mpGrayscaleAction;
   QAction* mpRgbAction;
   QMenu* mpStretchMenu;
   QAction* mpLinear0Action;
   QAction* mpLinear2Action;
   QAction* mpLinear5Action;
   QAction* mpEqualAction;
   QAction* mpTrueColorAction;

   bool channelToXml(XMLWriter* pXml, const RasterElement* pElem, const RegionUnits &units,
      const DimensionDescriptor &descriptor, const std::vector<double> &values) const;
   bool xmlToChannel(DOMNode* pDocument, RasterElement*& pElem, 
      RegionUnits &units, DimensionDescriptor &descriptor, double &minValue, double &maxValue);
   double calculateThresholdForEncodingType(EncodingType type) const;
};

#define RASTERLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define RASTERLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   bool isGpuImageSupported() const \
   { \
      return impClass::isGpuImageSupported(); \
   } \
   void enableGpuImage(bool bEnable) \
   { \
      impClass::enableGpuImage(bEnable); \
   } \
   bool isGpuImageEnabled() const \
   { \
      return impClass::isGpuImageEnabled(); \
   } \
   void setDisplayMode(const DisplayMode& eMode) \
   { \
      impClass::setDisplayMode(eMode); \
   } \
   DisplayMode getDisplayMode() const \
   { \
      return impClass::getDisplayMode(); \
   } \
   void toggleDisplayMode() \
   { \
      impClass::toggleDisplayMode(); \
   } \
   void setColorMap(const std::string& name, const std::vector<ColorType>& colorMap) \
   { \
      impClass::setColorMap(name, colorMap); \
   } \
   const std::vector<ColorType>& getColorMap() const \
   { \
      return impClass::getColorMap().getTable(); \
   } \
   const std::string& getColorMapName() const \
   { \
      return impClass::getColorMap().getName(); \
   } \
   void setComplexComponent(const ComplexComponent& eComponent) \
   { \
      impClass::setComplexComponent(eComponent); \
   } \
   ComplexComponent getComplexComponent() const \
   { \
      return impClass::getComplexComponent(); \
   } \
   void setStretchType(const DisplayMode& eMode, const StretchType& eType) \
   { \
      impClass::setStretchType(eMode, eType); \
   } \
   StretchType getStretchType(const DisplayMode& eMode) const \
   { \
      return impClass::getStretchType(eMode); \
   } \
   void setStretchUnits(const DisplayMode& eMode, const RegionUnits& eUnits) \
   { \
      impClass::setStretchUnits(eMode, eUnits); \
   } \
   void setStretchUnits(const RasterChannelType& eColor, const RegionUnits& eUnits) \
   { \
      impClass::setStretchUnits(eColor, eUnits); \
   } \
   RegionUnits getStretchUnits(const RasterChannelType& eColor) const \
   { \
      return impClass::getStretchUnits(eColor); \
   } \
   void setStretchValues(const RasterChannelType& eColor, double dLower, double dUpper) \
   { \
      impClass::setStretchValues(eColor, dLower, dUpper); \
   } \
   void getStretchValues(const RasterChannelType& eColor, double& dLower, double& dUpper) const \
   { \
      impClass::getStretchValues(eColor, dLower, dUpper); \
   } \
   double convertStretchValue(const RasterChannelType& eColor, const RegionUnits& eUnits, \
                                                double dStretchValue, const RegionUnits& eNewUnits) const \
   { \
      return impClass::convertStretchValue(eColor, eUnits, dStretchValue, eNewUnits); \
   } \
   double convertStretchValue(const RasterChannelType& eColor, double dStretchValue, \
                                                const RegionUnits& eNewUnits) const \
   { \
      return impClass::convertStretchValue(eColor, dStretchValue, eNewUnits); \
   } \
   void setAlpha(unsigned int alpha) \
   { \
      impClass::setAlpha(alpha); \
   } \
   unsigned int getAlpha() const \
   { \
      return impClass::getAlpha(); \
   } \
   bool canApplyFastContrastStretch() const \
   { \
      return impClass::canApplyFastContrastStretch(); \
   } \
   bool enableFastContrastStretch(bool enable) \
   { \
      return impClass::enableFastContrastStretch(enable); \
   } \
   bool isFilterSupported(const std::string& filterName) const \
   { \
      return impClass::isFilterSupported(filterName); \
   } \
   std::vector<std::string> getSupportedFilters() const \
   { \
      return impClass::getSupportedFilters(); \
   } \
   void enableFilter(const std::string& filterName) \
   { \
      impClass::enableFilter(filterName); \
   } \
   void enableFilters(const std::vector<std::string>& filterNames) \
   { \
      impClass::enableFilters(filterNames); \
   } \
   void disableFilter(const std::string& filterName) \
   { \
      impClass::disableFilter(filterName); \
   } \
   bool isFilterEnabled(const std::string& filterName) const \
   { \
      return impClass::isFilterEnabled(filterName); \
   } \
   ImageFilterDescriptor* getEnabledFilter(const std::string& filterName) const \
   { \
      return impClass::getEnabledFilter(filterName); \
   } \
   const std::vector<ImageFilterDescriptor*>& getEnabledFilters() const \
   { \
      return impClass::getEnabledFilters(); \
   } \
   std::vector<std::string> getEnabledFilterNames() const \
   { \
      return impClass::getEnabledFilterNames(); \
   } \
   void resetFilter(const std::string& filterName) \
   { \
      impClass::resetFilter(filterName); \
   } \
   void freezeFilter(const std::string& filterName, bool toggle = true) \
   { \
      impClass::freezeFilter(filterName, toggle); \
   } \
   unsigned int readFilterBuffer(double xCoord, double yCoord, int width, int height, std::vector<float>& values) \
   { \
      return impClass::readFilterBuffer(xCoord, yCoord, width, height, values); \
   } \
   void setDisplayedBand(RasterChannelType eColor, DimensionDescriptor pBand, RasterElement* pRasterElement = NULL) \
   { \
      impClass::setDisplayedBand(eColor, pBand, pRasterElement); \
   } \
   DimensionDescriptor getDisplayedBand(RasterChannelType eColor) const \
   { \
      return impClass::getDisplayedBand(eColor); \
   } \
   RasterElement* getDisplayedRasterElement(RasterChannelType eColor) const \
   { \
      return impClass::getDisplayedRasterElement(eColor); \
   } \
   bool isBandDisplayed(RasterChannelType eColor, DimensionDescriptor pBand, \
      const RasterElement* pRasterElement = NULL) const \
   { \
      return impClass::isBandDisplayed(eColor, pBand, pRasterElement); \
   } \
   void setAnimation(Animation* pAnimation) \
   { \
      impClass::setAnimation(pAnimation); \
   } \
   Animation* getAnimation() const \
   { \
      return impClass::getAnimation(); \
   } \
   Statistics *getStatistics(RasterChannelType eChannel) const \
   { \
      return impClass::getStatistics(eChannel); \
   }

#endif
