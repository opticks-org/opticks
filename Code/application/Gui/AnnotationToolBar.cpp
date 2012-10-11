/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QActionEvent>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>

#include "AnnotationLayerAdapter.h"
#include "AnnotationToolBar.h"
#include "AppVerify.h"
#include "ClassificationLayer.h"
#include "ColorMenu.h"
#include "DesktopServices.h"
#include "FontSizeComboBox.h"
#include "GraphicObjectImp.h"
#include "GraphicObjectTypeGrid.h"
#include "GraphicUtilities.h"
#include "MenuBarImp.h"
#include "PixmapGrid.h"
#include "PlotView.h"
#include "ProductView.h"
#include "Service.h"
#include "Undo.h"

#include <string>
#include <vector>
using namespace std;

AnnotationToolBar::AnnotationToolBar(const string& id, QWidget* parent) :
   ToolBarAdapter(id, "Annotation", parent),
   mpLayerLocked(NULL),
   mpObjectGroup(NULL),
   mpMove(NULL),
   mpRotate(NULL),
   mpObject(NULL),
   mpFont_Combo(NULL),
   mpSize_Combo(NULL),
   mpBold(NULL),
   mpItalic(NULL),
   mpUnderline(NULL),
   mpTextColorMenu(NULL),
   mpLineWidthMenu(NULL),
   mpLineWidthGrid(NULL),
   mpLineColorMenu(NULL),
   mpFillColorMenu(NULL),
   mpSnapToGrid(NULL),
   mpAnnotationLayer(NULL)
{
   Service<ConfigurationSettings> pSettings;
   Service<DesktopServices> pDesktop;
   pSettings->attach(SIGNAL_NAME(ConfigurationSettings, SettingModified),
      Slot(this, &AnnotationToolBar::optionsModified));

   string shortcutContext = windowTitle().toStdString();

   // Add the annotation menu
   MenuBarImp* pMenuBar = static_cast<MenuBarImp*>(getMenuBar());
   if (pMenuBar != NULL)
   {
      // Annotation menu
      QMenu* pAnnotationMenu = new QMenu(windowTitle(), pMenuBar);
      if (pAnnotationMenu != NULL)
      {
         pMenuBar->insertMenu(pAnnotationMenu);

         // Group menu
         QMenu* pGroupMenu = pAnnotationMenu->addMenu("&Group");
         if (pGroupMenu != NULL)
         {
            // Group
            QAction* pGroupAction = new QAction(QIcon(":/icons/Group"), "&Group", this);
            pGroupAction->setAutoRepeat(false);
            pGroupAction->setToolTip("Group");
            pGroupAction->setStatusTip("Groups the selected objects");
            VERIFYNR(connect(pGroupAction, SIGNAL(triggered()), this, SLOT(group())));
            pMenuBar->insertCommand(pGroupAction, pGroupMenu, shortcutContext);

            // Ungroup
            QAction* pUngroupAction = new QAction(QIcon(":/icons/Ungroup"), "&Ungroup", this);
            pUngroupAction->setAutoRepeat(false);
            pUngroupAction->setToolTip("Ungroup");
            pUngroupAction->setStatusTip("Ungroups the selected objects");
            VERIFYNR(connect(pUngroupAction, SIGNAL(triggered()), this, SLOT(ungroup())));
            pMenuBar->insertCommand(pUngroupAction, pGroupMenu, shortcutContext);
         }

         // Draw order menu
         QMenu* pOrderMenu = pAnnotationMenu->addMenu("&Order");
         if (pOrderMenu != NULL)
         {
            // Pop to front
            QAction* pFrontAction = new QAction(QIcon(":/icons/PopFront"), "To &Front", this);
            pFrontAction->setAutoRepeat(false);
            pFrontAction->setToolTip("Pop To Front");
            pFrontAction->setStatusTip("Moves the object to the front");
            VERIFYNR(connect(pFrontAction, SIGNAL(triggered()), this, SLOT(popFront())));
            pMenuBar->insertCommand(pFrontAction, pOrderMenu, shortcutContext);

            // Push to back
            QAction* pBackAction = new QAction(QIcon(":/icons/PushBack"), "To &Back", this);
            pBackAction->setAutoRepeat(false);
            pBackAction->setToolTip("Push To Back");
            pBackAction->setStatusTip("Moves the object to the back");
            VERIFYNR(connect(pBackAction, SIGNAL(triggered()), this, SLOT(pushBack())));
            pMenuBar->insertCommand(pBackAction, pOrderMenu, shortcutContext);
         }

         // Nudge menu
         QMenu* pNudgeMenu = pAnnotationMenu->addMenu("&Nudge");
         if (pNudgeMenu != NULL)
         {
            string nudgeContext = shortcutContext + string("/Nudge");

            // Up
            QAction* pUpAction = new QAction(QIcon(":/icons/NudgeUp"), "&Up", this);
            pUpAction->setShortcut(QKeySequence("Ctrl+Up"));
            pUpAction->setStatusTip("Moves the object up by a small amount");
            VERIFYNR(connect(pUpAction, SIGNAL(triggered()), this, SLOT(nudgeUp())));
            pMenuBar->insertCommand(pUpAction, pNudgeMenu, nudgeContext);

            // Down
            QAction* pDownAction = new QAction(QIcon(":/icons/NudgeDown"), "&Down", this);
            pDownAction->setShortcut(QKeySequence("Ctrl+Down"));
            pDownAction->setStatusTip("Moves the object down by a small amount");
            VERIFYNR(connect(pDownAction, SIGNAL(triggered()), this, SLOT(nudgeDown())));
            pMenuBar->insertCommand(pDownAction, pNudgeMenu, nudgeContext);

            // Left
            QAction* pLeftAction = new QAction(QIcon(":/icons/NudgeLeft"), "&Left", this);
            pLeftAction->setShortcut(QKeySequence("Ctrl+Left"));
            pLeftAction->setStatusTip("Moves the object left by a small amount");
            VERIFYNR(connect(pLeftAction, SIGNAL(triggered()), this, SLOT(nudgeLeft())));
            pMenuBar->insertCommand(pLeftAction, pNudgeMenu, nudgeContext);

            // Right
            QAction* pRightAction = new QAction(QIcon(":/icons/NudgeRight"), "&Right", this);
            pRightAction->setShortcut(QKeySequence("Ctrl+Right"));
            pRightAction->setStatusTip("Moves the object right by a small amount");
            VERIFYNR(connect(pRightAction, SIGNAL(triggered()), this, SLOT(nudgeRight())));
            pMenuBar->insertCommand(pRightAction, pNudgeMenu, nudgeContext);
         }

         // Align menu
         QMenu* pAlignMenu = pAnnotationMenu->addMenu("&Align and Distribute");
         if (pAlignMenu != NULL)
         {
            string alignContext = shortcutContext + string("/Align and Distribute");

            // Left
            QAction* pLeftAction = new QAction(QIcon(":/icons/AlignLeft"), "Align &Left", this);
            pLeftAction->setAutoRepeat(false);
            VERIFYNR(connect(pLeftAction, SIGNAL(triggered()), this, SLOT(alignLeft())));
            pMenuBar->insertCommand(pLeftAction, pAlignMenu, alignContext);

            // Center
            QAction* pCenterAction = new QAction(QIcon(":/icons/AlignCenter"), "Align &Center", this);
            pCenterAction->setAutoRepeat(false);
            VERIFYNR(connect(pCenterAction, SIGNAL(triggered()), this, SLOT(alignCenter())));
            pMenuBar->insertCommand(pCenterAction, pAlignMenu, alignContext);

            // Right
            QAction* pRightAction = new QAction(QIcon(":/icons/AlignRight"), "Align &Right", this);
            pRightAction->setAutoRepeat(false);
            VERIFYNR(connect(pRightAction, SIGNAL(triggered()), this, SLOT(alignRight())));
            pMenuBar->insertCommand(pRightAction, pAlignMenu, alignContext);

            pAlignMenu->addSeparator();

            // Top
            QAction* pTopAction = new QAction(QIcon(":/icons/AlignTop"), "Align &Top", this);
            pTopAction->setAutoRepeat(false);
            VERIFYNR(connect(pTopAction, SIGNAL(triggered()), this, SLOT(alignTop())));
            pMenuBar->insertCommand(pTopAction, pAlignMenu, alignContext);

            // Middle
            QAction* pMiddleAction = new QAction(QIcon(":/icons/AlignMiddle"), "Align &Middle", this);
            pMiddleAction->setAutoRepeat(false);
            VERIFYNR(connect(pMiddleAction, SIGNAL(triggered()), this, SLOT(alignMiddle())));
            pMenuBar->insertCommand(pMiddleAction, pAlignMenu, alignContext);

            // Bottom
            QAction* pBottomAction = new QAction(QIcon(":/icons/AlignBottom"), "Align &Bottom", this);
            pBottomAction->setAutoRepeat(false);
            VERIFYNR(connect(pBottomAction, SIGNAL(triggered()), this, SLOT(alignBottom())));
            pMenuBar->insertCommand(pBottomAction, pAlignMenu, alignContext);

            pAlignMenu->addSeparator();

            // Distribute horizontally
            QAction* pHorizontalAction = new QAction(QIcon(":/icons/DistributeHorizontally"),
               "Distribute &Horizontally", this);
            pHorizontalAction->setAutoRepeat(false);
            VERIFYNR(connect(pHorizontalAction, SIGNAL(triggered()), this, SLOT(distributeHorizontally())));
            pMenuBar->insertCommand(pHorizontalAction, pAlignMenu, alignContext);

            // Distribute vertically
            QAction* pVerticalAction = new QAction(QIcon(":/icons/DistributeVertically"), "Distribute &Vertically", this);
            pVerticalAction->setAutoRepeat(false);
            VERIFYNR(connect(pVerticalAction, SIGNAL(triggered()), this, SLOT(distributeVertically())));
            pMenuBar->insertCommand(pVerticalAction, pAlignMenu, alignContext);
         }

         pAnnotationMenu->addSeparator();

         // Select all action
         QAction* pSelectAllAction = new QAction("&Select All", this);
         pSelectAllAction->setAutoRepeat(false);
         pSelectAllAction->setShortcut(QKeySequence("Ctrl+A"));
         pSelectAllAction->setToolTip("Select All");
         pSelectAllAction->setStatusTip("Selects all objects in the active annotation layer");
         VERIFYNR(connect(pSelectAllAction, SIGNAL(triggered()), this, SLOT(selectAll())));
         pMenuBar->insertCommand(pSelectAllAction, pAnnotationMenu, shortcutContext);

         pAnnotationMenu->addSeparator();

         // Properties action
         QAction* pPropertiesAction = new QAction(QIcon(":/icons/Properties"), "&Properties", this);
         pPropertiesAction->setAutoRepeat(false);
         pPropertiesAction->setShortcut(QKeySequence("Ctrl+P"));
         pPropertiesAction->setToolTip("Properties");
         pPropertiesAction->setStatusTip("Invokes the Properties dialog for the active annotation layer");
         VERIFYNR(connect(pPropertiesAction, SIGNAL(triggered()), this, SLOT(modifyLayerProperties())));
         pMenuBar->insertCommand(pPropertiesAction, pAnnotationMenu, shortcutContext);
      }

      addSeparator();

      // Lock Layer check box
      mpLayerLocked = new QCheckBox("Lock", this);
      mpLayerLocked->setToolTip("If checked, the layer is locked. Click to toggle on and off");
      VERIFYNR(connect(mpLayerLocked, SIGNAL(toggled(bool)), this, SLOT(setLayerLocked(bool))));
      addWidget(mpLayerLocked);

      addSeparator();

      // Annotation object group
      string mouseModeContext = shortcutContext + string("/Mouse Mode");

      mpObjectGroup = new QButtonGroup(this);
      if (mpObjectGroup != NULL)
      {
         mpObjectGroup->setExclusive(true);
         VERIFYNR(connect(mpObjectGroup, SIGNAL(buttonClicked(int)), this, SLOT(selectionObjectChanged())));

         QToolButton* pMoveButton = new QToolButton(this);
         mpMove = new QAction(QIcon(":/icons/Pan"), "Move Object", 0);
         pDesktop->initializeAction(mpMove, mouseModeContext);
         pMoveButton->setDefaultAction(mpMove);
         mpMove->setAutoRepeat(false);
         mpMove->setCheckable(true);
         mpMove->setStatusTip("Allows selection, placement and resizing of objects");
         mpObjectGroup->addButton(pMoveButton);
         addWidget(pMoveButton);

         QToolButton* pRotateButton = new QToolButton(this);
         mpRotate = new QAction(QIcon(":/icons/FreeRotate"), "Rotate Object", 0);
         pDesktop->initializeAction(mpRotate, mouseModeContext);
         pRotateButton->setDefaultAction(mpRotate);
         mpRotate->setAutoRepeat(false);
         mpRotate->setCheckable(true);
         mpRotate->setStatusTip("Allows the rotation of objects");
         mpObjectGroup->addButton(pRotateButton);
         addWidget(pRotateButton);

         mpObject = new GraphicObjectTypeButton(GraphicObjectTypeGrid::VIEW_ANNOTATION, this);
         mpObject->setCheckable(true);
         mpObject->setAutoRepeat(false);
         mpObject->setStatusTip("Adds a new annotation object to the annotation layer at the "
            "mouse location");
         mpObject->setToolTip("Add New Object");
         mpObject->setClickShowsMenu(false);
         mpObjectGroup->addButton(mpObject);
         addWidget(mpObject);

         VERIFYNR(connect(mpObject, SIGNAL(valueChanged(GraphicObjectType)), this,
            SLOT(setSelectionObject(GraphicObjectType))));

         addSeparator();
      }

      setSelectionObject(RECTANGLE_OBJECT);

      // Font combo
      mpFont_Combo = new QFontComboBox(this);
      mpFont_Combo->setEditable(false);
      mpFont_Combo->setStatusTip("Lists available system fonts for the annotation text boxes");
      mpFont_Combo->setToolTip("Font");
      VERIFYNR(connect(mpFont_Combo, SIGNAL(activated(const QString&)), this, SLOT(setFontFace(const QString&))));


      string fontFace = GraphicLayer::getSettingTextFont();
      if (!fontFace.empty())
      {
         QFont font(QString::fromStdString(fontFace));
         mpFont_Combo->setCurrentFont(font);
      }

      addWidget(mpFont_Combo);

      // Font size combo
      mpSize_Combo = new FontSizeComboBox(this);
      mpSize_Combo->setStatusTip("Lists available font sizes for the annotation text boxes");
      mpSize_Combo->setToolTip("Font Size");
      mpSize_Combo->setCurrentValue(GraphicLayer::getSettingTextFontSize());
      VERIFYNR(connect(mpSize_Combo, SIGNAL(valueEdited(int)), this, SLOT(setFontSize(int))));
      VERIFYNR(connect(mpSize_Combo, SIGNAL(valueActivated(int)), this, SLOT(setFontSize(int))));
      addWidget(mpSize_Combo);

      // Font modifier buttons
      mpBold = new QAction(QIcon(":/icons/Bold"), "Bold", this);
      mpBold->setAutoRepeat(false);
      mpBold->setCheckable(true);
      mpBold->setStatusTip("Toggles the Bold state of the selected text objects");
      VERIFYNR(connect(mpBold, SIGNAL(triggered(bool)), this, SLOT(setFontBold(bool))));
      addButton(mpBold, shortcutContext);

      mpItalic = new QAction(QIcon(":/icons/Italics"), "Italic", this);
      mpItalic->setAutoRepeat(false);
      mpItalic->setCheckable(true);
      mpItalic->setStatusTip("Toggles the Italic state of the selected text objects");
      VERIFYNR(connect(mpItalic, SIGNAL(triggered(bool)), this, SLOT(setFontItalic(bool))));
      addButton(mpItalic, shortcutContext);

      mpUnderline = new QAction(QIcon(":/icons/Underline"), "Underline", this);
      mpUnderline->setAutoRepeat(false);
      mpUnderline->setCheckable(true);
      mpUnderline->setStatusTip("Toggles the Underline state of the selected text objects");
      VERIFYNR(connect(mpUnderline, SIGNAL(triggered(bool)), this, SLOT(setFontUnderline(bool))));
      addButton(mpUnderline, shortcutContext);

      addSeparator();

      // Text color button
      mpTextColorMenu = new ColorMenu(this);
      if (mpTextColorMenu != NULL)
      {
         QAction* pTextColorAction = mpTextColorMenu->menuAction();
         if (pTextColorAction != NULL)
         {
            pTextColorAction->setIcon(QIcon(":/icons/TextColor"));
            pTextColorAction->setStatusTip("Changes the text color of the selected text boxes "
               "on the current annotation layer");
            pTextColorAction->setToolTip("Text Color");
            VERIFYNR(connect(pTextColorAction, SIGNAL(triggered()), mpTextColorMenu, SLOT(setCustomColor())));

            addAction(pTextColorAction);
         }

         VERIFYNR(connect(mpTextColorMenu, SIGNAL(aboutToShow()), this, SLOT(initializeTextColorMenu())));
         VERIFYNR(connect(mpTextColorMenu, SIGNAL(colorSelected(const QColor&)), this,
            SLOT(setTextColor(const QColor&))));
      }

      // Line width button
      mpLineWidthMenu = new QMenu(this);
      if (mpLineWidthMenu != NULL)
      {
         mpLineWidthGrid = new PixmapGrid(mpLineWidthMenu);
         mpLineWidthGrid->setCellTracking(true);
         mpLineWidthGrid->installEventFilter(this);
         mpLineWidthGrid->setNumRows(7);
         mpLineWidthGrid->setNumColumns(1);
         mpLineWidthGrid->setPixmap(0, 0, QPixmap(), "NoLine", "No Line");

         for (int i = 1; i < 7; ++i)
         {
            QPixmap pix(100, i);
            pix.fill(Qt::black);
            mpLineWidthGrid->setPixmap(i, 0, pix,
               QString::number(i), "Line Width: " + QString::number(i));
         }

         QWidgetAction* pGridAction = new QWidgetAction(mpLineWidthMenu);
         pGridAction->setDefaultWidget(mpLineWidthGrid);
         mpLineWidthMenu->addAction(pGridAction);

         QAction* pLineWidthAction = mpLineWidthMenu->menuAction();
         if (pLineWidthAction != NULL)
         {
            pLineWidthAction->setIcon(QIcon(":/icons/LineWidth"));
            pLineWidthAction->setStatusTip("Changes the line width of the selected drawing objects on the "
               "current annotation layer");
            pLineWidthAction->setToolTip("Line Width");

            QToolButton* pLineWidthButton = new QToolButton(this);
            pLineWidthButton->setDefaultAction(pLineWidthAction);
            addWidget(pLineWidthButton);
            VERIFYNR(connect(pLineWidthAction, SIGNAL(triggered()), pLineWidthButton, SLOT(showMenu())));
         }

         VERIFYNR(connect(mpLineWidthMenu, SIGNAL(aboutToShow()), this, SLOT(initializeLineWidthGrid())));
      }

      // Line color button
      mpLineColorMenu = new ColorMenu(this);
      if (mpLineColorMenu != NULL)
      {
         QAction* pLineColorAction = mpLineColorMenu->menuAction();
         if (pLineColorAction != NULL)
         {
            pLineColorAction->setIcon(QIcon(":/icons/LineColor"));
            pLineColorAction->setStatusTip("Changes the line color of the selected drawing objects "
               "on the current annotation layer");
            pLineColorAction->setToolTip("Line Color");
            VERIFYNR(connect(pLineColorAction, SIGNAL(triggered()), mpLineColorMenu, SLOT(setCustomColor())));

            addAction(pLineColorAction);
         }

         VERIFYNR(connect(mpLineColorMenu, SIGNAL(aboutToShow()), this, SLOT(initializeLineColorMenu())));
         VERIFYNR(connect(mpLineColorMenu, SIGNAL(colorSelected(const QColor&)), this,
            SLOT(setLineColor(const QColor&))));
      }

      // Fill color button
      mpFillColorMenu = new ColorMenu(this);
      if (mpFillColorMenu != NULL)
      {
         QAction* pInsertAction = NULL;

         QList<QAction*> menuActions = mpFillColorMenu->actions();
         if (menuActions.empty() == false)
         {
            pInsertAction = menuActions.front();
         }

         QAction* pNoFillAction = new QAction("&No Fill", this);
         pNoFillAction->setAutoRepeat(false);
         pNoFillAction->setStatusTip("Sets the fill style of the selected objects to empty");
         pNoFillAction->setToolTip("No Fill");
         VERIFYNR(connect(pNoFillAction, SIGNAL(triggered()), this, SLOT(setFillStateFalse())));

         pDesktop->initializeAction(pNoFillAction, shortcutContext);

         mpFillColorMenu->insertAction(pInsertAction, pNoFillAction);
         mpFillColorMenu->insertSeparator(pInsertAction);

         QAction* pFillColorAction = mpFillColorMenu->menuAction();
         if (pFillColorAction != NULL)
         {
            pFillColorAction->setIcon(QIcon(":/icons/FillColor"));
            pFillColorAction->setStatusTip("Changes the fill color of the selected drawing objects "
               "on the current annotation layer");
            pFillColorAction->setToolTip("Fill Color");
            VERIFYNR(connect(pFillColorAction, SIGNAL(triggered()), mpFillColorMenu, SLOT(setCustomColor())));

            addAction(pFillColorAction);
         }

         VERIFYNR(connect(mpFillColorMenu, SIGNAL(aboutToShow()), this, SLOT(initializeFillColorMenu())));
         VERIFYNR(connect(mpFillColorMenu, SIGNAL(colorSelected(const QColor&)), this,
            SLOT(setFillColor(const QColor&))));
      }

      addSeparator();

      // Snap-to-grid button
      mpSnapToGrid = new QAction(QIcon(":/icons/SnapGrid"), "Snap to Grid", this);
      mpSnapToGrid->setAutoRepeat(false);
      mpSnapToGrid->setCheckable(true);
      mpSnapToGrid->setStatusTip("Forces moved objects to be at integer scene coordinates");
      VERIFYNR(connect(mpSnapToGrid, SIGNAL(toggled(bool)), this, SIGNAL(snapToGridChanged(bool))));
      addButton(mpSnapToGrid, shortcutContext);
   }
}

