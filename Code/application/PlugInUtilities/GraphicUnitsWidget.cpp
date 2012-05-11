/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "AppVerify.h"
#include "GraphicUnitsWidget.h"
#include "StringUtilities.h"

#include <limits>
using namespace std;

GraphicUnitsWidget::GraphicUnitsWidget(QWidget* pParent) :
   QWidget(pParent)
{
   QLabel* pUnitsLabel = new QLabel("Unit System:", this);
   mpUnitSystemComboBox = new QComboBox(this);

   // Units
   vector<string> values = StringUtilities::getAllEnumValuesAsDisplayString<UnitSystem>();
   for (vector<string>::iterator iter = values.begin();
        iter != values.end();
        ++iter)
   {
      mpUnitSystemComboBox->addItem(QString::fromStdString(*iter));
   }
   mpUnitSystemComboBox->setEditable(false);

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pUnitsLabel);
   pLayout->addWidget(mpUnitSystemComboBox, 10);

   // Connections
   VERIFYNR(connect(mpUnitSystemComboBox, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

UnitSystem GraphicUnitsWidget::getUnitSystem() const
{
   string textValue = mpUnitSystemComboBox->currentText().toStdString();
   return StringUtilities::fromDisplayString<UnitSystem>(textValue);
}

void GraphicUnitsWidget::setUnitSystem(UnitSystem units)
{
   int index = -1;
   if (units.isValid())
   {
      QString strValue = QString::fromStdString(StringUtilities::toDisplayString(units));
      index = mpUnitSystemComboBox->findText(strValue);
   }
   mpUnitSystemComboBox->setCurrentIndex(index);
}

void GraphicUnitsWidget::translateActivated(int newIndex)
{
   emit unitSystemChanged(getUnitSystem());
}