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
#include <QtGui/QMenu>

#include "ToolBarAdapter.h"

#include "PixmapGrid.h"
#include "PixmapGridButton.h"
#include "TypesFile.h"

class AnnotationLayer;
class DistanceUnitsButton;
class Layer;

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
   void measurementsLayerDeleted(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void setDrawMode(bool on);
   void showLayer(bool bShow);
   void setDrawLabels(bool drawLabels);
   void setLocationUnit(const QString& locationIdentifier);
   void setDistanceUnit(DistanceUnits units);
   void redrawObjects();

private:
   QAction* mpDrawAction;
   QAction* mpShowAction;
   QAction* mpLabelsAction;

   // Location units actions
   PixmapGridButton* mpLocationUnits;

   // Distance units actions
   DistanceUnitsButton* mpDistanceUnits;

   AnnotationLayer* mpMeasurementsLayer;
};

class DistanceUnitsGrid : public PixmapGrid
{
   Q_OBJECT

public:
   DistanceUnitsGrid(QWidget* pParent);
   void setCurrentValue(DistanceUnits value);
   DistanceUnits getCurrentValue() const;

signals: 
   void valueChanged(DistanceUnits value);

private slots:
   void translateChange(const QString&);
};

class DistanceUnitsButton : public PixmapGridButton
{
   Q_OBJECT

public:
   DistanceUnitsButton(QWidget* pParent);

   void setCurrentValue(DistanceUnits value);
   DistanceUnits getCurrentValue() const;

signals:
   void valueChanged(DistanceUnits value);

private slots:
   void translateChange();
};

#endif
