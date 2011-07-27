/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataDescriptor.h"
#include "DynamicObject.h"
#include "EnviImporter.h"
#include "EnviLibraryImporter.h"
#include "FileDescriptor.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "Signature.h"
#include "SignatureLibrary.h"
#include "RasterElement.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksENVI, EnviLibraryImporter);

EnviLibraryImporter::EnviLibraryImporter() :
   mpStep(NULL),
   mpProgress(NULL),
   mpSignatureLibrary(NULL)
{
   setName("ENVI Library Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("ENVI Signature Library Files (*.sli *.spl *.dat *.hdr)");
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

   string headerFile = filename;
   string dataFile;
   // assume filename is a header file and try to parse
   bool bSuccess = mFields.populateFromHeader(filename);
   if (bSuccess == false)
   {
      dataFile = filename;           // was passed data file name instead of header file name
      headerFile = findHeaderFile(dataFile);
      if (headerFile.empty() == false)
      {
         bSuccess = mFields.populateFromHeader(headerFile);
      }
   }
   if (bSuccess == true)
   {
      if (dataFile.empty())  // was passed header file name and now need to find the data file name
      {
         dataFile = findDataFile(headerFile);
      }
      if (dataFile.empty())  // no data file found for the header
      {
         return descriptors;
      }

      EnviField* pField = mFields.find("file type");
      if (pField != NULL)
      {
         if (pField->mValue == "ENVI Spectral Library" || 
             pField->mValue == "Spectral Library")
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
                  // Metadata
                  FactoryResource<DynamicObject> pMetadata;
                  if (pDescriptionField != NULL)
                  {
                     vector<EnviField*>& children = pDescriptionField->mChildren;
                     for (vector<EnviField*>::iterator iter = children.begin(); iter != children.end(); ++iter)
                     {
                        EnviField* pField = *iter;
                        if (pField != NULL)
                        {
                           if ((pField->mTag.empty() == false) && (pField->mValue.empty() == false))
                           {
                              pMetadata->setAttribute(pField->mTag, pField->mValue);
                           }
                        }
                     }
                  }

                  // Signature names
                  EnviField* pSigNamesField = mFields.find("spectra names");
                  if (pSigNamesField != NULL)
                  {
                     vector<string> sigNames;
                     for (unsigned int i = 0; i < pSigNamesField->mChildren.size(); i++)
                     {
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
                     }

                     if (sigNames.empty() == false)
                     {
                        pMetadata->setAttribute("Signature Names", sigNames);
                     }
                  }

                  // Wavelengths
                  EnviField* pSamplesField = mFields.find("samples");
                  if (pSamplesField != NULL)
                  {
                     unsigned int numWavelengths = StringUtilities::fromXmlString<unsigned int>(pSamplesField->mValue);

                     vector<double> wavelengths;
                     unsigned int uiNanometerValues = 0;

                     EnviField* pWavelengthField = mFields.find("wavelength");
                     if (pWavelengthField != NULL)
                     {
                        vector<unsigned int> goodBands;
                        EnviField* pBblField = mFields.find("bbl");
                        if (pBblField != NULL)
                        {
                           // Parse the bad bands list. This method puts the indices of good bands in ascending order.
                           EnviImporter::parseBbl(pBblField, goodBands);

                           // Sort in descending order so that the last one can be popped later
                           // A pop_back is much faster than an erase on the first element
                           reverse(goodBands.begin(), goodBands.end());
                        }

                        unsigned int numWavelengthsRead = 0;
                        for (std::vector<EnviField*>::const_iterator iter = pWavelengthField->mChildren.begin();
                           iter != pWavelengthField->mChildren.end();
                           ++iter)
                        {
                           EnviField* pField = *iter;
                           if (pField != NULL)
                           {
                              vector<char> bufferVector(pField->mValue.size() + 1);
                              char* pBuffer = &(bufferVector.front());
                              strcpy(pBuffer, pField->mValue.c_str());

                              char* pPtr = strtok(pBuffer, ",");
                              while (pPtr != NULL)
                              {
                                 double dWavelength = 0.0;
                                 if (sscanf(pPtr, "%lf", &dWavelength) == 1)
                                 {
                                    if (dWavelength > 50.0)    // Assumed to be in nanometers
                                    {
                                       uiNanometerValues++;
                                    }

                                    // Restrict the number of wavelengths to the number of samples in the header file
                                    if (numWavelengthsRead < numWavelengths)
                                    {
                                       // Only write the wavelength if the value is valid
                                       // Since the bands are in descending order,
                                       // goodBands.back() always holds the next good band.
                                       if (pBblField == NULL ||
                                          (goodBands.empty() == false && goodBands.back() == numWavelengthsRead))
                                       {
                                          if (goodBands.empty() == false)
                                          {
                                             goodBands.pop_back();
                                          }

                                          wavelengths.push_back(dWavelength);
                                       }
                                    }

                                    ++numWavelengthsRead;
                                 }

                                 pPtr = strtok(NULL, ",");
                              }
                           }
                        }

                        VERIFYNR(goodBands.empty() == true);
                     }

                     // Wavelength units
                     bool bConvertWavelengths = false;
                     bool bDetermineUnits = true;

                     EnviField* pUnitsField = mFields.find("wavelength units");
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
                     }

                     string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
                        CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                     pMetadata->setAttributeByPath(pCenterPath, wavelengths);
                  }

                  if (pMetadata->getNumAttributes() > 0)
                  {
                     pDescriptor->setMetadata(pMetadata.get());
                  }

                  // Create the file descriptor
                  FactoryResource<FileDescriptor> pFileDescriptor;
                  if (pFileDescriptor.get() != NULL)
                  {
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
      return Importer::CAN_LOAD + 5; // ensures that it beats EnviImporter
   }
}

bool EnviLibraryImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<SignatureLibrary>(Importer::ImportElementArg(), NULL, "Signature library to be imported."));
   return true;
}

