/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorMenu.h"
#include "MenuBarImp.h"
#include "TiePointLayerAdapter.h"
#include "TiePointToolbar.h"

#include <string>
using namespace std;

TiePointToolBar::TiePointToolBar(const string& id, QWidget* parent) :
   ToolBarAdapter(id, "Tie Point", parent),
   mpLabelsEnabledAction(NULL),
   mpColorAction(NULL),
   mpColorMenu(NULL),
   mpTiePointLayer(NULL)
{
   string shortcutContext = windowTitle().toStdString();

   // Color
   mpColorMenu = new ColorMenu(this);
   if (mpColorMenu != NULL)
   {
      mpColorAction = mpColorMenu->menuAction();
      if (mpColorAction != NULL)
      {
         mpColorAction->setIcon(QIcon(":/icons/TiePointColor"));
         mpColorAction->setStatusTip("Changes the pixel marker color for the current tie point layer");
         mpColorAction->setToolTip("Marker Color");
         connect(mpColorAction, SIGNAL(triggered()), mpColorMenu, SLOT(setCustomColor()));

         addAction(mpColorAction);
      }

      connect(mpColorMenu, SIGNAL(aboutToShow()), this, SLOT(initializeColorMenu()));
      connect(mpColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setMarkerColor(const QColor&)));
   }

   // Labels
   mpLabelsEnabledAction = new QAction(QIcon(":/icons/TiePointLabels"), "Labels Enabled", this);
   mpLabelsEnabledAction->setAutoRepeat(false);
   mpLabelsEnabledAction->setStatusTip("Toggles the bold state of the selected text objects");
   mpLabelsEnabledAction->setCheckable(true);
   connect(mpLabelsEnabledAction, SIGNAL(triggered()), this, SLOT(setLabelsOnOff()));
   addButton(mpLabelsEnabledAction, shortcutContext);
}

TiePointToolBar::~TiePointToolBar()
{
}

Layer* TiePointToolBar::getTiePointLayer() const
{
   return mpTiePointLayer;
}

void TiePointToolBar::setEnabled(bool bEnable)
{
   MenuBarImp* pMenuBar = static_cast<MenuBarImp*>(getMenuBar());
   if (pMenuBar != NULL)
   {
      pMenuBar->setEnabled(true);
   }

   mpColorAction->setEnabled(bEnable);
   mpLabelsEnabledAction->setEnabled(bEnable);
}

bool TiePointToolBar::setTiePointLayer(Layer* pLayer)
{
   if (pLayer == mpTiePointLayer)
   {
      return false;
   }

   if (pLayer != NULL)
   {
      if (pLayer->getLayerType() != TIEPOINT_LAYER)
      {
         return false;
      }
   }

   if (mpTiePointLayer != NULL)
   {
      disconnect(dynamic_cast<TiePointLayerImp*>(mpTiePointLayer), SIGNAL(modified()), this,
         SLOT(updateLabelsCheckBox()));
      mpTiePointLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointToolBar::tiePointLayerDeleted));
   }

   mpTiePointLayer = dynamic_cast<TiePointLayer*>(pLayer);

   if (mpTiePointLayer != NULL)
   {
      connect(dynamic_cast<TiePointLayerImp*>(mpTiePointLayer), SIGNAL(modified()), this,
         SLOT(updateLabelsCheckBox()));
      mpTiePointLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointToolBar::tiePointLayerDeleted));

      updateLabelsCheckBox();
   }

   return true;
}

void TiePointToolBar::tiePointLayerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<TiePointLayer*>(&subject) == mpTiePointLayer)
   {
      setTiePointLayer(NULL);
      setEnabled(false);
   }
}

void TiePointToolBar::updateLabelsCheckBox()
{
   if ((mpTiePointLayer != NULL) && (mpLabelsEnabledAction != NULL))
   {
      mpLabelsEnabledAction->setChecked(mpTiePointLayer->areLabelsEnabled());
   }
}

void TiePointToolBar::setLabelsOnOff()
{
   if (mpTiePointLayer != NULL)
   {
      bool enable = mpLabelsEnabledAction->isChecked();
      mpTiePointLayer->enableLabels(enable);
   }
}

void TiePointToolBar::initializeColorMenu()
{
   TiePointLayerImp* pLayer = dynamic_cast<TiePointLayerImp*>(mpTiePointLayer);
   if (pLayer != NULL)
   {
      QColor markerColor = pLayer->getColor();
      mpColorMenu->setSelectedColor(markerColor);
   }
}

void TiePointToolBar::setMarkerColor(const QColor& markerColor)
{
   TiePointLayerImp* pLayer = dynamic_cast<TiePointLayerImp*>(mpTiePointLayer);
   if (pLayer != NULL)
   {
      pLayer->setColor(markerColor);
   }
}
