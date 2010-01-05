/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>

#include "ApplicationWindow.h"
#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "CustomTreeWidget.h"
#include "DesktopServices.h"
#include "FileBrowser.h"
#include "Filename.h"
#include "LabeledSection.h"
#include "MessageLogMgrImp.h"
#include "ObjectResource.h"
#include "OptionsFileLocations.h"
#include "ProductView.h"
#include "UtilityServices.h"

#include <string>
using namespace std;

const QString OptionsFileLocations::sBookmarkListSentinal = "Enter path...";

namespace
{
   LabeledSection* createLabeledSection(CustomTreeWidget* pTree,
      const QStringList& columnNames, const QString& text, QWidget* pParent)
   {
      VERIFYRV(pTree != NULL, NULL);
      pTree->setColumnCount(columnNames.count());
      pTree->setHeaderLabels(columnNames);
      pTree->setRootIsDecorated(false);
      pTree->setSelectionMode(QAbstractItemView::SingleSelection);
      pTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);

      QHeaderView* pHeader = pTree->header();
      if (pHeader != NULL)
      {
         pHeader->setSortIndicatorShown(true);
         pHeader->setMovable(false);
         pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      }

      return new LabeledSection(pTree, text, pParent);
   }
}

OptionsFileLocations::OptionsFileLocations() :
   LabeledSectionGroup(NULL)
{
   // Path locations
   QStringList columnNames;
   columnNames.append("Type");
   columnNames.append("Location");

   mpPathTree = new CustomTreeWidget(this);
   addSection(createLabeledSection(mpPathTree, columnNames, "Default Paths", this), 1000);

   // File locations
   columnNames.append("Arguments");
   mpFileTree = new CustomTreeWidget(this);
   addSection(createLabeledSection(mpFileTree, columnNames, "Default Files", this), 1000);
   addStretch(1);

   mFileLocations.push_back(FileLocationDescriptor("Default Export Path",
      ConfigurationSettings::getSettingExportPathKey()));

   mFileLocations.push_back(FileLocationDescriptor("Default Import Path",
      ConfigurationSettings::getSettingImportPathKey()));

   mFileLocations.push_back(FileLocationDescriptor("Default Session Save/Open Path",
      ConfigurationSettings::getSettingSaveOpenSessionPathKey()));

   FileBrowser* pTemplateFileBrowser = new FileBrowser(mpFileTree);
   pTemplateFileBrowser->setBrowseCaption("Select Template File");
   pTemplateFileBrowser->setBrowseFileFilters("Template Files (*.spg);;All Files (*)");
   pTemplateFileBrowser->hide();
   mFileLocations.push_back(FileLocationDescriptor("Default Product Template",
      ProductView::getSettingTemplateFileKey(), pTemplateFileBrowser));

   mFileLocations.push_back(FileLocationDescriptor("Message Log Path",
      ConfigurationSettings::getSettingMessageLogPathKey()));

   mFileLocations.push_back(FileLocationDescriptor("Product Template Path",
      ProductView::getSettingTemplatePathKey()));

   mFileLocations.push_back(FileLocationDescriptor("Temp Path",
      ConfigurationSettings::getSettingTempPathKey()));

   mFileLocations.push_back(FileLocationDescriptor("Support Files Path",
      ConfigurationSettings::getSettingSupportFilesPathKey()));

   mFileLocations.push_back(FileLocationDescriptor("Wizard Path",
      ConfigurationSettings::getSettingWizardPathKey()));

   FileBrowser* pTextEditorBrowser = new FileBrowser(mpFileTree);
   pTextEditorBrowser->setBrowseCaption("Select Text Editor");
   pTextEditorBrowser->setToolTip(
      "If %1 is specified as an argument it will be replaced by the name of the file to be edited at run time.\n"
      "Otherwise the name of the file will be appended to the command line after all of the specified arguments.");
#if defined(WIN_API)
   pTextEditorBrowser->setBrowseFileFilters("Text Editors (*.exe);;All Files (*)");
#endif

   pTextEditorBrowser->hide();
   mFileLocations.push_back(FileLocationDescriptor("Text Editor", ConfigurationSettings::getSettingTextEditorKey(),
      pTextEditorBrowser, ConfigurationSettings::getSettingTextEditorArgumentsKey()));

   Service<ConfigurationSettings> pSettings;
   for (vector<FileLocationDescriptor>::iterator iter = mFileLocations.begin(); iter != mFileLocations.end(); ++iter)
   {
      QString dir;
      const Filename* pFilename = dv_cast<Filename>(&pSettings->getSetting(iter->getKey()));
      if (pFilename != NULL)
      {
         dir = QString::fromStdString(pFilename->getFullPathAndName());
         dir.replace(QRegExp("\\\\"), "/");
      }

      QTreeWidgetItem* pItem = new QTreeWidgetItem(iter->getFileBrowser() == NULL ? mpPathTree : mpFileTree);
      if (pItem != NULL)
      {
         pItem->setText(0, QString::fromStdString(iter->getText()));
         pItem->setText(1, dir);

         if (iter->getFileBrowser() == NULL)
         {
             mpPathTree->setCellWidgetType(pItem, 1, CustomTreeWidget::BROWSE_DIR_EDIT);
         }
         else
         {
            mpFileTree->setCellWidgetType(pItem, 1, CustomTreeWidget::BROWSE_FILE_EDIT);
            mpFileTree->setFileBrowser(pItem, 1, iter->getFileBrowser());
            if (iter->getArgumentKey().empty())
            {
               pItem->setBackgroundColor(2, QColor(235, 235, 235));
            }
            else
            {
               const string& arguments = dv_cast<string>(pSettings->getSetting(iter->getArgumentKey()));
               mpFileTree->setCellWidgetType(pItem, 2, CustomTreeWidget::LINE_EDIT);
               pItem->setText(2, QString::fromStdString(arguments));
            }
         }
      }

      if (iter->getText() == "Wizard Path")
      {
         mWizardPath = pFilename->getFullPathAndName();
      }
   }

   mpPathTree->resizeColumnToContents(0);
   mpFileTree->resizeColumnToContents(0);
}

