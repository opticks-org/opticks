/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PanLimitTypeComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

PanLimitTypeComboBox::PanLimitTypeComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(NO_LIMIT)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(CUBE_EXTENTS)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(MAX_EXTENTS)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void PanLimitTypeComboBox::setCurrentValue(PanLimitType value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

PanLimitType PanLimitTypeComboBox::getCurrentValue() const
{
   PanLimitType retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<PanLimitType>(curText);
   }
   return retValue;
}

void PanLimitTypeComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      PanLimitType curType = StringUtilities::fromDisplayString<PanLimitType>(curText);
      emit valueChanged(curType);
   }
}