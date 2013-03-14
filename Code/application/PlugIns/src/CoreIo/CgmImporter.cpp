/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "CgmImporter.h"
#include "CgmObject.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "Endian.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProductView.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "Undo.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksCoreIo, CgmImporter);

CgmImporter::CgmImporter()
{
   setName("CGM Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("CGM Files (*.cgm *.CGM)");
   setDescription("Import CGM Elements");
   setSubtype("CGM");
   setDescriptorId("{09EA7F29-219D-4415-A056-53D89AE93CEC}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

CgmImporter::~CgmImporter()
{
}

bool CgmImporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   PlugInArg* pArg = NULL;
   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(Executable::ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArg->setDescription(Executable::ProgressArgDescription());
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(Importer::ImportElementArg());
   pArg->setType("DataElement");
   pArg->setDefaultValue(NULL);
   pArg->setDescription("Data element to be imported.");
   pInArgList->addArg(*pArg);

   return true;
}

bool CgmImporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

vector<ImportDescriptor*> CgmImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;
   if (!filename.empty())
   {
      FactoryResource<Filename> pFullFilename;
      pFullFilename->setFullPathAndName(filename);

      ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(filename, "AnnotationElement", NULL);
      if (pImportDescriptor != NULL)
      {
         DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
         if (pDescriptor != NULL)
         {
            FactoryResource<FileDescriptor> pFileDescriptor;
            if (pFileDescriptor.get() != NULL)
            {
               pFileDescriptor->setFilename(filename);
               pDescriptor->setFileDescriptor(pFileDescriptor.get());
            }
         }

         descriptors.push_back(pImportDescriptor);
      }
   }

   return descriptors;
}

unsigned char CgmImporter::getFileAffinity(const std::string& filename)
{
   const uint16_t COMMAND_MASK = 0xFFE0;
   const uint16_t LENGTH_MASK = 0x001F; // bitwise complement of COMMAND_MASK
   const uint16_t BEGIN_METAFILE_VAL = 0x0020;
   const uint32_t METAFILE_VERSION_VAL = 0x10220001;
   const uint16_t END_PICTURE_VAL = 0x00A0;
   const uint16_t END_METAFILE_VAL = 0x0040;

   FileResource cgmFile(filename.c_str(), "rb");
   if (cgmFile.get() == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }
   Endian swapper(BIG_ENDIAN_ORDER);

   // Read the first two bytes of the file, which should be the start of the Begin Metafile element
   uint16_t beginMetafileVal;
   if (fread(&beginMetafileVal, sizeof(beginMetafileVal), 1, cgmFile) != 1)
   {
      return Importer::CAN_NOT_LOAD;
   }
   swapper.swapValue(beginMetafileVal);

   // Check the last 11 bits of the element for the required value
   if ((beginMetafileVal & COMMAND_MASK) != BEGIN_METAFILE_VAL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // Check the first five bits of the element to get the parameter list length,
   // which also determines long or short form input
   uint16_t paramLength = beginMetafileVal & LENGTH_MASK;
   if (paramLength == 31)  // long form
   {
      // Read the next two bytes to determine the parameter list length
      if (fread(&paramLength, sizeof(paramLength), 1, cgmFile) != 1)
      {
         return Importer::CAN_NOT_LOAD;
      }
      swapper.swapValue(paramLength);
   }

   // Add padding to the parameter list length, if necessary
   if (paramLength & 1)    // odd number
   {
      ++paramLength;       // padded
   }

   // Seek past the parameter list to the Metafile Version element
   if (fseek(cgmFile, paramLength, SEEK_CUR) != 0)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // Read the next four bytes, which is the Metafile Version element
   uint32_t metafileVersion;
   if (fread(&metafileVersion, sizeof(metafileVersion), 1, cgmFile) != 1)
   {
      return Importer::CAN_NOT_LOAD;
   }
   swapper.swapValue(metafileVersion);

   // Check the read value with the required Metafile Version value
   if (metafileVersion != METAFILE_VERSION_VAL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // Seek to four bytes before the end of the file
   if (fseek(cgmFile, -4, SEEK_END) != 0)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // Read the last four bytes of the file, which should be the End Picture and End Metafile elements
   uint16_t endPictureVal;
   uint16_t endMetafileVal;
   if (fread(&endPictureVal, sizeof(endPictureVal), 1, cgmFile) != 1)
   {
      return Importer::CAN_NOT_LOAD;
   }
   if (fread(&endMetafileVal, sizeof(endMetafileVal), 1, cgmFile) != 1)
   {
      return Importer::CAN_NOT_LOAD;
   }
   swapper.swapValue(endPictureVal);
   swapper.swapValue(endMetafileVal);

   // Check the read values with the required End Picture and End Metafile values
   if ((endPictureVal == END_PICTURE_VAL) && (endMetafileVal == END_METAFILE_VAL))
   {
      return Importer::CAN_LOAD;
   }

   return Importer::CAN_NOT_LOAD;
}

bool CgmImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = NULL;
   DataElement* pElement = NULL;
   StepResource pStep("Import cgm element", "app", "8D5522FE-4A89-44cb-9735-6920A3BFC903");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "A1735AC7-C182-45e6-826F-690DBA15D84A");

      pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));
      
      pElement = pInArgList->getPlugInArgValue<DataElement>(Importer::ImportElementArg());
      if (pElement == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No data element", 0, ERRORS);
         }
         pStep->finalize(Message::Failure, "No data element");
         return false;
      }
      pMsg->addProperty("Element name", pElement->getName());
   }
   if (pProgress != NULL)
   {
      pProgress->updateProgress((string("Read and parse file ") + pElement->getFilename()), 20, NORMAL);
   }

   // Create a new annotation layer for a spatial data view or get the layout layer for a product view
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Create a new layer", 30, NORMAL);
   }

   View* pView = mpDesktop->getCurrentWorkspaceWindowView();
   if (pView == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Could not access the current view.", 0, ERRORS);
      }

      pStep->finalize(Message::Failure, "Could not access the current view.");
      return false;
   }

   UndoGroup undoGroup(pView, "Import CGM");
   AnnotationLayer* pLayer = NULL;

   SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
   if (pSpatialDataView != NULL)
   {
      // Set the parent element of the annotation element to the primary raster element
      LayerList* pLayerList = pSpatialDataView->getLayerList();
      if (pLayerList != NULL)
      {
         RasterElement* pNewParentElement = pLayerList->getPrimaryRasterElement();
         if (pNewParentElement != NULL)
         {
            Service<ModelServices> pModel;
            pModel->setElementParent(pElement, pNewParentElement);
         }
      }

      pLayer = dynamic_cast<AnnotationLayer*>(pSpatialDataView->createLayer(ANNOTATION, pElement));
   }
   else
   {
      ProductView* pProductView = dynamic_cast<ProductView*>(mpDesktop->getCurrentWorkspaceWindowView());
      if (pProductView != NULL)
      {
         pLayer = pProductView->getLayoutLayer();
      }
   }

   if (pLayer == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to get the annotation layer", 0, ERRORS);
      }

      pStep->finalize(Message::Failure, "Unable to get the annotation layer");
      return false;
   }

   // add the CGM object
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Create the CGM object", 60, NORMAL);
   }
   CgmObject* pCgmObject = dynamic_cast<CgmObject*>(pLayer->addObject(CGM_OBJECT));
   if (pCgmObject == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to create the CGM object", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Unable to create the CGM object");
      return false;
   }

   // load the CGM file
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Load the CGM file", 90, NORMAL);
   }
   string fname = pElement->getDataDescriptor()->getFileDescriptor()->getFilename().getFullPathAndName();
   if (!pCgmObject->deserializeCgm(fname))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Error loading the CGM element", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Unable to parse the CGM file.");
      return false;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Successfully loaded the CGM file", 100, NORMAL);
   }
   pStep->finalize(Message::Success);
   return true;
}
