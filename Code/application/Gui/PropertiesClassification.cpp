/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "Classification.h"
#include "ClassificationWidget.h"
#include "DataElement.h"
#include "PlotWidget.h"
#include "PropertiesClassification.h"
#include "View.h"

PropertiesClassification::PropertiesClassification()
{
   setName("Classification Properties");
   setPropertiesName("Classification");
   setDescription("Classification markings for a session item");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{44EF2642-BAE1-411C-B939-F86FA80DF3DD}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesClassification::~PropertiesClassification()
{}

bool PropertiesClassification::initialize(SessionItem* pSessionItem)
{
   ClassificationWidget* pClassificationPage = dynamic_cast<ClassificationWidget*>(getWidget());
   if (pClassificationPage == NULL)
   {
      return false;
   }

   DataElement* pElement = dynamic_cast<DataElement*>(pSessionItem);
   if (pElement != NULL)
   {
      mpClassification->setClassification(pElement->getClassification());
   }
   else
   {
      View* pView = dynamic_cast<View*>(pSessionItem);
      if (pView != NULL)
      {
         mpClassification->setClassification(pView->getClassification());
      }
      else
      {
         PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(pSessionItem);
         if (pPlotWidget != NULL)
         {
            mpClassification->setClassification(pPlotWidget->getClassification());
         }
         else
         {
            return false;
         }
      }
   }

   pClassificationPage->setClassification(mpClassification.get());
   return PropertiesShell::initialize(pSessionItem);
}

bool PropertiesClassification::applyChanges()
{
   ClassificationWidget* pClassificationPage = dynamic_cast<ClassificationWidget*>(getWidget());
   VERIFY(pClassificationPage != NULL);

   bool success = pClassificationPage->applyChanges();
   if (success == false)
   {
      return false;
   }

   DataElement* pElement = dynamic_cast<DataElement*>(getSessionItem());
   if (pElement != NULL)
   {
      pElement->setClassification(mpClassification.get());
   }
   else
   {
      View* pView = dynamic_cast<View*>(getSessionItem());
      if (pView != NULL)
      {
         pView->setClassification(mpClassification.get());
      }
      else
      {
         PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(getSessionItem());
         if (pPlotWidget != NULL)
         {
            pPlotWidget->setClassification(mpClassification.get());
         }
         else
         {
            return false;
         }
      }
   }

   return true;
}

QWidget* PropertiesClassification::createWidget()
{
   QWidget* pWidget = new ClassificationWidget();
   return pWidget;
}
