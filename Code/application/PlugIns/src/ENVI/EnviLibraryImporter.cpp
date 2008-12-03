/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "EnviLibraryImporter.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "FileDescriptor.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "Signature.h"
#include "SignatureLibrary.h"
#include "RasterElement.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"

#include <sstream>
using namespace std;

EnviLibraryImporter::EnviLibraryImporter() :
   mbAbort(false),
   mpStep(NULL),
   mpProgress(NULL),
   mpSignatureLibrary(NULL)
{
   setName("ENVI Library Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("ENVI Signature Library Files (*.sli *.hdr)");
   setSubtype("Signature Set");
   setDescriptorId("{638DB32A-D6A5-43c9-A4DB-4E059C750367}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

EnviLibraryImporter::~EnviLibraryImporter()
{
}

vector<ImportDescriptor*> EnviLibraryImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   bool bSuccess = parseHeader(filename);
   if (bSuccess == true)
   {
      EnviField* pField = mFields.find("file type");
      if (pField != NULL)
      {
         if (pField->mValue == "ENVI Spectral Library")
         {
            // Get the name and dataset from the header values
            string name;
            string dataset;

            EnviField* pBandNamesField = mFields.find("band names");
            if (pBandNamesField != NULL)
            {
               if (pBandNamesField->mChildren.empty() == false)
               {
                  EnviField* pNameField = pBandNamesField->mChildren.front();
                  if (pNameField != NULL)
                  {
                     name = pNameField->mValue;
                  }
               }
            }

            EnviField* pDescriptionField = mFields.find("description");
            if (pDescriptionField != NULL)
            {
               // Library name
               if (name.empty() == true)
               {
                  EnviField* pNameField = pDescriptionField->find("library name");
                  if (pNameField != NULL)
                  {
                     name = pNameField->mValue;
                  }
               }

               // Dataset
               EnviField* pDatasetField = pDescriptionField->find("dataset");
               if (pDatasetField != NULL)
               {
                  dataset = pDatasetField->mValue;
               }
            }

            // Create the data descriptor
            Service<ModelServices> pModel;

            RasterElement* pRasterElement = NULL;
            if (dataset.empty() == false)
            {
               pRasterElement = dynamic_cast<RasterElement*>(pModel->getElement(dataset, "RasterElement", NULL));
            }

            if (name.empty() == true)
            {
               // Create a unique default name
               unsigned int libraryNumber = pModel->getElements(pRasterElement, "SignatureLibrary").size();

               DataElement* pSignatureLibrary = NULL;
               do
               {
                  char buffer[64];
                  sprintf(buffer, "%u", ++libraryNumber);

                  name = "Spectral Library " + string(buffer);
                  pSignatureLibrary = pModel->getElement(name, "SignatureLibrary", pRasterElement);
               }
               while (pSignatureLibrary != NULL);
            }

            ImportDescriptor* pImportDescriptor =
               pModel->createImportDescriptor(name, "SignatureLibrary", pRasterElement);
            if (pImportDescriptor != NULL)
            {
               DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
               if (pDescriptor != NULL)
               {
                  // Create the file descriptor
                  FactoryResource<FileDescriptor> pFileDescriptor;
                  if (pFileDescriptor.get() != NULL)
                  {
                     // Set the filename as the data file
                     string dataFile = findDataFile(filename);

                     pFileDescriptor->setFilename(dataFile);
                     pDescriptor->setFileDescriptor(pFileDescriptor.get());
                  }
               }

               descriptors.push_back(pImportDescriptor);
            }
         }
      }
   }

   return descriptors;
}

unsigned char EnviLibraryImporter::getFileAffinity(const string& filename)
{
   if (getImportDescriptors(filename).empty())
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD+1; // ensures that it beats EnviImporter
   }
}

bool EnviLibraryImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<SignatureLibrary>(ImportElementArg(), NULL));

   return true;
}

bool EnviLibraryImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   return true;
}

bool EnviLibraryImporter::hasAbort()
{
   return true;
}

