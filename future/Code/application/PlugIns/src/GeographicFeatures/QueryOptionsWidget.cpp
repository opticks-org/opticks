/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QScrollArea>

#include "AppAssert.h"
#include "AppVerify.h"
#include "CustomColorButton.h"
#include "GraphicFillWidget.h"
#include "GraphicLineWidget.h"
#include "GraphicSymbolWidget.h"
#include "LabeledSection.h"
#include "QueryOptions.h"
#include "QueryOptionsWidget.h"
#include "QueryBuilderWidget.h"

#include <sstream>
using namespace std;

QueryOptionsWidget::QueryOptionsWidget(QWidget *parent) :
   LabeledSectionGroup(parent)
{
   // Query
   QWidget* pQuerySectionWidget = new QWidget(this);
   QLabel* pQueryNameLabel = new QLabel("Name:", pQuerySectionWidget);
   mpQueryNameEdit = new QLineEdit(pQuerySectionWidget);
   mpQueryBuilderWidget = new QueryBuilderWidget(pQuerySectionWidget);
   
   QScrollArea* pScrollArea = new QScrollArea(pQuerySectionWidget);
   pScrollArea->setWidgetResizable(true);
   pScrollArea->setWidget(mpQueryBuilderWidget);

   QGridLayout* pQueryLayout = new QGridLayout(pQuerySectionWidget);
   pQueryLayout->setMargin(0);
   pQueryLayout->setSpacing(5);
   pQueryLayout->addWidget(pQueryNameLabel, 0, 0);
   pQueryLayout->addWidget(mpQueryNameEdit, 0, 1);
   pQueryLayout->addWidget(pScrollArea, 1, 0, 1, 2);

   mpQuerySection = new LabeledSection(pQuerySectionWidget, "Query", this);

   // Display
   mpDisplayOptionsStack = new AutoResizeStackedWidget(this);
   mpDisplayOptionsStack->setEnabled(false);

   mpSymbolWidget = new GraphicSymbolWidget(mpDisplayOptionsStack);
   mpLineWidget = new GraphicLineWidget(mpDisplayOptionsStack);
   mpPolygonWidget = new QWidget(mpDisplayOptionsStack);
   mpPolygonLineWidget = new GraphicLineWidget(mpPolygonWidget);
   mpPolygonFillWidget = new GraphicFillWidget(mpPolygonWidget);

   mpDisplayOptionsStack->addWidget(mpSymbolWidget);
   mpDisplayOptionsStack->addWidget(mpLineWidget);
   mpDisplayOptionsStack->addWidget(mpPolygonWidget);

   QBoxLayout* pPolygonLayout = new QHBoxLayout(mpPolygonWidget);
   pPolygonLayout->addWidget(mpPolygonLineWidget);
   pPolygonLayout->addWidget(mpPolygonFillWidget);

   LabeledSection* pDisplaySection = new LabeledSection(mpDisplayOptionsStack, "Display", this);

   // Fields
   QWidget* pFieldsSectionWidget = new QWidget(this);
   QLabel* pDisplayedFieldsLabel = new QLabel("Displayed:", this);
   QLabel* pAvailableFieldsLabel = new QLabel("Available:", this);

   mpFormatStringEdit = new QLineEdit(this);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");
   columnNames.append("Sample Value");

   mpFieldList = new QTreeWidget(this);
   ENSURE(mpFieldList != NULL);

   mpFieldList->setColumnCount(3);
   mpFieldList->setHeaderLabels(columnNames);
   mpFieldList->setSelectionMode(QAbstractItemView::SingleSelection);
   mpFieldList->setSortingEnabled(true);
   mpFieldList->sortItems(0, Qt::AscendingOrder);
   mpFieldList->setRootIsDecorated(false);

   QHeaderView* pHeader = mpFieldList->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(true);
      pHeader->setSortIndicatorShown(true);
   }

   QGridLayout* pFieldsLayout = new QGridLayout(pFieldsSectionWidget);
   pFieldsLayout->setMargin(0);
   pFieldsLayout->setSpacing(5);
   pFieldsLayout->addWidget(pDisplayedFieldsLabel, 0, 0);
   pFieldsLayout->addWidget(mpFormatStringEdit, 0, 1);
   pFieldsLayout->addWidget(pAvailableFieldsLabel, 1, 0, Qt::AlignTop);
   pFieldsLayout->addWidget(mpFieldList, 1, 1);
   pFieldsLayout->setRowStretch(1, 10);
   pFieldsLayout->setColumnStretch(1, 10);

   LabeledSection* pFieldsSection = new LabeledSection(pFieldsSectionWidget, "Fields", this);

   connect(mpFieldList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
      this, SLOT(addItemToFormatString(QTreeWidgetItem*, int)));

   // File Info
   QWidget* pFileInfoSectionWidget = new QWidget(this);
   mpNumberFile = new QLabel("0 shapes in file", pFileInfoSectionWidget);
   mpShapeType = new QLabel("Shape type:", pFileInfoSectionWidget);

   QHBoxLayout* pFileInfoLayout = new QHBoxLayout(pFileInfoSectionWidget);
   pFileInfoLayout->setMargin(0);
   pFileInfoLayout->setSpacing(5);
   pFileInfoLayout->addWidget(mpNumberFile);
   pFileInfoLayout->addWidget(mpShapeType);

   LabeledSection* pFileInfoSection = new LabeledSection(pFileInfoSectionWidget, "File info", this);

   // Layout
   addSection(mpQuerySection, 1000);
   addSection(pDisplaySection);
   addSection(pFieldsSection, 1000);
   addSection(pFileInfoSection);
   addStretch(1);
   setSizeHint(500, 350);
}

