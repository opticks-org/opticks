/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

#include "AppVerify.h"
#include "Axis.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "CustomColorButton.h"
#include "DesktopAPITestGui.h"
#include "DesktopAPITestProperties.h"
#include "DesktopServices.h"
#include "Histogram.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"
#include "PlotSet.h"
#include "PlotView.h"
#include "PlotWindow.h"
#include "Signature.h"
#include "ToolBar.h"

using namespace std;

#define APP_DESKTOPAPITESTGUI_FIRST_TEST_ACTION "APP_DESKTOPAPITESTGUI_FIRST_TEST_ACTION"
#define APP_DESKTOPAPITESTGUI_SECOND_TEST_ACTION "APP_DESKTOPAPITESTGUI_SECOND_TEST_ACTION"
#define APP_DESKTOPAPITESTGUI_THIRD_TEST_ACTION "APP_DESKTOPAPITESTGUI_THIRD_TEST_ACTION"

namespace
{
const char* const OpenIcon[]={
   "16 16 5 1",
   "# c #000000",
   "c c #808000",
   ". c #c0c0c0",
   "b c #ffff00",
   "a c #ffffff",
   "................",
   "..........###...",
   ".........#...#.#",
   "..............##",
   "..###........###",
   ".#aba#######....",
   ".#babababab#....",
   ".#ababababa#....",
   ".#baba##########",
   ".#aba#ccccccccc#",
   ".#ba#ccccccccc#.",
   ".#a#ccccccccc#..",
   ".##ccccccccc#...",
   ".###########....",
   "................",
   "................"};

const char* const PrintIcon[]={
   "16 16 5 1",
   "# c #000000",
   "b c #808080",
   ". c #c0c0c0",
   "c c #ffff00",
   "a c #ffffff",
   "................",
   ".....#########..",
   "....#aaaaaaaa#..",
   "....#a#####a#...",
   "...#aaaaaaaa#...",
   "...#a#####a####.",
   "..#aaaaaaaa#.#.#",
   ".##########.#.##",
   "#..........#.#.#",
   "#############..#",
   "#......bbb..#.#.",
   "#......ccc..###.",
   "#############.#.",
   ".#.........#.#..",
   "..###########...",
   "................"};
};

