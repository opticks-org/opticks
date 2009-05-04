/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AppVersion.h"
#include "Aspam.h"
#include "AspamImporter.h"
#include "AspamManager.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "UInt64.h"

#include <sstream>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksAspam, AspamImporter);

/******
 * Parsing helper functions
 ******/
namespace
{
inline bool checkForToken(stringstream& sstr, string token, bool unget = false)
{
   streampos sstrPosition(sstr.tellg());

   bool result(true);
   stringstream tokenStream(token);

   while (!tokenStream.eof())
   {
      string a;
      string b;

      sstr >> a;
      tokenStream >> b;
      if (a != b)
      {
         result = false;
         break;
      }
   }

   if (unget && !result)
   {
      sstr.seekg(sstrPosition, ios_base::beg);
   }
   return result;
}

inline bool checkForString(stringstream& sstr, string str, bool unget = false)
{
   streampos sstrPosition(sstr.tellg());

   bool result(true);
   for (unsigned int p = 0; p < str.size(); p++)
   {
      char c;
      sstr >> c;
      if (c != str[p])
      {
         result = false;
         break;
      }
   }
   
   if (unget && !result)
   {
      sstr.seekg(sstrPosition, ios_base::beg);
   }
   return result;
}

template <class T>
inline void readNumOrNoData(stringstream& sstr, T& val)
{
   streampos sstrPosition(sstr.tellg());
   string buf;
   sstr >> buf;
   if (buf.empty() || buf[0] != '*')
   {
      sstr.seekg(sstrPosition, ios_base::beg);
      sstr >> val;
   }
   else if (buf[0] == '*')
   {
      for (unsigned int i = 0; i < buf.size(); i++)
      {
         if (buf[i] != '*')
         {
            sstr.seekg(static_cast<streamoff>(sstrPosition) + i + 1, ios_base::beg);
            break;
         }
      }
   }
}

template <class T>
inline void readFixedLength(stringstream& sstr, T& val, unsigned int length)
{
   string buf(length, ' ');
   sstr.get(const_cast<char*>(buf.c_str()), length);
   stringstream bufs(buf);
   bufs >> val;
}

inline const char* findParagraphStart(const char* pStart, const char* pParagraphStart)
{
   string buf(pStart);
   string::size_type pos = buf.find(pParagraphStart);
   if (pos == string::npos)
   {
      return NULL;
   }
   return pStart + pos;
}
};

