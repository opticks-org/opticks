/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include "MeasurementToolBar.h"
#include "AnnotationLayer.h"
#include "AnnotationLayerImp.h"
#include "AppAssert.h"
#include "DesktopServicesImp.h"
#include "Icons.h"
#include "Layer.h"
#include "LayerList.h"
#include "MeasurementObjectImp.h"
#include "PixmapGrid.h"
#include "PixmapGridButton.h"
#include "ProductView.h"
#include "RasterElement.h"
#include "SpatialDataViewImp.h"

#include <string>
using namespace std;

MeasurementToolBar::MeasurementToolBar(const string& id, QWidget* parent) :
   ToolBarAdapter(id, "Measurement", parent),
   mpDrawAction(NULL),
   mpShowAction(NULL),
   mpLabelsAction(NULL),
   mpLocationUnits(NULL),
   mpDistanceUnits(NULL),
   mpMeasurementsLayer(NULL)
{
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);

   string shortcutContext = windowTitle().toStdString();

   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   REQUIRE(pDesktop != NULL);

   // Draw button
   mpDrawAction = new QAction(pIcons->mMeasurementMarker, "Measurement Draw Mode", this);
   mpDrawAction->setAutoRepeat(false);
   mpDrawAction->setCheckable(true);
   mpDrawAction->setStatusTip("Allows items in the measurement overlay to be drawn");
   addButton(mpDrawAction, shortcutContext);

   addSeparator();

   // Show/hide button
   mpShowAction = new QAction(pIcons->mShowLayers, "Show/Hide Layer", this);
   mpShowAction->setAutoRepeat(false);
   mpShowAction->setCheckable(true);
   mpShowAction->setStatusTip("Toggles the display of all measurement objects");
   addButton(mpShowAction, shortcutContext);

   // Show labels button
   mpLabelsAction = new QAction(pIcons->mMeasurementLabelsOnOff, "Show/Hide Labels", this);
   mpLabelsAction->setAutoRepeat(false);
   mpLabelsAction->setCheckable(true);
   mpLabelsAction->setStatusTip("Allows label display in the active view to be toggled on/off");
   addButton(mpLabelsAction, shortcutContext);

   // Location units button
   PixmapGrid* pLocationGrid = new PixmapGrid(this);
   pLocationGrid->setCellTracking(true);
   pLocationGrid->setNumRows(2);
   pLocationGrid->setNumColumns(3);
   pLocationGrid->setPixmap(0, 0, pIcons->mMeasurementNoLocUnit, "NoUnit", "No Unit");
   pLocationGrid->setPixmap(0, 1, pIcons->mMeasurementDecDeg, "DecimalDegrees", "Decimal Degrees");
   pLocationGrid->setPixmap(0, 2, pIcons->mMeasurementDecMin, "DecimalMinutes", "Decimal Minutes");
   pLocationGrid->setPixmap(1, 0, pIcons->mMeasurementDms, "DMS", "DMS");
   pLocationGrid->setPixmap(1, 1, pIcons->mMeasurementUtm, "UTM", "UTM");
   pLocationGrid->setPixmap(1, 2, pIcons->mMeasurementMgrs, "MGRS", "MGRS");

   mpLocationUnits = new PixmapGridButton(pLocationGrid, true, this);
   mpLocationUnits->setStatusTip("Allows the units for the location to be changed");
   mpLocationUnits->setToolTip("Location Units");
   addWidget(mpLocationUnits);

   addSeparator();

   // Distance units button
   mpDistanceUnits = new DistanceUnitsButton(this);
   mpDistanceUnits->setStatusTip("Allows the units for the distance to be changed");
   mpDistanceUnits->setToolTip("Distance Units");
   addWidget(mpDistanceUnits);

   // Initialization
   mpShowAction->setChecked(true);
   mpDrawAction->setChecked(true);
   mpLabelsAction->setChecked(MeasurementObjectImp::getDrawLabels());

   setDrawMode(true);
   setLocationUnit("NoUnit");
   setDistanceUnit(NO_DISTANCE_UNIT);

   // Connections
   connect(mpShowAction, SIGNAL(toggled(bool)), this, SLOT(showLayer(bool)));
   connect(mpDrawAction, SIGNAL(toggled(bool)), this, SLOT(setDrawMode(bool)));
   connect(mpLabelsAction, SIGNAL(toggled(bool)), this, SLOT(setDrawLabels(bool)));
   connect(mpLocationUnits, SIGNAL(valueChanged(const QString&)), this, SLOT(setLocationUnit(const QString&)));
   connect(mpDistanceUnits, SIGNAL(valueChanged(DistanceUnits)), this, SLOT(setDistanceUnit(DistanceUnits)));
}