QueryOptionsWidget::~QueryOptionsWidget()
{
}

void QueryOptionsWidget::setDisplayOptions(const QueryOptions &options)
{
   mpQueryNameEdit->setText(QString::fromStdString(options.getQueryName()));
   mpFormatStringEdit->setText(QString::fromStdString(options.getFormatString()));
   mpQueryBuilderWidget->setQuery(options.getQueryString());

   mpSymbolWidget->setSymbolName(QString::fromStdString(options.getSymbolName()));
   mpSymbolWidget->setSymbolSize(options.getSymbolSize());

   mpLineWidget->setLineState(options.getLineState());
   mpLineWidget->setLineStyle(options.getLineStyle());
   mpLineWidget->setLineWidth(options.getLineWidth());
   mpLineWidget->setLineColor(options.getLineColor());
   mpLineWidget->setLineScaled(options.getLineScaled());

   mpPolygonLineWidget->setLineState(options.getLineState());
   mpPolygonLineWidget->setLineStyle(options.getLineStyle());
   mpPolygonLineWidget->setLineWidth(options.getLineWidth());
   mpPolygonLineWidget->setLineColor(options.getLineColor());
   mpPolygonLineWidget->setLineScaled(options.getLineScaled());

   mpPolygonFillWidget->setFillColor(COLORTYPE_TO_QCOLOR(options.getFillColor()));
   mpPolygonFillWidget->setFillStyle(options.getFillStyle());
   mpPolygonFillWidget->setHatchStyle(options.getHatchStyle());
}

