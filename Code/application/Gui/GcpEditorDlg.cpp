/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QRadioButton>

#include "GcpEditorDlg.h"
#include "AppAssert.h"
#include "CustomTreeWidget.h"
#include "DesktopServices.h"
#include "GeoPoint.h"
#include "GcpLayerImp.h"
#include "GcpList.h"
#include "GcpListUndo.h"
#include "Icons.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "SpatialDataView.h"

#include <boost/any.hpp>

using namespace std;

#define LATLON_BUTTON   0
#define UTM_BUTTON      1
#define MGRS_BUTTON     2

GcpEditorDlg::GcpEditorDlg(QWidget* parent) :
   QDialog(parent)
{
   mpGcpList = NULL;
   mpLayer = NULL;
   mGeocoordType = GEOCOORD_LATLON;
   mbModified = false;

   // GCP list
   QLabel* pListLabel = new QLabel("GCP List:", this);
   mpListCombo = new QComboBox(this);
   mpListCombo->setEditable(false);

   QHBoxLayout* pListLayout = new QHBoxLayout();
   pListLayout->setMargin(0);
   pListLayout->setSpacing(5);
   pListLayout->addWidget(pListLabel);
   pListLayout->addWidget(mpListCombo, 10);

   // GCP group
   QGroupBox* pGcpGroup = new QGroupBox("Ground Control Points", this);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("X Reference");
   columnNames.append("Y Reference");
   columnNames.append(QString());
   columnNames.append(QString());
   columnNames.append(QString());
   columnNames.append(QString());
   columnNames.append("X RMS Error");
   columnNames.append("Y RMS Error");

   mpGcpView = new CustomTreeWidget(pGcpGroup);

   QHeaderView* pHeader = mpGcpView->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(false);
      pHeader->setDefaultSectionSize(75);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   mpGcpView->setColumnCount(9);
   mpGcpView->setHeaderLabels(columnNames);
   mpGcpView->setRootIsDecorated(false);
   mpGcpView->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpGcpView->setSortingEnabled(false);

   mpNewButton = new QPushButton("&New", pGcpGroup);
   mpDeleteButton = new QPushButton("&Delete", pGcpGroup);

   QHBoxLayout* pGcpButtonLayout = new QHBoxLayout();
   pGcpButtonLayout->setMargin(0);
   pGcpButtonLayout->setSpacing(5);
   pGcpButtonLayout->addStretch(10);
   pGcpButtonLayout->addWidget(mpNewButton);
   pGcpButtonLayout->addWidget(mpDeleteButton);

   QVBoxLayout* pGcpLayout = new QVBoxLayout(pGcpGroup);
   pGcpLayout->setMargin(10);
   pGcpLayout->setSpacing(10);
   pGcpLayout->addWidget(mpGcpView, 10);
   pGcpLayout->addLayout(pGcpButtonLayout);

   // Coordinate group
   mpCoordGroupBox = new QGroupBox("Coordinate Format", this);

   QRadioButton* pLatLonRadio = new QRadioButton("Latitude/Longitude", mpCoordGroupBox);
   QRadioButton* pUtmCoordinatesRadio = new QRadioButton("UTM Coordinates", mpCoordGroupBox);
   QRadioButton* pMgrsRadio = new QRadioButton("MGRS", mpCoordGroupBox);

   QButtonGroup* pCoordButtonGroup = new QButtonGroup(mpCoordGroupBox);
   pCoordButtonGroup->addButton(pLatLonRadio, 0);
   pCoordButtonGroup->addButton(pUtmCoordinatesRadio, 1);
   pCoordButtonGroup->addButton(pMgrsRadio, 2);

   QVBoxLayout* pCoordLayout = new QVBoxLayout(mpCoordGroupBox);
   pCoordLayout->setMargin(10);
   pCoordLayout->setSpacing(5);
   pCoordLayout->addWidget(pLatLonRadio);
   pCoordLayout->addWidget(pUtmCoordinatesRadio);
   pCoordLayout->addWidget(pMgrsRadio);

   // Auto apply check box
   mpAutoApply = new QCheckBox("Auto Apply", this);

   // Buttons
   mpPropertiesButton = new QPushButton("&Properties...", this);
   mpPropertiesButton->setFixedWidth(100);

   mpApplyButton = new QPushButton("&Apply", this);
   QPushButton* pCloseButton = new QPushButton("&Close", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addLayout(pListLayout, 0, 0, 1, 4);
   pGrid->setRowMinimumHeight(1, 5);
   pGrid->addWidget(pGcpGroup, 2, 0, 1, 4);
   pGrid->addWidget(mpCoordGroupBox, 3, 0, 2, 1);
   pGrid->addWidget(mpAutoApply, 3, 1, 1, 3, Qt::AlignTop | Qt::AlignRight);
   pGrid->addWidget(mpPropertiesButton, 4, 1, Qt::AlignBottom | Qt::AlignLeft);
   pGrid->addWidget(mpApplyButton, 4, 2, Qt::AlignBottom);
   pGrid->addWidget(pCloseButton, 4, 3, Qt::AlignBottom);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialize
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setWindowIcon(pIcons->mGCPMarker);

   setModal(false);
   setWindowTitle("GCP Editor");
   resize(650, 450);
   pLatLonRadio->setChecked(true);
   setCoordinateFormat(0);
   updateLayers();
   enableGcp();

   // Connections
   connect(mpListCombo, SIGNAL(activated(int)), this, SLOT(setActiveLayer(int)));
   connect(mpGcpView, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this, SLOT(updateGcp(QTreeWidgetItem*, int)));
   connect(mpNewButton, SIGNAL(clicked()), this, SLOT(newGcp()));
   connect(mpDeleteButton, SIGNAL(clicked()), this, SLOT(deleteGcp()));
   connect(pCoordButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setCoordinateFormat(int)));
   connect(mpPropertiesButton, SIGNAL(clicked()), this, SLOT(setGcpProperties()));
   connect(mpApplyButton, SIGNAL(clicked()), this, SLOT(apply()));
   connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));
}

