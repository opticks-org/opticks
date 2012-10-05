/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>

#include "AppVerify.h"
#include "GeoPoint.h"
#include "RasterElement.h"
#include "ZoomAndPanToPoint.h"

using namespace std;

const int ZOOMBOX_MIN = 1;
const int ZOOMBOX_MAX = 5000;

// =============================================================================
ZoomAndPanToPointDlg::ZoomAndPanToPointDlg(RasterElement* pRaster, GeocoordType coordType, QWidget* pParent) :
   QDialog(pParent),
   mpLatitudeEdit(NULL),
   mpLongitudeEdit(NULL),
   mpZoneEdit(NULL),
   mpHemEdit(NULL),
   mCoordType(coordType),
   mpRaster(pRaster)
{
   setMinimumWidth(350);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);

   QComboBox* pSelectCoordType = new QComboBox(this);

   switch(mCoordType)
   {
      case GEOCOORD_MGRS:
         pSelectCoordType->addItem("MGRS");
         break;
      case GEOCOORD_LATLON:
         pSelectCoordType->addItem("Latitude/Longitude");
         break;
      case GEOCOORD_UTM:
         pSelectCoordType->addItem("UTM");
         break;
      default:
         break;
   }
   pSelectCoordType->addItem("Pixel Coordinates");

   pGrid->addWidget(new QLabel("Pan based on:", this), 0, 0);
   pGrid->addWidget(pSelectCoordType, 1, 0, 1, 4, Qt::AlignLeft);

   QString caption = "Coordinate Locator";

   mpLatitudeLabel = new QLabel(this);
   mpLongitudeLabel = new QLabel(this);
   mpZoneLabel = new QLabel(this);
   mpHemisphereLabel = new QLabel(this);

   pGrid->addWidget(mpLatitudeLabel, 2, 0);
   pGrid->addWidget(mpLongitudeLabel, 2, 1);
   pGrid->addWidget(mpZoneLabel, 2, 2);
   pGrid->addWidget(mpHemisphereLabel, 2, 3);

   mpLatitudeEdit = new QLineEdit(this);
   mpLongitudeEdit = new QLineEdit(this);
   mpZoneEdit = new QLineEdit(this);
   mpHemEdit = new QLineEdit(this);

   pGrid->addWidget(mpLatitudeEdit, 3, 0);
   pGrid->addWidget(mpLongitudeEdit, 3, 1);
   pGrid->addWidget(mpZoneEdit, 3, 2);
   pGrid->addWidget(mpHemEdit, 3, 3);

   setWindowTitle(caption);

   if (mpLatitudeEdit != NULL)
   {
      VERIFYNR(connect(mpLatitudeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk())));
      VERIFYNR(connect(mpLatitudeEdit, SIGNAL(editingFinished()), this, SLOT(latModified())));
   }

   if (mpLongitudeEdit != NULL)
   {
      VERIFYNR(connect(mpLongitudeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk())));
      VERIFYNR(connect(mpLongitudeEdit, SIGNAL(editingFinished()), this, SLOT(lonModified())));
   }

   if (mpZoneEdit != NULL)
   {
      VERIFYNR(connect(mpZoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk())));
      VERIFYNR(connect(mpZoneEdit, SIGNAL(editingFinished()), this, SLOT(zoneModified())));
   }

   if (mpHemEdit != NULL)
   {
      VERIFYNR(connect(mpHemEdit, SIGNAL(textChanged(const QString&)), this, SLOT(allowOk())));
      VERIFYNR(connect(mpHemEdit, SIGNAL(editingFinished()), this, SLOT(hemModified())));
   }

   mpZoomBox = new QSpinBox(this);
   mpZoomBox->setToolTip("Use to change zoom factor of view");
   mpZoomBox->setMinimum(ZOOMBOX_MIN);
   mpZoomBox->setMaximum(ZOOMBOX_MAX);
   mpZoomBox->setSingleStep(1);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // create a button box
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);

   mpOK = pButtonBox->addButton(QDialogButtonBox::Ok);

   pGrid->addWidget(new QLabel("Zoom Percentage:", this), 4, 0, 1, 4);
   pGrid->addWidget(mpZoomBox, 5, 0, 1, 4, Qt::AlignLeft);
   pGrid->setRowStretch(6, 10);
   pGrid->addWidget(pLine, 7, 0, 1, 4);
   pGrid->addWidget(pButtonBox, 8, 0, 1, 4);

   // Initialization
   setModal(true);

   mpOK->setEnabled(false); // can't click OK until the text field changes

   coordTypeChanged(pSelectCoordType->currentText());

   // Connections
   VERIFYNR(connect(pSelectCoordType, SIGNAL(activated(const QString&)), this, SLOT(coordTypeChanged(const QString&))));
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

