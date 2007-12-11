/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QImage>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QSplitter>

#include "ApplicationServices.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "PlotWidgetImp.h"
#include "AnnotationLayer.h"
#include "AnnotationToolBar.h"
#include "AxisAdapter.h"
#include "CartesianGridlines.h"
#include "CartesianPlot.h"
#include "Classification.h"
#include "ConfigurationSettings.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "ContextMenuImp.h"
#include "AppAssert.h"
#include "DesktopServices.h"
#include "FloatingLabel.h"
#include "GridlinesImp.h"
#include "Icons.h"
#include "Legend.h"
#include "Locator.h"
#include "LocatorImp.h"
#include "MouseMode.h"
#include "PlotSet.h"
#include "PlotViewImp.h"
#include "PlotWidget.h"
#include "PrintPixmap.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManagerImp.h"
#include "StringUtilities.h"
#include "SystemServicesImp.h"
#include "xmlreader.h"

#include <string>

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
using XERCES_CPP_NAMESPACE_QUALIFIER XMLString;

PlotWidgetImp::PlotWidgetImp(const string& id, const string& plotName, PlotType plotType, PlotSet* pPlotSet,
   QWidget* parent) :
   QWidget(parent),
   SessionItemImp(id, plotName),
   mpPrintAction(NULL),
   mpSeparatorAction(NULL),
   mpLegendAction(NULL),
   mpSeparator2Action(NULL),
   mpAnnotationToolBar(NULL),
   mpPlotSet(pPlotSet),
   mpPlot(NULL),
   mpPlotWidget(NULL),
   mpXMouseLabel(NULL),
   mpYMouseLabel(NULL),
   mpXAxis(NULL),
   mpYAxis(NULL),
   mpLegend(NULL),
   mClassificationPosition(CENTER),
   mOrganizationPosition(TOP_RIGHT_BOTTOM_LEFT)
{
   // Context menu actions
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   Service<DesktopServices> pDesktop;
   string shortcutContext = "Plot";

   mpPrintAction = new QAction(pIcons->mPrint, "&Print...", this);
   mpPrintAction->setAutoRepeat(false);
   mpPrintAction->setShortcut(QKeySequence("Ctrl+P"));
   mpPrintAction->setStatusTip("Sends the displayed area of the current plot to the printer");
   connect(mpPrintAction, SIGNAL(triggered()), this, SLOT(print()));
   pDesktop->initializeAction(mpPrintAction, shortcutContext);

   mpSeparatorAction = new QAction(this);
   mpSeparatorAction->setSeparator(true);

   mpLegendAction = new QAction("&Legend", this);
   mpLegendAction->setAutoRepeat(false);
   mpLegendAction->setCheckable(true);
   mpLegendAction->setShortcut(QKeySequence("Ctrl+L"));
   mpLegendAction->setStatusTip("Toggles the display of the legend");
   connect(mpLegendAction, SIGNAL(toggled(bool)), this, SLOT(showLegend(bool)));
   pDesktop->initializeAction(mpLegendAction, shortcutContext);

   mpSeparator2Action = new QAction(this);
   mpSeparator2Action->setSeparator(true);

   // Annotation toolbar
   mpAnnotationToolBar = new AnnotationToolBar(SessionItemImp::generateUniqueId(), this);
   mpAnnotationToolBar->hide();

   initialize(NULL, plotName, plotType);
}

PlotWidgetImp::~PlotWidgetImp()
{
   Service<DesktopServices> pDesktop;
   detach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowContextMenu)));
}

