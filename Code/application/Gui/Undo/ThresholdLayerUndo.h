/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef THRESHOLDLAYERUNDO_H
#define THRESHOLDLAYERUNDO_H

#include "ColorType.h"
#include "DimensionDescriptor.h"
#include "LayerUndo.h"
#include "UndoAction.h"
#include "TypesFile.h"

class ThresholdLayer;

class SetThresholdValues : public UndoAction
{
public:
   SetThresholdValues(ThresholdLayer* pLayer, double dOldLower, double dNewLower, double dOldUpper, double dNewUpper);

   void executeUndo();
   void executeRedo();

private:
   SetThresholdValues(const SetThresholdValues& rhs);
   SetThresholdValues& operator=(const SetThresholdValues& rhs);
   double mOldLowerThreshold;
   double mNewLowerThreshold;
   double mOldUpperThreshold;
   double mNewUpperThreshold;
};


class SetThresholdPassArea : public UndoAction
{
public:
   SetThresholdPassArea(ThresholdLayer* pLayer, PassArea oldPassArea, PassArea newPassArea);

   void executeUndo();
   void executeRedo();

private:
   SetThresholdPassArea(const SetThresholdPassArea& rhs);
   SetThresholdPassArea& operator=(const SetThresholdPassArea& rhs);
   PassArea mOldPassArea;
   PassArea mNewPassArea;
};


class SetThresholdUnits : public UndoAction
{
public:
   SetThresholdUnits(ThresholdLayer* pLayer, RegionUnits oldUnits, RegionUnits newUnits);

   void executeUndo();
   void executeRedo();

private:
   SetThresholdUnits(const SetThresholdUnits& rhs);
   SetThresholdUnits& operator=(const SetThresholdUnits& rhs);
   RegionUnits mOldUnits;
   RegionUnits mNewUnits;
};


class SetThresholdColor : public UndoAction
{
public:
   SetThresholdColor(ThresholdLayer* pLayer, const ColorType& oldColor, const ColorType& newColor);

   void executeUndo();
   void executeRedo();

private:
   SetThresholdColor(const SetThresholdColor& rhs);
   SetThresholdColor& operator=(const SetThresholdColor& rhs);
   ColorType mOldColor;
   ColorType mNewColor;
};

class SetThresholdSymbol : public UndoAction
{
public:
   SetThresholdSymbol(ThresholdLayer* pLayer, SymbolType oldSymbol, SymbolType newSymbol);

   void executeUndo();
   void executeRedo();

private:
   SetThresholdSymbol(const SetThresholdSymbol& rhs);
   SetThresholdSymbol& operator=(const SetThresholdSymbol& rhs);
   SymbolType mOldSymbol;
   SymbolType mNewSymbol;
};


class SetThresholdBand : public UndoAction
{
public:
   SetThresholdBand(ThresholdLayer* pLayer, DimensionDescriptor oldBand, DimensionDescriptor newBand);

   void executeUndo();
   void executeRedo();

private:
   SetThresholdBand(const SetThresholdBand& rhs);
   SetThresholdBand& operator=(const SetThresholdBand& rhs);
   DimensionDescriptor mOldBand;
   DimensionDescriptor mNewBand;
};

class ThresholdLayerMemento : public LayerMemento
{
public:
   ThresholdLayerMemento(ThresholdLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   ThresholdLayerMemento(const ThresholdLayerMemento& rhs);
   ThresholdLayerMemento& operator=(const ThresholdLayerMemento& rhs);
   double mLowerThreshold;
   double mUpperThreshold;
   PassArea mPassArea;
   RegionUnits mUnits;
   ColorType mColor;
   SymbolType mSymbol;
   DimensionDescriptor mDisplayedBand;
};

#endif
