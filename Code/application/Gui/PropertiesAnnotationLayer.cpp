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

#include "AnnotationLayer.h"
#include "AnnotationLayerImp.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "GraphicObject.h"
#include "PropertiesAnnotationLayer.h"
#include "PropertiesGraphicObject.h"
#include "Undo.h"

#include <list>
using namespace std;

PropertiesAnnotationLayer::PropertiesAnnotationLayer() :
   QWidget(NULL),
   mpAnnotationLayer(NULL)
{
   QSplitter* pSplitter = new QSplitter(Qt::Horizontal, this);

   mpObjectsList = new QListWidget(pSplitter);
   mpObjectsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpObjectsList->setMinimumWidth(150);

   mpNoObjectLabel = new QLabel("Select one or more objects to modify the properties");
   mpNoObjectLabel->setAlignment(Qt::AlignCenter);

   mpPropertiesWidget = new PropertiesGraphicObject();

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

PropertiesAnnotationLayer::~PropertiesAnnotationLayer()
{
}

bool PropertiesAnnotationLayer::initialize(SessionItem* pSessionItem)
{
   mpAnnotationLayer = dynamic_cast<AnnotationLayer*>(pSessionItem);
   if (mpAnnotationLayer == NULL)
   {
      return false;
   }

   VERIFYNR(disconnect(mpObjectsList, SIGNAL(itemSelectionChanged()), this, SLOT(updateProperties())));

   mObjects.clear();
   mpObjectsList->clear();

   list<GraphicObject*> objects;
   mpAnnotationLayer->getObjects(objects);

   for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         QListWidgetItem* pItem = new QListWidgetItem(QString::fromStdString(pObject->getName()), mpObjectsList);
         if (pItem != NULL)
         {
            mObjects[pItem] = pObject;
            if (mpAnnotationLayer->isObjectSelected(pObject) == true)
            {
               pItem->setSelected(true);
            }
         }
      }
   }

   mpPropertiesWidget->initialize(objects);
   mpLockCheck->setChecked(mpAnnotationLayer->getLayerLocked());
   mPropertiesModifier.setModified(false);
   updateProperties();

   VERIFYNR(connect(mpObjectsList, SIGNAL(itemSelectionChanged()), this, SLOT(updateProperties())));
   return true;
}

bool PropertiesAnnotationLayer::applyChanges()
{
   if (mpAnnotationLayer == NULL)
   {
      return false;
   }

   string actionText = "Set " + getName();
   UndoGroup group(mpAnnotationLayer->getView(), actionText);

   bool bSuccess = mpPropertiesWidget->applyChanges();
   if (bSuccess == true)
   {
      mpAnnotationLayer->setLayerLocked(mpLockCheck->isChecked());
      mPropertiesModifier.setModified(false);
   }

   return bSuccess;
}

const string& PropertiesAnnotationLayer::getName()
{
   static string name = "Annotation Layer Properties";
   return name;
}

const string& PropertiesAnnotationLayer::getPropertiesName()
{
   static string propertiesName = "Annotation Layer";
   return propertiesName;
}

const string& PropertiesAnnotationLayer::getDescription()
{
   static string description = "General setting properties of an annotation layer";
   return description;
}

const string& PropertiesAnnotationLayer::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesAnnotationLayer::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesAnnotationLayer::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesAnnotationLayer::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesAnnotationLayer::getDescriptorId()
{
   static string id = "{E018596C-1FF6-428A-85CF-2530EED24D48}";
   return id;
}

bool PropertiesAnnotationLayer::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesAnnotationLayer::updateProperties()
{
   if (mpAnnotationLayer == NULL)
   {
      return;
   }

   if (mPropertiesModifier.isModified() == true)
   {
      if (QMessageBox::warning(this, "Properties", "Do you want to apply the changes to the currently "
         "selected object(s)?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      {
         applyChanges();
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
