/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AnyData.h"
#include "AspamPlotSelectionDialog.h"
#include "AspamViewer.h"
#include "AspamViewerDialog.h"
#include "Axis.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "GeoPoint.h"
#include "PlotSet.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PlotWindow.h"
#include "Point.h"
#include "PointSet.h"
#include "StringUtilities.h"

#include <QtCore/QDate>
#include <QtGui/QCloseEvent>
#include <QtGui/QComboBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QFontMetrics>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPalette>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTextCursor>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <vector>

using namespace std;

namespace
{
const int sMarginSize = 10;

inline QString getUniquePlotName(const QString& baseName, const QStringList& plotNames)
{
   QString name = baseName;
   int count = 1;
   while (plotNames.contains(name))
   {
      name = baseName + QString::number(count++);
   }
   return name;
}


inline double getDataPointFromCloudLayer(unsigned int index, const Aspam::CloudLayer& data)
{
   switch (index)
   {
      case 0:
         return data.mCoverage;
      case 2:
         return data.mBaseAltitude;
      case 4:
         return data.mTopAltitude;
      default:
         break; // blank
   }
   return 0.0;
}

inline double getDataPointFromAnalytic(unsigned int index, const Aspam::Analytic& data)
{
   switch (index)
   {
      case 0:
         return data.mHeight;
      case 1:
         return data.mWindDirection;
      case 2:
         return data.mWindSpeed;
      case 3:
         return data.mTemperature;
      case 4:
         return data.mHumidity;
      case 5:
         return data.mDensity;
      case 6:
         return data.mPressure;
      default:
         break; // blank
   }
   return 0.0;
}

inline double getDataPointFromAerosol(unsigned int index, const Aspam::Aerosol& data)
{
   switch (index)
   {
      case 0:
         return data.mHeight;
      case 1:
         return data.mPressure;
      case 2:
         return data.mTemperature;
      case 3:
         return data.mWaterVaporDensity;
      case 4:
         return data.mAlternateTemperature;
      case 5:
         return data.mAlternateWaterVaporDensity;
      case 6:
         return data.mLatitude;
      case 7:
         return data.mLongtitude;
      case 8:
         return data.mOzoneRatio;
      default:
         break; // blank
   }
   return 0.0;
}

inline double getDataPointFromSurfaceWeather(unsigned int index, const Aspam::SurfaceWeather& data)
{
   switch (index)
   {
      case 0:
      {
         unsigned int year = data.mYear;
         year += (year > 30) ? 1900 : 2000;
         QDate date = QDate(year, 1, 1).addDays(data.mJulianDay - 1);
         QTime time(data.mHour, data.mMinutes);
         QDateTime dateTime(date, time);
         return dateTime.toTime_t();
      }
      case 1:
         return data.mCloudBase1;
      case 2:
         return data.mCloudCoverage1;
      case 3:
         return data.mCloudThickness1;
      case 4:
         return data.mCloudBase2;
      case 5:
         return data.mCloudCoverage2;
      case 6:
         return data.mCloudThickness2;
      case 7:
         return data.mCloudBase3;
      case 8:
         return data.mCloudCoverage3;
      case 9:
         return data.mCloudThickness3;
      case 10:
         return data.mCloudBase4;
      case 11:
         return data.mCloudCoverage4;
      case 12:
         return data.mCloudThickness4;
      case 13:
         return data.mTotalCoverage;
      case 14:
         return data.mVisibility;
      case 17:
         return data.mPressure;
      case 18:
         return data.mTemperature;
      case 19:
         return data.mDewpoint;
      case 20:
         return data.mWindSpeed;
      case 21:
         return data.mWindDirection;
      case 22:
         return data.mAlternateWindSpeed;
      default:
         break; // blank
   }
   return 0.0;
}
};

AspamViewerDialog::AspamViewerDialog(AspamViewer* pViewer, QWidget* pParent) :
   QDialog(pParent),
   mpViewer(pViewer)
{
   QString viewName = QString::fromStdString(pViewer->getName());
   setWindowTitle(viewName);
   QVBoxLayout* pTopLevel = new QVBoxLayout(this);
   pTopLevel->setMargin(sMarginSize);
   pTopLevel->setSpacing(5);

   mpDataStack = new QStackedWidget(this);
   pTopLevel->addWidget(mpDataStack);

   mpDataWidget = new QTabWidget(this);
   mpDataStack->addWidget(mpDataWidget);
   mpErrorWidget = new QLabel("No Data Present or Invalid Data", this);
   mpDataStack->addWidget(mpErrorWidget);
   mpDataStack->setCurrentWidget(mpDataWidget);

   mpParagraphAgui = new ParagraphAgui(this);
   mpParagraphBgui = new ParagraphBgui(this);
   mpParagraphDgui = new ParagraphDgui(this);
   mpParagraphFgui = new ParagraphFgui(this);
   mpParagraphGgui = new ParagraphGgui(this);
   mpParagraphHgui = new ParagraphHgui(this);
   mpParagraphJgui = new ParagraphJgui(this);
   mpDataWidget->addTab(mpParagraphAgui, "Paragraph A");
   mpDataWidget->addTab(mpParagraphBgui, "B");
   mpDataWidget->addTab(mpParagraphDgui, "D");
   mpDataWidget->addTab(mpParagraphFgui, "F");
   mpDataWidget->addTab(mpParagraphGgui, "G");
   mpDataWidget->addTab(mpParagraphHgui, "H");
   mpDataWidget->addTab(mpParagraphJgui, "J");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphAgui), "Paragraph A - SITE");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphBgui), "Paragraph B - TIME");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphDgui), "Paragraph D - WEATHER AT SITE");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphFgui),
      "Paragraph F - WINDS, TEMPERATURE, ABS HUMIDITY, DENSITY, PRESSURE");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphGgui), "Paragraph G - REMARKS");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphHgui), "Paragraph H - AEROSOL PARAMETERS");
   mpDataWidget->setTabToolTip(mpDataWidget->indexOf(mpParagraphJgui), "Paragraph J - SURFACE WEATHER HISTORY");

   QHBoxLayout* pBottomLayout = new QHBoxLayout;
   pBottomLayout->setMargin(0);
   pBottomLayout->setSpacing(sMarginSize);
   pTopLevel->addLayout(pBottomLayout);

   mpAspamList = new QComboBox(this);
   mpAspamList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
   pBottomLayout->addWidget(mpAspamList, 0);
   pBottomLayout->addStretch(10);
   mpUnloadButton = new QPushButton("Unload", this);
   pBottomLayout->addWidget(mpUnloadButton, 0);
   QPushButton* pCloseButton = new QPushButton("Close", this);
   pBottomLayout->addWidget(pCloseButton, 0);

   resize(sizeHint());

   connect(mpUnloadButton, SIGNAL(clicked()), this, SLOT(unloadAspam()));
   connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));
   connect(mpAspamList, SIGNAL(activated(const QString&)), this, SLOT(setDataSet(const QString&)));
   connect(mpAspamList, SIGNAL(activated(const QString&)), this, SLOT(checkPlotStatus()));
   connect(mpDataWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(checkPlotStatus()));

   connect(mpParagraphDgui, SIGNAL(plot()), this, SLOT(plot()));
   connect(mpParagraphDgui, SIGNAL(plotDataUpdated()), this, SLOT(checkPlotStatus()));
   connect(mpParagraphFgui, SIGNAL(plot()), this, SLOT(plot()));
   connect(mpParagraphFgui, SIGNAL(plotDataUpdated()), this, SLOT(checkPlotStatus()));
   connect(mpParagraphHgui, SIGNAL(plot()), this, SLOT(plot()));
   connect(mpParagraphHgui, SIGNAL(plotDataUpdated()), this, SLOT(checkPlotStatus()));
   connect(mpParagraphJgui, SIGNAL(plot()), this, SLOT(plot()));
   connect(mpParagraphJgui, SIGNAL(plotDataUpdated()), this, SLOT(checkPlotStatus()));

   mpDataStack->setCurrentWidget(mpErrorWidget);
   populateAspamList();
}

