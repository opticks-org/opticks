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

class LabeledSection;
class GraphicSymbolWidget;
class GraphicLineWidget;
class GraphicFillWidget;
class QGroupBox;
class QLabel;
class QueryBuilderWidget;
class QueryOptions;

class QueryOptionsWidget : public LabeledSectionGroup
{
   Q_OBJECT

public:
   QueryOptionsWidget(QWidget* parent = NULL);
   ~QueryOptionsWidget(void);

   void setDisplayOptions(const QueryOptions &options);
   QueryOptions getDisplayOptions() const;

   void setFeatureType(ArcProxyLib::FeatureType featureType);
   void setFeatureFields(const std::vector<std::string> &fields,
      const std::vector<std::string> &types,
      const std::vector<std::string> &sampleValues);
   void setFeatureCount(unsigned int count);

   void setHideQueryBuilder(bool hidden);

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
         QWidget *pCurrentWidget = currentWidget();
         if (pCurrentWidget != NULL)
         {
            return pCurrentWidget->sizeHint();
         }
         return QStackedWidget::sizeHint();
      }
   };

   QLineEdit *mpQueryNameEdit;
   QTreeWidget *mpFieldList;
   QLineEdit *mpFormatStringEdit;
   
   LabeledSection *mpQuerySection;
   QLabel *mpNumberFile;
   QLabel *mpShapeType;

   QStackedWidget *mpDisplayOptionsStack;

   GraphicSymbolWidget *mpSymbolWidget;
   GraphicLineWidget *mpLineWidget;
   QWidget *mpPolygonWidget;
   GraphicLineWidget *mpPolygonLineWidget;
   GraphicFillWidget *mpPolygonFillWidget;
   QueryBuilderWidget *mpQueryBuilderWidget;
};

#endif
