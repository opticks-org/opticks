/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXTENSIONLISTDELEGATE_H__
#define EXTENSIONLISTDELEGATE_H__

#include <QtGui/QAbstractItemDelegate>

class ExtensionListDelegate : public QAbstractItemDelegate
{
   Q_OBJECT

public:
   enum Role { NameRole = Qt::DisplayRole,
               DescriptionRole = Qt::ToolTipRole,
               VersionRole = Qt::UserRole + 1,
               ExtensionIdRole = Qt::UserRole + 2,
               IconRole = Qt::DecorationRole,
               UninstallPendingRole = Qt::UserRole + 3 };

   ExtensionListDelegate(QObject* pParent = NULL, bool allowEditing = true);
   virtual ~ExtensionListDelegate();

   virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const;
   virtual void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const;
   virtual void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
   virtual void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
   bool mAllowEditing;
};

#endif