AspamViewerDialog::~AspamViewerDialog()
{}

void AspamViewerDialog::populateAspamList()
{
   QString currentItemText(mpAspamList->currentText());
   mAspams.clear();
   mpAspamList->clear();

   try
   {
      vector<DataElement*> aspamsDe = mpModelServices->getElements("Aspam");
      for (vector<DataElement*>::const_iterator ait = aspamsDe.begin(); ait != aspamsDe.end(); ++ait)
      {
         Any* pAspamContainer(static_cast<Any*>(*ait));
         if (pAspamContainer != NULL)
         {
            string name = pAspamContainer->getDisplayName();
            if (name.empty() == true)
            {
               name = pAspamContainer->getName();
            }

            QString text = QString::fromStdString(name);
            mAspams[text] = pAspamContainer;
            mpAspamList->addItem(text);
            if (text == currentItemText)
            {
               mpAspamList->setCurrentIndex(mpAspamList->count() - 1);
            }
         }
      }
      mpAspamList->setMinimumSize(mpAspamList->sizeHint());
      setDataSet(mpAspamList->currentText());
      mpUnloadButton->setEnabled(mpAspamList->count() > 0);
   }
   catch (AssertException exc)
   {
      Service<DesktopServices> pDesktop;
      pDesktop->showMessageBox("ASPAM Error", string("A critical error occurred: ") + exc.getText(), "Ok");
   }
}

void AspamViewerDialog::setDataSet(const QString& dataset)
{
   if (mAspams.contains(dataset))
   {
      const Aspam* pAspam = model_cast<Aspam*>(mAspams[dataset]);
      mpDataStack->setCurrentWidget(mpDataWidget);
      mpParagraphAgui->setData(pAspam->getParagraphA(), mpDataWidget);
      mpParagraphBgui->setData(pAspam->getParagraphB(), mpDataWidget);
      mpParagraphDgui->setData(pAspam->getParagraphD(), mpDataWidget);
      mpParagraphFgui->setData(pAspam->getParagraphF(), mpDataWidget);
      mpParagraphGgui->setData(pAspam->getParagraphG(), mpDataWidget);
      mpParagraphHgui->setData(pAspam->getParagraphH(), mpDataWidget);
      mpParagraphJgui->setData(pAspam->getParagraphJ(), mpDataWidget);

      Service<DesktopServices> pDesktopServices;
      if (pDesktopServices.get() != NULL)
      {
         PlotWindow* pPlotWindow(static_cast<PlotWindow*>(pDesktopServices->getWindow("Aspam", PLOT_WINDOW)));
         if (pPlotWindow != NULL)
         {
            PlotSet* pPlotSet(pPlotWindow->getPlotSet(dataset.toStdString()));
            if (pPlotSet != NULL)
            {
               pPlotWindow->setCurrentPlotSet(pPlotSet);
            }
         }
      }
   }
   else
   {
      mpDataStack->setCurrentWidget(mpErrorWidget);
   }
}

void AspamViewerDialog::plot()
{
   QPushButton* pButton(NULL); // dummy
   QMap<QString, unsigned int> selected(getPlotSelectionList(pButton));

   if (!selected.empty())
   {
      Service<DesktopServices> pDesktopServices;
      PlotWindow* pPlotWindow(NULL);
      PlotSet* pPlotSet(NULL);
      QStringList plotNames;
      QString currentPlotName;

      if (pDesktopServices.get() != NULL)
      {
         pPlotWindow = static_cast<PlotWindow*>(pDesktopServices->createWindow("Aspam", PLOT_WINDOW));
         if (pPlotWindow == NULL)
         {
            pPlotWindow = static_cast<PlotWindow*>(pDesktopServices->getWindow("Aspam", PLOT_WINDOW));
         }
      }
      if (pPlotWindow != NULL)
      {
         pPlotSet = pPlotWindow->createPlotSet(mpAspamList->currentText().toStdString());
         if (pPlotSet == NULL)
         {
            pPlotSet = pPlotWindow->getPlotSet(mpAspamList->currentText().toStdString());
         }
      }
      if (pPlotSet != NULL)
      {
         vector<PlotWidget*> plots;
         pPlotSet->getPlots(plots);

         for (vector<PlotWidget*>::const_iterator pit = plots.begin(); pit != plots.end(); ++pit)
         {
            PlotWidget* pWidget(*pit);
            if (pWidget != NULL)
            {
               PlotView* pView(pWidget->getPlot());
               if (pView != NULL)
               {
                  string name = pView->getName();
                  plotNames << name.c_str();
                  if (pPlotSet->getCurrentPlot() == pWidget)
                  {
                     currentPlotName = name.c_str();
                  }
               }
            }
         }
      }

      QStringList columnTitles;
      for (QMap<QString, unsigned int>::iterator sit = selected.begin(); sit != selected.end(); ++sit)
      {
         columnTitles << sit.key();
      }
      AspamPlotSelectionDialog selectionDialog(columnTitles, this);
      selectionDialog.setPlotNames(plotNames, currentPlotName);
      if (selectionDialog.exec() == QDialog::Accepted)
      {
         PlotWidget* pPlot(NULL);
         PlotView* pPlotView(NULL);
         string plotName(selectionDialog.getPlotName().toStdString());
         if (plotName == "New Plot")
         {
            plotName = getUniquePlotName(selectionDialog.getPrimaryAxis(), plotNames).toStdString();
         }
         if (pPlotSet != NULL)
         {
            pPlot = pPlotSet->getPlot(plotName);
            if (pPlot == NULL)
            {
               pPlot = pPlotSet->createPlot(plotName, CARTESIAN_PLOT);
               if (pPlot != NULL)
               {
                  pPlot->getAxis(AXIS_BOTTOM)->setTitle(selectionDialog.getPrimaryAxis().toStdString());
                  pPlot->showLegend(true);
               }
            }
         }
         if (pPlot != NULL)
         {
            pPlotView = pPlot->getPlot();
         }
         if (pPlotView == NULL)
         {
            QMessageBox::warning(this, "Can't create plot",
                                 QString("Unable to create plot ") + plotName.c_str(), QMessageBox::Ok, 0);
         }
         else
         {
            unsigned int primary(selected[selectionDialog.getPrimaryAxis()]);
            for (QMap<QString, unsigned int>::iterator iter = selected.begin(); iter != selected.end(); ++iter)
            {
               if (iter.value() == primary)
               {
                  continue;
               }
               PointSet* pCurve(static_cast<PointSet*>(pPlotView->addObject(POINT_SET, true)));
               if (pCurve != NULL)
               {
                  pCurve->setObjectName(iter.key().toStdString());
                  vector<Point*> dataPoints;
                  getPlotSelectionData(dataPoints, pPlotView, primary, iter.value());
                  pCurve->setPoints(dataPoints);
               }
            }
         }
         pPlotView->zoomExtents();
         pPlotSet->setCurrentPlot(pPlot);
      }
   }
}

void AspamViewerDialog::checkPlotStatus()
{
   QPushButton* pButton(NULL);
   QMap<QString, unsigned int> selected(getPlotSelectionList(pButton));
   if (pButton != NULL)
   {
      pButton->setEnabled(selected.count() >= 2);
   }
}

