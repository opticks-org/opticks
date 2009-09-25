/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QString>

#include "ApplicationServices.h"
#include "AppVerify.h"
#include "ConfigurationSettingsImp.h"
#include "DataDescriptor.h"
#include "DataDescriptorImp.h"
#include "DataElement.h"
#include "DateTimeImp.h"
#include "DesktopServices.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "ImportAgentImp.h"
#include "ImportDescriptor.h"
#include "ImportDlg.h"
#include "Importer.h"
#include "ImportOptionsDlg.h"
#include "ModelServices.h"
#include "MruFile.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"

#include <algorithm>
#include <map>
#include <queue>
using namespace std;

namespace
{
   struct ElementDepthComparator : less<vector<string>::size_type>
   {
      bool operator()(ImportDescriptor* pImportDescriptor1, ImportDescriptor* pImportDescriptor2)
      {
         if (pImportDescriptor1 == NULL || pImportDescriptor2 == NULL)
         {
            return false;
         }

         DataDescriptor* pDataDescriptor1 = pImportDescriptor1->getDataDescriptor();
         DataDescriptor* pDataDescriptor2 = pImportDescriptor2->getDataDescriptor();
         if (pDataDescriptor1 == NULL || pDataDescriptor2 == NULL)
         {
            return false;
         }

         return less<vector<string>::size_type>::operator()(pDataDescriptor1->getParentDesignator().size(),
            pDataDescriptor2->getParentDesignator().size());
      }
   };
};

