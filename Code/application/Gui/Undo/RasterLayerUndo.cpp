/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorMap.h"
#include "ImageFilterDescriptor.h"
#include "RasterLayer.h"
#include "RasterLayerUndo.h"
#include "SessionManager.h"

using namespace std;

///////////////////////
// SetRasterGpuImage //
///////////////////////

SetRasterGpuImage::SetRasterGpuImage(RasterLayer* pLayer, bool oldGpuImage, bool newGpuImage) :
   UndoAction(pLayer),
   mOldGpuImage(oldGpuImage),
   mNewGpuImage(newGpuImage)
{
   setText("Enable Raster Dynamic Texture Generation");
}

void SetRasterGpuImage::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->enableGpuImage(mOldGpuImage);
   }
}

void SetRasterGpuImage::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->enableGpuImage(mNewGpuImage);
   }
}

//////////////////////////
// SetRasterDisplayMode //
//////////////////////////

SetRasterDisplayMode::SetRasterDisplayMode(RasterLayer* pLayer, DisplayMode oldMode, DisplayMode newMode) :
   UndoAction(pLayer),
   mOldMode(oldMode),
   mNewMode(newMode)
{
   setText("Set Raster Display Mode");
}

void SetRasterDisplayMode::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setDisplayMode(mOldMode);
   }
}

void SetRasterDisplayMode::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setDisplayMode(mNewMode);
   }
}

///////////////////////
// SetRasterColorMap //
///////////////////////

SetRasterColorMap::SetRasterColorMap(RasterLayer* pLayer, const ColorMap& oldColorMap, const ColorMap& newColorMap) :
   UndoAction(pLayer),
   mOldColorMap(oldColorMap),
   mNewColorMap(newColorMap)
{
   setText("Set Raster Color Map");
}

void SetRasterColorMap::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColorMap(mOldColorMap);
   }
}

void SetRasterColorMap::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColorMap(mNewColorMap);
   }
}

///////////////////////////////
// SetRasterComplexComponent //
///////////////////////////////

SetRasterComplexComponent::SetRasterComplexComponent(RasterLayer* pLayer, ComplexComponent oldComponent,
                                                     ComplexComponent newComponent) :
   UndoAction(pLayer),
   mOldComplexComponent(oldComponent),
   mNewComplexComponent(newComponent)
{
   setText("Set Raster Complex Data Component");
}

void SetRasterComplexComponent::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setComplexComponent(mOldComplexComponent);
   }
}

void SetRasterComplexComponent::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setComplexComponent(mNewComplexComponent);
   }
}

//////////////////////////
// SetRasterStretchType //
//////////////////////////

SetRasterStretchType::SetRasterStretchType(RasterLayer* pLayer, DisplayMode displayMode,
                                           StretchType oldStretchType, StretchType newStretchType) :
   UndoAction(pLayer),
   mDisplayMode(displayMode),
   mOldStretchType(oldStretchType),
   mNewStretchType(newStretchType)
{
   setText("Set Raster Stretch Type");
}

void SetRasterStretchType::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStretchType(mDisplayMode, mOldStretchType);
   }
}

void SetRasterStretchType::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStretchType(mDisplayMode, mNewStretchType);
   }
}

///////////////////////////
// SetRasterStretchUnits //
///////////////////////////

SetRasterStretchUnits::SetRasterStretchUnits(RasterLayer* pLayer, RasterChannelType channel, RegionUnits oldUnits,
                                             RegionUnits newUnits) :
   UndoAction(pLayer),
   mChannel(channel),
   mOldUnits(oldUnits),
   mNewUnits(newUnits)
{
   setText("Set Raster Stretch Units");
}

void SetRasterStretchUnits::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStretchUnits(mChannel, mOldUnits);
   }
}

void SetRasterStretchUnits::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStretchUnits(mChannel, mNewUnits);
   }
}

////////////////////////////
// SetRasterStretchValues //
////////////////////////////

