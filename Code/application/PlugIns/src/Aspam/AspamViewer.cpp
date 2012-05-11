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
#include "AspamViewer.h"
#include "AspamViewerDialog.h"
#include "AppAssert.h"
#include "DesktopServices.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "Slot.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksAspam, AspamViewer);

AspamViewer::AspamViewer() : mpMainWindow(NULL), mbSessionClosing(false)
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
   setWizardSupported(false);

   mpMgrAttachment.addSignal(SIGNAL_NAME(AspamManager, AspamInitialized), 
      Slot(this, &AspamViewer::updateAspams));
   mpModelAttachment.reset(mpModelServices.get());
   mpModelAttachment.addSignal(SIGNAL_NAME(ModelServices, ElementDestroyed),
      Slot(this, &AspamViewer::updateAspams));
   Service<ApplicationServices> pServices;
   mpAppSrvcsAttachment.reset(pServices.get());
   mpAppSrvcsAttachment.addSignal(SIGNAL_NAME(ApplicationServices, SessionClosed),
      Slot(this, &AspamViewer::sessionClosing));
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

bool AspamViewer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   try
   {
      mpMainWindow = new AspamViewerDialog(this);
      if (mpMainWindow == NULL)
      {
         throw AssertException("Unable to create ASPAM viewer dialog.");
      }

      mpMainWindow->show();

      vector<PlugIn*> instances = Service<PlugInManagerServices>()->getPlugInInstances("ASPAM Manager");
      AspamManager* pAspamManager = instances.empty() ? NULL : dynamic_cast<AspamManager*>(instances.front());
      if (pAspamManager != NULL)
      {
         mpMgrAttachment.reset(pAspamManager);
      }
   }
   catch (AssertException& exc)
   {
      Service<DesktopServices> pDesktop;
      pDesktop->showMessageBox("ASPAM Error", string("A critical error occurred: ") + exc.getText(), "Ok");
      return false;
   }

   return true;
}

QWidget* AspamViewer::getWidget() const
{
   return mpMainWindow;
}

void AspamViewer::updateAspams(Subject& subject, const string& signal, const boost::any& data)
{
   if (mbSessionClosing)
   {
      return;
   }

   bool bRefreshNeeded(false);
   if (dynamic_cast<Aspam*>(&subject) != NULL)
   {
      bRefreshNeeded = true;
   }
   else if (dynamic_cast<AspamManager*>(&subject) != NULL)
   {
      Any* pElement(boost::any_cast<Any*>(data));
      Aspam* pAspam = model_cast<Aspam*>(pElement);
      if (pAspam != NULL)
      {
         pAspam->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &AspamViewer::updateAspams));
      }
   }
   else if (signal == "ModelServices::ElementDestroyed")
   {
      DataElement* pElement(boost::any_cast<DataElement*>(data));
      if (pElement->getType() == "Aspam")
      {
         bRefreshNeeded = true;
      }
   }

   if (bRefreshNeeded)
   {
      mpMainWindow->populateAspamList();
   }
}

bool AspamViewer::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter writer("AspamViewer");
   const AspamManager* pMgr = mpMgrAttachment.get();
   if (pMgr != NULL)
   {
      writer.addAttr("managerId", pMgr->getId());
   }
   return serializer.serialize(writer);
}

bool AspamViewer::deserialize(SessionItemDeserializer& deserializer)
{
   XmlReader reader (NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, "AspamViewer");
   if (pRootElement != NULL)
   {
      string mgrId = A(pRootElement->getAttribute(X("managerId")));
      SessionItem* pSessionItem = Service<SessionManager>()->getSessionItem(mgrId);
      AspamManager* pMgr = dynamic_cast<AspamManager*>(pSessionItem);
      if (pMgr != NULL)
      {
         mpMgrAttachment.reset(pMgr);
      }
      else
      {
         vector<PlugIn*> instances = Service<PlugInManagerServices>()->getPlugInInstances("ASPAM Manager");
         AspamManager* pAspamManager = instances.empty() ? NULL : dynamic_cast<AspamManager*>(instances.front());
         if (pAspamManager != NULL)
         {
            mpMgrAttachment.reset(pAspamManager);
         }
      }
   }

   mpMainWindow = new AspamViewerDialog(this);
   if (mpMainWindow == NULL)
   {
      throw AssertException("Unable to create ASPAM viewer dialog.");
   }

   mpMainWindow->show();

   return true;
}

void AspamViewer::sessionClosing(Subject& pSubject, const string& signal, const boost::any& data)
{
   mbSessionClosing = true;
}
