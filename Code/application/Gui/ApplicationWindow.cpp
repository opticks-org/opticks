/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtCore/QMap>
#include <QtCore/QRegExp>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QActionGroup>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QClipboard>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QSplashScreen>
#include <QtGui/QToolTip>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWhatsThis>

#include "AboutDlg.h"
#include "Aeb.h"
#include "AebIo.h"
#include "AnimationController.h"
#include "AnimationToolBarAdapter.h"
#include "AnnotationLayer.h"
#include "AnnotationLayerImp.h"
#include "AnnotationToolBar.h"
#include "AoiLayer.h"
#include "AoiToolBar.h"
#include "AppConfig.h"
#include "ApplicationServices.h"
#include "ApplicationWindow.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "BackgroundPluginWindow.h"
#include "BatchEditorDlg.h"
#include "BatchExportDlg.h"
#include "BrightnessToolBar.h"
#if defined(CG_SUPPORTED)
#include "CgContext.h"
#endif
#include "Classification.h"
#include "ClassificationWidget.h"
#include "ConfigurationSettingsImp.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DataDescriptorAdapter.h"
#include "DateTimeImp.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "DisplayToolBar.h"
#include "DockWindowAdapter.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "ExportDlg.h"
#include "ExtensionListDialog.h"
#include "FileDescriptor.h"
#include "FileDescriptorImp.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "FileResource.h"
#include "GcpEditorDlg.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "GcpToolBar.h"
#include "GeoAlgorithms.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "GraphicObjectImp.h"
#include "HistogramWindowAdapter.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "InstallerServices.h"
#include "InstallWizard.h"
#include "LatLonLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "LinkDlg.h"
#include "LocationType.h"
#include "MeasurementToolBar.h"
#include "MenuBarImp.h"
#include "MessageLogResource.h"
#include "MessageLogWindow.h"
#include "ModelServices.h"
#include "MouseModeImp.h"
#include "MruFile.h"
#include "ObjectResource.h"
#include "OptionsDlg.h"
#include "PaperSizeDlg.h"
#include "PlotWindowAdapter.h"
#include "PlugIn.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInResource.h"
#include "ProductViewAdapter.h"
#include "ProductWindowAdapter.h"
#include "ProgressAdapter.h"
#include "ProgressDlg.h"
#include "ProgressResource.h"
#include "PropertiesDlg.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayerAdapter.h"
#include "RasterUtilities.h"
#include "ScriptingWindow.h"
#include "SessionExplorerAdapter.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "SessionManagerImp.h"
#include "Signature.h"
#include "SignatureSet.h"
#include "SpatialDataViewAdapter.h"
#include "SpatialDataWindowAdapter.h"
#include "SpecialMetadata.h"
#include "StatusBar.h"
#include "TiePointEditor.h"
#include "TiePointLayer.h"
#include "TiePointToolbar.h"
#include "ToolBarAdapter.h"
#include "Undo.h"
#include "UndoButton.h"
#include "UndoStack.h"
#include "UtilityServicesImp.h"
#include "ViewObjectImp.h"
#include "WindowSelectionDlg.h"
#include "WizardBuilder.h"
#include "WizardObject.h"
#include "WizardUtilities.h"
#include "Workspace.h"
#include "WorkspaceWindowAdapter.h"
#include "xmlreader.h"
#include "xmlwriter.h"
#include "ZoomAndPanToPoint.h"

#if defined(WIN_API)
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#if defined(UNIX_API)
#include <sstream>
#endif
using namespace std;

using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

