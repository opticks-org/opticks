/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataVariant.h"
#include "Endian.h"
#include "EnviLibraryExporter.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "Signature.h"
#include "SignatureSet.h"

#include <algorithm>

using namespace std;

EnviLibraryExporter::EnviLibraryExporter() :
   mbAbort(false),
   mpProgress(NULL),
   mpSignatureSet(NULL),
   mpFileDescriptor(NULL),
   mpStep(NULL)
{
   setName("ENVI Library Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("ENVI Signature Library Files (*.sli)");
   setSubtype(TypeConverter::toString<SignatureSet>());
   setDescriptorId("{FB8D86B6-BD5D-4657-91B0-5B2797060C8E}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

EnviLibraryExporter::~EnviLibraryExporter()
{
}

bool EnviLibraryExporter::setBatch()
{
   return true;
}

bool EnviLibraryExporter::setInteractive()
{
   return true;
}

bool EnviLibraryExporter::getInputSpecification(PlugInArgList*& pArgList)
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
   pArg->setName(ExportItemArg());
   pArg->setType("SignatureSet");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ExportDescriptorArg());
   pArg->setType("FileDescriptor");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool EnviLibraryExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool EnviLibraryExporter::hasAbort()
{
   return true;
}

bool EnviLibraryExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute ENVI Signature Library Exporter", "app", "11AA6583-E6E8-4830-8ADA-B880E30BEACB");
   mpStep = pStep.get();

   if (!extractPlugInArgs(pInArgList))
   {
      return false;
   }

   string message = "";

   // Check for a valid number of signatures
   vector<Signature*> signatures;
   VERIFY(mpSignatureSet != NULL);
   if (!extractSignatures(*mpSignatureSet, signatures))
   {
      message = "Cannot extract all of the signatures from the library!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (signatures.empty() == true)
   {
      message = "The signature set contains no signatures!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Verify that all spectra have the same wavelengths (required for ENVI spectral library)
   // Verify that all spectra have same scaling factor and units name
   unsigned int numSignatures = signatures.size();

   vector<double> spectrumWaves;
   double scalingFactor = -999.9;
   UnitType unitType = DISTANCE; // should not occur for a spectrum

   for (unsigned int i = 0; i < numSignatures; ++i)
   {
      Signature* pSignature = signatures[i];
      if (pSignature != NULL)
      {
         const DataVariant& variant = pSignature->getData("Wavelength");
         vector<double> currentWaves;
         variant.getValue(currentWaves);
         if ((currentWaves != spectrumWaves) && (spectrumWaves.empty() == false))
         {
            message = "Error ENVI Library Exporter004: All signatures must have the same wavelengths!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ERRORS);
            }

            pStep->finalize(Message::Failure, message);
            return false;
         }

         const Units* pUnits = pSignature->getUnits("Reflectance");
         if (pUnits != NULL)
         {
            double currentScalingFactor = pUnits->getScaleFromStandard(); 
            if (currentScalingFactor != scalingFactor && scalingFactor > -1.0 )
            {
               message = "Error ENVI Library Exporter012: All signatures must have the same scaling factor!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               return false;
            }

            UnitType currentUnitType = pUnits->getUnitType();
            if (currentUnitType != unitType && unitType != DISTANCE)
            {
               message = "Error ENVI Library Exporter013: All signatures must be in the same type of units!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               return false;
            }
            scalingFactor = currentScalingFactor;
            unitType = currentUnitType;
         }

         if (spectrumWaves.empty())
         {
            spectrumWaves = currentWaves;
         }
      }
   }

   unsigned int numValues = spectrumWaves.size();

   // Get the name of the spectral information file
   const string& filename = mpFileDescriptor->getFilename();
   string headerFile = filename;

   // Build the name of the hdr file
   string::size_type pos = filename.rfind(".");
   if (pos != string::npos)
   {
      headerFile = filename.substr(0, pos);
   }
   headerFile += ".hdr";

// Check for overwriting hdr file – 
//    If the header belongs to another file, it will exist but the .sli file will not
//    All other times we can overwrite the .hdr file
   if (QFile::exists(QString::fromStdString(headerFile)) && !QFile::exists(QString::fromStdString(filename)))
   {
      message = "Header file exists in the current directory and may describe another unrelated data file!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Write out the spectral information
   FILE* pFp = fopen(filename.c_str(), "wb");
   if (pFp == NULL)
   {
      message = "Could not open the library file for writing!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   message = "Exporting signature data...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 0, NORMAL);
   }

   for (unsigned int i = 0; i < numSignatures; ++i)
   {
      if (mbAbort)
      {
         message = "ENVI signature library export aborted!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message, 0, ABORT);
         }

         pStep->finalize(Message::Abort);

         fclose(pFp);
         remove(filename.c_str());
         return false;
      }

      Signature* pSignature = signatures[i];
      if (pSignature != NULL)
      {
         vector<double> spectrumValues;
         DataVariant reflectanceVariant = pSignature->getData("Reflectance");
         reflectanceVariant.getValue(spectrumValues);

         for (vector<double>::size_type j = 0; j < spectrumValues.size(); j++)
         {
            float fValue = static_cast<float>(spectrumValues[j]);
            fwrite(&fValue, sizeof(float), 1, pFp);
         }
      }

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, (i + 1) * 100 / numSignatures / 2, NORMAL);
      }
   }

   fclose(pFp);

   // Open the header file
   pFp = fopen(headerFile.c_str(), "w");
   if (pFp == NULL)
   {
      message = "Could not open the header file for writing!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      remove(filename.c_str());
      return false;
   }

   message = "Exporting header data...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 50, NORMAL);
   }

   pStep->finalize(Message::Failure, message);

   // Write the header information
   fprintf(pFp, "ENVI\n");
   fprintf(pFp, "description = {\n");
   // TODO: get from metadata
   //fprintf(pFp, "    DESCRIPTION = %s\n", mpSignatureSet->getDescription().c_str());
   fprintf(pFp, "    FILENAME = %s }\n", filename.c_str());
   fprintf(pFp, "samples = %u\n", numValues);
   fprintf(pFp, "lines = %u\n", numSignatures);
   fprintf(pFp, "bands = 1\n");
   fprintf(pFp, "header offset = 0\n");
   fprintf(pFp, "file type = ENVI Spectral Library\n");
   fprintf(pFp, "data type = 4\n");
   fprintf(pFp, "interleave = bsq\n");
   fprintf(pFp, "byte order = %d\n", Endian::getSystemEndian()==BIG_ENDIAN);    
   fprintf(pFp, "wavelength units = Micrometers\n");
   if (unitType == REFLECTANCE)
   {
      fprintf(pFp, "reflectance scale factor = %0.6f\n", scalingFactor);
   }

   fprintf(pFp, "band names = { %s }\n", mpSignatureSet->getName().c_str());
   fprintf(pFp, "spectra names = {\n");

   for (unsigned int i = 0; i < numSignatures; ++i)
   {
      if (mbAbort)
      {
         message = "ENVI signature library export aborted!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message, 0, ABORT);
         }

         pStep->finalize(Message::Failure, message);
         fclose(pFp);
         remove(filename.c_str());
         remove(headerFile.c_str());
         return false;
      }

      Signature* pSignature = signatures[i];
      if (pSignature != NULL)
      {
         string spectrumName = pSignature->getName().c_str();
         if (spectrumName.empty() == false)
         {
            // replace all commas with semicolons so ENVI will import correctly
            std::replace(spectrumName.begin(), spectrumName.end(), ',', ';'); 

            if (i > 0)
            {
               fprintf(pFp, ",\n");
            }

            fprintf(pFp, "%s", spectrumName.c_str());
         }
      }

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 50 + ((i + 1) * 100 / numSignatures / 4), NORMAL);
      }
   }

   fprintf(pFp, " }\n");
   fprintf(pFp, "wavelength = {\n");

   for (unsigned int i = 0; i < numValues; i++)
   {
      if (mbAbort)
      {
         message = "ENVI signature library export aborted!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message, 0, ABORT);
         }

         pStep->finalize(Message::Failure, message);
         fclose(pFp);
         remove(filename.c_str());
         remove(headerFile.c_str());
         return false;
      }

      if (i > 0)
      {
         fprintf(pFp, ",\n");
      }

      fprintf(pFp, "%g", spectrumWaves[i]);

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 75 + ((i + 1) * 100 / numValues / 4), NORMAL);
      }
   }

   fprintf(pFp, " }\n");
   fclose(pFp);

   message = "ENVI signature library export complete!";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool EnviLibraryExporter::abort()
{
   mbAbort = true;
   return true;
}

