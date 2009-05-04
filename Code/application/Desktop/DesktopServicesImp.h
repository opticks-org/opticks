/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DESKTOPSERVICESIMP_H
#define DESKTOPSERVICESIMP_H

#include "DesktopServices.h"
#include "SubjectImp.h"

#include <string>
#include <vector>

class Axis;
class GpuResourceManager;
class ImageFilterManager;
class MenuBar;
class PlotView;
class PlotWidget;
class PlugIn;
class ProductWindow;
class ProgressDlg;
class QObject;
class QString;
class QUrl;
class View;

/**
 *  Implementation of the Desktop Services interface.
 */
class DesktopServicesImp : public DesktopServices, public SubjectImp
{
public:
   /**
    *  Retrieves a pointer to the class.
    *
    *  @return   A pointer to the only instance of the class.
    */
   static DesktopServicesImp* instance();
   static void destroy();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

   QWidget* getMainWidget() const;
   MenuBar* getMainMenuBar() const;

   /**
    *  Returns the singleton which manages GPU resources.
    *
    *  @return A pointer to the GpuResourceManager or NULL if
    *          the application is in batch mode.
    */
   virtual GpuResourceManager* getGpuResourceManager() const;

   /**
    *  Returns the singleton which manages GPU image filters.
    *
    *  @return A pointer to the ImageFilterManager or NULL if
    *          the application is in batch mode.
    */
   virtual ImageFilterManager* getImageFilterManager() const;

   // Window services
   Window* createWindow(const std::string& windowName, WindowType windowType);
   Window* getWindow(const std::string& windowName, WindowType windowType) const;
   Window* getWindow(const std::string& uniqueId) const;
   void getWindows(WindowType windowType, std::vector<Window*>& windows) const;
   void getWindows(const std::string& windowType, std::vector<Window*>& windows) const;
   void getWindows(std::vector<Window*>& windows) const;
   unsigned int getNumWindows(WindowType windowType) const;
   unsigned int getNumWindows(const std::string& windowType) const;
   unsigned int getNumWindows() const;
   bool deleteWindow(Window* pWindow);
   void deleteAllWindows();

   // Workspace window services
   bool setCurrentWorkspaceWindow(WorkspaceWindow* pWindow);
   WorkspaceWindow* getCurrentWorkspaceWindow() const;
   bool getCurrentWorkspaceWindowName(std::string& windowName) const;
   View* getCurrentWorkspaceWindowView() const;
   void cascadeWorkspaceWindows();
   void tileWorkspaceWindows(TilingType eType = TILE_GRID);
   bool tileWorkspaceWindows(const std::vector<WorkspaceWindow*>& windows, bool maxFirst = true,
      TilingType eType = TILE_GRID);

   // Dock window services
   DockWindowAreaType getDockWindowArea(const DockWindow& dockWindow) const;
   bool setDockWindowArea(DockWindow* pDockWindow, DockWindowAreaType dockArea);

   // View services
   View* createView(const std::string& viewName, ViewType viewType, QWidget* pParent = 0);
   PlotView* createPlot(const std::string& plotName, PlotType plotType, QWidget* pParent = 0);
   void deleteView(View* pView);
   bool isKindOfView(const std::string& className, const std::string& viewName);
   void getViewTypes(const std::string& className, std::vector<std::string>& classList);
   bool isKindOfLayer(const std::string& className, const std::string& layerName);
   void getLayerTypes(const std::string& className, std::vector<std::string>& classList);
   ProductWindow* deriveProduct(View* pView);

   // Plot services
   PlotWidget* createPlotWidget(const std::string& plotName, PlotType plotType, QWidget* pParent = 0);
   void deletePlotWidget(PlotWidget* pPlot);
   Axis* createAxis(AxisPosition position, QWidget* pParent = 0);
   void deleteAxis(Axis* pAxis);

   // Mode services
   void initializeAction(QAction* pAction, const std::string& shortcutContext);

   MouseMode* createMouseMode(const std::string& modeName, const char* const mouseCursor[],
      const char* const cursorMask[], int iHotX = 0, int iHotY = 0, QAction* pAction = NULL) const;
   MouseMode* createMouseMode(const std::string& modeName, const QCursor& mouseCursor, QAction* pAction = NULL) const;
   void deleteMouseMode(MouseMode* pMouseMode) const;

   void setPanMode(PanModeType panMode);
   PanModeType getPanMode() const;

   void setAoiSelectionTool(GraphicObjectType toolType, ModeType modeType);
   void setAoiAddMode(AoiAddMode mode);
   GraphicObjectType getAoiSelectionTool() const;
   ModeType getAoiSelectionMode() const;
   AoiAddMode getAoiAddMode() const;

   void setAnnotationObject(GraphicObjectType objectType);
   GraphicObjectType getAnnotationObject() const;

   // Data services
   bool importFile(const std::string& importerSubtype = std::string(), Progress* pProgress = NULL);
   bool importFile(const std::string& importerSubtype, Progress* pProgress, std::vector<DataElement*>& importedElements);
   bool exportSessionItem(SessionItem* pItem, FileDescriptor* pNewFileDescriptor = NULL, Progress* pProgress = NULL);
   bool exportSessionItems(const std::vector<SessionItem*>& items, Progress* pProgress = NULL);

   // Help services
   bool displayHelpHome() const;
   bool displayHelp(const std::string& filename) const;
   void displayProperties(SessionItem* pItem) const;
   void displayProperties(SessionItem* pItem, const std::vector<std::string>& displayedPages,
      bool bEmitSignal = true) const;
   bool displayHelp(const QString& filename) const;
   bool displayHelp(const QUrl& url) const;
   void setStatusBarMessage(const std::string& messageText) const;
   void setSuppressibleMsgDlgState(const std::string& id, bool bState);
   bool getSuppressibleMsgDlgState(const std::string& id);
   void showSuppressibleMsgDlg(const std::string& dialogTitle, const std::string& dialogMsg, MessageType type, 
      const std::string& id, QWidget* pParent = NULL);
   void useMessageBox(bool enable);
   int showMessageBox(const std::string& caption, const std::string& text,
      const std::string& button0 = std::string(), const std::string& button1 = std::string(),
      const std::string& button2 = std::string(), int defaultButton = 0, int escapeButton = -1) const;

   virtual bool createProgressDialog(const std::string& caption, Progress* pProgress) const;
   ProgressDlg* createProgressDialog(const std::string& caption, Progress* pProgress, QObject* pObject,
      const char* pSlot) const;

   // Plug-in services
   void addBackgroundPlugIn(PlugIn* pPlugIn, Progress* pProgress) const;
   virtual bool registerCallback(PlugInCallbackType eType, PlugInCallback* pCallback) const;

   const std::vector<std::string>& getAvailableSymbolNames() const;
   const QImage& getSymbolImage(const std::string& symbol) const;

protected:
   /**
    *  Constructs the desktop services object and initializes the services arg lists
    */
   DesktopServicesImp();

   /**
    *  Destructs the desktop services object and deletes all arg lists
    */
   virtual ~DesktopServicesImp();

private:
   static DesktopServicesImp* spInstance;
   static bool mDestroyed;

   // these are mutable since they are really standalone singletons
   // whose lifetimes are managed by DesktopServicesImp
   GpuResourceManager* mpGpuResourceManager;
   ImageFilterManager* mpImageFilterManager;
   int mDisplayMessageBox;
};

#endif
