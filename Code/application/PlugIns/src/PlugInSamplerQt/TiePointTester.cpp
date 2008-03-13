/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "TiePointTester.h"
#include "TiePointTesterAlgorithm.h"
#include "PlugInArgList.h"

using namespace std;

/**
 *  Obligation from the base class. Indicates if the algorithm can
 *  be run in batch mode.
 *
 *  @return true
 */
bool TiePointTester::canRunBatch() const
{
   return true;
}

/**
 *  Obligation from the base class. Indicates if the algorithm can
 *  be run in interactive mode.
 *
 *  @return true
 */
bool TiePointTester::canRunInteractive() const
{
   return true;
}

/**
 *  Baseclass obligation. Creates the batch mode input arglist. Called
 *  by getInputSpecification. For the Tester plug-in, this is a super-set
 *  of the interactive mode arguments.
 *
 *  @param pArgList
 *       The arglist to put the args into. Guaranteed by the base class
 *       to be non-NULL.
 *  @return true if successfull, false otherwise
 */
bool TiePointTester::populateBatchInputArgList(PlugInArgList *pArgList)
{
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
bool TiePointTester::populateInteractiveInputArgList(PlugInArgList *pArgList)
{
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg(), NULL));

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
bool TiePointTester::populateDefaultOutputArgList(PlugInArgList *pArgList)
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
bool TiePointTester::parseInputArgList(PlugInArgList *pArgList)
{

   VERIFY(pArgList != NULL);
   mpProgress = pArgList->getPlugInArgValue<Progress>(ProgressArg());

   RasterElement *pRasterElement = pArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   if (pRasterElement == NULL)
   {
      std::string msg = "Error Tester001: The raster element input value is invalid!";
      if (mpProgress != NULL) mpProgress->updateProgress(msg, 0, ERRORS);
      return false;
   }

   // create and register the algorithm object
   mpTesterAlg = new TiePointTesterAlgorithm(*pRasterElement, mpProgress, isInteractive());

   setAlgorithmPattern(Resource<AlgorithmPattern>(mpTesterAlg));

   return true;
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
bool TiePointTester::setActualValuesInOutputArgList(PlugInArgList *pArgList)
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
QDialog *TiePointTester::getGui(void *pAlgData)
{
   return NULL;
}

/**
 *  A baseclass obligation. Should call any sub-object abort methods as
 *  required.
 */
void TiePointTester::propagateAbort()
{
}

/**
 *  A baseclass obligation. Extracts the user inputs from the GUI on OK
 *  or Apply.
 *  @return true if successful, or false otherwise
 */
bool TiePointTester::extractFromGui()
{
   return false;
}

/**
 *  A baseclass obligation. Indicates if the plug-in supports abort.
 *  @return true if the algorithm can abort, false otherwise
 */
bool TiePointTester::hasAbort()
{
   return false;
}

/**
 *  Constructor for the Tester plug-in. Initializes descriptive elements.
 */
TiePointTester::TiePointTester() :
   AlgorithmPlugIn(&mInputs),
   mpProgress(NULL),
   mpTesterAlg(NULL)
{
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(false);
   setName("Tie Point Tester");
   setShortDescription("Tie Point Tester");
   setDescription("Tie Point Tester Plugin");
   setMenuLocation("[Demo]\\Tie Point Tester");
   setDescriptorId("{4923A4BC-8819-40cd-8372-E28AC27D6416}");
}

/**
 *  Destructor. In the case of the demo plug-in, this does nothing
 */
TiePointTester::~TiePointTester()
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
bool TiePointTester::needToRunAlgorithm()
{
   return true;
}
