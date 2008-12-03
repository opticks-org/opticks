/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsSample.h"

#include "ConfigurationSettings.h"
#include "LabeledSection.h"
#include "UtilityServices.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsSample::OptionsSample() :
   QWidget(NULL)
{
   // Section 1
   QLabel* pTextLabel = new QLabel("Text Setting:", this);
   mpTextSetting = new QLineEdit(this);
   QWidget* pSection1Widget = new QWidget(this);
   QHBoxLayout* pSection1Layout = new QHBoxLayout(pSection1Widget);
   pSection1Layout->setMargin(0);
   pSection1Layout->setSpacing(5);
   pSection1Layout->addWidget(pTextLabel);
   pSection1Layout->addWidget(mpTextSetting);
   pSection1Layout->addStretch(10);
   LabeledSection* pSection1 = new LabeledSection(pSection1Widget, "Section 1", this);

   // Section 2
   QLabel* pIntegerLabel = new QLabel("Positive Integer Settings:", this);
   mpIntSetting = new QSpinBox(this);
   mpIntSetting->setMinimum(0);
   mpIntSetting->setMaximum(numeric_limits<int>::max());
   QWidget* pSection2Widget = new QWidget(this);
   QGridLayout* pSection2Layout = new QGridLayout(pSection2Widget);
   pSection2Layout->setMargin(0);
   pSection2Layout->setSpacing(5);
   pSection2Layout->addWidget(pIntegerLabel, 0, 0);
   pSection2Layout->addWidget(mpIntSetting, 0, 1, Qt::AlignLeft);
   pSection2Layout->setColumnStretch(1, 10);
   pSection2Layout->setRowStretch(1, 10);
   LabeledSection* pSection2 = new LabeledSection(pSection2Widget, "Section 2", this);

   // Section 3
   mpBoolSetting = new QCheckBox("Boolean Setting", this);
   LabeledSection* pSection3 = new LabeledSection(mpBoolSetting, "Section 3", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pSection1);
   pLayout->addWidget(pSection2);
   pLayout->addWidget(pSection3);
   pLayout->addStretch(10);

   // Initialize From Settings
   if (OptionsSample::hasSettingTestTextSetting())
   {
      mpTextSetting->setText(QString::fromStdString(OptionsSample::getSettingTestTextSetting()));
   }
   if (OptionsSample::hasSettingTestIntSetting())
   {
      mpIntSetting->setValue(OptionsSample::getSettingTestIntSetting());
   }
   if (OptionsSample::hasSettingTestBoolSetting())
   {
      mpBoolSetting->setChecked(OptionsSample::getSettingTestBoolSetting());
   }
}
   
void OptionsSample::applyChanges()
{  
   OptionsSample::setSettingTestTextSetting(mpTextSetting->text().toStdString());
   OptionsSample::setSettingTestIntSetting(mpIntSetting->value());
   OptionsSample::setSettingTestBoolSetting(mpBoolSetting->isChecked());
}

OptionsSample::~OptionsSample()
{
}