SetRasterStretchValues::SetRasterStretchValues(RasterLayer* pLayer, RasterChannelType channel, double oldLowerValue,
                                               double newLowerValue, double oldUpperValue, double newUpperValue) :
   UndoAction(pLayer),
   mChannel(channel),
   mOldLower(oldLowerValue),
   mNewLower(newLowerValue),
   mOldUpper(oldUpperValue),
   mNewUpper(newUpperValue)
{
   setText("Set Raster Stretch Values");
}

void SetRasterStretchValues::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStretchValues(mChannel, mOldLower, mOldUpper);
   }
}

void SetRasterStretchValues::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStretchValues(mChannel, mNewLower, mNewUpper);
   }
}

////////////////////////////
// SetRasterDisplayedBand //
////////////////////////////

SetRasterDisplayedBand::SetRasterDisplayedBand(RasterLayer* pLayer, RasterChannelType channel,
                                               DimensionDescriptor oldBand, DimensionDescriptor newBand,
                                               RasterElement* pOldElement, RasterElement* pNewElement) :
   UndoAction(pLayer),
   mChannel(channel),
   mOldBand(oldBand),
   mNewBand(newBand)
{
   if (pOldElement != NULL)
   {
      mOldElementId = pOldElement->getId();
   }

   if (pNewElement != NULL)
   {
      mNewElementId = pNewElement->getId();
   }

   setText("Set Raster Displayed Band");
}

void SetRasterDisplayedBand::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      if (oldId == mOldElementId)
      {
         mOldElementId = newId;
      }

      if (oldId == mNewElementId)
      {
         mNewElementId = newId;
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void SetRasterDisplayedBand::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      Service<SessionManager> pManager;
      RasterElement* pElement = dynamic_cast<RasterElement*>(pManager->getSessionItem(mOldElementId));
      pLayer->setDisplayedBand(mChannel, mOldBand, pElement);
   }
}

void SetRasterDisplayedBand::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      Service<SessionManager> pManager;
      RasterElement* pElement = dynamic_cast<RasterElement*>(pManager->getSessionItem(mNewElementId));
      pLayer->setDisplayedBand(mChannel, mNewBand, pElement);
   }
}

///////////////////////////
// SetRasterImageFilters //
///////////////////////////

SetRasterImageFilters::SetRasterImageFilters(RasterLayer* pLayer, const vector<ImageFilterDescriptor*>& oldFilters,
                                             const vector<ImageFilterDescriptor*>& newFilters) :
   UndoAction(pLayer)
{
   for (vector<ImageFilterDescriptor*>::const_iterator iter = oldFilters.begin(); iter != oldFilters.end(); ++iter)
   {
      ImageFilterDescriptor* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         const string& filterName = pDescriptor->getName();
         if (filterName.empty() == false)
         {
            mOldImageFilters.push_back(filterName);
         }
      }
   }

   for (vector<ImageFilterDescriptor*>::const_iterator iter = newFilters.begin(); iter != newFilters.end(); ++iter)
   {
      ImageFilterDescriptor* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         const string& filterName = pDescriptor->getName();
         if (filterName.empty() == false)
         {
            mNewImageFilters.push_back(filterName);
         }
      }
   }

   setText("Set Raster Image Filters");
}

SetRasterImageFilters::SetRasterImageFilters(RasterLayer* pLayer, const vector<string>& oldFilterNames,
                                             const vector<string>& newFilterNames) :
   UndoAction(pLayer),
   mOldImageFilters(oldFilterNames),
   mNewImageFilters(newFilterNames)
{
   setText("Set Raster Image Filters");
}

void SetRasterImageFilters::executeUndo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->enableFilters(mOldImageFilters);
   }
}

void SetRasterImageFilters::executeRedo()
{
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->enableFilters(mNewImageFilters);
   }
}

////////////////////////
// RasterLayerMemento //
////////////////////////

