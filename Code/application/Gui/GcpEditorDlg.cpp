/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "CustomTreeWidget.h"
#include "DesktopServices.h"
#include "DmsFormatTypeComboBox.h"
#include "GcpEditorDlg.h"
#include "GcpLayer.h"
#include "GeocoordTypeComboBox.h"
#include "GeoPoint.h"
#include "GeoreferenceDescriptor.h"
#include "GcpLayerImp.h"
#include "GcpListUndo.h"
#include "LayerList.h"
#include "RasterElement.h"
#include "SpatialDataView.h"

#include <boost/any.hpp>

using namespace std;

GcpEditorDlg::GcpEditorDlg(QWidget* parent) :
   QDialog(parent),
   mpLayer(NULL),
   mbModified(false)
{
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

   mpCoordTypeLabel = new QLabel("Coordinate Type:", this);
   mpCoordTypeCombo = new GeocoordTypeComboBox(this);
   mpCoordTypeCombo->setGeocoordType(GeoreferenceDescriptor::getSettingGeocoordType());
   mpLatLonFormatLabel = new QLabel("Latitude/Longitude Format:", this);
   mpLatLonFormatCombo = new DmsFormatTypeComboBox(this);
   mpLatLonFormatCombo->setCurrentValue(GeoreferenceDescriptor::getSettingLatLonFormat());

   mpNewButton = new QPushButton("&New", pGcpGroup);
   mpDeleteButton = new QPushButton("&Delete", pGcpGroup);

   QGridLayout* pGcpLayout = new QGridLayout();
   pGcpLayout->setMargin(0);
   pGcpLayout->setSpacing(5);
   pGcpLayout->addWidget(mpCoordTypeLabel, 0, 0);
   pGcpLayout->addWidget(mpCoordTypeCombo, 0, 1);
   pGcpLayout->addWidget(mpLatLonFormatLabel, 1, 0);
   pGcpLayout->addWidget(mpLatLonFormatCombo, 1, 1);
   pGcpLayout->addWidget(mpNewButton, 1, 3);
   pGcpLayout->addWidget(mpDeleteButton, 1, 4);
   pGcpLayout->setColumnStretch(2, 10);

   QVBoxLayout* pGcpGroupLayout = new QVBoxLayout(pGcpGroup);
   pGcpGroupLayout->setMargin(10);
   pGcpGroupLayout->setSpacing(10);
   pGcpGroupLayout->addWidget(mpGcpView, 10);
   pGcpGroupLayout->addLayout(pGcpLayout);

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
   pGrid->addLayout(pListLayout, 0, 0, 1, 5);
   pGrid->setRowMinimumHeight(1, 5);
   pGrid->addWidget(pGcpGroup, 2, 0, 1, 5);
   pGrid->addWidget(mpPropertiesButton, 5, 0);
   pGrid->addWidget(mpAutoApply, 5, 2);
   pGrid->addWidget(mpApplyButton, 5, 3);
   pGrid->addWidget(pCloseButton, 5, 4);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialize
   setWindowIcon(QIcon(":/icons/GcpMarker"));

   setModal(false);
   setWindowTitle("GCP Editor");
   resize(650, 450);
   setCoordinateFormat(mpCoordTypeCombo->getGeocoordType());
   updateLayers();
   enableGcp();

   // Connections
   VERIFYNR(connect(mpListCombo, SIGNAL(activated(int)), this, SLOT(setActiveLayer(int))));
   VERIFYNR(connect(mpGcpView, SIGNAL(cellTextChanged(QTreeWidgetItem*, int)), this,
      SLOT(updateGcp(QTreeWidgetItem*, int))));
   VERIFYNR(connect(mpNewButton, SIGNAL(clicked()), this, SLOT(newGcp())));
   VERIFYNR(connect(mpDeleteButton, SIGNAL(clicked()), this, SLOT(deleteGcp())));
   VERIFYNR(connect(mpCoordTypeCombo, SIGNAL(geocoordTypeChanged(GeocoordType)), this,
      SLOT(setCoordinateFormat(GeocoordType))));
   VERIFYNR(connect(mpLatLonFormatCombo, SIGNAL(valueChanged(DmsFormatType)), this,
      SLOT(setLatLonFormat(DmsFormatType))));
   VERIFYNR(connect(mpPropertiesButton, SIGNAL(clicked()), this, SLOT(setGcpProperties())));
   VERIFYNR(connect(mpApplyButton, SIGNAL(clicked()), this, SLOT(apply())));
   VERIFYNR(connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close())));

   Service<ConfigurationSettings> pSettings;
   VERIFYNR(pSettings->attach(SIGNAL_NAME(ConfigurationSettings, SettingModified),
      Slot(this, &GcpEditorDlg::optionsModified)));
}

