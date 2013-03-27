/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DmsFormatTypeComboBox.h"
#include "DynamicObject.h"
#include "GeocoordTypeComboBox.h"
#include "Georeference.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceWidget.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "Slot.h"

#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QStyleOptionButton>

#include <boost/any.hpp>
#include <vector>

#define NO_PLUGIN_LABEL "No georeference plug-ins support the current data set"
#define NO_OPTIONS_LABEL "No options are available for the selected plug-in"
#define NO_SELECTED_PLUGIN_LABEL "No plug-in selected"

GeoreferenceWidget::GeoreferenceWidget(QWidget* pParent) :
   QWidget(pParent),
   mpDescriptor(NULL)
{
   // Georeference plug-ins
   QLabel* pPlugInLabel = new QLabel("Georeference Plug-Ins:", this);
   mpPlugInList = new QListWidget(this);
   mpPlugInList->setSelectionMode(QAbstractItemView::SingleSelection);
   mpPlugInList->setFixedWidth(125);

   // Plug-in widget stack
   mpPlugInStack = new QStackedWidget(this);

   mpNoWidgetLabel = new QLabel(NO_PLUGIN_LABEL, this);
   mpNoWidgetLabel->setAlignment(Qt::AlignCenter);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Georeference parameters
   mpCreateLayerCheck = new QCheckBox("Create layer for the georeference results", this);
   mpCreateLayerCheck->setToolTip("Toggles whether or not a layer is created for the georeference results.\n"
      "If a layer already exists, it will be updated for the new results regardless of this setting.");

   mpLayerNameLabel = new QLabel("Layer Name:", this);
   mpLayerNameEdit = new QLineEdit(this);

   mpCoordTypeLabel = new QLabel("Coordinate Type:", this);
   mpCoordTypeCombo = new GeocoordTypeComboBox(this);

   mpLatLonFormatLabel = new QLabel("Latitude/Longitude Format:", this);
   mpLatLonFormatCombo = new DmsFormatTypeComboBox(this);

   mpDisplayLayerCheck = new QCheckBox("Display the georeference results layer", this);
   mpDisplayLayerCheck->setToolTip("Toggles whether or not the created georeference results layer is displayed.\n"
      "If the layer isn't created, this setting is ignored.");

   // Layout
   QGridLayout* pParametersLayout = new QGridLayout();
   pParametersLayout->setMargin(0);
   pParametersLayout->setSpacing(5);
   pParametersLayout->addWidget(mpCreateLayerCheck, 0, 0, 1, 3);
   pParametersLayout->addWidget(mpLayerNameLabel, 1, 1);
   pParametersLayout->addWidget(mpLayerNameEdit, 1, 2);
   pParametersLayout->addWidget(mpCoordTypeLabel, 2, 1);
   pParametersLayout->addWidget(mpCoordTypeCombo, 2, 2, Qt::AlignLeft);
   pParametersLayout->addWidget(mpLatLonFormatLabel, 3, 1);
   pParametersLayout->addWidget(mpLatLonFormatCombo, 3, 2, Qt::AlignLeft);
   pParametersLayout->addWidget(mpDisplayLayerCheck, 4, 1, 1, 2);
   pParametersLayout->setColumnStretch(2, 10);

   QStyleOptionButton option;
   option.initFrom(mpCreateLayerCheck);
   int checkWidth = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option).width();
   pParametersLayout->setColumnMinimumWidth(0, checkWidth);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(10);
   pGrid->addWidget(pPlugInLabel, 0, 0);
   pGrid->addWidget(mpPlugInList, 1, 0);
   pGrid->addWidget(mpPlugInStack, 1, 1);
   pGrid->addWidget(pLine, 2, 0, 1, 2);
   pGrid->addLayout(pParametersLayout, 3, 0, 1, 2);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   mpPlugInStack->addWidget(mpNoWidgetLabel);
   mpPlugInStack->setCurrentWidget(mpNoWidgetLabel);
   mpLayerNameLabel->setEnabled(false);
   mpLayerNameEdit->setEnabled(false);
   mpCoordTypeLabel->setEnabled(false);
   mpCoordTypeCombo->setEnabled(false);
   mpLatLonFormatLabel->setEnabled(false);
   mpLatLonFormatCombo->setEnabled(false);
   mpDisplayLayerCheck->setEnabled(false);

   // Connections
   VERIFYNR(connect(mpPlugInList, SIGNAL(itemSelectionChanged()), this, SLOT(setGeoreferencePlugIn())));
   VERIFYNR(connect(mpCreateLayerCheck, SIGNAL(toggled(bool)), this, SLOT(setCreateLayer(bool))));
   VERIFYNR(connect(mpLayerNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(setLayerName(const QString&))));
   VERIFYNR(connect(mpCoordTypeCombo, SIGNAL(geocoordTypeChanged(GeocoordType)), this,
      SLOT(setGeocoordType(GeocoordType))));
   VERIFYNR(connect(mpLatLonFormatCombo, SIGNAL(valueChanged(DmsFormatType)), this,
      SLOT(setLatLonFormat(DmsFormatType))));
   VERIFYNR(connect(mpDisplayLayerCheck, SIGNAL(toggled(bool)), this, SLOT(setDisplayLayer(bool))));
}