void AspamViewerDialog::unloadAspam()
{
   try
   {
      Service<DesktopServices> pDesktopServices;
      QString datasetName(mpAspamList->currentText());
      Any* pAspamContainer = mAspams[datasetName];
      if (pAspamContainer == NULL)
      {
         throw;
      }
      if (mpModelServices->destroyElement(pAspamContainer))
      {
         mAspams.remove(datasetName);
         populateAspamList();

         PlotWindow* pPlotWindow(static_cast<PlotWindow*>(pDesktopServices->getWindow("Aspam", PLOT_WINDOW)));
         if (pPlotWindow != NULL)
         {
            PlotSet* pPlotSet(pPlotWindow->getPlotSet(datasetName.toStdString()));
            if (pPlotSet != NULL)
            {
               pPlotWindow->deletePlotSet(pPlotSet);
            }
         }
      }
   }
   catch (AssertException exc)
   {
      QMessageBox::critical(NULL, "ASPAM Error", QString("A critical error occured: ") + exc.getText().c_str(),
                            QMessageBox::Ok, QMessageBox::NoButton);
   }
}

QMap<QString, unsigned int> AspamViewerDialog::getPlotSelectionList(QPushButton*& pButton) const
{
   QMap<QString, unsigned int> selected;
   pButton = NULL;
   if (mpDataStack->currentWidget() == mpDataWidget)
   {
      ParagraphGui* pGui = dynamic_cast<ParagraphGui*>(mpDataWidget->currentWidget());
      QTableWidget* pTable(NULL);
      if (pGui != NULL)
      {
         pTable = pGui->getTable();
         pButton = pGui->getButton();
      }
      if (pTable != NULL)
      {
         for (int col = 0; col < pTable->columnCount(); ++col)
         {
            if (pTable->isItemSelected(pTable->item(0, col)))
            {
               QString key(pTable->horizontalHeaderItem(col)->text());
               key.replace(QChar('\n'), " ");
               selected[key] = col;
            }
         }
      }
   }

   return selected;
}

void AspamViewerDialog::getPlotSelectionData(vector<Point*>& points, PlotView* pView, unsigned int primary,
                                             unsigned int secondary) const
{
   if (mpDataStack->currentWidget() == mpDataWidget)
   {
      points.clear();
      ParagraphGui* pGui = dynamic_cast<ParagraphGui*>(mpDataWidget->currentWidget());
      if (pGui != NULL)
      {
         pGui->getPlotData(points, pView, primary, secondary);
      }
   }
}

void AspamViewerDialog::closeEvent(QCloseEvent* pEvent)
{
   QDialog::closeEvent(pEvent);
   mpViewer->abort();
}

/***** Paragraph GUIs *****/
ParagraphGui::ParagraphGui(QWidget* pParent) :
   QFrame(pParent)
{}

QTableWidget* ParagraphGui::getTable() const
{
   return NULL;
}

QPushButton* ParagraphGui::getButton() const
{
   return NULL;
}

void ParagraphGui::getPlotData(vector<Point*>& points, PlotView* pView, unsigned int primary, unsigned int secondary)
{}

ParagraphAgui::ParagraphAgui(QWidget* pParent) :
   ParagraphGui(pParent)
{
   QLabel* pSiteIdLabel = new QLabel("Site ID", this);

   mpSiteId = new QLineEdit(this);
   mpSiteId->setReadOnly(true);

   // Layout
   QVBoxLayout* pTopLayout = new QVBoxLayout(this);
   pTopLayout->setMargin(sMarginSize);

   QHBoxLayout* pSiteIdLayout = new QHBoxLayout;
   pTopLayout->addLayout(pSiteIdLayout);
   pSiteIdLayout->addWidget(pSiteIdLabel, 0);
   pSiteIdLayout->addSpacing(sMarginSize);
   pSiteIdLayout->addWidget(mpSiteId, 0);
   QFontMetrics fontMetrics = mpSiteId->fontMetrics();
   QSize siteIdSize(fontMetrics.width("N00*00'00, W000*00'000000"),
                    mpSiteId->minimumSize().height());
   mpSiteId->setMinimumSize(siteIdSize);
   pSiteIdLayout->addStretch(1);

   pTopLayout->addStretch(10);
}

ParagraphAgui::~ParagraphAgui()
{}

ParagraphBgui::ParagraphBgui(QWidget* pParent) :
   ParagraphGui(pParent)
{
   QLabel* pDateTimeLabel = new QLabel("Date/Time", this);

   mpDateTime = new QDateTimeEdit(this);
   mpDateTime->setEnabled(false);
   QPalette palette(mpDateTime->palette());
   mpDateTime->setPalette(palette);

   // Layout
   QVBoxLayout* pTopLayout = new QVBoxLayout(this);
   pTopLayout->setMargin(sMarginSize);

   QHBoxLayout* pDateTimeLayout = new QHBoxLayout;
   pTopLayout->addLayout(pDateTimeLayout);
   pDateTimeLayout->addWidget(pDateTimeLabel, 0);
   pDateTimeLayout->addSpacing(sMarginSize);
   pDateTimeLayout->addWidget(mpDateTime, 0);
   pDateTimeLayout->addStretch(1);

   pTopLayout->addStretch(10);
}

ParagraphBgui::~ParagraphBgui()
{}

ParagraphDgui::ParagraphDgui(QWidget* pParent) :
   ParagraphGui(pParent),
   mpRawData(NULL)
{
   QLabel* pSurfaceVisibilityLabel = new QLabel("Surface Visibility", this);
   QLabel* pTotalCoverageLabel = new QLabel("Total Coverage", this);
   QLabel* pRemarkLabel = new QLabel("Remarks", this);
   QLabel* pCloudsLabel = new QLabel("Cloud Layers", this);

   mpSurfaceVisibility = new QLineEdit(this);
   mpSurfaceVisibility->setReadOnly(true);

   mpTotalCoverage = new QSpinBox(this);
   mpTotalCoverage->setSuffix("/8");
   mpTotalCoverage->setMinimum(0);
   mpTotalCoverage->setMaximum(8);
   mpTotalCoverage->setEnabled(false);
   QPalette palette(mpTotalCoverage->palette());
   mpTotalCoverage->setPalette(palette);

   mpRemark = new QTextEdit(this);
   mpRemark->setReadOnly(true);

   QGroupBox* pWindGroup = new QGroupBox("Wind", this);

   QLabel* pWindDirectionLabel = new QLabel("Direction", pWindGroup);
   QLabel* pWindSpeedLabel = new QLabel("Speed", pWindGroup);
   QLabel* pGustSpeedLabel = new QLabel("Gust", pWindGroup);
   mpWindDirection = new QLineEdit(pWindGroup);
   mpWindDirection->setReadOnly(true);

   mpWindSpeed = new QLineEdit(pWindGroup);
   mpWindSpeed->setReadOnly(true);

   mpGustSpeed = new QLineEdit(pWindGroup);
   mpGustSpeed->setReadOnly(true);

   mpClouds = new QTableWidget(4, 4, this);
   mpClouds->setSelectionBehavior(QAbstractItemView::SelectColumns);
   QStringList cloudColumnLabels;
   cloudColumnLabels << "Coverage" << "Cloud Type" << "Base Altitude" << "Top Altitude";
   mpClouds->setHorizontalHeaderLabels(cloudColumnLabels);

   mpPlotButton = new QPushButton("Plot", this);

   // Top Level Layout
   QGridLayout* pTopLayout = new QGridLayout(this);
   pTopLayout->setMargin(sMarginSize);
   pTopLayout->setSpacing(sMarginSize);

   pTopLayout->addWidget(pSurfaceVisibilityLabel, 0, 0);
   pTopLayout->addWidget(mpSurfaceVisibility, 0, 1);
   QFontMetrics fontMetrics = mpSurfaceVisibility->fontMetrics();
   QSize surfaceVisibilitySize(fontMetrics.width("00000"),
                               mpSurfaceVisibility->minimumSize().height());
   mpSurfaceVisibility->setMinimumSize(surfaceVisibilitySize);

   pTopLayout->addWidget(pTotalCoverageLabel, 1, 0);
   pTopLayout->addWidget(mpTotalCoverage, 1, 1);

   QGridLayout* pWindLayout = new QGridLayout(pWindGroup);
   pWindLayout->setMargin(sMarginSize);
   pWindLayout->addWidget(pWindDirectionLabel, 0, 0);
   pWindLayout->addWidget(mpWindDirection, 0, 1);
   pWindLayout->addWidget(pWindSpeedLabel, 1, 0);
   pWindLayout->addWidget(mpWindSpeed, 1, 1);
   pWindLayout->addWidget(pGustSpeedLabel, 2, 0);
   pWindLayout->addWidget(mpGustSpeed, 2, 1);
   pTopLayout->addWidget(pWindGroup, 2, 0, 1, 2);

   pTopLayout->addWidget(pRemarkLabel, 3, 0, Qt::AlignTop);
   pTopLayout->addWidget(mpRemark, 3, 1);
   fontMetrics = mpRemark->fontMetrics();
   QSize remarksSize(fontMetrics.width("000000000000000"),
                     mpRemark->minimumSize().height());
   mpRemark->setMinimumSize(remarksSize);
   mpRemark->resize(remarksSize);

   pTopLayout->addWidget(pCloudsLabel, 4, 0);
   pTopLayout->addWidget(mpClouds, 5, 0, 1, 2);
   mpClouds->setMinimumSize(mpClouds->sizeHint());

   QHBoxLayout* pButtonLayout = new QHBoxLayout;
   pTopLayout->addLayout(pButtonLayout, 6, 0, 6, 6);
   pButtonLayout->addWidget(mpPlotButton, 0);
   pButtonLayout->addStretch(1);

   pTopLayout->setRowStretch(3, 10);
   pTopLayout->setColumnStretch(2, 10);

   connect(mpPlotButton, SIGNAL(clicked()), this, SIGNAL(plot()));
   connect(mpClouds, SIGNAL(itemSelectionChanged()), this, SIGNAL(plotDataUpdated()));
}

