/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "FeatureClass.h"
#include "GeographicFeaturesTabs.h"
#include "GeographicFeaturesWidget.h"
#include "GraphicLayer.h"
#include "LayerList.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

GeographicFeaturesWidget::GeographicFeaturesWidget(QWidget* pParent) :
   QStackedWidget(pParent)
{
   // Default label widget
   QWidget* pDefaultWidget = new QWidget(this);

   QLabel* pDefaultLabel = new QLabel("No geographic features are available for display.", pDefaultWidget);
   pDefaultLabel->setAlignment(Qt::AlignCenter);

   QLayout* pDefaultLayout = new QVBoxLayout(pDefaultWidget);
   pDefaultLayout->setMargin(10);
   pDefaultLayout->addWidget(pDefaultLabel);

   // View widget
   QWidget* pViewWidget = new QWidget(this);

   QLabel* pViewLabel = new QLabel("View:", pViewWidget);
   mpViewCombo = new QComboBox(pViewWidget);
   mpViewStack = new QStackedWidget(pViewWidget);

   QGridLayout* pViewLayout = new QGridLayout(pViewWidget);
   pViewLayout->setMargin(10);
   pViewLayout->setSpacing(5);
   pViewLayout->addWidget(pViewLabel, 0, 0);
   pViewLayout->addWidget(mpViewCombo, 0, 1);
   pViewLayout->addWidget(mpViewStack, 1, 0, 1, 2);
   pViewLayout->setRowStretch(1, 10);
   pViewLayout->setColumnStretch(1, 10);

   // Initialization
   addWidget(pDefaultWidget);
   addWidget(pViewWidget);

   // Connections
   VERIFYNR(connect(mpViewCombo, SIGNAL(currentIndexChanged(int)), mpViewStack, SLOT(setCurrentIndex(int))));

   Service<DesktopServices> pDesktop;
   VERIFYNR(pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowAdded),
      Slot(this, &GeographicFeaturesWidget::windowAdded)));
   VERIFYNR(pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowRemoved),
      Slot(this, &GeographicFeaturesWidget::windowRemoved)));

   // Listen to SessionRestored instead of overriding serialize/deserialize because some of the needed connections
   // had not been restored by time deserialize would have been called.
   Service<SessionManager> pSessionMgr;
   VERIFYNR(pSessionMgr->attach(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &GeographicFeaturesWidget::initializeSession)));
}

GeographicFeaturesWidget::~GeographicFeaturesWidget()
{
   Service<DesktopServices> pDesktop;
   VERIFYNR(pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded),
      Slot(this, &GeographicFeaturesWidget::windowAdded)));
   VERIFYNR(pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowRemoved),
      Slot(this, &GeographicFeaturesWidget::windowRemoved)));

   Service<SessionManager> pSessionMgr;
   VERIFYNR(pSessionMgr->detach(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &GeographicFeaturesWidget::initializeSession)));
}

bool GeographicFeaturesWidget::displayLayer(GraphicLayer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   View* pView = pLayer->getView();
   if (pView == NULL)
   {
      return false;
   }

   QString viewName = QString::fromStdString(pView->getName());
   VERIFY(viewName.isEmpty() == false);

   int index = mpViewCombo->findData(QVariant(viewName));
   if (index > -1)
   {
      GeographicFeaturesTabs* pTabWidget = dynamic_cast<GeographicFeaturesTabs*>(mpViewStack->widget(index));
      if (pTabWidget != NULL)
      {
         if (pTabWidget->activateTab(pLayer) == true)
         {
            mpViewCombo->setCurrentIndex(index);
            mpViewStack->setCurrentIndex(index);
            return true;
         }
      }
   }

   return false;
}