bool EnviLibraryExporter::extractPlugInArgs(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Progress
   if (pArgList->getArg(ProgressArg(), pArg) && (pArg != NULL))
   {
      mpProgress = pArg->getPlugInArgValue<Progress>();
   }

   // Signature set
   if (!pArgList->getArg(ExportItemArg(), pArg) || (pArg == NULL))
   {
      string message = "Could not read the signature set input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   mpSignatureSet = pArg->getPlugInArgValue<SignatureSet>();
   if (mpSignatureSet == NULL)
   {
      string message = "The signature set input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   // File descriptor
   if (!pArgList->getArg(ExportDescriptorArg(), pArg) || (pArg == NULL))
   {
      string message = "Could not read the file descriptor input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   mpFileDescriptor = pArg->getPlugInArgValue<FileDescriptor>();
   if (mpFileDescriptor == NULL)
   {
      string message = "The file descriptor input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   return true;
}

bool EnviLibraryExporter::extractSignatures(SignatureSet& signatureSet, vector<Signature*>& signatures)
{
   vector<Signature*> setSigs = signatureSet.getSignatures();

   for (vector<Signature*>::iterator iter = setSigs.begin(); iter != setSigs.end(); ++iter)
   {
      Signature* pCurrentSignature = *iter;
      if (pCurrentSignature != NULL)
      {
         SignatureSet* pCurrentSigSet = dynamic_cast<SignatureSet*>(pCurrentSignature);
         if (pCurrentSigSet != NULL)
         {
            if (extractSignatures(*pCurrentSigSet, signatures) == false)
            {
               return false;
            }
         }
         else
         {
            signatures.push_back(pCurrentSignature);
         }
      }
   }

   return true;
}
