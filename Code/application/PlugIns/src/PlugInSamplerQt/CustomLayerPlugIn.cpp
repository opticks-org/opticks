/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AppVerify.h"
#include "CustomLayerDrawObject.h"
#include "CustomLayerPlugIn.h"
#include "DataVariant.h"
#include "DataVariantAnyData.h"
#include "DesktopServices.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "View.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtGui/QCursor>

#include <vector>
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, CustomLayerPlugIn);

CustomLayerPlugIn::CustomLayerPlugIn() :
   mpLayer(NULL)
{
   setName("CustomLayerPlugIn");
   setProductionStatus(false);
   setDescriptorId("{F3C9B492-B594-41A0-AF9E-154ACCC4A3EE}");
   allowMultipleInstances(true);
   destroyAfterExecute(false);   // The module containing the plug-in that creates the custom draw object
                                 // must stay resident in memory or the draw object will become invalid
                                 // causing a crash when the custom layer attempts to call its draw method.
   setType("Sample");
   setAbortSupported(true);
   setWizardSupported(false);
   setMenuLocation("[Demo]/Custom Layer PlugIn");

   // Connections
   mpLayer.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &CustomLayerPlugIn::layerDeleted));
}

CustomLayerPlugIn::~CustomLayerPlugIn()
{}

bool CustomLayerPlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL));
   return true;
}

bool CustomLayerPlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool CustomLayerPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);

   // Create the layer
   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (pView == NULL)
   {
      Service<DesktopServices>()->showMessageBox("Custom Layer Plug-In Demo", "No spatial data view available "
         "for custom drawing.");
      return false;
   }

   mpLayer.reset(dynamic_cast<CustomLayer*>(pView->createLayer(CUSTOM_LAYER, NULL, "TestCustomLayer")));
   VERIFY(mpLayer.get() != NULL);

   // Turn on mouse processing and set a mouse cursor to distinguish the layer
   mpLayer->setAcceptsMouseEvents(true);
   mpLayer->setEditMouseCursor(QCursor(Qt::PointingHandCursor));

   // Set the custom layer as the active layer and set mouse mode "LayerMode"
   pView->setActiveLayer(mpLayer.get());
   pView->setMouseMode("LayerMode");

   // Set draw data into the layer
   setLayerDrawObject();

   // Display the instructions
   std::string instructions = 
      "This plug-in demonstrates drawing in a custom layer\n"
      "at the position of a left mouse click. When the custom\n"
      "layer is active and the Layer Edit mouse mode is enabled,\n"
      "the mouse cursor will be a pointing hand.\n"
      "Left click in the view to draw a large transparent\n"
      "red rectangle with black outline centered on the mouse position.\n"
      "Click on a different location in the custom layer and the\n"
      "rectangle will be moved to be centered at the new mouse position.";
   Service<DesktopServices>()->showMessageBox("Custom Layer Demonstration", instructions);

   return true;
}

bool CustomLayerPlugIn::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml("CustomLayerPlugIn");
   if (mpLayer.get() != NULL)
   {
      xml.pushAddPoint(xml.addElement("CustomLayer"));
      xml.addAttr("layerId", mpLayer->getId());

      // Save the element ID separately because the element may not yet
      // be restored into the layer when this object is deserialized
      Any* pElement = dynamic_cast<Any*>(mpLayer->getDataElement());
      if (pElement != NULL)
      {
         xml.addAttr("elementId", pElement->getId());
         const DataVariantAnyData* pAnyData = dynamic_cast<const DataVariantAnyData*>(pElement->getData());
         if (pAnyData != NULL)
         {
            DataVariant variant = pAnyData->getAttribute();
            if (variant.isValid())
            {
               std::string valuesStr = variant.toXmlString();
               xml.addAttr("dataType", variant.getTypeName());
               xml.addAttr("dataValues", valuesStr);
            }
         }
      }
      xml.popAddPoint();
   }

   return serializer.serialize(xml);
}

bool CustomLayerPlugIn::deserialize(SessionItemDeserializer& deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement* pRoot = deserializer.deserialize(reader, "CustomLayerPlugIn");
   if (pRoot == NULL)
   {
      return false;
   }

   for (DOMNode* pChild = pRoot->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("CustomLayer")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pChild);

         // Restore the layer
         std::string layerId = A(pElement->getAttribute(X("layerId")));

         CustomLayer* pLayer = dynamic_cast<CustomLayer*>(Service<SessionManager>()->getSessionItem(layerId));
         if (pLayer == NULL)
         {
            return false;
         }

         mpLayer.reset(pLayer);
         std::string elementId = A(pElement->getAttribute(X("elementId")));
         Any* pAny = dynamic_cast<Any*>(Service<SessionManager>()->getSessionItem(elementId));
         if (pAny == NULL)
         {
            return false;
         }
         std::string typeStr = A(pElement->getAttribute(X("dataType")));
         std::string valuesStr = A(pElement->getAttribute(X("dataValues")));
         if (typeStr.empty() == false && valuesStr.empty() == false)
         {
            DataVariant variant;
            variant.fromXmlString(typeStr, valuesStr);
            FactoryResource<DataVariantAnyData> pAnyData;
            pAnyData->setAttribute(variant);
            pAny->setData(pAnyData.release());
         }
      }
   }
   setLayerDrawObject();
   return true;
}

QWidget* CustomLayerPlugIn::getWidget() const
{
   // No widget to display
   return NULL;
}

void CustomLayerPlugIn::setLayerDrawObject()
{
   if (mpLayer.get() == NULL)
   {
      return;
   }

   // Create a new custom draw object
   CustomLayerDrawObject* pDrawObj = new CustomLayerDrawObject();
   VERIFYNRV(pDrawObj != NULL);

   // Add the custom draw object to the custom layer
   mpLayer->setDrawObject(pDrawObj);
}

void CustomLayerPlugIn::layerDeleted(Subject& subject, const std::string& signal, const boost::any& value)
{
   // Destroy the plug-in when the layer is deleted
   abort();
}
