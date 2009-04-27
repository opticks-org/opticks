/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QStringList>
#include <QtGui/QHeaderView>
#include <QtGui/QLayout>

#include "AppVersion.h"
#include "LabeledSection.h"
#include "PlugInArgList.h"
#include "PlugInDescriptorImp.h"
#include "PropertiesPlugInDescriptor.h"

using namespace std;

PropertiesPlugInDescriptor::PropertiesPlugInDescriptor() :
   LabeledSectionGroup(NULL)
{
   // PlugIn
   QWidget* pPlugInWidget = new QWidget(this);
   QLabel* pNameLabel = new QLabel("Name:", pPlugInWidget);
   mpNameLabel = new QLabel(pPlugInWidget);
   QLabel* pVersionLabel = new QLabel("Version:", pPlugInWidget);
   mpVersionLabel = new QLabel(pPlugInWidget);
   QLabel* pProductionStatusLabel = new QLabel("Acceptable for Production Use:", pPlugInWidget);
   mpProductionStatusLabel = new QLabel(pPlugInWidget);
   QLabel* pCreatorLabel = new QLabel("Creator:", pPlugInWidget);
   mpCreatorLabel = new QLabel(pPlugInWidget);
   QLabel* pCopyrightLabel = new QLabel("Copyright:", pPlugInWidget);
   mpCopyrightLabel = new QLabel(pPlugInWidget);
   /**
    * Note: the dependencyCopyright information is not displayed here since it appears in
    *       the application About dialog
    */
   QLabel* pDescriptionLabel = new QLabel("Description:", pPlugInWidget);
   mpDescriptionLabel = new QLabel(pPlugInWidget);
   mpDescriptionLabel->setWordWrap(true);
   QLabel* pShortDescriptionLabel = new QLabel("Short Description:", pPlugInWidget);
   mpShortDescriptionLabel = new QLabel(pPlugInWidget);
   QLabel* pTypeLabel = new QLabel("Type:", pPlugInWidget);
   mpTypeLabel = new QLabel(pPlugInWidget);
   QLabel* pSubtypeLabel = new QLabel("Subtype:", pPlugInWidget);
   mpSubtypeLabel = new QLabel(pPlugInWidget);
   QLabel* pMultipleInstancesLabel = new QLabel("Multiple Instance Support:", pPlugInWidget);
   mpMultipleInstancesLabel = new QLabel(pPlugInWidget);
   QLabel* pRunningInstancesLabel = new QLabel("Running Instances:", pPlugInWidget);
   mpRunningInstancesLabel = new QLabel(pPlugInWidget);
   mpPlugInSection = new LabeledSection(pPlugInWidget, "Plug-In", this);

   QGridLayout* pPlugInGrid = new QGridLayout(pPlugInWidget);
   pPlugInGrid->setMargin(0);
   pPlugInGrid->setSpacing(5);
   pPlugInGrid->addWidget(pNameLabel, 0, 0);
   pPlugInGrid->addWidget(mpNameLabel, 0, 2);
   pPlugInGrid->addWidget(pVersionLabel, 1, 0);
   pPlugInGrid->addWidget(mpVersionLabel, 1, 2);
   pPlugInGrid->addWidget(pProductionStatusLabel, 2, 0);
   pPlugInGrid->addWidget(mpProductionStatusLabel, 2, 2);
   pPlugInGrid->addWidget(pCreatorLabel, 3, 0);
   pPlugInGrid->addWidget(mpCreatorLabel, 3, 2);
   pPlugInGrid->addWidget(pCopyrightLabel, 4, 0);
   pPlugInGrid->addWidget(mpCopyrightLabel, 4, 2);
   pPlugInGrid->addWidget(pDescriptionLabel, 5, 0, Qt::AlignTop);
   pPlugInGrid->addWidget(mpDescriptionLabel, 5, 2);
   pPlugInGrid->addWidget(pShortDescriptionLabel, 6, 0);
   pPlugInGrid->addWidget(mpShortDescriptionLabel, 6, 2);
   pPlugInGrid->addWidget(pTypeLabel, 7, 0);
   pPlugInGrid->addWidget(mpTypeLabel, 7, 2);
   pPlugInGrid->addWidget(pSubtypeLabel, 8, 0);
   pPlugInGrid->addWidget(mpSubtypeLabel, 8, 2);
   pPlugInGrid->addWidget(pMultipleInstancesLabel, 9, 0);
   pPlugInGrid->addWidget(mpMultipleInstancesLabel, 9, 2);
   pPlugInGrid->addWidget(pRunningInstancesLabel, 10, 0);
   pPlugInGrid->addWidget(mpRunningInstancesLabel, 10, 2);
   pPlugInGrid->setColumnMinimumWidth(1, 15);
   pPlugInGrid->setColumnStretch(2, 10);

   // Executable
   QWidget* pExecutableWidget = new QWidget(this);
   QLabel* pStartupLabel = new QLabel("Executed on Startup:", pExecutableWidget);
   mpStartupLabel = new QLabel(pExecutableWidget);
   QLabel* pDestroyedLabel = new QLabel("Destroyed After Executed:", pExecutableWidget);
   mpDestroyedLabel = new QLabel(pExecutableWidget);
   QLabel* pMenuLocationLabel = new QLabel("Menu Location(s):", pExecutableWidget);
   mpMenuLocationLabel = new QLabel(pExecutableWidget);
   QLabel* pMenuIconLabel = new QLabel("Menu Icon:", pExecutableWidget);
   mpMenuIconLabel = new QLabel(pExecutableWidget);
   QLabel* pAbortLabel = new QLabel("Abort Support:", pExecutableWidget);
   mpAbortLabel = new QLabel(pExecutableWidget);
   QLabel* pWizardLabel = new QLabel("Wizard Support:", pExecutableWidget);
   mpWizardLabel = new QLabel(pExecutableWidget);
   QLabel* pBatchModeLabel = new QLabel("Batch Mode Support:", pExecutableWidget);
   mpBatchModeLabel = new QLabel(pExecutableWidget);
   QLabel* pInteractiveModeLabel = new QLabel("Interactive Mode Support:", pExecutableWidget);
   mpInteractiveModeLabel = new QLabel(pExecutableWidget);

   QLabel* pArgsLabel = new QLabel("Input/Output Arguments:", pExecutableWidget);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");
   columnNames.append("Description");

   mpArgsTree = new QTreeWidget(pExecutableWidget);
   mpArgsTree->setColumnCount(3);
   mpArgsTree->setHeaderLabels(columnNames);
   mpArgsTree->setSelectionMode(QAbstractItemView::NoSelection);
   mpArgsTree->setSortingEnabled(false);
   mpArgsTree->setRootIsDecorated(true);
   mpArgsTree->setMinimumHeight(100);

   QHeaderView* pHeader = mpArgsTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setSortIndicatorShown(false);
      pHeader->resizeSection(0, 150);
   }

   mpExecutableSection = new LabeledSection(pExecutableWidget, "Execution", this);

   QGridLayout* pExecutableGrid = new QGridLayout(pExecutableWidget);
   pExecutableGrid->setMargin(0);
   pExecutableGrid->setSpacing(5);
   pExecutableGrid->addWidget(pStartupLabel, 0, 0);
   pExecutableGrid->addWidget(mpStartupLabel, 0, 2);
   pExecutableGrid->addWidget(pDestroyedLabel, 1, 0);
   pExecutableGrid->addWidget(mpDestroyedLabel, 1, 2);
   pExecutableGrid->addWidget(pMenuLocationLabel, 2, 0, Qt::AlignTop);
   pExecutableGrid->addWidget(mpMenuLocationLabel, 2, 2);
   pExecutableGrid->addWidget(pMenuIconLabel, 3, 0);
   pExecutableGrid->addWidget(mpMenuIconLabel, 3, 2);
   pExecutableGrid->addWidget(pAbortLabel, 4, 0);
   pExecutableGrid->addWidget(mpAbortLabel, 4, 2);
   pExecutableGrid->addWidget(pWizardLabel, 5, 0);
   pExecutableGrid->addWidget(mpWizardLabel, 5, 2);
   pExecutableGrid->addWidget(pBatchModeLabel, 6, 0);
   pExecutableGrid->addWidget(mpBatchModeLabel, 6, 2);
   pExecutableGrid->addWidget(pInteractiveModeLabel, 7, 0);
   pExecutableGrid->addWidget(mpInteractiveModeLabel, 7, 2);
   pExecutableGrid->addWidget(pArgsLabel, 8, 0, Qt::AlignTop);
   pExecutableGrid->addWidget(mpArgsTree, 8, 2);
   pExecutableGrid->setColumnMinimumWidth(1, 15);
   pExecutableGrid->setRowStretch(8, 10);
   pExecutableGrid->setColumnStretch(2, 10);

   // Importer
   QWidget* pImporterWidget = new QWidget(this);
   QLabel* pImportExtensionsLabel = new QLabel("File Extensions:", pImporterWidget);
   mpImportExtensionsEdit = new QTextEdit(pImporterWidget);
   mpImportExtensionsEdit->setLineWrapMode(QTextEdit::NoWrap);
   mpImportExtensionsEdit->setTextInteractionFlags(Qt::NoTextInteraction);
   mpImportExtensionsEdit->setMinimumHeight(100);
   mpImporterSection = new LabeledSection(pImporterWidget, "Import", this);

   QGridLayout* pImporterGrid = new QGridLayout(pImporterWidget);
   pImporterGrid->setMargin(0);
   pImporterGrid->setSpacing(5);
   pImporterGrid->addWidget(pImportExtensionsLabel, 0, 0, Qt::AlignTop);
   pImporterGrid->addWidget(mpImportExtensionsEdit, 0, 1);
   pImporterGrid->setRowStretch(0, 10);
   pImporterGrid->setColumnStretch(1, 10);

   // Exporter
   QWidget* pExporterWidget = new QWidget(this);
   QLabel* pExportExtensionsLabel = new QLabel("File Extensions:", pExporterWidget);
   mpExportExtensionsEdit = new QTextEdit(pExporterWidget);
   mpExportExtensionsEdit->setLineWrapMode(QTextEdit::NoWrap);
   mpExportExtensionsEdit->setTextInteractionFlags(Qt::NoTextInteraction);
   mpExportExtensionsEdit->setMinimumHeight(100);
   mpExporterSection = new LabeledSection(pExporterWidget, "Export", this);

   QGridLayout* pExporterGrid = new QGridLayout(pExporterWidget);
   pExporterGrid->setMargin(0);
   pExporterGrid->setSpacing(5);
   pExporterGrid->addWidget(pExportExtensionsLabel, 0, 0, Qt::AlignTop);
   pExporterGrid->addWidget(mpExportExtensionsEdit, 0, 1);
   pExporterGrid->setRowStretch(0, 10);
   pExporterGrid->setColumnStretch(1, 10);

   // Interpreter
   QWidget* pInterpreterWidget = new QWidget(this);
   QLabel* pInterpreterExtensionsLabel = new QLabel("File Extensions:", pInterpreterWidget);
   mpInterpreterExtensionsEdit = new QTextEdit(pInterpreterWidget);
   mpInterpreterExtensionsEdit->setLineWrapMode(QTextEdit::NoWrap);
   mpInterpreterExtensionsEdit->setTextInteractionFlags(Qt::NoTextInteraction);
   mpInterpreterExtensionsEdit->setMinimumHeight(100);
   mpInterpreterSection = new LabeledSection(pInterpreterWidget, "Interpreter", this);

   QGridLayout* pInterpreterGrid = new QGridLayout(pInterpreterWidget);
   pInterpreterGrid->setMargin(0);
   pInterpreterGrid->setSpacing(5);
   pInterpreterGrid->addWidget(pInterpreterExtensionsLabel, 0, 0, Qt::AlignTop);
   pInterpreterGrid->addWidget(mpInterpreterExtensionsEdit, 0, 1);
   pInterpreterGrid->setRowStretch(0, 10);
   pInterpreterGrid->setColumnStretch(1, 10);

   // Testable
   QWidget* pTestableWidget = new QWidget(this);
   QLabel* pTestableLabel = new QLabel("Test Support:", pTestableWidget);
   mpTestableLabel = new QLabel(pTestableWidget);
   mpTestableSection = new LabeledSection(pTestableWidget, "Test", this);

   QGridLayout* pTestableGrid = new QGridLayout(pTestableWidget);
   pTestableGrid->setMargin(0);
   pTestableGrid->setSpacing(5);
   pTestableGrid->addWidget(pTestableLabel, 0, 0);
   pTestableGrid->addWidget(mpTestableLabel, 0, 1);
   pTestableGrid->setRowStretch(0, 10);
   pTestableGrid->setColumnStretch(1, 10);

   // Initialization
   setSizeHint(600, 525);
}