void PlotWidgetImp::initialize(PlotViewImp *pPlotView, const string& plotName, PlotType plotType)
{
   // cleanup a previous initialization
   delete mpPlotWidget;

   // Parent widget for the plot widgets
   mpPlotWidget = new QWidget(this);

   // Top labels
   mpTopLeftLabel = new QLabel(mpPlotWidget);
   mpTopCenterLabel = new QLabel(mpPlotWidget);
   mpTopRightLabel = new QLabel(mpPlotWidget);

   mpTopCenterLabel->setAlignment(Qt::AlignHCenter);

   // Bottom labels
   mpBottomLeftLabel = new QLabel(mpPlotWidget);
   mpBottomCenterLabel = new QLabel(mpPlotWidget);
   mpBottomRightLabel = new QLabel(mpPlotWidget);

   mpBottomCenterLabel->setAlignment(Qt::AlignHCenter);

   // Title
   mpTitleLabel = new QLabel(mpPlotWidget);
   mpTitleLabel->setAlignment(Qt::AlignHCenter);
   mpTitleLabel->hide();

   // Plot widget
   QWidget* pPlotWidget = new QWidget();

   // Plot
   if(pPlotView == NULL)
   {
      mpPlot = dynamic_cast<PlotViewImp*>(Service<DesktopServices>()->createPlot(plotName, plotType, pPlotWidget));
   }
   else
   {
      mpPlot = pPlotView;
   }
   mpPlot->installEventFilter(this);

   CartesianPlot* pCartesianPlot = dynamic_cast<CartesianPlot*>(mpPlot);
   if (pCartesianPlot != NULL)
   {
      // Mouse locator labels
      mpXMouseLabel = new FloatingLabel(Qt::Horizontal, pPlotWidget);
      mpYMouseLabel = new FloatingLabel(Qt::Vertical, pPlotWidget);

      // Axes
      mpXAxis = new AxisAdapter(AXIS_BOTTOM, pPlotWidget);
      mpYAxis = new AxisAdapter(AXIS_LEFT, pPlotWidget);

      QFont ftAxis;
      ftAxis.setBold(true);
      ftAxis.setPointSize(10);

      mpXAxis->setFont(ftAxis);
      mpYAxis->setFont(ftAxis);

      QString strXAxisText = QString::fromStdString(pCartesianPlot->getXDataType());
      QString strYAxisText = QString::fromStdString(pCartesianPlot->getYDataType());
      mpXAxis->setTitle(strXAxisText);
      mpYAxis->setTitle(strYAxisText);
   }

   // Legend
   mpLegend = new Legend();
   mpLegend->setFrameStyle(QFrame::NoFrame);
   mpLegend->setMinimumWidth(120);

   // Splitter widget
   QSplitter* pSplitter = new QSplitter(Qt::Horizontal, mpPlotWidget);
   pSplitter->setOpaqueResize(true);
   pSplitter->installEventFilter(this);
   pSplitter->insertWidget(0, pPlotWidget);
   pSplitter->insertWidget(1, mpLegend);
   pSplitter->setStretchFactor(0, 10);

   // Layout
   QHBoxLayout* pTopLabelLayout = new QHBoxLayout();
   pTopLabelLayout->setMargin(0);
   pTopLabelLayout->setSpacing(5);
   pTopLabelLayout->addWidget(mpTopLeftLabel, 0, Qt::AlignLeft);
   pTopLabelLayout->addWidget(mpTopCenterLabel, 0, Qt::AlignHCenter);
   pTopLabelLayout->addWidget(mpTopRightLabel, 0, Qt::AlignRight);

   QHBoxLayout* pBottomLabelLayout = new QHBoxLayout();
   pBottomLabelLayout->setMargin(0);
   pBottomLabelLayout->setSpacing(5);
   pBottomLabelLayout->addWidget(mpBottomLeftLabel, 0, Qt::AlignLeft);
   pBottomLabelLayout->addWidget(mpBottomCenterLabel, 0, Qt::AlignHCenter);
   pBottomLabelLayout->addWidget(mpBottomRightLabel, 0, Qt::AlignRight);

   if (pCartesianPlot == NULL)
   {
      QVBoxLayout* pPlotLayout = new QVBoxLayout(pPlotWidget);
      pPlotLayout->setMargin(0);
      pPlotLayout->setSpacing(5);
      pPlotLayout->addWidget(mpPlot);
   }
   else
   {
      QGridLayout* pPlotGrid = new QGridLayout(pPlotWidget);
      pPlotGrid->setMargin(0);
      pPlotGrid->setSpacing(0);
      pPlotGrid->addWidget(mpXMouseLabel, 0, 1);
      pPlotGrid->setRowMinimumHeight(1, 5);
      pPlotGrid->addWidget(mpYAxis, 2, 0);
      pPlotGrid->addWidget(mpPlot, 2, 1);
      pPlotGrid->setColumnMinimumWidth(2, 5);
      pPlotGrid->addWidget(mpYMouseLabel, 2, 3);
      pPlotGrid->setColumnMinimumWidth(3, mpYMouseLabel->sizeHint().width());
      pPlotGrid->addWidget(mpXAxis, 3, 1);
      pPlotGrid->setRowStretch(2, 10);
      pPlotGrid->setColumnStretch(1, 10);
   }

   QVBoxLayout* pPlotLayout = new QVBoxLayout(mpPlotWidget);
   pPlotLayout->setMargin(10);
   pPlotLayout->setSpacing(5);
   pPlotLayout->addLayout(pTopLabelLayout);
   pPlotLayout->addWidget(mpTitleLabel);
   pPlotLayout->addWidget(pSplitter, 10);
   pPlotLayout->addLayout(pBottomLabelLayout);

   QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(layout());
   if(pLayout == NULL)
   {
      pLayout = new QVBoxLayout(this);
   }
   pLayout->setMargin(0);
   pLayout->setSpacing(0);
   pLayout->addWidget(mpAnnotationToolBar);
   pLayout->addWidget(mpPlotWidget, 10);

   // Initialization
   QPalette plotPalette = mpPlotWidget->palette();
   plotPalette.setColor(QPalette::Window, Qt::white);
   mpPlotWidget->setPalette(plotPalette);

   mpPlotWidget->setAutoFillBackground(true);
   setContextMenuPolicy(Qt::DefaultContextMenu);

   AnnotationLayer* pAnnotationLayer = mpPlot->getAnnotationLayer();
   if (pAnnotationLayer != NULL)
   {
      mpAnnotationToolBar->setAnnotationLayer(pAnnotationLayer);
   }

   QString strClassificationText = mpPlot->getClassificationText();
   if (strClassificationText.isEmpty() == false)
   {
      setClassificationText(strClassificationText);
   }

   list<PlotObject*> plotObjects = mpPlot->getObjects();
   for (list<PlotObject*>::iterator iter = plotObjects.begin(); iter != plotObjects.end(); ++iter)
   {
      PlotObject* pObject = *iter;
      if (pObject != NULL)
      {
         mpLegend->insertItem(pObject);
      }
   }

   showLegend(false);

   // Connections
   VERIFYNR(connect(mpPlot, SIGNAL(renamed(const QString&)), this, SLOT(updateName(const QString&))));
   VERIFYNR(connect(mpPlot, SIGNAL(mouseModeChanged(const MouseMode*)), this,
      SLOT(enableAnnotationToolBar(const MouseMode*))));
   VERIFYNR(connect(mpPlot, SIGNAL(classificationChanged(const QString&)), this,
      SLOT(setClassificationText(const QString&))));
   VERIFYNR(connect(mpPlot, SIGNAL(classificationFontChanged(const QFont&)), this,
      SLOT(setClassificationFont(const QFont&))));
   VERIFYNR(connect(mpPlot, SIGNAL(classificationColorChanged(const QColor&)), this,
      SLOT(setClassificationColor(const QColor&))));
   VERIFYNR(connect(mpPlot, SIGNAL(objectAdded(PlotObject*)), mpLegend, SLOT(insertItem(PlotObject*))));
   VERIFYNR(connect(mpPlot, SIGNAL(objectDeleted(PlotObject*)), mpLegend, SLOT(removeItem(PlotObject*))));
   VERIFYNR(connect(mpPlot, SIGNAL(objectSelected(PlotObject*, bool)), mpLegend,
      SLOT(setItemSelected(PlotObject*, bool))));
   VERIFYNR(connect(mpLegend, SIGNAL(itemSelected(PlotObject*, bool)), this,
      SLOT(selectPlotObject(PlotObject*, bool))));

   if (pCartesianPlot != NULL)
   {
      VERIFYNR(connect(mpPlot, SIGNAL(displayAreaChanged()), this, SLOT(updateScaleRange())));
      VERIFYNR(connect(mpPlot, SIGNAL(xScaleTypeChanged(ScaleType)), mpXAxis, SLOT(setScaleType(ScaleType))));
      VERIFYNR(connect(mpPlot, SIGNAL(yScaleTypeChanged(ScaleType)), mpYAxis, SLOT(setScaleType(ScaleType))));
      VERIFYNR(connect(mpPlot, SIGNAL(xDataTypeChanged(const QString&)), mpXAxis, SLOT(setTitle(const QString&))));
      VERIFYNR(connect(mpPlot, SIGNAL(yDataTypeChanged(const QString&)), mpYAxis, SLOT(setTitle(const QString&))));

      GridlinesImp* pGridlines = dynamic_cast<GridlinesImp*>(pCartesianPlot->getGridlines(HORIZONTAL));
      if (pGridlines != NULL)
      {
         VERIFYNR(connect(pGridlines, SIGNAL(maxNumMajorLinesChanged(int)), mpYAxis, SLOT(setMaxNumMajorTicks(int))));
         VERIFYNR(connect(pGridlines, SIGNAL(maxNumMinorLinesChanged(int)), mpYAxis, SLOT(setMaxNumMinorTicks(int))));
         VERIFYNR(connect(mpYAxis, SIGNAL(maxNumMajorTicksChanged(int)), pGridlines, SLOT(setMaxNumMajorLines(int))));
         VERIFYNR(connect(mpYAxis, SIGNAL(maxNumMinorTicksChanged(int)), pGridlines, SLOT(setMaxNumMinorLines(int))));
      }

      pGridlines = dynamic_cast<GridlinesImp*>(pCartesianPlot->getGridlines(VERTICAL));
      if (pGridlines != NULL)
      {
         VERIFYNR(connect(pGridlines, SIGNAL(maxNumMajorLinesChanged(int)), mpXAxis, SLOT(setMaxNumMajorTicks(int))));
         VERIFYNR(connect(pGridlines, SIGNAL(maxNumMinorLinesChanged(int)), mpXAxis, SLOT(setMaxNumMinorTicks(int))));
         VERIFYNR(connect(mpXAxis, SIGNAL(maxNumMajorTicksChanged(int)), pGridlines, SLOT(setMaxNumMajorLines(int))));
         VERIFYNR(connect(mpXAxis, SIGNAL(maxNumMinorTicksChanged(int)), pGridlines, SLOT(setMaxNumMinorLines(int))));
      }
   }

   LocatorImp* pLocator = dynamic_cast<LocatorImp*>(mpPlot->getMouseLocator());
   if (pLocator != NULL)
   {
      VERIFYNR(connect(pLocator, SIGNAL(textChanged(const QString&, const QString&)), this,
         SLOT(updateMouseLabel(const QString&, const QString&))));
   }
}

