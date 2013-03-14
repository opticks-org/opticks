/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TiePointLayer.h"
#include "TiePointLayerUndo.h"

///////////////////////////
// SetTiePointSymbolSize //
///////////////////////////

SetTiePointSymbolSize::SetTiePointSymbolSize(TiePointLayer* pLayer, int oldSize, int newSize) :
   UndoAction(pLayer),
   mOldSize(oldSize),
   mNewSize(newSize)
{
   setText("Set Tie Point Symbol Size");
}

void SetTiePointSymbolSize::executeUndo()
{
   TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbolSize(mOldSize);
   }
}

void SetTiePointSymbolSize::executeRedo()
{
   TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbolSize(mNewSize);
   }
}

//////////////////////
// SetTiePointColor //
//////////////////////

SetTiePointColor::SetTiePointColor(TiePointLayer* pLayer, const ColorType& oldColor, const ColorType& newColor) :
   UndoAction(pLayer),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set Tie Point Color");
}

void SetTiePointColor::executeUndo()
{
   TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mOldColor);
   }
}

void SetTiePointColor::executeRedo()
{
   TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mNewColor);
   }
}

///////////////////////
// SetTiePointLabels //
///////////////////////

SetTiePointLabels::SetTiePointLabels(TiePointLayer* pLayer, bool oldLabels, bool newLabels) :
   UndoAction(pLayer),
   mOldLabels(oldLabels),
   mNewLabels(newLabels)
{
   setText("Set Tie Point Labels");
}

void SetTiePointLabels::executeUndo()
{
   TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->enableLabels(mOldLabels);
   }
}

void SetTiePointLabels::executeRedo()
{
   TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->enableLabels(mNewLabels);
   }
}

//////////////////////////
// TiePointLayerMemento //
//////////////////////////

TiePointLayerMemento::TiePointLayerMemento(TiePointLayer* pLayer) :
   mSymbolSize(TiePointLayer::getSettingMarkerSize()),
   mColor(TiePointLayer::getSettingMarkerColor()),
   mLabels(TiePointLayer::getSettingLabelEnabled()),
   mIsMission(false)
{
   if (pLayer != NULL)
   {
      mSymbolSize = pLayer->getSymbolSize();
      mColor = pLayer->getColor();
      mLabels = pLayer->areLabelsEnabled();
      mIsMission = pLayer->isMission();
   }
}

void TiePointLayerMemento::toLayer(Layer* pLayer) const
{
   TiePointLayer* pTiePointLayer = dynamic_cast<TiePointLayer*>(pLayer);
   if (pTiePointLayer != NULL)
   {
      pTiePointLayer->setSymbolSize(mSymbolSize);
      pTiePointLayer->setColor(mColor);
      pTiePointLayer->enableLabels(mLabels);
      pTiePointLayer->setIsMission(mIsMission);
   }
}