GcpEditorDlg::~GcpEditorDlg()
{
}

void GcpEditorDlg::attached(Subject &subject, const string &signal, const Slot &slot)
{
   elementModified(subject, signal, boost::any());
}

void GcpEditorDlg::elementModified(Subject &subject, const string &signal, const boost::any &data)
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(&subject);
   if (pGcpList == mpGcpList && pGcpList != NULL)
   {
      const list<GcpPoint>& points = mpGcpList->getSelectedPoints();
      updateGcpView(points);
      mbModified = false;
   }
}

bool GcpEditorDlg::setGcpLayer(Layer* pLayer)
{
   mpGcpView->closeActiveCellWidget(true);

   if (pLayer == NULL)
   {
      return false;
   }

   if (pLayer->getLayerType() != GCP_LAYER)
   {
      return false;
   }

   GcpLayer* pGcpLayer = (GcpLayer*) pLayer;

   int iIndex = mGcpLayers.indexOf(pGcpLayer);
   if (iIndex == -1)
   {
      return false;
   }

   // Get the new GCP list element from the layer
   GcpList* pGcpList = static_cast<GcpList*>(pLayer->getDataElement());

   if (mpGcpList != NULL)
   {
      mpGcpList->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &GcpEditorDlg::elementModified));
   }

   mpLayer = pGcpLayer;
   mpGcpList = pGcpList;

   if (mpGcpList != NULL)
   {
      mpGcpList->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &GcpEditorDlg::elementModified));
   }
   else
   {
      mpGcpView->clear();
   }

   // Set the current combo box item
   if (iIndex < mpListCombo->count())
   {
      mpListCombo->setCurrentIndex(iIndex);
   }

   emit layerActivated(pLayer);
   return true;
}

void GcpEditorDlg::updateLayers()
{
   clearLayers();

   Service<DesktopServices> pDesktop;

   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      vector<Layer*> gcpLayers;

      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL)
      {
         pLayerList->getLayers(GCP_LAYER, gcpLayers);
      }

      for (vector<Layer*>::size_type i = 0; i < gcpLayers.size(); i++)
      {
         Layer* pLayer = gcpLayers[i];
         if (pLayer != NULL)
         {
            addLayer(pLayer);
         }
      }
   }
}

