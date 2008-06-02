/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#define _USRDLL

#include <QtGui/QDialog>
#include <QtGui/QMessageBox>

#include "AlgorithmPattern.h"
#include "AlgorithmDialog.h"
#include "ApplicationServices.h"
#include "BitMask.h"
#include "ColorType.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "Filename.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "PseudocolorLayer.h"
#include "RasterElement.h"
#include "Signature.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "ThresholdLayer.h"
#include "Undo.h"
#include "UtilityServices.h"

#include <vector>
using namespace std;

static const int CHUNK_SIZE = 200;

AlgorithmPlugIn::AlgorithmPlugIn(void *pAlgData) :
   mpAlgorithm(NULL),
   mInteractive(true),
   mpAlgorithmData(pAlgData)
{
   mpObjFact = mpApplicationServices->getObjectFactory();
   ENSURE(mpObjFact != NULL);
}

AlgorithmPlugIn::~AlgorithmPlugIn()
{
}

bool AlgorithmPlugIn::isInteractive() const
{
   return mInteractive;
}

void AlgorithmPlugIn::setAlgorithmPattern(Resource<AlgorithmPattern> pAlgorithm)
{
   mpAlgorithm = pAlgorithm;
}

bool AlgorithmPlugIn::setBatch()
{
   bool batchIsSupported = canRunBatch();
   mInteractive = false;

   return batchIsSupported;
}

bool AlgorithmPlugIn::setInteractive()
{
   bool interactiveIsSupported = canRunInteractive();
   mInteractive = true;

   return interactiveIsSupported;
}

bool AlgorithmPlugIn::getInputSpecification(PlugInArgList *&argList)
{
   argList = mpPlugInManagerServices->getPlugInArgList();
   VERIFY(argList != NULL);

   if (mInteractive)
   {
      return populateInteractiveInputArgList(argList);
   }

   return  populateBatchInputArgList(argList);
}

bool AlgorithmPlugIn::getOutputSpecification(PlugInArgList *&argList)
{
   argList = NULL;

   bool bModeSupported = mInteractive ? canRunInteractive() : canRunBatch();
   if (!bModeSupported)
   {
      return false;
   }

   argList = mpPlugInManagerServices->getPlugInArgList();
   VERIFY(argList != NULL);

   return populateDefaultOutputArgList(argList);
}

bool AlgorithmPlugIn::runDialog(QDialog *pDialog)
{
   if (pDialog == NULL)
   {
      return false;
   }

   int result = QDialog::Rejected;
   if (pDialog->isModal() == true)
   {
      result = pDialog->exec();
   }
   else
   {
      AlgorithmDialog* pAlgorithmDialog = dynamic_cast<AlgorithmDialog*>(pDialog);
      if (pAlgorithmDialog != NULL)
      {
         pAlgorithmDialog->show();
         result = pAlgorithmDialog->enterLoop();
      }
   }

   if (result == QDialog::Rejected)
   {
      abort();
   }

   return (result == QDialog::Accepted);
}

bool AlgorithmPlugIn::execute(PlugInArgList *inputArgList,
                              PlugInArgList *outputArgList)
{
   bool success = true;

   if (inputArgList == NULL)
   {
      success = false;
   }

   success = parseInputArgList(inputArgList);
   if (success && (mpAlgorithm.get() != NULL))
   {
      success = mpAlgorithm->runPreprocess();
   }

   QDialog* pDialog = NULL;
   if (success)
   {
      if (mInteractive)
      {
         try
         {
            pDialog = getGui(mpAlgorithmData);
         }
         catch (std::bad_alloc)
         {
            QMessageBox::critical(NULL, "Out of memory", 
               "Insufficient memory available to create the GUI.", "OK");
            return false;
         }

         if (pDialog != NULL)
         {
            success = runDialog(pDialog);
            if (success)
            {
               success = runAlgorithmFromGuiInputs();
            }
         }
         else
         {
            success = setupAndRunAlgorithm();
         }
      }
      else // mInteractive == false
      {
         success = setupAndRunAlgorithm();
      }
   }

   if (success)
   {
      if (mpAlgorithm.get() != NULL)
      {
         success = mpAlgorithm->runPostprocess();
      }
   }

   if (success)
   {
      if (outputArgList != NULL)
      {
         success = setActualValuesInOutputArgList(outputArgList);
      }
   }

   if (pDialog != NULL)
   {
      delete pDialog;
   }

   return success;
}

