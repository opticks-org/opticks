/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>

#include "FilterWidget.h"

FilterWidget::FilterWidget(QWidget* pParent) :
   QWidget(pParent)
{
   mpFilterEdit = new QLineEdit(this);
   mpWildcardCheck = new QCheckBox("Enable Wildcarding", this);
   mpCaseCheck = new QCheckBox("Case Sensitive", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpFilterEdit);
   pLayout->addWidget(mpWildcardCheck);
   pLayout->addWidget(mpCaseCheck);
}

FilterWidget::~FilterWidget()
{}

void FilterWidget::setFilter(const QRegExp& filter)
{
   mpFilterEdit->setText(filter.pattern());

   Qt::CaseSensitivity caseSensitivity = filter.caseSensitivity();
   mpCaseCheck->setChecked(caseSensitivity == Qt::CaseSensitive);

   QRegExp::PatternSyntax syntax = filter.patternSyntax();
   mpWildcardCheck->setChecked(syntax == QRegExp::Wildcard);
}

QRegExp FilterWidget::getFilter() const
{
   QString pattern = mpFilterEdit->text();
   Qt::CaseSensitivity caseSensitivity = mpCaseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   QRegExp::PatternSyntax syntax = mpWildcardCheck->isChecked() ? QRegExp::Wildcard : QRegExp::FixedString;

   return QRegExp(pattern, caseSensitivity, syntax);
}
