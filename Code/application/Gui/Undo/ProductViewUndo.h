/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRODUCTVIEWUNDO_H
#define PRODUCTVIEWUNDO_H

#include "ColorType.h"
#include "UndoAction.h"

class ProductView;

class SetPaperSize : public UndoAction
{
public:
   SetPaperSize(ProductView* pView, double oldWidth, double oldHeight, double newWidth, double newHeight);

   void executeUndo();
   void executeRedo();

private:
   SetPaperSize(const SetPaperSize& rhs);
   SetPaperSize& operator=(const SetPaperSize& rhs);
   double mOldWidth;
   double mOldHeight;
   double mNewWidth;
   double mNewHeight;
};


class SetPaperColor : public UndoAction
{
public:
   SetPaperColor(ProductView* pView, const ColorType& oldColor, const ColorType& newColor);

   void executeUndo();
   void executeRedo();

private:
   SetPaperColor(const SetPaperColor& rhs);
   SetPaperColor& operator=(const SetPaperColor& rhs);
   ColorType mOldColor;
   ColorType mNewColor;
};

#endif
