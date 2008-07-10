/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMPLOTIMP_H
#define HISTOGRAMPLOTIMP_H

#include <QtGui/QAction>
#include <QtGui/QListWidget>
#include <QtGui/QMenu>
#include <QtGui/QWidget>

#include "AttachmentPtr.h"
#include "EnumWrapper.h"
#include "CartesianPlotImp.h"
#include "ComplexData.h"
#include "Layer.h"
#include "Observer.h"
#include "RasterElement.h"
#include "StringUtilities.h"
#include "TypesFile.h"

#include <map>

class HistogramImp;
class Layer;
class PlotWindow;
class RegionObjectAdapter;

Q_DECLARE_METATYPE(RasterElement*)

class HistogramPlotImp : public CartesianPlotImp, public Observer
{
   Q_OBJECT

public:
   HistogramPlotImp(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~HistogramPlotImp();

   using SessionItemImp::setIcon;
   std::list<ContextMenuAction> getContextMenuActions() const;
   bool canRename() const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   HistogramPlotImp& operator= (const HistogramPlotImp& histogramPlot);
   View* copy(QGLContext* pDrawContext = 0, QWidget* pParent = 0) const;
   bool copy(View *pView) const;

   PlotType getPlotType() const;
   void elementModified(Subject &subject, const std::string &signal, const boost::any &v);
   void elementDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void attached(Subject &subject, const std::string &signal, const Slot &slot);
   void detached(Subject &subject, const std::string &signal, const Slot &slot);

   void setName(const std::string& name);

   bool setHistogram(Layer* pLayer);
   bool setHistogram(Layer* pLayer, RasterChannelType color);
   bool setHistogram(unsigned int binCount, const double* pBinCenters, const double* pValues,
      const double* pBinWidths = NULL, bool bAbove = true);

   QColor getHistogramColor() const;
   RasterChannelType getRasterChannelType() const;
   Layer* getLayer() const;

   void setAlternateColormap(const std::vector<ColorType>* pColormap);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   enum ValuesTypeEnum { NO_VALUE, LOWER_VALUE, UPPER_VALUE };
   typedef EnumWrapper<ValuesTypeEnum> ValuesType;

public slots:
   void setHistogramColor(const QColor& clrHistogram);
   void setComplexComponent(const ComplexComponent& eComponent);

protected:
   void mousePressEvent(QMouseEvent* pEvent);
   void mouseMoveEvent(QMouseEvent* pEvent);
   void mouseReleaseEvent(QMouseEvent* pEvent);
   void showEvent(QShowEvent * pEvent);

   bool getDataMinMax(double& minValue, double& maxValue) const;
   bool getDataLowerUpper(double& lowerLimit, double& upperLimit) const;
   Statistics* getStatistics() const;
   PassArea getLayerPassArea() const;
   QColor getLayerColor() const;

   void updateStretchTypeAction();
   void updateStretchUnitAction();
   void updateHistogramRegions(double lowerLimit, double upperLimit, double minValue, double maxValue,
      const PassArea& eArea, const QColor& layerColor, bool showRegions);

protected slots:
   void saveHistogram();
   void setResolution();
   void setSecurityMarkings();
   void updateSelectedColorMap();
   void setColorMapFromFile();
   void saveColorMapToFile();
   void createColorMap();
   void setPreloadedColorMap(QListWidgetItem* pItem);
   void setThresholdMode(QAction* pAction);
   void setStretchMode(QAction* pAction);
   void setStretchUnits(QAction* pAction);
   void setComplexComponent(QAction* pAction);
   void setDisplayedElement(QListWidgetItem* pItem);
   void updateElement();
   void initializeElementList();
   void setBand(QListWidgetItem* pItem);
   void initializeBandList();
   void setBadValues();

   void updateHistogramName();
   void updateHistogramValues();
   void updateHistogramRegions();
   void updateHistogramRegionExtents();

private:
   class HistogramUpdater
   {
   public:
      HistogramUpdater(HistogramPlotImp *pPlot);
      void initialize();
      void update();
   private:
      HistogramPlotImp *mpPlot;
      bool mNeedsUpdated;
   };

   void updateHistogramValues(bool force);
   void updateHistogramRegions(bool force);
   void updateHistogramName(bool force);

   class MenuListWidget : public QListWidget
   {
   public:
      MenuListWidget(QWidget* pParent = NULL) : QListWidget(pParent) {}
      QSize sizeHint() const;
   };

   void preloadColormaps();
   bool setColorMapFromFile(const QString& filename);
   std::map<std::string, std::string> mPreloadedColorMaps; // pair<name, filename>

   HistogramImp* mpHistogram;
   RegionObjectAdapter* mpRegion;
   RegionObjectAdapter* mpRegion2;
   const std::vector<ColorType> *mpAlternateColormap;
   HistogramUpdater mUpdater;

   ValuesType meSelectedValue;

   RasterChannelType mRasterChannelType;
   AttachmentPtr<Layer> mpLayer;
   AttachmentPtr<RasterElement> mpElement;

   QAction* mpBelowAction;
   QAction* mpAboveAction;
   QAction* mpBetweenAction;
   QAction* mpOutsideAction;
   QAction* mpLinearAction;
   QAction* mpExponentialAction;
   QAction* mpLogarithmicAction;
   QAction* mpEqualizationAction;
   QAction* mpRawAction;
   QAction* mpPercentageAction;
   QAction* mpPercentileAction;
   QAction* mpStdDevAction;
   QAction* mpMagnitudeAction;
   QAction* mpPhaseAction;
   QAction* mpInPhaseAction;
   QAction* mpQuadratureAction;
   QMenu* mpPassAreaMenu;
   QMenu* mpStretchUnitsMenu;
   QMenu* mpStretchTypeMenu;
   QMenu* mpElementMenu;
   MenuListWidget* mpElementList;
   QMenu* mpBandMenu;
   MenuListWidget* mpBandList;
   QMenu* mpComplexDataMenu;
   QMenu* mpColorMapMenu;
   MenuListWidget* mpColorMapList;

   QAction* mpSaveAction;
   QAction* mpBadValuesAction;
   QAction* mpThresholdSeparatorAction;
   QAction* mpRasterSeparatorAction;
   QAction* mpSamplingAction;
   QAction* mpEndSeparatorAction;
};

#define HISTOGRAMPLOTADAPTEREXTENSION_CLASSES \
   CARTESIANPLOTADAPTEREXTENSION_CLASSES

#define HISTOGRAMPLOTADAPTER_METHODS(impClass) \
   CARTESIANPLOTADAPTER_METHODS(impClass) \
   bool setHistogram(Layer* pLayer) \
   { \
      return impClass::setHistogram(pLayer); \
   } \
   bool setHistogram(Layer* pLayer, RasterChannelType color) \
   { \
      return impClass::setHistogram(pLayer, color); \
   } \
   bool setHistogram(unsigned int binCount, const double* pBinCenters, const double* pValues, \
      const double* pBinWidths = NULL, bool bAbove = true) \
   { \
      return impClass::setHistogram(binCount, pBinCenters, pValues, pBinWidths, bAbove); \
   } \
   void setHistogramColor(const ColorType& histogramColor) \
   { \
      QColor clrHistogram; \
      if (histogramColor.isValid() == true) \
      { \
         clrHistogram.setRgb(histogramColor.mRed, histogramColor.mGreen, histogramColor.mBlue); \
      } \
      \
      impClass::setHistogramColor(clrHistogram); \
   } \
   ColorType getHistogramColor() const \
   { \
      ColorType histogramColor; \
      \
      QColor clrHistogram = impClass::getHistogramColor(); \
      if (clrHistogram.isValid() == true) \
      { \
         histogramColor.mRed = clrHistogram.red(); \
         histogramColor.mGreen = clrHistogram.green(); \
         histogramColor.mBlue = clrHistogram.blue(); \
      } \
      \
      return histogramColor; \
   }

#endif
