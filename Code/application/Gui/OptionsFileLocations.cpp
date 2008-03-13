/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsFileLocations.h"

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
#include "ProductView.h"
#include "UtilityServices.h"

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

const QString OptionsFileLocations::sBookmarkListSentinal = "Enter path...";

OptionsFileLocations::OptionsFileLocations() :
   QWidget(NULL)
{
   // File Locations
   QStringList columnNames;
   columnNames.append("Type");
   columnNames.append("Location");

   mpFileTree = new CustomTreeWidget(this);
   mpFileTree->setColumnCount(columnNames.count());
   mpFileTree->setHeaderLabels(columnNames);
   mpFileTree->setRootIsDecorated(false);
   mpFileTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpFileTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);

   QHeaderView* pHeader = mpFileTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setMovable(false);
      pHeader->setResizeMode(1, QHeaderView::Stretch);
      pHeader->setStretchLastSection(false);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   }
   LabeledSection *pFileSection = new LabeledSection(mpFileTree, "Default File Locations", this);

   mpBookmarkList = new CustomTreeWidget(this);
   mpBookmarkList->setColumnCount(1);
   mpBookmarkList->setRootIsDecorated(false);
   mpBookmarkList->setGridlinesShown(Qt::Horizontal, true);
   mpBookmarkList->header()->hide();
   LabeledSection *pBookmarkSection = new LabeledSection(mpBookmarkList, "Path Bookmarks", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pFileSection, 10);
   pLayout->addWidget(pBookmarkSection, 10);

   mFileLocations.push_back(pair<string,string>("Default Import/Export Path", ConfigurationSettings::getSettingImportExportPathKey()));
   mFileLocations.push_back(pair<string,string>("Default Session Save/Open Path", ConfigurationSettings::getSettingSaveOpenSessionPathKey()));
   mFileLocations.push_back(pair<string,string>("Default Product Template", ProductView::getSettingTemplateFileKey()));
   mFileLocations.push_back(pair<string,string>("Message Log Path", ConfigurationSettings::getSettingMessageLogPathKey()));
   mFileLocations.push_back(pair<string,string>("Plug-In Path", ConfigurationSettings::getSettingPlugInPathKey()));
   mFileLocations.push_back(pair<string,string>("Product Template Path", ProductView::getSettingTemplatePathKey()));
   mFileLocations.push_back(pair<string,string>("Temp Path", ConfigurationSettings::getSettingTempPathKey()));
   mFileLocations.push_back(pair<string,string>("Support Files Path", ConfigurationSettings::getSettingSupportFilesPathKey()));
   mFileLocations.push_back(pair<string,string>("Wizard Path", ConfigurationSettings::getSettingWizardPathKey()));

   Service<ConfigurationSettings> pSettings;
   for (vector<pair<string,string> >::iterator iter = mFileLocations.begin();
        iter != mFileLocations.end(); ++iter)
   {
      const Filename* pFilename = dv_cast<Filename>(&pSettings->getSetting(iter->second));
      QString dir = "";
      if (pFilename != NULL)
      {
         dir = QString::fromStdString(pFilename->getFullPathAndName());
         dir.replace(QRegExp("\\\\"), "/");
      }
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpFileTree);
      if (pItem != NULL)
      {
         pItem->setText(0, QString::fromStdString(iter->first));
         pItem->setText(1, dir);

         if (iter->first != "Default Product Template")
         {
            mpFileTree->setCellWidgetType(pItem, 1, CustomTreeWidget::BROWSE_DIR_EDIT);
         }
         else
         {
            FileBrowser* pFileBrowser = new FileBrowser(mpFileTree);
            pFileBrowser->setBrowseCaption("Select Template File");
            pFileBrowser->setBrowseFileFilters("Template Files (*.spg);;All Files (*)");
            pFileBrowser->hide();

            mpFileTree->setCellWidgetType(pItem, 1, CustomTreeWidget::BROWSE_FILE_EDIT);
            mpFileTree->setFileBrowser(pItem, 1, pFileBrowser);
         }
      }

      if (iter->first == "Plug-In Path")
      {
         mPlugInPath = pFilename->getFullPathAndName();
      }
      else if (iter->first == "Wizard Path")
      {
         mWizardPath = pFilename->getFullPathAndName();
      }
   }

   vector<Filename*> pathBookmarks = Service<ConfigurationSettings>()->getSettingPathBookmarks();
   for(vector<Filename*>::iterator pathBookmark = pathBookmarks.begin(); pathBookmark != pathBookmarks.end(); ++pathBookmark)
   {
      Filename *pPath = *pathBookmark;
      if(pPath != NULL)
      {
         QTreeWidgetItem *pItem = new QTreeWidgetItem(mpBookmarkList);
         pItem->setText(0, QString::fromStdString(pPath->getFullPathAndName()));
         mpBookmarkList->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_DIR_EDIT);
      }
   }
   addBookmark();
   VERIFYNRV(connect(mpBookmarkList, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(addBookmark())));
}
   
