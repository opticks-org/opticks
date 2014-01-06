/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FeatureMenuEditorDlg.h"

#include "ConfigurationSettings.h"
#include "AppVerify.h"
#include "FeatureClassWidget.h"
#include "FeatureProxyConnector.h"
#include "Filename.h"
#include "ListInspectorWidget.h"
#include "ProgressResource.h"
#include "QueryOptions.h"
#include "QueryOptionsWidget.h"
#include "FeatureManager.h"

#include <QtGui/QPushButton>
#include <QtGui/QListWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QDialogButtonBox>

using namespace std;

FeatureMenuEditorDlg::FeatureMenuEditorDlg(QWidget *pParent) : QDialog(pParent)
{
   setWindowTitle("Feature Menu Editor");

   const DynamicObject* pOptionsSetting = FeatureManager::getSettingOptions();
   if (pOptionsSetting != NULL)
   {
      mpOptionsSet->merge(pOptionsSetting);
   }

   FeatureClassWidget* pInspectorWidget = new FeatureClassWidget(this);
   
   FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
   std::vector<ArcProxyLib::ConnectionType> availableTypes = pProxy->getAvailableConnectionTypes();
   pInspectorWidget->setAvailableConnectionTypes(availableTypes);

   mpListInspector = new ListInspectorWidget(pInspectorWidget, this);
   QLayout* pListInspectorLayout = mpListInspector->layout();
   if (pListInspectorLayout != NULL)
   {
      pListInspectorLayout->setMargin(0);
   }

   VERIFYNR(connect(mpListInspector, SIGNAL(addItems()),
      this, SLOT(addItems())));
   VERIFYNR(connect(mpListInspector, SIGNAL(saveInspector(QWidget*, QListWidgetItem*)), 
      this, SLOT(saveInspector(QWidget*, QListWidgetItem*))));
   VERIFYNR(connect(mpListInspector, SIGNAL(loadInspector(QWidget*, QListWidgetItem*)), 
      this, SLOT(loadInspector(QWidget*, QListWidgetItem*))));
   VERIFYNR(connect(mpListInspector, SIGNAL(removeItem(QListWidgetItem*)), 
      this, SLOT(removeItem(QListWidgetItem*))));

   QDialogButtonBox* pButtons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpListInspector);
   pLayout->addWidget(pHLine);
   pLayout->addWidget(pButtons);

   // Initialization
   vector<string> names;
   mpOptionsSet->getAttributeNames(names);

   QListWidgetItem* pFirstItem = NULL;
   for (vector<string>::iterator iter = names.begin(); iter != names.end(); ++iter)
   {
      QListWidgetItem* pItem = mpListInspector->addItem(*iter);
      if (pFirstItem == NULL)
      {
         pFirstItem = pItem;
      }
   }

   if (pFirstItem != NULL)
   {
      mpListInspector->setCurrentItem(pFirstItem);
   }

   // Connections
   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(reject())));
}

void FeatureMenuEditorDlg::accept()
{
   mpListInspector->applyChanges();

   FeatureManager::setSettingOptionsVersion(FeatureManager::mCurrentVersion);
   FeatureManager::setSettingOptions(mpOptionsSet.get());
  
   QDialog::accept();
}

void FeatureMenuEditorDlg::addItems()
{
   FeatureClass featureClass;

   string newName = mpListInspector->getUniqueName(featureClass.getLayerName());
   featureClass.setLayerName(newName);
   FactoryResource<DynamicObject> pOptions(featureClass.toDynamicObject());
   mpOptionsSet->setAttribute(newName, *pOptions.get());

   QListWidgetItem* pItem = mpListInspector->addItem(newName);
   if (pItem != NULL)
   {
      mpListInspector->setCurrentItem(pItem);
   }
}

void FeatureMenuEditorDlg::loadInspector(QWidget *pInspector, QListWidgetItem *pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   FeatureClassWidget* pInspectorWidget = dynamic_cast<FeatureClassWidget*>(pInspector);
   VERIFYNRV(pInspectorWidget != NULL);

   string name = pItem->text().toStdString();

   mCurrentFeatureClass.fromDynamicObject(mpOptionsSet->getAttribute(name).getPointerToValue<DynamicObject>());

   pInspectorWidget->initialize(&mCurrentFeatureClass);
   pInspectorWidget->testConnection(false);
}

void FeatureMenuEditorDlg::saveInspector(QWidget *pInspector, QListWidgetItem *pItem)
{
   FeatureClassWidget* pInspectorWidget = dynamic_cast<FeatureClassWidget*>(pInspector);

   if (pItem == NULL)
   {
      return;
   }

   VERIFYNRV(pInspectorWidget != NULL);
   pInspectorWidget->applyChanges();

   string name = mCurrentFeatureClass.getLayerName();

   string oldName = pItem->text().toStdString();
   if (name != oldName)
   {
      name = mpListInspector->getUniqueName(name);
   }
   mCurrentFeatureClass.setLayerName(name);

   pItem->setText(QString::fromStdString(name));

   FactoryResource<DynamicObject> pOptions(mCurrentFeatureClass.toDynamicObject());
   mpOptionsSet->removeAttribute(oldName);
   mpOptionsSet->setAttribute(name, *pOptions);
}

void FeatureMenuEditorDlg::removeItem(QListWidgetItem *pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   VERIFYNRV(mpOptionsSet.get() != NULL);

   mpOptionsSet->removeAttribute(pItem->text().toStdString());
}





