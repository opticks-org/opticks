/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ArcRegionComboBox.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

using namespace std;

ArcRegionComboBox::ArcRegionComboBox(QWidget* pParent)
: QComboBox(pParent)
{
   setEditable(false);

   addItem(QString::fromStdString(StringUtilities::toDisplayString(ARC_CENTER)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(ARC_CHORD)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(ARC_OPEN)));

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

void ArcRegionComboBox::setCurrentValue(ArcRegion value)
{
   QString strValue = QString::fromStdString(StringUtilities::toDisplayString(value));
   int index = findText(strValue);
   setCurrentIndex(index);
}

ArcRegion ArcRegionComboBox::getCurrentValue() const
{
   ArcRegion retValue;
   int index = currentIndex();
   if (index != -1)
   {
      string curText = currentText().toStdString();
      retValue = StringUtilities::fromDisplayString<ArcRegion>(curText);
   }
   return retValue;
}

void ArcRegionComboBox::translateActivated(int newIndex)
{
   if (newIndex != -1)
   {
      string curText = currentText().toStdString();
      ArcRegion curType = StringUtilities::fromDisplayString<ArcRegion>(curText);
      emit valueChanged(curType);
   }
}