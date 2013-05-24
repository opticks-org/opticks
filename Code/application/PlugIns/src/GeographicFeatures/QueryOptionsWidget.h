/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef QUERYOPTIONSWIDGET_H
#define QUERYOPTIONSWIDGET_H

#include <QtGui/QLineEdit>
#include <QtGui/QTreeWidget>
#include <QtGui/QStackedWidget>

#include "Feature.h"
#include "LabeledSectionGroup.h"
#include "TypesFile.h"

class DisplaySelectionWidget;
class DisplayQueryOptions;
class FeatureClass;
class FeatureQueryOptions;
class GraphicSymbolWidget;
class GraphicLineWidget;
class GraphicFillWidget;
class LabeledSection;
class QueryBuilderWidget;
class QueryOptions;

class QGroupBox;
class QLabel;

class QueryOptionsWidget : public LabeledSectionGroup
{
   Q_OBJECT

public:
   QueryOptionsWidget(QWidget* pParent = NULL);
   virtual ~QueryOptionsWidget();

   void setDisplayOptions(const QueryOptions& options);
   FeatureQueryOptions getDisplayOptions() const;

   void setFeatureType(ArcProxyLib::FeatureType featureType);
   void setFeatureFields(const std::vector<std::string> &fields,
      const std::vector<std::string> &types,
      const std::vector<std::string> &sampleValues);
   void setFeatureCount(unsigned int count);
   void setFeatureClass(FeatureClass* pFeatureClass);

   void setHideQueryBuilder(bool hidden);
   void setDisplayQueryOptions(const std::string& queryName);
   void populateFieldValues(const std::string& field, std::vector<std::string>& values);

public slots:
   void selectDisplayQuery(const std::vector<DisplayQueryOptions*>& displayQueries);
   bool updateQueries();

protected slots:
   void addItemToFormatString(QTreeWidgetItem *pItem, int column);

private:
   /**
    * AutoResizeStackedWidget is a subclass of QStackedWidget
    * which automatically resizes when the displayed widget
    * changes.
    */
   class AutoResizeStackedWidget : public QStackedWidget
   {
   public:
      AutoResizeStackedWidget(QWidget *pParent = NULL) : QStackedWidget(pParent)
      {
      }

      QSize sizeHint() const
      {
         QWidget* pCurrentWidget = currentWidget();
         if (pCurrentWidget != NULL)
         {
            return pCurrentWidget->sizeHint();
         }
         return QStackedWidget::sizeHint();
      }

   private:
      AutoResizeStackedWidget(const AutoResizeStackedWidget& rhs);
      AutoResizeStackedWidget& operator=(const AutoResizeStackedWidget& rhs);
   };

   QLineEdit* mpQueryNameEdit;
   QTreeWidget* mpFieldList;
   QLineEdit* mpFormatStringEdit;

   LabeledSection* mpQuerySection;
   QLabel* mpNumberFile;
   QLabel* mpShapeType;

   QueryBuilderWidget* mpQueryBuilderWidget;
   QStackedWidget* mpDisplayOptionsStack;
   FeatureClass* mpFeatureClass;
   std::vector<std::string> mSelectedDisplayQueryNames;
   bool mbChangingSelection;

   //point
   QWidget* mpSymbolContainerWidget;
   DisplaySelectionWidget* mpSymbolAttributePropertiesSection;
   GraphicSymbolWidget* mpSymbolWidget;

   //polygon
   DisplaySelectionWidget* mpPolygonAttributePropertiesSection;
   QWidget* mpPolygonWidget;
   GraphicLineWidget* mpPolygonLineWidget;
   GraphicFillWidget* mpPolygonFillWidget;

   //polyline
   QWidget* mpLineContainerWidget;
   DisplaySelectionWidget* mpLineAttributePropertiesSection;
   GraphicLineWidget* mpLineWidget;

private:
   QueryOptionsWidget(const QueryOptionsWidget& rhs);
   QueryOptionsWidget& operator=(const QueryOptionsWidget& rhs);

   void addDisplayUpdateSignals();
   void removeDisplayUpdateSignals();
};

#endif
