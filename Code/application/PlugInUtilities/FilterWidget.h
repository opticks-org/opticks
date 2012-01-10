/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QtCore/QRegExp>
#include <QtGui/QWidget>

class QCheckBox;
class QLineEdit;

class FilterWidget : public QWidget
{
public:
   FilterWidget(QWidget* pParent = NULL);
   virtual ~FilterWidget();

   void setFilter(const QRegExp& filter);
   QRegExp getFilter() const;

private:
   FilterWidget(const FilterWidget& rhs);
   FilterWidget operator=(const FilterWidget& rhs);
   QLineEdit* mpFilterEdit;
   QCheckBox* mpWildcardCheck;
   QCheckBox* mpCaseCheck;
};

#endif
