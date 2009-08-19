/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiLayer.h"
#include "AoiLayerUndo.h"

/////////////////
// SetAoiColor //
/////////////////

SetAoiColor::SetAoiColor(AoiLayer* pLayer, const ColorType& oldColor, const ColorType& newColor) :
   UndoAction(pLayer),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set AOI Color");
}

void SetAoiColor::executeUndo()
{
   AoiLayer* pLayer = dynamic_cast<AoiLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mOldColor);
   }
}

void SetAoiColor::executeRedo()
{
   AoiLayer* pLayer = dynamic_cast<AoiLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mNewColor);
   }
}

//////////////////
// SetAoiSymbol //
//////////////////

SetAoiSymbol::SetAoiSymbol(AoiLayer* pLayer, SymbolType oldSymbol, SymbolType newSymbol) :
   UndoAction(pLayer),
   mOldSymbol(oldSymbol),
   mNewSymbol(newSymbol)
{
   setText("Set AOI Symbol");
}

void SetAoiSymbol::executeUndo()
{
   AoiLayer* pLayer = dynamic_cast<AoiLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mOldSymbol);
   }
}

void SetAoiSymbol::executeRedo()
{
   AoiLayer* pLayer = dynamic_cast<AoiLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mNewSymbol);
   }
}

/////////////////////
// AoiLayerMemento //
/////////////////////

AoiLayerMemento::AoiLayerMemento(AoiLayer* pLayer) :
   GraphicLayerMemento(pLayer),
   mColor(ColorType()),
   mSymbol(SOLID)
{
   if (pLayer != NULL)
   {
      mColor = pLayer->getColor();
      mSymbol = pLayer->getSymbol();
   }
}

void AoiLayerMemento::toLayer(Layer* pLayer) const
{
   GraphicLayerMemento::toLayer(pLayer);

   AoiLayer* pAoiLayer = dynamic_cast<AoiLayer*>(pLayer);
   if (pAoiLayer != NULL)
   {
      pAoiLayer->setColor(mColor);
      pAoiLayer->setSymbol(mSymbol);
   }
}
