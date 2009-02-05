/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterTimingTest.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "Undo.h"

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <vector>
#include <time.h>

using namespace std;

RasterTimingTest::RasterTimingTest()
{
   AlgorithmShell::setName("Raster Timing Test");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Runs a test to see how many frames per second a raster layer can be updated.");
   setProductionStatus(false);
   setDescriptorId("{8BCBD0E4-8EFD-49b6-AC65-8D59C1049A94}");
   allowMultipleInstances(false);
   setWizardSupported(false);
   executeOnStartup(false);
   destroyAfterExecute(true);
   setMenuLocation("[Demo]\\Raster Timing Test");
   setInteractive();
}

bool RasterTimingTest::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool RasterTimingTest::getOutputSpecification(PlugInArgList*& pArgList)
{
   if (isBatch())
   {
      VERIFY(pArgList = Service<PlugInManagerServices>()->getPlugInArgList());
      VERIFY(pArgList->addArg<double>("Framerate"));
   }
   else
   {
      pArgList = NULL;
   }
   return true;
}

bool RasterTimingTest::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (isBatch())
   {
      VERIFY(pOutArgList != NULL);
   }
   Service<DesktopServices> pDesktop;
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
   if (pView)
   {
      UndoLock lock(pView);

      RasterElement* pElement = pView->getLayerList()->getPrimaryRasterElement();
      RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      int bands = pDesc->getBandCount();
      int frameNumber = 0;
      RasterLayer* pLayer = NULL;
      vector<Layer*> layers;
      pView->getLayerList()->getLayers(RASTER, layers);
      for (vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
      {
         RasterLayer* pRasterLayer = static_cast<RasterLayer*>(*iter);
         if (pRasterLayer != NULL)
         {
            RasterElement* pCurrentRasterElement = dynamic_cast<RasterElement*>(pRasterLayer->getDataElement());
            if (pCurrentRasterElement == pElement)
            {
               pLayer = pRasterLayer;
               break;
            }
         }
      }
      for (int i = 0; i < bands; ++i)
      {
         pElement->getStatistics(pDesc->getActiveBand(i))->getMin();
      }
      // set grayscale display mode
      DisplayMode initialDisplayMode = pLayer->getDisplayMode();
      pLayer->setDisplayMode(GRAYSCALE_MODE);
      const int frameiterations = 10000;
      clock_t startTime = clock();
      QWidget* pWidget = pView->getWidget();
      int i = 0;
      for (i = 0; i < frameiterations; ++i, ++frameNumber)
      {
         if (frameNumber >= bands)
         {
            frameNumber = 0;
         }

         pLayer->setDisplayedBand(GRAY, pDesc->getActiveBand(frameNumber));
         if (pWidget)
         {
            pWidget->repaint();
         }

         if ((i + 1) % (frameiterations / 100) == 0)
         {
            QString message = QString("Frame ") + QString::number(i+1) + QString(" of ") +
               QString::number(frameiterations);
            pDesktop->setStatusBarMessage(message.toStdString());
         }
         if ((i + 1) % 20 == 0)
         {
            clock_t stopTime = clock();
            double elapsedTime = static_cast<double>(stopTime - startTime) / CLOCKS_PER_SEC;
            if (elapsedTime > 30)
            {
               ++i;
               break;
            }
         }
      }
      clock_t stopTime = clock();
      double framesPerSec = i / (static_cast<double>(stopTime - startTime) / CLOCKS_PER_SEC);

      // restore display mode
      pLayer->setDisplayMode(initialDisplayMode);

      if (isBatch())
      {
         pOutArgList->setPlugInArgValue<double>("Framerate", &framesPerSec);
      }
      else
      {
         QMessageBox::information(pDesktop->getMainWidget(), "Frame Rate", 
            QString("The number of frames per second was: %1\nGPU Acceleration was%2 enabled\n").arg(framesPerSec)
                     .arg(pLayer->isGpuImageEnabled() ? "" : " not"));
      }

      return true;
   }

   return false;
}
