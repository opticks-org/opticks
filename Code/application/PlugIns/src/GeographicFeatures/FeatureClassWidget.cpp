/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConnectionParametersWidget.h"
#include "AppVerify.h"
#include "FeatureClass.h"
#include "FeatureClassProperties.h"
#include "FeatureClassWidget.h"
#include "LatLonLineEdit.h"
#include "ListInspectorWidget.h"
#include "QueryOptionsWidget.h"

#include <QtGui/QTabWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidgetItem>
#include <QtGui/QPushButton>
#include <QtGui/QProgressBar>
#include <QtGui/QRadioButton>

#include <algorithm>
#include <boost/bind.hpp>

FeatureClassWidget::FeatureClassWidget(QWidget* pParent) :
   QWidget(pParent),
   mpFeatureClass(NULL)
{
   QTabWidget* pTabWidget = new QTabWidget(this);

   QWidget* pConnectionTab = new QWidget(this);
   QGridLayout* pConnectionLayout = new QGridLayout(pConnectionTab);
   QLabel* pLayerNameLabel = new QLabel("Layer name:", pConnectionTab);
   mpLayerNameEdit = new QLineEdit(pConnectionTab);
   mpConnection = new ConnectionParametersWidget(pConnectionTab);

   pConnectionLayout->addWidget(pLayerNameLabel, 0, 0);
   pConnectionLayout->addWidget(mpLayerNameEdit, 0, 1);
   pConnectionLayout->addWidget(mpConnection, 1, 0, 1, 2);

   QueryOptionsWidget* pInspector = new QueryOptionsWidget;
   mpDisplay = new ListInspectorWidget(pInspector, this);

   mpClipping = new QWidget(this);
   mpNoClipButton = new QRadioButton("No clip", mpClipping);
   mpSceneClipButton = new QRadioButton("Clip to scene", mpClipping);
   mpSpecifiedClipButton = new QRadioButton("Specified clip", mpClipping);
   mpNorthEdit = new LatLonLineEdit(mpClipping);
   mpSouthEdit = new LatLonLineEdit(mpClipping);
   mpEastEdit = new LatLonLineEdit(mpClipping);
   mpWestEdit = new LatLonLineEdit(mpClipping);
   QLabel* pNorthLabel = new QLabel("North:", mpClipping);
   QLabel* pSouthLabel = new QLabel("South:", mpClipping);
   QLabel* pEastLabel = new QLabel("East:", mpClipping);
   QLabel* pWestLabel = new QLabel("West:", mpClipping);

   mSpecifiedClipWidgets.push_back(mpNorthEdit);
   mSpecifiedClipWidgets.push_back(mpSouthEdit);
   mSpecifiedClipWidgets.push_back(mpEastEdit);
   mSpecifiedClipWidgets.push_back(mpWestEdit);
   mSpecifiedClipWidgets.push_back(pNorthLabel);
   mSpecifiedClipWidgets.push_back(pSouthLabel);
   mSpecifiedClipWidgets.push_back(pEastLabel);
   mSpecifiedClipWidgets.push_back(pWestLabel);

   QGridLayout* pClipLayout = new QGridLayout(mpClipping);
   pClipLayout->setColumnStretch(2, 10);
   pClipLayout->setColumnMinimumWidth(0, 15);
   pClipLayout->addWidget(mpNoClipButton, 0, 0, 1, 3);
   pClipLayout->addWidget(mpSceneClipButton, 1, 0, 1, 3);
   pClipLayout->addWidget(mpSpecifiedClipButton, 2, 0, 1, 3);
   pClipLayout->addWidget(pNorthLabel, 3, 1);
   pClipLayout->addWidget(mpNorthEdit, 3, 2);
   pClipLayout->addWidget(pSouthLabel, 4, 1);
   pClipLayout->addWidget(mpSouthEdit, 4, 2);
   pClipLayout->addWidget(pEastLabel, 5, 1);
   pClipLayout->addWidget(mpEastEdit, 5, 2);
   pClipLayout->addWidget(pWestLabel, 6, 1);
   pClipLayout->addWidget(mpWestEdit, 6, 2);
   pClipLayout->setRowStretch(7, 10);

   pTabWidget->addTab(pConnectionTab, "Connection");
   pTabWidget->addTab(mpDisplay, "Display");
   pTabWidget->addTab(mpClipping, "Clipping");

   mpErrorLabel = new QLabel(this);
   // Font
   QFont errorFont = mpErrorLabel->font();
   errorFont.setBold(true);
   mpErrorLabel->setFont(errorFont);

   // Text color
   QPalette errorPalette = mpErrorLabel->palette();
   errorPalette.setColor(QPalette::WindowText, Qt::red);
   mpErrorLabel->setPalette(errorPalette);

   // Word wrap
   mpErrorLabel->setWordWrap(true);

   QPushButton* pTestConnectionButton = new QPushButton("Test connection", this);

   mpProgressBar = new QProgressBar(this);
   mpProgressBar->setRange(0, 0);
   mpProgressBar->setHidden(true);
   
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->setColumnStretch(1, 10);

   pLayout->addWidget(pTabWidget, 0, 0, 1, 3);
   pLayout->addWidget(mpProgressBar, 1, 0);
   pLayout->addWidget(mpErrorLabel, 1, 1);
   pLayout->addWidget(pTestConnectionButton, 1, 2);

   // Initialization
   setWindowTitle("Shape File");

   // Connections
   VERIFYNR(connect(pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(testConnection())));
   VERIFYNR(connect(pTestConnectionButton, SIGNAL(clicked()), this, SLOT(testConnection())));

   VERIFYNR(connect(mpDisplay, SIGNAL(addItems()), this, SLOT(addDisplayItems())));
   VERIFYNR(connect(mpDisplay, SIGNAL(saveInspector(QWidget*, QListWidgetItem*)), 
      this, SLOT(saveDisplayInspector(QWidget*, QListWidgetItem*))));
   VERIFYNR(connect(mpDisplay, SIGNAL(loadInspector(QWidget*, QListWidgetItem*)), 
      this, SLOT(loadDisplayInspector(QWidget*, QListWidgetItem*))));
   VERIFYNR(connect(mpDisplay, SIGNAL(removeItem(QListWidgetItem*)), 
      this, SLOT(removeDisplayItem(QListWidgetItem*))));

   VERIFYNR(connect(mpNoClipButton, SIGNAL(toggled(bool)), this, SLOT(clipButtonClicked())));
   VERIFYNR(connect(mpSceneClipButton, SIGNAL(toggled(bool)), this, SLOT(clipButtonClicked())));
   VERIFYNR(connect(mpSpecifiedClipButton, SIGNAL(toggled(bool)), this, SLOT(clipButtonClicked())));
}

