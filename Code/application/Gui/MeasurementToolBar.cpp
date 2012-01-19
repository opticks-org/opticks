/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include "DesktopServicesImp.h"
#include "Layer.h"
#include "LayerList.h"
#include "MeasurementLayer.h"
#include "MeasurementLayerImp.h"
#include "MeasurementObjectImp.h"
#include "MeasurementToolBar.h"
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
   mpBearingLabelAction(NULL),
   mpDistanceLabelAction(NULL),
   mpEndPointsLabelAction(NULL),
   mpLocationUnits(NULL),
   mpDistanceUnits(NULL),
   mbToolbarEnabled(false),
   mpMeasurementsLayer(NULL)
{
   string shortcutContext = windowTitle().toStdString();

   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   REQUIRE(pDesktop != NULL);

   mpObjectGroup = new QButtonGroup(this);
   mpObjectGroup->setExclusive(true);

   QToolButton* pMoveButton = new QToolButton(this);
   mpMoveAction = new QAction(QIcon(":/icons/Pan"), "Move Measurement Object", this);
   pMoveButton->setDefaultAction(mpMoveAction);
   mpMoveAction->setAutoRepeat(false);
   mpMoveAction->setCheckable(true);
   mpMoveAction->setStatusTip("Allows selection, placement and resizing of measurement objects");
   mpObjectGroup->addButton(pMoveButton);
   addWidget(pMoveButton);

   QToolButton* pDrawButton = new QToolButton(this);
   mpDrawAction = new QAction(QIcon(":/icons/MeasurementMarker"), "Add Measurement Object", this);
   pDrawButton->setDefaultAction(mpDrawAction);
   mpDrawAction->setAutoRepeat(false);
   mpDrawAction->setCheckable(true);
   mpDrawAction->setStatusTip("Adds new measurement object to the layer");
   mpObjectGroup->addButton(pDrawButton);
   addWidget(pDrawButton);
   addSeparator();

   // Show/hide button
   mpShowAction = new QAction(QIcon(":/icons/ShowLayers"), "Show/Hide Layer", this);
   mpShowAction->setAutoRepeat(false);
   mpShowAction->setCheckable(true);
   mpShowAction->setStatusTip("Toggles the display of all measurement objects");
   addButton(mpShowAction, shortcutContext);

   // Show label buttons
   mpBearingLabelAction = new QAction(QIcon(":/icons/MeasurementBearingLabelOnOff"), "Show/Hide Bearing Label", this);
   mpBearingLabelAction->setAutoRepeat(false);
   mpBearingLabelAction->setCheckable(true);
   mpBearingLabelAction->setStatusTip("Allows the bearing label display in the active view to be toggled on/off");
   addButton(mpBearingLabelAction, shortcutContext);

   mpDistanceLabelAction = new QAction(QIcon(":/icons/MeasurementDistanceLabelOnOff"), "Show/Hide Distance Label", this);
   mpDistanceLabelAction->setAutoRepeat(false);
   mpDistanceLabelAction->setCheckable(true);
   mpDistanceLabelAction->setStatusTip("Allows the distance label display in the active view to be toggled on/off");
   addButton(mpDistanceLabelAction, shortcutContext);

   mpEndPointsLabelAction = new QAction(QIcon(":/icons/MeasurementEndPtsLabelOnOff"), "Show/Hide End Point Labels", this);
   mpEndPointsLabelAction->setAutoRepeat(false);
   mpEndPointsLabelAction->setCheckable(true);
   mpEndPointsLabelAction->setStatusTip("Allows the end point labels display in the active view to be toggled on/off");
   addButton(mpEndPointsLabelAction, shortcutContext);

   addSeparator();

   // Location units button
   mpLocationUnits = new LocationUnitsButton(this);
   mpLocationUnits->setStatusTip("Allows the units for the end point locations to be changed");
   mpLocationUnits->setToolTip("Select End Point Location Units");
   addWidget(mpLocationUnits);

   addSeparator();

   // Distance units button
   mpDistanceUnits = new DistanceUnitsButton(this);
   mpDistanceUnits->setStatusTip("Allows the units for the distance to be changed");
   mpDistanceUnits->setToolTip("Select Distance Units");
   addWidget(mpDistanceUnits);

   // Initialization
   mpShowAction->setChecked(true);
   mpDrawAction->setChecked(true);
   mpBearingLabelAction->setChecked(MeasurementLayer::getSettingDisplayBearingLabel());
   mpDistanceLabelAction->setChecked(MeasurementLayer::getSettingDisplayDistanceLabel());
   mpEndPointsLabelAction->setChecked(MeasurementLayer::getSettingDisplayEndPointsLabel());

   setLocationUnit(GEOCOORD_GENERAL, DMS_FULL);
   setDistanceUnit(NO_DISTANCE_UNIT);

   // Connections
   VERIFYNR(connect(mpShowAction, SIGNAL(toggled(bool)), this, SLOT(showLayer(bool))));
   VERIFYNR(connect(mpDrawAction, SIGNAL(toggled(bool)), this, SLOT(setDrawMode(bool))));
   VERIFYNR(connect(mpBearingLabelAction, SIGNAL(toggled(bool)), this, SLOT(setDrawBearingLabel(bool))));
   VERIFYNR(connect(mpDistanceLabelAction, SIGNAL(toggled(bool)), this, SLOT(setDrawDistanceLabel(bool))));
   VERIFYNR(connect(mpEndPointsLabelAction, SIGNAL(toggled(bool)), this, SLOT(setDrawEndPointsLabel(bool))));
   VERIFYNR(connect(mpLocationUnits, SIGNAL(valueChanged(GeocoordType, DmsFormatType)), 
      this, SLOT(setLocationUnit(GeocoordType, DmsFormatType))));
   VERIFYNR(connect(mpDistanceUnits, SIGNAL(valueChanged(DistanceUnits)), 
      this, SLOT(setDistanceUnit(DistanceUnits))));

   mpRaster.addSignal(SIGNAL_NAME(RasterElement, GeoreferenceModified), 
      Slot(this, &MeasurementToolBar::georeferenceModified));
}