bool EnviLibraryImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
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
   string headerFile = findHeaderFile(dataFile);
   if (!mFields.populateFromHeader(headerFile))
   {
      message = "Cannot access the spectral library file!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   message = "Mapping the library...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 0, NORMAL);
   }

   bool success = mpSignatureLibrary->import(dataFile, "ENVI Importer", mpProgress);
   if (success == true)
   {
      message = "ENVI library import complete!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 100, NORMAL);
      }

      pStep->finalize(Message::Success);
   }
   else
   {
      message = "The ENVI library signatures could not be imported.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
   }

   return success;
}

bool EnviLibraryImporter::extractPlugInArgs(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   // Progress
   mpProgress = pArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   // SignatureLibrary
   mpSignatureLibrary = pArgList->getPlugInArgValue<SignatureLibrary>(Importer::ImportElementArg());
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

string EnviLibraryImporter::findDataFile(const string& headerFilename)
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

   string dataFile = findFileByExtension(headerFilename, ".sli", "rb");
   if (dataFile.empty())
   {
      dataFile = findFileByExtension(headerFilename, ".spl", "rb");

      if (dataFile.empty())
      {
         dataFile = findFileByExtension(headerFilename, ".dat", "rb");
      }
   }
   return dataFile;
}

string EnviLibraryImporter::findHeaderFile(const string& dataFilename)
{
   return findFileByExtension(dataFilename, ".hdr", "rt");
}

string EnviLibraryImporter::findFileByExtension(const string& filename, const string& fileExtension,
                                                const string& openMode)
{
   if (filename.empty() == true)
   {
      return string();
   }
   string::size_type pos = filename.rfind(fileExtension);
   if (pos == (filename.length() - fileExtension.length()))
   {
      return filename;
   }

   // If the provided file extension exists in the filename, truncate the filename
   FILE* pFp = NULL;
   string matchFile;

   if (pos != string::npos)
   {
      matchFile = filename.substr(0, pos) + fileExtension;
      pFp = fopen(matchFile.c_str(), openMode.c_str());
   }

   // Append the provided file extension to the input filename
   if (pFp == NULL)
   {
      matchFile = filename + fileExtension;
      pFp = fopen(matchFile.c_str(), openMode.c_str());
   }

   // Truncate the input filename extension and append the provided file extension
   if (pFp == NULL)
   {
      pos = filename.rfind('.');
      if (pos != string::npos)
      {
         matchFile = filename.substr(0, pos) + fileExtension;
         pFp = fopen(matchFile.c_str(), openMode.c_str());
      }
   }

   if (pFp != NULL)
   {
      fclose(pFp);
   }
   else
   {
      matchFile.erase();
   }

   return matchFile;
}
