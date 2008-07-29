/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationController.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "CreateAnimation.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "SpatialDataView.h"

#include <string>
using namespace std;

CreateAnimation::CreateAnimation()
{
   setName("Create Animation");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Creates an animation which displays bands in a view");
   setDescriptorId("{48DA6AAB-307D-4328-9DB9-5DFFCA9B0D9B}");
   allowMultipleInstances(true);
}

CreateAnimation::~CreateAnimation()
{
}

bool CreateAnimation::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if(mbInteractive)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));

      pArgList->addArg<SpatialDataView>(ViewArg());
   }

   return true;
}

bool CreateAnimation::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if(mbInteractive)
   {
      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Set up list
      pArgList = pPlugInManager->getPlugInArgList();
      VERIFY(pArgList != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Animation");
      pArg->setType("Animation");
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Animation Controller");
      pArg->setType("AnimationController");
      pArgList->addArg(*pArg);
   }

   return true;
}

bool CreateAnimation::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "43C84DFB-6C5E-4DEF-BB94-5C35EB0EDB56");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if(!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "5E158F48-6089-4A88-ABD0-55C717BD13E2");
      return false;
   }
   SpatialDataView *pView = pInArgList->getPlugInArgValue<SpatialDataView>(ViewArg());
   if(pView == NULL)
   {
      reportError("No view provided.", "852F585B-D239-4C0A-B993-70EE68EC8DEE");
      return false;
   }

   Animation* pAnimation = pView->createDefaultAnimation();
   if (pAnimation == NULL)
   {
      reportError("Unable to create the default animation!", "96AFBB13-BEC0-446C-BFE2-492A005F39B2");
      return false;
   }

   // Set the output values
   if (pOutArgList != NULL)
   {
      // Animation
      if (pOutArgList->setPlugInArgValue("Animation", pAnimation) == false)
      {
         reportError("Could not set the animation output value!", "6A5CED56-AB03-456C-A085-5EE059C60CA3");
         return false;
      }

      // Animation controller
      AnimationController* pController = pView->getAnimationController();
      if (pController == NULL)
      {
         reportError("Unable to get the animation controller!", "B00BEF55-537E-47D4-BF5D-B84760E3C459");
         return false;
      }

      if (pOutArgList->setPlugInArgValue("Animation Controller", pController) == false)
      {
         reportError("Could not set the animation controller output value!", "3C53EDAE-DC70-4141-9759-ECD3EB9BE186");
         return false;
      }
   }

   reportComplete();
   pStep->finalize(Message::Success);
   return true;
}
