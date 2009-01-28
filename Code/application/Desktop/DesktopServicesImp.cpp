/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "GpuResourceManager.h"
#include "ImageFilterManager.h"

#if defined(WIN_API)
#include <windows.h>
#endif

#include <QtCore/QUrl>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QCursor>
#include <QtGui/QDesktopServices>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QStatusBar>

#include "DesktopServicesImp.h"
#include "ApplicationWindow.h"
#include "AxisAdapter.h"
#include "BackgroundPluginWindow.h"
#include "CartesianPlotAdapter.h"
#include "ConfigurationSettingsImp.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "HistogramPlotAdapter.h"
#include "MenuBar.h"
#include "MouseModeImp.h"
#include "ObjectResource.h"
#include "PlotWidgetAdapter.h"
#include "PolarPlotAdapter.h"
#include "ProductViewAdapter.h"
#include "ProgressDlg.h"
#include "SignaturePlotAdapter.h"
#include "SpatialDataViewAdapter.h"
#include "SymbolManager.h"
#include "TypesFile.h"
#include "ViewImp.h"
#include "WorkspaceWindow.h"

#include <boost/bind.hpp>
#include <iostream>
using namespace std;

DesktopServicesImp* DesktopServicesImp::spInstance = NULL;
bool DesktopServicesImp::mDestroyed = false;

DesktopServicesImp* DesktopServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use DesktopServices after destroying it.");
      }
      spInstance = new DesktopServicesImp();
   }

   return spInstance;
}

void DesktopServicesImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy DesktopServices after destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

DesktopServicesImp::DesktopServicesImp() :
   mpGpuResourceManager(new GpuResourceManager()),
   mpImageFilterManager(new ImageFilterManager()),
   mDisplayMessageBox(true)
{
   ENSURE(mpGpuResourceManager != NULL);
   ENSURE(mpImageFilterManager != NULL);
}

DesktopServicesImp::~DesktopServicesImp()
{
   // Destroy the application window
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      delete pAppWindow;
   }
   delete mpGpuResourceManager;
   delete mpImageFilterManager;
}

const string& DesktopServicesImp::getObjectType() const
{
   static string sType("DesktopServicesImp");
   return sType;
}

bool DesktopServicesImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DesktopServices"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

QWidget* DesktopServicesImp::getMainWidget() const
{
   QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
   for (int i = 0; i < topLevelWidgets.count(); ++i)
   {
      QWidget* pMainWidget = dynamic_cast<ApplicationWindow*>(topLevelWidgets.at(i));
      if (pMainWidget != NULL)
      {
         return pMainWidget;
      }
   }

   return NULL;
}

MenuBar* DesktopServicesImp::getMainMenuBar() const
{
   MenuBar* pMenuBar = NULL;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pMenuBar = dynamic_cast<MenuBar*>(pAppWindow->menuBar());
   }

   return pMenuBar;
}

GpuResourceManager* DesktopServicesImp::getGpuResourceManager() const
{
   return mpGpuResourceManager;
}

ImageFilterManager* DesktopServicesImp::getImageFilterManager() const
{
   return mpImageFilterManager;
}

Window* DesktopServicesImp::createWindow(const string& windowName, WindowType windowType)
{
   if (windowName.empty() == true)
   {
      return NULL;
   }

   Window* pWindow = NULL;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pWindow = pAppWindow->createWindow(QString::fromStdString(windowName), windowType);
   }

   return pWindow;
}

Window* DesktopServicesImp::getWindow(const string& windowName, WindowType windowType) const
{
   if (windowName.empty() == true)
   {
      return NULL;
   }

   Window* pWindow = NULL;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pWindow = pAppWindow->getWindow(QString::fromStdString(windowName), windowType);
   }

   return pWindow;
}

Window* DesktopServicesImp::getWindow(const string& uniqueId) const
{
   if (uniqueId.empty() == true)
   {
      return NULL;
   }

   Window* pWindow = NULL;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pWindow = pAppWindow->getWindow(uniqueId);
   }

   return pWindow;
}

