/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>

#include "AppVerify.h"
#include "Georeference.h"
#include "GeoreferenceDlg.h"

using namespace std;

GeoreferenceDlg::GeoreferenceDlg(const QString& title,
   const vector<string> &geoPluginNameList,
   const vector<QWidget*> &geoPluginWidgetList, QWidget *parent) :
   QDialog(parent), mWidgets(geoPluginWidgetList)
{
   // Georeference plug-ins
   QLabel* pGeoListLabel = new QLabel("Georeference Plug-Ins:", this);
   mpGeoList = new QListWidget(this);
   mpGeoList->setFixedWidth(125);

   // Plug-in widget stack 
   mpStack = new QStackedWidget(this);

   // Other labels
   QLabel* pNoUILabel = new QLabel("The selected plug-in has no options");
   pNoUILabel->setAlignment(Qt::AlignCenter);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Results name
   QLabel* pResultsLabel = new QLabel("Results Name:", this);
   mpResultsEdit = new QLineEdit(this);

   // create layer option
   mpCreateLayer = new QCheckBox("Create layer for the georeference results", this);
   mpCreateLayer->setToolTip("Toggles whether or not a layer is created for the georeference results.\n"
      "If a layer already exists, it will be updated for the new results regardless of this setting.");

   // display layer option
   mpDisplayLayer = new QCheckBox("Display the georeference results layer", this);
   mpDisplayLayer->setToolTip("Toggles whether or not the created georeference results layer is displayed.\n"
      "If the layer isn't created, this setting is ignored.");

   // Coordinate type
   QLabel* pCoordLabel = new QLabel("Coordinate Type:", this);
   mpCoordCombo = new QComboBox(this);
   mpCoordCombo->setEditable(false);
   mpCoordCombo->addItem("Latitude/Longitude");
   mpCoordCombo->addItem("UTM");
   mpCoordCombo->addItem("MGRS");

   // Horizontal line
   QFrame* pHLine2 = new QFrame(this);
   pHLine2->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QVBoxLayout* pPlugInLayout = new QVBoxLayout();
   pPlugInLayout->setMargin(0);
   pPlugInLayout->setSpacing(5);
   pPlugInLayout->addWidget(pGeoListLabel);
   pPlugInLayout->addWidget(mpGeoList, 10);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addLayout(pPlugInLayout, 0, 0, 1, 2);
   pGrid->addWidget(mpStack, 0, 2);
   pGrid->addWidget(pHLine, 1, 0, 1, 3);
   pGrid->addWidget(pResultsLabel, 2, 0);
   pGrid->addWidget(mpResultsEdit, 2, 1, 1, 2);
   pGrid->addWidget(pCoordLabel, 3, 0);
   pGrid->addWidget(mpCoordCombo, 3, 1, 1, 2, Qt::AlignLeft);
   pGrid->addWidget(mpCreateLayer, 4, 1, 1, 2, Qt::AlignLeft);
   pGrid->addWidget(mpDisplayLayer, 5, 1, 1, 2, Qt::AlignLeft);
   pGrid->addWidget(pHLine2, 6, 0, 1, 3);
   pGrid->addLayout(pButtonLayout, 7, 0, 1, 3);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnStretch(2, 10);

   // Initialization
   setWindowTitle(title);
   setModal(true);
   resize(400, 250);
   mpCreateLayer->setChecked(Georeference::getSettingCreateLatLonLayer());
   mpDisplayLayer->setChecked(mpCreateLayer->isChecked() ? Georeference::getSettingDisplayLatLonLayer() : false);
   mpDisplayLayer->setEnabled(mpCreateLayer->isChecked());

   if (geoPluginWidgetList.empty() == true)
   {
      mpGeoList->clearSelection();

      QLabel* pNoPlugInsLabel = new QLabel("No Georeference plug-ins can\nuse the current data cube", this);
      pNoPlugInsLabel->setAlignment(Qt::AlignCenter);

      mpStack->addWidget(pNoPlugInsLabel);
      mpStack->addWidget(pNoUILabel);
      mpStack->setCurrentWidget(pNoPlugInsLabel);
      pOkButton->setEnabled(false);
   }
   else
   {
      int defaultGeorefPlugInIndex = 0; // Default to 0th index
      if (mWidgets.size() == geoPluginNameList.size())
      {
         for (unsigned int i = 0; i < mWidgets.size(); ++i)
         {
            string plugInName = geoPluginNameList[i];
            if (plugInName.empty() == false)
            {
               mpGeoList->insertItem(i, QString::fromStdString(plugInName));
               if (plugInName == "RPC Georeference")
               {
                  defaultGeorefPlugInIndex = i;
               }
            }

            QWidget* pWidget = mWidgets[i];
            if (pWidget != NULL)
            {
               mpStack->addWidget(pWidget);
            }
            else
            {
               mWidgets[i] = pNoUILabel;
            }
         }
      }

      mpStack->addWidget(pNoUILabel);
      mpGeoList->setCurrentRow(defaultGeorefPlugInIndex);
      setPlugin(defaultGeorefPlugInIndex);
   }

   // Connections
   VERIFYNR(connect(mpGeoList, SIGNAL(currentRowChanged(int)), this, SLOT(setPlugin(int))));
   VERIFYNR(connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
   VERIFYNR(connect(mpCreateLayer, SIGNAL(toggled(bool)), this, SLOT(createLayerChanged(bool))));
}

GeoreferenceDlg::~GeoreferenceDlg()
{
}

void GeoreferenceDlg::setResultsName(const string& name)
{
   QString strName;
   if (name.empty() == false)
   {
      strName = QString::fromStdString(name);
   }

   mpResultsEdit->setText(strName);
}

string GeoreferenceDlg::getResultsName() const
{
   string name = "";

   QString strName = mpResultsEdit->text();
   if (strName.isEmpty() == false)
   {
      name = strName.toStdString();
   }

   return name;
}

GeocoordType GeoreferenceDlg::getGeocoordType() const
{
   GeocoordType eType = GEOCOORD_LATLON;

   int iIndex = mpCoordCombo->currentIndex();
   if (iIndex == 1)
   {
      eType = GEOCOORD_UTM;
   }
   else if (iIndex == 2)
   {
      eType = GEOCOORD_MGRS;
   }

   return eType;
}

int GeoreferenceDlg::getGeorefAlgorithmIndex() const
{
   int iIndex = -1;

   QList<QListWidgetItem*> items = mpGeoList->selectedItems();
   if (items.empty() == false)
   {
      QListWidgetItem* pItem = items.front();
      if (pItem != NULL)
      {
         iIndex = mpGeoList->row(pItem);
      }
   }

   return iIndex;
}

void GeoreferenceDlg::setPlugin(int iPluginIndex)
{
   if (static_cast<size_t>(iPluginIndex) < mWidgets.size())
   {
      mpStack->setCurrentWidget(mWidgets[iPluginIndex]);
   }
}

void GeoreferenceDlg::accept()
{
   if (getGeorefAlgorithmIndex() < 0)
   {
      QMessageBox::critical(this, "Georeference", "Please select a valid Georeference plug-in!");
      return;
   }

   if (getResultsName().empty() == true)
   {
      QMessageBox::critical(this, "Georeference", "Please enter a valid results layer name!");
      return;
   }

   QDialog::accept();
}

bool GeoreferenceDlg::getCreateLayer() const
{
   return mpCreateLayer->isChecked();
}

bool GeoreferenceDlg::getDisplayLayer() const
{
   return (getCreateLayer() && mpDisplayLayer->isChecked());
}

void GeoreferenceDlg::createLayerChanged(bool create)
{
   if (create == false)
   {
      mpDisplayLayer->setChecked(false);
   }

   mpDisplayLayer->setEnabled(create);
}