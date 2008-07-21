/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include "GcpToolBar.h"

#include "ColorMenu.h"
#include "AppAssert.h"
#include "GcpLayer.h"
#include "GcpLayerImp.h"
#include "GcpSymbolGrid.h"
#include "Icons.h"
#include "MenuBarImp.h"

using namespace std;

GcpToolBar::GcpToolBar(const string& id, QWidget* parent) :
   ToolBarAdapter(id, "Geo", parent),
   mpSymbol(NULL),
   mpColorAction(NULL),
   mpColorMenu(NULL),
   mpGcpLayer(NULL)
{
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   // Symbol button
   mpSymbol = new GcpSymbolButton(this);
   mpSymbol->setSyncIcon(false);
   mpSymbol->setIcon(pIcons->mShape);
   mpSymbol->setStatusTip("Changes the pixel marker symbol for the current GCP layer");
   mpSymbol->setToolTip("GCP Marker Symbol");
   addWidget(mpSymbol);
   connect(mpSymbol, SIGNAL(valueChanged(GcpSymbol)), this, SLOT(setMarkerSymbol(GcpSymbol)));

   // Symbol color button
   mpColorMenu = new ColorMenu(this);
   if (mpColorMenu != NULL)
   {
      mpColorAction = mpColorMenu->menuAction();
      if (mpColorAction != NULL)
      {
         mpColorAction->setIcon(pIcons->mGCPColor);
         mpColorAction->setStatusTip("Changes the pixel marker color for the current GCP layer");
         mpColorAction->setToolTip("GCP Marker Color");
         connect(mpColorAction, SIGNAL(triggered()), mpColorMenu, SLOT(setCustomColor()));

         addAction(mpColorAction);
      }

      connect(mpColorMenu, SIGNAL(aboutToShow()), this, SLOT(initializeColorMenu()));
      connect(mpColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setMarkerColor(const QColor&)));
   }
}

GcpToolBar::~GcpToolBar()
{
}

Layer* GcpToolBar::getGcpLayer() const
{
   return mpGcpLayer;
}

void GcpToolBar::setEnabled(bool bEnable)
{
   MenuBarImp* pMenuBar = static_cast<MenuBarImp*>(getMenuBar());
   if (pMenuBar != NULL)
   {
      pMenuBar->setEnabled(true);
   }

   mpSymbol->setEnabled(bEnable);
   mpColorAction->setEnabled(bEnable);
}

bool GcpToolBar::setGcpLayer(Layer* pLayer)
{
   if (pLayer == mpGcpLayer)
   {
      return false;
   }

   if (pLayer != NULL)
   {
      if (pLayer->getLayerType() != GCP_LAYER)
      {
         return false;
      }
   }

   if (mpGcpLayer != NULL)
   {
      mpGcpLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &GcpToolBar::gcpLayerDeleted));
   }

   mpGcpLayer = static_cast<GcpLayer*>(pLayer);

   if (mpGcpLayer != NULL)
   {
      mpGcpLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &GcpToolBar::gcpLayerDeleted));

      disconnect(mpSymbol, SIGNAL(valueChanged(GcpSymbol)), this, SLOT(setMarkerSymbol(GcpSymbol)));
      mpSymbol->setCurrentValue(mpGcpLayer->getSymbol());
      connect(mpSymbol, SIGNAL(valueChanged(GcpSymbol)), this, SLOT(setMarkerSymbol(GcpSymbol)));
   }

   return true;
}

void GcpToolBar::gcpLayerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<GcpLayer*>(&subject) == mpGcpLayer)
   {
      setGcpLayer(NULL);
   }
}

void GcpToolBar::initializeColorMenu()
{
   GcpLayerImp* pLayer = dynamic_cast<GcpLayerImp*>(mpGcpLayer);
   if (pLayer != NULL)
   {
      QColor clrMarker = pLayer->getColor();
      mpColorMenu->setSelectedColor(clrMarker);
   }
}

void GcpToolBar::setMarkerSymbol(GcpSymbol markerSymbol)
{
   if (mpGcpLayer != NULL)
   {
      mpGcpLayer->setSymbol(markerSymbol);
   }
}

void GcpToolBar::setMarkerColor(const QColor& markerColor)
{
   GcpLayerImp* pLayer = dynamic_cast<GcpLayerImp*>(mpGcpLayer);
   if (pLayer != NULL)
   {
      pLayer->setColor(markerColor);
   }
}