bool GcpEditorDlg::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      QEvent::Type eventType = pEvent->type();
      if (eventType == QEvent::WindowDeactivate)
      {
         mpGcpView->closeActiveCellWidget(true);

         if (mbModified == true)
         {
            int iReturn = QMessageBox::Yes;
            if (mpAutoApply->isChecked() == false)
            {
               iReturn = QMessageBox::question(this, "GCP Editor", "The current GCP list has been modified.  "
                  "Do you want to apply the changes?", QMessageBox::Yes, QMessageBox::No);
            }

            if (iReturn == QMessageBox::Yes)
            {
               apply();
            }
            else if (iReturn == QMessageBox::No)
            {
               list<GcpPoint> points;
               if (mpGcpList != NULL)
               {
                  points = mpGcpList->getSelectedPoints();
               }

               updateGcpView(points);
               mbModified = false;
            }
         }
      }
   }

   return QDialog::event(pEvent);
}

void GcpEditorDlg::showEvent(QShowEvent* e)
{
   QDialog::showEvent(e);
   emit visibilityChanged(true);
}

void GcpEditorDlg::closeEvent(QCloseEvent* e)
{
   QDialog::closeEvent(e);
   emit visibilityChanged(false);
}

QTreeWidgetItem* GcpEditorDlg::insertGcp(const GcpPoint& point)
{
   int numItems = mpGcpView->topLevelItemCount();

   QString strName = "GCP " + QString::number(numItems + 1);
   QString strPixelX = QString::number(point.mPixel.mX + 1.0);
   QString strPixelY = QString::number(point.mPixel.mY + 1.0);
   QString strRmsX = QString::number(point.mRmsError.mX);
   QString strRmsY = QString::number(point.mRmsError.mY);

   QTreeWidgetItem* pItem = NULL;

   LatLonPoint latLonPoint(point.mCoordinate);
   switch (mGeocoordType)
   {
      case GEOCOORD_GENERAL:
         break;

      case GEOCOORD_LATLON:
      {
         QString strLatitude;
         QString strLongitude;

         string latText = latLonPoint.getLatitudeText();
         if (latText.empty() == false)
         {
            strLatitude = QString::fromStdString(latText);
         }

         string longText = latLonPoint.getLongitudeText();
         if (longText.empty() == false)
         {
            strLongitude = QString::fromStdString(longText);
         }

         pItem = new QTreeWidgetItem(mpGcpView);
         if (pItem != NULL)
         {
            pItem->setText(0, strName);
            pItem->setText(1, strPixelX);
            pItem->setText(2, strPixelY);
            pItem->setText(3, strLatitude);
            pItem->setText(4, strLongitude);
            pItem->setText(5, QString());
            pItem->setText(6, QString());
            pItem->setText(7, strRmsX);
            pItem->setText(8, strRmsY);

            mpGcpView->setCellWidgetType(pItem, 1, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 2, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 3, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 4, CustomTreeWidget::LINE_EDIT);
         }

         break;
      }

      case GEOCOORD_UTM:
      {
         UtmPoint utmPoint(latLonPoint);

         QString strEasting;
         strEasting.sprintf("%.1f", utmPoint.getEasting());

         QString strNorthing;
         strNorthing.sprintf("%.1f", utmPoint.getNorthing());

         QString strZone = QString::number(utmPoint.getZone());
         QString strHemisphere = QChar(utmPoint.getHemisphere());

         pItem = new QTreeWidgetItem(mpGcpView);
         if (pItem != NULL)
         {
            pItem->setText(0, strName);
            pItem->setText(1, strPixelX);
            pItem->setText(2, strPixelY);
            pItem->setText(3, strEasting);
            pItem->setText(4, strNorthing);
            pItem->setText(5, strZone);
            pItem->setText(6, strHemisphere);
            pItem->setText(7, strRmsX);
            pItem->setText(8, strRmsY);

            mpGcpView->setCellWidgetType(pItem, 1, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 2, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 3, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 4, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 5, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 6, CustomTreeWidget::LINE_EDIT);
         }

         break;
      }

      case GEOCOORD_MGRS:
      {
         MgrsPoint mgrsPoint(latLonPoint);

         QString strMgrs;

         string mgrsText = mgrsPoint.getText();
         if (mgrsText.empty() == false)
         {
            strMgrs = QString::fromStdString(mgrsText);
         }

         pItem = new QTreeWidgetItem(mpGcpView);
         if (pItem != NULL)
         {
            pItem->setText(0, strName);
            pItem->setText(1, strPixelX);
            pItem->setText(2, strPixelY);
            pItem->setText(3, QString());
            pItem->setText(4, QString());
            pItem->setText(5, strMgrs);
            pItem->setText(6, QString());
            pItem->setText(7, strRmsX);
            pItem->setText(8, strRmsY);

            mpGcpView->setCellWidgetType(pItem, 1, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 2, CustomTreeWidget::LINE_EDIT);
            mpGcpView->setCellWidgetType(pItem, 5, CustomTreeWidget::LINE_EDIT);
            pItem->setText(8, strRmsY);
         }

         break;
      }

      default:
         break;
   }

   return pItem;
}