DesktopAPITestGui::DesktopAPITestGui(QWidget* pParent) :
   QDialog(pParent),
   mpOpenAction(NULL),
   mpPrintAction(NULL),
   mpPlotWidget(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu), Slot(this, &DesktopAPITestGui::updateContextMenu)),
   mpDockWindow(NULL),
   mpDockPlotWidget(NULL)
{
   Service<DesktopServices> pDesktop;

   // Toolbar buttons
   QWidget* pToolBarWidget = new QWidget(this);
   QCheckBox* pBrowseCheck = new QCheckBox("Add a browse button to the Demo toolbar", pToolBarWidget);
   QCheckBox* pPrintCheck = new QCheckBox("Add a print button to the Tests toolbar", pToolBarWidget);

   LabeledSection* pToolBarSection = new LabeledSection(pToolBarWidget, "Toolbar Buttons", this);

   // Plot widget
   QWidget* pPlotWidget = new QWidget(this);

   mpPlotWidget.reset(pDesktop->createPlotWidget("Test Plot Widget", HISTOGRAM_PLOT, pPlotWidget));

   QLabel* pClassificationLabel = new QLabel("Set Classification:", pPlotWidget);
   mpClassificationEdit = new QLineEdit(pPlotWidget);
   QPushButton* pClassificationApplyButton = new QPushButton("Apply", pPlotWidget);

   QLabel* pMouseModeLabel = new QLabel("Mouse Modes:", pPlotWidget);
   mpMouseModeCombo = new QComboBox(pPlotWidget);
   mpMouseModeCombo->setEditable(false);
   mpMouseModeCombo->addItem("Enable All");
   mpMouseModeCombo->addItem("Disable All");
   mpMouseModeCombo->addItem("Disable Pan");
   mpMouseModeCombo->addItem("Disable Zoom");
   mpMouseModeCombo->addItem("Activate Annotation");
   QPushButton* pMouseModeApplyButton = new QPushButton("Apply", pPlotWidget);

   QLabel* pMarginLabel = new QLabel("Margin Factor:", pPlotWidget);
   mpMarginEdit = new QLineEdit(pPlotWidget);
   mpMarginEdit->setFixedWidth(75);

   QLabel* pTextColorLabel = new QLabel("Classification Text Color:", pPlotWidget);
   CustomColorButton* pTextColorButton = new CustomColorButton(pPlotWidget);
   pTextColorButton->usePopupGrid(true);

   QLabel* pTitleColorLabel = new QLabel("Axis Title Color:", pPlotWidget);
   CustomColorButton* pTitleColorButton = new CustomColorButton(pPlotWidget);
   pTitleColorButton->usePopupGrid(true);

   mpContextMenuCheck = new QCheckBox("Add commands to the context menu", pPlotWidget);

   LabeledSection* pPlotWidgetSection = new LabeledSection(pPlotWidget, "Plot Widget", this);

   // Dock window
   mpDockWindow = static_cast<PlotWindow*>(pDesktop->createWindow("Test Plot Window", PLOT_WINDOW));
   mpDockWindow->enableSessionItemDrops(this);

   PlotSet* pPlotSet = mpDockWindow->createPlotSet("Test Plot Set");
   if (pPlotSet != NULL)
   {
      mpDockPlotWidget = pPlotSet->createPlot("Test Plot", SIGNATURE_PLOT);
      if (mpDockPlotWidget != NULL)
      {
         mpDockPlotWidget->getAxis(AXIS_LEFT)->setTitle("Reflectance");
         mpDockPlotWidget->getAxis(AXIS_BOTTOM)->setTitle("Wavelength");
      }
   }

   QWidget* pDockWindowWidget = new QWidget(this);
   mpDockedCheck = new QCheckBox("Docked", pDockWindowWidget);
   mpDockedCheck->setChecked(true);
   mpDragDropCheck = new QCheckBox("Enable drag-and-drop to add signatures", pDockWindowWidget);
   mpPropertiesCheck = new QCheckBox("Enable custom properties", pDockWindowWidget);
   QPushButton* pPropertiesButton = new QPushButton("Properties", pDockWindowWidget);

   LabeledSection* pDockWindowSection = new LabeledSection(pDockWindowWidget, "Dock Window", this);

   // Labeled section group
   LabeledSectionGroup* pSectionGroup = new LabeledSectionGroup(this);
   pSectionGroup->addSection(pToolBarSection);
   pSectionGroup->addSection(pPlotWidgetSection, 1000);
   pSectionGroup->addSection(pDockWindowSection);
   pSectionGroup->addStretch(1);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(this);
   pButtonBox->setStandardButtons(QDialogButtonBox::Close);
   pButtonBox->setOrientation(Qt::Horizontal);

   // Layout
   QVBoxLayout* pToolBarLayout = new QVBoxLayout(pToolBarWidget);
   pToolBarLayout->setMargin(0);
   pToolBarLayout->setSpacing(5);
   pToolBarLayout->addWidget(pBrowseCheck);
   pToolBarLayout->addWidget(pPrintCheck);

   QGridLayout* pPlotWidgetGrid = new QGridLayout(pPlotWidget);
   pPlotWidgetGrid->setMargin(0);
   pPlotWidgetGrid->setSpacing(5);
   pPlotWidgetGrid->addWidget(mpPlotWidget->getWidget(), 0, 0, 7, 1);
   pPlotWidgetGrid->addWidget(pClassificationLabel, 0, 2);
   pPlotWidgetGrid->addWidget(mpClassificationEdit, 0, 3);
   pPlotWidgetGrid->addWidget(pClassificationApplyButton, 0, 4);
   pPlotWidgetGrid->addWidget(pMouseModeLabel, 1, 2);
   pPlotWidgetGrid->addWidget(mpMouseModeCombo, 1, 3);
   pPlotWidgetGrid->addWidget(pMouseModeApplyButton, 1, 4);
   pPlotWidgetGrid->addWidget(pMarginLabel, 2, 2);
   pPlotWidgetGrid->addWidget(mpMarginEdit, 2, 3, 1, 2, Qt::AlignLeft);
   pPlotWidgetGrid->addWidget(pTextColorLabel, 3, 2);
   pPlotWidgetGrid->addWidget(pTextColorButton, 3, 3, 1, 2, Qt::AlignLeft);
   pPlotWidgetGrid->addWidget(pTitleColorLabel, 4, 2);
   pPlotWidgetGrid->addWidget(pTitleColorButton, 4, 3, 1, 2, Qt::AlignLeft);
   pPlotWidgetGrid->addWidget(mpContextMenuCheck, 5, 2, 1, 3);
   pPlotWidgetGrid->setRowStretch(6, 10);
   pPlotWidgetGrid->setColumnStretch(0, 10);
   pPlotWidgetGrid->setColumnStretch(3, 5);
   pPlotWidgetGrid->setColumnMinimumWidth(1, 15);

   QVBoxLayout* pDockWindowLayout = new QVBoxLayout(pDockWindowWidget);
   pDockWindowLayout->setMargin(0);
   pDockWindowLayout->setSpacing(5);
   pDockWindowLayout->addWidget(mpDockedCheck);
   pDockWindowLayout->addWidget(mpDragDropCheck);
   pDockWindowLayout->addWidget(mpPropertiesCheck);
   pDockWindowLayout->addWidget(pPropertiesButton, 0, Qt::AlignLeft);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pSectionGroup, 10);
   pLayout->addWidget(pHLine);
   pLayout->addWidget(pButtonBox);

   // Initialization
   setWindowTitle("Desktop API Test");
   setModal(false);
   resize(700, 550);

   PlotView* pView = mpPlotWidget->getPlot();
   if (pView != NULL)
   {
      double binCenters[256];
      double binValues[256];
      for (int count = 0; count < 256; ++count)
      {
         binCenters[count] = count;
         binValues[count] = count;
      }

      Histogram* pHistogram = static_cast<Histogram*>(pView->addObject(HISTOGRAM, true));
      if (pHistogram != NULL)
      {
         pHistogram->setHistogramData(256, binCenters, binValues);
         pView->refresh();
      }

      PlotViewExt1* pViewExt = dynamic_cast<PlotViewExt1*>(pView);
      if (pViewExt != NULL)
      {
         double marginFactor = pViewExt->getExtentsMargin();
         mpMarginEdit->setText(QString::number(marginFactor));
      }
   }

   Axis* pLeftAxis = mpPlotWidget->getAxis(AXIS_LEFT);
   if (pLeftAxis != NULL)
   {
      pLeftAxis->setTitle("Y-Axis");
      pTitleColorButton->setColor(COLORTYPE_TO_QCOLOR(pLeftAxis->getTitleColor()));
   }

   Axis* pBottomAxis = mpPlotWidget->getAxis(AXIS_BOTTOM);
   if (pBottomAxis != NULL)
   {
      pBottomAxis->setTitle("X-Axis");
   }

   mpPlotWidget->showLegend(false);
   mpClassificationEdit->setText(QString::fromStdString(mpPlotWidget->getClassificationText()));
   pTextColorButton->setColor(mpPlotWidget->getClassificationColor());

   // Connections
   VERIFYNR(connect(pBrowseCheck, SIGNAL(toggled(bool)), this, SLOT(addBrowseButton(bool))));
   VERIFYNR(connect(pPrintCheck, SIGNAL(toggled(bool)), this, SLOT(addPrintButton(bool))));
   VERIFYNR(connect(mpClassificationEdit, SIGNAL(returnPressed()), this, SLOT(setClassificationText())));
   VERIFYNR(connect(pClassificationApplyButton, SIGNAL(clicked()), this, SLOT(setClassificationText())));
   VERIFYNR(connect(pMouseModeApplyButton, SIGNAL(clicked()), this, SLOT(enableMouseMode())));
   VERIFYNR(connect(mpMarginEdit, SIGNAL(editingFinished()), this, SLOT(setPlotMargin())));
   VERIFYNR(connect(pTextColorButton, SIGNAL(colorChanged(const QColor&)), this, SLOT(setTextColor(const QColor&))));
   VERIFYNR(connect(pTitleColorButton, SIGNAL(colorChanged(const QColor&)), this, SLOT(setTitleColor(const QColor&))));
   VERIFYNR(connect(pPropertiesButton, SIGNAL(clicked()), this, SLOT(displayProperties())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
   VERIFYNR(connect(mpDockedCheck, SIGNAL(toggled(bool)), this, SLOT(setDocked(bool))));
   VERIFYNR(mpDockWindow->attach(SIGNAL_NAME(Window, SessionItemDropped),
      Slot(this, &DesktopAPITestGui::dropSessionItem)));
   VERIFYNR(pDesktop->attach(SIGNAL_NAME(DesktopServices, AboutToShowPropertiesDialog),
      Slot(this, &DesktopAPITestGui::updatePropertiesDialog)));
   VERIFYNR(mpDockWindow->attach(SIGNAL_NAME(DockWindow, Docked),
      Slot(this, &DesktopAPITestGui::docked)));
   VERIFYNR(mpDockWindow->attach(SIGNAL_NAME(DockWindow, Undocked),
      Slot(this, &DesktopAPITestGui::undocked)));
}

DesktopAPITestGui::~DesktopAPITestGui()
{
   Service<DesktopServices> pDesktop;
   pDesktop->detach(SIGNAL_NAME(DesktopServices, AboutToShowPropertiesDialog),
      Slot(this, &DesktopAPITestGui::updatePropertiesDialog));
   pDesktop->deleteWindow(mpDockWindow);
}

bool DesktopAPITestGui::accept(SessionItem* pItem) const
{
   bool bAccept = false;
   if (mpDragDropCheck->isChecked() == true)
   {
      bAccept = dynamic_cast<Signature*>(pItem) != NULL;
   }

   return bAccept;
}

void DesktopAPITestGui::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   // Only add the actions to the plot widget in the dialog
   if (dynamic_cast<PlotWidget*>(&subject) != mpPlotWidget.get())
   {
      return;
   }

   // Only add custom context menu actions if the box is checked
   if (mpContextMenuCheck->isChecked() == false)
   {
      return;
   }

   // Get the context menu
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   // When the context menu is invoked for the plot widget, the session items for the menu should include one
   // plot widget and one plot view, so make sure that the plot widget is the plot widget in the dialog
   vector<PlotWidget*> plotWidgets = pMenu->getSessionItems<PlotWidget>();
   if ((plotWidgets.size() != 1) || (plotWidgets.front() != mpPlotWidget.get()))
   {
      return;
   }

   QObject* pParent = pMenu->getActionParent();

   QAction* pAction1 = new QAction("Custom Plug-In Item 1", pParent);
   pAction1->setAutoRepeat(false);
   pMenu->addActionBefore(pAction1, APP_DESKTOPAPITESTGUI_FIRST_TEST_ACTION, APP_PLOTWIDGET_PRINT_SEPARATOR_ACTION);

   QAction* pAction2 = new QAction("Custom Plug-In Item 2", pParent);
   pAction2->setAutoRepeat(false);
   pMenu->addActionBefore(pAction2, APP_DESKTOPAPITESTGUI_SECOND_TEST_ACTION,
      APP_PLOTVIEW_RESCALE_AXES_SEPARATOR_ACTION);

   QAction* pAction3 = new QAction("Custom Plug-In Item 3", pParent);
   pAction3->setAutoRepeat(false);
   pMenu->addActionBefore(pAction3, APP_DESKTOPAPITESTGUI_THIRD_TEST_ACTION, APP_PLOTVIEW_SECURITY_MARKINGS_ACTION);
}

