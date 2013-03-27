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

#include <QtGui/QWidget>

#include <boost/any.hpp>
#include <string>

class DynamicObject;
class DynamicObjectItemModel;
class QCheckBox;
class QComboBox;
class QModelIndex;
class QRegExp;
class QSortFilterProxyModel;
class QToolButton;
class QTreeView;
class Subject;

class MetadataWidget : public QWidget
{
   Q_OBJECT

public:
   MetadataWidget(QWidget* parent = 0);
   ~MetadataWidget();

   void setMetadata(DynamicObject* pMetadata);
   static const std::string& getEditWarningDialogId();

   QSize sizeHint() const;

protected slots:
   void enableFilters(bool enable);
   void applyFilter(int filterIndex);
   void createFilter();
   void editFilter();
   void deleteFilter();
   void modifyValues();
   void addKey();
   void editCurrentValue();
   void deleteKey();
   void clearKeys();
   void currentChanged(const QModelIndex& selectedItem, const QModelIndex& deselectedItem);
   void editSelectedValue(const QModelIndex& selectedItem);
   void copyIdToClipboard();
   void copyValueToClipboard();

protected:
   enum ItemDataRole
   {
      DataVariantRole = Qt::UserRole
   };

   void metadataDeleted(Subject& subject, const std::string& signal, const boost::any& value);

private:
   MetadataWidget(const MetadataWidget& rhs);
   MetadataWidget& operator=(const MetadataWidget& rhs);
   bool applyFilter(int row, const QModelIndex& parent, const QRegExp& nameFilter, const QRegExp& valueFilter,
      bool parentMatch = false);
   bool isFilterNameUnique(const QString& filterName, int ignoreIndex = -1) const;

   QTreeView* mpMetadataTree;
   QCheckBox* mpFilterCheck;
   QComboBox* mpFilterCombo;
   QToolButton* mpCreateFilterButton;
   QToolButton* mpEditFilterButton;
   QToolButton* mpDeleteFilterButton;
   QToolButton* mpModifyButton;
   QToolButton* mpAddChildButton;
   QToolButton* mpAddSiblingButton;
   QToolButton* mpEditButton;
   QToolButton* mpDeleteButton;
   QToolButton* mpClearButton;

   DynamicObject* mpMetadata;
   DynamicObjectItemModel* mpMetadataModel;
   QSortFilterProxyModel* mpMetadataSortingModel;
};

#endif
