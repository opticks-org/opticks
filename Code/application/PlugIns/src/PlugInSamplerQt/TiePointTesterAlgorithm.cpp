/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <time.h>

#include "TiePointTesterAlgorithm.h"
#include "AppAssert.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TiePointList.h"
#include "TiePointLayer.h"
#include "WorkspaceWindow.h"

/**
 *  A baseclass obligation. Performs any necessary preprocessing steps
 *  prior to the display of the GUI. For the Tester plug-in, this does nothing.
 *
 *  @return true on success, false otherwise.
 */
bool TiePointTesterAlgorithm::preprocess()
{
   return true;
}

/**
 *  A baseclass obligation. Runs the algorithm, based on the inputs in mInputs,
 *  which has been populated from the GUI or the input arglist.
 *
 *  @return true on success, false otherwise.
 *
 *  @see Tester::parseArgList
 *  @see Tester::extractFromGui
 */
bool TiePointTesterAlgorithm::processAll()
{
   REQUIRE(getRasterElement() != NULL);

   StepResource pStep("Run Tie Point Tester", "app", "AFB5C331-DD5E-4d1b-9A8C-4D3206E586FE");
   pStep->addProperty("Cube", getRasterElement()->getName());

   RasterElement* pCube[2] = { NULL, NULL };
   if (findCubes(pCube) != 2)
   {
      return false;
   }

   if (pCube[0] == NULL || pCube[1] == NULL)
   {
      return false;
   }

   const RasterDataDescriptor* pDescriptor0 = dynamic_cast<const RasterDataDescriptor*>(pCube[0]->getDataDescriptor());
   const RasterDataDescriptor* pDescriptor1 = dynamic_cast<const RasterDataDescriptor*>(pCube[1]->getDataDescriptor());

   size_t xSize = std::min(pDescriptor0->getColumnCount(), pDescriptor1->getColumnCount());
   size_t ySize = std::min(pDescriptor0->getRowCount(), pDescriptor1->getRowCount());

   srand(static_cast<unsigned int>(time(NULL)));

   std::vector<TiePoint> points;
   computeTiePoints(xSize, ySize, points);

   Service<ModelServices> pModel;
   TiePointList* pList = static_cast<TiePointList*>(pModel->getElement("TiePointList", "TiePointList", pCube[0]));
   if (pList == NULL)
   {
      DataDescriptor* pTiePointDescriptor = pModel->createDataDescriptor("TiePointList", "TiePointList", pCube[0]);
      if (pTiePointDescriptor != NULL)
      {
         pList = static_cast<TiePointList*>(pModel->createElement(pTiePointDescriptor));
      }
   }
   INVARIANT(pList != NULL);
   pList->adoptTiePoints(points);

   addLayer(pCube[0], pList, false);
   addLayer(pCube[1], pList, true);

   pList->setMissionDatasetName(pCube[1]->getName());

   reportProgress(NORMAL, 100, "Tester Complete");
   pStep->finalize(Message::Success);

   return true;
}

void TiePointTesterAlgorithm::addLayer(RasterElement *pCube, TiePointList *pList, bool isMission)
{
   REQUIRE(pCube != NULL);
   REQUIRE(pList != NULL);

   Service<DesktopServices> pDesktop;
   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->getWindow(pCube->getName(), 
      SPATIAL_DATA_WINDOW));
   SpatialDataView* pView = static_cast<SpatialDataView*>(pWindow->getView());
   INVARIANT(pView != NULL);
   TiePointLayer* pLayer = static_cast<TiePointLayer*>(pView->createLayer(TIEPOINT_LAYER, pList, pList->getName()));
   if (pLayer == NULL)
   {
      LayerList* pLayerList = pView->getLayerList();
      INVARIANT(pLayerList != NULL);
      pLayer = static_cast<TiePointLayer*>(pLayerList->getLayer(TIEPOINT_LAYER, pList, pList->getName()));
   }
   INVARIANT(pLayer != NULL);
   pLayer->setSymbolSize(1);

   pLayer->setIsMission(isMission);
}