void DesktopServicesImp::getWindows(WindowType windowType, vector<Window*>& windows) const
{
   windows.clear();

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      windows = pAppWindow->getWindows(windowType);
   }
}

void DesktopServicesImp::getWindows(const string& windowType, vector<Window*>& windows) const
{
   windows.clear();

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      QString strWindowType;
      if (windowType.empty() == false)
      {
         strWindowType = QString::fromStdString(windowType);
      }

      windows = pAppWindow->getWindows(strWindowType);
   }
}

void DesktopServicesImp::getWindows(vector<Window*>& windows) const
{
   windows.clear();

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      windows = pAppWindow->getWindows();
   }
}

unsigned int DesktopServicesImp::getNumWindows(WindowType windowType) const
{
   vector<Window*> windows;
   getWindows(windowType, windows);

   return windows.size();
}

unsigned int DesktopServicesImp::getNumWindows(const string& windowType) const
{
   vector<Window*> windows;
   getWindows(windowType, windows);

   return windows.size();
}

unsigned int DesktopServicesImp::getNumWindows() const
{
   vector<Window*> windows;
   getWindows(windows);

   return windows.size();
}

bool DesktopServicesImp::deleteWindow(Window* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   bool bSuccess = false;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      bSuccess = pAppWindow->deleteWindow(pWindow);
   }

   return bSuccess;
}

void DesktopServicesImp::deleteAllWindows()
{
   vector<Window*> windows;
   getWindows(windows);
   for_each(windows.begin(), windows.end(), boost::bind(&DesktopServicesImp::deleteWindow, this, _1));
}

bool DesktopServicesImp::setCurrentWorkspaceWindow(WorkspaceWindow* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   bool bSuccess = false;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      bSuccess = pAppWindow->setCurrentWorkspaceWindow(pWindow);
   }

   return bSuccess;
}

WorkspaceWindow* DesktopServicesImp::getCurrentWorkspaceWindow() const
{
   WorkspaceWindow* pWindow = NULL;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pWindow = pAppWindow->getCurrentWorkspaceWindow();
   }

   return pWindow;
}

bool DesktopServicesImp::getCurrentWorkspaceWindowName(string& windowName) const
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      QString strWindowName = pAppWindow->getCurrentWorkspaceWindowName();
      if (strWindowName.isEmpty() == false)
      {
         windowName = strWindowName.toStdString();
         return true;
      }
   }

   return false;
}

View* DesktopServicesImp::getCurrentWorkspaceWindowView() const
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      return pAppWindow->getCurrentWorkspaceWindowView();
   }

   return NULL;
}

void DesktopServicesImp::cascadeWorkspaceWindows()
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->cascadeWorkspaceWindows();
   }
}

void DesktopServicesImp::tileWorkspaceWindows(TilingType eType)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->tileWorkspaceWindows(eType);
   }
}

bool DesktopServicesImp::tileWorkspaceWindows(const vector<WorkspaceWindow*>& windows, 
                                              bool maxFirst, TilingType eType)
{
   bool bSuccess = false;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      bSuccess = pAppWindow->tileWorkspaceWindows(windows, maxFirst, eType);
   }

   return bSuccess;
}

View* DesktopServicesImp::createView(const string& viewName, ViewType viewType, QWidget* pParent)
{
   if (viewName.empty() == true)
   {
      return NULL;
   }

   View* pView = NULL;
   switch (viewType)
   {
      case SPATIAL_DATA_VIEW:
         pView = new SpatialDataViewAdapter(SessionItemImp::generateUniqueId(), viewName, NULL, pParent);
         break;

      case PRODUCT_VIEW:
         pView = new ProductViewAdapter(SessionItemImp::generateUniqueId(), viewName, NULL, pParent);
         break;

      case PLOT_VIEW:
         pView = new CartesianPlotAdapter(SessionItemImp::generateUniqueId(), viewName, NULL, pParent);
         break;

      default:
         break;
   }

   return pView;
}

