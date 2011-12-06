/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ZOOMANDPANTOPOINT_H
#define ZOOMANDPANTOPOINT_H

#include <QtGui/QDialog>

#include "LocationType.h"
#include "TypesFile.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class RasterElement;

class ZoomAndPanToPointDlg : public QDialog
{
   Q_OBJECT

public:
   ZoomAndPanToPointDlg(RasterElement* pRaster, GeocoordType coordType = GEOCOORD_GENERAL, QWidget* pParent = NULL);
   virtual ~ZoomAndPanToPointDlg();

   void setZoomPct(float pct);
   float getZoomPct() const;

   LocationType getCenter() const;

private:
   ZoomAndPanToPointDlg(const ZoomAndPanToPointDlg& rhs);
   ZoomAndPanToPointDlg& operator=(const ZoomAndPanToPointDlg& rhs);

   QLineEdit* mpLatitudeEdit;
   QLineEdit* mpLongitudeEdit;
   QLineEdit* mpZoneEdit;
   QLineEdit* mpHemEdit;
   QSpinBox* mpZoomBox;
   QPushButton* mpOK;
   GeocoordType mCoordType;

   QLabel* mpLatitudeLabel;
   QLabel* mpLongitudeLabel;
   QLabel* mpZoneLabel;
   QLabel* mpHemisphereLabel;

   RasterElement* mpRaster;

private slots:
   void allowOk();

   void latModified();
   void lonModified();
   void zoneModified();
   void hemModified();
   void coordTypeChanged(const QString& newCoordSelection);
};

#endif
