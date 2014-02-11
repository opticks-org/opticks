/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>

#include "AppConfig.h"
#include "AppVersion.h"
#if defined (CG_SUPPORTED)
#include "CgContext.h"
#endif
#include "ComplexComponentComboBox.h"
#include "CustomColorButton.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "ImageFilterManager.h"
#include "LabeledSection.h"
#include "ModelServices.h"
#include "PointCloudView.h"
#include "PropertiesPointCloudView.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "RasterUtilities.h"
#include "RegionUnitsComboBox.h"
#include "StretchTypeComboBox.h"
#include "Undo.h"
#include "View.h"

#include <limits>
using namespace std;

PropertiesPointCloudView::PropertiesPointCloudView() :
   LabeledSectionGroup(NULL),
   mInitializing(false),
   mpPointCloud(NULL)
{
   // Display configuration
   QWidget* pDisplayWidget = new QWidget(this);

   QLabel* pColorizeByLabel = new QLabel("Colorize By:", pDisplayWidget);
   mpColorizeBy = new QComboBox(pDisplayWidget);
   mpColorizeBy->addItem("Height");
   mpColorizeBy->addItem("Intensity");
   mpColorizeBy->addItem("Classification");
   mColorizeByModifier.attachSignal(mpColorizeBy, SIGNAL(currentIndexChanged(int)));

   QLabel* pZExaggerationLabel = new QLabel("Z Exaggeration:", pDisplayWidget);
   mpZExaggerationSpin = new QDoubleSpinBox(pDisplayWidget);
   mpZExaggerationSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpZExaggerationSpin->setDecimals(6);
   mZExaggerationModifier.attachSignal(mpZExaggerationSpin, SIGNAL(valueChanged(double)));

   QLabel* pDecimationLabel = new QLabel("Decimation:", pDisplayWidget);
   mpDecimationSpin = new QSpinBox(pDisplayWidget);
   mpDecimationSpin->setMinimum(0);
   mpDecimationSpin->setMaximum(numeric_limits<int>::max());
   mpDecimationSpin->setSingleStep(1);
   mDecimationModifier.attachSignal(mpDecimationSpin, SIGNAL(valueChanged(int)));

   QLabel* pPointSizeLabel = new QLabel("Point Size:", pDisplayWidget);
   mpPointSizeSpin = new QDoubleSpinBox(pDisplayWidget);
   mpPointSizeSpin->setMinimum(1.);
   mpPointSizeSpin->setSingleStep(1);
   mpPointSizeSpin->setDecimals(1);
   mPointSizeModifier.attachSignal(mpPointSizeSpin, SIGNAL(valueChanged(double)));

   QLabel* pLowerStretchLabel = new QLabel("Lower Stretch:", pDisplayWidget);
   mpLowerStretchSpin = new QDoubleSpinBox(pDisplayWidget);
   mpLowerStretchSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpLowerStretchSpin->setDecimals(6);

   QLabel* pUpperStretchLabel = new QLabel("Upper Stretch:", pDisplayWidget);
   mpUpperStretchSpin = new QDoubleSpinBox(pDisplayWidget);
   mpUpperStretchSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpUpperStretchSpin->setDecimals(6);
   mStretchModifier.attachSignal(mpLowerStretchSpin, SIGNAL(valueChanged(double)));
   mStretchModifier.attachSignal(mpUpperStretchSpin, SIGNAL(valueChanged(double)));

   QLabel* pLowerColorLabel = new QLabel("Lower Color:", pDisplayWidget);
   mpLowerColorButton = new CustomColorButton(pDisplayWidget);
   mpLowerColorButton->usePopupGrid(true);

   QLabel* pUpperColorLabel = new QLabel("Upper Color:", pDisplayWidget);
   mpUpperColorButton = new CustomColorButton(pDisplayWidget);
   mpUpperColorButton->usePopupGrid(true);
   mColorModifier.attachSignal(mpLowerColorButton, SIGNAL(colorChanged(const QColor&)));
   mColorModifier.attachSignal(mpUpperColorButton, SIGNAL(colorChanged(const QColor&)));

   QLabel* pUseColorMapLabel = new QLabel("Use Color Map:", pDisplayWidget);
   mpUseColorMap = new QCheckBox(pDisplayWidget);
   mUseColorMapModifier.attachSignal(mpUseColorMap, SIGNAL(stateChanged(int)));

   QLabel* pColorMapLabel = new QLabel("Color Map:", pDisplayWidget);
   mpColorMapList = new QListWidget(pDisplayWidget);
   mpColorMapList->setSelectionMode(QAbstractItemView::SingleSelection);

   LabeledSection* pDisplaySection = new LabeledSection(pDisplayWidget, "Display Configuration", this);

   QFormLayout* pDisplayGrid = new QFormLayout(pDisplayWidget);
   pDisplayGrid->addRow(pColorizeByLabel, mpColorizeBy);
   pDisplayGrid->addRow(pZExaggerationLabel, mpZExaggerationSpin);
   pDisplayGrid->addRow(pDecimationLabel, mpDecimationSpin);
   pDisplayGrid->addRow(pPointSizeLabel, mpPointSizeSpin);
   pDisplayGrid->addRow(pLowerStretchLabel, mpLowerStretchSpin);
   pDisplayGrid->addRow(pUpperStretchLabel, mpUpperStretchSpin);
   pDisplayGrid->addRow(pLowerColorLabel, mpLowerColorButton);
   pDisplayGrid->addRow(pUpperColorLabel, mpUpperColorButton);
   pDisplayGrid->addRow(pUseColorMapLabel, mpUseColorMap);
   pDisplayGrid->addRow(pColorMapLabel, mpColorMapList);

   // Initialization
   addSection(pDisplaySection);
   setSizeHint(600, 610);
}