void OptionsFileLocations::applyChanges()
{  
   mpFileTree->closeActiveCellWidget(true); //accept changes in any active cells

   Service<ConfigurationSettings> pSettings;
   QTreeWidgetItemIterator iter(mpFileTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         string type = pItem->text(0).toStdString();
         string confSettingsKey = "";
         for (vector<pair<string,string> >::iterator iter2 = mFileLocations.begin();
              iter2 != mFileLocations.end(); ++iter2)
         {
            if (iter2->first == type)
            {
               confSettingsKey = iter2->second;
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
            else if (type == "Plug-In Path")
            {
               if (location != mPlugInPath)
               {
                  Service<DesktopServices> pDesktop;
                  QMessageBox::information(pDesktop->getMainWidget(), APP_NAME, 
                     "Currently loaded plug-ins will not be updated until a new session is started");

               }
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
      }
      ++iter;
   }

   vector<Filename*> pathBookmarks;
   for(QTreeWidgetItemIterator iter(mpBookmarkList); *iter != NULL; ++iter)
   {
      QTreeWidgetItem *pItem = *iter;
      QString pathName(pItem->text(0));
      QDir path(pathName);
      if(pathName != sBookmarkListSentinal && path.exists())
      {
         FactoryResource<Filename> bookmark;
         if(bookmark.get() != NULL)
         {
            bookmark->setFullPathAndName(path.absolutePath().toStdString());
            pathBookmarks.push_back(bookmark.release());
         }
      }
   }
   Service<ConfigurationSettings>()->setSettingPathBookmarks(pathBookmarks);
}

OptionsFileLocations::~OptionsFileLocations()
{
}

void OptionsFileLocations::addBookmark()
{
   QTreeWidgetItem *pItem = NULL;
   QList<QTreeWidgetItem*> items = mpBookmarkList->findItems(sBookmarkListSentinal, Qt::MatchExactly);
   if(items.empty())
   {
      pItem = new QTreeWidgetItem(mpBookmarkList);
   }
   else
   {
      foreach(QTreeWidgetItem *pTmpItem, items)
      {
         if(pItem == NULL)
         {
            pItem = pTmpItem;
         }
         else
         {
            QTreeWidgetItem *pRemoveItem = mpBookmarkList->takeTopLevelItem(mpBookmarkList->indexOfTopLevelItem(pItem));
            if(pRemoveItem == pItem)
            {
               pItem = NULL;
            }
            delete pRemoveItem;
         }
      }
   }
   if(pItem != NULL)
   {
      pItem->setText(0, sBookmarkListSentinal);
      mpBookmarkList->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_DIR_EDIT);
      mpBookmarkList->setCurrentItem(pItem);
   }
   // now remove empty sections
   items = mpBookmarkList->findItems("", Qt::MatchExactly);
   foreach(QTreeWidgetItem *pTmpItem, items)
   {
      delete pTmpItem;
   }
}
