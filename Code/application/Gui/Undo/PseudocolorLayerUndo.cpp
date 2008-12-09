/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PseudocolorClass.h"
#include "PseudocolorLayer.h"
#include "PseudocolorLayerImp.h"
#include "PseudocolorLayerUndo.h"

using namespace std;

//////////////////////////
// SetPseudocolorSymbol //
//////////////////////////

SetPseudocolorSymbol::SetPseudocolorSymbol(PseudocolorLayer* pLayer, SymbolType oldSymbol, SymbolType newSymbol) :
   UndoAction(pLayer),
   mOldSymbol(oldSymbol),
   mNewSymbol(newSymbol)
{
   setText("Set Pseudocolor Symbol");
}

void SetPseudocolorSymbol::executeUndo()
{
   PseudocolorLayer* pLayer = dynamic_cast<PseudocolorLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mOldSymbol);
   }
}

void SetPseudocolorSymbol::executeRedo()
{
   PseudocolorLayer* pLayer = dynamic_cast<PseudocolorLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setSymbol(mNewSymbol);
   }
}

///////////////////////////////////
// CreateDestroyPseudocolorClass //
///////////////////////////////////

CreateDestroyPseudocolorClass::CreateDestroyPseudocolorClass(Layer* pLayer, PseudocolorClass* pClass) :
   UndoAction(pLayer),
   mId(-1),
   mName(QString()),
   mValue(-1),
   mColor(QColor()),
   mDisplayed(false)
{
   if (pClass != NULL)
   {
      PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
      if (pLayerImp != NULL)
      {
         mId = pLayerImp->getClassID(pClass);
      }

      mName = pClass->getName();
      mValue = pClass->getValue();
      mColor = pClass->getColor();
      mDisplayed = pClass->isDisplayed();
   }
}

CreateDestroyPseudocolorClass::~CreateDestroyPseudocolorClass()
{
}

void CreateDestroyPseudocolorClass::createClass()
{
   PseudocolorLayer* pLayer = dynamic_cast<PseudocolorLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      PseudocolorClass* pClass = new PseudocolorClass(pLayer);
      if (pClass != NULL)
      {
         pClass->setClassName(mName);
         pClass->setValue(mValue);
         pClass->setColor(mColor);
         pClass->setDisplayed(mDisplayed);

         PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
         if (pLayerImp != NULL)
         {
            pLayerImp->insertClass(pClass, mId);
         }
      }
   }
}

void CreateDestroyPseudocolorClass::destroyClass()
{
   PseudocolorLayerImp* pLayer = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayer != NULL)
   {
      PseudocolorClass* pClass = pLayer->getClassById(mId);
      if (pClass != NULL)
      {
         pLayer->removeClass(pClass);
      }
   }
}

/////////////////////////
// AddPseudocolorClass //
/////////////////////////

AddPseudocolorClass::AddPseudocolorClass(PseudocolorLayer* pLayer, PseudocolorClass* pClass) :
   CreateDestroyPseudocolorClass(pLayer, pClass)
{
   setText("Add Pseudocolor Class");
}

void AddPseudocolorClass::executeUndo()
{
   destroyClass();
}

void AddPseudocolorClass::executeRedo()
{
   createClass();
}

////////////////////////////
// DeletePseudocolorClass //
////////////////////////////

DeletePseudocolorClass::DeletePseudocolorClass(PseudocolorLayer* pLayer, PseudocolorClass* pClass) :
   CreateDestroyPseudocolorClass(pLayer, pClass)
{
   setText("Delete Pseudocolor Class");
}

void DeletePseudocolorClass::executeUndo()
{
   createClass();
}

void DeletePseudocolorClass::executeRedo()
{
   destroyClass();
}

/////////////////////////////
// SetPseudocolorClassName //
/////////////////////////////

SetPseudocolorClassName::SetPseudocolorClassName(PseudocolorLayer* pLayer, PseudocolorClass* pClass,
                                                 const QString& oldName, const QString& newName) :
   UndoAction(pLayer),
   mClassId(-1),
   mOldName(oldName),
   mNewName(newName)
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
   if ((pLayerImp != NULL) && (pClass != NULL))
   {
      mClassId = pLayerImp->getClassID(pClass);
   }

   setText("Set Pseudocolor Class Name");
}

void SetPseudocolorClassName::executeUndo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setClassName(mOldName);
      }
   }
}

