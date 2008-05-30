/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "FeatureClass.h"
#include "FeatureClassWidget.h"
#include "FeatureManager.h"
#include "FeatureProxyConnector.h"
#include "ProgressResource.h"
#include "PropertiesFeatureClass.h"

using namespace std;

PropertiesFeatureClass::PropertiesFeatureClass() :
   mpFeatureClass(NULL)
{
   setName("Feature Class Properties");
   setPropertiesName("Feature Class");
   setDescription("General setting properties of a feature class");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{3B7D23F1-7415-4563-B409-0B2235022EA9}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesFeatureClass::~PropertiesFeatureClass()
{
}

bool PropertiesFeatureClass::initialize(SessionItem* pSessionItem)
{
   FeatureClassWidget* pWidget = dynamic_cast<FeatureClassWidget*>(getWidget());
   if (pWidget == NULL)
   {
      return false;
   }

   vector<SessionItem*> sessionItems;
   sessionItems.push_back(pSessionItem);

   vector<FeatureClass*> featureClasses = FeatureManager::getFeatureClasses(sessionItems);
   VERIFY(featureClasses.size() == 1);

   mpFeatureClass = featureClasses.front();
   VERIFY(mpFeatureClass != NULL);

   FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
   VERIFY(pProxy != NULL);

   pWidget->initialize(mpFeatureClass);
   pWidget->setAvailableConnectionTypes(pProxy->getAvailableConnectionTypes());

   return true;
}

bool PropertiesFeatureClass::applyChanges()
{
   if (mpFeatureClass == NULL)
   {
      return false;
   }

   FeatureClassWidget* pWidget = dynamic_cast<FeatureClassWidget*>(getWidget());
   if (pWidget == NULL)
   {
      return false;
   }

   bool bSuccess = pWidget->applyChanges();
   if (bSuccess == true)
   {
      ProgressResource pProgress("Geographic feature");
      string errorMessage;
      mpFeatureClass->update(pProgress.get(), errorMessage);

      pProgress->updateProgress("Complete", 100, NORMAL);
   }

   return bSuccess;
}

QWidget* PropertiesFeatureClass::createWidget()
{
   Service<DesktopServices> pDesktop;
   QWidget* pParent = pDesktop->getMainWidget();

   QWidget* pWidget = new FeatureClassWidget(pParent);
   return pWidget;
}
