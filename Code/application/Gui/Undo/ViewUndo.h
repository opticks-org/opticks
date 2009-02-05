/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWUNDO_H
#define VIEWUNDO_H

#include <QtGui/QColor>
#include <QtGui/QFont>

#include "LocationType.h"
#include "TypesFile.h"
#include "UndoAction.h"

class PerspectiveView;
class View;

class SetViewClassificationFont : public UndoAction
{
public:
   SetViewClassificationFont(View* pView, const QFont& oldFont, const QFont& newFont);

   void executeUndo();
   void executeRedo();

private:
   QFont mOldFont;
   QFont mNewFont;
};


class SetViewClassificationColor : public UndoAction
{
public:
   SetViewClassificationColor(View* pView, const QColor& oldColor, const QColor& newColor);

   void executeUndo();
   void executeRedo();

private:
   QColor mOldColor;
   QColor mNewColor;
};


class SetViewBackgroundColor : public UndoAction
{
public:
   SetViewBackgroundColor(View* pView, const QColor& oldColor, const QColor& newColor);

   void executeUndo();
   void executeRedo();

private:
   QColor mOldColor;
   QColor mNewColor;
};


class SetViewDataOrigin : public UndoAction
{
public:
   SetViewDataOrigin(View* pView, DataOrigin oldOrigin, DataOrigin newOrigin);

   void executeUndo();
   void executeRedo();

private:
   DataOrigin mOldOrigin;
   DataOrigin mNewOrigin;
};


class ZoomViewBox : public UndoAction
{
public:
   ZoomViewBox(View* pView, const LocationType& oldWorldLowerLeft, const LocationType& newWorldLowerLeft,
      const LocationType& oldWorldUpperRight, const LocationType& newWorldUpperRight);

   void executeUndo();
   void executeRedo();

private:
   LocationType mOldLowerLeft;
   LocationType mNewLowerLeft;
   LocationType mOldUpperRight;
   LocationType mNewUpperRight;
};


class ZoomViewPercent : public UndoAction
{
public:
   ZoomViewPercent(PerspectiveView* pView, double oldPercent, double newPercent);

   void executeUndo();
   void executeRedo();

private:
   double mOldPercent;
   double mNewPercent;
};


class PanView : public UndoAction
{
public:
   PanView(View* pView, const LocationType& oldCenter, const LocationType& newCenter);

   void executeUndo();
   void executeRedo();

private:
   LocationType mOldCenter;
   LocationType mNewCenter;
};


class RotateView : public UndoAction
{
public:
   RotateView(PerspectiveView* pView, double oldAngle, double newAngle);

   void executeUndo();
   void executeRedo();

private:
   double mOldAngle;
   double mNewAngle;
};


class FlipView : public UndoAction
{
public:
   FlipView(PerspectiveView* pView, double oldPitch, double newPitch);

   void executeUndo();
   void executeRedo();

private:
   double mOldPitch;
   double mNewPitch;
};

#endif
