/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "Any.h"
#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "Endian.h"
#include "FeatureClass.h"
#include "FeatureClassWidget.h"
#include "FeatureProxyConnector.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "FileResource.h"
#include "GraphicObject.h"
#include "ImportDescriptor.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "RasterElement.h"
#include "ShapeFileImporter.h"
#include "SpatialDataView.h"
#include "Undo.h"
#include "UtilityServices.h"

#include <vector>
#include <cctype>

#include <QtGui/QLayout>

using namespace std;

const std::string ShapeFileImporter::PLUGIN_NAME = "Shape File Importer";
const std::string ShapeFileImporter::PLUGIN_SUBTYPE = "Shape File";

ShapeFileImporter::ShapeFileImporter() :
   mpProgress(NULL), mpStep(NULL), mpOptionsWidget(NULL)
{
   setName(PLUGIN_NAME);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{7B511DE1-C282-4725-BF87-01CC68DBA6AC}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("Shape Files (*.shp)");
   setSubtype(PLUGIN_SUBTYPE);
   setAbortSupported(false);
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

ShapeFileImporter::~ShapeFileImporter()
{
   delete mpOptionsWidget;
}

vector<ImportDescriptor*> ShapeFileImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   if (getFileAffinity(filename) != Importer::CAN_NOT_LOAD)
   {
      Service<ModelServices> pModel;

      ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(filename, "AnnotationElement", NULL);
      VERIFYRV(pImportDescriptor != NULL, descriptors);

      DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
      VERIFYRV(pDescriptor != NULL, descriptors);

      FactoryResource<FileDescriptor> pFileDescriptor;
      pFileDescriptor->setFilename(filename);
      pDescriptor->setFileDescriptor(pFileDescriptor.get());

      descriptors.push_back(pImportDescriptor);
   }

   return descriptors;
}

unsigned char ShapeFileImporter::getFileAffinity(const std::string& filename)
{
   // Check that the file exists.
   FileResource pFile(filename.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   Endian endian(BIG_ENDIAN);

   // check the magic numbers in a SHP file
   vector<uint32_t> buffer(7);
   fread(&buffer[0], sizeof(uint32_t), buffer.size(), pFile);
   endian.swapBuffer(&buffer[0], buffer.size());

   if (buffer[0] != 9994 || buffer[1] != 0 || buffer[2] != 0 || 
      buffer[3] != 0 || buffer[4] != 0 || buffer[5] != 0)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // check the file size against the size in the SHP file header
   QFileInfo fileInfo(QString::fromStdString(filename));
   if (buffer[6]*2 != fileInfo.size())
   {
      return Importer::CAN_NOT_LOAD;
   }

   QDir dir = fileInfo.dir();
   dir.setFilter(QDir::Files);

   // make sure there are DBF and SHX files associated with the SHP
   if (dir.entryList(QStringList("*.dbf"), QDir::Files, QDir::IgnoreCase).isEmpty())
   {
      return Importer::CAN_NOT_LOAD;
   }

   if (dir.entryList(QStringList("*.shx"), QDir::Files, QDir::IgnoreCase).isEmpty())
   {
      return Importer::CAN_NOT_LOAD;
   }

   return Importer::CAN_LOAD;
}

bool ShapeFileImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   pArgList->addArg<Progress>(ProgressArg(), NULL);
   pArgList->addArg<AnnotationElement>(ImportElementArg(), NULL);
   pArgList->addArg<SpatialDataView>(ViewArg(), NULL);

   return true;
}

