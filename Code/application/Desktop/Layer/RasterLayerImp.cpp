/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include "Animation.h"
#include "AppConfig.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "glCommon.h"
#include "Image.h"
#include "ImageFilterDescriptor.h"
#include "ImageFilterDescriptorImp.h"
#include "ImageFilterManager.h"
#include "MathUtil.h"
#include "PropertiesRasterLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayerAdapter.h"
#include "RasterLayerImp.h"
#include "RasterLayerUndo.h"
#include "RasterUtilities.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManagerImp.h"
#include "SpatialDataView.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "Undo.h"
#include "ViewImp.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

#if defined(CG_SUPPORTED)
#include "CgContext.h"
#include "GpuImage.h"
#endif

#include <algorithm>
#include <boost/tuple/tuple.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE

RasterLayerImp::RasterLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mpImage(NULL),
   mUseGpuImage(false),
   mbRegenerate(true),
   meDisplayMode(GRAYSCALE_MODE),
   mEnableFastContrastStretch(true),
   mlstGrayStretchValues(2),
   mlstRedStretchValues(2),
   mlstGreenStretchValues(2),
   mlstBlueStretchValues(2),
   mpAnimation(NULL),
   mpSeparatorAction(NULL),
   mpDisplayModeMenu(NULL),
   mpGrayscaleAction(NULL),
   mpRgbAction(NULL),
   mpStretchMenu(NULL),
   mpLinear0Action(NULL),
   mpLinear2Action(NULL),
   mpLinear5Action(NULL),
   mpEqualAction(NULL),
   mpTrueColorAction(NULL)
{
   // Context menu actions
   Service<DesktopServices> pDesktop;
   string shortcutContext = "Layer/Raster";

   // Display mode menu
   string displayModeContext = shortcutContext + string("/Display Mode");
   mpDisplayModeMenu = new QMenu("Display Mode");

   QActionGroup* pDisplayModeGroup = new QActionGroup(mpDisplayModeMenu);
   pDisplayModeGroup->setExclusive(true);

   mpGrayscaleAction = pDisplayModeGroup->addAction("Grayscale");
   mpGrayscaleAction->setAutoRepeat(false);
   mpGrayscaleAction->setCheckable(true);
   pDesktop->initializeAction(mpGrayscaleAction, displayModeContext);

   mpRgbAction = pDisplayModeGroup->addAction("RGB");
   mpRgbAction->setAutoRepeat(false);
   mpRgbAction->setCheckable(true);
   pDesktop->initializeAction(mpRgbAction, displayModeContext);

   mpDisplayModeMenu->addActions(pDisplayModeGroup->actions());

   // Stretch menu
   mpStretchMenu = new QMenu("Image Stretch");
   string stretchContext = shortcutContext + string("/Image Stretch");

   mpLinear0Action = mpStretchMenu->addAction("Linear 0-100%ile");
   mpLinear0Action->setAutoRepeat(false);
   pDesktop->initializeAction(mpLinear0Action, stretchContext);

   mpLinear2Action = mpStretchMenu->addAction("Linear 2-98%ile");
   mpLinear2Action->setAutoRepeat(false);
   pDesktop->initializeAction(mpLinear2Action, stretchContext);

   mpLinear5Action = mpStretchMenu->addAction("Linear 5-95%ile");
   mpLinear5Action->setAutoRepeat(false);
   pDesktop->initializeAction(mpLinear5Action, stretchContext);

   mpEqualAction = mpStretchMenu->addAction("Equalization");
   mpEqualAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpEqualAction, stretchContext);

   // True color display
   mpTrueColorAction = new QAction("Display as True Color", this);
   mpTrueColorAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpTrueColorAction, shortcutContext);

   // Separator
   mpSeparatorAction = new QAction(this);
   mpSeparatorAction->setSeparator(true);

   // Initialization
   setIcon(QIcon(":/icons/RasterLayer"));
   addPropertiesPage(PropertiesRasterLayer::getName());
   reset();
   updateDisplayModeAction(meDisplayMode);      // The member display mode is set in reset()

   // Connections
   connect(this, SIGNAL(gpuImageEnabled(bool)), this, SIGNAL(modified()));
   connect(this, SIGNAL(displayedBandChanged(RasterChannelType, DimensionDescriptor)), this, SIGNAL(modified()));
   connect(this, SIGNAL(displayModeChanged(const DisplayMode&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(complexComponentChanged(const ComplexComponent&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(stretchTypeChanged(const DisplayMode&, const StretchType&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(stretchUnitsChanged(const RasterChannelType&, const RegionUnits&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(stretchValuesChanged(const RasterChannelType&, double, double)), this, SIGNAL(modified()));
   connect(this, SIGNAL(alphaChanged(unsigned int)), this, SIGNAL(modified()));
   connect(this, SIGNAL(colorMapChanged(const ColorMap&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(filtersChanged(const std::vector<ImageFilterDescriptor*>&)), this, SIGNAL(modified()));
   connect(pDisplayModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(setDisplayMode(QAction*)));
   connect(this, SIGNAL(displayModeChanged(const DisplayMode&)), this,
      SLOT(updateDisplayModeAction(const DisplayMode&)));
   connect(mpStretchMenu, SIGNAL(triggered(QAction*)), this, SLOT(changeStretch(QAction*)));
   connect(mpTrueColorAction, SIGNAL(triggered()), this, SLOT(displayAsTrueColor()));

   // Force redraws when a remote element changes
   mpGrayRasterElement.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterLayerImp::elementModifiedGray));
   mpRedRasterElement.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterLayerImp::elementModifiedRed));
   mpGreenRasterElement.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterLayerImp::elementModifiedGreen));
   mpBlueRasterElement.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterLayerImp::elementModifiedBlue));
   // Force redraws when a remote element is deleted
   mpGrayRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementModifiedGray));
   mpRedRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementModifiedRed));
   mpGreenRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementModifiedGreen));
   mpBlueRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementModifiedBlue));
   // Force texture regeneration when a remote element is deleted
   mpGrayRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementDeletedGray));
   mpRedRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementDeletedRed));
   mpGreenRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementDeletedGreen));
   mpBlueRasterElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::elementDeletedBlue));
   // Force total image regeneration when the underlying raster data changes
   mpGrayRasterElement.addSignal(SIGNAL_NAME(RasterElement, DataModified), Slot(this, &RasterLayerImp::fullImageRegenGray));
   mpRedRasterElement.addSignal(SIGNAL_NAME(RasterElement, DataModified), Slot(this, &RasterLayerImp::fullImageRegenRed));
   mpGreenRasterElement.addSignal(SIGNAL_NAME(RasterElement, DataModified), Slot(this, &RasterLayerImp::fullImageRegenGreen));
   mpBlueRasterElement.addSignal(SIGNAL_NAME(RasterElement, DataModified), Slot(this, &RasterLayerImp::fullImageRegenBlue));
}

RasterLayerImp::~RasterLayerImp()
{
   // Delete the image first to ensure that any image filters are destroyed before their descriptors
   if (mpImage != NULL)
   {
      delete mpImage;
      mpImage = NULL; // must be reset here due to the elementDeleted slot may be called from within LayerImp's d-tor
   }

   setAnimation(NULL);
   reset();

   delete mpStretchMenu;
}

const string& RasterLayerImp::getObjectType() const
{
   static string type("RasterLayerImp");
   return type;
}

bool RasterLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

void RasterLayerImp::elementModifiedGray(Subject& subject, const string& signal, const boost::any& v)
{
   elementModified(subject, signal, v);
   setImageChanged(true);
}

void RasterLayerImp::elementModifiedRed(Subject& subject, const string& signal, const boost::any& v)
{
   elementModified(subject, signal, v);
   setImageChanged(true);
}

void RasterLayerImp::elementModifiedGreen(Subject& subject, const string& signal, const boost::any& v)
{
   elementModified(subject, signal, v);
   setImageChanged(true);
}

void RasterLayerImp::elementModifiedBlue(Subject& subject, const string& signal, const boost::any& v)
{
   elementModified(subject, signal, v);
   setImageChanged(true);
}

void RasterLayerImp::elementDeletedGray(Subject& subject, const string& signal, const boost::any& data)
{
   setDisplayedBand(GRAY, DimensionDescriptor());
}

void RasterLayerImp::elementDeletedRed(Subject& subject, const string& signal, const boost::any& data)
{
   setDisplayedBand(RED, DimensionDescriptor());
}

void RasterLayerImp::elementDeletedGreen(Subject& subject, const string& signal, const boost::any& data)
{
   setDisplayedBand(GREEN, DimensionDescriptor());
}

void RasterLayerImp::elementDeletedBlue(Subject& subject, const string& signal, const boost::any& data)
{
   setDisplayedBand(BLUE, DimensionDescriptor());
}

void RasterLayerImp::fullImageRegenGray(Subject& subject, const std::string& signal, const boost::any& v)
{
   setImage(NULL);
}

void RasterLayerImp::fullImageRegenRed(Subject& subject, const std::string& signal, const boost::any& v)
{
   setImage(NULL);
}

void RasterLayerImp::fullImageRegenGreen(Subject& subject, const std::string& signal, const boost::any& v)
{
   setImage(NULL);
}

void RasterLayerImp::fullImageRegenBlue(Subject& subject, const std::string& signal, const boost::any& v)
{
   setImage(NULL);
}

bool RasterLayerImp::isKindOfLayer(const string& className)
{
   if ((className == "RasterLayerImp") || (className == "RasterLayer"))
   {
      return true;
   }

   return LayerImp::isKindOfLayer(className);
}

void RasterLayerImp::getLayerTypes(vector<string>& classList)
{
   classList.push_back("RasterLayer");
   LayerImp::getLayerTypes(classList);
}

RasterLayerImp& RasterLayerImp::operator= (const RasterLayerImp& rasterLayer)
{
   if (this != &rasterLayer)
   {
      LayerImp::operator =(rasterLayer);

      setDisplayedBand(GRAY, rasterLayer.mGrayBand, rasterLayer.getDisplayedRasterElement(GRAY));
      setDisplayedBand(RED, rasterLayer.mRedBand, rasterLayer.getDisplayedRasterElement(RED));
      setDisplayedBand(GREEN, rasterLayer.mGreenBand, rasterLayer.getDisplayedRasterElement(GREEN));
      setDisplayedBand(BLUE, rasterLayer.mBlueBand, rasterLayer.getDisplayedRasterElement(BLUE));

      meDisplayMode = rasterLayer.meDisplayMode;
      mComplexComponent = rasterLayer.mComplexComponent;

      meGrayStretchType = rasterLayer.meGrayStretchType;
      meRgbStretchType = rasterLayer.meRgbStretchType;

      meGrayStretchUnits = rasterLayer.meGrayStretchUnits;
      meRedStretchUnits = rasterLayer.meRedStretchUnits;
      meGreenStretchUnits = rasterLayer.meGreenStretchUnits;
      meBlueStretchUnits = rasterLayer.meBlueStretchUnits;

      mlstGrayStretchValues = rasterLayer.mlstGrayStretchValues;
      mlstRedStretchValues = rasterLayer.mlstRedStretchValues;
      mlstGreenStretchValues = rasterLayer.mlstGreenStretchValues;
      mlstBlueStretchValues = rasterLayer.mlstBlueStretchValues;

      mColorMap = rasterLayer.mColorMap;
      mAlpha = rasterLayer.mAlpha;
      mUseGpuImage = rasterLayer.mUseGpuImage;
      enableFilters(rasterLayer.getEnabledFilterNames());
      setAnimation(rasterLayer.mpAnimation);

      // setting all these properties will probably need a regenerate
      mbRegenerate = true;
   }

   return *this;
}

LayerType RasterLayerImp::getLayerType() const
{
   return RASTER;
}

void RasterLayerImp::draw()
{
   // Do not draw the image if there are no displayed bands
   DisplayMode displayMode = getDisplayMode();
   if (displayMode == GRAYSCALE_MODE)
   {
      if (mGrayBand.isValid() == false || getDisplayedRasterElement(GRAY) == NULL)
      {
         return;
      }
   }
   else if (displayMode == RGB_MODE)
   {
      if ((mRedBand.isValid() == false || getDisplayedRasterElement(RED) == NULL) && 
         (mGreenBand.isValid() == false || getDisplayedRasterElement(GREEN) == NULL) &&
         (mBlueBand.isValid() == false || getDisplayedRasterElement(BLUE) == NULL))
      {
         return;
      }
   }

   bool bFastContrastEnabled = RasterLayer::getSettingFastContrastStretch();
   if (bFastContrastEnabled != mEnableFastContrastStretch)
   {
      mbRegenerate = true;
      mEnableFastContrastStretch = bFastContrastEnabled;
   }

   if (mbRegenerate == true)
   {
      generateImage();
   }

   if (mpImage != NULL)
   {
      GLint textureMode = GL_NEAREST;

      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(getView());
      if (pSpatialDataView != NULL)
      {
         if (pSpatialDataView->getTextureMode() == TEXTURE_LINEAR)
         {
            textureMode = GL_LINEAR;
         }
      }

      glColor3f(1.0, 1.0, 1.0);

      mpImage->draw(textureMode);
      if (canApplyFastContrastStretch())
      {
         applyFastContrastStretch();
      }
   }

   // Draw the pixel values
   bool bDrawPixels = needToDrawPixelValues();
   if (bDrawPixels == true)
   {
      drawPixelValues();
   }
}