GeoreferenceWidget::~GeoreferenceWidget()
{
   // Detach from the data descriptor and destroy the georeference plug-ins
   setDataDescriptor(NULL);
}

void GeoreferenceWidget::setDataDescriptor(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == mpDescriptor)
   {
      return;
   }

   if (mpDescriptor != NULL)
   {
      VERIFYNR(mpDescriptor->detach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &GeoreferenceWidget::descriptorDeleted)));

      DynamicObject* pMetadata = mpDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         VERIFYNR(pMetadata->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &GeoreferenceWidget::metadataModified)));
      }

      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferencePlugInNameChanged),
            Slot(this, &GeoreferenceWidget::georeferencePlugInNameModified)));
         VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, CreateLayerChanged),
            Slot(this, &GeoreferenceWidget::createLayerModified)));
         VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, LayerNameChanged),
            Slot(this, &GeoreferenceWidget::layerNameModified)));
         VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, DisplayLayerChanged),
            Slot(this, &GeoreferenceWidget::displayLayerModified)));
         VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, GeocoordTypeChanged),
            Slot(this, &GeoreferenceWidget::geocoordTypeModified)));
         VERIFYNR(pGeorefDescriptor->detach(SIGNAL_NAME(GeoreferenceDescriptor, LatLonFormatChanged),
            Slot(this, &GeoreferenceWidget::latLonFormatModified)));
      }
   }

   mpDescriptor = pDescriptor;

   // Update the list of available plug-ins
   updatePlugInItems();

   // Update the widgets based on the georeference parameters
   bool createLayer = GeoreferenceDescriptor::getSettingCreateLayer();
   std::string layerName;
   bool displayLayer = GeoreferenceDescriptor::getSettingDisplayLayer();
   GeocoordType geocoordType = GeoreferenceDescriptor::getSettingGeocoordType();
   DmsFormatType latLonFormat = GeoreferenceDescriptor::getSettingLatLonFormat();

   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         const std::string& plugInName = pGeorefDescriptor->getGeoreferencePlugInName();
         if (plugInName.empty() == false)
         {
            QList<QListWidgetItem*> items = mpPlugInList->findItems(QString::fromStdString(plugInName),
               Qt::MatchExactly);
            if (items.empty() == false)
            {
               VERIFYNR(items.size() == 1);
               mpPlugInList->setItemSelected(items.front(), true);
            }
         }

         createLayer = pGeorefDescriptor->getCreateLayer();
         layerName = pGeorefDescriptor->getLayerName();
         displayLayer = pGeorefDescriptor->getDisplayLayer();
         geocoordType = pGeorefDescriptor->getGeocoordType();
         latLonFormat = pGeorefDescriptor->getLatLonFormat();
      }
   }

   updateNoWidgetLabel();

   mpCreateLayerCheck->setChecked(createLayer);
   mpLayerNameEdit->setText(QString::fromStdString(layerName));
   mpCoordTypeCombo->setGeocoordType(geocoordType);
   mpLatLonFormatCombo->setCurrentValue(latLonFormat);
   mpDisplayLayerCheck->setChecked(displayLayer);

   // Attachments
   if (mpDescriptor != NULL)
   {
      VERIFYNR(mpDescriptor->attach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &GeoreferenceWidget::descriptorDeleted)));

      DynamicObject* pMetadata = mpDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         VERIFYNR(pMetadata->attach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &GeoreferenceWidget::metadataModified)));
      }

      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, GeoreferencePlugInNameChanged),
            Slot(this, &GeoreferenceWidget::georeferencePlugInNameModified)));
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, CreateLayerChanged),
            Slot(this, &GeoreferenceWidget::createLayerModified)));
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, LayerNameChanged),
            Slot(this, &GeoreferenceWidget::layerNameModified)));
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, DisplayLayerChanged),
            Slot(this, &GeoreferenceWidget::displayLayerModified)));
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, GeocoordTypeChanged),
            Slot(this, &GeoreferenceWidget::geocoordTypeModified)));
         VERIFYNR(pGeorefDescriptor->attach(SIGNAL_NAME(GeoreferenceDescriptor, LatLonFormatChanged),
            Slot(this, &GeoreferenceWidget::latLonFormatModified)));
      }
   }
}