void DesktopAPITestGui::updatePropertiesDialog(Subject& subject, const string& signal, const boost::any& value)
{
   if (mpDockPlotWidget == NULL)
   {
      return;
   }

   // Only add the custom properties page if the box is checked
   if (mpPropertiesCheck->isChecked() == false)
   {
      return;
   }

   pair<SessionItem*, vector<string>*> properties = boost::any_cast<pair<SessionItem*, vector<string>*> >(value);

   SessionItem* pItem = properties.first;
   if ((dynamic_cast<PlotWidget*>(pItem) == mpDockPlotWidget) ||
      (dynamic_cast<PlotView*>(pItem) == mpDockPlotWidget->getPlot()))
   {
      vector<string>* pPlugInNames = properties.second;
      if (pPlugInNames != NULL)
      {
         pPlugInNames->push_back(DesktopAPITestProperties::getName());
      }
   }
}

void DesktopAPITestGui::dropSessionItem(Subject& subject, const string& signal, const boost::any& value)
{
   Signature* pSignature = dynamic_cast<Signature*>(boost::any_cast<SessionItem*>(value));
   if (pSignature != NULL)
   {
      if (mpDockWindow != NULL)
      {
         // Add the signature to the plot
         mpDockWindow->plotData(*pSignature, "Wavelength", "Reflectance", "Test Plot");

         // Refresh the plot
         if (mpDockPlotWidget != NULL)
         {
            PlotView* pPlotView = mpDockPlotWidget->getPlot();
            if (pPlotView != NULL)
            {
               pPlotView->zoomExtents();
               pPlotView->refresh();
            }
         }
      }
   }
}