void GeographicFeaturesWidget::windowAdded(Subject& subject, const std::string& signal, const boost::any& data)
{
   // Not supporting views displayed in product windows.
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(boost::any_cast<Window*>(data));
   if (pWindow != NULL)
   {
      SpatialDataView* pView = pWindow->getSpatialDataView();
      if (pView != NULL)
      {
         LayerList* pList = pView->getLayerList();
         VERIFYNRV(pList != NULL);

         // Attach to the layer list
         VERIFYNR(pList->attach(SIGNAL_NAME(LayerList, LayerAdded), Slot(this, &GeographicFeaturesWidget::layerAdded)));
         VERIFYNR(pList->attach(SIGNAL_NAME(LayerList, LayerDeleted),
            Slot(this, &GeographicFeaturesWidget::layerDeleted)));

         // Attach to each layer already in the layer list and update the tab for the view
         std::vector<Layer*> layers = pList->getLayers(ANNOTATION);
         for (std::vector<Layer*>::const_iterator layerIter = layers.begin(); layerIter != layers.end(); ++layerIter)
         {
            layerAdded(*pList, SIGNAL_NAME(LayerList, LayerAdded), boost::any(*layerIter));
         }
      }
   }
}

void GeographicFeaturesWidget::windowRemoved(Subject& subject, const std::string& signal, const boost::any& data)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(boost::any_cast<Window*>(data));
   if (pWindow != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pWindow->getSpatialDataView());
      if (pView != NULL)
      {
         LayerList* pList = pView->getLayerList();
         VERIFYNRV(pList != NULL);

         // Detach from the layer list
         VERIFYNR(pList->detach(SIGNAL_NAME(LayerList, LayerAdded), Slot(this, &GeographicFeaturesWidget::layerAdded)));
         VERIFYNR(pList->detach(SIGNAL_NAME(LayerList, LayerDeleted),
            Slot(this, &GeographicFeaturesWidget::layerDeleted)));

         // Detach from each layer and update the tab for the view
         std::vector<Layer*> layers = pList->getLayers(ANNOTATION);
         for (std::vector<Layer*>::const_iterator layerIter = layers.begin(); layerIter != layers.end(); ++layerIter)
         {
            layerDeleted(*pList, SIGNAL_NAME(LayerList, LayerDeleted), boost::any(*layerIter));
         }
      }
   }
}

void GeographicFeaturesWidget::layerAdded(Subject& subject, const std::string& signal, const boost::any& data)
{
   GraphicLayer* pGraphicLayer = dynamic_cast<GraphicLayer*>(boost::any_cast<Layer*>(data));
   if (pGraphicLayer == NULL)
   {
      return;
   }

   AnnotationElement* pAnnotationElement = dynamic_cast<AnnotationElement*>(pGraphicLayer->getDataElement());
   if (pAnnotationElement == NULL)
   {
      return;
   }

   std::vector<DataElement*> elements = Service<ModelServices>()->getElements(pAnnotationElement, "FeatureClass");
   if (!elements.empty())
   {
      FeatureClass* pFeatureClass = model_cast<FeatureClass*>(elements.front());
      if (pFeatureClass != NULL)
      {
         View* pView = pGraphicLayer->getView();
         if (pView != NULL)
         {
            GeographicFeaturesTabs* pTabWidget = NULL;

            // Add an item to the view combo box and a tab widget if necessary
            QString viewName = QString::fromStdString(pView->getName());

            int index = mpViewCombo->findData(QVariant(viewName));
            if (index == -1)
            {
               // View combo box
               QString viewDisplayName = QString::fromStdString(pView->getDisplayName(true));
               mpViewCombo->addItem(viewDisplayName, QVariant(viewName));

               // Tab widget
               pTabWidget = new GeographicFeaturesTabs(mpViewStack);
               index = mpViewStack->addWidget(pTabWidget);
            }
            else
            {
               pTabWidget = dynamic_cast<GeographicFeaturesTabs*>(mpViewStack->widget(index));
            }

            if (pTabWidget == NULL)
            {
               return;
            }

            // Activate the view item in the combo box, which will automatically update the tab widget
            mpViewCombo->setCurrentIndex(index);

            // Attach to the geographic features Any element
            std::vector<DataElement*> anyElements = Service<ModelServices>()->getElements(pAnnotationElement,
               TypeConverter::toString<Any>());
            if (anyElements.empty() == false)
            {
               Any* pAny = dynamic_cast<Any*>(anyElements.front());
               if (pAny != NULL)
               {
                  VERIFYNR(pAny->attach(SIGNAL_NAME(Subject, Deleted),
                     Slot(this, &GeographicFeaturesWidget::featureClassDeleted)));
               }
            }

            // Create a new tab in the tab widget
            pTabWidget->addTab(pFeatureClass, pGraphicLayer);

            // Show the tab widgets instead of the default label widget if necessary
            if (currentIndex() != 1)
            {
               setCurrentIndex(1);
            }
         }
      }
   }
}