MeasurementToolBar::~MeasurementToolBar()
{
}

Layer* MeasurementToolBar::getMeasurementsLayer() const
{
   return dynamic_cast<Layer*>(mpMeasurementsLayer);
}

void MeasurementToolBar::setEnabled(bool bEnable)
{
   mbToolbarEnabled = bEnable;
   mpMoveAction->setEnabled(bEnable);
   mpDrawAction->setEnabled(bEnable);
   mpShowAction->setEnabled(bEnable);
   mpBearingLabelAction->setEnabled(bEnable);
   mpDistanceLabelAction->setEnabled(bEnable);
   mpEndPointsLabelAction->setEnabled(bEnable);
   mpLocationUnits->setEnabled(bEnable && isGeoreferenced());
   mpDistanceUnits->setEnabled(bEnable && isGeoreferenced());

   if (bEnable)
   {
      setDrawMode(mpDrawAction->isChecked());
   }
}

bool MeasurementToolBar::setMeasurementsLayer(Layer* pLayer)
{
   MeasurementLayerImp* pMeasLayer = dynamic_cast<MeasurementLayerImp*>(pLayer);
   if (pLayer != NULL && pMeasLayer == NULL)
   {
      return false;
   }

   if (pMeasLayer == mpMeasurementsLayer)
   {
      return false;
   }

   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->detach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &MeasurementToolBar::measurementsLayerDeleted));
      VERIFY(disconnect(mpMeasurementsLayer, SIGNAL(currentTypeChanged(GraphicObjectType)),
         this, SLOT(setSelectionObject(GraphicObjectType))));
      VERIFY(disconnect(mpMeasurementsLayer, SIGNAL(objectAdded(GraphicObject*)),
         this, SLOT(updateRaster())));
      mpRaster.reset(NULL);
   }

   mpMeasurementsLayer = pMeasLayer;

   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*> (mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         bool bShow = pView->isMeasurementsLayerShown();
         mpShowAction->setChecked(bShow);

         LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL)
         {
            RasterElement* pElement = pLayerList->getPrimaryRasterElement();
            mpRaster.reset(pElement);
         }
      }

      mpBearingLabelAction->setChecked(mpMeasurementsLayer->getDisplayBearing());
      mpDistanceLabelAction->setChecked(mpMeasurementsLayer->getDisplayDistance());
      mpEndPointsLabelAction->setChecked(mpMeasurementsLayer->getDisplayEndPoints());

      if (mpMeasurementsLayer->getCurrentGraphicObjectType() == MEASUREMENT_OBJECT)
      {
         mpDrawAction->setChecked(true);
      }
      else
      {
         mpMoveAction->setChecked(true);
      }

      updateGeoreference();

      mpMeasurementsLayer->attach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &MeasurementToolBar::measurementsLayerDeleted));
      VERIFY(connect(mpMeasurementsLayer, SIGNAL(currentTypeChanged(GraphicObjectType)),
         this, SLOT(setSelectionObject(GraphicObjectType))));
      VERIFY(connect(mpMeasurementsLayer, SIGNAL(objectAdded(GraphicObject*)),
         this, SLOT(updateRaster())));
   }
   else  
   {
      updateGeoreference();
   }
   

   return true;
}

void MeasurementToolBar::measurementsLayerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<MeasurementLayerImp*>(&subject) == mpMeasurementsLayer)
   {
      setMeasurementsLayer(NULL);
      setEnabled(false);
   }
}

