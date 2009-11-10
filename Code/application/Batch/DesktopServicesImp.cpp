/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <iostream>

#include "ConfigurationSettings.h"
#include "DataVariant.h"
#include "DesktopServicesImp.h"
#include <QtGui/QImage>

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

DesktopServicesImp::DesktopServicesImp()
{
}

DesktopServicesImp::~DesktopServicesImp()
{
}

const string& DesktopServicesImp::getObjectType() const
{
   static string sType("DesktopServicesImp");
   return sType;
}

bool DesktopServicesImp::isKindOf(const string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return false;
}

bool DesktopServicesImp::attach(const string& signal, const Slot& slot)
{
   return false;
}

bool DesktopServicesImp::detach(const string& signal, const Slot& slot)
{
   return false;
}

void DesktopServicesImp::enableSignals(bool enabled)
{
}

bool DesktopServicesImp::signalsEnabled() const
{
   return false;
}

QWidget* DesktopServicesImp::getMainWidget() const
{
   return NULL;
}

MenuBar* DesktopServicesImp::getMainMenuBar() const
{
   return NULL;
}

GpuResourceManager* DesktopServicesImp::getGpuResourceManager() const
{
   return NULL;
}

ImageFilterManager* DesktopServicesImp::getImageFilterManager() const
{
   return NULL;
}

Window* DesktopServicesImp::createWindow(const string& windowName, WindowType windowType)
{
   return NULL;
}

Window* DesktopServicesImp::getWindow(const string& windowName, WindowType windowType) const
{
   return NULL;
}

Window* DesktopServicesImp::getWindow(const string& uniqueId) const
{
   return NULL;
}

void DesktopServicesImp::getWindows(WindowType windowType, vector<Window*>& windows) const
{
}

void DesktopServicesImp::getWindows(const string& windowType, vector<Window*>& windows) const
{
}

void DesktopServicesImp::getWindows(vector<Window*>& windows) const
{
}

unsigned int DesktopServicesImp::getNumWindows(WindowType windowType) const
{
   return 0;
}

unsigned int DesktopServicesImp::getNumWindows(const string& windowType) const
{
   return 0;
}

unsigned int DesktopServicesImp::getNumWindows() const
{
   return 0;
}

bool DesktopServicesImp::deleteWindow(Window* pWindow)
{
   return false;
}

void DesktopServicesImp::deleteAllWindows()
{
}

bool DesktopServicesImp::setCurrentWorkspaceWindow(WorkspaceWindow* pWindow)
{
   return false;
}

WorkspaceWindow* DesktopServicesImp::getCurrentWorkspaceWindow() const
{
   return NULL;
}

bool DesktopServicesImp::getCurrentWorkspaceWindowName(string& windowName) const
{
   return false;
}

View* DesktopServicesImp::getCurrentWorkspaceWindowView() const
{
   return NULL;
}

void DesktopServicesImp::cascadeWorkspaceWindows()
{
}

void DesktopServicesImp::tileWorkspaceWindows(TilingType eType)
{
}

bool DesktopServicesImp::tileWorkspaceWindows(const vector<WorkspaceWindow*>& windows, bool maxFirst, TilingType eType)
{
   return false;
}

View* DesktopServicesImp::createView(const string& viewName, ViewType viewType, QWidget* pParent)
{
   return NULL;
}

PlotView* DesktopServicesImp::createPlot(const string& plotName, PlotType plotType, QWidget* pParent)
{
   return NULL;
}

void DesktopServicesImp::deleteView(View* pView)
{
}

bool DesktopServicesImp::isKindOfView(const string& className, const string& viewName)
{
   return false;
}

void DesktopServicesImp::getViewTypes(const string& className, vector<string>& classList)
{
}

bool DesktopServicesImp::isKindOfLayer(const std::string& className, const std::string& layerName)
{
   return false;
}

void DesktopServicesImp::getLayerTypes(const std::string& className, std::vector<std::string>& classList)
{
}

ProductWindow* DesktopServicesImp::deriveProduct(View* pView)
{
   return NULL;
}

PlotWidget* DesktopServicesImp::createPlotWidget(const string& plotName, PlotType plotType, QWidget* pParent)
{
   return NULL;
}

void DesktopServicesImp::deletePlotWidget(PlotWidget* pPlot)
{
}

Axis* DesktopServicesImp::createAxis(AxisPosition position, QWidget* pParent)
{
   return NULL;
}

void DesktopServicesImp::deleteAxis(Axis* pAxis)
{
}

void DesktopServicesImp::initializeAction(QAction* pAction, const string& shortcutContext)
{
}

MouseMode* DesktopServicesImp::createMouseMode(const string& modeName, const char* const mouseCursor[],
                                               const char* const cursorMask[], int iHotX, int iHotY,
                                               QAction* pAction) const
{
   return NULL;
}