PlotView* DesktopServicesImp::createPlot(const string& plotName, PlotType plotType, QWidget* pParent)
{
   if (plotName.empty() == true)
   {
      return NULL;
   }

   PlotView* pPlot = NULL;
   switch (plotType)
   {
      case CARTESIAN_PLOT:
         pPlot = new CartesianPlotAdapter(SessionItemImp::generateUniqueId(), plotName, NULL, pParent);
         break;

      case HISTOGRAM_PLOT:
         pPlot = new HistogramPlotAdapter(SessionItemImp::generateUniqueId(), plotName, NULL, pParent);
         break;

      case SIGNATURE_PLOT:
         pPlot = new SignaturePlotAdapter(SessionItemImp::generateUniqueId(), plotName, NULL, pParent);
         break;

      case POLAR_PLOT:
         pPlot = new PolarPlotAdapter(SessionItemImp::generateUniqueId(), plotName, NULL, pParent);
         break;

      default:
         break;
   }

   return pPlot;
}

void DesktopServicesImp::deleteView(View* pView)
{
   if (pView != NULL)
   {
      delete dynamic_cast<ViewImp*>(pView);
   }
}

bool DesktopServicesImp::isKindOfView(const string& className, const string& viewName)
{
   bool bSuccess = false;

   if ((className == "CartesianPlot") || (className == "CartesianPlotAdapter") || (className == "CartesianPlotImp"))
   {
      bSuccess = CartesianPlotImp::isKindOfView(viewName);
   }
   else if ((className == "HistogramPlot") || (className == "HistogramPlotAdapter") ||
      (className == "HistogramPlotImp"))
   {
      bSuccess = HistogramPlotImp::isKindOfView(viewName);
   }
   else if ((className == "OrthographicView") || (className == "OrthographicViewImp"))
   {
      bSuccess = OrthographicViewImp::isKindOfView(viewName);
   }
   else if ((className == "PerspectiveView") || (className == "PerspectiveViewImp"))
   {
      bSuccess = PerspectiveViewImp::isKindOfView(viewName);
   }
   else if ((className == "PlotView") || (className == "PlotViewImp"))
   {
      bSuccess = PlotViewImp::isKindOfView(viewName);
   }
   else if ((className == "PolarPlot") || (className == "PolarPlotAdapter") || (className == "PolarPlotImp"))
   {
      bSuccess = PolarPlotImp::isKindOfView(viewName);
   }
   else if ((className == "ProductView") || (className == "ProductViewAdapter") || (className == "ProductViewImp"))
   {
      bSuccess = ProductViewImp::isKindOfView(viewName);
   }
   else if ((className == "SpatialDataView") || (className == "SpatialDataViewAdapter") ||
      (className == "SpatialDataViewImp"))
   {
      bSuccess = SpatialDataViewImp::isKindOfView(viewName);
   }
   else if ((className == "SignaturePlot") || (className == "SignaturePlotAdapter") ||
      (className == "SignaturePlotImp"))
   {
      bSuccess = SignaturePlotImp::isKindOfView(viewName);
   }
   else if ((className == "View") || (className == "ViewImp"))
   {
      bSuccess = ViewImp::isKindOfView(viewName);
   }

   return bSuccess;
}