ParagraphDgui::~ParagraphDgui()
{}

QTableWidget* ParagraphDgui::getTable() const
{
   return mpClouds;
}

QPushButton* ParagraphDgui::getButton() const
{
   return mpPlotButton;
}

ParagraphFgui::ParagraphFgui(QWidget* pParent) :
   ParagraphGui(pParent),
   mpRawData(NULL)
{
   QLabel* pLevelLabel = new QLabel("Level", this);
   mpLevel = new QLineEdit(this);
   mpLevel->setReadOnly(true);

   QLabel* pAnalyticLabel = new QLabel("Analytic Data", this);
   mpAnalytic = new QTableWidget(0, 7, this);
   mpAnalytic->setSelectionBehavior(QAbstractItemView::SelectColumns);
   QStringList analyticColumnLabels;
   analyticColumnLabels << "Height"
                        << "Wind Direction"
                        << "Wind Speed"
                        << "Temperature"
                        << "Humidity"
                        << "Density"
                        << "Pressure";
   mpAnalytic->setHorizontalHeaderLabels(analyticColumnLabels);
   mpPlotButton = new QPushButton("Plot", this);

   // Layout
   QVBoxLayout* pTopLayout = new QVBoxLayout(this);
   pTopLayout->setMargin(sMarginSize);

   QHBoxLayout* pLabelLayout = new QHBoxLayout;
   pTopLayout->addLayout(pLabelLayout);

   pLabelLayout->addWidget(pLevelLabel, 0);
   pLabelLayout->addSpacing(sMarginSize);
   pLabelLayout->addWidget(mpLevel, 10);

   pTopLayout->addWidget(pAnalyticLabel, 0);
   pTopLayout->addSpacing(sMarginSize);
   pTopLayout->addWidget(mpAnalytic, 10);
   pTopLayout->addSpacing(sMarginSize);

   QHBoxLayout* pButtonLayout = new QHBoxLayout;
   pTopLayout->addLayout(pButtonLayout);
   pButtonLayout->addWidget(mpPlotButton, 0);
   pButtonLayout->addStretch(1);

   connect(mpPlotButton, SIGNAL(clicked()), this, SIGNAL(plot()));
   connect(mpAnalytic, SIGNAL(itemSelectionChanged()), this, SIGNAL(plotDataUpdated()));
}

ParagraphFgui::~ParagraphFgui()
{}

QTableWidget* ParagraphFgui::getTable() const
{
   return mpAnalytic;
}

QPushButton* ParagraphFgui::getButton() const
{
   return mpPlotButton;
}

ParagraphGgui::ParagraphGgui(QWidget* pParent) :
   ParagraphGui(pParent)
{
   QLabel* pRemarksLabel = new QLabel("Remarks", this);
   mpRemarks = new QTextEdit(this);
   mpRemarks->setReadOnly(true);
   mpRemarks->setWordWrapMode(QTextOption::NoWrap);
   mpRemarks->setLineWrapMode(QTextEdit::NoWrap);

   // Layout
   QVBoxLayout* pTopLayout = new QVBoxLayout(this);
   pTopLayout->setMargin(sMarginSize);

   QHBoxLayout* pLabelLayout = new QHBoxLayout;
   pTopLayout->addLayout(pLabelLayout);
   pLabelLayout->addWidget(pRemarksLabel, 0);
   pLabelLayout->addStretch(1);

   pTopLayout->addSpacing(sMarginSize);
   pTopLayout->addWidget(mpRemarks, 10);
}

ParagraphGgui::~ParagraphGgui()
{}

