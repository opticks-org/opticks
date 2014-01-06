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
#include "OptionsGeographicFeatures.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SessionResource.h"
#include "ShapeFileImporter.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "Undo.h"
#include "UtilityServices.h"

#include <algorithm>
#include <vector>
#include <cctype>

#include <QtGui/QLayout>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksGeographicFeatures, ShapeFileImporter);

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
   addDependencyCopyright("shapelib", Service<UtilityServices>()->getTextFromFile(":/licenses/shapelib"));
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

   Endian endian(BIG_ENDIAN_ORDER);

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

bool ShapeFileImporter::validate(const DataDescriptor* pDescriptor,
                                 const vector<const DataDescriptor*>& importedDescriptors,
                                 string& errorMessage) const
{
   if (ImporterShell::validate(pDescriptor, importedDescriptors, errorMessage) == false)
   {
      return false;
   }

   // Get the feature class
   const FeatureClass* pFeatureClass = NULL;
   if (mpOptionsWidget != NULL)
   {
      pFeatureClass = mpOptionsWidget->getEditFeatureClass();
   }

   if (pFeatureClass == NULL)
   {
      pFeatureClass = mpFeatureClass.get();
   }

   if (pFeatureClass != NULL)
   {
      // Check for a valid connection type
      ArcProxyLib::ConnectionType connection = pFeatureClass->getConnectionParameters().getConnectionType();
      if (connection == ArcProxyLib::UNKNOWN_CONNECTION)
      {
         errorMessage = "The connection type is invalid.";
         return false;
      }

      // Check for a valid ArcGIS license
      if ((connection == ArcProxyLib::SHP_CONNECTION) || (connection == ArcProxyLib::SDE_CONNECTION))
      {
         FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
         VERIFY(pProxy != NULL);

         if (pProxy->isProcessInitialized() == true)
         {
            vector<ArcProxyLib::ConnectionType> connections = pProxy->getAvailableConnectionTypes();
            if (find(connections.begin(), connections.end(), connection) == connections.end())
            {
               errorMessage = "A valid ArcGIS license could not be obtained to load the shape file.";
               return false;
            }
         }
         else if (mpOptionsWidget != NULL)
         {
            // The feature proxy connector has not yet attempted to get a valid ArcGIS license, so display
            // a warning in the import options widget only indicating that a valid license is required
            errorMessage = "The selected connection type requires a valid ArcGIS license that will be "
               "obtained when loading the shape file, changing to the Display tab or Clipping tab, or "
               "clicking the Test Connection button.";
         }
      }
   }

   return true;
}

bool ShapeFileImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription());
   pArgList->addArg<AnnotationElement>(Importer::ImportElementArg(), NULL, "Shapefile to be imported.");
   pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL,
      "View in which the imported shapefile will be inserted.");

   return true;
}