GcpPoint GcpEditorDlg::getGcp(QTreeWidgetItem* pItem)
{
   GcpPoint point;

   if (pItem != NULL)
   {
      // Pixel coordinate
      point.mPixel.mX = pItem->text(1).toDouble() - 1.0;
      point.mPixel.mY = pItem->text(2).toDouble() - 1.0;

      // Geocoordinate
      switch (mGeocoordType)
      {
         case GEOCOORD_GENERAL:
            break;

         case GEOCOORD_LATLON:
         {
            LatLonPoint latLonPoint(pItem->text(3).toStdString(), pItem->text(4).toStdString());
            point.mCoordinate = latLonPoint.getCoordinates();
            break;
         }

         case GEOCOORD_UTM:
         {
            UtmPoint utmPoint(pItem->text(3).toDouble(), pItem->text(4).toDouble(),
               pItem->text(5).toInt(), pItem->text(6).toStdString().at(0));

            LatLonPoint latLonPoint = utmPoint.getLatLonCoordinates();
            point.mCoordinate = latLonPoint.getCoordinates();
            break;
         }

         case GEOCOORD_MGRS:
         {
            MgrsPoint mgrsPoint(pItem->text(5).toStdString());

            LatLonPoint latLonPoint = mgrsPoint.getLatLonCoordinates();
            point.mCoordinate = latLonPoint.getCoordinates();
            break;
         }

         default:
            break;
      }

      // RMS error
      point.mRmsError.mX = pItem->text(7).toDouble();
      point.mRmsError.mY = pItem->text(8).toDouble();
   }

   return point;
}

bool GcpEditorDlg::addLayer(Layer* pLayer)
{
   // Check for an invalid layer type
   GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(pLayer);
   if (pGcpLayer == NULL)
   {
      return false;
   }

   // Check if the layer already exists in the list
   if (mGcpLayers.contains(pGcpLayer) == true)
   {
      return false;
   }

   // Add the layer to the member list and combo box
   const string& layerName = pGcpLayer->getName();
   if (layerName.empty() == true)
   {
      return false;
   }

   mpGcpView->closeActiveCellWidget(true);

   mpListCombo->addItem(QString::fromStdString(layerName));
   mGcpLayers.append(pGcpLayer);

   GcpLayerImp* pGcpLayerImp = dynamic_cast<GcpLayerImp*>(pGcpLayer);
   if (pGcpLayerImp != NULL)
   {
      connect(pGcpLayerImp, SIGNAL(nameChanged(const QString&)), this, SLOT(updateLayerNames()));
   }

   enableGcp();
   setGcpLayer(pLayer);
   return true;
}

bool GcpEditorDlg::removeLayer(Layer* pLayer)
{
   GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(pLayer);
   if (pGcpLayer == NULL)
   {
      return false;
   }

   int iIndex = mGcpLayers.indexOf(pGcpLayer);
   if (iIndex != -1)
   {
      mpGcpView->closeActiveCellWidget(true);

      if (mpListCombo->currentIndex() == iIndex)
      {
         mpLayer = NULL;
         mpGcpList = NULL;
      }

      mGcpLayers.removeAt(iIndex);
      mpListCombo->removeItem(iIndex);

      GcpLayerImp* pGcpLayerImp = dynamic_cast<GcpLayerImp*>(pGcpLayer);
      if (pGcpLayerImp != NULL)
      {
         disconnect(pGcpLayerImp, SIGNAL(nameChanged(const QString&)), this, SLOT(updateLayerNames()));
      }

      enableGcp();
      return true;
   }

   return false;
}