AspamImporter::AspamImporter() :
   mpAspam(NULL)
{
   setName("ASPAM Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("ASPAM Files (*.wxdata *.WXDATA)");
   setDescription("Import Atmospheric Slant Path Analysis Model (ASPAM) data files.");
   setSubtype("ASPAM");
   setDescriptorId("{F0CDDAEF-7300-4882-A6AB-B4ACC3EC88CA}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

AspamImporter::~AspamImporter()
{}

bool AspamImporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   Service<PlugInManagerServices> pPlugInManager;

   pInArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   pInArgList->addArg<Progress>(ProgressArg(), NULL);

   Service<ModelServices> pModel;
   pModel->addElementType("Aspam");

   PlugInArg* pArg = NULL;
   VERIFY((pArg = pPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ImportElementArg());
   pArg->setType("Aspam");
   pInArgList->addArg(*pArg);

   return true;
}

bool AspamImporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

vector<ImportDescriptor*> AspamImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;
   if (!filename.empty())
   {
      Service<ModelServices> pModel;

      ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(filename, "Aspam", NULL);
      if (pImportDescriptor != NULL)
      {
         DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
         if (pDescriptor != NULL)
         {
            FactoryResource<FileDescriptor> pFileDescriptor;
            if (pFileDescriptor.get() != NULL)
            {
               pFileDescriptor->setFilename(filename);
               pDescriptor->setFileDescriptor(pFileDescriptor.get());
            }
         }

         descriptors.push_back(pImportDescriptor);
      }
   }
   return descriptors;
}

unsigned char AspamImporter::getFileAffinity(const std::string& filename)
{
   FileResource pFile(filename.c_str(), "rt");
   if (pFile.get() == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }
   string data(128, '\0'); // 128 should be plenty to locate start of Paragraph A
   unsigned int numberRead = fread(&data[0], sizeof(char), 127, pFile);
   stringstream sstr(data);
   if (checkForToken(sstr, "A.", true) && checkForToken(sstr, "SITE"))
   {
      return Importer::CAN_LOAD;
   }
   return Importer::CAN_NOT_LOAD;
}

bool AspamImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Any* pAspamContainer = NULL;
   Progress* pProgress = NULL;
   StepResource pStep("Import ASPAM", "app", "82AFD6B1-841C-4B15-9F8D-5431757057E5");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "69A4FC3B-9FBB-4327-BEAF-03321046EF60");

      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));

      pAspamContainer = pInArgList->getPlugInArgValueUnsafe<Any>(ImportElementArg());
      if (pAspamContainer == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No data element", 100, ERRORS);
         }
         pStep->finalize(Message::Failure, "No data element");
         return false;
      }
      pMsg->addProperty("Element name", pAspamContainer->getName());
      if (pAspamContainer->getFilename().empty())
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No file", 100, ERRORS);
         }
         pStep->finalize(Message::Failure, "No file");
         return false;
      }
   }

   // initialize the Aspam data
   mpAspam = NULL;
   vector<PlugIn*> instances = Service<PlugInManagerServices>()->getPlugInInstances("ASPAM Manager");
   AspamManager* pAspamManager = instances.empty() ? NULL : dynamic_cast<AspamManager*>(instances.front());
   if (pAspamManager != NULL)
   {
      if (pAspamManager->initializeAspam(pAspamContainer))
      {
         mpAspam = dynamic_cast<Aspam*>(pAspamContainer->getData());
      }
   }
   if (mpAspam == NULL)
   {
      string message = "Unable to initialize the ASPAM.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Open ASPAM file", 20, NORMAL);
   }
   FileResource pFile(pAspamContainer->getFilename().c_str(), "rt");
   if (pFile.get() == NULL)
   {
      string msg = "Unable to open file.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 100, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Parse ASPAM file", 40, NORMAL);
   }
   bool rval = deserialize(pFile.get());
   // ensure at least one paragraph loads.
   rval = rval && (mpAspam->getParagraphA().mLoaded ||
                   mpAspam->getParagraphB().mLoaded ||
                   mpAspam->getParagraphC().mLoaded ||
                   mpAspam->getParagraphD().mLoaded ||
                   mpAspam->getParagraphE().mLoaded ||
                   mpAspam->getParagraphF().mLoaded ||
                   mpAspam->getParagraphG().mLoaded ||
                   mpAspam->getParagraphH().mLoaded ||
                   mpAspam->getParagraphI().mLoaded ||
                   mpAspam->getParagraphJ().mLoaded ||
                   mpAspam->getParagraphK().mLoaded);
   if (!rval)
   {
      string msg = "Unable to parse ASPAM file";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 100, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Finished importing ASPAM", 100, NORMAL);
   }
   pStep->finalize(Message::Success);
   return true;
}