void GeographicFeaturesWidget::layerDeleted(Subject& subject, const std::string& signal, const boost::any& data)
{
   Layer* pLayer = boost::any_cast<Layer*>(data);
   if (pLayer != NULL)
   {
      removeLayer(pLayer);
   }
}

void GeographicFeaturesWidget::featureClassDeleted(Subject& subject, const std::string& signal, const boost::any& data)
{
   Any* pAny = dynamic_cast<Any*>(&subject);
   if (pAny == NULL)
   {
      return;
   }

   GraphicElement* pGraphicElement = dynamic_cast<GraphicElement*>(pAny->getParent());
   if (pGraphicElement == NULL)
   {
      return;
   }

   Service<DesktopServices> pDesktop;

   for (int i = 0; i < mpViewCombo->count(); ++i)
   {
      QString viewName = mpViewCombo->itemData(i).toString();
      if (viewName.isEmpty() == false)
      {
         SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(viewName.toStdString(),
            SPATIAL_DATA_WINDOW));
         if (pWindow != NULL)
         {
            SpatialDataView* pView = pWindow->getSpatialDataView();
            if (pView != NULL)
            {
               LayerList* pLayerList = pView->getLayerList();
               if (pLayerList != NULL)
               {
                  Layer* pLayer = pLayerList->getLayer(ANNOTATION, pGraphicElement);
                  if (pLayer != NULL)
                  {
                     removeLayer(pLayer);
                     break;
                  }
               }
            }
         }
      }
   }
}

void GeographicFeaturesWidget::removeLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return;
   }

   View* pView = pLayer->getView();
   if (pView == NULL)
   {
      return;
   }

   QString viewName = QString::fromStdString(pView->getName());
   int index = mpViewCombo->findData(QVariant(viewName));
   if (index != -1)
   {
      // Remove the layer tab in the tab widget
      GeographicFeaturesTabs* pTabWidget = dynamic_cast<GeographicFeaturesTabs*>(mpViewStack->widget(index));
      if (pTabWidget != NULL)
      {
         if (pTabWidget->removeTab(pLayer) == true)
         {
            // Detach from the geographic features Any element
            std::vector<DataElement*> anyElements = Service<ModelServices>()->getElements(pLayer->getDataElement(),
               TypeConverter::toString<Any>());
            if (anyElements.empty() == false)
            {
               Any* pAny = dynamic_cast<Any*>(anyElements.front());
               if (pAny != NULL)
               {
                  VERIFYNR(pAny->detach(SIGNAL_NAME(Subject, Deleted),
                     Slot(this, &GeographicFeaturesWidget::featureClassDeleted)));
               }
            }

            // If no feature layers remain in the view, remove the tab widget and view combo box item
            if (pTabWidget->count() <= 0)
            {
               mpViewCombo->removeItem(index);
               mpViewStack->removeWidget(pTabWidget);

               // Show the default label widget if no views remain
               if (mpViewCombo->count() == 0)
               {
                  setCurrentIndex(0);
               }
            }
         }
      }
   }
}

void GeographicFeaturesWidget::initializeSession(Subject& subject, const std::string& signal, const boost::any& data)
{
   std::vector<Window*> windows;
   Service<DesktopServices> pDesktop;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);

   for (std::vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      Window* pWindow = *iter;
      if (pWindow != NULL)
      {
         windowAdded(*(pDesktop.get()), SIGNAL_NAME(DesktopServices, WindowAdded), boost::any(pWindow));
      }
   }
}