bool AlgorithmRunner::runAlgorithmFromGuiInputs()
{
   bool success = true;
   if (needToRunAlgorithm())
   {
      success = extractFromGui();
      if (success)
      {
         success = setupAndRunAlgorithm();
      }
   }

   return success;
}

bool AlgorithmPlugIn::setupAndRunAlgorithm()
{
   bool success = false;
   if (mpAlgorithm.get() != NULL)
   {
      success = mpAlgorithm->setGuiData(mpAlgorithmData);
      if (success)
      {
         success = mpAlgorithm->runProcess();
      }
   }

   return success;
}

bool AlgorithmPlugIn::abort()
{
   bool canAbort = AlgorithmShell::abort();
   if (canAbort)
   {
      propagateAbort();
      if ((mpAlgorithm.get() != NULL) && (mpAlgorithm->hasAbort()))
      {
         mpAlgorithm->abort();
      }
   }

   return canAbort;
}

AlgorithmReporter::AlgorithmReporter(Progress *pProgress) : 
   mpProgress(pProgress), mCurrentStage(0), mpStep(NULL)
{
   clearStages();
}

AlgorithmReporter::~AlgorithmReporter()
{
}

void AlgorithmReporter::reportProgress(ReportingLevel rptLevel, int progress, std::string message) const
{
   updateProgress(rptLevel, progress, message);
}

void AlgorithmReporter::updateProgress(ReportingLevel rptLevel, int progress, std::string message) const
{
   if (mpProgress != NULL)
   {
      string stageMessage = message;
      int stageProgress = progress;
      if (!mStages.empty() && (mCurrentStage < mStages.size()))
      {
         const Stage& current = mStages[mCurrentStage];
         stageMessage = current.mMessage + "\n>>> " + message;
         stageProgress = (100 * current.mWeightSum + progress * current.mWeight) / mWeightSum;
      }

      mpProgress->updateProgress(stageMessage, stageProgress, rptLevel);
   }

   if (mpStep != NULL)
   {
      switch (rptLevel)
      {
         case ABORT:
            mpStep->finalize(Message::Abort, message);
            break;
         case ERRORS:
            mpStep->finalize(Message::Failure, message);
            break;
         case WARNING:
         {
            Message *pMsg = mpStep->addMessage("Warning", "app", "BEFE67AB-8398-4976-9790-F2F6BC09CE36");
            if (pMsg != NULL)
            {
               pMsg->addProperty("Message", message);
            }
         }
         default:
            break;
      }
   }
}

Progress *AlgorithmReporter::getProgress() const
{
   return mpProgress;
}

int AlgorithmReporter::getProgressPercent() const
{
   int percent = 0;
   if (mpProgress != NULL)
   {
      string text;
      ReportingLevel gran;
      mpProgress->getProgress(text, percent, gran);
   }

   return percent;
}

void AlgorithmReporter::addStage(string stageMessage, int weight)
{
   int prevWeight = 0;
   if (!mStages.empty())
   {
      prevWeight = mStages.back().mWeightSum + mStages.back().mWeight;
   }

   Stage stage(stageMessage, weight, prevWeight);
   mStages.push_back(stage);

   mWeightSum = stage.mWeightSum + stage.mWeight;
}

void AlgorithmReporter::nextStage()
{
   if (mCurrentStage < mStages.size())
   {
      ++mCurrentStage;
   }
}

void AlgorithmReporter::clearStages()
{
   mStages.clear();
   mCurrentStage = 0;
}

AlgorithmReporter::Stage::Stage(std::string message, int weight, int weightSum) :
   mMessage(message),
   mWeight(weight),
   mWeightSum(weightSum)
{
}

AlgorithmReporter::Stage::Stage(const AlgorithmReporter::Stage& stage) :
   mMessage(stage.mMessage),
   mWeight(stage.mWeight),
   mWeightSum(stage.mWeightSum)
{
}