AnnotationToolBar::~AnnotationToolBar()
{
   Service<ConfigurationSettings> pSettings;
   pSettings->detach(SIGNAL_NAME(ConfigurationSettings, SettingModified),
      Slot(this, &AnnotationToolBar::optionsModified));

   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->detach(SIGNAL_NAME(Subject, Deleted), 
         Slot(this, &AnnotationToolBar::annotationLayerDeleted));
   }
}

void AnnotationToolBar::optionsModified(Subject &subject, const string &signal, const boost::any &v)
{
   if (NN(dynamic_cast<ConfigurationSettings*>(&subject)))
   {
      VERIFYNR(signal == SIGNAL_NAME(ConfigurationSettings, SettingModified));
      string key = boost::any_cast<string>(v);
      if (key.find("GraphicLayer") == 0)
      {
         updateDefaultProperties();
      }
   }
}

Layer* AnnotationToolBar::getAnnotationLayer() const
{
   return dynamic_cast<Layer*>(mpAnnotationLayer);
}

void AnnotationToolBar::enableSelectionTools()
{
   // Determine if the layer is one of the layers in a product view
   bool bPlot = false;
   bool bProduct = false;
   bool bClassification = false;

   AnnotationLayer* pLayer = dynamic_cast<AnnotationLayer*>(mpAnnotationLayer);
   if (pLayer != NULL)
   {
      ProductView* pProductView = dynamic_cast<ProductView*>(pLayer->getView());
      if (pProductView != NULL)
      {
         bProduct = true;

         if (pLayer == pProductView->getClassificationLayer())
         {
            bClassification = true;
         }
      }

      if (dynamic_cast<PlotView*>(pLayer->getView()) != NULL)
      {
         bPlot = true;
      }
   }

   // Enable the specialized buttons
   mpMove->setEnabled(!bClassification);
   mpRotate->setEnabled(!bProduct && !bPlot);
   mpObject->setEnabled(!bClassification);
   mpSnapToGrid->setEnabled(!bClassification);

   // Ensure that the product-specific selection objects are not selected
   if (bProduct == false)
   {
      GraphicObjectType currentObject = mpObject->getCurrentValue();
      if (!currentObject.isValid())
      {
         mpObject->setCurrentValue(RECTANGLE_OBJECT);
         if (mpObject->isChecked())
         {
            selectionObjectChanged();
         }
      }
   }
}

