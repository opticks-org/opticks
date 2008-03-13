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

#include <vector>

class GpuResourceManager;
class ImageFilterManager;
class ProductWindow;
class ProgressDlg;
class QObject;

/**
 *  Batch Implementation of the Desktop Services interface.
 *
 *  The following Desktop Services class does nothing.  It has no real 
 *  function in batch mode.  Each method executes, but only returns NULL and 
 *  values representing a failed operation.
 *
 *  @see   BatchProcessor
 */
class DesktopServicesImp : public DesktopServices
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

   bool attach(const std::string& signal, const Slot& slot);
   bool detach(const std::string& signal, const Slot& slot);

   QWidget* getMainWidget() const;
   MenuBar* getMainMenuBar() const;

   /**
    *  Returns the singleton which manages GPU resources.
    *
    *  @return A pointer to the GpuResourceManager or NULL if
    *          the application is in batch mode.
    */
   virtual GpuResourceManager *getGpuResourceManager() const;

   /**
    *  Returns the singleton which manages GPU image filters.
    *
    *  @return A pointer to the ImageFilterManager or NULL if
    *          the application is in batch mode.
    */
   virtual ImageFilterManager *getImageFilterManager() const;

   /**
    * Non-functional desktop services
    *
    * All of the following Desktop Services methods do nothing.  They have no real 
    * function in batch mode.  Each method executes, but only returns NULL and 
    * values representing a failed operation.
    */
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

   bool setCurrentWorkspaceWindow(WorkspaceWindow* pWindow);
   WorkspaceWindow* getCurrentWorkspaceWindow() const;
   bool getCurrentWorkspaceWindowName(std::string& windowName) const;
   View* getCurrentWorkspaceWindowView() const;
   void cascadeWorkspaceWindows();
   void tileWorkspaceWindows(TilingType eType = TILE_GRID);
   bool tileWorkspaceWindows(const std::vector<WorkspaceWindow*>& windows, 
                             bool maxFirst = true, TilingType eType = TILE_GRID);

   View* createView(const std::string& viewName, ViewType viewType, QWidget* pParent = 0);
   PlotView* createPlot(const std::string& plotName, PlotType plotType, QWidget* pParent = 0);
   void getViewTypes(const std::string& className, std::vector<std::string>& classList);
   void deleteView(View* pView);
   bool isKindOfView(const std::string& className, const std::string& viewName);
   ProductWindow *deriveProduct(View *pView);

   PlotWidget* createPlotWidget(const std::string& plotName, PlotType plotType, QWidget* pParent = 0);
   void deletePlotWidget(PlotWidget* pPlot);
   Axis* createAxis(AxisPosition position, QWidget* pParent = 0);
   void deleteAxis(Axis* pAxis);

   void initializeAction(QAction* pAction, const std::string& shortcutContext);

   MouseMode* createMouseMode(const std::string& modeName, const char* const mouseCursor[],
      const char* const cursorMask[], int iHotX = 0, int iHotY = 0, QAction* pAction = NULL) const;
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

   bool importFile(const std::string& importerSubtype = std::string(), Progress* pProgress = NULL,
      std::vector<DataElement*>& importedElements = std::vector<DataElement*>());
   bool exportSessionItem(SessionItem* pItem, FileDescriptor* pNewFileDescriptor = NULL, Progress* pProgress = NULL);
   bool exportSessionItems(const std::vector<SessionItem*>& items, Progress* pProgress = NULL);

   bool displayHelpHome() const;
   bool displayHelp(const std::string& filename) const;
   void displayProperties(SessionItem* pItem) const;
   void displayProperties(SessionItem* pItem, const std::vector<std::string>& displayedPages,
      bool bEmitSignal = true) const;
   void setStatusBarMessage(const std::string& messageText) const;
   void useMessageBox(bool enable);
   int showMessageBox(const std::string &caption, const std::string &text, 
      const std::string &button0=std::string(), const std::string &button1=std::string(), 
      const std::string &button2=std::string(), int defaultButton=0, 
      int escapeButton=-1) const;

   virtual bool createProgressDialog(const std::string &caption, Progress *pProgress) const;
   ProgressDlg* createProgressDialog(const std::string &caption, Progress *pProgress,
      QObject *pObject, const char *pSlot) const;

   void addBackgroundPlugIn(PlugIn* pPlugIn, Progress* pProgress) const;
   virtual bool registerCallback(PlugInCallbackType eType, PlugInCallback *callback) const;

   DockWindowAreaType getDockWindowArea(const DockWindow &dockWindow) const;
   bool setDockWindowArea(DockWindow *pDockWindow, 
      DockWindowAreaType dockArea);

   const std::vector<std::string> &getAvailableSymbolNames() const;
   const QImage &getSymbolImage(const std::string &symbol) const;
   
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
};

#endif   // DESKTOPSERVICESIMP_H