bool ShapeFileImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ShapeFileImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   // prevent session auto save while importing shape file
   SessionSaveLock lock;

   StepResource pStep("Run Importer", "app", "F5264701-1D60-474b-AB62-C674A6AC1477");
   mpStep = pStep.get();
   pStep->addProperty("name", getName());

   mpProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   Progress* pProgress = mpProgress;

   // interactive

   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   RasterElement* pRaster = NULL;

   FAIL_IF(pView == NULL, "Could not find view to insert into.", return false);

   // The redo actions associated with an undo group for importing a shape file will not work. The FeatureClass
   // associated with the annotation element for the shape file layer is lost when the group undo is performed
   // and can not be restored with the redo actions. Set undo lock for the import and to prevent problems, clear the
   // undo stack if import was successful.
   UndoLock undoLock(pView);

   AnnotationElement* pAnno = pInArgList->getPlugInArgValue<AnnotationElement>(Importer::ImportElementArg());
   FAIL_IF(pAnno == NULL, "Could not find created element.", return false);

   AnnotationLayer* pAnnotationLayer = NULL;
   const LayerList* pLayerList = pView->getLayerList();
   FAIL_IF(pLayerList == NULL, "Could not find layer list.", return false);

   pRaster = pLayerList->getPrimaryRasterElement();

   FAIL_IF(pRaster == NULL, "No data cube could be found.", return false);
   FAIL_IF(!pRaster->isGeoreferenced(), "No georeference could be found.", return false)

   const DataDescriptor* pDescriptor = pAnno->getDataDescriptor();
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

   const ArcProxyLib::ConnectionParameters& connect = mpFeatureClass->getConnectionParameters();
   pStep->addProperty("connectionParameters", connect.toString());

   vector<ArcProxyLib::ConnectionType> availableConnections = getAvailableConnectionTypes();
   FAIL_IF(find(availableConnections.begin(), availableConnections.end(), 
      connect.getConnectionType()) == availableConnections.end(), 
      "The selected connection type is not available.", return false)

   // Create annotation element, incrementing requested name by 1, until
   // annotation element can be created.
   Service<ModelServices> pModel;
   std::string featureClassName = connect.getFeatureClass();
   std::string newElementName = featureClassName;
   int limit = 1;
   bool found = true;
   found = pModel->getElement(featureClassName, TypeConverter::toString<AnnotationElement>(), pRaster);
   while ((found) && (limit < 1000))
   {
      ++limit;
      newElementName = featureClassName + StringUtilities::toDisplayString<int>(limit);
      DataElement* pElement = pModel->getElement(newElementName, TypeConverter::toString<AnnotationElement>(), pRaster);
      if (pElement == NULL)
      {
         found = false;
      }
   }
   VERIFY(pModel->setElementParent(pAnno, pRaster) && pModel->setElementName(pAnno, newElementName));

   FAIL_IF(pAnno->setGeocentric(true) == false, "Could not set the element to geocentric.", return false)

   string layerName = mpFeatureClass->getLayerName();

   // Create the feature class element before creating the layer to ensure that the feature class is available
   // to the Geographic Features Window when the layer is added to the view
   ModelResource<Any> pAny("Geographic feature", pAnno, "FeatureClass");
   mpFeatureClass->setParentElement(pAnno);
   pAny->setData(mpFeatureClass.release());

   // Need to get the FeatureClass pointer again since release() NULLs the pointer value
   FeatureClass* pFeatureClass = dynamic_cast<FeatureClass*>(pAny->getData());

   pAnnotationLayer = static_cast<AnnotationLayer*>(pLayerList->getLayer(ANNOTATION, pAnno, layerName));
   if (pAnnotationLayer == NULL)
   {
      // Create layer with requested layer name.
      pAnnotationLayer = static_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, pAnno, layerName));
   }
   if (pAnnotationLayer != NULL && pFeatureClass->hasLabels())
   {
      pAnnotationLayer->setShowLabels(true);
   }

   pAnnotationLayer->setLayerLocked(true);

   if (!pFeatureClass->open(mMessageText) || !pFeatureClass->update(mpProgress, mMessageText, false))
   {
      if (mpProgress)
      {
         mpProgress->updateProgress("Error: " + mMessageText, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessageText);
      return false;
   }

   pAny.release();
   mpProgress->updateProgress("Complete", 100, NORMAL);
   pStep->finalize(Message::Success);

   // import was successful, so clear the undo stack
   pView->clearUndo();

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

   QLayout* pLayout = mpOptionsWidget->layout();
   if (pLayout != NULL)
   {
      pLayout->setMargin(10);
   }

   return mpOptionsWidget;
}

void ShapeFileImporter::createFeatureClassIfNeeded(const DataDescriptor *pDescriptor)
{
   if (mpFeatureClass.get() == NULL)
   {
      vector<ArcProxyLib::ConnectionType> types = getAvailableConnectionTypes();
      if (types.empty())
      {
         mMessageText = "Cannot find a valid connection type.";
         if (mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }

         if (mpStep)
         {
            mpStep->finalize(Message::Failure, mMessageText);
         }

         return;
      }

      VERIFYNRV(pDescriptor != NULL);
      const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
      VERIFYNRV(pFileDescriptor != NULL);
      const Filename& filename = pFileDescriptor->getFilename();

      FeatureClass* pFeatureClass = new FeatureClass();
      mpFeatureClass.reset(pFeatureClass);

      ArcProxyLib::ConnectionParameters connect = mpFeatureClass->getConnectionParameters();
      connect.setDatabase(filename.getPath());
      connect.setFeatureClass(filename.getFileName());

      ArcProxyLib::ConnectionType connectionType = ArcProxyLib::UNKNOWN_CONNECTION;
      if (OptionsGeographicFeatures::getSettingUseArcAsDefaultConnection() == true)
      {
         if (find(types.begin(), types.end(), ArcProxyLib::SHP_CONNECTION) != types.end())
         {
            connectionType = ArcProxyLib::SHP_CONNECTION;
         }
      }
      else if (find(types.begin(), types.end(), ArcProxyLib::SHAPELIB_CONNECTION) != types.end())
      {
         connectionType = ArcProxyLib::SHAPELIB_CONNECTION;
      }

      connect.setConnectionType(connectionType);

      mpFeatureClass->setConnectionParameters(connect);
   }
}

std::vector<ArcProxyLib::ConnectionType> ShapeFileImporter::getAvailableConnectionTypes()
{
   vector<ArcProxyLib::ConnectionType> types;

   FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
   VERIFYRV(pProxy != NULL, types);

   // Do not query the actual available connection types if ArcProxy has not yet been started
   // to avoid potentially getting an Arc license unnecessarily
   if (pProxy->isProcessInitialized() == true)
   {
      types = pProxy->getAvailableConnectionTypes();
      vector<ArcProxyLib::ConnectionType>::iterator sdeIter =
         find(types.begin(), types.end(), ArcProxyLib::SDE_CONNECTION);
      if (sdeIter != types.end())
      {
         types.erase(sdeIter);
      }
   }
   else
   {
      types.push_back(ArcProxyLib::SHAPELIB_CONNECTION);
      types.push_back(ArcProxyLib::SHP_CONNECTION);
   }

   return types;
}