bool RasterLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   VERIFY(pRasterElement != NULL);

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   translateDataToWorld(0, 0, x1, y1);
   translateDataToWorld(pDescriptor->getColumnCount(), pDescriptor->getRowCount(),
      x4, y4);

   return true;
}

list<ContextMenuAction> RasterLayerImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = LayerImp::getContextMenuActions();
   menuActions.push_front(ContextMenuAction(mpSeparatorAction, APP_RASTERLAYER_SEPARATOR_ACTION));

   DataElement* pElement = getDataElement();
   if (pElement != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         if (RasterUtilities::canBeDisplayedInTrueColor(pDescriptor))
         {
            menuActions.push_front(ContextMenuAction(mpTrueColorAction, APP_RASTERLAYER_TRUE_COLOR_ACTION));
         }
      }
   }

   menuActions.push_front(ContextMenuAction(mpStretchMenu->menuAction(), APP_RASTERLAYER_STRETCH_MENU_ACTION));
   menuActions.push_front(ContextMenuAction(mpDisplayModeMenu->menuAction(), APP_RASTERLAYER_DISPLAY_MODE_MENU_ACTION));
   return menuActions;
}

bool RasterLayerImp::isGpuImageSupported() const
{
#if defined(CG_SUPPORTED)
   // Check if the hardware supports the GPU image
   if (CgContext::instance() == NULL)
   {
      return false;
   }

   // The GPU image does not support complex data
   DisplayMode eMode = getDisplayMode();
   if (eMode == GRAYSCALE_MODE)
   {
      RasterElement* pGrayElement = getDisplayedRasterElement(GRAY);
      if (pGrayElement)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pGrayElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            EncodingType eEncoding = pDescriptor->getDataType();
            if ((eEncoding != INT4SCOMPLEX) && (eEncoding != FLT8COMPLEX))
            {
               return true;
            }
         }
      }
   }
   else if (eMode == RGB_MODE)
   {
      RasterElement* pRedElement = getDisplayedRasterElement(RED);
      RasterElement* pGreenElement = getDisplayedRasterElement(GREEN);
      RasterElement* pBlueElement = getDisplayedRasterElement(BLUE);
      if ((pRedElement != NULL) && (pGreenElement != NULL) && (pBlueElement != NULL))
      {
         const RasterDataDescriptor* pRedDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRedElement->getDataDescriptor());
         const RasterDataDescriptor* pGreenDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pGreenElement->getDataDescriptor());
         const RasterDataDescriptor* pBlueDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pBlueElement->getDataDescriptor());

         if ((pRedDescriptor != NULL) && (pGreenDescriptor != NULL) && (pBlueDescriptor != NULL))
         {
            EncodingType eRedEncoding = pRedDescriptor->getDataType();
            EncodingType eGreenEncoding = pGreenDescriptor->getDataType();
            EncodingType eBlueEncoding = pBlueDescriptor->getDataType();

            if ((eRedEncoding != INT4SCOMPLEX) && (eRedEncoding != FLT8COMPLEX) &&
               (eGreenEncoding != INT4SCOMPLEX) && (eGreenEncoding != FLT8COMPLEX) &&
               (eBlueEncoding != INT4SCOMPLEX) && (eBlueEncoding != FLT8COMPLEX))
            {
               return true;
            }
         }
      }
   }
#endif

   return false;
}

bool RasterLayerImp::isGpuImageEnabled() const
{
   return mUseGpuImage;
}

double RasterLayerImp::calculateThresholdForEncodingType(EncodingType type) const
{

   //calculate the width and height in screen pixels required to display the raw pixel values
   //according to the given encoding type, assume that 3 raw pixel values will be
   //shown, ie. RGB mode.
   //this code assume's that for a given font that all digits are the same width.
   QFont textFont = QApplication::font(); 
   textFont.setPointSize(View::getSettingPixelValueMinimumFontSize());
   textFont.setBold(false);
   QFontMetrics metrics(textFont);

   QSize fontSize = metrics.size(0, "N/A");
   if (type == INT1SBYTE)
   {
      fontSize = metrics.size(0, QString::number(numeric_limits<signed char>::min()));
   }
   else if (type == INT1UBYTE)
   {
      fontSize = metrics.size(0, QString::number(numeric_limits<unsigned char>::max()));
   }
   else if (type == INT2SBYTES)
   {
      fontSize = metrics.size(0, QString::number(numeric_limits<short>::min()));
   }
   else if (type == INT2UBYTES)
   {
      fontSize = metrics.size(0, QString::number(numeric_limits<unsigned short>::max()));
   }
   else if (type == INT4SBYTES)
   {
      fontSize = metrics.size(0, QString::number(numeric_limits<int>::min()));
   }
   else if (type == INT4UBYTES)
   {
      fontSize = metrics.size(0, QString::number(numeric_limits<unsigned int>::max()));
   }
   else if (type == FLT4BYTES ||
      type == FLT8COMPLEX ||
      type == FLT8BYTES ||
      type == INT4SCOMPLEX /* treat integer complex as float because we want
                           threshold to be the same regardless of the ComplexComponent */)
   {
      //for floating point data, test a negative value with a both a positive and negative
      //scientific notation exponent to determine which is larger in the given font.
      QSize double1 = metrics.size(0, QString::number(-1 * numeric_limits<double>::min()));
      QSize double2 = metrics.size(0, QString::number(-1 * numeric_limits<double>::max()));
      fontSize.setHeight(max(double1.height(), double2.height()));
      fontSize.setWidth(max(double1.width(), double2.width()));
   }

   if (type == INT1SBYTE || type == INT4SCOMPLEX || type == INT1UBYTE ||
       type == INT2SBYTES || type == INT2UBYTES)
   {
      //We want the threshold to be the same regardless of whether View::getSettingInsetShowCoordinates()
      //returns true or false.
      //For the above encoding types, the pixel coordinates which are 4-byte integers
      //could be larger in size then the data values when rendered, so let's calculate
      RasterElement* pElement = dynamic_cast<RasterElement*>(getDataElement());
      if (pElement != NULL)
      {
         const RasterDataDescriptor* pDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
         unsigned int largestDimension = max(pDescriptor->getRowCount(), pDescriptor->getColumnCount());
         QString pixelCoordStr = QString::number(largestDimension);
         QSize pixelCoordSize = metrics.size(0, pixelCoordStr);
         fontSize.setWidth(max(fontSize.width(), pixelCoordSize.width()));
         fontSize.setHeight(max(fontSize.height(), pixelCoordSize.height()));
      }
   }
   //always require at least 32 screen pixels to draw any text, this
   //is to minimize pixel value drawing becoming turned on when it could
   //be very slow to perform the draw.
   const int PIXEL_WIDTH_PADDING = 2;
   const int ROWS_OF_DATA = 3;
   double rawPixelRequiredWidth = max(fontSize.width() + PIXEL_WIDTH_PADDING, 32) / getXScaleFactor();
   double rawPixelRequiredHeight = max(fontSize.height() * ROWS_OF_DATA, 32) / getYScaleFactor();

   double requiredZoomPercentage = max(rawPixelRequiredHeight, rawPixelRequiredWidth) * 100.0 - 1.0;

   return requiredZoomPercentage;
}

double RasterLayerImp::getNumberThreshold() const
{
   double dZoomThreshold = 0.0;

   DisplayMode displayMode = getDisplayMode();
   if (displayMode == GRAYSCALE_MODE)
   {
      const RasterDataDescriptor* pDescriptor = 
         dynamic_cast<const RasterDataDescriptor*>(mpGrayRasterElement->getDataDescriptor());
      dZoomThreshold = calculateThresholdForEncodingType(pDescriptor->getDataType());
   }
   else
   {
      EncodingType redEncoding;
      if (mpRedRasterElement.get() != NULL)
      {
         const RasterDataDescriptor* pRedDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(mpRedRasterElement->getDataDescriptor());
         redEncoding = pRedDescriptor->getDataType();
      }
      double redThreshold = calculateThresholdForEncodingType(redEncoding);
      EncodingType greenEncoding;
      if (mpGreenRasterElement.get() != NULL)
      {
         const RasterDataDescriptor* pGreenDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(mpGreenRasterElement->getDataDescriptor());
         greenEncoding = pGreenDescriptor->getDataType();
      }
      double greenThreshold = calculateThresholdForEncodingType(greenEncoding);

      EncodingType blueEncoding;
      if (mpBlueRasterElement.get() != NULL)
      {
         const RasterDataDescriptor* pBlueDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(mpBlueRasterElement->getDataDescriptor());
         blueEncoding = pBlueDescriptor->getDataType();
      }
      double blueThreshold = calculateThresholdForEncodingType(blueEncoding);
      dZoomThreshold = max(max(redThreshold, greenThreshold), blueThreshold);
   }

   return dZoomThreshold;
}

bool RasterLayerImp::needToDrawPixelValues() const
{
   DisplayMode displayMode = getDisplayMode();
   if ((displayMode == GRAYSCALE_MODE) && (mGrayBand.isValid() == false || mpGrayRasterElement.get() == NULL))
   {
      return false;
   }
   else if ((displayMode == RGB_MODE) &&
      (mRedBand.isValid() == false || mpRedRasterElement.get() == NULL) &&
      (mGreenBand.isValid() == false || mpGreenRasterElement.get() == NULL) &&
      (mBlueBand.isValid() == false || mpBlueRasterElement.get() == NULL))
   {
      return false;
   }

   // Check for an appropriate zoom level
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      double dZoom = pView->getZoomPercentage();
      double dZoomThreshold = getNumberThreshold();

      if (dZoom > dZoomThreshold)
      {
         return true;
      }
   }

   return false;
}

namespace
{
   template<typename T>
   void stringifyValue(T* pValue, ComplexComponent component, const vector<int>& badValues, QString& strValue)
   {
      T value = ModelServices::getDataValue(*pValue, component);
      strValue = QString::number(value);

      int badValue = static_cast<int>(roundDouble(value));
      if (binary_search(badValues.begin(), badValues.end(), badValue) == true)
      {
         strValue.clear();
      }
   }
};