void DesktopAPITestGui::docked(Subject& subject, const string& signal, const boost::any& value)
{
   mpDockedCheck->setChecked(true);
}

void DesktopAPITestGui::undocked(Subject& subject, const string& signal, const boost::any& value)
{
   mpDockedCheck->setChecked(false);
}

void DesktopAPITestGui::addBrowseButton(bool bAdd)
{
   Service<DesktopServices> pDesktop;

   ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Demo", TOOLBAR));
   if (pToolBar == NULL)
   {
      return;
   }

   if (bAdd == true)
   {
      if (mpOpenAction == NULL)
      {
         QPixmap browsePixmap(OpenIcon);
         browsePixmap.setMask(browsePixmap.createHeuristicMask());
         QIcon browseIcon(browsePixmap);

         mpOpenAction = new QAction(this);
         mpOpenAction->setIcon(browseIcon);
      }

      pToolBar->addButton(mpOpenAction);
   }
   else
   {
      pToolBar->removeItem(mpOpenAction);
   }
}

void DesktopAPITestGui::addPrintButton(bool bAdd)
{
   Service<DesktopServices> pDesktop;

   ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Tests", TOOLBAR));
   if (pToolBar == NULL)
   {
      return;
   }

   if (bAdd == true)
   {
      if (mpPrintAction == NULL)
      {
         QPixmap printPixmap(PrintIcon);
         printPixmap.setMask(printPixmap.createHeuristicMask());
         QIcon printIcon(printPixmap);

         mpPrintAction = new QAction(this);
         mpPrintAction->setIcon(printIcon);
      }

      pToolBar->addButton(mpPrintAction);
   }
   else
   {
      pToolBar->removeItem(mpPrintAction);
   }
}

