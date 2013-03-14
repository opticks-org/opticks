/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "LatLonLineEdit.h"

#include <string>
using namespace std;

LatLonLineEdit::LatLonLineEdit(QWidget* parent) :
   QLineEdit(parent),
   mAutoUpdate(true),
   mDisplayType(DmsPoint::DMS_DECIMAL)
{
}

LatLonLineEdit::LatLonLineEdit(DmsPoint dmsPoint, QWidget* parent) :
   QLineEdit(parent),
   mAutoUpdate(true),
   mDisplayType(DmsPoint::DMS_DECIMAL)
{
   setValue(dmsPoint);
}

LatLonLineEdit::~LatLonLineEdit()
{
}

void LatLonLineEdit::setValue(DmsPoint dmsPoint)
{
   QString strValue;

   string value = dmsPoint.getValueText();
   if (value.empty() == false)
   {
      strValue = QString::fromStdString(value);
   }

   mDisplayType = dmsPoint.getType();
   setText(strValue);
}

double LatLonLineEdit::getValue() const
{
   double value = 0.0;

   QString strValue = text();
   if (strValue.isEmpty() == false)
   {
      DmsPoint dmsPoint(mDisplayType, strValue.toStdString());
      value = dmsPoint.getValue();
   }

   return value;
}

void LatLonLineEdit::setAutoUpdate(bool bAutoUpdate)
{
   mAutoUpdate = bAutoUpdate;
}

bool LatLonLineEdit::hasAutoUpdate() const
{
   return mAutoUpdate;
}

void LatLonLineEdit::focusOutEvent(QFocusEvent* pEvent)
{
   if (mAutoUpdate == true)
   {
      DmsPoint dmsPoint(mDisplayType, getValue());
      setValue(dmsPoint);
   }

   QLineEdit::focusOutEvent(pEvent);
}