AlgorithmPattern::AlgorithmPattern(RasterElement* pRasterElement,
      Progress* pProgress, bool interactive,
      const BitMask* pRoi) :
   AlgorithmReporter(pProgress),
   mpRasterElement(pRasterElement),
   mpSubCube(NULL),
   mpRoi(pRoi),
   mpPixelsToProcess(NULL),
   mRowOffset(0),
   mInteractive(interactive)
{
   mpObjFact = mpApplicationServices->getObjectFactory();
   ENSURE(mpObjFact != NULL);

   if (mpRasterElement != NULL)
   {
      mpRasterElement->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AlgorithmPattern::elementDeleted));
   }
}

AlgorithmPattern::~AlgorithmPattern()
{
   if (mpRasterElement != NULL)
   {
      mpRasterElement->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AlgorithmPattern::elementDeleted));
   }

   if (mpPixelsToProcess != NULL)
   {
      mpObjFact->destroyObject(mpPixelsToProcess, "BitMask");
   }
}

int AlgorithmPattern::getRowOffset() const
{
   return mRowOffset;
}

bool AlgorithmPattern::runPreprocess()
{
   return preprocess();
}

bool AlgorithmPattern::runProcess()
{
   try
   {
      if (determinePixelsToProcess())
      {
         return processAll();
      }
   }
   catch (AssertException message)
   {
      reportProgress(ERRORS, 0, message.getText());
   }

   return false;
}

bool AlgorithmPattern::runPostprocess()
{
   return postprocess();
}

RasterElement *AlgorithmPattern::getRasterElement() const
{
   RasterElement* pRasterElement = NULL;

   if (mpSubCube == NULL)
   {
      pRasterElement = mpRasterElement;
   }
   else
   {
      pRasterElement = mpSubCube;
   }

   return pRasterElement;
}

int AlgorithmPattern::getSubCube(int startRow, int numRows)
{
   return 0;
}

void AlgorithmPattern::setRoi(const BitMask *pRoi)
{
   mpRoi = pRoi;
}

bool AlgorithmPattern::determinePixelsToProcess() const
{
   if (mpPixelsToProcess != NULL)
   {
      mpPixelsToProcess->clear();
   }
   else
   {
      mpPixelsToProcess = static_cast<BitMask*>(mpObjFact->createObject("BitMask"));
      if (mpPixelsToProcess == NULL)
      {
         return false;
      }
   }

   mpPixelsToProcess->invert();

   if (mpRoi != NULL)
   {
      mpPixelsToProcess->intersect(*mpRoi);
   }

   return true;
}

const BitMask *AlgorithmPattern::getPixelsToProcess() const
{
   if (mpPixelsToProcess == NULL)
   {
      determinePixelsToProcess();
   }

   return mpPixelsToProcess;
}

bool AlgorithmPattern::setGuiData(void *pAlgorithmData)
{
   if (pAlgorithmData == NULL)
   {
      return false;
   }

   return initialize(pAlgorithmData);
}

bool AlgorithmPattern::isInteractive() const
{
   return mInteractive;
}

bool AlgorithmPattern::hasAbort() const
{
   return canAbort();
}

bool AlgorithmPattern::abort()
{
   if (canAbort())
   {
      return doAbort();
   }

   return false;
}

void AlgorithmPattern::elementDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   if (&subject != mpRasterElement)
   {
      return;
   }

   mpRasterElement = NULL;
}

