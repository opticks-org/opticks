/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationControllerImp.h"
#include "AnimationFrame.h"
#include "assert.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <memory>
#include <vector>

using namespace std;

class AnimationStepTestCase : public TestCase
{
public:
   AnimationStepTestCase() : TestCase("Step") {}
   bool run()
   {
      bool success = true;
      const double timeStep = 24.0/7.0; // want time values to be non-integral
      const double timeOffset = timeStep/10.0;

      auto_ptr<AnimationControllerImp> pController(new AnimationControllerImp(FRAME_TIME, "{A06CD2E6-6AB7-48de-826D-D7C5DCB0E0B9}"));
      Animation *pAnimation1 = pController->createAnimation("{598E7785-8584-4195-B63C-1BF2E4A70817}");
      Animation *pAnimation2 = pController->createAnimation("{B33F0903-4605-40a2-BA54-C645EA44E1B3}");
      Animation *pAnimation3 = pController->createAnimation("{91E2D8EB-D16D-413c-82DE-EB20321C2600}");

      vector<AnimationFrame> frames;

      // All the same
      for (int i=0; i<10; ++i)
      {
         AnimationFrame frame("frame", i, i*timeStep);
         frames.push_back(frame);
      }

      pAnimation1->setFrames(frames);
      pAnimation2->setFrames(frames);
      pAnimation3->setFrames(frames);

      pController->stepForward();
      issearf(pController->getCurrentFrame() == timeStep);
      pController->stepForward();
      issearf(pController->getCurrentFrame() == 2.0*timeStep);
      pController->stepBackward();
      issearf(pController->getCurrentFrame() == timeStep);

      // All slightly offset
      vector<AnimationFrame> frames2;
      for (int i=0; i<10; ++i)
      {
         AnimationFrame frame("frame", i, i*timeStep+timeOffset);
         frames2.push_back(frame);
      }

      vector<AnimationFrame> frames3;
      for (int i=0; i<10; ++i)
      {
         AnimationFrame frame("frame", i, i*timeStep+2.0*timeOffset);
         frames3.push_back(frame);
      }

      pAnimation2->setFrames(frames2);
      pAnimation3->setFrames(frames3);

      pController->setCurrentFrame(0.0);
      pController->stepForward();
      issearf(pController->getCurrentFrame() == timeOffset);
      pController->stepForward();
      issearf(pController->getCurrentFrame() == timeOffset+timeOffset);
      pController->stepForward();
      issearf(pController->getCurrentFrame() == timeStep);
      pController->stepBackward();
      issearf(pController->getCurrentFrame() == timeOffset+timeOffset);
      pController->stepBackward();
      issearf(pController->getCurrentFrame() == timeOffset);

      return success;
   }
};

class SetFramesTestCase : public TestCase
{
public:
   SetFramesTestCase() : TestCase("SetFrames") {}
   bool run()
   {
      bool success = true;

      auto_ptr<AnimationControllerImp> pController
         (new AnimationControllerImp(FRAME_TIME, "{A915ABB9-18A4-4512-9797-9F4F973EFD43}"));
      Animation *pAnimation1 = pController->createAnimation("{790797F3-7601-44da-9746-2082CDB5EB41}");
      issearf(pAnimation1 != NULL);

      vector<AnimationFrame> oddValues;
      vector<AnimationFrame> evenValues;
      const unsigned int maxValue = 10;
      const unsigned int valueToTest = 4;
      for (unsigned int i = 0; i < 10; i++)
      {
         AnimationFrame frame("frame", i, i / 2);
         if ((i % 2) == 0)
         {
            evenValues.push_back(frame);
         }
         else
         {
            oddValues.push_back(frame);
         }
      }

      // Assign some frames to pAnimation1.
      pAnimation1->setFrames(evenValues);

      // Set the pController to valueToTest.
      pController->setCurrentFrame(valueToTest);
      issearf(pController->getCurrentFrame() == valueToTest);

      // Create a new Animation and add it to pController. Ensure that pController->getCurrentFrame() never changes.
      Animation *pAnimation2 = pController->createAnimation("{8DBE24C4-5C92-4590-BD28-D66FD001262F}");
      issearf(pAnimation2 != NULL);
      issearf(pController->getCurrentFrame() == valueToTest);

      // Change the frames associated with pAnimation2 and pAnimation1.
      // Ensure that pController->getCurrentFrame() never changes.
      pAnimation2->setFrames(evenValues);
      issearf(pController->getCurrentFrame() == valueToTest);

      pAnimation2->setFrames(oddValues);
      issearf(pController->getCurrentFrame() == valueToTest);

      pAnimation1->setFrames(oddValues);
      issearf(pController->getCurrentFrame() == valueToTest);

      pAnimation1->setFrames(evenValues);
      issearf(pController->getCurrentFrame() == valueToTest);

      // Destroy pAnimation2. Ensure that pController->getCurrentFrame() never changes.
      pController->destroyAnimation(pAnimation2);
      pAnimation2 = NULL;
      issearf(pController->getCurrentFrame() == valueToTest);

      return success;
   }
};

