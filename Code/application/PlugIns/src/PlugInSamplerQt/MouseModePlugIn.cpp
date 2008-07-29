/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>

#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "Layer.h"
#include "LayerList.h"
#include "MouseMode.h"
#include "MouseModePlugIn.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "ToolBar.h"

using namespace std;

static const char* const MouseModeIcon[] =
{
   "16 16 2 1",
   "# c #000000",
   ". c #ffffff",
   "................",
   "................",
   "................",
   "................",
   "................",
   ".#...#....#...#.",
   "..#.#.....#...#.",
   "...#.......#.#..",
   "...#.......#.#..",
   "..#.#...#...#...",
   ".#...#..#...#...",
   "........#..#....",
   ".......#...#....",
   "................",
   "................",
   "................"
};

MouseModePlugIn::MouseModePlugIn() :
   mpMouseMode(NULL),
   mpMouseModeAction(NULL)
{
   AlgorithmShell::setName("Custom Mouse Mode Plug-In");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Demonstrates creating a custom mouse mode.");
   setDescriptorId("{3C32A63F-77C5-4F78-8AE2-E9DEFC2F625E}");
   setProductionStatus(false);
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setAbortSupported(false);
   setWizardSupported(false);
}

MouseModePlugIn::~MouseModePlugIn()
{
   Service<DesktopServices> pDesktop;

   // Remove the toolbar button and delete the mouse mode action
   ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Demo", TOOLBAR));
   if (pToolBar != NULL)
   {
      if (mpMouseModeAction != NULL)
      {
         pToolBar->removeItem(mpMouseModeAction);
         delete mpMouseModeAction;
      }
   }

   // Detach from desktop services
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &MouseModePlugIn::windowAdded));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &MouseModePlugIn::windowActivated));
   pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &MouseModePlugIn::windowRemoved));

   // Remove the mouse mode from the views
   vector<Window*> windows;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pView = pWindow->getSpatialDataView();
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->removeEventFilter(this);
            }

            removeMouseMode(pView);
         }
      }
   }

   // Delete the custom mouse mode
   if (mpMouseMode != NULL)
   {
      pDesktop->deleteMouseMode(mpMouseMode);
   }
}

bool MouseModePlugIn::setBatch()
{
   AlgorithmShell::setBatch();
   return false;
}

bool MouseModePlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return !isBatch();
}

bool MouseModePlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return !isBatch();
}

bool MouseModePlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (isBatch() == true)
   {
      return false;
   }

   Service<DesktopServices> pDesktop;

   // Create the custom mouse mode action
   QPixmap mouseModePix = QPixmap(MouseModeIcon);
   mouseModePix.setMask(mouseModePix.createHeuristicMask());
   QIcon mouseModeIcon(mouseModePix);

   mpMouseModeAction = new QAction(mouseModeIcon, "Display Pixel Coordinate", this);
   mpMouseModeAction->setAutoRepeat(false);
   mpMouseModeAction->setCheckable(true);
   mpMouseModeAction->setStatusTip("Displays the coordinate of a pixel selected with the mouse");

   // Add a button to the Demo toolbar
   ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Demo", TOOLBAR));
   if (pToolBar != NULL)
   {
      pToolBar->addButton(mpMouseModeAction);
   }

   // Initialization
   enableAction();

   // Connections
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &MouseModePlugIn::windowAdded));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowActivated), Slot(this, &MouseModePlugIn::windowActivated));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowRemoved), Slot(this, &MouseModePlugIn::windowRemoved));

   return true;
}

