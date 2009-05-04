/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorType.h"
#include "AppVerify.h"
#include "Demo.h"
#include "DemoGuiImp.h"
#include "DemoAlgorithm.h"
#include "DesktopServices.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Resource.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, Demo);
/**
 *  Obligation from the base class. Indicates if the algorithm can
 *  be run in batch mode.
 *
 *  @return true
 */
bool Demo::canRunBatch() const
{
   return true;
}

/**
 *  Obligation from the base class. Indicates if the algorithm can
 *  be run in interactive mode.
 *
 *  @return true
 */
bool Demo::canRunInteractive() const
{
   return true;
}

/**
 *  Baseclass obligation. Creates the batch mode input arglist. Called
 *  by getInputSpecification. For the Demo plug-in, this is a super-set
 *  of the interactive mode arguments.
 *
 *  @param pArgList
 *       The arglist to put the args into. Guaranteed by the base class
 *       to be non-NULL.
 *  @return true if successfull, false otherwise
 */
bool Demo::populateBatchInputArgList(PlugInArgList *pArgList)
{
   static double black = 0.0;
   static double red = 1.0/3.0;
   static double yellow = 2.0/3.0;
   static double white = 1.0;

   VERIFY(populateInteractiveInputArgList(pArgList));
   
   VERIFY(pArgList->addArg<double>("Black", black));
   VERIFY(pArgList->addArg<double>("Red", black));
   VERIFY(pArgList->addArg<double>("Yellow", yellow));
   VERIFY(pArgList->addArg<double>("White", white));

   return true;
}

/**
 *  Baseclass obligation. Creates the interactive mode input arglist. Called
 *  by getInputSpecification.
 *
 *  @param pArgList
 *       The arglist to put the args into. Guaranteed by the base class
 *       to be non-NULL.
 *  @return true if successfull, false otherwise
 */
bool Demo::populateInteractiveInputArgList(PlugInArgList *pArgList)
{
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(ProgressArg()));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg()));

   return true;
}

/**
 *  Baseclass obligation. Creates the output arglist. Called
 *  by getOutputSpecification For the demo-plug-in, this method does nothing,
 *  since it has no output arguments.
 *
 *  @param pArgList
 *       The arglist to put the values into. Guaranteed by the base class
 *       to be non-NULL.
 *  @return true if successfull, false otherwise
 */
bool Demo::populateDefaultOutputArgList(PlugInArgList *pArgList)
{
   return true;
}

/**
 *  Baseclass obligation. Parses the input arglist on execution. Also creates
 *  and registers the algorithm object itself, if parsing of the arglist 
 *  succeeds.
 *
 *  @param pArgList
 *       The arglist containing the input args. Guaranteed by the base class
 *       to be non-NULL.
 *  @return true if successfull, false otherwise
 */
bool Demo::parseInputArgList(PlugInArgList *pArgList)
{
   bool success = true;

   VERIFY(pArgList != NULL);
   mpProgress = pArgList->getPlugInArgValue<Progress>(ProgressArg());

   RasterElement* pRasterElement = pArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   if (pRasterElement == NULL)
   {
      string msg = "Error Demo001: The raster element input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }

   // parse the batch-mode specific arguments
   if (isInteractive() == false)
   {
      double black;
      double red;
      double yellow;
      double white;

      success = success && pArgList->getPlugInArgValue<double>("Black", black);
      success = success && pArgList->getPlugInArgValue<double>("Red", red);
      success = success && pArgList->getPlugInArgValue<double>("Yellow", yellow);
      success = success && pArgList->getPlugInArgValue<double>("White", white);

      if (!success)
      {
         return false;
      }

      mInputs.mNodes.push_back(PlugInSamplerQt::Node(black, ColorType(0, 0, 0)));
      mInputs.mNodes.push_back(PlugInSamplerQt::Node(red, ColorType(0xff, 0, 0)));
      mInputs.mNodes.push_back(PlugInSamplerQt::Node(yellow, ColorType(0xff, 0xff, 0)));
      mInputs.mNodes.push_back(PlugInSamplerQt::Node(white, ColorType(0xff, 0xff, 0xff)));
   }

   // create and register the algorithm object
   if (success == true)
   {
      mpDemoAlg = new DemoAlgorithm(*pRasterElement, mpProgress, isInteractive());

      setAlgorithmPattern(Resource<AlgorithmPattern>(mpDemoAlg));
   }
   else
   {
      string msg = "Error Demo002: Not all input values could be extracted!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(msg, 0, ERRORS);
      }
   }

   return success;
}

/**
 *  Baseclass obligation. Populates the actual values into the output arglist 
 *  after execution. For the demo-plug-in, this method does nothing.
 *
 *  @param pArgList
 *       The arglist to put the values into. Guaranteed by the base class
 *       to be non-NULL.
 *  @return true if successfull, false otherwise
 */
bool Demo::setActualValuesInOutputArgList(PlugInArgList *pArgList)
{
   return true;
}

/**
 *  A baseclass obligation. Creates, registers and returns the GUI.
 *
 *  @param pAlgData
 *       Initialization data to pass to the GUI.
 *  @return a pointer to the algorithm's GUI, or NULL if it has none
 */
QDialog *Demo::getGui(void *pAlgData)
{
   if (mpDemoGui == NULL)
   {
      Service<DesktopServices> pDesktop;
      VERIFYRV(pDesktop.get() != NULL, NULL);
      mpDemoGui = new DemoGuiImp(pDesktop->getMainWidget(), this);
   }

   return mpDemoGui;
}

/**
 *  A baseclass obligation. Should call any sub-object abort methods as
 *  required.
 */
void Demo::propagateAbort()
{
}

/**
 *  A baseclass obligation. Extracts the user inputs from the GUI on OK
 *  or Apply.
 *  @return true if successful, or false otherwise
 */
bool Demo::extractFromGui()
{
   if (mpDemoGui != NULL)
   {
      mInputs.mNodes = mpDemoGui->getNodes();
      mInputs.mNumberOfCells = 256;
   }
   else
   {
      return false;
   }

   return true;
}

/**
 *  A baseclass obligation. Indicates if the plug-in supports abort.
 *  @return true if the algorithm can abort, false otherwise
 */
bool Demo::hasAbort()
{
   return false;
}

/**
 *  Constructor for the Demo plug-in. Initializes descriptive elements.
 */
Demo::Demo() :
   AlgorithmPlugIn(&mInputs),
   mpProgress(NULL),
   mpDemoAlg(NULL),
   mpDemoGui(NULL)
{
   setName("Demo");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setShortDescription("Demo");
   setDescription("Demo Plugin, showing a dynamic colormap");
   setMenuLocation("[Demo]\\Dynamic Colormap");
   setProductionStatus(false);
   setDescriptorId("{C94ED1C5-DC04-418b-84FC-E3BC2B3530C6}");
   allowMultipleInstances(true);
}

/**
 *  Destructor. In the case of the demo plug-in, this does nothing
 */
Demo::~Demo()
{
}

/**
 *  A baseclass obligation. Specifies if the algorithm inputs have changed
 *  since the algorithm was last run. This will always be true if the algorithm
 *  doesn't have an 'Apply' capability. If the algorithm has 'Apply', this will
 *  return an indication of whether the GUI inputs have changed since the last
 *  Apply.
 *
 *  @return true if the GUI inputs have changed since the last apply, 
 *     false otherwise
 */
bool Demo::needToRunAlgorithm()
{
   if (mpDemoGui != NULL)
   {
      return mpDemoGui->getModified();
   }

   return true;
}