class FrameRateTestCase : public TestCase
{
public:
   FrameRateTestCase() : TestCase("FrameRate") {}
   bool run()
   {
      bool success = true;

      std::string filename = TestUtilities::getTestDataPath() + "ice/cube512x512x256x2ulsb.re.ice.h5";

      SpatialDataWindow* pWindow = TestUtilities::loadDataSet(filename, "Auto Importer");
      issearf(pWindow != NULL);
      LayerList* pLList = pWindow->getSpatialDataView()->getLayerList();
      RasterLayer* pLayer = static_cast<RasterLayer*>(pLList->getLayer(RASTER, pLList->getPrimaryRasterElement()));
      if (pLayer->isGpuImageSupported())
      {
         pLayer->enableGpuImage(true);
      }

      /**
       * Test data from autotest machines
       *  date     machine          gpu?    framerate
       * --------------------------------------------
       *  6Feb09   autotest-pc      yes     259
       *  6Feb09   autotest-pc      yes     260
       * 27Mar09   autotest-pc      yes     234
       *  1Apr09   autotest-pc      yes     208
       *  6Feb09   navi             no      201 
       *  6Feb09   navi             no      202
       * Initial Calibration: GPU=200  No GPU=180
       */

      /**
       * These tests are designed to run with vertical sync disabled and (if using
       * NVIDIA drivers) dual-view disabled.
       */
      ExecutableResource pRasterTimingTest("Raster Timing Test");
      issearf(pRasterTimingTest.get() != NULL);
      issearf(pRasterTimingTest->execute());
      double framerate = 0.0;
      pRasterTimingTest->getOutArgList().getPlugInArgValue("Framerate", framerate);
      std::cout << "Actual framerate is: " << static_cast<long>(framerate) << " fps." << std::endl;
      if (pLayer->isGpuImageEnabled())
      {
         std::cout << "GPU image is enabled." << std::endl;
         // Re-enable this when a more stable baseline is available (tclarke)
         //issea(framerate > 200.0);
      }
      else
      {
         // Re-enable this when a more stable baseline is available (tclarke)
         //issea(framerate > 180.0);
      }
      if (!success)
      {
         std::cout << "The minimum framerate value has been calibrated to a specific test machine. If this test fails on "
                   << "a different machine, use an earlier version of Opticks to establish a proper baseline before considering "
                   << "this a true failure." << std::endl
                   << "Also, ensure that your video drivers have vertical sync disabled and dual-view disabled (for NVIDIA cards)."
                   << std::endl;
      }

      return success;
   }
};

class AnimationTestSuite : public TestSuiteNewSession
{
public:
   AnimationTestSuite() : TestSuiteNewSession("Animation")
   {
      addTestCase(new AnimationStepTestCase);
      addTestCase(new SetFramesTestCase);
      addTestCase(new FrameRateTestCase);
   }
};

REGISTER_SUITE(AnimationTestSuite)
