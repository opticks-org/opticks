/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AnnotationLayer.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "GraphicElement.h"
#include "GraphicGroup.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterElement.h"
#include "ShapeFile.h"
#include "ShapeFileExporter.h"
#include "ShapeFileOptionsWidget.h"
#include "ShapeFileTypes.h"
#include "SpatialDataView.h"
#include "TypeConverter.h"
#include "UtilityServices.h"

#include <QtCore/QString>
#include <QtGui/QMessageBox>

#include <list>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksShapeFileExporter, ShapeFileExporter);

ShapeFileExporter::ShapeFileExporter() :
   mpFileDesc(NULL),
   mpGraphicElement(NULL),
   mpGeoref(NULL),
   mpLayers(NULL)
{
   setName("Shape File Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{6A4FBFF5-1642-4d66-95DC-AFBB1A7E5B1E}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("Shape Files (*.shp *.shx *.dbf)");
   setSubtype(TypeConverter::toString<GraphicLayer>());
   addDependencyCopyright("shapelib", Service<UtilityServices>()->getTextFromFile(":/licenses/shapelib"));
}

ShapeFileExporter::~ShapeFileExporter()
{}

bool ShapeFileExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), NULL,
      "File descriptor for the output file."));

   if (isBatch())
   {
      VERIFY(pArgList->addArg<GraphicElement>(TypeConverter::toString<GraphicElement>(), NULL,
         "The graphic element to be exported"));
      VERIFY(pArgList->addArg<RasterElement>(TypeConverter::toString<RasterElement>(), NULL,
         "Source of georeference for the graphic element being exported"));
   }
   else
   {
      VERIFY(pArgList->addArg<GraphicLayer>(Exporter::ExportItemArg(), NULL,
         "The layer to be exported as a shape file"));
   }

   return true;
}

bool ShapeFileExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ShapeFileExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Shape File Exporter", "app", "EA0C2EEB-8A9E-4017-8456-DB316B0742CC");
   string msg;
   if (pInArgList == NULL)
   {
      msg = "Input argument list is invalid.";
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   // Progress
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   if (mpOptionsWidget.get() == NULL)  // then need to extract inputs and add mpGraphicElement to mShapefile
   {
      if (extractInputs(pInArgList, msg) == false)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, msg);
         return false;
      }
   }

   pStep->addProperty("sourceFile", mShapefile.getFilename());

   // Save the shape file
   bool bSuccess = mShapefile.save(pProgress, msg);
   if (bSuccess == true)
   {
      msg = "Shape file export complete!";
      pStep->finalize(Message::Success);
   }
   else
   {
      pStep->finalize(Message::Failure, msg);
   }

   return bSuccess;
}

QWidget* ShapeFileExporter::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   string msg;
   Service<DesktopServices> pDesktop;
   if (pInArgList == NULL)
   {
      msg = "Input argument list is invalid.";
      QMessageBox::warning(pDesktop->getMainWidget(), "Error Extracting Inputs", QString::fromStdString(msg));
      return NULL;
   }

   if (mpOptionsWidget.get() == NULL)
   {
      if (extractInputs(pInArgList, msg) == false)
      {
         QMessageBox::warning(pDesktop->getMainWidget(), "Error Extracting Inputs", QString::fromStdString(msg));
         return NULL;
      }
      vector<Layer*> graphicLayers;
      if (mpLayers != NULL)
      {
         graphicLayers = mpLayers->getLayers();
      }
      vector<GraphicElement*> elements;
      for (vector<Layer*>::iterator it = graphicLayers.begin(); it != graphicLayers.end(); ++it)
      {
         GraphicElement* pGraphicElement = dynamic_cast<GraphicElement*>((*it)->getDataElement());
         if (pGraphicElement != NULL)
         {
            elements.push_back(pGraphicElement);
         }
      }

      mpOptionsWidget.reset(new ShapeFileOptionsWidget(mpFileDesc, &mShapefile, mpGraphicElement, elements, mpGeoref));
   }

   return mpOptionsWidget.get();
}

