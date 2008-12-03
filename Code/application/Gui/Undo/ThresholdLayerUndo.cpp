/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ThresholdLayer.h"
#include "ThresholdLayerUndo.h"

////////////////////////
// SetThresholdValues //
////////////////////////

SetThresholdValues::SetThresholdValues(ThresholdLayer* pLayer, double dOldLower, double dNewLower, double dOldUpper,
                                       double dNewUpper) :
   UndoAction(pLayer),
   mOldLowerThreshold(dOldLower),
   mNewLowerThreshold(dNewLower),
   mOldUpperThreshold(dOldUpper),
   mNewUpperThreshold(dNewUpper)
{
   setText("Set Threshold Values");
}

void SetThresholdValues::executeUndo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      if (pLayer->getFirstThreshold() != mOldLowerThreshold)
      {
         pLayer->setFirstThreshold(mOldLowerThreshold);
      }

      if (pLayer->getSecondThreshold() != mOldUpperThreshold)
      {
         pLayer->setSecondThreshold(mOldUpperThreshold);
      }
   }
}

void SetThresholdValues::executeRedo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      if (pLayer->getFirstThreshold() != mNewLowerThreshold)
      {
         pLayer->setFirstThreshold(mNewLowerThreshold);
      }

      if (pLayer->getSecondThreshold() != mNewUpperThreshold)
      {
         pLayer->setSecondThreshold(mNewUpperThreshold);
      }
   }
}

//////////////////////////
// SetThresholdPassArea //
//////////////////////////

SetThresholdPassArea::SetThresholdPassArea(ThresholdLayer* pLayer, PassArea oldPassArea, PassArea newPassArea) :
   UndoAction(pLayer),
   mOldPassArea(oldPassArea),
   mNewPassArea(newPassArea)
{
   setText("Set Threshold Pass Area");
}

void SetThresholdPassArea::executeUndo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setPassArea(mOldPassArea);
   }
}

void SetThresholdPassArea::executeRedo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setPassArea(mNewPassArea);
   }
}

///////////////////////
// SetThresholdUnits //
///////////////////////

SetThresholdUnits::SetThresholdUnits(ThresholdLayer* pLayer, RegionUnits oldUnits, RegionUnits newUnits) :
   UndoAction(pLayer),
   mOldUnits(oldUnits),
   mNewUnits(newUnits)
{
   setText("Set Threshold Units");
}

void SetThresholdUnits::executeUndo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setRegionUnits(mOldUnits);
   }
}

void SetThresholdUnits::executeRedo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setRegionUnits(mNewUnits);
   }
}

///////////////////////
// SetThresholdColor //
///////////////////////

SetThresholdColor::SetThresholdColor(ThresholdLayer* pLayer, const ColorType& oldColor, const ColorType& newColor) :
   UndoAction(pLayer),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set Threshold Color");
}

void SetThresholdColor::executeUndo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mOldColor);
   }
}

void SetThresholdColor::executeRedo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mNewColor);
   }
}

////////////////////////
// SetThresholdSymbol //
////////////////////////

SetThresholdSymbol::SetThresholdSymbol(ThresholdLayer* pLayer, SymbolType oldSymbol, SymbolType newSymbol) :
   UndoAction(pLayer),
   mOldSymbol(oldSymbol),
   mNewSymbol(newSymbol)
{
   setText("Set Threshold Symbol");
}

void SetThresholdSymbol::executeUndo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mOldSymbol);
   }
}

void SetThresholdSymbol::executeRedo()
{
   ThresholdLayer* pLayer = dynamic_cast<ThresholdLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mNewSymbol);
   }
}

///////////////////////////
// ThresholdLayerMemento //
///////////////////////////

ThresholdLayerMemento::ThresholdLayerMemento(ThresholdLayer* pLayer) :
   mLowerThreshold(0.0),
   mUpperThreshold(0.0),
   mColor(ColorType()),
   mSymbol(SOLID)
{
   if (pLayer != NULL)
   {
      mLowerThreshold = pLayer->getFirstThreshold();
      mUpperThreshold = pLayer->getSecondThreshold();
      mPassArea = pLayer->getPassArea();
      mUnits = pLayer->getRegionUnits();
      mColor = pLayer->getColor();
      mSymbol = pLayer->getSymbol();
   }
}

void ThresholdLayerMemento::toLayer(Layer* pLayer) const
{
   ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(pLayer);
   if (pThresholdLayer != NULL)
   {
      pThresholdLayer->setFirstThreshold(mLowerThreshold);
      pThresholdLayer->setSecondThreshold(mUpperThreshold);
      pThresholdLayer->setPassArea(mPassArea);
      pThresholdLayer->setRegionUnits(mUnits);
      pThresholdLayer->setColor(mColor);
      pThresholdLayer->setSymbol(mSymbol);
   }
}