void RasterLayerImp::drawPixelValues()
{
   ViewImp* pView = dynamic_cast<ViewImp*>(getView());
   VERIFYNRV(pView != NULL);

   SpatialDataView* pSpatialView = dynamic_cast<SpatialDataView*>(pView);
   VERIFYNRV(pSpatialView != NULL);

   bool bDrawCoords = View::getSettingInsetShowCoordinates();

   double dHeading = 0.0;
   double dPitch = 0.0;
   LocationType lowerLeft;
   LocationType upperLeft;
   LocationType upperRight;
   LocationType lowerRight;

   GLboolean bScissorEnable;
   glGetBooleanv(GL_SCISSOR_TEST, &bScissorEnable);
   if ((bScissorEnable == GL_TRUE) && (pView->isInsetEnabled() == true))
   {
      // inset
      GLint pScreenLocation[4];
      glGetIntegerv(GL_SCISSOR_BOX, pScreenLocation);
      int iMinX = pScreenLocation[0];
      int iMinY = pScreenLocation[1];

      // Map the scissor box screen coordinates from the product view to the
      // spatial data view if the inset pixel values are drawn in a product
      if (dynamic_cast<ViewImp*>(pView->parentWidget()) != NULL)
      {
         QPoint screenCoord = pView->mapFromParent(QPoint(iMinX, iMinY));
         iMinX = screenCoord.x();
         iMinY = screenCoord.y();
      }

      int iMaxX = iMinX + pScreenLocation[2];
      int iMaxY = iMinY + pScreenLocation[3];
      pView->translateScreenToWorld(iMinX, iMinY, lowerLeft.mX, lowerLeft.mY);
      pView->translateScreenToWorld(iMinX, iMaxY, upperLeft.mX, upperLeft.mY);
      pView->translateScreenToWorld(iMaxX, iMaxY, upperRight.mX, upperRight.mY);
      pView->translateScreenToWorld(iMaxX, iMinY, lowerRight.mX, lowerRight.mY);
   }
   else
   {
      pView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);
   }

   dHeading = pSpatialView->getRotation();
   dPitch = pSpatialView->getPitch();

   // Get the bounding box of the view
   translateWorldToData(lowerLeft.mX, lowerLeft.mY, lowerLeft.mX, lowerLeft.mY);
   translateWorldToData(upperLeft.mX, upperLeft.mY, upperLeft.mX, upperLeft.mY);
   translateWorldToData(upperRight.mX, upperRight.mY, upperRight.mX, upperRight.mY);
   translateWorldToData(lowerRight.mX, lowerRight.mY, lowerRight.mX, lowerRight.mY);

   // Get the cube extents
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;
   getExtents(dMinX, dMinY, dMaxX, dMaxY);
   translateWorldToData(dMinX, dMinY, dMinX, dMinY);
   translateWorldToData(dMaxX, dMaxY, dMaxX, dMaxY);

   // Clip the bounding box to the cube extents
   double extreme;
   extreme = lowerLeft.mX;
   extreme = min(extreme, upperLeft.mX);
   extreme = min(extreme, upperRight.mX);
   extreme = min(extreme, lowerRight.mX);
   extreme = min(extreme, dMaxX - 1.0);
   extreme = max(extreme, dMinX);
   int x1 = extreme;

   extreme = lowerLeft.mX;
   extreme = max(extreme, upperLeft.mX);
   extreme = max(extreme, upperRight.mX);
   extreme = max(extreme, lowerRight.mX);
   extreme = max(extreme, dMinX);
   extreme = min(extreme, dMaxX - 1.0);
   int x2 = extreme;

   extreme = lowerLeft.mY;
   extreme = min(extreme, upperLeft.mY);
   extreme = min(extreme, upperRight.mY);
   extreme = min(extreme, lowerRight.mY);
   extreme = min(extreme, dMaxY - 1.0);
   extreme = max(extreme, dMinY);
   int y1 = extreme;

   extreme = lowerLeft.mY;
   extreme = max(extreme, upperLeft.mY);
   extreme = max(extreme, upperRight.mY);
   extreme = max(extreme, lowerRight.mY);
   extreme = max(extreme, dMinY);
   extreme = min(extreme, dMaxY - 1.0);
   int y2 = extreme;

   glLineWidth (1.5);
   glMatrixMode(GL_MODELVIEW);

   QPoint shadowOffset(1, -1);
   double radius = pSpatialView->getZoomPercentage() / 100.0;

   double minScaleFactor = min(getXScaleFactor(), getYScaleFactor());
   radius *= minScaleFactor;
   unsigned int fontSize = View::getSettingPixelValueMinimumFontSize();
   fontSize *= pSpatialView->getZoomPercentage() / getNumberThreshold();
   fontSize = min(fontSize, View::getSettingPixelValueMaximumFontSize());

   QFont textFont = QApplication::font();
   textFont.setPointSize(fontSize);
   textFont.setBold(false);
   QFontMetrics fm(textFont);

   GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
   GLfloat white[] = {1.0, 1.0, 1.0, 1.0};

   if (bDrawCoords == true)
   {
      for (int j = y1; j <= y2; ++j)
      {
         for (int i = x1; i <= x2; ++i)
         {
            LocationType centerWorld;
            translateDataToWorld(i + 0.5, j + 0.5, centerWorld.mX, centerWorld.mY);
            LocationType centerScreen;
            pView->translateWorldToScreen(centerWorld.mX, centerWorld.mY, centerScreen.mX, centerScreen.mY);
            QPoint centerPoint(centerScreen.mX, centerScreen.mY - fm.ascent() / 2);

            QString strX(QString::number(i + 1));
            QString strY(QString::number(j + 1));

            QPoint xTextOffset(-fm.width(strX) / 2, radius / 6);
            QPoint yTextOffset(-fm.width(strY) / 2, -radius / 6);

            glColor4fv(black);

            int screenX = centerPoint.x() + xTextOffset.x() + shadowOffset.x();
            int screenY = pView->height() - (centerPoint.y() + xTextOffset.y() + shadowOffset.y());
            pView->renderText(screenX, screenY, strX, textFont);

            screenX = centerPoint.x() + yTextOffset.x() + shadowOffset.x();
            screenY = pView->height() - (centerPoint.y() + yTextOffset.y() + shadowOffset.y());
            pView->renderText(screenX, screenY, strY, textFont);

            glColor4fv(white);

            screenX = centerPoint.x() + xTextOffset.x();
            screenY = pView->height() - (centerPoint.y() + xTextOffset.y());
            pView->renderText(screenX, screenY, strX, textFont);

            screenX = centerPoint.x() + yTextOffset.x();
            screenY = pView->height() - (centerPoint.y() + yTextOffset.y());
            pView->renderText(screenX, screenY, strY, textFont);
         }
      }
   }
   else
   {
      ComplexComponent complexComponent = getComplexComponent();

      DisplayMode displayMode = getDisplayMode();
      if (displayMode == GRAYSCALE_MODE)
      {
         DataAccessor accessor(NULL, NULL);
         EncodingType dataType;
         vector<int> badValues;

         if (mGrayBand.isValid() == true)
         {
            RasterElement* pElement = getDisplayedRasterElement(GRAY);
            if (pElement != NULL)
            {
               RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
               VERIFYNRV(pDescriptor != NULL);

               FactoryResource<DataRequest> pRequest;
               pRequest->setRows(pDescriptor->getActiveRow(y1), pDescriptor->getActiveRow(y2));
               pRequest->setColumns(pDescriptor->getActiveColumn(x1), pDescriptor->getActiveColumn(x2));
               pRequest->setBands(mGrayBand, mGrayBand);

               accessor = pElement->getDataAccessor(pRequest.release());
               dataType = pDescriptor->getDataType();

               Statistics* pStatistics = pElement->getStatistics(mGrayBand);
               if (pStatistics != NULL)
               {
                  badValues = pStatistics->getBadValues();
               }
            }
         }

         if ((accessor.isValid() == true) && (dataType.isValid()))
         {
            for (int j = y1; j <= y2; ++j)
            {
               if (accessor.isValid() == true)
               {
                  for (int i = x1; i <= x2; ++i)
                  {
                     LocationType centerWorld;
                     translateDataToWorld(i + 0.5, j + 0.5, centerWorld.mX, centerWorld.mY);
                     LocationType centerScreen;
                     pView->translateWorldToScreen(centerWorld.mX, centerWorld.mY, centerScreen.mX, centerScreen.mY);
                     QPoint centerPoint(centerScreen.mX, centerScreen.mY - fm.ascent() / 2);

                     // Draw the text
                     QString strGray;
                     switchOnEncoding(dataType, stringifyValue, accessor->getColumn(), complexComponent,
                        badValues, strGray);
                     if (strGray.isEmpty() == false)
                     {
                        QPoint textOffset(-fm.width(strGray) / 2, 0);

                        glColor4fv(black);

                        int screenX = centerPoint.x() + textOffset.x() + shadowOffset.x();
                        int screenY = pView->height() - (centerPoint.y() + textOffset.y() + shadowOffset.y());
                        pView->renderText(screenX, screenY, strGray, textFont);

                        glColor4fv(white);

                        screenX = centerPoint.x() + textOffset.x();
                        screenY = pView->height() - (centerPoint.y() + textOffset.y());
                        pView->renderText(screenX, screenY, strGray, textFont);
                     }

                     accessor->nextColumn();
                  }

                  accessor->nextRow();
               }
            }
         }
      }
      else if (displayMode == RGB_MODE)
      {
         VERIFYNRV(mRedBand.isValid() || mGreenBand.isValid() || mBlueBand.isValid());

         DataAccessor redAccessor(NULL, NULL);
         DataAccessor greenAccessor(NULL, NULL);
         DataAccessor blueAccessor(NULL, NULL);

         EncodingType redDataType;
         EncodingType greenDataType;
         EncodingType blueDataType;

         vector<int> redBadValues;
         vector<int> greenBadValues;
         vector<int> blueBadValues;

         if (mRedBand.isValid() == true)
         {
            RasterElement* pElement = getDisplayedRasterElement(RED);
            if (pElement != NULL)
            {
               RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
               VERIFYNRV(pDescriptor != NULL);

               FactoryResource<DataRequest> pRequest;
               pRequest->setRows(pDescriptor->getActiveRow(y1), pDescriptor->getActiveRow(y2));
               pRequest->setColumns(pDescriptor->getActiveColumn(x1), pDescriptor->getActiveColumn(x2));
               pRequest->setBands(mRedBand, mRedBand);

               redAccessor = pElement->getDataAccessor(pRequest.release());
               redDataType = pDescriptor->getDataType();

               Statistics* pStatistics = pElement->getStatistics(mRedBand);
               if (pStatistics != NULL)
               {
                  redBadValues = pStatistics->getBadValues();
               }
            }
         }

         if (mGreenBand.isValid() == true)
         {
            RasterElement* pElement = getDisplayedRasterElement(GREEN);
            if (pElement != NULL)
            {
               RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
               VERIFYNRV(pDescriptor != NULL);

               FactoryResource<DataRequest> pRequest;
               pRequest->setRows(pDescriptor->getActiveRow(y1), pDescriptor->getActiveRow(y2));
               pRequest->setColumns(pDescriptor->getActiveColumn(x1), pDescriptor->getActiveColumn(x2));
               pRequest->setBands(mGreenBand, mGreenBand);

               greenAccessor = pElement->getDataAccessor(pRequest.release());
               greenDataType = pDescriptor->getDataType();

               Statistics* pStatistics = pElement->getStatistics(mGreenBand);
               if (pStatistics != NULL)
               {
                  greenBadValues = pStatistics->getBadValues();
               }
            }
         }

         if (mBlueBand.isValid() == true)
         {
            RasterElement* pElement = getDisplayedRasterElement(BLUE);
            if (pElement != NULL)
            {
               RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
               VERIFYNRV(pDescriptor != NULL);

               FactoryResource<DataRequest> pRequest;
               pRequest->setRows(pDescriptor->getActiveRow(y1), pDescriptor->getActiveRow(y2));
               pRequest->setColumns(pDescriptor->getActiveColumn(x1), pDescriptor->getActiveColumn(x2));
               pRequest->setBands(mBlueBand, mBlueBand);

               blueAccessor = pElement->getDataAccessor(pRequest.release());
               blueDataType = pDescriptor->getDataType();

               Statistics* pStatistics = pElement->getStatistics(mBlueBand);
               if (pStatistics != NULL)
               {
                  blueBadValues = pStatistics->getBadValues();
               }
            }
         }

         for (int j = y1; j <= y2; ++j)
         {
            bool redValid = redAccessor.isValid();
            bool greenValid = greenAccessor.isValid();
            bool blueValid = blueAccessor.isValid();

            for (int i = x1; i <= x2; ++i)
            {
               LocationType centerWorld;
               translateDataToWorld(i + 0.5, j + 0.5, centerWorld.mX, centerWorld.mY);
               LocationType centerScreen;
               pView->translateWorldToScreen(centerWorld.mX, centerWorld.mY, centerScreen.mX, centerScreen.mY);
               QPoint centerPoint(centerScreen.mX, centerScreen.mY - fm.ascent() / 2);

               // Red text
               QString strRed = "N/A";
               if ((redValid == true) && (redDataType.isValid()))
               {
                  switchOnEncoding(redDataType, stringifyValue, redAccessor->getColumn(), complexComponent,
                     redBadValues, strRed);
                  redAccessor->nextColumn();
               }

               // Green text
               QString strGreen = "N/A";
               if ((greenValid == true) && (greenDataType.isValid()))
               {
                  switchOnEncoding(greenDataType, stringifyValue, greenAccessor->getColumn(), complexComponent,
                     greenBadValues, strGreen);
                  greenAccessor->nextColumn();
               }

               // Blue text
               QString strBlue = "N/A";
               if ((blueValid == true) && (blueDataType.isValid()))
               {
                  switchOnEncoding(blueDataType, stringifyValue, blueAccessor->getColumn(), complexComponent,
                     blueBadValues, strBlue);
                  blueAccessor->nextColumn();
               }

               // Draw the text
               QPoint redTextOffset(-fm.width(strRed) / 2, radius / 4);
               QPoint greenTextOffset(-fm.width(strGreen) / 2, 0);
               QPoint blueTextOffset(-fm.width(strBlue) / 2, -radius / 4);

               glColor4fv(black);

               if (strRed.isEmpty() == false)
               {
                  int screenX = centerPoint.x() + redTextOffset.x() + shadowOffset.x();
                  int screenY = pView->height() - (centerPoint.y() + redTextOffset.y() + shadowOffset.y());
                  pView->renderText(screenX, screenY, strRed, textFont);
               }

               if (strGreen.isEmpty() == false)
               {
                  int screenX = centerPoint.x() + greenTextOffset.x() + shadowOffset.x();
                  int screenY = pView->height() - (centerPoint.y() + greenTextOffset.y() + shadowOffset.y());
                  pView->renderText(screenX, screenY, strGreen, textFont);
               }

               if (strBlue.isEmpty() == false)
               {
                  int screenX = centerPoint.x() + blueTextOffset.x() + shadowOffset.x();
                  int screenY = pView->height() - (centerPoint.y() + blueTextOffset.y() + shadowOffset.y());
                  pView->renderText(screenX, screenY, strBlue, textFont);
               }

               glColor4fv(white);

               if (strRed.isEmpty() == false)
               {
                  int screenX = centerPoint.x() + redTextOffset.x();
                  int screenY = pView->height() - (centerPoint.y() + redTextOffset.y());
                  pView->renderText(screenX, screenY, strRed, textFont);
               }

               if (strGreen.isEmpty() == false)
               {
                  int screenX = centerPoint.x() + greenTextOffset.x();
                  int screenY = pView->height() - (centerPoint.y() + greenTextOffset.y());
                  pView->renderText(screenX, screenY, strGreen, textFont);
               }

               if (strBlue.isEmpty() == false)
               {
                  int screenX = centerPoint.x() + blueTextOffset.x();
                  int screenY = pView->height() - (centerPoint.y() + blueTextOffset.y());
                  pView->renderText(screenX, screenY, strBlue, textFont);
               }
            }

            if (redValid == true)
            {
               redAccessor->nextRow();
            }

            if (greenValid == true)
            {
               greenAccessor->nextRow();
            }

            if (blueValid == true)
            {
               blueAccessor->nextRow();
            }
         }
      }
   }
}