bool ShapeFileImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ShapeFileImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run Importer", "app", "F5264701-1D60-474b-AB62-C674A6AC1477");
   mpStep = pStep.get();
   pStep->addProperty("name", getName());

   mpProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
   Progress *pProgress = mpProgress;

   // interactive

   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(ViewArg());
   RasterElement *pRaster = NULL;

   FAIL_IF(pView == NULL, "Could not find view to insert into.", return false);

   AnnotationElement *pAnno = pInArgList->getPlugInArgValue<AnnotationElement>(ImportElementArg());
   FAIL_IF(pAnno == NULL, "Could not find created element.", return false);

   AnnotationLayer *pAnnotationLayer = NULL;
   const LayerList* pLayerList = pView->getLayerList();
   FAIL_IF(pLayerList == NULL, "Could not find layer list.", return false);

   pRaster = pLayerList->getPrimaryRasterElement();

   FAIL_IF(pRaster == NULL, "No data cube could be found.", return false);
   FAIL_IF(!pRaster->isGeoreferenced(), "No georeference could be found.", return false)

   const DataDescriptor *pDescriptor = pAnno->getDataDescriptor();
   FAIL_IF(pDescriptor == NULL, "The descriptor is invalid.", return false);

   createFeatureClassIfNeeded(pDescriptor);
   if (mpFeatureClass.get() == NULL)
   {
      // Progress and Step has been taken care of
      return false;
   }

   if (mpOptionsWidget != NULL)
   {
      mpOptionsWidget->applyChanges();
   }

   const ArcProxyLib::ConnectionParameters &connect = mpFeatureClass->getConnectionParameters();
   pStep->addProperty("connectionParameters", connect.toString());

   vector<ArcProxyLib::ConnectionType> availableConnections = getAvailableConnectionTypes();
   FAIL_IF(find(availableConnections.begin(), availableConnections.end(), 
      connect.getConnectionType()) == availableConnections.end(), 
      "The selected connection type is not available.", return false)

   Service<ModelServices> pModel;
   FAIL_IF(!pModel->setElementParent(pAnno, pRaster) || !pModel->setElementName(pAnno, connect.getFeatureClass()),
      "This shape file has already been imported", return false)

   FAIL_IF(pAnno->setGeocentric(true) == false, "Could not set the element to geocentric.", return false)

   UndoGroup group(pView, "Set Object Properties");

   string layerName = mpFeatureClass->getLayerName();

   pAnnotationLayer = static_cast<AnnotationLayer*>(pLayerList->getLayer(ANNOTATION, pAnno, layerName));
   if(pAnnotationLayer == NULL)
   {
      pAnnotationLayer = static_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, pAnno, layerName));
   }
   if (pAnnotationLayer != NULL && mpFeatureClass->hasLabels())
   {
      pAnnotationLayer->setShowLabels(true);
   }

   pAnnotationLayer->setLayerLocked(true);

   ModelResource<Any> pAny("Geographic feature", pAnno, "FeatureClass");

   mpFeatureClass->setParentElement(pAnno);

   if (!mpFeatureClass->open(mMessageText) || !mpFeatureClass->update(mpProgress, mMessageText))
   {
      if(mpProgress) mpProgress->updateProgress("Error: " + mMessageText, 0, ERRORS);
      pStep->finalize(Message::Failure, mMessageText);
      return false;
   }

   pAny->setData(mpFeatureClass.release());
   pAny.release();
   mpProgress->updateProgress("Complete", 100, NORMAL);
   pStep->finalize(Message::Success);

   return true;
}

QWidget *ShapeFileImporter::getImportOptionsWidget(DataDescriptor *pDescriptor)
{
   createFeatureClassIfNeeded(pDescriptor);
   if (mpFeatureClass.get() == NULL)
   {
      return NULL;
   }

   if (mpOptionsWidget == NULL)
   {
      mpOptionsWidget = new FeatureClassWidget;
   }

   VERIFYRV(mpOptionsWidget != NULL, NULL);

   mpOptionsWidget->initialize(mpFeatureClass.get());
   mpOptionsWidget->setAvailableConnectionTypes(getAvailableConnectionTypes());

   QLayout *pLayout = mpOptionsWidget->layout();
   if (pLayout != NULL)
   {
      pLayout->setMargin(10);
   }

   return mpOptionsWidget;
}

bool ShapeFileImporter::validate(const DataDescriptor *pDescriptor, std::string &errorMessage) const
{
   VERIFY(pDescriptor != NULL);

   if (pDescriptor->getProcessingLocation() != IN_MEMORY)
   {
      errorMessage = "Processing location must be \"In Memory\"";
      return false;
   }

   return true;
}

void ShapeFileImporter::createFeatureClassIfNeeded(const DataDescriptor *pDescriptor)
{
   if (mpFeatureClass.get() == NULL)
   {
      vector<ArcProxyLib::ConnectionType> types = getAvailableConnectionTypes();
      if (types.empty())
      {
         mMessageText = "Cannot find a valid connection type.";
         if(mpProgress) mpProgress->updateProgress(mMessageText, 0, ERRORS);
         if(mpStep) mpStep->finalize(Message::Failure, mMessageText);
         return;
      }

      VERIFYNRV(pDescriptor != NULL);
      const FileDescriptor *pFileDescriptor = pDescriptor->getFileDescriptor();
      VERIFYNRV(pFileDescriptor != NULL);
      const Filename &filename = pFileDescriptor->getFilename();

      mpFeatureClass.reset(new FeatureClass);

      ArcProxyLib::ConnectionParameters connect = mpFeatureClass->getConnectionParameters();
      connect.setDatabase(filename.getPath());
      connect.setFeatureClass(filename.getFileName());

      connect.setConnectionType(types.front());

      mpFeatureClass->setConnectionParameters(connect);
   }
}

std::vector<ArcProxyLib::ConnectionType> ShapeFileImporter::getAvailableConnectionTypes()
{
   vector<ArcProxyLib::ConnectionType> types;

   FeatureProxyConnector *pProxy = FeatureProxyConnector::instance();
   VERIFYRV(pProxy != NULL, types);

   types = pProxy->getAvailableConnectionTypes();
   vector<ArcProxyLib::ConnectionType>::iterator sdeIter = 
      find(types.begin(), types.end(), ArcProxyLib::SDE_CONNECTION);
   if (sdeIter != types.end())
   {
      types.erase(sdeIter);
   }

   return types;
}