ZoomAndPanToPointDlg::~ZoomAndPanToPointDlg()
{}

void ZoomAndPanToPointDlg::setZoomPct(float pct)
{
   mpZoomBox->setValue(static_cast<int>(pct));
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
   // mpLatitudeEdit is used for all coordinate types, so it will always need to be checked.
   bool enable = !invalid(mpLatitudeEdit->text());
   switch (mCoordType)
   {
      case GEOCOORD_LATLON:   // Fall through
      default:                // Pixel coords and lat/lon use the same line edits, so lump them together.
         enable &= !invalid(mpLongitudeEdit->text());
         break;

      case GEOCOORD_UTM:
         // Check all three other QLineEdits that UTM requires (longitude, hemisphere, and zone)
         enable &= !invalid(mpLongitudeEdit->text()) && !invalid(mpZoneEdit->text()) && !invalid(mpHemEdit->text());
         break;

      case GEOCOORD_MGRS:
         // MGRS only uses mpLatitudeEdit, so break.
         break;
   }

   mpOK->setEnabled(enable);
}

void ZoomAndPanToPointDlg::zoneModified()
{
   if ((mpZoneEdit != NULL) && !invalid(mpZoneEdit->text()))
   {
      int zone = mpZoneEdit->text().toInt();
      if (zone > 31 || zone < 1)
      {
         zone = 31;
      }

      mpZoneEdit->setText(QString::number(zone));
   }
}