ApplicationWindow::ApplicationWindow(QWidget* pSplash) :
   QMainWindow(NULL),
   SettableSessionItemAdapter("{D21A6DE0-174B-4fd2-9886-963D8BB1FC48}"),
   mAutoTimerRetryOnLock(false),
   mClipboard(SessionItemImp::generateUniqueId(), GROUP_OBJECT, NULL, LocationType()),
   mpWorkspace(NULL),
   mpCurrentWnd(NULL),
   mpCurrentEditView(NULL),
   mpGcpEditor(NULL),
   mpUndoGroup(new QUndoGroup(this)),
   mClearingUndoStacks(false),
   mDropNewSession(false)
{
   // make sure we have enough colors
   checkColorDepth(pSplash);
   checkGpuImageSupport();

   ConfigurationSettingsImp* pConfSettings = dynamic_cast<ConfigurationSettingsImp*>(Service<ConfigurationSettings>().get());

   // Set the title bar caption
   QString strCaption = QString::fromStdString(pConfSettings->getProduct());

   setWindowTitle(strCaption);
   setAttribute(Qt::WA_QuitOnClose);

   ///////////////
   // Auto Save //
   ///////////////

   Service<ConfigurationSettings>()->attach(SIGNAL_NAME(ConfigurationSettings, SettingModified), Slot(this, &ApplicationWindow::optionsModified));
   mpSaveTimer = new QTimer(this);
   mpSaveTimer->setSingleShot(true);
   VERIFYNR(connect(mpSaveTimer, SIGNAL(timeout()), this, SLOT(autoSaveSession())));
   setAutoSaveTimerInterval(SessionManager::getSettingAutoSaveInterval());
   if (SessionManager::getSettingAutoSaveEnabled())
   {
      setAutoSaveTimerEnabled(true);
   }

   //////////////////
   // User Actions //
   //////////////////

   // File
   m_pOpen_Action = new QAction(QIcon(":/icons/Open"), "&Import Data...", this);
   m_pOpen_Action->setAutoRepeat(false);
   m_pOpen_Action->setShortcut(QKeySequence("Ctrl+O"));
   m_pOpen_Action->setToolTip("Import Data");
   m_pOpen_Action->setStatusTip("Imports an existing data file into the current session");
   VERIFYNR(connect(m_pOpen_Action, SIGNAL(triggered()), this, SLOT(importFile())));

   m_pClose_Action = new QAction(QIcon(":/icons/Close"), "&Close", this);
   m_pClose_Action->setAutoRepeat(false);
   m_pClose_Action->setToolTip("Close");
   m_pClose_Action->setStatusTip("Closes the currently active data set");
   VERIFYNR(connect(m_pClose_Action, SIGNAL(triggered()), this, SLOT(closeWorkspaceWindow())));

   m_pPrint_Setup_Action = new QAction(QIcon(":/icons/Print"), "&Print...", this);
   m_pPrint_Setup_Action->setAutoRepeat(false);
   m_pPrint_Setup_Action->setShortcut(QKeySequence::Print);
   m_pPrint_Setup_Action->setToolTip("Print Setup");
   m_pPrint_Setup_Action->setStatusTip("Invokes a dialog to set the print options for the "
      "currently active data set");
   VERIFYNR(connect(m_pPrint_Setup_Action, SIGNAL(triggered()), this, SLOT(printSetup())));

   m_pPrint_Action = new QAction(QIcon(":/icons/Print"), "P&rint", this);
   m_pPrint_Action->setAutoRepeat(false);
   m_pPrint_Action->setToolTip("Print");
   m_pPrint_Action->setStatusTip("Prints the currently active data set");
   VERIFYNR(connect(m_pPrint_Action, SIGNAL(triggered()), this, SLOT(print())));

   m_pNewSession_Action = new QAction(QIcon(":/icons/New"), "&New Session", this);
   m_pNewSession_Action->setAutoRepeat(false);
   m_pNewSession_Action->setToolTip("Create a new Session");
   m_pNewSession_Action->setStatusTip("Closes the current session and creates a new empty one");
   VERIFYNR(connect(m_pNewSession_Action, SIGNAL(triggered()), this, SLOT(newSession())));

   m_pOpenSession_Action = new QAction(QIcon(":/icons/Open"), "&Open Session...", this);
   m_pOpenSession_Action->setAutoRepeat(false);
   m_pOpenSession_Action->setToolTip("Open Session");
   m_pOpenSession_Action->setStatusTip("Opens an existing session");
   VERIFYNR(connect(m_pOpenSession_Action, SIGNAL(triggered()), this, SLOT(openSession())));

   m_pSaveSession_Action = new QAction(QIcon(":/icons/Save"), "&Save Session", this);
   m_pSaveSession_Action->setAutoRepeat(false);
   m_pSaveSession_Action->setToolTip("Save Session");
   m_pSaveSession_Action->setStatusTip("Saves the session");
   VERIFYNR(connect(m_pSaveSession_Action, SIGNAL(triggered()), this, SLOT(saveSession())));

   m_pSaveSessionAs_Action = new QAction("Save Session &As...", this);
   m_pSaveSessionAs_Action->setAutoRepeat(false);
   m_pSaveSessionAs_Action->setToolTip("Save Session As");
   m_pSaveSessionAs_Action->setStatusTip("Saves the open session to a new file");
   VERIFYNR(connect(m_pSaveSessionAs_Action, SIGNAL(triggered()), this, SLOT(saveSessionAs())));

   mpExportFileMenuAction = new QAction(QIcon(":/icons/Save"), "&Export...", this);
   mpExportFileMenuAction->setAutoRepeat(false);
   mpExportFileMenuAction->setToolTip("Export");
   mpExportFileMenuAction->setStatusTip("Exports the current view");
   VERIFYNR(connect(mpExportFileMenuAction, SIGNAL(triggered()), this, SLOT(exportFileMenu())));

   // Edit
   UndoButton* pUndoButton = new UndoButton(true, mpUndoGroup, this);
   UndoButton* pRedoButton = new UndoButton(false, mpUndoGroup, this);

   m_pCut_Action = new QAction(QIcon(":/icons/Cut"), "Cu&t", this);
   m_pCut_Action->setAutoRepeat(false);
   m_pCut_Action->setShortcut(QKeySequence::Cut);
   m_pCut_Action->setToolTip("Cut");
   m_pCut_Action->setStatusTip("Cuts the current selection and moves it to the clipboard");
   VERIFYNR(connect(m_pCut_Action, SIGNAL(triggered()), this, SLOT(cut())));

   m_pCopy_Action = new QAction(QIcon(":/icons/Copy"), "&Copy", this);
   m_pCopy_Action->setAutoRepeat(false);
   m_pCopy_Action->setShortcut(QKeySequence::Copy);
   m_pCopy_Action->setToolTip("Copy");
   m_pCopy_Action->setStatusTip("Copies the current selection to the clipboard");
   VERIFYNR(connect(m_pCopy_Action, SIGNAL(triggered()), this, SLOT(copy())));

   m_pPaste_Action = new QAction(QIcon(":/icons/Paste"), "&Paste", this);
   m_pPaste_Action->setAutoRepeat(false);
   m_pPaste_Action->setShortcut(QKeySequence::Paste);
   m_pPaste_Action->setToolTip("Paste");
   m_pPaste_Action->setStatusTip("Inserts the clipboard contents at the current cursor location");
   VERIFYNR(connect(m_pPaste_Action, SIGNAL(triggered()), this, SLOT(paste())));

   // Status Bar
   QAction* pStatus_Bar_Action = new QAction("&Status Bar", this);
   pStatus_Bar_Action->setAutoRepeat(false);
   pStatus_Bar_Action->setCheckable(true);
   pStatus_Bar_Action->setToolTip("Status Bar");
   pStatus_Bar_Action->setStatusTip("Toggles the display of the application status bar");
   VERIFYNR(connect(pStatus_Bar_Action, SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool))));

   // View
   m_pView_Mode_Group = new QActionGroup(this);
   m_pView_Mode_Group->setExclusive(true);
   VERIFYNR(connect(m_pView_Mode_Group, SIGNAL(triggered(QAction*)), this, SLOT(setMouseMode(QAction*))));

   m_pNo_View_Mode_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/Edit"), "&Edit Mode Off");
   m_pNo_View_Mode_Action->setAutoRepeat(false);
   m_pNo_View_Mode_Action->setCheckable(true);
   m_pNo_View_Mode_Action->setToolTip("Edit Mode Off");
   m_pNo_View_Mode_Action->setStatusTip("Disables all mouse modes");

   mpLayerEditAction = m_pView_Mode_Group->addAction(QIcon(":/icons/Layers"), "&Layer Mode");
   mpLayerEditAction->setAutoRepeat(false);
   mpLayerEditAction->setCheckable(true);
   mpLayerEditAction->setToolTip("Layer Edit Mode");
   mpLayerEditAction->setStatusTip("Toggles the edit mode for the active layer");

   m_pMeasurement_Edit_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/MeasurementMarker"), "&Measurement Mode");
   m_pMeasurement_Edit_Action->setAutoRepeat(false);
   m_pMeasurement_Edit_Action->setCheckable(true);
   m_pMeasurement_Edit_Action->setToolTip("Measurement Mode");
   m_pMeasurement_Edit_Action->setStatusTip("Toggles the measurement edit mode");

   mpCreateAnimationAction = new QAction("Create &Animation", this);
   mpCreateAnimationAction->setAutoRepeat(false);
   mpCreateAnimationAction->setToolTip("Create Animation");
   mpCreateAnimationAction->setStatusTip("Creates an animation in the animation window for the current data set");
   VERIFYNR(connect(mpCreateAnimationAction, SIGNAL(triggered()), this, SLOT(createDataSetAnimation())));

   m_pRefresh_Action = new QAction(QIcon(":/icons/Refresh"), "&Refresh", this);
   m_pRefresh_Action->setAutoRepeat(false);
   m_pRefresh_Action->setShortcut(QKeySequence::Refresh);
   m_pRefresh_Action->setToolTip("Refresh");
   m_pRefresh_Action->setStatusTip("Redraws the data in the active spatial data window");
   VERIFYNR(connect(m_pRefresh_Action, SIGNAL(triggered()), this, SLOT(refresh())));

   mpResetStretchAction = new QAction(QIcon(":/icons/ResetStretch"), "R&eset", this);
   mpResetStretchAction->setAutoRepeat(false);
   mpResetStretchAction->setShortcut(QKeySequence("Ctrl+E"));
   mpResetStretchAction->setStatusTip("Reset all stretch changes for visible layers in the active spatial data window");
   VERIFYNR(connect(mpResetStretchAction, SIGNAL(triggered()), this, SLOT(resetStretch())));

   m_pDisplay_Mode_Action = new QAction(QIcon(":/icons/DisplayMode"), "Toggle Display Mode", this);
   m_pDisplay_Mode_Action->setAutoRepeat(false);
   m_pDisplay_Mode_Action->setToolTip("Toggle Display Mode");
   m_pDisplay_Mode_Action->setStatusTip("Toggles the spatial data display between grayscale and RGB modes");
   VERIFYNR(connect(m_pDisplay_Mode_Action, SIGNAL(triggered()), this, SLOT(toggleDisplayMode())));

   mpGenerateImageAction = new QAction(QIcon(":/icons/Generate"), "&Generate Full Image", this);
   mpGenerateImageAction->setAutoRepeat(false);
   mpGenerateImageAction->setToolTip("Generate Full Image");
   mpGenerateImageAction->setStatusTip("Prepares the full image for rendering");
   VERIFYNR(connect(mpGenerateImageAction, SIGNAL(triggered()), this, SLOT(generateFullImage())));

   mpClearMarkingsAction = new QAction("Clear Markings", this);
   mpClearMarkingsAction->setAutoRepeat(false);
   mpClearMarkingsAction->setToolTip("Clear Markings");
   mpClearMarkingsAction->setStatusTip("Clears all markings from the current view");
   VERIFYNR(connect(mpClearMarkingsAction, SIGNAL(triggered()), this, SLOT(clearMarkings())));

   // Pan
   m_pPan_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/Pan"), "Pa&n");
   m_pPan_Action->setAutoRepeat(false);
   m_pPan_Action->setCheckable(true);
   m_pPan_Action->setToolTip("Pan");
   m_pPan_Action->setStatusTip("Shifts the display while clicking and dragging the mouse within the active view");

   m_pPan_Mode_Group = new QActionGroup(this);
   m_pPan_Mode_Group->setExclusive(true);

   m_pPan_Delay_Action = m_pPan_Mode_Group->addAction("Pan &Delay");
   m_pPan_Delay_Action->setAutoRepeat(false);
   m_pPan_Delay_Action->setCheckable(true);
   m_pPan_Delay_Action->setToolTip("Pan Delay");
   m_pPan_Delay_Action->setStatusTip("Sets the pan mode to update the display after panning is complete");

   m_pPan_Instant_Action = m_pPan_Mode_Group->addAction("Pan &Instant");
   m_pPan_Instant_Action->setAutoRepeat(false);
   m_pPan_Instant_Action->setCheckable(true);
   m_pPan_Instant_Action->setToolTip("Pan Instant");
   m_pPan_Instant_Action->setStatusTip("Sets the pan mode to continuously update the display while panning");
   m_pPan_Instant_Action->setChecked(true);

   // Rotate
   m_pRotate_Left_Action = new QAction(QIcon(":/icons/RotateLeft"), "Rotate &Left 90 Degrees", this);
   m_pRotate_Left_Action->setToolTip("Rotate Left 90 Degrees");
   m_pRotate_Left_Action->setStatusTip("Rotates the current data set 90 degrees counterclockwise");
   VERIFYNR(connect(m_pRotate_Left_Action, SIGNAL(triggered()), this, SLOT(rotateLeft())));

   m_pRotate_Right_Action = new QAction(QIcon(":/icons/RotateRight"), "Rotate &Right 90 Degrees", this);
   m_pRotate_Right_Action->setToolTip("Rotate Right 90 Degrees");
   m_pRotate_Right_Action->setStatusTip("Rotates the current data set 90 degrees clockwise");
   VERIFYNR(connect(m_pRotate_Right_Action, SIGNAL(triggered()), this, SLOT(rotateRight())));

   m_pFlip_Horiz_Action = new QAction(QIcon(":/icons/FlipHorizontally"), "Flip &Horizontally", this);
   m_pFlip_Horiz_Action->setAutoRepeat(false);
   m_pFlip_Horiz_Action->setShortcut(QKeySequence(Qt::Key_H));
   m_pFlip_Horiz_Action->setToolTip("Flip Horizontally");
   m_pFlip_Horiz_Action->setStatusTip("Flips the data set from left to right");
   VERIFYNR(connect(m_pFlip_Horiz_Action, SIGNAL(triggered()), this, SLOT(flipHoriz())));

   m_pFlip_Vert_Action = new QAction(QIcon(":/icons/FlipVertically"), "Flip &Vertically", this);
   m_pFlip_Vert_Action->setAutoRepeat(false);
   m_pFlip_Vert_Action->setShortcut(QKeySequence(Qt::Key_V));
   m_pFlip_Vert_Action->setToolTip("Flip Vertically");
   m_pFlip_Vert_Action->setStatusTip("Flips the data set from top to bottom");
   VERIFYNR(connect(m_pFlip_Vert_Action, SIGNAL(triggered()), this, SLOT(flipVert())));

   m_pRotate_By_Action = new QAction(QIcon(":/icons/RotateBy"), "Rotate &By", this);
   m_pRotate_By_Action->setAutoRepeat(false);
   m_pRotate_By_Action->setToolTip("Rotate By");
   m_pRotate_By_Action->setStatusTip("Rotates the data set by a specified number of degrees");
   VERIFYNR(connect(m_pRotate_By_Action, SIGNAL(triggered()), this, SLOT(rotateBy())));

   m_pFree_Rotate_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/FreeRotate"), "&Free Rotate");
   m_pFree_Rotate_Action->setAutoRepeat(false);
   m_pFree_Rotate_Action->setCheckable(true);
   m_pFree_Rotate_Action->setToolTip("Free Rotate");
   m_pFree_Rotate_Action->setStatusTip("Rotates the data set while clicking and dragging the mouse");

   mpNorthUpAction = new QAction(QIcon(":/icons/NorthUp"), "North Up", this);
   mpNorthUpAction->setAutoRepeat(false);
   mpNorthUpAction->setShortcut(QKeySequence(Qt::Key_U));
   mpNorthUpAction->setToolTip("North Up");
   mpNorthUpAction->setStatusTip("Rotates the data set such that North is up");
   VERIFYNR(connect(mpNorthUpAction, SIGNAL(triggered()), this, SLOT(northUp())));

   mpSensorUpAction = new QAction(QIcon(":/icons/SensorUp"), "Sensor Up", this);
   mpSensorUpAction->setAutoRepeat(false);
   mpSensorUpAction->setShortcut(QKeySequence(Qt::Key_S));
   mpSensorUpAction->setToolTip("Sensor Up");
   mpSensorUpAction->setStatusTip("Rotates the data set such that the sensor flight line is up");
   VERIFYNR(connect(mpSensorUpAction, SIGNAL(triggered()), this, SLOT(sensorUp())));

   m_pReset_Action = new QAction(QIcon(":/icons/ResetOrientation"), "R&eset", this);
   m_pReset_Action->setAutoRepeat(false);
   m_pReset_Action->setShortcut(QKeySequence(Qt::Key_W));
   m_pReset_Action->setToolTip("Reset");
   m_pReset_Action->setStatusTip("Returns the data set to its original orientation");
   VERIFYNR(connect(m_pReset_Action, SIGNAL(triggered()), this, SLOT(reset())));

   // Zoom
   m_pZoom_In_Action = new QAction(QIcon(":/icons/ZoomIn"), "&In", this);
   m_pZoom_In_Action->setShortcut(QKeySequence(Qt::Key_Z));
   m_pZoom_In_Action->setToolTip("Zoom In");
   m_pZoom_In_Action->setStatusTip("Increases the zoom level in the active view about the window center");
   VERIFYNR(connect(m_pZoom_In_Action, SIGNAL(triggered()), this, SLOT(zoomIn())));

   m_pZoom_Out_Action = new QAction(QIcon(":/icons/ZoomOut"), "&Out", this);
   m_pZoom_Out_Action->setShortcut(QKeySequence("Shift+Z"));
   m_pZoom_Out_Action->setToolTip("Zoom Out");
   m_pZoom_Out_Action->setStatusTip("Decreases the zoom level in the active view about the window center");
   VERIFYNR(connect(m_pZoom_Out_Action, SIGNAL(triggered()), this, SLOT(zoomOut())));

   m_pZoom_Point_In_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/ZoomPointIn"), "&In");
   m_pZoom_Point_In_Action->setAutoRepeat(false);
   m_pZoom_Point_In_Action->setCheckable(true);
   m_pZoom_Point_In_Action->setToolTip("Zoom In on Mouse");
   m_pZoom_Point_In_Action->setStatusTip("Increases the zoom level in the active view about a clicked point");

   m_pZoom_Point_Out_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/ZoomPointOut"), "&Out");
   m_pZoom_Point_Out_Action->setAutoRepeat(false);
   m_pZoom_Point_Out_Action->setCheckable(true);
   m_pZoom_Point_Out_Action->setToolTip("Zoom Out on Mouse");
   m_pZoom_Point_Out_Action->setStatusTip("Decreases the zoom level in the active view about a clicked point");

   m_pZoom10_Action = new QAction("10%", this);
   m_pZoom10_Action->setAutoRepeat(false);
   m_pZoom10_Action->setToolTip("Zoom to 10%");
   m_pZoom10_Action->setStatusTip("Zooms the active view to 10% of the data extents");
   VERIFYNR(connect(m_pZoom10_Action, SIGNAL(triggered()), this, SLOT(zoom10())));

   m_pZoom50_Action = new QAction("50%", this);
   m_pZoom50_Action->setAutoRepeat(false);
   m_pZoom50_Action->setToolTip("Zoom to 50%");
   m_pZoom50_Action->setStatusTip("Zooms the active view to 50% of the data extents");
   VERIFYNR(connect(m_pZoom50_Action, SIGNAL(triggered()), this, SLOT(zoom50())));

   m_pZoom75_Action = new QAction("75%", this);
   m_pZoom75_Action->setAutoRepeat(false);
   m_pZoom75_Action->setToolTip("Zoom to 75%");
   m_pZoom75_Action->setStatusTip("Zooms the active view to 75% of the data extents");
   VERIFYNR(connect(m_pZoom75_Action, SIGNAL(triggered()), this, SLOT(zoom75())));

   m_pZoom100_Action = new QAction("100%", this);
   m_pZoom100_Action->setAutoRepeat(false);
   m_pZoom100_Action->setToolTip("Zoom to 100%");
   m_pZoom100_Action->setStatusTip("Zooms the active view to 100% of the data extents");
   VERIFYNR(connect(m_pZoom100_Action, SIGNAL(triggered()), this, SLOT(zoom100())));

   m_pZoom150_Action = new QAction("150%", this);
   m_pZoom150_Action->setAutoRepeat(false);
   m_pZoom150_Action->setToolTip("Zoom to 150%");
   m_pZoom150_Action->setStatusTip("Zooms the active view to 150% of the data extents");
   VERIFYNR(connect(m_pZoom150_Action, SIGNAL(triggered()), this, SLOT(zoom150())));

   m_pZoom200_Action = new QAction("200%", this);
   m_pZoom200_Action->setAutoRepeat(false);
   m_pZoom200_Action->setToolTip("Zoom to 200%");
   m_pZoom200_Action->setStatusTip("Zooms the active view to 200% of the data extents");
   VERIFYNR(connect(m_pZoom200_Action, SIGNAL(triggered()), this, SLOT(zoom200())));

   m_pZoom400_Action = new QAction("400%", this);
   m_pZoom400_Action->setAutoRepeat(false);
   m_pZoom400_Action->setToolTip("Zoom to 400%");
   m_pZoom400_Action->setStatusTip("Zooms the active view to 400% of the data extents");
   VERIFYNR(connect(m_pZoom400_Action, SIGNAL(triggered()), this, SLOT(zoom400())));

   m_pZoom800_Action = new QAction("800%", this);
   m_pZoom800_Action->setAutoRepeat(false);
   m_pZoom800_Action->setToolTip("Zoom to 800%");
   m_pZoom800_Action->setStatusTip("Zooms the active view to 800% of the data extents");
   VERIFYNR(connect(m_pZoom800_Action, SIGNAL(triggered()), this, SLOT(zoom800())));

   m_pZoom_Rect_Action = m_pView_Mode_Group->addAction(QIcon(":/icons/ZoomRect"), "&Rectangle");
   m_pZoom_Rect_Action->setAutoRepeat(false);
   m_pZoom_Rect_Action->setCheckable(true);
   m_pZoom_Rect_Action->setToolTip("Zoom Rectangle");
   m_pZoom_Rect_Action->setStatusTip("Zooms the active view to an area defined with the mouse");

   m_pZoom_To_Fit_Action = new QAction(QIcon(":/icons/ZoomToFit"), "To &Fit", this);
   m_pZoom_To_Fit_Action->setAutoRepeat(false);
   m_pZoom_To_Fit_Action->setShortcut(QKeySequence(Qt::Key_E));
   m_pZoom_To_Fit_Action->setToolTip("Zoom to Fit");
   m_pZoom_To_Fit_Action->setStatusTip("Zooms the active view to the maximum extent of the data");
   VERIFYNR(connect(m_pZoom_To_Fit_Action, SIGNAL(triggered()), this, SLOT(zoomToFit())));

   m_pZoom_And_Pan_To_Point_Action = new QAction(QIcon(":/icons/ZoomAndPanToPoint"), "Zoom and Pan To Point", this);
   m_pZoom_And_Pan_To_Point_Action->setAutoRepeat(false);
   m_pZoom_And_Pan_To_Point_Action->setToolTip("Zoom and Pan to Point");
   m_pZoom_And_Pan_To_Point_Action->setStatusTip("Zooms the active view to the point and zoom level specified");
   VERIFYNR(connect(m_pZoom_And_Pan_To_Point_Action, SIGNAL(triggered()), this, SLOT(showZapDlg())));

   // Properties
   QAction* pPropertiesAction = new QAction(QIcon(":/icons/Properties"), "&Properties...", this);
   pPropertiesAction->setAutoRepeat(false);
   pPropertiesAction->setToolTip("Properties");
   pPropertiesAction->setStatusTip("Displays properties for application windows, plug-ins, and current "
      "cube and layer properties");
   VERIFYNR(connect(pPropertiesAction, SIGNAL(triggered()), this, SLOT(viewProperties())));

   // Layout actions
   QAction* pNewProductAction = new QAction("&New Product", this);
   pNewProductAction->setAutoRepeat(false);
   pNewProductAction->setToolTip("New Product");
   pNewProductAction->setStatusTip("Creates a new product window and view applying the default template - "
      "all view objects are uninitialized");
   VERIFYNR(connect(pNewProductAction, SIGNAL(triggered()), this, SLOT(newProduct())));

   mpDeriveProductAction = new QAction("&Derive Product", this);
   mpDeriveProductAction->setAutoRepeat(false);
   mpDeriveProductAction->setToolTip("Derive Product");
   mpDeriveProductAction->setStatusTip("Creates a new product window and view applying the default template - "
      "all view objects are initialized to the active view");
   VERIFYNR(connect(mpDeriveProductAction, SIGNAL(triggered()), this, SLOT(deriveProduct())));

   m_pLoadTemplate_Action = new QAction("&Load Template...", this);
   m_pLoadTemplate_Action->setAutoRepeat(false);
   m_pLoadTemplate_Action->setToolTip("Load Template");
   m_pLoadTemplate_Action->setStatusTip("Loads and applies a product template");
   VERIFYNR(connect(m_pLoadTemplate_Action, SIGNAL(triggered()), this, SLOT(loadTemplate())));

   m_pSaveTemplate_Action = new QAction("&Save Template...", this);
   m_pSaveTemplate_Action->setAutoRepeat(false);
   m_pSaveTemplate_Action->setToolTip("Save Template");
   m_pSaveTemplate_Action->setStatusTip("Saves the current layout as a product template");
   VERIFYNR(connect(m_pSaveTemplate_Action, SIGNAL(triggered()), this, SLOT(saveTemplate())));

   mpEditViewAction = new QAction("&Edit Selected View", this);
   mpEditViewAction->setAutoRepeat(false);
   mpEditViewAction->setCheckable(true);
   mpEditViewAction->setToolTip("Edit Selected View");
   mpEditViewAction->setStatusTip("Enables or disables direct editing of the selected view object");
   VERIFYNR(connect(mpEditViewAction, SIGNAL(toggled(bool)), this, SLOT(editViewObject(bool))));

   m_pPaperSize_Action = new QAction("Pape&r Size...", this);
   m_pPaperSize_Action->setAutoRepeat(false);
   m_pPaperSize_Action->setToolTip("Paper Size");
   m_pPaperSize_Action->setStatusTip("Sets the paper size of the current product");
   VERIFYNR(connect(m_pPaperSize_Action, SIGNAL(triggered()), this, SLOT(setPaperSize())));

   // Tools
   mpSessionExplorerAction = new QAction(QIcon(":/icons/SessionExplorer"), "Session E&xplorer", this);
   mpSessionExplorerAction->setAutoRepeat(false);
   mpSessionExplorerAction->setCheckable(true);
   mpSessionExplorerAction->setToolTip("Session Explorer");
   mpSessionExplorerAction->setStatusTip("Toggles the display of the Session Explorer");

   m_pHistogram_Wnd_Action = new QAction(QIcon(":/icons/HistogramWindow"), "&Histogram Window", this);
   m_pHistogram_Wnd_Action->setAutoRepeat(false);
   m_pHistogram_Wnd_Action->setCheckable(true);
   m_pHistogram_Wnd_Action->setToolTip("Histogram Window");
   m_pHistogram_Wnd_Action->setStatusTip("Toggles the display of the Histogram Window");

   m_pMessage_Log_Wnd_Action = new QAction(QIcon(":/icons/MessageLogWindow"), "&Message Log Window", this);
   m_pMessage_Log_Wnd_Action->setAutoRepeat(false);
   m_pMessage_Log_Wnd_Action->setCheckable(true);
   m_pMessage_Log_Wnd_Action->setToolTip("Message Log Window");
   m_pMessage_Log_Wnd_Action->setStatusTip("Toggles the display of the Message Log Window");

   m_pBackground_Plugins_Wnd_Action = new QAction(QIcon(":/icons/BackgroundTask"), "&Background Plugins Window", this);
   m_pBackground_Plugins_Wnd_Action->setAutoRepeat(false);
   m_pBackground_Plugins_Wnd_Action->setCheckable(true);
   m_pBackground_Plugins_Wnd_Action->setToolTip("Background Plug-Ins Window");
   m_pBackground_Plugins_Wnd_Action->setStatusTip("Toggles the display of the Background Plugin Window");

   m_pScripting_Wnd_Action = new QAction(QIcon(":/icons/Script"), "S&cripting Window", this);
   m_pScripting_Wnd_Action->setAutoRepeat(false);
   m_pScripting_Wnd_Action->setCheckable(true);
   m_pScripting_Wnd_Action->setToolTip("Scripting Window");
   m_pScripting_Wnd_Action->setStatusTip("Toggles the display of the Scripting Window");

   m_pOverview_Wnd_Action = new QAction(QIcon(":/icons/OverviewWindow"), "O&verview Window", this);
   m_pOverview_Wnd_Action->setAutoRepeat(false);
   m_pOverview_Wnd_Action->setCheckable(true);
   m_pOverview_Wnd_Action->setToolTip("Overview Window");
   m_pOverview_Wnd_Action->setStatusTip("Displays a small thumbnail of the active view");
   VERIFYNR(connect(m_pOverview_Wnd_Action, SIGNAL(toggled(bool)), this, SLOT(showOverviewWindow(bool))));

   m_pChipping_Wnd_Action = new QAction(QIcon(":/icons/ChipImage"), "Image Chipping Window", this);
   m_pChipping_Wnd_Action->setAutoRepeat(false);
   m_pChipping_Wnd_Action->setToolTip("Image Chipping Window");
   m_pChipping_Wnd_Action->setStatusTip("Opens a thumbnail view of the active view for chipping a new image");
   VERIFYNR(connect(m_pChipping_Wnd_Action, SIGNAL(triggered()), this, SLOT(showChippingWindow())));

   m_pGCP_Editor_Action = new QAction(QIcon(":/icons/GcpEditor"), "&GCP Editor...", this);
   m_pGCP_Editor_Action->setAutoRepeat(false);
   m_pGCP_Editor_Action->setCheckable(true);
   m_pGCP_Editor_Action->setToolTip("GCP Editor");
   m_pGCP_Editor_Action->setStatusTip("Configures the GCPs in the current GCP list");
   VERIFYNR(connect(m_pGCP_Editor_Action, SIGNAL(toggled(bool)), this, SLOT(showGcpEditor(bool))));

   m_pTiePoint_Editor_Action = new QAction(QIcon(":/icons/TiePointEditor"), "&Tie Point Editor...", this);
   m_pTiePoint_Editor_Action->setAutoRepeat(false);
   m_pTiePoint_Editor_Action->setCheckable(true);
   m_pTiePoint_Editor_Action->setToolTip("Tie Point Editor");
   m_pTiePoint_Editor_Action->setStatusTip("Configures the Tie Points in the current Tie Point list");
   VERIFYNR(connect(m_pTiePoint_Editor_Action, SIGNAL(toggled(bool)), this, SLOT(showTiePointEditor(bool))));

   QAction* pWizard_Builder_Action = new QAction(QIcon(":/icons/WizardBuilder"), "Wi&zard Builder...", this);
   pWizard_Builder_Action->setAutoRepeat(false);
   pWizard_Builder_Action->setToolTip("Wizard Builder");
   pWizard_Builder_Action->setStatusTip("Invokes an editor to create and edit custom wizards");
   VERIFYNR(connect(pWizard_Builder_Action, SIGNAL(triggered()), this, SLOT(showWizardBuilder())));

   QAction* pBatch_Wizard_Editor_Action = new QAction("Batch Wizard Editor...", this);
   pBatch_Wizard_Editor_Action->setAutoRepeat(false);
   pBatch_Wizard_Editor_Action->setToolTip("Batch Wizard Editor");
   pBatch_Wizard_Editor_Action->setStatusTip("Invokes an editor to edit batch wizards");
   VERIFYNR(connect(pBatch_Wizard_Editor_Action, SIGNAL(triggered()), this, SLOT(showBatchEditor())));

   m_pLink_Action = new QAction(QIcon(":/icons/Link"), "Lin&k/Unlink...", this);
   m_pLink_Action->setAutoRepeat(false);
   m_pLink_Action->setToolTip("Link/Unlink");
   m_pLink_Action->setStatusTip("Connects layer and elements across multiple spatial data windows");
   VERIFYNR(connect(m_pLink_Action, SIGNAL(triggered()), this, SLOT(linkWindows())));

   QAction* pUpdate_Wizards_Action = new QAction("Update &Wizard List", this);
   pUpdate_Wizards_Action->setAutoRepeat(false);
   pUpdate_Wizards_Action->setToolTip("Update Wizard List");
   pUpdate_Wizards_Action->setStatusTip("Checks for new wizards in the wizard directory");
   VERIFYNR(connect(pUpdate_Wizards_Action, SIGNAL(triggered()), this, SLOT(updateWizardCommands())));

   // Options
   QAction* pOptions_Action = new QAction("&Options...", this);
   pOptions_Action->setAutoRepeat(false);
   pOptions_Action->setToolTip("Options");
   pOptions_Action->setStatusTip(QString("Changes the interface settings for the current %1 session").arg(APP_NAME));
   VERIFYNR(connect(pOptions_Action, SIGNAL(triggered()), this, SLOT(invokeOptionsDlg())));

   // Window
   mpCascadeAction = new QAction(QIcon(":/icons/Cascade"), "&Cascade", this);
   mpCascadeAction->setAutoRepeat(false);
   mpCascadeAction->setToolTip("Cascade");
   mpCascadeAction->setStatusTip("Arranges windows in an overlapping fashion");
   VERIFYNR(connect(mpCascadeAction, SIGNAL(triggered()), this, SLOT(cascadeWorkspaceWindows())));

   QActionGroup* pWinGroup = new QActionGroup(this);
   VERIFYNR(connect(pWinGroup, SIGNAL(triggered(QAction*)), this, SLOT(arrangeWorkspaceWindows(QAction*))));

   mpTileAction = pWinGroup->addAction(QIcon(":/icons/Tile"), "Ti&le");
   mpTileAction->setAutoRepeat(false);
   mpTileAction->setToolTip("Tile");
   mpTileAction->setStatusTip("Arranges windows as non-overlapping tiles");

   mpTileHorizontalAction = pWinGroup->addAction(QIcon(":/icons/TileHoriz"), "Tile Horizontally");
   mpTileHorizontalAction->setAutoRepeat(false);
   mpTileHorizontalAction->setToolTip("Tile horizontally");
   mpTileHorizontalAction->setStatusTip("Arranges windows as non-overlapping vertical tiles");

   mpTileVerticalAction = pWinGroup->addAction(QIcon(":/icons/TileVert"), "Tile Vertically");
   mpTileVerticalAction->setAutoRepeat(false);
   mpTileVerticalAction->setToolTip("Tile vertically");
   mpTileVerticalAction->setStatusTip("Arranges windows as non-overlapping horizontal tiles");

   mpTileSelectedAction = new QAction("Tile Custom ...", this);
   mpTileSelectedAction->setAutoRepeat(false);
   mpTileSelectedAction->setToolTip("Tile Selected");
   mpTileSelectedAction->setStatusTip("Select which Windows to tile and how to display");
   VERIFYNR(connect(mpTileSelectedAction, SIGNAL(triggered()), this, SLOT(tileSelectedWindows())));

   // Help
   QAction* pHelp_Topics_Action = new QAction(QIcon(":/icons/HelpTopics"), "Application &Help...", this);
   pHelp_Topics_Action->setAutoRepeat(false);
   pHelp_Topics_Action->setToolTip("Application Help");
   pHelp_Topics_Action->setStatusTip(QString("Lists available help topics for the %1 application").arg(APP_NAME));
   VERIFYNR(connect(pHelp_Topics_Action, SIGNAL(triggered()), this, SLOT(helpTopics())));

   QAction* pExtensions_Action = new QAction(QString("&Extensions..."), this);
   pExtensions_Action->setAutoRepeat(false);
   pExtensions_Action->setToolTip(QString("Display Extensions"));
   pExtensions_Action->setStatusTip(QString("Display information about extensions and check for updates."));
   VERIFYNR(connect(pExtensions_Action, SIGNAL(triggered()), this, SLOT(displayExtensions())));

   QAction* pAbout_Action = new QAction(QIcon(":/icons/About"), QString("&About %1...").arg(APP_NAME), this);
   pAbout_Action->setAutoRepeat(false);
   pAbout_Action->setToolTip(QString("About %1").arg(APP_NAME));
   pAbout_Action->setStatusTip(QString("Displays %1 application, version number, and copyright information").arg(APP_NAME));
   VERIFYNR(connect(pAbout_Action, SIGNAL(triggered()), this, SLOT(aboutApp())));

   // SessionItem context menu
   mpExportContextMenuAction = new QAction(QIcon(":/icons/Save"), "Export", this);
   mpExportContextMenuAction->setAutoRepeat(false);
   mpExportContextMenuAction->setToolTip("Export");
   mpExportContextMenuAction->setStatusTip("Export this session item");
   VERIFYNR(connect(mpExportContextMenuAction, SIGNAL(triggered()), this, SLOT(exportSessionItem())));

   mpPropertiesAction = new QAction(QIcon(QIcon(":/icons/Properties")), "Properties...", this);
   mpPropertiesAction->setAutoRepeat(false);
   mpPropertiesAction->setToolTip("Properties");
   mpPropertiesAction->setStatusTip("Displays the properties for the selected item");
   VERIFYNR(connect(mpPropertiesAction, SIGNAL(triggered()), this, SLOT(displaySessionItemProperties())));

   //////////////
   // Menu Bar //
   //////////////

   // Create the main menu bar menus
   m_pFile = new QMenu("&File", this);
   m_pEdit = new QMenu("&Edit", this);
   m_pView = new QMenu("&View", this);
   m_pLayout = new QMenu("&Layout", this);
   m_pTools = new QMenu("&Tools", this);
   m_pWindow = new QMenu("&Window", this);
   m_pHelp = new QMenu("&Help", this);

   // Add the menus to the menu bar
   mpMenuBar = new MenuBarImp("Menu Bar", this);
   mpMenuBar->insertMenu(m_pFile);
   mpMenuBar->insertMenu(m_pEdit);
   mpMenuBar->insertMenu(m_pView);
   mpMenuBar->insertMenu(m_pLayout);
   mpMenuBar->insertMenu(m_pTools);
   mpMenuBar->insertMenu(m_pWindow);
   mpMenuBar->insertMenu(m_pHelp);

   setMenuBar(mpMenuBar);

   //////////////////////
   // Dockable Windows //
   //////////////////////

   // Session explorer
   mpSessionExplorer = new SessionExplorerAdapter(SessionItemImp::generateUniqueId(), this);
   addWindow(mpSessionExplorer);
   addDockWidget(Qt::LeftDockWidgetArea, mpSessionExplorer, Qt::Vertical);
   mpSessionExplorer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   mpSessionExplorer->attach(SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu), 
      Slot(this, &ApplicationWindow::updateContextMenu));
   SessionManagerImp::instance()->attach(SIGNAL_NAME(SessionManager, AboutToRestore),
      Slot(this, &ApplicationWindow::sessionAboutToRestore));
   SessionManagerImp::instance()->attach(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &ApplicationWindow::sessionLoaded));

   // Histogram window
   m_pHistogram = new HistogramWindowAdapter("{F2A66DD7-0C17-44ed-94FF-2BB4994222B9}", this);
   addWindow(m_pHistogram);
   m_pHistogram->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   m_pHistogram->hide();

   // Message log window
   m_pMessage_Log = new MessageLogWindow("{0FDD098B-33D8-493d-A27B-853FC7A7774A}", this);
   addWindow(m_pMessage_Log);
   m_pMessage_Log->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   m_pMessage_Log->hide();

   // Background plugin information window
   m_pBackground_Plugins = new BackgroundPluginWindow("{A0B14B71-4A33-4312-82F6-AFC274D1BA5C}", this);
   addWindow(m_pBackground_Plugins);
   addDockWidget(Qt::RightDockWidgetArea, m_pBackground_Plugins, Qt::Vertical);
   m_pBackground_Plugins->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   m_pBackground_Plugins->hide();

   // Scripting window
   m_pScripting = new ScriptingWindow("{66FF4DFC-2DDB-4f9b-9170-4C9E5CB22567}", this);
   addWindow(m_pScripting);
   m_pScripting->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   m_pScripting->hide();

   //////////////
   // Toolbars //
   //////////////

   // Standard toolbar
   mpStandardToolBar = new ToolBarAdapter("{039ADB46-B06A-4375-84D9-5A6C817BA413}", "Standard", this);
   if (mpStandardToolBar != NULL)
   {
      mpStandardToolBar->addAction(m_pOpen_Action);
      mpStandardToolBar->addAction(m_pClose_Action);
      mpStandardToolBar->addSeparator();
      mpStandardToolBar->addAction(m_pPrint_Action);
      mpStandardToolBar->addSeparator();
      mpStandardToolBar->addAction(m_pCut_Action);
      mpStandardToolBar->addAction(m_pCopy_Action);
      mpStandardToolBar->addAction(m_pPaste_Action);
      mpStandardToolBar->addSeparator();
      mpStandardToolBar->addWidget(pUndoButton);
      mpStandardToolBar->addWidget(pRedoButton);
      mpStandardToolBar->addSeparator();
      mpStandardToolBar->addAction(pAbout_Action);
      mpStandardToolBar->addAction(pHelp_Topics_Action);

      addToolBarBreak();
      addWindow(mpStandardToolBar);
      mpStandardToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   }

   // Toolbox toolbar
   string viewContext = m_pView->menuAction()->toolTip().toStdString();
   string mouseModeContext = viewContext + string("/Mouse Mode");

   mpToolboxToolBar = new ToolBarAdapter("{3D6084DD-D4F5-4ee8-A636-15BECAE1FDA2}", "Toolbox", this);
   if (mpToolboxToolBar != NULL)
   {
      mpToolboxToolBar->addAction(mpSessionExplorerAction);
      mpToolboxToolBar->addAction(m_pHistogram_Wnd_Action);
      mpToolboxToolBar->addAction(m_pMessage_Log_Wnd_Action);
      mpToolboxToolBar->addAction(m_pBackground_Plugins_Wnd_Action);
      mpToolboxToolBar->addAction(m_pScripting_Wnd_Action);
      mpToolboxToolBar->addSeparator();
      mpToolboxToolBar->addAction(m_pOverview_Wnd_Action);
      mpToolboxToolBar->addAction(m_pChipping_Wnd_Action);
      mpToolboxToolBar->addSeparator();
      mpToolboxToolBar->addAction(m_pLink_Action);
      mpToolboxToolBar->addSeparator();
      mpToolboxToolBar->addButton(m_pNo_View_Mode_Action, mouseModeContext);
      mpToolboxToolBar->addButton(mpLayerEditAction, mouseModeContext);
      mpToolboxToolBar->addSeparator();
      mpToolboxToolBar->addAction(pPropertiesAction);

      addWindow(mpToolboxToolBar);
      mpToolboxToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   }

   // Display toolbar
   mpDisplayToolBar = new DisplayToolBar("{3B90BF34-B2F7-4200-A8C5-43CB563D76F5}", this);
   if (mpDisplayToolBar != NULL)
   {
      mpDisplayToolBar->addAction(m_pZoom_In_Action);
      mpDisplayToolBar->addAction(m_pZoom_Out_Action);
      mpDisplayToolBar->addAction(m_pZoom_Point_In_Action);
      mpDisplayToolBar->addAction(m_pZoom_Point_Out_Action);
      mpDisplayToolBar->addAction(m_pZoom_Rect_Action);
      mpDisplayToolBar->addPercentageCombo();
      mpDisplayToolBar->addAction(m_pZoom_To_Fit_Action);
      mpDisplayToolBar->addAction(m_pZoom_And_Pan_To_Point_Action);
      mpDisplayToolBar->addSeparator();
      mpDisplayToolBar->addAction(m_pPan_Action);
      mpDisplayToolBar->addSeparator();
      mpDisplayToolBar->addAction(m_pRotate_Left_Action);
      mpDisplayToolBar->addAction(m_pRotate_Right_Action);
      mpDisplayToolBar->addAction(m_pFlip_Horiz_Action);
      mpDisplayToolBar->addAction(m_pFlip_Vert_Action);
      mpDisplayToolBar->addAction(m_pRotate_By_Action);
      mpDisplayToolBar->addButton(m_pFree_Rotate_Action);
      mpDisplayToolBar->addButton(mpNorthUpAction);
      mpDisplayToolBar->addButton(mpSensorUpAction);
      mpDisplayToolBar->addAction(m_pReset_Action);
      mpDisplayToolBar->addSeparator();
      mpDisplayToolBar->addAction(m_pDisplay_Mode_Action);
      mpDisplayToolBar->addSeparator();
      mpDisplayToolBar->addAction(mpResetStretchAction);
      mpDisplayToolBar->addAction(m_pRefresh_Action);
      mpDisplayToolBar->addAction(mpGenerateImageAction);

      addToolBarBreak();
      addWindow(mpDisplayToolBar);
      mpDisplayToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
   }

   // AOI toolbar
   mpAoiToolBar = new AoiToolBar("{D82BF3FF-B773-4d5e-970E-CCB6D6E73223}", this);
   if (mpAoiToolBar != NULL)
   {
      addToolBarBreak();
      addWindow(mpAoiToolBar);
      mpAoiToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpAoiToolBar->hide();
   }

   // GCP toolbar
   mpGcpToolBar = new GcpToolBar("{34268D08-4933-456a-8ED8-75D4619F5B53}", this);
   if (mpGcpToolBar != NULL)
   {
      mpGcpToolBar->addAction(m_pGCP_Editor_Action);

      addWindow(mpGcpToolBar);
      mpGcpToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpGcpToolBar->hide();
   }

   // TiePoint toolbar
   mpTiePointToolBar = new TiePointToolBar("{845013A9-A224-4aa1-B886-84ACF9A0D033}", this);
   if (mpTiePointToolBar != NULL)
   {
      mpTiePointToolBar->addAction(m_pTiePoint_Editor_Action);

      addWindow(mpTiePointToolBar);
      mpTiePointToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpTiePointToolBar->hide();
   }

   // Annotation toolbar
   mpAnnotationToolBar = new AnnotationToolBar("{471661DB-5785-4f46-ABE3-963C2537BDE1}", this);
   if (mpAnnotationToolBar != NULL)
   {
      addToolBarBreak();
      addWindow(mpAnnotationToolBar);
      mpAnnotationToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpAnnotationToolBar->hide();
      mpAnnotationToolBar->installEventFilter(this);
   }

   // Brightness toolbar
   mpBrightnessToolbar = new BrightnessToolBar("{6BB07518-E7FF-4916-98BF-166717D284C8}", this);
   if (mpBrightnessToolbar != NULL)
   {
      addToolBarBreak();
      addWindow(mpBrightnessToolbar);
      mpBrightnessToolbar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpBrightnessToolbar->hide();
   }

   // Measurement toolbar
   mpMeasurementToolBar = new MeasurementToolBar("{A2C9AD32-CE1E-428e-8F36-4B1FF89A0935}", this);
   if (mpMeasurementToolBar != NULL)
   {
      mpMeasurementToolBar->addSeparator();
      mpMeasurementToolBar->addButton(m_pMeasurement_Edit_Action, mouseModeContext);

      addToolBarBreak();
      addWindow(mpMeasurementToolBar);
      mpMeasurementToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpMeasurementToolBar->hide();
   }

   // Animation toolbar
   mpAnimationToolBar = new AnimationToolBarAdapter("{AFC0B557-9A3A-47f3-BF4A-C7BB2D951403}", this);
   if (mpAnimationToolBar != NULL)
   {
      addToolBarBreak();
      addWindow(mpAnimationToolBar);
      mpAnimationToolBar->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      mpAnimationToolBar->hide();
   }

   ///////////
   // Menus //
   ///////////

   // Create the submenus
   mpToolbarsMenu = new QMenu ("Toolbars", m_pView);
   QMenu* pPanMenu = new QMenu("Pan &Mode", m_pView);
   QMenu* pRotateMenu = new QMenu("Rot&ate", m_pView);
   QMenu* pZoomMenu = new QMenu("&Zoom", m_pView);
   QMenu* pCenterMenu = new QMenu("&Center", pZoomMenu);
   QMenu* pPointMenu = new QMenu("Poi&nt", pZoomMenu);
   QMenu* pPercentMenu = new QMenu("&Percent", pZoomMenu);

   // File menu
   string fileContext = m_pFile->menuAction()->toolTip().toStdString();
   mpMenuBar->insertCommand(m_pNewSession_Action, m_pFile, fileContext);
   mpMenuBar->insertCommand(m_pOpenSession_Action, m_pFile, fileContext);
   mpMenuBar->insertCommand(m_pSaveSession_Action, m_pFile, fileContext);
   mpMenuBar->insertCommand(m_pSaveSessionAs_Action, m_pFile, fileContext);
   m_pFile->addSeparator();

   mpMenuBar->insertCommand(m_pOpen_Action, m_pFile, fileContext);   // Add the commands from the menu bar so that
                                                                     // the shortcut will be added to config settings
   mpMenuBar->insertCommand(m_pClose_Action, m_pFile, fileContext);
   m_pFile->addSeparator();
   mpMenuBar->insertCommand(mpExportFileMenuAction, m_pFile, fileContext);    // Add the menu from the menu bar so that plug-ins
                                                     // can use this menu to add commands
   m_pFile->addSeparator();
   mpMenuBar->insertCommand(m_pPrint_Setup_Action, m_pFile, fileContext);
   m_pFile->addSeparator();

   mpRecentFileAction = m_pFile->addAction("Recent File");
   mpRecentFileAction->setEnabled(false);

   mpMruInsertAction = m_pFile->addSeparator();

   QAction* pExitAction = mpMenuBar->addCommand("&File/E&xit", fileContext);
   pExitAction->setAutoRepeat(false);
   pExitAction->setToolTip("Exit");
   pExitAction->setStatusTip("Closes all active views and dismisses the application");
   VERIFYNR(connect(pExitAction, SIGNAL(triggered()), this, SLOT(close())));

   // Edit menu
   string editContext = m_pEdit->menuAction()->toolTip().toStdString();
   mpMenuBar->insertCommand(pUndoButton->defaultAction(), m_pEdit, editContext);
   mpMenuBar->insertCommand(pRedoButton->defaultAction(), m_pEdit, editContext);
   m_pEdit->addSeparator();
   mpMenuBar->insertCommand(m_pCut_Action, m_pEdit, editContext);
   mpMenuBar->insertCommand(m_pCopy_Action, m_pEdit, editContext);
   mpMenuBar->insertCommand(m_pPaste_Action, m_pEdit, editContext);

   // View menu
   mpMenuBar->insertCommand(pStatus_Bar_Action, m_pView, viewContext);
   mpMenuBar->insertMenu(mpToolbarsMenu, m_pView);
   m_pView->addSeparator();
   mpMenuBar->insertCommand(m_pPan_Action, m_pView, mouseModeContext);
   mpMenuBar->insertMenu(pPanMenu, m_pView);
   m_pView->addSeparator();
   mpMenuBar->insertMenu(pRotateMenu, m_pView);
   mpMenuBar->insertMenu(pZoomMenu, m_pView);
   m_pView->addSeparator();
   mpMenuBar->insertCommand(mpCreateAnimationAction, m_pView, viewContext);
   mpMenuBar->insertCommand(mpResetStretchAction, m_pView, viewContext);
   mpMenuBar->insertCommand(m_pRefresh_Action, m_pView, viewContext);
   mpMenuBar->insertCommand(mpGenerateImageAction, m_pView, viewContext);
   m_pView->addSeparator();
   mpMenuBar->insertCommand(mpClearMarkingsAction, m_pView, viewContext);
   mpMenuBar->insertCommand(pPropertiesAction, m_pView, viewContext);

   // Pan Mode popup menu
   string panContext = viewContext + string("/") + pPanMenu->menuAction()->toolTip().toStdString();;
   mpMenuBar->insertCommand(m_pPan_Delay_Action, pPanMenu, panContext);
   mpMenuBar->insertCommand(m_pPan_Instant_Action, pPanMenu, panContext);

   // Rotate popup menu
   string rotateContext = viewContext + string("/") + pRotateMenu->menuAction()->toolTip().toStdString();;
   mpMenuBar->insertCommand(m_pRotate_Left_Action, pRotateMenu, rotateContext);
   mpMenuBar->insertCommand(m_pRotate_Right_Action, pRotateMenu, rotateContext);
   pRotateMenu->addSeparator();
   mpMenuBar->insertCommand(m_pFlip_Horiz_Action, pRotateMenu, rotateContext);
   mpMenuBar->insertCommand(m_pFlip_Vert_Action, pRotateMenu, rotateContext);
   pRotateMenu->addSeparator();
   mpMenuBar->insertCommand(m_pRotate_By_Action, pRotateMenu, rotateContext);
   mpMenuBar->insertCommand(m_pFree_Rotate_Action, pRotateMenu, mouseModeContext);
   pRotateMenu->addSeparator();
   mpMenuBar->insertCommand(mpNorthUpAction, pRotateMenu, rotateContext);
   mpMenuBar->insertCommand(mpSensorUpAction, pRotateMenu, rotateContext);
   pRotateMenu->addSeparator();
   mpMenuBar->insertCommand(m_pReset_Action, pRotateMenu, rotateContext);

   // Zoom popup menu
   string zoomContext = viewContext + string("/") + pZoomMenu->menuAction()->toolTip().toStdString();
   mpMenuBar->insertMenu(pCenterMenu, pZoomMenu);
   mpMenuBar->insertMenu(pPointMenu, pZoomMenu);
   mpMenuBar->insertMenu(pPercentMenu, pZoomMenu);
   mpMenuBar->insertCommand(m_pZoom_Rect_Action, pZoomMenu, mouseModeContext);
   pZoomMenu->addSeparator();
   mpMenuBar->insertCommand(m_pZoom_To_Fit_Action, pZoomMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom_And_Pan_To_Point_Action, pZoomMenu, zoomContext);

   // Center popup menu
   mpMenuBar->insertCommand(m_pZoom_In_Action, pCenterMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom_Out_Action, pCenterMenu, zoomContext);

   // Point popup menu
   mpMenuBar->insertCommand(m_pZoom_Point_In_Action, pPointMenu, mouseModeContext);
   mpMenuBar->insertCommand(m_pZoom_Point_Out_Action, pPointMenu, mouseModeContext);

   // Percent popup menu
   mpMenuBar->insertCommand(m_pZoom10_Action, pPercentMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom50_Action, pPercentMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom75_Action, pPercentMenu, zoomContext);
   pPercentMenu->addSeparator();
   mpMenuBar->insertCommand(m_pZoom100_Action, pPercentMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom150_Action, pPercentMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom200_Action, pPercentMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom400_Action, pPercentMenu, zoomContext);
   mpMenuBar->insertCommand(m_pZoom800_Action, pPercentMenu, zoomContext);

   // Layout menu
   string layoutContext = m_pLayout->menuAction()->toolTip().toStdString();
   mpMenuBar->insertCommand(pNewProductAction, m_pLayout, layoutContext);
   mpMenuBar->insertCommand(mpDeriveProductAction, m_pLayout, layoutContext);
   m_pLayout->addSeparator();
   mpMenuBar->insertCommand(m_pLoadTemplate_Action, m_pLayout, layoutContext);
   mpMenuBar->insertCommand(m_pSaveTemplate_Action, m_pLayout, layoutContext);
   m_pLayout->addSeparator();
   mpMenuBar->insertCommand(mpEditViewAction, m_pLayout, layoutContext);
   m_pLayout->addSeparator();
   mpMenuBar->insertCommand(m_pPaperSize_Action, m_pLayout, layoutContext);

   // Tools menu
   string toolsContext = m_pTools->menuAction()->toolTip().toStdString();
   mpMenuBar->insertCommand(mpSessionExplorerAction, m_pTools, toolsContext);
   mpMenuBar->insertCommand(m_pHistogram_Wnd_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(m_pMessage_Log_Wnd_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(m_pBackground_Plugins_Wnd_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(m_pScripting_Wnd_Action, m_pTools, toolsContext);
   m_pTools->addSeparator();
   mpMenuBar->insertCommand(m_pOverview_Wnd_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(m_pChipping_Wnd_Action, m_pTools, toolsContext);
   m_pTools->addSeparator();
   mpMenuBar->insertCommand(m_pTiePoint_Editor_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(m_pGCP_Editor_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(pWizard_Builder_Action, m_pTools, toolsContext);
   mpMenuBar->insertCommand(pBatch_Wizard_Editor_Action, m_pTools, toolsContext);
   m_pTools->addSeparator();
   mpMenuBar->insertCommand(m_pLink_Action, m_pTools, toolsContext);
   m_pTools->addSeparator();
   mpMenuBar->insertCommand(pUpdate_Wizards_Action, m_pTools, toolsContext);
   m_pTools->addSeparator();
   mpMenuBar->insertCommand(pOptions_Action, m_pTools, toolsContext);

   // Help menu
   string helpContext = m_pHelp->menuAction()->toolTip().toStdString();
   mpMenuBar->insertCommand(pHelp_Topics_Action, m_pHelp, helpContext);
   map<string, string> helpEntries = Service<InstallerServices>()->getHelpEntries();
   for(map<string, string>::const_iterator entry = helpEntries.begin(); entry != helpEntries.end(); ++entry)
   {
      QAction* pAction = new QAction(this);
      pAction->setText(QString::fromStdString(entry->first));
      pAction->setStatusTip(QString("Displays help topics for ") + QString::fromStdString(entry->first));
      QMap<QString, QVariant> actionData;
      actionData["helpWebpage"] = QString::fromStdString(entry->second);
      pAction->setData(actionData);
      connect(pAction, SIGNAL(triggered()), this, SLOT(displayPlugInHelp()));
      mpMenuBar->insertCommand(pAction, m_pHelp, helpContext); 
   }

   m_pHelp->addSeparator();
   mpMenuBar->insertCommand(pExtensions_Action, m_pHelp, helpContext);
   m_pHelp->addSeparator();
   mpMenuBar->insertCommand(pAbout_Action, m_pHelp, helpContext);

   ////////////////
   // Status Bar //
   ////////////////

   m_pStatus_Bar = new StatusBar(this);
   pStatus_Bar_Action->setChecked(true);
   setStatusBar(m_pStatus_Bar);

   ///////////////////
   // MDI Workspace //
   ///////////////////

   QFrame* pWorkspaceFrame = new QFrame(this);
   pWorkspaceFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);

   mpWorkspace = new Workspace(pWorkspaceFrame);

   QPalette workspacePalette = mpWorkspace->palette();
   workspacePalette.setColor(QPalette::Window, Qt::darkGray);
   mpWorkspace->setPalette(workspacePalette);

   QVBoxLayout* pWorkspaceLayout = new QVBoxLayout(pWorkspaceFrame);
   pWorkspaceLayout->setMargin(0);
   pWorkspaceLayout->addWidget(mpWorkspace, 10);

   setCentralWidget(pWorkspaceFrame);

   ///////////
   // Tools //
   ///////////

   mpGcpEditor = new GcpEditorDlg(this);
   mpTiePointEditor = new TiePointEditor(this);

   ////////////////////
   // Initialization //
   ////////////////////

   setAnimated(true);
   setDockNestingEnabled(true);

   // Add the plug-in and wizard commands to the menu bars
   updatePlugInCommands();
   updateWizardCommands();

   // Disable the actions and toolbar buttons that are dependent on a spatial data window
   enableActions(false);
   enableToolBars(false);

   m_pNo_View_Mode_Action->setChecked(true);

   mpSessionExplorer->initialize();
   qApp->installEventFilter(this);

   Service<DesktopServices> pDesktop;
   string shortcutContext = "View/Snapshot";
   mpClipboardSizedAction = new QAction(QIcon(":/icons/Copy"), "Copy snapshot...", this);
   mpClipboardSizedAction->setStatusTip
      ("Presents the copy snapshot dialog and copies a snapshot of the current view into the clipboard");
   mpClipboardSizedAction->setToolTip("Copy Snapshot With Dialog");
   mpClipboardSizedAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpClipboardSizedAction, shortcutContext);
   addAction(mpClipboardSizedAction);

   mpClipboardAction = new QAction(QIcon(":/icons/Copy"), "Copy snapshot", this);
   mpClipboardAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
   mpClipboardAction->setStatusTip
      ("Copies a snapshot of the current view using the default resolution into the clipboard");
   mpClipboardAction->setToolTip("Copy Snapshot");
   mpClipboardAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpClipboardAction, shortcutContext);
   addAction(mpClipboardAction);

   /////////////////
   // Connections //
   /////////////////

   VERIFYNR(connect(mpMenuBar, SIGNAL(triggered(QAction*)), this, SLOT(executeCommand(QAction*))));
   VERIFYNR(connect(m_pFile, SIGNAL(aboutToShow()), this, SLOT(constructFileMenu())));
   VERIFYNR(connect(m_pFile, SIGNAL(triggered(QAction*)), this, SLOT(fileMenuActivated(QAction*))));
   VERIFYNR(connect(m_pWindow, SIGNAL(aboutToShow()), this, SLOT(constructWindowMenu())));
   VERIFYNR(connect(m_pWindow, SIGNAL(triggered(QAction*)), this, SLOT(windowMenuActivated(QAction*))));
   VERIFYNR(connect(mpSessionExplorerAction, SIGNAL(triggered(bool)), mpSessionExplorer, SLOT(setVisible(bool))));
   VERIFYNR(connect(mpSessionExplorer, SIGNAL(visibilityChanged(bool)), mpSessionExplorerAction,
      SLOT(setChecked(bool))));
   VERIFYNR(connect(m_pHistogram_Wnd_Action, SIGNAL(triggered(bool)), static_cast<HistogramWindowImp*>(m_pHistogram),
      SLOT(setVisible(bool))));
   VERIFYNR(connect(m_pHistogram, SIGNAL(visibilityChanged(bool)), m_pHistogram_Wnd_Action, SLOT(setChecked(bool))));
   VERIFYNR(connect(m_pMessage_Log_Wnd_Action, SIGNAL(triggered(bool)), m_pMessage_Log, SLOT(setVisible(bool))));
   VERIFYNR(connect(m_pMessage_Log, SIGNAL(visibilityChanged(bool)), m_pMessage_Log_Wnd_Action,
      SLOT(setChecked(bool))));
   VERIFYNR(connect(m_pBackground_Plugins_Wnd_Action, SIGNAL(triggered(bool)), m_pBackground_Plugins,
      SLOT(setVisible(bool))));
   VERIFYNR(connect(m_pBackground_Plugins, SIGNAL(visibilityChanged(bool)), m_pBackground_Plugins_Wnd_Action,
      SLOT(setChecked(bool))));
   VERIFYNR(connect(mpGcpEditor, SIGNAL(visibilityChanged(bool)), m_pGCP_Editor_Action, SLOT(setChecked(bool))));
   VERIFYNR(connect(mpTiePointEditor, SIGNAL(visibilityChanged(bool)), m_pTiePoint_Editor_Action,
      SLOT(setChecked(bool))));
   VERIFYNR(connect(mpDisplayToolBar, SIGNAL(zoomChanged(double)), this, SLOT(zoomTo(double))));
   VERIFYNR(connect(m_pScripting_Wnd_Action, SIGNAL(triggered(bool)), m_pScripting, SLOT(setVisible(bool))));
   VERIFYNR(connect(m_pScripting, SIGNAL(visibilityChanged(bool)), m_pScripting_Wnd_Action, SLOT(setChecked(bool))));
   VERIFYNR(connect(mpWorkspace, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateActiveWindow(QMdiSubWindow*))));
   VERIFYNR(connect(mpToolbarsMenu, SIGNAL(aboutToShow()), this, SLOT(showToolbarsMenu())));
   VERIFYNR(connect(mpClipboardSizedAction, SIGNAL(triggered()), SLOT(forwardSnapshot())));
   VERIFYNR(connect(mpClipboardAction, SIGNAL(triggered()), SLOT(forwardSnapshot())));

   attach(SIGNAL_NAME(ApplicationWindow, WindowAdded),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, WindowAdded)));
   attach(SIGNAL_NAME(ApplicationWindow, WindowRemoved),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, WindowRemoved)));
   attach(SIGNAL_NAME(ApplicationWindow, WindowActivated),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, WindowActivated)));
   attach(SIGNAL_NAME(ApplicationWindow, AboutToShowPropertiesDialog),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowPropertiesDialog)));
   attach(SIGNAL_NAME(Subject, Modified), Signal(pDesktop.get(), SIGNAL_NAME(Subject, Modified)));
   attach(SIGNAL_NAME(ApplicationWindow, Closed), Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, ApplicationWindowClosed)));
   pDesktop->attach(SIGNAL_NAME(DesktopServices, AboutToShowContextMenu),
      Slot(this, &ApplicationWindow::updateContextMenu));

   Service<ApplicationServices> pApp;
   attach(SIGNAL_NAME(ApplicationWindow, Closed), Signal(pApp.get(), SIGNAL_NAME(ApplicationServices, ApplicationClosed)));

   ////////////////////////
   // Check the message log
   // path to see if it is
   // writable
   ////////////////////////
   const Filename* pMessageLogPath = ConfigurationSettings::getSettingMessageLogPath();
   string messageLogPath;
   if (pMessageLogPath != NULL)
   {
      messageLogPath = pMessageLogPath->getFullPathAndName();
   }
   QFileInfo tempFileInfo(tempnam(messageLogPath.c_str(), NULL));
   // need to split out the unique temp file name and re-append
   // the path as tempnam() on Windows will use %TMP% as the path
   // if available regardless of the path you pass in
   QFile tempFile(QString(messageLogPath.c_str()) + "/" + tempFileInfo.fileName());
   if (!tempFile.open(QIODevice::WriteOnly))
   {
      // hide the splash screen so it doesn't obscure the message dialog
      if (pSplash != NULL)
      {
         pSplash->hide();
      }

      QString errorMessage("Unable to access log files\n");
      errorMessage += tempFile.errorString();
      QMessageBox::warning(this,
                           "Log File Error",
                           errorMessage,
                           QMessageBox::Ignore,
                           QMessageBox::NoButton);
   }
   else
   {
      tempFile.close();
      tempFile.remove();
   }

   QMainWindow::setAcceptDrops(true);
}

