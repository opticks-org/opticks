/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PerspectiveView.h"
#include "ViewImp.h"
#include "ViewUndo.h"

using namespace std;

///////////////////////////////
// SetViewClassificationFont //
///////////////////////////////

SetViewClassificationFont::SetViewClassificationFont(View* pView, const QFont& oldFont, const QFont& newFont) :
   UndoAction(pView),
   mOldFont(oldFont),
   mNewFont(newFont)
{
   setText("Set Classification Font");
}

void SetViewClassificationFont::executeUndo()
{
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getSessionItem());
   if (pViewImp != NULL)
   {
      pViewImp->setClassificationFont(mOldFont);
      pViewImp->refresh();
   }
}

void SetViewClassificationFont::executeRedo()
{
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getSessionItem());
   if (pViewImp != NULL)
   {
      pViewImp->setClassificationFont(mNewFont);
      pViewImp->refresh();
   }
}

////////////////////////////////
// SetViewClassificationColor //
////////////////////////////////

SetViewClassificationColor::SetViewClassificationColor(View* pView, const QColor& oldColor, const QColor& newColor) :
   UndoAction(pView),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set Classification Color");
}

void SetViewClassificationColor::executeUndo()
{
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getSessionItem());
   if (pViewImp != NULL)
   {
      pViewImp->setClassificationColor(mOldColor);
      pViewImp->refresh();
   }
}

void SetViewClassificationColor::executeRedo()
{
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getSessionItem());
   if (pViewImp != NULL)
   {
      pViewImp->setClassificationColor(mNewColor);
      pViewImp->refresh();
   }
}

////////////////////////////
// SetViewBackgroundColor //
////////////////////////////

SetViewBackgroundColor::SetViewBackgroundColor(View* pView, const QColor& oldColor, const QColor& newColor) :
   UndoAction(pView),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set Background Color");
}

void SetViewBackgroundColor::executeUndo()
{
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getSessionItem());
   if (pViewImp != NULL)
   {
      pViewImp->setBackgroundColor(mOldColor);
      pViewImp->refresh();
   }
}

void SetViewBackgroundColor::executeRedo()
{
   ViewImp* pViewImp = dynamic_cast<ViewImp*>(getSessionItem());
   if (pViewImp != NULL)
   {
      pViewImp->setBackgroundColor(mNewColor);
      pViewImp->refresh();
   }
}

///////////////////////
// SetViewDataOrigin //
///////////////////////

SetViewDataOrigin::SetViewDataOrigin(View* pView, DataOrigin oldOrigin, DataOrigin newOrigin) :
   UndoAction(pView),
   mOldOrigin(oldOrigin),
   mNewOrigin(newOrigin)
{
   setText("Set Data Origin");
}

void SetViewDataOrigin::executeUndo()
{
   View* pView = dynamic_cast<View*>(getSessionItem());
   if (pView != NULL)
   {
      pView->setDataOrigin(mOldOrigin);
      pView->refresh();
   }
}

void SetViewDataOrigin::executeRedo()
{
   View* pView = dynamic_cast<View*>(getSessionItem());
   if (pView != NULL)
   {
      pView->setDataOrigin(mNewOrigin);
      pView->refresh();
   }
}

/////////////////
// ZoomViewBox //
/////////////////

ZoomViewBox::ZoomViewBox(View* pView, const LocationType& oldWorldLowerLeft, const LocationType& newWorldLowerLeft,
                         const LocationType& oldWorldUpperRight, const LocationType& newWorldUpperRight) :
   UndoAction(pView),
   mOldLowerLeft(oldWorldLowerLeft),
   mNewLowerLeft(newWorldLowerLeft),
   mOldUpperRight(oldWorldUpperRight),
   mNewUpperRight(newWorldUpperRight)
{
   setText("Set Zoom Rectangle");
}

void ZoomViewBox::executeUndo()
{
   View* pView = dynamic_cast<View*>(getSessionItem());
   if (pView != NULL)
   {
      pView->zoomToBox(mOldLowerLeft, mOldUpperRight);
      pView->refresh();
   }
}

void ZoomViewBox::executeRedo()
{
   View* pView = dynamic_cast<View*>(getSessionItem());
   if (pView != NULL)
   {
      pView->zoomToBox(mNewLowerLeft, mNewUpperRight);
      pView->refresh();
   }
}

/////////////////////
// ZoomViewPercent //
/////////////////////

ZoomViewPercent::ZoomViewPercent(PerspectiveView* pView, double oldPercent, double newPercent) :
   UndoAction(pView),
   mOldPercent(oldPercent),
   mNewPercent(newPercent)
{
   setText("Set Zoom Percent");
}

void ZoomViewPercent::executeUndo()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->zoomTo(mOldPercent);
      pView->refresh();
   }
}

void ZoomViewPercent::executeRedo()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->zoomTo(mNewPercent);
      pView->refresh();
   }
}

/////////////
// PanView //
/////////////

PanView::PanView(View* pView, const LocationType& oldCenter, const LocationType& newCenter) :
   UndoAction(pView),
   mOldCenter(oldCenter),
   mNewCenter(newCenter)
{
   setText("Pan View");
}

void PanView::executeUndo()
{
   View* pView = dynamic_cast<View*>(getSessionItem());
   if (pView != NULL)
   {
      pView->panTo(mOldCenter);
      pView->refresh();
   }
}

void PanView::executeRedo()
{
   View* pView = dynamic_cast<View*>(getSessionItem());
   if (pView != NULL)
   {
      pView->panTo(mNewCenter);
      pView->refresh();
   }
}

////////////////
// RotateView //
////////////////

RotateView::RotateView(PerspectiveView* pView, double oldAngle, double newAngle) :
   UndoAction(pView),
   mOldAngle(oldAngle),
   mNewAngle(newAngle)
{
   setText("Rotate View");
}

void RotateView::executeUndo()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->rotateTo(mOldAngle);
      pView->refresh();
   }
}

void RotateView::executeRedo()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->rotateTo(mNewAngle);
      pView->refresh();
   }
}

//////////////
// FlipView //
//////////////

FlipView::FlipView(PerspectiveView* pView, double oldPitch, double newPitch) :
   UndoAction(pView),
   mOldPitch(oldPitch),
   mNewPitch(newPitch)
{
   setText("Flip View");
}

void FlipView::executeUndo()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->flipTo(mOldPitch);
      pView->refresh();
   }
}

void FlipView::executeRedo()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getSessionItem());
   if (pView != NULL)
   {
      pView->flipTo(mNewPitch);
      pView->refresh();
   }
}
