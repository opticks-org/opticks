/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef APPLICATIONWINDOW_H
#define APPLICATIONWINDOW_H

#include <QtCore/QList>
#include <QtGui/QAction>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QUndoGroup>

#include "ConfigurationSettings.h"
#include "DynamicObject.h"
#include "EnumWrapper.h"
#include "GraphicGroupAdapter.h"
#include "ImportAgent.h"
#include "SafePtr.h"
#include "SettableSessionItemAdapter.h"
#include "SubjectAdapter.h"
#include "TypesFile.h"
#include "View.h"

#include <boost/any.hpp>
#include <vector>

class AnimationToolBarAdapter;
class AnnotationLayer;
class AnnotationToolBar;
class AoiToolBarAdapter;
class BackgroundPluginWindow;
class BrightnessToolBar;
class DataElement;
class DisplayToolBar;
class DockWindow;
class FileDescriptor;
class GcpEditorDlg;
class GcpToolBar;
class HistogramWindowAdapter;
class Layer;
class MeasurementToolBar;
class MenuBarImp;
class MessageLogWindow;
class MouseMode;
class MruFile;
class PerspectiveView;
class PlotWidget;
class PlugInDescriptor;
class ProductWindow;
class Progress;
class ScriptingWindow;
class SessionExplorerAdapter;
class SessionItem;
class Signature;
class Subject;
class StatusBar;
class TiePointEditor;
class TiePointToolBar;
class ToolBarAdapter;
class Window;
class Workspace;
class WorkspaceWindow;
class WorkspaceWindowImp;

class QMdiSubWindow;

class ApplicationWindow : public QMainWindow, public SubjectAdapter, public SettableSessionItemAdapter
{
   Q_OBJECT

public:
   ApplicationWindow(QWidget* pSplash = NULL);
   virtual ~ApplicationWindow();

   void setSessionFilename(const std::string& sessionFilename);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SETTING(Configuration, ApplicationWindow, std::string, "")
   SETTING(Geometry, ApplicationWindow, std::string, "")
   SETTING_PTR(Shortcuts, ApplicationWindow, DynamicObject)

   SIGNAL_METHOD(ApplicationWindow, Closed)
   SIGNAL_METHOD(ApplicationWindow, WindowAdded)
   SIGNAL_METHOD(ApplicationWindow, WindowRemoved)
   SIGNAL_METHOD(ApplicationWindow, WindowActivated)
   SIGNAL_METHOD(ApplicationWindow, AboutToShowPropertiesDialog)

   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& value);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

   // Windows
   Window* createWindow(const QString& strWindowName, WindowType windowType);
   bool addWindow(Window* pWindow);
   Window* getWindow(const QString& strWindowName, WindowType windowType) const;
   Window* getWindow(const std::string& uniqueId) const;
   std::vector<Window*> getWindows(WindowType windowType) const;
   std::vector<Window*> getWindows(const QString& strWindowType) const;
   std::vector<Window*> getWindows() const;
   bool containsWindow(Window* pWindow) const;
   unsigned int getNumWindows(WindowType windowType) const;
   unsigned int getNumWindows(const QString& strWindowType) const;
   unsigned int getNumWindows() const;
   bool removeWindow(Window* pWindow);
   bool deleteWindow(Window* pWindow);

   // Workspace windows
   bool setCurrentWorkspaceWindow(WorkspaceWindow* pWindow);
   WorkspaceWindow* getCurrentWorkspaceWindow() const;
   QString getCurrentWorkspaceWindowName() const;
   View* getCurrentWorkspaceWindowView() const;
   void tileWorkspaceWindows(TilingType eType = TILE_GRID);
   bool tileWorkspaceWindows(const std::vector<WorkspaceWindow*>& windowList, 
                             bool maxFirst = true, 
                             TilingType eType = TILE_GRID);

   // Modes
   void setPanMode(PanModeType panMode);
   PanModeType getPanMode() const;

   void setAnnotationObject(GraphicObjectType objectType);
   GraphicObjectType getAnnotationObject() const;

   // Application Window management
   void saveConfiguration() const;
   void restoreConfiguration();
   QSize sizeHint() const;
   QSizePolicy sizePolicy() const;

   void setAutoSaveTimerEnabled(bool enabled);
   void setAutoSaveTimerInterval(unsigned int interval); // interval is a number of minutes in the range [1,1440].
   void optionsModified(Subject &subject, const std::string &signal, const boost::any &value);