const string& PlotWidgetImp::getObjectType() const
{
   static string type("PlotWidgetImp");
   return type;
}

bool PlotWidgetImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotWidget"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

PlotSet* PlotWidgetImp::getPlotSet() const
{
   return mpPlotSet;
}

PlotView* PlotWidgetImp::getPlot() const
{
   return dynamic_cast<PlotView*>(mpPlot);
}

void PlotWidgetImp::setName(const std::string& name)
{
   if (name != getName())
   {
      SessionItemImp::setName(name);
      if (name != mpPlot->getName())
      {
         mpPlot->setName(name);
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& PlotWidgetImp::getDisplayName() const
{
   const string& displayName = SessionItemImp::getDisplayName();
   if (displayName.empty() == false)
   {
      return displayName;
   }

   return mpPlot->getDisplayName();
}

const string& PlotWidgetImp::getDisplayText() const
{
   const string& displayText = SessionItemImp::getDisplayText();
   if (displayText.empty() == false)
   {
      return displayText;
   }

   return mpPlot->getDisplayText();
}

list<ContextMenuAction> PlotWidgetImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions;
   menuActions.push_back(ContextMenuAction(mpPrintAction, APP_PLOTWIDGET_PRINT_ACTION));
   menuActions.push_back(ContextMenuAction(mpSeparatorAction, APP_PLOTWIDGET_PRINT_SEPARATOR_ACTION));

   list<ContextMenuAction> plotActions = mpPlot->getContextMenuActions();
   std::copy(plotActions.begin(), plotActions.end(), std::back_inserter(menuActions));

   ContextMenuAction legendAction(mpLegendAction, APP_PLOTWIDGET_LEGEND_ACTION);
   legendAction.mBuddyType = ContextMenuAction::AFTER;
   legendAction.mBuddyId = APP_PLOTVIEW_MOUSE_MODE_SEPARATOR_ACTION;
   menuActions.push_back(legendAction);

   ContextMenuAction separatorAction(mpSeparator2Action, APP_PLOTWIDGET_LEGEND_SEPARATOR_ACTION);
   separatorAction.mBuddyType = ContextMenuAction::AFTER;
   separatorAction.mBuddyId = APP_PLOTWIDGET_LEGEND_ACTION;
   menuActions.push_back(separatorAction);

   return menuActions;
}

vector<string> PlotWidgetImp::getPropertiesPages() const
{
   vector<string> propertiesPages = SessionItemImp::getPropertiesPages();

   vector<string> plotPages = mpPlot->getPropertiesPages();
   copy(plotPages.begin(), plotPages.end(), back_inserter(propertiesPages));

   return propertiesPages;
}

QImage PlotWidgetImp::getCurrentImage()
{
   QImage image;
   getCurrentImage(image);
   return image;
}

bool PlotWidgetImp::getCurrentImage(QImage& image)
{
   // Get the pixmap for the widget
   QPixmap pixPlotWidget = QPixmap::grabWidget(mpPlotWidget);

   // Update the plot view to use a temporary display list when getting the pixmap
   mpPlot->useTemporaryDisplayList(true);

   // Get the pixmap for the plot view
   int iWidth = image.width();
   int iHeight = image.height();
   mpPlot->temporaryGlContextChange();
   QPixmap pixPlotView = mpPlot->renderPixmap(iWidth, iHeight);

   // Restore the original plot view display list
   mpPlot->useTemporaryDisplayList(false);

   // Draw the plot view pixmap in the widget pixmap
   if ((pixPlotWidget.isNull() == false) && (pixPlotView.isNull() == false))
   {
      QPoint ptPlot = mpPlot->mapTo(mpPlotWidget, QPoint(0, 0));
      QRect rcPlot(ptPlot, QSize(mpPlot->width(), mpPlot->height()));

      QPainter painterWidget(&pixPlotWidget);
      painterWidget.drawPixmap(rcPlot, pixPlotView);
      painterWidget.end();
   }

   if (pixPlotWidget.isNull() == true)
   {
      return false;
   }

   image = pixPlotWidget.toImage();
   return true;
}

void PlotWidgetImp::setClassificationPosition(PositionType ePosition)
{
   if (ePosition == mClassificationPosition)
   {
      return;
   }

   // Clear the text properties in the current position
   QString strClassification = getClassificationText();
   QFont ftClassification = getClassificationFont();
   QColor clrClassification = getClassificationColor();

   setClassificationText(QString());
   setClassificationFont(QFont());
   setClassificationColor(Qt::black);

   // Set the new position
   mClassificationPosition = ePosition;

   // Notify attached objects of the change
   notify(SIGNAL_NAME(Subject, Modified));

   // Set the text properties at the new position
   setClassificationText(strClassification);
   setClassificationFont(ftClassification);
   setClassificationColor(clrClassification);
}

PositionType PlotWidgetImp::getClassificationPosition() const
{
   return mClassificationPosition;
}

void PlotWidgetImp::setClassificationText(const Classification* pClassification)
{
   QString strClassification;

   if (pClassification != NULL)
   {
      if (pClassification->isValid() == true)
      {
         string classificationText = "";
         pClassification->getClassificationText(classificationText);
         if (classificationText.empty() == false)
         {
            strClassification = QString::fromStdString(classificationText);
         }
      }
   }

   setClassificationText(strClassification);
}

void PlotWidgetImp::setClassificationText(const QString& strClassification)
{
   QString strOrganization = getOrganizationText();
   setLabelText(strClassification, strOrganization);
}

QString PlotWidgetImp::getClassificationText() const
{
   // The top and bottom classification text is the same, so just get the top label text
   QString strClassification;
   switch (mClassificationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
      case TOP_LEFT_BOTTOM_RIGHT:
         strClassification = mpTopLeftLabel->text();
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
      case TOP_RIGHT_BOTTOM_RIGHT:
         strClassification = mpTopRightLabel->text();
         break;

      case CENTER:
         strClassification = mpTopCenterLabel->text();
         break;

      default:
         break;
   }

   // Remove any release info
   Service<ConfigurationSettings> pConfigSettings;

   QString releaseType = QString::fromStdString(StringUtilities::toDisplayString(pConfigSettings->getReleaseType()));

   int iPos = strClassification.lastIndexOf(releaseType);
   if (iPos != -1)
   {
      strClassification.truncate(iPos);
   }

   iPos = strClassification.lastIndexOf("\n");
   if (iPos != -1)
   {
      strClassification.truncate(iPos);
   }

   return strClassification;
}

void PlotWidgetImp::setClassificationFont(const QFont& ftClassification)
{
   mClassificationFont = ftClassification;

   QFont ftOrganization = getOrganizationFont();
   setLabelFont(ftClassification, ftOrganization);
}

QFont PlotWidgetImp::getClassificationFont() const
{
   return mClassificationFont.toQFont();
}

void PlotWidgetImp::setClassificationColor(const QColor& clrClassification)
{
   if (clrClassification.isValid() == false)
   {
      return;
   }

   QColor clrOrganization = getOrganizationColor();
   setLabelColor(clrClassification, clrOrganization);
}

QColor PlotWidgetImp::getClassificationColor() const
{
   // The top and bottom classification color is the same, so just get the top label color
   QColor clrClassification;
   switch (mClassificationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
      case TOP_LEFT_BOTTOM_RIGHT:
         clrClassification = mpTopLeftLabel->palette().color(QPalette::WindowText);
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
      case TOP_RIGHT_BOTTOM_RIGHT:
         clrClassification = mpTopRightLabel->palette().color(QPalette::WindowText);
         break;

      case CENTER:
         clrClassification = mpTopCenterLabel->palette().color(QPalette::WindowText);
         break;

      default:
         break;
   }

   return clrClassification;
}

void PlotWidgetImp::setOrganizationPosition(PositionType ePosition)
{
   if (ePosition == mOrganizationPosition)
   {
      return;
   }

   // Clear the text properties in the current position
   QString strOrganization = getOrganizationText();
   QFont ftOrganization = getOrganizationFont();
   QColor clrOrganization = getOrganizationColor();

   setOrganizationText(QString());
   setOrganizationFont(QFont());
   setOrganizationColor(Qt::black);

   // Set the new position
   mOrganizationPosition = ePosition;

   // Notify attached objects of the change
   notify(SIGNAL_NAME(Subject, Modified));

   // Set the text properties at the new position
   setOrganizationText(strOrganization);
   setOrganizationFont(ftOrganization);
   setOrganizationColor(clrOrganization);
}

PositionType PlotWidgetImp::getOrganizationPosition() const
{
   return mOrganizationPosition;
}

void PlotWidgetImp::setOrganizationText(const QString& strOrganization)
{
   QString strClassification = getClassificationText();
   setLabelText(strClassification, strOrganization);
}

QString PlotWidgetImp::getOrganizationText() const
{
   // The top and bottom organization text is the same, so just get the top label text
   QString strOrganization;
   switch (mOrganizationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
      case TOP_LEFT_BOTTOM_RIGHT:
         strOrganization = mpTopLeftLabel->text();
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
      case TOP_RIGHT_BOTTOM_RIGHT:
         strOrganization = mpTopRightLabel->text();
         break;

      case CENTER:
         strOrganization = mpTopCenterLabel->text();
         break;

      default:
         break;
   }

   // Remove any release info
   Service<ConfigurationSettings> pConfigSettings;

   QString releaseType = QString::fromStdString(StringUtilities::toDisplayString(pConfigSettings->getReleaseType()));

   int iPos = strOrganization.lastIndexOf(releaseType);
   if (iPos != -1)
   {
      strOrganization.truncate(iPos);
   }

   iPos = strOrganization.lastIndexOf("\n");
   if (iPos != -1)
   {
      strOrganization.truncate(iPos);
   }

   return strOrganization;
}

void PlotWidgetImp::setOrganizationFont(const QFont& ftOrganization)
{
   mOrganizationFont = ftOrganization;

   QFont ftClassification = getClassificationFont();
   setLabelFont(ftClassification, ftOrganization);
}

QFont PlotWidgetImp::getOrganizationFont() const
{
   return mOrganizationFont.toQFont();
}

void PlotWidgetImp::setOrganizationColor(const QColor& clrOrganization)
{
   if (clrOrganization.isValid() == false)
   {
      return;
   }

   QColor clrClassification = getClassificationColor();
   setLabelColor(clrClassification, clrOrganization);
}

QColor PlotWidgetImp::getOrganizationColor() const
{
   // The top and bottom organization color is the same, so just get the top label color
   QColor clrOrganization;
   switch (mOrganizationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
      case TOP_LEFT_BOTTOM_RIGHT:
         clrOrganization = mpTopLeftLabel->palette().color(QPalette::WindowText);
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
      case TOP_RIGHT_BOTTOM_RIGHT:
         clrOrganization = mpTopRightLabel->palette().color(QPalette::WindowText);
         break;

      case CENTER:
         clrOrganization = mpTopCenterLabel->palette().color(QPalette::WindowText);
         break;

      default:
         break;
   }

   return clrOrganization;
}

void PlotWidgetImp::setTitle(const QString& strTitle)
{
   if (strTitle != getTitle())
   {
      mpTitleLabel->setText(strTitle);
      mpTitleLabel->setVisible(!strTitle.isEmpty());
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

QString PlotWidgetImp::getTitle() const
{
   return mpTitleLabel->text();
}

void PlotWidgetImp::setTitleFont(const QFont& ftTitle)
{
   if (ftTitle != getTitleFont())
   {
      mTitleFont = ftTitle;
      mpTitleLabel->setFont(ftTitle);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

QFont PlotWidgetImp::getTitleFont() const
{
   return mTitleFont.toQFont();
}

void PlotWidgetImp::showAxis(AxisPosition axis, bool bShow)
{
   if ((axis == AXIS_LEFT) && (mpYAxis != NULL))
   {
      mpYAxis->setShown(bShow);
   }
   else if ((axis == AXIS_BOTTOM) && (mpXAxis != NULL))
   {
      mpXAxis->setShown(bShow);
   }
}

bool PlotWidgetImp::isAxisShown(AxisPosition axis) const
{
   bool bShown = false;
   if ((axis == AXIS_LEFT) && (mpYAxis != NULL))
   {
      bShown = mpYAxis->isVisible();
   }
   else if ((axis == AXIS_BOTTOM) && (mpXAxis != NULL))
   {
      bShown = mpXAxis->isVisible();
   }

   return bShown;
}

Axis* PlotWidgetImp::getAxis(AxisPosition axis) const
{
   Axis* pAxis = NULL;
   if (axis == AXIS_LEFT)
   {
      pAxis = dynamic_cast<Axis*>(mpYAxis);
   }
   else if (axis == AXIS_BOTTOM)
   {
      pAxis = dynamic_cast<Axis*>(mpXAxis);
   }

   return pAxis;
}

bool PlotWidgetImp::isLegendShown() const
{
   bool bShown = mpLegend->isVisible();
   return bShown;
}

Legend* PlotWidgetImp::getLegend() const
{
   return mpLegend;
}

void PlotWidgetImp::showLegend(bool bShow)
{
   if (bShow == false)
   {
      mpLegend->hide();
   }
   else if (bShow == true)
   {
      mpLegend->show();
   }

   mpLegendAction->setChecked(bShow);
}

void PlotWidgetImp::print(bool bDialog)
{
   SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " is Printing a Plot Widget");
   QImage widgetImage = getCurrentImage();
   PrintPixmap(QPixmap::fromImage(widgetImage), bDialog, this);
}

bool PlotWidgetImp::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::Polish)
      {
         Service<DesktopServices> pDesktop;
         attach(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu),
            Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowContextMenu)));
      }
   }

   return QWidget::event(pEvent);
}

