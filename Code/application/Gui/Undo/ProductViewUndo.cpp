/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ProductView.h"
#include "ProductViewUndo.h"

//////////////////
// SetPaperSize //
//////////////////

SetPaperSize::SetPaperSize(ProductView* pView, double oldWidth, double oldHeight, double newWidth, double newHeight) :
   UndoAction(pView),
   mOldWidth(oldWidth),
   mOldHeight(oldHeight),
   mNewWidth(newWidth),
   mNewHeight(newHeight)
{
   setText("Set Paper Size");
}

void SetPaperSize::executeUndo()
{
   ProductView* pView = dynamic_cast<ProductView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->setPaperSize(mOldWidth, mOldHeight);
      pView->refresh();
   }
}

void SetPaperSize::executeRedo()
{
   ProductView* pView = dynamic_cast<ProductView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->setPaperSize(mNewWidth, mNewHeight);
      pView->refresh();
   }
}

///////////////////
// SetPaperColor //
///////////////////

SetPaperColor::SetPaperColor(ProductView* pView, const ColorType& oldColor, const ColorType& newColor) :
   UndoAction(pView),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set Paper Color");
}

void SetPaperColor::executeUndo()
{
   ProductView* pView = dynamic_cast<ProductView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->setPaperColor(mOldColor);
      pView->refresh();
   }
}

void SetPaperColor::executeRedo()
{
   ProductView* pView = dynamic_cast<ProductView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->setPaperColor(mNewColor);
      pView->refresh();
   }
}