void GcpEditorDlg::setActiveLayer(int iIndex)
{
   GcpLayer* pGcpLayer = mGcpLayers.at(iIndex);
   if (pGcpLayer != NULL)
   {
      Service<DesktopServices> pDesktop;

      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
      if (pView != NULL)
      {
         pView->setActiveLayer(pGcpLayer);
      }
   }
}

void GcpEditorDlg::updateLayerNames()
{
   for (int i = 0; i < mGcpLayers.count(); i++)
   {
      GcpLayerImp* pLayer = dynamic_cast<GcpLayerImp*>(mGcpLayers[i]);
      if (pLayer != NULL)
      {
         QString strName = QString::fromStdString(pLayer->getName());
         if (strName.isEmpty() == false)
         {
            mpListCombo->setItemText(i, strName);
         }
      }
   }
}

void GcpEditorDlg::clearLayers()
{
   while (mGcpLayers.empty() == false)
   {
      GcpLayer* pLayer = mGcpLayers.front();
      if (pLayer != NULL)
      {
         removeLayer(pLayer);
      }
      else
      {
         break;
      }
   }

   mpGcpView->clear();
}

void GcpEditorDlg::enableGcp()
{
   bool bEnable = false;
   if (mGcpLayers.count() > 0)
   {
      bEnable = true;
   }

   mpGcpView->setEnabled(bEnable);
   mpNewButton->setEnabled(bEnable);
   mpDeleteButton->setEnabled(bEnable);
   mpCoordGroupBox->setEnabled(bEnable);
   mpAutoApply->setEnabled(bEnable);
   mpPropertiesButton->setEnabled(bEnable);
   mpApplyButton->setEnabled(bEnable);
}

void GcpEditorDlg::updateGcpView(const list<GcpPoint>& gcps)
{
   mpGcpView->clear();

   list<GcpPoint>::const_iterator iter = gcps.begin();
   while (iter != gcps.end())
   {
      GcpPoint gcp = *iter;
      insertGcp(gcp);

      ++iter;
   }

   // Adjust the column width
   if (gcps.size() > 0)
   {
      for (int i = 0; i < mpGcpView->columnCount(); ++i)
      {
         mpGcpView->resizeColumnToContents(i);
      }
   }
}

void GcpEditorDlg::updateGcp(QTreeWidgetItem* pItem, int iColumn)
{
   if (pItem == NULL)
   {
      return;
   }

   mbModified = true;

   QString strText = pItem->text(iColumn);
   if (strText.isEmpty() == true)
   {
      return;
   }

   // Only convert the text on the geocoord cells
   if ((iColumn != 3) && (iColumn != 4))
   {
      return;
   }

   switch (mGeocoordType)
   {
      case GEOCOORD_GENERAL:
         break;

      case GEOCOORD_LATLON:
      {
         QString strCoordinate;

         DmsPoint::DmsType eDmsType = DmsPoint::DMS_LATITUDE;
         if (iColumn == 4)
         {
            eDmsType = DmsPoint::DMS_LONGITUDE;
         }

         DmsPoint dmsPoint(eDmsType, strText.toStdString());

         string dmsText = dmsPoint.getValueText();
         if (dmsText.empty() == false)
         {
            strCoordinate = QString::fromStdString(dmsText);
         }

         if (strCoordinate.isEmpty() == false)
         {
            pItem->setText(iColumn, strCoordinate);
         }

         break;
      }

      case GEOCOORD_UTM:
      {
         QString strCoordinate;

         double dUtmValue = strText.toDouble();
         strCoordinate.sprintf("%.1f", dUtmValue);

         if (strCoordinate.isEmpty() == false)
         {
            pItem->setText(iColumn, strCoordinate);
         }

         break;
      }

      case GEOCOORD_MGRS:
         break;

      default:
         break;
   }
}

void GcpEditorDlg::newGcp()
{
   mpGcpView->closeActiveCellWidget(true);

   GcpPoint point;
   insertGcp(point);
   mbModified = true;
}

