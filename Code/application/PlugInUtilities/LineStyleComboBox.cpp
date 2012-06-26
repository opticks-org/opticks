/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LineStyleComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

LineStyleComboBox::LineStyleComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(SOLID_LINE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(DASHED)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(DOT)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(DASH_DOT)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(DASH_DOT_DOT)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void LineStyleComboBox::setCurrentValue(LineStyle value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

LineStyle LineStyleComboBox::getCurrentValue() const
{
   LineStyle retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<LineStyle>(curText);
   }
   return retValue;
}

void LineStyleComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      LineStyle curType = StringUtilities::fromDisplayString<LineStyle>(curText);
      emit valueChanged(curType);
   }
}