bool AnnotationToolBar::setAnnotationLayer(Layer* pLayer)
{
   AnnotationLayerImp* pAnnoLayer = dynamic_cast<AnnotationLayerImp*>(pLayer);
   if (pLayer != NULL && pAnnoLayer == NULL)
   {
      return false;
   }

   if (pAnnoLayer == mpAnnotationLayer)
   {
      return false;
   }

   if (mpAnnotationLayer != NULL)
   {
      disconnect(mpAnnotationLayer, SIGNAL(modified()), this, SLOT(updateSelectedProperties()));
      disconnect(mpAnnotationLayer, SIGNAL(objectsSelected(std::list<GraphicObject*>&)),
         this, SLOT(updateSelectedProperties(std::list<GraphicObject*>&)));
      disconnect(mpAnnotationLayer, SIGNAL(currentTypeChanged(GraphicObjectType)),
         this, SLOT(setSelectionObject(GraphicObjectType)));
      disconnect(this, SIGNAL(graphicObjectTypeChanged(GraphicObjectType)),
         mpAnnotationLayer, SLOT(setCurrentGraphicObjectType(GraphicObjectType)));
      disconnect(mpAnnotationLayer, SIGNAL(snapToGridChanged(bool)), 
         this, SLOT(setSnapToGrid(bool)));
      disconnect(this, SIGNAL(snapToGridChanged(bool)), 
         mpAnnotationLayer, SLOT(setSnapToGrid(bool)));
      mpAnnotationLayer->detach(SIGNAL_NAME(Subject, Deleted), 
         Slot(this, &AnnotationToolBar::annotationLayerDeleted));
   }

   mpAnnotationLayer = pAnnoLayer;

   if (mpAnnotationLayer != NULL)
   {
      VERIFYNR(connect(mpAnnotationLayer, SIGNAL(modified()), this, SLOT(updateSelectedProperties())));
      VERIFYNR(connect(mpAnnotationLayer, SIGNAL(objectsSelected(std::list<GraphicObject*>&)),
         this, SLOT(updateSelectedProperties(std::list<GraphicObject*>&))));
      VERIFYNR(connect(mpAnnotationLayer, SIGNAL(currentTypeChanged(GraphicObjectType)),
         this, SLOT(setSelectionObject(GraphicObjectType))));
      VERIFYNR(connect(this, SIGNAL(graphicObjectTypeChanged(GraphicObjectType)),
         mpAnnotationLayer, SLOT(setCurrentGraphicObjectType(GraphicObjectType))));
      VERIFYNR(connect(mpAnnotationLayer, SIGNAL(snapToGridChanged(bool)),
         this, SLOT(setSnapToGrid(bool))));
      VERIFYNR(connect(this, SIGNAL(snapToGridChanged(bool)),
         mpAnnotationLayer, SLOT(setSnapToGrid(bool))));
      mpAnnotationLayer->attach(SIGNAL_NAME(Subject, Deleted), 
         Slot(this, &AnnotationToolBar::annotationLayerDeleted));

      // Determine if the current layer is one of the layers in a product view
      bool bProduct = false;
      bool bPlot = false;
      if (mpAnnotationLayer != NULL)
      {
         ProductView* pProductView = dynamic_cast<ProductView*>(mpAnnotationLayer->getView());
         if (pProductView != NULL)
         {
            bProduct = true;
         }
         if (dynamic_cast<PlotView*>(mpAnnotationLayer->getView()) != NULL)
         {
            bPlot = true;
         }
      }

      if (bProduct)
      {
         mpObject->setMode(GraphicObjectTypeGrid::PRODUCT_ANNOTATION);
      }
      else if (bPlot)
      {
         mpObject->setMode(GraphicObjectTypeGrid::PLOT_ANNOTATION);
      }
      else
      {
         mpObject->setMode(GraphicObjectTypeGrid::VIEW_ANNOTATION);
      }
   }

   updateSelectedProperties();
   enableSelectionTools();
   selectionObjectChanged();
   return true;
}