void DesktopAPITestGui::enableMouseMode()
{
   if (mpPlotWidget.get() == NULL)
   {
      return;
   }

   QString text = mpMouseModeCombo->currentText();
   if (text.isEmpty() == true)
   {
      return;
   }

   PlotView* pPlotView = mpPlotWidget->getPlot();
   VERIFYNRV(pPlotView != NULL);

   MouseMode* pObjectSelectMode = pPlotView->getMouseMode("SelectionMode");
   VERIFYNRV(pObjectSelectMode != NULL);

   MouseMode* pPanMode = pPlotView->getMouseMode("PanMode");
   VERIFYNRV(pPanMode != NULL);

   MouseMode* pZoomMode = pPlotView->getMouseMode("ZoomBoxMode");
   VERIFYNRV(pZoomMode != NULL);

   MouseMode* pLocatorMode = pPlotView->getMouseMode("LocatorMode");
   VERIFYNRV(pLocatorMode != NULL);

   MouseMode* pAnnotationMode = pPlotView->getMouseMode("AnnotationMode");
   VERIFYNRV(pAnnotationMode != NULL);

   if (text == "Enable All")
   {
      pPlotView->enableMouseMode(pObjectSelectMode, true);
      pPlotView->enableMouseMode(pPanMode, true);
      pPlotView->enableMouseMode(pZoomMode, true);
      pPlotView->enableMouseMode(pLocatorMode, true);
      pPlotView->enableMouseMode(pAnnotationMode, true);
   }
   else if (text == "Disable All")
   {
      pPlotView->enableMouseMode(pObjectSelectMode, false);
      pPlotView->enableMouseMode(pPanMode, false);
      pPlotView->enableMouseMode(pZoomMode, false);
      pPlotView->enableMouseMode(pLocatorMode, false);
      pPlotView->enableMouseMode(pAnnotationMode, false);
   }
   else if (text == "Disable Pan")
   {
      pPlotView->enableMouseMode(pPanMode, false);
   }
   else if (text == "Disable Zoom")
   {
      pPlotView->enableMouseMode(pZoomMode, false);
   }
   else if (text == "Activate Annotation")
   {
      pPlotView->enableMouseMode(pAnnotationMode, true);
      pPlotView->setMouseMode("AnnotationMode");
   }
}

