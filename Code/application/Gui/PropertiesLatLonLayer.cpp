/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGroupBox>
#include <QtGui/QLayout>

#include "AppVersion.h"
#include "AppVerify.h"
#include "CustomColorButton.h"
#include "DmsFormatTypeComboBox.h"
#include "Font.h"
#include "LabeledSection.h"
#include "LatLonLayer.h"
#include "LatLonLayerImp.h"
#include "LatLonLineEdit.h"
#include "LatLonStyleComboBox.h"
#include "LineWidthComboBox.h"
#include "PropertiesLatLonLayer.h"
#include "Undo.h"
#include "View.h"

using namespace std;

PropertiesLatLonLayer::PropertiesLatLonLayer() :
   LabeledSectionGroup(NULL),
   mpLatLonLayer(NULL)
{
   // Coordinates
   QWidget* pCoordinatesWidget = new QWidget(this);

   QGroupBox* pTypeGroup = new QGroupBox("Type", pCoordinatesWidget);
   mpLatLonRadio = new QRadioButton("Latitude/Longitude:", pTypeGroup);
   mpLatLonFormatCombo = new DmsFormatTypeComboBox(pTypeGroup);
   mpUtmRadio = new QRadioButton("UTM", pTypeGroup);
   mpMgrsRadio = new QRadioButton("MGRS", pTypeGroup);

   QLabel* pFontLabel = new QLabel("Font:", pCoordinatesWidget);
   mpFontCombo = new QFontComboBox(pCoordinatesWidget);
   mpFontCombo->setEditable(false);

   QLabel* pFontSizeLabel = new QLabel("Font Size:", pCoordinatesWidget);
   mpFontSizeCombo = new QComboBox(pCoordinatesWidget);
   mpFontSizeCombo->setEditable(true);
   mpFontSizeCombo->setAutoCompletion(false);
   mpFontSizeCombo->setInsertPolicy(QComboBox::NoInsert);

   QList<int> fontSizes = QFontDatabase::standardSizes();
   for (int i = 0; i < fontSizes.count(); ++i)
   {
      QString strSize = QString::number(fontSizes[i]);
      mpFontSizeCombo->addItem(strSize);
   }

   LabeledSection* pCoordinatesSection = new LabeledSection(pCoordinatesWidget, "Coordinates", this);

   QGridLayout* pTypeGrid = new QGridLayout(pTypeGroup);
   pTypeGrid->setMargin(10);
   pTypeGrid->setSpacing(5);
   pTypeGrid->addWidget(mpLatLonRadio, 0, 0, 1, 2);
   pTypeGrid->addWidget(mpLatLonFormatCombo, 1, 1);
   pTypeGrid->addWidget(mpUtmRadio, 2, 0, 1, 2);
   pTypeGrid->addWidget(mpMgrsRadio, 3, 0, 1, 2);
   pTypeGrid->setColumnMinimumWidth(0, 15);

   QGridLayout* pFontLayout = new QGridLayout();
   pFontLayout->setMargin(0);
   pFontLayout->setSpacing(5);
   pFontLayout->addWidget(pFontLabel, 0, 0);
   pFontLayout->addWidget(mpFontCombo, 0, 1);
   pFontLayout->addWidget(pFontSizeLabel, 1, 0);
   pFontLayout->addWidget(mpFontSizeCombo, 1, 1, Qt::AlignLeft);
   pFontLayout->setRowStretch(2, 10);

   QHBoxLayout* pCoordinatesLayout = new QHBoxLayout(pCoordinatesWidget);
   pCoordinatesLayout->setMargin(0);
   pCoordinatesLayout->setSpacing(10);
   pCoordinatesLayout->addWidget(pTypeGroup);
   pCoordinatesLayout->addLayout(pFontLayout);
   pCoordinatesLayout->addStretch();

   // Gridlines
   QWidget* pGridlinesWidget = new QWidget(this);

   QLabel* pStyleLabel = new QLabel("Style:", pGridlinesWidget);
   mpStyleCombo = new LatLonStyleComboBox(pGridlinesWidget);

   QLabel* pWidthLabel = new QLabel("Width:", pGridlinesWidget);
   mpWidthCombo = new LineWidthComboBox(pGridlinesWidget);

   QLabel* pColorLabel = new QLabel("Color:", pGridlinesWidget);
   mpColorButton = new CustomColorButton(pGridlinesWidget);
   mpColorButton->usePopupGrid(true);

   QGroupBox* pSpacingGroup = new QGroupBox("Spacing", pGridlinesWidget);
   mpAutomaticRadio = new QRadioButton("Automatic", pSpacingGroup);
   mpCustomRadio = new QRadioButton("Custom", pSpacingGroup);
   mpLatitudeLabel = new QLabel(pSpacingGroup);
   mpLatitudeEdit = new LatLonLineEdit(pSpacingGroup);
   mpLongitudeLabel = new QLabel(pSpacingGroup);
   mpLongitudeEdit = new LatLonLineEdit(pSpacingGroup);

   LabeledSection* pGridlinesSection = new LabeledSection(pGridlinesWidget, "Gridlines", this);

   QGridLayout* pGridlinesGrid = new QGridLayout();
   pGridlinesGrid->setMargin(0);
   pGridlinesGrid->setSpacing(5);
   pGridlinesGrid->addWidget(pStyleLabel, 0, 0);
   pGridlinesGrid->addWidget(mpStyleCombo, 0, 1, Qt::AlignLeft);
   pGridlinesGrid->addWidget(pWidthLabel, 1, 0);
   pGridlinesGrid->addWidget(mpWidthCombo, 1, 1);
   pGridlinesGrid->addWidget(pColorLabel, 2, 0);
   pGridlinesGrid->addWidget(mpColorButton, 2, 1, Qt::AlignLeft);
   pGridlinesGrid->setRowStretch(3, 10);

   QGridLayout* pSpacingGrid = new QGridLayout(pSpacingGroup);
   pSpacingGrid->setMargin(10);
   pSpacingGrid->setSpacing(5);
   pSpacingGrid->addWidget(mpAutomaticRadio, 0, 0, 1, 3);
   pSpacingGrid->addWidget(mpCustomRadio, 1, 0, 1, 3);
   pSpacingGrid->addWidget(mpLatitudeLabel, 2, 1);
   pSpacingGrid->addWidget(mpLatitudeEdit, 2, 2);
   pSpacingGrid->addWidget(mpLongitudeLabel, 3, 1);
   pSpacingGrid->addWidget(mpLongitudeEdit, 3, 2);
   pSpacingGrid->setColumnMinimumWidth(0, 15);

   QHBoxLayout* pGridlinesLayout = new QHBoxLayout(pGridlinesWidget);
   pGridlinesLayout->setMargin(0);
   pGridlinesLayout->setSpacing(10);
   pGridlinesLayout->addLayout(pGridlinesGrid);
   pGridlinesLayout->addWidget(pSpacingGroup, 0, Qt::AlignLeft);
   pGridlinesLayout->addStretch();

   // Initialization
   addSection(pCoordinatesSection);
   addSection(pGridlinesSection);
   addStretch(10);
   setSizeHint(450, 325);

   // Connections
   VERIFYNR(connect(mpLatLonRadio, SIGNAL(toggled(bool)), mpLatLonFormatCombo, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpAutomaticRadio, SIGNAL(toggled(bool)), this, SLOT(autoTickSpacingEnabled(bool))));
   VERIFYNR(connect(mpLatLonRadio, SIGNAL(toggled(bool)), this, SLOT(unitsChangedToLatLon(bool))));
}

