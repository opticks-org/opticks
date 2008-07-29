/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PassAreaComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

PassAreaComboBox::PassAreaComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(LOWER)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(UPPER)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(MIDDLE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(OUTSIDE)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void PassAreaComboBox::setCurrentValue(PassArea value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

PassArea PassAreaComboBox::getCurrentValue() const
{
   PassArea retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<PassArea>(curText);
   }
   return retValue;
}

void PassAreaComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      PassArea curType = StringUtilities::fromDisplayString<PassArea>(curText);
      emit valueChanged(curType);
   }
}