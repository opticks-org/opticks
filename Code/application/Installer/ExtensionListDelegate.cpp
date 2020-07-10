/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ExtensionListDelegate.h"
#include "ExtensionListDialog.h"
#include "ExtensionListItem.h"
#include <QtCore/QEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>

#include <memory>

ExtensionListDelegate::ExtensionListDelegate(QObject* pParent, bool allowEditing) :
   QAbstractItemDelegate(pParent), mAllowEditing(allowEditing)
{
}

ExtensionListDelegate::~ExtensionListDelegate()
{
}

QWidget* ExtensionListDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   ExtensionListItem* pEditor = new ExtensionListItem(true, mAllowEditing, pParent);
   connect(pEditor, SIGNAL(finished(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
   return pEditor;
}

void ExtensionListDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
   pEditor->setProperty("name", index.model()->data(index, NameRole));
   pEditor->setProperty("description", index.model()->data(index, DescriptionRole));
   pEditor->setProperty("version", index.model()->data(index, VersionRole));
   pEditor->setProperty("icon", index.model()->data(index, IconRole));
   pEditor->setProperty("id", index.model()->data(index, ExtensionIdRole));
   if (qvariant_cast<bool>(index.model()->data(index, UninstallPendingRole)))
   {
      pEditor->setProperty("uninstallable", false);
   }
   else
   {
      pEditor->setProperty("uninstallable", true);
   }
   pEditor->setProperty("useUninstallHelper", qvariant_cast<bool>(index.model()->data(index, UseUninstallHelperRole)));
}

void ExtensionListDelegate::setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const
{
   bool uninstallable = pEditor->property("uninstallable").toBool();
   bool useHelper = pEditor->property("useUninstallHelper").toBool();
   pModel->setData(index, !uninstallable, UninstallPendingRole);
   pModel->setData(index, useHelper, UseUninstallHelperRole);
}

void ExtensionListDelegate::updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   QRect editRect = option.rect;
   editRect.setHeight(editRect.height() - 1.0); //leave space for border
   pEditor->setGeometry(editRect);
}

QSize ExtensionListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   std::auto_ptr<ExtensionListItem> pWidget(new ExtensionListItem(mAllowEditing, mAllowEditing));
   setEditorData(pWidget.get(), index);
   return pWidget->sizeHint();
}

void ExtensionListDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   std::auto_ptr<ExtensionListItem> pWidget(new ExtensionListItem(false, mAllowEditing));
   setEditorData(pWidget.get(), index);
   pWidget->resize(option.rect.width(), option.rect.height());
   QRect pixMapRect;
   pixMapRect.setWidth(option.rect.width());
   pixMapRect.setHeight(option.rect.height());
//VS2017   QPixmap pix = QPixmap::grabWidget(pWidget.get());
   QPixmap pix = pWidget.get()->grab();
   pPainter->save();
   pPainter->drawPixmap(option.rect, pix);
   pPainter->setPen(QPen(option.palette.mid(), 1.0, Qt::DashLine));
   pPainter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
   pPainter->restore();
}

void ExtensionListDelegate::commit()
{
   QWidget* pEditor = qobject_cast<QWidget*>(sender());
   emit commitData(pEditor);
}