bool ShapeFileExporter::extractInputs(const PlugInArgList* pInArgList, string& message)
{
   if (pInArgList == NULL)
   {
      message = "Invalid argument list.";
      return false;
   }

   mpFileDesc = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (mpFileDesc == NULL)
   {
      message = "No file specified.";
      return false;
   }
   mShapefile.setFilename(mpFileDesc->getFilename().getFullPathAndName());

   GraphicLayer* pLayer = NULL;

   if (isBatch())
   {
      mpGraphicElement = pInArgList->getPlugInArgValue<GraphicElement>("GraphicElement");
      mpGeoref = pInArgList->getPlugInArgValue<RasterElement>("RasterElement");
   }
   else
   {
      pLayer = pInArgList->getPlugInArgValue<GraphicLayer>(Exporter::ExportItemArg());
      if (pLayer == NULL)
      {
         message = "Input argument list did not include anything to export.";
         return false;
      }

      mpGraphicElement = dynamic_cast<GraphicElement*>(pLayer->getDataElement());
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         mpLayers = pView->getLayerList();
         if (mpLayers != NULL)
         {
            mpGeoref = mpLayers->getPrimaryRasterElement();
         }
      }
   }

   if (mpGraphicElement == NULL)
   {
      message = "Could not identify the data element to export.";
      return false;
   }

   // The BitMaskIterator does not support negative extents and
   // the BitMask does not correctly handle the outside flag so
   // the BitMaskIterator is used for cases when the outside flag is true and
   // the BitMask is used for cases when the outside flag is false.
   // This is the case when the outside flag is false. The case where the
   // outside flag is true for this condition is handled in
   // ShapeFile::addFeatures()
   bool outsideSelected = false;
   AoiElement* pAoi = dynamic_cast<AoiElement*>(mpGraphicElement);
   if (pAoi != NULL)
   {
      const BitMask* pMask = pAoi->getSelectedPoints();
      if (pMask != NULL)
      {
         outsideSelected = pMask->isOutsideSelected();
      }
   }
   GraphicGroup* pGroup = mpGraphicElement->getGroup();
   VERIFY(pGroup != NULL);

   std::list<GraphicObject*> objects;
   // If annotation layer, get only the selected objects.
   if (dynamic_cast<AnnotationLayer*>(pLayer) != NULL)
   {
      pLayer->getSelectedObjects(objects);
   }
   // If no objects selected, or this is an AOI layer, get all objects.
   if (objects.empty())
   {
      objects = pGroup->getObjects();
   }
   if (objects.empty() == true && outsideSelected == false)
   {
      message = "The graphic element does not contain any objects to export.";
      return false;
   }

   if (mpGeoref == NULL)
   {
      message = "Could not identify the georeference to use for export.";
      return false;
   }

   // Add the graphic element's objects to the shape file
   ShapefileTypes::ShapeType shapeType;

   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject == NULL)
      {
         continue;
      }

      ShapefileTypes::ShapeType currentShapeType;

      GraphicObjectType objectType = pObject->getGraphicObjectType();
      switch (objectType)        // The cases in the switch represent the acceptable
                                 // graphic object types in the graphic layer
      {
      case MULTIPOINT_OBJECT:    // Fall through
      case BITMASK_OBJECT:       // Fall through
      default:
         currentShapeType = ShapefileTypes::MULTIPOINT_SHAPE;
         break;

      case LINE_OBJECT:          // Fall through
      case HLINE_OBJECT:         // Fall through
      case VLINE_OBJECT:         // Fall through
      case POLYLINE_OBJECT:
         currentShapeType = ShapefileTypes::POLYLINE_SHAPE;
         break;

      case POLYGON_OBJECT:           // Fall through
      case RECTANGLE_OBJECT:         // Fall through
      case ROUNDEDRECTANGLE_OBJECT:  // Fall through
      case TRIANGLE_OBJECT:          // Fall through
      case ARC_OBJECT:               // Fall through
      case ELLIPSE_OBJECT:
         currentShapeType = ShapefileTypes::POLYGON_SHAPE;
         break;
      }

      if (currentShapeType == shapeType)
      {
         continue;
      }

      if (shapeType.isValid() == true)
      {
         // Default to the multi-point shape if the graphic layer contains mixed object types
         shapeType = ShapefileTypes::MULTIPOINT_SHAPE;
         break;
      }

      shapeType = currentShapeType;
   }

   mShapefile.setShape(shapeType);
   string err;
   mShapefile.addFeatures(mpGraphicElement, NULL, mpGeoref, err);

   return true;
}
