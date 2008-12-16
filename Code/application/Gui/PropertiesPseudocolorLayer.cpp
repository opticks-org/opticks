/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AppVerify.h"
#include "AppVersion.h"
#include "CustomTreeWidget.h"
#include "LabeledSection.h"
#include "PropertiesPseudocolorLayer.h"
#include "PseudocolorLayer.h"
#include "PseudocolorLayerImp.h"
#include "SymbolTypeGrid.h"
#include "Undo.h"
#include "View.h"

#include <limits>
using namespace std;

PropertiesPseudocolorLayer::PropertiesPseudocolorLayer() :
   LabeledSectionGroup(NULL),
   mpPseudocolorLayer(NULL)
{
   // Pixel marker
   QWidget* pMarkerWidget = new QWidget(this);

   QLabel* pSymbolLabel = new QLabel("Symbol:", pMarkerWidget);
   mpSymbolButton = new SymbolTypeButton(pMarkerWidget);
   mpSymbolButton->setBorderedSymbols(true);

   LabeledSection* pMarkerSection = new LabeledSection(pMarkerWidget, "Pixel Marker", this);

   QGridLayout* pMarkerGrid = new QGridLayout(pMarkerWidget);
   pMarkerGrid->setMargin(0);
   pMarkerGrid->setSpacing(5);
   pMarkerGrid->addWidget(pSymbolLabel, 0, 0);
   pMarkerGrid->addWidget(mpSymbolButton, 0, 1, Qt::AlignLeft);
   pMarkerGrid->setRowStretch(1, 10);
   pMarkerGrid->setColumnStretch(1, 10);

   // Classes
   QWidget* pClassesWidget = new QWidget(this);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Data Value");
   columnNames.append("Red");
   columnNames.append("Green");
   columnNames.append("Blue");
   columnNames.append("Color");

   mpClassesTree = new CustomTreeWidget(pClassesWidget);
   mpClassesTree->setColumnCount(columnNames.count());
   mpClassesTree->setHeaderLabels(columnNames);
   mpClassesTree->setRootIsDecorated(false);
   mpClassesTree->setSortingEnabled(false);
   mpClassesTree->setFullCellColor(true);

   QHeaderView* pHeader = mpClassesTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(false);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(0, 125);
      pHeader->resizeSection(1, 75);
      pHeader->resizeSection(2, 50);
      pHeader->resizeSection(3, 50);
      pHeader->resizeSection(4, 50);
      pHeader->resizeSection(5, 50);
   }

   QPushButton* pAddButton = new QPushButton("&Add", pClassesWidget);
   QPushButton* pRemoveButton = new QPushButton("&Remove", pClassesWidget);

   LabeledSection* pClassesSection = new LabeledSection(pClassesWidget, "Classes", this);

   QGridLayout* pClassesGrid = new QGridLayout(pClassesWidget);
   pClassesGrid->setMargin(0);
   pClassesGrid->setSpacing(5);
   pClassesGrid->addWidget(mpClassesTree, 0, 0, 1, 2);
   pClassesGrid->addWidget(pAddButton, 1, 0);
   pClassesGrid->addWidget(pRemoveButton, 1, 1, Qt::AlignLeft);
   pClassesGrid->setRowStretch(0, 10);
   pClassesGrid->setColumnStretch(1, 10);

   // Initialization
   addSection(pMarkerSection);
   addSection(pClassesSection, 10);
   setSizeHint(450, 250);

   // Connections
   VERIFYNR(connect(mpClassesTree, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this,
      SLOT(updateColorBox(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpClassesTree, SIGNAL(cellColorChanged(QTreeWidgetItem*, int)), this,
      SLOT(updateRgbText(QTreeWidgetItem*, int))));
   VERIFYNR(connect(pAddButton, SIGNAL(clicked()), this, SLOT(addClass())));
   VERIFYNR(connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeSelectedClasses())));
}

PropertiesPseudocolorLayer::~PropertiesPseudocolorLayer()
{
}