ApplicationWindow::~ApplicationWindow()
{
   notify(SIGNAL_NAME(Subject, Deleted));

   mpSessionExplorer->detach(SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &ApplicationWindow::updateContextMenu));

   Service<DesktopServices> pDesktop;
   pDesktop->detach(SIGNAL_NAME(DesktopServices, AboutToShowContextMenu),
      Slot(this, &ApplicationWindow::updateContextMenu));
}

void ApplicationWindow::setSessionFilename(const string& sessionFilename)
{
   mSessionFilename = sessionFilename;
}

const string& ApplicationWindow::getObjectType() const
{
   static string type("ApplicationWindow");
   return type;
}

bool ApplicationWindow::isKindOf(const string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return SubjectAdapter::isKindOf(className);
}

void ApplicationWindow::windowRemoved(Subject &subject, const string &signal, const boost::any &data)
{
   Window *pWindow = dynamic_cast<Window*>(&subject);
   if (NN(pWindow))
   {
      removeWindow(pWindow);
   }
}

Window* ApplicationWindow::createWindow(const QString& strWindowName, WindowType windowType)
{
   if (strWindowName.isEmpty() == true)
   {
      return NULL;
   }

   // Do not create a window that already exists
   Window* pWindow = NULL;
   pWindow = getWindow(strWindowName, windowType);
   if (pWindow != NULL)
   {
      return NULL;
   }

   if (windowType == DOCK_WINDOW)
   {
      pWindow = new DockWindowAdapter(SessionItemImp::generateUniqueId(), strWindowName.toStdString(), this);
   }
   else if (windowType == PLOT_WINDOW)
   {
      pWindow = new PlotWindowAdapter(SessionItemImp::generateUniqueId(), strWindowName.toStdString(), this);
   }
   else if ((windowType == WORKSPACE_WINDOW) || (windowType == SPATIAL_DATA_WINDOW) || (windowType == PRODUCT_WINDOW))
   {
      QString strName = strWindowName;
      strName.replace(QRegExp("\\\\"), "/");

      if (windowType == WORKSPACE_WINDOW)
      {
         pWindow = new WorkspaceWindowAdapter(SessionItemImp::generateUniqueId(), strName.toStdString(), mpWorkspace);
      }
      else if (windowType == SPATIAL_DATA_WINDOW)
      {
         pWindow = new SpatialDataWindowAdapter(SessionItemImp::generateUniqueId(), strName.toStdString(), mpWorkspace);
      }
      else if (windowType == PRODUCT_WINDOW)
      {
         pWindow = new ProductWindowAdapter(SessionItemImp::generateUniqueId(), strName.toStdString(), mpWorkspace);
      }
   }
   else if (windowType == TOOLBAR)
   {
      pWindow = new ToolBarAdapter(SessionItemImp::generateUniqueId(), strWindowName.toStdString(), this);
   }

   if (pWindow != NULL)
   {
      addWindow(pWindow);
   }

   return pWindow;
}

bool ApplicationWindow::addWindow(Window* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   // Do not insert the window if it already exists in the list
   if (containsWindow(pWindow) == true)
   {
      return false;
   }

   // Add the window
   mWindows.push_back(pWindow);

   // Notify attached objects
   notify(SIGNAL_NAME(ApplicationWindow, WindowAdded), boost::any(pWindow));

   // Initialization
   WorkspaceWindow* pWorkspaceWindow = dynamic_cast<WorkspaceWindow*> (pWindow);
   if (pWorkspaceWindow != NULL)
   {
      // Add the window to the workspace and install an event filter
      WorkspaceWindowImp* pWorkspaceWindowImp = dynamic_cast<WorkspaceWindowImp*> (pWorkspaceWindow);
      if (pWorkspaceWindowImp != NULL)
      {
         mpWorkspace->addSubWindow(pWorkspaceWindowImp);
         pWorkspaceWindowImp->installEventFilter(this);

         // Show the window -- do not maximize during session load since the window
         //  position and state were restored already in WorkspaceWindowImp::fromXml.
         WindowSizeType eWindowSize = WorkspaceWindow::getSettingWindowSize();
         if (!Service<SessionManager>()->isSessionLoading() && eWindowSize == MAXIMIZED)
         {
            pWorkspaceWindowImp->showMaximized();
         }
         else
         {
            pWorkspaceWindowImp->show();
         }
      }

      // Set the current mouse mode in the new window
      QAction* pMouseModeAction = m_pView_Mode_Group->checkedAction();
      setMouseMode(pMouseModeAction);
   }

   ViewWindow* pViewWindow = dynamic_cast<ViewWindow*>(pWindow);
   if (pViewWindow != NULL)
   {
      ViewImp* pView = dynamic_cast<ViewImp*>(pViewWindow->getView());
      if (pView != NULL)
      {
         UndoStack* pUndoStack = pView->getUndoStack();
         if (pUndoStack != NULL)
         {
            mpUndoGroup->addStack(pUndoStack);
            VERIFYNR(connect(pUndoStack, SIGNAL(sessionItemChanged(const std::string&, const std::string&)), this,
               SLOT(updateUndoActions(const std::string&, const std::string&))));
         }
      }
   }

   DockWindowImp* pDockWindow = dynamic_cast<DockWindowImp*>(pWindow);
   if (pDockWindow != NULL)
   {
      pDockWindow->setObjectName(QString::fromStdString(pDockWindow->getName()));
      addDockWidget(Qt::BottomDockWidgetArea, pDockWindow);
      pDockWindow->restoreState();
   }

   ToolBarImp* pToolBar = dynamic_cast<ToolBarImp*>(pWindow);
   if (pToolBar != NULL)
   {
      pToolBar->setObjectName(QString::fromStdString(pToolBar->getName()));

      MenuBarImp* pMenuBar = static_cast<MenuBarImp*>(pToolBar->getMenuBar());
      if (pMenuBar != NULL)
      {
         VERIFYNR(connect(pMenuBar, SIGNAL(triggered(QAction*)), this, SLOT(executeCommand(QAction*))));
      }

      addToolBar(pToolBar);
   }

   // Connections
   pWindow->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));

   return true;
}

Window* ApplicationWindow::getWindow(const QString& strWindowName, WindowType windowType) const
{
   if (strWindowName.isEmpty() == true)
   {
      return NULL;
   }

   for (unsigned int i = 0; i < mWindows.size(); i++)
   {
      Window* pWindow = NULL;
      pWindow = mWindows.at(i);
      if (pWindow != NULL)
      {
         QString strCurrentName;

         string windowName = pWindow->getName();
         if (windowName.empty() == false)
         {
            strCurrentName = QString::fromStdString(windowName);
         }

         WindowType currentType = pWindow->getWindowType();

         if ((strCurrentName == strWindowName) && (currentType == windowType))
         {
            return pWindow;
         }
      }
   }

   return NULL;
}

Window* ApplicationWindow::getWindow(const string& uniqueId) const
{
   if (uniqueId.empty() == true)
   {
      return NULL;
   }

   for (unsigned int i = 0; i < mWindows.size(); i++)
   {
      Window* pWindow = NULL;
      pWindow = mWindows.at(i);
      if (pWindow != NULL)
      {
         if (uniqueId == pWindow->getId())
         {
            return pWindow;
         }
      }
   }

   return NULL;
}

vector<Window*> ApplicationWindow::getWindows(WindowType windowType) const
{
   vector<Window*> windows;
   for (unsigned int i = 0; i < mWindows.size(); i++)
   {
      Window* pWindow = NULL;
      pWindow = mWindows.at(i);
      if (pWindow != NULL)
      {
         WindowType currentType = pWindow->getWindowType();
         if (currentType == windowType)
         {
            windows.push_back(pWindow);
         }
      }
   }

   return windows;
}

vector<Window*> ApplicationWindow::getWindows(const QString& strWindowType) const
{
   vector<Window*> windows;
   if (strWindowType.isEmpty() == false)
   {
      for (unsigned int i = 0; i < mWindows.size(); i++)
      {
         Window* pWindow = NULL;
         pWindow = mWindows.at(i);
         if (pWindow != NULL)
         {
            if (pWindow->isKindOf(strWindowType.toStdString()) == true)
            {
               windows.push_back(pWindow);
            }
         }
      }
   }

   return windows;
}

