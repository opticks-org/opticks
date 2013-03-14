/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DISPLAYSELECTIONWIDGET_H
#define DISPLAYSELECTIONWIDGET_H

#include "QueryOptions.h"

#include <QtGui/QWidget>

#include <string>
#include <vector>

class CustomTreeWidget;
class DisplayQueryOptions;
class FeatureClass;
class QAction;
class QComboBox;
class QMenu;
class QPushButton;
class QTreeWidgetItem;

class DisplaySelectionWidget : public QWidget
{
   Q_OBJECT

public:
   DisplaySelectionWidget(QWidget* pParent = NULL);
   virtual ~DisplaySelectionWidget();

   void setDefaultQuery(const QueryOptions& query);
   const QueryOptions& getDefaultQuery();

   void setFeatureClass(FeatureClass* pFeatureClass);
   void populateAttributeList(const std::vector<std::string>& names, const std::vector<std::string>& types);

   void initializeFromQueries(const std::vector<DisplayQueryOptions>& queries);
   void populateFieldValues(const std::string& field, const std::vector<std::string>& values);

protected slots:
   void fieldSelected(const QString& text);
   void loadRecentQuery(QAction* pAction);
   void textModified(QTreeWidgetItem* pItem, int iColumn);
   void treeItemCheckChanged(QTreeWidgetItem* pItem, int iColumn);
   void removeButtonPressed();
   void addButtonPressed();
   void addQueriesButtonPressed();
   void querySelectionChanged();
   void displayContextMenu(const QPoint& menuPoint);
   void populateContextMenu();
   void openFilePressed();
   void saveFilePressed();
   void moveUpButtonPressed();
   void moveDownButtonPressed();
   void moveTopButtonPressed();
   void moveBottomButtonPressed();
   void checkAllButtonPressed();
   void unCheckAllButtonPressed();
   void clearQueryElements();

signals:
   void selectDisplayQuery(const std::vector<DisplayQueryOptions*>& pQueries);

protected:
   DisplayQueryOptions* createQueryOption(QTreeWidgetItem* pItem);
   QString createQueryString(QString& field, QString& value);
   void createAllObjectsEntry();
   void populateFieldValueCombos(std::string field);
   DisplayQueryOptions* addNewQueryItem(std::string& field, std::string& value);
   void moveItems(int index);
   void addToRecentlyUsedList(std::string filename);
   void getRecentlyUsedList(std::vector<std::string>& list);
   void saveQuery(const QString& fileLocation) const;
   bool loadQuery(const QString& fileLocation);

private:
   QueryOptions mDefaultQuery;
   QComboBox* mpAttributeCombo;
   std::vector<QComboBox*> mpValueCombos;
   QPushButton* mpEditButton;
   QMenu* mpMenu;
   CustomTreeWidget* mpTree;
   std::vector<std::string> mFieldTypes;
   std::vector<std::vector<std::string> > mFieldValues;
   FeatureClass* mpFeatureClass;
};

#endif