GcpEditorDlg::~GcpEditorDlg()
{
   Service<ConfigurationSettings> pSettings;
   pSettings->detach(SIGNAL_NAME(ConfigurationSettings, SettingModified), Slot(this, &GcpEditorDlg::optionsModified));
}

void GcpEditorDlg::attached(Subject& subject, const string& signal, const Slot& slot)
{
   elementModified(subject, signal, boost::any());
}

void GcpEditorDlg::elementModified(Subject& subject, const string& signal, const boost::any& value)
{
   if (mpLayer == NULL)
   {
      return;
   }

   GcpList* pGcpList = dynamic_cast<GcpList*>(mpLayer->getDataElement());
   if ((pGcpList != NULL) && (dynamic_cast<GcpList*>(&subject) == pGcpList))
   {
      const list<GcpPoint>& points = pGcpList->getSelectedPoints();
      updateGcpView(points);
      mbModified = false;
   }
}

void GcpEditorDlg::optionsModified(Subject &subject, const string &signal, const boost::any &value)
{
   string key = boost::any_cast<string>(value);
   if (key == GeoreferenceDescriptor::getSettingGeocoordTypeKey())
   {
      setCoordinateFormat(GeoreferenceDescriptor::getSettingGeocoordType());
   }
   else if (key == GeoreferenceDescriptor::getSettingLatLonFormatKey())
   {
      setLatLonFormat(GeoreferenceDescriptor::getSettingLatLonFormat());
   }
}

