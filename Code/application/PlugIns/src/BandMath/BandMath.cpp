/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <limits.h>
#include <assert.h>
#include <float.h>

#include "AppVersion.h"
#include "BandMath.h"
#include "bm.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TypesFile.h"
#include "Undo.h"

#include <boost/bind.hpp>

REGISTER_PLUGIN_BASIC(OpticksBandMath, BandMath);

using namespace std;

BandMath::BandMath() :
   mpStep(NULL),
   mbInteractive(true),
   mbDisplayResults(false),
   mbCubeMath(false),
   mpCube(NULL)
{
   // set description values
   setName("Band Math");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace and Technologies Corporation");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Perform Math on a Image Cube");
   setDescription("Perform Math on a Image Cube");
   setMenuLocation("[General Algorithms]\\Band Math");
   setDescriptorId("{A34C32D2-98AD-4b9a-A337-C47C8ECA31E2}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);

   // Initialize private variables
   initialize();
}

BandMath::~BandMath()
{ 
   if ((mpResultData != NULL) && mbError)
   {
      mpDataModel->destroyElement(mpResultData);
   }
}

bool BandMath::isInputValid(PlugInArgList& pArgList)
{
   PlugInArg* pArg = NULL;
   bool bUseFullDataset = true;

   // The following must always have values set for them
   // Sensor data
   if (!pArgList.getArg(DataElementArg(), pArg) || (pArg == NULL) || !pArg->isActualSet())
   {
      mpStep->finalize(Message::Failure, "Sensor data input argument not present.");
      meGabbiness = ERRORS;
      return false;
   }

   if (!mbInteractive)
   {
      // Sensor data
      if (!pArgList.getArg("Cube 2", pArg) || (pArg == NULL) || !pArg->isActualSet())
      {
         mpStep->finalize(Message::Failure, "Sensor data input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }

      // Sensor data
      if (!pArgList.getArg("Cube 3", pArg) || (pArg == NULL) || !pArg->isActualSet())
      {
         mpStep->finalize(Message::Failure, "Sensor data input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }

      // Sensor data
      if (!pArgList.getArg("Cube 4", pArg) || (pArg == NULL))
      {
         mpStep->finalize(Message::Failure, "Sensor data input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }

      // Sensor data
      if (!pArgList.getArg("Cube 5", pArg) || (pArg == NULL))
      {
         mpStep->finalize(Message::Failure, "Sensor data input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }

      // Test 
      if (pArgList.getArg("Input Expression", pArg) || (pArg == NULL) || !pArg->isActualSet())
      {
         mpStep->finalize(Message::Failure, "Expression input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }

      // Test Display Results
      if (pArgList.getArg("Display Results", pArg) || (pArg == NULL) || !pArg->isActualSet())
      {
         mpStep->finalize(Message::Failure, "Display Results input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }

      // Test Degrees
      if (pArgList.getArg("Degrees", pArg) || (pArg == NULL) || !pArg->isActualSet())
      {
         mpStep->finalize(Message::Failure, "Degrees input argument not present.");
         meGabbiness = ERRORS;
         return false;
      }
   }

   return true;
}

bool BandMath::setBatch()
{
   mbInteractive = false;
   return true;
}

bool BandMath::setInteractive()
{
   mbInteractive = true;
   return true;
}

bool BandMath::getInputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPluginManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPluginManager->getPlugInArg();  //progress
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPluginManager->getPlugInArg();     //cube
   VERIFY(pArg != NULL);
   pArg->setName(DataElementArg());
   pArg->setType("RasterElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   if (!mbInteractive)
   {
      pArg = mpPluginManager->getPlugInArg();    //cube2
      VERIFY(pArg != NULL);
      pArg->setName("Raster Element 2");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = mpPluginManager->getPlugInArg();     //cube3
      VERIFY(pArg != NULL);
      pArg->setName("Raster Element 3");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = mpPluginManager->getPlugInArg();     //cube4
      VERIFY(pArg != NULL);
      pArg->setName("Raster Element 4");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = mpPluginManager->getPlugInArg();     //cube5
      VERIFY(pArg != NULL);
      pArg->setName("Raster Element 5");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = mpPluginManager->getPlugInArg();    // Input Expression
      VERIFY(pArg != NULL);
      pArg->setName("Input Expression");
      pArg->setType("string");
      pArg->setDefaultValue(&mExpression);
      pArgList->addArg(*pArg);

      bool temp = true;
      pArg = mpPluginManager->getPlugInArg();    // Display Results
      VERIFY(pArg != NULL);
      pArg->setName("Display Results");
      pArg->setType("bool");
      pArg->setDefaultValue(&temp);
      pArgList->addArg(*pArg);

      temp = false;
      pArg = mpPluginManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Degrees");
      pArg->setType("bool");
      pArg->setDefaultValue(&temp);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool BandMath::getOutputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPluginManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPluginManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Band Math Result");
   pArg->setType("RasterElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool BandMath::initialize()
{
   // initialize private BandMath values
   mpProgress = NULL;
   mResultsName = "Blank RESULT";
   mExpression = " ";  // prevent WizardNode from considering the empty string
                       // a failure
   meGabbiness = NORMAL;
   mbError = false;

   mpResultData = NULL;

   mpCube = NULL;
   mpCube2 = NULL;
   mpCube3 = NULL;
   mpCube4 = NULL;
   mpCube5 = NULL;
   mCubeRows = 0;
   mCubeColumns = 0;
   mCubeBands = 0;

   mbGuiIsNeeded = false;
   mbDegrees = false;
   mbAsLayerOnExistingView = false;

   // Other necessary things
   mstrProgressString = "";

   return true;
}

bool BandMath::execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList)
{
   StepResource pStep("Start BandMath", "app", "02E18066-1355-4a5f-ABC5-0366D9890C1C");
   mpStep = pStep.get();

   // Initilize variables
   initialize();

   // Get pointers
   // parse arg in list
   if (!parse(pInputArgList, pOutputArgList))
   {
      meGabbiness = ERRORS;
      displayErrorMessage();
      return false;
   }

   // Get important stuff
   pStep->addProperty("Dataset", mpCube->getFilename());

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpCube->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      mCubeRows = pDescriptor->getRowCount();
      mCubeColumns = pDescriptor->getColumnCount();
      mCubeBands = pDescriptor->getBandCount();
   }

   std::string tmpResultName;

   vector<DataElement*> elements = mpDataModel->getElements("RasterElement");

   vector<DataElement*>::iterator iter;
   for (iter = elements.begin(); iter != elements.end(); ++iter)
   {
      RasterElement* pRaster = static_cast<RasterElement*>(*iter);
      if (pRaster != NULL)
      {
         const RasterDataDescriptor* pCurrentDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
         if (pCurrentDescriptor != NULL)
         {
            if ((mCubeRows == pCurrentDescriptor->getRowCount()) &&
               (mCubeColumns == pCurrentDescriptor->getColumnCount()) &&
               (mCubeBands == pCurrentDescriptor->getBandCount()))
            {
               mCubesList.push_back(pRaster);
            }
         }
      }
   }

   char errorVal[80];
   int errorCode = -1;  

   if (mbInteractive)
   {
      QWidget* pParent = mpDesktop->getMainWidget();

      FrmBM frmASIT(pDescriptor, mCubesList, pParent);

      if (frmASIT.exec() == QDialog::Rejected)
      {
         pStep->finalize(Message::Abort);
         return false;
      }

      mExpression = frmASIT.getExpression(true).toStdString();
      mbDegrees = frmASIT.isDegrees();
      mbCubeMath = frmASIT.isMultiCube();
      mbAsLayerOnExistingView = frmASIT.isResultsMatrix();
   }
   else
   {
      mbCubeMath = false;

      //check for cube math
      unsigned int pos = mExpression.find_first_of(string("cC"));
      if ((pos >= 0) && (pos < (mExpression.length() - 1)) && ((mExpression[pos + 1] > '0') &&
                                                              (mExpression[pos + 1] < '9')))
      {
         mbCubeMath = true;
      }
   }
   if (!createReturnValue(mExpression))
   {
      mstrProgressString = "Could not allocate space for results.";
      meGabbiness = ERRORS;
      displayErrorMessage();
      return false;
   }
   {
      StepResource pResultStep("Compute result", "app", "CDCC12AC-32DD-4831-BC6B-225538C92053");
      mpStep = pResultStep.get();
      pResultStep->addProperty("Expression", mExpression);

      FactoryResource<DataRequest> pReturnRequest;
      pReturnRequest->setInterleaveFormat(BIP);
      pReturnRequest->setWritable(true);
      DataAccessor returnDa = mpResultData->getDataAccessor(pReturnRequest.release());
      if (!returnDa.isValid())
      {
         mstrProgressString = "Could not access the result data.";
         meGabbiness = ERRORS;
         displayErrorMessage();
         return false;
      }

      if (!mbCubeMath)
      {
         FactoryResource<DataRequest> pCubeRequest;
         pCubeRequest->setInterleaveFormat(BIP);
         DataAccessor cubeDa = mpCube->getDataAccessor(pCubeRequest.release());
         if (!cubeDa.isValid())
         {
            mstrProgressString = "Reading this cube format is not supported.";
            meGabbiness = ERRORS;
            displayErrorMessage();
            return false;
         }

         vector<DataAccessor> accessors(1, cubeDa);
         vector<EncodingType> types(1, pDescriptor->getDataType());

         char* mutableExpression = new char[mExpression.size() + 1];
         strcpy(mutableExpression, mExpression.c_str());

         errorCode = eval(mpProgress, accessors, types, mCubeRows, mCubeColumns,
            mCubeBands, mutableExpression, returnDa, mbDegrees, errorVal, mbCubeMath, mbInteractive);

         delete [] mutableExpression;
      }
      else // cube math
      {
         EncodingType type = pDescriptor->getDataType();

         vector<DataAccessor> accessors;
         vector<EncodingType> dataTypes;
         for (unsigned int i = 0; i < mCubesList.size(); ++i)
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setInterleaveFormat(BIP);
            DataAccessor daCube = mCubesList[i]->getDataAccessor(pRequest.release());
            accessors.push_back(daCube);
            const RasterDataDescriptor* pDdCube = dynamic_cast<RasterDataDescriptor*>(mCubesList.at(i)->
               getDataDescriptor());
            if (pDdCube != NULL)
            {
               dataTypes.push_back(pDdCube->getDataType());
            }
            else
            {
               mstrProgressString = "Could not get data description for cube.";
               meGabbiness = ERRORS;
               displayErrorMessage();
               return false;
            }
         }

         char* mutableExpression = new char[mExpression.size() + 1];
         strcpy(mutableExpression, mExpression.c_str());

         errorCode = eval(mpProgress, accessors, dataTypes, mCubeRows,
            mCubeColumns, mCubeBands, mutableExpression, returnDa,
            mbDegrees, errorVal, mbCubeMath, mbInteractive);

         delete [] mutableExpression;
      }

      if (errorCode != 0)
      {
         mbError = true;
         if (errorCode == -1)
         {
            mstrProgressString = errorVal;
            meGabbiness = ERRORS;
         }
         else if (errorCode == -2)
         {
            mstrProgressString = "BandMath was cancelled due to an error in the input expression.";
            meGabbiness = ABORT;
         }
         else
         {
            mstrProgressString = "Unknown error has occured while executing BandMath.";
            meGabbiness = ERRORS;
         }
         displayErrorMessage();
         return false;
      }

      pResultStep->finalize(Message::Success);
   }
   mpStep = pStep.get();

   if (!createReturnGuiElement())
   {
      mstrProgressString = "Could not create GUI element.";
      meGabbiness = ERRORS;
      displayErrorMessage();
      return false;
   }     

   // Fill output arg list
   if (pOutputArgList != NULL)
   {
      PlugInArg* pArg = NULL;
      pOutputArgList->getArg("Band Math Result", pArg);

      VERIFY(pArg != NULL);
      pArg->setActualValue(mpResultData);
   }

   mpResultData->updateData();

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Algorithm completed successfully", 100, meGabbiness);
   }

   pStep->finalize(Message::Success);
   mpStep = NULL;
   return true;
}

bool BandMath::parse(PlugInArgList* pArgInList, PlugInArgList* pArgOutList)
{
   PlugInArg* pArg = NULL;

   //get the progress pointer---------------------------------------------------
   if (!pArgInList->getArg(ProgressArg(), pArg) || (pArg == NULL))
   {
      mstrProgressString = "Progress input argument not present.";
      return false;
   }
   mpProgress = pArg->getPlugInArgValue<Progress>();

   //get the pointer to the cube data------------------------------------------
   if (!pArgInList->getArg(DataElementArg(), pArg) || (pArg == NULL))
   {
      mstrProgressString = "Raster Element input argument not present.";
      return false;
   }
   mpCube = pArg->getPlugInArgValue<RasterElement>();

   if (mpCube == NULL)
   {
      mstrProgressString = "Sensor data input argument not set.";
      return false;
   }

   EncodingType dataType;

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpCube->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      dataType = pDescriptor->getDataType();
   }

   if ((dataType == INT4SCOMPLEX) || (dataType == FLT8COMPLEX))
   {
      mstrProgressString = "Complex data is not supported!";
      return false;
   }

   PlugInArg* paExpressionArg = NULL;
   PlugInArg* paResultsNameArg = NULL;

   bool bAllParametersProvided = (pArgInList->getArg("Input Expression", paExpressionArg) &&
      pArgInList->getArg("Results Name", paResultsNameArg));
   mbGuiIsNeeded = !bAllParametersProvided;

   if (!mbInteractive || bAllParametersProvided)
   {
      //get the pointer to the cube data------------------------------------------
      if (!pArgInList->getArg("Raster Element 2", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Raster Element input argument not present.";
         return false;
      }
      mpCube2 = pArg->getPlugInArgValue<RasterElement>();

      //get the pointer to the cube data------------------------------------------
      if (!pArgInList->getArg("Raster Element 3", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Raster Element input argument not present.";
         return false;
      }
      mpCube3 = pArg->getPlugInArgValue<RasterElement>();

      //get the pointer to the cube data------------------------------------------
      if (!pArgInList->getArg("Raster Element 4", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Raster Element input argument not present.";
         return false;
      }
      mpCube4 = pArg->getPlugInArgValue<RasterElement>();

      //get the pointer to the cube data------------------------------------------
      if (!pArgInList->getArg("Raster Element 5", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Raster Element input argument not present.";
         return false;
      }
      mpCube5 = pArg->getPlugInArgValue<RasterElement>();

      //get the Expression --------------------------------------------------------
      if (!pArgInList->getArg( "Input Expression", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Input Expression argument not present.";
         return false;
      }
      {
         string* pTmp = pArg->getPlugInArgValue<string>();
         if (pTmp != NULL)
         {
            mExpression = *pTmp;
         }
         else
         {
            mstrProgressString = "Input Expression argument not set.";
            return false;
         }
      }

      //get the Display Results Value--------------------------------------------------------
      if (!pArgInList->getArg("Display Results", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Display Results input argument not present.";
         return false;
      }
      {
         bool* pTmp = pArg->getPlugInArgValue<bool>();
         if (pTmp != NULL)
         {
            mbDisplayResults = *pTmp;
         }
         else
         {
            mstrProgressString = "Display Results input argument not set.";
            return false;
         }
      }

      //   get the Degrees Value--------------------------------------------------------
      if (!pArgInList->getArg("Degrees", pArg) || (pArg == NULL))
      {
         mstrProgressString = "Degrees input argument not present.";
         return false;
      }
      {
         bool* pTmp = pArg->getPlugInArgValue<bool>();
         if (pTmp != NULL)
         {
            mbDegrees = *pTmp;
         }
         else
         {
            mstrProgressString = "Degrees input argument not set.";
            return false;
         }
      }
   }

   return true;
}

void BandMath::displayErrorMessage()
{
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mstrProgressString, 0, meGabbiness);
      if (meGabbiness == ABORT)
      {
         mpStep->finalize(Message::Abort, mstrProgressString);
      }
      else if (meGabbiness == ERRORS)
      {
         mpStep->finalize(Message::Failure, mstrProgressString);
      }
   }
}

bool BandMath::createReturnValue(string partialResultsName)
{
   // Set the short and long result names
   FactoryResource<Filename> pFilename;
   string shortResultsName;
   string longResultsName;
   if (pFilename.get() != NULL)
   {
      pFilename->setFullPathAndName(mpCube->getFilename());
      shortResultsName = pFilename->getTitle() + " = " + partialResultsName;
      longResultsName = pFilename->getPath() + "/" + pFilename->getTitle() + " = " + partialResultsName;
   }
   mResultsName = longResultsName;

   const RasterDataDescriptor* pOrigDescriptor = dynamic_cast<RasterDataDescriptor*>(mpCube->getDataDescriptor());
   const vector<DimensionDescriptor>& origRows = pOrigDescriptor->getRows();
   const vector<DimensionDescriptor>& origColumns = pOrigDescriptor->getColumns();

   mpResultData = NULL;

   unsigned int bandCount = mCubeBands;
   if (mbCubeMath == false)
   {
      bandCount = 1;
   }

   RasterElement* pParent = NULL;
   if (mbAsLayerOnExistingView)
   {
      pParent = mpCube;
   }
   RasterElement* pRaster = RasterUtilities::createRasterElement(mResultsName, origRows.size(),
      origColumns.size(), bandCount, FLT4BYTES, BIP, pOrigDescriptor->getProcessingLocation() == IN_MEMORY, pParent);

   if (pRaster == NULL)
   {
      mstrProgressString = "Error creating result raster element";
      meGabbiness = ERRORS;
      displayErrorMessage();
      mbError = true;
      return false;
   }

   if (!mbAsLayerOnExistingView)
   {
      // need to copy classification since parent was NULL in call to createRasterElement
      pRaster->copyClassification(mpCube);

      // make copies of existing GcpLists only if going into a new view
      vector<DataElement*> gcps = mpDataModel->getElements(mpCube, "GcpList");
      if (!gcps.empty())
      {
         vector<DataElement*>::iterator iter;
         for (iter = gcps.begin(); iter != gcps.end(); ++iter)
         {
            GcpList* theGcp = dynamic_cast<GcpList*>(*iter);
            theGcp->copy(theGcp->getName(), pRaster);
         }
      }
   }

   mpResultData = pRaster;

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>
      (mpResultData->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      // Rows
      vector<DimensionDescriptor> rows = pDescriptor->getRows();
      for (unsigned int i = 0; i < origRows.size(); ++i)
      {
         // Original number
         DimensionDescriptor origRow = origRows[i];
         if (origRow.isOriginalNumberValid() == true)
         {
            rows[i].setOriginalNumber(origRow.getOriginalNumber());
         }
      }
      pDescriptor->setRows(rows);

      // Columns
      vector<DimensionDescriptor> columns = pDescriptor->getColumns();
      for (unsigned int i = 0; i < origColumns.size(); ++i)
      {
         // Original number
         DimensionDescriptor origColumn = origColumns[i];
         if (origColumn.isOriginalNumberValid() == true)
         {
            columns[i].setOriginalNumber(origColumn.getOriginalNumber());
         }
      }
      pDescriptor->setColumns(columns);
   }

   return true;
}

bool BandMath::createReturnGuiElement()
{
   bool bSuccess = false;

   if (mbInteractive || mbDisplayResults) 
   {
      SpatialDataWindow* pWindow = NULL;

      if (mbAsLayerOnExistingView) 
      {
         // Create the spectral data window
         // Attach results matrix to data set view
         pWindow = static_cast<SpatialDataWindow*>(mpDesktop->getWindow(
                                                            mpCube->getName(),
                                                            SPATIAL_DATA_WINDOW));
      }
      else
      {
         // Create the spectral data window
         pWindow = static_cast<SpatialDataWindow*>(mpDesktop->createWindow(
                                                                           mResultsName.c_str(),
                                                                           SPATIAL_DATA_WINDOW));
      }

      VERIFYRV(pWindow != NULL, NULL);
      
      SpatialDataView* pView = pWindow->getSpatialDataView();
      VERIFYRV(pView != NULL, NULL);

      UndoLock lock(pView);

      if (!mbAsLayerOnExistingView)
      {
         pView->setPrimaryRasterElement(mpResultData);
      }

      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL)
      {
         Layer* pLayer = pLayerList->getLayer(RASTER, mpResultData);
         if (pLayer == NULL)
         {
            if (pView->createLayer(RASTER, mpResultData) != NULL)
            {
               bSuccess = true;
            }
            if (!mbAsLayerOnExistingView)
            {
               Service<ModelServices> pModel;
               vector<DataElement*> elements = pModel->getElements(mpResultData, "GcpList");
               for_each(elements.begin(), elements.end(), 
                  boost::bind(&SpatialDataView::createLayer, pView, GCP_LAYER, _1));
            }
         }
      }
   }
   else // no GUI required, method has successfully noop'd
   {
      bSuccess = true;
   }
   return bSuccess;
}