void DesktopServicesImp::getViewTypes(const string& className, vector<string>& classList)
{
   if ((className == "CartesianPlot") || (className == "CartesianPlotAdapter") || (className == "CartesianPlotImp"))
   {
      CartesianPlotImp::getViewTypes(classList);
   }
   else if ((className == "HistogramPlot") || (className == "HistogramPlotAdapter") ||
      (className == "HistogramPlotImp"))
   {
      HistogramPlotImp::getViewTypes(classList);
   }
   else if ((className == "OrthographicView") || (className == "OrthographicViewImp"))
   {
      OrthographicViewImp::getViewTypes(classList);
   }
   else if ((className == "PerspectiveView") || (className == "PerspectiveViewImp"))
   {
      PerspectiveViewImp::getViewTypes(classList);
   }
   else if ((className == "PlotView") || (className == "PlotViewImp"))
   {
      PlotViewImp::getViewTypes(classList);
   }
   else if ((className == "PolarPlot") || (className == "PolarPlotAdapter") || (className == "PolarPlotImp"))
   {
      PolarPlotImp::getViewTypes(classList);
   }
   else if ((className == "ProductView") || (className == "ProductViewAdapter") || (className == "ProductViewImp"))
   {
      ProductViewImp::getViewTypes(classList);
   }
   else if ((className == "SpatialDataView") || (className == "SpatialDataViewAdapter") ||
      (className == "SpatialDataViewImp"))
   {
      SpatialDataViewImp::getViewTypes(classList);
   }
   else if ((className == "SignaturePlot") || (className == "SignaturePlotAdapter") ||
      (className == "SignaturePlotImp"))
   {
      SignaturePlotImp::getViewTypes(classList);
   }
   else if ((className == "View") || (className == "ViewImp"))
   {
      ViewImp::getViewTypes(classList);
   }
}

ProductWindow* DesktopServicesImp::deriveProduct(View* pView)
{
   ProductWindow* pWindow = NULL;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pWindow = pAppWindow->deriveProduct(pView);
   }

   return pWindow;
}

PlotWidget* DesktopServicesImp::createPlotWidget(const string& plotName, PlotType plotType, QWidget* pParent)
{
   if (plotName.empty() == true)
   {
      return NULL;
   }

   return new PlotWidgetAdapter(SessionItemImp::generateUniqueId(), plotName, plotType, NULL, pParent);
}

void DesktopServicesImp::deletePlotWidget(PlotWidget* pPlot)
{
   delete dynamic_cast<PlotWidgetImp*>(pPlot);
}

Axis* DesktopServicesImp::createAxis(AxisPosition position, QWidget* pParent)
{
   return new AxisAdapter(position, pParent);
}

void DesktopServicesImp::deleteAxis(Axis* pAxis)
{
   delete dynamic_cast<AxisImp*>(pAxis);
}

void DesktopServicesImp::initializeAction(QAction* pAction, const string& shortcutContext)
{
   if ((pAction == NULL) || (shortcutContext.empty() == true))
   {
      return;
   }

   // Ensure the context string contains forward slashes
   string context = shortcutContext;
   std::replace(context.begin(), context.end(), '\\', '/'); 

   // Store the shortcut context in the command action
   QMap<QString, QVariant> actionData = pAction->data().toMap();
   actionData["ShortcutContext"] = QVariant(QString::fromStdString(context));

   pAction->setData(QVariant(actionData));

   // Update the keyboard shortcut from the value in the configuration settings
   QString strName = pAction->toolTip();
   if (strName.isEmpty() == false)
   {
      const DynamicObject* pShortcuts = ApplicationWindow::getSettingShortcuts();
      string key = context + "/" + strName.toStdString();
      bool storeShortcut = true;
      if (pShortcuts != NULL)
      {
         const DataVariant& dv = pShortcuts->getAttributeByPath(key);
         if (dv.isValid() && dv.getTypeName() == "string")
         {
            pAction->setShortcut(QString::fromStdString(dv_cast<string>(dv)));
            storeShortcut = false; //shortcut already in config settings, no need to store current action value
         }
      }
      if (storeShortcut)
      {
         FactoryResource<DynamicObject> pNewShortcuts;
         VERIFYNRV(pNewShortcuts.get() != NULL);
         if (pShortcuts != NULL)
         {
            pNewShortcuts->merge(pShortcuts);
         }
         QString strShortcut = pAction->shortcut().toString();
         pNewShortcuts->setAttributeByPath(key, strShortcut.toStdString());
         ApplicationWindow::setSettingShortcuts(pNewShortcuts.get());
      }
   }
}