void ImportAgentImp::instantiate(Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw logic_error("ImportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(pProgress, batch);
   mpElement = NULL;
}

void ImportAgentImp::instantiate(const string& importerName, Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw logic_error("ImportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
}

void ImportAgentImp::instantiate(const string& importerName, const string& filename, Progress* pProgress,
                                 bool batch)
{
   if (getInstantiated())
   {
      throw logic_error("ImportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
   setFilename(filename);
}

void ImportAgentImp::instantiate(const string& importerName, const vector<string>& filenames, Progress* pProgress,
                                 bool batch)
{
   if (getInstantiated())
   {
      throw logic_error("ImportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
   setFilenames(filenames);
}

void ImportAgentImp::instantiate(const string& importerName,
                                 const map<string, vector<ImportDescriptor*> >& datasets, Progress* pProgress,
                                 bool batch)
{
   if (getInstantiated())
   {
      throw logic_error("ImportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
   setDatasets(datasets);
}

void ImportAgentImp::instantiate(PlugIn* pPlugIn, const map<string, vector<ImportDescriptor*> >& descriptors,
                                 Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw logic_error("ImportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(pPlugIn, string(), pProgress, batch);
   mpElement = NULL;
   setDatasets(descriptors);
}

ImportAgentImp::ImportAgentImp() :
   mEditType(ImportAgent::NEVER_EDIT),
   mUpdateMruList(false),
   mpElement(NULL)
{}

ImportAgentImp::~ImportAgentImp()
{
   Service<ModelServices> pModel;

   for (QMap<QString, vector<ImportDescriptor*> >::iterator fileIter = mDatasets.begin();
      fileIter != mDatasets.end();
      ++fileIter)
   {
      vector<ImportDescriptor*> descriptors = fileIter.value();
      for (vector<ImportDescriptor*>::iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter)
      {
         if (*iter != NULL)
         {
            pModel->destroyImportDescriptor(*iter);
         }
      }
   }
}

void ImportAgentImp::setImporterSubtype(const string& subtype)
{
   checkInstantiate();

   Service<ApplicationServices> pApp;
   if (pApp->isInteractive() == true)
   {
      mImporterSubtype = subtype;
   }
}

string ImportAgentImp::getImporterSubtype() const
{
   checkInstantiate();
   return mImporterSubtype;
}

void ImportAgentImp::setFilename(const string& filename)
{
   checkInstantiate();
   setDatasets(map<string, vector<ImportDescriptor*> >());    // Delete any existing descriptors

   QString strFilename = QString::fromStdString(filename);
   strFilename.replace(QRegExp("\\\\"), "/");

   if (strFilename.isEmpty() == false)
   {
      mDatasets[strFilename] = vector<ImportDescriptor*>();
   }
}

void ImportAgentImp::setFilenames(const vector<string>& filenames)
{
   checkInstantiate();
   setDatasets(map<string, vector<ImportDescriptor*> >());    // Delete any existing descriptors

   for (vector<string>::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter)
   {
      QString filename = QString::fromStdString(*iter);
      filename.replace(QRegExp("\\\\"), "/");

      if (filename.isEmpty() == false)
      {
         mDatasets[filename] = vector<ImportDescriptor*>();
      }
   }
}

void ImportAgentImp::setEditType(ImportAgent::EditType editType)
{
   checkInstantiate();

   Service<ApplicationServices> pApp;
   if (pApp->isInteractive() == true)
   {
      mEditType = editType;
   }
}

ImportAgent::EditType ImportAgentImp::getEditType() const
{
   checkInstantiate();
   return mEditType;
}

void ImportAgentImp::updateMruFileList(bool updateList)
{
   checkInstantiate();
   mUpdateMruList = updateList;
}

bool ImportAgentImp::isMruFileListUpdated() const
{
   checkInstantiate();
   return mUpdateMruList;
}

void ImportAgentImp::setDatasets(const map<string, vector<ImportDescriptor*> >& descriptors)
{
   checkInstantiate();
   Service<ModelServices> pModel;

   // Destroy any descriptors not in the new descriptors
   vector<ImportDescriptor*> originalDescriptors;
   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      vector<ImportDescriptor*>& fileDescriptors = iter.value();
      if (fileDescriptors.empty() == false)
      {
         copy(fileDescriptors.begin(), fileDescriptors.end(), back_inserter(originalDescriptors));
      }
   }

   vector<ImportDescriptor*> newDescriptors;
   for (map<string, vector<ImportDescriptor*> >::const_iterator iter = descriptors.begin();
      iter != descriptors.end();
      ++iter)
   {
      vector<ImportDescriptor*> fileDescriptors = iter->second;
      if (fileDescriptors.empty() == false)
      {
         copy(fileDescriptors.begin(), fileDescriptors.end(), back_inserter(newDescriptors));
      }
   }

   for (vector<ImportDescriptor*>::iterator iter = originalDescriptors.begin();
      iter != originalDescriptors.end();
      ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         vector<ImportDescriptor*>::iterator newIter = find(newDescriptors.begin(), newDescriptors.end(),
            pImportDescriptor);
         if (newIter == newDescriptors.end())
         {
            pModel->destroyImportDescriptor(pImportDescriptor);
         }
      }
   }

   mDatasets.clear();

   for (map<string, vector<ImportDescriptor*> >::const_iterator iter = descriptors.begin();
      iter != descriptors.end();
      ++iter)
   {
      QString filename = QString::fromStdString(iter->first);
      filename.replace(QRegExp("\\\\"), "/");
      mDatasets[filename] = iter->second;
   }
}

vector<ImportDescriptor*> ImportAgentImp::getImportDescriptors()
{
   checkInstantiate();
   vector<ImportDescriptor*> importDescriptors;

   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      QString filename = iter.key();
      VERIFYRV(filename.isEmpty() == false, vector<ImportDescriptor*>());

      vector<ImportDescriptor*>& descriptors = iter.value();
      if (descriptors.empty() == true)
      {
         Importer* pImporter = dynamic_cast<Importer*>(getPlugIn());
         if (pImporter != NULL)
         {
            descriptors = pImporter->getImportDescriptors(filename.toStdString());
         }
      }

      if (descriptors.empty() == false)
      {
         copy(descriptors.begin(), descriptors.end(), back_inserter(importDescriptors));
      }
   }

   return importDescriptors;
}

vector<ImportDescriptor*> ImportAgentImp::getImportDescriptors(const std::string& filename)
{
   checkInstantiate();

   QString mapFilename = QString::fromStdString(filename);
   mapFilename.replace(QRegExp("\\\\"), "/");

   vector<ImportDescriptor*> importDescriptors;
   if (mapFilename.isEmpty() == true)
   {
      return importDescriptors;
   }

   QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.find(mapFilename);
   if (iter != mDatasets.end())
   {
      vector<ImportDescriptor*>& currentDescriptors = iter.value();
      if (currentDescriptors.empty() == true)
      {
         Importer* pImporter = dynamic_cast<Importer*>(getPlugIn());
         if (pImporter != NULL)
         {
            currentDescriptors = pImporter->getImportDescriptors(mapFilename.toStdString());
         }
      }

      importDescriptors = currentDescriptors;
   }

   return importDescriptors;
}

map<string, vector<ImportDescriptor*> > ImportAgentImp::getDatasets()
{
   checkInstantiate();

   map<string, vector<ImportDescriptor*> > datasets;
   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      QString filename = iter.key();
      if (filename.isEmpty() == true)
      {
         return map<string, vector<ImportDescriptor*> >();
      }

      vector<ImportDescriptor*>& descriptors = iter.value();
      if (descriptors.empty() == true)
      {
         Importer* pImporter = dynamic_cast<Importer*>(getPlugIn());
         if (pImporter != NULL)
         {
            descriptors = pImporter->getImportDescriptors(filename.toStdString());
         }
      }

      datasets[filename.toStdString()] = descriptors;
   }

   return datasets;
}

string ImportAgentImp::getDefaultExtensions() const
{
   checkInstantiate();
   string extensions;

   const Importer* pImporter = dynamic_cast<const Importer*>(getPlugIn());
   if (pImporter != NULL)
   {
      extensions = pImporter->getDefaultExtensions();
   }

   return extensions;
}

bool ImportAgentImp::execute()
{
   checkInstantiate();
   Service<ApplicationServices> pApp;
   Service<ModelServices> pModel;

   Importer* pImporter = dynamic_cast<Importer*>(getPlugIn());
   vector<ImportDescriptor*> importDescriptors = getImportDescriptors();

   if (pApp->isBatch() == true)
   {
      // Use the Auto Importer if an importer has not been set
      if (pImporter == NULL)
      {
         if (importDescriptors.empty() == false)
         {
            return false;
         }

         setPlugIn("Auto Importer");
         pImporter = dynamic_cast<Importer*>(getPlugIn());

         // Update the data sets to import if necessary
         importDescriptors = getImportDescriptors();
      }
   }
   else if (mDatasets.empty() == true)
   {
      Service<DesktopServices> pDesktop;

      // Get the import information from the user
      string importerName;    // Will default to Auto Importer in the import dialog if no plug-in is available

      const PlugIn* pPlugIn = getPlugIn();
      if (pPlugIn != NULL)
      {
         importerName = pPlugIn->getName();
      }

      ImportDlg importDlg(QString::fromStdString(mImporterSubtype), QString::fromStdString(importerName),
         pDesktop->getMainWidget());
      if (importDlg.exec() == QDialog::Rejected)
      {
         return false;
      }

      // Importer
      pImporter = importDlg.getSelectedImporter();
      setPlugIn(dynamic_cast<PlugIn*>(pImporter));

      // Data sets
      mDatasets = importDlg.getImportDescriptors();
      importDescriptors = getImportDescriptors();
   }
   else
   {
      // Use the Auto Importer if an importer has not been set
      if (pImporter == NULL)
      {
         if (importDescriptors.empty() == false)
         {
            return false;
         }

         setPlugIn("Auto Importer");
         pImporter = dynamic_cast<Importer*>(getPlugIn());

         // Update the data sets to import if necessary
         importDescriptors = getImportDescriptors();
      }

      // Validate the descriptors
      string errorMessage;
      unsigned int numValidDescriptors = validateImportDescriptors(importDescriptors, pImporter, errorMessage);

      // Display the options dialog if necessary
      unsigned int numImportedDatasets = 0;
      for (vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         if ((pImportDescriptor != NULL) && (pImportDescriptor->isImported() == true))
         {
            ++numImportedDatasets;
         }
      }

      if ((pImporter != NULL) && ((mEditType == ImportAgent::ALWAYS_EDIT) ||
         ((mEditType == ImportAgent::AS_NEEDED_EDIT) && (numValidDescriptors < numImportedDatasets))))
      {
         Service<DesktopServices> pDesktop;
         vector<ImportDescriptor*> originalDescriptors = importDescriptors;

         ImportOptionsDlg optionsDlg(pImporter, mDatasets, pDesktop->getMainWidget());
         if (optionsDlg.exec() == QDialog::Rejected)
         {
            return false;
         }

         // Update the import descriptors to load
         importDescriptors.clear();

         for (vector<ImportDescriptor*>::iterator iter = originalDescriptors.begin();
            iter != originalDescriptors.end();
            ++iter)
         {
            ImportDescriptor* pImportDescriptor = *iter;
            if (pImportDescriptor != NULL)
            {
               if (pImportDescriptor->isImported() == true)
               {
                  importDescriptors.push_back(pImportDescriptor);
               }
            }
         }
      }
      else if (numValidDescriptors == 0)
      {
         Progress* pProgress = getProgress();
         if ((pProgress != NULL) && (errorMessage.empty() == false))
         {
            createProgressDialog();
            pProgress->updateProgress(errorMessage, 0, ERRORS);
         }

         return false;
      }
   }

   if (pImporter == NULL)
   {
      return false;
   }

   if (importDescriptors.empty() == true)
   {
      return false;
   }

   Progress* pProgress = getProgress();
   createProgressDialog();

   // Create the data elements for all the selected data sets
   vector<ImportDescriptor*> selectedDescriptors;
   for (vector<ImportDescriptor*>::iterator iter = importDescriptors.begin(); iter != importDescriptors.end(); ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor == NULL || !pImportDescriptor->isImported())
      {
         continue;
      }

      DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
      if (pDescriptor == NULL)
      {
         continue;
      }

      // Allow the importer to modify the data descriptor if necessary
      pImporter->polishDataDescriptor(pDescriptor);

      string errorMessage;
      if (pImporter->validate(pDescriptor, errorMessage) == false)
      {
         if (pProgress != NULL)
         {
            if (errorMessage.empty() == false)
            {
               errorMessage += "\n";
            }

            string datasetName = pDescriptor->getName();
            if (datasetName.empty() == false)
            {
               errorMessage += "The '" + datasetName + "' data set cannot be loaded.\n";
            }
            else
            {
               errorMessage += "The data set cannot be loaded.\n";
            }

            pProgress->updateProgress(errorMessage, 0, ERRORS);
         }

         continue;
      }
      else if (!errorMessage.empty())
      {
         // warning from validate
         if (pProgress != NULL)
         {
            int currentPercent;
            string currentText;
            ReportingLevel currentLevel;
            pProgress->getProgress(currentText, currentPercent, currentLevel);
            pProgress->updateProgress(errorMessage, currentPercent, WARNING);
         }
      }

      selectedDescriptors.push_back(pImportDescriptor);
   }

   map<DataDescriptor*, ImportDescriptor*> selectedDatasets;
   vector<DataElement*> createdElementsVec;

   stable_sort(selectedDescriptors.begin(), selectedDescriptors.end(), ElementDepthComparator());
   for (vector<ImportDescriptor*>::iterator iter = selectedDescriptors.begin();
      iter != selectedDescriptors.end();
      ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         DataElement* pElement = pModel->createElement(pImportDescriptor->getDataDescriptor());
         if (pElement != NULL)
         {
            DataDescriptor* pDescriptor = pElement->getDataDescriptor();
            if (pDescriptor != NULL)
            {
               selectedDatasets[pDescriptor] = pImportDescriptor;
               createdElementsVec.push_back(pElement);
            }
         }
      }
   }

   if (createdElementsVec.size() < selectedDescriptors.size())
   {
      if (pProgress != NULL)
      {
         if (createdElementsVec.empty())
         {
            pProgress->updateProgress("Unable to create any data elements", 0, ERRORS);
            return false;
         }
         else
         {
            pProgress->updateProgress("Unable to create some of the data elements", 0, ERRORS);
         }
      }
   }

   list<DataElement*> createdElements;
   copy(createdElementsVec.begin(), createdElementsVec.end(), back_inserter(createdElements));

   // build a queue of elements to import
   queue<DataElement*> toVisit;
   for (list<DataElement*>::iterator elmnt = createdElements.begin(); elmnt != createdElements.end(); ++elmnt)
   {
      DataElement* pElement = *elmnt;
      if (pElement != NULL)
      {
         // does the element's parent need to be imported?
         if (find(createdElements.begin(), createdElements.end(), pElement->getParent()) == createdElements.end())
         {
            // if not, add it to the visit queue
            toVisit.push(pElement);
         }
      }
   }

   // Load the data sets
   bool success = false;
   map<ImportDescriptor*, bool> importedDescriptors;

   while (!toVisit.empty())
   {
      mpElement = toVisit.front();
      toVisit.pop();
      createdElements.remove(mpElement);

      bool plugInSuccess = ExecutableAgentImp::executePlugIn();

      DataDescriptor* pElementDescriptor = mpElement->getDataDescriptor();
      if (pElementDescriptor != NULL)
      {
         map<DataDescriptor*, ImportDescriptor*>::iterator iter = selectedDatasets.find(pElementDescriptor);
         if (iter != selectedDatasets.end())
         {
            importedDescriptors[iter->second] = plugInSuccess;
         }
      }

      if (plugInSuccess == false)
      {
         if (pProgress != NULL)
         {
            string text;
            int percent;
            ReportingLevel gran;
            pProgress->getProgress(text, percent, gran);
            if (gran != ABORT)
            {
               string message = "The '" + mpElement->getName() + "' data set failed to load!";
               pProgress->updateProgress(message, 0, WARNING);
            }
         }
         pModel->destroyElement(mpElement);
      }
      else
      {
         DataDescriptorImp* pDdi = dynamic_cast<DataDescriptorImp*>(mpElement->getDataDescriptor());
         if (pDdi != NULL)
         {
            pDdi->setImporterName(getPlugIn()->getName());
         }
         mImportedElements.push_back(mpElement);
         for (list<DataElement*>::iterator elmnt = createdElements.begin(); elmnt != createdElements.end(); ++elmnt)
         {
            if ((*elmnt)->getParent() == mpElement)
            {
               toVisit.push(*elmnt);
            }
         }

         success = true;
      }

      mpElement = NULL;
   }

   if (success == true)
   {
      updateMruFileList(importedDescriptors);
   }

   return success;
}

vector<DataElement*> ImportAgentImp::getImportedElements() const
{
   checkInstantiate();
   return mImportedElements;
}

unsigned int ImportAgentImp::validateImportDescriptors(const vector<ImportDescriptor*>& descriptors,
                                                       Importer* pImporter, string& errorMessage)
{
   if ((descriptors.empty() == true) || (pImporter == NULL))
   {
      return 0;
   }

   errorMessage.clear();

   unsigned int numValid = 0;
   for (vector<ImportDescriptor*>::const_iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         // At least one data set must be selected for import
         if (pImportDescriptor->isImported() == true)
         {
            DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();

            string currentError;
            bool validDescriptor = pImporter->validate(pDescriptor, currentError);
            if (currentError.empty() == false)
            {
               if (errorMessage.empty() == false)
               {
                  errorMessage += "\n\n";
               }

               const string& datasetName = pDescriptor->getName();
               errorMessage += datasetName + string(":\n") + currentError;
            }

            if (validDescriptor == true)
            {
               numValid++;
            }
         }
      }
   }

   return numValid;
}

void ImportAgentImp::populateArgValues(PlugInArgList *pArgList)
{
   checkInstantiate();
   if (pArgList != NULL)
   {
      ExecutableAgentImp::populateArgValues(pArgList);

      // Data element
      PlugInArg* pImportElementArg = NULL;
      pArgList->getArg(Importer::ImportElementArg(), pImportElementArg);
      if (pImportElementArg != NULL)
      {
         if (pImportElementArg->isActualSet() == false)
         {
            pImportElementArg->setPlugInArgValueLoose(mpElement);
         }
      }
   }
}

void ImportAgentImp::updateMruFileList(const map<ImportDescriptor*, bool>& importedDescriptors)
{
   if (mUpdateMruList == false)
   {
      return;
   }

   for (QMap<QString, vector<ImportDescriptor*> >::iterator iter = mDatasets.begin(); iter != mDatasets.end(); ++iter)
   {
      // Determine if the filename really is a file
      QString filename = iter.key();
      if (QFile::exists(filename) == false)
      {
         continue;
      }

      // If no data sets in the file were successfully imported, do nothing to the list
      vector<ImportDescriptor*> fileDescriptors = iter.value();
      bool fileHasImportedDatasets = false;

      for (vector<ImportDescriptor*>::iterator descriptorIter = fileDescriptors.begin();
         descriptorIter != fileDescriptors.end();
         ++descriptorIter)
      {
         ImportDescriptor* pImportDescriptor = *descriptorIter;
         if (pImportDescriptor != NULL)
         {
            map<ImportDescriptor*, bool>::const_iterator successIter = importedDescriptors.find(pImportDescriptor);
            if ((successIter != importedDescriptors.end()) && (successIter->second == true))
            {
               fileHasImportedDatasets = true;
               break;
            }
         }
      }

      if (fileHasImportedDatasets == false)
      {
         continue;
      }

      // Remove the current file if it exists in the list
      ConfigurationSettingsImp* pSettings = ConfigurationSettingsImp::instance();
      pSettings->removeMruFile(filename.toStdString());

      // Get the number of MRU files
      vector<MruFile> mruFiles = pSettings->getMruFiles();
      unsigned int maxNumFiles = ConfigurationSettings::getSettingNumberOfMruFiles();
      Service<ModelServices> pModel;

      while ((mruFiles.size() >= maxNumFiles) && (mruFiles.empty() == false))
      {
         // Destroy the existing import descriptors in the MRU file
         MruFile mruFile = mruFiles.back();

         for (vector<ImportDescriptor*>::iterator descriptorIter = mruFile.mDescriptors.begin();
            descriptorIter != mruFile.mDescriptors.end();
            ++descriptorIter)
         {
            ImportDescriptor* pImportDescriptor = *descriptorIter;
            if (pImportDescriptor != NULL)
            {
               pModel->destroyImportDescriptor(pImportDescriptor);
            }
         }

         // Remove the MRU file from the vector
         mruFiles.pop_back();
      }

      // Add only the descriptors that are selected to import to the MRU file, regardless of the import success
      if (maxNumFiles > 0)
      {
         vector<ImportDescriptor*> mruDescriptors;

         for (vector<ImportDescriptor*>::iterator descriptorIter = fileDescriptors.begin();
            descriptorIter != fileDescriptors.end();
            ++descriptorIter)
         {
            ImportDescriptor* pImportDescriptor = *descriptorIter;
            if ((pImportDescriptor != NULL) && (pImportDescriptor->isImported() == true))
            {
               // Copy the import descriptor to set in the MRU file since they are destroyed in the destructor
               DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
               if (pDescriptor != NULL)
               {
                  DataDescriptor* pMruDescriptor = pDescriptor->copy();
                  if (pMruDescriptor != NULL)
                  {
                     ImportDescriptor* pMruImportDescriptor = pModel->createImportDescriptor(pMruDescriptor, true);
                     if (pMruImportDescriptor != NULL)
                     {
                        mruDescriptors.push_back(pMruImportDescriptor);
                     }
                  }
               }
            }
         }

         // Add the file to the MRU file vector
         string importer;

         const PlugIn* pPlugIn = getPlugIn();
         if (pPlugIn != NULL)
         {
            importer = pPlugIn->getName();
         }

         FilenameImp filenameImp(filename.toStdString());
         string filePath = filenameImp.getPath();
         string baseFilename = filenameImp.getFileName();

         DateTimeImp modificationTime;

         FileFinderImp fileFinder;
         fileFinder.findFile(filePath, baseFilename);
         if (fileFinder.findNextFile() == true)
         {
            fileFinder.getLastModificationTime(modificationTime);
         }

         mruFiles.insert(mruFiles.begin(), MruFile(filename.toStdString(), importer, mruDescriptors, modificationTime));
      }

      // Update the MRU file list in the configuration settings
      pSettings->setMruFiles(mruFiles);
   }
}