PropertiesPlugInDescriptor::~PropertiesPlugInDescriptor()
{
}

bool PropertiesPlugInDescriptor::initialize(SessionItem* pSessionItem)
{
   PlugInDescriptorImp* pDescriptor = dynamic_cast<PlugInDescriptorImp*>(pSessionItem);
   if (pDescriptor == NULL)
   {
      return false;
   }

   // PlugIn
   mpNameLabel->setText(QString::fromStdString(pDescriptor->getName()));
   mpVersionLabel->setText(QString::fromStdString(pDescriptor->getVersion()));
   mpProductionStatusLabel->setText(pDescriptor->isProduction() ? "Yes" : "No");
   mpCreatorLabel->setText(QString::fromStdString(pDescriptor->getCreator()));
   mpCopyrightLabel->setText(QString::fromStdString(pDescriptor->getCopyright()));
   mpDescriptionLabel->setText(QString::fromStdString(pDescriptor->getDescription()));
   mpShortDescriptionLabel->setText(QString::fromStdString(pDescriptor->getShortDescription()));
   mpTypeLabel->setText(QString::fromStdString(pDescriptor->getType()));
   mpSubtypeLabel->setText(QString::fromStdString(pDescriptor->getSubtype()));
   mpMultipleInstancesLabel->setText(pDescriptor->areMultipleInstancesAllowed() ? "Yes" : "No");
   mpRunningInstancesLabel->setText(QString::number(pDescriptor->getNumPlugIns()));
   addSection(mpPlugInSection);

   // Executable
   if (pDescriptor->hasExecutableInterface() == true)
   {
      // Get the menu locations as a string
      QStringList menuLocations;

      const vector<string>& locations = pDescriptor->getMenuLocations();
      for (vector<string>::const_iterator iter = locations.begin(); iter != locations.end(); ++iter)
      {
         string location = *iter;
         if (location.empty() == false)
         {
            menuLocations.append(QString::fromStdString(location));
         }
      }

      QString menuLocationText = menuLocations.join("\n");

      // Get the menu icon
      QPixmap menuIcon;

      const QIcon icon = pDescriptor->getIcon();
      if (icon.isNull() == false)
      {
         menuIcon = icon.pixmap(QSize(16,16));
      }

      // Initialize the widgets
      mpStartupLabel->setText(pDescriptor->isExecutedOnStartup() ? "Yes" : "No");
      mpDestroyedLabel->setText(pDescriptor->isDestroyedAfterExecute() ? "Yes" : "No");
      mpMenuLocationLabel->setText(menuLocationText);
      mpMenuIconLabel->setPixmap(menuIcon);
      mpAbortLabel->setText(pDescriptor->hasAbort() ? "Yes" : "No");
      mpWizardLabel->setText(pDescriptor->hasWizardSupport() ? "Yes" : "No");
      mpBatchModeLabel->setText(pDescriptor->hasBatchSupport() ? "Yes" : "No");
      mpInteractiveModeLabel->setText(pDescriptor->hasInteractiveSupport() ? "Yes" : "No");

      if (pDescriptor->hasBatchSupport() == true)
      {
         QTreeWidgetItem* pBatchItem = new QTreeWidgetItem(mpArgsTree);
         if (pBatchItem != NULL)
         {
            pBatchItem->setText(0, "Batch Mode");

            // Input args
            const PlugInArgList* pInArgList = pDescriptor->getBatchInputArgList();
            initializeArgTree(pInArgList, true, pBatchItem);

            // Output args
            const PlugInArgList* pOutArgList = pDescriptor->getBatchOutputArgList();
            initializeArgTree(pOutArgList, false, pBatchItem);
         }
      }

      if (pDescriptor->hasInteractiveSupport() == true)
      {
         QTreeWidgetItem* pInteractiveItem = new QTreeWidgetItem(mpArgsTree);
         if (pInteractiveItem != NULL)
         {
            pInteractiveItem->setText(0, "Interactive Mode");

            // Input args
            const PlugInArgList* pInArgList = pDescriptor->getInteractiveInputArgList();
            initializeArgTree(pInArgList, true, pInteractiveItem);

            // Output args
            const PlugInArgList* pOutArgList = pDescriptor->getInteractiveOutputArgList();
            initializeArgTree(pOutArgList, false, pInteractiveItem);
         }
      }

      mpArgsTree->expandAll();
      addSection(mpExecutableSection, 1000);
   }
   else
   {
      mpExecutableSection->hide();
   }

   // Importer
   QString extensions = QString::fromStdString(pDescriptor->getFileExtensions());
   extensions.replace(";;", "\n");

   if (pDescriptor->hasImporterInterface() == true)
   {
      mpImportExtensionsEdit->setText(extensions);
      addSection(mpImporterSection, 1000);
   }
   else
   {
      mpImporterSection->hide();
   }

   // Exporter
   if (pDescriptor->hasExporterInterface() == true)
   {
      mpExportExtensionsEdit->setText(extensions);
      addSection(mpExporterSection, 1000);
   }
   else
   {
      mpExporterSection->hide();
   }

   // Interpreter
   if (pDescriptor->hasInterpreterInterface() == true)
   {
      mpInterpreterExtensionsEdit->setText(extensions);
      addSection(mpInterpreterSection, 1000);
   }
   else
   {
      mpInterpreterSection->hide();
   }

   // Testable
   if (pDescriptor->hasTestableInterface() == true)
   {
      mpTestableLabel->setText(pDescriptor->isTestable() ? "Yes" : "No");
      addSection(mpTestableSection);
   }
   else
   {
      mpTestableSection->hide();
   }

   addStretch(1);
   return true;
}

