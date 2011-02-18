/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Aeb.h"
#include "ExtensionListItem.h"
#include "InstallerServices.h"
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

ExtensionListItem::ExtensionListItem(bool editor, bool showUpdateInfo, QWidget* pParent) : QWidget(pParent)
{
   //KIP - Fix up the layout of this widget.
   QVBoxLayout* pTopLevel = new QVBoxLayout(this);

   QHBoxLayout* pInfo = new QHBoxLayout;
   pTopLevel->addLayout(pInfo);
   mpIcon = new QLabel("pix", this);
   pInfo->addWidget(mpIcon);
   pInfo->addSpacing(10);

   QVBoxLayout* pText = new QVBoxLayout;
   pInfo->addLayout(pText);
   pInfo->addStretch();

   QHBoxLayout* pFirst = new QHBoxLayout;
   pText->addLayout(pFirst);
   mpName = new QLabel(this);
   mpName->setWordWrap(true);
   QFont boldFont = mpName->font();
   boldFont.setBold(true);
   mpName->setFont(boldFont);
   pFirst->addWidget(mpName);
   mpVersion = new QLabel(this);
   pFirst->addWidget(mpVersion);

   mpDescription = new QLabel(this);
   mpDescription->setWordWrap(true);
   pText->addWidget(mpDescription);

   if (showUpdateInfo)
   {
      mpUpdateInfo = new QLabel(this);
      mpUpdateInfo->setFont(boldFont);
      pText->addWidget(mpUpdateInfo);
   }
   else
   {
      mpUpdateInfo = NULL;
   }

   if (editor)
   {
      mpButtons = new QDialogButtonBox(this);
      mpButtons->addButton("Disable", QDialogButtonBox::ActionRole)->setEnabled(false);
      mpUninstallButton = mpButtons->addButton("Uninstall", QDialogButtonBox::DestructiveRole);
      connect(mpUninstallButton, SIGNAL(clicked()), this, SLOT(uninstall()));
      mpButtons->addButton("About", QDialogButtonBox::HelpRole);
      mpButtons->addButton("Update", QDialogButtonBox::ApplyRole)->setEnabled(false);
      pTopLevel->addWidget(mpButtons);
      connect(mpButtons, SIGNAL(helpRequested()), this, SLOT(about()));
   }
   else
   {
      mpButtons = NULL;
      mpUninstallButton = NULL;
   }
   pTopLevel->addStretch();
}

ExtensionListItem::~ExtensionListItem()
{
}

QString ExtensionListItem::getName() const
{
   return mpName->text();
}

QString ExtensionListItem::getDescription() const
{
   return mpDescription->text();
}

QString ExtensionListItem::getVersion() const
{
   return mpVersion->text();
}

QPixmap ExtensionListItem::getIcon() const
{
   const QPixmap* pPix = mpIcon->pixmap();
   return pPix == NULL ? QPixmap() : *pPix;
}

QString ExtensionListItem::getId() const
{
   return mExtensionId;
}

QString ExtensionListItem::getUpdateInfo() const
{
   if (mpUpdateInfo == NULL)
   {
      return QString();
   }
   return mpUpdateInfo->text();
}

bool ExtensionListItem::getUninstallable() const
{
   if (mpUninstallButton == NULL)
   {
      return true;
   }
   return mpUninstallButton->isEnabled();
}

void ExtensionListItem::setName(const QString& v)
{
   mpName->setText(v);
}

void ExtensionListItem::setDescription(const QString& v)
{
   mpDescription->setText(v);
}

void ExtensionListItem::setVersion(const QString& v)
{
   mpVersion->setText(v);
}

void ExtensionListItem::setIcon(const QPixmap& v)
{
   const QPixmap* pPix = mpIcon->pixmap();
   if (pPix != NULL)
   {
      mpIcon->setPixmap(v.scaled(pPix->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
   }
   else
   {
      mpIcon->setPixmap(v);
   }
}

void ExtensionListItem::setId(const QString& v)
{
   mExtensionId = v;
}

void ExtensionListItem::setUpdateInfo(const QString& v)
{
   if (mpUpdateInfo != NULL)
   {
      mpUpdateInfo->setText(v);
   }
}

void ExtensionListItem::accepted()
{
   emit finished(this);
}

void ExtensionListItem::about()
{
   const Aeb* pExtension = Service<InstallerServices>()->getAeb(mExtensionId.toStdString());
   if (pExtension != NULL && pExtension->validate())
   {
      QMessageBox::about(this, QString("About %1").arg(QString::fromStdString(pExtension->getName())),
         QString("<h3>%1</h3>version %2<br>%3<hr><b>Created By:</b><br>%4<br><a href=\"%5\">Visit Home Page</a>")
         .arg(QString::fromStdString(pExtension->getName()))
         .arg(QString::fromStdString(pExtension->getVersion().toString()))
         .arg(QString::fromStdString(pExtension->getDescription()))
         .arg(QString::fromStdString(pExtension->getCreator()))
         .arg(QString::fromStdString(pExtension->getHomepageURL())));
   }
}

void ExtensionListItem::uninstall()
{
   std::string errMsg;
   if (Service<InstallerServices>()->uninstallExtension(getId().toStdString(), errMsg))
   {
      setUninstallable(false);
   }
   else
   {
      QMessageBox::warning(this, "Can't uninstall extension", "Can't uninstall extension.\n" + QString::fromStdString(errMsg));
   }
}

void ExtensionListItem::setUninstallable(bool v)
{
   if (mpUninstallButton != NULL)
   {
      mpUninstallButton->setEnabled(v);
   }
   if (!v)
   {
      setUpdateInfo("This extension will be uninstalled when Opticks is restarted.");
   }
   else
   {
      setUpdateInfo("");
   }
   update();
}