void MeasurementToolBar::setDrawMode(bool on)
{
   GraphicLayerImp* pGraphic = dynamic_cast<GraphicLayerImp*>(mpMeasurementsLayer);
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

void MeasurementToolBar::setDrawBearingLabel(bool drawLabels)
{
   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->setDisplayBearing(drawLabels);
      redrawObjects();
   }
}

void MeasurementToolBar::setDrawDistanceLabel(bool drawLabels)
{
   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->setDisplayDistance(drawLabels);
      redrawObjects();
   }
}

void MeasurementToolBar::setDrawEndPointsLabel(bool drawLabels)
{
   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->setDisplayEndPoints(drawLabels);
      redrawObjects();
   }
}

void MeasurementToolBar::setLocationUnit(GeocoordType geoType, DmsFormatType geoFormat)
{
   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->setGeocoordTypes(geoType, geoFormat);
   }

   mpLocationUnits->setCurrentValue(geoType, geoFormat);
   redrawObjects();
}

void MeasurementToolBar::setDistanceUnit(DistanceUnits unit)
{
   if (mpMeasurementsLayer != NULL)
   {
      mpMeasurementsLayer->setDistanceUnits(unit);
   }

   mpDistanceUnits->setCurrentValue(unit);
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

void MeasurementToolBar::setSelectionObject(GraphicObjectType eType)
{
   if (eType == MOVE_OBJECT)
   {
      mpMoveAction->setChecked(true);
   }
   else if (eType == MEASUREMENT_OBJECT)
   {
      mpDrawAction->setChecked(true);
   }
}

bool MeasurementToolBar::isGeoreferenced() const
{
   bool bGeoreferenced(false);
   if (mpRaster.get() != NULL)
   {
      bGeoreferenced = mpRaster->isGeoreferenced();
   }

   return bGeoreferenced;
}

void MeasurementToolBar::updateGeoreference()
{
   DistanceUnits distance;
   GeocoordType geocoord;
   DmsFormatType dms;
   if (mpMeasurementsLayer != NULL && isGeoreferenced())
   {
      distance = mpMeasurementsLayer->getDistanceUnits();
      mpMeasurementsLayer->getGeocoordTypes(geocoord, dms);
      mpDistanceUnits->setEnabled(mbToolbarEnabled);
      mpLocationUnits->setEnabled(mbToolbarEnabled);
   }
   else
   {
      distance = NO_DISTANCE_UNIT;
      geocoord = GEOCOORD_GENERAL;
      dms = DMS_FULL;
      mpDistanceUnits->setEnabled(false);
      mpLocationUnits->setEnabled(false);
   }

   // block buttons from sending to the MeasurementLayer. Prevent overriding non-georeferenced
   // MeasurementLayer default distance and location units.
   mpDistanceUnits->blockSignals(true);
   mpLocationUnits->blockSignals(true);
   mpDistanceUnits->setCurrentValue(distance);
   mpLocationUnits->setCurrentValue(geocoord, dms);
   mpDistanceUnits->blockSignals(false);
   mpLocationUnits->blockSignals(false);
}

void MeasurementToolBar::georeferenceModified(Subject& subject, const std::string& signal, 
                                              const boost::any& value)
{
   updateGeoreference();
}

void MeasurementToolBar::updateRaster()
{
   if (mpRaster.get() != NULL)
   {
      return;
   }

   if (mpMeasurementsLayer != NULL)
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*> (mpMeasurementsLayer->getView());
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL)
         {
            RasterElement* pElement = pLayerList->getPrimaryRasterElement();
            mpRaster.reset(pElement);
            updateGeoreference();
         }
      }
   }
}