void TiePointTesterAlgorithm::computeTiePoints(size_t xSize, size_t ySize, std::vector<TiePoint>& points)
{
   const int noise = 300;
   for (size_t x = 0; x < xSize; x += 2)
   {
      for (size_t y = 0; y < ySize; y += 2)
      {
         TiePoint point;
         point.mReferencePoint.mX = static_cast<int>(x);
         point.mReferencePoint.mY = static_cast<int>(y);
         point.mMissionOffset.mX = static_cast<float>((rand() % noise)/100.0 + (rand() % noise)/100.0 -
            ((noise-1)/100.0));
         point.mMissionOffset.mY = static_cast<float>((rand() % noise)/100.0 + (rand() % noise)/100.0 -
            ((noise-1)/100.0));
         point.mConfidence = 0;
         point.mPhi = 0;
         points.push_back(point);
         reportProgress(NORMAL, static_cast<int>(100 * x / xSize), "Adding tie points");
      }
   }
}

int TiePointTesterAlgorithm::findCubes(RasterElement *pCube[2])
{
   // Get first two cubes
   std::vector<Window*> windows;
   Service<DesktopServices> pDesktop;
   pDesktop->getWindows(windows);
   SpatialDataWindow *pWindow[2];
   int windowsFound = 0;
   std::vector<Window*>::iterator ppWindow;
   for (ppWindow = windows.begin(); ppWindow != windows.end() && windowsFound < 2; ++ppWindow)
   {
      INVARIANT(*ppWindow != NULL);
      if ((*ppWindow)->isKindOf("SpatialDataWindow"))
      {
         pWindow[windowsFound] = static_cast<SpatialDataWindow*>(*ppWindow);
         SpatialDataView* pView = static_cast<SpatialDataView*>(pWindow[windowsFound]->getView());
         INVARIANT(pView != NULL);
         const LayerList* pLayerList = pView->getLayerList();
         pCube[windowsFound] = pLayerList->getPrimaryRasterElement();
         ++windowsFound;
      }
   }
   return windowsFound;
}

/**
 *  A baseclass obligation. Performs any required postprocessing
 *  after the dismissal of the GUI and after processAll. For the Tester
 *  plug-in, this does nothing.
 *
 *  @return true on success, false otherwise
 */
bool TiePointTesterAlgorithm::postprocess()
{
   return true;
}

/**
 *  Constructor for the algorithm object.
 *
 *  @param rasterElement
 *       The data cube being displayed
 *  @param pProgress
 *       The progress object to report back via
 *  @param interactive
 *       true if the algorithm is being run interactively, false otherwise
 */
TiePointTesterAlgorithm::TiePointTesterAlgorithm(RasterElement &rasterElement, 
                                                 Progress *pProgress, bool interactive) :
   AlgorithmPattern(&rasterElement, pProgress, interactive, NULL)
{
}

/**
 *  A baseclass obligation. Passes in the inputs from the GUI or the input
 *  arglist. Copies and validates the respective fields.
 *
 *  @param pAlgorithmData
 *       Initialization data to run the algorithm on
 *  @return true if the data were valid, false otherwise
 */
bool TiePointTesterAlgorithm::initialize(void* pAlgorithmData)
{
   bool success = true;
   REQUIRE(pAlgorithmData != NULL);

   mInputs = *(reinterpret_cast<TiePointTesterInputs*>(pAlgorithmData));

   return success;
}

/**
 *  A baseclass obligation. Indicates whether the algorithm can abort or not.
 *
 *  @return true if the algorithm can abort, false otherwise
 */
bool TiePointTesterAlgorithm::canAbort() const
{
   return false;
}

/**
 *  A baseclass obligation. Performs an abort, if supported.
 *
 *  @return true if the algorithm was successfully aborted, false otherwise
 */
bool TiePointTesterAlgorithm::doAbort()
{
   return false;
}
