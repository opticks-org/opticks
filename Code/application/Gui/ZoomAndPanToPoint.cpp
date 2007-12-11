/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>

#include "TypesFile.h"
#include "GeoPoint.h"
#include "RasterElement.h"
#include "ZoomAndPanToPoint.h"

using namespace std;

const int ZOOMBOX_MIN = 1;
const int ZOOMBOX_MAX = 5000;

// =============================================================================
ZoomAndPanToPointDlg::ZoomAndPanToPointDlg(RasterElement *pRaster, GeocoordType coordType, QWidget* parent) :
   QDialog(parent),
   mCoordType(coordType),
   mpRaster(pRaster),
   mpLatitudeEdit(NULL),
   mpLongitudeEdit(NULL),
   mpZoneEdit(NULL),
   mpHemEdit(NULL)
{
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);

   QString tipStrLat = "Accepts the following format/s:\n\n";
   QString tipStrLon(tipStrLat);
   QString tipStrZon(tipStrLat);
   QString tipStrHem(tipStrLat);
   switch (mCoordType)
   {
   case GEOCOORD_MGRS:
      mpLatitudeEdit = new QLineEdit(this);
      if (mpLatitudeEdit != NULL)
      {
         tipStrLat += "GGGSSE..EN..N\nG = Grid Zone Designation\nS = 100,000-meter square id\n";
         tipStrLat += "E|N = digits designating meters easting and northing\n\n";
         tipStrLat += "18TUU8401 - zone 18T, square UU, E84000, N1000\n";
         tipStrLat += "18TUU845016 - zone 18T, square UU, E84500, N1600\n";
         mpLatitudeEdit->setToolTip(tipStrLat);
      }
      pGrid->addWidget(mpLatitudeEdit, 1, 0, 1, 4);
      break;
   case GEOCOORD_LATLON:
      mpLatitudeEdit = new QLineEdit(this);
      if (mpLatitudeEdit != NULL)
      {
         tipStrLat += "space delimited:  DD MM SS.SSS,   e.g. 40 6 31.982\n";
         tipStrLat += "dms delimited:    DDdMMmSS.SSSs,  e.g. 40d6m31.982s\n";
         tipStrLat += "symbol delimited: DD°MM'SS.SSS\", e.g. 40°6'31.982\"\n";
         tipStrLat += "decimal degrees:  DD.DDDDDD,      e.g. 40.108884\n";
         tipStrLat += "concatenated:     DDMMSS.SSS,     e.g. 400631.982\n";
         tipStrLat += "   Note: 2 chars for minutes and whole seconds, i.e. 06, not 6\n\n";
         tipStrLat += "Preface with 'S', 's' or '-' for southern hemisphere";
         mpLatitudeEdit->setToolTip(tipStrLat);
      }

      mpLongitudeEdit = new QLineEdit(this);
      if (mpLongitudeEdit != NULL)
      {
         tipStrLon += "space delimited:  DDD MM SS.SSS,   e.g. -114 6 31.982\n";
         tipStrLon += "dms delimited:    DDDdMMmSS.SSSs,  e.g. -114d6m31.982s\n";
         tipStrLon += "symbol delimited: DDD°MM'SS.SSS\", e.g. -114°6'31.982\"\n";
         tipStrLon += "decimal degrees:  DDD.DDDDDD,      e.g. -114.108884\n";
         tipStrLon += "concatenated:     DDDMMSS.SSS,     e.g. -1140631.982\n";
         tipStrLon += "   Note: 2 chars for minutes and whole seconds, i.e. 06, not 6\n\n";
         tipStrLon += "Preface with 'W', 'w' or '-' for western hemisphere";
         mpLongitudeEdit->setToolTip(tipStrLon);
      }

      pGrid->addWidget(mpLatitudeEdit, 1, 0, 1, 2);
      pGrid->addWidget(mpLongitudeEdit, 1, 2, 1, 2);
      break;
   case GEOCOORD_UTM: 
      mpLatitudeEdit = new QLineEdit(this);
      if (mpLatitudeEdit != NULL)
      {
         tipStrLat += "Meters North of Zone reference point, e.g. 41000";
         mpLatitudeEdit->setToolTip(tipStrLat);
      }

      mpLongitudeEdit = new QLineEdit(this);
      if (mpLongitudeEdit != NULL)
      {
         tipStrLon += "Meters East of Zone reference point, e.g. 10500";
         mpLongitudeEdit->setToolTip(tipStrLon);
      }

      pGrid->addWidget(mpLatitudeEdit, 1, 0);
      pGrid->addWidget(mpLongitudeEdit, 1, 1);

      if (mpZoneEdit == NULL) 
      {
         mpZoneEdit = new QLineEdit(this);
         if (mpZoneEdit != NULL)
         {
            tipStrZon += "Zone designator, e.g. 17";
            mpZoneEdit->setToolTip(tipStrZon);
         }
      }
      pGrid->addWidget(mpZoneEdit, 1, 2);

      if (mpHemEdit == NULL) 
      {
         mpHemEdit = new QLineEdit(this);
         if (mpHemEdit != NULL)
         {
            tipStrHem += "Hemisphere designator, e.g. N";
            mpHemEdit->setToolTip(tipStrHem);
         }
      }
      pGrid->addWidget(mpHemEdit, 1, 3);
      break;
   default:
      mpLatitudeEdit = new QLineEdit(this);
      if (mpLatitudeEdit != NULL)
      {
         tipStrLat += "Pixel column number, e.g. 150 (first column = 1)";
         mpLatitudeEdit->setToolTip(tipStrLat);
      }

      mpLongitudeEdit = new QLineEdit(this);
      if (mpLongitudeEdit != NULL)
      {
         tipStrLon += "Pixel row number, e.g. 150 (first row = 1)";
         mpLongitudeEdit->setToolTip(tipStrLon);
      }

      pGrid->addWidget(mpLatitudeEdit, 1, 0, 1, 2);
      pGrid->addWidget(mpLongitudeEdit, 1, 2, 1, 2);
      break;
   }

   QString caption = "Coordinate Locator";
   switch(mCoordType)
   {
   case GEOCOORD_LATLON:
      pGrid->addWidget(new QLabel("Latitude:", this), 0, 0, 1, 2);
      pGrid->addWidget(new QLabel("Longitude:", this), 0, 2, 1, 2);
      break;
   case GEOCOORD_UTM:
      pGrid->addWidget(new QLabel("Northing:", this), 0, 0);
      pGrid->addWidget(new QLabel("Easting:", this), 0, 1);
      pGrid->addWidget(new QLabel("Zone:", this), 0, 2);
      pGrid->addWidget(new QLabel("Hemisphere:", this), 0, 3);
      break;
   case GEOCOORD_MGRS:
      pGrid->addWidget(new QLabel("MGRS Reference:", this), 0, 0, 1, 4);
      break;
   default:
      caption = "Point Locator";
      pGrid->addWidget(new QLabel("X Coordinate:", this), 0, 0, 1, 2);
      pGrid->addWidget(new QLabel("Y Coordinate:", this), 0, 2, 1, 2);
      break;
   }

   mpZoomBox = new QSpinBox(this);
   mpZoomBox->setToolTip("Use to change zoom factor of view");
   mpZoomBox->setMinimum(ZOOMBOX_MIN);
   mpZoomBox->setMaximum(ZOOMBOX_MAX);
   mpZoomBox->setSingleStep(1);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // OK, Cancel
   mpOK = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // create a grid layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(mpOK);
   pButtonLayout->addWidget(pCancel);

   pGrid->setRowMinimumHeight(2, 5);
   pGrid->addWidget(new QLabel("Zoom Percentage:", this), 3, 0, 1, 4);
   pGrid->addWidget(mpZoomBox, 4, 0, 1, 4, Qt::AlignLeft);
   pGrid->setRowStretch(5, 10);
   pGrid->addWidget(pLine, 6, 0, 1, 4);
   pGrid->addLayout(pButtonLayout, 7, 0, 1, 4);

   // Initialization
   setWindowTitle(caption);
   setModal(true);

   mpOK->setEnabled(false); // can't click OK until the text field changes

   // Connections
   if (mpLatitudeEdit != NULL)
   {
      connect(mpLatitudeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk()));
      connect(mpLatitudeEdit, SIGNAL(editingFinished()), this, SLOT(latModified()));
   }

   if (mpLongitudeEdit != NULL)
   {
      connect(mpLongitudeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk()));
      connect(mpLongitudeEdit, SIGNAL(editingFinished()), this, SLOT(lonModified()));
   }

   if (mpZoneEdit != NULL)
   {
     connect(mpZoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk()));
     connect(mpZoneEdit, SIGNAL(editingFinished()), this, SLOT(zoneModified()));
   }

   if (mpHemEdit != NULL)
   {
      connect(mpHemEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk()));
      connect(mpHemEdit, SIGNAL(editingFinished()), this, SLOT(hemModified()));
   }

   connect(mpOK, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void ZoomAndPanToPointDlg::setZoomPct(float pct)
{
   mpZoomBox->setValue((int)pct);
}

float ZoomAndPanToPointDlg::getZoomPct() const
{
   if (mpZoomBox != NULL)
   {
      return mpZoomBox->value();
   }

   return 0;
}

bool invalid(const QString& str)
{
   return (str.isEmpty());
}

void ZoomAndPanToPointDlg::allowOk()
{
   QString latStr, lonStr;

   if (invalid(mpLatitudeEdit->text()) || // this one will always need to be there
       (mCoordType == GEOCOORD_LATLON &&
              invalid(mpLongitudeEdit->text())) ||
       (mCoordType == GEOCOORD_UTM && // UTM and bad zone values
              (invalid(mpLongitudeEdit->text()) ||
               invalid(mpZoneEdit->text()) ||
               invalid(mpHemEdit->text()))))
      {
         mpOK->setEnabled(false); // disallow user to click OK
      }
   else mpOK->setEnabled(true); // allow user to click OK
}

void ZoomAndPanToPointDlg::zoneModified()
{
   if ((mpZoneEdit != NULL) &&!invalid(mpZoneEdit->text()))
   {
      int zone = mpZoneEdit->text().toInt();
      if (zone > 31 || zone < 1) zone = 31;
      mpZoneEdit->setText(QString::number(zone));
   }
}

void ZoomAndPanToPointDlg::hemModified()
{
   if ((mpHemEdit != NULL) &&!invalid(mpHemEdit->text()))
   {
      QString str = (mpHemEdit->text()).toUpper();
      if (! (str == QString("N") || str == QString("S")) )
         str = QString("N");
      mpHemEdit->setText(str);
   }
}

void ZoomAndPanToPointDlg::latModified()
{
   string lat(mpLatitudeEdit->text().toStdString());
   string lon = "";
   if (mCoordType != GEOCOORD_MGRS && mpLongitudeEdit != NULL)
   {
      lon = mpLongitudeEdit->text().toStdString();
   }

   switch(mCoordType)
   {
      case GEOCOORD_LATLON:
      {
         LatLonPoint pt(lat, lon);
         mpLatitudeEdit->setText(pt.getLatitudeText().c_str());
         break;
      }

      case GEOCOORD_UTM:
      {
         QString latStr = mpLatitudeEdit->text(), lonStr = mpLongitudeEdit->text(), zoneStr = mpZoneEdit->text();

         //if (invalid(latStr) || invalid(lonStr) || invalid(zoneStr) || invalid(mpHemEdit->text()))
         //   return; // can't do anything if any are invalid

         if (lonStr[0] == 'N')
         {
            lonStr.remove( 0, 1 );
         }
         if (latStr[0] == 'E')
         {
            latStr.remove( 0, 1 );
         }

         string hem = mpHemEdit->text().toStdString();
         UtmPoint pt(lonStr.toDouble(), latStr.toDouble(), zoneStr.toInt(), hem[0]);         
         mpLatitudeEdit->setText(pt.getNorthingText().c_str());

         break;
      }

      case GEOCOORD_MGRS: 
/*
      {
         string mgrstext(mpLatitudeEdit->text().toStdString());
         MgrsPoint pt(mgrstext);
         mpLatitudeEdit->setText(QString::fromStdString(pt.getText()));
      }
*/

      default:
         break;
   }
}

void ZoomAndPanToPointDlg::lonModified()
{
  if (mCoordType == GEOCOORD_MGRS)
  {
     return; // nothing to do since MGRS is 1 coord
  }


   switch(mCoordType)
   {
      case GEOCOORD_LATLON:
      {
         string lat(mpLatitudeEdit->text().toStdString());
         string lon(mpLongitudeEdit->text().toStdString());
         LatLonPoint pt(lat, lon);
         mpLongitudeEdit->setText(pt.getLongitudeText().c_str());
         break;
      }

      case GEOCOORD_UTM: 
      {
         QString latStr = mpLatitudeEdit->text(), lonStr = mpLongitudeEdit->text(), zoneStr = mpZoneEdit->text();

         //if (invalid(latStr) || invalid(lonStr) || invalid(zoneStr) || invalid(mpHemEdit->text()))
         //   return; // can't do anything if any are invalid

         lonStr.remove( 0, 1 );
         latStr.remove( 0, 1 );
         string hem = mpHemEdit->text().toStdString();
         UtmPoint pt(lonStr.toDouble(), latStr.toDouble(), zoneStr.toInt(), hem[0]);         
         mpLongitudeEdit->setText(UtmPoint(pt).getEastingText().c_str());
         break;
      }

      default:
         break;
   }
}

LocationType ZoomAndPanToPointDlg::getCenter() const
{
   QString latStr, lonStr, zone;
   string hem;

   // by the time the user hits OK, the QString cannot be empty
   if (mpLatitudeEdit != NULL)
   {
      latStr = mpLatitudeEdit->text();
   }

   if (mpLongitudeEdit != NULL)
   {
      lonStr = mpLongitudeEdit->text();
   }

   if (mpHemEdit != NULL)
   {
      hem = mpHemEdit->text().toStdString();
   }

   if (mpZoneEdit != NULL)
   {
      zone = mpZoneEdit->text();
   }

   LatLonPoint pt;
   switch (mCoordType)
   {
      case GEOCOORD_LATLON:
         pt = LatLonPoint(latStr.toStdString(), lonStr.toStdString());
         break;

      case GEOCOORD_UTM:
      {
         lonStr.remove( 0, 1 );
         latStr.remove( 0, 1 );
         UtmPoint utp(lonStr.toDouble(), latStr.toDouble(), zone.toInt(), hem[0]);
         pt = utp.getLatLonCoordinates();
         break;
      }

      case GEOCOORD_MGRS:
      {
         string mgrs = latStr.toStdString();
         pt = MgrsPoint(mgrs).getLatLonCoordinates();
         break;
      }

      default: // pixel coordinates
         return LocationType (latStr.toInt(), lonStr.toInt());
   }

   // we'll only get here if the cube is georeferenced... so now convert the pixel
   if (mpRaster == NULL) // something bad happened
   {
      return LocationType();
   }

   return mpRaster->convertGeocoordToPixel(pt.getCoordinates());
}