MouseMode* DesktopServicesImp::createMouseMode(const string& modeName, const QCursor& mouseCursor,
                                               QAction* pAction) const
{
   return NULL;
}

void DesktopServicesImp::deleteMouseMode(MouseMode* pMouseMode) const
{
}

void DesktopServicesImp::setPanMode(PanModeType panMode)
{
}

PanModeType DesktopServicesImp::getPanMode() const
{
   return PanModeType();
}

void DesktopServicesImp::setAoiSelectionTool(GraphicObjectType toolType, ModeType modeType)
{
}

void DesktopServicesImp::setAoiAddMode(AoiAddMode mode)
{
}

GraphicObjectType DesktopServicesImp::getAoiSelectionTool() const
{
   return GraphicObjectType();
}

ModeType DesktopServicesImp::getAoiSelectionMode() const
{
   return ModeType();
}

AoiAddMode DesktopServicesImp::getAoiAddMode() const
{
   return AoiAddMode();
}

void DesktopServicesImp::setAnnotationObject(GraphicObjectType objectType)
{
}

GraphicObjectType DesktopServicesImp::getAnnotationObject() const
{
   return GraphicObjectType();
}

bool DesktopServicesImp::importFile(const string& importerSubtype, Progress* pProgress)
{
   return false;
}

bool DesktopServicesImp::importFile(const string& importerSubtype, Progress* pProgress,
                                    vector<DataElement*>& importedElements)
{
   return false;
}

bool DesktopServicesImp::exportSessionItem(SessionItem* pItem, FileDescriptor* pNewFileDescriptor, Progress* pProgress)
{
   return false;
}

bool DesktopServicesImp::exportSessionItems(const vector<SessionItem*>& items, Progress* pProgress)
{
   return false;
}

bool DesktopServicesImp::displayHelpHome() const
{
   return false;
}

bool DesktopServicesImp::displayHelp(const string& filename) const
{
   return false;
}

void DesktopServicesImp::displayProperties(SessionItem* pItem) const
{
}

void DesktopServicesImp::displayProperties(SessionItem* pItem, const vector<string>& displayedPages,
                                           bool bEmitSignal) const
{
}

void DesktopServicesImp::setStatusBarMessage(const string& messageText) const
{
   cout << "SHORT MESSAGE: " << messageText << endl;
}

void DesktopServicesImp::setSuppressibleMsgDlgState(const string& id, bool bState)
{
   Service<ConfigurationSettings> pSettings;
   pSettings->setSetting(id, bState);
}

bool DesktopServicesImp::getSuppressibleMsgDlgState(const string& id)
{
   Service<ConfigurationSettings> pSettings;
   const bool* pState = dv_cast<bool>(&pSettings->getSetting(id));
   return pState != NULL ? *pState : false;
}

void DesktopServicesImp::showSuppressibleMsgDlg(const string& dialogTitle, const string& dialogMsg, MessageType type, 
                                        const string& id, QWidget* pParent) 
{ 
   if (!getSuppressibleMsgDlgState(id))
   {
      cout << "SUPPRESSIBLE MESSAGE DIALOG: \nTitle: " << dialogTitle << ":\nMessage: " << dialogMsg << endl;
   }
}

int DesktopServicesImp::showMessageBox(const string& caption, const string& text, const string& button0,
                                       const string& button1, const string& button2, int defaultButton,
                                       int escapeButton) const
{
   cout << "MESSAGE BOX: \nTitle: " << caption << ":\nMessage: " << text << endl;
   return defaultButton;
}

void DesktopServicesImp::useMessageBox(bool enable)
{
}

bool DesktopServicesImp::registerCallback(PlugInCallbackType eType, PlugInCallback* pCallback) const
{
   return false;
}

void DesktopServicesImp::addBackgroundPlugIn(PlugIn* pPlugIn, Progress* pProgress) const
{
}

DockWindowAreaType DesktopServicesImp::getDockWindowArea(const DockWindow& dockWindow) const
{
   return DockWindowAreaType();
}

bool DesktopServicesImp::setDockWindowArea(DockWindow* pDockWindow, DockWindowAreaType dockArea)
{
   return false;
}

bool DesktopServicesImp::createProgressDialog(const string& caption, Progress* pProgress) const
{
   return false;
}

ProgressDlg* DesktopServicesImp::createProgressDialog(const string& caption, Progress* pProgress, QObject* pObject,
                                                      const char* pSlot) const
{
   return NULL;
}

const vector<string>& DesktopServicesImp::getAvailableSymbolNames() const
{
   static vector<string> sEmpty;
   return sEmpty;
}

const QImage& DesktopServicesImp::getSymbolImage(const string& symbol) const
{
   static QImage sImage;
   return sImage;
}
