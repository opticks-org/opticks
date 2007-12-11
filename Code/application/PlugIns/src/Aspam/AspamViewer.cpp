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
#include "AspamManager.h"
#include "AspamViewer.h"
#include "AspamViewerDialog.h"
#include "AppAssert.h"
#include "DesktopServices.h"
#include "PlugInManagerServices.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "Slot.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

AspamViewer::AspamViewer() : mpMainWindow(NULL)
{
   setName("ASPAM Viewer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("View Atmospheric Slant Path Analysis Model (ASPAM) data files.");
   setMenuLocation("[ASPAM]/View &ASPAM");
   setDescriptorId("{B2AD86B5-3234-4f10-B60D-C3E320B026C2}");
   allowMultipleInstances(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

AspamViewer::~AspamViewer()
{
}

bool AspamViewer::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool AspamViewer::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool AspamViewer::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   try
   {
      mpMainWindow = new AspamViewerDialog(this);
      if(mpMainWindow == NULL)
      {
         throw AssertException("Unable to create ASPAM viewer dialog.");
      }

      mpMainWindow->show();

      vector<PlugIn*> instances = Service<PlugInManagerServices>()->getPlugInInstances("ASPAM Manager");
      AspamManager *pAspamManager = instances.empty() ? NULL : dynamic_cast<AspamManager*>(instances.front());
      if(pAspamManager != NULL)
      {
         pAspamManager->attach(SIGNAL_NAME(AspamManager, AspamInitialized), Slot(this, &AspamViewer::updateAspams));
      }
   }
   catch(AssertException &exc)
   {
      Service<DesktopServices> pDesktop;
      pDesktop->showMessageBox("ASPAM Error", string("A critical error occured: ") + exc.getText(), "Ok");
      return false;
   }

   return true;
}

QWidget* AspamViewer::getWidget() const
{
   return mpMainWindow;
}

bool AspamViewer::abort()
{
   try
   {
      vector<PlugIn*> instances = Service<PlugInManagerServices>()->getPlugInInstances("ASPAM Manager");
      AspamManager *pAspamManager = instances.empty() ? NULL : dynamic_cast<AspamManager*>(instances.front());
      if(pAspamManager != NULL)
      {
         pAspamManager->detach(SIGNAL_NAME(AspamManager, AspamInitialized),
               Slot(this, &AspamViewer::updateAspams));
      }
      return ViewerShell::abort();
   }
   catch (...)
   {
      return false;
   }
}

void AspamViewer::updateAspams(Subject &subject, const string &signal, const boost::any &data)
{
   if(dynamic_cast<AspamManager*>(&subject) != NULL)
   {
      Any *pElement(boost::any_cast<Any*>(data));
      Aspam *pAspam = model_cast<Aspam*>(pElement);
      if(pAspam != NULL)
      {
         pAspam->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &AspamViewer::updateAspams));
      }
   }
   mpMainWindow->populateAspamList();
}

bool AspamViewer::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter writer("AspamViewer");
   XML_ADD_POINT(writer, aspams)
   {
      vector<DataElement*> aspamElements = Service<ModelServices>()->getElements("Aspam");
      for (vector<DataElement*>::iterator ppElement=aspamElements.begin();
         ppElement!=aspamElements.end();
         ++ppElement)
      {
         Aspam *pAspam = model_cast<Aspam*>(*ppElement);
         if (pAspam != NULL)
         {
            XML_ADD_POINT(writer, aspam)
            {
               writer.addAttr("id", (*ppElement)->getId());
            }
         }
      }
   }
   return serializer.serialize(writer);
}

bool AspamViewer::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader (NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, "AspamViewer");
   if (pRootElement != NULL)
   {
      FOR_EACH_DOMNODE(pRootElement, pAspamNode)
      {
         if (XMLString::equals(pAspamNode->getNodeName(), X("aspam")))
         {
            DOMElement *pElement = dynamic_cast<DOMElement*>(pAspamNode);
            if (pElement)
            {
               string id = A(pElement->getAttribute(X("id")));
               SessionItem *pSessionItem = Service<SessionManager>()->getSessionItem(id);
               Subject *pSubject = dynamic_cast<Subject*>(pSessionItem);
               if (pSubject != NULL)
               {
                  pSubject->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &AspamViewer::updateAspams));
               }
            }
         }
      }
   }

   mpMainWindow = new AspamViewerDialog(this);
   if(mpMainWindow == NULL)
   {
      throw AssertException("Unable to create ASPAM viewer dialog.");
   }

   mpMainWindow->show();

   return true;
}
