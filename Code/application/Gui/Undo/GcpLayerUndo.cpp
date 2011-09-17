/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GcpLayer.h"
#include "GcpLayerUndo.h"

/////////////////
// SetGcpColor //
/////////////////

SetGcpColor::SetGcpColor(GcpLayer* pLayer, const ColorType& oldColor, const ColorType& newColor) :
   UndoAction(pLayer),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set GCP Color");
}

void SetGcpColor::executeUndo()
{
   GcpLayer* pLayer = dynamic_cast<GcpLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mOldColor);
   }
}

void SetGcpColor::executeRedo()
{
   GcpLayer* pLayer = dynamic_cast<GcpLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mNewColor);
   }
}

//////////////////
// SetGcpSymbol //
//////////////////

SetGcpSymbol::SetGcpSymbol(GcpLayer* pLayer, GcpSymbol oldSymbol, GcpSymbol newSymbol) :
   UndoAction(pLayer),
   mOldSymbol(oldSymbol),
   mNewSymbol(newSymbol)
{
   setText("Set GCP Symbol");
}

void SetGcpSymbol::executeUndo()
{
   GcpLayer* pLayer = dynamic_cast<GcpLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mOldSymbol);
   }
}

void SetGcpSymbol::executeRedo()
{
   GcpLayer* pLayer = dynamic_cast<GcpLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mNewSymbol);
   }
}

//////////////////////
// SetGcpSymbolSize //
//////////////////////

SetGcpSymbolSize::SetGcpSymbolSize(GcpLayer* pLayer, int oldSize, int newSize) :
   UndoAction(pLayer),
   mOldSize(oldSize),
   mNewSize(newSize)
{
   setText("Set GCP Symbol Size");
}

void SetGcpSymbolSize::executeUndo()
{
   GcpLayer* pLayer = dynamic_cast<GcpLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbolSize(mOldSize);
   }
}

void SetGcpSymbolSize::executeRedo()
{
   GcpLayer* pLayer = dynamic_cast<GcpLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbolSize(mNewSize);
   }
}

/////////////////////
// GcpLayerMemento //
/////////////////////

GcpLayerMemento::GcpLayerMemento(GcpLayer* pLayer) :
   mColor(ColorType()),
   mSymbol(GCP_X),
   mSymbolSize(10)
{
   if (pLayer != NULL)
   {
      mColor = pLayer->getColor();
      mSymbol = pLayer->getSymbol();
      mSymbolSize = pLayer->getSymbolSize();
   }
}

void GcpLayerMemento::toLayer(Layer* pLayer) const
{
   GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(pLayer);
   if (pGcpLayer != NULL)
   {
      pGcpLayer->setColor(mColor);
      pGcpLayer->setSymbol(mSymbol);
      pGcpLayer->setSymbolSize(mSymbolSize);
   }
}