DisplayMode RasterLayerImp::getDisplayMode() const
{
   return meDisplayMode;
}

void RasterLayerImp::toggleDisplayMode()
{
   if (meDisplayMode == GRAYSCALE_MODE)
   {
      setDisplayMode(RGB_MODE);
   }
   else if (meDisplayMode == RGB_MODE)
   {
      setDisplayMode(GRAYSCALE_MODE);
   }
}

DimensionDescriptor RasterLayerImp::getDisplayedBand(RasterChannelType eColor) const
{
   if (eColor == GRAY)
   {
      return mGrayBand;
   }
   else if (eColor == RED)
   {
      return mRedBand;
   }
   else if (eColor == GREEN)
   {
      return mGreenBand;
   }
   else if (eColor == BLUE)
   {
      return mBlueBand;
   }

   return DimensionDescriptor();
}

RasterElement* RasterLayerImp::getDisplayedRasterElement(RasterChannelType eColor) const
{
   const DataElement* pElement = NULL;

   if (eColor == GRAY)
   {
      pElement = mpGrayRasterElement.get();
   }
   else if (eColor == RED)
   {
      pElement = mpRedRasterElement.get();
   }
   else if (eColor == GREEN)
   {
      pElement = mpGreenRasterElement.get();
   }
   else if (eColor == BLUE)
   {
      pElement = mpBlueRasterElement.get();
   }

   return dynamic_cast<RasterElement*>(const_cast<DataElement*>(pElement));
}

bool RasterLayerImp::isBandDisplayed(RasterChannelType eColor, DimensionDescriptor band,
                                     const RasterElement* pRasterElement) const
{
   if ((band.isValid() == true) && (pRasterElement == NULL))
   {
      pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   }

   if (eColor == GRAY)
   {
      if ((mGrayBand == band) && (getDisplayedRasterElement(GRAY) == pRasterElement))
      {
         return true;
      }
   }
   else if (eColor == RED)
   {
      if ((mRedBand == band) && (getDisplayedRasterElement(RED) == pRasterElement))
      {
         return true;
      }
   }
   else if (eColor == GREEN)
   {
      if ((mGreenBand == band) && (getDisplayedRasterElement(GREEN) == pRasterElement))
      {
         return true;
      }
   }
   else if (eColor == BLUE)
   {
      if ((mBlueBand == band) && (getDisplayedRasterElement(BLUE) == pRasterElement))
      {
         return true;
      }
   }

   return false;
}

void RasterLayerImp::setDisplayedBand(RasterChannelType eColor, DimensionDescriptor band)
{
   setDisplayedBand(eColor, band, NULL);
}

void RasterLayerImp::setDisplayedBand(RasterChannelType eColor, DimensionDescriptor band,
                                      RasterElement* pRasterElement)
{
   if (band.isValid() && (pRasterElement == NULL))
   {
      pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
      if (pRasterElement == NULL)
      {
         return;
      }
   }
   else if ((band.isValid() == false) && (pRasterElement != NULL))
   {
      pRasterElement = NULL;
   }

   if (isBandDisplayed(eColor, band, pRasterElement))
   {
      return;
   }

   DisplayMode eMode = GRAYSCALE_MODE;

   //make sure band descriptor came from raster element
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         if (band.isValid())
         {
            DimensionDescriptor foundBand = pDescriptor->getActiveBand(band.getActiveNumber());
            if (foundBand != band)
            {
               return;
            }
         }
      }
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         // For performance reasons, do not create the undo action if it will not be added
         if (pView->isUndoBlocked() == false)
         {
            DimensionDescriptor oldBand;
            RasterElement* pOldRasterElement = NULL;

            switch (eColor)
            {
               case GRAY:
                  oldBand = mGrayBand;
                  pOldRasterElement = mpGrayRasterElement.get();
                  break;

               case RED:
                  oldBand = mRedBand;
                  pOldRasterElement = mpRedRasterElement.get();
                  break;

               case GREEN:
                  oldBand = mGreenBand;
                  pOldRasterElement = mpGreenRasterElement.get();
                  break;

               case BLUE:
                  oldBand = mBlueBand;
                  pOldRasterElement = mpBlueRasterElement.get();
                  break;

               default:
                  break;
            }

            pView->addUndoAction(new SetRasterDisplayedBand(dynamic_cast<RasterLayer*>(this), eColor, oldBand, band,
               pOldRasterElement, pRasterElement));
         }
      }

      switch (eColor)
      {
         case GRAY:
            mpGrayRasterElement.reset(pRasterElement);
            mGrayBand = band;
            eMode = GRAYSCALE_MODE;
            break;

         case RED:
            mpRedRasterElement.reset(pRasterElement);
            mRedBand = band;
            eMode = RGB_MODE;
            break;

         case GREEN:
            mpGreenRasterElement.reset(pRasterElement);
            mGreenBand = band;
            eMode = RGB_MODE;
            break;

         case BLUE:
            mpBlueRasterElement.reset(pRasterElement);
            mBlueBand = band;
            eMode = RGB_MODE;
            break;

         default:
            break;
      }

      if (getDisplayMode() == eMode)
      {
         setImageChanged(true);
      }

      emit displayedBandChanged(eColor, band);
      notify(SIGNAL_NAME(RasterLayer, DisplayedBandChanged),
         boost::any(pair<RasterChannelType, DimensionDescriptor>(eColor, band)));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setDisplayedBand(eColor, band, pRasterElement);
         }

         ++iter;
      }

      mbLinking = false;
   }
}

bool RasterLayerImp::setColorMap(const string& name, const vector<ColorType>& colorTable)
{
   try
   {
      return setColorMap(ColorMap(name.c_str(), colorTable));
   }
   catch (exception& exc)
   {
      QMessageBox::warning(NULL, "Bad ColorMap", exc.what());
      return false;
   }
}

bool RasterLayerImp::setColorMap(const ColorMap& colorMap)
{
   if (colorMap == mColorMap)
   {
      return false;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetRasterColorMap(dynamic_cast<RasterLayer*>(this), mColorMap, colorMap));
      }

      mColorMap = colorMap;
      mbRegenerate = true;
      emit colorMapChanged(mColorMap);
      notify(SIGNAL_NAME(RasterLayer, ColorMapChanged), boost::any(mColorMap.getName()));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setColorMap(colorMap);
         }
      }

      mbLinking = false;
   }

   return true;
}

const ColorMap& RasterLayerImp::getColorMap() const
{
   return mColorMap;
}

void RasterLayerImp::setComplexComponent(const ComplexComponent& eComponent)
{
   if (eComponent == mComplexComponent)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetRasterComplexComponent(dynamic_cast<RasterLayer*>(this), mComplexComponent,
            eComponent));
      }

      mComplexComponent = eComponent;
      mbRegenerate = true;
      emit complexComponentChanged(mComplexComponent);
      notify(SIGNAL_NAME(RasterLayer, ComplexComponentChanged), boost::any(eComponent));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setComplexComponent(eComponent);
         }
      }

      mbLinking = false;
   }
}

ComplexComponent RasterLayerImp::getComplexComponent() const
{
   return mComplexComponent;
}

StretchType RasterLayerImp::getStretchType(const DisplayMode& eMode) const
{
   StretchType eStretch;

   if (eMode == GRAYSCALE_MODE)
   {
      eStretch = meGrayStretchType;
   }
   else if (eMode == RGB_MODE)
   {
      eStretch = meRgbStretchType;
   }

   return eStretch;
}

RegionUnits RasterLayerImp::getStretchUnits(const RasterChannelType& eColor) const
{
   RegionUnits eUnits;

   if (eColor == GRAY)
   {
      eUnits = meGrayStretchUnits;
   }
   else if (eColor == RED)
   {
      eUnits = meRedStretchUnits;
   }
   else if (eColor == GREEN)
   {
      eUnits = meGreenStretchUnits;
   }
   else if (eColor == BLUE)
   {
      eUnits = meBlueStretchUnits;
   }

   return eUnits;
}

void RasterLayerImp::getStretchValues(const RasterChannelType& eColor, double& dLower, double& dUpper) const
{
   dLower = 0.0;
   dUpper = 0.0;

   switch (eColor)
   {
      case GRAY:
         dLower = mlstGrayStretchValues[0];
         dUpper = mlstGrayStretchValues[1];
         break;

      case RED:
         dLower = mlstRedStretchValues[0];
         dUpper = mlstRedStretchValues[1];
         break;

      case GREEN:
         dLower = mlstGreenStretchValues[0];
         dUpper = mlstGreenStretchValues[1];
         break;

      case BLUE:
         dLower = mlstBlueStretchValues[0];
         dUpper = mlstBlueStretchValues[1];
         break;

      default:
         break;
   }
}

unsigned int RasterLayerImp::getAlpha() const
{
   return mAlpha;
}

QString RasterLayerImp::getStretchUnitsAsString(const RasterChannelType& eColor) const
{
   RegionUnits eMethod = getStretchUnits(eColor);
   switch (eMethod)
   {
      case RAW_VALUE:
         break;

      case PERCENTAGE:
         return "%age";

      case PERCENTILE:
         return "%ile";

      case STD_DEV:
      {
         QChar tmpchar(0x03C3);    // Unicode sigma
         QString tmpstr;
         tmpstr.setUnicode(&tmpchar, 1);
         return tmpstr;
      }

      default:
         break;
   }

   return QString();
}

double RasterLayerImp::convertStretchValue(const RasterChannelType& eColor, const RegionUnits& eUnits,
                                           double dStretchValue, const RegionUnits& eNewUnits) const
{
   double dNewValue = 0.0;

   Statistics* pStatistics = getStatistics(eColor);
   if (pStatistics == NULL)
   {
      return dNewValue;
   }

   double dMin = pStatistics->getMin(mComplexComponent);
   double dMax = pStatistics->getMax(mComplexComponent);
   double dAverage = pStatistics->getAverage(mComplexComponent);
   double dStdDev = pStatistics->getStandardDeviation(mComplexComponent);
   const double* pdPercentiles = pStatistics->getPercentiles(mComplexComponent);

   // Convert the stretch value to a raw value
   double dRawValue = 0.0;
   switch (eUnits)
   {
      case RAW_VALUE:
         dRawValue = dStretchValue;
         break;

      case PERCENTAGE:
         dRawValue = (((dMax - dMin) * dStretchValue) / 100) + dMin;
         break;

      case PERCENTILE:
         dRawValue = percentileToRaw(dStretchValue, pdPercentiles);
         break;

      case STD_DEV:
         dRawValue = (dStretchValue * dStdDev) + dAverage;
         break;

      default:
         break;
   }

   // Convert the raw stretch value to the new value
   switch (eNewUnits)
   {
      case RAW_VALUE:
         dNewValue = dRawValue;
         break;

      case PERCENTAGE:
      {
         double range = 1.0;
         if (dMax != dMin)
         {
            range = dMax - dMin;
         }
         dNewValue = (((dRawValue - dMin) / range) * 100);
         break;
      }

      case PERCENTILE:
         dNewValue = rawToPercentile(dRawValue, pdPercentiles);
         break;

      case STD_DEV:
         if (dStdDev != 0.0)
         {
            dNewValue = (dRawValue - dAverage) / dStdDev;
         }
         break;

      default:
         break;
   }

   return dNewValue;
}

double RasterLayerImp::convertStretchValue(const RasterChannelType& eColor, double dStretchValue,
                                           const RegionUnits& eNewUnits) const
{
   RegionUnits eUnits = getStretchUnits(eColor);

   double dNewValue = 0.0;
   dNewValue = convertStretchValue(eColor, eUnits, dStretchValue, eNewUnits);

   return dNewValue;
}

bool RasterLayerImp::isFilterSupported(const string& filterName) const
{
   if (filterName.empty())
   {
      return false;
   }

   vector<string> supportedFilters = getSupportedFilters();

   vector<string>::iterator iter = find(supportedFilters.begin(), supportedFilters.end(), filterName);
   return (iter != supportedFilters.end());
}

vector<string> RasterLayerImp::getSupportedFilters() const
{
   vector<string> supportedFilters;
   if (isGpuImageSupported() == true)
   {
      Service<ImageFilterManager> pManager;
      supportedFilters = pManager->getAvailableFilters();
   }

   return supportedFilters;
}