bool PlotWidgetImp::eventFilter(QObject* o, QEvent* e)
{
   if ((e != NULL) && (o != NULL) && (o == mpPlot))
   {
      QEvent::Type eventType = e->type();
      if (eventType == QEvent::ContextMenu)
      {
         QContextMenuEvent* pMenuEvent = static_cast<QContextMenuEvent*>(e);
         contextMenuEvent(pMenuEvent);
         return true;
      }
   }

   return QWidget::eventFilter(o, e);
}

void PlotWidgetImp::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (pEvent != NULL)
   {
      // Create the context menu
      const QPoint& mouseLocation = pEvent->globalPos();
      list<ContextMenuAction> defaultActions = getContextMenuActions();

      vector<SessionItem*> sessionItems;
      sessionItems.push_back(dynamic_cast<SessionItem*>(this));
      sessionItems.push_back(dynamic_cast<SessionItem*>(mpPlot));

      ContextMenuImp menu(sessionItems, mouseLocation, defaultActions, this);

      // Notify to allow additional actions to be added
      notify(SIGNAL_NAME(PlotWidget, AboutToShowContextMenu), boost::any(static_cast<ContextMenu*>(&menu)));

      // Invoke the menu
      if (menu.show() == true)
      {
         return;
      }
   }

   QWidget::contextMenuEvent(pEvent);
}

