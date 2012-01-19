/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEASUREMENTTOOLBAR_H
#define MEASUREMENTTOOLBAR_H

#include <QtGui/QAction>
#include <QtGui/QButtonGroup>
#include <QtGui/QMenu>

#include "AttachmentPtr.h"
#include "PixmapGrid.h"
#include "PixmapGridButton.h"
#include "RasterElement.h"
#include "ToolBarAdapter.h"
#include "TypesFile.h"

class DistanceUnitsButton;
class Layer;
class LocationUnitsButton;
class MeasurementLayerImp;
class QString;

class MeasurementToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   /**
    *  Creates the Measurement toolbar.
    *
    *  @param    id
    *            The unique ID for the toolbar.
    *  @param    parent
    *            The main window to which the toolbar is attached.
    */
   MeasurementToolBar(const std::string& id, QWidget* parent = 0);

   /**
    *  Destroys the Measurement toolbar.
    */
   ~MeasurementToolBar();

   Layer* getMeasurementsLayer() const;

public slots:
   void setEnabled(bool bEnable);
   bool setMeasurementsLayer(Layer* pLayer);

protected:
   void georeferenceModified(Subject& subject, const std::string& signal, const boost::any& value);
   void measurementsLayerDeleted(Subject& subject, const std::string& signal, const boost::any& value);
   bool isGeoreferenced() const;

protected slots:
   void setDrawMode(bool on);
   void showLayer(bool bShow);
   void setDrawBearingLabel(bool drawLabels);
   void setDrawDistanceLabel(bool drawLabels);
   void setDrawEndPointsLabel(bool drawLabels);
   void setLocationUnit(GeocoordType geoType, DmsFormatType geoFormat);
   void setDistanceUnit(DistanceUnits unit);
   void redrawObjects();
   void setSelectionObject(GraphicObjectType eType);
   void updateGeoreference();
   void updateRaster();

private:
   MeasurementToolBar(const MeasurementToolBar& rhs);
   MeasurementToolBar& operator=(const MeasurementToolBar& rhs);
   QButtonGroup* mpObjectGroup;
   QAction* mpDrawAction;
   QAction* mpMoveAction;
   QAction* mpShowAction;
   QAction* mpBearingLabelAction;
   QAction* mpDistanceLabelAction;
   QAction* mpEndPointsLabelAction;

   // Location units actions
   LocationUnitsButton* mpLocationUnits;

   // Distance units actions
   DistanceUnitsButton* mpDistanceUnits;

   bool mbToolbarEnabled;
   MeasurementLayerImp* mpMeasurementsLayer;
   AttachmentPtr<RasterElement> mpRaster;
};

class DistanceUnitsGrid : public PixmapGrid
{
   Q_OBJECT

public:
   DistanceUnitsGrid(QWidget* pParent = NULL);
   void setCurrentValue(DistanceUnits value);
   DistanceUnits getCurrentValue() const;

signals: 
   void valueChanged(DistanceUnits value);

private slots:
   void translateChange(const QString&);

private:
   DistanceUnitsGrid(const DistanceUnitsGrid& rhs);
   DistanceUnitsGrid& operator=(const DistanceUnitsGrid& rhs);
};

class DistanceUnitsButton : public PixmapGridButton
{
   Q_OBJECT

public:
   DistanceUnitsButton(QWidget* pParent = NULL);

   void setCurrentValue(DistanceUnits value);
   DistanceUnits getCurrentValue() const;

signals:
   void valueChanged(DistanceUnits value);

private:
   DistanceUnitsButton(const DistanceUnitsButton& rhs);
   DistanceUnitsButton& operator=(const DistanceUnitsButton& rhs);
};

class LocationUnitsGrid : public PixmapGrid
{
   Q_OBJECT

public:
   LocationUnitsGrid(QWidget* pParent = NULL);
   void setCurrentValue(GeocoordType geoType, DmsFormatType geoFormat);
   void getCurrentValue(GeocoordType &geoType, DmsFormatType &geoFormat) const;
   void stringToLocationUnits(const QString &strVal, GeocoordType &geoType, DmsFormatType &geoFormat) const;

signals: 
   void valueChanged(GeocoordType geoType, DmsFormatType geoFormat);

private slots:
   void translateChange(const QString&);

private:
   LocationUnitsGrid(const LocationUnitsGrid& rhs);
   LocationUnitsGrid& operator=(const LocationUnitsGrid& rhs);
};

class LocationUnitsButton : public PixmapGridButton
{
   Q_OBJECT

public:
   LocationUnitsButton(QWidget* pParent = NULL);

   void setCurrentValue(GeocoordType geoType, DmsFormatType geoFormat);
   void getCurrentValue(GeocoordType &geoType, DmsFormatType &geoFormat) const;

signals:
   void valueChanged(GeocoordType geoType, DmsFormatType geoFormat);

private:
   LocationUnitsButton(const LocationUnitsButton& rhs);
   LocationUnitsButton& operator=(const LocationUnitsButton& rhs);
};

#endif