RasterDataDescriptor* GeoreferenceWidget::getDataDescriptor()
{
   return mpDescriptor;
}

const RasterDataDescriptor* GeoreferenceWidget::getDataDescriptor() const
{
   return mpDescriptor;
}

Georeference* GeoreferenceWidget::getSelectedPlugIn() const
{
   QListWidgetItem* pItem = NULL;

   QList<QListWidgetItem*> items = mpPlugInList->selectedItems();
   if (items.empty() == false)
   {
      VERIFYRV(items.size() == 1, NULL);
      pItem = items.front();
   }

   if (pItem != NULL)
   {
      return dynamic_cast<Georeference*>(pItem->data(PlugInRole).value<PlugIn*>());
   }

   return NULL;
}

QListWidgetItem* GeoreferenceWidget::addPlugInItem(const std::string& plugInName)
{
   if ((plugInName.empty() == true) || (mpDescriptor == NULL))
   {
      return NULL;
   }

   PlugInResource pPlugIn(plugInName);

   Georeference* pGeoreference = dynamic_cast<Georeference*>(pPlugIn.get());
   if (pGeoreference != NULL)
   {
      unsigned char affinity = pGeoreference->getGeoreferenceAffinity(mpDescriptor);
      VERIFYRV(affinity > Georeference::CAN_NOT_GEOREFERENCE, NULL);

      QListWidgetItem* pItem = new QListWidgetItem(QString::fromStdString(plugInName), mpPlugInList);
      pItem->setData(PlugInRole, QVariant::fromValue(pPlugIn.release()));

      QWidget* pWidget = pGeoreference->getWidget(mpDescriptor);
      if (pWidget != NULL)
      {
         pItem->setData(WidgetRole, QVariant::fromValue(pWidget));
         mpPlugInStack->addWidget(pWidget);
      }
      else
      {
         pItem->setData(WidgetRole, QVariant());
      }

      return pItem;
   }

   return NULL;
}

void GeoreferenceWidget::removePlugInItem(QListWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   // Remove the plug-in widget from the widget stack and reset the parent
   // so that the georeference plug-in can successfully destroy the widget
   QWidget* pWidget = pItem->data(WidgetRole).value<QWidget*>();
   if (pWidget != NULL)
   {
      mpPlugInStack->removeWidget(pWidget);
      pWidget->setParent(NULL);
   }

   // Destroy the plug-in
   PlugIn* pPlugIn = pItem->data(PlugInRole).value<PlugIn*>();
   if (pPlugIn != NULL)
   {
      Service<PlugInManagerServices>()->destroyPlugIn(pPlugIn);
   }

   // Remove the plug-in from the list
   delete pItem;
}