bool EnviLibraryImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run ENVI library importer", "app", "B696802B-1529-4F20-9EB1-8EED15CC9258");
   mpStep = pStep.get();

   string message;

   if (!extractPlugInArgs(pInArgList))
   {
      return false;
   }

   // Populate the header fields member
   string dataFile = mpSignatureLibrary->getFilename();
   if (parseHeader(dataFile) == false)
   {
      message = "Cannot access the spectral library file!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   message = "Reading the values from the header file...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 0, NORMAL);
   }

   DynamicObject* pMetadata = mpSignatureLibrary->getMetadata();
   VERIFY(pMetadata != NULL);

   // Description
   EnviField* pDescriptionField = mFields.find("description");
   if (pDescriptionField != NULL)
   {
      EnviField* pDescriptionTextField = pDescriptionField->find("description");
      if (pDescriptionTextField != NULL)
      {
         string description;
         if (pDescriptionTextField->mValue.empty() == true)
         {
            if (pDescriptionField->mChildren.size() == 1)
            {
               pDescriptionTextField = pDescriptionField->mChildren.front();
               if (pDescriptionTextField != NULL)
               {
                  if (pDescriptionTextField->mValue.empty() == false)
                  {
                     description = pDescriptionTextField->mValue;
                  }
               }
            }
         }
         else
         {
            description = pDescriptionTextField->mValue;
         }
         pMetadata->setAttribute("Description", description);
      }

      // metadata
      vector<EnviField*>& children = pDescriptionField->mChildren;
      vector<EnviField*>::iterator it;
      for (it = children.begin(); it != children.end(); ++it)
      {
         if ((*it) != NULL && (*it)->mTag.empty() == false && (*it)->mValue.empty() == false)
         {
            EnviField* pField = *it;
            pMetadata->setAttribute(pField->mTag, pField->mValue);
         }
      }
   }


   if (mbAbort)
   {
      message = "ENVI signature library import aborted!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ABORT);
      }

      pStep->finalize(Message::Abort);
      return false;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 10, NORMAL);
   }

   // Signature names
   vector<string> sigNames;

   EnviField* pSigNamesField = mFields.find("spectra names");
   if (pSigNamesField != NULL)
   {
      for (unsigned int i = 0; i < pSigNamesField->mChildren.size(); i++)
      {
         if (mbAbort)
         {
            message = "ENVI signature library import aborted!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ABORT);
            }

            pStep->finalize(Message::Abort);
            return false;
         }

         EnviField* pField = pSigNamesField->mChildren[i];
         if (pField != NULL)
         {
            vector<char> bufferVector(pField->mValue.size() + 1);
            char* pBuffer = &bufferVector.front();
            strcpy(pBuffer, pField->mValue.c_str());

            char* pPtr = strtok(pBuffer, ",");
            while (pPtr != NULL)
            {
               string sigName = StringUtilities::stripWhitespace(string(pPtr));
               sigNames.push_back(sigName);

               pPtr = strtok(NULL, ",");
            }
         }

         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message,
               10 + ((i + 1) * 100 / pSigNamesField->mChildren.size() / 4), NORMAL);
         }
      }
   }

   pMetadata->setAttribute("Signature Names", sigNames);

   // Wavelengths
   EnviField* pSamplesField = mFields.find("samples");
   VERIFY(pSamplesField != NULL);

   unsigned int numWavelengths = StringUtilities::fromXmlString<unsigned int>(pSamplesField->mValue);

   vector<double> wavelengths;
   unsigned int uiNanometerValues = 0;

   EnviField* pWavelengthField = mFields.find("wavelength");
   if (pWavelengthField != NULL)
   {
      for (unsigned int i = 0; i < pWavelengthField->mChildren.size(); i++)
      {
         if (mbAbort)
         {
            message = "ENVI signature library import aborted!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ABORT);
            }

            pStep->finalize(Message::Abort);
            return false;
         }

         EnviField* pField = pWavelengthField->mChildren[i];
         if (pField != NULL)
         {
            vector<char> bufferVector(pField->mValue.size() + 1);
            char* pBuffer = &(bufferVector.front());
            strcpy(pBuffer, pField->mValue.c_str());

            char* pPtr = strtok(pBuffer, ",");
            while (pPtr != NULL)
            {
               double dWavelength = 0.0;
               int count = sscanf(pPtr, "%lf", &dWavelength);
               if (count == 1)
               {
                  if (dWavelength > 50.0)    // Assumed to be in nanometers
                  {
                     uiNanometerValues++;
                  }

                  // Restrict the number of wavelengths to the number of samples in the header file
                  if (i < numWavelengths)
                  {
                     wavelengths.push_back(dWavelength);
                  }
               }

               pPtr = strtok(NULL, ",");
            }
         }

         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message,
               35 + ((i + 1) * 15 / pWavelengthField->mChildren.size()), NORMAL);
         }
      }
   }

   // wavelength units
   bool bConvertWavelengths = false;
   bool bDetermineUnits = true;
   EnviField* pUnitsField = NULL;
   pUnitsField = mFields.find("wavelength units");
   if (pUnitsField != NULL)
   {
      if (pUnitsField->mValue == "Micrometers")
      {
         bDetermineUnits = false;
      }
      else if (pUnitsField->mValue == "Nanometers")
      {
         bDetermineUnits = false;
         bConvertWavelengths = true;
      }
   }

   if (bDetermineUnits)
   {
      if ((uiNanometerValues * 100) / wavelengths.size() > 50)
      {
         bConvertWavelengths = true;
      }
   }

   if (bConvertWavelengths == true)
   {
      for (vector<double>::size_type i = 0; i < wavelengths.size(); i++)
      {
         wavelengths[i] *= 0.001;
      }

      message = "Warning ENVI Library Importer002: The wavelengths for all signatures "
         "were converted from nanometers to microns!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 60, WARNING);
      }

      pStep->addMessage(message, "app", "31FAD3F3-78F7-49DE-A0A8-1211750F9141");
   }

   string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
      CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
   pMetadata->setAttributeByPath(pCenterPath, wavelengths);

   message = "Mapping the library...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 50, NORMAL);
   }

   bool cubeLoaded = mpSignatureLibrary->import(dataFile, "ENVI Importer");

   message = "ENVI library import complete!";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool EnviLibraryImporter::abort()
{
   mbAbort = true;
   return true;
}

