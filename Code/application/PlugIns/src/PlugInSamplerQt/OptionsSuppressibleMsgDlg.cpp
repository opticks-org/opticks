/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QStringList>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "ConfigurationSettings.h"
#include "CustomTreeWidget.h"
#include "DesktopServices.h"
#include "LabeledSection.h"
#include "OptionsSuppressibleMsgDlg.h"

#include <string>
using namespace std;

OptionsSuppressibleMsgDlg::OptionsSuppressibleMsgDlg() :
   LabeledSectionGroup(NULL)
{
   mpDialogTree = new CustomTreeWidget(this);
   mpDialogTree->setRootIsDecorated(false);
   mpDialogTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpDialogTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);

   QHeaderView* pHeader = mpDialogTree->header();
   if (pHeader != NULL)
   {
      pHeader->hide();
   }

   LabeledSection* pDialogSection = new LabeledSection(mpDialogTree, "Suppressible Message Dialogs", this);
   
   addSection(pDialogSection, 10);
   
   mDialogList.push_back(std::pair<string, string>("This is a test dialog", "{E2D52CF8-14C4-462a-AECD-27DAF74EA354}"));
   mDialogList.push_back(std::pair<string, string>("This is another test dialog", "{6E22C35B-6B64-458c-9C41-E1BF9B1E6861}"));

   Service<DesktopServices> pDesktop;
   for (std::vector<std::pair<string, string>>::iterator iter = mDialogList.begin(); iter != mDialogList.end(); ++iter)
   {
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpDialogTree);
      if (pItem != NULL)
      { 
         bool state = pDesktop->getSuppressibleMsgDlgState(iter->second);
         pItem->setText(0, QString::fromStdString(iter->first));
         mpDialogTree->setCellCheckState(pItem, 0, state ? CustomTreeWidget::CHECKED : CustomTreeWidget::UNCHECKED);
      }
   }
}

void OptionsSuppressibleMsgDlg::applyChanges()
{
   Service<DesktopServices> pDesktop;
   QTreeWidgetItemIterator iter(mpDialogTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         string msg = pItem->text(0).toStdString();
         for (vector<pair<string, string> >::iterator iter2 = mDialogList.begin(); iter2 != mDialogList.end(); ++iter2)
         {
            if (iter2->first == msg)
            {
               CustomTreeWidget::CheckState state = mpDialogTree->getCellCheckState(pItem, 0);

               if (state == CustomTreeWidget::CHECKED)
               {
                  pDesktop->setSuppressibleMsgDlgState(iter2->second, true);
               }
               else if (state == CustomTreeWidget::UNCHECKED)
               {
                  pDesktop->setSuppressibleMsgDlgState(iter2->second, false);
               }

               break;
            }
         }
      }
      ++iter;
   }
}

OptionsSuppressibleMsgDlg::~OptionsSuppressibleMsgDlg() {}