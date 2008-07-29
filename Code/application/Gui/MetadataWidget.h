/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef METADATAWIDGET_H
#define METADATAWIDGET_H

#include <QtGui/QPushButton>
#include <QtGui/QTreeView>
#include <QtGui/QWidget>

class DynamicObject;
class DynamicObjectAdapter;
class DynamicObjectItemModel;
class QSortFilterProxyModel;

class MetadataWidget : public QWidget
{
   Q_OBJECT

public:
   MetadataWidget(QWidget* parent = 0);
   ~MetadataWidget();

   void setMetadata(DynamicObject* pMetadata);
   bool isModified() const;
   bool applyChanges();

   QSize sizeHint() const;

signals:
   void modified();

protected slots:
   void modifyValues();
   void addKey();
   void editCurrentValue();
   void deleteKey();
   void clearKeys();
   void currentChanged(const QModelIndex& selected, const QModelIndex& deselected);
   void editSelectedValue(const QModelIndex& selectedItem);

private:
   QTreeView* mpMetadataTree;
   DynamicObjectAdapter* mpObject;
   DynamicObject* mpMetadata;
   DynamicObjectItemModel* mpMetadataModel;
   QSortFilterProxyModel* mpMetadataSortingModel;

   bool mModified;
   static bool mEditWarning;

   QPushButton* mpModifyButton;
   QPushButton* mpAddChildButton;
   QPushButton* mpAddSiblingButton;
   QPushButton* mpEditButton;
   QPushButton* mpDeleteButton;
   QPushButton* mpClearButton;
};

#endif
