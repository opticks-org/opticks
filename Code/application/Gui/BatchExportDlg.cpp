/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "AppVerify.h"
#include "BatchExportDlg.h"
#include "PlugInDescriptor.h"
#include "PlugInResource.h"

using namespace std;

BatchExportDlg::BatchExportDlg(ExporterResource& exporter, const vector<PlugInDescriptor*>& availablePlugIns,
                               QWidget* pParent) :
   QDialog(pParent),
   mpExporter(exporter)
{
   // Directory
   QLabel* pDirectoryLabel = new QLabel("Directory:", this);
   mpDirectoryEdit = new QLineEdit(this);

   QIcon icnBrowse(":/icons/Open");

   QPushButton* pBrowseButton = new QPushButton(icnBrowse, QString(), this);
   pBrowseButton->setFixedWidth(27);

   // Exporter
   QLabel* pExporterLabel = new QLabel("Exporter:", this);
   mpExporterCombo = new QComboBox(this);
   mpExporterCombo->setEditable(false);

   // File extension
   QLabel* pExtensionLabel = new QLabel("File Extension:", this);
   mpExtensionCombo = new QComboBox(this);
   mpExtensionCombo->setEditable(false);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(this);
   pButtonBox->setOrientation(Qt::Horizontal);
   pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(pDirectoryLabel, 0, 0);
   pGrid->addWidget(mpDirectoryEdit, 0, 1);
   pGrid->addWidget(pBrowseButton, 0, 2);
   pGrid->addWidget(pExporterLabel, 1, 0);
   pGrid->addWidget(mpExporterCombo, 1, 1, 1, 2);
   pGrid->addWidget(pExtensionLabel, 2, 0);
   pGrid->addWidget(mpExtensionCombo, 2, 1, 1, 2, Qt::AlignLeft);
   pGrid->addWidget(pHLine, 3, 0, 1, 3, Qt::AlignBottom);
   pGrid->addWidget(pButtonBox, 4, 0, 1, 3);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   mpDirectoryEdit->setText(QDir::currentPath());

   vector<PlugInDescriptor*>::const_iterator iter;
   for (iter = availablePlugIns.begin(); iter != availablePlugIns.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         // Exporters
         QString strExporter = QString::fromStdString(pDescriptor->getName());
         mpExporterCombo->addItem(strExporter);

         // File extensions
         QStringList extensions;

         string filters = pDescriptor->getFileExtensions();
         if (filters.empty() == false)
         {
            QString strFilters = QString::fromStdString(filters);
            QStringList filterList = strFilters.split(";;", QString::SkipEmptyParts);

            for (int i = 0; i < filterList.count(); ++i)
            {
               QString strFilter = filterList[i];
               if (strFilter.isEmpty() == false)
               {
                  QFileInfo filterInfo(strFilter);
                  QString strDefault = filterInfo.completeSuffix();

                  int parenIndex = strDefault.indexOf(")");
                  int spaceIndex = strDefault.indexOf(" ");

                  int index = parenIndex;
                  if (parenIndex != -1)
                  {
                     if ((spaceIndex < parenIndex) && (spaceIndex != -1))
                     {
                        index = spaceIndex;
                     }

                     strDefault.truncate(index);

                     QString strExtension = strDefault.trimmed();
                     if (strExtension.isEmpty() == false)
                     {
                        extensions.append(strExtension);
                     }
                  }
               }
            }
         }

         mpExtensionCombo->addItems(extensions);
         mFileExtensions.insert(strExporter, extensions);
      }
   }

   updateExporter(mpExporterCombo->currentText());
   updateFileExtensions();

   setModal(true);
   setWindowTitle("Export");
   resize(450, 200);

   // Connections
   VERIFYNR(connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browse())));
   VERIFYNR(connect(mpExporterCombo, SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(updateExporter(const QString&))));
   VERIFYNR(connect(mpExporterCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFileExtensions())));
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

BatchExportDlg::~BatchExportDlg()
{
}

QString BatchExportDlg::getExportDirectory() const
{
   return mpDirectoryEdit->text();
}

QString BatchExportDlg::getExporterName() const
{
   return mpExporterCombo->currentText();
}

QString BatchExportDlg::getFileExtension() const
{
   return mpExtensionCombo->currentText();
}

void BatchExportDlg::browse()
{
   QString directory = QFileDialog::getExistingDirectory(this, windowTitle(), getExportDirectory());
   if (directory.isEmpty() == false)
   {
      mpDirectoryEdit->setText(directory);
   }
}

void BatchExportDlg::updateExporter(const QString& strExporter)
{
   if (strExporter.isEmpty() == false)
   {
      mpExporter->setPlugIn(strExporter.toStdString());
   }
   else
   {
      mpExporter->setPlugIn(NULL);
   }
}

void BatchExportDlg::updateFileExtensions()
{
   QStringList extensions;

   QString strExporter = getExporterName();
   if (strExporter.isEmpty() == false)
   {
      QMap<QString, QStringList>::iterator iter = mFileExtensions.find(strExporter);
      if (iter != mFileExtensions.end())
      {
         extensions = iter.value();
      }
   }

   mpExtensionCombo->clear();
   mpExtensionCombo->addItems(extensions);
}
