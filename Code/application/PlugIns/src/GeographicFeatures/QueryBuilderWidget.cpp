/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "QueryBuilderWidget.h"

#include <QtCore/QStringList>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QStringListModel>

QueryBuilderWidget::QueryBuilderWidget(QWidget *pParent) : QWidget(pParent)
{
   QLabel *pMatchLabel1 = new QLabel("Match", this);
   QLabel *pMatchLabel2 = new QLabel("of the following rules:", this);

   mpCombineCombo = new QComboBox(this);
   mpCombineCombo->addItem("any", "OR");
   mpCombineCombo->addItem("all", "AND");

   QPushButton *pAddButton = new QPushButton("Add", this);
   mpAllShapesLabel = new QLabel("Include all shapes");

   mpFieldModel = new QStringListModel(this);

   VERIFYNR(connect(pAddButton, SIGNAL(clicked(bool)), this, SLOT(addQueryElement())));

   mpRulesWidget = new QWidget(this);

   QGridLayout *pLayout = new QGridLayout(this);
   pLayout->setColumnStretch(2, 10);

   pLayout->addWidget(pMatchLabel1, 0, 0);
   pLayout->addWidget(mpCombineCombo, 0, 1);
   pLayout->addWidget(pMatchLabel2, 0, 2, Qt::AlignLeft);
   pLayout->addWidget(pAddButton, 0, 3);
   pLayout->addWidget(mpRulesWidget, 1, 0, 1, 4);

   mpRulesLayout = new QVBoxLayout(mpRulesWidget);
   mpRulesLayout->setMargin(0);
   mpRulesLayout->addWidget(mpAllShapesLabel);
}

QueryBuilderWidget::~QueryBuilderWidget()
{
}

std::string QueryBuilderWidget::getQuery() const
{
   QList<QueryBuilderWidgetElement*> elements = mpRulesWidget->findChildren<QueryBuilderWidgetElement*>();
   if (elements.empty())
   {
      return "";
   }

   int combineIndex = mpCombineCombo->currentIndex();
   std::string query = mpCombineCombo->itemData(combineIndex).
      value<QString>().toStdString();

   for (QList<QueryBuilderWidgetElement*>::const_iterator iter = elements.begin();
      iter != elements.end(); ++iter)
   {
      std::string field;
      std::string compare;
      std::string value;
      (*iter)->getQueryElement(field, compare, value);
      value = QString::fromStdString(value).replace(",", "%~").toStdString();
      query += "," + field + "," + compare + "," + value;
   }

   return query;
}

void QueryBuilderWidget::setQuery(const std::string &queryString)
{
   clearQueryElements();

   if (queryString.empty())
   {
      return;
   }

   QStringList queryList = QString::fromStdString(queryString).split(",");

   int combineComboIndex = mpCombineCombo->findData(queryList.takeFirst());
   mpCombineCombo->setCurrentIndex(combineComboIndex);

   VERIFYNRV(queryList.count() % 3 == 0);

   QStringList::const_iterator iter = queryList.begin();
   while (iter != queryList.end())
   {
      std::string field = (*iter++).toStdString();
      std::string compare = (*iter++).toStdString();
      std::string value = (*iter++).toStdString();
      value = QString::fromStdString(value).replace("%~", ",").toStdString();
      QueryBuilderWidgetElement *pElement = addQueryElement();
      if (NN(pElement))
      {
         pElement->setQueryElement(field, compare, value);
      }
   }
}

void QueryBuilderWidget::setFields(const std::vector<std::string> &fields)
{
   QStringList stringList;
   for (std::vector<std::string>::const_iterator iter = fields.begin();
      iter != fields.end(); ++iter)
   {
      stringList.append(QString::fromStdString(*iter));
   }

   if (stringList != mpFieldModel->stringList())
   {
      clearQueryElements();
      mpFieldModel->setStringList(stringList);
   }
}

QueryBuilderWidgetElement *QueryBuilderWidget::addQueryElement()
{
   QueryBuilderWidgetElement *pElement = new QueryBuilderWidgetElement(mpFieldModel);
   VERIFYNR(connect(pElement, SIGNAL(removeButtonPressed()), 
      this, SLOT(elementRemoveButtonPressed())));

   mpRulesLayout->addWidget(pElement);
   mpAllShapesLabel->setHidden(true);

   return pElement;
}

void QueryBuilderWidget::elementRemoveButtonPressed()
{
   QObject *pSender = sender();

   mpRulesLayout->removeWidget(dynamic_cast<QWidget*>(pSender));

   delete pSender;
   if (mpRulesWidget->findChildren<QueryBuilderWidgetElement*>().empty())
   {
      mpAllShapesLabel->setHidden(false);
   }
}

void QueryBuilderWidget::clearQueryElements()
{
   QList<QueryBuilderWidgetElement*> elements = mpRulesWidget->findChildren<QueryBuilderWidgetElement*>();
   for (QList<QueryBuilderWidgetElement*>::const_iterator iter = elements.begin();
      iter != elements.end(); ++iter)
   {
      mpRulesLayout->removeWidget(*iter);
      delete *iter;
   }

   mpAllShapesLabel->setHidden(false);

}

QueryBuilderWidgetElement::QueryBuilderWidgetElement(QStringListModel *pFieldModel, QWidget *pParent) : QWidget(pParent)
{
   mpFieldCombo = new QComboBox(this);
   mpFieldCombo->setModel(pFieldModel);
   
   mpCompareCombo = new QComboBox(this);
   mpCompareCombo->addItem("equals", "=");
   mpCompareCombo->addItem("does not equal", "<>");
   mpCompareCombo->addItem("is greater than", ">");
   mpCompareCombo->addItem("is less than", "<");
   mpCompareCombo->addItem("is greater or equal to", ">=");
   mpCompareCombo->addItem("is less or equal to", "<=");

   mpValueEdit = new QLineEdit(this);

   QPushButton *pRemoveButton = new QPushButton("Remove", this);

   QHBoxLayout *pLayout = new QHBoxLayout(this);
   pLayout->setMargin(0); // intended to be placed in another layout
   pLayout->addWidget(mpFieldCombo);
   pLayout->addWidget(mpCompareCombo);
   pLayout->addWidget(mpValueEdit);
   pLayout->addWidget(pRemoveButton);

   VERIFYNR(connect(pRemoveButton, SIGNAL(clicked(bool)), this, SIGNAL(removeButtonPressed())));
}

QueryBuilderWidgetElement::~QueryBuilderWidgetElement()
{
}

void QueryBuilderWidgetElement::getQueryElement(std::string &field, 
   std::string &compare, std::string &value) const
{
   field = mpFieldCombo->currentText().toStdString();

   int compareIndex = mpCompareCombo->currentIndex();
   compare = mpCompareCombo->itemData(compareIndex).
      value<QString>().toStdString();
   value = mpValueEdit->text().toStdString();
}

void QueryBuilderWidgetElement::setQueryElement(const std::string &field, 
   const std::string &compare, const std::string &value)
{
   int fieldIndex = mpFieldCombo->findText(QString::fromStdString(field));
   mpFieldCombo->setCurrentIndex(fieldIndex);

   int compareIndex = mpCompareCombo->findData(QString::fromStdString(compare));
   mpCompareCombo->setCurrentIndex(compareIndex);

   mpValueEdit->setText(QString::fromStdString(value));

}