bool EnviLibraryImporter::extractPlugInArgs(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   // Progress
   mpProgress = pArgList->getPlugInArgValue<Progress>(ProgressArg());

   // SignatureLibrary
   mpSignatureLibrary = pArgList->getPlugInArgValue<SignatureLibrary>(ImportElementArg());
   if (mpSignatureLibrary == NULL)
   {
      string message = "The spectral library input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   return true;
}

string EnviLibraryImporter::findHeaderFile(const string& filename)
{
   if (filename.empty() == true)
   {
      return string();
   }

   // Check if the input filename is the header file
   string headerFileExtension = ".hdr";

   string::size_type pos = filename.rfind(headerFileExtension);
   if (pos == (filename.length() - 4))
   {
      return filename;
   }

   // If the header file extension exists in the filename, truncate the filename
   FILE* pFp = NULL;
   string headerFile;

   if (pos != string::npos)
   {
      headerFile = filename.substr(0, pos) + headerFileExtension;
      pFp = fopen(headerFile.c_str(), "rt");
   }

   // Append the header file extension to the input filename
   if (pFp == NULL)
   {
      headerFile = filename + headerFileExtension;
      pFp = fopen(headerFile.c_str(), "rt");
   }

   // Truncate the input filename extension and append the header file extension
   if (pFp == NULL)
   {
      pos = filename.rfind('.');
      if (pos != string::npos)
      {
         headerFile = filename.substr(0, pos) + headerFileExtension;
         pFp = fopen(headerFile.c_str(), "rt");
      }
   }

   if (pFp != NULL)
   {
      fclose(pFp);
   }
   else
   {
      headerFile.erase();
   }

   return headerFile;
}

string EnviLibraryImporter::findDataFile(const string& filename)
{
   // Check the fields from the header
   EnviField* pField = mFields.find("description");
   if (pField != NULL)
   {
      EnviField* pFilenameField = pField->find("filename");
      if (pFilenameField != NULL)
      {
         if (!pFilenameField->mValue.empty())
         {
            return pFilenameField->mValue;
         }
      }
   }

   // Check for an invalid filename
   if (filename.empty() == true)
   {
      return string();
   }

   // Check if the input filename is the data file
   string dataFileExtension = ".sli";

   string::size_type pos = filename.rfind(dataFileExtension);
   if (pos == (filename.length() - 4))
   {
      return filename;
   }

   // If the data file extension exists in the filename, truncate the filename
   FILE* pFp = NULL;
   string dataFile;

   if (pos != string::npos)
   {
      dataFile = filename.substr(0, pos) + dataFileExtension;
      pFp = fopen(dataFile.c_str(), "rb");
   }

   // Append the data file extension to the input filename
   if (pFp == NULL)
   {
      dataFile = filename + dataFileExtension;
      pFp = fopen(dataFile.c_str(), "rb");
   }

   // Truncate the input filename extension and append the data file extension
   if (pFp == NULL)
   {
      pos = filename.rfind('.');
      if (pos != string::npos)
      {
         dataFile = filename.substr(0, pos) + dataFileExtension;
         pFp = fopen(dataFile.c_str(), "rb");
      }
   }

   if (pFp != NULL)
   {
      fclose(pFp);
   }
   else
   {
      dataFile.erase();
   }

   return dataFile;
}

bool EnviLibraryImporter::parseHeader(const string& filename)
{
   if (filename.empty() == true)
   {
      return false;
   }

   bool bSuccess = mFields.populateFromHeader(filename);
   if (bSuccess == false)
   {
      string headerFile = findHeaderFile(filename);
      bSuccess = mFields.populateFromHeader(headerFile);
   }

   return bSuccess;
}