PropertiesLatLonLayer::~PropertiesLatLonLayer()
{}

bool PropertiesLatLonLayer::initialize(SessionItem* pSessionItem)
{
   mpLatLonLayer = dynamic_cast<LatLonLayer*>(pSessionItem);
   if (mpLatLonLayer == NULL)
   {
      return false;
   }

   // Coordinates
   const Font& font = mpLatLonLayer->getFont();
   mpFontCombo->setCurrentFont(font.getQFont());

   int fontSize = font.getPointSize();
   QList<int> fontSizes = QFontDatabase::standardSizes();

   int iIndex = fontSizes.indexOf(fontSize);
   if (iIndex != -1)
   {
      mpFontSizeCombo->setCurrentIndex(iIndex);
   }
   else if (fontSize > 0)
   {
      mpFontSizeCombo->setEditText(QString::number(fontSize));
   }
   else
   {
      mpFontSizeCombo->clearEditText();
   }

   GeocoordType coordType = mpLatLonLayer->getGeocoordType();
   switch (coordType)
   {
      case GEOCOORD_LATLON:
         mpLatLonRadio->setChecked(true);
         break;

      case GEOCOORD_UTM:
         mpUtmRadio->setChecked(true);
         break;

      case GEOCOORD_MGRS:
         mpMgrsRadio->setChecked(true);
         break;

      default:
         break;
   }

   DmsFormatType eFormat = mpLatLonLayer->getLatLonFormat();
   mpLatLonFormatCombo->setCurrentValue(eFormat);
   mpLatLonFormatCombo->setEnabled(coordType == GEOCOORD_LATLON);

   // Gridlines
   mpStyleCombo->setCurrentValue(mpLatLonLayer->getStyle());
   mpWidthCombo->setCurrentValue(mpLatLonLayer->getWidth());
   mpColorButton->setColor(mpLatLonLayer->getColor());

   setSpacing(coordType);

   bool autoSpacing = mpLatLonLayer->getAutoTickSpacing();
   if (autoSpacing == true)
   {
      mpAutomaticRadio->setChecked(true);
   }
   else
   {
      mpCustomRadio->setChecked(true);
   }

   return true;
}

