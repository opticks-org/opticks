/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURECLASSDLG_H
#define FEATURECLASSDLG_H

#include "DynamicObject.h"
#include "ObjectResource.h"

#include <QtGui/QDialog>
#include <QtGui/QStyledItemDelegate>

class QListWidget;
class QListWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;

class FeatureClassDlg : public QDialog
{
   Q_OBJECT

public:
   FeatureClassDlg(QWidget* pParent = NULL);
   virtual ~FeatureClassDlg();

public slots:
   virtual void accept();

protected:
   void loadFromSettings();
   void saveToSettings() const;

protected slots:
   void addFeatureClass();
   void removeFeatureClass();
   void setFeatureClassData(QListWidgetItem* pItem);
   void addField();
   void removeField();
   void setFieldData(QTreeWidgetItem* pItem, int column);
   void updateFields();

private:
   DynamicObject* getCurrentFeatureClass();

   FactoryResource<DynamicObject> mpFeatureClasses;

   QListWidget* mpClassList;
   QTreeWidget* mpFieldTree;
};

class FieldNameDelegate : public QStyledItemDelegate
{
public:
   FieldNameDelegate(QObject* pParent = NULL);
   virtual ~FieldNameDelegate();

   virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const;
   virtual void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const;
   virtual void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
      const QModelIndex& index) const;
};

class FieldTypeDelegate : public QStyledItemDelegate
{
public:
   FieldTypeDelegate(QObject* pParent = NULL);
   virtual ~FieldTypeDelegate();

   virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const;
   virtual void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const;
   virtual void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
      const QModelIndex& index) const;
};

class FieldValueDelegate : public QStyledItemDelegate
{
public:
   FieldValueDelegate(QObject* pParent = NULL);
   virtual ~FieldValueDelegate();

   virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const;
   virtual void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const;
   virtual void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
      const QModelIndex& index) const;
};

#endif