MouseMode* DesktopServicesImp::createMouseMode(const string& modeName, const char* const mouseCursor[],
                                               const char* const cursorMask[], int iHotX, int iHotY,
                                               QAction* pAction) const
{
   if (modeName.empty() == true)
   {
      return NULL;
   }

   QBitmap bmpCursor;
   if (mouseCursor != NULL)
   {
      bmpCursor = QPixmap(mouseCursor);
   }

   QBitmap bmpMask;
   if (cursorMask != NULL)
   {
      bmpMask = QPixmap(cursorMask);
   }

   QCursor cursor(Qt::ArrowCursor);
   if (bmpCursor.isNull() == false)
   {
      if (bmpMask.isNull() == false)
      {
         cursor = QCursor(bmpCursor, bmpMask, iHotX, iHotY);
      }
      else
      {
         cursor = QCursor(bmpCursor, iHotX, iHotY);
      }
   }

   return createMouseMode(modeName, cursor, pAction);
}

MouseMode* DesktopServicesImp::createMouseMode(const string& modeName, const QCursor& mouseCursor,
                                               QAction* pAction) const
{
   if (modeName.empty() == true)
   {
      return NULL;
   }

   MouseMode* pMouseMode = new MouseModeImp(QString::fromStdString(modeName), mouseCursor, pAction);
   return pMouseMode;
}

void DesktopServicesImp::deleteMouseMode(MouseMode* pMouseMode) const
{
   MouseModeImp* pMouseModeImp = static_cast<MouseModeImp*> (pMouseMode);
   if (pMouseModeImp != NULL)
   {
      delete pMouseModeImp;
   }
}

void DesktopServicesImp::setPanMode(PanModeType panMode)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->setPanMode(panMode);
   }
}

PanModeType DesktopServicesImp::getPanMode() const
{
   PanModeType panMode;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      panMode = pAppWindow->getPanMode();
   }

   return panMode;
}

void DesktopServicesImp::setAoiSelectionTool(GraphicObjectType toolType, ModeType modeType)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->setAoiSelectionTool(toolType, modeType);
   }
}

void DesktopServicesImp::setAoiAddMode(AoiAddMode mode)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->setAoiAddMode(mode);
   }
}

GraphicObjectType DesktopServicesImp::getAoiSelectionTool() const
{
   GraphicObjectType toolType;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      toolType = pAppWindow->getAoiSelectionTool();
   }

   return toolType;
}

ModeType DesktopServicesImp::getAoiSelectionMode() const
{
   ModeType modeType;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      modeType = pAppWindow->getAoiSelectionMode();
   }

   return modeType;
}

AoiAddMode DesktopServicesImp::getAoiAddMode() const
{
   AoiAddMode mode;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      mode = pAppWindow->getAoiAddMode();
   }

   return mode;
}

void DesktopServicesImp::setAnnotationObject(GraphicObjectType objectType)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->setAnnotationObject(objectType);
   }
}

GraphicObjectType DesktopServicesImp::getAnnotationObject() const
{
   GraphicObjectType objectType;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      objectType = pAppWindow->getAnnotationObject();
   }

   return objectType;
}

bool DesktopServicesImp::importFile(const string& importerSubtype, Progress* pProgress,
                                    vector<DataElement*>& importedElements)
{
   bool bSuccess = false;

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      bSuccess = pAppWindow->importFile(QString::fromStdString(importerSubtype), pProgress, importedElements);
   }

   return bSuccess;
}

bool DesktopServicesImp::exportSessionItem(SessionItem* pItem, FileDescriptor* pNewFileDescriptor, Progress* pProgress)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   VERIFY(pAppWindow != NULL);

   return pAppWindow->exportSessionItem(pItem, pNewFileDescriptor, pProgress);
}

bool DesktopServicesImp::exportSessionItems(const vector<SessionItem*>& items, Progress* pProgress)
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   VERIFY(pAppWindow != NULL);

   return pAppWindow->exportSessionItems(items, pProgress);
}