const FontImp& PlotWidgetImp::getClassificationFontImp() const
{
   return mClassificationFont;
}

const FontImp& PlotWidgetImp::getOrganizationFontImp() const
{
   return mOrganizationFont;
}

const FontImp& PlotWidgetImp::getTitleFontImp() const
{
   return mTitleFont;
}

void PlotWidgetImp::updateName(const QString& strName)
{
   const string& widgetName = getName();
   string viewName = strName.toStdString();

   if (viewName != widgetName)
   {
      setName(viewName);
   }
}

void PlotWidgetImp::enableAnnotationToolBar(const MouseMode* pMouseMode)
{
   bool bShow = false;
   if (pMouseMode != NULL)
   {
      string modeName;
      pMouseMode->getName(modeName);
      if (modeName == "AnnotationMode")
      {
         bShow = true;
      }
   }

   mpAnnotationToolBar->setShown(bShow);
}

void PlotWidgetImp::updateScaleRange()
{
   if ((mpXAxis == NULL) || (mpYAxis == NULL))
   {
      return;
   }

   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;

   mpPlot->translateScreenToData(0, 0, dMinX, dMinY);
   mpPlot->translateScreenToData(mpPlot->width(), mpPlot->height(), dMaxX, dMaxY);

   // Update the axes
   mpXAxis->setValueRange(dMinX, dMaxX);
   mpYAxis->setValueRange(dMinY, dMaxY);
}