void RasterLayerImp::enableFilter(const string& filterName)
{
   if ((filterName.empty() == true) || (isGpuImageEnabled() == false))
   {
      return;
   }

   if (mbLinking == false)
   {
      if (isFilterEnabled(filterName) == false)
      {
         Service<ImageFilterManager> pManager;

         ImageFilterDescriptor* pDescriptor = pManager->createFilterDescriptor(filterName);
         if (pDescriptor != NULL)
         {
            View* pView = getView();
            if (pView != NULL)
            {
               vector<ImageFilterDescriptor*> newFilters = mEnabledFilters;
               newFilters.push_back(pDescriptor);

               pView->addUndoAction(new SetRasterImageFilters(dynamic_cast<RasterLayer*>(this), mEnabledFilters,
                  newFilters));
            }

            mEnabledFilters.push_back(pDescriptor);
            mbRegenerate = true;
            emit filtersChanged(mEnabledFilters);
            notify(SIGNAL_NAME(RasterLayer, FiltersChanged), boost::any(mEnabledFilters));
         }
      }

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->enableFilter(filterName);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::enableFilters(const vector<string>& filterNames)
{
   vector<string> currentFilters = getEnabledFilterNames();
   if ((filterNames == currentFilters) || (isGpuImageEnabled() == false))
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetRasterImageFilters(dynamic_cast<RasterLayer*>(this), currentFilters,
            filterNames));
      }

      // Delete currently enabled filters that are not in the new enabled vector
      vector<string> newFilters = filterNames;

      vector<ImageFilterDescriptor*>::iterator enabledIter = mEnabledFilters.begin();
      while (enabledIter != mEnabledFilters.end())
      {
         ImageFilterDescriptor* pDescriptor = *enabledIter;
         VERIFYNRV(pDescriptor != NULL);

         const string& enabledName = pDescriptor->getName();

         vector<string>::iterator newIter = find(newFilters.begin(), newFilters.end(), enabledName);
         if (newIter == newFilters.end())
         {
#if defined (CG_SUPPORTED)
            ViewImp* pViewImp = dynamic_cast<ViewImp*>(getView());
            GpuImage* pGpuImage = dynamic_cast<GpuImage*>(getImage());

            if ((pGpuImage != NULL) && (pViewImp != NULL))
            {
               GlContextSave contextSave(pViewImp);
               pGpuImage->disableFilter(pDescriptor);
            }
#endif

            delete dynamic_cast<ImageFilterDescriptorImp*>(pDescriptor);
            enabledIter = mEnabledFilters.erase(enabledIter);
         }
         else
         {
            // Remove the filter name from the vector of names to enable
            newFilters.erase(newIter);
            ++enabledIter;
         }
      }

      // Add new image filter descriptors for the remaining filter names
      vector<string>::iterator newIter;
      for (newIter = newFilters.begin(); newIter != newFilters.end(); ++newIter)
      {
         string filterName = *newIter;
         if (filterName.empty() == false)
         {
            Service<ImageFilterManager> pManager;

            ImageFilterDescriptor* pDescriptor = pManager->createFilterDescriptor(filterName);
            if (pDescriptor != NULL)
            {
               mEnabledFilters.push_back(pDescriptor);
            }
         }
      }

      mbRegenerate = true;
      emit filtersChanged(mEnabledFilters);
      notify(SIGNAL_NAME(RasterLayer, FiltersChanged), boost::any(mEnabledFilters));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->enableFilters(filterNames);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::disableFilter(const string& filterName)
{
   if ((filterName.empty() == true) || (isGpuImageEnabled() == false))
   {
      return;
   }

   if (mbLinking == false)
   {
      ImageFilterDescriptor* pDescriptor = getEnabledFilter(filterName);
      if (pDescriptor != NULL)
      {
         vector<ImageFilterDescriptor*>::iterator iter =
            find(mEnabledFilters.begin(), mEnabledFilters.end(), pDescriptor);
         if (iter != mEnabledFilters.end())
         {
#if defined (CG_SUPPORTED)
            ViewImp* pViewImp = dynamic_cast<ViewImp*>(getView());
            GpuImage* pGpuImage = dynamic_cast<GpuImage*>(getImage());

            if ((pGpuImage != NULL) && (pViewImp != NULL))
            {
               GlContextSave contextSave(pViewImp);
               pGpuImage->disableFilter(pDescriptor);
            }
#endif

            vector<ImageFilterDescriptor*> oldFilters = mEnabledFilters;
            mEnabledFilters.erase(iter);

            View* pView = getView();
            if (pView != NULL)
            {
               pView->addUndoAction(new SetRasterImageFilters(dynamic_cast<RasterLayer*>(this), oldFilters,
                  mEnabledFilters));
            }

            delete dynamic_cast<ImageFilterDescriptorImp*>(pDescriptor);

            mbRegenerate = true;
            emit filtersChanged(mEnabledFilters);
            notify(SIGNAL_NAME(RasterLayer, FiltersChanged), boost::any(mEnabledFilters));
         }
      }

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->disableFilter(filterName);
         }
      }

      mbLinking = false;
   }
}

bool RasterLayerImp::isFilterEnabled(const string& filterName) const
{
   if (filterName.empty() == true)
   {
      return false;
   }

   ImageFilterDescriptor* pDescriptor = getEnabledFilter(filterName);
   return (pDescriptor != NULL);
}

ImageFilterDescriptor* RasterLayerImp::getEnabledFilter(const string& filterName) const
{
   if (filterName.empty() == true)
   {
      return NULL;
   }

   vector<ImageFilterDescriptor*>::const_iterator iter;
   for (iter = mEnabledFilters.begin(); iter != mEnabledFilters.end(); ++iter)
   {
      ImageFilterDescriptor* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         if (pDescriptor->getName() == filterName)
         {
            return pDescriptor;
         }
      }
   }

   return NULL;
}

const vector<ImageFilterDescriptor*>& RasterLayerImp::getEnabledFilters() const
{
   return mEnabledFilters;
}

vector<string> RasterLayerImp::getEnabledFilterNames() const
{
   vector<string> filterNames;

   vector<ImageFilterDescriptor*>::const_iterator iter;
   for (iter = mEnabledFilters.begin(); iter != mEnabledFilters.end(); ++iter)
   {
      ImageFilterDescriptor* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         string filterName = pDescriptor->getName();
         if (filterName.empty() == false)
         {
            filterNames.push_back(filterName);
         }
      }
   }

   return filterNames;
}

void RasterLayerImp::resetFilter(const string& filterName)
{
   if ((filterName.empty() == true) || (isGpuImageEnabled() == false))
   {
      return;
   }

   if (mbLinking == false)
   {
      ImageFilterDescriptor* pDescriptor = getEnabledFilter(filterName);
      if (pDescriptor != NULL)
      {
#if defined (CG_SUPPORTED)
         ViewImp* pViewImp = dynamic_cast<ViewImp*>(getView());
         GpuImage* pGpuImage = dynamic_cast<GpuImage*>(getImage());

         if ((pGpuImage != NULL) && (pViewImp != NULL))
         {
            GlContextSave contextSave(pViewImp);
            pGpuImage->resetFilter(pDescriptor);
            mbRegenerate = true;
         }
#endif
      }

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->resetFilter(filterName);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::freezeFilter(const string& filterName, bool toggle)
{
   if ((filterName.empty() == true) || (isGpuImageEnabled() == false))
   {
      return;
   }

   if (mbLinking == false)
   {
      ImageFilterDescriptor* pDescriptor = getEnabledFilter(filterName);
      if (pDescriptor != NULL)
      {
#if defined (CG_SUPPORTED)
         ViewImp* pViewImp = dynamic_cast<ViewImp*>(getView());
         GpuImage* pGpuImage = dynamic_cast<GpuImage*>(getImage());

         if ((pGpuImage != NULL) && (pViewImp != NULL))
         {
            GlContextSave contextSave(pViewImp);
            pGpuImage->freezeFilter(pDescriptor, toggle);
            mbRegenerate = true;
         }
#endif
      }

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->freezeFilter(filterName, toggle);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::getDefaultStretchValues(const RasterChannelType& eColor, double& dLower, double& dUpper)
{
   dLower = 0.0;
   dUpper = 0.0;

   if (eColor == GRAY)
   {
      dLower = RasterLayer::getSettingGrayLowerStretchValue();
      dUpper = RasterLayer::getSettingGrayUpperStretchValue();
   }
   else if (eColor == RED)
   {
      dLower = RasterLayer::getSettingRedLowerStretchValue();
      dUpper = RasterLayer::getSettingRedUpperStretchValue();
   }
   else if (eColor == GREEN)
   {
      dLower = RasterLayer::getSettingGreenLowerStretchValue();
      dUpper = RasterLayer::getSettingGreenUpperStretchValue();
   }
   else if (eColor == BLUE)
   {
      dLower = RasterLayer::getSettingBlueLowerStretchValue();
      dUpper = RasterLayer::getSettingBlueUpperStretchValue();
   }
}

unsigned int RasterLayerImp::getDefaultAlpha()
{
   return 255;
}

void RasterLayerImp::enableGpuImage(bool bEnable)
{
   if (bEnable == mUseGpuImage)
   {
      return;
   }

   if (isGpuImageSupported() == false)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetRasterGpuImage(dynamic_cast<RasterLayer*>(this), mUseGpuImage, bEnable));
      }

      mUseGpuImage = bEnable;
      mbRegenerate = true;
      emit gpuImageEnabled(mUseGpuImage);
      notify(SIGNAL_NAME(RasterLayer, GpuImageEnabled), boost::any(mUseGpuImage));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::const_iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*> (*iter);
         if (pLayer != NULL)
         {
            pLayer->enableGpuImage(bEnable);
         }

         ++iter;
      }

      mbLinking = false;
   }
}

void RasterLayerImp::setStretchType(const DisplayMode& eMode, const StretchType& eType)
{
   if (eMode == GRAYSCALE_MODE)
   {
      if (meGrayStretchType == eType)
      {
         return;
      }
   }
   else if (eMode == RGB_MODE)
   {
      if (meRgbStretchType == eType)
      {
         return;
      }
   }
   else
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         StretchType oldType = meGrayStretchType;
         if (eMode == RGB_MODE)
         {
            oldType = meRgbStretchType;
         }

         pView->addUndoAction(new SetRasterStretchType(dynamic_cast<RasterLayer*>(this), eMode, oldType, eType));
      }

      if (eMode == GRAYSCALE_MODE)
      {
         meGrayStretchType = eType;
      }
      else if (eMode == RGB_MODE)
      {
         meRgbStretchType = eType;
      }

      mbRegenerate = true;
      emit stretchTypeChanged(eMode, eType);
      notify(SIGNAL_NAME(RasterLayer, StretchTypeChanged), boost::any(pair<DisplayMode, StretchType>(eMode, eType)));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setStretchType(eMode, eType);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::setStretchUnits(const DisplayMode& eMode, const RegionUnits& eUnits)
{
   if (eMode == GRAYSCALE_MODE)
   {
      setStretchUnits(GRAY, eUnits);
   }
   else
   {
      setStretchUnits(RED, eUnits);
      setStretchUnits(GREEN, eUnits);
      setStretchUnits(BLUE, eUnits);
   }
}

void RasterLayerImp::setStretchUnits(const RasterChannelType& eColor, const RegionUnits& eUnits)
{
   if (eColor == GRAY)
   {
      if (meGrayStretchUnits == eUnits)
      {
         return;
      }
   }
   else if (eColor == RED)
   {
      if (meRedStretchUnits == eUnits)
      {
         return;
      }
   }
   else if (eColor == GREEN)
   {
      if (meGreenStretchUnits == eUnits)
      {
         return;
      }
   }
   else if (eColor == BLUE)
   {
      if (meBlueStretchUnits == eUnits)
      {
         return;
      }
   }
   else
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         RegionUnits oldUnits;
         switch (eColor)
         {
            case GRAY:
               oldUnits = meGrayStretchUnits;
               break;

            case RED:
               oldUnits = meRedStretchUnits;
               break;

            case GREEN:
               oldUnits = meGreenStretchUnits;
               break;

            case BLUE:
               oldUnits = meBlueStretchUnits;
               break;

            default:
               break;
         }


         pView->addUndoAction(new SetRasterStretchUnits(dynamic_cast<RasterLayer*>(this), eColor, oldUnits, eUnits));
      }

      if (eColor == GRAY)
      {
         meGrayStretchUnits = eUnits;
      }
      else if (eColor == RED)
      {
         meRedStretchUnits = eUnits;
      }
      else if (eColor == GREEN)
      {
         meGreenStretchUnits = eUnits;
      }
      else if (eColor == BLUE)
      {
         meBlueStretchUnits = eUnits;
      }

      mbRegenerate = true;
      emit stretchUnitsChanged(eColor, eUnits);
      notify(SIGNAL_NAME(RasterLayer, StretchUnitsChanged),
         boost::any(pair<RasterChannelType, RegionUnits>(eColor, eUnits)));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      for (vector<Layer*>::iterator iter = linkedLayers.begin();
           iter != linkedLayers.end(); ++iter)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setStretchUnits(eColor, eUnits);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::setStretchValues(const RasterChannelType& eColor, double dLower, double dUpper)
{
   switch (eColor)
   {
      case GRAY:
         if ((mlstGrayStretchValues[0] == dLower) && (mlstGrayStretchValues[1] == dUpper))
         {
            return;
         }

         break;

      case RED:
         if ((mlstRedStretchValues[0] == dLower) && (mlstRedStretchValues[1] == dUpper))
         {
            return;
         }

         break;

      case GREEN:
         if ((mlstGreenStretchValues[0] == dLower) && (mlstGreenStretchValues[1] == dUpper))
         {
            return;
         }

         break;

      case BLUE:
         if ((mlstBlueStretchValues[0] == dLower) && (mlstBlueStretchValues[1] == dUpper))
         {
            return;
         }

         break;

      default:
         break;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         double oldLowerValue = 0.0;
         double oldUpperValue = 0.0;

         switch (eColor)
         {
            case GRAY:
               oldLowerValue = mlstGrayStretchValues[0];
               oldUpperValue = mlstGrayStretchValues[1];
               break;

            case RED:
               oldLowerValue = mlstRedStretchValues[0];
               oldUpperValue = mlstRedStretchValues[1];
               break;

            case GREEN:
               oldLowerValue = mlstGreenStretchValues[0];
               oldUpperValue = mlstGreenStretchValues[1];
               break;

            case BLUE:
               oldLowerValue = mlstBlueStretchValues[0];
               oldUpperValue = mlstBlueStretchValues[1];
               break;

            default:
               break;
         }

         pView->addUndoAction(new SetRasterStretchValues(dynamic_cast<RasterLayer*>(this), eColor,
            oldLowerValue, dLower, oldUpperValue, dUpper));
      }

      switch (eColor)
      {
         case GRAY:
            mlstGrayStretchValues.clear();
            mlstGrayStretchValues.push_back(dLower);
            mlstGrayStretchValues.push_back(dUpper);
            break;

         case RED:
            mlstRedStretchValues.clear();
            mlstRedStretchValues.push_back(dLower);
            mlstRedStretchValues.push_back(dUpper);
            break;

         case GREEN:
            mlstGreenStretchValues.clear();
            mlstGreenStretchValues.push_back(dLower);
            mlstGreenStretchValues.push_back(dUpper);
            break;

         case BLUE:
            mlstBlueStretchValues.clear();
            mlstBlueStretchValues.push_back(dLower);
            mlstBlueStretchValues.push_back(dUpper);
            break;

         default:
            break;
      }

      mbRegenerate = true;
      emit stretchValuesChanged(eColor, dLower, dUpper);
      notify(SIGNAL_NAME(RasterLayer, StretchValuesChanged), boost::any(
         boost::tuple<RasterChannelType, double, double>(eColor, dLower, dUpper)));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setStretchValues(eColor, dLower, dUpper);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::setAlpha(unsigned int alpha)
{
   if (alpha > 255)
   {
      alpha = 255;
   }

   if (alpha == mAlpha)
   {
      return;
   }

   if (mbLinking == false)
   {
      bool oldFastContrast = canApplyFastContrastStretch();
      mAlpha = alpha;
      bool newFastContrast = canApplyFastContrastStretch();
      if (oldFastContrast != newFastContrast)
      {
         mbRegenerate = true;
      }

      if (mpImage != NULL)
      {
         mpImage->setAlpha(mAlpha);
      }

      mbRegenerate = true;
      emit alphaChanged(mAlpha);
      notify(SIGNAL_NAME(RasterLayer, AlphaChanged), boost::any(mAlpha));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setAlpha(alpha);
         }
      }

      mbLinking = false;
   }
}

