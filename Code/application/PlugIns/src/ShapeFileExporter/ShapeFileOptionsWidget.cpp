/*
* The information in this file is
* Copyright(c) 2008 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#include <QtCore/QFileInfo>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>

#include "AddFeatureDlg.h"
#include "AddFieldDlg.h"
#include "AoiElement.h"
#include "CustomTreeWidget.h"
#include "DataElement.h"
#include "DataVariant.h"
#include "Feature.h"
#include "IconImages.h"
#include "RasterElement.h"
#include "ShapeFile.h"
#include "ShapeFileOptionsWidget.h"

#include <string>
using namespace std;

ShapeFileOptionsWidget::ShapeFileOptionsWidget(ShapeFile* pShapefile, const vector<AoiElement*>& aois, 
                                               RasterElement* pRaster) :
   QWidget(NULL), mpShapeFile(pShapefile), mAois(aois), mpGeoref(pRaster)
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
   QPixmap pixOpen(IconImages::OpenIcon);
   pixOpen.setMask(pixOpen.createHeuristicMask());
   QIcon icnBrowse(pixOpen);

   QPushButton* pBrowseButton = new QPushButton(icnBrowse, QString(), this);
   pBrowseButton->setFixedWidth(27);

   // Shape type
   QLabel* pShapeLabel = new QLabel("Shape:", this);
   pShapeLabel->setFont(ftBold);

   mpShapeCombo = new QComboBox(this);
   mpShapeCombo->setEditable(false);
   mpShapeCombo->addItem("Point");
   mpShapeCombo->addItem("Polyline");
   mpShapeCombo->addItem("Polygon");
   mpShapeCombo->addItem("Multi-Point");
   mpShapeCombo->setFixedWidth(150);

   // Feature list
   QLabel* pFeatureLabel = new QLabel("Features:", this);

   QStringList columnNames;
   columnNames.append("Feature");

   mpFeatureTree = new CustomTreeWidget(this);
   mpFeatureTree->setColumnCount(columnNames.count());
   mpFeatureTree->setHeaderLabels(columnNames);
   mpFeatureTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpFeatureTree->setRootIsDecorated(false);
   mpFeatureTree->setSortingEnabled(true);

   QHeaderView* pHeader = mpFeatureTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   // Feature buttons
   QPushButton* pAddFeatureButton = new QPushButton("Add Feature", this);
   QPushButton* pRemoveFeatureButton = new QPushButton("Remove Feature", this);
   QPushButton* pClearFeatureButton = new QPushButton("Clear Features", this);

   // Field buttons
   QPushButton* pAddFieldButton = new QPushButton("Add Field", this);
   QPushButton* pRemoveFieldButton = new QPushButton("Remove Field", this);

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

   QVBoxLayout* pButtonLayout = new QVBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(pAddFeatureButton);
   pButtonLayout->addWidget(pRemoveFeatureButton);
   pButtonLayout->addWidget(pClearFeatureButton);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pAddFieldButton);
   pButtonLayout->addWidget(pRemoveFieldButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pFilePathLabel, 0, 0);
   pGrid->addLayout(pFilePathLayout, 0, 1, 1, 3);
   pGrid->addWidget(pBaseNameLabel, 1, 0);
   pGrid->addLayout(pNameShapeLayout, 1, 1, 1, 3);
   pGrid->addWidget(pShpLabel, 2, 0);
   pGrid->addWidget(mpShpFileLabel, 2, 1, 1, 3);
   pGrid->addWidget(pShxLabel, 3, 0);
   pGrid->addWidget(mpShxFileLabel, 3, 1, 1, 3);
   pGrid->addWidget(pDbfLabel, 4, 0);
   pGrid->addWidget(mpDbfFileLabel, 4, 1, 1, 3);
   pGrid->setRowMinimumHeight(5, 12);
   pGrid->addWidget(pFeatureLabel, 6, 0, 1, 4);
   pGrid->addWidget(mpFeatureTree, 7, 0, 1, 2);
   pGrid->setColumnMinimumWidth(2, 2);
   pGrid->addLayout(pButtonLayout, 7, 3);
   pGrid->setRowStretch(7, 10);
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
      ShapeType eShape = mpShapeFile->getShape();
      switch(eShape)
      {
      case POINT_SHAPE:
         mpShapeCombo->setCurrentIndex(0);
         break;
      case POLYLINE_SHAPE:
         mpShapeCombo->setCurrentIndex(1);
         break;
      case POLYGON_SHAPE:
         mpShapeCombo->setCurrentIndex(2);
         break;
      case MULTIPOINT_SHAPE:
         mpShapeCombo->setCurrentIndex(3);
         break;
      default:
         ; // blank
      }

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
   connect(mpFilePathEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateFilenames()));
   connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browse()));
   connect(mpBaseNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateFilenames()));
   connect(mpShapeCombo, SIGNAL(activated(const QString&)), this, SLOT(setShape(const QString&)));
   connect(pAddFeatureButton, SIGNAL(clicked()), this, SLOT(addFeature()));
   connect(pRemoveFeatureButton, SIGNAL(clicked()), this, SLOT(removeFeature()));
   connect(pClearFeatureButton, SIGNAL(clicked()), this, SLOT(clearFeatures()));
   connect(pAddFieldButton, SIGNAL(clicked()), this, SLOT(addField()));
   connect(pRemoveFieldButton, SIGNAL(clicked()), this, SLOT(removeField()));
   connect(mpFeatureTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this, SLOT(setFieldValue(QTreeWidgetItem*, int)));
}

ShapeFileOptionsWidget::~ShapeFileOptionsWidget()
{
}

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

void ShapeFileOptionsWidget::updateFilenames()
{
   QString strDirectory = mpFilePathEdit->text();
   QString strBaseName = mpBaseNameEdit->text();

   QString strFilename;

   if (!strDirectory.isEmpty() && !strBaseName.isEmpty())
   {
      strFilename = strDirectory + "/" + strBaseName;
      if (!strFilename.isEmpty())
      {
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
      mpFilePathEdit->setText(strDirectory);
   }
}

void ShapeFileOptionsWidget::setShape(const QString& strShape)
{
   if (mpShapeFile == NULL)
   {
      return;
   }

   ShapeType eShape;
   if (strShape == "Point")
   {
      eShape = POINT_SHAPE;
   }
   else if (strShape == "Polyline")
   {
      eShape = POLYLINE_SHAPE;
   }
   else if (strShape == "Polygon")
   {
      eShape = POLYGON_SHAPE;
   }
   else if (strShape == "Multi-Point")
   {
      eShape = MULTIPOINT_SHAPE;
   }
   else
   {
      return;
   }

   ShapeType eCurrentShape = mpShapeFile->getShape();
   if ((eCurrentShape != POINT_SHAPE) && (eShape == POINT_SHAPE))
   {
      if (mpFeatureTree->topLevelItemCount() > 0)
      {
         QMessageBox::warning(this, windowTitle(), "Changing to the Point shape may cause a loss of data "
            "since only the first point in a feature will be used.");
      }
   }

   mpShapeFile->setShape(eShape);
}

void ShapeFileOptionsWidget::addFeature()
{
   AddFeatureDlg dlg(mAois, this);

   if (dlg.exec() == QDialog::Accepted)
   {
      vector<AoiElement*> elements = dlg.getAoiElements();
      for (vector<AoiElement*>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
      {
         AoiElement* pElement = *iter;
         if (pElement != NULL)
         {
            string message = "";
            vector<Feature*> features = mpShapeFile->addFeatures(pElement, mpGeoref, message);

            for (unsigned int i = 0; i < features.size(); i++)
            {
               Feature* pFeature = NULL;
               pFeature = features[i];
               if (pFeature != NULL)
               {
                  int iFeatures = mpFeatureTree->topLevelItemCount();

                  QTreeWidgetItem* pItem = new QTreeWidgetItem(mpFeatureTree);
                  if (pItem != NULL)
                  {
                     pItem->setText(0, QString::number(iFeatures + 1));
                     mFeatures.insert(pItem, pFeature);
                  }
               }
            }

            if (!message.empty())
            {
               QMessageBox::warning(this, windowTitle(), QString::fromStdString(message));
            }
         }
      }

      updateFieldValues();
   }
}

void ShapeFileOptionsWidget::removeFeature()
{
   int iFeatureNumber = 0;
   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpFeatureTree->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

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
            iFeatureNumber = strFeatureNumber.toInt();
         }

         mFeatures.erase(iter);
         delete pItem;
      }
   }
   else
   {
      QMessageBox::critical(this, windowTitle(), "Please select a feature to remove!");
      return;
   }

   // Update the numbers on the remaining feature items to account for the removed feature
   if (iFeatureNumber == 0)
   {
      return;
   }

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

         if (iFeature > iFeatureNumber)
         {
            iFeature--;
            pItem->setText(0, QString::number(iFeature));
         }
      }
   }
}

void ShapeFileOptionsWidget::clearFeatures()
{
   if (mpShapeFile != NULL)
   {
      mpShapeFile->removeAllFeatures();
   }

   mpFeatureTree->clear();
   mFeatures.clear();
}

void ShapeFileOptionsWidget::addField()
{
   AddFieldDlg dlg(this);

   if (dlg.exec() == QDialog::Accepted)
   {
      QString strFieldName = dlg.getFieldName();
      QString strFieldType = dlg.getFieldType();
      addField(strFieldName, strFieldType);
   }
}

void ShapeFileOptionsWidget::addField(const QString& strName, const QString& strType)
{
   if (!strName.isEmpty() && !strType.isEmpty())
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
            mpFeatureTree->setCellWidgetType(pItem, iNumColumns, CustomTreeWidget::LINE_EDIT);
         }
      }

      if (mpShapeFile != NULL)
      {
         string name = strName.toStdString();
         string type = strType.toStdString();
         mpShapeFile->addField(name, type);
      }
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
         QString strColumn = pHeaderItem->text(i);
         if (!strColumn.isEmpty())
         {
            fieldNames.append(strColumn);
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

   for (QMap<QTreeWidgetItem*, Feature*>::iterator iter = mFeatures.find(pItem); iter != mFeatures.end(); ++iter)
   {
      Feature *pFeature = iter.value();
      if (pFeature != NULL)
      {
         QString strFieldName;
         QString strValue = pItem->text(iColumn);

         QTreeWidgetItem* pHeaderItem = mpFeatureTree->headerItem();
         if (pHeaderItem != NULL)
         {
            strFieldName = pHeaderItem->text(iColumn);
         }

         if (!strFieldName.isEmpty() && !strValue.isEmpty())
         {
            string name = strFieldName.toStdString();
            string type = mpShapeFile->getFieldType(name);

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

               const DataVariant &var = pFeature->getFieldValue(name);
               strValue = QString::fromStdString(var.toDisplayString());

               int iColumn = getColumn(QString::fromLatin1(name.c_str()));
               if (iColumn == -1)
               {
                  addField(QString::fromLatin1(name.c_str()), QString::fromLatin1(type.c_str()));
                  iColumn = getColumn(QString::fromLatin1(name.c_str()));
               }

               if (iColumn != -1)
               {
                  pItem->setText(iColumn, strValue);
               }
            }
         }
      }
   }
}
