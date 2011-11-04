/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOILAYERUNDO_H
#define AOILAYERUNDO_H

#include "UndoAction.h"
#include "ColorType.h"
#include "GraphicLayerUndo.h"
#include "TypesFile.h"

class AoiLayer;

class SetAoiColor : public UndoAction
{
public:
   SetAoiColor(AoiLayer* pLayer, const ColorType& oldColor, const ColorType& newColor);

   void executeUndo();
   void executeRedo();

private:
   SetAoiColor(const SetAoiColor& rhs);
   SetAoiColor& operator=(const SetAoiColor& rhs);
   ColorType mOldColor;
   ColorType mNewColor;
};


class SetAoiSymbol : public UndoAction
{
public:
   SetAoiSymbol(AoiLayer* pLayer, SymbolType oldSymbol, SymbolType newSymbol);

   void executeUndo();
   void executeRedo();

private:
   SetAoiSymbol(const SetAoiSymbol& rhs);
   SetAoiSymbol& operator=(const SetAoiSymbol& rhs);
   SymbolType mOldSymbol;
   SymbolType mNewSymbol;
};


class AoiLayerMemento : public GraphicLayerMemento
{
public:
   AoiLayerMemento(AoiLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   AoiLayerMemento(const AoiLayerMemento& rhs);
   AoiLayerMemento& operator=(const AoiLayerMemento& rhs);
   ColorType mColor;
   SymbolType mSymbol;
};

#endif
