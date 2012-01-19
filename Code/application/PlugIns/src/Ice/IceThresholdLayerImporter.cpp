/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "Hdf5File.h"
#include "Hdf5Resource.h"
#include "IceThresholdLayerImporter.h"
#include "IceReader.h"
#include "IceUtilities.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksIce, IceThresholdLayerImporter);

IceThresholdLayerImporter::IceThresholdLayerImporter() :
   IceImporterShell(IceUtilities::THRESHOLD_LAYER),
   mpView(NULL)
{
   setName("Ice Threshold Layer Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setShortDescription("Imports Ice threshold layer files");
   // the thl.ice.h5 extension is to allow standard HDF5 tools to recognize the file as HDF5
   setExtensions("Ice Threshold Layer Files (*.thl.ice.h5)");
   setDescriptorId("{4259B602-6FB7-4BA9-9AC5-3F9DD53C1294}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

IceThresholdLayerImporter::~IceThresholdLayerImporter()
{}

QWidget* IceThresholdLayerImporter::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   return NULL;
}

bool IceThresholdLayerImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(IceImporterShell::getInputSpecification(pArgList) == true);
   return pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL);
}

bool IceThresholdLayerImporter::parseInputArgList(PlugInArgList* pInArgList)
{
   // Get the View arg -- if no View is specified, use the current workspace window's view.
   DO_IF(IceImporterShell::parseInputArgList(pInArgList) == false, return false);
   mpView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (mpView == NULL)
   {
      Service<DesktopServices> pDesktop;

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
      if (pWindow != NULL)
      {
         mpView = pWindow->getSpatialDataView();
      }

      if (mpView == NULL)
      {
         Progress* pProgress = getProgress();
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Unable to create the threshold layer - no spatial data view is specified.",
               0, ERRORS);
         }

         return false;
      }
   }

   return true;
}

SpatialDataView* IceThresholdLayerImporter::createView() const
{
   Progress* pProgress = getProgress();

   hid_t fileHandle;
   Hdf5FileResource fileResource;
   if (getFileHandle(fileHandle, fileResource) == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the file handle.", 0, ERRORS);
      }

      return NULL;
   }

   RasterElement* pElement = getRasterElement();
   if (pElement == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the RasterElement.", 0, ERRORS);
      }

      return NULL;
   }

   Hdf5File file(pElement->getFilename(), fileHandle);
   const string layerPath = "/Layers/ThresholdLayer1";
   if (file.readFileData(layerPath) == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the file data.", 0, ERRORS);
      }

      return NULL;
   }

   IceReader reader(file);
   string warningMessage;
   if (reader.createLayer(layerPath, mpView, pElement, warningMessage) == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to load the layer from the file. " + warningMessage, 100, ERRORS);
      }

      return NULL;
   }

   if (warningMessage.empty() == false && pProgress != NULL)
   {
      pProgress->updateProgress(warningMessage, 0, WARNING);
   }

   return mpView;
}
