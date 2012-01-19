/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QTreeWidget>

#include "SignaturePropertiesDlg.h"
#include "Classification.h"
#include "DataVariant.h"
#include "Signature.h"

#include <string>
using namespace std;

SignaturePropertiesDlg::SignaturePropertiesDlg(Signature* pSignature, QWidget* parent) :
   QDialog(parent)
{
   QLabel* pNameLabel = new QLabel(this);
   QLabel* pFileLabel = new QLabel(this);
   QLabel* pClassificationLabel = new QLabel(this);
   QLabel* pAcquisitionMethodLabel = new QLabel(this);
   QLabel* pAcquisitionDateLabel = new QLabel(this);
   QLabel* pValidationSourceLabel = new QLabel(this);

   QFont ftLabel = QApplication::font();
   ftLabel.setBold(true);

   pNameLabel->setFont(ftLabel);
   pFileLabel->setFont(ftLabel);
   pClassificationLabel->setFont(ftLabel);
   pAcquisitionMethodLabel->setFont(ftLabel);
   pAcquisitionDateLabel->setFont(ftLabel);
   pValidationSourceLabel->setFont(ftLabel);

   // Description edit
   QTextEdit* pDescriptionEdit = new QTextEdit(this);
   pDescriptionEdit->setLineWrapMode(QTextEdit::WidgetWidth);
   pDescriptionEdit->setReadOnly(true);

   QPalette pltDescription = pDescriptionEdit->palette();
   pltDescription.setColor(QPalette::Base, Qt::lightGray);
   pDescriptionEdit->setPalette(pltDescription);

   // Metadata list
   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Value");

   QTreeWidget* pMetadataTree = new QTreeWidget(this);
   pMetadataTree->setColumnCount(columnNames.count());
   pMetadataTree->setHeaderLabels(columnNames);
   pMetadataTree->setSelectionMode(QAbstractItemView::NoSelection);
   pMetadataTree->setSortingEnabled(true);
   pMetadataTree->setRootIsDecorated(false);

   QHeaderView* pHeader = pMetadataTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   }

   // Widget labels
   QLabel* pName = new QLabel("Name:", this);
   QLabel* pFile = new QLabel("File:", this);
   QLabel* pClassification = new QLabel("Classification:", this);
   QLabel* pMethod = new QLabel("Acquisition Method:", this);
   QLabel* pDate = new QLabel("Acquisition Date:", this);
   QLabel* pSource = new QLabel("Validation Source:", this);
   QLabel* pDescription = new QLabel("Description:", this);
   QLabel* pMetadata = new QLabel("Metadata:", this);

   // OK button
   QPushButton* pOkButton = new QPushButton("&OK", this);
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pName, 0, 0);
   pGrid->addWidget(pNameLabel, 0, 2);
   pGrid->addWidget(pFile, 1, 0);
   pGrid->addWidget(pFileLabel, 1, 2);
   pGrid->addWidget(pClassification, 2, 0);
   pGrid->addWidget(pClassificationLabel, 2, 2);
   pGrid->addWidget(pMethod, 3, 0);
   pGrid->addWidget(pAcquisitionMethodLabel, 3, 2);
   pGrid->addWidget(pDate, 4, 0);
   pGrid->addWidget(pAcquisitionDateLabel, 4, 2);
   pGrid->addWidget(pSource, 5, 0);
   pGrid->addWidget(pValidationSourceLabel, 5, 2);
   pGrid->addWidget(pDescription, 6, 0, Qt::AlignBottom);
   pGrid->addWidget(pDescriptionEdit, 7, 0, 1, 3);
   pGrid->addWidget(pMetadata, 8, 0, Qt::AlignBottom);
   pGrid->addWidget(pMetadataTree, 9, 0, 1, 3);
   pGrid->addWidget(pOkButton, 11, 0, 1, 3, Qt::AlignRight);
   pGrid->setColumnMinimumWidth(1, 5);
   pGrid->setColumnStretch(2, 10);
   pGrid->setRowMinimumHeight(6, 25);
   pGrid->setRowStretch(7, 3);
   pGrid->setRowMinimumHeight(8, 25);
   pGrid->setRowStretch(9, 10);
   pGrid->setRowMinimumHeight(10, 5);

   // Initialization
   setWindowTitle("Signature Properties");
   setModal(true);
   resize(375, 465);

   QString strName;
   QString strFile;
   QString strClassification;
   QString strMethod;
   QString strDate;
   QString strSource;
   QString strDescription;

   if (pSignature != NULL)
   {
      const string& name = pSignature->getName();
      if (name.empty() == false)
      {
         strName = QString::fromStdString(name);
      }

      const string& file = pSignature->getFilename();
      if (file.empty() == false)
      {
         strFile = QString::fromStdString(file);
      }

      const Classification* pClassification = pSignature->getClassification();
      if (pClassification != NULL)
      {
         string classificationText = "";
         pClassification->getClassificationText(classificationText);
         if (classificationText.empty() == false)
         {
            strClassification = QString::fromStdString(classificationText);
         }
      }

      const DynamicObject* pMetadata = pSignature->getMetadata();
      if (pMetadata != NULL)
      {
         vector<string> metadataKeys;
         pMetadata->getAttributeNames(metadataKeys);

         for (unsigned int i = 0; i < metadataKeys.size(); i++)
         {
            string metadataName = metadataKeys[i];
            if (metadataName.empty() == false)
            {
               QString strValue;

               string value = pMetadata->getAttribute(metadataName).toDisplayString();
               if (value.empty() == false)
               {
                  strValue = QString::fromStdString(value);
               }

               QTreeWidgetItem* pItem = new QTreeWidgetItem(pMetadataTree);
               if (pItem != NULL)
               {
                  pItem->setText(0, QString::fromStdString(metadataName));
                  pItem->setText(1, strValue);
               }
            }
         }
      }
   }

   pNameLabel->setText(strName);
   pFileLabel->setText(strFile);
   pClassificationLabel->setText(strClassification);
   pAcquisitionMethodLabel->setText(strMethod);
   pAcquisitionDateLabel->setText(strDate);
   pValidationSourceLabel->setText(strSource);
   pDescriptionEdit->setPlainText(strDescription);
}

SignaturePropertiesDlg::~SignaturePropertiesDlg()
{
}