void PropertiesLatLonLayer::setSpacing(GeocoordType coordType)
{
   LocationType tickSpacing = mpLatLonLayer->getTickSpacing();
   GeocoordType geoType = mpLatLonLayer->getGeocoordType();
   switch (coordType)
   {
      case GEOCOORD_LATLON:
      {
         if (geoType != GEOCOORD_LATLON)
         {
            tickSpacing.mX = 0.0;
            tickSpacing.mY = 0.0;
         }

         mpLatitudeLabel->setText("Latitude:");
         mpLatitudeEdit->setMaximumWidth(75);
         if (!mpAutomaticRadio->isChecked())
         {
            mpLatitudeEdit->setValue(DmsPoint(DmsPoint::DMS_DECIMAL, tickSpacing.mX));
         }
         else
         {
            mpLatitudeEdit->clear();
         }
         mpLatitudeEdit->setAutoUpdate(true);

         mpLongitudeLabel->setText("Longitude:");
         mpLongitudeEdit->setMaximumWidth(75);
         if (!mpAutomaticRadio->isChecked())
         {
            mpLongitudeEdit->setValue(DmsPoint(DmsPoint::DMS_DECIMAL, tickSpacing.mY));
         }
         else
         {
            mpLongitudeEdit->clear();
         }
         mpLongitudeEdit->setAutoUpdate(true);
         break;
      }

      case GEOCOORD_UTM:      // Intentional fall through
      case GEOCOORD_MGRS:
      {
         if (!(geoType == GEOCOORD_UTM || geoType == GEOCOORD_MGRS))
         {
            tickSpacing.mX = 0.0;
            tickSpacing.mY = 0.0;
         }

         mpLatitudeLabel->setText("Northing:");
         if (!mpAutomaticRadio->isChecked())
         {
            mpLatitudeEdit->setText(QString::number(tickSpacing.mY));
         }
         else
         {
            mpLatitudeEdit->clear();
         }
         mpLatitudeEdit->setAutoUpdate(false);

         mpLongitudeLabel->setText("Easting:");
         if (!mpAutomaticRadio->isChecked())
         {
            mpLongitudeEdit->setText(QString::number(tickSpacing.mX));
         }
         else
         {
            mpLongitudeEdit->clear();
         }
         mpLongitudeEdit->setAutoUpdate(false);
         break;
      }

      default:
         break;
   }
}

