/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef QUERYBUILDERWIDGET_H
#define QUERYBUILDERWIDGET_H

#include "EnumWrapper.h"

#include <QtGui/QWidget>
#include <string>
#include <vector>

class QComboBox;
class QLineEdit;
class QVBoxLayout;
class QStringListModel;
class QLabel;

class QueryBuilderWidgetElement;

class QueryBuilderWidget : public QWidget
{
   Q_OBJECT
public:
   QueryBuilderWidget(QWidget *pParent = NULL);
   ~QueryBuilderWidget();

   std::string getQuery() const;
   void setQuery(const std::string &queryString);

   void setFields(const std::vector<std::string> &fields);

   void clearQueryElements();

public slots:
   QueryBuilderWidgetElement *addQueryElement();
   void elementRemoveButtonPressed();

private:
   enum CombinationTypeEnum
   {
      ANY_COMBINATION,
      ALL_COMBINATION
   };

   /**
    * @EnumWrapper QueryBuilderWidget::CombinationTypeEnum.
    */
   typedef EnumWrapper<CombinationTypeEnum> CombinationType;

   QLabel* mpAllShapesLabel;
   QStringListModel* mpFieldModel;
   QComboBox* mpCombineCombo;
   QWidget* mpRulesWidget;
   QVBoxLayout* mpRulesLayout;

};

// This would be a nested class, but moc doesn't support nested classes
class QueryBuilderWidgetElement : public QWidget
{
   Q_OBJECT

public:
   QueryBuilderWidgetElement(QStringListModel *pFieldModel, QWidget *pParent = NULL);
   ~QueryBuilderWidgetElement();

   void getQueryElement(std::string &field, std::string &compare, std::string &value) const;
   void setQueryElement(const std::string &field, const std::string &compare, const std::string &value);

signals:
   void addButtonPressed();
   void removeButtonPressed();

private:
   QComboBox* mpFieldCombo;
   QComboBox* mpCompareCombo;
   QLineEdit* mpValueEdit;
};

#endif