vector<Window*> ApplicationWindow::getWindows() const
{
   return mWindows;
}

bool ApplicationWindow::containsWindow(Window* pWindow) const
{
   if (pWindow == NULL)
   {
      return false;
   }

   for (unsigned int i = 0; i < mWindows.size(); i++)
   {
      Window* pCurrentWindow = NULL;
      pCurrentWindow = mWindows.at(i);
      if (pCurrentWindow == pWindow)
      {
         return true;
      }
   }

   return false;
}

unsigned int ApplicationWindow::getNumWindows(WindowType windowType) const
{
   vector<Window*> windows = getWindows(windowType);
   return windows.size();
}

unsigned int ApplicationWindow::getNumWindows(const QString& strWindowType) const
{
   vector<Window*> windows = getWindows(strWindowType);
   return windows.size();
}

unsigned int ApplicationWindow::getNumWindows() const
{
   return mWindows.size();
}

bool ApplicationWindow::removeWindow(Window* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   if (isDefaultWindow(pWindow) == true)
   {
      return false;
   }

   vector<Window*>::iterator iter = std::find(mWindows.begin(), mWindows.end(), pWindow);
   if (iter != mWindows.end())
   {
      ViewWindow* pViewWindow = dynamic_cast<ViewWindow*>(pWindow);
      if (pViewWindow != NULL)
      {
         ViewImp* pView = dynamic_cast<ViewImp*>(pViewWindow->getView());
         if (pView != NULL)
         {
            UndoStack* pUndoStack = pView->getUndoStack();
            if (pUndoStack != NULL)
            {
               mpUndoGroup->removeStack(pUndoStack);
               disconnect(pUndoStack, SIGNAL(sessionItemChanged(const std::string&, const std::string&)), this,
                  SLOT(updateUndoActions(const std::string&, const std::string&)));
            }
         }
      }

      ToolBarImp* pToolBar = dynamic_cast<ToolBarImp*>(pWindow);
      if (pToolBar != NULL)
      {
         MenuBarImp* pMenuBar = static_cast<MenuBarImp*>(pToolBar->getMenuBar());
         if (pMenuBar != NULL)
         {
            VERIFYNR(disconnect(pMenuBar, SIGNAL(triggered(QAction*)), this, SLOT(executeCommand(QAction*))));
         }
      }

      mWindows.erase(iter);
      pWindow->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ApplicationWindow::windowRemoved));
      WorkspaceWindowImp *pWorkspaceWindow = dynamic_cast<WorkspaceWindowImp*>(pWindow);
      if (pWorkspaceWindow != NULL)
      {
         pWorkspaceWindow->removeEventFilter(this);
      }

      notify(SIGNAL_NAME(ApplicationWindow, WindowRemoved), boost::any(pWindow));
      return true;
   }

   return false;
}

bool ApplicationWindow::deleteWindow(Window* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   if (isDefaultWindow(pWindow) == true)
   {
      return false;
   }

   // if dock window, save geometry
   DockWindowImp* pDockWindow = dynamic_cast<DockWindowImp*>(pWindow);
   if (pDockWindow != NULL)
   {
      pDockWindow->saveState();
   }

   // Close the widget before removing the window to cancel out if the user does not confirm the close
   QWidget* pWidget = dynamic_cast<QWidget*>(pWindow);
   if (pWidget != NULL)
   {
      if (pWidget->close() == false)
      {
         return false;
      }
   }

   // Remove the window from the member vector
   if (removeWindow(pWindow) == false)
   {
      return false;
   }

   delete pWidget;
   return true;
}

bool ApplicationWindow::setCurrentWorkspaceWindow(WorkspaceWindow* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   if (containsWindow(pWindow) == false)
   {
      return false;
   }

   WorkspaceWindowImp* pWindowImp = dynamic_cast<WorkspaceWindowImp*>(pWindow);
   if (pWindowImp != NULL)
   {
      mpWorkspace->setActiveSubWindow(pWindowImp);
      pWindowImp->setFocus(Qt::ActiveWindowFocusReason);
      return true;
   }

   return false;
}

WorkspaceWindow* ApplicationWindow::getCurrentWorkspaceWindow() const
{
   WorkspaceWindow* pWindow = NULL;
   if (mpWorkspace != NULL)
   {
      // mpWorkspace->activeSubWindow() cannot be used as it returns NULL when a widget outside mpWorkspace is active
      pWindow = dynamic_cast<WorkspaceWindow*>(mpWorkspace->currentSubWindow());
   }

   return pWindow;
}

QString ApplicationWindow::getCurrentWorkspaceWindowName() const
{
   QString strWindowName;

   WorkspaceWindowImp* pWindow = dynamic_cast<WorkspaceWindowImp*>(getCurrentWorkspaceWindow());
   if (pWindow != NULL)
   {
      strWindowName = QString::fromStdString(pWindow->getName());
   }

   return strWindowName;
}

View* ApplicationWindow::getCurrentWorkspaceWindowView() const
{
   WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      return pWindow->getActiveView();
   }

   return NULL;
}

void ApplicationWindow::cascadeWorkspaceWindows()
{
   mpWorkspace->cascadeSubWindows();
}

void ApplicationWindow::tileWorkspaceWindows(TilingType eType)
{
   mpWorkspace->tile(eType);
}

bool ApplicationWindow::tileWorkspaceWindows(const vector<WorkspaceWindow*>& windows, 
                                             bool maxFirst, TilingType eType)
{
   return mpWorkspace->tileWindows(windows, maxFirst, eType);
}

void ApplicationWindow::setPanMode(PanModeType panMode)
{
   if (panMode == PAN_DELAY)
   {
      m_pPan_Delay_Action->setChecked(true);
   }
   else if (panMode == PAN_INSTANT)
   {
      m_pPan_Instant_Action->setChecked(true);
   }
}

PanModeType ApplicationWindow::getPanMode() const
{
   PanModeType panMode;
   if (m_pPan_Delay_Action->isChecked() == true)
   {
      panMode = PAN_DELAY;
   }
   else if (m_pPan_Instant_Action->isChecked() == true)
   {
      panMode = PAN_INSTANT;
   }

   return panMode;
}

void ApplicationWindow::setAoiSelectionTool(GraphicObjectType toolType, ModeType modeType)
{
   mpAoiToolBar->setSelectionTool(toolType, modeType);
}

void ApplicationWindow::setAoiAddMode(AoiAddMode mode)
{
   mpAoiToolBar->setAddMode(mode);
}

GraphicObjectType ApplicationWindow::getAoiSelectionTool() const
{
   return mpAoiToolBar->getSelectionTool();
}

ModeType ApplicationWindow::getAoiSelectionMode() const
{
   return mpAoiToolBar->getSelectionMode();
}

AoiAddMode ApplicationWindow::getAoiAddMode() const
{
   return mpAoiToolBar->getAddMode();
}

void ApplicationWindow::setAnnotationObject(GraphicObjectType objectType)
{
   mpAnnotationToolBar->setSelectionObject(objectType);
}

GraphicObjectType ApplicationWindow::getAnnotationObject() const
{
   AnnotationLayerImp *pLayer = dynamic_cast<AnnotationLayerImp*>(mpAnnotationToolBar->getAnnotationLayer());
   return pLayer->getCurrentGraphicObjectType();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Menu management

void ApplicationWindow::constructFileMenu()
{
   ConfigurationSettingsImp* pSettings = ConfigurationSettingsImp::instance();
   if (pSettings == NULL)
   {
      return;
   }

   // Check for no updates
   const vector<MruFile*>& mruFiles = pSettings->getMruFiles();
   if ((mruFiles.empty() == true) && (mMruFileCommands.empty() == true))
   {
      return;
   }

   // Remove the existing MRU file commands
   if (mMruFileCommands.empty() == true)
   {
      m_pFile->removeAction(mpRecentFileAction);
   }
   else
   {
      QMap<QAction*, MruFile*>::iterator iter;
      for (iter = mMruFileCommands.begin(); iter != mMruFileCommands.end(); ++iter)
      {
         QAction* pAction = iter.key();
         if (pAction != NULL)
         {
            m_pFile->removeAction(pAction);
         }
      }

      mMruFileCommands.clear();
   }

   // Add the current MRU file commands
   if (mruFiles.empty() == true)
   {
      m_pFile->insertAction(mpMruInsertAction, mpRecentFileAction);
   }
   else
   {
      for (vector<MruFile*>::size_type i = 0; i < mruFiles.size(); ++i)
      {
         MruFile* pMruFile = mruFiles[i];
         if (NN(pMruFile))
         {
            QString strName = QString::fromStdString(pMruFile->getName());
            QString strCommand = "&" + QString::number(i + 1) + "  " + strName;

            QAction* pMruAction = new QAction(strCommand, this);
            m_pFile->insertAction(mpMruInsertAction, pMruAction);

            mMruFileCommands[pMruAction] = pMruFile;
         }
      }
   }
}

void ApplicationWindow::fileMenuActivated(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   // Check if the action is an MRU file action
   QMap<QAction*, MruFile*>::iterator mruIter = mMruFileCommands.find(pAction);
   if (mruIter == mMruFileCommands.end())
   {
      return;
   }

   MruFile* pMruFile = mruIter.value();
   if (pMruFile == NULL)
   {
      return;
   }

   const string& mruFileName = pMruFile->getName();
   const vector<ImportDescriptor*>& mruFileDescriptors = pMruFile->getDescriptors();

   // Create the importer resource
   ImporterResource importer(pMruFile->getImporterName(), mruFileName, NULL, false);
   importer->createProgressDialog(true);
   importer->updateMruFileList(true);

   // Check if the file to import has been modified since the MRU file was created
   DateTimeImp modificationTime;
   if (mruFileName.empty() == false)
   {
      FilenameImp filename(mruFileName);
      string filePath = filename.getPath();
      string baseFilename = filename.getFileName();

      FileFinderImp fileFinder;
      fileFinder.findFile(filePath, baseFilename);
      if (fileFinder.findNextFile() == true)
      {
         fileFinder.getLastModificationTime(modificationTime);
      }
   }

   if (modificationTime != pMruFile->getModificationTime())
   {
      ConfigurationSettingsImp* pSettings = ConfigurationSettingsImp::instance();

      if (modificationTime.isValid() == true)
      {
         int button = QMessageBox::question(this, APP_NAME, "The '" + QString::fromStdString(mruFileName) +
            "' file has been modified since it was added to the recent files list.  How do you want to continue?",
            "Import File", "Remove File From List", "Cancel");
         if (button == 1)
         {
            pSettings->removeMruFile(pMruFile);
            return;
         }
         else if (button == 2)
         {
            return;
         }
      }
      else
      {
         QMessageBox::StandardButton button = QMessageBox::question(this, APP_NAME, "The '" +
            QString::fromStdString(mruFileName) + "' file cannot be opened.  Do you want to "
            "remove the file from the recent files list?", QMessageBox::Yes | QMessageBox::No);
         if (button == QMessageBox::Yes)
         {
            pSettings->removeMruFile(pMruFile);
         }

         return;
      }

      // Do not set any import descriptors to cause the resource to use the descriptors obtained by the importer
      importer->setEditType(ImportAgent::ALWAYS_EDIT);
   }
   else
   {
      // Set the import descriptors to load into the importer resource from the MRU file
      vector<ImportDescriptor*> resourceDescriptors;
      Service<ModelServices> pModel;

      Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
      if (modifiers & Qt::ShiftModifier)
      {
         // The import options dialog will be displayed so show all datasets in the file to the user, replacing
         // the appropriate import descriptors from the importer with the descriptors from the MRU file
         Importer* pImporter = dynamic_cast<Importer*>(importer->getPlugIn());
         VERIFYNRV(pImporter != NULL);

         resourceDescriptors = pImporter->getImportDescriptors(mruFileName);

         vector<ImportDescriptor*>::iterator importerIter;
         for (importerIter = resourceDescriptors.begin(); importerIter != resourceDescriptors.end(); ++importerIter)
         {
            if (*importerIter != NULL)
            {
               bool bImported = false;

               for (vector<ImportDescriptor*>::const_iterator mruIter = mruFileDescriptors.begin();
                  mruIter != mruFileDescriptors.end();
                  ++mruIter)
               {
                  if (*mruIter != NULL)
                  {
                     DataDescriptor* pImporterDescriptor = (*importerIter)->getDataDescriptor();
                     DataDescriptor* pMruDescriptor = (*mruIter)->getDataDescriptor();
                     if ((pImporterDescriptor != NULL) && (pMruDescriptor != NULL))
                     {
                        if ((pImporterDescriptor->getName() == pMruDescriptor->getName()) &&
                           (pImporterDescriptor->getType() == pMruDescriptor->getType()))
                        {
                           // Copy the MRU file data descriptor into the importer resource descriptors
                           // since the resource takes ownership
                           DataDescriptor* pNewResourceDataDescriptor = pMruDescriptor->copy();
                           VERIFYNRV(pNewResourceDataDescriptor != NULL);

                           // Set the MRU descriptor into the import descriptor obtained from the importer,
                           // which destroys the data descriptor obtained from the importer
                           (*importerIter)->setDataDescriptor(pNewResourceDataDescriptor);

                           // Import this data set
                           bImported = true;
                           break;
                        }
                     }
                  }
               }

               (*importerIter)->setImported(bImported);
            }
         }
      }
      else
      {
         // Copy the MRU file import descriptors into the resource since the resource takes ownership of them
         for (vector<ImportDescriptor*>::const_iterator iter = mruFileDescriptors.begin();
            iter != mruFileDescriptors.end();
            ++iter)
         {
            const ImportDescriptor* pImportDescriptor = *iter;
            if (pImportDescriptor != NULL)
            {
               DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
               if (pDescriptor != NULL)
               {
                  DataDescriptor* pResourceDescriptor = pDescriptor->copy();
                  if (pResourceDescriptor != NULL)
                  {
                     ImportDescriptor* pResourceImportDescriptor =
                        pModel->createImportDescriptor(pResourceDescriptor, pImportDescriptor->isImported());
                     if (pResourceImportDescriptor != NULL)
                     {
                        resourceDescriptors.push_back(pResourceImportDescriptor);
                     }
                  }
               }
            }
         }
      }

      map<string, vector<ImportDescriptor*> > resourceDatasets;
      resourceDatasets[mruFileName] = resourceDescriptors;

      importer->setDatasets(resourceDatasets);
      importer->setEditType(modifiers & Qt::ShiftModifier ? ImportAgent::ALWAYS_EDIT : ImportAgent::AS_NEEDED_EDIT);
   }

   // Import the MRU file
   importer->execute();
}

void ApplicationWindow::constructWindowMenu()
{
   m_pWindow->clear();

   string context = m_pWindow->menuAction()->toolTip().toStdString();
   mpMenuBar->insertCommand(mpCascadeAction, m_pWindow, context);
   mpMenuBar->insertCommand(mpTileAction, m_pWindow, context);
   mpMenuBar->insertCommand(mpTileHorizontalAction, m_pWindow, context);
   mpMenuBar->insertCommand(mpTileVerticalAction, m_pWindow, context);
   mpMenuBar->insertCommand(mpTileSelectedAction, m_pWindow, context);

   QList<QMdiSubWindow*> windowList = mpWorkspace->subWindowList(); // uses the default of CreationOrder

   unsigned int numWindows = windowList.count();
   if (numWindows > 0)
   {
      m_pWindow->addSeparator();
   }

   for (unsigned int i = 0; i < numWindows; i++)
   {
      WorkspaceWindowImp* pWindow = dynamic_cast<WorkspaceWindowImp*>(windowList.at(i));
      if (pWindow != NULL)
      {
         QString strName = pWindow->windowTitle();
         QString strCommand = "&" + QString::number(i + 1) + "  " + strName;

         QAction* pWindowAction = m_pWindow->addAction(strCommand);
         pWindowAction->setCheckable(true);

         WorkspaceWindowImp* pActiveWindow = dynamic_cast<WorkspaceWindowImp*>(getCurrentWorkspaceWindow());
         pWindowAction->setChecked(pWindow == pActiveWindow);
      }
   }
}

void ApplicationWindow::windowMenuActivated(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   QString strWindowName = pAction->text();
   if (strWindowName.isEmpty() == false)
   {
      int iStartPos = strWindowName.indexOf("  ");
      if (iStartPos != -1)
      {
         strWindowName = strWindowName.mid(iStartPos + 2);
      }

      QList<QMdiSubWindow*> windowList = mpWorkspace->subWindowList(); // uses the default of CreationOrder
      for (int i = 0; i < windowList.count(); ++i)
      {
         WorkspaceWindowImp* pWindow = static_cast<WorkspaceWindowImp*>(windowList.at(i));
         if (pWindow != NULL)
         {
            QString strCurrentName = QString::fromStdString(pWindow->getName());
            if (strCurrentName == strWindowName)
            {
               setCurrentWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(pWindow));
               break;
            }
         }
      }
   }
}

