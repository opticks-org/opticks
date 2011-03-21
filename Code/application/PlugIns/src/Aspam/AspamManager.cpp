/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AnyData.h"
#include "AppVersion.h"
#include "AspamAdapter.h"
#include "AspamImporter.h"
#include "AspamManager.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "Filename.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksAspam, AspamManager);

AspamManager::AspamManager()
{
   setName("ASPAM Manager");
   setDescription("Singleton plug-in to manage the Aspam data type.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{5B16EAC1-2B5A-4bcd-BF94-25699CC682FF}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setWizardSupported(false);
}

AspamManager::~AspamManager()
{
   notify(SIGNAL_NAME(Subject, Deleted));
   vector<DataElement*> aspamElements = Service<ModelServices>()->getElements("Aspam");
   for (vector<DataElement*>::iterator element = aspamElements.begin(); element != aspamElements.end(); ++element)
   {
      Any* pAnyElement = dynamic_cast<Any*>(*element);
      if (model_cast<AspamAdapter*>(*element) != NULL)
      {
         pAnyElement->setData(NULL);
      }
   }
}

bool AspamManager::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = NULL;
   return true;
}

bool AspamManager::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool AspamManager::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Service<ModelServices> pModel;
   pModel->addElementType("Aspam");

   return true;
}

bool AspamManager::initializeAspam(Any* pAspamContainer)
{
   VERIFY(pAspamContainer != NULL);
   if (pAspamContainer->getData() != NULL)
   {
      // already initialized
      return false;
   }

   AspamAdapter* pData = new AspamAdapter;
   pAspamContainer->setData(pData);
   notify(SIGNAL_NAME(AspamManager, AspamInitialized), boost::any(pAspamContainer));
   return pAspamContainer->getData() != NULL;
}

const string& AspamManager::getObjectType() const
{
   static string sType("AspamManager");
   return sType;
}

bool AspamManager::isKindOf(const string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

bool AspamManager::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml("AspamManager");
   vector<DataElement*> aspamElements = Service<ModelServices>()->getElements("Aspam");
   for (vector<DataElement*>::iterator element = aspamElements.begin(); element != aspamElements.end(); ++element)
   {
      if (*element != NULL)
      {
         FileDescriptor* pFileDescriptor = (*element)->getDataDescriptor()->getFileDescriptor();
         if (pFileDescriptor != NULL)
         {
            xml.pushAddPoint(xml.addElement("Aspam"));
            xml.addAttr("filename", XmlBase::PathToURL(pFileDescriptor->getFilename().getFullPathAndName()));
            xml.addAttr("aspamId", (*element)->getId());
            xml.popAddPoint();
         }
      }
   }
   return serializer.serialize(xml);
}

bool AspamManager::deserialize(SessionItemDeserializer& deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement* pRoot = deserializer.deserialize(reader, "AspamManager");
   if (pRoot == NULL)
   {
      return false;
   }

   for (DOMNode* pChld = pRoot->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("Aspam")))
      {
         DOMElement* pElmnt = static_cast<DOMElement*>(pChld);
         string filename = A(pElmnt->getAttribute(X("filename")));
         AspamImporter* pImporter = new AspamImporter();
         if (pImporter->getImportDescriptors(filename).size() != 1)
         {
            return false;
         }
         PlugInArgList* pInputSpec = NULL;
         if (!pImporter->getInputSpecification(pInputSpec))
         {
            return false;
         }
         string aspamId = A(pElmnt->getAttribute(X("aspamId")));
         DataElement* pElement = dynamic_cast<DataElement*>(Service<SessionManager>()->getSessionItem(aspamId));
         pInputSpec->setPlugInArgValueLoose(Importer::ImportElementArg(), pElement);
         if (!pImporter->execute(pInputSpec, NULL))
         {
            return false;
         }
      }
   }

   return true;
}