bool PropertiesPseudocolorLayer::initialize(SessionItem* pSessionItem)
{
   mpPseudocolorLayer = dynamic_cast<PseudocolorLayer*>(pSessionItem);
   if (mpPseudocolorLayer == NULL)
   {
      return false;
   }

   // Pixel marker
   mpSymbolButton->setCurrentValue(mpPseudocolorLayer->getSymbol());

   // Classes
   mpClassesTree->clear();

   PseudocolorLayerImp* pLayer = dynamic_cast<PseudocolorLayerImp*>(mpPseudocolorLayer);
   if (pLayer != NULL)
   {
      vector<PseudocolorClass*> classes = pLayer->getAllClasses();
      for (unsigned int i = 0; i < classes.size(); ++i)
      {
         PseudocolorClass* pClass = classes[i];
         if (pClass != NULL)
         {
            bool bDisplayed = pClass->isDisplayed();
            QString strClassName = pClass->getName();
            int iValue = pClass->getValue();
            QColor clrClass = pClass->getColor();

            CustomTreeWidget::CheckState displayState = CustomTreeWidget::UNCHECKED;
            if (bDisplayed == true)
            {
               displayState = CustomTreeWidget::CHECKED;
            }

            QTreeWidgetItem* pItem = new QTreeWidgetItem(mpClassesTree);
            if (pItem != NULL)
            {
               pItem->setText(0, strClassName);
               pItem->setText(1, QString::number(iValue));
               pItem->setText(2, QString::number(clrClass.red()));
               pItem->setText(3, QString::number(clrClass.green()));
               pItem->setText(4, QString::number(clrClass.blue()));
               pItem->setText(5, QString());

               mpClassesTree->setCellWidgetType(pItem, 0, CustomTreeWidget::LINE_EDIT);
               mpClassesTree->setCellWidgetType(pItem, 1, CustomTreeWidget::NO_WIDGET);
               mpClassesTree->setCellWidgetType(pItem, 2, CustomTreeWidget::LINE_EDIT);
               mpClassesTree->setCellWidgetType(pItem, 3, CustomTreeWidget::LINE_EDIT);
               mpClassesTree->setCellWidgetType(pItem, 4, CustomTreeWidget::LINE_EDIT);
               mpClassesTree->setCellWidgetType(pItem, 5, CustomTreeWidget::NO_WIDGET);

               mpClassesTree->setFullCellEdit(pItem, 0, false);
               mpClassesTree->setCellCheckState(pItem, 0, displayState);
               mpClassesTree->setCellColor(pItem, 5, clrClass);
            }
         }
      }
   }

   return true;
}