void RasterLayerImp::reset()
{
   // Get the first active band to display in each color
   DimensionDescriptor grayBand;
   DimensionDescriptor redBand;
   DimensionDescriptor greenBand;
   DimensionDescriptor blueBand;
   DisplayMode eDisplayMode = GRAYSCALE_MODE;

   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         DimensionDescriptor grayDisplayBand = pDescriptor->getDisplayBand(GRAY);
         if (grayDisplayBand.isOriginalNumberValid())
         {
            grayBand = pDescriptor->getOriginalBand(grayDisplayBand.getOriginalNumber());
         }
         DimensionDescriptor redDisplayBand = pDescriptor->getDisplayBand(RED);
         if (redDisplayBand.isOriginalNumberValid())
         {
            redBand = pDescriptor->getOriginalBand(redDisplayBand.getOriginalNumber());
         }
         DimensionDescriptor greenDisplayBand = pDescriptor->getDisplayBand(GREEN);
         if (greenDisplayBand.isOriginalNumberValid())
         {
            greenBand = pDescriptor->getOriginalBand(greenDisplayBand.getOriginalNumber());
         }
         DimensionDescriptor blueDisplayBand = pDescriptor->getDisplayBand(BLUE);
         if (blueDisplayBand.isOriginalNumberValid())
         {
            blueBand = pDescriptor->getOriginalBand(blueDisplayBand.getOriginalNumber());
         }
         eDisplayMode = pDescriptor->getDisplayMode();

         // If no band is displayed in a color, set the displayed band as the first band
         const vector<DimensionDescriptor>& bands = pDescriptor->getBands();
         if (bands.empty() == false)
         {
            DimensionDescriptor firstBand = bands.front();
            if (!grayBand.isValid())
            {
               grayBand = firstBand;
            }

            if (!redBand.isValid() && !greenBand.isValid() && !blueBand.isValid())
            {
               redBand = firstBand;
               greenBand = firstBand;
               blueBand = firstBand;
            }
         }
      }
   }

   // Update the displayed bands and display mode
   setDisplayedBand(GRAY, grayBand, grayBand.isActiveNumberValid() ? pRasterElement : NULL);
   setDisplayedBand(RED, redBand, redBand.isActiveNumberValid() ? pRasterElement : NULL);
   setDisplayedBand(GREEN, greenBand, greenBand.isActiveNumberValid() ? pRasterElement : NULL);
   setDisplayedBand(BLUE, blueBand, blueBand.isActiveNumberValid() ? pRasterElement : NULL);
   setDisplayMode(eDisplayMode);

   double dGrayLower = 0.0;
   double dGrayUpper = 0.0;
   double dRedLower = 0.0;
   double dRedUpper = 0.0;
   double dGreenLower = 0.0;
   double dGreenUpper = 0.0;
   double dBlueLower = 0.0;
   double dBlueUpper = 0.0;

   getDefaultStretchValues(GRAY, dGrayLower, dGrayUpper);
   getDefaultStretchValues(RED, dRedLower, dRedUpper);
   getDefaultStretchValues(GREEN, dGreenLower, dGreenUpper);
   getDefaultStretchValues(BLUE, dBlueLower, dBlueUpper);

   setComplexComponent(RasterLayer::getSettingComplexComponent());
   setStretchType(GRAYSCALE_MODE, RasterLayer::getSettingGrayscaleStretchType());
   setStretchType(RGB_MODE, RasterLayer::getSettingRgbStretchType());
   setStretchUnits(GRAY, RasterLayer::getSettingGrayscaleStretchUnits());
   setStretchUnits(RED, RasterLayer::getSettingRedStretchUnits());
   setStretchUnits(GREEN, RasterLayer::getSettingGreenStretchUnits());
   setStretchUnits(BLUE, RasterLayer::getSettingBlueStretchUnits());
   setStretchValues(GRAY, dGrayLower, dGrayUpper);
   setStretchValues(RED, dRedLower, dRedUpper);
   setStretchValues(GREEN, dGreenLower, dGreenUpper);
   setStretchValues(BLUE, dBlueLower, dBlueUpper);
   setColorMap(ColorMap());
   setAlpha(RasterLayerImp::getDefaultAlpha());

   enableFilters(vector<string>());
   enableGpuImage(RasterLayer::getSettingGpuImage());
}

void RasterLayerImp::setDisplayMode(const DisplayMode& eMode)
{
   if (eMode == meDisplayMode)
   {
      return;
   }

   if (mbLinking == false)
   {
      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetRasterDisplayMode(dynamic_cast<RasterLayer*>(this), meDisplayMode, eMode));
      }

      meDisplayMode = eMode;

      bool usingGpu = false;
#if defined(CG_SUPPORTED)
      usingGpu = (dynamic_cast<GpuImage*>(mpImage) != NULL);
#endif
      if (usingGpu)
      {
         delete mpImage;
         mpImage = NULL;
      }

      mbRegenerate = true;

      emit displayModeChanged(meDisplayMode);
      notify(SIGNAL_NAME(RasterLayer, DisplayModeChanged), boost::any(meDisplayMode));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();
      for (vector<Layer*>::iterator iter = linkedLayers.begin(); iter != linkedLayers.end(); ++iter)
      {
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setDisplayMode(eMode);
         }
      }

      mbLinking = false;
   }
}

Image* RasterLayerImp::getImage()
{
   return mpImage;
}

void RasterLayerImp::setImage(Image* pImage)
{
   if ((mpImage != pImage) && (mpImage != NULL))
   {
      delete mpImage;
   }

   mpImage = pImage;

   if (pImage == NULL)
   {
      mbRegenerate = true;
   }
   else
   {
      mbRegenerate = false;
   }
}

void RasterLayerImp::setImageChanged(bool bChanged)
{
   mbRegenerate = bChanged;
}

vector<double> RasterLayerImp::getRawStretchValues(const RasterChannelType& eColor) const
{
   double dLower = 0.0;
   double dUpper = 0.0;
   getStretchValues(eColor, dLower, dUpper);

   RegionUnits eUnits = getStretchUnits(eColor);
   if (eUnits != RAW_VALUE)
   {
      dLower = convertStretchValue(eColor, dLower, RAW_VALUE);
      dUpper = convertStretchValue(eColor, dUpper, RAW_VALUE);
   }

   vector<double> lstStretchValues;
   lstStretchValues.push_back(dLower);
   lstStretchValues.push_back(dUpper);

   return lstStretchValues;
}

const vector<ColorType>& RasterLayerImp::getColorTable() const
{
   return mColorMap.getTable();
}

void RasterLayerImp::generateImage()
{
   RasterElement* pRaster = dynamic_cast<RasterElement*>(getDataElement());
   if (pRaster == NULL)
   {
      return;
   }

   unsigned int rows = 0;
   unsigned int columns = 0;
   unsigned int bands = 0;

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      rows = pDescriptor->getRowCount();
      columns = pDescriptor->getColumnCount();
      bands = pDescriptor->getBandCount();
   }

   DisplayMode eMode = getDisplayMode();
   StretchType eType = getStretchType(eMode);
   ComplexComponent eComponent = getComplexComponent();

   Image* pImage = getImage();

#if defined(CG_SUPPORTED)
   GpuImage* pGpuImage = dynamic_cast<GpuImage*>(pImage);
   bool bGpuImage = isGpuImageEnabled();

   if ((pGpuImage == NULL) && (bGpuImage == true))
   {
      pGpuImage = new GpuImage();
      pImage = pGpuImage;
   }
   else if ((pGpuImage != NULL) && (bGpuImage == false))
   {
      pImage = NULL;
   }
#endif

   if (pImage == NULL)
   {
      pImage = new Image();
      VERIFYNRV(pImage != NULL);
   }

   pImage->setAlpha(getAlpha());

   if (eMode == GRAYSCALE_MODE)
   {
      vector<double> lstStretchValues = getRawStretchValues(GRAY);

      RasterElement* pGrayElement = getDisplayedRasterElement(GRAY);
      if (pGrayElement == NULL)
      {
         return;
      }
      const RasterDataDescriptor* pGrayDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pGrayElement->getDataDescriptor());
      VERIFYNRV(pGrayDescriptor);
      EncodingType eEncodingGray = pGrayDescriptor->getDataType();
      vector<int> badValues;

      Statistics* pStatistics = getStatistics(GRAY);
      if (pStatistics != NULL)
      {
         badValues = pStatistics->getBadValues();
      }
      if (getColorMap().isDefault())
      {
         if (canApplyFastContrastStretch())
         {
            if (pStatistics != NULL)
            {
               lstStretchValues[0] = pStatistics->getMin(eComponent);
               lstStretchValues[1] = pStatistics->getMax(eComponent);
            }
         }

         GLenum format = (badValues.empty() ? GL_LUMINANCE:GL_LUMINANCE_ALPHA);
         pImage->initialize(512, 512, mGrayBand, columns, rows, bands, format,
            eEncodingGray, eComponent, NULL, eType, lstStretchValues, pGrayElement, badValues);
      }
      else // using a colormap
      {
         GLenum format = ((badValues.empty() && mColorMap.isFullyOpaque()) ? GL_RGB : GL_RGBA);
         pImage->initialize(512, 512, mGrayBand, columns, rows, bands, format, eEncodingGray, eComponent,
            NULL, eType, lstStretchValues, pGrayElement, getColorTable(), badValues);
      }
   }
   else if (eMode == RGB_MODE)
   {
      RasterElement* pRedElement = getDisplayedRasterElement(RED);
      RasterElement* pGreenElement = getDisplayedRasterElement(GREEN);
      RasterElement* pBlueElement = getDisplayedRasterElement(BLUE);

      vector<double> lstRedStretchValues = getRawStretchValues(RED);
      vector<double> lstGreenStretchValues = getRawStretchValues(GREEN);
      vector<double> lstBlueStretchValues = getRawStretchValues(BLUE);

      if (canApplyFastContrastStretch())
      {
         Statistics* pStatistics = getStatistics(RED);
         if (pStatistics != NULL)
         {
            lstRedStretchValues[0] = pStatistics->getMin(eComponent);
            lstRedStretchValues[1] = pStatistics->getMax(eComponent);
         }

         pStatistics = getStatistics(GREEN);
         if (pStatistics != NULL)
         {
            lstGreenStretchValues[0] = pStatistics->getMin(eComponent);
            lstGreenStretchValues[1] = pStatistics->getMax(eComponent);
         }

         pStatistics = getStatistics(BLUE);
         if (pStatistics != NULL)
         {
            lstBlueStretchValues[0] = pStatistics->getMin(eComponent);
            lstBlueStretchValues[1] = pStatistics->getMax(eComponent);
         }
      }

      const RasterDataDescriptor* pRedDescriptor = NULL;
      if (pRedElement != NULL)
      {
         pRedDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRedElement->getDataDescriptor());
      }

      const RasterDataDescriptor* pGreenDescriptor = NULL;
      if (pGreenElement != NULL)
      {
         pGreenDescriptor = dynamic_cast<const RasterDataDescriptor*>(pGreenElement->getDataDescriptor());
      }

      const RasterDataDescriptor* pBlueDescriptor = NULL;
      if (pBlueElement != NULL)
      {
         pBlueDescriptor = dynamic_cast<const RasterDataDescriptor*>(pBlueElement->getDataDescriptor());
      }

      EncodingType eRedEncoding;
      if (pRedDescriptor != NULL)
      {
         eRedEncoding = pRedDescriptor->getDataType();
      }

      EncodingType eGreenEncoding;
      if (pGreenDescriptor != NULL)
      {
         eGreenEncoding = pGreenDescriptor->getDataType();
      }

      EncodingType eBlueEncoding;
      if (pBlueDescriptor != NULL)
      {
         eBlueEncoding = pBlueDescriptor->getDataType();
      }

      pImage->initialize(512, 512, mRedBand, mGreenBand,
         mBlueBand, columns, rows, bands, GL_RGB, eRedEncoding, eGreenEncoding,
         eBlueEncoding, eComponent, NULL, eType, lstRedStretchValues, lstGreenStretchValues,
         lstBlueStretchValues, pRedElement, pGreenElement, pBlueElement);
   }

