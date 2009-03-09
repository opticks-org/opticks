/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QMenu>

#include "CartesianPlotAdapter.h"
#include "CartesianPlotImp.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "AppAssert.h"
#include "DesktopServices.h"
#include "Icons.h"
#include "Undo.h"
#include "xmlreader.h"
#include "ZoomCustomDlg.h"

#include <limits>
#include <string>
using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

CartesianPlotImp::CartesianPlotImp(const string& id, const string& viewName, QGLContext* pDrawContext,
                                   QWidget* pParent) :
   PlotViewImp(id, viewName, pDrawContext, pParent),
   mHorizontalGridlines(HORIZONTAL, this, false),
   mVerticalGridlines(VERTICAL, this, false),
   mXScaleType(SCALE_LINEAR),
   mYScaleType(SCALE_LINEAR)
{
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   Service<DesktopServices> pDesktop;
   string shortcutContext = "Cartesian Plot/Gridlines";

   // Gridlines menu
   QMenu* pGridlinesMenu = new QMenu("&Gridlines", this);
   if (pGridlinesMenu != NULL)
   {
      // Major
      QAction* pMajorHorizGridAction = pGridlinesMenu->addAction(pIcons->mMajorHorizGrid, "Major &Horizontal");
      pMajorHorizGridAction->setAutoRepeat(false);
      pMajorHorizGridAction->setCheckable(true);
      pMajorHorizGridAction->setStatusTip("Toggles the display of horizontal gridlines at the major tickmarks");
      pMajorHorizGridAction->setToolTip("Major Horizontal Gridlines");
      connect(pMajorHorizGridAction, SIGNAL(toggled(bool)), this, SLOT(setMajorHorizontalGridlines(bool)));
      connect(&mHorizontalGridlines, SIGNAL(visibilityChanged(bool)), pMajorHorizGridAction, SLOT(setChecked(bool)));
      pDesktop->initializeAction(pMajorHorizGridAction, shortcutContext);

      QAction* pMajorVertGridAction = pGridlinesMenu->addAction(pIcons->mMajorVertGrid, "Major &Vertical");
      pMajorVertGridAction->setAutoRepeat(false);
      pMajorVertGridAction->setCheckable(true);
      pMajorVertGridAction->setStatusTip("Toggles the display of vertical gridlines at the major tickmarks");
      pMajorVertGridAction->setToolTip("Major Vertical Gridlines");
      connect(pMajorVertGridAction, SIGNAL(toggled(bool)), this, SLOT(setMajorVerticalGridlines(bool)));
      connect(&mVerticalGridlines, SIGNAL(visibilityChanged(bool)), pMajorVertGridAction, SLOT(setChecked(bool)));
      pDesktop->initializeAction(pMajorVertGridAction, shortcutContext);

      // Separator
      pGridlinesMenu->addSeparator();

      // Minor
      QAction* pMinorHorizGridAction = pGridlinesMenu->addAction(pIcons->mMinorHorizGrid, "Minor Hori&zontal");
      pMinorHorizGridAction->setAutoRepeat(false);
      pMinorHorizGridAction->setCheckable(true);
      pMinorHorizGridAction->setStatusTip("Toggles the display of horizontal gridlines at the minor tickmarks");
      pMinorHorizGridAction->setToolTip("Minor Horizontal Gridlines");
      connect(pMinorHorizGridAction, SIGNAL(toggled(bool)), this, SLOT(setMinorHorizontalGridlines(bool)));
      connect(&mHorizontalGridlines, SIGNAL(minorGridlinesEnabled(bool)), pMinorHorizGridAction,
         SLOT(setChecked(bool)));
      pDesktop->initializeAction(pMinorHorizGridAction, shortcutContext);

      QAction* pMinorVertGridAction = pGridlinesMenu->addAction(pIcons->mMinorVertGrid, "Minor Ver&tical");
      pMinorVertGridAction->setAutoRepeat(false);
      pMinorVertGridAction->setCheckable(true);
      pMinorVertGridAction->setStatusTip("Toggles the display of vertical gridlines at the minor tickmarks");
      pMinorVertGridAction->setToolTip("Minor Vertical Gridlines");
      connect(pMinorVertGridAction, SIGNAL(toggled(bool)), this, SLOT(setMinorVerticalGridlines(bool)));
      connect(&mVerticalGridlines, SIGNAL(minorGridlinesEnabled(bool)), pMinorVertGridAction, SLOT(setChecked(bool)));
      pDesktop->initializeAction(pMinorVertGridAction, shortcutContext);
   }

   // Custom zoom action
   QAction* pCustomZoomAction = new QAction("&Custom Zoom...", this);
   pCustomZoomAction->setAutoRepeat(false);
   pCustomZoomAction->setStatusTip("Zooms the plot axes to user-defined extents");
   connect(pCustomZoomAction, SIGNAL(triggered()), this, SLOT(zoomCustom()));
   pDesktop->initializeAction(pCustomZoomAction, shortcutContext);

   // Add the actions to the base class action list
   ContextMenuAction gridlinesAction(pGridlinesMenu->menuAction(), APP_CARTESIANPLOT_GRIDLINES_MENU_ACTION);
   gridlinesAction.mBuddyType = ContextMenuAction::BEFORE;
   gridlinesAction.mBuddyId = APP_PLOTVIEW_MOUSE_MODE_MENU_ACTION;
   addContextMenuAction(gridlinesAction);

   ContextMenuAction zoomAction(pCustomZoomAction, APP_CARTESIANPLOT_CUSTOM_ZOOM_ACTION);
   zoomAction.mBuddyType = ContextMenuAction::BEFORE;
   zoomAction.mBuddyId = APP_PLOTVIEW_RESCALE_AXES_ACTION;
   addContextMenuAction(zoomAction);

   // Initialization
   mHorizontalGridlines.setLineStyle(DOT);
   mHorizontalGridlines.setColor(ColorType(212, 208, 200));
   mHorizontalGridlines.setVisible(false);

   mVerticalGridlines.setLineStyle(DOT);
   mVerticalGridlines.setColor(ColorType(212, 208, 200));
   mVerticalGridlines.setVisible(false);
}