void ApplicationWindow::updateUndoActions(const string& oldId, const string& newId)
{
   UndoStack* pUpdatedStack = dynamic_cast<UndoStack*>(sender());

   QList<QUndoStack*> undoStacks = mpUndoGroup->stacks();
   for (int i = 0; i < undoStacks.count(); ++i)
   {
      UndoStack* pUndoStack = dynamic_cast<UndoStack*>(undoStacks[i]);
      if ((pUndoStack != NULL) && (pUndoStack != pUpdatedStack))
      {
         VERIFYNR(disconnect(pUndoStack, SIGNAL(sessionItemChanged(const std::string&, const std::string&)), this,
            SLOT(updateUndoActions(const std::string&, const std::string&))));

         pUndoStack->updateActions(oldId, newId);

         VERIFYNR(connect(pUndoStack, SIGNAL(sessionItemChanged(const std::string&, const std::string&)), this,
            SLOT(updateUndoActions(const std::string&, const std::string&))));
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// File actions

bool ApplicationWindow::importFile(const QString& strImporterSubtype, Progress* pProgress)
{
   vector<DataElement*> dummy;
   return importFile(strImporterSubtype, pProgress, dummy);
}

bool ApplicationWindow::importFile(const QString& strImporterSubtype, Progress* pProgress,
                                   vector<DataElement*>& importedElements)
{
   importedElements.clear();

   // Create the importer resource
   ImporterResource importer(pProgress, false);
   importer->setImporterSubtype(strImporterSubtype.toStdString());
   importer->createProgressDialog(pProgress == NULL);
   importer->updateMruFileList(true);

   // Import
   bool bSuccess = importer->execute();
   if (bSuccess == true)
   {
      importedElements = importer->getImportedElements();
   }

   return bSuccess;
}

bool ApplicationWindow::exportSessionItem(SessionItem* pItem, FileDescriptor* pNewFileDescriptor, Progress* pProgress)
{
   if (pItem == NULL)
   {
      return false;
   }

   // Create a file descriptor if necessary
   bool bDeleteFileDescriptor = false;
   if (pNewFileDescriptor == NULL)
   {
      DataDescriptor* pDescriptor = NULL;

      DataElement* pElement = dynamic_cast<DataElement*>(pItem);
      if (pElement != NULL)
      {
         pDescriptor = pElement->getDataDescriptor();
      }

      pNewFileDescriptor = RasterUtilities::generateFileDescriptorForExport(pDescriptor, "");
      if (pNewFileDescriptor != NULL)
      {
         bDeleteFileDescriptor = true;
      }
   }

   // Create an exporter resource to ensure that the exporter is unloaded
   ExporterResource pExporter(NULL, pItem, pNewFileDescriptor, pProgress, false);
   pExporter->createProgressDialog(pProgress == NULL);

   ExportDlg exportDlg(pExporter, getAvailableExporters(pItem), this);
   if (exportDlg.exec() == QDialog::Rejected)
   {
      return false;
   }

   if (pExporter->getPlugIn() == NULL)
   {
      return false;
   }

   // Export the item
   bool bSuccess = pExporter->execute();

   // Delete the created file descriptor
   if (bDeleteFileDescriptor == true)
   {
      delete dynamic_cast<FileDescriptorImp*>(pNewFileDescriptor);
   }

   return bSuccess;
}

bool ApplicationWindow::exportSessionItems(const vector<SessionItem*>& items, Progress* pProgress)
{
   if (items.empty() == true)
   {
      return false;
   }

   if (items.size() == 1)
   {
      return exportSessionItem(items.front(), NULL, pProgress);
   }

   // Verify that each session item is of the same type
   string objectType;
   for (vector<SessionItem*>::size_type i = 0; i < items.size(); ++i)
   {
      TypeAwareObject* pTypeObject = dynamic_cast<TypeAwareObject*>(items[i]);
      if (pTypeObject != NULL)
      {
         const string& currentType = pTypeObject->getObjectType();
         if (objectType.empty() == true)
         {
            objectType = currentType;
         }
         else if (currentType != objectType)
         {
            return false;
         }
      }
   }

   // Get the directory, exporter, and default file extension from the user
   ExporterResource pExporter(NULL, pProgress, false);
   const vector<PlugInDescriptor*>& availableExporters = getAvailableExporters(items.front());

   BatchExportDlg dialog(pExporter, availableExporters, this);
   if (dialog.exec() == QDialog::Rejected)
   {
      return false;
   }

   QString directory = dialog.getExportDirectory();
   QString fileExtension = dialog.getFileExtension();

   // Create a progress dialog for the export if necessary
   if (pProgress == NULL)
   {
      string exporterName = dialog.getExporterName().toStdString();

      Service<DesktopServices> pDesktop;
      pDesktop->createProgressDialog(exporterName, pExporter->getProgress());
   }

   // Assign unique filenames for the selected signatures
   bool bSuccess = false;
   bool bOverwrite = false;

   for (vector<SessionItem*>::size_type i = 0; i < items.size(); ++i)
   {
      SessionItem* pItem = items[i];
      if (pItem != NULL)
      {
         // Get the item name
         string name = pItem->getName();
         VERIFY(name.empty() == false);

         // Check if the item name is a file
         QFileInfo fileInfo = QFileInfo(QString::fromStdString(name));
         if (fileInfo.isFile() == true)
         {
            // Use the base name
            name = fileInfo.completeBaseName().toStdString();
         }
         else
         {
            // Replace invalid filename characters
            size_t pos = name.find(":", 0);
            while (pos != string::npos)
            {
               name.replace(pos, 1, "_");
               pos = name.find(":", pos + 1);
            }

            pos = name.find("\\", 0);
            while (pos != string::npos)
            {
               name.replace(pos, 1, "_");
               pos = name.find("\\", pos + 1);
            }

            pos = name.find("/", 0);
            while (pos != string::npos)
            {
               name.replace(pos, 1, "_");
               pos = name.find("/", pos + 1);
            }
         }

         // Get the save filename based on the item name
         QString filename = directory + "/" + QString::fromStdString(name) + "." + fileExtension;
         filename.replace(QRegExp("\\\\"), "/");
         if (filename.isEmpty() == false)
         {
            // Prompt for overwrite
            QFileInfo fileInfo(filename);
            if (fileInfo.exists() == true)
            {
               if (bOverwrite == false)
               {
                  int iReplace = QMessageBox::warning(this, windowTitle(), filename +
                     " already exists.\nDo you want to replace it?",
                     QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No);
                  if (iReplace == QMessageBox::YesToAll)
                  {
                     bOverwrite = true;
                  }
                  else if (iReplace == QMessageBox::No)
                  {
                     if (pProgress != NULL)
                     {
                        QString strError = "Warning: The " + filename + " file was not saved.";
                        pProgress->updateProgress(strError.toStdString(), 0, WARNING);
                     }

                     continue;
                  }
               }
            }

            // Create a file descriptor for the export
            FileDescriptor* pFileDescriptor = NULL;

            DataElement* pElement = dynamic_cast<DataElement*>(pItem);
            if (pElement != NULL)
            {
               DataDescriptor* pDescriptor = pElement->getDataDescriptor();
               if (pDescriptor != NULL)
               {
                  pFileDescriptor = RasterUtilities::generateFileDescriptorForExport(pDescriptor,
                     filename.toStdString());
               }
            }

            if (pFileDescriptor == NULL)
            {
               FactoryResource<FileDescriptor> pFactoryDescriptor;
               pFactoryDescriptor->setFilename(filename.toStdString());

               pFileDescriptor = pFactoryDescriptor.release();
            }

            // Export the data
            pExporter->setItem(pItem);
            pExporter->setFileDescriptor(pFileDescriptor);

            if (pExporter->execute() == true)
            {
               bSuccess = true;
            }

            // Destroy the file descriptor
            delete dynamic_cast<FileDescriptorImp*>(pFileDescriptor);
         }
      }
   }

   return bSuccess;
}

void ApplicationWindow::closeWorkspaceWindow()
{
   Window* pWindow = dynamic_cast<Window*>(getCurrentWorkspaceWindow());
   if (pWindow != NULL)
   {
      deleteWindow(pWindow);
   }
}

void ApplicationWindow::printSetup()
{
   WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      pWindow->print(true);
   }
}

void ApplicationWindow::print()
{
   WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      pWindow->print(false);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Session actions

void ApplicationWindow::openSession()
{
   QString pathStr;
   const Filename* pSessionDir = ConfigurationSettings::getSettingSaveOpenSessionPath();
   if (pSessionDir != NULL)
   {
      pathStr = QString::fromStdString(pSessionDir->getFullPathAndName());
   }
   QString filename = QFileDialog::getOpenFileName(this, "Open Session File", 
      pathStr, "Session Files (*.session)");
   if (filename.isEmpty() == false)
   {
      FactoryResource<Filename> pDir;
      pDir->setFullPathAndName(filename.toStdString());
      pDir->setFullPathAndName(pDir->getPath());
      Service<ConfigurationSettings>()->setTemporarySetting(
         ConfigurationSettings::getSettingSaveOpenSessionPathKey(), *pDir.get());
      openSession(filename);
   }
}

void ApplicationWindow::openSession(const QString& filename)
{
   if (filename.isEmpty() == false)
   {
      if (filename.toStdString() == mSessionFilename)
      {
         QString msg = "You have selected to open the current session file, which "
            "will lose all changes since the session was last saved.  Do you want to "
            "open the session file?";
         if (QMessageBox::question(this, "Re-opening Current Session", msg, 
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
         {
            return;
         }
      }
      else
      {
         SessionSaveType eSave = SessionManager::getSettingQueryForSave();
         int buttonVal = (eSave == SESSION_AUTO_SAVE ? QMessageBox::Yes : QMessageBox::No);
         if (eSave == SESSION_QUERY_SAVE)
         {
            buttonVal = QMessageBox::question(this, "Save Session", 
               "Do you want to save the current session before closing it?", 
               QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 
               QMessageBox::Cancel | QMessageBox::Escape);
         }
         if (buttonVal == QMessageBox::Yes)
         {
            if (!saveSession()) // user pressed Cancel in response to Save As, or save failed
            {
               return;
            }
         }
         else if (buttonVal == QMessageBox::Cancel)
         {
            return;
         }
      }

      ProgressAdapter progress;
      Service<DesktopServices>()->createProgressDialog("Open Session", &progress);
      mSessionFilename.clear();
      if (SessionManagerImp::instance()->open(filename.toStdString(), &progress))
      {
         mSessionFilename = filename.toStdString();
      }
   }
}

bool ApplicationWindow::newSession()
{
   SessionSaveType eSave = SessionManager::getSettingQueryForSave();
   int buttonVal = (eSave == SESSION_AUTO_SAVE ? QMessageBox::Yes : QMessageBox::No);
   if (eSave == SESSION_QUERY_SAVE)
   {
         buttonVal = QMessageBox::question(this, "Close Session", 
            "Do you want to save the session before closing it?", 
            QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 
            QMessageBox::Cancel | QMessageBox::Escape);
   }
   if (buttonVal == QMessageBox::Yes)
   {
      if (!saveSession()) // user pressed Cancel in response to Save As, or save failed
      {
         return false;
      }
   }
   if (buttonVal != QMessageBox::Cancel)
   {
      QCursor currentCursor = cursor();
      QCursor waitCursor(Qt::WaitCursor);
      setCursor(waitCursor);
      QApplication::processEvents();
      SessionManagerImp *pManagerImp = SessionManagerImp::instance();
      pManagerImp->newSession();
      setCursor(currentCursor);
      mSessionFilename.clear();
      return true;
   }
   return false;
}

void ApplicationWindow::autoSaveSession()
{
   Service<SessionManager> pManager;
   if(!pManager->isSessionSaveLocked())
   {
      string filename;

#ifdef WIN_API
      const char *pBaseDir = getenv("USERPROFILE");
#else
      const char *pBaseDir = getenv("HOME");
#endif

      if (pBaseDir == NULL)
      {
         MessageResource pMessage("Session Auto-save Error",
            "app",
            "A227E6A5-65B8-4689-B259-AE991F1CEEA7");
         pMessage->addProperty("Message", "Unable to detect the default user directory");
         return;
      }

      string dirName = pBaseDir + SLASH + string(APP_NAME);
      QDir dir(dirName.c_str());
      if (dir.exists() == false)
      {
         bool dirMade = dir.mkpath(QString::fromStdString(dirName));
         if (!dirMade)
         {
            MessageResource pMessage("Session Auto-save Error",
               "app",
               "FEBC9041-EA1D-4607-AC21-474984ACE4DF");
            pMessage->addProperty("Message", "Unable to create directory");
            return;
         }
      }

      filename = dirName + "/AutoSave.session";

      ProgressAdapter progress;
      Service<DesktopServices>()->createProgressDialog("Save Session", &progress);
      if(pManager->serialize(filename, &progress).first == SessionManager::LOCKED)
      {
         // temporarily set timer to 1 minutes until we successfully save.
         mAutoTimerRetryOnLock = true;
         setAutoSaveTimerInterval(1);
      }
      else if(mAutoTimerRetryOnLock)
      {
         // reset the auto save timer when session save is no longer locked
         mAutoTimerRetryOnLock = false;
         setAutoSaveTimerInterval(SessionManager::getSettingAutoSaveInterval());
      }
   }
   mpSaveTimer->start();
}

bool ApplicationWindow::saveSession()
{
   Service<SessionManager> pManager;
   if (mSessionFilename.empty() == true)
   {
      return saveSessionAs();
   }
   else
   {
      ProgressAdapter progress;
      Service<DesktopServices>()->createProgressDialog("Save Session", &progress);
      pair<SessionManager::SerializationStatus,vector<pair<SessionItem*, string> > > status =
         pManager->serialize(mSessionFilename, &progress);
      if (status.first == SessionManager::FAILURE) 
      {
         progress.updateProgress("Session saving failed.", 0, ERRORS);
         return false;
      }
      else if (status.first == SessionManager::LOCKED)
      {
         progress.updateProgress("Session saving is temporarily locked.", 0, ERRORS);
         return false;
      }
      Service<MessageLogMgr> pLogMgr;
      MessageLog *pLog = pLogMgr->getLog();
      if (pLog != NULL && status.first == SessionManager::SUCCESS)
      {
         pLog->createMessage("Session saved: " + mSessionFilename,"app","7AD6D0B4-08E4-4556-A20D-2595B797B4F3");
      }
      else if(pLog != NULL && status.first == SessionManager::PARTIAL_SUCCESS)
      {
         pLog->createMessage("Session saved. Not all session items were successfully loaded: " +
            mSessionFilename, "app", "C85E14F3-69B0-4495-AD91-F3E1B7013A8E");
      }
      return true;
   }
   return false;
}

bool ApplicationWindow::saveSessionAs()
{
   QString initial = QString::fromStdString(mSessionFilename);
   if (initial.isEmpty())
   {
      QString pathStr;
      const Filename* pSessionDir = ConfigurationSettings::getSettingSaveOpenSessionPath();
      if (pSessionDir != NULL)
      {
         pathStr = QString::fromStdString(pSessionDir->getFullPathAndName());
      }
      initial = pathStr + "/" + QString(APP_NAME) + ".session";
   }
   for (;;)
   {
      QString filename = QFileDialog::getSaveFileName(this, "Save Session As...", initial,
         "Session Files (*.session)", NULL, QFileDialog::DontConfirmOverwrite);
      if (filename.isEmpty()) // user pressed Cancel button
      {
         return false;
      }

      QFileInfo fileInfo = QFileInfo(filename);
      if (fileInfo.suffix() != "session")
      {
         filename += ".session";
      }

      FileResource file(filename.toStdString().c_str(), "r");
      if (file.get() != NULL)
      {
         int button = QMessageBox::question(this, "Save Session As", "The file already exists.  "
            "Do you wish to overwrite it?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
            QMessageBox::Cancel | QMessageBox::Escape);
         if (button == QMessageBox::Cancel)
         {
            return false;
         }
         else if (button == QMessageBox::No)
         {
            initial = filename;
            continue;
         }
      }

      FactoryResource<Filename> pDir;
      pDir->setFullPathAndName(filename.toStdString());
      pDir->setFullPathAndName(pDir->getPath());
      Service<ConfigurationSettings>()->setTemporarySetting(
         ConfigurationSettings::getSettingSaveOpenSessionPathKey(), *pDir.get());
      mSessionFilename = filename.toStdString();
      return saveSession();
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Edit actions

void ApplicationWindow::cut()
{
   GraphicLayer* pLayer = getClipboardLayer();
   if (pLayer != NULL)
   {
      copy();
      pLayer->deleteSelectedObjects();
   }
}

void ApplicationWindow::copy()
{
   GraphicLayer* pLayer = getClipboardLayer();
   if (pLayer == NULL)
   {
      return;
   }

   // Delete all objects currently on the clipboard
   mClipboard.removeAllObjects(true);

   // Copy the selected objects from the layer to the clipboard
   list<GraphicObject*> selectedObjects;
   pLayer->getSelectedObjects(selectedObjects);

   for (list<GraphicObject*>::iterator iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         GraphicObjectType objectType = pObject->getGraphicObjectType();

         GraphicObject* pNewObject = mClipboard.addObject(objectType);
         if (pNewObject != NULL)
         {
            GraphicObjectImp* pNewObjectImp = dynamic_cast<GraphicObjectImp*>(pNewObject);
            if (pNewObjectImp != NULL)
            {
               pNewObjectImp->replicateObject(pObject);
            }
         }
      }
   }
}

void ApplicationWindow::paste()
{
   GraphicLayer* pLayer = getClipboardLayer();
   if (pLayer == NULL)
   {
      return;
   }

   UndoGroup group(pLayer->getView(), "Add Graphic Objects");

   // Deselect layer objects
   pLayer->deselectAllObjects();

   // Copy the objects from the clipboard to the layer
   list<GraphicObject*> objectsToSelect;

   const list<GraphicObject*>& objects = mClipboard.getObjects();
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         GraphicObjectType objectType = pObject->getGraphicObjectType();

         GraphicObject* pNewObject = pLayer->addObject(objectType);
         if (pNewObject != NULL)
         {
            GraphicObjectImp* pNewObjectImp = dynamic_cast<GraphicObjectImp*>(pNewObject);
            if (pNewObjectImp != NULL)
            {
               pNewObjectImp->replicateObject(pObject);
            }

            // Select objects later since replicating a group object deselects all objects in the layer
            objectsToSelect.push_back(pNewObject);
         }
      }
   }

   // Select the new layer objects
   for_each(objectsToSelect.begin(), objectsToSelect.end(), boost::bind(&GraphicLayer::selectObject, pLayer, _1));
}

void ApplicationWindow::clearUndoStacks()
{
   if (mClearingUndoStacks)
   {
      return;
   }

   mClearingUndoStacks = true;
   for (vector<Window*>::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter)
   {
      ViewWindow* pViewWindow = dynamic_cast<ViewWindow*>(*iter);
      if (pViewWindow != NULL)
      {
         View* pView = pViewWindow->getView();
         if (pView != NULL)
         {
            pView->clearUndo();
         }
      }
   }

   mClearingUndoStacks = false;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Status Bar action

void ApplicationWindow::showStatusBar(bool bShow)
{
   if (bShow == true)
   {
      m_pStatus_Bar->show();
   }
   else
   {
      m_pStatus_Bar->hide();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// View action

void ApplicationWindow::setMouseMode(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   // Update the mouse mode in all open windows
   vector<Window*> windows = getWindows();
   for (unsigned int i = 0; i < windows.size(); i++)
   {
      Window* pWindow = NULL;
      pWindow = windows.at(i);
      if (pWindow != NULL)
      {
         if (pWindow->isKindOf("WorkspaceWindow") == true)
         {
            View* pView = NULL;
            pView = ((WorkspaceWindow*) pWindow)->getView();
            if (pView != NULL)
            {
               if (pAction == m_pNo_View_Mode_Action)
               {
                  setMouseMode(pView, QString());
               }
               else if (pAction == mpLayerEditAction)
               {
                  setMouseMode(pView, "LayerMode");
               }
               else if (pAction == m_pMeasurement_Edit_Action)
               {
                  setMouseMode(pView, "MeasurementMode");
               }
               else if (pAction == m_pPan_Action)
               {
                  setMouseMode(pView, "PanMode");
               }
               else if (pAction == m_pFree_Rotate_Action)
               {
                  setMouseMode(pView, "RotateMode");
               }
               else if (pAction == m_pZoom_Point_In_Action)
               {
                  setMouseMode(pView, "ZoomInMode");
               }
               else if (pAction == m_pZoom_Point_Out_Action)
               {
                  setMouseMode(pView, "ZoomOutMode");
               }
               else if (pAction == m_pZoom_Rect_Action)
               {
                  setMouseMode(pView, "ZoomBoxMode");
               }
               else
               {
                  vector<MouseMode*> mouseModes;
                  pView->getMouseModes(mouseModes);

                  vector<MouseMode*>::iterator iter = mouseModes.begin();
                  while (iter != mouseModes.end())
                  {
                     MouseModeImp* pMouseMode = static_cast<MouseModeImp*> (*iter);
                     if (pMouseMode != NULL)
                     {
                        QAction* pCurrentAction = pMouseMode->getAction();
                        if (pCurrentAction == pAction)
                        {
                           QString strModeName = pMouseMode->getName();
                           if (strModeName.isEmpty() == false)
                           {
                              setMouseMode(pView, strModeName);
                              break;
                           }
                        }
                     }

                     ++iter;
                  }
               }
            }
         }
      }
   }

   // Enable the toolbars
   if (pAction == m_pNo_View_Mode_Action)
   {
      enableToolBars(true);
   }
   else
   {
      Layer* pActiveLayer = NULL;

      WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
      if (pWindow != NULL)
      {
         SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pWindow->getActiveView());
         if (pSpatialDataView != NULL)
         {
            pActiveLayer = pSpatialDataView->getActiveLayer();
         }

         ProductView* pProductView = dynamic_cast<ProductView*>(pWindow->getActiveView());
         if (pProductView != NULL)
         {
            pActiveLayer = pProductView->getActiveLayer();
         }
      }

      initializeToolBars(pActiveLayer);
   }
}

void ApplicationWindow::setMouseMode(View* pView, const QString& strMouseMode)
{
   if (pView == NULL)
   {
      return;
   }

   // Set the mouse mode in the view
   if (strMouseMode.isEmpty() == false)
   {
      pView->setMouseMode(strMouseMode.toStdString());
   }
   else
   {
      pView->setMouseMode(NULL);
   }

   // Set the mouse mode in each view object for a product view
   if (pView->isKindOf("ProductView") == true)
   {
      GraphicLayer* pLayoutLayer = NULL;
      pLayoutLayer = ((ProductView*) pView)->getLayoutLayer();
      if (pLayoutLayer != NULL)
      {
         list<GraphicObject*> objects;
         pLayoutLayer->getObjects(objects);

         list<GraphicObject*>::iterator iter = objects.begin();
         while (iter != objects.end())
         {
            GraphicObject* pObject = NULL;
            pObject = *iter;
            if (pObject != NULL)
            {
               View* pObjectView = NULL;
               pObjectView = pObject->getObjectView();
               if (pObjectView != NULL)
               {
                  setMouseMode(pObjectView, strMouseMode);
               }
            }

            ++iter;
         }
      }
   }
}

void ApplicationWindow::addMouseModeToGroup(const MouseMode* pMouseMode)
{
   if (pMouseMode == NULL)
   {
      return;
   }

   QAction* pAction = pMouseMode->getAction();
   if (pAction != NULL)
   {
      m_pView_Mode_Group->addAction(pAction);
      pAction->setCheckable(true);
   }
}

void ApplicationWindow::updateMouseAction(const MouseMode* pMouseMode)
{
   string modeName = "";
   QAction* pAction = NULL;

   if (pMouseMode != NULL)
   {
      pMouseMode->getName(modeName);
      pAction = pMouseMode->getAction();
   }

   if (modeName == "LayerMode")
   {
      mpLayerEditAction->trigger();
   }
   else if (modeName == "MeasurementMode")
   {
      m_pMeasurement_Edit_Action->trigger();
   }
   else if (modeName == "PanMode")
   {
      m_pPan_Action->trigger();
   }
   else if (modeName == "RotateMode")
   {
      m_pFree_Rotate_Action->trigger();
   }
   else if (modeName == "ZoomInMode")
   {
      m_pZoom_Point_In_Action->trigger();
   }
   else if (modeName == "ZoomOutMode")
   {
      m_pZoom_Point_Out_Action->trigger();
   }
   else if (modeName == "ZoomBoxMode")
   {
      m_pZoom_Rect_Action->trigger();
   }
   else if (pAction != NULL)
   {
      pAction->trigger();
   }
   else
   {
      m_pNo_View_Mode_Action->trigger();
   }
}

void ApplicationWindow::refresh()
{
   WorkspaceWindow* pWindow = NULL;
   pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      View* pView = NULL;
      pView = pWindow->getView();
      if (pView != NULL)
      {
         if (pView->isKindOf("ProductView") == true)
         {
            View* pEditView = NULL;
            pEditView = ((ProductView*) pView)->getActiveEditView();
            if (pEditView != NULL)
            {
               pView = pEditView;
            }
         }

         pView->refresh();
      }
   }
}

void ApplicationWindow::resetStretch()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*> (getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->resetStretch();
   }
}

void ApplicationWindow::generateFullImage()
{
   SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(getCurrentWorkspaceWindowView());
   if (pSpatialDataView != NULL)
   {
      pSpatialDataView->generateFullImage();
   }
}

void ApplicationWindow::toggleDisplayMode()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*> (getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      RasterLayer* pLayer = static_cast<RasterLayer*> (pView->getTopMostLayer(RASTER));
      if (pLayer != NULL)
      {
         pLayer->toggleDisplayMode();
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Rotate actions

void ApplicationWindow::rotateLeft()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->rotateBy(-90.0);
      pView->refresh();
   }
}

void ApplicationWindow::rotateRight()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->rotateBy(90.0);
      pView->refresh();
   }
}

void ApplicationWindow::flipHoriz()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->flipHorizontal();
      pView->refresh();
   }
}

void ApplicationWindow::flipVert()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->flipVertical();
      pView->refresh();
   }
}

void ApplicationWindow::rotateBy()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      bool bSuccess = false;

      double dAngle = QInputDialog::getDouble(this, "Rotate By", "Enter the number of degrees by which to rotate "
         "the view from -360 to 360:", 0.0, -360.0, 360.0, 0, &bSuccess);
      if (bSuccess == true)
      {
         pView->rotateBy(dAngle);
         pView->refresh();
      }
   }
}

void ApplicationWindow::northUp()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getCurrentWorkspaceWindowView());
   if (pView == NULL)
   {
      QMessageBox::warning(this, "Error", "No view available");
      return;
   }

   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList == NULL)
   {
      QMessageBox::warning(this, "Error", "No layer list available");
      return;
   }

   double angle;
   if (GeoAlgorithms::getAngleToNorth(pLayerList->getPrimaryRasterElement(),pView, angle) == false)
   {
      QMessageBox::warning(this, "Error", "No georeferencing information available");
      return;
   }

   UndoGroup group(pView, "North Up");
   pView->rotateTo(angle);
   pView->refresh();
}

void ApplicationWindow::sensorUp()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getCurrentWorkspaceWindowView());
   if (pView == NULL)
   {
      QMessageBox::warning(this, "Error", "No view available");
      return;
   }

   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList == NULL)
   {
      QMessageBox::warning(this, "Error", "No layer list available");
      return;
   }

   RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
   VERIFYNRV(pRasterElement != NULL);

   DynamicObject* pMetadata = pRasterElement->getMetadata();
   VERIFYNRV(pMetadata != NULL);

   double* pAngle = dv_cast<double>(&pMetadata->getAttributeByPath(SENSOR_UP_ANGLE_METADATA_PATH));
   if (pAngle == NULL)
   {
      QMessageBox::warning(this, "Error", "Metadata does not contain sensor up information");
      return;
   }

   UndoGroup group(pView, "Sensor Up");
   pView->rotateTo(pView->getPitch() > 0 ? *pAngle + 180.0 : *pAngle); // Reverse if the image is flipped
   pView->refresh();
}

void ApplicationWindow::reset()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->resetOrientation();
      pView->refresh();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Zoom actions

void ApplicationWindow::zoomIn()
{
   PerspectiveViewImp* pView = dynamic_cast<PerspectiveViewImp*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      if (pView->isInsetEnabled() == true)
      {
         pView->zoomInset(true);
      }
      else
      {
         double dZoom = pView->getZoomPercentage() * 1.25;
         pView->zoomTo(dZoom);
      }

      pView->refresh();
   }
}

void ApplicationWindow::zoomOut()
{
   PerspectiveViewImp* pView = dynamic_cast<PerspectiveViewImp*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      if (pView->isInsetEnabled() == true)
      {
         pView->zoomInset(false);
      }
      else
      {
         double dZoom = pView->getZoomPercentage() / 1.25;
         pView->zoomTo(dZoom);
      }

      pView->refresh();
   }
}