MeasurementToolBar::~MeasurementToolBar()
{
}

Layer* MeasurementToolBar::getMeasurementsLayer() const
{
   return mpMeasurementsLayer;
}

void MeasurementToolBar::setEnabled(bool bEnable)
{
   mpDrawAction->setEnabled(bEnable);
   mpShowAction->setEnabled(bEnable);
   mpLabelsAction->setEnabled(bEnable);
   mpLocationUnits->setEnabled(bEnable);
   mpDistanceUnits->setEnabled(bEnable);

   if (bEnable == false)
   {
      DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
      if (pDesktop != NULL)
      {
         pDesktop->setAnnotationObject(MOVE_OBJECT);
      }
   }
   else
   {
      setDrawMode(mpDrawAction->isChecked());
   }
}

bool MeasurementToolBar::setMeasurementsLayer(Layer* pLayer)
{
   if (pLayer == mpMeasurementsLayer)
   {
      return false;
   }

   if (pLayer != NULL)
   {
      if (pLayer->getLayerType() != ANNOTATION)
      {
         return false;
      }
   }

   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->detach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &MeasurementToolBar::measurementsLayerDeleted));
   }

   mpMeasurementsLayer = static_cast<AnnotationLayer*> (pLayer);

   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*> (mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         bool bShow = pView->isMeasurementsLayerShown();
         mpShowAction->setChecked(bShow);
      }

      mpMeasurementsLayer->attach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &MeasurementToolBar::measurementsLayerDeleted));
   }

   return true;
}

void MeasurementToolBar::measurementsLayerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<AnnotationLayer*>(&subject) == mpMeasurementsLayer)
   {
      setMeasurementsLayer(NULL);
   }
}

void MeasurementToolBar::setDrawMode(bool on)
{
   GraphicLayerImp *pGraphic = dynamic_cast<GraphicLayerImp*>(
      mpMeasurementsLayer);
   if (pGraphic != NULL)
   {
      if (on)
      {
         pGraphic->setCurrentGraphicObjectType(MEASUREMENT_OBJECT);
      }
      else
      {
         pGraphic->setCurrentGraphicObjectType(MOVE_OBJECT);
      }
   }
}

void MeasurementToolBar::showLayer(bool bShow)
{
   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*> (mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         pView->showMeasurementsLayer(bShow);
         pView->refresh();
      }
   }
}

void MeasurementToolBar::setDrawLabels(bool drawLabels)
{
   MeasurementObjectImp::setDrawLabels(drawLabels);
   redrawObjects();
}

void MeasurementToolBar::setLocationUnit(const QString& locationIdentifier)
{
   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         VERIFYNRV(pLayerList != NULL);

         RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
         if (pRaster != NULL)
         {
            if (pRaster->isGeoreferenced() == false)
            {
               QMessageBox::warning(pView, QString::fromStdString(mpMeasurementsLayer->getName()),
                  "The change to the location units will not take effect until the data set is georeferenced.");
            }
         }
      }
   }

   mpLocationUnits->setCurrentIdentifier(locationIdentifier);
   GeocoordType geocoord = GEOCOORD_GENERAL;
   DmsFormatType dmsFormat = DMS_FULL;

   if (locationIdentifier == "DecimalDegrees")
   {
      geocoord = GEOCOORD_LATLON;
      dmsFormat = DMS_FULL_DECIMAL;
   }
   else if (locationIdentifier == "DecimalMinutes")
   {
      geocoord = GEOCOORD_LATLON;
      dmsFormat = DMS_MINUTES_DECIMAL;
   }
   else if (locationIdentifier == "DMS")
   {
      geocoord = GEOCOORD_LATLON;
      dmsFormat = DMS_FULL;
   }
   else if (locationIdentifier == "UTM")
   {
      geocoord = GEOCOORD_UTM;
   }
   else if (locationIdentifier == "MGRS")
   {
      geocoord = GEOCOORD_MGRS;
   }

   MeasurementObjectImp::setGeocoordTypes(geocoord, dmsFormat);
   redrawObjects();
}