void GeoreferenceWidget::updatePlugInItems()
{
   // Clear the list of available plug-ins
   mpPlugInList->blockSignals(true);   // Block signals to prevent the selected plug-in from being
                                       // reset in mpDescriptor when the selected item is cleared

   while (mpPlugInList->count() > 0)
   {
      QListWidgetItem* pItem = mpPlugInList->item(0);
      VERIFYNRV(pItem != NULL);

      removePlugInItem(pItem);
   }

   mpPlugInList->blockSignals(false);
   VERIFYNRV(mpPlugInList->count() == 0);

   if (mpDescriptor == NULL)
   {
      return;
   }

   // Update the list of available plug-ins that support the given data set
   const std::vector<std::string>& plugIns = mpDescriptor->getValidGeoreferencePlugIns();
   for (std::vector<std::string>::const_iterator iter = plugIns.begin(); iter != plugIns.end(); ++iter)
   {
      std::string plugInName = *iter;
      if (plugInName.empty() == false)
      {
         addPlugInItem(plugInName);
      }
   }
}

void GeoreferenceWidget::updateNoWidgetLabel()
{
   QListWidgetItem* pItem = NULL;

   QList<QListWidgetItem*> items = mpPlugInList->selectedItems();
   if (items.empty() == false)
   {
      VERIFYNRV(items.size() == 1);
      pItem = items.front();
   }

   if (pItem != NULL)
   {
      QWidget* pWidget = pItem->data(WidgetRole).value<QWidget*>();
      if (pWidget == NULL)
      {
         mpNoWidgetLabel->setText(NO_OPTIONS_LABEL);
      }
   }
   else if (mpPlugInList->count() > 0)
   {
      mpNoWidgetLabel->setText(NO_SELECTED_PLUGIN_LABEL);
   }
   else
   {
      mpNoWidgetLabel->setText(NO_PLUGIN_LABEL);
   }
}

void GeoreferenceWidget::metadataModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   // Get the selected plug-in
   QString selectedPlugIn;

   QList<QListWidgetItem*> selectedItems = mpPlugInList->selectedItems();
   if (selectedItems.empty() == false)
   {
      VERIFYNR(selectedItems.size() == 1);

      QListWidgetItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         selectedPlugIn = pItem->text();
      }
   }

   // Update the list of available georeference plug-ins since some plug-ins may be dependent on the metadata
   updatePlugInItems();

   // Set the previously selected plug-in
   if (mpPlugInList->count() > 0)
   {
      QListWidgetItem* pItem = mpPlugInList->item(0);
      if (selectedPlugIn.isEmpty() == false)
      {
         QList<QListWidgetItem*> items = mpPlugInList->findItems(selectedPlugIn, Qt::MatchExactly);
         if (items.empty() == false)
         {
            VERIFYNR(items.size() == 1);
            pItem = items.front();
         }
      }

      if (pItem != NULL)
      {
         mpPlugInList->setItemSelected(pItem, true);
      }
   }
}

void GeoreferenceWidget::georeferencePlugInNameModified(Subject& subject, const std::string& signal,
                                                        const boost::any& value)
{
   std::string plugInName = boost::any_cast<std::string>(value);
   if (plugInName.empty() == false)
   {
      QList<QListWidgetItem*> items = mpPlugInList->findItems(QString::fromStdString(plugInName), Qt::MatchExactly);
      if (items.empty() == false)
      {
         VERIFYNR(items.size() == 1);
         mpPlugInList->setItemSelected(items.front(), true);
      }
   }
   else
   {
      QList<QListWidgetItem*> selectedItems = mpPlugInList->selectedItems();
      if (selectedItems.empty() == false)
      {
         VERIFYNR(selectedItems.size() == 1);
         mpPlugInList->setItemSelected(selectedItems.front(), false);
      }
   }
}