FeatureClassWidget::~FeatureClassWidget()
{
}

namespace
{
   template<typename T>
   typename T::second_type second(const T &t)
   {
      return t.second;
   }
}
bool FeatureClassWidget::applyChanges()
{
   VERIFY(mpFeatureClass != NULL);

   mpFeatureClass->setLayerName(mpLayerNameEdit->text().toStdString());

   mpFeatureClass->setConnectionParameters(mpConnection->getConnectionParameters());

   mpDisplay->applyChanges();

   mpFeatureClass->clearQueries();
   std::for_each(mQueries.begin(), mQueries.end(), boost::bind(&FeatureClass::addQuery, mpFeatureClass, 
      boost::bind(second<std::pair<QListWidgetItem *const, QueryOptions> >, _1)));

   if (mpNoClipButton->isChecked())
   {
      mpFeatureClass->setClippingType(FeatureClass::NO_CLIP);
   }
   if (mpSceneClipButton->isChecked())
   {
      mpFeatureClass->setClippingType(FeatureClass::SCENE_CLIP);
   }
   if (mpSpecifiedClipButton->isChecked())
   {
      mpFeatureClass->setClippingType(FeatureClass::NO_CLIP);
      LocationType ll(mpSouthEdit->getValue(), mpEastEdit->getValue());
      LocationType ur(mpNorthEdit->getValue(), mpWestEdit->getValue());
      mpFeatureClass->setClipping(ll, ur);
   }


   return true;
}

void FeatureClassWidget::initialize(FeatureClass *pFeatureClass)
{
   if (pFeatureClass == NULL)
   {
      return;
   }

   mpFeatureClass = pFeatureClass;

   mpLayerNameEdit->setText(QString::fromStdString(mpFeatureClass->getLayerName()));

   // Initialize may have be called more than once, so reset related things
   mpDisplay->clearList();
   mQueries.clear();

   const std::vector<QueryOptions>& queries = mpFeatureClass->getQueries();
   for (std::vector<QueryOptions>::const_iterator iter = queries.begin();
      iter != queries.end(); ++iter)
   {
      mQueries[mpDisplay->addItem(iter->getQueryName())] = *iter;
   }

   const ArcProxyLib::ConnectionParameters& connect = mpFeatureClass->getConnectionParameters();

   setFeatureClassProperties(mpFeatureClass->getFeatureClassProperties(), connect.getConnectionType());

   mpConnection->setConnectionParameters(connect);

   std::pair<LocationType, LocationType> clipping = mpFeatureClass->getClipping();
   mpNorthEdit->setValue(DmsPoint(DmsPoint::DMS_LATITUDE, clipping.second.mX));
   mpSouthEdit->setValue(DmsPoint(DmsPoint::DMS_LATITUDE, clipping.first.mX));
   mpEastEdit->setValue(DmsPoint(DmsPoint::DMS_LONGITUDE, clipping.first.mY));
   mpWestEdit->setValue(DmsPoint(DmsPoint::DMS_LONGITUDE, clipping.second.mY));

   switch (mpFeatureClass->getClippingType())
   {
   case FeatureClass::NO_CLIP:
      mpNoClipButton->setChecked(true);
      break;
   case FeatureClass::SCENE_CLIP:
      mpSceneClipButton->setChecked(true);
      break;
   case FeatureClass::SPECIFIED_CLIP:
      mpSpecifiedClipButton->setChecked(true);
      break;
   default:
      break;
   }
}