QueryOptions QueryOptionsWidget::getDisplayOptions() const
{
   QueryOptions options;

   options.setQueryName(mpQueryNameEdit->text().toStdString());
   options.setFormatString(mpFormatStringEdit->text().toStdString());
   options.setQueryString(mpQueryBuilderWidget->getQuery());

   QWidget* pCurrent = mpDisplayOptionsStack->currentWidget();

   if (pCurrent == mpSymbolWidget)
   {
      options.setSymbolName(mpSymbolWidget->getSymbolName().toStdString());
      options.setSymbolSize(mpSymbolWidget->getSymbolSize());
   }
   else if (pCurrent == mpLineWidget)
   {
      options.setLineColor(mpLineWidget->getLineColor());
      options.setLineScaled(mpLineWidget->getLineScaled());
      options.setLineState(mpLineWidget->getLineState());
      options.setLineStyle(mpLineWidget->getLineStyle());
      options.setLineWidth(mpLineWidget->getLineWidth());
   }
   else if (pCurrent == mpPolygonWidget)
   {
      options.setLineColor(mpPolygonLineWidget->getLineColor());
      options.setLineScaled(mpPolygonLineWidget->getLineScaled());
      options.setLineState(mpPolygonLineWidget->getLineState());
      options.setLineStyle(mpPolygonLineWidget->getLineStyle());
      options.setLineWidth(mpPolygonLineWidget->getLineWidth());
      options.setFillColor(QCOLOR_TO_COLORTYPE(mpPolygonFillWidget->getFillColor()));
      options.setFillStyle(mpPolygonFillWidget->getFillStyle());
      options.setHatchStyle(mpPolygonFillWidget->getHatchStyle());
   }

   return options;
}

void QueryOptionsWidget::setFeatureType(ArcProxyLib::FeatureType featureType)
{
   QWidget* pPropertyWidget = NULL;

   QString shapeType = "Shape type: ";
   switch (featureType)
   {
   case ArcProxyLib::MULTIPOINT:
      shapeType += "Multipoint";
      pPropertyWidget = mpSymbolWidget;
      break;
   case ArcProxyLib::POINT:
      shapeType += "Point";
      pPropertyWidget = mpSymbolWidget;
      break;
   case ArcProxyLib::POLYLINE:
      shapeType += "Polyline";
      pPropertyWidget = mpLineWidget;
      break;
   case ArcProxyLib::POLYGON:
      shapeType += "Polygon";
      pPropertyWidget = mpPolygonWidget;
      break;
   default:
      shapeType += "Unknown";
      break;
   };
   if (pPropertyWidget != NULL)
   {
      mpDisplayOptionsStack->setEnabled(true);
      mpDisplayOptionsStack->setCurrentWidget(pPropertyWidget);
   }
   else
   {
      mpDisplayOptionsStack->setEnabled(false);
   }
   VERIFYNRV(mpShapeType);
   mpShapeType->setText(shapeType);
}

void QueryOptionsWidget::setFeatureFields(const std::vector<std::string> &fields,
   const std::vector<std::string> &types,
   const std::vector<std::string> &sampleValues)
{
   VERIFYNRV(fields.size() == types.size());
   VERIFYNRV(mpFieldList != NULL);

   mpFieldList->clear();

   for (std::vector<std::string>::size_type i = 0; i < fields.size(); ++i)
   {
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpFieldList);
      if (pItem != NULL)
      {
         pItem->setText(0, QString::fromStdString(fields[i]));
         pItem->setText(1, QString::fromStdString(types[i]));
         if (i < sampleValues.size())
         {
            pItem->setText(2, QString::fromStdString(sampleValues[i]));
         }
      }
   }

   mpQueryBuilderWidget->setFields(fields);
}

void QueryOptionsWidget::setFeatureCount(unsigned int count)
{
   QString numShapesText;
   if (count == 0)
   {
      numShapesText = "No shapes in file";
   }
   else if (count == 1)
   {
      numShapesText = "1 shape in file";
   }
   else
   {
      numShapesText = QString::number(count) + " shapes in file";
   }
   VERIFYNRV(mpNumberFile);
   mpNumberFile->setText(numShapesText);
}

void QueryOptionsWidget::addItemToFormatString(QTreeWidgetItem *pItem, int column)
{
   VERIFYNRV(pItem != NULL);

   QString featureName = pItem->text(0);

   QString formatString = mpFormatStringEdit->text();

   if (!formatString.isEmpty())
   {
      if (formatString[formatString.size()-1] == ']')
      {
         formatString += " - ";
      }
   }
   formatString += "[" + featureName + "]";

   mpFormatStringEdit->setText(formatString);
}

void QueryOptionsWidget::setHideQueryBuilder(bool hidden)
{
   mpQuerySection->setHidden(hidden);
   mpQueryBuilderWidget->clearQueryElements();
}