bool DesktopServicesImp::displayHelpHome() const
{
   return displayHelp(QString("%1/Help/Opticks/index.htm").arg(QString::fromStdString(
      Service<ConfigurationSettings>()->getHome())));
}

bool DesktopServicesImp::displayHelp(const QUrl& url) const
{
   if (url.isEmpty() || !url.isValid())
   {
      return false;
   }
   return QDesktopServices::openUrl(url);
}

bool DesktopServicesImp::displayHelp(const QString& filename) const
{
   return displayHelp(QUrl::fromLocalFile(filename));
}

bool DesktopServicesImp::displayHelp(const string& filename) const
{
   return displayHelp(QString::fromStdString(filename));
}

void DesktopServicesImp::displayProperties(SessionItem* pItem) const
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->displayProperties(pItem);
   }
}

void DesktopServicesImp::displayProperties(SessionItem* pItem, const vector<string>& displayedPages,
                                           bool bEmitSignal) const
{
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->displayProperties(pItem, displayedPages, bEmitSignal);
   }
}

void DesktopServicesImp::setStatusBarMessage(const string& messageText) const
{
   QString strMessage;
   if (messageText.empty() == false)
   {
      strMessage = QString::fromStdString(messageText);
   }

   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(getMainWidget());
   if (pAppWindow != NULL)
   {
      QStatusBar* pStatusBar = pAppWindow->statusBar();
      if (pStatusBar != NULL)
      {
         pStatusBar->showMessage(strMessage, 5000);
      }
   }
}

bool DesktopServicesImp::registerCallback(PlugInCallbackType eType, PlugInCallback* pCallback) const
{
   bool rval(false);

   BackgroundPluginWindow* pBpw = static_cast<BackgroundPluginWindow*>(getWindow("Background Plug-In Window",
      DOCK_WINDOW));
   if (pBpw != NULL && eType == BACKGROUND_COMPLETE)
   {
      rval = pBpw->addCallback(pCallback);
   }
   return rval;
}

void DesktopServicesImp::addBackgroundPlugIn(PlugIn* pPlugIn, Progress* pProgress) const
{
   BackgroundPluginWindow* pBpw = static_cast<BackgroundPluginWindow*>(getWindow("Background Plug-In Window",
      DOCK_WINDOW));
   if (pBpw != NULL)
   {
      pBpw->addItem(pPlugIn, pProgress);
      pBpw->show();
   }
}

DockWindowAreaType DesktopServicesImp::getDockWindowArea(const DockWindow& dockWindow) const
{
   // use the default initializer value (may be undefined)
   // since there is not "default" value. We should always
   // get some valid value so using this default means
   // bad things
   DockWindowAreaType area(DOCK_FLOATING);

   QMainWindow* pMain = dynamic_cast<QMainWindow*>(getMainWidget());
   const QDockWidget* pDockWidget = dynamic_cast<const QDockWidget*>(&dockWindow);
   VERIFYRV((pMain != NULL) && (pDockWidget != NULL), area);

   if (pDockWidget->isFloating())
   {
      area = DOCK_FLOATING;
   }
   else
   {
      // querying the area is a const operation but the function is not
      // declared const so this is a safe const_cast
      Qt::DockWidgetArea qtArea = pMain->dockWidgetArea(const_cast<QDockWidget*>(pDockWidget));
      switch (qtArea)
      {
         case Qt::LeftDockWidgetArea:
            area = DOCK_LEFT;
            break;
         case Qt::RightDockWidgetArea:
            area = DOCK_RIGHT;
            break;
         case Qt::TopDockWidgetArea:
            area = DOCK_TOP;
            break;
         case Qt::BottomDockWidgetArea:
            area = DOCK_BOTTOM;
            break;
         default:
            break;
      }
   }
   return area;
}

