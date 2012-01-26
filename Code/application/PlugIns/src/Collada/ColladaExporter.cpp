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
#include "ColladaExporter.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "FileResource.h"
#include "GraphicObject.h"
#include "GraphicGroup.h"
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
#include "StringUtilities.h"
#include "TypeConverter.h"

#include <set>

REGISTER_PLUGIN_BASIC(OpticksCollada, ColladaExporter);

ColladaExporter::ColladaExporter()
{
   setName("COLLADA Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{9EC0847B-0CA1-4991-A13A-E13F68F966C3}");
   allowMultipleInstances(true);
   setWizardSupported(false);
   setSubtype(TypeConverter::toString<AnnotationElement>());
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("COLLADA Files (*.shape.dae)");
}

ColladaExporter::~ColladaExporter()
{
}

bool ColladaExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress"));
   VERIFY(pArgList->addArg<AnnotationElement>(Exporter::ExportItemArg(), NULL, "AnnotationElement"));
   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), NULL, "File descriptor for the output file."));
   return true;
}

bool ColladaExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ColladaExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   if (pProgress == NULL)
   {
      return false;
   }
   AnnotationElement* pAnnoElement = pInArgList->getPlugInArgValue<AnnotationElement>(Exporter::ExportItemArg());
   if (pAnnoElement == NULL)
   {
      pProgress->updateProgress("Cannot get annotation element", 0, ERRORS);
      return false;
   }
   if (pAnnoElement->getMetadata()->getNumAttributes() > 0)
   {
      pProgress->updateProgress("The COLLADA exporter does not support exporting metadata for AnnotationElements",
         0, WARNING);
   }

   GraphicGroup* pGroup = pAnnoElement->getGroup();
   if (pGroup == NULL)
   {
      pProgress->updateProgress("Annotation has invalid GraphicGroup", 0, ERRORS);
      return false;
   }
   std::list<GraphicObject*> objects = pGroup->getObjects();
   std::list<GraphicObject*> objectsToExport(0);
   std::set<std::string> unsupportedTypes;
   for (std::list<GraphicObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
   {
      if (isGraphicObjectSupported((*it)->getGraphicObjectType()))
      {
         objectsToExport.push_back((*it));
      }
      else
      {
         unsupportedTypes.insert(StringUtilities::toDisplayString((*it)->getGraphicObjectType()));
      }
   }

   pProgress->updateProgress("Exporting layer...", 50, NORMAL);
   if (!unsupportedTypes.empty())
   {
      std::string message = "Graphic objects of the following unsupported types were detected:\n";
      for (std::set<std::string>::iterator it = unsupportedTypes.begin(); it != unsupportedTypes.end(); ++it)
      {
         message += (*it) + "\n";
      }
      message += "Objects of these types cannot be exported by the COLLADA exporter.";
      pProgress->updateProgress(message, 50, WARNING);
   }

   if (objectsToExport.empty())
   {
      pProgress->updateProgress("No supported annotations found for export", 100, NORMAL);
      return true;
   }

   FileDescriptor* pFileDesc = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (pFileDesc == NULL)
   {
      pProgress->updateProgress("Cannot get the file descriptor", 0, ERRORS);
      return false;
   }
   ColladaStreamWriter* pWriter = new ColladaStreamWriter(pFileDesc->getFilename().getFullPathAndName());
   pWriter->writeGraphicObjects(objectsToExport);
   pProgress->updateProgress("File exported successfully", 100, NORMAL);
   return true;
}

bool ColladaExporter::isGraphicObjectSupported(GraphicObjectType type)
{
   bool isSupported = false;
   switch (type)
   {
      case RECTANGLE_OBJECT:        // fall through
      case TRIANGLE_OBJECT:         // fall through
      case POLYGON_OBJECT:
         isSupported = true;
         break;
      case POLYLINE_OBJECT:         // fall through
      case LINE_OBJECT:             // fall through
      case TEXT_OBJECT:             // fall through
      case FRAME_LABEL_OBJECT:      // fall through
      case ARROW_OBJECT:            // fall through
      case ELLIPSE_OBJECT:          // fall through
      case ROUNDEDRECTANGLE_OBJECT: // fall through
      case ARC_OBJECT:              // fall through
      case MOVE_OBJECT:             // fall through
      case ROTATE_OBJECT:           // fall through
      case SCALEBAR_OBJECT:         // fall through
      case GROUP_OBJECT:            // fall through
      case CGM_OBJECT:              // fall through
      case RAW_IMAGE_OBJECT:        // fall through
      case FILE_IMAGE_OBJECT:       // fall through
      case WIDGET_IMAGE_OBJECT:     // fall through
      case LATLONINSERT_OBJECT:     // fall through
      case NORTHARROW_OBJECT:       // fall through
      case EASTARROW_OBJECT:        // fall through
      case VIEW_OBJECT:             // fall through
      case MULTIPOINT_OBJECT:       // fall through
      case MEASUREMENT_OBJECT:      // fall through
      case BITMASK_OBJECT:          // fall through
      case HLINE_OBJECT:            // fall through
      case VLINE_OBJECT:            // fall through
      case ROW_OBJECT:              // fall through
      case COLUMN_OBJECT:           // fall through
      case TRAIL_OBJECT:            // fall through
      default:
         break;
   }
   return isSupported;
}