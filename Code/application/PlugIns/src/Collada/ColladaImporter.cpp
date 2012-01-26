/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "ColladaImporter.h"
#include "ColladaUtil.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "FileResource.h"
#include "GraphicObject.h"
#include "ImportDescriptor.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "PolygonObject.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TypeConverter.h"

REGISTER_PLUGIN_BASIC(OpticksCollada, ColladaImporter);

ColladaImporter::ColladaImporter()
{
   setName("COLLADA Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{55A08469-EEB5-4D9E-AE4C-DA5C1B20A2E8}");
   allowMultipleInstances(true);
   setWizardSupported(false);
   setSubtype(TypeConverter::toString<AnnotationElement>());
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("Collada Files (*.shape.dae)");
}

ColladaImporter::~ColladaImporter()
{
}

bool ColladaImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress"));
   VERIFY(pArgList->addArg<AnnotationElement>(Importer::ImportElementArg(), NULL, "Annotation element"))
   VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL, "View in which the layer will be inserted."));
   return true;
}

bool ColladaImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ColladaImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   if (pProgress == NULL)
   {
      return false;
   }
   pProgress->updateProgress("Importing COLLADA file...", 0, NORMAL);

   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (pView == NULL)
   {
      pProgress->updateProgress("Invalid view", 0, ERRORS);
      return false;
   }

   AnnotationElement* pAnnoElement = pInArgList->getPlugInArgValue<AnnotationElement>(Importer::ImportElementArg());
   if (pAnnoElement == NULL)
   {
      pProgress->updateProgress("Invalid annotation element", 0, ERRORS);
      return false;
   }

   // Parent the annotation element to the main raster element
   LayerList* pLayerList = pView->getLayerList();
   RasterElement* pRasterElement = NULL;
   if (pLayerList != NULL)
   {
      pRasterElement = pLayerList->getPrimaryRasterElement();
      if (pRasterElement != NULL)
      {
         vector<string> elementNames = Service<ModelServices>()->getElementNames(pRasterElement,
            TypeConverter::toString<AnnotationElement>());
         for (vector<string>::iterator it = elementNames.begin(); it != elementNames.end(); ++it)
         {
            if (*it == pAnnoElement->getName())
            {
               pProgress->updateProgress("AnnotationElement by the same name already exists", 0, ERRORS);
               return false;
            }
         }
         if (Service<ModelServices>()->setElementParent(pAnnoElement, pRasterElement) == false)
         {
            pProgress->updateProgress("Error parenting AnnotationElement to primary RasterElement", 0, ERRORS);
            return false;
         }
      }
   }

   // Create the annotation layer
   AnnotationLayer* pAnnoLayer = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, pAnnoElement));
   if (pAnnoLayer == NULL)
   {
      pProgress->updateProgress("Layer was not created", 0, ERRORS);
      return false;
   }

   FileDescriptor* pFileDesc = pAnnoElement->getDataDescriptor()->getFileDescriptor();
   if (pFileDesc == NULL)
   {
      pProgress->updateProgress("Invalid data descriptor", 0, ERRORS);
      return false;
   }

   ColladaStreamReader reader(pAnnoElement->getFilename());
   std::vector<std::string> geomIds;
   reader.populateGeometryIds(geomIds);
   if (geomIds.empty())
   {
      pProgress->updateProgress("Empty or malformed file, nothing to import.", 0, ERRORS);
      return false;
   }

   int count = 0;
   int curPercent = 0;
   for (std::vector<std::string>::iterator idIt = geomIds.begin(); idIt != geomIds.end(); ++idIt)
   {
      // Check if the geometry was exported by Opticks by checking its ID
      size_t suffixPos = (*idIt).find(getAnnotationSuffix());
      if (suffixPos != std::string::npos)
      {
         string sessionName = string((*idIt).begin(), (*idIt).begin() + suffixPos);

         if (!reader.read(*idIt))
         {
            pProgress->updateProgress("Error reading file", 0, ERRORS);
            return false;
         }
         std::vector<Opticks::Location<float, 3>> vertices;
         reader.getVertices(vertices);
         if (vertices.empty())
         {
            pProgress->updateProgress("No vertices found for object " + sessionName,
               curPercent, WARNING);
         }

         std::vector<std::vector<unsigned int>> polygons;
         reader.getPolygons(polygons);
         if (polygons.empty())
         {
            pProgress->updateProgress("No polygonal information found for object " + sessionName, curPercent, WARNING);
            return false;
         }

         std::vector<LocationType> location2dCopy(0);
         for (std::vector<Opticks::Location<float, 3>>::iterator vertIt = vertices.begin();
            vertIt != vertices.end(); ++vertIt)
         {
            location2dCopy.push_back(LocationType((*vertIt).mX, (*vertIt).mY));
         }

         // Create the graphic object
         PolygonObject* pGraphicObject = dynamic_cast<PolygonObject*>(pAnnoLayer->addObject(POLYGON_OBJECT));
         if (pGraphicObject == NULL)
         {
            pProgress->updateProgress("Polygon object could not be created", 0, ERRORS);
            return false;
         }
         pGraphicObject->setName(sessionName);

         // Populate the graphic objects with the vertices based on the face information
         for (std::vector<std::vector<unsigned int>>::iterator polyIt = polygons.begin();
            polyIt != polygons.end(); ++polyIt)
         {
            // Copy the vertices needed based on the face information
            std::vector<LocationType> currentFaceVertices;
            for (unsigned int i = 0; i < (*polyIt).size(); ++i)
            {
               unsigned int index = (*polyIt).at(i);
               currentFaceVertices.push_back(location2dCopy.at(index));
            }
            // Close the figure before adding the vertices to the graphic object
            currentFaceVertices.push_back(location2dCopy.at(0));

            pGraphicObject->addVertices(currentFaceVertices);
            pGraphicObject->setRotation(reader.getRotation());
         }
      }
      ++count;
      int curPercent = (count * 100) / geomIds.size();
      pProgress->updateProgress("Reading geometry...", curPercent, NORMAL);
   }

   return true;
}

std::vector<ImportDescriptor*> ColladaImporter::getImportDescriptors(const std::string& filename)
{
   std::vector<ImportDescriptor*> descriptors(0);
   if (!filename.empty())
   {
      ImportDescriptor* pImportDescriptor = Service<ModelServices>()->createImportDescriptor(filename,
         "AnnotationElement", NULL);
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

unsigned char ColladaImporter::getFileAffinity(const std::string& filename)
{
   if (filename.rfind(".shape.dae") != std::string::npos)
   {
      return Importer::CAN_LOAD;
   }
   return Importer::CAN_NOT_LOAD;
}