ParagraphHgui::ParagraphHgui(QWidget* pParent) :
   ParagraphGui(pParent),
   mpRawData(NULL)
{
   QLabel* pLevelsLabel = new QLabel("Levels", this);
   QLabel* pSeasonalDependenceLabel = new QLabel("Seasonal Dependence", this);
   QLabel* pStratosphericAerosolLabel = new QLabel("Profile and Extinction Type\nfor Stratospheric Aerosol", this);
   QLabel* pOzoneProfileLabel = new QLabel("Ozone Profile", this);
   QLabel* pBlpqiLabel = new QLabel("Boundary Layer Parameter\nQuality Index", this);
   QLabel* pPrimaryLabel = new QLabel("Primary", this);
   QLabel* pAlternateLabel = new QLabel("Alternate", this);
   QLabel* pBlapLabel = new QLabel("Bounding Layer\nAerosol Parameter", this);
   QLabel* pAirParcelTypeLabel = new QLabel("Air Parcel Type", this);
   QLabel* pSurfaceVisibilityLabel = new QLabel("Surface Visibility", this);
   QLabel* pAerosolLabel = new QLabel("Aerosol Data", this);

   mpLevels = new QLineEdit(this);
   mpLevels->setReadOnly(true);

   mpSeasonalDependence = new QComboBox(this);
   mpSeasonalDependence->setEnabled(false);
   QStringList seasonalDependenceItems;
   seasonalDependenceItems <<
      "1 - Spring/Summer" <<
      "2 - Fall/Winter";
   mpSeasonalDependence->addItems(seasonalDependenceItems);
   QPalette palette = mpSeasonalDependence->palette();
   mpSeasonalDependence->setPalette(palette);

   mpStratosphericAerosol = new QComboBox(this);
   mpStratosphericAerosol->setEnabled(false);
   QStringList stratosphericAerosoltems;
   stratosphericAerosoltems <<
      "1 - Background (small amount)" <<
      "2 - Moderate Concentration (aged profile)" <<
      "3 - High Concentration (fresh profile)" <<
      "4 - High Concentration (aged profile)" <<
      "5 - Moderate Concentration (fresh profile)" <<
      "6 - Missing";
   mpStratosphericAerosol->addItems(stratosphericAerosoltems);
   palette = mpStratosphericAerosol->palette();
   mpStratosphericAerosol->setPalette(palette);

   mpOzoneProfile = new QComboBox(this);
   mpOzoneProfile->setEnabled(false);
   QStringList ozoneProfileItems;
   ozoneProfileItems <<
      "1 - Latitude < 20°" <<
      "2 - Latitude 20°-70° (summer)" <<
      "3 - Latitude 20°-70° (winter)" <<
      "4 - Latitude > 70° (summer)" <<
      "5 - Latitude > 70° (winter)" <<
      "6 - US Standard";
   mpOzoneProfile->addItems(ozoneProfileItems);
   palette = mpOzoneProfile->palette();
   mpOzoneProfile->setPalette(palette);

   mpBlpqi = new QLineEdit(this);
   mpBlpqi->setReadOnly(true);

   mpPrimaryBlap = new QComboBox(this);
   mpPrimaryBlap->setEnabled(false);
   QStringList blapItems;
   blapItems <<
      "1 - Rural" <<
      "2 - Urban" <<
      "3 - Open Ocean" <<
      "4 - Coastal" <<
      "5 - Tropospheric" <<
      "6 - Fog";
   mpPrimaryBlap->addItems(blapItems);
   palette = mpPrimaryBlap->palette();
   mpPrimaryBlap->setPalette(palette);

   mpAirParcelType = new QLineEdit(this);
   mpAirParcelType->setReadOnly(true);

   mpSurfaceVisibility = new QLineEdit(this);
   mpSurfaceVisibility->setReadOnly(true);

   mpAlternateBlap = new QComboBox(this);
   mpAlternateBlap->setEnabled(false);
   mpAlternateBlap->addItems(blapItems);
   palette = mpAlternateBlap->palette();
   mpAlternateBlap->setPalette(palette);

   mpAlternateAirParcelType = new QLineEdit(this);
   mpAlternateAirParcelType->setReadOnly(true);

   mpAlternateSurfaceVisibility = new QLineEdit(this);
   mpAlternateSurfaceVisibility->setReadOnly(true);

   mpAerosol = new QTableWidget(0, 9, this);
   mpAerosol->setSelectionBehavior(QAbstractItemView::SelectColumns);
   QStringList aerosolColumnLabels;
   aerosolColumnLabels <<
      "Height" <<
      "Pressure" <<
      "Temperature" <<
      "Water Vapor\nDensity" <<
      "Alternate\nTemperature" <<
      "Alternate Water\nVapor Density" <<
      "Latitude" <<
      "Longtitude" <<
      "Ozone Ratio";
   mpAerosol->setHorizontalHeaderLabels(aerosolColumnLabels);

   mpPlotButton = new QPushButton("Plot", this);

   // Layout
   QGridLayout* pTopLayout = new QGridLayout(this);
   pTopLayout->setMargin(sMarginSize);
   pTopLayout->setSpacing(sMarginSize);

   pTopLayout->addWidget(pLevelsLabel, 0, 0);
   pTopLayout->addWidget(mpLevels, 0, 1, 1, 2);
   QFontMetrics fontMetrics = mpLevels->fontMetrics();
   QSize levelsSize(fontMetrics.width("000"), mpLevels->minimumSize().height());
   mpLevels->setMinimumSize(levelsSize);

   pTopLayout->addWidget(pSeasonalDependenceLabel, 1, 0);
   pTopLayout->addWidget(mpSeasonalDependence, 1, 1, 1, 2);

   pTopLayout->addWidget(pStratosphericAerosolLabel, 2, 0);
   pTopLayout->addWidget(mpStratosphericAerosol, 2, 1, 1, 2);

   pTopLayout->addWidget(pOzoneProfileLabel, 3, 0);
   pTopLayout->addWidget(mpOzoneProfile, 3, 1, 1, 2);

   pTopLayout->addWidget(pBlpqiLabel, 4, 0);
   pTopLayout->addWidget(mpBlpqi, 4, 1, 1, 2);
   fontMetrics = mpBlpqi->fontMetrics();
   QSize blpqiSize(fontMetrics.width("000"), mpBlpqi->minimumSize().height());
   mpBlpqi->setMinimumSize(blpqiSize);

   pTopLayout->addWidget(pPrimaryLabel, 5, 1, Qt::AlignCenter);
   pTopLayout->addWidget(pAlternateLabel, 5, 2, Qt::AlignCenter);

   pTopLayout->addWidget(pBlapLabel, 6, 0);
   pTopLayout->addWidget(mpPrimaryBlap, 6, 1);
   pTopLayout->addWidget(mpAlternateBlap, 6, 2);

   pTopLayout->addWidget(pAirParcelTypeLabel, 7, 0);
   pTopLayout->addWidget(mpAirParcelType, 7, 1);
   pTopLayout->addWidget(mpAlternateAirParcelType, 7, 2);
   fontMetrics = mpAirParcelType->fontMetrics();
   QSize airParcelTypeSize(fontMetrics.width("000"), mpAirParcelType->minimumSize().height());
   mpAirParcelType->setMinimumSize(airParcelTypeSize);
   mpAlternateAirParcelType->setMinimumSize(airParcelTypeSize);

   pTopLayout->addWidget(pSurfaceVisibilityLabel, 8, 0);
   pTopLayout->addWidget(mpSurfaceVisibility, 8, 1);
   pTopLayout->addWidget(mpAlternateSurfaceVisibility, 8, 2);
   fontMetrics = mpSurfaceVisibility->fontMetrics();
   QSize surfaceVisibilitySize(fontMetrics.width("000"), mpSurfaceVisibility->minimumSize().height());
   mpSurfaceVisibility->setMinimumSize(surfaceVisibilitySize);
   mpAlternateSurfaceVisibility->setMinimumSize(surfaceVisibilitySize);

   pTopLayout->addWidget(pAerosolLabel, 9, 0);
   pTopLayout->addWidget(mpAerosol, 10, 0, 1, 4);
   mpAerosol->setMinimumSize(mpAerosol->sizeHint());

   QHBoxLayout* pButtonLayout = new QHBoxLayout;
   pTopLayout->addLayout(pButtonLayout, 11, 0, 1, 2);
   pButtonLayout->addWidget(mpPlotButton, 0);
   pButtonLayout->addStretch(1);

   pTopLayout->setRowStretch(0, 0);
   pTopLayout->setRowStretch(1, 0);
   pTopLayout->setRowStretch(2, 0);
   pTopLayout->setRowStretch(3, 0);
   pTopLayout->setRowStretch(4, 0);
   pTopLayout->setRowStretch(5, 0);
   pTopLayout->setRowStretch(6, 0);
   pTopLayout->setRowStretch(7, 0);
   pTopLayout->setRowStretch(8, 0);
   pTopLayout->setRowStretch(9, 0);
   pTopLayout->setRowStretch(10, 10);
   pTopLayout->setRowStretch(11, 0);
   pTopLayout->setColumnStretch(0, 0);
   pTopLayout->setColumnStretch(1, 0);
   pTopLayout->setColumnStretch(2, 0);
   pTopLayout->setColumnStretch(3, 10);

   connect(mpPlotButton, SIGNAL(clicked()), this, SIGNAL(plot()));
   connect(mpAerosol, SIGNAL(itemSelectionChanged()), this, SIGNAL(plotDataUpdated()));
}

ParagraphHgui::~ParagraphHgui()
{}

QTableWidget* ParagraphHgui::getTable() const
{
   return mpAerosol;
}

QPushButton* ParagraphHgui::getButton() const
{
   return mpPlotButton;
}