void ApplicationWindow::zoom10()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(10.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom50()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(50.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom75()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(75.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom100()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(100.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom150()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(150.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom200()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(200.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom400()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(400.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoom800()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(800.0);
      pView->refresh();
   }
}

void ApplicationWindow::zoomToFit()
{
   View* pView = getCurrentWorkspaceWindowView();
   if (pView != NULL)
   {
      pView->zoomExtents();
      pView->refresh();
   }
}

void ApplicationWindow::zoomTo(double dZoom)
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      pView->zoomTo(dZoom);
      pView->refresh();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Properties actions

void ApplicationWindow::viewProperties()
{
   View* pView = getCurrentWorkspaceWindowView();
   if (pView != NULL)
   {
      displayProperties(pView);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Tools actions

void ApplicationWindow::showOverviewWindow(bool bShow)
{
   SpatialDataWindow *pWindow = dynamic_cast<SpatialDataWindow*>(getCurrentWorkspaceWindow());
   if (pWindow != NULL)
   {
      pWindow->showOverviewWindow(bShow);
   }
}

void ApplicationWindow::showChippingWindow()
{
   WorkspaceWindow* pWindow = NULL;
   pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      if (pWindow->getWindowType() == SPATIAL_DATA_WINDOW)
      {
         ((SpatialDataWindow*) pWindow)->exportSubset();
      }
   }
}

void ApplicationWindow::showGcpEditor(bool bShow)
{
   if (m_pGCP_Editor_Action->isChecked() != bShow)
   {
      m_pGCP_Editor_Action->setChecked(bShow);
   }

   if (mpGcpEditor != NULL)
   {
      if (bShow == false)
      {
         mpGcpEditor->close();
      }
      else
      {
         mpGcpEditor->show();
      }
   }
}

void ApplicationWindow::showTiePointEditor(bool bShow)
{
   if (m_pTiePoint_Editor_Action->isChecked() != bShow)
   {
      m_pTiePoint_Editor_Action->setChecked(bShow);
   }

   if (mpTiePointEditor != NULL)
   {
      if (bShow == false)
      {
         mpTiePointEditor->close();
      }
      else
      {
         mpTiePointEditor->show();
      }
   }
}

void ApplicationWindow::showWizardBuilder()
{
   WizardBuilder* pWizard = new WizardBuilder();
   if (pWizard != NULL)
   {
      pWizard->show();
   }
}

void ApplicationWindow::showBatchEditor()
{
   BatchEditorDlg* pEditor = new BatchEditorDlg();
   if (pEditor != NULL)
   {
      pEditor->show();
   }
}

void ApplicationWindow::showZapDlg()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getCurrentWorkspaceWindowView());
   if (pView == NULL)
   {
      return;
   }

   GeocoordType geocoordType = GEOCOORD_GENERAL;
   RasterElement *pRaster = NULL;

   SpatialDataView *pSpatialView = dynamic_cast<SpatialDataView*>(pView);
   if (pSpatialView != NULL)
   {
      LayerList *pLayerList = pSpatialView->getLayerList();
      VERIFYNRV(pLayerList != NULL);
      pRaster = pLayerList->getPrimaryRasterElement();

      // get the geocoord layer, even if it is not visible.
      LatLonLayer* pLatLonLayer = NULL;
      std::vector<Layer*> layerList;
      pSpatialView->getLayerList()->getLayers(LAT_LONG, layerList);
      for (unsigned int i = 0; i < layerList.size(); ++i)
      {
         if (dynamic_cast<RasterElement*>(layerList[i]->getDataElement()) == pRaster)
         {
            pLatLonLayer = dynamic_cast<LatLonLayer*>(layerList[i]);
            if (pLatLonLayer != NULL)
            {
               break;
            }
         }
      }

      if (pLatLonLayer != NULL)
      {
         geocoordType = pLatLonLayer->getGeocoordType();
      }
      else
      {
         if (pRaster->isGeoreferenced())
         {
            geocoordType = LatLonLayer::getSettingGeocoordType();
         }
      }
   }

   ZoomAndPanToPointDlg zapDlg(pRaster, geocoordType, this);
   zapDlg.setZoomPct(pView->getZoomPercentage());

   int retVal = zapDlg.exec(); // show the dialog
   if (retVal == QDialog::Accepted)
   {
      LocationType pixel = zapDlg.getCenter();
      double dZoomPercent = zapDlg.getZoomPct();

      pView->zoomToPoint(pixel, dZoomPercent);
      pView->refresh();
   }
}

void ApplicationWindow::registerPlugIns()
{
   // Get the plug-in directory from the options
   Service<ConfigurationSettings> pSettings;

   string plugInDir = pSettings->getPlugInPath();
   if (plugInDir.empty())
   {
      plugInDir = QDir::currentPath().toStdString();
   }

   // Build the plug-in list
   PlugInManagerServicesImp::instance()->buildPlugInList(plugInDir);

   // Update the menus
   updatePlugInCommands();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Options actions

void ApplicationWindow::invokeOptionsDlg()
{
   OptionsDlg dlgOptions(this);
   dlgOptions.exec();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Window actions

void ApplicationWindow::linkWindows()
{
   LinkDlg dlgLink(this);
   dlgLink.exec();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Help actions

void ApplicationWindow::helpTopics()
{
   Service<DesktopServices> pDesktop;
   bool bSuccess = pDesktop->displayHelpHome();

   if (bSuccess == false)
   {
      QMessageBox::critical(this, QString::fromStdString(APP_NAME), "Unable to open the online help files!");
   }
}

void ApplicationWindow::displayExtensions()
{
   ExtensionListDialog dlg(this);
   dlg.exec();
}

void ApplicationWindow::aboutApp()
{
   AboutDlg dlgAbout(this);
   dlgAbout.exec();
}

void ApplicationWindow::displayPlugInHelp()
{
   QAction* pSender = dynamic_cast<QAction*>(sender());
   bool bSuccess = false;
   if (pSender != NULL)
   {
      QVariant data = pSender->data();
      QMap<QString, QVariant> mapData = data.toMap();
      if (mapData.contains("helpWebpage"))
      {
         string webpage = mapData["helpWebpage"].toString().toStdString();
         Service<DesktopServices> pDesktop;
         bSuccess = pDesktop->displayHelp(webpage);
      }
   }
   if (!bSuccess)
   {
      QMessageBox::critical(this, QString::fromStdString(APP_NAME), "Unable to open the online help files!");
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Menu bars

void ApplicationWindow::updatePlugInCommands()
{
   // Remove the current commands from the menu bars
   removeMenuCommands(mPlugInCommands);
   mPlugInCommands.clear();

   // Get the list of plug-ins from the plug-in manager
   Service<PlugInManagerServices> pManager;
   vector<PlugInDescriptor*> plugInDesc = pManager->getPlugInDescriptors();

   QMap<QString, QString> importerSubTypes;
   QMap<QString, QString> exporterSubTypes;

   // Add the commands to the menus
   vector<PlugInDescriptor*>::iterator plugIn;
   for (plugIn = plugInDesc.begin(); plugIn != plugInDesc.end(); plugIn++)
   {
      PlugInDescriptor* pDescriptor = *plugIn;
      if (pDescriptor == NULL)
      {
         continue;
      }
      QIcon plugInIcon;
      vector<string> locations;

      const QIcon icon = pDescriptor->getIcon();
      if (icon.isNull() == false)
      {
         QPixmap pix = icon.pixmap(QSize(16,16));
         pix.setMask(pix.createHeuristicMask());
         plugInIcon.addPixmap(pix);
      }

      locations = pDescriptor->getMenuLocations();

      // if this is an importer or exporter, add the subtype to the list
      if((pDescriptor->getType() == PlugInManagerServices::ImporterType()) && (pDescriptor->getSubtype() != "Auto"))
      {
         string subtype = pDescriptor->getSubtype();
         importerSubTypes.insert(QString::fromStdString(subtype),
                                 QString::fromStdString(pDescriptor->getName()));
      }
      else if((pDescriptor->getType() == PlugInManagerServices::ExporterType()) && (pDescriptor->getSubtype() != "Auto"))
      {
         string subtype = pDescriptor->getSubtype();
         exporterSubTypes.insert(QString::fromStdString(subtype),
                                 QString::fromStdString(pDescriptor->getName()));
      }
      vector<string>::iterator locationIter;
      for (locationIter = locations.begin(); locationIter != locations.end(); ++locationIter)
      {
         QString strLocation = QString::fromStdString(*locationIter);
         strLocation.replace(QRegExp("/"), "\\");

         // Get a pointer to the menu bar
         MenuBarImp* pMenuBar = NULL;

         int iPos = strLocation.indexOf("]");
         if (iPos == -1)
         {
            pMenuBar = mpMenuBar;
         }
         else
         {
            QString strToolBar = strLocation.left(iPos);
            strLocation = strLocation.mid(iPos + 1);

            int iStartPos = strToolBar.indexOf("[");
            if (iStartPos != -1)
            {
               strToolBar = strToolBar.mid(iStartPos + 1);
            }

            int iMenuPos = strLocation.indexOf("\\");
            if (iMenuPos == 0)
            {
               strLocation.prepend(strToolBar);
            }

            ToolBar* pToolBar = static_cast<ToolBar*>(getWindow(strToolBar, TOOLBAR));
            if (pToolBar == NULL)
            {
               pToolBar = static_cast<ToolBar*>(createWindow(strToolBar, TOOLBAR));
               if (pToolBar != NULL)
               {
                  addWindow(pToolBar);
               }
            }

            if (pToolBar != NULL)
            {
               pMenuBar = static_cast<MenuBarImp*>(pToolBar->getMenuBar());
            }
         }

         if (pMenuBar != NULL)
         {
            QAction* pAction = pMenuBar->addCommand(strLocation.toStdString(), pDescriptor->getName());
            if (pAction != NULL)
            {
               QMap<QString, QVariant> actionData = pAction->data().toMap();
               actionData.insert("PlugIn", QVariant(QString::fromStdString(pDescriptor->getName())));

               pAction->setData(QVariant(actionData));
               pAction->setIcon(plugInIcon);

               mPlugInCommands.append(pAction);
            }
         }
      }
   }
}

void ApplicationWindow::updateWizardCommands()
{
   // Remove the current commands from the menu bars
   removeMenuCommands(mWizardCommands);
   mWizardCommands.clear();

   // Get the wizard directories
   QStringList wizardDirs;

   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      QString wizardDir = QString::fromStdString(pSupportFilesPath->getFullPathAndName() + "/Wizards");
      wizardDirs.append(wizardDir);
   }

   const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
   if (pWizardPath != NULL)
   {
      QString wizardDir = QString::fromStdString(pWizardPath->getFullPathAndName());
      wizardDirs.append(wizardDir);
   }

   if (wizardDirs.isEmpty())
   {
      wizardDirs.append(QDir::currentPath());
   }

   // Get the wizard filenames
   QStringList strlWizardFiles;
   for (int i = 0; i < wizardDirs.count(); ++i)
   {
      QString wizardDir = wizardDirs[i];
      if (wizardDir.isEmpty() == false)
      {
         QDir dirWizards = QDir(wizardDir, "*.wiz");
         for (unsigned int j = 0; j < dirWizards.count(); ++j)
         {
            QString strFilename = wizardDir + "/";
            strFilename += dirWizards[j];
            strlWizardFiles.append(strFilename);
         }
      }
   }

   // Add the commands to the menus
   for (int i = 0; i < strlWizardFiles.count(); ++i)
   {
      QString strFilename = strlWizardFiles[i];

      FactoryResource<WizardObject> pWizard(WizardUtilities::readWizard(strFilename.toStdString()));
      if (pWizard.get() == NULL)
      {
         continue;
      }

      string location = pWizard->getMenuLocation();
      if (location.empty() == false)
      {
         QString strLocation = QString::fromStdString(location);
         strLocation.replace(QRegExp("/"), "\\");

         // Get a pointer to the menu bar
         MenuBar* pMenuBar = NULL;

         int iPos = strLocation.indexOf("]");
         if (iPos == -1)
         {
            pMenuBar = mpMenuBar;
         }
         else
         {
            QString strToolBar = strLocation.left(iPos);
            strLocation = strLocation.mid(iPos + 1);

            int iStartPos = strToolBar.indexOf("[");
            if (iStartPos != -1)
            {
               strToolBar = strToolBar.mid(iStartPos + 1);
            }

            int iMenuPos = strLocation.indexOf("\\");
            if (iMenuPos == 0)
            {
               strLocation.prepend(strToolBar);
            }

            ToolBar* pToolBar = static_cast<ToolBar*>(getWindow(strToolBar, TOOLBAR));
            if (pToolBar == NULL)
            {
               pToolBar = static_cast<ToolBar*>(createWindow(strToolBar, TOOLBAR));
               if (pToolBar != NULL)
               {
                  addWindow(pToolBar);
               }
            }

            if (pToolBar != NULL)
            {
               pMenuBar = pToolBar->getMenuBar();
            }
         }

         if (pMenuBar != NULL)
         {
            QAction* pAction = pMenuBar->addCommand(strLocation.toStdString(), "Wizards");
            if (pAction != NULL)
            {
               QMap<QString, QVariant> actionData = pAction->data().toMap();
               actionData.insert("WizardFile", QVariant(strFilename));

               pAction->setData(QVariant(actionData));

               mWizardCommands.append(pAction);
            }
         }
      }
   }
}

void ApplicationWindow::executeCommand(QAction* pAction)
{
   if (pAction == NULL)
   {
      return;
   }

   if (mPlugInCommands.contains(pAction) == true)
   {
      // Get the menu command from the action
      QString strCommand = pAction->text();

      // Get the plug-in name from the action
      QString strPlugIn;

      QMap<QString, QVariant> actionData = pAction->data().toMap();

      QMap<QString, QVariant>::const_iterator iter = actionData.find("PlugIn");
      if (iter != actionData.end())
      {
         strPlugIn = iter.value().toString();
      }

      if (strPlugIn.isEmpty() == true)
      {
         strPlugIn = strCommand;
         if (strPlugIn.isEmpty() == true)
         {
            return;
         }
      }

      // Create a plug-in resource to ensure that the plug-in is unloaded
      string plugInName = strPlugIn.toStdString();
      string commandName = strCommand.toStdString();
      ExecutableResource plugIn(plugInName, commandName, NULL, false);
      plugIn->createProgressDialog(true);

      // Execute the plug-in
      plugIn->execute();
   }
   else if (mWizardCommands.contains(pAction) == true)
   {
      // Get the wizard filename from the action
      QMap<QString, QVariant> actionData = pAction->data().toMap();

      QMap<QString, QVariant>::const_iterator iter = actionData.find("WizardFile");
      if (iter != actionData.end())
      {
         QString strWizardFile = iter.value().toString();
         if (strWizardFile.isEmpty() == false)
         {
            // Execute the wizard
            runWizard(strWizardFile);
         }
      }
   }
}

void ApplicationWindow::runWizard(QString strWizard)
{
   // Load the wizard from the file
   FactoryResource<WizardObject> pWizard(WizardUtilities::readWizard(strWizard.toStdString()));
   if (pWizard.get() != NULL)
   {
      // Launch the wizard
      WizardUtilities::runWizard(pWizard.get());
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Layout actions

ProductWindow* ApplicationWindow::newProduct()
{
   // Assign a unique window name
   unsigned int productNum = getNumWindows(PRODUCT_WINDOW) + 1;
   QString strProduct = "Product " + QString::number(productNum);

   while (getWindow(strProduct, PRODUCT_WINDOW) != NULL)
   {
      if (productNum > 10000)
      {
         // Prevent an infinite loop if there is a problem when getting the window
         break;
      }

      productNum++;
      strProduct = "Product " + QString::number(productNum);
   }

   // Create the product window and view
   ProductWindow* pWindow = NULL;
   pWindow = (ProductWindow*) createWindow(strProduct, PRODUCT_WINDOW);
   if (pWindow != NULL)
   {
      // Load the default template
      ProductView* pView = NULL;
      pView = pWindow->getProductView();
      if (pView != NULL)
      {
         const Filename* pTemplateFile = ProductView::getSettingTemplateFile();
         if (pTemplateFile != NULL)
         {
            string templateFile = pTemplateFile->getFullPathAndName();
            if (!templateFile.empty())
            {
               pView->loadTemplate(templateFile);
            }
         }
      }
   }

   return pWindow;
}

ProductWindow* ApplicationWindow::deriveProduct(View *pView)
{
   // If no view is specified then
   // get the view from the currently active spatial data window
   if(pView == NULL)
   {
      SpatialDataWindow* pActiveWindow = dynamic_cast<SpatialDataWindow*>(getCurrentWorkspaceWindow());
      if(pActiveWindow != NULL)
      {
         pView = pActiveWindow->getView();
      }
   }

   // Create a new product
   ProductWindow* pWindow = newProduct();
   if((pWindow != NULL) && (pView != NULL))
   {
      ProductView* pProductView = pWindow->getProductView();
      if(pProductView != NULL)
      {
         pProductView->setClassification(pView->getClassification());

         GraphicLayer* pLayoutLayer = pProductView->getLayoutLayer();
         if(pLayoutLayer != NULL)
         {
            // Set the view in all view objects in the default template
            if(pLayoutLayer->getNumObjects() > 0)
            {
               list<GraphicObject*> viewObjects;
               pLayoutLayer->getObjects(VIEW_OBJECT, viewObjects);

               for(list<GraphicObject*>::iterator iter = viewObjects.begin(); iter != viewObjects.end(); ++iter)
               {
                  GraphicObject* pObject = *iter;
                  if(pObject != NULL)
                  {
                     pObject->setObjectView(pView);
                  }
               }
            }
            else
            {
               // Prevent undo actions from being registered
               UndoLock lock(pProductView);

               // No objects in the default template, so add an initial view object
               GraphicObject* pObject = pLayoutLayer->addObject(VIEW_OBJECT);
               if(pObject != NULL)
               {
                  pObject->setObjectView(pView);

                  double dPaperWidth = 0.0;
                  double dPaperHeight = 0.0;
                  pProductView->getPaperSize(dPaperWidth, dPaperHeight);

                  unsigned int dpi = pProductView->getDpi();

                  LocationType llCorner(dpi, dpi);
                  LocationType urCorner((dPaperWidth - 1.0) * dpi, (dPaperHeight - 1.0) * dpi);
                  pObject->setBoundingBox(llCorner, urCorner);
               }
            }
         }
      }
   }

   return pWindow;
}

void ApplicationWindow::loadTemplate()
{
   WorkspaceWindow* pWindow = NULL;
   pWindow = getCurrentWorkspaceWindow();
   if (pWindow == NULL)
   {
      return;
   }

   if (pWindow->isKindOf("ProductWindow") == false)
   {
      return;
   }

   ProductView* pView = NULL;
   pView = ((ProductWindow*) pWindow)->getProductView();
   if (pView != NULL)
   {
      // Get the default template directory from the Options
      QString strDefaultDir;
      const Filename* pTemplatePath = ProductView::getSettingTemplatePath();
      if (pTemplatePath != NULL)
      {
         strDefaultDir = QString::fromStdString(pTemplatePath->getFullPathAndName());
      }

      // Get the template file to load from the user
      QString strTemplateFile = QFileDialog::getOpenFileName(this, "Load Template", strDefaultDir,
         "Template Files (*.spg);;All Files (*.*)");
      if (strTemplateFile.isEmpty() == false)
      {
         // Load the template
         string templateFile = strTemplateFile.toStdString();
         pView->loadTemplate(templateFile);
      }
   }
}

void ApplicationWindow::saveTemplate()
{
   ProductWindow* pWindow = dynamic_cast<ProductWindow*>(getCurrentWorkspaceWindow());
   if (pWindow == NULL)
   {
      return;
   }

   ProductViewImp* pView = dynamic_cast<ProductViewImp*>(pWindow->getProductView());
   if (pView != NULL)
   {
      // Get the default template directory from the Options
      QString strDefaultDir;
      const Filename* pTemplatePath = ProductView::getSettingTemplatePath();
      if (pTemplatePath != NULL)
      {
         strDefaultDir = QString::fromStdString(pTemplatePath->getFullPathAndName());
      }
      if (strDefaultDir.isEmpty())
      {
         strDefaultDir = QDir::currentPath();
      }

      // Create a default filename based on the product name
      QString strDefaultFile = QString::fromStdString(pView->getName()) + ".spg";

      // Get the actual template filename from the user
      QFileDialog dlg(this, "Save Template", strDefaultDir, "Template Files (*.spg);;All Files (*)");
      dlg.setAcceptMode(QFileDialog::AcceptSave);
      dlg.setFileMode(QFileDialog::AnyFile);
      dlg.setConfirmOverwrite(true);
      dlg.setDefaultSuffix("spg");
      dlg.selectFile(strDefaultFile);

      if (dlg.exec() == QDialog::Accepted)
      {
         QString strTemplateFile = dlg.selectedFiles().front();
         if (strTemplateFile.isEmpty() == false)
         {
            // Save the template
            pView->saveTemplate(strTemplateFile);
         }
      }
   }
}

void ApplicationWindow::editViewObject(bool bEdit)
{
   ProductView* pView = NULL;

   WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      pView = dynamic_cast<ProductView*>(pWindow->getView());
   }

   if (pView == NULL)
   {
      return;
   }

   View* pCurrentEditView = pView->getActiveEditView();
   if (((pCurrentEditView != NULL) && (bEdit == true)) || ((pCurrentEditView == NULL) && (bEdit == false)))
   {
      return;
   }

   // Get the edit view from the product view
   View* pEditView = NULL;
   if (bEdit == true)
   {
      GraphicLayer* pActiveLayer = pView->getActiveLayer();
      GraphicLayer* pLayoutLayer = pView->getLayoutLayer();
      if (pLayoutLayer != NULL)
      {
         if (pActiveLayer == pLayoutLayer)
         {
            list<GraphicObject*> viewObjects;
            pLayoutLayer->getSelectedObjects(VIEW_OBJECT, viewObjects);

            unsigned int numObjects = viewObjects.size();
            if (numObjects == 0)
            {
               QMessageBox::critical(this, windowTitle(), "Please select a view object to edit!");
            }
            else if (numObjects == 1)
            {
               ViewObjectImp* pObject = NULL;
               pObject = dynamic_cast<ViewObjectImp*>(viewObjects.front());
               if (pObject != NULL)
               {
                  pEditView = pObject->getView();
               }

               if (pEditView != NULL)
               {
                  if (pEditView->isKindOf("SpatialDataView") == false)
                  {
                     QMessageBox::critical(this, windowTitle(), "The selected view object cannot be edited!");
                     pEditView = NULL;
                  }
               }
               else
               {
                  QMessageBox::critical(this, windowTitle(), "The selected view cannot be edited!");
               }
            }
            else if (numObjects > 1)
            {
               QMessageBox::critical(this, windowTitle(), "Please select only one view object to edit!");
            }
         }
         else
         {
            QMessageBox::critical(this, windowTitle(), "To edit a selected view, "
               "first activate the layout layer and then select a view object!");
         }
      }
   }

   // Set the new edit view
   bool bSuccess = pView->setActiveEditView(pEditView);
   if (bSuccess == true)
   {
      // Update the toolbars and dock windows
      updateActiveWindow(dynamic_cast<QMdiSubWindow*>(pWindow));
   }

   // Update the action toggle state
   mpEditViewAction->setChecked(bEdit & bSuccess);
}

void ApplicationWindow::createDataSetAnimation()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getCurrentWorkspaceWindowView());
   if (pView == NULL)
   {
      QMessageBox::critical(this, windowTitle(), "An animation can only be created from an active spatial data view!");
      return;
   }

   if (pView->createDefaultAnimation() != NULL)
   {
      if (mpAnimationToolBar != NULL)
      {
         // Activate the animation controller
         AnimationController* pController = pView->getAnimationController();
         if (pController != NULL)
         {
            mpAnimationToolBar->setAnimationController(pController);
         }

         // Show the toolbar if necessary
         mpAnimationToolBar->show();
      }
   }
   else
   {
      QMessageBox::critical(this, windowTitle(), "Could not create an animation from the active spatial data view!");
   }
}

void ApplicationWindow::setPaperSize()
{
   WorkspaceWindow* pWindow = NULL;
   pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      if (pWindow->getWindowType() == PRODUCT_WINDOW)
      {
         ProductView* pView = NULL;
         pView = ((ProductWindow*) pWindow)->getProductView();
         if (pView != NULL)
         {
            double dWidth = 0.0;
            double dHeight = 0.0;
            pView->getPaperSize(dWidth, dHeight);

            PaperSizeDlg sizeDlg(dWidth, dHeight, this);

            int iResult = sizeDlg.exec();
            if (iResult == QDialog::Accepted)
            {
               sizeDlg.getSize(dWidth, dHeight);
               pView->setPaperSize(dWidth, dHeight);
            }
         }
      }
      else
      {
         QMessageBox::critical(this, QString::fromStdString(APP_NAME),
            "The paper size can only be set on a product window!");
      }
   }
}

void ApplicationWindow::updateActiveWindow(QMdiSubWindow* pWindow)
{
   // Check for the currently active window
   WorkspaceWindowImp* pWorkspaceWindow = dynamic_cast<WorkspaceWindowImp*>(pWindow);
   if (pWorkspaceWindow == mpCurrentWnd)
   {
      // Check to see if the product edit view has changed
      bool bModified = false;
      ProductWindowImp *pProduct = dynamic_cast<ProductWindowImp*>(pWorkspaceWindow);
      if (pProduct != NULL)
      {
         ProductView* pView = pProduct->getProductView();
         if (pView != NULL)
         {
            View* pEditView = NULL;
            pEditView = pView->getActiveEditView();
            if (pEditView != mpCurrentEditView)
            {
               bModified = true;
            }
         }
      }

      if (bModified == false)
      {
         return;
      }
   }

   // Disconnect the previously active window and view
   if (mpCurrentWnd != NULL)
   {
      // Window connections
      SpatialDataWindowImp *pSpatial = dynamic_cast<SpatialDataWindowImp*>(mpCurrentWnd);
      if (pSpatial != NULL)
      {
         VERIFYNR(disconnect(pSpatial, SIGNAL(overviewVisibilityChanged(bool)), m_pOverview_Wnd_Action,
            SLOT(setChecked(bool))));
      }

      VERIFYNR(disconnect(mpSessionExplorer, SIGNAL(visibilityChanged(bool)), mpCurrentWnd, SLOT(setFocus())));
      VERIFYNR(disconnect(static_cast<HistogramWindowImp*>(m_pHistogram), SIGNAL(visibilityChanged(bool)),
         mpCurrentWnd, SLOT(setFocus())));
      VERIFYNR(disconnect(m_pMessage_Log, SIGNAL(visibilityChanged(bool)), mpCurrentWnd, SLOT(setFocus())));
      VERIFYNR(disconnect(m_pBackground_Plugins, SIGNAL(visibilityChanged(bool)), mpCurrentWnd, SLOT(setFocus())));
      VERIFYNR(disconnect(m_pScripting, SIGNAL(visibilityChanged(bool)), mpCurrentWnd, SLOT(setFocus())));

      // Get the active view in the window
      View* pView = mpCurrentView.get();
      if (pView != NULL)
      {
         if (pView->isKindOf("ProductView") == true)
         {
            if (mpCurrentEditView != NULL)
            {
               pView = mpCurrentEditView;
            }
         }
      }

      // Active view connections
      if (pView != NULL)
      {
         ViewImp* pViewImp = dynamic_cast<ViewImp*>(pView);
         if (pViewImp != NULL)
         {
            VERIFYNR(disconnect(pViewImp, SIGNAL(mouseModeChanged(const MouseMode*)), this,
               SLOT(updateMouseAction(const MouseMode*))));
         }

         PerspectiveViewImp* pPerspectiveView = dynamic_cast<PerspectiveViewImp*>(pView);
         if (pPerspectiveView != NULL)
         {
            // Toolbars
            VERIFYNR(disconnect(pPerspectiveView, SIGNAL(zoomChanged(double)), mpDisplayToolBar,
               SLOT(setZoomPercentage(double))));
         }

         SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(pView);
         if (pSpatialDataView != NULL)
         {
            VERIFYNR(disconnect(pSpatialDataView, SIGNAL(layerActivated(Layer*)), this,
               SLOT(initializeToolBars(Layer*))));

            // GCP editor
            if (mpGcpEditor != NULL)
            {
               VERIFYNR(disconnect(pSpatialDataView, SIGNAL(layerAdded(Layer*)), mpGcpEditor, SLOT(addLayer(Layer*))));
               VERIFYNR(disconnect(pSpatialDataView, SIGNAL(layerDeleted(Layer*)), mpGcpEditor,
                  SLOT(removeLayer(Layer*))));
               VERIFYNR(disconnect(pSpatialDataView, SIGNAL(layerActivated(Layer*)), mpGcpEditor,
                  SLOT(setGcpLayer(Layer*))));
            }

            // Tie Point editor
            if (mpTiePointEditor != NULL)
            {
               VERIFYNR(disconnect(pSpatialDataView, SIGNAL(layerActivated(Layer*)), mpTiePointEditor,
                  SLOT(setTiePointLayer(Layer*))));
            }
         }

         ProductViewImp* pProductView = dynamic_cast<ProductViewImp*>(pView);
         if (pProductView != NULL)
         {
            VERIFYNR(disconnect(pProductView, SIGNAL(layerActivated(Layer*)), this, SLOT(initializeToolBars(Layer*))));
         }
      }
   }

   // Update the current window
   mpCurrentWnd = pWorkspaceWindow;
   if (mpCurrentWnd == NULL)
   {
      mpCurrentView.reset(NULL);
   }
   else
   {
      mpCurrentView.reset(mpCurrentWnd->getView());
   }

   mpCurrentEditView = NULL;
   UndoLock lock(mpCurrentView.get());

   double dZoomPercent = -1.0;
   bool bOverview = false;
   Layer* pAnnotationLayer = NULL;
   Layer* pAoiLayer = NULL;
   Layer* pGcpLayer = NULL;
   Layer* pTiePointLayer = NULL;
   Layer* pMeasurementsLayer = NULL;
   const MouseMode* pMouseMode = NULL;

   if (pWorkspaceWindow != NULL)
   {
      // Window connections
      SpatialDataWindowImp *pSpatial = dynamic_cast<SpatialDataWindowImp*>(pWorkspaceWindow);
      if (pSpatial != NULL)
      {
         VERIFYNR(connect(pSpatial, SIGNAL(overviewVisibilityChanged(bool)), m_pOverview_Wnd_Action,
            SLOT(setChecked(bool))));
      }

      VERIFYNR(connect(mpSessionExplorer, SIGNAL(visibilityChanged(bool)), pWorkspaceWindow, SLOT(setFocus())));
      VERIFYNR(connect(static_cast<HistogramWindowImp*>(m_pHistogram), SIGNAL(visibilityChanged(bool)),
         pWorkspaceWindow, SLOT(setFocus())));
      VERIFYNR(connect(m_pMessage_Log, SIGNAL(visibilityChanged(bool)), pWorkspaceWindow, SLOT(setFocus())));
      VERIFYNR(connect(m_pBackground_Plugins, SIGNAL(visibilityChanged(bool)), pWorkspaceWindow, SLOT(setFocus())));
      VERIFYNR(connect(m_pScripting, SIGNAL(visibilityChanged(bool)), pWorkspaceWindow, SLOT(setFocus())));

      // Get the active view in the window
      View* pView = mpCurrentView.get();
      if (pView != NULL)
      {
         if (pView->isKindOf("ProductView") == true)
         {
            mpCurrentEditView = ((ProductView*) pView)->getActiveEditView();
            if (mpCurrentEditView != NULL)
            {
               pView = mpCurrentEditView;
            }
         }
      }

      // Active view connections
      if (pView != NULL)
      {
         ViewImp* pViewImp = dynamic_cast<ViewImp*>(pView);
         if (pViewImp != NULL)
         {
            VERIFYNR(connect(pViewImp, SIGNAL(mouseModeChanged(const MouseMode*)), this,
               SLOT(updateMouseAction(const MouseMode*))));
         }

         PerspectiveViewImp* pPerspectiveView = dynamic_cast<PerspectiveViewImp*>(pView);
         if (pPerspectiveView != NULL)
         {
            // Toolbars
            VERIFYNR(connect(pPerspectiveView, SIGNAL(zoomChanged(double)), mpDisplayToolBar,
               SLOT(setZoomPercentage(double))));
         }

         SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(pView);
         if (pSpatialDataView != NULL)
         {
            VERIFYNR(connect(pSpatialDataView, SIGNAL(layerActivated(Layer*)), this, SLOT(initializeToolBars(Layer*))));

            // GCP editor
            if (mpGcpEditor != NULL)
            {
               VERIFYNR(connect(pSpatialDataView, SIGNAL(layerAdded(Layer*)), mpGcpEditor, SLOT(addLayer(Layer*))));
               VERIFYNR(connect(pSpatialDataView, SIGNAL(layerDeleted(Layer*)), mpGcpEditor,
                  SLOT(removeLayer(Layer*))));
               VERIFYNR(connect(pSpatialDataView, SIGNAL(layerActivated(Layer*)), mpGcpEditor,
                  SLOT(setGcpLayer(Layer*))));
            }

            // Tie point editor
            if (mpTiePointEditor != NULL)
            {
               VERIFYNR(connect(pSpatialDataView, SIGNAL(layerActivated(Layer*)), mpTiePointEditor,
                  SLOT(setTiePointLayer(Layer*))));
            }
         }

         ProductViewImp* pProductView = dynamic_cast<ProductViewImp*>(pView);
         if (pProductView != NULL)
         {
            VERIFYNR(connect(pProductView, SIGNAL(layerActivated(Layer*)), this, SLOT(initializeToolBars(Layer*))));
         }
      }

      // Dock windows
      string viewName = "";
      if (pView != NULL)
      {
         viewName = pView->getName();
      }

      if (m_pHistogram != NULL)
      {
         PlotSet* pPlotSet = m_pHistogram->getPlotSet(viewName);
         m_pHistogram->setCurrentPlotSet(pPlotSet);
      }

      // Toolbars
      enableActions(true);
      enableToolBars(true);

      bOverview = false;
      SpatialDataWindow *pSpatialWindow = dynamic_cast<SpatialDataWindow*>(pWorkspaceWindow);
      if (pSpatialWindow != NULL)
      {
         bOverview = pSpatialWindow->isOverviewWindowShown();
      }

      if (pView != NULL)
      {
         if (pView->isKindOf("PerspectiveView") == true)
         {
            dZoomPercent = ((PerspectiveView*) pView)->getZoomPercentage();
         }

         if (pView->isKindOf("SpatialDataView") == true)
         {
            pAnnotationLayer = ((SpatialDataView*) pView)->getTopMostLayer(ANNOTATION);
            pAoiLayer = ((SpatialDataView*) pView)->getTopMostLayer(AOI_LAYER);
            pGcpLayer = ((SpatialDataView*) pView)->getTopMostLayer(GCP_LAYER);
            pTiePointLayer = ((SpatialDataView*) pView)->getTopMostLayer(TIEPOINT_LAYER);

            SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*> (pView);
            if (pSpatialDataView != NULL)
            {
               pMeasurementsLayer = pSpatialDataView->getMeasurementsLayer();
            }
         }
         else if (pView->isKindOf("ProductView") == true)
         {
            pAnnotationLayer = ((ProductView*) pView)->getActiveLayer();
         }

         pMouseMode = pView->getCurrentMouseMode();
      }
   }
   else
   {
      enableActions(false);
      enableToolBars(false);
      mpWorkspace->setFocus();
   }

   mpEditViewAction->setChecked(mpCurrentEditView != NULL);
   mpDisplayToolBar->setZoomPercentage(dZoomPercent);
   m_pOverview_Wnd_Action->setChecked(bOverview);
   mpAnnotationToolBar->setAnnotationLayer(pAnnotationLayer);
   mpAoiToolBar->setAoiLayer(pAoiLayer);
   mpGcpToolBar->setGcpLayer(pGcpLayer);
   mpTiePointToolBar->setTiePointLayer(pTiePointLayer);
   mpBrightnessToolbar->updateForNewView();
   mpMeasurementToolBar->setMeasurementsLayer(pMeasurementsLayer);
   updateMouseAction(pMouseMode);

   // Update the current undo stack
   UndoStack* pUndoStack = NULL;

   ViewImp* pWindowViewImp = dynamic_cast<ViewImp*>(mpCurrentView.get());
   if (pWindowViewImp != NULL)
   {
      pUndoStack = pWindowViewImp->getUndoStack();
   }

   mpUndoGroup->setActiveStack(pUndoStack);

   // Update the GCP Editor
   if (mpGcpEditor != NULL)
   {
      mpGcpEditor->updateLayers();
      mpGcpEditor->setGcpLayer(pGcpLayer);
   }

   // Update the TiePoint Editor
   if (mpTiePointEditor != NULL)
   {
      mpTiePointEditor->setTiePointLayer(pTiePointLayer);
   }

   // Notify attached objects
   notify(SIGNAL_NAME(ApplicationWindow, WindowActivated), boost::any(dynamic_cast<WorkspaceWindow*>(mpCurrentWnd)));
}

void ApplicationWindow::initializeToolBars(Layer* pLayer)
{
   enableToolBars(true);

   if (pLayer == NULL)
   {
      return;
   }

   SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pLayer->getView());
   if (pView != NULL)
   {
      if (pLayer == pView->getMeasurementsLayer())
      {
         mpMeasurementToolBar->setMeasurementsLayer(pLayer);
         mpMeasurementToolBar->show();
         return;
      }
   }

   if (dynamic_cast<AnnotationLayer*>(pLayer) != NULL)
   {
      mpAnnotationToolBar->setAnnotationLayer(pLayer);
      mpAnnotationToolBar->show();
   }
   else if (dynamic_cast<AoiLayer*>(pLayer) != NULL)
   {
      mpAoiToolBar->setAoiLayer(pLayer);
      mpAoiToolBar->show();
   }
   else if (dynamic_cast<GcpLayer*>(pLayer) != NULL)
   {
      mpGcpToolBar->setGcpLayer(pLayer);
      mpGcpToolBar->show();
   }
   else if (dynamic_cast<TiePointLayer*>(pLayer) != NULL)
   {
      mpTiePointToolBar->setTiePointLayer(pLayer);
      mpTiePointToolBar->show();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
// Application Window management

void ApplicationWindow::enableActions(bool bEnable)
{
   bool bView = false;
   bool bSpatialDataView = false;
   bool bProductView = false;
   bool bProductWindow = false;
   bool bEnableEditActions = false;

   WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      bool bLayerMode = false;
      bool bMeasurementMode = false;

      if (dynamic_cast<ProductWindow*>(pWindow) != NULL)
      {
         bProductWindow = true;
      }

      View* pView = pWindow->getActiveView();
      if (pView != NULL)
      {
         bView = true;

         const MouseMode* pMouseMode = pView->getCurrentMouseMode();
         if (pMouseMode != NULL)
         {
            string modeName = "";
            pMouseMode->getName(modeName);
            if (modeName == "LayerMode")
            {
               bLayerMode = true;
            }
            else if (modeName == "MeasurementMode")
            {
               bMeasurementMode = true;
            }
         }
      }

      SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(pView);
      if (pSpatialDataView != NULL)
      {
         bSpatialDataView = true;

         Layer* pActiveLayer = pSpatialDataView->getActiveLayer();
         if ((bLayerMode == true) && (pActiveLayer != NULL))
         {
            bEnableEditActions = (dynamic_cast<AnnotationLayer*>(pActiveLayer) != NULL);
         }
         else if (bMeasurementMode == true)
         {
            bEnableEditActions = pSpatialDataView->isMeasurementsLayerShown();
         }
      }

      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         bProductView = true;

         GraphicLayer* pActiveLayer = pProductView->getActiveLayer();
         if ((bLayerMode == true) && (pActiveLayer != NULL))
         {
            bEnableEditActions = true;
         }
      }
   }

   // File actions
   m_pClose_Action->setEnabled(bEnable);
   m_pPrint_Setup_Action->setEnabled(bEnable);
   m_pPrint_Action->setEnabled(bEnable);
   mpExportFileMenuAction->setEnabled(bEnable);

   // Edit actions
   m_pCut_Action->setEnabled(bEnable && bEnableEditActions);
   m_pCopy_Action->setEnabled(bEnable && bEnableEditActions);
   m_pPaste_Action->setEnabled(bEnable && bEnableEditActions);

   // View actions
   m_pView_Mode_Group->setEnabled(bEnable);
   mpLayerEditAction->setEnabled(bEnable);
   m_pMeasurement_Edit_Action->setEnabled(bEnable && bSpatialDataView);
   mpCreateAnimationAction->setEnabled(bEnable && bSpatialDataView);
   m_pRefresh_Action->setEnabled(bEnable);
   m_pReset_Action->setEnabled(bEnable && bSpatialDataView);
   mpResetStretchAction->setEnabled(bEnable && bSpatialDataView);
   m_pDisplay_Mode_Action->setEnabled(bEnable && bSpatialDataView);
   mpClearMarkingsAction->setEnabled(bEnable && bSpatialDataView);

   // Pan actions
   m_pPan_Action->setEnabled(bEnable);

   // Rotate actions
   m_pRotate_Left_Action->setEnabled(bEnable && bSpatialDataView);
   m_pRotate_Right_Action->setEnabled(bEnable && bSpatialDataView);
   m_pFlip_Horiz_Action->setEnabled(bEnable && bSpatialDataView);
   m_pFlip_Vert_Action->setEnabled(bEnable && bSpatialDataView);
   m_pRotate_By_Action->setEnabled(bEnable && bSpatialDataView);
   m_pFree_Rotate_Action->setEnabled(bEnable && bSpatialDataView);
   m_pReset_Action->setEnabled(bEnable && bSpatialDataView);
   mpNorthUpAction->setEnabled(bEnable && bSpatialDataView);
   mpSensorUpAction->setEnabled(bEnable && bSpatialDataView);

   // Zoom actions
   m_pZoom_In_Action->setEnabled(bEnable && bView);
   m_pZoom_Out_Action->setEnabled(bEnable && bView);
   m_pZoom_Point_In_Action->setEnabled(bEnable && bView);
   m_pZoom_Point_Out_Action->setEnabled(bEnable && bView);
   m_pZoom10_Action->setEnabled(bEnable && bView);
   m_pZoom50_Action->setEnabled(bEnable && bView);
   m_pZoom75_Action->setEnabled(bEnable && bView);
   m_pZoom100_Action->setEnabled(bEnable && bView);
   m_pZoom150_Action->setEnabled(bEnable && bView);
   m_pZoom200_Action->setEnabled(bEnable && bView);
   m_pZoom400_Action->setEnabled(bEnable && bView);
   m_pZoom800_Action->setEnabled(bEnable && bView);
   m_pZoom_To_Fit_Action->setEnabled(bEnable && bView);
   m_pZoom_And_Pan_To_Point_Action->setEnabled(bEnable && bView);
   m_pZoom_Rect_Action->setEnabled(bEnable && bView);

   // Layout actions
   mpDeriveProductAction->setEnabled(bEnable && bSpatialDataView && !bProductView);
   m_pLoadTemplate_Action->setEnabled(bEnable && bProductWindow);
   m_pSaveTemplate_Action->setEnabled(bEnable && bProductWindow);
   mpEditViewAction->setEnabled(bEnable && bProductWindow);
   m_pPaperSize_Action->setEnabled(bEnable && bProductWindow);

   // Tools actions
   m_pOverview_Wnd_Action->setEnabled(bEnable && bSpatialDataView);     // Temporary: Enable the overview window
                                                                        // for product views when annotation layers
                                                                        // are linked in the overview window
   m_pChipping_Wnd_Action->setEnabled(bEnable && bSpatialDataView);
   m_pLink_Action->setEnabled(bEnable && bSpatialDataView);

   // Window actions
   mpCascadeAction->setEnabled(bEnable);
   mpTileAction->setEnabled(bEnable);
   mpTileHorizontalAction->setEnabled(bEnable);
   mpTileVerticalAction->setEnabled(bEnable);
   mpTileSelectedAction->setEnabled(bEnable);
}

void ApplicationWindow::enableToolBars(bool bEnable)
{
   bool bView = false;
   bool bSpatialDataView = false;
   bool bEnableAnnotation = false;
   bool bEnableAoi = false;
   bool bEnableGcp = false;
   bool bEnableTiePoint = false;
   bool bEnableMeasurement = false;

   WorkspaceWindow* pWindow = getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      View* pView = pWindow->getActiveView();

      bool bLayerMode = false;
      bool bMeasurementMode = false;
      if (pView != NULL)
      {
         bView = true;

         const MouseMode* pMouseMode = pView->getCurrentMouseMode();
         if (pMouseMode != NULL)
         {
            string modeName = "";
            pMouseMode->getName(modeName);
            if (modeName == "LayerMode")
            {
               bLayerMode = true;
            }
            else if (modeName == "MeasurementMode")
            {
               bMeasurementMode = true;
            }
         }
      }

      SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(pView);
      if (pSpatialDataView != NULL)
      {
         bSpatialDataView = true;

         Layer* pActiveLayer = pSpatialDataView->getActiveLayer();
         if ((bLayerMode == true) && (pActiveLayer != NULL))
         {
            bEnableAnnotation = (dynamic_cast<AnnotationLayer*>(pActiveLayer) != NULL);
            bEnableAoi = (dynamic_cast<AoiLayer*>(pActiveLayer) != NULL);
            bEnableGcp = (dynamic_cast<GcpLayer*>(pActiveLayer) != NULL);
            bEnableTiePoint = (dynamic_cast<TiePointLayer*>(pActiveLayer) != NULL);
         }
         else if (bMeasurementMode == true)
         {
            bEnableMeasurement = true;
         }
      }

      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         GraphicLayer* pActiveLayer = pProductView->getActiveLayer();
         if ((bLayerMode == true) && (pActiveLayer != NULL))
         {
            bEnableAnnotation = true;
         }
      }
   }

   mpDisplayToolBar->setEnabled(bEnable && bView);
   mpBrightnessToolbar->setEnabled(bEnable && bSpatialDataView);
   mpAnnotationToolBar->setEnabled(bEnable && bEnableAnnotation);
   mpAoiToolBar->setEnabled(bEnable && bEnableAoi);
   mpGcpToolBar->setEnabled(bEnable && bEnableGcp);
   mpTiePointToolBar->setEnabled(bEnable && bEnableTiePoint);
   mpMeasurementToolBar->setEnabled(bEnable && bEnableMeasurement);
}

void ApplicationWindow::saveConfiguration() const
{
   // save dock window states
   vector<Window*> dockWindows = getWindows(DOCK_WINDOW); 
   vector<Window*>::iterator it; 

   ConfigurationSettings* pConfig = ConfigurationSettingsImp::instance();
   if (pConfig == NULL)
   {
      return;
   }

   // Dock window and toolbar configuration
   for (it=dockWindows.begin(); it!=dockWindows.end(); ++it) 
   { 
      DockWindowImp* pDock = dynamic_cast<DockWindowImp*>(*it); 
      if (pDock != NULL) 
      { 
         pDock->saveState(); 
      } 
   } 

   QByteArray windowConfiguration = saveState().toBase64();

   QString strConfiguration(windowConfiguration);
   if (strConfiguration.isEmpty() == false)
   {
      string configData = strConfiguration.toStdString();
      ApplicationWindow::setSettingConfiguration(configData);
   }

   // Application window geometry
   QByteArray windowGeometry = saveGeometry().toBase64();

   QString strGeometry(windowGeometry);
   if (strGeometry.isEmpty() == false)
   {
      string geometryData = strGeometry.toStdString();
      ApplicationWindow::setSettingGeometry(geometryData);
   }
}

void ApplicationWindow::restoreConfiguration()
{
   if (ApplicationWindow::hasSettingConfiguration())
   {
      string configData = ApplicationWindow::getSettingConfiguration();
      // Dock window and toolbar configuration
      if (!configData.empty())
      {
         QByteArray windowConfiguration(configData.c_str(), configData.size());
         restoreState(QByteArray::fromBase64(windowConfiguration));
      }
   }

   if (ApplicationWindow::hasSettingGeometry())
   {
      // Application window geometry
      string geometryData = ApplicationWindow::getSettingGeometry();
      if (!geometryData.empty())
      {
         QByteArray windowGeometry(geometryData.c_str(), geometryData.size());
         restoreGeometry(QByteArray::fromBase64(windowGeometry));
      }
   }
}

QSize ApplicationWindow::sizeHint() const
{
   return QSize(865, 650);
}

QSizePolicy ApplicationWindow::sizePolicy() const
{
   return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ApplicationWindow::setAutoSaveTimerEnabled(bool enabled)
{
   if(enabled)
   {
      mpSaveTimer->start();
   }
   else
   {
      mpSaveTimer->stop();
   }
}

void ApplicationWindow::setAutoSaveTimerInterval(unsigned int interval)
{
   unsigned int minute = 1000*60;
   mpSaveTimer->setInterval(interval < 1 ? 1*minute : interval > 1440 ? 1440*minute : interval*minute);
}

void ApplicationWindow::optionsModified(Subject &subject, const string &signal, const boost::any &value)
{
   string key = boost::any_cast<string>(value);
   if(key == SessionManager::getSettingAutoSaveIntervalKey())
   {
      setAutoSaveTimerInterval(SessionManager::getSettingAutoSaveInterval());
   }
   else if(key == SessionManager::getSettingAutoSaveEnabledKey())
   {
      setAutoSaveTimerEnabled(SessionManager::getSettingAutoSaveEnabled());
   }
}

void ApplicationWindow::closeEvent(QCloseEvent* e)
{
   // Close the wizard builder instances
   vector<WizardBuilder*> wizardBuilders;

   QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
   for (int i = 0; i < topLevelWidgets.count(); ++i)
   {
      WizardBuilder* pWizardBuilder = dynamic_cast<WizardBuilder*>(topLevelWidgets[i]);
      if (pWizardBuilder != NULL)
      {
         // Push the pointers into a vector to prevent corrupting the widget list when the wizard builder is closed
         wizardBuilders.push_back(pWizardBuilder);
      }
   }

   for (vector<WizardBuilder*>::iterator iter = wizardBuilders.begin(); iter != wizardBuilders.end(); ++iter)
   {
      WizardBuilder* pWizardBuilder = *iter;
      if ((pWizardBuilder != NULL) && (pWizardBuilder->close() == false))
      {
         e->ignore();
         return;
      }
   }

   // Save the position and the visibility state of the toolbars and dock windows
   saveConfiguration();

   // Close the session
   SessionSaveType eSave = SessionManager::getSettingQueryForSave();
   int button = (eSave == SESSION_AUTO_SAVE ? QMessageBox::Yes : QMessageBox::No);
   if (eSave == SESSION_QUERY_SAVE)
   {
         button = QMessageBox::question(this, "Close Session", 
            "Do you want to save the session before closing it?", 
            QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 
            QMessageBox::Cancel | QMessageBox::Escape);
   }
   if (button == QMessageBox::Cancel)
   {
      e->ignore();
      return;
   }
   if (button == QMessageBox::Yes)
   {
      if (!saveSession()) // user pressed Cancel in response to Save As, or save failed
      {
         e->ignore();
         return;
      }
   }

   notify (SIGNAL_NAME(ApplicationWindow, Closed));
   SessionManagerImp *pSessionImp = SessionManagerImp::instance();
   pSessionImp->close();

   qApp->quit();
}

static QTime sIdleTime;
bool ApplicationWindow::eventFilter(QObject* pObj, QEvent* pEvent)
{
   if (pEvent->type() == QEvent::ChildRemoved || pEvent->type() == QEvent::Destroy)
   {
      return QMainWindow::eventFilter(pObj, pEvent);
   }

   if (pEvent->type() != QEvent::Timer)
   {
      sIdleTime = QTime::currentTime();
      sIdleTime = sIdleTime.addSecs(3);
   }

   if (pObj == mpAnnotationToolBar && pEvent->type() == QEvent::EnabledChange)
   {
      bool enable(mpAnnotationToolBar->isEnabled());
      m_pCopy_Action->setEnabled(enable);
      m_pCut_Action->setEnabled(enable);
      m_pPaste_Action->setEnabled(enable);
   }

   return QMainWindow::eventFilter(pObj, pEvent);
}

bool ApplicationWindow::isDefaultWindow(Window* pWindow) const
{
   if (pWindow == NULL)
   {
      return false;
   }

   if ((pWindow == mpStandardToolBar) || (pWindow == mpToolboxToolBar) || (pWindow == mpDisplayToolBar) ||
      (pWindow == mpAoiToolBar) || (pWindow == mpAnnotationToolBar) || (pWindow == mpGcpToolBar) ||
      (pWindow == mpBrightnessToolbar) || (pWindow == mpSessionExplorer) || (pWindow == m_pHistogram) ||
      (pWindow == m_pMessage_Log) || (pWindow == mpTiePointToolBar) || (pWindow == m_pBackground_Plugins) ||
      (pWindow == mpMeasurementToolBar) || (pWindow == m_pScripting) || (pWindow == mpAnimationToolBar))
   {
      return true;
   }

   return false;
}

void ApplicationWindow::checkColorDepth(QWidget* pSplash)
{
   // ensure we have enough colors available
#if defined(UNIX_API)
   // Solaris supports different color depths for different applications
   // so we get the color depth of the current application by creating
   // a dummy OpenGL window and checking the depth of its X11 device
   class tmpGL : public QGLWidget {
   public:
      tmpGL(QWidget *parent) : QGLWidget(parent) {}
   protected:
      void initializeGL() {}
      void resizeGL() {}
      void paintGL() {}
   } tmp(this);

   int nbp(tmp.context()->device()->depth());
#else
   // on Windows, all applications use the desktop's color depth
   // so we can check the number of bit planes available to QColor
   int nbp(QPixmap::defaultDepth());
#endif

   if (nbp < 16)
   {
      // hide the splash screen so it doesn't obscure the message dialog
      if (pSplash != NULL)
      {
         pSplash->hide();
      }

      QString msg = QString("The graphics display on this computer has a color depth\n"
                  "of %1 bits. %2 requires a display which supports\n"
                  "16 bit color. Colors may not display properly. \n"
                  "It is possible that your display adapter supports\n"
                  "the necessary mode but is incorrectly configured.\n"
                  "\nPlease contact your local computer administrator to\n"
                  "correct this problem.").arg(nbp).arg(APP_NAME);
      if (QMessageBox::warning(this, QString::fromStdString(APP_NAME), msg,
         QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Abort)
      {
         // we use the system exit() instead of QApplication::exit()
         // since we are not in the Qt event loop at this point
         exit(42);
      }
   }
}

void ApplicationWindow::checkGpuImageSupport()
{
   bool systemSupportsGpuImage = false;
#if defined(CG_SUPPORTED)
   class tmpGL : public QGLWidget {
   public:
      tmpGL(QWidget *parent) : QGLWidget(parent) {}
   protected:
      void initializeGL() {}
      void resizeGL() {}
      void paintGL() {}
   } tmp(this);
   tmp.makeCurrent();
   if (CgContext::instance() != NULL)
   {
      systemSupportsGpuImage = true;
   }
#endif
   bool defaultGpuImage = RasterLayer::getSettingGpuImage();
   if (!systemSupportsGpuImage && defaultGpuImage)
   {
      Service<ConfigurationSettings> pSettings;
      pSettings->setTemporarySetting(RasterLayer::getSettingGpuImageKey(), false);
   }
}


// Reset view
void ApplicationWindow::clearMarkings()
{
   WorkspaceWindow *pWindow = getCurrentWorkspaceWindow();

   if (pWindow == NULL)
   {
      QMessageBox::information(this, "No view selected", "There is no view currently selected", 
         QMessageBox::Ok | QMessageBox::Default);

      return;
   }

   if (pWindow->getWindowType() == PRODUCT_WINDOW)
   {
      QMessageBox::information(this, "Product window selected", "Clear markings is not currently supported for a Product window", QMessageBox::Ok | QMessageBox::Default);
      return;
   }

   int buttonVal = QMessageBox::warning(this, "Warning", "This action will clear the undo history.  "
      "Do you wish to continue?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);
   if (buttonVal == QMessageBox::No)
   {
      return;
   }

   if (pWindow->getWindowType() == SPATIAL_DATA_WINDOW)
   {
      SpatialDataWindow *pSpatialDW = (SpatialDataWindow*)pWindow;
      SpatialDataViewAdapter *pSpatialDV = (SpatialDataViewAdapter*)pSpatialDW->getSpatialDataView();
      pSpatialDV->clearMarkings();
   }
}

void ApplicationWindow::forwardSnapshot()
{
   ViewWindow* pViewWindow = getCurrentWorkspaceWindow();
   ViewImp* pView = dynamic_cast<ViewImp*>((pViewWindow == NULL) ? NULL : pViewWindow->getView());
   if (pView == NULL)
   {
      return;
   }
   if (sender() == mpClipboardSizedAction)
   {
      pView->snapshotSized();
   }
   else if (sender() == mpClipboardAction)
   {
      pView->snapshot();
   }
}

void ApplicationWindow::pregenerateTexture()
{
   QTime currentTime = QTime::currentTime();
   if (currentTime < sIdleTime)
   {
      return;
   }

   vector<Window*>::iterator ppWindow;
   for (ppWindow=mWindows.begin(); ppWindow!=mWindows.end(); ++ppWindow)
   {
      if (*ppWindow != NULL && (*ppWindow)->isKindOf("WorkspaceWindow"))
      {
         WorkspaceWindow *pWsWindow = static_cast<WorkspaceWindow*>(*ppWindow);
         View *pView = pWsWindow->getView();
         if (pView != NULL && pView->isKindOf("SpatialDataView"))
         {
            SpatialDataView *pSdView = static_cast<SpatialDataView*>(pView);
            const LayerList *pLayers = pSdView->getLayerList();
            if (pLayers != NULL)
            {
               vector<Layer*> layers;
               pLayers->getLayers(layers);
               vector<Layer*>::iterator ppLayer;
               for (ppLayer=layers.begin(); ppLayer!=layers.end(); ++ppLayer)
               {
                  if (*ppLayer != NULL && (*ppLayer)->isKindOf("RasterLayer"))
                  {
                     RasterLayerImp *pRasterLayer = static_cast<RasterLayerAdapter*>(*ppLayer);
                     if (pRasterLayer->generateFullResTexture() == true)
                     {
                        return;
                     }
                  }
               }
            }
         }
      }
   }
}

void ApplicationWindow::dragEnterEvent(QDragEnterEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   if ((pEvent->proposedAction() != Qt::CopyAction) && (pEvent->proposedAction() != Qt::MoveAction))
   {
      return;
   }

   if (pEvent->mimeData()->hasUrls())
   {
      const QList<QUrl> urls = pEvent->mimeData()->urls();
      for (int i = 0; i < urls.count(); ++i)
      {
         QUrl url = urls[i];
         string filename = url.toLocalFile().toStdString();
         if (filename.empty() == false)
         {
            pEvent->acceptProposedAction();
            return;
         }
      }
   }
   else if (pEvent->mimeData()->hasFormat("text/plain"))
   {
      QString filename = pEvent->mimeData()->text();
      if (filename.isEmpty() == false)
      {
         pEvent->acceptProposedAction();
      }
   }
}

void ApplicationWindow::dropEvent(QDropEvent *pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   if ((pEvent->proposedAction() != Qt::CopyAction) && (pEvent->proposedAction() != Qt::MoveAction))
   {
      return;
   }

   // Get the list of dropped files
   mDropFiles.clear();

   if (pEvent->mimeData()->hasUrls())
   {
      const QList<QUrl> urls = pEvent->mimeData()->urls();
      for (int i=0; i<urls.count(); ++i)
      {
         QUrl url = urls[i];
         string filename = url.toLocalFile().toStdString();
         if (filename.empty() == false)
         {
            mDropFiles.push_back(filename);
         }
      }
   }
   else if (pEvent->mimeData()->hasFormat("text/plain"))
   {
      QString filename = pEvent->mimeData()->text();
      if (filename.isEmpty() == false)
      {
         QByteArray charArray = filename.toAscii();
         for (int i=0; i<filename.length(); ++i)
         {
            if (charArray[i] == '\\')
            {
               charArray[i] = '/';
            }
            else if (charArray[i] == '\n' || charArray[i] == '\r')
            {
               charArray[i] = 0;
               break;
            }
         }
         mDropFiles.push_back(charArray.data());
      }
   }

   if (mDropFiles.empty() == true)
   {
      return;
   }

   // Get the type of files being dropped
   mDropFilesType = DropFilesType();
   for (vector<string>::iterator iter = mDropFiles.begin(); iter != mDropFiles.end(); ++iter)
   {
      QString strFilename = QString::fromStdString(*iter);
      if (strFilename.isEmpty() == false)
      {
         QFileInfo info(strFilename);
         if (info.suffix() == "aeb")            // One or more extension files
         {
            if ((mDropFilesType.isValid() == true) && (mDropFilesType != EXTENSION_FILE))
            {
               mDropFilesType = DropFilesType();
               break;
            }

            mDropFilesType = EXTENSION_FILE;
         }
         else if (info.suffix() == "session")   // One session file
         {
            if (mDropFilesType.isValid() == true)
            {
               mDropFilesType = DropFilesType();
               break;
            }

            mDropFilesType = SESSION_FILE;
         }
         else if (info.suffix() == "wiz")       // One wizard file
         {
            if (mDropFilesType.isValid() == true)
            {
               mDropFilesType = DropFilesType();
               break;
            }

            mDropFilesType = WIZARD_FILE;
         }
         else if (info.suffix() == "batchwiz")  // One batch wizard file
         {
            if (mDropFilesType.isValid() == true)
            {
               mDropFilesType = DropFilesType();
               break;
            }

            mDropFilesType = BATCH_WIZARD_FILE;
         }
         else                                   // One or more data sets
         {
            if ((mDropFilesType.isValid() == true) && (mDropFilesType != DATASET_FILE))
            {
               mDropFilesType = DropFilesType();
               break;
            }

            mDropFilesType = DATASET_FILE;
         }
      }
   }

   if (mDropFilesType.isValid() == false)
   {
      QString msg = "Unable to load the dropped files.\n\nOnly one of the following may be dropped:\n"
         "- One or more extension files (*.aeb)\n- One session file (*.session)\n- One wizard file (*.wiz)\n"
         "- One batch wizard file (*.batchwiz)\n- One or more data set files (*)";
      QMessageBox::warning(this, APP_NAME, msg);
      return;
   }

   // Get context menu options
   mDropEditType = ImportAgent::NEVER_EDIT;
   mDropNewSession = false;

#if defined(WIN_API)
   Qt::MouseButton contextMenuButton = Qt::RightButton;
#else
   Qt::MouseButton contextMenuButton = Qt::MidButton;
#endif

   if (pEvent->mouseButtons() == contextMenuButton)
   {
      if (mDropFilesType == EXTENSION_FILE)
      {
         QMenu contextMenu(this);
         QAction* pInstallExtensionsAction = contextMenu.addAction("Install extensions");
         contextMenu.addSeparator();
         contextMenu.addAction("Cancel");
         contextMenu.setDefaultAction(pInstallExtensionsAction);

         if (contextMenu.exec(mapToGlobal(pEvent->pos())) != pInstallExtensionsAction)
         {
            return;
         }
      }
      else if (mDropFilesType == SESSION_FILE)
      {
         QMenu contextMenu(this);
         QAction* pOpenSessionAction = contextMenu.addAction("Open session");
         contextMenu.addSeparator();
         contextMenu.addAction("Cancel");
         contextMenu.setDefaultAction(pOpenSessionAction);

         if (contextMenu.exec(mapToGlobal(pEvent->pos())) != pOpenSessionAction)
         {
            return;
         }
      }
      else if ((mDropFilesType == WIZARD_FILE) || (mDropFilesType == BATCH_WIZARD_FILE))
      {
         QMenu contextMenu(this);
         QAction* pRunWizardAction = contextMenu.addAction("Run wizard in current session");
         QAction* pRunWizardNewSessionAction = contextMenu.addAction("Run wizard in new session");
         contextMenu.addSeparator();
         QAction* pCancelAction = contextMenu.addAction("Cancel");
         contextMenu.setDefaultAction(pRunWizardAction);

         QAction* pSelectedAction = contextMenu.exec(mapToGlobal(pEvent->pos()));
         if ((pSelectedAction == pCancelAction) || (pSelectedAction == NULL))
         {
            return;
         }

         if (pSelectedAction == pRunWizardNewSessionAction)
         {
            mDropNewSession = true;
         }
      }
      else if (mDropFilesType == DATASET_FILE)
      {
         QMenu contextMenu(this);
         QAction* pAlwaysAction = contextMenu.addAction("Import and show options dialog");
         QAction* pAsNeededAction = contextMenu.addAction("Import and show options dialog only if needed");
         QAction* pNeverAction = contextMenu.addAction("Import and do not show options dialog");
         contextMenu.addSeparator();
         contextMenu.addAction("Cancel");
         contextMenu.setDefaultAction(pNeverAction);

         QAction* pSelectedAction = contextMenu.exec(mapToGlobal(pEvent->pos()));
         if (pSelectedAction == pAlwaysAction)
         {
            mDropEditType = ImportAgent::ALWAYS_EDIT;
         }
         else if (pSelectedAction == pAsNeededAction)
         {
            mDropEditType = ImportAgent::AS_NEEDED_EDIT;
         }
         else if (pSelectedAction == pNeverAction)
         {
            mDropEditType = ImportAgent::NEVER_EDIT;
         }
         else
         {
            return;
         }
      }
   }

   // Accept the drop action
   pEvent->acceptProposedAction();

   // Invoke an immediate timer so that any currently queued events are processed first
   QTimer::singleShot(0, this, SLOT(processDropFiles()));
}

void ApplicationWindow::removeMenuCommands(const QList<QAction*>& commands)
{
   QList<QAction*>::const_iterator commandIter;
   for (commandIter = commands.begin(); commandIter != commands.end();  ++commandIter)
   {
      QAction* pCommandAction = *commandIter;
      if (pCommandAction != NULL)
      {
         bool bRemoved = false;

         // Menu bar
         MenuBarImp* pMenuBar = static_cast<MenuBarImp*>(menuBar());
         if (pMenuBar != NULL)
         {
            bRemoved = pMenuBar->removeMenuItem(pCommandAction);
         }

         // Toolbars
         if (bRemoved == false)
         {
            vector<Window*> toolbars = getWindows(TOOLBAR);

            vector<Window*>::iterator toolbarIter;
            for (toolbarIter = toolbars.begin(); toolbarIter != toolbars.end(); ++toolbarIter)
            {
               ToolBar* pToolBar = static_cast<ToolBar*>(*toolbarIter);
               if (pToolBar != NULL)
               {
                  pMenuBar = static_cast<MenuBarImp*>(pToolBar->getMenuBar());
                  if (pMenuBar != NULL)
                  {
                     bRemoved = pMenuBar->removeMenuItem(pCommandAction);
                     if (bRemoved == true)
                     {
                        QList<QAction*> actions = pMenuBar->actions();
                        if (actions.empty() == true)
                        {
                           bool bEmptyToolBar = true;

                           QList<QAction*> toolbarActions = (dynamic_cast<ToolBarImp*>(pToolBar))->actions();
                           for (int i = 0; i < toolbarActions.count(); ++i)
                           {
                              QAction* pCurrentAction = toolbarActions[i];
                              if (pCurrentAction != NULL)
                              {
                                 QWidgetAction* pWidgetAction = dynamic_cast<QWidgetAction*>(pCurrentAction);
                                 if (pWidgetAction != NULL)
                                 {
                                    if (pWidgetAction->defaultWidget() == pMenuBar)
                                    {
                                       continue;
                                    }
                                 }

                                 if ((pCurrentAction->isSeparator() == false) &&
                                    (pCurrentAction->parentWidget() != pMenuBar))
                                 {
                                    bEmptyToolBar = false;
                                    break;
                                 }
                              }
                           }

                           if (bEmptyToolBar == true)
                           {
                              deleteWindow(pToolBar);
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

void ApplicationWindow::arrangeWorkspaceWindows(QAction* pAction)
{
   TilingType eType;

   if (pAction == mpTileAction)
   {
      eType = TILE_GRID;
   }
   else if (pAction == mpTileHorizontalAction)
   {
      eType = TILE_HORIZONTAL;
   }
   else if (pAction == mpTileVerticalAction)
   {
      eType = TILE_VERTICAL;
   }
   else
   {      
      return;
   }

   tileWorkspaceWindows(eType);
}

void ApplicationWindow::tileSelectedWindows()
{
   WindowSelectionDlg dlg(this);
   if (dlg.exec() == QDialog::Rejected)
   {
      return;
   }

   vector<WorkspaceWindow*> selectedWindows = dlg.getSelectedWindows();
   bool bMaxFirst = dlg.maximizeFirstColumn();

   tileWorkspaceWindows(selectedWindows, bMaxFirst, dlg.getTilingType());
}

void ApplicationWindow::displayProperties(SessionItem* pItem)
{
   vector<string> propertiesPages;
   if (pItem != NULL)
   {
      propertiesPages = pItem->getPropertiesPages();
   }

   displayProperties(pItem, propertiesPages);
}

void ApplicationWindow::displayProperties(SessionItem* pItem, const vector<string>& displayedPages, bool bEmitSignal)
{
   if (pItem == NULL)
   {
      return;
   }

   vector<string> propertiesPages = displayedPages;
   if (bEmitSignal == true)
   {
      vector<string> plugInNames;

      pair<SessionItem*, vector<string>*> properties;
      properties.first = pItem;
      properties.second = &plugInNames;
      notify(SIGNAL_NAME(ApplicationWindow, AboutToShowPropertiesDialog), boost::any(properties));

      if (plugInNames.empty() == false)
      {
         std::copy(plugInNames.begin(), plugInNames.end(), back_inserter(propertiesPages));
      }
   }

   PropertiesDlg dlg(pItem, propertiesPages, this);
   dlg.exec();
}

void ApplicationWindow::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   // Action to edit classification for multiple selected view windows or data elements
   vector<ViewWindow*> viewItems = pMenu->getSessionItems<ViewWindow>();
   vector<DataElement*> elementItems = pMenu->getSessionItems<DataElement>();
   if ((viewItems.size() > 1) || (elementItems.size() > 1))
   {
      QList<QVariant> itemList;
      if (elementItems.size() > 1)
      {
         for (vector<DataElement*>::const_iterator iter = elementItems.begin(); iter != elementItems.end(); ++iter)
         {
            SessionItem* pItem = *iter;
            if (pItem != NULL)
            {
               itemList.append(QVariant::fromValue(pItem));
            }
         }
      }
      else if (viewItems.size() > 1)
      {
         for (vector<ViewWindow*>::const_iterator iter = viewItems.begin(); iter != viewItems.end(); ++iter)
         {
            SessionItem* pItem = (*iter)->getView();
            if (pItem != NULL)
            {
               itemList.append(QVariant::fromValue(pItem));
            }
         }
      }

      QAction* pClassificationAction = new QAction("Edit &Classification...", pMenu->getActionParent());
      pClassificationAction->setAutoRepeat(false);
      pClassificationAction->setStatusTip("Edits the classification for the selected items");
      pClassificationAction->setData(QVariant::fromValue(itemList));
      VERIFYNR(connect(pClassificationAction, SIGNAL(triggered()), this, SLOT(editClassification())));
      pMenu->addAction(pClassificationAction, APP_APPLICATIONWINDOW_EDIT_CLASSIFICATION_ACTION);

      return;
   }

   vector<SessionItem*> sessionItems = pMenu->getSessionItems();
   if (sessionItems.size() != 1)
   {
      if (dynamic_cast<SessionExplorer*>(&subject) != NULL)
      {
         return;
      }
   }

   SessionItem* pItem = sessionItems.front();
   VERIFYNRV(pItem != NULL);

   // Delete element action
   if (dynamic_cast<DataElement*>(pItem) != NULL)
   {
      QAction* pDeleteAction = new QAction(QIcon(":/icons/Delete"), "&Delete", pMenu->getActionParent());
      pDeleteAction->setAutoRepeat(false);
      pDeleteAction->setStatusTip("Destroys the selected data element");
      pDeleteAction->setData(QVariant::fromValue(pItem));
      VERIFYNR(connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(deleteSelectedElement())));
      pMenu->addAction(pDeleteAction, APP_APPLICATIONWINDOW_DATAELEMENT_DELETE_ACTION);
   }

   // Properties and export actions
   ViewWindow *pWindow = dynamic_cast<ViewWindow*>(pItem);
   if (pWindow != NULL)
   {
      View* pView = pWindow->getView();
      if (pView != NULL)
      {
         pItem = pView;
      }
   }

   VERIFYNRV(pItem != NULL);

   // Always add the actions so they are there to be removed or
   // placed, but remove the actions if they are not relevant
   mpPropertiesAction->setData(QVariant::fromValue(pItem));
   pMenu->addAction(mpPropertiesAction, APP_APPLICATIONWINDOW_PROPERTIES_ACTION);

   vector<string> propertyPages = pItem->getPropertiesPages();
   vector<string> plugInNames;

   pair<SessionItem*, vector<string>*> properties;
   properties.first = pItem;
   properties.second = &plugInNames;
   notify(SIGNAL_NAME(ApplicationWindow, AboutToShowPropertiesDialog), boost::any(properties));

   if ((propertyPages.empty() == true) && (plugInNames.empty() == true))
   {
      pMenu->removeAction(APP_APPLICATIONWINDOW_PROPERTIES_ACTION);
   }

   mpExportContextMenuAction->setData(QVariant::fromValue(pItem));
   pMenu->addActionBefore(mpExportContextMenuAction, APP_APPLICATIONWINDOW_EXPORT_ACTION,
      APP_APPLICATIONWINDOW_PROPERTIES_ACTION);
   if (getAvailableExporters(pItem).empty())
   {
      pMenu->removeAction(APP_APPLICATIONWINDOW_EXPORT_ACTION);
   }

   if (dynamic_cast<SpatialDataView*>(pItem) != NULL || dynamic_cast<ProductView*>(pItem) != NULL)
   {
      // Copy Snapshot Actions
      QAction* pViewSeparatorAction = new QAction(pMenu->getActionParent());
      pViewSeparatorAction->setSeparator(true);
      string beforeAction = APP_SPATIALDATAVIEW_PROPERTIES_SEPARATOR_ACTION;
      if (dynamic_cast<ProductView*>(pItem) != NULL)
      {
         beforeAction = APP_PRODUCTVIEW_PROPERTIES_SEPARATOR_ACTION;
      }
      pMenu->addActionBefore(pViewSeparatorAction, APP_APPLICATIONWINDOW_COPY_SNAPSHOT_SEPARATOR_ACTION, beforeAction);
      pMenu->addActionBefore(mpClipboardAction, APP_APPLICATIONWINDOW_COPY_SNAPSHOT_ACTION, beforeAction);
      pMenu->addActionBefore(mpClipboardSizedAction, APP_APPLICATIONWINDOW_COPY_SNAPSHOT_SIZED_ACTION, beforeAction);
   }
}

void ApplicationWindow::sessionAboutToRestore(Subject& subject, const string& signal, const boost::any& value)
{
   mPreviousSize = size();
}

void ApplicationWindow::sessionLoaded(Subject& subject, const string& signal, const boost::any& value)
{
   clearUndoStacks();
   resize(mPreviousSize);
}

void ApplicationWindow::exportFileMenu()
{
   SessionItem *pItem = NULL;

   pItem = getCurrentWorkspaceWindowView();
   SpatialDataView *pSpatialDataView = dynamic_cast<SpatialDataView*>(pItem);
   if (pSpatialDataView != NULL)
   {
      LayerList *pLayerList = pSpatialDataView->getLayerList();
      VERIFYNRV(pLayerList != NULL);
      pItem = pLayerList->getPrimaryRasterElement();
   }

   exportSessionItem(pItem);
}

void ApplicationWindow::deleteSelectedElement()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction == NULL)
   {
      return;
   }

   SessionItem* pItem = pAction->data().value<SessionItem*>();
   DataElement* pElement = dynamic_cast<DataElement*>(pItem);
   VERIFYNRV(pElement != NULL);

   Service<ModelServices> pModel;
   pModel->destroyElement(pElement);
}

void ApplicationWindow::editClassification()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction == NULL)
   {
      return;
   }

   QDialog classificationDialog(this);
   classificationDialog.setWindowTitle("Edit Classification");

   ClassificationWidget* pClassificationWidget = new ClassificationWidget(&classificationDialog);
   QFrame* pLine = new QFrame(&classificationDialog);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal,
      &classificationDialog);
   QVBoxLayout* pLayout = new QVBoxLayout(&classificationDialog);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pClassificationWidget, 10);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);

   FactoryResource<Classification> pClassification;
   pClassificationWidget->setClassification(pClassification.get(), false);

   QPushButton* pOkButton = pButtonBox->button(QDialogButtonBox::Ok);
   pOkButton->setEnabled(false);

   VERIFYNR(connect(pClassificationWidget, SIGNAL(modified(bool)), pOkButton, SLOT(setEnabled(bool))));
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &classificationDialog, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &classificationDialog, SLOT(reject())));

   if (classificationDialog.exec() == QDialog::Rejected)
   {
      return;
   }

   if (pClassificationWidget->applyChanges() == true)
   {
      QList<QVariant> itemList = pAction->data().toList();
      for (int i = 0; i < itemList.count(); ++i)
      {
         SessionItem* pItem = itemList[i].value<SessionItem*>();

         DataElement* pElement = dynamic_cast<DataElement*>(pItem);
         if (pElement != NULL)
         {
            pElement->setClassification(pClassification.get());
         }

         View* pView = pView = dynamic_cast<View*>(pItem);
         if (pView != NULL)
         {
            pView->setClassification(pClassification.get());
         }
      }
   }
}

void ApplicationWindow::exportSessionItem()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction == NULL)
   {
      return;
   }

   SessionItem* pItem = pAction->data().value<SessionItem*>();

   ViewWindow* pWindow = dynamic_cast<ViewWindow*>(pItem);
   if (pWindow != NULL)
   {
      View* pView = pWindow->getView();
      if (pView != NULL)
      {
         pItem = pView;
      }
   }

   VERIFYNRV(pItem != NULL);

   exportSessionItem(pItem);
}

void ApplicationWindow::displaySessionItemProperties()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction == NULL)
   {
      return;
   }

   SessionItem* pItem = pAction->data().value<SessionItem*>();

   ViewWindow* pWindow = dynamic_cast<ViewWindow*>(pItem);
   if (pWindow != NULL)
   {
      View* pView = pWindow->getView();
      if (pView != NULL)
      {
         pItem = pView;
      }
   }

   VERIFYNRV(pItem != NULL);

   displayProperties(pItem);
}

const vector<PlugInDescriptor*> &ApplicationWindow::getAvailableExporters(const SessionItem *pItem) const
{
   static vector<PlugInDescriptor*> availableExporters;
   static const SessionItem *pLastItem = NULL;

   if (pLastItem == pItem)
   {
      return availableExporters;
   }

   availableExporters.clear();

   const TypeAwareObject *pTao = dynamic_cast<const TypeAwareObject*>(pItem);
   if (pTao == NULL)
   {
      return availableExporters;
   }

   Service<PlugInManagerServices> pPims;
   vector<PlugInDescriptor*> exporters = pPims->getPlugInDescriptors(PlugInManagerServices::ExporterType());

   for (vector<PlugInDescriptor*>::const_iterator iter = exporters.begin(); iter != exporters.end(); ++iter)
   {
      PlugInDescriptor *pDescriptor = *iter;
      LOG_IF(pDescriptor == NULL, continue);

      string subtype = pDescriptor->getSubtype();

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove SignatureSet special case when SignatureSet " \
   "no longer inherits Signature. (dsulgrov)")
      if ((pTao->isKindOf(TypeConverter::toString<SignatureSet>()) == true) &&
         (subtype == TypeConverter::toString<Signature>()))
      {
         continue;
      }

      if (pTao->isKindOf(subtype))
      {
         availableExporters.push_back(*iter);
      }
   }
   pLastItem = pItem;
   return availableExporters;
}

GraphicLayer* ApplicationWindow::getClipboardLayer() const
{
   GraphicLayer* pLayer = NULL;

   View* pView = getCurrentWorkspaceWindowView();
   if (pView != NULL)
   {
      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
      if (pSpatialDataView != NULL)
      {
         pLayer = dynamic_cast<GraphicLayer*>(pSpatialDataView->getTopMostLayer(ANNOTATION));
      }

      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         pLayer = pProductView->getLayoutLayer();
      }
   }

   return pLayer;
}