#if defined (CG_SUPPORTED)
   // Enable the filters after the image is initialized to ensure the tiles are created
   if ((pGpuImage != NULL) && (bGpuImage == true))
   {
      ViewImp* pViewImp = dynamic_cast<ViewImp*>(getView());
      if (pViewImp != NULL)
      {
         GlContextSave contextSave(pViewImp);
         pGpuImage->enableFilters(mEnabledFilters);
      }
   }
#endif

   setImage(pImage);
}

void RasterLayerImp::generateFullImage()
{
   Image* pImage = getImage();
   if (pImage != NULL)
   {
      pImage->generateAllFullResTextures();
   }
}

Statistics* RasterLayerImp::getStatistics(RasterChannelType eColor) const
{
   DimensionDescriptor band;
   RasterElement* pRasterElement = NULL;

   if (eColor == GRAY)
   {
      band = mGrayBand;
      pRasterElement = getDisplayedRasterElement(GRAY);
   }
   else if (eColor == RED)
   {
      band = mRedBand;
      pRasterElement = getDisplayedRasterElement(RED);
   }
   else if (eColor == GREEN)
   {
      band = mGreenBand;
      pRasterElement = getDisplayedRasterElement(GREEN);
   }
   else if (eColor == BLUE)
   {
      band = mBlueBand;
      pRasterElement = getDisplayedRasterElement(BLUE);
   }

   if (pRasterElement == NULL)
   {
      pRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   }

   if (pRasterElement == NULL)
   {
      // May be NULL while the RasterElement is being destroyed
      return NULL;
   }

   return pRasterElement->getStatistics(band);
}

double RasterLayerImp::percentileToRaw(double value, const double* pdPercentiles) const
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
   else if (lower > 999)
   {
      return pdPercentiles[1000];
   }

   return pdPercentiles[lower] + (pdPercentiles[lower + 1] - pdPercentiles[lower]) *
      (10.0 * value - static_cast<double>(lower));
}

double RasterLayerImp::rawToPercentile(double value, const double* pdPercentiles) const
{
   if (pdPercentiles == NULL)
   {
      return -1.0;
   }

   double range = 1.0;

   if (pdPercentiles[1000] != pdPercentiles[0])
   {
      range = pdPercentiles[1000] - pdPercentiles[0];
   }

   if (value < pdPercentiles[0] || value > pdPercentiles[1000])
   {
      return 100.0 * (value - pdPercentiles[0]) / range;
   }

   int i = 0;
   for (i = 0; i < 1000; i++)
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
   else if (lower > 999)
   {
      return 100.0;
   }

   range = pdPercentiles[lower + 1] - pdPercentiles[lower];
   if (range == 0.0)
   {
      range = 1.0;
   }

   return (lower + (value - pdPercentiles[lower]) / range) / 10.0;
}

bool RasterLayerImp::toXml(XMLWriter* pXml) const
{
   if (!LayerImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("useGpuImage", mUseGpuImage);
   pXml->addAttr("displayMode", meDisplayMode);
   pXml->addAttr("complexComponent", mComplexComponent);
   pXml->addAttr("stretchTypeGray", meGrayStretchType);
   pXml->addAttr("stretchTypeRgb", meRgbStretchType);
   pXml->addAttr("alpha", mAlpha);
   pXml->addAttr("enableFastStretch", mEnableFastContrastStretch);

   pXml->pushAddPoint(pXml->addElement("ChannelInfo"));
   pXml->addAttr("channel", RasterChannelType(GRAY));
   if (!channelToXml(pXml, getDisplayedRasterElement(GRAY), meGrayStretchUnits, mGrayBand, mlstGrayStretchValues))
   {
      return false;
   }
   pXml->popAddPoint();

   pXml->pushAddPoint(pXml->addElement("ChannelInfo"));
   pXml->addAttr("channel", RasterChannelType(RED));
   if (!channelToXml(pXml, getDisplayedRasterElement(RED), meRedStretchUnits, mRedBand, mlstRedStretchValues))
   {
      return false;
   }
   pXml->popAddPoint();

   pXml->pushAddPoint(pXml->addElement("ChannelInfo"));
   pXml->addAttr("channel", RasterChannelType(GREEN));
   if (!channelToXml(pXml, getDisplayedRasterElement(GREEN), meGreenStretchUnits, mGreenBand, mlstGreenStretchValues))
   {
      return false;
   }
   pXml->popAddPoint();

   pXml->pushAddPoint(pXml->addElement("ChannelInfo"));
   pXml->addAttr("channel", RasterChannelType(BLUE));
   if (!channelToXml(pXml, getDisplayedRasterElement(BLUE), meBlueStretchUnits, mBlueBand, mlstBlueStretchValues))
   {
      return false;
   }
   pXml->popAddPoint();

   if (mEnabledFilters.size() > 0)
   {
      pXml->pushAddPoint(pXml->addElement("ImageFilters"));
      for (vector<ImageFilterDescriptor*>::const_iterator fit = mEnabledFilters.begin();
               fit != mEnabledFilters.end(); ++fit)
      {
         pXml->addAttr("name", (*fit)->getName(), pXml->addElement("filter"));
      }
      pXml->popAddPoint();
   }

   if (mpAnimation != NULL)
   {
      pXml->addAttr("animationId", mpAnimation->getId());
   }

   return true;
}

bool RasterLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!LayerImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pDocElmnt = static_cast<DOMElement*>(pDocument);
   setDisplayMode(StringUtilities::fromXmlString<DisplayMode>(A(pDocElmnt->getAttribute(X("displayMode")))));
   setComplexComponent(StringUtilities::fromXmlString<ComplexComponent>(
      A(pDocElmnt->getAttribute(X("complexComponent")))));
   setStretchType(GRAYSCALE_MODE, StringUtilities::fromXmlString<StretchType>(
      A(pDocElmnt->getAttribute(X("stretchTypeGray")))));
   setStretchType(RGB_MODE, StringUtilities::fromXmlString<StretchType>(
      A(pDocElmnt->getAttribute(X("stretchTypeRgb")))));
   setAlpha(StringUtilities::fromXmlString<unsigned int>(A(pDocElmnt->getAttribute(X("alpha")))));
   enableFastContrastStretch(StringUtilities::fromXmlString<bool>(A(pDocElmnt->getAttribute(X("enableFastStretch")))));
   mpAnimation = NULL;
   if (pDocElmnt->hasAttribute(X("animationId")))
   {
      setAnimation(dynamic_cast<Animation*>(SessionManagerImp::instance()->getSessionItem(
         A(pDocElmnt->getAttribute(X("animationId"))))));
   }
   for (DOMNode* pNode = pDocElmnt->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("ChannelInfo")))
      {
         DOMElement* pElmnt = static_cast<DOMElement*>(pNode);
         DimensionDescriptor band;
         RasterElement* pDataElement = NULL;
         RegionUnits stretchUnits;
         double stretchValueMin = 0.0;
         double stretchValueMax = 0.0;
         if (!xmlToChannel(pElmnt, pDataElement, stretchUnits,
                          band, stretchValueMin, stretchValueMax))
         {
            return false;
         }
         RasterChannelType channel = StringUtilities::fromXmlString<RasterChannelType>(
            A(pElmnt->getAttribute(X("channel"))));
         if (!channel.isValid())
         {
            return false;
         }
         setDisplayedBand(channel, band, pDataElement);
         setStretchUnits(channel, stretchUnits);
         setStretchValues(channel, stretchValueMin, stretchValueMax);
      }
   }

   // have to restore display channels before restoring useGpuImage attribute
   enableGpuImage(StringUtilities::fromXmlString<bool>(A(pDocElmnt->getAttribute(X("useGpuImage")))));

   // Must restore the filters after the GPU image attribute to correctly enable them
   for (DOMNode* pNode = pDocElmnt->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("ImageFilters")))
      {
         vector<string> filters;
         for (DOMNode* pFiltNode = pNode->getFirstChild(); pFiltNode != NULL; pFiltNode = pFiltNode->getNextSibling())
         {
            if (XMLString::equals(pFiltNode->getNodeName(), X("filter")))
            {
               string name = A(static_cast<DOMElement*>(pFiltNode)->getAttribute(X("name")));
               if (name.empty() == false)
               {
                  filters.push_back(name);
               }
            }
         }
         enableFilters(filters);
      }
   }

   mbRegenerate = true;
   emit extentsModified();
   notify(SIGNAL_NAME(Layer, ExtentsModified));

   return true;
}

bool RasterLayerImp::serialize(SessionItemSerializer& serializer) const
{
   if (!LayerImp::serialize(serializer))
   {
      return false;
   }
   string colorMapData;
   if (!mColorMap.saveToBuffer(colorMapData))
   {
      return false;
   }
   serializer.endBlock();
   return serializer.serialize(colorMapData.c_str(), colorMapData.size());
}

bool RasterLayerImp::deserialize(SessionItemDeserializer& deserializer)
{
   if (!LayerImp::deserialize(deserializer))
   {
      return false;
   }
   deserializer.nextBlock();
   string colorMapData;
   colorMapData.resize(deserializer.getBlockSizes()[1]);
   if (!deserializer.deserialize(const_cast<char*>(colorMapData.c_str()), colorMapData.size()))
   {
      return false;
   }
   return mColorMap.loadFromBuffer(colorMapData);
}

bool RasterLayerImp::channelToXml(XMLWriter* pXml, const RasterElement* pElem, const RegionUnits& units,
                                  const DimensionDescriptor& descriptor, const vector<double>& values) const
{
   vector<double>::const_iterator it;
   if (units.isValid())
   {
      pXml->addAttr("stretchUnits", units);
   }
   pXml->pushAddPoint(pXml->addElement("DimensionDescriptor"));
   if (descriptor.isOriginalNumberValid())
   {
      pXml->addAttr("originalNumber", descriptor.getOriginalNumber());
   }
   if (descriptor.isOnDiskNumberValid())
   {
      pXml->addAttr("onDiskNumber", descriptor.getOnDiskNumber());
   }
   if (descriptor.isActiveNumberValid())
   {
      pXml->addAttr("activeNumber", descriptor.getActiveNumber());
   }
   pXml->popAddPoint();

   if (pElem != NULL)
   {
      pXml->addAttr("rasterElementId", pElem->getId());
   }

   if (values.size() > 0)
   {
      pXml->addText(values, pXml->addElement("stretchValues"));
   }

   return true;
}

bool RasterLayerImp::xmlToChannel(DOMNode* pDocument, RasterElement*& pElem, RegionUnits& units,
                                  DimensionDescriptor& descriptor, double& minValue, double& maxValue)
{
   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   if (pElement->hasAttribute(X("stretchUnits")))
   {
      units = StringUtilities::fromXmlString<RegionUnits>(
         A(pElement->getAttribute(X("stretchUnits"))));
   }
   if (pElement->hasAttribute(X("rasterElementId")))
   {
      pElem = dynamic_cast<RasterElement*>(SessionManagerImp::instance()->getSessionItem(
         A(pElement->getAttribute(X("rasterElementId")))));
   }
   else
   {
      pElem = NULL;
   }
   for (DOMNode* pNode = pElement->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("DimensionDescriptor")))
      {
         DimensionDescriptor dd;
         DOMElement* pElmnt = static_cast<DOMElement*>(pNode);
         if (pElmnt->hasAttribute(X("originalNumber")))
         {
            dd.setOriginalNumber(StringUtilities::fromXmlString<unsigned int>(
               A(pElmnt->getAttribute(X("originalNumber")))));
         }
         if (pElmnt->hasAttribute(X("onDiskNumber")))
         {
            dd.setOnDiskNumber(StringUtilities::fromXmlString<unsigned int>(
               A(pElmnt->getAttribute(X("onDiskNumber")))));
         }
         if (pElmnt->hasAttribute(X("activeNumber")))
         {
            dd.setActiveNumber(StringUtilities::fromXmlString<unsigned int>(
               A(pElmnt->getAttribute(X("activeNumber")))));
         }
         descriptor = dd;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("stretchValues")))
      {
         vector<double> values;
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(values, pNode->getTextContent());
         if (values.size() != 2)
         {
            return false;
         }
         minValue = values[0];
         maxValue = values[1];
      }
   }
   return true;
}