void GeoreferenceWidget::createLayerModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   mpCreateLayerCheck->setChecked(boost::any_cast<bool>(value));
}

void GeoreferenceWidget::layerNameModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   mpLayerNameEdit->setText(QString::fromStdString(boost::any_cast<std::string>(value)));
}

void GeoreferenceWidget::displayLayerModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   mpDisplayLayerCheck->setChecked(boost::any_cast<bool>(value));
}

void GeoreferenceWidget::geocoordTypeModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   mpCoordTypeCombo->setGeocoordType(boost::any_cast<GeocoordType>(value));
}

void GeoreferenceWidget::latLonFormatModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   mpLatLonFormatCombo->setCurrentValue(boost::any_cast<DmsFormatType>(value));
}

void GeoreferenceWidget::descriptorDeleted(Subject& subject, const std::string& signal, const boost::any& value)
{
   // Detach from the data descriptor and destroy the georeference plug-ins
   setDataDescriptor(NULL);
}

void GeoreferenceWidget::setGeoreferencePlugIn()
{
   // Get the selected item
   QListWidgetItem* pItem = NULL;

   QList<QListWidgetItem*> items = mpPlugInList->selectedItems();
   if (items.empty() == false)
   {
      VERIFYNRV(items.size() == 1);
      pItem = items.front();
   }

   // Update the plug-in name in the georeference descriptor
   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         std::string plugInName;
         if (pItem != NULL)
         {
            plugInName = pItem->text().toStdString();
         }

         pGeorefDescriptor->setGeoreferencePlugInName(plugInName);
      }
   }

   // Activate the appropriate widget in the widget stack
   QWidget* pWidget = mpNoWidgetLabel;
   if (pItem != NULL)
   {
      QWidget* pPlugInWidget = pItem->data(WidgetRole).value<QWidget*>();
      if (pPlugInWidget != NULL)
      {
         pWidget = pPlugInWidget;
      }
   }

   mpPlugInStack->setCurrentWidget(pWidget);

   // Update the text displayed in the label widget indicating no georefeence parameters are available
   if (pWidget == mpNoWidgetLabel)
   {
      updateNoWidgetLabel();
   }
}

void GeoreferenceWidget::setCreateLayer(bool createLayer)
{
   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setCreateLayer(createLayer);
      }
   }

   mpLayerNameLabel->setEnabled(createLayer);
   mpLayerNameEdit->setEnabled(createLayer);
   mpCoordTypeLabel->setEnabled(createLayer);
   mpCoordTypeCombo->setEnabled(createLayer);
   mpDisplayLayerCheck->setEnabled(createLayer);

   GeocoordType geocoordType = mpCoordTypeCombo->getGeocoordType();
   bool enableLatLonFormat = createLayer && (geocoordType == GEOCOORD_LATLON);
   mpLatLonFormatLabel->setEnabled(enableLatLonFormat);
   mpLatLonFormatCombo->setEnabled(enableLatLonFormat);
}

void GeoreferenceWidget::setLayerName(const QString& layerName)
{
   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setLayerName(layerName.toStdString());
      }
   }
}

void GeoreferenceWidget::setGeocoordType(GeocoordType geocoordType)
{
   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setGeocoordType(geocoordType);
      }
   }

   bool enableLatLonFormat = (geocoordType == GEOCOORD_LATLON);
   mpLatLonFormatLabel->setEnabled(enableLatLonFormat);
   mpLatLonFormatCombo->setEnabled(enableLatLonFormat);
}

void GeoreferenceWidget::setLatLonFormat(DmsFormatType latLonFormat)
{
   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setLatLonFormat(latLonFormat);
      }
   }
}

void GeoreferenceWidget::setDisplayLayer(bool displayLayer)
{
   if (mpDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = mpDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setDisplayLayer(displayLayer);
      }
   }
}