bool ApplicationWindow::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter writer("ApplicationWindow");

   // save active window id
   WorkspaceWindow* pWork = getCurrentWorkspaceWindow();
   if (pWork != NULL)
   {
      writer.addAttr("currentWorkspaceWindowId", pWork->getId());
   }

   writer.pushAddPoint(writer.addElement("Geometry"));
   writer.addText(saveState().toBase64().data());
   writer.popAddPoint();

   serializer.serialize(writer);
   return true;
}

bool ApplicationWindow::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement *pElement = deserializer.deserialize(reader, "ApplicationWindow");

   // restore the app window geometry
   DOMElement *pConfig = dynamic_cast<DOMElement*>(findChildNode(pElement, "Geometry"));
   if(pConfig)
   {
      DOMNode *pChld = pConfig->getFirstChild();
      if (pChld != NULL)
      {
         restoreState(QByteArray::fromBase64(A(pChld->getNodeValue())));
      }
   }

   // set active window
   WorkspaceWindow* pWork(NULL);
   if (pElement->hasAttribute(X("currentWorkspaceWindowId")))
   {
      string winId(A(pElement->getAttribute(X("currentWorkspaceWindowId"))));
      pWork = dynamic_cast<WorkspaceWindow*>(SessionManagerImp::instance()->getSessionItem(winId));
   }
   if(mpWorkspace != NULL)
   {
      mpWorkspace->setActiveSubWindow(NULL);
   }
   setCurrentWorkspaceWindow(pWork);

   return true;
}