bool GcpEditorDlg::setGcpLayer(Layer* pLayer)
{
   mpGcpView->closeActiveCellWidget(true);

   GcpLayer* pGcpLayer = NULL;
   int layerIndex = -1;

   if (pLayer != NULL)
   {
      // Check if the given layer is not a GCP layer
      pGcpLayer = dynamic_cast<GcpLayer*>(pLayer);
      if (pGcpLayer == NULL)
      {
         return false;
      }

      // Do not set the layer if it is not one of the available GCP layers for the active view
      layerIndex = mGcpLayers.indexOf(pGcpLayer);
      if (layerIndex == -1)
      {
         return false;
      }
   }

   // Check if the given GCP layer is already the active layer
   if (pGcpLayer == mpLayer)
   {
      return false;
   }

   // Detach from the previously active GCP list
   if (mpLayer != NULL)
   {
      GcpList* pGcpList = dynamic_cast<GcpList*>(mpLayer->getDataElement());
      VERIFY(pGcpList != NULL);

      pGcpList->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &GcpEditorDlg::elementModified));
   }

   mpLayer = pGcpLayer;

   // Attach to the newly active GCP list
   if (mpLayer != NULL)
   {
      GcpList* pGcpList = dynamic_cast<GcpList*>(mpLayer->getDataElement());
      VERIFY(pGcpList != NULL);

      pGcpList->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &GcpEditorDlg::elementModified));
   }
   else
   {
      mpGcpView->clear();
      mEditGcps.clear();
   }

   // Set the current combo box item
   if (layerIndex < mpListCombo->count())
   {
      mpListCombo->setCurrentIndex(layerIndex);    // layerIndex can be -1 if setting NULL as the active GCP layer
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
               if (mpLayer != NULL)
               {
                  GcpList* pGcpList = dynamic_cast<GcpList*>(mpLayer->getDataElement());
                  if (pGcpList != NULL)
                  {
                     points = pGcpList->getSelectedPoints();
                  }
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
   switch (mpCoordTypeCombo->getGeocoordType())
   {
      case GEOCOORD_LATLON:
      {
         QString strLatitude;
         QString strLongitude;

         string latText = latLonPoint.getLatitudeText(mpLatLonFormatCombo->getCurrentValue());
         if (latText.empty() == false)
         {
            strLatitude = QString::fromStdString(latText);
         }

         string longText = latLonPoint.getLongitudeText(mpLatLonFormatCombo->getCurrentValue());
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

   if (pItem != NULL)
   {
      mEditGcps.insert(pItem, point);
   }

   return pItem;
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
      VERIFYNR(connect(pGcpLayerImp, SIGNAL(nameChanged(const QString&)), this, SLOT(updateLayerNames())));
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
         setGcpLayer(NULL);
      }

      mGcpLayers.removeAt(iIndex);
      mpListCombo->removeItem(iIndex);

      GcpLayerImp* pGcpLayerImp = dynamic_cast<GcpLayerImp*>(pGcpLayer);
      if (pGcpLayerImp != NULL)
      {
         VERIFYNR(disconnect(pGcpLayerImp, SIGNAL(nameChanged(const QString&)), this, SLOT(updateLayerNames())));
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
   mEditGcps.clear();
}

void GcpEditorDlg::enableGcp()
{
   bool bEnable = false;
   if (mGcpLayers.count() > 0)
   {
      bEnable = true;
   }

   GeocoordType geocoordType = mpCoordTypeCombo->getGeocoordType();

   mpGcpView->setEnabled(bEnable);
   mpNewButton->setEnabled(bEnable);
   mpDeleteButton->setEnabled(bEnable);
   mpCoordTypeLabel->setEnabled(bEnable);
   mpCoordTypeCombo->setEnabled(bEnable);
   mpLatLonFormatLabel->setEnabled(bEnable && geocoordType == GEOCOORD_LATLON);
   mpLatLonFormatCombo->setEnabled(bEnable && geocoordType == GEOCOORD_LATLON);
   mpAutoApply->setEnabled(bEnable);
   mpPropertiesButton->setEnabled(bEnable);
   mpApplyButton->setEnabled(bEnable);
}

void GcpEditorDlg::updateGcpView(const list<GcpPoint>& gcps)
{
   mpGcpView->clear();
   mEditGcps.clear();

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
   if ((pItem == NULL) || (iColumn == 0))
   {
      return;
   }

   QString strText = pItem->text(iColumn);
   if (strText.isEmpty() == true)
   {
      QString gcpName = pItem->text(0);

      list<GcpPoint> points;
      for (int i = 0; i < mpGcpView->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* pCurrentItem = mpGcpView->topLevelItem(i);
         if (pCurrentItem != NULL)
         {
            GcpPoint point = mEditGcps.value(pCurrentItem);
            points.push_back(point);
         }
      }

      updateGcpView(points);

      for (int i = 0; i < mpGcpView->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* pCurrentItem = mpGcpView->topLevelItem(i);
         if (pCurrentItem != NULL)
         {
            if (pCurrentItem->text(0) == gcpName)
            {
               pCurrentItem->setSelected(true);
               break;
            }
         }
      }

      return;
   }

   GcpPoint point = mEditGcps.value(pItem);
   if (iColumn == 1)
   {
      point.mPixel.mX = strText.toDouble() - 1.0;
   }
   else if (iColumn == 2)
   {
      point.mPixel.mY = strText.toDouble() - 1.0;
   }
   else if (iColumn == 7)
   {
      point.mRmsError.mX = strText.toDouble();
   }
   else if (iColumn == 8)
   {
      point.mRmsError.mY = strText.toDouble();
   }
   else
   {
      switch (mpCoordTypeCombo->getGeocoordType())
      {
         case GEOCOORD_LATLON:
         {
            VERIFYNRV((iColumn == 3) || (iColumn == 4));

            // Convert the text string
            QString strCoordinate;

            DmsPoint::DmsType eDmsType = DmsPoint::DMS_LATITUDE;
            if (iColumn == 4)
            {
               eDmsType = DmsPoint::DMS_LONGITUDE;
            }

            DmsPoint dmsPoint(eDmsType, strText.toStdString());

            string dmsText = dmsPoint.getValueText(mpLatLonFormatCombo->getCurrentValue());
            if (dmsText.empty() == false)
            {
               strCoordinate = QString::fromStdString(dmsText);
            }

            if (strCoordinate.isEmpty() == false)
            {
               pItem->setText(iColumn, strCoordinate);
            }

            // Update the GCP coordinate
            if (iColumn == 3)
            {
               point.mCoordinate.mX = dmsPoint.getValue();
            }
            else if (iColumn == 4)
            {
               point.mCoordinate.mY = dmsPoint.getValue();
            }

            break;
         }

         case GEOCOORD_UTM:
         {
            VERIFYNRV((iColumn == 3) || (iColumn == 4) || (iColumn == 5) || (iColumn == 6));

            // Update the GCP coordinate
            LatLonPoint latLonPoint(point.mCoordinate);
            UtmPoint utmPoint(latLonPoint);

            double northing = utmPoint.getNorthing();
            double easting = utmPoint.getEasting();
            int zone = utmPoint.getZone();
            char hemisphere = utmPoint.getHemisphere();

            if ((iColumn == 3) || (iColumn == 4))
            {
               // Convert the text string
               QString strCoordinate;

               double dUtmValue = strText.toDouble();
               strCoordinate.sprintf("%.1f", dUtmValue);

               if (strCoordinate.isEmpty() == false)
               {
                  pItem->setText(iColumn, strCoordinate);
               }

               // Update the UTM value
               if (iColumn == 3)
               {
                  easting = dUtmValue;
               }
               else if (iColumn == 4)
               {
                  northing = dUtmValue;
               }
            }
            else if (iColumn == 5)
            {
               zone = strText.toInt();
            }
            else if (iColumn == 6)
            {
               hemisphere = strText.toStdString().at(0);
            }

            utmPoint = UtmPoint(easting, northing, zone, hemisphere);
            latLonPoint = utmPoint.getLatLonCoordinates();
            point.mCoordinate = latLonPoint.getCoordinates();

            break;
         }

         case GEOCOORD_MGRS:
         {
            VERIFYNRV(iColumn == 5);

            MgrsPoint mgrsPoint(strText.toStdString());
            LatLonPoint latLonPoint = mgrsPoint.getLatLonCoordinates();
            point.mCoordinate = latLonPoint.getCoordinates();

            break;
         }

         default:
            return;
      }
   }

   mEditGcps[pItem] = point;
   mbModified = true;
}

void GcpEditorDlg::newGcp()
{
   GcpPoint point;
   insertGcp(point);
   mbModified = true;
}

void GcpEditorDlg::deleteGcp()
{
   list<GcpPoint> gcps;

   int numItems = mpGcpView->topLevelItemCount();
   for (int i = 0; i < numItems; ++i)
   {
      QTreeWidgetItem* pItem = mpGcpView->topLevelItem(i);
      if (pItem != NULL)
      {
         if (mpGcpView->isItemSelected(pItem) == false)
         {
            GcpPoint point = mEditGcps.value(pItem);
            gcps.push_back(point);
         }
      }
   }

   if (static_cast<int>(gcps.size()) == numItems)
   {
      QMessageBox::critical(this, "GCP Editor", "Please select one or more GCPs to remove!");
      return;
   }

   updateGcpView(gcps);
   mbModified = true;
}

void GcpEditorDlg::setCoordinateFormat(GeocoordType geocoordType)
{
   QHeaderView* pHeader = mpGcpView->header();
   QTreeWidgetItem* pHeaderItem = mpGcpView->headerItem();
   if ((pHeader == NULL) || (pHeaderItem == NULL))
   {
      return;
   }

   // Update the combo box value
   mpCoordTypeCombo->setGeocoordType(geocoordType);

   // Get the list view GCPs
   list<GcpPoint> points;
   for (int i = 0; i < mpGcpView->topLevelItemCount(); ++i)
   {
      QTreeWidgetItem* pItem = mpGcpView->topLevelItem(i);
      if (pItem != NULL)
      {
         GcpPoint point = mEditGcps.value(pItem);
         points.push_back(point);
      }
   }

   // Update the columns
   switch (geocoordType)
   {
      case GEOCOORD_LATLON:
         pHeaderItem->setText(3, "Latitude");
         pHeaderItem->setText(4, "Longitude");
         pHeaderItem->setText(5, QString());
         pHeaderItem->setText(6, QString());
         pHeader->showSection(3);
         pHeader->showSection(4);
         pHeader->hideSection(5);
         pHeader->hideSection(6);
         break;

      case GEOCOORD_UTM:
         pHeaderItem->setText(3, "Easting");
         pHeaderItem->setText(4, "Northing");
         pHeaderItem->setText(5, "Zone");
         pHeaderItem->setText(6, "Hemisphere");
         pHeader->showSection(3);
         pHeader->showSection(4);
         pHeader->showSection(5);
         pHeader->showSection(6);
         break;

      case GEOCOORD_MGRS:
         pHeaderItem->setText(3, QString());
         pHeaderItem->setText(4, QString());
         pHeaderItem->setText(5, "MGRS string");
         pHeaderItem->setText(6, QString());
         pHeader->hideSection(3);
         pHeader->hideSection(4);
         pHeader->showSection(5);
         pHeader->hideSection(6);
         break;

      default:
         pHeader->hideSection(3);
         pHeader->hideSection(4);
         pHeader->hideSection(5);
         pHeader->hideSection(6);
         break;
   }

   // Update the GCPs for the new coordinate format
   updateGcpView(points);

   // Enable/disable the latitude/longitude format widgets
   mpLatLonFormatLabel->setEnabled(geocoordType == GEOCOORD_LATLON);
   mpLatLonFormatCombo->setEnabled(geocoordType == GEOCOORD_LATLON);
}

void GcpEditorDlg::setLatLonFormat(DmsFormatType latLonFormat)
{
   if (latLonFormat.isValid() == false)
   {
      return;
   }

   // Update the combo box value
   mpLatLonFormatCombo->setCurrentValue(latLonFormat);

   // Update the GCPs for the new latitude/longitude format
   if (mpCoordTypeCombo->getGeocoordType() == GEOCOORD_LATLON)
   {
      list<GcpPoint> points;
      for (int i = 0; i < mpGcpView->topLevelItemCount(); ++i)
      {
         QTreeWidgetItem* pItem = mpGcpView->topLevelItem(i);
         if (pItem != NULL)
         {
            GcpPoint point = mEditGcps.value(pItem);
            points.push_back(point);
         }
      }

      updateGcpView(points);
   }
}

void GcpEditorDlg::setGcpProperties()
{
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
   if ((mbModified == false) || (mpLayer == NULL))
   {
      return;
   }

   GcpList* pGcpList = dynamic_cast<GcpList*>(mpLayer->getDataElement());
   if (pGcpList == NULL)
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
         GcpPoint point = mEditGcps.value(pItem);
         points.push_back(point);
      }
   }

   View* pView = mpLayer->getView();
   if (pView != NULL)
   {
      const list<GcpPoint>& oldPoints = pGcpList->getSelectedPoints();
      pView->addUndoAction(new SetGcpPoints(pGcpList, oldPoints, points));
   }

   pGcpList->clearPoints();
   pGcpList->addPoints(points);

   mbModified = false;
}