RasterLayerMemento::RasterLayerMemento(RasterLayer* pLayer) :
   mUseGpuImage(false),
   mGrayLower(0.0),
   mGrayUpper(0.0),
   mRedLower(0.0),
   mRedUpper(0.0),
   mGreenLower(0.0),
   mGreenUpper(0.0),
   mBlueLower(0.0),
   mBlueUpper(0.0),
   mAlpha(255),
   mEnableFastContrastStretch(false),
   mpAnimation(NULL)
{
   mUseGpuImage = pLayer->isGpuImageEnabled();
   mDisplayMode = pLayer->getDisplayMode();
   mComplexComponent = pLayer->getComplexComponent();
   mGrayStretchType = pLayer->getStretchType(GRAYSCALE_MODE);
   mRgbStretchType = pLayer->getStretchType(RGB_MODE);
   mGrayStretchUnits = pLayer->getStretchUnits(GRAY);
   mRedStretchUnits = pLayer->getStretchUnits(RED);
   mGreenStretchUnits = pLayer->getStretchUnits(GREEN);
   mBlueStretchUnits = pLayer->getStretchUnits(BLUE);
   mGrayBand = pLayer->getDisplayedBand(GRAY);
   mRedBand = pLayer->getDisplayedBand(RED);
   mGreenBand = pLayer->getDisplayedBand(GREEN);
   mBlueBand = pLayer->getDisplayedBand(BLUE);
   mpGrayRasterElement.reset(pLayer->getDisplayedRasterElement(GRAY));
   mpRedRasterElement.reset(pLayer->getDisplayedRasterElement(RED));
   mpGreenRasterElement.reset(pLayer->getDisplayedRasterElement(GREEN));
   mpBlueRasterElement.reset(pLayer->getDisplayedRasterElement(BLUE));
   pLayer->getStretchValues(GRAY, mGrayLower, mGrayUpper);
   pLayer->getStretchValues(RED, mRedLower, mRedUpper);
   pLayer->getStretchValues(GREEN, mGreenLower, mGreenUpper);
   pLayer->getStretchValues(BLUE, mBlueLower, mBlueUpper);
   mColorMap = pLayer->getColorMap();
   mAlpha = pLayer->getAlpha();
   mEnableFastContrastStretch = pLayer->canApplyFastContrastStretch();
   mEnabledFilters = pLayer->getEnabledFilterNames();
   mpAnimation = pLayer->getAnimation();
}

void RasterLayerMemento::toLayer(Layer* pLayer) const
{
   RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
   if (pRasterLayer != NULL)
   {
      pRasterLayer->enableGpuImage(mUseGpuImage);
      pRasterLayer->setDisplayMode(mDisplayMode);
      pRasterLayer->setComplexComponent(mComplexComponent);
      pRasterLayer->setStretchType(GRAYSCALE_MODE, mGrayStretchType);
      pRasterLayer->setStretchType(RGB_MODE, mRgbStretchType);
      pRasterLayer->setStretchUnits(GRAY, mGrayStretchUnits);
      pRasterLayer->setStretchUnits(RED, mRedStretchUnits);
      pRasterLayer->setStretchUnits(GREEN, mGreenStretchUnits);
      pRasterLayer->setStretchUnits(BLUE, mBlueStretchUnits);
      pRasterLayer->setDisplayedBand(GRAY, mGrayBand, const_cast<RasterElement*>(mpGrayRasterElement.get()));
      pRasterLayer->setDisplayedBand(RED, mRedBand, const_cast<RasterElement*>(mpRedRasterElement.get()));
      pRasterLayer->setDisplayedBand(GREEN, mGreenBand, const_cast<RasterElement*>(mpGreenRasterElement.get()));
      pRasterLayer->setDisplayedBand(BLUE, mBlueBand, const_cast<RasterElement*>(mpBlueRasterElement.get()));
      pRasterLayer->setStretchValues(GRAY, mGrayLower, mGrayUpper);
      pRasterLayer->setStretchValues(RED, mRedLower, mRedUpper);
      pRasterLayer->setStretchValues(GREEN, mGreenLower, mGreenUpper);
      pRasterLayer->setStretchValues(BLUE, mBlueLower, mBlueUpper);
      pRasterLayer->setColorMap(mColorMap);
      pRasterLayer->setAlpha(mAlpha);
      pRasterLayer->enableFilters(mEnabledFilters);
      pRasterLayer->setAnimation(mpAnimation);
   }
}