void DesktopAPITestGui::setClassificationText()
{
   if (mpPlotWidget.get() != NULL)
   {
      QString classificationText = mpClassificationEdit->text();
      mpPlotWidget->setClassificationText(classificationText.toStdString());
   }
}

void DesktopAPITestGui::setPlotMargin()
{
   if (mpPlotWidget.get() != NULL)
   {
      PlotViewExt1* pPlotView = dynamic_cast<PlotViewExt1*>(mpPlotWidget->getPlot());
      if (pPlotView != NULL)
      {
         double marginFactor = 0.0;

         QString marginText = mpMarginEdit->text();
         if (marginText.isEmpty() == false)
         {
            marginFactor = marginText.toDouble();
         }

         pPlotView->setExtentsMargin(marginFactor);
      }
   }
}

void DesktopAPITestGui::setTextColor(const QColor& textColor)
{
   if ((textColor.isValid() == true) && (mpPlotWidget.get() != NULL))
   {
      mpPlotWidget->setClassificationColor(QCOLOR_TO_COLORTYPE(textColor));
   }
}

void DesktopAPITestGui::setTitleColor(const QColor& titleColor)
{
   if ((titleColor.isValid() == true) && (mpPlotWidget.get() != NULL))
   {
      Axis* pLeftAxis = mpPlotWidget->getAxis(AXIS_LEFT);
      if (pLeftAxis != NULL)
      {
         pLeftAxis->setTitleColor(QCOLOR_TO_COLORTYPE(titleColor));
      }

      Axis* pBottomAxis = mpPlotWidget->getAxis(AXIS_BOTTOM);
      if (pBottomAxis != NULL)
      {
         pBottomAxis->setTitleColor(QCOLOR_TO_COLORTYPE(titleColor));
      }
   }
}

void DesktopAPITestGui::displayProperties()
{
   if (mpDockPlotWidget != NULL)
   {
      Service<DesktopServices> pDesktop;
      pDesktop->displayProperties(mpDockPlotWidget);
   }
}

void DesktopAPITestGui::setDocked(bool docked)
{
   if (docked == true)
   {
      mpDockWindow->dock();
   }
   else
   {
      mpDockWindow->undock();
   }
}
