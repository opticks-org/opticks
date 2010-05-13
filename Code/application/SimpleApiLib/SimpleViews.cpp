/*
* The information in this file is
* Copyright(c) 2007 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AppConfig.h"
#include "ColorMap.h"
#include "DesktopServices.h"
#include "FileFinder.h"
#include "Layer.h"
#include "LayerList.h"
#include "ObjectResource.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SessionManager.h"
#include "SimpleApiErrors.h"
#include "SimpleViews.h"
#include "SpatialDataView.h"
#include "Statistics.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"
#include "TypeConverter.h"
#include "Undo.h"
#include "View.h"
#include "ViewWindow.h"

#include <QtGui/QWidget>
#include <float.h>
#include <string>
#include <vector>
#if defined(WIN_API)
#define isnan _isnan
#endif

namespace
{
   std::vector<std::string> splitIdentifier(const std::string& name)
   {
      return StringUtilities::split(name, '|');
   }

   uint32_t encodeColor(ColorType color)
   {
      return (color.mRed << 24) | (color.mGreen << 16) | (color.mBlue << 8) | color.mAlpha;
   }

   ColorType decodeColor(uint32_t val)
   {
      ColorType color;
      color.mRed = (val >> 24) & 0xff;
      color.mGreen = (val >> 16) & 0xff;
      color.mBlue = (val >> 8) & 0xff;
      color.mAlpha = val & 0xff;
      return color;
   }
}

extern "C"
{

   Layer* getLayer(const char* pName, const char* pType)
   {
      Layer* pLayer = NULL;
      const std::string name(pName == NULL ? std::string() : pName);
      const std::string type(pType == NULL ? std::string() : pType);
      SessionItem* pSessionItem = Service<SessionManager>()->getSessionItem(name);
      if (pSessionItem != NULL)
      {
         pLayer = dynamic_cast<Layer*>(pSessionItem);
         if (pLayer == NULL || (!type.empty() && !pLayer->isKindOf(type)))
         {
            setLastError(SIMPLE_WRONG_TYPE);
            return NULL;
         }
      }
      else
      {
         std::vector<std::string> id = splitIdentifier(name);
         SpatialDataView* pView = static_cast<SpatialDataView*>(getView(id.size() == 0 ? NULL : id.front().c_str(),
            TypeConverter::toString<SpatialDataView>()));
         LayerList* pLayerList = (pView == NULL) ? NULL : pView->getLayerList();
         if (pLayerList == NULL)
         {
            setLastError(SIMPLE_NOT_FOUND);
            return NULL;
         }
         if (id.size() < 2)
         {
            if (!type.empty() && type == TypeConverter::toString<RasterLayer>())
            {
               pLayer = pLayerList->getLayer(RASTER, pLayerList->getPrimaryRasterElement());
            }
            else
            {
               pLayer = pView->getActiveLayer();
               if (pLayer != NULL && !type.empty() && !pLayer->isKindOf(type))
               {
                  pLayer = NULL;
               }
               if (pLayer == NULL)
               {
                  if (type.empty())
                  {
                     pLayer = pView->getTopMostLayer();
                  }
                  else
                  {
                     pLayer = pView->getTopMostLayer(StringUtilities::fromDisplayString<LayerType>(type));
                  }
               }
            }
         }
         if (pLayer == NULL)
         {
            std::vector<Layer*> layers;
            pLayerList->getLayers(layers);
            for (std::vector<Layer*>::reverse_iterator layer = layers.rbegin();
               layer != layers.rend();
               ++layer)
            {
               if ((type.empty() || (*layer)->isKindOf(type)) && 
                  (id.empty() || (*layer)->getName() == id.back() || (*layer)->getDisplayName() == id.back()))
               {
                  pLayer = *layer;
                  break;
               }
            }
         }
         if (pLayer == NULL)
         {
            setLastError(SIMPLE_NOT_FOUND);
            return NULL;
         }
      }
      setLastError(SIMPLE_NO_ERROR);
      return pLayer;
   }

   void destroyLayer(Layer* pLayer)
   {
      SpatialDataView* pSdv = (pLayer == NULL) ? NULL : dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pSdv != NULL)
      {
         pSdv->deleteLayer(pLayer);
      }
   }

   uint32_t getLayerName(Layer* pLayer, char* pName, uint32_t nameSize)
   {
      if (pLayer == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pLayer->getName();
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   uint32_t getLayerType(Layer* pLayer, char* pType, uint32_t typeSize)
   {
      if (pLayer == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type = StringUtilities::toXmlString(pLayer->getLayerType());
      if (typeSize == 0)
      {
         return type.size() + 1;
      }
      else if (type.size() > typeSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pType, type.c_str(), type.size());
      pType[type.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return type.size();
   }

   DataElement* getLayerElement(Layer* pLayer)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pLayer->getDataElement();
   }

   View* getLayerView(Layer* pLayer)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pLayer->getView();
   }

   int getLayerScaleOffset(Layer* pLayer, double* pScaleX, double* pScaleY, double* pOffsetX, double* pOffsetY)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (pScaleX != NULL)
      {
         *pScaleX = pLayer->getXScaleFactor();
      }
      if (pScaleY != NULL)
      {
         *pScaleY = pLayer->getYScaleFactor();
      }
      if (pOffsetX != NULL)
      {
         *pOffsetX = pLayer->getXOffset();
      }
      if (pOffsetY != NULL)
      {
         *pOffsetY = pLayer->getYOffset();
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setLayerScaleOffset(Layer* pLayer, double scaleX, double scaleY, double offsetX, double offsetY)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (isnan(scaleX) == 0)
      {
         pLayer->setXScaleFactor(scaleX);
      }
      if (isnan(scaleY) == 0)
      {
         pLayer->setYScaleFactor(scaleY);
      }
      if (isnan(offsetX) == 0)
      {
         pLayer->setXOffset(offsetX);
      }
      if (isnan(offsetY) == 0)
      {
         pLayer->setYOffset(offsetY);
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int isLayerDisplayed(Layer* pLayer)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return static_cast<int>(pView->isLayerDisplayed(pLayer));
   }

   int setLayerDisplayed(Layer* pLayer, int displayed)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 1;
      }
      bool success = true;
      if (displayed != 0)
      {
         if (!pView->isLayerDisplayed(pLayer))
         {
            success = pView->showLayer(pLayer);
         }
      }
      else
      {
         if (pView->isLayerDisplayed(pLayer))
         {
            success = pView->hideLayer(pLayer);
         }
      }
      if (!success)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   Layer* deriveLayer(Layer* pLayer, const char* pName, const char* pType)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return NULL;
      }
      pLayer = pView->deriveLayer(pLayer, StringUtilities::fromXmlString<LayerType>(std::string(pType)));
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      if (pName != NULL)
      {
         pView->getLayerList()->renameLayer(pLayer, std::string(pName));
      }
      setLastError(SIMPLE_NO_ERROR);
      return pLayer;
   }

   Layer* convertLayer(Layer* pLayer, const char* pType)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return NULL;
      }
      pLayer = pView->convertLayer(pLayer, StringUtilities::fromXmlString<LayerType>(std::string(pType)));
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pLayer;
   }

   int isLayerActive(Layer* pLayer)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return static_cast<int>(pView->getActiveLayer() == pLayer);
   }

   int activateLayer(Layer* pLayer)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 1;
      }
      pView->setActiveLayer(pLayer);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getLayerDisplayIndex(Layer* pLayer)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pView->getLayerDisplayIndex(pLayer);
   }

   int setLayerDisplayIndex(Layer* pLayer, uint32_t newIndex)
   {
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 1;
      }
      if (!pView->setLayerDisplayIndex(pLayer, newIndex))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int getThresholdLayerInfo(Layer* pLayer, struct ThresholdLayerInfo* pInfo)
   {
      ThresholdLayer* pThresh = dynamic_cast<ThresholdLayer*>(pLayer);
      if (pThresh == NULL || pInfo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      // convert the threshold values to the current threshold region units
      pInfo->firstThreshold = pThresh->convertThreshold(RAW_VALUE,
         pThresh->getFirstThreshold(), pThresh->getRegionUnits());
      pInfo->secondThreshold = pThresh->convertThreshold(RAW_VALUE,
         pThresh->getSecondThreshold(), pThresh->getRegionUnits());
      switch(pThresh->getPassArea())
      {
      case LOWER:
         pInfo->passArea = 0;
         break;
      case UPPER:
         pInfo->passArea = 1;
         break;
      case MIDDLE:
         pInfo->passArea = 2;
         break;
      case OUTSIDE:
         pInfo->passArea = 3;
         break;
      default:
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      switch(pThresh->getRegionUnits())
      {
      case RAW_VALUE:
         pInfo->regionUnits = 0;
         break;
      case PERCENTAGE:
         pInfo->regionUnits = 1;
         break;
      case PERCENTILE:
         pInfo->regionUnits = 2;
         break;
      case STD_DEV:
         pInfo->regionUnits = 3;
         break;
      default:
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setThresholdLayerInfo(Layer* pLayer, struct ThresholdLayerInfo* pInfo)
   {
      ThresholdLayer* pThresh = dynamic_cast<ThresholdLayer*>(pLayer);
      if (pThresh == NULL || pInfo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      switch(pInfo->passArea)
      {
      case 0:
         pThresh->setPassArea(LOWER);
         break;
      case 1:
         pThresh->setPassArea(UPPER);
         break;
      case 2:
         pThresh->setPassArea(MIDDLE);
         break;
      case 3:
         pThresh->setPassArea(OUTSIDE);
         break;
      default:
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      switch(pInfo->regionUnits)
      {
      case 0:
         pThresh->setRegionUnits(RAW_VALUE);
         break;
      case 1:
         pThresh->setRegionUnits(PERCENTAGE);
         break;
      case 2:
         pThresh->setRegionUnits(PERCENTILE);
         break;
      case 3:
         pThresh->setRegionUnits(STD_DEV);
         break;
      default:
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      // convert the thresholds to raw from the specified units
      pThresh->setFirstThreshold(pThresh->convertThreshold(pInfo->firstThreshold, RAW_VALUE));
      pThresh->setSecondThreshold(pThresh->convertThreshold(pInfo->secondThreshold, RAW_VALUE));

      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getPseudocolorClassCount(Layer* pLayer)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPseudo->getClassCount();
   }

   int32_t getPseudocolorClassId(Layer* pLayer, uint32_t index)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return -1;
      }
      std::vector<int> classIds;
      pPseudo->getClassIDs(classIds);
      if (index >= classIds.size())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return -1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return classIds[index];
   }

   uint32_t getPseudocolorClassName(Layer* pLayer, int32_t id, char* pName, uint32_t nameSize)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name;
      if (!pPseudo->getClassName(id, name))
      {
         setLastError(SIMPLE_NOT_FOUND);
         return 0;
      }
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   int32_t getPseudocolorClassValue(Layer* pLayer, int32_t id)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPseudo->getClassValue(id);
   }

   uint32_t getPseudocolorClassColor(Layer* pLayer, int32_t id)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      ColorType color = pPseudo->getClassColor(id);
      if (!color.isValid())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return encodeColor(color);
   }

   int isPseudocolorClassDisplayed(Layer* pLayer, int32_t id)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return static_cast<int>(pPseudo->isClassDisplayed(id));
   }

   int32_t addPseudocolorClass(Layer* pLayer, const char* pName, int32_t* pValue, uint32_t* pColor, int* pDisplayed)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return -1;
      }
      if (pName != NULL && (pValue == NULL || pColor == NULL || pDisplayed == NULL))
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return -1;
      }
      if (pName == NULL && (pValue != NULL || pColor != NULL || pDisplayed != NULL))
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return -1;
      }
      int32_t classId = 0;
      if (pName == NULL)
      {
         classId = pPseudo->addClass();
      }
      else
      {
         ColorType color = decodeColor(*pColor);
         classId = pPseudo->addInitializedClass(std::string(pName), *pValue, color, *pDisplayed != 0);
      }
      if (classId == -1)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return -1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return classId;
   }

   int setPseudocolorClassName(Layer* pLayer, int32_t id, const char* pName)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (!pPseudo->setClassName(id, std::string(pName)))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setPseudocolorClassValue(Layer* pLayer, int32_t id, int32_t value)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (!pPseudo->setClassValue(id, value))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setPseudocolorClassColor(Layer* pLayer, int32_t id, uint32_t color)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      ColorType cval = decodeColor(color);
      if (!pPseudo->setClassColor(id, cval))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setPseudocolorClassDisplayed(Layer* pLayer, int32_t id, int display)
   {
      PseudocolorLayer* pPseudo = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (!pPseudo->setClassDisplayed(id, display != 0))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int getRasterLayerStretchInfo(Layer* pLayer, uint32_t channel, struct RasterLayerStretchInfo* pInfo)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      RasterChannelType chan(static_cast<RasterChannelTypeEnum>(channel));
      if (pRaster == NULL || pInfo == NULL || !chan.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      DisplayMode mode(chan == GRAY ? GRAYSCALE_MODE : RGB_MODE);
      pRaster->getStretchValues(chan, pInfo->lowerStretch, pInfo->upperStretch);
      pInfo->stretchType = pRaster->getStretchType(mode);
      pInfo->stretchUnits = pRaster->getStretchUnits(chan);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setRasterLayerStretchInfo(Layer* pLayer, uint32_t channel, struct RasterLayerStretchInfo* pInfo)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      RasterChannelType chan(static_cast<RasterChannelTypeEnum>(channel));
      if (pRaster == NULL || pInfo == NULL || !chan.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      DisplayMode mode(chan == GRAY ? GRAYSCALE_MODE : RGB_MODE);
      pRaster->setStretchValues(chan, pInfo->lowerStretch, pInfo->upperStretch);
      pRaster->setStretchType(mode, static_cast<StretchTypeEnum>(pInfo->stretchType));
      pRaster->setStretchUnits(chan, static_cast<RegionUnitsEnum>(pInfo->stretchUnits));
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getRasterLayerComplexComponent(Layer* pLayer)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pRaster->getComplexComponent();
   }

   int setRasterLayerComplexComponent(Layer* pLayer, uint32_t component)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      ComplexComponent comp(static_cast<ComplexComponentEnum>(component));
      if (pRaster == NULL || !comp.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pRaster->setComplexComponent(comp);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getRasterLayerColormapName(Layer* pLayer, char* pName, uint32_t nameSize)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pRaster->getColorMapName();
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   int setRasterLayerColormapName(Layer* pLayer, const char* pName)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      std::string name(pName);
      ColorMap map;
      if (!map.loadFromFile(name))
      {
         // see if name is a colormap name
         const Filename* pSupportFiles = Service<ConfigurationSettings>()->getSettingSupportFilesPath();
         if (pSupportFiles == NULL)
         {
            setLastError(SIMPLE_OTHER_FAILURE);
            return 1;
         }
         std::string mapDir = pSupportFiles->getFullPathAndName() + SLASH + "ColorTables" + SLASH;
         if (!map.loadFromFile(mapDir + name) && !map.loadFromFile(mapDir + name + ".clu")
          && !map.loadFromFile(mapDir + name + ".cgr"))
         {
            // scan all the default files and check for a valid name
            bool located = false;
            FactoryResource<FileFinder> pFinder;
            pFinder->findFile(mapDir, "*.c??");
            while (pFinder->findNextFile())
            {
               std::string filePath;
               pFinder->getFullPath(filePath);
               if (map.loadFromFile(filePath) && map.getName() == name)
               {
                  located = true;
                  break;
               }
            }
            if (!located)
            {
               setLastError(SIMPLE_NOT_FOUND);
               return 1;
            }
         }
      }
      pRaster->setColorMap(map.getName(), map.getTable());
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int getRasterLayerColormapValues(Layer* pLayer, uint32_t* pColormap)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL || pColormap == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      const std::vector<ColorType>& colormap = pRaster->getColorMap();
      if (colormap.size() != 256)
      {
         setLastError(SIMPLE_NO_MEM);
         return 1;
      }
      for (size_t idx = 0; idx < colormap.size(); ++idx)
      {
         pColormap[idx] = encodeColor(colormap[idx]);
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setRasterLayerColormapValues(Layer* pLayer, const char* pName, uint32_t* pColormap)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL || pColormap == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      std::vector<ColorType> colormap;
      colormap.reserve(256);
      for (size_t idx = 0; idx < 256; ++idx)
      {
         colormap.push_back(decodeColor(pColormap[idx]));
      }
      pRaster->setColorMap(std::string((pName == NULL) ? "" : pName), colormap);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int isRasterLayerRgbDisplayed(Layer* pLayer)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return static_cast<int>(pRaster->getDisplayMode() == RGB_MODE);
   }

   int setRasterLayerRgbDisplayed(Layer* pLayer, int rgb)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pRaster->setDisplayMode(rgb == 0 ? GRAYSCALE_MODE : RGB_MODE);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getRasterLayerDisplayedBand(Layer* pLayer, uint32_t channel, DataElement** pElement)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      RasterChannelType chan(static_cast<RasterChannelTypeEnum>(channel));
      if (pRaster == NULL || !chan.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      if (pElement != NULL)
      {
         *pElement = pRaster->getDisplayedRasterElement(chan);
      }
      setLastError(SIMPLE_NO_ERROR);
      return pRaster->getDisplayedBand(chan).getActiveNumber();
   }

   int setRasterLayerDisplayedBand(Layer* pLayer, uint32_t channel, uint32_t band, DataElement* pElement)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      RasterChannelType chan(static_cast<RasterChannelTypeEnum>(channel));
      if (pRaster == NULL || !chan.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      RasterElement* pRasterElement =
         (pElement == NULL) ? pRaster->getDisplayedRasterElement(chan) : dynamic_cast<RasterElement*>(pElement);
      if (pRasterElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      DimensionDescriptor bandDD(static_cast<const RasterDataDescriptor*>(
         pRasterElement->getDataDescriptor())->getActiveBand(band));
      pRaster->setDisplayedBand(chan, bandDD, pRasterElement);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int getRasterLayerStatistics(Layer* pLayer, uint32_t channel,
                                uint32_t component, struct RasterStatistics* pStatistics)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      RasterChannelType chan(static_cast<RasterChannelTypeEnum>(channel));
      ComplexComponent comp(static_cast<ComplexComponentEnum>(component));
      if (pRaster == NULL || pStatistics == NULL || !chan.isValid() || !comp.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      Statistics* pStats = pRaster->getStatistics(chan);
      if (pStats == NULL || !pStats->areStatisticsCalculated(comp))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      pStatistics->min = pStats->getMin(comp);
      pStatistics->max = pStats->getMax(comp);
      pStatistics->mean = pStats->getAverage(comp);
      pStatistics->stddev = pStats->getStandardDeviation(comp);
      const double* pTmp = NULL;
      const unsigned int* pTmp2 = NULL;
      pStats->getHistogram(pTmp, pTmp2, comp);
      pStatistics->pHistogramCenters = const_cast<double*>(pTmp);
      pStatistics->pHistogramCounts = const_cast<unsigned int*>(pTmp2);
      pStatistics->pPercentiles = const_cast<double*>(pStats->getPercentiles(comp));
      pStatistics->resolution = pStats->getStatisticsResolution();

      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int getRasterLayerGpuEnabled(Layer* pLayer)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return static_cast<int>(pRaster->isGpuImageEnabled());
   }

   int setRasterLayerGpuEnabled(Layer* pLayer, int gpu)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (!pRaster->isGpuImageSupported())
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      pRaster->enableGpuImage(gpu != 0);
      return 0;
   }

   uint32_t getRasterLayerFilterCount(Layer* pLayer, int enabled)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return (enabled != 0) ? pRaster->getEnabledFilterNames().size() : pRaster->getSupportedFilters().size();
   }

   uint32_t getRasterLayerFilterName(Layer* pLayer, uint32_t index, int enabled, char* pName, uint32_t nameSize)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::vector<std::string> filters =
         (enabled != 0) ? pRaster->getEnabledFilterNames() : pRaster->getSupportedFilters();
      if (index >= filters.size())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = filters[index];
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   int setRasterLayerFilters(Layer* pLayer, uint32_t count, const char** pFilters)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL || (count > 0 && pFilters == NULL))
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      std::vector<std::string> filters;
      filters.reserve(count);
      for (size_t idx = 0; idx < count; ++idx)
      {
         if (pFilters[idx] == NULL)
         {
            setLastError(SIMPLE_BAD_PARAMS);
            return 1;
         }
         filters.push_back(std::string(pFilters[idx]));
      }
      pRaster->enableFilters(filters);
      pRaster->getView()->refresh();
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int resetRasterLayerFilter(Layer* pLayer, const char* pFilterName)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL || pFilterName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pRaster->resetFilter(std::string(pFilterName));
      pRaster->getView()->refresh();
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int setRasterLayerFilterFrozen(Layer* pLayer, const char* pFilterName, int freeze)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pRaster == NULL || pFilterName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pRaster->freezeFilter(std::string(pFilterName), freeze != 0);
      pRaster->getView()->refresh();
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   View* getView(const char* pName, const char* pType)
   {
      View* pView = NULL;
      const std::string name(pName == NULL ? std::string() : pName);
      const std::string type(pType == NULL ? std::string() : pType);
      SessionItem* pSessionItem = Service<SessionManager>()->getSessionItem(name);
      if (pSessionItem != NULL)
      {
         pView = dynamic_cast<View*>(pSessionItem);
         if (pView == NULL || (!type.empty() && !pView->isKindOf(type)))
         {
            setLastError(SIMPLE_WRONG_TYPE);
            return NULL;
         }
      }
      else
      {
         std::vector<std::string> id = splitIdentifier(name);
         if (id.empty() || id.front().empty())
         {
            pView = Service<DesktopServices>()->getCurrentWorkspaceWindowView();
            if (pView == NULL)
            {
               setLastError(SIMPLE_NOT_FOUND);
               return NULL;
            }
            else
            {
               if (!type.empty() && !pView->isKindOf(type))
               {
                  setLastError(SIMPLE_WRONG_TYPE);
                  return NULL;
               }
            }
         }
         else
         {
            std::vector<Window*> windows;
            Service<DesktopServices>()->getWindows(windows);
            for (std::vector<Window*>::iterator window = windows.begin(); window != windows.end(); ++window)
            {
               ViewWindow* pTmp = dynamic_cast<ViewWindow*>(*window);
               View* pTmpView = pTmp == NULL ? NULL : pTmp->getView();
               if (pTmpView != NULL && (pTmpView->getName() == id.front() || pTmpView->getDisplayName() == id.front()))
               {
                  if (!type.empty() && !pTmpView->isKindOf(type))
                  {
                     setLastError(SIMPLE_WRONG_TYPE);
                     return NULL;
                  }
                  else
                  {
                     if (id.size() == 1)
                     {
                        pView = pTmpView;
                     }
                     else
                     {
                        SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pTmpView);
                        if (pSpatialDataView != NULL)
                        {
                           LayerList* pLayerList = pSpatialDataView->getLayerList();
                           if (pLayerList != NULL)
                           {
                              std::vector<Layer*> layers;
                              pLayerList->getLayers(layers);
                              for (std::vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
                              {
                                 Layer* pLayer = *iter;
                                 if (pLayer != NULL && 
                                    (pLayer->getName() == id.back() || pLayer->getDisplayName() == id.back()))
                                 {
                                    pView = pTmpView;
                                    break;
                                 }
                              }
                           }
                        }
                     }
                  }
                  break;
               }
            }
            if (pView == NULL)
            {
               setLastError(SIMPLE_NOT_FOUND);
               return NULL;
            }
         }
      }
      setLastError(SIMPLE_NO_ERROR);
      return pView;
   }

   View* createView(const char* pName, const char* pType, DataElement* pElement)
   {
      if (pName == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      ViewType type(StringUtilities::fromXmlString<ViewType>(std::string(pType)));
      RasterElement* pRaster = dynamic_cast<RasterElement*>(pElement);
      if (type == SPATIAL_DATA_VIEW && pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      WindowType windowType;
      switch(type)
      {
      case SPATIAL_DATA_VIEW:
         windowType = SPATIAL_DATA_WINDOW;
         break;
      case PRODUCT_VIEW:
         windowType = PRODUCT_WINDOW;
         break;
      case PLOT_VIEW:
         windowType = PLOT_WINDOW;
         break;
      }
      Window* pWindow = Service<DesktopServices>()->createWindow(std::string(pName), windowType);
      View* pView = pWindow == NULL ? NULL : static_cast<ViewWindow*>(pWindow)->getView();
      if (pView == NULL)
      {
         if (Service<DesktopServices>()->getWindow(std::string(pName), windowType) != NULL)
         {
            setLastError(SIMPLE_EXISTS);
         }
         else
         {
            setLastError(SIMPLE_OTHER_FAILURE);
         }
         return NULL;
      }
      if (type == SPATIAL_DATA_VIEW)
      {
         SpatialDataView* pSdv = static_cast<SpatialDataView*>(pView);
         if (!pSdv->setPrimaryRasterElement(pRaster))
         {
            Service<DesktopServices>()->deleteWindow(pWindow);
            setLastError(SIMPLE_OTHER_FAILURE);
            return NULL;
         }
         RasterLayer* pLayer = NULL;
         { // scope
            UndoLock lock(pView);
            if (pSdv->createLayer(RASTER, pRaster) == NULL)
            {
               Service<DesktopServices>()->deleteWindow(pWindow);
               setLastError(SIMPLE_OTHER_FAILURE);
               return NULL;
            }
         }
      }
      setLastError(SIMPLE_NO_ERROR);
      return pView;
   }

   void destroyView(View* pView)
   {
      Service<DesktopServices>()->deleteView(pView);
   }

   uint32_t getViewName(View* pView, char* pName, uint32_t nameSize)
   {
      if (pView == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pView->getName();
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   int setViewName(View* pView, const char* pName)
   {
      if (pView == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pView->setName(std::string(pName));
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getViewType(View* pView, char* pType, uint32_t typeSize)
   {
      if (pView == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type = StringUtilities::toXmlString(pView->getViewType());
      if (typeSize == 0)
      {
         return type.size() + 1;
      }
      else if (type.size() > typeSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pType, type.c_str(), type.size());
      pType[type.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return type.size();
   }

   DataElement* getViewPrimaryRasterElement(View* pView)
   {
      if (pView == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pView);
      if (pSdv == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pSdv->getLayerList()->getPrimaryRasterElement();
   }

   Layer* createLayer(View* pView, DataElement* pElement, const char* pType, const char* pName)
   {
      if (pView == NULL || pElement == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pView);
      if (pSdv == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return NULL;
      }
      LayerType type(StringUtilities::fromXmlString<LayerType>(std::string(pType)));
      if (!type.isValid())
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      Layer* pLayer;
      if (pName == NULL)
      {
         pLayer = pSdv->createLayer(type, pElement);
      }
      else
      {
         pLayer = pSdv->createLayer(type, pElement, std::string(pName));
      }
      if (pLayer == NULL)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pLayer;
   }

   uint32_t getViewLayerCount(View* pView)
   {
      if (pView == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pView);
      if (pSdv == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pSdv->getLayerList()->getNumLayers();
   }

   Layer* getViewLayer(View* pView, uint32_t index)
   {
      if (pView == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pView);
      if (pSdv == NULL)
      {
         setLastError(SIMPLE_WRONG_VIEW_TYPE);
         return NULL;
      }
      if (index >= pSdv->getLayerList()->getNumLayers())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      std::vector<Layer*> layers;
      pSdv->getLayerList()->getLayers(layers);
      setLastError(SIMPLE_NO_ERROR);
      return layers[index];
   }
};
