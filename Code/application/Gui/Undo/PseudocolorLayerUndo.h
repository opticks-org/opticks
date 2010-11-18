/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PSEUDOCOLORLAYERUNDO_H
#define PSEUDOCOLORLAYERUNDO_H

#include <QtCore/QString>
#include <QtGui/QColor>

#include "LayerUndo.h"
#include "TypesFile.h"
#include "UndoAction.h"

#include <vector>

class PseudocolorLayer;
class PseudocolorClass;

class SetPseudocolorSymbol : public UndoAction
{
public:
   SetPseudocolorSymbol(PseudocolorLayer* pLayer, SymbolType oldSymbol, SymbolType newSymbol);

   void executeUndo();
   void executeRedo();

private:
   SymbolType mOldSymbol;
   SymbolType mNewSymbol;
};


class CreateDestroyPseudocolorClass : public UndoAction
{
public:
   CreateDestroyPseudocolorClass(Layer* pLayer, PseudocolorClass* pClass);
   ~CreateDestroyPseudocolorClass();

protected:
   void createClass();
   void destroyClass();

private:
   int mId;
   QString mName;
   int mValue;
   QColor mColor;
   bool mDisplayed;
};


class AddPseudocolorClass : public CreateDestroyPseudocolorClass
{
public:
   AddPseudocolorClass(PseudocolorLayer* pLayer, PseudocolorClass* pClass);

   void executeUndo();
   void executeRedo();
};


class DeletePseudocolorClass : public CreateDestroyPseudocolorClass
{
public:
   DeletePseudocolorClass(PseudocolorLayer* pLayer, PseudocolorClass* pClass);

   void executeUndo();
   void executeRedo();
};


class SetPseudocolorClassName : public UndoAction
{
public:
   SetPseudocolorClassName(PseudocolorLayer* pLayer, PseudocolorClass* pClass, const QString& oldName,
      const QString& newName);

   void executeUndo();
   void executeRedo();

private:
   int mClassId;
   QString mOldName;
   QString mNewName;
};


class SetPseudocolorClassValue : public UndoAction
{
public:
   SetPseudocolorClassValue(PseudocolorLayer* pLayer, PseudocolorClass* pClass, int oldValue, int newValue);

   void executeUndo();
   void executeRedo();

private:
   int mClassId;
   int mOldValue;
   int mNewValue;
};


class SetPseudocolorClassColor : public UndoAction
{
public:
   SetPseudocolorClassColor(PseudocolorLayer* pLayer, PseudocolorClass* pClass, const QColor& oldColor,
      const QColor& newColor);

   void executeUndo();
   void executeRedo();

private:
   int mClassId;
   QColor mOldColor;
   QColor mNewColor;
};


class SetPseudocolorClassDisplayed : public UndoAction
{
public:
   SetPseudocolorClassDisplayed(PseudocolorLayer* pLayer, PseudocolorClass* pClass, bool oldDisplayed,
      bool newDisplayed);

   void executeUndo();
   void executeRedo();

private:
   int mClassId;
   bool mOldDisplayed;
   bool mNewDisplayed;
};


class PseudocolorLayerMemento : public LayerMemento
{
public:
   PseudocolorLayerMemento(PseudocolorLayer* pLayer);
   ~PseudocolorLayerMemento();

   void toLayer(Layer* pLayer) const;

private:
   std::vector<PseudocolorClass*> mClasses;
};

#endif
