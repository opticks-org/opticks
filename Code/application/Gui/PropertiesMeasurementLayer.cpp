/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>

#include "AppVerify.h"
#include "AppVersion.h"
#include "GraphicObject.h"
#include "MeasurementLayer.h"
#include "MeasurementLayerImp.h"
#include "PropertiesMeasurementLayer.h"
#include "PropertiesMeasurementObject.h"
#include "Undo.h"
#include "View.h"

#include <list>
using namespace std;

PropertiesMeasurementLayer::PropertiesMeasurementLayer() :
   QWidget(NULL),
   mpMeasurementLayer(NULL)
{
   QSplitter* pSplitter = new QSplitter(Qt::Horizontal, this);

   mpObjectsList = new QListWidget(pSplitter);
   mpObjectsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpObjectsList->setMinimumWidth(150);

   mpNoObjectLabel = new QLabel("Select one or more objects to modify the properties");
   mpNoObjectLabel->setAlignment(Qt::AlignCenter);

   mpPropertiesWidget = new PropertiesMeasurementObject();

   mpStack = new QStackedWidget(pSplitter);
   mpStack->addWidget(mpNoObjectLabel);
   mpStack->addWidget(mpPropertiesWidget);
   mpStack->setCurrentWidget(mpNoObjectLabel);

   mpLockCheck = new QCheckBox("Lock Layer", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pSplitter, 10);
   pLayout->addWidget(mpLockCheck);

   // Initialization
   pSplitter->insertWidget(0, mpObjectsList);
   pSplitter->insertWidget(1, mpStack);
   pSplitter->setStretchFactor(0, 1);
   pSplitter->setStretchFactor(1, 5);

   // Connections
   VERIFYNR(connect(mpObjectsList, SIGNAL(itemSelectionChanged()), this, SLOT(updateProperties())));
   VERIFYNR(mPropertiesModifier.attachSignal(mpPropertiesWidget, SIGNAL(modified())));
}

PropertiesMeasurementLayer::~PropertiesMeasurementLayer()
{
}

bool PropertiesMeasurementLayer::initialize(SessionItem* pSessionItem)
{
   mpMeasurementLayer = dynamic_cast<MeasurementLayer*>(pSessionItem);
   if (mpMeasurementLayer == NULL)
   {
      return false;
   }

   VERIFYNR(disconnect(mpObjectsList, SIGNAL(itemSelectionChanged()), this, SLOT(updateProperties())));

   mObjects.clear();
   mpObjectsList->clear();

   list<GraphicObject*> objects;
   mpMeasurementLayer->getObjects(objects);

   for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         QListWidgetItem* pItem = new QListWidgetItem(QString::fromStdString(pObject->getName()), mpObjectsList);
         if (pItem != NULL)
         {
            mObjects[pItem] = pObject;
            if (mpMeasurementLayer->isObjectSelected(pObject) == true)
            {
               pItem->setSelected(true);
            }
         }
      }
   }

   VERIFY(mpPropertiesWidget->initialize(objects));
   mpLockCheck->setChecked(mpMeasurementLayer->getLayerLocked());
   mPropertiesModifier.setModified(false);
   updateProperties();

   VERIFYNR(connect(mpObjectsList, SIGNAL(itemSelectionChanged()), this, SLOT(updateProperties())));
   return true;
}

bool PropertiesMeasurementLayer::applyChanges()
{
   if (mpMeasurementLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpMeasurementLayer->getView(), actionText);

   bool bSuccess = mpPropertiesWidget->applyChanges();
   if (bSuccess == true)
   {
      mpMeasurementLayer->setLayerLocked(mpLockCheck->isChecked());
      mPropertiesModifier.setModified(false);
      mpMeasurementLayer->getView()->refresh();
   }

   return bSuccess;
}

const string& PropertiesMeasurementLayer::getName()
{
   static string name = "Measurement Layer Properties";
   return name;
}

const string& PropertiesMeasurementLayer::getPropertiesName()
{
   static string propertiesName = "Measurement Layer";
   return propertiesName;
}

const string& PropertiesMeasurementLayer::getDescription()
{
   static string description = "General setting properties of a measurement layer";
   return description;
}

const string& PropertiesMeasurementLayer::getShortDescription()
{
   static string description = "General setting properties of a measurement layer";
   return description;
}

const string& PropertiesMeasurementLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesMeasurementLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesMeasurementLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesMeasurementLayer::getDescriptorId()
{
   static string id = "{2AEFF93F-D224-4087-AE04-3AF9BB7217BB}";
   return id;
}

bool PropertiesMeasurementLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesMeasurementLayer::updateProperties()
{
   if (mpMeasurementLayer == NULL)
   {
      return;
   }

   if (mPropertiesModifier.isModified() == true)
   {
      if (QMessageBox::warning(this, "Properties", "Do you want to apply the changes to the currently "
         "selected object(s)?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      {
         if (!applyChanges()) // warn user if an error occurred
         {
            QMessageBox::warning(this, APP_NAME, "A problem occurred and "
               "one or more of the property changes may not have been applied");
         }
      }
   }

   QList<QListWidgetItem*> selectedObjects = mpObjectsList->selectedItems();
   if (selectedObjects.empty() == false)
   {
      list<GraphicObject*> objects;
      for (int i = 0; i < selectedObjects.count(); ++i)
      {
         QListWidgetItem* pItem = selectedObjects[i];
         if (pItem != NULL)
         {
            map<QListWidgetItem*, GraphicObject*>::iterator iter = mObjects.find(pItem);
            if (iter != mObjects.end())
            {
               GraphicObject* pObject = iter->second;
               if (pObject != NULL)
               {
                  objects.push_back(pObject);
               }
            }
         }
      }

      mpPropertiesWidget->initialize(objects);
      mpStack->setCurrentWidget(mpPropertiesWidget);
   }
   else
   {
      mpStack->setCurrentWidget(mpNoObjectLabel);
   }

   mPropertiesModifier.setModified(false);
}