public slots:
   // File actions
   void openSession(const QString& filename);
   bool importFile(const QString& strImporterSubtype = QString(), Progress* pProgress = NULL);
   bool importFile(const QString& strImporterSubtype, Progress* pProgress, std::vector<DataElement*>& importedElements);
   bool exportSessionItem(SessionItem *pItem, FileDescriptor *pNewFileDescriptor = NULL, Progress *pProgress = NULL);
   bool exportSessionItems(const std::vector<SessionItem*>& items, Progress* pProgress = NULL);
   void runWizard(QString strWizard);

   // Edit actions
   void clearUndoStacks();

   // View actions
   void createDataSetAnimation();
   void addMouseModeToGroup(const MouseMode* pMouseMode);
   ProductWindow* deriveProduct(View *pView=NULL);

   // Tools actions
   void registerPlugIns();

   // Window actions
   void cascadeWorkspaceWindows();
   void tileSelectedWindows();
   void arrangeWorkspaceWindows(QAction* pAction);

   // Menu bars
   void updatePlugInCommands();
   void updateWizardCommands();

   // Timer action
   void pregenerateTexture();

   // Properties
   void displayProperties(SessionItem* pItem);
   void displayProperties(SessionItem* pItem, const std::vector<std::string>& displayedPages, bool bEmitSignal = true);

protected:
   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   void sessionAboutToRestore(Subject& subject, const std::string& signal, const boost::any& value);
   void sessionLoaded(Subject& subject, const std::string& signal, const boost::any& value);

   QTimer *mpSaveTimer;
   bool mAutoTimerRetryOnLock;

   // Application Window management
   bool eventFilter(QObject* o, QEvent* e);
   void closeEvent(QCloseEvent* e);
   void enableActions(bool bEnable);
   void enableToolBars(bool bEnable);

protected slots:
   // File actions
   void closeWorkspaceWindow();
   void printSetup();
   void print();

   // Session actions
   void openSession();
   bool newSession();
   void autoSaveSession();
   bool saveSession();
   bool saveSessionAs();

   // Edit actions
   void cut();
   void copy();
   void paste();

   // Status Bar action
   void showStatusBar(bool bShow);

   // View actions
   void setMouseMode(QAction* pAction);
   void setMouseMode(View* pView, const QString& strMouseMode);
   void updateMouseAction(const MouseMode* pMouseMode);
   void refresh();
   void resetStretch();
   void generateFullImage();
   void toggleDisplayMode();
   void clearMarkings();
   void forwardSnapshot();

   // Rotate actions
   void rotateLeft();
   void rotateRight();
   void flipHoriz();
   void flipVert();
   void rotateBy();
   void northUp();
   void sensorUp();
   void reset();

   // Zoom actions
   void zoomIn();
   void zoomOut();
   void zoom10();
   void zoom50();
   void zoom75();
   void zoom100();
   void zoom150();
   void zoom200();
   void zoom400();
   void zoom800();
   void zoomToFit();
   void zoomTo(double dZoom);

   // Properties actions
   void viewProperties();

   // Layout actions
   ProductWindow* newProduct();
   void loadTemplate();
   void saveTemplate();
   void editViewObject(bool bEdit);
   void setPaperSize();

   // Tools actions
   void showOverviewWindow(bool bShow);
   void showChippingWindow();
   void showGcpEditor(bool bShow);
   void showTiePointEditor(bool bShow);
   void showWizardBuilder();
   void showZapDlg();
   void showBatchEditor();

   // Options actions
   void invokeOptionsDlg();

   // Window actions
   void linkWindows();
   void linkAllSpatialDataWindows();

   // Help actions
   void helpTopics();
   void displayExtensions();
   void displayOpticksSite();
   void displayExtensionsSite();
   void displayMailingListsSite();
   void displayRealTimeChatSite();
   void displaySampleDataSite();
   void aboutApp();
   void displayPlugInHelp();

   // Menu bars
   void executeCommand(QAction* pAction);

   // Application Window management
   void updateActiveSubWindow(QMdiSubWindow* pWindow);
   void initializeToolBars(Layer* pLayer);

   // SessionItem context menu actions
   void linkSelectedViews();
   void unlinkSelectedViews();
   void unlinkSelectedView();
   void deleteSelectedElement();
   void editClassification();
   void exportSessionItem();
   void displaySessionItemProperties();