void ZoomAndPanToPointDlg::hemModified()
{
   if ((mpHemEdit != NULL) && !invalid(mpHemEdit->text()))
   {
      QString str = (mpHemEdit->text()).toUpper();
      if (!(str == QString("N") || str == QString("S")))
      {
         str = QString("N");
      }

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

   switch (mCoordType)
   {
      case GEOCOORD_LATLON:
      {
         LatLonPoint pt(lat, lon);
         mpLatitudeEdit->setText(pt.getLatitudeText().c_str());
         break;
      }

      case GEOCOORD_UTM:
      {
         QString latStr = mpLatitudeEdit->text();
         QString lonStr = mpLongitudeEdit->text();
         QString zoneStr = mpZoneEdit->text();

         if (lonStr[0] == 'N')
         {
            lonStr.remove( 0, 1 );
         }
         if (latStr[0] == 'E')
         {
            latStr.remove( 0, 1 );
         }

         string hem = " ";
         if (!mpHemEdit->text().isEmpty())
         {
            hem = mpHemEdit->text().toStdString();
         }
         UtmPoint pt(lonStr.toDouble(), latStr.toDouble(), zoneStr.toInt(), hem[0]);
         mpLatitudeEdit->setText(pt.getNorthingText().c_str());

         break;
      }

      case GEOCOORD_MGRS: 

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

   switch (mCoordType)
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
         QString latStr = mpLatitudeEdit->text();
         QString lonStr = mpLongitudeEdit->text();
         QString zoneStr = mpZoneEdit->text();

         lonStr.remove(0, 1);
         latStr.remove(0, 1);
         string hem = " ";
         if (!mpHemEdit->text().isEmpty())
         {
            hem = mpHemEdit->text().toStdString();
         }
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
   QString latStr;
   QString lonStr;
   QString zone;
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
         lonStr.remove(0, 1);
         latStr.remove(0, 1);
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
         return LocationType(latStr.toInt(), lonStr.toInt());
   }

   // we'll only get here if the cube is georeferenced... so now convert the pixel
   if (mpRaster == NULL) // something bad happened
   {
      return LocationType();
   }

   return mpRaster->convertGeocoordToPixel(pt.getCoordinates());
}

void ZoomAndPanToPointDlg::coordTypeChanged(const QString& newCoordSelection)
{
   if (newCoordSelection == "MGRS")
   {
      mCoordType = GEOCOORD_MGRS;
   }
   else if (newCoordSelection == "Latitude/Longitude")
   {
      mCoordType = GEOCOORD_LATLON;
   }
   else if (newCoordSelection == "UTM")
   {
      mCoordType = GEOCOORD_UTM;
   }
   else
   {
      mCoordType = GeocoordType();
   }

   mpLatitudeEdit->clear();
   mpLongitudeEdit->clear();
   mpZoneEdit->clear();
   mpHemEdit->clear();

   QString tipStr = "Accepts the following text format(s):\n";
   QString tipLat(tipStr);
   QString tipLon(tipStr);
   QString tipZone(tipStr);
   QString tipHem(tipStr);

   mpLongitudeLabel->setVisible(mCoordType != GEOCOORD_MGRS);
   mpLongitudeEdit->setVisible(mCoordType != GEOCOORD_MGRS);
   mpZoneLabel->setVisible(mCoordType == GEOCOORD_UTM);
   mpZoneEdit->setVisible(mCoordType == GEOCOORD_UTM);
   mpHemisphereLabel->setVisible(mCoordType == GEOCOORD_UTM);
   mpHemEdit->setVisible(mCoordType == GEOCOORD_UTM);

   switch (mCoordType)
   {
      case GEOCOORD_MGRS:
         mpLatitudeLabel->setText("MGRS Reference:");
         tipLat += "   GGGSSE..EN..N\n";
         tipLat += "   G = Grid zone designation\n";
         tipLat += "   S = 100,000 meter square ID\n";
         tipLat += "   E|N = Digits designating meters easting and northing\n";
         tipLat += "   (e.g. 18TUU8401 - zone 18T, square UU, E84000, N1000)\n";
         tipLat += "   (e.g. 18TUU845016 - zone 18T, square UU, E84500, N1600)";
         break;

      case GEOCOORD_LATLON:
         mpLatitudeLabel->setText("Latitude:");
         mpLongitudeLabel->setText("Longitude:");

         tipLat += "   Space delimited: DD MM SS.SSS (e.g. 40 06 31.982)\n";
         tipLat += "   DMS delimited: DDdMMmSS.SSSs (e.g. 40d06m31.982s)\n";
         tipLat += "   Symbol delimited: DD°MM'SS.SSS\" (e.g. 40°06'31.982\")\n";
         tipLat += "   Decimal degrees: DD.DDDDDD (e.g. 40.108884)\n";
         tipLat += "   Concatenated: DDMMSS.SSS (e.g. 400631.982)\n";
         tipLat += "\nNotes:\n";
         tipLat += "   For the concatenated format, two characters are required for degrees, minutes,\n";
         tipLat += "       and seconds (e.g. 06, not 6).\n";
         tipLat += "   To designate the southern hemisphere, preface the value with 'S', 's', or '-'.";

         tipLon += "   Space delimited: DDD MM SS.SSS (e.g. -114 06 31.982)\n";
         tipLon += "   DMS delimited: DDDdMMmSS.SSSs (e.g. -114d06m31.982s)\n";
         tipLon += "   Symbol delimited: DDD°MM'SS.SSS\" (e.g. -114°06'31.982\")\n";
         tipLon += "   Decimal degrees: DDD.DDDDDD (e.g. -114.108884)\n";
         tipLon += "   Concatenated: DDDMMSS.SSS (e.g. -1140631.982)\n";
         tipLon += "\nNotes:\n";
         tipLon += "   For the concatenated format, a minimum of two characters are required for degrees,\n";
         tipLon += "       and two characters are required for minutes and seconds (e.g. 06, not 6).\n";
         tipLon += "   To designate the western hemisphere, preface the value with 'W', 'w', or '-'.";
         break;

      case GEOCOORD_UTM:
         mpLatitudeLabel->setText("Northing:");
         mpLongitudeLabel->setText("Easting:");
         mpZoneLabel->setText("Zone:");
         mpHemisphereLabel->setText("Hemisphere:");

         tipLat += "   Meters north of zone reference point (e.g. 41000)";
         tipLon += "   Meters east of zone reference point (e.g. 10500)";
         tipZone += "   Zone designator (e.g. 17)";
         tipHem += "   Hemisphere designator (e.g. N)";
         break;

      default:
         mpLatitudeLabel->setText("X Coordinate:");
         mpLongitudeLabel->setText("Y Coordinate:");

         tipLat += "   X pixel coordinate.";
         tipLon += "   Y pixel coordinate.";
         break;
   }

   mpLatitudeEdit->setToolTip(tipLat);
   mpLongitudeEdit->setToolTip(tipLon);
   mpZoneEdit->setToolTip(tipZone);
   mpHemEdit->setToolTip(tipHem);
}
