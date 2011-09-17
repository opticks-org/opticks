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
   SymbolType mOldSymbol;
   SymbolType mNewSymbol;
};


class ThresholdLayerMemento : public LayerMemento
{
public:
   ThresholdLayerMemento(ThresholdLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   double mLowerThreshold;
   double mUpperThreshold;
   PassArea mPassArea;
   RegionUnits mUnits;
   ColorType mColor;
   SymbolType mSymbol;
};

#endif