void FeatureClassWidget::testConnection(bool onlyIfModified)
{
   if (!onlyIfModified || mpConnection->isModified())
   {
      mpErrorLabel->clear();
      mpProgressBar->setHidden(false);

      ArcProxyLib::FeatureClassProperties featureClassProperties;
      std::string errorMessage;

      ArcProxyLib::ConnectionParameters connect = mpConnection->getConnectionParameters();
      if (FeatureClass::testConnection(connect, featureClassProperties,
         errorMessage))
      {
         if (mpLayerNameEdit->text().startsWith(QString::fromStdString(FeatureClass::DEFAULT_LAYER_NAME)))
         {
            mpLayerNameEdit->setText(QString::fromStdString(connect.getFeatureClass()));
         }
         setFeatureClassProperties(featureClassProperties, 
            mpConnection->getConnectionParameters().getConnectionType());
         mpDisplay->setEnabled(true);
         mpClipping->setEnabled(true);
      }
      else
      {
         mpDisplay->setEnabled(false);
         mpClipping->setEnabled(false);
         mpErrorLabel->setText(QString("Connection: %1").arg(QString::fromStdString(errorMessage)));
      }
      mpProgressBar->setHidden(true);

   }

   mpConnection->setModified(false);
}

void FeatureClassWidget::setFeatureClassProperties(
   const ArcProxyLib::FeatureClassProperties &featureClassProperties,
   ArcProxyLib::ConnectionType connectionType)
{
   if (featureClassProperties.getFeatureType() == ArcProxyLib::UNKNOWN)
   {
      return;
   }

   QueryOptionsWidget* pInspector = dynamic_cast<QueryOptionsWidget*>(mpDisplay->getInspector());
   VERIFYNRV(pInspector != NULL);

   bool hideQueries = connectionType == ArcProxyLib::SHAPELIB_CONNECTION;
   if (hideQueries)
   {
      mpDisplay->clearList();
      
      VERIFYNRV(!mQueries.empty());
      QueryOptions options = mQueries.begin()->second;
      
      mQueries.clear();
      mQueries[mpDisplay->addItem(options.getQueryName())] = options;
   }
   mpDisplay->setHideList(hideQueries);
   pInspector->setHideQueryBuilder(hideQueries);

   pInspector->setFeatureType(featureClassProperties.getFeatureType());
   pInspector->setFeatureCount(featureClassProperties.getFeatureCount());
   pInspector->setFeatureFields(featureClassProperties.getFields(), 
      featureClassProperties.getTypes(), 
      featureClassProperties.getSampleValues());
}

void FeatureClassWidget::addDisplayItems()
{
   QueryOptions options;
   options.setQueryName(mpDisplay->getUniqueName(options.getQueryName()));
   mQueries[mpDisplay->addItem(options.getQueryName())] = options;
}

void FeatureClassWidget::saveDisplayInspector(QWidget *pInspector, QListWidgetItem *pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   QueryOptionsWidget* pOptionsWidget = dynamic_cast<QueryOptionsWidget*>(pInspector);
   VERIFYNRV(pOptionsWidget != NULL);

   std::map<QListWidgetItem*, QueryOptions>::iterator iter = mQueries.find(pItem);
   if (iter == mQueries.end())
   {
      return;
   }

   iter->second = pOptionsWidget->getDisplayOptions();
   if (pItem->text().toStdString() != iter->second.getQueryName())
   {
      std::string newName = mpDisplay->getUniqueName(iter->second.getQueryName());
      iter->second.setQueryName(newName);
      pItem->setText(QString::fromStdString(newName));
   }
}

void FeatureClassWidget::loadDisplayInspector(QWidget* pInspector, QListWidgetItem* pItem)
{
   QueryOptionsWidget* pOptionsWidget = dynamic_cast<QueryOptionsWidget*>(pInspector);
   VERIFYNRV(pOptionsWidget != NULL);

   std::map<QListWidgetItem*, QueryOptions>::iterator iter = mQueries.find(pItem);
   if (iter == mQueries.end())
   {
      return;
   }

   pOptionsWidget->setDisplayOptions(iter->second);
}

void FeatureClassWidget::removeDisplayItem(QListWidgetItem* pItem)
{
   std::map<QListWidgetItem*, QueryOptions>::iterator iter = mQueries.find(pItem);
   if (iter == mQueries.end())
   {
      return;
   }

   mQueries.erase(iter);
}

void FeatureClassWidget::clipButtonClicked()
{
   bool useLineEdits = true;
   if (mpNoClipButton->isChecked() || mpSceneClipButton->isChecked())
   {
      useLineEdits = false;
   }

   std::for_each(mSpecifiedClipWidgets.begin(), mSpecifiedClipWidgets.end(),
      boost::bind(&QWidget::setEnabled, _1, useLineEdits));
}

void FeatureClassWidget::setAvailableConnectionTypes(
   const std::vector<ArcProxyLib::ConnectionType> &types)
{
   mpConnection->setAvailableConnectionTypes(types);
}