void AnnotationToolBar::annotationLayerDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   if (dynamic_cast<AnnotationLayerImp*>(&subject) == mpAnnotationLayer)
   {
      setAnnotationLayer(NULL);
      setEnabled(false);
   }
}

void AnnotationToolBar::setSelectionObject(GraphicObjectType eObject)
{
   if (eObject == MOVE_OBJECT)
   {
      mpMove->setChecked(true);
   }
   else if (eObject == ROTATE_OBJECT)
   {
      mpRotate->setChecked(true);
   }
   else
   {
      mpObject->setCurrentValue(eObject);
      mpObject->setChecked(true);
   }

   selectionObjectChanged();
}

GraphicObjectType AnnotationToolBar::getSelectionObject()
{
   GraphicObjectType eObject = MOVE_OBJECT;

   if (mpMove->isChecked() == true)
   {
      eObject = MOVE_OBJECT;
   }
   else if (mpRotate->isChecked() == true)
   {
      eObject = ROTATE_OBJECT;
   }
   else if (mpObject->isChecked() == true)
   {
      eObject = mpObject->getCurrentValue();
   }

   return eObject;
}

bool AnnotationToolBar::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pEvent->type() == QEvent::MouseButtonRelease)
      {
         if (pObject == mpLineWidthGrid)
         {
            if (mpLineWidthMenu != NULL)
            {
               mpLineWidthMenu->close();
            }

            QPixmap lineWidthPix = mpLineWidthGrid->getSelectedPixmap();
            setLineWidth(lineWidthPix);
         }
      }
   }

   return ToolBarAdapter::eventFilter(pObject, pEvent);
}