void PlotWidgetImp::updateMouseLabel(const QString& strTextX, const QString& strTextY)
{
   if ((mpXMouseLabel == NULL) || (mpYMouseLabel == NULL))
   {
      return;
   }

   // Set the label text
   mpXMouseLabel->setText(strTextX);
   mpYMouseLabel->setText(strTextY);

   // Set the label position
   LocationType position;

   const Locator* pLocator = mpPlot->getMouseLocator();
   if (pLocator != NULL)
   {
      position = pLocator->getLocation();
   }

   double dX = 0;
   double dY = 0;
   mpPlot->translateDataToScreen(position.mX, position.mY, dX, dY);

   mpXMouseLabel->setTextPosition(dX);
   mpYMouseLabel->setTextPosition(mpPlot->height() - dY);
}

void PlotWidgetImp::setLabelText(const QString& strClassification, const QString& strOrganization)
{
   QString strTopLeftText;
   QString strTopCenterText;
   QString strTopRightText;
   QString strBottomLeftText;
   QString strBottomCenterText;
   QString strBottomRightText;

   // Set the organization text first so that the classification text takes prescedence
   switch (mOrganizationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
         strTopLeftText = strOrganization;
         strBottomLeftText = strOrganization;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:
         strTopLeftText = strOrganization;
         strBottomRightText = strOrganization;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
         strTopRightText = strOrganization;
         strBottomLeftText = strOrganization;
         break;

      case TOP_RIGHT_BOTTOM_RIGHT:
         strTopRightText = strOrganization;
         strBottomRightText = strOrganization;
         break;

      case CENTER:
         strTopCenterText = strOrganization;
         strBottomCenterText = strOrganization;
         break;

      default:
         break;
   }

   switch (mClassificationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
         strTopLeftText = strClassification;
         strBottomLeftText = strClassification;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:
         strTopLeftText = strClassification;
         strBottomRightText = strClassification;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
         strTopRightText = strClassification;
         strBottomLeftText = strClassification;
         break;

      case TOP_RIGHT_BOTTOM_RIGHT:
         strTopRightText = strClassification;
         strBottomRightText = strClassification;
         break;

      case CENTER:
         strTopCenterText = strClassification;
         strBottomCenterText = strClassification;
         break;

      default:
         break;
   }

   // Add the release info
   Service<ConfigurationSettings> pConfigSettings;

   if (pConfigSettings->isProductionRelease() == false)
   {
      // Top text
      if (strTopCenterText.isEmpty() == false)
      {
         strTopCenterText.append("\n");
      }

      strTopCenterText.append(QString::fromStdString(
         StringUtilities::toDisplayString(pConfigSettings->getReleaseType())));

      // Bottom text
      bool bNewLine = !(strBottomCenterText.isEmpty());
      QString strProductionUse = "Not for Production Use";

      strBottomCenterText.prepend(strProductionUse);

      if (bNewLine == true)
      {
         strBottomCenterText.insert(strProductionUse.length(), "\n");
      }
   }

   mpTopLeftLabel->setText(strTopLeftText);
   mpTopCenterLabel->setText(strTopCenterText);
   mpTopRightLabel->setText(strTopRightText);
   mpBottomLeftLabel->setText(strBottomLeftText);
   mpBottomCenterLabel->setText(strBottomCenterText);
   mpBottomRightLabel->setText(strBottomRightText);

   notify(SIGNAL_NAME(Subject, Modified));
}

