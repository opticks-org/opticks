/*
* The information in this file is
* Copyright(c) 2008 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AddFeatureDlg.h"
#include "AddFieldDlg.h"
#include "AoiElement.h"
#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "CustomTreeWidget.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "Feature.h"
#include "FeatureClassDlg.h"
#include "GraphicGroup.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "ShapeFile.h"
#include "ShapeFileOptionsWidget.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"

#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QIntValidator>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QTreeWidgetItem>

#include <algorithm>
#include <string>
using namespace std;

ShapeFileOptionsWidget::ShapeFileOptionsWidget(ShapeFile* pShapefile, AoiElement* pDefaultAoi,
                                               const vector<AoiElement*>& aois, RasterElement* pRaster,
                                               QWidget* pParent) :
   QWidget(pParent),
   mpShapeFile(pShapefile),
   mpDefaultAoi(pDefaultAoi),
   mAois(aois),
   mpGeoref(pRaster)
{
   // Filenames
   QLabel* pFilePathLabel = new QLabel("File Path:", this);
   QLabel* pBaseNameLabel = new QLabel("Base Name:", this);
   QLabel* pShpLabel = new QLabel("SHP:", this);
   QLabel* pShxLabel = new QLabel("SHX:", this);
   QLabel* pDbfLabel = new QLabel("DBF:", this);

   QFont ftBold = QApplication::font();
   ftBold.setBold(true);

   pFilePathLabel->setFont(ftBold);
   pBaseNameLabel->setFont(ftBold);
   pShpLabel->setFont(ftBold);
   pShxLabel->setFont(ftBold);
   pDbfLabel->setFont(ftBold);

   mpFilePathEdit = new QLineEdit(this);
   mpFilePathEdit->setReadOnly(true);

   mpBaseNameEdit = new QLineEdit(this);
   mpBaseNameEdit->setFixedWidth(150);

   mpShpFileLabel = new QLabel(this);
   mpShxFileLabel = new QLabel(this);
   mpDbfFileLabel = new QLabel(this);

   // Browse button
   QIcon icnBrowse(":/icons/Open");
   QPushButton* pBrowseButton = new QPushButton(icnBrowse, QString(), this);
   pBrowseButton->setFixedWidth(27);

   // Shape type
   QLabel* pShapeLabel = new QLabel("Shape:", this);
   pShapeLabel->setFont(ftBold);

   mpShapeCombo = new QComboBox(this);
   mpShapeCombo->setEditable(false);
   mpShapeCombo->setFixedWidth(150);
   vector<string> comboText = StringUtilities::getAllEnumValuesAsDisplayString<ShapefileTypes::ShapeType>();
   for (vector<string>::iterator it = comboText.begin(); it != comboText.end(); ++it)
   {
      mpShapeCombo->addItem(QString::fromStdString(*it));
   }

   // Feature list
   QLabel* pFeatureLabel = new QLabel("Features:", this);

   QStringList columnNames;
   columnNames.append("Feature");

   mpFeatureTree = new CustomTreeWidget(this);
   mpFeatureTree->setColumnCount(columnNames.count());
   mpFeatureTree->setHeaderLabels(columnNames);
   mpFeatureTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpFeatureTree->setRootIsDecorated(false);
   mpFeatureTree->setAllColumnsShowFocus(true);
   mpFeatureTree->setSortingEnabled(true);
   mpFeatureTree->setContextMenuPolicy(Qt::CustomContextMenu);
   mpFeatureTree->sortByColumn(0, Qt::AscendingOrder);

   QHeaderView* pHeader = mpFeatureTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   mpIntEdit = new QLineEdit(mpFeatureTree);
   mpIntEdit->setValidator(new QIntValidator(mpIntEdit));
   mpIntEdit->hide();

   mpDoubleEdit = new QLineEdit(mpFeatureTree);
   mpDoubleEdit->setValidator(new QDoubleValidator(mpDoubleEdit));
   mpDoubleEdit->hide();

   // Feature buttons
   QToolButton* pAddFeatureButton = new QToolButton(this);
   pAddFeatureButton->setAutoRaise(true);
   pAddFeatureButton->setIcon(QIcon(":/icons/New"));
   pAddFeatureButton->setToolTip("Add Features");

   QToolButton* pRemoveFeatureButton = new QToolButton(this);
   pRemoveFeatureButton->setAutoRaise(true);
   pRemoveFeatureButton->setIcon(QIcon(":/icons/Delete"));
   pRemoveFeatureButton->setToolTip("Remove Features");

   QFrame* pSeparator = new QFrame(this);
   pSeparator->setFrameStyle(QFrame::VLine);

   QPalette separatorPalette = pSeparator->palette();
   separatorPalette.setColor(QPalette::WindowText, Qt::lightGray);
   pSeparator->setPalette(separatorPalette);

   mpFeatureDisplayModeGroup = new QActionGroup(this);
   mpFeatureDisplayModeGroup->setExclusive(true);

   QAction* pFeatureModeOffAction = mpFeatureDisplayModeGroup->addAction(QIcon(":/icons/Edit"),
      "Feature Display Mode Off");
   pFeatureModeOffAction->setAutoRepeat(false);
   pFeatureModeOffAction->setCheckable(true);
   pFeatureModeOffAction->setChecked(true);
   pFeatureModeOffAction->setWhatsThis("When activated, selecting a feature item does not change the "
      "displayed area in the view.");

   mpFeatureZoomAction = mpFeatureDisplayModeGroup->addAction(QIcon(":/icons/Zoom"), "Feature Zoom Mode");
   mpFeatureZoomAction->setAutoRepeat(false);
   mpFeatureZoomAction->setCheckable(true);
   mpFeatureZoomAction->setWhatsThis("When activated, selecting a feature item pans the view such that the "
      "object representing the feature is centered.  The zoom percentage is also changed such that the object "
      "fills most of the view area.");

   mpFeaturePanAction = mpFeatureDisplayModeGroup->addAction(QIcon(":/icons/Pan"), "Feature Pan Mode");
   mpFeaturePanAction->setAutoRepeat(false);
   mpFeaturePanAction->setCheckable(true);
   mpFeaturePanAction->setWhatsThis("When activated, selecting a feature item pans the view such that the "
      "object representing the feature is centered, but zoom percentage is not changed.");

   QToolButton* pFeatureModeOffButton = new QToolButton(this);
   pFeatureModeOffButton->setAutoRaise(true);
   pFeatureModeOffButton->setDefaultAction(pFeatureModeOffAction);

   QToolButton* pFeatureZoomButton = new QToolButton(this);
   pFeatureZoomButton->setAutoRaise(true);
   pFeatureZoomButton->setDefaultAction(mpFeatureZoomAction);

   QToolButton* pFeaturePanButton = new QToolButton(this);
   pFeaturePanButton->setAutoRaise(true);
   pFeaturePanButton->setDefaultAction(mpFeaturePanAction);

   QFrame* pSeparator2 = new QFrame(this);
   pSeparator2->setFrameStyle(QFrame::VLine);
   pSeparator2->setPalette(separatorPalette);

   mpSelectFeatureButton = new QToolButton(this);
   mpSelectFeatureButton->setAutoRaise(true);
   mpSelectFeatureButton->setCheckable(true);
   mpSelectFeatureButton->setIcon(QIcon(":/icons/SelectObject"));
   mpSelectFeatureButton->setToolTip("Select Feature");
   mpSelectFeatureButton->setWhatsThis("When activated, selecting a feature item also selects the object in the "
      "view representing the feature.  All other objects are deselected.");

   // Layout
   QHBoxLayout* pFilePathLayout = new QHBoxLayout();
   pFilePathLayout->setMargin(0);
   pFilePathLayout->setSpacing(5);
   pFilePathLayout->addWidget(mpFilePathEdit, 10);
   pFilePathLayout->addWidget(pBrowseButton);

   QHBoxLayout* pNameShapeLayout = new QHBoxLayout();
   pNameShapeLayout->setMargin(0);
   pNameShapeLayout->setSpacing(5);
   pNameShapeLayout->addWidget(mpBaseNameEdit);
   pNameShapeLayout->addSpacing(20);
   pNameShapeLayout->addWidget(pShapeLabel);
   pNameShapeLayout->addWidget(mpShapeCombo);
   pNameShapeLayout->addStretch();

   QGridLayout* pFeatureLayout = new QGridLayout();
   pFeatureLayout->setMargin(0);
   pFeatureLayout->setSpacing(0);
   pFeatureLayout->addWidget(pFeatureLabel, 0, 0);
   pFeatureLayout->addWidget(pAddFeatureButton, 0, 2);
   pFeatureLayout->addWidget(pRemoveFeatureButton, 0, 3);
   pFeatureLayout->addWidget(pSeparator, 0, 4, Qt::AlignHCenter);
   pFeatureLayout->addWidget(pFeatureModeOffButton, 0, 5);
   pFeatureLayout->addWidget(pFeatureZoomButton, 0, 6);
   pFeatureLayout->addWidget(pFeaturePanButton, 0, 7);
   pFeatureLayout->addWidget(pSeparator2, 0, 8, Qt::AlignHCenter);
   pFeatureLayout->addWidget(mpSelectFeatureButton, 0, 9);
   pFeatureLayout->addWidget(mpFeatureTree, 2, 0, 1, 10);
   pFeatureLayout->setRowMinimumHeight(1, 2);
   pFeatureLayout->setColumnMinimumWidth(1, 10);
   pFeatureLayout->setColumnMinimumWidth(4, 7);
   pFeatureLayout->setColumnMinimumWidth(8, 7);
   pFeatureLayout->setRowStretch(2, 10);
   pFeatureLayout->setColumnStretch(0, 10);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pFilePathLabel, 0, 0);
   pGrid->addLayout(pFilePathLayout, 0, 1);
   pGrid->addWidget(pBaseNameLabel, 1, 0);
   pGrid->addLayout(pNameShapeLayout, 1, 1);
   pGrid->addWidget(pShpLabel, 2, 0);
   pGrid->addWidget(mpShpFileLabel, 2, 1);
   pGrid->addWidget(pShxLabel, 3, 0);
   pGrid->addWidget(mpShxFileLabel, 3, 1);
   pGrid->addWidget(pDbfLabel, 4, 0);
   pGrid->addWidget(mpDbfFileLabel, 4, 1);
   pGrid->setRowMinimumHeight(5, 12);
   pGrid->addLayout(pFeatureLayout, 6, 0, 1, 2);
   pGrid->setRowStretch(6, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Shape File");

   if (mpShapeFile != NULL)
   {
      // Filename
      const string& filename = mpShapeFile->getFilename();
      if (!filename.empty())
      {
         QFileInfo fileInfo(QString::fromStdString(filename));
         mpFilePathEdit->setText(fileInfo.absolutePath());
         mpBaseNameEdit->setText(fileInfo.completeBaseName());
      }

      updateFilenames();

      // Shape
      int index = ShapefileTypes::getIndex(mpShapeFile->getShape());
      if (index < 0)
      {
         index = 0;
      }
      mpShapeCombo->setCurrentIndex(index);

      // Features
      const vector<Feature*>& features = mpShapeFile->getFeatures();
      for (unsigned int i = 0; i < features.size(); i++)
      {
         Feature* pFeature = features[i];
         if (pFeature != NULL)
         {
            QTreeWidgetItem* pItem = new QTreeWidgetItem(mpFeatureTree);
            if (pItem != NULL)
            {
               pItem->setText(0, QString::number(i + 1));
               mFeatures.insert(pItem, pFeature);
            }
         }
      }

      // Fields
      updateFieldValues();
   }

   // Connections
   VERIFYNR(connect(mpFilePathEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateFilenames())));
   VERIFYNR(connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browse())));
   VERIFYNR(connect(mpBaseNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateFilenames())));
   VERIFYNR(connect(mpShapeCombo, SIGNAL(activated(const QString&)), this, SLOT(setShape(const QString&))));
   VERIFYNR(connect(pAddFeatureButton, SIGNAL(clicked()), this, SLOT(addFeature())));
   VERIFYNR(connect(pRemoveFeatureButton, SIGNAL(clicked()), this, SLOT(removeFeature())));
   VERIFYNR(connect(mpFeatureDisplayModeGroup, SIGNAL(triggered(QAction*)), this,
      SLOT(featureDisplayModeChanged(QAction*))));
   VERIFYNR(connect(mpSelectFeatureButton, SIGNAL(toggled(bool)), this, SLOT(selectFeature(bool))));
   VERIFYNR(connect(mpFeatureTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this,
      SLOT(setFieldValue(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpFeatureTree, SIGNAL(customContextMenuRequested(const QPoint&)), this,
      SLOT(displayFeatureContextMenu(const QPoint&))));
   VERIFYNR(connect(mpFeatureTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
      SLOT(currentFeatureChanged(QTreeWidgetItem*))));
}

ShapeFileOptionsWidget::~ShapeFileOptionsWidget()
{}

int ShapeFileOptionsWidget::getColumn(const QString& strField) const
{
   if (strField.isEmpty())
   {
      return -1;
   }

   QTreeWidgetItem* pHeaderItem = mpFeatureTree->headerItem();
   if (pHeaderItem != NULL)
   {
      for (int i = 0; i < pHeaderItem->columnCount(); i++)
      {
         QString strColumn = pHeaderItem->text(i);
         if (strColumn == strField)
         {
            return i;
         }
      }
   }

   return -1;
}

vector<Feature*> ShapeFileOptionsWidget::addFeatures(AoiElement* pAoi, GraphicObject* pObject, QString& message)
{
   if ((pAoi == NULL) || (mpShapeFile == NULL))
   {
      return vector<Feature*>();
   }

   QTreeWidgetItem* pHeaderItem = mpFeatureTree->headerItem();
   string msg;

   vector<Feature*> features = mpShapeFile->addFeatures(pAoi, pObject, mpGeoref, msg);
   for (vector<Feature*>::const_iterator iter = features.begin(); iter != features.end(); ++iter)
   {
      Feature* pFeature = *iter;
      if (pFeature != NULL)
      {
         QTreeWidgetItem* pItem = new QTreeWidgetItem(mpFeatureTree);
         pItem->setText(0, QString::number(mpFeatureTree->topLevelItemCount()));

         int numColumns = mpFeatureTree->columnCount();
         for (int i = 1; i < numColumns; ++i)   // Start with the second column to prevent changing the feature number
         {
            QLineEdit* pLineEdit = NULL;
            if (pHeaderItem != NULL)
            {
               QString fieldName = pHeaderItem->text(i);

               string fieldType = pFeature->getFieldType(fieldName.toStdString());
               if (fieldType == "int")
               {
                  pLineEdit = mpIntEdit;
               }
               else if (fieldType == "double")
               {
                  pLineEdit = mpDoubleEdit;
               }
            }

            if (pLineEdit != NULL)
            {
               mpFeatureTree->setCellWidgetType(pItem, i, CustomTreeWidget::CUSTOM_LINE_EDIT);
               mpFeatureTree->setCustomLineEdit(pItem, i, pLineEdit);
            }
            else
            {
               mpFeatureTree->setCellWidgetType(pItem, i, CustomTreeWidget::LINE_EDIT);
            }
         }

         mFeatures.insert(pItem, pFeature);
      }
   }

   message = QString::fromStdString(msg);
   return features;
}

void ShapeFileOptionsWidget::applyFeatureClass(const string& className)
{
   if (className.empty() == true)
   {
      return;
   }

   Service<ConfigurationSettings> pSettings;
   const DataVariant& featureClasses = pSettings->getSetting("ShapeFileExporter/FeatureClasses");

   const DynamicObject* pFeatureClasses = featureClasses.getPointerToValue<DynamicObject>();
   if (pFeatureClasses == NULL)
   {
      return;
   }

   const DataVariant& featureClass = pFeatureClasses->getAttribute(className);

   const DynamicObject* pFeatureClass = featureClass.getPointerToValue<DynamicObject>();
   if (pFeatureClass == NULL)
   {
      return;
   }

   vector<string> fieldNames;
   pFeatureClass->getAttributeNames(fieldNames);
   for (vector<string>::const_iterator fieldIter = fieldNames.begin(); fieldIter != fieldNames.end(); ++fieldIter)
   {
      string fieldName = *fieldIter;
      if (fieldName.empty() == false)
      {
         const DataVariant& fieldValue = pFeatureClass->getAttribute(fieldName);
         addField(QString::fromStdString(fieldName), QString::fromStdString(fieldValue.getTypeName()));

         // Initialize the field values to the default value in the feature class
         const vector<Feature*>& features = mpShapeFile->getFeatures();
         for (vector<Feature*>::const_iterator iter = features.begin(); iter != features.end(); ++iter)
         {
            Feature* pFeature = *iter;
            if (pFeature != NULL)
            {
               pFeature->setFieldValue(fieldName, fieldValue);
            }
         }
      }
   }

   updateFieldValues();
}

void ShapeFileOptionsWidget::updateFilenames()
{
   QString strDirectory = mpFilePathEdit->text();
   QString strBaseName = mpBaseNameEdit->text();

   if (!strDirectory.isEmpty() && !strBaseName.isEmpty())
   {
      QString strFilename = strDirectory + "/" + strBaseName;
      if (!strFilename.isEmpty())
      {
         strFilename.replace(QRegExp("\\\\"), "/");

         mpShpFileLabel->setText(strFilename + ".shp");
         mpShxFileLabel->setText(strFilename + ".shx");
         mpDbfFileLabel->setText(strFilename + ".dbf");
         if (mpShapeFile != NULL)
         {
            string filename = strFilename.toStdString();
            mpShapeFile->setFilename(filename);
         }
      }
   }
}

void ShapeFileOptionsWidget::browse()
{
   QString strCurrentDir = mpFilePathEdit->text();

   QString strDirectory = QFileDialog::getExistingDirectory(this, "Select Shape File Directory", strCurrentDir);
   if (!strDirectory.isEmpty())
   {
      strDirectory.replace(QRegExp("\\\\"), "/");
      mpFilePathEdit->setText(strDirectory);
   }
}

void ShapeFileOptionsWidget::setShape(const QString& strShape)
{
   if (mpShapeFile == NULL)
   {
      return;
   }

   ShapefileTypes::ShapeType newShape =
      StringUtilities::fromDisplayString<ShapefileTypes::ShapeType>(strShape.toStdString());
   ShapefileTypes::ShapeType currentShape = mpShapeFile->getShape();
   if (newShape == currentShape)
   {
      return;
   }

   int button = QMessageBox::question(this, windowTitle(), "Changing the output shape type will clear all existing "
      "features.  Would you like to replace the features with the default features contained in the AOI layer, "
      "which could take some time based on the number of selected pixels in the AOI?", "Clear", "Replace", "Cancel");
   if (button == 2)  // Cancel
   {
      int index = ShapefileTypes::getIndex(currentShape);
      index = (index < 0 ? 0 : index);
      mpShapeCombo->setCurrentIndex(index);
      return;
   }

   QApplication::setOverrideCursor(Qt::WaitCursor);
   clearFeatures();
   mpShapeFile->setShape(newShape);

   if (button == 1)  // Replace
   {
      QString message;

      vector<Feature*> features = addFeatures(mpDefaultAoi, NULL, message);
      if (features.empty() == true)
      {
         QApplication::restoreOverrideCursor();
         if (message.isEmpty() == false)
         {
            QMessageBox::warning(this, windowTitle(), message);
         }

         return;
      }

      updateFieldValues();
   }

   QApplication::restoreOverrideCursor();
}

void ShapeFileOptionsWidget::displayFeatureContextMenu(const QPoint& pos)
{
   QMenu contextMenu(mpFeatureTree);
   contextMenu.addAction(QIcon(":/icons/New"), "Add Features...", this, SLOT(addFeature()));

   QList<QTreeWidgetItem*> items = mpFeatureTree->selectedItems();
   if (items.count() == 1)
   {
      contextMenu.addAction(QIcon(":/icons/Delete"), "Remove Feature", this, SLOT(removeFeature()));
   }
   else if (items.empty() == false)
   {
      contextMenu.addAction(QIcon(":/icons/Delete"), "Remove Features", this, SLOT(removeFeature()));
   }

   if (mpFeatureTree->topLevelItemCount() > 0)
   {
      contextMenu.addAction("Clear Features", this, SLOT(clearFeatures()));
   }

   contextMenu.addSeparator();
   contextMenu.addAction("Add Field...", this, SLOT(addField()));
   if (mpFeatureTree->columnCount() > 1)
   {
      contextMenu.addAction("Remove Field...", this, SLOT(removeField()));
   }

   contextMenu.addSeparator();
   contextMenu.addAction("Apply Feature Class...", this, SLOT(applyFeatureClass()));
   contextMenu.addAction("Edit Feature Classes...", this, SLOT(editFeatureClasses()));
   contextMenu.exec(mpFeatureTree->viewport()->mapToGlobal(pos));
}

void ShapeFileOptionsWidget::featureDisplayModeChanged(QAction* pAction)
{
   QTreeWidgetItem* pItem = mpFeatureTree->currentItem();
   if (pItem != NULL)
   {
      if (pAction == mpFeatureZoomAction)
      {
         zoomToFeature(pItem);
      }
      else if (pAction == mpFeaturePanAction)
      {
         panToFeature(pItem);
      }
   }
}

void ShapeFileOptionsWidget::currentFeatureChanged(QTreeWidgetItem* pItem)
{
   QAction* pAction = mpFeatureDisplayModeGroup->checkedAction();
   if (pAction == mpFeatureZoomAction)
   {
      zoomToFeature(pItem);
   }
   else if (pAction == mpFeaturePanAction)
   {
      panToFeature(pItem);
   }

   if (mpSelectFeatureButton->isChecked() == true)
   {
      selectFeature(pItem, true);
   }
}

void ShapeFileOptionsWidget::zoomToFeature(QTreeWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   SpatialDataView* pView = NULL;
   vector<LocationType> featureExtents;
   double marginFactor = 0.5;

   Feature* pFeature = mFeatures.value(pItem, NULL);
   if (pFeature != NULL)
   {
      GraphicObject* pObject = dynamic_cast<GraphicObject*>(pFeature->getSessionItem());
      if (pObject != NULL)
      {
         pObject->getRotatedExtents(featureExtents);
         if (featureExtents.empty() == false)
         {
            GraphicLayer* pLayer = pObject->getLayer();
            if (pLayer != NULL)
            {
               // Get the view
               pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
               marginFactor = 1.25;
            }
         }
      }

      AoiElement* pAoi = dynamic_cast<AoiElement*>(pFeature->getSessionItem());
      if (pAoi != NULL)
      {
         Service<DesktopServices> pDesktop;

         vector<Window*> windows;
         pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
         for (vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
         {
            SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(*iter);
            if (pWindow != NULL)
            {
               SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
               if (pCurrentView != NULL)
               {
                  LayerList* pLayerList = pCurrentView->getLayerList();
                  if (pLayerList != NULL)
                  {
                     vector<Layer*> layers = pLayerList->getLayers(pAoi);
                     if (layers.empty() == false)
                     {
                        GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(layers.front());
                        if (pLayer != NULL)
                        {
                           pLayer->getExtents(featureExtents);
                           pView = pCurrentView;
                           marginFactor = 0.75;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   // Zoom to the selected feature and zoom out to see the surrounding pixels
   if ((pView != NULL) && (featureExtents.empty() == false))
   {
      pView->zoomToArea(featureExtents);

      LocationType lowerLeft;
      LocationType upperLeft;
      LocationType upperRight;
      LocationType lowerRight;
      pView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);

      double visibleWidth = fabs(upperRight.mX - lowerLeft.mX) * marginFactor;
      double visibleHeight = fabs(upperRight.mY - lowerLeft.mY) * marginFactor;
      LocationType center = pView->getVisibleCenter();
      LocationType newLowerLeft(center.mX - visibleWidth, center.mY - visibleHeight);
      LocationType newUpperRight(center.mX + visibleWidth, center.mY + visibleHeight);
      pView->zoomToBox(newLowerLeft, newUpperRight);

      pView->refresh();
   }
}

void ShapeFileOptionsWidget::panToFeature(QTreeWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   SpatialDataView* pView = NULL;
   LocationType center;

   Feature* pFeature = mFeatures.value(pItem, NULL);
   if (pFeature != NULL)
   {
      GraphicObject* pObject = dynamic_cast<GraphicObject*>(pFeature->getSessionItem());
      if (pObject != NULL)
      {
         GraphicLayer* pLayer = pObject->getLayer();
         if (pLayer != NULL)
         {
            // Center
            LocationType lowerLeft = pObject->getLlCorner();
            LocationType upperRight = pObject->getUrCorner();

            center.mX = (lowerLeft.mX + upperRight.mX) / 2.0;
            center.mY = (lowerLeft.mY + upperRight.mY) / 2.0;

            // View
            pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
         }
      }

      AoiElement* pAoi = dynamic_cast<AoiElement*>(pFeature->getSessionItem());
      if (pAoi != NULL)
      {
         Service<DesktopServices> pDesktop;

         vector<Window*> windows;
         pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
         for (vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
         {
            SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(*iter);
            if (pWindow != NULL)
            {
               SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
               if (pCurrentView != NULL)
               {
                  LayerList* pLayerList = pCurrentView->getLayerList();
                  if (pLayerList != NULL)
                  {
                     vector<Layer*> layers = pLayerList->getLayers(pAoi);
                     if (layers.empty() == false)
                     {
                        GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(layers.front());
                        if (pLayer != NULL)
                        {
                           GraphicGroup* pGroup = pAoi->getGroup();
                           if (pGroup != NULL)
                           {
                              // Center
                              LocationType lowerLeft = pGroup->getLlCorner();
                              LocationType upperRight = pGroup->getUrCorner();

                              center.mX = (lowerLeft.mX + upperRight.mX) / 2.0;
                              center.mY = (lowerLeft.mY + upperRight.mY) / 2.0;

                              // View
                              pView = pCurrentView;
                              break;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   // Pan to the selected feature
   if (pView != NULL)
   {
      pView->panTo(center);
      pView->refresh();
   }
}

void ShapeFileOptionsWidget::selectFeature(QTreeWidgetItem* pItem, bool select)
{
   if (pItem == NULL)
   {
      return;
   }

   Feature* pFeature = mFeatures.value(pItem, NULL);
   if (pFeature != NULL)
   {
      GraphicObject* pObject = dynamic_cast<GraphicObject*>(pFeature->getSessionItem());
      if (pObject != NULL)
      {
         GraphicLayer* pLayer = pObject->getLayer();
         if (pLayer != NULL)
         {
            pLayer->deselectAllObjects();
            if (select == true)
            {
               pLayer->selectObject(pObject);
            }
         }
      }

      AoiElement* pAoi = dynamic_cast<AoiElement*>(pFeature->getSessionItem());
      if (pAoi != NULL)
      {
         Service<DesktopServices> pDesktop;

         vector<Window*> windows;
         pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
         for (vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
         {
            SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(*iter);
            if (pWindow != NULL)
            {
               SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
               if (pCurrentView != NULL)
               {
                  LayerList* pLayerList = pCurrentView->getLayerList();
                  if (pLayerList != NULL)
                  {
                     vector<Layer*> layers = pLayerList->getLayers(pAoi);
                     if (layers.empty() == false)
                     {
                        GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(layers.front());
                        if (pLayer != NULL)
                        {
                           if (select == true)
                           {
                              pLayer->selectAllObjects();
                           }
                           else
                           {
                              pLayer->deselectAllObjects();
                           }

                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

void ShapeFileOptionsWidget::addFeature()
{
   if (mpShapeFile == NULL)
   {
      return;
   }

   AddFeatureDlg dlg(mAois, mpShapeFile->getShape(), this);
   if (dlg.exec() == QDialog::Rejected)
   {
      return;
   }

   map<AoiElement*, vector<GraphicObject*> > featureItems = dlg.getFeatureItems();
   if (featureItems.empty() == true)
   {
      return;
   }

   for (map<AoiElement*, vector<GraphicObject*> >::const_iterator featureIter = featureItems.begin();
      featureIter != featureItems.end();
      ++featureIter)
   {
      AoiElement* pAoi = featureIter->first;
      if (pAoi != NULL)
      {
         vector<GraphicObject*> objects = featureIter->second;
         if (objects.empty() == true)    // An empty vector indicates that all objects in the AOI should be used
         {
            QString message;

            vector<Feature*> features = addFeatures(pAoi, NULL, message);
            if ((features.empty() == true) && (message.isEmpty() == false))
            {
               QMessageBox::warning(this, windowTitle(), message);
            }
         }
         else
         {
            for (vector<GraphicObject*>::const_iterator objectIter = objects.begin();
               objectIter != objects.end();
               ++objectIter)
            {
               GraphicObject* pObject = *objectIter;
               if (pObject != NULL)
               {
                  QString message;

                  vector<Feature*> features = addFeatures(pAoi, pObject, message);
                  if ((features.empty() == true) && (message.isEmpty() == false))
                  {
                     QMessageBox::warning(this, windowTitle(), message);
                  }
               }
            }
         }
      }
   }

   // Update the field values here to add the name field column before any feature class fields
   updateFieldValues();

   // Apply a feature class
   QString className = dlg.getFeatureClass();
   if (className.isEmpty() == false)
   {
      applyFeatureClass(className.toStdString());
   }
}

void ShapeFileOptionsWidget::selectFeature(bool select)
{
   QTreeWidgetItem* pItem = mpFeatureTree->currentItem();
   if (pItem != NULL)
   {
      selectFeature(pItem, select);
   }
}

void ShapeFileOptionsWidget::removeFeature()
{
   QList<QTreeWidgetItem*> items = mpFeatureTree->selectedItems();
   if (items.empty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please select a feature to remove!");
      return;
   }

   vector<int> removedFeatureNumbers;

   // Remove the features from the shape file and the items from the tree widget
   for (int i = 0; i < items.count(); ++i)
   {
      QTreeWidgetItem* pItem = items[i];
      if (pItem != NULL)
      {
         QMap<QTreeWidgetItem*, Feature*>::iterator iter = mFeatures.find(pItem);
         if (iter != mFeatures.end())
         {
            Feature* pFeature = iter.value();
            if (pFeature != NULL)
            {
               mpShapeFile->removeFeature(pFeature);
            }

            QString strFeatureNumber = pItem->text(0);
            if (!strFeatureNumber.isEmpty())
            {
               removedFeatureNumbers.push_back(strFeatureNumber.toInt());
            }

            mFeatures.erase(iter);
            delete pItem;
         }
      }
   }

   // Update the numbers on the remaining feature items to account for the removed feature
   sort(removedFeatureNumbers.begin(), removedFeatureNumbers.end());

   while (removedFeatureNumbers.empty() == false)
   {
      for (QMap<QTreeWidgetItem*, Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
      {
         QTreeWidgetItem* pItem = iter.key();
         if (pItem != NULL)
         {
            int iFeature = 0;

            QString strFeatureNumber = pItem->text(0);
            if (!strFeatureNumber.isEmpty())
            {
               iFeature = strFeatureNumber.toInt();
            }

            if (iFeature > removedFeatureNumbers.back())
            {
               iFeature--;
               pItem->setText(0, QString::number(iFeature));
            }
         }
      }

      vector<int>::iterator iter = removedFeatureNumbers.end() - 1;
      removedFeatureNumbers.erase(iter);
   }
}

void ShapeFileOptionsWidget::clearFeatures()
{
   if (mpShapeFile != NULL)
   {
      mpShapeFile->removeAllFields();
      mpShapeFile->removeAllFeatures();
   }

   mpFeatureTree->clear();
   mpFeatureTree->setColumnCount(1);
   mFeatures.clear();
}

void ShapeFileOptionsWidget::applyFeatureClass()
{
   QStringList classNames;

   Service<ConfigurationSettings> pSettings;
   const DataVariant& featureClasses = pSettings->getSetting("ShapeFileExporter/FeatureClasses");

   const DynamicObject* pFeatureClasses = featureClasses.getPointerToValue<DynamicObject>();
   if (pFeatureClasses != NULL)
   {
      vector<string> classes;
      pFeatureClasses->getAttributeNames(classes);
      for (vector<string>::const_iterator iter = classes.begin(); iter != classes.end(); ++iter)
      {
         QString className = QString::fromStdString(*iter);
         if (className.isEmpty() == false)
         {
            classNames.append(className);
         }
      }
   }

   if (classNames.empty() == true)
   {
      QMessageBox::critical(this, "Apply Feature Class", "No feature classes are available.");
      return;
   }

   bool applyClass = false;

   QString className = QInputDialog::getItem(this, "Apply Feature Class", "Available feature classes:",
      classNames, 0, false, &applyClass);
   if (applyClass == true)
   {
      applyFeatureClass(className.toStdString());
   }
}

void ShapeFileOptionsWidget::editFeatureClasses()
{
   FeatureClassDlg dlg(this);
   dlg.exec();
}

void ShapeFileOptionsWidget::addField()
{
   AddFieldDlg dlg(this);
   if (dlg.exec() == QDialog::Rejected)
   {
      return;
   }

   // Add a new column to the tree widget and add a new field in the ShapeFile object
   QString fieldName = dlg.getFieldName();
   QString fieldType = dlg.getFieldType();
   addField(fieldName, fieldType);

   // Initialize the field values to the default value in the dialog
   if (mpShapeFile != NULL)
   {
      DataVariant fieldValue = dlg.getFieldValue();

      const vector<Feature*>& features = mpShapeFile->getFeatures();
      for (vector<Feature*>::const_iterator iter = features.begin(); iter != features.end(); ++iter)
      {
         Feature* pFeature = *iter;
         if (pFeature != NULL)
         {
            pFeature->setFieldValue(fieldName.toStdString(), fieldValue);
         }
      }

      updateFieldValues();
   }
}

void ShapeFileOptionsWidget::addField(const QString& strName, const QString& strType)
{
   if ((strName.isEmpty() == true) || (strType.isEmpty() == true))
   {
      return;
   }

   string name = strName.toStdString();
   if (mpShapeFile != NULL)
   {
      if (mpShapeFile->hasField(name) == false)
      {
         mpShapeFile->addField(name, strType.toStdString());
      }
   }

   int column = getColumn(strName);
   if (column == -1)
   {
      int iNumColumns = mpFeatureTree->columnCount();
      mpFeatureTree->setColumnCount(iNumColumns + 1);

      QTreeWidgetItem* pHeaderItem = mpFeatureTree->headerItem();
      if (pHeaderItem != NULL)
      {
         pHeaderItem->setText(iNumColumns, strName);
      }

      for (QTreeWidgetItemIterator iter(mpFeatureTree); *iter != NULL; ++iter)
      {
         QTreeWidgetItem* pItem = *iter;
         if (pItem != NULL)
         {
            QLineEdit* pLineEdit = NULL;
            if (strType == "int")
            {
               pLineEdit = mpIntEdit;
            }
            else if (strType == "double")
            {
               pLineEdit = mpDoubleEdit;
            }

            if (pLineEdit != NULL)
            {
               mpFeatureTree->setCellWidgetType(pItem, iNumColumns, CustomTreeWidget::CUSTOM_LINE_EDIT);
               mpFeatureTree->setCustomLineEdit(pItem, iNumColumns, pLineEdit);
            }
            else
            {
               mpFeatureTree->setCellWidgetType(pItem, iNumColumns, CustomTreeWidget::LINE_EDIT);
            }
         }
      }
   }
   else if (mpFeatureTree->isColumnHidden(column) == true)
   {
      mpFeatureTree->showColumn(column);
   }
}

void ShapeFileOptionsWidget::removeField()
{
   QStringList fieldNames;

   QTreeWidgetItem* pHeaderItem = mpFeatureTree->headerItem();
   if (pHeaderItem != NULL)
   {
      for (int i = 0; i < pHeaderItem->columnCount(); i++)
      {
         if (mpFeatureTree->isColumnHidden(i) == false)
         {
            QString strColumn = pHeaderItem->text(i);
            if (!strColumn.isEmpty())
            {
               fieldNames.append(strColumn);
            }
         }
      }
   }

   if (fieldNames.empty())
   {
      QMessageBox::critical(this, windowTitle(), "None of the current fields can be removed!");
      return;
   }

   bool bRemove = false;

   QString strFieldName = QInputDialog::getItem(this, "Remove Field", "Field Name:", fieldNames, 0, false, &bRemove);
   if (bRemove)
   {
      removeField(strFieldName);
   }
}

void ShapeFileOptionsWidget::removeField(const QString& strName)
{
   if (strName.isEmpty())
   {
      return;
   }

   int iColumn = getColumn(strName);
   if (iColumn != -1)
   {
      QHeaderView* pHeader = mpFeatureTree->header();
      if (pHeader != NULL)
      {
         pHeader->hideSection(iColumn);
      }
   }

   if (mpShapeFile != NULL)
   {
      string name = strName.toStdString();
      mpShapeFile->removeField(name);
   }
}

void ShapeFileOptionsWidget::setFieldValue(QTreeWidgetItem* pItem, int iColumn)
{
   if ((pItem == NULL) || (mpShapeFile == NULL))
   {
      return;
   }

   QString strFieldName;
   QString strValue = pItem->text(iColumn);

   QTreeWidgetItem* pHeaderItem = mpFeatureTree->headerItem();
   if (pHeaderItem != NULL)
   {
      strFieldName = pHeaderItem->text(iColumn);
   }

   if ((strFieldName.isEmpty() == true) || (strValue.isEmpty() == true))
   {
      return;
   }

   string name = strFieldName.toStdString();
   string type = mpShapeFile->getFieldType(name);

   QList<QTreeWidgetItem*> items = mpFeatureTree->selectedItems();
   for (int i = 0; i < items.count(); ++i)
   {
      QTreeWidgetItem* pCurrentItem = items[i];
      if (pCurrentItem != NULL)
      {
         QMap<QTreeWidgetItem*, Feature*>::iterator iter = mFeatures.find(pCurrentItem);
         if (iter != mFeatures.end())
         {
            Feature* pFeature = iter.value();
            if (pFeature != NULL)
            {
               if (type == "int")
               {
                  int iValue = strValue.toInt();
                  pFeature->setFieldValue(name, iValue);
               }
               else if (type == "double")
               {
                  double dValue = strValue.toDouble();
                  pFeature->setFieldValue(name, dValue);
               }
               else if (type == "string")
               {
                  string value = strValue.toStdString();
                  pFeature->setFieldValue(name, value);
               }
            }
         }
      }
   }

   if (items.size() > 1)
   {
      updateFieldValues();
   }
}

void ShapeFileOptionsWidget::updateFieldValues()
{
   for (QMap<QTreeWidgetItem*, Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
   {
      QTreeWidgetItem* pItem = iter.key();
      Feature* pFeature = iter.value();

      if ((pItem != NULL) && (pFeature != NULL))
      {
         vector<string> fieldNames = pFeature->getFieldNames();
         for (vector<string>::iterator fieldIter = fieldNames.begin(); fieldIter != fieldNames.end(); ++fieldIter)
         {
            string name = *fieldIter;
            if (!name.empty())
            {
               string type = pFeature->getFieldType(name);
               QString strValue;

               const DataVariant& var = pFeature->getFieldValue(name);
               strValue = QString::fromStdString(var.toDisplayString());

               int iColumn = getColumn(QString::fromStdString(name));
               if (iColumn == -1)
               {
                  addField(QString::fromStdString(name), QString::fromStdString(type));
                  iColumn = getColumn(QString::fromStdString(name));
               }

               if (iColumn != -1)
               {
                  pItem->setText(iColumn, strValue);
               }
            }
         }
      }
   }

   int numColumns = mpFeatureTree->columnCount();
   for (int i = 0; i < numColumns; ++i)
   {
      mpFeatureTree->resizeColumnToContents(i);
   }
}