void ApplicationWindow::processDropFiles()
{
   VERIFYNRV(mDropFiles.empty() == false);

   if (mDropFilesType == EXTENSION_FILE)
   {
      QList<Aeb*> extensions;
      AebListResource extensionRes;
      ProgressResource pProgress(APP_NAME);

      for (vector<string>::size_type i = 0; i < mDropFiles.size(); ++i)
      {
         string filename = mDropFiles[i];
         if (filename.empty() == false)
         {
            pProgress->updateProgress("Processing extension files...", i * 100 / mDropFiles.size(), NORMAL);

            extensionRes.push_back(new Aeb());
            AebIo deserializer(*extensionRes.back());

            string errorMessage;
            if (deserializer.fromFile(filename, errorMessage) == false)
            {
               pProgress->updateProgress("Invalid extension bundle: " + filename + "\n" + errorMessage, 0, WARNING);
            }
            else
            {
               if (!extensionRes.back()->checkTargetApplication(errorMessage))
               {
                  pProgress->updateProgress("Invalid extension bundle: " + extensionRes.back()->getName() + "\n" + errorMessage, 0, WARNING);
               }
               else
               {
                  extensions.push_back(extensionRes.back());
               }
            }
         }
      }

      pProgress->updateProgress("Finished processing extension files", 100, NORMAL);

      if (extensions.empty() == false)
      {
         InstallWizard wiz(extensions, pProgress.get(), this);
         wiz.exec();
      }
   }
   else if (mDropFilesType == SESSION_FILE)
   {
      VERIFYNRV(mDropFiles.size() == 1);
      openSession(QString::fromStdString(mDropFiles.front()));
   }
   else if ((mDropFilesType == WIZARD_FILE) || (mDropFilesType == BATCH_WIZARD_FILE))
   {
      VERIFYNRV(mDropFiles.size() == 1);

      if (mDropNewSession == true)
      {
         if (newSession() == false)
         {
            return;
         }
      }

      if (mDropFilesType == WIZARD_FILE)
      {
         runWizard(QString::fromStdString(mDropFiles.front()));
      }
      else
      {
         WizardUtilities::runBatchFiles(mDropFiles, NULL);
      }
   }
   else if (mDropFilesType == DATASET_FILE)
   {
      ImporterResource importer("Auto Importer", mDropFiles, NULL, false);
      importer->setEditType(mDropEditType);
      importer->createProgressDialog(true);
      importer->updateMruFileList(true);
      importer->execute();
   }
}

void ApplicationWindow::showToolbarsMenu()
{
   mpToolbarsMenu->clear();

   vector<Window*> windows = getWindows(TOOLBAR);
   vector<Window*>::iterator winIt;
   for (winIt = windows.begin(); winIt != windows.end(); ++winIt)
   {
      QToolBar* pToolbar = dynamic_cast<QToolBar*>(*winIt);
      if (pToolbar != NULL)
      {
         mpToolbarsMenu->addAction(pToolbar->toggleViewAction());
      }
   }
}