bool MouseModePlugIn::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pEvent->type() == QEvent::MouseButtonPress)
      {
         QMouseEvent* pMouseEvent = static_cast<QMouseEvent*> (pEvent);
         if (pMouseEvent->button() == Qt::LeftButton)
         {
            Service<DesktopServices> pDesktop;

            SpatialDataView* pSpatialDataView =
               dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
            if (pSpatialDataView != NULL)
            {
               QWidget* pViewWidget  = pSpatialDataView->getWidget();
               if (pViewWidget == pObject)
               {
                  MouseMode* pMouseMode = pSpatialDataView->getCurrentMouseMode();
                  if (pMouseMode != NULL)
                  {
                     string mouseMode = "";
                     pMouseMode->getName(mouseMode);
                     if (mouseMode == "DisplayPixelCoordinateMode")
                     {
                        QPoint ptMouse = pMouseEvent->pos();
                        ptMouse.setY(pViewWidget->height() - pMouseEvent->pos().y());

                        LocationType pixelCoord;
                        pSpatialDataView->translateScreenToWorld(ptMouse.x(), ptMouse.y(),
                           pixelCoord.mX, pixelCoord.mY);

                        double dMinX = 0.0;
                        double dMinY = 0.0;
                        double dMaxX = 0.0;
                        double dMaxY = 0.0;
                        pSpatialDataView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

                        if ((pixelCoord.mX >= dMinX) && (pixelCoord.mX <= dMaxX) && (pixelCoord.mY >= dMinY) &&
                           (pixelCoord.mY <= dMaxY))
                        {
                           LayerList* pLayerList = pSpatialDataView->getLayerList();
                           if (pLayerList != NULL)
                           {
                              RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
                              if (pRaster != NULL)
                              {
                                 Layer* pLayer = pLayerList->getLayer(RASTER, pRaster);
                                 if (pLayer != NULL)
                                 {
                                    LocationType dataCoord;
                                    pLayer->translateWorldToData(pixelCoord.mX, pixelCoord.mY,
                                       dataCoord.mX, dataCoord.mY);

                                    // Get the original pixel coordinates
                                    const RasterDataDescriptor* pDescriptor =
                                       dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
                                    if (pDescriptor != NULL)
                                    {
                                       const vector<DimensionDescriptor>& activeRows = pDescriptor->getRows();
                                       const vector<DimensionDescriptor>& activeColumns = pDescriptor->getColumns();
                                       if ((dataCoord.mY >= 0) &&
                                          (activeRows.size() > static_cast<unsigned int>(dataCoord.mY)) &&
                                          (activeRows[dataCoord.mY].isValid()) &&
                                          (dataCoord.mX >= 0) &&
                                          (activeColumns.size() > static_cast<unsigned int>(dataCoord.mX)) &&
                                          (activeColumns[dataCoord.mX].isValid()))
                                       {
                                          DimensionDescriptor rowDim = activeRows[dataCoord.mY];
                                          DimensionDescriptor columnDim = activeColumns[dataCoord.mX];

                                          unsigned int originalSceneX = columnDim.getOriginalNumber() + 1;
                                          unsigned int originalSceneY = rowDim.getOriginalNumber() + 1;

                                          QMessageBox::information(pViewWidget, "Display Pixel Coordinate",
                                             "The coordinate of the selected pixel is (" +
                                             QString::number(originalSceneX) + ", " +
                                             QString::number(originalSceneY) + ")");
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return QObject::eventFilter(pObject, pEvent);
}

void MouseModePlugIn::windowAdded(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) != NULL)
   {
      Window* pWindow = boost::any_cast<Window*>(value);

      SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*>(pWindow);
      if (pSpatialDataWindow != NULL)
      {
         SpatialDataView* pView = pSpatialDataWindow->getSpatialDataView();
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->installEventFilter(this);
            }

            addMouseMode(pView);
         }
      }
   }
}

void MouseModePlugIn::windowActivated(Subject& subject, const string& signal, const boost::any& value)
{
   enableAction();
}

void MouseModePlugIn::windowRemoved(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) != NULL)
   {
      Window* pWindow = boost::any_cast<Window*>(value);

      SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*>(pWindow);
      if (pSpatialDataWindow != NULL)
      {
         SpatialDataView* pView = pSpatialDataWindow->getSpatialDataView();
         if (pView != NULL)
         {
            QWidget* pViewWidget = pView->getWidget();
            if (pViewWidget != NULL)
            {
               pViewWidget->removeEventFilter(this);
            }

            removeMouseMode(pView);
         }
      }
   }
}

void MouseModePlugIn::addMouseMode(SpatialDataView* pView)
{
   if (pView == NULL)
   {
      return;
   }

   Service<DesktopServices> pDesktop;

   // Create the custom mouse mode
   if (mpMouseMode == NULL)
   {
      // To set a custom mouse cursor for the custom mouse mode change the
      // NULL pointers when creating the mouse mode to the custom cursor
      mpMouseMode = pDesktop->createMouseMode("DisplayPixelCoordinateMode", NULL, NULL, -1, -1, mpMouseModeAction);
   }

   // Add the mouse mode to the view
   if (mpMouseMode != NULL)
   {
      pView->addMouseMode(mpMouseMode);
   }
}

void MouseModePlugIn::removeMouseMode(SpatialDataView* pView)
{
   // Remove the mouse mode from the view
   if ((pView != NULL) && (mpMouseMode != NULL))
   {
      pView->removeMouseMode(mpMouseMode);
   }
}

void MouseModePlugIn::enableAction()
{
   if (mpMouseModeAction != NULL)
   {
      Service<DesktopServices> pDesktop;
      bool bEnable = false;

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
      if (pWindow != NULL)
      {
         SpatialDataView* pView = pWindow->getSpatialDataView();
         if (pView != NULL)
         {
            bEnable = true;
         }
      }

      mpMouseModeAction->setEnabled(bEnable);
   }
}