void AnnotationToolBar::initializeLineWidthGrid()
{
   if (mpAnnotationLayer != NULL)
   {
      QString identifier;

      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == false)
      {
         if (GraphicUtilities::getLineState(selectedObjects) == true)
         {
            double dWidth = GraphicUtilities::getLineWidth(selectedObjects);
            if (dWidth >= 1.0)
            {
               identifier = QString::number(static_cast<int>(dWidth));
            }
         }
      }

      mpLineWidthGrid->setSelectedPixmap(identifier);
   }
}

void AnnotationToolBar::setLineWidth(const QPixmap& lineWidthPix)
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The line width property cannot be set because "
            "there are no selected objects!");
         mpLineWidthGrid->setSelectedPixmap("NoLine");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Line Width");
         if (lineWidthPix.isNull() == false)
         {
            double lineWidth = lineWidthPix.height();
            GraphicUtilities::setLineWidth(selectedObjects, lineWidth);
            GraphicUtilities::setLineState(selectedObjects, true);
         }
         else
         {
            GraphicUtilities::setLineState(selectedObjects, false);
         }
      }
   }
}

void AnnotationToolBar::initializeLineColorMenu()
{
   if (mpAnnotationLayer != NULL)
   {
      QColor lineColor;

      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == false)
      {
         lineColor = COLORTYPE_TO_QCOLOR(GraphicUtilities::getLineColor(selectedObjects));
      }

      disconnect(mpLineColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setLineColor(const QColor&)));
      mpLineColorMenu->setSelectedColor(lineColor);
      VERIFYNR(connect(mpLineColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setLineColor(const QColor&))));
   }
}