ParagraphJgui::ParagraphJgui(QWidget* pParent) :
   ParagraphGui(pParent),
   mpRawData(NULL)
{
   QLabel* pMaxTempLabel = new QLabel("24 Hour Maximum Temperature", this);
   QLabel* pMinTempLabel = new QLabel("24 Hour Minimum Temperature", this);
   QLabel* pSnowDepthLabel = new QLabel("Snow Depth", this);
   QLabel* pSurfaceWeatherLabel = new QLabel("Surface Weather", this);

   mpMaxTemperature = new QLineEdit(this);
   mpMaxTemperature->setReadOnly(true);

   mpMinTemperature = new QLineEdit(this);
   mpMinTemperature->setReadOnly(true);

   mpSnowDepth = new QLineEdit(this);
   mpSnowDepth->setReadOnly(true);

   mpSurfaceWeather = new QTableWidget(0, 23, this);
   mpSurfaceWeather->setSelectionBehavior(QAbstractItemView::SelectColumns);
   QStringList surfaceWeatherColumnLabels;
   surfaceWeatherColumnLabels <<
      "Date" <<
      "Cloud Base 1" <<
      "Cloud Coverage 1" <<
      "Cloud Thickness 1" <<
      "Cloud Base 2" <<
      "Cloud Coverage 2" <<
      "Cloud Thickness 2" <<
      "Cloud Base 3" <<
      "Cloud Coverage 3" <<
      "Cloud Thickness 3" <<
      "Cloud Base 4" <<
      "Cloud Coverage 4" <<
      "Cloud Thickness 4" <<
      "Total Coverage" <<
      "Visibility" <<
      "Precipitation\nType" <<
      "Obscuration" <<
      "Pressure" <<
      "Temperature" <<
      "Dewpoint" <<
      "Wind Speed" <<
      "Wind Direction" <<
      "Alternate\nWind Speed";
   mpSurfaceWeather->setHorizontalHeaderLabels(surfaceWeatherColumnLabels);

   mpPlotButton = new QPushButton("Plot", this);

   // Layout
   QGridLayout* pTopLayout = new QGridLayout(this);
   pTopLayout->setMargin(sMarginSize);
   pTopLayout->setSpacing(sMarginSize);

   pTopLayout->addWidget(pMaxTempLabel, 0, 0);
   pTopLayout->addWidget(mpMaxTemperature, 0, 1);
   QFontMetrics fontMetrics = mpMaxTemperature->fontMetrics();
   QSize maxTempSize(fontMetrics.width("0000"), mpMaxTemperature->minimumSize().height());
   mpMaxTemperature->setMinimumSize(maxTempSize);

   pTopLayout->addWidget(pMinTempLabel, 1, 0);
   pTopLayout->addWidget(mpMinTemperature, 1, 1);
   fontMetrics = mpMinTemperature->fontMetrics();
   QSize minTempSize(fontMetrics.width("0000"), mpMinTemperature->minimumSize().height());
   mpMinTemperature->setMinimumSize(minTempSize);

   pTopLayout->addWidget(pSnowDepthLabel, 2, 0);
   pTopLayout->addWidget(mpSnowDepth, 2, 1);
   fontMetrics = mpSnowDepth->fontMetrics();
   QSize snowDepthSize(fontMetrics.width("0000"), mpSnowDepth->minimumSize().height());
   mpSnowDepth->setMinimumSize(snowDepthSize);

   pTopLayout->addWidget(pSurfaceWeatherLabel, 3, 0);
   pTopLayout->addWidget(mpSurfaceWeather, 4, 0, 1, 3);
   mpSurfaceWeather->setMinimumSize(mpSurfaceWeather->sizeHint());

   QHBoxLayout* pButtonLayout = new QHBoxLayout;
   pTopLayout->addLayout(pButtonLayout, 5, 0, 1, 2);
   pButtonLayout->addWidget(mpPlotButton, 0);
   pButtonLayout->addStretch(1);

   pTopLayout->setRowStretch(0, 0);
   pTopLayout->setRowStretch(1, 0);
   pTopLayout->setRowStretch(2, 0);
   pTopLayout->setRowStretch(3, 0);
   pTopLayout->setRowStretch(4, 10);
   pTopLayout->setRowStretch(5, 0);
   pTopLayout->setColumnStretch(0, 0);
   pTopLayout->setColumnStretch(1, 0);
   pTopLayout->setColumnStretch(2, 10);

   connect(mpPlotButton, SIGNAL(clicked()), this, SIGNAL(plot()));
   connect(mpSurfaceWeather, SIGNAL(itemSelectionChanged()), this, SIGNAL(plotDataUpdated()));
}

ParagraphJgui::~ParagraphJgui()
{}

QTableWidget* ParagraphJgui::getTable() const
{
   return mpSurfaceWeather;
}

QPushButton* ParagraphJgui::getButton() const
{
   return mpPlotButton;
}

void ParagraphAgui::setData(const Aspam::ParagraphA& value, QTabWidget* pTabWidget)
{
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }

   mpSiteId->setText(QString::fromStdString(StringUtilities::latLonToText(value.mSiteId)));
}

void ParagraphBgui::setData(const Aspam::ParagraphB& value, QTabWidget* pTabWidget)
{
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }
   VERIFYNRV(value.mpDateTime.get() != NULL);
   string strTime;
   strTime = value.mpDateTime->getFormattedUtc("%Y-%m-%dT%H:%M:%SZ");
   mpDateTime->setDateTime(QDateTime::fromString(QString::fromStdString(strTime), Qt::ISODate));
}

void ParagraphDgui::setData(const Aspam::ParagraphD& value, QTabWidget* pTabWidget)
{
   mpRawData = &value;
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }
   mpSurfaceVisibility->setText(QString::number(value.mSurfaceVisibility) + " " + value.mUnits.c_str());
   mpTotalCoverage->setValue(value.mTotalCoverage);
   mpWindDirection->setText(QString::number(value.mWindDirection) + "°");
   mpWindSpeed->setText(QString::number(value.mWindSpeed) + " kt.");
   mpGustSpeed->setText(QString::number(value.mGustSpeed) + " kt.");
   mpRemark->setPlainText(QString::fromLatin1(value.mRemark.c_str()));

   mpClouds->setRowCount(value.mCloudLayers.size());
   mpClouds->setColumnCount(4);
   int row = 0;
   for (vector<Aspam::CloudLayer>::const_iterator cit = value.mCloudLayers.begin();
      cit != value.mCloudLayers.end();
      ++cit, ++row)
   {
      QTableWidgetItem* pItem0 = new QTableWidgetItem(QString::number(cit->mCoverage) + "/8");
      QTableWidgetItem* pItem1 = new QTableWidgetItem(QString::fromLatin1(cit->mType.c_str()));
      QTableWidgetItem* pItem2 = new QTableWidgetItem(QString::number(cit->mBaseAltitude * 100) + " ft AGL");
      QTableWidgetItem* pItem3 = new QTableWidgetItem(QString::number(cit->mTopAltitude * 100) + " ft AGL");

      mpClouds->setItem(row, 0, pItem0);
      mpClouds->setItem(row, 1, pItem1);
      mpClouds->setItem(row, 2, pItem2);
      mpClouds->setItem(row, 3, pItem3);
   }
   mpClouds->resizeColumnsToContents();
}

