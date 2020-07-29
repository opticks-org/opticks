/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Aeb.h"
#include "AebIo.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "ExtensionListDelegate.h"
#include "ExtensionListDialog.h"
#include "ExtensionListItem.h"
#include "InstallerServices.h"
#include "InstallWizard.h"
#include "ProgressResource.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMessageBox>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QVBoxLayout>

ExtensionListDialog::ExtensionListDialog(QWidget* pParent) : QDialog(pParent)
{
   setWindowTitle("Extensions");
   mpExtensionList = new QListWidget(this);
   mpExtensionList->setItemDelegate(new ExtensionListDelegate(this));
   mpExtensionList->setEditTriggers(QAbstractItemView::CurrentChanged);
   reloadExtensions();

   QVBoxLayout* pTopLevel = new QVBoxLayout(this);
   pTopLevel->addWidget(mpExtensionList, 10);
   QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
   pButtons->addButton("&Install...", QDialogButtonBox::AcceptRole);
   pTopLevel->addWidget(pButtons);
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(install())));
   resize(480, 500);
}

ExtensionListDialog::~ExtensionListDialog()
{
}

void ExtensionListDialog::accept()
{
   QDialog::accept();
   if (Service<InstallerServices>()->useLaunchHelper())
   {
      std::vector<std::string> uninstallIds;
      for (int idx = 0; idx < mpExtensionList->count(); ++idx)
      {
         QListWidgetItem* pItem = mpExtensionList->item(idx);
         if (pItem != NULL && qvariant_cast<bool>(pItem->data(ExtensionListDelegate::UseUninstallHelperRole)))
         {
            QString extensionId(qvariant_cast<QString>(pItem->data(ExtensionListDelegate::ExtensionIdRole)));
            uninstallIds.push_back(extensionId.toStdString());
         }
      }
      if (!uninstallIds.empty() || !mPendingInstalls.empty())
      {
         if (Service<DesktopServices>()->showMessageBox("Elevate Privileges",
            std::string("Installing and uninstalling extensions requires Administrator privileges.\n")
            + APP_NAME + " will exit and update the extensions.\n"
            + "Are you sure you want to proceed?", "&Yes", "&No") == 0)
         {
            Service<InstallerServices>()->launchHelper(mPendingInstalls, uninstallIds);
            QCoreApplication::instance()->quit();
         }
      }
   }
}

void ExtensionListDialog::reloadExtensions()
{
   mpExtensionList->clear();
   // populate the list
   QList<const Aeb*> extensions = QList<const Aeb*>::fromStdList(Service<InstallerServices>()->getAebs());
   foreach (const Aeb* pExtension, extensions)
   {
      if (pExtension == NULL || pExtension->isHidden())
      {
         continue;
      }
      QListWidgetItem* pItem = new QListWidgetItem(mpExtensionList);
      pItem->setData(ExtensionListDelegate::NameRole, QString::fromStdString(pExtension->getName()));
      pItem->setData(ExtensionListDelegate::DescriptionRole, QString::fromStdString(pExtension->getDescription()));
      pItem->setData(ExtensionListDelegate::VersionRole, QString::fromStdString(pExtension->getVersion().toString()));
      pItem->setData(ExtensionListDelegate::ExtensionIdRole, QString::fromStdString(pExtension->getId()));
      pItem->setData(ExtensionListDelegate::IconRole, pExtension->getIcon());
      pItem->setData(ExtensionListDelegate::UninstallPendingRole, Service<InstallerServices>()->isPendingUninstall(pExtension->getId()));
      pItem->setData(ExtensionListDelegate::UseUninstallHelperRole, false);
      pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      mpExtensionList->openPersistentEditor(pItem);
   }
}

void ExtensionListDialog::install()
{
   static QByteArray state;
   QFileDialog dlg(this);
   dlg.setFileMode(QFileDialog::ExistingFiles);
   dlg.setAcceptMode(QFileDialog::AcceptOpen);
   dlg.setDefaultSuffix("*.aeb");
   dlg.setConfirmOverwrite(false);
   dlg.setReadOnly(true);
   dlg.setResolveSymlinks(true);
   dlg.setNameFilters(QStringList() << "Extension Bundles (*.aeb)" << "Extension Metadata (install.rdf)" << "All Files (*)");
   dlg.setWindowTitle("Select extensions");
   dlg.restoreState(state);

   if (dlg.exec() == QDialog::Rejected)
   {
      return;
   }
   state = dlg.saveState();
   QStringList filenames = dlg.selectedFiles();

   AebListResource extensionRes; //manage lifetime of Aeb*'s that are allocated on the heap
   QList<Aeb*> extensions;
   foreach (QString filename, filenames)
   {
      extensionRes.push_back(new Aeb()); //take ownership
      AebIo deserializer(*extensionRes.back());
      std::string errMsg;
      if (!deserializer.fromFile(filename.toStdString(), errMsg))
      {
         QMessageBox::critical(Service<DesktopServices>()->getMainWidget(), "Extension Error",
            "Invalid extension bundle: " + filename + ".\n" + QString::fromStdString(errMsg));
      }
      else
      {
         if (!extensionRes.back()->checkTargetApplication(errMsg))
         {
            QMessageBox::critical(Service<DesktopServices>()->getMainWidget(), "Extension Error",
               "Invalid extension bundle: " + QString::fromStdString(extensionRes.back()->getName()) + ".\n" + QString::fromStdString(errMsg));
         }
         else
         {
            extensions.push_back(extensionRes.back());
         }
      }
   }
   if (!extensions.empty())
   {
      ProgressResource progress("Install extensions");
      InstallWizard wiz(extensions, progress.get(), this);
      wiz.exec();
      std::vector<std::string> newPending = wiz.getPendingInstalls();
      mPendingInstalls.reserve(mPendingInstalls.size() + newPending.size());
      mPendingInstalls.insert(mPendingInstalls.end(), newPending.begin(), newPending.end());
   }
   reloadExtensions();
}