DistanceUnitsGrid::DistanceUnitsGrid(QWidget* pParent)
: PixmapGrid(pParent)
{
   setNumRows(3);
   setNumColumns(3);

   setPixmap(0, 0, QPixmap(":/icons/MeasurementNoDistUnit"),
      QString::fromStdString(StringUtilities::toXmlString(NO_DISTANCE_UNIT)),
      QString::fromStdString(StringUtilities::toDisplayString(NO_DISTANCE_UNIT)));
   setPixmap(0, 1, QPixmap(":/icons/MeasurementKm"),
      QString::fromStdString(StringUtilities::toXmlString(KILOMETER)),
      QString::fromStdString(StringUtilities::toDisplayString(KILOMETER)));
   setPixmap(0, 2, QPixmap(":/icons/MeasurementStatMile"),
      QString::fromStdString(StringUtilities::toXmlString(MILE)),
      QString::fromStdString(StringUtilities::toDisplayString(MILE)));
   setPixmap(1, 0, QPixmap(":/icons/MeasurementNautMile"),
      QString::fromStdString(StringUtilities::toXmlString(NAUTICAL_MILE)),
      QString::fromStdString(StringUtilities::toDisplayString(NAUTICAL_MILE)));
   setPixmap(1, 1, QPixmap(":/icons/MeasurementMeter"),
      QString::fromStdString(StringUtilities::toXmlString(METER)),
      QString::fromStdString(StringUtilities::toDisplayString(METER)));
   setPixmap(1, 2, QPixmap(":/icons/MeasurementYard"),
      QString::fromStdString(StringUtilities::toXmlString(YARD)),
      QString::fromStdString(StringUtilities::toDisplayString(YARD)));
   setPixmap(2, 0, QPixmap(":/icons/MeasurementFoot"),
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
   VERIFYNR(connect(pGrid, SIGNAL(valueChanged(DistanceUnits)), this, SIGNAL(valueChanged(DistanceUnits))));
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

LocationUnitsGrid::LocationUnitsGrid(QWidget* pParent) :
   PixmapGrid(pParent)
{
   setNumRows(2);
   setNumColumns(3);

   setPixmap(0, 0, QPixmap(":/icons/MeasurementNoLocUnit"), "NoUnit", "No Unit");
   setPixmap(0, 1, QPixmap(":/icons/MeasurementDecDeg"), "DecimalDegrees", "Decimal Degrees");
   setPixmap(0, 2, QPixmap(":/icons/MeasurementDecMin"), "DecimalMinutes", "Decimal Minutes");
   setPixmap(1, 0, QPixmap(":/icons/MeasurementDms"), "DMS", "DMS");
   setPixmap(1, 1, QPixmap(":/icons/MeasurementUtm"), "UTM", "UTM");
   setPixmap(1, 2, QPixmap(":/icons/MeasurementMgrs"), "MGRS", "MGRS");

   // Set the current symbol
   setSelectedPixmap("NoUnit");

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void LocationUnitsGrid::setCurrentValue(GeocoordType geoType, DmsFormatType geoFormat)
{
   QString strValue;
   switch (geoType)
   {
   case GEOCOORD_LATLON:
      switch (geoFormat)
      {
      case DMS_FULL_DECIMAL:
         strValue = "DecimalDegrees";
         break;
      case DMS_MINUTES_DECIMAL:
         strValue = "DecimalMinutes";
         break;
      case DMS_FULL:
         strValue = "DMS";
         break;
      default:
         strValue = "NoUnit";
         break;
      }
      break;

   case GEOCOORD_UTM:
      strValue = "UTM";
      break;

   case GEOCOORD_MGRS:
      strValue = "MGRS";
      break;

   default:
      strValue = "NoUnit";
   }

   setSelectedPixmap(strValue);
}

void LocationUnitsGrid::getCurrentValue(GeocoordType &geoType, DmsFormatType &geoFormat) const
{
   QString curText = getSelectedPixmapIdentifier();
   if (!curText.isEmpty())
   {
      stringToLocationUnits(curText, geoType, geoFormat);
   }
}

void LocationUnitsGrid::translateChange(const QString& strText)
{
   GeocoordType geoType;
   DmsFormatType geoFormat;
   stringToLocationUnits(strText, geoType, geoFormat);

   emit valueChanged(geoType, geoFormat);
}

void LocationUnitsGrid::stringToLocationUnits(const QString &strVal, GeocoordType &geoType, 
                                              DmsFormatType &geoFormat) const
{
   geoType = GEOCOORD_GENERAL;
   geoFormat = DMS_FULL;

   if (strVal == "DecimalDegrees")
   {
      geoType = GEOCOORD_LATLON;
      geoFormat = DMS_FULL_DECIMAL;
   }
   else if (strVal == "DecimalMinutes")
   {
      geoType = GEOCOORD_LATLON;
      geoFormat = DMS_MINUTES_DECIMAL;
   }
   else if (strVal == "DMS")
   {
      geoType = GEOCOORD_LATLON;
      geoFormat = DMS_FULL;
   }
   else if (strVal == "UTM")
   {
      geoType = GEOCOORD_UTM;
   }
   else if (strVal == "MGRS")
   {
      geoType = GEOCOORD_MGRS;
   }
}

LocationUnitsButton::LocationUnitsButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   LocationUnitsGrid* pGrid = new LocationUnitsGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(pGrid, SIGNAL(valueChanged(GeocoordType, DmsFormatType)), 
      this, SIGNAL(valueChanged(GeocoordType, DmsFormatType))));
}

void LocationUnitsButton::setCurrentValue(GeocoordType geoType, DmsFormatType geoFormat)
{
   LocationUnitsGrid* pGrid = dynamic_cast<LocationUnitsGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(geoType, geoFormat);
   }
}

void LocationUnitsButton::getCurrentValue(GeocoordType &geoType, DmsFormatType &geoFormat) const
{
   LocationUnitsGrid* pGrid = dynamic_cast<LocationUnitsGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->getCurrentValue(geoType, geoFormat);
   }
}