void SetPseudocolorClassName::executeRedo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setClassName(mNewName);
      }
   }
}

//////////////////////////////
// SetPseudocolorClassValue //
//////////////////////////////

SetPseudocolorClassValue::SetPseudocolorClassValue(PseudocolorLayer* pLayer, PseudocolorClass* pClass,
                                                   int oldValue, int newValue) :
   UndoAction(pLayer),
   mClassId(-1),
   mOldValue(oldValue),
   mNewValue(newValue)
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
   if ((pLayerImp != NULL) && (pClass != NULL))
   {
      mClassId = pLayerImp->getClassID(pClass);
   }

   setText("Set Pseudocolor Class Value");
}

void SetPseudocolorClassValue::executeUndo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setValue(mOldValue);
      }
   }
}

void SetPseudocolorClassValue::executeRedo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setValue(mNewValue);
      }
   }
}

//////////////////////////////
// SetPseudocolorClassColor //
//////////////////////////////

SetPseudocolorClassColor::SetPseudocolorClassColor(PseudocolorLayer* pLayer, PseudocolorClass* pClass,
                                                   const QColor& oldColor, const QColor& newColor) :
   UndoAction(pLayer),
   mClassId(-1),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
   if ((pLayerImp != NULL) && (pClass != NULL))
   {
      mClassId = pLayerImp->getClassID(pClass);
   }

   setText("Set Pseudocolor Class Color");
}

void SetPseudocolorClassColor::executeUndo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setColor(mOldColor);
      }
   }
}

void SetPseudocolorClassColor::executeRedo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setColor(mNewColor);
      }
   }
}

//////////////////////////////////
// SetPseudocolorClassDisplayed //
//////////////////////////////////

SetPseudocolorClassDisplayed::SetPseudocolorClassDisplayed(PseudocolorLayer* pLayer, PseudocolorClass* pClass,
                                                           bool oldDisplayed, bool newDisplayed) :
   UndoAction(pLayer),
   mClassId(-1),
   mOldDisplayed(oldDisplayed),
   mNewDisplayed(newDisplayed)
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
   if ((pLayerImp != NULL) && (pClass != NULL))
   {
      mClassId = pLayerImp->getClassID(pClass);
   }

   setText("Display Pseudocolor Class");
}

void SetPseudocolorClassDisplayed::executeUndo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setDisplayed(mOldDisplayed);
      }
   }
}

void SetPseudocolorClassDisplayed::executeRedo()
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(getSessionItem());
   if (pLayerImp != NULL)
   {
      PseudocolorClass* pClass = pLayerImp->getClassById(mClassId);
      if (pClass != NULL)
      {
         pClass->setDisplayed(mNewDisplayed);
      }
   }
}

/////////////////////////////
// PseudocolorLayerMemento //
/////////////////////////////

PseudocolorLayerMemento::PseudocolorLayerMemento(PseudocolorLayer* pLayer)
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
   if (pLayerImp != NULL)
   {
      vector<PseudocolorClass*> classes = pLayerImp->getAllClasses();
      for (vector<PseudocolorClass*>::iterator iter = classes.begin(); iter != classes.end(); ++iter)
      {
         PseudocolorClass* pClass = *iter;
         if (pClass != NULL)
         {
            mClasses.push_back(new PseudocolorClass(*pClass));
         }
      }
   }
}

PseudocolorLayerMemento::~PseudocolorLayerMemento()
{
   for (vector<PseudocolorClass*>::iterator iter = mClasses.begin(); iter != mClasses.end(); ++iter)
   {
      PseudocolorClass* pClass = *iter;
      if (pClass != NULL)
      {
         delete pClass;
      }
   }
}

void PseudocolorLayerMemento::toLayer(Layer* pLayer) const
{
   PseudocolorLayerImp* pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
   if (pLayerImp != NULL)
   {
      pLayerImp->clear();

      for (vector<PseudocolorClass*>::const_iterator iter = mClasses.begin(); iter != mClasses.end(); ++iter)
      {
         PseudocolorClass* pClass = *iter;
         if (pClass != NULL)
         {
            QString name = pClass->getName();
            int value = pClass->getValue();
            QColor color = pClass->getColor();
            bool displayed = pClass->isDisplayed();

            pLayerImp->addClass(name, value, color, displayed);
         }
      }
   }
}