const char* AspamImporter::parseParagraphA(const char* pStart)
{
   Aspam::ParagraphA paragraphA;
   MessageResource pMsg("Paragraph A", "app", "0090303F-B432-49E2-B7CC-18B6DD428B13");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "A.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   // BEGIN PARSING

   stringstream sstr(pStart);

   if (checkForToken(sstr, "A.", true) && 
      (checkForToken(sstr, "SITE -", true) || checkForToken(sstr, "SITE")))
   {
      success = true;
      sstr >> paragraphA.mSiteId.mX;
      
      // some weather files have a 5 digit value after SITE but it is not a lat/long
      if ((paragraphA.mSiteId.mX > 180) || (paragraphA.mSiteId.mX < -180))
      {
         paragraphA.mSiteId.mX = 0.0; 
         sstr >> paragraphA.mSiteId.mX >> paragraphA.mSiteId.mY;
      }
      else
      {
         sstr >> paragraphA.mSiteId.mY;
      }
   }
   paragraphA.mLoaded = success;
   mpAspam->setParagraphA(paragraphA);

   const char* pStop = pStart + sstr.tellg();

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

const char* AspamImporter::parseParagraphB(const char* pStart)
{
   Aspam::ParagraphB paragraphB;
   MessageResource pMsg("Paragraph B", "app", "2C87848E-C3BC-4D11-821A-DE7C3354A870");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "B.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   // BEGIN PARSING

   stringstream sstr(pStart);

   if (checkForToken(sstr, "B.", true) && 
     (checkForToken(sstr, "TIME -", true) || checkForToken(sstr, "TIME"))) 
   {
      int ztime;
      char cbuf;
      sstr >> ztime >> cbuf;
      if (cbuf == 'Z')
      {
         int minute = ztime % 100;
         int hour = ztime / 100;

         int day;
         int month;
         int year;

         string monthBuf;
         sstr >> day >> monthBuf >> year;
         if (monthBuf == "JAN")
         {
            month = 1;
         }
         else if (monthBuf == "FEB")
         {
            month = 2;
         }
         else if (monthBuf == "MAR")
         {
            month = 3;
         }
         else if (monthBuf == "APR")
         {
            month = 4;
         }
         else if (monthBuf == "MAY")
         {
            month = 5;
         }
         else if (monthBuf == "JUN")
         {
            month = 6;
         }
         else if (monthBuf == "JUL")
         {
            month = 7;
         }
         else if (monthBuf == "AUG")
         {
            month = 8;
         }
         else if (monthBuf == "SEP")
         {
            month = 9;
         }
         else if (monthBuf == "OCT")
         {
            month = 10;
         }
         else if (monthBuf == "NOV")
         {
            month = 11;
         }
         else if (monthBuf == "DEC")
         {
            month = 12;
         }
         else
         {
            month = 0;
         }
         if (year < 100) // this is a 2 digit year
         {
            year += 1900;
         }
         success = paragraphB.mpDateTime->set(year, month, day, hour, minute, 0);
      }
   }
   paragraphB.mLoaded = success;
   mpAspam->setParagraphB(paragraphB);

   const char* pStop = pStart + sstr.tellg();

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

const char* AspamImporter::parseParagraphD(const char* pStart)
{
   Aspam::ParagraphD paragraphD;
   MessageResource pMsg("Paragraph D", "app", "D1A66370-096E-43A8-B4B5-B2C165405821");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "D.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   // BEGIN PARSING

   stringstream sstr(pStart);

   if (checkForToken(sstr, "D.", true) && (checkForToken(sstr, "WEATHER AT SITE -", true))
      || checkForToken(sstr, "WEATHER AT SITE"))
   {     
      if (checkForToken(sstr, "VSBY"))
      {
         // some files only have visibility in D, thus set success to true here
         sstr >> paragraphD.mSurfaceVisibility >> paragraphD.mUnits;
         success = true;

         for (int layerNum = 0; layerNum < 4; layerNum++)
         {
            Aspam::CloudLayer layer;
            sstr >> layer.mCoverage;
            checkForToken(sstr, "/8");
            sstr >> layer.mType;
            readNumOrNoData(sstr, layer.mBaseAltitude);
            checkForString(sstr, "/");
            
            // code to replace the problem that readNumOrNoData had with commas
            string buf;
            sstr >> buf;
            if (buf == "")
            {
               layer.mTopAltitude = 0;
            }
            else
            {
               layer.mTopAltitude = atoi( buf.c_str() );
            }
         
            paragraphD.mCloudLayers.push_back(layer);

            // The standard says there is a space between layers but some
            // files seem to have comma and space so we read in a character
            // to throw away any invalid character.
            sstr.get();
         }
      }
      if ((checkForToken(sstr, "TOTAL CLOUD COVERAGE IS", true)) || 
         (checkForToken(sstr, "TOTAL CLOUD COVERAGE")))
      {
         sstr >> paragraphD.mTotalCoverage;
         checkForToken(sstr, "/8");
         checkForToken(sstr, "WIND");
         unsigned int windValue(0);

         readNumOrNoData(sstr, windValue);
         paragraphD.mWindSpeed = windValue % 1000;
         paragraphD.mWindDirection = windValue / 1000;
         checkForString(sstr, "G");
         readNumOrNoData(sstr, paragraphD.mGustSpeed);
         getline(sstr, paragraphD.mRemark);
      }

   }
   paragraphD.mLoaded = success;
   mpAspam->setParagraphD(paragraphD);

   const char* pStop = pStart + sstr.tellg();

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

const char* AspamImporter::parseParagraphF(const char* pStart)
{
   Aspam::ParagraphF paragraphF;
   MessageResource pMsg("Paragraph F", "app", "293B5D84-CE8D-4891-8E3E-DC7B67F11878");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "F.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   const char* pStop = pStart;

   // BEGIN PARSING

   stringstream sstr(pStart);

   if (checkForToken(sstr, "F.", true) && 
     (checkForToken(sstr, "WINDS, TEMPERATURE, ABS HUMIDITY, DENSITY, PRESSURE -", true) ||
      checkForToken(sstr, "WINDS, TEMPERATURE, ABS HUMIDITY. DENSITY, PRESSURE -", true) ||
      checkForToken(sstr, "WINDS, TEMPERATURE, ABS HUMIDITY, DENSITY, PRESSURE")))
   {
      checkForToken(sstr, "-", true);
      if (checkForToken(sstr, "HEIGHT", true))
      {
         // The spec does not say anything about the existance of this
         // line but all the "official" examples have this so we assume
         // it exists...if not, we probably have a Paragraph F declaration
         // with no data in it.
         string buf;
         getline(sstr, buf);

         if (checkForToken(sstr, "F", true))
         {
            //need to check for MSL as well as AGL
            sstr >> paragraphF.mLevel;
            getline(sstr, buf);
         }

         for ( ; ; ) // this will break which done reading
         {
            Aspam::Analytic data;
            if (checkForToken(sstr, "SFC", true))
            {
               data.mHeight = 0;
            }
            else if (checkForToken(sstr, "0M", true))
            {
               data.mHeight = 0;
               data.mUnits = 'M';
            }
            else
            {
               streampos sstrPosition(sstr.tellg());
               sstr >> data.mHeight >> data.mUnits;

               // height will be zero 
               if (data.mHeight == 0)
               {
                  // we've reached the end of the data
                  pStop += sstrPosition;
                  break;
               }
            }
            readNumOrNoData(sstr, data.mWindDirection);
            readNumOrNoData(sstr, data.mWindSpeed);
            readNumOrNoData(sstr, data.mTemperature);
            readNumOrNoData(sstr, data.mHumidity);
            readNumOrNoData(sstr, data.mDensity);
            readNumOrNoData(sstr, data.mPressure);
            paragraphF.mAnalytic.push_back(data);
         }
         success = true;
      }
   }
   paragraphF.mLoaded = success;
   mpAspam->setParagraphF(paragraphF);

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

const char* AspamImporter::parseParagraphG(const char* pStart)
{
   Aspam::ParagraphG paragraphG;
   MessageResource pMsg("Paragraph G", "app", "6A05FA26-9F59-4693-B3CD-29B2EBD680EC");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "G.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   // BEGIN PARSING

   stringstream sstr(pStart);

   if (checkForToken(sstr, "G.", true) && checkForToken(sstr, "REMARKS -"))
   {
      paragraphG.mRemarks.erase();
      while (!sstr.eof())
      {
         string buf;
         getline(sstr, buf);
         if (buf.size() == 0)
         {
            success = true;
            break;
         }
         paragraphG.mRemarks += buf + "\n";
      }
   }
   paragraphG.mLoaded = success;
   mpAspam->setParagraphG(paragraphG);

   const char* pStop = pStart + sstr.tellg();

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

const char* AspamImporter::parseParagraphH(const char* pStart)
{
   Aspam::ParagraphH paragraphH;
   MessageResource pMsg("Paragraph H", "app", "542EC588-12B5-4925-996A-9AC36549BFC6");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "H.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   // BEGIN PARSING

   stringstream sstr(pStart);

   bool foundH = (checkForToken(sstr, "H.", true) && checkForToken(sstr, "AEROSOL PARAMETERS", true));
   if (foundH)
   {
      streampos sstrPosition(sstr.tellg());
      char dummy;
      const char pCheckString[] = "VERTICALPROFILEINFORMATION";
      for (size_t i = 0; foundH && i < strlen(pCheckString); i++)
      {
         sstr >> dummy;
         foundH = (dummy == pCheckString[i]);
      }
      if (!foundH)
      {
         sstr.seekg(sstrPosition, ios_base::beg);
      }
   }
   if (foundH)
   {
      if (sstr.get() != ' ') // get the space
      {
         sstr.unget();
      }
      readFixedLength(sstr, paragraphH.mLevels, 2);
      readFixedLength(sstr, paragraphH.mPrimaryBoundaryLayerAerosolParameter, 1);
      if (paragraphH.mPrimaryBoundaryLayerAerosolParameter == 3)
      {
         readFixedLength(sstr, paragraphH.mAirParcelType, 2);
      }
      else if (sstr.get() != ' ') // get the space
      {
         sstr.unget();
      }
      readFixedLength(sstr, paragraphH.mSeasonalDependence, 1);
      readFixedLength(sstr, paragraphH.mStratosphericAerosol, 1);
      readFixedLength(sstr, paragraphH.mSurfaceVisibility, 2);
      readFixedLength(sstr, paragraphH.mOzoneProfile, 1);
      readFixedLength(sstr, paragraphH.mBoundaryLayerParameterQualityIndex, 2);
      readFixedLength(sstr, paragraphH.mAlternateSurfaceVisibility, 2);
      readFixedLength(sstr, paragraphH.mAlternateBoundaryLayerAerosolParameter, 1);
      if (paragraphH.mAlternateBoundaryLayerAerosolParameter == 3)
      {
         readFixedLength(sstr, paragraphH.mAlternateAirParcelType, 2);
      }
      int stage; // 0 - searching for data, 1 - parsing data, 2 - finished parsing data
      for (stage = 0; sstr.good() && stage != 2; )
      {
         string line;
         getline(sstr, line);
         if (stage == 0 && line.size() >= 30)
         {
            stage = 1;
         }
         if (stage == 1)
         {
            if (line.size() < 30)
            {
               stage = 2;
            }
            else
            {
               stringstream lineStr(line);
               Aspam::Aerosol aerosol;
               lineStr >> aerosol.mHeight >> aerosol.mPressure >> aerosol.mTemperature >>
                  aerosol.mWaterVaporDensity >> aerosol.mAlternateTemperature >> aerosol.mAlternateWaterVaporDensity;
               long location;
               lineStr >> location;
               aerosol.mLatitude = location / 100000;
               aerosol.mLongtitude = abs(location % 100000);
               lineStr >> aerosol.mOzoneRatio;
               paragraphH.mAerosol.push_back(aerosol);
            }
         }
      }
      success = (stage == 2);
   }
   paragraphH.mLoaded = success;
   mpAspam->setParagraphH(paragraphH);

   const char* pStop = pStart + sstr.tellg();

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

const char* AspamImporter::parseParagraphJ(const char* pStart)
{
   Aspam::ParagraphJ paragraphJ;
   MessageResource pMsg("Paragraph J", "app", "F7569E21-BA50-48A2-86BD-B603F3CC43E0");
   bool success = false;

   const char* pParagraphStart = findParagraphStart(pStart, "J.");
   if (pParagraphStart != NULL)
   {
      pStart = pParagraphStart;
   }

   // BEGIN PARSING

   stringstream sstr(pStart);

   if (checkForToken(sstr, "J.", true) && checkForToken(sstr, "SURFACE WEATHER HISTORY", true)
      && checkForToken(sstr, "24 HOUR SURFACE WEATHER HISTORY", true))
   {
      int stage; // 0 - searching for wx data, 1 - parsing wx data, 2 - finished parsing data
      for (stage = 0; sstr.good() && stage != 2; )
      {
         string line;
         getline(sstr, line);
         if (stage == 0 && line.size() >= 60)
         {
            stage = 1;
         }
         if (stage == 1)
         {
            if (line.size() < 60)
            {
               stage = 2;
            }
            else
            {
               stringstream lineStr(line);
               Aspam::SurfaceWeather wx;
               readFixedLength(lineStr, wx.mYear, 2);
               readFixedLength(lineStr, wx.mJulianDay, 3);
               readFixedLength(lineStr, wx.mHour, 2);
               readFixedLength(lineStr, wx.mMinutes, 2);
               readFixedLength(lineStr, wx.mCloudBase1, 3);
               readFixedLength(lineStr, wx.mCloudCoverage1, 2);
               readFixedLength(lineStr, wx.mCloudThickness1, 2);
               if (wx.mCloudBase1 == 999)
               {
                  wx.mCloudBase1 = 0;
                  wx.mCloudCoverage1 = 0;
                  wx.mCloudThickness1 = 0;
               }
               readFixedLength(lineStr, wx.mCloudBase2, 3);
               readFixedLength(lineStr, wx.mCloudCoverage2, 2);
               readFixedLength(lineStr, wx.mCloudThickness2, 2);
               if (wx.mCloudBase2 == 999)
               {
                  wx.mCloudBase2 = 0;
                  wx.mCloudCoverage2 = 0;
                  wx.mCloudThickness2 = 0;
               }
               readFixedLength(lineStr, wx.mCloudBase3, 3);
               readFixedLength(lineStr, wx.mCloudCoverage3, 2);
               readFixedLength(lineStr, wx.mCloudThickness3, 2);
               if (wx.mCloudBase3 == 999)
               {
                  wx.mCloudBase3 = 0;
                  wx.mCloudCoverage3 = 0;
                  wx.mCloudThickness3 = 0;
               }
               readFixedLength(lineStr, wx.mCloudBase4, 3);
               readFixedLength(lineStr, wx.mCloudCoverage4, 2);
               readFixedLength(lineStr, wx.mCloudThickness4, 2);
               if (wx.mCloudBase4 == 999)
               {
                  wx.mCloudBase4 = 0;
                  wx.mCloudCoverage4 = 0;
                  wx.mCloudThickness4 = 0;
               }
               readFixedLength(lineStr, wx.mTotalCoverage, 2);
               readFixedLength(lineStr, wx.mVisibility, 2);
               readFixedLength(lineStr, wx.mPrecipitationType, 2);
               if (wx.mPrecipitationType == "99")
               {
                  wx.mPrecipitationType = "";
               }
               readFixedLength(lineStr, wx.mObscuration, 1);
               if (wx.mObscuration == "9")
               {
                  wx.mObscuration = "";
               }
               lineStr.get(); // skip space
               readFixedLength(lineStr, wx.mPressure, 4);
               lineStr.get(); // skip space
               readFixedLength(lineStr, wx.mTemperature, 3);
               lineStr.get(); // skip space
               readFixedLength(lineStr, wx.mDewpoint, 3);
               readFixedLength(lineStr, wx.mWindDirection, 2);
               readFixedLength(lineStr, wx.mWindSpeed, 3);
               readFixedLength(lineStr, wx.mAlternateWindSpeed, 3);
               paragraphJ.mSurfaceWeather.push_back(wx);
            }
         }
         if (stage == 2)
         {
            stringstream lineStr(line);
            lineStr >> paragraphJ.mMaxTemperature >> paragraphJ.mMinTemperature >> paragraphJ.mSnowDepth;
         }
      }

      success = (stage == 2);
   }
   paragraphJ.mLoaded = success;
   mpAspam->setParagraphJ(paragraphJ);

   const char* pStop = pStart + sstr.tellg();

   // END PARSING

   if (success)
   {
      pMsg->addProperty("Result", "Success");
   }
   else if (pStop != pStart)
   {
      pMsg->addProperty("Result", "Failed to parse");
   }
   else
   {
      pMsg->addProperty("Result", "Not present");
   }
   string remaining(pStop, pStop + 10);
   pMsg->addProperty("Remaining data", remaining);
   pMsg->finalize();

   return pStop;
}

bool AspamImporter::deserialize(FILE* pFp)
{
   string data = "";
   { // scope the resource
      MessageResource pLoadMsg("Load data from file", "app", "AC28F536-5970-4FD0-AD70-C949BCF63B43");
      while (!feof(pFp))
      {
         string buf(1024, ' ');
         unsigned int numberRead = fread(const_cast<char*>(buf.c_str()), sizeof(char), 1024, pFp);
         data += buf.substr(0, numberRead);
      }
      pLoadMsg->addProperty("Length", UInt64(data.size()));
      pLoadMsg->finalize(Message::Success);
   }

   MessageResource pParseMsg("Parse ASPAM data", "app", "ED0C895C-9E01-4FD0-A82F-177CCB14430C");

   const char* pParseLocation = data.c_str();
   pParseLocation = parseParagraphA(pParseLocation);
   pParseLocation = parseParagraphB(pParseLocation);
   pParseLocation = parseParagraphD(pParseLocation);
   pParseLocation = parseParagraphF(pParseLocation);
   pParseLocation = parseParagraphG(pParseLocation);
   pParseLocation = parseParagraphH(pParseLocation);
   pParseLocation = parseParagraphJ(pParseLocation);

   // Skip any remaining whitespace
   while (pParseLocation != NULL &&
         *pParseLocation != NULL &&
         (*pParseLocation == ' ' ||
          *pParseLocation == '\t' ||
          *pParseLocation == '\n' ||
          *pParseLocation == '\r'))
   {
      pParseLocation++;
   }

   if (*pParseLocation != NULL)
   {
      string remaining(pParseLocation, pParseLocation + 10);
      pParseMsg->addProperty("Remaining Data", remaining);
   }
   return true;
}