void AnnotationToolBar::setLineColor(const QColor& lineColor)
{
   if (lineColor.isValid() == false)
   {
      return;
   }

   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The line color property cannot be set because "
            "there are no selected objects!");
         mpLineColorMenu->setSelectedColor(QColor());
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Line Color");
         GraphicUtilities::setLineColor(selectedObjects, QCOLOR_TO_COLORTYPE(lineColor));
      }
   }
}

void AnnotationToolBar::initializeFillColorMenu()
{
   if (mpAnnotationLayer != NULL)
   {
      QColor fillColor;

      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == false)
      {
         fillColor = COLORTYPE_TO_QCOLOR(GraphicUtilities::getFillColor(selectedObjects));
      }

      disconnect(mpFillColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setFillColor(const QColor&)));
      mpFillColorMenu->setSelectedColor(fillColor);
      VERIFYNR(connect(mpFillColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setFillColor(const QColor&))));
   }
}

void AnnotationToolBar::setFillColor(const QColor& fillColor)
{
   if (fillColor.isValid() == false)
   {
      return;
   }

   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The fill color property cannot be set because "
            "there are no selected objects!");
         mpFillColorMenu->setSelectedColor(QColor());
      }
      else
      {
         ColorType newColor(fillColor.red(), fillColor.green(), fillColor.blue());
         ColorType currentColor = GraphicUtilities::getFillColor(selectedObjects);

         if (newColor != currentColor)
         {
            UndoGroup group(mpAnnotationLayer->getView(), "Set Fill Color");
            GraphicUtilities::setFillColor(selectedObjects, newColor);
            GraphicUtilities::setFillState(selectedObjects, true);
         }
      }
   }
}

