/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QMenu>

#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "PolarPlotAdapter.h"
#include "PolarPlotImp.h"
#include "Undo.h"
#include "xmlreader.h"

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

PolarPlotImp::PolarPlotImp(const string& id, const string& viewName, QGLContext* pDrawContext, QWidget* pParent) :
   PlotViewImp(id, viewName, pDrawContext, pParent),
   mGridlines(this, false)
{
   // Gridlines menu
   QMenu* pGridlinesMenu = new QMenu("&Gridlines", this);
   if (pGridlinesMenu != NULL)
   {
      Service<DesktopServices> pDesktop;
      string shortcutContext = "Polar Plot/Gridlines";

      // Major
      QAction* pMajorGridlinesAction = pGridlinesMenu->addAction("Ma&jor");
      pMajorGridlinesAction->setAutoRepeat(false);
      pMajorGridlinesAction->setCheckable(true);
      pMajorGridlinesAction->setStatusTip("Toggles the display of the gridlines at the major tickmarks");
      pMajorGridlinesAction->setToolTip("Major Gridlines");
      pMajorGridlinesAction->setChecked(true);
      connect(pMajorGridlinesAction, SIGNAL(toggled(bool)), this, SLOT(setMajorGridlines(bool)));
      connect(&mGridlines, SIGNAL(visibilityChanged(bool)), pMajorGridlinesAction, SLOT(setChecked(bool)));
      pDesktop->initializeAction(pMajorGridlinesAction, shortcutContext);

      // Minor
      QAction* pMinorGridlinesAction = pGridlinesMenu->addAction("Mi&nor");
      pMinorGridlinesAction->setAutoRepeat(false);
      pMinorGridlinesAction->setCheckable(true);
      pMinorGridlinesAction->setStatusTip("Toggles the display of the gridlines at the minor tickmarks");
      pMinorGridlinesAction->setToolTip("Minor Gridlines");
      connect(pMinorGridlinesAction, SIGNAL(toggled(bool)), this, SLOT(setMinorGridlines(bool)));
      connect(&mGridlines, SIGNAL(minorGridlinesEnabled(bool)), pMinorGridlinesAction, SLOT(setChecked(bool)));
      pDesktop->initializeAction(pMinorGridlinesAction, shortcutContext);

      // Add the gridlines menu action
      ContextMenuAction gridlinesAction(pGridlinesMenu->menuAction(), APP_POLARPLOT_GRIDLINES_MENU_ACTION);
      gridlinesAction.mBuddyType = ContextMenuAction::BEFORE;
      gridlinesAction.mBuddyId = APP_PLOTVIEW_MOUSE_MODE_MENU_ACTION;
      addContextMenuAction(gridlinesAction);
   }

   // Initialization
   lockAspectRatio(true);

   mGridlines.setLineStyle(SOLID_LINE);
   mGridlines.setColor(ColorType(212, 208, 200));
   mGridlines.setVisible(true);

   // Connections
   connect(&mGridlines, SIGNAL(modified()), this, SLOT(refresh()));
}

PolarPlotImp::~PolarPlotImp()
{
}

const string& PolarPlotImp::getObjectType() const
{
   static string type("PolarPlotImp");
   return type;
}

bool PolarPlotImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PolarPlot"))
   {
      return true;
   }

   return PlotViewImp::isKindOf(className);
}

bool PolarPlotImp::isKindOfView(const string& className)
{
   if ((className == "PolarPlotImp") || (className == "PolarPlot"))
   {
      return true;
   }

   return PlotViewImp::isKindOfView(className);
}

void PolarPlotImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("PolarPlot");
   PlotViewImp::getViewTypes(classList);
}

PolarPlotImp& PolarPlotImp::operator= (const PolarPlotImp& polarPlot)
{
   if (this != &polarPlot)
   {
      PlotViewImp::operator= (polarPlot);

      mGridlines = polarPlot.mGridlines;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

View* PolarPlotImp::copy(QGLContext* pDrawContext, QWidget* pParent) const
{
   string viewName = getName();

   PolarPlotAdapter* pView = new PolarPlotAdapter(SessionItemImp::generateUniqueId(), viewName,
      pDrawContext, pParent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *(static_cast<PolarPlotImp*>(pView)) = *this;
   }

   return pView;
}

bool PolarPlotImp::copy(View *pView) const
{
   PolarPlotImp *pViewImp = dynamic_cast<PolarPlotImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

PlotType PolarPlotImp::getPlotType() const
{
   return POLAR_PLOT;
}

PolarGridlines* PolarPlotImp::getGridlines()
{
   return &mGridlines;
}

const PolarGridlines* PolarPlotImp::getGridlines() const
{
   return &mGridlines;
}

void PolarPlotImp::translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const
{
   worldX = dataX * cos(dataY);
   worldY = dataX * sin(dataY);
}

void PolarPlotImp::translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const
{
   dataX = sqrt((worldX * worldX) + (worldY * worldY));
   dataY = atan2(worldY, worldX);
}

void PolarPlotImp::drawGridlines()
{
   mGridlines.draw();
}

void PolarPlotImp::setMajorGridlines(bool bShow)
{
   if (bShow == false)
   {
      mGridlines.enableMinorGridlines(false);
   }

   mGridlines.setVisible(bShow);
   refresh();
}

void PolarPlotImp::setMinorGridlines(bool bShow)
{
   if (bShow == true)
   {
      mGridlines.setVisible(true);
   }

   mGridlines.enableMinorGridlines(bShow);
   refresh();
}

bool PolarPlotImp::toXml(XMLWriter* pXml) const
{
   if(!PlotViewImp::toXml(pXml))
   {
      return false;
   }

   pXml->pushAddPoint(pXml->addElement("gridLines"));
   if(!mGridlines.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();

   return true;
}

bool PolarPlotImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if(!PlotViewImp::fromXml(pDocument, version))
   {
      return false;
   }
   for(DOMNode *pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if(XMLString::equals(pChld->getNodeName(), X("gridLines")))
      {
         if(!mGridlines.fromXml(pChld, version))
         {
            return false;
         }
      }
   }
   return true;
}