void ParagraphFgui::setData(const Aspam::ParagraphF& value, QTabWidget* pTabWidget)
{
   mpRawData = &value;
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }

   mpLevel->setText(QString::fromStdString(value.mLevel));
   mpAnalytic->setRowCount(value.mAnalytic.size());
   mpAnalytic->setColumnCount(7);
   int row = 0;
   for (vector<Aspam::Analytic>::const_iterator ait = value.mAnalytic.begin(); ait != value.mAnalytic.end(); ++ait, ++row)
   {
      if (ait->mHeight == 0 && ait->mUnits == ' ')
      {
         QTableWidgetItem* pItem = new QTableWidgetItem("SFC");
         mpAnalytic->setItem(row, 0, pItem);
      }
      else
      {
         QString height("");
         if (ait->mUnits == 'M')
         {
            height = QString::number(ait->mHeight * 1000) + " ft";
         }
         else if (ait->mUnits == 'H')
         {
            height = QString::number(ait->mHeight * 100) + " m";
         }

         QTableWidgetItem* pItem = new QTableWidgetItem(height);
         mpAnalytic->setItem(row, 0, pItem);
      }

      QTableWidgetItem* pItem1 = new QTableWidgetItem(QString::number(ait->mWindDirection) + "°");
      QTableWidgetItem* pItem2 = new QTableWidgetItem(QString::number(ait->mWindSpeed) + " m/s");
      QTableWidgetItem* pItem3 = new QTableWidgetItem(QString::number(ait->mTemperature) + "°C");
      QTableWidgetItem* pItem4 = new QTableWidgetItem(QString::number(ait->mHumidity) + " g/m^3");
      QTableWidgetItem* pItem5 = new QTableWidgetItem(QString::number(ait->mDensity) + " g/cm^3");
      QTableWidgetItem* pItem6 = new QTableWidgetItem(QString::number(ait->mPressure) + " mb");

      mpAnalytic->setItem(row, 1, pItem1);
      mpAnalytic->setItem(row, 2, pItem2);
      mpAnalytic->setItem(row, 3, pItem3);
      mpAnalytic->setItem(row, 4, pItem4);
      mpAnalytic->setItem(row, 5, pItem5);
      mpAnalytic->setItem(row, 6, pItem6);
   }
   mpAnalytic->resizeColumnsToContents();
}

void ParagraphGgui::setData(const Aspam::ParagraphG& value, QTabWidget* pTabWidget)
{
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }
   mpRemarks->setPlainText(QString::fromLatin1(value.mRemarks.c_str()));
   mpRemarks->selectAll();
#if defined(UNIX_API)
   mpRemarks->setCurrentFont(QFont("Lucidatypewriter", 9));
#elif defined(WIN_API)
   mpRemarks->setCurrentFont(QFont("Courier", 9));
#endif // don't need an else #error as the default font is ok...it's just variable width
   mpRemarks->textCursor().clearSelection();
}

void ParagraphHgui::setData(const Aspam::ParagraphH& value, QTabWidget* pTabWidget)
{
   mpRawData = &value;
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }
   mpLevels->setText(QString::number(value.mLevels));
   mpSeasonalDependence->setCurrentIndex(value.mSeasonalDependence - 1);
   mpStratosphericAerosol->setCurrentIndex(value.mStratosphericAerosol - 1);
   mpOzoneProfile->setCurrentIndex(value.mOzoneProfile - 1);
   mpBlpqi->setText(QString::number(value.mBoundaryLayerParameterQualityIndex));

   mpPrimaryBlap->setCurrentIndex(value.mPrimaryBoundaryLayerAerosolParameter - 1);
   if (value.mPrimaryBoundaryLayerAerosolParameter == 3)
   {
      mpAirParcelType->setText(QString::number(value.mAirParcelType));
      mpAirParcelType->setEnabled(true);
   }
   else
   {
      mpAirParcelType->setText("");
      mpAirParcelType->setEnabled(false);
   }
   mpSurfaceVisibility->setText(QString::number(value.mSurfaceVisibility));

   mpAlternateBlap->setCurrentIndex(value.mAlternateBoundaryLayerAerosolParameter - 1);
   if (value.mAlternateBoundaryLayerAerosolParameter == 3)
   {
      mpAlternateAirParcelType->setText(QString::number(value.mAlternateAirParcelType));
      mpAlternateAirParcelType->setEnabled(true);
   }
   else
   {
      mpAlternateAirParcelType->setText("");
      mpAlternateAirParcelType->setEnabled(false);
   }
   mpAlternateSurfaceVisibility->setText(QString::number(value.mAlternateSurfaceVisibility));

   mpAerosol->setRowCount(value.mAerosol.size());
   mpAerosol->setColumnCount(9);
   int row = 0;
   for (vector<Aspam::Aerosol>::const_iterator ait = value.mAerosol.begin(); ait != value.mAerosol.end(); ++ait, ++row)
   {
      QTableWidgetItem* pItem0 = new QTableWidgetItem(QString::number(ait->mHeight / 100.0, 'f', 2) + " km MSL");
      QTableWidgetItem* pItem1 = new QTableWidgetItem(QString::number(ait->mPressure / 10.0) + " mb");
      QTableWidgetItem* pItem2 = new QTableWidgetItem(QString::number(ait->mTemperature / 10.0) + "°C");
      QTableWidgetItem* pItem3 = new QTableWidgetItem(QString::number(ait->mWaterVaporDensity) + " g/m^3");
      QTableWidgetItem* pItem4 = new QTableWidgetItem(QString::number(ait->mAlternateTemperature) + "°C");
      QTableWidgetItem* pItem5 = new QTableWidgetItem(QString::number(ait->mAlternateWaterVaporDensity) + " g/m^3");

      mpAerosol->setItem(row, 0, pItem0);
      mpAerosol->setItem(row, 1, pItem1);
      mpAerosol->setItem(row, 2, pItem2);
      mpAerosol->setItem(row, 3, pItem3);
      mpAerosol->setItem(row, 4, pItem4);
      mpAerosol->setItem(row, 5, pItem5);
      LocationType loc;
      loc.mX = ait->mLatitude / 100.0;
      loc.mY = ait->mLongtitude / 100.0;
      if (loc.mY > 180.0)
      {
         loc.mY = -(loc.mY - 180.0);
      }
      LatLonPoint geo(loc);
      QTableWidgetItem* pItem6 = new QTableWidgetItem(QString::fromLatin1(geo.getLatitudeText().c_str()));
      QTableWidgetItem* pItem7 = new QTableWidgetItem(QString::fromLatin1(geo.getLongitudeText().c_str()));
      QTableWidgetItem* pItem8 = new QTableWidgetItem(QString::number(ait->mOzoneRatio) + " ppm");

      mpAerosol->setItem(row, 6, pItem6);
      mpAerosol->setItem(row, 7, pItem7);
      mpAerosol->setItem(row, 8, pItem8);
   }
   mpAerosol->resizeColumnsToContents();
}