void GcpEditorDlg::deleteGcp()
{
   mpGcpView->closeActiveCellWidget(true);

   list<GcpPoint> gcps;

   int numItems = mpGcpView->topLevelItemCount();
   for (int i = 0; i < numItems; ++i)
   {
      QTreeWidgetItem* pItem = mpGcpView->topLevelItem(i);
      if (pItem != NULL)
      {
         if (mpGcpView->isItemSelected(pItem) == false)
         {
            GcpPoint point = getGcp(pItem);
            gcps.push_back(point);
         }
      }
   }

   if (gcps.size() == numItems)
   {
      QMessageBox::critical(this, "GCP Editor", "Please select one or more GCPs to remove!");
      return;
   }

   updateGcpView(gcps);
   mbModified = true;
}

void GcpEditorDlg::setCoordinateFormat(int iIndex)
{
   mpGcpView->closeActiveCellWidget(true);

   QHeaderView* pHeader = mpGcpView->header();
   QTreeWidgetItem* pHeaderItem = mpGcpView->headerItem();
   if ((pHeader == NULL) || (pHeaderItem == NULL))
   {
      return;
   }

   // Get the list view GCPs
   list<GcpPoint> points;
   for (int i = 0; i < mpGcpView->topLevelItemCount(); ++i)
   {
      QTreeWidgetItem* pItem = mpGcpView->topLevelItem(i);
      if (pItem != NULL)
      {
         GcpPoint point = getGcp(pItem);
         points.push_back(point);
      }
   }

   // Update the columns
   switch (iIndex)
   {
      case LATLON_BUTTON:
         mGeocoordType = GEOCOORD_LATLON;
         pHeaderItem->setText(3, "Latitude");
         pHeaderItem->setText(4, "Longitude");
         pHeaderItem->setText(5, QString());
         pHeaderItem->setText(6, QString());
         pHeader->showSection(3);
         pHeader->showSection(4);
         pHeader->hideSection(5);
         pHeader->hideSection(6);
         break;

      case UTM_BUTTON:
         mGeocoordType = GEOCOORD_UTM;
         pHeaderItem->setText(3, "Easting");
         pHeaderItem->setText(4, "Northing");
         pHeaderItem->setText(5, "Zone");
         pHeaderItem->setText(6, "Hemisphere");
         pHeader->showSection(3);
         pHeader->showSection(4);
         pHeader->showSection(5);
         pHeader->showSection(6);
         break;

      case MGRS_BUTTON:
         mGeocoordType = GEOCOORD_MGRS;
         pHeaderItem->setText(3, QString());
         pHeaderItem->setText(4, QString());
         pHeaderItem->setText(5, "MGRS string");
         pHeaderItem->setText(6, QString());
         pHeader->hideSection(3);
         pHeader->hideSection(4);
         pHeader->showSection(5);
         pHeader->hideSection(6);
         break;
   }

   // Update the GCPs for the new coordinate format
   updateGcpView(points);
}

void GcpEditorDlg::setGcpProperties()
{
   mpGcpView->closeActiveCellWidget(true);

   if (mpLayer == NULL)
   {
      QMessageBox::critical(this, "GCP Editor", "The GCP list layer could not be found!");
      return;
   }

   Service<DesktopServices> pDesktop;
   pDesktop->displayProperties(mpLayer);
}

void GcpEditorDlg::apply()
{
   mpGcpView->closeActiveCellWidget(true);

   if ((mbModified == false) || (mpGcpList == NULL))
   {
      return;
   }

   if (mpApplyButton != NULL)
   {
      mpApplyButton->setFocus();
   }

   list<GcpPoint> points;
   for (int i = 0; i < mpGcpView->topLevelItemCount(); ++i)
   {
      QTreeWidgetItem* pItem = mpGcpView->topLevelItem(i);
      if (pItem != NULL)
      {
         GcpPoint point = getGcp(pItem);
         points.push_back(point);
      }
   }

   if (mpLayer != NULL)
   {
      View* pView = mpLayer->getView();
      if (pView != NULL)
      {
         const list<GcpPoint>& oldPoints = mpGcpList->getSelectedPoints();
         pView->addUndoAction(new SetGcpPoints(mpGcpList, oldPoints, points));
      }
   }

   mpGcpList->clearPoints();
   mpGcpList->addPoints(points);

   mbModified = false;
}