void MeasurementToolBar::setDistanceUnit(DistanceUnits unit)
{
   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         VERIFYNRV(pLayerList != NULL);

         RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
         if (pRaster != NULL)
         {
            if (pRaster->isGeoreferenced() == false)
            {
               QMessageBox::warning(pView, QString::fromStdString(mpMeasurementsLayer->getName()),
                  "The change to the distance units will not take effect until the data set is georeferenced.");
            }
         }
      }
   }

   mpDistanceUnits->setCurrentValue(unit);
   MeasurementObjectImp::setDistanceUnit(unit);
   redrawObjects();
}

void MeasurementToolBar::redrawObjects()
{
   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*> (mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}

DistanceUnitsGrid::DistanceUnitsGrid(QWidget* pParent)
: PixmapGrid(pParent)
{
   setNumRows(3);
   setNumColumns(3);

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setPixmap(0, 0, pIcons->mMeasurementNoDistUnit,
      QString::fromStdString(StringUtilities::toXmlString(NO_DISTANCE_UNIT)),
      QString::fromStdString(StringUtilities::toDisplayString(NO_DISTANCE_UNIT)));
   setPixmap(0, 1, pIcons->mMeasurementKm,
      QString::fromStdString(StringUtilities::toXmlString(KILOMETER)),
      QString::fromStdString(StringUtilities::toDisplayString(KILOMETER)));
   setPixmap(0, 2, pIcons->mMeasurementStatMile,
      QString::fromStdString(StringUtilities::toXmlString(MILE)),
      QString::fromStdString(StringUtilities::toDisplayString(MILE)));
   setPixmap(1, 0, pIcons->mMeasurementNautMile,
      QString::fromStdString(StringUtilities::toXmlString(NAUTICAL_MILE)),
      QString::fromStdString(StringUtilities::toDisplayString(NAUTICAL_MILE)));
   setPixmap(1, 1, pIcons->mMeasurementMeter,
      QString::fromStdString(StringUtilities::toXmlString(METER)),
      QString::fromStdString(StringUtilities::toDisplayString(METER)));
   setPixmap(1, 2, pIcons->mMeasurementYard,
      QString::fromStdString(StringUtilities::toXmlString(YARD)),
      QString::fromStdString(StringUtilities::toDisplayString(YARD)));
   setPixmap(2, 0, pIcons->mMeasurementFoot,
      QString::fromStdString(StringUtilities::toXmlString(FOOT)),
      QString::fromStdString(StringUtilities::toDisplayString(FOOT)));

   // Set the current symbol
   setSelectedPixmap(QString::fromStdString(StringUtilities::toXmlString(NO_DISTANCE_UNIT)));

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void DistanceUnitsGrid::setCurrentValue(DistanceUnits value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

DistanceUnits DistanceUnitsGrid::getCurrentValue() const
{
   DistanceUnits retValue;
   string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<DistanceUnits>(curText);
   }
   return retValue;
}

void DistanceUnitsGrid::translateChange(const QString& strText)
{
   DistanceUnits curType = StringUtilities::fromXmlString<DistanceUnits>(strText.toStdString());
   emit valueChanged(curType);
}

DistanceUnitsButton::DistanceUnitsButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   DistanceUnitsGrid* pGrid = new DistanceUnitsGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(this, SIGNAL(valueChanged(const QString&)), this, SLOT(translateChange())));
}

void DistanceUnitsButton::setCurrentValue(DistanceUnits value)
{
   DistanceUnitsGrid* pGrid = dynamic_cast<DistanceUnitsGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

DistanceUnits DistanceUnitsButton::getCurrentValue() const
{
   DistanceUnits retValue;
   DistanceUnitsGrid* pGrid = dynamic_cast<DistanceUnitsGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}

void DistanceUnitsButton::translateChange()
{
   DistanceUnits distUnits = getCurrentValue();
   emit valueChanged(distUnits);
}
