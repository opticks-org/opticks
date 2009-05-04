/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CustomElementData.h"
#include "CustomElementPlugIn.h"
#include "ModelServices.h"
#include "PlugInRegistration.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "xmlreader.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, CustomElementPlugIn);

CustomElementPlugIn::CustomElementPlugIn()
{
   setName("Custom Data Element Plug-In");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Adds a custom element type to the data model that can be used in conjunction "
      "with an Any element.  This plug-in executes on application startup so that the module will "
      "remain loaded, which preserves the memory of all CustomElementData instances.");
   setShortDescription("Adds a custom element type to the data model.");
   setProductionStatus(false);
   setDescriptorId("{059117B1-01DB-4d58-BB64-10EA9853F9B8}");
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
}

CustomElementPlugIn::~CustomElementPlugIn()
{
}

bool CustomElementPlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool CustomElementPlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool CustomElementPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   // Add the custom data element type to the data model
   Service<ModelServices> pModel;
   pModel->addElementType("CustomElement");

   return true;
}

bool CustomElementPlugIn::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml("CustomElementPlugIn");
   vector<DataElement*> elements = Service<ModelServices>()->getElements("CustomElement");
   for (vector<DataElement*>::iterator element = elements.begin(); element != elements.end(); ++element)
   {
      CustomElementData* pData = model_cast<CustomElementData*>(*element);
      if (pData != NULL)
      {
         xml.pushAddPoint(xml.addElement("CustomElement"));
         xml.addAttr("dataElementId", (*element)->getId());
         xml.addAttr("value", pData->getValue());
         xml.popAddPoint();
      }
   }
   return serializer.serialize(xml);
}

bool CustomElementPlugIn::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement* pRoot = deserializer.deserialize(reader, "CustomElementPlugIn");
   if (pRoot == NULL)
   {
      return false;
   }
   for (DOMNode *pChld = pRoot->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("CustomElement")))
      {
         DOMElement* pElmnt = static_cast<DOMElement*>(pChld);
         string elementId = A(pElmnt->getAttribute(X("dataElementId")));
         Any* pElement = dynamic_cast<Any*>(Service<SessionManager>()->getSessionItem(elementId));
         if (pElement == NULL)
         {
            return false;
         }
         CustomElementData* pData = new CustomElementData(StringUtilities::fromXmlString<int>(
            A(pElmnt->getAttribute(X("value")))));
         pElement->setData(pData);
      }
   }
   return true;
}
