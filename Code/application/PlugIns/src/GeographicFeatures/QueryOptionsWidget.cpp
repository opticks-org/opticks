/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QScrollArea>

#include "AppAssert.h"
#include "AppVerify.h"
#include "CustomColorButton.h"
#include "FeatureClass.h"
#include "GraphicFillWidget.h"
#include "GraphicLineWidget.h"
#include "GraphicSymbolWidget.h"
#include "LabeledSection.h"
#include "QueryOptions.h"
#include "QueryOptionsWidget.h"
#include "QueryBuilderWidget.h"
#include "DisplaySelectionWidget.h"

#include <sstream>
using namespace std;

QueryOptionsWidget::QueryOptionsWidget(QWidget* pParent) :
   LabeledSectionGroup(pParent),
   mpFeatureClass(NULL),
   mbChangingSelection(false)
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

   mpSymbolContainerWidget = new QWidget(mpDisplayOptionsStack);
   mpLineContainerWidget = new QWidget(mpDisplayOptionsStack);
   mpSymbolAttributePropertiesSection = new DisplaySelectionWidget(mpSymbolContainerWidget);
   mpLineAttributePropertiesSection = new DisplaySelectionWidget(mpLineContainerWidget);

   mpSymbolWidget = new GraphicSymbolWidget(mpSymbolContainerWidget);
   mpLineWidget = new GraphicLineWidget(mpLineContainerWidget);
   mpPolygonWidget = new QWidget(mpDisplayOptionsStack);
   mpPolygonAttributePropertiesSection = new DisplaySelectionWidget(mpPolygonWidget);
   mpPolygonLineWidget = new GraphicLineWidget(mpPolygonWidget);
   mpPolygonFillWidget = new GraphicFillWidget(mpPolygonWidget);

   mpDisplayOptionsStack->addWidget(mpSymbolContainerWidget);
   mpDisplayOptionsStack->addWidget(mpLineContainerWidget);
   mpDisplayOptionsStack->addWidget(mpPolygonWidget);

   QHBoxLayout* pPolygonLayout = new QHBoxLayout(mpPolygonWidget);
   pPolygonLayout->setMargin(0);
   pPolygonLayout->setSpacing(10);
   pPolygonLayout->addWidget(mpPolygonAttributePropertiesSection, 10);
   pPolygonLayout->addWidget(mpPolygonLineWidget);
   pPolygonLayout->addWidget(mpPolygonFillWidget);

   QHBoxLayout* pLineLayout = new QHBoxLayout(mpLineContainerWidget);
   pLineLayout->setMargin(0);
   pLineLayout->setSpacing(10);
   pLineLayout->addWidget(mpLineAttributePropertiesSection, 10);
   pLineLayout->addWidget(mpLineWidget);

   QHBoxLayout* pSymbolLayout = new QHBoxLayout(mpSymbolContainerWidget);
   pSymbolLayout->setMargin(0);
   pSymbolLayout->setSpacing(10);
   pSymbolLayout->addWidget(mpSymbolAttributePropertiesSection, 10);
   pSymbolLayout->addWidget(mpSymbolWidget);

   LabeledSection* pDisplaySection = new LabeledSection(mpDisplayOptionsStack, "Display", this);

   addDisplayUpdateSignals();

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
   addSection(pDisplaySection, 1000);
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
   removeDisplayUpdateSignals();
   mpQueryNameEdit->setText(QString::fromStdString(options.getQueryName()));

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

   addDisplayUpdateSignals();
   const FeatureQueryOptions* pFeatureQuery = dynamic_cast<const FeatureQueryOptions*>(&options);
   if (pFeatureQuery != NULL)
   {
      mpFormatStringEdit->setText(QString::fromStdString(pFeatureQuery->getFormatString()));
      mpQueryBuilderWidget->setQuery(options.getQueryString());
      mpSymbolAttributePropertiesSection->setDefaultQuery(options);
      mpPolygonAttributePropertiesSection->setDefaultQuery(options);
      mpLineAttributePropertiesSection->setDefaultQuery(options);
   }
}