void OptionsFileLocations::applyChanges()
{
   applyChanges(mpFileTree);
   applyChanges(mpPathTree);
}

void OptionsFileLocations::applyChanges(CustomTreeWidget* pTree)
{
   VERIFYNR(pTree != NULL);
   Service<ConfigurationSettings> pSettings;
   QTreeWidgetItemIterator iter(pTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         string type = pItem->text(0).toStdString();
         string confSettingsKey;
         string confSettingsArgumentKey;
         for (vector<FileLocationDescriptor>::iterator iter2 = mFileLocations.begin();
            iter2 != mFileLocations.end();
            ++iter2)
         {
            if (iter2->getText() == type)
            {
               confSettingsKey = iter2->getKey();
               confSettingsArgumentKey = iter2->getArgumentKey();
               break;
            }
         }
         if (!confSettingsKey.empty())
         {
            QString strLocation = pItem->text(1);
            strLocation.replace(QRegExp("\\\\"), "/");
            FactoryResource<Filename> pFilename;
            string location = strLocation.toStdString();
            pFilename->setFullPathAndName(location);
            pSettings->setSetting(confSettingsKey, *(pFilename.get()));

            if (type == "Message Log Path")
            {
               MessageLogMgrImp::instance()->setPath(location);
            }
            else if (type == "Wizard Path")
            {
               Service<DesktopServices> pDesktop;
               ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(pDesktop->getMainWidget());
               if ((location != mWizardPath) && (pAppWindow != NULL))
               {
                  pAppWindow->updateWizardCommands();
               }
            }
         }
         if (!confSettingsArgumentKey.empty())
         {
            pSettings->setSetting(confSettingsArgumentKey, pItem->text(2).toStdString());
         }
      }
      ++iter;
   }
}

OptionsFileLocations::~OptionsFileLocations()
{
}