void AnnotationToolBar::setFillStateFalse()
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The fill property cannot be set because "
            "there are no selected objects!");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Fill State");
         GraphicUtilities::setFillState(selectedObjects, false);
      }
   }
}

void AnnotationToolBar::updateDefaultProperties()
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);

      for (list<GraphicObject*>::iterator iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
      {
         GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
         if (pObject != NULL)
         {
            if (pObject->hasProperty("Font") == true)
            {
               return;
            }
         }
      }
   }

   // Font
   mpFont_Combo->setCurrentIndex(-1);
   mpFont_Combo->setEnabled(false);

   // Font size
   mpSize_Combo->clearEditText();
   mpSize_Combo->setEnabled(false);

   // Bold, italics, underline
   mpBold->setChecked(false);
   mpBold->setEnabled(false);
   mpItalic->setChecked(false);
   mpItalic->setEnabled(false);
   mpUnderline->setChecked(false);
   mpUnderline->setEnabled(false);
}

void AnnotationToolBar::updateSelectedProperties()
{
   if (mpAnnotationLayer == NULL)
   {
      updateDefaultProperties();
      mpLayerLocked->setChecked(false);
   }
   else
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);

      updateSelectedProperties(selectedObjects);
      mpLayerLocked->setChecked(mpAnnotationLayer->getLayerLocked());
   }
}

void AnnotationToolBar::updateSelectedProperties(list<GraphicObject*>& selectedObjects)
{
   int iTextObjects = 0;

   // if there are no objects selected and we are in rotate mode, switch to select mode
   //
   //if (selectedObjects.size() == 0 && mpObjectGroup->checkedAction() == mpRotate)
   //{
   //   mpMove->setChecked(true);
   //}

   for (list<GraphicObject*>::iterator iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         if (pObject->hasProperty("Font") == true)
         {
            iTextObjects++;
         }
      }
   }

   if (iTextObjects == 0)
   {
      updateDefaultProperties();
   }
   else
   {
      string fontName = GraphicUtilities::getFontName(selectedObjects);
      int fontSize = GraphicUtilities::getFontSize(selectedObjects);
      bool bBold = GraphicUtilities::getFontBold(selectedObjects);
      bool bItalics = GraphicUtilities::getFontItalics(selectedObjects);
      bool bUnderline = GraphicUtilities::getFontUnderline(selectedObjects);

      mpFont_Combo->setEnabled(true);
      mpFont_Combo->setCurrentIndex(mpFont_Combo->findText(QString::fromStdString(fontName)));

      mpSize_Combo->setEnabled(true);
      mpSize_Combo->setCurrentValue(fontSize);

      mpBold->setEnabled(true);
      mpBold->setChecked(bBold);
      mpItalic->setEnabled(true);
      mpItalic->setChecked(bItalics);
      mpUnderline->setEnabled(true);
      mpUnderline->setChecked(bUnderline);
   }
}

