/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLayout>

#include "ResultsOptionsWidget.h"

#include "PassAreaComboBox.h"
#include "StringUtilities.h"

ResultsOptionsWidget::ResultsOptionsWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Pass area
   QLabel* pPassAreaLabel = new QLabel("Pass Area:", this);
   mpPassAreaCombo = new PassAreaComboBox(this);

   // Thresholds
   QLabel* pFirstThresholdLabel = new QLabel("First Threshold:", this);
   mpFirstThresholdEdit = new QLineEdit(this);
   mpSecondThresholdLabel = new QLabel("Second Threshold:", this);
   mpSecondThresholdEdit = new QLineEdit(this);

   // Geocoord type
   mpCoordLabel = new QLabel("Coordinate Type:", this);
   mpCoordLabel->setEnabled(false);

   mpCoordCombo = new QComboBox(this);
   mpCoordCombo->setEditable(false);
   mpCoordCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(GEOCOORD_LATLON)));
   mpCoordCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(GEOCOORD_UTM)));
   mpCoordCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(GEOCOORD_MGRS)));
   mpCoordCombo->setEnabled(false);

   // Metadata
   mpMetadataCheck = new QCheckBox("Save Metadata", this);

   // Append to file
   mpAppendCheck = new QCheckBox("Append To File", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pPassAreaLabel, 1, 0);
   pGrid->addWidget(mpPassAreaCombo, 1, 1);
   pGrid->setColumnMinimumWidth(2, 10);
   pGrid->addWidget(mpMetadataCheck, 1, 3);
   pGrid->addWidget(pFirstThresholdLabel, 2, 0);
   pGrid->addWidget(mpFirstThresholdEdit, 2, 1);
   pGrid->addWidget(mpAppendCheck, 2, 3, Qt::AlignTop);
   pGrid->addWidget(mpSecondThresholdLabel, 3, 0);
   pGrid->addWidget(mpSecondThresholdEdit, 3, 1);
   pGrid->addWidget(mpCoordLabel, 4, 0);
   pGrid->addWidget(mpCoordCombo, 4, 1);
   pGrid->setRowStretch(5, 10);
   pGrid->setColumnStretch(3, 10);

   // Initialization
   mpMetadataCheck->setChecked(true);

   // Connections
   connect(mpPassAreaCombo, SIGNAL(valueChanged(PassArea)), this, SLOT(enableSecondThreshold(PassArea)));
}

ResultsOptionsWidget::~ResultsOptionsWidget()
{
}

void ResultsOptionsWidget::setAppendToFile(bool bAppend)
{
   mpAppendCheck->setChecked(bAppend);
}

bool ResultsOptionsWidget::appendToFile() const
{
   return mpAppendCheck->isChecked();
}

void ResultsOptionsWidget::enableMetadata(bool bEnable)
{
   mpMetadataCheck->setChecked(bEnable);
}

bool ResultsOptionsWidget::isMetadataEnabled() const
{
   return mpMetadataCheck->isChecked();
}

void ResultsOptionsWidget::setFirstThreshold(double dThreshold)
{
   QString strThreshold = QString::number(dThreshold);
   mpFirstThresholdEdit->setText(strThreshold);
}

double ResultsOptionsWidget::getFirstThreshold() const
{
   double dThreshold = 0.0;

   QString strThreshold = mpFirstThresholdEdit->text();
   if (strThreshold.isEmpty() == false)
   {
      dThreshold = strThreshold.toDouble();
   }

   return dThreshold;
}

void ResultsOptionsWidget::setSecondThreshold(double dThreshold)
{
   QString strThreshold = QString::number(dThreshold);
   mpSecondThresholdEdit->setText(strThreshold);
}

double ResultsOptionsWidget::getSecondThreshold() const
{
   double dThreshold = 0.0;

   QString strThreshold = mpSecondThresholdEdit->text();
   if (strThreshold.isEmpty() == false)
   {
      dThreshold = strThreshold.toDouble();
   }

   return dThreshold;
}

void ResultsOptionsWidget::setPassArea(PassArea passArea)
{
   mpPassAreaCombo->setCurrentValue(passArea);
   enableSecondThreshold(passArea);
}

PassArea ResultsOptionsWidget::getPassArea() const
{
   return mpPassAreaCombo->getCurrentValue();
}

void ResultsOptionsWidget::setGeocoordType(GeocoordType geocoordType)
{
   int iIndex = mpCoordCombo->findText(QString::fromStdString(StringUtilities::toDisplayString(geocoordType)));
   if (iIndex != -1)
   {
      mpCoordCombo->setCurrentIndex(iIndex);
   }
   mpCoordLabel->setEnabled(true);
   mpCoordCombo->setEnabled(true);
}

GeocoordType ResultsOptionsWidget::getGeocoordType() const
{
   GeocoordType eType = GEOCOORD_LATLON;

   QString strGeocoord = mpCoordCombo->currentText();
   if (strGeocoord.isEmpty() == false)
   {
      eType = StringUtilities::fromDisplayString<GeocoordType>(strGeocoord.toStdString());
   }

   return eType;
}

void ResultsOptionsWidget::enableSecondThreshold(PassArea area)
{
   bool bEnable = false;
   if ((area == MIDDLE) || (area == OUTSIDE))
   {
      bEnable = true;
   }

   mpSecondThresholdLabel->setEnabled(bEnable);
   mpSecondThresholdEdit->setEnabled(bEnable);
}