bool RasterLayerImp::canApplyFastContrastStretch() const
{
   bool isLinearGrayscale = mColorMap.isDefault() &&
      meDisplayMode == GRAYSCALE_MODE &&
      meGrayStretchType == LINEAR &&
      mlstGrayStretchValues[1] >= mlstGrayStretchValues[0];
   bool isLinearRgb = meDisplayMode == RGB_MODE &&
      meRgbStretchType == LINEAR &&
      mlstRedStretchValues[1] >= mlstRedStretchValues[0] &&
      mlstGreenStretchValues[1] >= mlstGreenStretchValues[0] &&
      mlstBlueStretchValues[1] >= mlstBlueStretchValues[0];

   return getGlBlendSubtractProc() != NULL &&
      (isLinearGrayscale || isLinearRgb) &&
      mAlpha == 255 &&
      mEnableFastContrastStretch;
}

RasterLayerImp::GlBlendSubtractProc RasterLayerImp::getGlBlendSubtractProc()
{
   static GlBlendSubtractProc proc = NULL;

// requires a Windows only call...might work with glX but this hasn't been explored yet
#if defined(WIN_API)
   static bool isInitialized = false;
   if (!isInitialized)
   {
      proc = (GlBlendSubtractProc)wglGetProcAddress("glBlendEquationEXT");
      isInitialized = true;
   }
#endif
   return proc;
}

void RasterLayerImp::applyFastContrastStretch()
{
   if (getDisplayMode() == GRAYSCALE_MODE)
   {
      applyFastContrastStretch(GRAY);
   }
   else
   {
      applyFastContrastStretch(RED);
      applyFastContrastStretch(GREEN);
      applyFastContrastStretch(BLUE);
   }
}

void RasterLayerImp::applyFastContrastStretch(RasterChannelType element)
{
   DataElement* pElement = getDataElement();
   VERIFYNRV(pElement != NULL);

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
   VERIFYNRV(pDescriptor != NULL);

   LocationType size(pDescriptor->getColumnCount(), pDescriptor->getRowCount());

   Statistics* pStatistics = getStatistics(element);
   if (pStatistics == NULL)
   {
      return;
   }

   const unsigned int* pHistogram = NULL;
   const double* pBinCenters = NULL;
   pStatistics->getHistogram(pBinCenters, pHistogram);
   VERIFYNRV(pBinCenters != NULL);
   VERIFYNRV(pHistogram != NULL);
   double minData = pStatistics->getMin(mComplexComponent);
   double maxData = pStatistics->getMax(mComplexComponent);
   double range = maxData - minData;

   vector<double> stretch = getRawStretchValues(element);

   double scale = 128.0;
   if (fabs(stretch[1]-stretch[0]) * 1e6 > range) // catch divide by 0.0
   {
      scale = range/(stretch[1]-stretch[0]);
   }
   double bias = 0.0;
   if (fabs(range) < fabs((stretch[0]-minData)))
   {
      bias = -1.0;
   }
   else
   {
      if (fabs(range) * 1e6 > fabs(stretch[0] - minData)) // catch divide by 0
      {
         bias = -(stretch[0] - minData)/range;
      }
   }

   double biases[3] = {0.0, 0.0, 0.0};
   double scales[3] = {0.0, 0.0, 0.0};
   double fullscales[3] = {0.0, 0.0, 0.0};
   switch (element)
   {
   case GRAY:
      biases[0] = biases[1] = biases[2] = bias; 
      scales[0] = scales[1] = scales[2] = scale;
      fullscales[0] = fullscales[1] = fullscales[2] = 1.0;
      break;
   case RED:
      biases[0] = bias; 
      scales[0] = scale;
      fullscales[0] = 1.0;
      break;
   case GREEN:
      biases[1] = bias; 
      scales[1] = scale;
      fullscales[1] = 1.0;
      break;
   case BLUE:
      biases[2] = bias; 
      scales[2] = scale;
      fullscales[2] = 1.0;
      break;
   default:
      break;
   }

   GlBlendSubtractProc glBlendEquationEXT;
   glBlendEquationEXT = getGlBlendSubtractProc();

   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);
   if (bias != 0) 
   {
      if (bias > 0) 
      {
         glColor4f(biases[0], biases[1], biases[2], 0.0);
      } 
      else 
      {
         if (glBlendEquationEXT)
         {
            glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);
         }

         int x = 5;
         x = 10;
         glColor4f(-biases[0], -biases[1], -biases[2], 0.0);
      }
      glRectd(0.0, 0.0, size.mX, size.mY);
   }
   float remainingScale;

   remainingScale = scale;
   if (glBlendEquationEXT)
   {
      glBlendEquationEXT(GL_FUNC_ADD_EXT);
   }

   glBlendFunc(GL_DST_COLOR, GL_ONE);
   if (remainingScale > 2.0) 
   {
      /* Clever cascading approach.  Example: if the
         scaling factor was 9.5, do 3 "doubling" blends
         (8x), then scale by the remaining 1.1875. */
      glColor4d(fullscales[0], fullscales[1], fullscales[2], 1);
      while (remainingScale > 2.0) 
      {
         glRectd(0.0, 0.0, size.mX, size.mY);
         remainingScale /= 2.0;
      }
   }
   switch (element)
   {
   case GRAY:
      scales[0] = remainingScale - 1.0;
      scales[1] = remainingScale - 1.0;
      scales[2] = remainingScale - 1.0;
      break;
   case RED:
      scales[0] = remainingScale - 1.0;
      break;
   case GREEN:
      scales[1] = remainingScale - 1.0;
      break;
   case BLUE:
      scales[2] = remainingScale - 1.0;
      break;
   default:
      break;
   }
   glColor4f(scales[0], scales[1], scales[2], 1);
   glRectd(0.0, 0.0, size.mX, size.mY);

   if (glBlendEquationEXT)
   {
      glBlendEquationEXT(GL_FUNC_ADD_EXT);
   }

   glDisable(GL_BLEND);
}

bool RasterLayerImp::enableFastContrastStretch(bool enable)
{
   return false;
}

bool RasterLayerImp::generateFullResTexture()
{
   if (mpImage == NULL)
   {
      return false;
   }

   return mpImage->generateFullResTexture();
}

void RasterLayerImp::setDisplayMode(QAction* pAction)
{
   DisplayMode displayMode;
   if (pAction == mpGrayscaleAction)
   {
      displayMode = GRAYSCALE_MODE;
   }
   else if (pAction == mpRgbAction)
   {
      displayMode = RGB_MODE;
   }

   setDisplayMode(displayMode);
}

void RasterLayerImp::updateDisplayModeAction(const DisplayMode& displayMode)
{
   if (displayMode == GRAYSCALE_MODE)
   {
      mpGrayscaleAction->setChecked(true);
   }
   else if (displayMode == RGB_MODE)
   {
      mpRgbAction->setChecked(true);
   }
}

void RasterLayerImp::changeStretch(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   RegionUnits eUnits = PERCENTILE;
   StretchType eType = LINEAR;
   double lower = 5.0;
   double upper = 95.0;

   if (pAction == mpLinear0Action)
   {
      eType = LINEAR;
      lower = 0.0;
      upper = 100.0;
   }
   else if (pAction == mpLinear2Action) 
   {
      eType = LINEAR;
      lower = 2.0;
      upper = 98.0;
   }
   else if (pAction == mpLinear5Action)
   {
      eType = LINEAR;
      lower = 5.0;
      upper = 95.0;
   }
   else if (pAction == mpEqualAction)
   {
      eType = EQUALIZATION;
   }
   else
   {
      return;
   }

   DisplayMode eMode = getDisplayMode();
   setStretchUnits(eMode, eUnits);
   setStretchType(eMode, eType);

   switch (eMode)
   {
   case GRAYSCALE_MODE:
      setStretchValues(GRAY, lower, upper);
      break;

   case RGB_MODE:
      setStretchValues(RED, lower, upper);
      setStretchValues(GREEN, lower, upper);
      setStretchValues(BLUE, lower, upper);
      break;
   default:
      break;
   }

}

void RasterLayerImp::setAnimation(Animation* pAnimation)
{
   if (pAnimation == mpAnimation)
   {
      return;
   }

   if (mpAnimation != NULL)
   {
      mpAnimation->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterLayerImp::movieUpdated));
      mpAnimation->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::movieDeleted));
   }

   mpAnimation = pAnimation;

   if (mpAnimation != NULL)
   {
      mpAnimation->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterLayerImp::movieUpdated));
      mpAnimation->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &RasterLayerImp::movieDeleted));
   }

   notify(SIGNAL_NAME(RasterLayer, AnimationChanged), boost::any(mpAnimation));
   updateFromMovie();
}

void RasterLayerImp::movieUpdated(Subject& subject, const string& signal, const boost::any& v)
{
   Animation* pAnimation = dynamic_cast<Animation*> (&subject);
   if (mpAnimation != NULL && pAnimation == mpAnimation)
   {
      updateFromMovie();
   }
}

void RasterLayerImp::updateFromMovie()
{
   if (mpAnimation == NULL)
   {
      return;
   }
   const AnimationFrame* pFrame = mpAnimation->getCurrentFrame();
   if (pFrame == NULL)
   {
      setDisplayMode(GRAYSCALE_MODE);
      setDisplayedBand(GRAY, DimensionDescriptor());
      return;
   }
   UndoLock lock(getView());

   unsigned int frameNumber = pFrame->mFrameNumber;
   if (getDisplayMode() == RGB_MODE)
   {
      const RasterDataDescriptor* pRedRasterDesc = mpRedRasterElement.get() == NULL ? NULL :
         static_cast<const RasterDataDescriptor*>(mpRedRasterElement->getDataDescriptor());
      const RasterDataDescriptor* pGreenRasterDesc = mpGreenRasterElement.get() == NULL ? NULL :
         static_cast<const RasterDataDescriptor*>(mpGreenRasterElement->getDataDescriptor());
      const RasterDataDescriptor* pBlueRasterDesc = mpBlueRasterElement.get() == NULL ? NULL :
         static_cast<const RasterDataDescriptor*>(mpBlueRasterElement->getDataDescriptor());
      if (pRedRasterDesc != NULL && frameNumber < pRedRasterDesc->getBandCount())
      {
         setDisplayedBand(RED, pRedRasterDesc->getActiveBand(frameNumber), mpRedRasterElement.get());
      }
      else
      {
         setDisplayedBand(RED, DimensionDescriptor(), mpRedRasterElement.get());
      }
      if (pGreenRasterDesc != NULL && frameNumber < pGreenRasterDesc->getBandCount())
      {
         setDisplayedBand(GREEN, pGreenRasterDesc->getActiveBand(frameNumber), mpGreenRasterElement.get());
      }
      else
      {
         setDisplayedBand(GREEN, DimensionDescriptor(), mpGreenRasterElement.get());
      }
      if (pBlueRasterDesc != NULL && frameNumber < pBlueRasterDesc->getBandCount())
      {
         setDisplayedBand(BLUE, pBlueRasterDesc->getActiveBand(frameNumber), mpBlueRasterElement.get());
      }
      else
      {
         setDisplayedBand(BLUE, DimensionDescriptor(), mpBlueRasterElement.get());
      }
   }
   else
   {
      const RasterDataDescriptor* pDescriptor = getDataElement() == NULL ? NULL :
         static_cast<const RasterDataDescriptor*>(getDataElement()->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         setDisplayedBand(GRAY, pDescriptor->getActiveBand(frameNumber));
      }
   }
}

void RasterLayerImp::movieDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   Animation* pAnimation = dynamic_cast<Animation*> (&subject);
   if (mpAnimation != NULL && pAnimation == mpAnimation)
   {
      mpAnimation = NULL;
   }
}

Animation* RasterLayerImp::getAnimation() const
{
   return mpAnimation;
}

void RasterLayerImp::displayAsTrueColor()
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(getDataElement()->getDataDescriptor());
   if (RasterUtilities::setDisplayBandsToTrueColor(pDescriptor))
   {
      UndoGroup group(getView(), "Display As True Color");

      setDisplayedBand(RED, pDescriptor->getDisplayBand(RED));
      setDisplayedBand(GREEN, pDescriptor->getDisplayBand(GREEN));
      setDisplayedBand(BLUE, pDescriptor->getDisplayBand(BLUE));
      setDisplayMode(RGB_MODE);
   }
}

unsigned int RasterLayerImp::readFilterBuffer(double xCoord, double yCoord, int width, int height,
                                              vector<float>& values)
{
   unsigned int numElements = 0;

#if defined (CG_SUPPORTED)
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getView());
   if (pViewImp != NULL)
   {
      GlContextSave contextSave(pViewImp);

      // make sure the image has been drawn before trying to read from it
      draw();

      GpuImage* pGpuImage = dynamic_cast<GpuImage*>(getImage());
      if (pGpuImage != NULL)
      {
         numElements = pGpuImage->readTiles(xCoord, yCoord, width, height, values);
      }
   }
#endif
   return numElements;
}