void AnnotationToolBar::modifyLayerProperties()
{
   if (mpAnnotationLayer != NULL)
   {
      // Invoke the Properties dialog
      Service<DesktopServices> pDesktop;
      pDesktop->displayProperties(dynamic_cast<SessionItem*>(mpAnnotationLayer));
   }
}

void AnnotationToolBar::initializeTextColorMenu()
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == false)
      {
         ColorType textColor = GraphicUtilities::getTextColor(selectedObjects);
         if (textColor.isValid() == true)
         {
            disconnect(mpTextColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setTextColor(const QColor&)));

            QColor clrText(textColor.mRed, textColor.mGreen, textColor.mBlue);
            mpTextColorMenu->setSelectedColor(clrText);

            VERIFYNR(connect(mpTextColorMenu, SIGNAL(colorSelected(const QColor&)), this,
               SLOT(setTextColor(const QColor&))));
         }
      }
   }
}

void AnnotationToolBar::setTextColor(const QColor& textColor)
{
   if (textColor.isValid() == false)
   {
      return;
   }

   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The text color property cannot be set because "
            "there are no selected text objects!");
         mpTextColorMenu->setSelectedColor(QColor());
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Text Color");
         GraphicUtilities::setTextColor(selectedObjects, QCOLOR_TO_COLORTYPE(textColor));
      }
   }
}

void AnnotationToolBar::setFontFace(const QString& strFont)
{
   if (strFont.isEmpty() == true)
   {
      return;
   }

   if (mpAnnotationLayer != NULL)
   {
      string fontName = strFont.toStdString();

      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The font property cannot be set because "
            "there are no selected text objects!");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Text Font");
         GraphicUtilities::setFontName(selectedObjects, strFont.toStdString());
      }
   }
}

void AnnotationToolBar::setFontSize(int fontSize)
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The font property cannot be set because "
            "there are no selected text objects!");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Text Font Size");
         GraphicUtilities::setFontSize(selectedObjects, fontSize);
      }
   }
}

void AnnotationToolBar::setFontBold(bool bBold)
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The font property cannot be set because "
            "there are no selected text objects!");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Text Bold");
         GraphicUtilities::setFontBold(selectedObjects, bBold);
      }
   }
}

void AnnotationToolBar::setFontItalic(bool bItalics)
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The font property cannot be set because "
            "there are no selected text objects!");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Text Italics");
         GraphicUtilities::setFontItalics(selectedObjects, bItalics);
      }
   }
}

void AnnotationToolBar::setFontUnderline(bool bUnderline)
{
   if (mpAnnotationLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      mpAnnotationLayer->getSelectedObjectsImpl(selectedObjects);
      if (selectedObjects.empty() == true)
      {
         QMessageBox::critical(this, "Annotation", "The font property cannot be set because "
            "there are no selected text objects!");
      }
      else
      {
         UndoGroup group(mpAnnotationLayer->getView(), "Set Text Underline");
         GraphicUtilities::setFontUnderline(selectedObjects, bUnderline);
      }
   }
}

void AnnotationToolBar::group()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->groupSelection();
   }
}

void AnnotationToolBar::ungroup()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->ungroupSelection();
   }
}

void AnnotationToolBar::popFront()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->popSelectedObjectToFront();
   }
}

void AnnotationToolBar::pushBack()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->pushSelectedObjectToBack();
   }
}

void AnnotationToolBar::nudgeUp()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->nudgeSelectedObjects(0, 1);
   }
}

void AnnotationToolBar::nudgeDown()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->nudgeSelectedObjects(0, -1);
   }
}

void AnnotationToolBar::nudgeLeft()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->nudgeSelectedObjects(-1, 0);
   }
}

void AnnotationToolBar::nudgeRight()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->nudgeSelectedObjects(1, 0);
   }
}

void AnnotationToolBar::alignLeft()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->alignSelectedObjects(ALIGN_LEFT);
   }
}

void AnnotationToolBar::alignCenter()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->alignSelectedObjects(ALIGN_CENTER);
   }
}

void AnnotationToolBar::alignRight()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->alignSelectedObjects(ALIGN_RIGHT);
   }
}

void AnnotationToolBar::alignTop()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->alignSelectedObjects(ALIGN_TOP);
   }
}

void AnnotationToolBar::alignMiddle()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->alignSelectedObjects(ALIGN_MIDDLE);
   }
}

void AnnotationToolBar::alignBottom()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->alignSelectedObjects(ALIGN_BOTTOM);
   }
}

void AnnotationToolBar::distributeVertically()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->distributeSelectedObjects(DISTRIBUTE_VERTICALLY);
   }
}

void AnnotationToolBar::distributeHorizontally()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->distributeSelectedObjects(DISTRIBUTE_HORIZONTALLY);
   }
}

void AnnotationToolBar::selectAll()
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->selectAllObjects();
   }
}

void AnnotationToolBar::selectionObjectChanged()
{
   emit graphicObjectTypeChanged(getSelectionObject());
}

void AnnotationToolBar::setSnapToGrid(bool snap)
{
   if (mpSnapToGrid->isChecked() != snap)
   {
      mpSnapToGrid->setChecked(snap);
      emit snapToGridChanged(snap);
   }
}

void AnnotationToolBar::setLayerLocked(bool locked)
{
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->setLayerLocked(locked);
   }
}