bool PropertiesLatLonLayer::applyChanges()
{
   if (mpLatLonLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpLatLonLayer->getView(), actionText);

   // Coordinates
   if (mpLatLonRadio->isChecked() == true)
   {
      mpLatLonLayer->setGeocoordType(GEOCOORD_LATLON);
      mpLatLonLayer->setLatLonFormat(mpLatLonFormatCombo->getCurrentValue());
   }
   else if (mpUtmRadio->isChecked() == true)
   {
      mpLatLonLayer->setGeocoordType(GEOCOORD_UTM);
   }
   else if (mpMgrsRadio->isChecked() == true)
   {
      mpLatLonLayer->setGeocoordType(GEOCOORD_MGRS);
   }
   else
   {
      return false;
   }

   QFont latLonFont(mpFontCombo->currentText(), mpFontSizeCombo->currentText().toInt());
   dynamic_cast<LatLonLayerImp*>(mpLatLonLayer)->setFont(latLonFont);

   // Gridlines
   mpLatLonLayer->setStyle(mpStyleCombo->getCurrentValue());
   mpLatLonLayer->setWidth(mpWidthCombo->getCurrentValue());
   mpLatLonLayer->setColor(QCOLOR_TO_COLORTYPE(mpColorButton->getColor()));

   LocationType tickSpacing;
   if (mpLatLonRadio->isChecked() == true)
   {
      tickSpacing.mX = mpLatitudeEdit->getValue();
      tickSpacing.mY = mpLongitudeEdit->getValue();
   }
   else
   {
      tickSpacing.mY = mpLatitudeEdit->text().toDouble();
      tickSpacing.mX = mpLongitudeEdit->text().toDouble();
   }

   mpLatLonLayer->setTickSpacing(tickSpacing);
   mpLatLonLayer->setAutoTickSpacing(mpAutomaticRadio->isChecked());

   // Refresh the view
   View* pView = mpLatLonLayer->getView();
   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

void PropertiesLatLonLayer::unitsChangedToLatLon(bool bToggled)
{
   if (bToggled)
   {
      setSpacing(GEOCOORD_LATLON);
   }
   else
   {
      setSpacing(GEOCOORD_UTM);
   }
}

const string& PropertiesLatLonLayer::getName()
{
   static string name = "Latitude/Longitude Layer Properties";
   return name;
}

const string& PropertiesLatLonLayer::getPropertiesName()
{
   static string propertiesName = "Latitude/Longitude Layer";
   return propertiesName;
}

const string& PropertiesLatLonLayer::getDescription()
{
   static string description = "General setting properties of a latitude/longitude layer";
   return description;
}

const string& PropertiesLatLonLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesLatLonLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesLatLonLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesLatLonLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesLatLonLayer::getDescriptorId()
{
   static string id = "{123B9826-BD1B-4BCE-9658-C086148C1842}";
   return id;
}

bool PropertiesLatLonLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesLatLonLayer::autoTickSpacingEnabled(bool bEnable)
{
   mpLatitudeLabel->setEnabled(!bEnable);
   mpLatitudeEdit->setEnabled(!bEnable);
   mpLongitudeLabel->setEnabled(!bEnable);
   mpLongitudeEdit->setEnabled(!bEnable);

   // Update the spacing values if necessary
   if (bEnable)
   {
      mpLatitudeEdit->clear();
      mpLongitudeEdit->clear();
   }
   else
   {
      if (mpLatLonRadio->isChecked() == true)
      {
         setSpacing(GEOCOORD_LATLON);
      }
      else if (mpUtmRadio->isChecked() == true)
      {
         setSpacing(GEOCOORD_UTM);
      }
      else if (mpMgrsRadio->isChecked() == true)
      {
         setSpacing(GEOCOORD_MGRS);
      }
   }
}
