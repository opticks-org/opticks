/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERLAYERUNDO_H
#define RASTERLAYERUNDO_H

#include "ColorMap.h"
#include "ComplexData.h"
#include "DimensionDescriptor.h"
#include "LayerUndo.h"
#include "RasterElement.h"
#include "SafePtr.h"
#include "TypesFile.h"
#include "UndoAction.h"

#include <string>
#include <vector>

class Animation;
class ImageFilterDescriptor;
class RasterLayer;
class RasterElement;

class SetRasterGpuImage : public UndoAction
{
public:
   SetRasterGpuImage(RasterLayer* pLayer, bool oldGpuImage, bool newGpuImage);

   void executeUndo();
   void executeRedo();

private:
   SetRasterGpuImage(const SetRasterGpuImage& rhs);
   SetRasterGpuImage& operator=(const SetRasterGpuImage& rhs);
   bool mOldGpuImage;
   bool mNewGpuImage;
};


class SetRasterDisplayMode : public UndoAction
{
public:
   SetRasterDisplayMode(RasterLayer* pLayer, DisplayMode oldMode, DisplayMode newMode);

   void executeUndo();
   void executeRedo();

private:
   SetRasterDisplayMode(const SetRasterDisplayMode& rhs);
   SetRasterDisplayMode& operator=(const SetRasterDisplayMode& rhs);
   DisplayMode mOldMode;
   DisplayMode mNewMode;
};


class SetRasterColorMap : public UndoAction
{
public:
   SetRasterColorMap(RasterLayer* pLayer, const ColorMap& oldColorMap, const ColorMap& newColorMap);

   void executeUndo();
   void executeRedo();

private:
   SetRasterColorMap(const SetRasterColorMap& rhs);
   SetRasterColorMap& operator=(const SetRasterColorMap& rhs);
   ColorMap mOldColorMap;
   ColorMap mNewColorMap;
};


class SetRasterComplexComponent : public UndoAction
{
public:
   SetRasterComplexComponent(RasterLayer* pLayer, ComplexComponent oldComponent, ComplexComponent newComponent);

   void executeUndo();
   void executeRedo();

private:
   SetRasterComplexComponent(const SetRasterComplexComponent& rhs);
   SetRasterComplexComponent& operator=(const SetRasterComplexComponent& rhs);
   ComplexComponent mOldComplexComponent;
   ComplexComponent mNewComplexComponent;
};


class SetRasterStretchType : public UndoAction
{
public:
   SetRasterStretchType(RasterLayer* pLayer, DisplayMode displayMode, StretchType oldStretchType,
      StretchType newStretchType);

   void executeUndo();
   void executeRedo();

private:
   SetRasterStretchType(const SetRasterStretchType& rhs);
   SetRasterStretchType& operator=(const SetRasterStretchType& rhs);
   DisplayMode mDisplayMode;
   StretchType mOldStretchType;
   StretchType mNewStretchType;
};


class SetRasterStretchUnits : public UndoAction
{
public:
   SetRasterStretchUnits(RasterLayer* pLayer, RasterChannelType channel, RegionUnits oldUnits, RegionUnits newUnits);

   void executeUndo();
   void executeRedo();

private:
   SetRasterStretchUnits(const SetRasterStretchUnits& rhs);
   SetRasterStretchUnits& operator=(const SetRasterStretchUnits& rhs);
   RasterChannelType mChannel;
   RegionUnits mOldUnits;
   RegionUnits mNewUnits;
};


class SetRasterStretchValues : public UndoAction
{
public:
   SetRasterStretchValues(RasterLayer* pLayer, RasterChannelType channel, double oldLowerValue, double newLowerValue,
      double oldUpperValue, double newUpperValue);

   void executeUndo();
   void executeRedo();

private:
   SetRasterStretchValues(const SetRasterStretchValues& rhs);
   SetRasterStretchValues& operator=(const SetRasterStretchValues& rhs);
   RasterChannelType mChannel;
   double mOldLower;
   double mNewLower;
   double mOldUpper;
   double mNewUpper;
};


class SetRasterDisplayedBand : public UndoAction
{
public:
   SetRasterDisplayedBand(RasterLayer* pLayer, RasterChannelType channel, DimensionDescriptor oldBand,
      DimensionDescriptor newBand, RasterElement* pOldElement, RasterElement* pNewElement);

   void updateSessionItem(const std::string& oldId, const std::string& newId);

   void executeUndo();
   void executeRedo();

private:
   SetRasterDisplayedBand(const SetRasterDisplayedBand& rhs);
   SetRasterDisplayedBand& operator=(const SetRasterDisplayedBand& rhs);
   RasterChannelType mChannel;
   DimensionDescriptor mOldBand;
   DimensionDescriptor mNewBand;
   std::string mOldElementId;
   std::string mNewElementId;
};


class SetRasterImageFilters : public UndoAction
{
public:
   SetRasterImageFilters(RasterLayer* pLayer, const std::vector<ImageFilterDescriptor*>& oldFilters,
      const std::vector<ImageFilterDescriptor*>& newFilters);
   SetRasterImageFilters(RasterLayer* pLayer, const std::vector<std::string>& oldFilterNames,
      const std::vector<std::string>& newFilterNames);

   void executeUndo();
   void executeRedo();

private:
   SetRasterImageFilters(const SetRasterImageFilters& rhs);
   SetRasterImageFilters& operator=(const SetRasterImageFilters& rhs);
   std::vector<std::string> mOldImageFilters;
   std::vector<std::string> mNewImageFilters;
};


class RasterLayerMemento : public LayerMemento
{
public:
   RasterLayerMemento(RasterLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   RasterLayerMemento(const RasterLayerMemento& rhs);
   RasterLayerMemento& operator=(const RasterLayerMemento& rhs);
   bool mUseGpuImage;

   DisplayMode mDisplayMode;
   ComplexComponent mComplexComponent;
   StretchType mGrayStretchType;
   StretchType mRgbStretchType;
   RegionUnits mGrayStretchUnits;
   RegionUnits mRedStretchUnits;
   RegionUnits mGreenStretchUnits;
   RegionUnits mBlueStretchUnits;

   DimensionDescriptor mGrayBand;
   DimensionDescriptor mRedBand;
   DimensionDescriptor mGreenBand;
   DimensionDescriptor mBlueBand;

   SafePtr<RasterElement> mpGrayRasterElement;
   SafePtr<RasterElement> mpRedRasterElement;
   SafePtr<RasterElement> mpGreenRasterElement;
   SafePtr<RasterElement> mpBlueRasterElement;

   double mGrayLower;
   double mGrayUpper;
   double mRedLower;
   double mRedUpper;
   double mGreenLower;
   double mGreenUpper;
   double mBlueLower;
   double mBlueUpper;

   ColorMap mColorMap;
   unsigned int mAlpha;
   bool mEnableFastContrastStretch;

   std::vector<std::string> mEnabledFilters;
   Animation* mpAnimation;
};

#endif