void AlgorithmPattern::displayThresholdResults(RasterElement *pRasterElement,
                                               ColorType color,
                                               PassArea passArea,
                                               double firstThreshold,
                                               double secondThreshold,
                                               Opticks::PixelOffset offset)
{
   REQUIRE(pRasterElement != NULL);

   SpatialDataView* pView = NULL;

   vector<Window*> windows;
   mpDesktopServices->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (unsigned int i = 0; i < windows.size() && pView == NULL; i++)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(windows[i]);
      if (pWindow != NULL)
      {
         SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
         if (pCurrentView != NULL)
         {
            LayerList *pLList = pCurrentView->getLayerList();
            REQUIRE(pLList != NULL);
            vector<Layer*> layers;
            pLList->getLayers(RASTER, layers);
            for (vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
            {
               if (*layer != NULL && static_cast<RasterElement*>((*layer)->getDataElement()) == getRasterElement())
               {
                  pView = pCurrentView;
                  break;
               }
            }
         }
      }
   }

   REQUIRE(pView != NULL);

   ThresholdLayer* pLayer = NULL;

   // Get or create a valid threshold layer
   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList != NULL)
   {
      pLayer = static_cast<ThresholdLayer*>(pLayerList->getLayer(THRESHOLD, pRasterElement));
      if (pLayer == NULL)
      {
         pLayer = static_cast<ThresholdLayer*>(pView->createLayer(THRESHOLD, pRasterElement));
      }

      // Remove existing layers of other types
      if (pLayer != NULL)
      {
         Layer* pRasterLayer = pLayerList->getLayer(RASTER, pRasterElement);
         if (pRasterLayer != NULL)
         {
            pView->deleteLayer(pRasterLayer);
         }

         Layer* pPseudocolorLayer = pLayerList->getLayer(PSEUDOCOLOR, pRasterElement);
         if (pPseudocolorLayer != NULL)
         {
            pView->deleteLayer(pPseudocolorLayer);
         }
      }
   }

   INVARIANT(pLayer != NULL);

   UndoLock lock(pView);

   if (color.isValid())
   {
      pLayer->setColor(color);
   }

   pLayer->setRegionUnits(RAW_VALUE);
   pLayer->setPassArea(passArea);
   pLayer->setFirstThreshold(firstThreshold);
   pLayer->setSecondThreshold(secondThreshold);
   pLayer->setXOffset(offset.mX);
   pLayer->setYOffset(offset.mY);
}

void AlgorithmPattern::displayPseudocolorResults(RasterElement *pRasterElement, std::vector<std::string>& sigNames,
                                                 Opticks::PixelOffset offset)
{
   REQUIRE(pRasterElement != NULL);

   SpatialDataView* pView = NULL;

   vector<Window*> windows;
   mpDesktopServices->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (unsigned int j = 0; j < windows.size() && pView == NULL; j++)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(windows[j]);
      if (pWindow != NULL)
      {
         SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
         if (pCurrentView != NULL)
         {
            LayerList *pLList = pCurrentView->getLayerList();
            REQUIRE(pLList != NULL);
            vector<Layer*> layers;
            pLList->getLayers(RASTER, layers);
            for (vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
            {
               if (*layer != NULL && static_cast<RasterElement*>((*layer)->getDataElement()) == getRasterElement())
               {
                  pView = pCurrentView;
                  break;
               }
            }
         }
      }
   }

   REQUIRE(pView != NULL);

   PseudocolorLayer* pLayer = NULL;

   // Get or create a valid pseudocolor layer
   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList != NULL)
   {
      pLayer =  static_cast<PseudocolorLayer*>(pLayerList->getLayer(PSEUDOCOLOR, pRasterElement));
      if (pLayer == NULL)
      {
         pLayer = static_cast<PseudocolorLayer*>(pView->createLayer(PSEUDOCOLOR, pRasterElement));
      }

      // Remove existing layers of other types
      if (pLayer != NULL)
      {
         Layer* pThresholdLayer = pLayerList->getLayer(THRESHOLD, pRasterElement);
         if (pThresholdLayer != NULL)
         {
            pView->deleteLayer(pThresholdLayer);
         }

         Layer* pRasterLayer = pLayerList->getLayer(RASTER, pRasterElement);
         if (pRasterLayer != NULL)
         {
            pView->deleteLayer(pRasterLayer);
         }
      }
   }

   INVARIANT(pLayer != NULL);

   UndoLock lock(pView);

   int iSignatureCount = sigNames.size();

   std::vector<ColorType> layerColors, excludeColors;
   excludeColors.push_back(ColorType (0,0,0));
   excludeColors.push_back(ColorType (255,255,255));
   // 1 for each sig + no sigs + multiple sigs
   ColorType::getUniqueColors(iSignatureCount + 2, layerColors, excludeColors);

   pLayer->clear();

   for (int i = 0; i < iSignatureCount; i++)
   {
      pLayer->addInitializedClass(sigNames[i], i + 1, layerColors[i], true);
   }

   pLayer->addInitializedClass(std::string("Indeterminate"), -1, layerColors[iSignatureCount], true);
   pLayer->addInitializedClass(std::string("No match"), 0, layerColors[iSignatureCount + 1], false);
   pLayer->setXOffset(offset.mX);
   pLayer->setYOffset(offset.mY);
}