private:
   ApplicationWindow(const ApplicationWindow& rhs);
   ApplicationWindow& operator=(const ApplicationWindow& rhs);

   // Main menu bar menus
   QMenu* m_pFile;
   QMenu* m_pSession;
   QMenu* m_pEdit;
   QMenu* m_pView;
   QMenu* mpToolbarsMenu;
   QMenu* m_pLayout;
   QMenu* m_pTools;
   QMenu* m_pWindow;
   QMenu* m_pHelp;

   // File actions
   QAction* m_pOpen_Action;
   QAction* m_pClose_Action;
   QAction* m_pPrint_Setup_Action;
   QAction* m_pPrint_Action;
   QAction* mpRecentFileAction;
   QAction* mpMruInsertAction;
   QAction *mpExportFileMenuAction;

   // Session actions
   QAction* m_pOpenSession_Action;
   QAction* m_pNewSession_Action;
   QAction* m_pSaveSession_Action;
   QAction* m_pSaveSessionAs_Action;

   // Edit actions
   QAction* m_pCut_Action;
   QAction* m_pCopy_Action;
   QAction* m_pPaste_Action;
   QAction* mpClearMarkingsAction;
   GraphicGroupAdapter mClipboard;

   // View actions
   QActionGroup* m_pView_Mode_Group;
   QAction* m_pNo_View_Mode_Action;
   QAction* mpLayerEditAction;
   QAction* mpCreateAnimationAction;
   QAction* m_pRefresh_Action;
   QAction* mpResetStretchAction;
   QAction* mpGenerateImageAction;
   QAction* m_pDisplay_Mode_Action;
   QAction* m_pMeasurement_Edit_Action;
   QAction* mpClipboardSizedAction;
   QAction* mpClipboardAction;

   // Pan actions
   QAction* m_pPan_Action;
   QActionGroup* m_pPan_Mode_Group;
   QAction* m_pPan_Delay_Action;
   QAction* m_pPan_Instant_Action;

   // Rotate actions
   QAction* m_pRotate_Left_Action;
   QAction* m_pRotate_Right_Action;
   QAction* m_pFlip_Horiz_Action;
   QAction* m_pFlip_Vert_Action;
   QAction* m_pRotate_By_Action;
   QAction* m_pFree_Rotate_Action;
   QAction* mpNorthUpAction;
   QAction* mpSensorUpAction;
   QAction* m_pReset_Action;

   // Zoom actions
   QAction* m_pZoom_In_Action;
   QAction* m_pZoom_Out_Action;
   QAction* m_pZoom_Point_In_Action;
   QAction* m_pZoom_Point_Out_Action;
   QAction* m_pZoom_Rect_Action;
   QAction* m_pZoom10_Action;
   QAction* m_pZoom50_Action;
   QAction* m_pZoom75_Action;
   QAction* m_pZoom100_Action;
   QAction* m_pZoom150_Action;
   QAction* m_pZoom200_Action;
   QAction* m_pZoom400_Action;
   QAction* m_pZoom800_Action;
   QAction* m_pZoom_To_Fit_Action;
   QAction* m_pZoom_And_Pan_To_Point_Action;

   // Layout actions
   QAction* mpDeriveProductAction;
   QAction* m_pLoadTemplate_Action;
   QAction* m_pSaveTemplate_Action;
   QAction* mpEditViewAction;
   QAction* m_pPaperSize_Action;

   // Tools actions
   QAction* mpSessionExplorerAction;
   QAction* m_pHistogram_Wnd_Action;
   QAction* m_pMessage_Log_Wnd_Action;
   QAction* m_pBackground_Plugins_Wnd_Action;
   QAction* m_pScripting_Wnd_Action;
   QAction* m_pOverview_Wnd_Action;
   QAction* m_pChipping_Wnd_Action;
   QAction* m_pGCP_Editor_Action;
   QAction* m_pTiePoint_Editor_Action;
   QAction* m_pLink_Action;
   QAction* mpLinkAllWindowsAction;

   // Window actions
   QAction* mpCascadeAction;
   QAction* mpTileHorizontalAction;
   QAction* mpTileVerticalAction;
   QAction* mpTileAction;
   QAction* mpTileSelectedAction;

   // SessionItem context menu actions
   QAction* mpUnlinkAction;
   QAction* mpExportContextMenuAction;
   QAction* mpPropertiesAction;

   // Menu bar
   MenuBarImp* mpMenuBar;
   QMap<QAction*, MruFile*> mMruFileCommands;
   QList<QAction*> mPlugInCommands;
   QList<QAction*> mWizardCommands;

   // Toolbars
   ToolBarAdapter* mpStandardToolBar;
   ToolBarAdapter* mpToolboxToolBar;
   DisplayToolBar* mpDisplayToolBar;
   AoiToolBarAdapter* mpAoiToolBar;
   AnnotationToolBar* mpAnnotationToolBar;
   GcpToolBar* mpGcpToolBar;
   TiePointToolBar* mpTiePointToolBar;
   BrightnessToolBar* mpBrightnessToolbar;
   MeasurementToolBar* mpMeasurementToolBar;
   AnimationToolBarAdapter *mpAnimationToolBar;

   // Workspace
   Workspace* mpWorkspace;
   std::vector<Window*> mWindows;
   WorkspaceWindowImp* mpCurrentWnd;
   SafePtr<View> mpCurrentView;
   View* mpCurrentEditView;

   // Docking windows
   SessionExplorerAdapter* mpSessionExplorer;
   HistogramWindowAdapter* mpHistogram;
   MessageLogWindow* m_pMessage_Log;
   BackgroundPluginWindow* m_pBackground_Plugins;
   ScriptingWindow* m_pScripting;

   // Status bar
   StatusBar* m_pStatus_Bar;

   // GCP Editor Dialog
   GcpEditorDlg* mpGcpEditor;

   // TiePoint Editor Dialog
   TiePointEditor* mpTiePointEditor;

   // Session
   std::string mSessionFilename;
   QSize mPreviousSize;

   // Undo
   QUndoGroup* mpUndoGroup;
   bool mClearingUndoStacks;

   // Drag and Drop File Import
   enum DropFilesTypeEnum
   {
      EXTENSION_FILE,
      SESSION_FILE,
      WIZARD_FILE,
      BATCH_WIZARD_FILE,
      DATASET_FILE
   };

   typedef EnumWrapper<DropFilesTypeEnum> DropFilesType;

   std::vector<std::string> mDropFiles;
   DropFilesType mDropFilesType;
   ImportAgent::EditType mDropEditType;
   bool mDropNewSession;

private:
   void updateActiveWindow(QMdiSubWindow* pWindow);
   bool isDefaultWindow(Window* pWindow) const;
   void checkColorDepth(QWidget* pSplash);
   void checkGpuImageSupport();
   void dragEnterEvent(QDragEnterEvent *pEvent);
   void dropEvent(QDropEvent *pEvent);
   void removeMenuCommands(const QList<QAction*>& commands);
   const std::vector<PlugInDescriptor*> &getAvailableExporters(const SessionItem *pItem) const;
   AnnotationLayer* getClipboardLayer() const;

private slots:
   void constructFileMenu();
   void fileMenuActivated(QAction* pAction);
   void exportFileMenu();
   void constructWindowMenu();
   void windowMenuActivated(QAction* pAction);
   void updateUndoActions(const std::string& oldId, const std::string& newId);
   void processDropFiles();
   void showToolbarsMenu();
};

#endif
