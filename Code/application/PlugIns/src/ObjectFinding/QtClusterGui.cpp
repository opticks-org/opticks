/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "QtClusterGui.h"
#include "StringUtilities.h"
#include "StringUtilitiesMacros.h"
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLineEdit>

namespace StringUtilities
{
BEGIN_ENUM_MAPPING(DisplayType)
ADD_ENUM_MAPPING(CENTROID, "Centroid", "centroid")
ADD_ENUM_MAPPING(BOUNDARY, "Boundary", "boundary")
ADD_ENUM_MAPPING(CENTROID_AND_BOUNDARY, "Centroid and Boundary", "centroid_and_boundary")
ADD_ENUM_MAPPING(PSEUDO, "Pseudocolor", "pseudocolor")
END_ENUM_MAPPING()
}

QtClusterGui::QtClusterGui(QWidget* pParent) : QDialog(pParent)
{
   mpClusterSize = new QDoubleSpinBox(this);
   mpClusterSize->setDecimals(1);
   mpClusterSize->setRange(0.1, 10000.0);
   mpClusterSize->setSuffix(" pixels");

   mpResultName = new QLineEdit(this);

   mpDisplayType = new QComboBox(this);
   std::vector<std::string> vals = StringUtilities::getAllEnumValuesAsDisplayString<DisplayType>();
   for (std::vector<std::string>::const_iterator val = vals.begin(); val != vals.end(); ++val)
   {
      mpDisplayType->addItem(QString::fromStdString(*val));
   }

   QDialogButtonBox* pButtons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

   QFormLayout* pTopLevel = new QFormLayout(this);
   pTopLevel->addRow("Cluster radius", mpClusterSize);
   pTopLevel->addRow("Result layer name", mpResultName);
   pTopLevel->addRow("Result type", mpDisplayType);
   pTopLevel->addRow(pButtons);

   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(reject())));

   // some reasonable default
   setDisplayType(CENTROID);
}

QtClusterGui::~QtClusterGui()
{
}

double QtClusterGui::getClusterSize() const
{
   return mpClusterSize->value();
}

QString QtClusterGui::getResultName() const
{
   return mpResultName->text();
}

DisplayType QtClusterGui::getDisplayType() const
{
   return StringUtilities::fromDisplayString<DisplayType>(
      mpDisplayType->currentText().toStdString());
}

void QtClusterGui::setClusterSize(double size)
{
   mpClusterSize->setValue(size);
}

void QtClusterGui::setResultName(const QString& name)
{
   mpResultName->setText(name);
}

void QtClusterGui::setDisplayType(DisplayType type)
{
   if (type.isValid())
   {
      mpDisplayType->setCurrentIndex(mpDisplayType->findText(
         QString::fromStdString(StringUtilities::toDisplayString(type))));
   }
}