CartesianPlotImp::~CartesianPlotImp()
{
}

const string& CartesianPlotImp::getObjectType() const
{
   static string type("CartesianPlotImp");
   return type;
}

bool CartesianPlotImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CartesianPlot"))
   {
      return true;
   }

   return PlotViewImp::isKindOf(className);
}

bool CartesianPlotImp::isKindOfView(const string& className)
{
   if ((className == "CartesianPlotImp") || (className == "CartesianPlot"))
   {
      return true;
   }

   return PlotViewImp::isKindOfView(className);
}

void CartesianPlotImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("CartesianPlot");
   PlotViewImp::getViewTypes(classList);
}

CartesianPlotImp& CartesianPlotImp::operator= (const CartesianPlotImp& cartesianPlot)
{
   if (this != &cartesianPlot)
   {
      PlotViewImp::operator= (cartesianPlot);

      mHorizontalGridlines = cartesianPlot.mHorizontalGridlines;
      mVerticalGridlines = cartesianPlot.mVerticalGridlines;
      mXScaleType = cartesianPlot.mXScaleType;
      mYScaleType = cartesianPlot.mYScaleType;
      mXDataType = cartesianPlot.mXDataType;
      mYDataType = cartesianPlot.mYDataType;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

View* CartesianPlotImp::copy(QGLContext* pDrawContext, QWidget* pParent) const
{
   string viewName = getName();

   CartesianPlotAdapter* pView = new CartesianPlotAdapter(SessionItemImp::generateUniqueId(), viewName,
      pDrawContext, pParent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *(static_cast<CartesianPlotImp*>(pView)) = *this;
   }

   return pView;
}

bool CartesianPlotImp::copy(View* pView) const
{
   CartesianPlotImp* pViewImp = dynamic_cast<CartesianPlotImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

PlotType CartesianPlotImp::getPlotType() const
{
   return CARTESIAN_PLOT;
}

CartesianGridlines* CartesianPlotImp::getGridlines(OrientationType orientation)
{
   if (orientation == HORIZONTAL)
   {
      return &mHorizontalGridlines;
   }
   else if (orientation == VERTICAL)
   {
      return &mVerticalGridlines;
   }

   return NULL;
}

const CartesianGridlines* CartesianPlotImp::getGridlines(OrientationType orientation) const
{
   if (orientation == HORIZONTAL)
   {
      return &mHorizontalGridlines;
   }
   else if (orientation == VERTICAL)
   {
      return &mVerticalGridlines;
   }

   return NULL;
}

ScaleType CartesianPlotImp::getXScaleType() const
{
   return mXScaleType;
}

ScaleType CartesianPlotImp::getYScaleType() const
{
   return mYScaleType;
}

QString CartesianPlotImp::getXDataType() const
{
   return mXDataType;
}

QString CartesianPlotImp::getYDataType() const
{
   return mYDataType;
}

void CartesianPlotImp::translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const
{
   if (mXScaleType == SCALE_LINEAR)
   {
      worldX = dataX;
   }
   else if (mXScaleType == SCALE_LOG)
   {
      // Cannot take log of negative or zero value, so set to the minimum float value
      if (dataX <= 0)
      {
         dataX = numeric_limits<float>::min();
      }

      worldX = log10(dataX);
   }

   if (mYScaleType == SCALE_LINEAR)
   {
      worldY = dataY;
   }
   else if (mYScaleType == SCALE_LOG)
   {
      // Cannot take log of negative or zero value, so set to the minimum float value
      if (dataY <= 0)
      {
         dataY = numeric_limits<float>::min();
      }

      worldY = log10(dataY);
   }
}

void CartesianPlotImp::translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const
{
   double base = 10.0;

   if (mXScaleType == SCALE_LINEAR)
   {
      dataX = worldX;
   }
   else if (mXScaleType == SCALE_LOG)
   {
      dataX = pow(base, worldX);
   }

   if (mYScaleType == SCALE_LINEAR)
   {
      dataY = worldY;
   }
   else if (mYScaleType == SCALE_LOG)
   {
      dataY = pow(base, worldY);
   }
}

void CartesianPlotImp::setXScaleType(ScaleType scaleType)
{
   if (scaleType != mXScaleType)
   {
      mXScaleType = scaleType;
      emit xScaleTypeChanged(mXScaleType);
      notify(SIGNAL_NAME(CartesianPlot, XScaleTypeChanged), boost::any(mXScaleType));
      updateExtents();
      zoomExtents();
   }
}

void CartesianPlotImp::setYScaleType(ScaleType scaleType)
{
   if (scaleType != mYScaleType)
   {
      mYScaleType = scaleType;
      emit yScaleTypeChanged(mYScaleType);
      notify(SIGNAL_NAME(CartesianPlot, YScaleTypeChanged), boost::any(mYScaleType));
      updateExtents();
      zoomExtents();
   }
}

void CartesianPlotImp::setXDataType(const QString& strDataType)
{
   if (strDataType != mXDataType)
   {
      mXDataType = strDataType;
      emit xDataTypeChanged(mXDataType);
      notify(SIGNAL_NAME(CartesianPlot, XDataTypeChanged), boost::any(mXDataType.toStdString()));
   }
}

void CartesianPlotImp::setYDataType(const QString& strDataType)
{
   if (strDataType != mYDataType)
   {
      mYDataType = strDataType;
      emit yDataTypeChanged(mYDataType);
      notify(SIGNAL_NAME(CartesianPlot, YDataTypeChanged), boost::any(mYDataType.toStdString()));
   }
}

void CartesianPlotImp::zoomCustom()
{
   // Get the current display extents from the plot
   LocationType llCorner;
   LocationType ulCorner;
   LocationType urCorner;
   LocationType lrCorner;
   getVisibleCorners(llCorner, ulCorner, urCorner, lrCorner);

   translateWorldToData(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
   translateWorldToData(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);

   // Get the new extents from the user
   ZoomCustomDlg zoomDlg(this);
   zoomDlg.setZoomBox(llCorner, urCorner);

   int iReturn = zoomDlg.exec();
   if (iReturn == QDialog::Accepted)
   {
      // Zoom the plot
      zoomDlg.getZoomBox(llCorner, urCorner);
      translateDataToWorld(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
      translateDataToWorld(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);

      zoomToBox(llCorner, urCorner);
      refresh();
   }
}

void CartesianPlotImp::drawGridlines()
{
   mHorizontalGridlines.draw();
   mVerticalGridlines.draw();
}

void CartesianPlotImp::setMajorHorizontalGridlines(bool bShow)
{
   if (bShow == false)
   {
      mHorizontalGridlines.enableMinorGridlines(false);
   }

   mHorizontalGridlines.setVisible(bShow);
   refresh();
}

void CartesianPlotImp::setMajorVerticalGridlines(bool bShow)
{
   if (bShow == false)
   {
      mVerticalGridlines.enableMinorGridlines(false);
   }

   mVerticalGridlines.setVisible(bShow);
   refresh();
}

void CartesianPlotImp::setMinorHorizontalGridlines(bool bShow)
{
   if (bShow == true)
   {
      mHorizontalGridlines.setVisible(true);
   }

   mHorizontalGridlines.enableMinorGridlines(bShow);
   refresh();
}

void CartesianPlotImp::setMinorVerticalGridlines(bool bShow)
{
   if (bShow == true)
   {
      mVerticalGridlines.setVisible(true);
   }

   mVerticalGridlines.enableMinorGridlines(bShow);
   refresh();
}

bool CartesianPlotImp::toXml(XMLWriter* pXml) const
{
   if (!PlotViewImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("xDataType", mXDataType.toStdString());
   pXml->addAttr("yDataType", mYDataType.toStdString());
   pXml->addAttr("xScaleType", static_cast<int>(mXScaleType));
   pXml->addAttr("yScaleType", static_cast<int>(mYScaleType));
   pXml->pushAddPoint(pXml->addElement("HorizontalGridLines"));
   if (!mHorizontalGridlines.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();
   pXml->pushAddPoint(pXml->addElement("VerticalGridLines"));
   if (!mVerticalGridlines.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();

   return true;
}

bool CartesianPlotImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!PlotViewImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   QString strType = A(pElem->getAttribute(X("xDataType")));
   setXDataType(strType);
   strType = A(pElem->getAttribute(X("yDataType")));
   setYDataType(strType);
   ScaleTypeEnum eType = static_cast<ScaleTypeEnum>(StringUtilities::fromXmlString<int>
      (A(pElem->getAttribute(X("xScaleType")))));
   setXScaleType(eType);
   eType = static_cast<ScaleTypeEnum>(StringUtilities::fromXmlString<int>
      (A(pElem->getAttribute(X("yScaleType")))));
   setYScaleType(eType);

   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      string name = A(pChld->getNodeName());
      if (name == "HorizontalGridLines")
      {
         if (!mHorizontalGridlines.fromXml(pChld, version))
         {
            return false;
         }
      }
      else if (name == "VerticalGridLines")
      {
         if (!mVerticalGridlines.fromXml(pChld, version))
         {
            return false;
         }
      }
   }

   return true;
}