void PlotWidgetImp::setLabelFont(const QFont& ftClassification, const QFont& ftOrganization)
{
   QFont ftTopLeft;
   QFont ftTopCenter;
   QFont ftTopRight;
   QFont ftBottomLeft;
   QFont ftBottomCenter;
   QFont ftBottomRight;

   // Set the organization font first so that the classification font takes prescedence
   switch (mOrganizationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
         ftTopLeft = ftOrganization;
         ftBottomLeft = ftOrganization;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:
         ftTopLeft = ftOrganization;
         ftBottomRight = ftOrganization;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
         ftTopRight = ftOrganization;
         ftBottomLeft = ftOrganization;
         break;

      case TOP_RIGHT_BOTTOM_RIGHT:
         ftTopRight = ftOrganization;
         ftBottomRight = ftOrganization;
         break;

      case CENTER:
         ftTopCenter = ftOrganization;
         ftBottomCenter = ftOrganization;
         break;

      default:
         break;
   }

   switch (mClassificationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
         ftTopLeft = ftClassification;
         ftBottomLeft = ftClassification;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:
         ftTopLeft = ftClassification;
         ftBottomRight = ftClassification;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
         ftTopRight = ftClassification;
         ftBottomLeft = ftClassification;
         break;

      case TOP_RIGHT_BOTTOM_RIGHT:
         ftTopRight = ftClassification;
         ftBottomRight = ftClassification;
         break;

      case CENTER:
         ftTopCenter = ftClassification;
         ftBottomCenter = ftClassification;
         break;

      default:
         break;
   }

   mpTopLeftLabel->setFont(ftTopLeft);
   mpTopCenterLabel->setFont(ftTopCenter);
   mpTopRightLabel->setFont(ftTopRight);
   mpBottomLeftLabel->setFont(ftBottomLeft);
   mpBottomCenterLabel->setFont(ftBottomCenter);
   mpBottomRightLabel->setFont(ftBottomRight);

   notify(SIGNAL_NAME(Subject, Modified));
}

void PlotWidgetImp::setLabelColor(const QColor& clrClassification, const QColor& clrOrganization)
{
   QColor clrTopLeft;
   QColor clrTopCenter;
   QColor clrTopRight;
   QColor clrBottomLeft;
   QColor clrBottomCenter;
   QColor clrBottomRight;

   // Set the organization color first so that the classification color takes prescedence
   switch (mOrganizationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
         clrTopLeft = clrOrganization;
         clrBottomLeft = clrOrganization;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:
         clrTopLeft = clrOrganization;
         clrBottomRight = clrOrganization;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
         clrTopRight = clrOrganization;
         clrBottomLeft = clrOrganization;
         break;

      case TOP_RIGHT_BOTTOM_RIGHT:
         clrTopRight = clrOrganization;
         clrBottomRight = clrOrganization;
         break;

      case CENTER:
         clrTopCenter = clrOrganization;
         clrBottomCenter = clrOrganization;
         break;

      default:
         break;
   }

   switch (mClassificationPosition)
   {
      case TOP_LEFT_BOTTOM_LEFT:
         clrTopLeft = clrClassification;
         clrBottomLeft = clrClassification;
         break;

      case TOP_LEFT_BOTTOM_RIGHT:
         clrTopLeft = clrClassification;
         clrBottomRight = clrClassification;
         break;

      case TOP_RIGHT_BOTTOM_LEFT:
         clrTopRight = clrClassification;
         clrBottomLeft = clrClassification;
         break;

      case TOP_RIGHT_BOTTOM_RIGHT:
         clrTopRight = clrClassification;
         clrBottomRight = clrClassification;
         break;

      case CENTER:
         clrTopCenter = clrClassification;
         clrBottomCenter = clrClassification;
         break;

      default:
         break;
   }

   QPalette topLeftPalette = mpTopLeftLabel->palette();
   topLeftPalette.setColor(QPalette::WindowText, clrTopLeft);
   mpTopLeftLabel->setPalette(topLeftPalette);

   QPalette topCenterPalette = mpTopCenterLabel->palette();
   topCenterPalette.setColor(QPalette::WindowText, clrTopCenter);
   mpTopCenterLabel->setPalette(topCenterPalette);

   QPalette topRightPalette = mpTopRightLabel->palette();
   topRightPalette.setColor(QPalette::WindowText, clrTopRight);
   mpTopRightLabel->setPalette(topRightPalette);

   QPalette bottomLeftPalette = mpBottomLeftLabel->palette();
   bottomLeftPalette.setColor(QPalette::WindowText, clrBottomLeft);
   mpBottomLeftLabel->setPalette(bottomLeftPalette);

   QPalette bottomCenterPalette = mpBottomCenterLabel->palette();
   bottomCenterPalette.setColor(QPalette::WindowText, clrBottomCenter);
   mpBottomCenterLabel->setPalette(bottomCenterPalette);

   QPalette bottomRightPalette = mpBottomRightLabel->palette();
   bottomRightPalette.setColor(QPalette::WindowText, clrBottomRight);
   mpBottomRightLabel->setPalette(bottomRightPalette);

   notify(SIGNAL_NAME(Subject, Modified));
}