FeatureQueryOptions QueryOptionsWidget::getDisplayOptions() const
{
   FeatureQueryOptions options;

   options.setQueryName(mpQueryNameEdit->text().toStdString());
   options.setFormatString(mpFormatStringEdit->text().toStdString());
   options.setQueryString(mpQueryBuilderWidget->getQuery());

   QWidget* pCurrent = mpDisplayOptionsStack->currentWidget();

   if (pCurrent == mpSymbolContainerWidget)
   {
      options.setSymbolName(mpSymbolWidget->getSymbolName().toStdString());
      options.setSymbolSize(mpSymbolWidget->getSymbolSize());
   }
   else if (pCurrent == mpLineContainerWidget)
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
      pPropertyWidget = mpSymbolContainerWidget;
      break;
   case ArcProxyLib::POINT:
      shapeType += "Point";
      pPropertyWidget = mpSymbolContainerWidget;
      break;
   case ArcProxyLib::POLYLINE:
      shapeType += "Polyline";
      pPropertyWidget = mpLineContainerWidget;
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
   mpSymbolAttributePropertiesSection->populateAttributeList(fields, types);
   mpPolygonAttributePropertiesSection->populateAttributeList(fields, types);
   mpLineAttributePropertiesSection->populateAttributeList(fields, types);

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

void QueryOptionsWidget::setDisplayQueryOptions(const std::string& queryName)
{
   const std::vector<DisplayQueryOptions> queries = mpFeatureClass->getDisplayQueryOptions(queryName);
   mpSymbolAttributePropertiesSection->initializeFromQueries(queries);
   mpPolygonAttributePropertiesSection->initializeFromQueries(queries);
   mpLineAttributePropertiesSection->initializeFromQueries(queries);
}

void QueryOptionsWidget::populateFieldValues(const std::string& field, std::vector<std::string>& values)
{
   mpSymbolAttributePropertiesSection->populateFieldValues(field, values);
   mpPolygonAttributePropertiesSection->populateFieldValues(field, values);
   mpLineAttributePropertiesSection->populateFieldValues(field, values);
}

bool QueryOptionsWidget::updateQueries()
{
   bool bSelected = false;
   if (mbChangingSelection == false)
   {
      FeatureQueryOptions origOption = getDisplayOptions();
      if (mSelectedDisplayQueryNames.empty() == false)
      {
         std::vector<DisplayQueryOptions*> options;
         for (unsigned int i = 0; i < mSelectedDisplayQueryNames.size(); i++)
         {
            QueryOptions* pQueryOption = dynamic_cast<QueryOptions*>(&origOption);
            DisplayQueryOptions* pOption = new DisplayQueryOptions(*pQueryOption);
            pOption->setUniqueName(mSelectedDisplayQueryNames[i]);
            options.push_back(pOption);
         }
         mpFeatureClass->modifyDisplayQuery(options, true);
         bSelected = true;
      }
      else
      {
         mpFeatureClass->updateQuery(origOption);
      }
   }
   return bSelected;
}

void QueryOptionsWidget::selectDisplayQuery(const std::vector<DisplayQueryOptions*>& displayQueries)
{
   //save off the previous selection
   mbChangingSelection = true;
   FeatureQueryOptions origOption = getDisplayOptions();
   const std::string queryName = origOption.getQueryName();
   if (displayQueries.empty() == true)
   {
      //display the feature class wide properties
      FeatureQueryOptions* pOption = mpFeatureClass->getQueryByName(queryName);
      if (pOption != NULL)
      {
         pOption->setFormatString(origOption.getFormatString());
         setDisplayOptions(*pOption);
      }
   }
   else
   {
      //display the display settings for the selected query option
      std::vector<DisplayQueryOptions> pFeatureOptions = mpFeatureClass->getDisplayQueryOptions(queryName);
      for (unsigned int i = 0; i < pFeatureOptions.size(); i++)
      {
         DisplayQueryOptions option = pFeatureOptions[i];
         if (option.getUniqueName() == displayQueries.front()->getUniqueName())
         {
            setDisplayOptions(option);
         }
      }
   }
   mSelectedDisplayQueryNames.clear();
   for (unsigned int i = 0; i < displayQueries.size(); i++)
   {
      mSelectedDisplayQueryNames.push_back(displayQueries[i]->getUniqueName());
   }
   mbChangingSelection = false;
}

void QueryOptionsWidget::setFeatureClass(FeatureClass* pFeatureClass)
{
   mpFeatureClass = pFeatureClass;
   mpLineAttributePropertiesSection->setFeatureClass(mpFeatureClass);
   mpPolygonAttributePropertiesSection->setFeatureClass(mpFeatureClass);
   mpSymbolAttributePropertiesSection->setFeatureClass(mpFeatureClass);
}

void QueryOptionsWidget::addDisplayUpdateSignals()
{
   //symbol signals
   VERIFYNR(connect(mpSymbolAttributePropertiesSection,
      SIGNAL(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&)),
      this, SLOT(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&))));
   VERIFYNR(connect(mpSymbolWidget, SIGNAL(nameChanged(const QString&)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpSymbolWidget, SIGNAL(sizeChanged(unsigned int)), this, SLOT(updateQueries())));

   //polygon signals
   VERIFYNR(connect(mpPolygonAttributePropertiesSection,
      SIGNAL(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&)),
      this, SLOT(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&))));
   VERIFYNR(connect(mpPolygonLineWidget, SIGNAL(stateChanged(bool)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonLineWidget, SIGNAL(styleChanged(LineStyle)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonLineWidget, SIGNAL(widthChanged(unsigned int)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonLineWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonLineWidget, SIGNAL(scaledChanged(bool)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonFillWidget, SIGNAL(styleChanged(FillStyle)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonFillWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpPolygonFillWidget, SIGNAL(hatchChanged(SymbolType)), this, SLOT(updateQueries())));

   //line signals
   VERIFYNR(connect(mpLineAttributePropertiesSection,
      SIGNAL(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&)),
      this, SLOT(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&))));
   VERIFYNR(connect(mpLineWidget, SIGNAL(stateChanged(bool)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpLineWidget, SIGNAL(styleChanged(LineStyle)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpLineWidget, SIGNAL(widthChanged(unsigned int)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpLineWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateQueries())));
   VERIFYNR(connect(mpLineWidget, SIGNAL(scaledChanged(bool)), this, SLOT(updateQueries())));
}

void QueryOptionsWidget::removeDisplayUpdateSignals()
{
   //symbol signals
   VERIFYNR(disconnect(mpSymbolAttributePropertiesSection,
      SIGNAL(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&)),
      this, SLOT(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&))));
   VERIFYNR(disconnect(mpSymbolWidget, SIGNAL(nameChanged(const QString&)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpSymbolWidget, SIGNAL(sizeChanged(unsigned int)), this, SLOT(updateQueries())));

   //polygon signals
   VERIFYNR(disconnect(mpPolygonAttributePropertiesSection,
      SIGNAL(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&)),
      this, SLOT(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&))));
   VERIFYNR(disconnect(mpPolygonLineWidget, SIGNAL(stateChanged(bool)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonLineWidget, SIGNAL(styleChanged(LineStyle)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonLineWidget, SIGNAL(widthChanged(unsigned int)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonLineWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonLineWidget, SIGNAL(scaledChanged(bool)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonFillWidget, SIGNAL(styleChanged(FillStyle)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonFillWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpPolygonFillWidget, SIGNAL(hatchChanged(SymbolType)), this, SLOT(updateQueries())));

   //line signals
   VERIFYNR(disconnect(mpLineAttributePropertiesSection,
      SIGNAL(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&)),
      this, SLOT(selectDisplayQuery(const std::vector<DisplayQueryOptions*>&))));
   VERIFYNR(disconnect(mpLineWidget, SIGNAL(stateChanged(bool)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpLineWidget, SIGNAL(styleChanged(LineStyle)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpLineWidget, SIGNAL(widthChanged(unsigned int)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpLineWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateQueries())));
   VERIFYNR(disconnect(mpLineWidget, SIGNAL(scaledChanged(bool)), this, SLOT(updateQueries())));
}