bool PropertiesPlugInDescriptor::applyChanges()
{
   // No modifications to the plug-in descriptor are needed
   return true;
}

const string& PropertiesPlugInDescriptor::getName()
{
   static string name = "Plug-In Descriptor Properties";
   return name;
}

const string& PropertiesPlugInDescriptor::getPropertiesName()
{
   static string propertiesName = "General";
   return propertiesName;
}

const string& PropertiesPlugInDescriptor::getDescription()
{
   static string description = "General setting properties of a plug-in descriptor";
   return description;
}

const string& PropertiesPlugInDescriptor::getShortDescription()
{
   static string description = "General setting properties of a plug-in descriptor";
   return description;
}

const string& PropertiesPlugInDescriptor::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesPlugInDescriptor::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesPlugInDescriptor::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesPlugInDescriptor::getDescriptorId()
{
   static string id = "{F60EFEFF-9FE9-4DED-8715-E9C8CDD09651}";
   return id;
}

bool PropertiesPlugInDescriptor::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesPlugInDescriptor::initializeArgTree(const PlugInArgList* pArgList, bool bInputArgs,
                                                   QTreeWidgetItem* pParentItem)
{
   if ((pArgList == NULL) || (pParentItem == NULL))
   {
      return;
   }

   unsigned short numArgs = pArgList->getCount();
   if (numArgs == 0)
   {
      return;
   }

   QTreeWidgetItem* pListItem = new QTreeWidgetItem(pParentItem);
   if (bInputArgs == true)
   {
      pListItem->setText(0, "Input Arguments");
   }
   else
   {
      pListItem->setText(0, "Output Arguments");
   }

   for (unsigned short i = 0; i < numArgs; ++i)
   {
      PlugInArg* pArg = NULL;
      if ((pArgList->getArg(i, pArg) == true) && (pArg != NULL))
      {
         const string& argName = pArg->getName();
         const string& argType = pArg->getType();
         const string& argDescription = pArg->getDescription();

         if (argName.empty() == false)
         {
            QTreeWidgetItem* pArgItem = new QTreeWidgetItem(pListItem);
            if (pArgItem != NULL)
            {
               pArgItem->setText(0, QString::fromStdString(argName));
               pArgItem->setText(1, QString::fromStdString(argType));
               pArgItem->setText(2, QString::fromStdString(argDescription));
            }
         }
      }
   }
}