bool PlotWidgetImp::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml(getObjectType().c_str());

   if (!toXml(&xml))
   {
      return false;
   }

   serializer.serialize(xml);
   return true;
}

bool PlotWidgetImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader xml(NULL, false);
   DOMNode* pRoot = deserializer.deserialize(xml, getObjectType().c_str());
   return fromXml(pRoot, XmlBase::VERSION);
}

bool PlotWidgetImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("name", getName());
   pXml->addAttr("displayName", getDisplayName());
   pXml->addAttr("displayText", getDisplayText());
   pXml->addAttr("type", getObjectType());

   pXml->addAttr("legendShown", isLegendShown());
   pXml->addAttr("classificationPosition", mClassificationPosition);
   pXml->addAttr("organizationPosition", mClassificationPosition);

   if (mpPlotSet != NULL)
   {
      pXml->addAttr("plotSetId", mpPlotSet->getId());
   }
   if (mpPlot != NULL)
   {
      pXml->addAttr("plotViewId", mpPlot->getId());
   }

   if (mpAnnotationToolBar != NULL)
   {
      pXml->addAttr("AnnotationToolbarId", mpAnnotationToolBar->getId());
   }

   pXml->pushAddPoint(pXml->addElement("ClassificationFont"));
   // no foul if font not stored - just keep defaults
   if(!mClassificationFont.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();

   pXml->pushAddPoint(pXml->addElement("OrganizationFont"));
   // no foul if font not stored - just keep defaults
   if(!mOrganizationFont.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();

   if (mpXAxis != NULL)
   {
      pXml->pushAddPoint(pXml->addElement("Xaxis"));
      if(!mpXAxis->toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }
   if (mpYAxis != NULL)
   {
      pXml->pushAddPoint(pXml->addElement("Yaxis"));
      if(!mpYAxis->toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }

   return true;
}

bool PlotWidgetImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   setName(A(pElem->getAttribute(X("name"))));
   setDisplayName(A(pElem->getAttribute(X("displayName"))));
   setDisplayText(A(pElem->getAttribute(X("displayText"))));

   PlotSet* pPlotSet = dynamic_cast<PlotSet*>(
      SessionManagerImp::instance()->getSessionItem(A(pElem->getAttribute(X("plotSetId")))));
   if (pPlotSet != NULL)
   {
      mpPlotSet = pPlotSet;
   }

   PlotViewImp* pPlotView = dynamic_cast<PlotViewImp*>(
      SessionManagerImp::instance()->getSessionItem(A(pElem->getAttribute(X("plotViewId")))));
   if (pPlotView != NULL)
   {
      initialize(pPlotView, "", PlotType());
   }

   showLegend(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("legendShown")))));

   setClassificationPosition(StringUtilities::fromXmlString<PositionType>(
      A(pElem->getAttribute(X("classificationPosition")))));

   setOrganizationPosition(StringUtilities::fromXmlString<PositionType>(
      A(pElem->getAttribute(X("organizationPosition")))));

   for(DOMNode *pChld = pDocument->getFirstChild();  
      pChld != NULL;
      pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("ClassificationFont")))
      {
         if(!mClassificationFont.fromXml(pChld, version))
         {
            return false;
         }
      }
      else if (XMLString::equals(pChld->getNodeName(), X("OrganizationFont")))
      {
         if(!mOrganizationFont.fromXml(pChld, version))
         {
            return false;
         }
      }
      else if (XMLString::equals(pChld->getNodeName(), X("Xaxis")))
      {
         if(!mpXAxis->fromXml(pChld, version))
         {
            return false;
         }
      }
      else if (XMLString::equals(pChld->getNodeName(), X("Yaxis")))
      {
         if(!mpYAxis->fromXml(pChld, version))
         {
            return false;
         }
      }
   }

   return true;
}

bool PlotWidgetImp::canRename() const
{
   return mpPlot->canRename();
}

bool PlotWidgetImp::rename(const string &newName, string &errorMessage)
{
   PlotSet *pPlotSet = getPlotSet();
   if(pPlotSet == NULL)
   {
      return SessionItemImp::rename(newName, errorMessage);
   }
   return pPlotSet->renamePlot(dynamic_cast<PlotWidget*>(this), newName);
}

void PlotWidgetImp::selectPlotObject(PlotObject* pObject, bool bSelect)
{
   mpPlot->selectObject(pObject, bSelect);
   mpPlot->updateGL();
}
