/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTLAYERUNDO_H
#define TIEPOINTLAYERUNDO_H

#include "LayerUndo.h"
#include "UndoAction.h"

class TiePointLayer;

class SetTiePointSymbolSize : public UndoAction
{
public:
   SetTiePointSymbolSize(TiePointLayer* pLayer, int oldSize, int newSize);

   void executeUndo();
   void executeRedo();

private:
   SetTiePointSymbolSize(const SetTiePointSymbolSize& rhs);
   SetTiePointSymbolSize& operator=(const SetTiePointSymbolSize& rhs);
   int mOldSize;
   int mNewSize;
};


class SetTiePointColor : public UndoAction
{
public:
   SetTiePointColor(TiePointLayer* pLayer, const ColorType& oldColor, const ColorType& newColor);

   void executeUndo();
   void executeRedo();

private:
   SetTiePointColor(const SetTiePointColor& rhs);
   SetTiePointColor& operator=(const SetTiePointColor& rhs);
   ColorType mOldColor;
   ColorType mNewColor;
};


class SetTiePointLabels : public UndoAction
{
public:
   SetTiePointLabels(TiePointLayer* pLayer, bool oldLabels, bool newLabels);

   void executeUndo();
   void executeRedo();

private:
   SetTiePointLabels(const SetTiePointLabels& rhs);
   SetTiePointLabels& operator=(const SetTiePointLabels& rhs);
   bool mOldLabels;
   bool mNewLabels;
};


class TiePointLayerMemento : public LayerMemento
{
public:
   TiePointLayerMemento(TiePointLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   TiePointLayerMemento(const TiePointLayerMemento& rhs);
   TiePointLayerMemento& operator=(const TiePointLayerMemento& rhs);
   int mSymbolSize;
   ColorType mColor;
   bool mLabels;
   bool mIsMission;
};

#endif