bool PropertiesPseudocolorLayer::applyChanges()
{
   if (mpPseudocolorLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpPseudocolorLayer->getView(), actionText);

   // Pixel marker
   mpPseudocolorLayer->setSymbol(mpSymbolButton->getCurrentValue());

   // Classes
   PseudocolorLayerImp* pLayer = dynamic_cast<PseudocolorLayerImp*>(mpPseudocolorLayer);
   if (pLayer != NULL)
   {
      unsigned int i = 0;

      // Remove classes
      vector<PseudocolorClass*> classes = pLayer->getAllClasses();
      for (i = 0; i < classes.size(); ++i)
      {
         PseudocolorClass* pClass = classes[i];
         if (pClass != NULL)
         {
            int currentValue = pClass->getValue();

            bool classStillExists = false;

            QTreeWidgetItemIterator iter(mpClassesTree);
            while (*iter != NULL)
            {
               QTreeWidgetItem* pItem = *iter;
               if (pItem != NULL)
               {
                  int value = pItem->text(1).toInt();
                  if (value == currentValue)
                  {
                     classStillExists = true;
                     break;
                  }
               }

               ++iter;
            }

            if (classStillExists == false)
            {
               pLayer->removeClass(pClass);
            }
         }
      }

      // Add or update classes
      QTreeWidgetItemIterator iter(mpClassesTree);
      while (*iter != NULL)
      {
         QTreeWidgetItem* pItem = *iter;
         if (pItem != NULL)
         {
            bool bDisplayed = false;

            CustomTreeWidget::CheckState displayState = mpClassesTree->getCellCheckState(pItem, 0);
            if (displayState == CustomTreeWidget::CHECKED)
            {
               bDisplayed = true;
            }

            QString strName = pItem->text(0);
            QString strValue = pItem->text(1);
            QColor clrClass = mpClassesTree->getCellColor(pItem, 5);

            int iValue = strValue.toInt();
            int iRed = clrClass.red();
            int iGreen = clrClass.green();
            int iBlue = clrClass.blue();

            classes = pLayer->getAllClasses();
            for (i = 0; i < classes.size(); i++)
            {
               PseudocolorClass* pClass = classes[i];
               if (pClass != NULL)
               {
                  int currentValue = pClass->getValue();
                  if (currentValue == iValue)
                  {
                     pClass->setClassName(strName);
                     pClass->setColor(QColor(iRed, iGreen, iBlue));
                     pClass->setDisplayed(bDisplayed);
                     break;
                  }
               }
            }

            if (i == classes.size())
            {
               PseudocolorClass* pClass = pLayer->addClass();
               if (pClass != NULL)
               {
                  pClass->setProperties(strName, iValue, QColor(iRed, iGreen, iBlue), bDisplayed);
               }
            }
         }

         ++iter;
      }
   }

   // Refresh the view
   View* pView = mpPseudocolorLayer->getView();
   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

const string& PropertiesPseudocolorLayer::getName()
{
   static string name = "Pseudocolor Layer Properties";
   return name;
}

const string& PropertiesPseudocolorLayer::getPropertiesName()
{
   static string propertiesName = "Pseudocolor Layer";
   return propertiesName;
}

const string& PropertiesPseudocolorLayer::getDescription()
{
   static string description = "General setting properties of a pseudocolor layer";
   return description;
}

const string& PropertiesPseudocolorLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesPseudocolorLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesPseudocolorLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesPseudocolorLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesPseudocolorLayer::getDescriptorId()
{
   static string id = "{3A36AB03-17D3-48D6-9FBD-985FBCAB801C}";
   return id;
}

bool PropertiesPseudocolorLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesPseudocolorLayer::updateColorBox(QTreeWidgetItem* pItem, int iColumn)
{
   if (pItem == NULL)
   {
      return;
   }

   if ((iColumn != 2) && (iColumn != 3) && (iColumn != 4))
   {
      return;
   }

   QString strRed = pItem->text(2);
   QString strGreen = pItem->text(3);
   QString strBlue = pItem->text(4);

   int iRed = strRed.toInt();
   int iGreen = strGreen.toInt();
   int iBlue = strBlue.toInt();

   QColor clrClass = QColor(iRed, iGreen, iBlue);
   mpClassesTree->setCellColor(pItem, 5, clrClass);
}

void PropertiesPseudocolorLayer::updateRgbText(QTreeWidgetItem* pItem, int iColumn)
{
   if ((pItem == NULL) || (iColumn != 5))
   {
      return;
   }

   QColor clrCell = mpClassesTree->getCellColor(pItem, iColumn);
   if (clrCell.isValid() == true)
   {
      int iRed = clrCell.red();
      int iGreen = clrCell.green();
      int iBlue = clrCell.blue();

      pItem->setText(2, QString::number(iRed));
      pItem->setText(3, QString::number(iGreen));
      pItem->setText(4, QString::number(iBlue));
   }
}

void PropertiesPseudocolorLayer::addClass()
{
   int classValue = 0;
   bool isUnique = false;
   bool ok = false;

   // Get a unique value from the user for the class
   while (isUnique == false)
   {
      classValue = QInputDialog::getInteger(this, "New Class", "Enter the data value to associate with the new class:",
         classValue, numeric_limits<int>::min(), numeric_limits<int>::max(), 1, &ok);
      if (ok == false)
      {
         return;
      }

      QList<QTreeWidgetItem*> existingClasses = mpClassesTree->findItems(QString::number(classValue),
         Qt::MatchExactly, 1);
      isUnique = existingClasses.empty();

      if (isUnique == false)
      {
         QMessageBox::information(this, "Duplicate Class", "There is already a class with this value.  "
            "You cannot have two classes with the same value.");
      }
   }

   // Create the class item in the tree widget
   QTreeWidgetItem* pItem = new QTreeWidgetItem(mpClassesTree);
   if (pItem != NULL)
   {
      // Get a unique name for the new class
      QString name;
      int index = 1;

      isUnique = false;
      while (isUnique == false)
      {
         name = "Class " + QString::number(index++);
         QList<QTreeWidgetItem*> existingClasses = mpClassesTree->findItems(name, Qt::MatchExactly, 0);
         isUnique = existingClasses.empty();
      }

      QColor clrClass = Qt::red;

      pItem->setText(0, name);
      pItem->setText(1, QString::number(classValue));
      pItem->setText(2, QString::number(clrClass.red()));
      pItem->setText(3, QString::number(clrClass.green()));
      pItem->setText(4, QString::number(clrClass.blue()));
      pItem->setText(5, QString());

      mpClassesTree->setCellWidgetType(pItem, 0, CustomTreeWidget::LINE_EDIT);
      mpClassesTree->setCellWidgetType(pItem, 1, CustomTreeWidget::NO_WIDGET);
      mpClassesTree->setCellWidgetType(pItem, 2, CustomTreeWidget::LINE_EDIT);
      mpClassesTree->setCellWidgetType(pItem, 3, CustomTreeWidget::LINE_EDIT);
      mpClassesTree->setCellWidgetType(pItem, 4, CustomTreeWidget::LINE_EDIT);
      mpClassesTree->setCellWidgetType(pItem, 5, CustomTreeWidget::NO_WIDGET);

      mpClassesTree->setFullCellEdit(pItem, 0, false);
      mpClassesTree->setCellCheckState(pItem, 0, CustomTreeWidget::CHECKED);
      mpClassesTree->setCellColor(pItem, 5, clrClass);
   }
}

void PropertiesPseudocolorLayer::removeSelectedClasses()
{
   QTreeWidgetItemIterator iter(mpClassesTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         if (mpClassesTree->isItemSelected(pItem) == true)
         {
            delete pItem;
         }
      }

      ++iter;
   }
}