void ParagraphJgui::setData(const Aspam::ParagraphJ& value, QTabWidget* pTabWidget)
{
   mpRawData = &value;
   if (pTabWidget != NULL)
   {
      pTabWidget->setTabEnabled(pTabWidget->indexOf(this), value.mLoaded);
   }
   mpMaxTemperature->setText(QString::number(value.mMaxTemperature)+ "°C");
   mpMinTemperature->setText(QString::number(value.mMinTemperature)+ "°C");
   mpSnowDepth->setText(QString::number(value.mSnowDepth)+ " in");

   mpSurfaceWeather->setRowCount(value.mSurfaceWeather.size());
   mpSurfaceWeather->setColumnCount(23);
   int row = 0;
   for (vector<Aspam::SurfaceWeather>::const_iterator wxit = value.mSurfaceWeather.begin();
      wxit != value.mSurfaceWeather.end();
      ++wxit, ++row)
   {
      unsigned int year = wxit->mYear;
      year += (year > 30) ? 1900 : 2000;
      QDate date = QDate(year, 1, 1).addDays(wxit->mJulianDay - 1);
      QTime time(wxit->mHour, wxit->mMinutes);

      QTableWidgetItem* pItem0 = new QTableWidgetItem(date.toString("yyyy MMM d") + " " + time.toString("hhmm") + "Z");
      QTableWidgetItem* pItem1 = new QTableWidgetItem(QString::number(wxit->mCloudBase1 * 100) + " m AGL");
      QTableWidgetItem* pItem2 = new QTableWidgetItem(QString::number(wxit->mCloudCoverage1 * 10) + "%");
      QTableWidgetItem* pItem3 = new QTableWidgetItem(QString::number(wxit->mCloudThickness1 * 100) + " m");
      QTableWidgetItem* pItem4 = new QTableWidgetItem(QString::number(wxit->mCloudBase2 * 100) + " m AGL");
      QTableWidgetItem* pItem5 = new QTableWidgetItem(QString::number(wxit->mCloudCoverage2 * 10) + "%");
      QTableWidgetItem* pItem6 = new QTableWidgetItem(QString::number(wxit->mCloudThickness2 * 100) + " m");
      QTableWidgetItem* pItem7 = new QTableWidgetItem(QString::number(wxit->mCloudBase3 * 100) + " m AGL");
      QTableWidgetItem* pItem8 = new QTableWidgetItem(QString::number(wxit->mCloudCoverage3 * 10) + "%");
      QTableWidgetItem* pItem9 = new QTableWidgetItem(QString::number(wxit->mCloudThickness3 * 100) + " m");
      QTableWidgetItem* pItem10 = new QTableWidgetItem(QString::number(wxit->mCloudBase4 * 100) + " m AGL");
      QTableWidgetItem* pItem11 = new QTableWidgetItem(QString::number(wxit->mCloudCoverage4 * 10) + "%");
      QTableWidgetItem* pItem12 = new QTableWidgetItem(QString::number(wxit->mCloudThickness4 * 100) + " m");
      QTableWidgetItem* pItem13 = new QTableWidgetItem(QString::number(wxit->mTotalCoverage * 10) + "%");
      QTableWidgetItem* pItem14 = new QTableWidgetItem(QString::number(wxit->mVisibility) + " km");
      QTableWidgetItem* pItem15 = new QTableWidgetItem(QString(" ") + QString::fromStdString(wxit->mPrecipitationType));
      QTableWidgetItem* pItem16 = new QTableWidgetItem(QString(" ") + QString::fromStdString(wxit->mObscuration));
      QTableWidgetItem* pItem17 = new QTableWidgetItem(QString::number(wxit->mPressure / 10.0) + " mb");
      QTableWidgetItem* pItem18 = new QTableWidgetItem(QString::number(wxit->mTemperature) + "°C");
      QTableWidgetItem* pItem19 = new QTableWidgetItem(QString::number(wxit->mDewpoint)+ "°C");
      QTableWidgetItem* pItem20 = new QTableWidgetItem(QString::number(wxit->mWindSpeed) + " m/s");
      QTableWidgetItem* pItem21 = new QTableWidgetItem(QString::number(wxit->mWindDirection * 10) + "°");
      QTableWidgetItem* pItem22 = new QTableWidgetItem(QString::number(wxit->mAlternateWindSpeed) + " m/s");

      mpSurfaceWeather->setItem(row, 0, pItem0);
      mpSurfaceWeather->setItem(row, 1, pItem1);
      mpSurfaceWeather->setItem(row, 2, pItem2);
      mpSurfaceWeather->setItem(row, 3, pItem3);
      mpSurfaceWeather->setItem(row, 4, pItem4);
      mpSurfaceWeather->setItem(row, 5, pItem5);
      mpSurfaceWeather->setItem(row, 6, pItem6);
      mpSurfaceWeather->setItem(row, 7, pItem7);
      mpSurfaceWeather->setItem(row, 8, pItem8);
      mpSurfaceWeather->setItem(row, 9, pItem9);
      mpSurfaceWeather->setItem(row, 10, pItem10);
      mpSurfaceWeather->setItem(row, 11, pItem11);
      mpSurfaceWeather->setItem(row, 12, pItem12);
      mpSurfaceWeather->setItem(row, 13, pItem13);
      mpSurfaceWeather->setItem(row, 14, pItem14);
      mpSurfaceWeather->setItem(row, 15, pItem15);
      mpSurfaceWeather->setItem(row, 16, pItem16);
      mpSurfaceWeather->setItem(row, 17, pItem17);
      mpSurfaceWeather->setItem(row, 18, pItem18);
      mpSurfaceWeather->setItem(row, 19, pItem19);
      mpSurfaceWeather->setItem(row, 20, pItem20);
      mpSurfaceWeather->setItem(row, 21, pItem21);
      mpSurfaceWeather->setItem(row, 22, pItem22);
   }
   mpSurfaceWeather->resizeColumnsToContents();
}

void ParagraphDgui::getPlotData(vector<Point*>& points, PlotView* pView, unsigned int primary, unsigned int secondary)
{
   VERIFYNRV(mpRawData);
   VERIFYNRV(pView);

   points.reserve(mpRawData->mCloudLayers.size());
   for (vector<Aspam::CloudLayer>::const_iterator cit = mpRawData->mCloudLayers.begin();
      cit != mpRawData->mCloudLayers.end();
      ++cit)
   {
      Point* pPoint = static_cast<Point*>(pView->createObject(POINT_OBJECT, false));
      VERIFYNRV(pPoint);
      pPoint->setLocation(getDataPointFromCloudLayer(primary, *cit),
                          getDataPointFromCloudLayer(secondary, *cit));
      points.push_back(pPoint);
   }
}

void ParagraphFgui::getPlotData(vector<Point*>& points, PlotView* pView, unsigned int primary, unsigned int secondary)
{
   VERIFYNRV(mpRawData);
   VERIFYNRV(pView);

   points.reserve(mpRawData->mAnalytic.size());
   for (vector<Aspam::Analytic>::const_iterator ait = mpRawData->mAnalytic.begin();
      ait != mpRawData->mAnalytic.end();
      ++ait)
   {
      Point* pPoint = static_cast<Point*>(pView->createObject(POINT_OBJECT, false));
      VERIFYNRV(pPoint);
      pPoint->setLocation(getDataPointFromAnalytic(primary, *ait),
                          getDataPointFromAnalytic(secondary, *ait));
      points.push_back(pPoint);
   }
}

void ParagraphHgui::getPlotData(vector<Point*>& points, PlotView* pView, unsigned int primary, unsigned int secondary)
{
   VERIFYNRV(mpRawData);
   VERIFYNRV(pView);

   points.reserve(mpRawData->mAerosol.size());
   for (vector<Aspam::Aerosol>::const_iterator ait = mpRawData->mAerosol.begin();
      ait != mpRawData->mAerosol.end();
      ++ait)
   {
      Point* pPoint = static_cast<Point*>(pView->createObject(POINT_OBJECT, false));
      VERIFYNRV(pPoint);
      pPoint->setLocation(getDataPointFromAerosol(primary, *ait),
                          getDataPointFromAerosol(secondary, *ait));
      points.push_back(pPoint);
   }
}

void ParagraphJgui::getPlotData(vector<Point*>& points, PlotView* pView, unsigned int primary, unsigned int secondary)
{
   VERIFYNRV(mpRawData);
   VERIFYNRV(pView);

   points.reserve(mpRawData->mSurfaceWeather.size());
   for (vector<Aspam::SurfaceWeather>::const_iterator wxit = mpRawData->mSurfaceWeather.begin();
      wxit != mpRawData->mSurfaceWeather.end();
      ++wxit)
   {
      Point* pPoint = static_cast<Point*>(pView->createObject(POINT_OBJECT, false));
      VERIFYNRV(pPoint);
      pPoint->setLocation(getDataPointFromSurfaceWeather(primary, *wxit),
                          getDataPointFromSurfaceWeather(secondary, *wxit));
      points.push_back(pPoint);
   }
}
