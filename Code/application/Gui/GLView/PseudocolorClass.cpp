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
#include "PseudocolorLayerUndo.h"
#include "Undo.h"
#include "View.h"

PseudocolorClass::PseudocolorClass(PseudocolorLayer* pLayer)
{
   mpLayer = pLayer;
   miValue = -1;
   mstrName = QString();
   mbDisplayed = true;
}

PseudocolorClass::PseudocolorClass(PseudocolorClass& rhs)
{
   miValue = rhs.miValue;
   mstrName = rhs.mstrName;
   mbDisplayed = rhs.mbDisplayed;
   mColor = rhs.mColor;
}

PseudocolorClass::~PseudocolorClass()
{
}

void PseudocolorClass::setLayer(PseudocolorLayer* pLayer)
{
   mpLayer = pLayer;
}

void PseudocolorClass::setProperties(const QString& strName, int iValue, const QColor& clrClass, bool bDisplayed)
{
   View* pView = NULL;
   if (mpLayer != NULL)
   {
      pView = mpLayer->getView();
   }

   UndoGroup group(pView, "Set Pseudocolor Class Properties");

   setClassName(strName);
   setValue(iValue);
   setColor(clrClass);
   setDisplayed(bDisplayed);
}

int PseudocolorClass::getValue() const
{
   return miValue;
}

QString PseudocolorClass::getName() const
{
   return mstrName;
}

QColor PseudocolorClass::getColor() const
{
   return mColor;
}

bool PseudocolorClass::isDisplayed() const
{
   return mbDisplayed;
}

void PseudocolorClass::setValue(int iValue)
{
   if (miValue != iValue)
   {
      if (mpLayer != NULL)
      {
         View* pView = mpLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetPseudocolorClassValue(mpLayer, this, miValue, iValue));
         }
      }

      miValue = iValue;
      emit valueChanged(miValue);
   }
}

void PseudocolorClass::setClassName(const QString& strName)
{
   if (mstrName != strName)
   {
      if (mpLayer != NULL)
      {
         View* pView = mpLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetPseudocolorClassName(mpLayer, this, mstrName, strName));
         }
      }

      mstrName = strName;
      emit nameChanged(mstrName);
   }
}

void PseudocolorClass::setColor(const QColor& clrClass)
{
   if (mColor != clrClass)
   {
      if (mpLayer != NULL)
      {
         View* pView = mpLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetPseudocolorClassColor(mpLayer, this, mColor, clrClass));
         }
      }

      mColor = clrClass;
      emit colorChanged(mColor);
   }
}

void PseudocolorClass::setDisplayed(bool bDisplay)
{
   if (mbDisplayed != bDisplay)
   {
      if (mpLayer != NULL)
      {
         View* pView = mpLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetPseudocolorClassDisplayed(mpLayer, this, mbDisplayed, bDisplay));
         }
      }

      mbDisplayed = bDisplay;
      emit displayStateChanged(mbDisplayed);
   }
}
