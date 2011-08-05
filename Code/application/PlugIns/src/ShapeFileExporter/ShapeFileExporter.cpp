/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "GraphicGroup.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterElement.h"
#include "ShapeFile.h"
#include "ShapeFileExporter.h"
#include "ShapeFileOptionsWidget.h"
#include "SpatialDataView.h"

#include <QtCore/QString>
#include <QtGui/QMessageBox>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksShapeFileExporter, ShapeFileExporter);

ShapeFileExporter::ShapeFileExporter() : mpAoi(NULL), mpGeoref(NULL), mpLayers(NULL)
{
   setName("Shape File Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{6A4FBFF5-1642-4d66-95DC-AFBB1A7E5B1E}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("Shape Files (*.shp *.shx *.dbf)");
   setSubtype(TypeConverter::toString<AoiLayer>());
   addDependencyCopyright("shapelib",
      "Copyright (c) 1999, Frank Warmerdam<br><br>"
      "This software is available under the following \"MIT Style\" license, or at the option of the licensee "
      "under the LGPL (see LICENSE.LGPL). This option is discussed in more detail in shapelib.html.<br><br>"
      "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated "
      "documentation files (the \"Software\"), to deal in the Software without restriction, including without "
      "limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the "
      "Software, and to permit persons to whom the Software is furnished to do so, subject to the following "
      "conditions:<br><br>"
      "The above copyright notice and this permission notice shall be included in all copies or substantial portions "
      "of the Software.<br><br>"
      "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT "
      "LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO "
      "EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN "
      "AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE "
      "OR OTHER DEALINGS IN THE SOFTWARE.<br>");
}

ShapeFileExporter::~ShapeFileExporter()
{
}

bool ShapeFileExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), NULL, "File descriptor for the output file."));

   if (isBatch())
   {
      VERIFY(pArgList->addArg<AoiElement>("AoiElement", NULL, "The AOI to be exported"));
      VERIFY(pArgList->addArg<RasterElement>("RasterElement", NULL, 
         "Source of georeference for the AOI being exported"));
   }
   else
   {
      VERIFY(pArgList->addArg<AoiLayer>(Exporter::ExportItemArg(), NULL, "The AOI layer to be exported as shape file"));
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

   if (mpOptionsWidget.get() == NULL)  // then need to extract inputs and add mpAoi to mShapefile
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
      vector<Layer*> aoiLayers;
      if (mpLayers != NULL)
      {
         mpLayers->getLayers(AOI_LAYER, aoiLayers);
      }
      vector<AoiElement*> elements;
      for (vector<Layer*>::iterator it = aoiLayers.begin(); it != aoiLayers.end(); ++it)
      {
         AoiElement* pAoi = dynamic_cast<AoiElement*>((*it)->getDataElement());
         if (pAoi != NULL)
         {
            elements.push_back(pAoi);
         }
      }

      mpOptionsWidget.reset(new ShapeFileOptionsWidget(&mShapefile, elements, mpGeoref));
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

   FileDescriptor* pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (pFileDescriptor == NULL)
   {
      message = "No file specified.";
      return false;
   }
   mShapefile.setFilename(pFileDescriptor->getFilename().getFullPathAndName());

   if (isBatch())
   {
      mpAoi = pInArgList->getPlugInArgValue<AoiElement>("AoiElement");
      mpGeoref = pInArgList->getPlugInArgValue<RasterElement>("RasterElement");
   }
   else
   {
      AoiLayer* pLayer = pInArgList->getPlugInArgValue<AoiLayer>(Exporter::ExportItemArg());
      if (pLayer == NULL)
      {
         message = "Input argument list did not include anything to export.";
         return false;
      }

      mpAoi = dynamic_cast<AoiElement*>(pLayer->getDataElement());
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

   if (mpAoi == NULL)
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
   const BitMask* pMask = mpAoi->getSelectedPoints();
   if (mpAoi->getGroup()->getObjects().empty() == true && 
      pMask->isOutsideSelected() == false)
   {
      message = "The AOI does not contain any points to export.";
      return false;
   }

   if (mpGeoref == NULL)
   {
      message = "Could not identify the georeference to use for export.";
      return false;
   }

   //add aoi to shape file
   mShapefile.setShape(ShapefileTypes::MULTIPOINT_SHAPE);
   string err;
   mShapefile.addFeatures(mpAoi, mpGeoref, err);

   return true;
}
