/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Aeb.h"
#include "AebIo.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "ExtensionListDelegate.h"
#include "ExtensionListDialog.h"
#include "ExtensionListItem.h"
#include "InstallerServices.h"
#include "InstallWizard.h"
#include "ProgressResource.h"

#include <QtGui/QDialogButtonBox>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QStandardItemModel>
#include <QtGui/QVBoxLayout>

ExtensionListDialog::ExtensionListDialog(QWidget* pParent) : QDialog(pParent)
{
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
      pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
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
   dlg.setFilters(QStringList() << "Extension Bundles (*.aeb)" << "Extension Metadata (install.rdf)" << "All Files (*)");
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
            "Invalid extension bundle.\n" + QString::fromStdString(errMsg));
      }
      else
      {
         extensions.push_back(extensionRes.back());
      }
   }
   if (!extensions.empty())
   {
      ProgressResource progress("Install extensions");
      InstallWizard wiz(extensions, progress.get(), this);
      wiz.exec();
   }
   reloadExtensions();
}
