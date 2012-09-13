/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "InterpolationComboBox.h"
#include "StringUtilities.h"

#include <string>
#include <vector>

InterpolationComboBox::InterpolationComboBox(QWidget* pParent) :
   QComboBox(pParent)
{
   // Initialization
   setEditable(false);

   std::vector<std::string> items = StringUtilities::getAllEnumValuesAsDisplayString<InterpolationType>();
   for (std::vector<std::string>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      if (iter->empty() == false)
      {
         addItem(QString::fromStdString(*iter));
      }
   }

   // Connections
   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

InterpolationComboBox::~InterpolationComboBox()
{}

void InterpolationComboBox::setInterpolation(InterpolationType interp)
{
   QString strInterp = QString::fromStdString(StringUtilities::toDisplayString(interp));

   int index = findText(strInterp);
   setCurrentIndex(index);
}

InterpolationType InterpolationComboBox::getInterpolation() const
{
   InterpolationType interp;
   if (currentIndex() != -1)
   {
      std::string unitsText = currentText().toStdString();
      interp = StringUtilities::fromDisplayString<InterpolationType>(unitsText);
   }

   return interp;
}

void InterpolationComboBox::translateActivated(int index)
{
   if (index != -1)
   {
      std::string interpText = currentText().toStdString();
      InterpolationType interp = StringUtilities::fromDisplayString<InterpolationType>(interpText);
      emit interpolationActivated(interp);
   }
}
