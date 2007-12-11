/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "Hdf5File.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "IcePseudocolorLayerImporter.h"
#include "IceReader.h"
#include "PlugInArgList.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

using namespace std;

IcePseudocolorLayerImporter::IcePseudocolorLayerImporter() :
   IceImporterShell(IceUtilities::PSEUDOCOLOR_LAYER),
   mpView(NULL)
{
   setName("Ice PseudocolorLayer Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setShortDescription("Imports Ice Pseudocolor Layer Files");
   // the psl.ice.h5 extension is to allow standard HDF5 tools to recognize the file as HDF5
   setExtensions("Ice Pseudocolor Layer Files (*.psl.ice.h5)");
   setDescriptorId("{680DD1DB-403F-49ec-9623-D81F5F4F01C9}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

IcePseudocolorLayerImporter::~IcePseudocolorLayerImporter()
{
}

QWidget* IcePseudocolorLayerImporter::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   return NULL;
}

bool IcePseudocolorLayerImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(IceImporterShell::getInputSpecification(pArgList) == true);
   return pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL);
}

bool IcePseudocolorLayerImporter::parseInputArgList(PlugInArgList* pInArgList)
{
   // Get the View arg -- if no View is specified, use the current workspace window's view.
   DO_IF(IceImporterShell::parseInputArgList(pInArgList) == false, return false);
   mpView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (mpView == NULL)
   {
      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(Service<DesktopServices>()->getCurrentWorkspaceWindow());
      if (pWindow != NULL)
      {
         mpView = pWindow->getSpatialDataView();
      }

      if (mpView == NULL)
      {
         Progress* pProgress = getProgress();
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Unable to create the Layer: No SpatialDataView specified.", 100, ERRORS);
         }

         return false;
      }
   }

   return true;
}

SpatialDataView* IcePseudocolorLayerImporter::createView() const
{
   Progress* pProgress = getProgress();

   hid_t fileHandle;
   Hdf5FileResource fileResource;
   if (getFileHandle(fileHandle, fileResource) == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the file handle.", 100, ERRORS);
      }

      return NULL;
   }

   RasterElement* pElement = getRasterElement();
   if (pElement == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the RasterElement.", 100, ERRORS);
      }

      return NULL;
   }

   Hdf5File file(pElement->getFilename(), fileHandle);
   const string layerPath = "/Layers/PseudocolorLayer1";
   if (file.readFileData(layerPath) == false)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the file data.", 100, ERRORS);
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
      pProgress->updateProgress(warningMessage, 100, WARNING);
   }

   return mpView;
}