PropertiesPointCloudView::~PropertiesPointCloudView()
{}

bool PropertiesPointCloudView::initialize(SessionItem* pSessionItem)
{
   mpPointCloud = dynamic_cast<PointCloudView*>(pSessionItem);
   if (mpPointCloud == NULL)
   {
      return false;
   }

   mInitializing = true;

   PointColorizationType type = mpPointCloud->getPointColorizationType();
   if (type == POINT_HEIGHT)
   {
      mpColorizeBy->setCurrentIndex(0);
   }
   else if (type == POINT_INTENSITY)
   {
      mpColorizeBy->setCurrentIndex(1);
   }
   else if (type == POINT_CLASSIFICATION)
   {
      mpColorizeBy->setCurrentIndex(2);
   }
   mColorizeByModifier.setModified(false);

   mpZExaggerationSpin->setValue(mpPointCloud->getZExaggeration());
   mZExaggerationModifier.setModified(false);
   mpDecimationSpin->setValue(mpPointCloud->getDecimation());
   mDecimationModifier.setModified(false);
   mpPointSizeSpin->setValue(mpPointCloud->getPointSize());
   mPointSizeModifier.setModified(false);
   mpLowerStretchSpin->setValue(mpPointCloud->getLowerStretch());
   mpUpperStretchSpin->setValue(mpPointCloud->getUpperStretch());
   mStretchModifier.setModified(false);

   mpLowerColorButton->setColor(mpPointCloud->getLowerStretchColor());
   mpUpperColorButton->setColor(mpPointCloud->getUpperStretchColor());
   mColorModifier.setModified(false);

   mpUseColorMap->setChecked(mpPointCloud->isUsingColorMap());
   mUseColorMapModifier.setModified(false);

   mpColorMapList->clear();
   mPreloadedColorMaps.clear();
   ColorMap defaultMap;

   const string& defaultName = defaultMap.getName();
   if (defaultName.empty() == false)
   {
      mpColorMapList->addItem(QString::fromStdString(defaultName));
   }

   QString colorMapDir;
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      colorMapDir = QString::fromStdString(pSupportFilesPath->getFullPathAndName() + SLASH + "ColorTables");
   }

   QDir dir(colorMapDir, "*.clu *.cgr");
   QString strDirectory = dir.absolutePath() + "/";

   for (unsigned int i = 0; i < dir.count(); ++i)
   {
      QString strFilename = strDirectory + dir[i];
      if (strFilename.isEmpty() == false)
      {
         string filename = strFilename.toStdString();
         try
         {
            ColorMap cmap(filename);
            mPreloadedColorMaps.insert(pair<string, string>(cmap.getName(), filename));
            mpColorMapList->addItem(QString::fromStdString(cmap.getName()));
         }
         catch (...) // bad color map file
         {
         }
      }
   }


   mInitializing = false;
   return true;
}

bool PropertiesPointCloudView::applyChanges()
{
   if (mpPointCloud == NULL)
   {
      return false;
   }

   if (mColorizeByModifier.isModified())
   {
      PointColorizationType type = POINT_HEIGHT;
      if (mpColorizeBy->currentIndex() == 1)
      {
         type = POINT_INTENSITY;
      }
      else if (mpColorizeBy->currentIndex() == 2)
      {
         type = POINT_CLASSIFICATION;
      }
      mpPointCloud->setPointColorizationType(type);
   }

   if (mZExaggerationModifier.isModified())
   {
      mpPointCloud->setZExaggeration(mpZExaggerationSpin->value());
   }

   if (mDecimationModifier.isModified())
   {
      mpPointCloud->setDecimation(mpDecimationSpin->value());
   }

   if (mPointSizeModifier.isModified())
   {
      mpPointCloud->setPointSize(mpPointSizeSpin->value());
   }

   if (mStretchModifier.isModified())
   {
      mpPointCloud->setLowerStretch(mpLowerStretchSpin->value());
      mpPointCloud->setUpperStretch(mpUpperStretchSpin->value());
   }

   if (mColorModifier.isModified())
   {
      mpPointCloud->setLowerStretchColor(mpLowerColorButton->getColorType());
      mpPointCloud->setUpperStretchColor(mpUpperColorButton->getColorType());
   }

   if (mUseColorMapModifier.isModified())
   {
      mpPointCloud->setUsingColorMap(mpUseColorMap->isChecked());
   }

   QList<QListWidgetItem*> selectedItems = mpColorMapList->selectedItems();
   if (!selectedItems.empty())
   {
      QListWidgetItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         map<string, string>::iterator iter = mPreloadedColorMaps.find(pItem->text().toStdString());
         if (iter != mPreloadedColorMaps.end())
         {
            mpPointCloud->setColorMap(ColorMap(iter->second));
         }
      }
   }
   mpColorMapList->clearSelection();

   // Refresh the view
   mpPointCloud->refresh();

   return true;
}

const string& PropertiesPointCloudView::getName()
{
   static string name = "Point Cloud View Properties";
   return name;
}

const string& PropertiesPointCloudView::getPropertiesName()
{
   static string propertiesName = "Point Cloud View";
   return propertiesName;
}

const string& PropertiesPointCloudView::getDescription()
{
   static string description = "General setting properties of a point cloud view";
   return description;
}

const string& PropertiesPointCloudView::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesPointCloudView::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesPointCloudView::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesPointCloudView::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesPointCloudView::getDescriptorId()
{
   static string id = "{EF3E6B26-80CB-494B-A6F3-62C8F25B65F4}";
   return id;
}

bool PropertiesPointCloudView::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
