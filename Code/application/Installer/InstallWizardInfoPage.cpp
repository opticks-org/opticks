/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QVBoxLayout>

#include "Aeb.h"
#include "ExtensionListDelegate.h"
#include "ExtensionListItem.h"
#include "InstallWizardInfoPage.h"

InstallWizardInfoPage::InstallWizardInfoPage(const QList<Aeb*>& packageDescriptors, QWidget* pParent) :
   QWizardPage(pParent)
{
   setTitle("General Information");
   setSubTitle("This wizard will guide you through the installation of the following plug-in suites.");
   mpList = new QListWidget(this);
   mpList->setItemDelegate(new ExtensionListDelegate(this, false));
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->addWidget(mpList);

   foreach (Aeb* pDescriptor, packageDescriptors)
   {
      if (pDescriptor == NULL)
      {
         continue;
      }
      QListWidgetItem* pItem = new QListWidgetItem(mpList);
      pItem->setData(ExtensionListDelegate::NameRole, QString::fromStdString(pDescriptor->getName()));
      pItem->setData(ExtensionListDelegate::DescriptionRole, QString::fromStdString(pDescriptor->getDescription()));
      pItem->setData(ExtensionListDelegate::VersionRole, QString::fromStdString(pDescriptor->getVersion().toString()));
      pItem->setData(ExtensionListDelegate::ExtensionIdRole, QString::fromStdString(pDescriptor->getId()));
      pItem->setData(ExtensionListDelegate::IconRole, pDescriptor->getIcon());
      pItem->setData(ExtensionListDelegate::UninstallPendingRole, false);
      pItem->setFlags(Qt::ItemIsEnabled);
   }
}

InstallWizardInfoPage::~InstallWizardInfoPage()
{
}

bool InstallWizardInfoPage::eventFilter(QObject* pObject, QEvent* pEvent)
{
   ExtensionListItem* pEditor = qobject_cast<ExtensionListItem*>(pObject);
   if (pEditor != NULL && pEvent->type() == QEvent::FocusOut)
   {
      return true;
   }
   return false;
}