bool DesktopServicesImp::setDockWindowArea(DockWindow* pDockWindow, DockWindowAreaType dockArea)
{
   VERIFY(pDockWindow != NULL);
   QDockWidget* pDockWidget = dynamic_cast<QDockWidget*>(pDockWindow);
   VERIFY(pDockWidget != NULL);

   Qt::DockWidgetArea dock = Qt::AllDockWidgetAreas;
   switch (dockArea)
   {
   case DOCK_FLOATING:
      dock = Qt::AllDockWidgetAreas;
      break;
   case DOCK_TOP:
      dock = Qt::TopDockWidgetArea;
      break;
   case DOCK_BOTTOM:
      dock = Qt::BottomDockWidgetArea;
      break;
   case DOCK_LEFT:
      dock = Qt::LeftDockWidgetArea;
      break;
   case DOCK_RIGHT:
      dock = Qt::RightDockWidgetArea;
      break;
   default:
      break;
   }

   if (dock == Qt::AllDockWidgetAreas)
   {
      pDockWidget->setFloating(true);
   }
   else
   {
      pDockWidget->setFloating(false);

      QMainWindow* pMainWindow = dynamic_cast<QMainWindow*>(this->getMainWidget());
      VERIFY(pMainWindow != NULL);
      pMainWindow->addDockWidget(dock, pDockWidget);
   }

   return true;
}

int DesktopServicesImp::showMessageBox(const string& caption, const string& text, const string& button0,
                                       const string& button1, const string& button2, int defaultButton,
                                       int escapeButton) const
{
   if (mDisplayMessageBox)
   {
      QString qButton0;
      QString qButton1;
      QString qButton2;
      if (button0 != string())
      {
         qButton0 = button0.c_str();
      }
      else
      {
         qButton0 = "Ok";
      }
      if (button1 != string())
      {
         qButton1 = button1.c_str();
      }
      if (button2 != string())
      {
         qButton2 = button2.c_str();
      }

      return QMessageBox::warning(getMainWidget(), caption.c_str(), text.c_str(), qButton0, qButton1, qButton2, 
         defaultButton, escapeButton);
   }
   else
   {
      cout << "MESSAGE BOX: \nTitle: " << caption << ":\nMessage: " << text << endl;
      return 0;
   }
}

void DesktopServicesImp::useMessageBox(bool enable)
{
   mDisplayMessageBox = enable;
}

bool DesktopServicesImp::createProgressDialog(const string& caption, Progress* pProgress) const
{
   bool success = false;
   if (pProgress != NULL)
   {
      ProgressDlg* pProgressDlg = new ProgressDlg(QString::fromStdString(caption), getMainWidget());
      if (pProgressDlg != NULL)
      {
         pProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
         pProgress->attach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));
         success = true;
      }
   }
   return success;
}

ProgressDlg* DesktopServicesImp::createProgressDialog(const string& caption, Progress* pProgress,
                                                      QObject* pObject, const char* pSlot) const
{
   ProgressDlg* pProgressDlg = NULL;
   if (pProgress != NULL)
   {
      pProgressDlg = new ProgressDlg(QString::fromStdString(caption), getMainWidget());
      if (pProgressDlg != NULL)
      {
         pProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
         pProgress->attach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));
         if (pObject != NULL && pSlot != NULL)
         {
            pProgressDlg->enableAbort(true);
            QObject::connect(pProgressDlg, SIGNAL(aborted()), pObject, pSlot);
         }
      }
   }
   return pProgressDlg;
}

const vector<string>& DesktopServicesImp::getAvailableSymbolNames() const
{
   static vector<string> sEmpty;

   SymbolManager* pSymbolManager = SymbolManager::instance();
   VERIFYRV(pSymbolManager != NULL, sEmpty);
   return pSymbolManager->getAvailableSymbolNames();
}

const QImage& DesktopServicesImp::getSymbolImage(const string& symbol) const
{
   static QImage sImage;

   SymbolManager* pSymbolManager = SymbolManager::instance();
   VERIFYRV(pSymbolManager != NULL, sImage);
   return pSymbolManager->getSymbolImage(symbol);
}
