/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "RasterElement.h"
#include "ShapeFile.h"
#include "ShapeFileDlg.h"
#include "ShapeFileExporter.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

using namespace std;

ShapeFileExporter::ShapeFileExporter()
{
   mbInteractive = false;

   setName("Shape File Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{6A4FBFF5-1642-4d66-95DC-AFBB1A7E5B1E}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("Shape Files (*.shp *.shx *.dbf)");
   setSubtype(TypeConverter::toString<AoiElement>());
   addDependencyCopyright("shapelib",
      "Copyright (c) 1999, Frank Warmerdam<br>"
      "<br>"
      "This software is available under the following \"MIT Style\" license, or at the option of the licensee under the LGPL (see LICENSE.LGPL). This option is discussed in more detail in shapelib.html.<br>"
      "<br>"
      "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:<br>"
      "<br>"
      "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.<br>"
      "<br>"
      "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.<br>");
}

ShapeFileExporter::~ShapeFileExporter()
{
}

bool ShapeFileExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);
   
   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ExportDescriptorArg());
   pArg->setType("FileDescriptor");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   if(!mbInteractive)
   {
      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName(ExportItemArg());
      pArg->setType("AoiElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool ShapeFileExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ShapeFileExporter::setBatch()
{
   mbInteractive = false;
   return true;
}

bool ShapeFileExporter::setInteractive()
{
   mbInteractive = true;
   return true;
}

bool ShapeFileExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Shape File Exporter", "app", "EA0C2EEB-8A9E-4017-8456-DB316B0742CC");

   // Progress
   Progress* pProgress = NULL;
   if(pInArgList != NULL)
   {
      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
   }

   // File Descriptor
   FileDescriptor* pFileDescriptor = NULL;
   if(pInArgList != NULL)
   {
      pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
   }
   if(pFileDescriptor == NULL)
   {
      string msg = "No file specified.";
      if(pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   // Create the shape file
   ShapeFile shapeFile;

   if(mbInteractive)
   {
      // Get the current view name from desktop services
      string filename = pFileDescriptor->getFilename().getFullPathAndName();
      if(filename.empty())
      {
         string message = "An active spatial data window does not exist!";
         if(pProgress != NULL) pProgress->updateProgress(message, 0, ERRORS);
         pStep->finalize(Message::Failure, message);
         return false;
      }

      pStep->addProperty("sourceFile", filename);

      // Set the filename in the shape file
      shapeFile.setFilename(filename);

      // Display the dialog for the user to add shapes and attribute values
      Service<DesktopServices> pDesktop;
      QWidget* pParent = pDesktop->getMainWidget();

      ShapeFileDlg dlg(&shapeFile, pParent);

      if(dlg.exec() == QDialog::Rejected)
      {
         pStep->finalize(Message::Failure, "Exporter cancelled by user.");
         return false;
      }
   }
   else
   {
      // AOI
      AoiElement* pAoi = NULL;
      if(pInArgList != NULL)
      {
         pAoi = pInArgList->getPlugInArgValue<AoiElement>(ExportItemArg());
      }

      if (pAoi == NULL)
      {
         string message = "The AOI input value is invalid!";
         if(pProgress != NULL) pProgress->updateProgress(message, 0, ERRORS);
         pStep->finalize(Message::Failure, message);
         return false;
      }

      string filename = pFileDescriptor->getFilename().getFullPathAndName();
      pStep->addProperty("sourceFile", filename);

      // Add the AOI to the shape file
      shapeFile.setFilename(filename);
      shapeFile.setShape(MULTIPOINT_SHAPE);
      shapeFile.addFeatures(pAoi);
   }

   // Save the shape file
   string message = "";
   bool bSuccess = shapeFile.save(pProgress, message);
   if (bSuccess == true)
   {
      message = "Shape file export complete!";
      pStep->finalize(Message::Success);
   }
   else
   {
      pStep->finalize(Message::Failure, message);
   }

   return bSuccess;
}
