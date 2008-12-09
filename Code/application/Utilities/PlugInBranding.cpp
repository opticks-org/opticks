/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlugInBranding.h"

#include "ConfigurationSettings.h"
#include "FileFinder.h"
#include "ObjectResource.h"
#include "StringUtilities.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

PlugInBranding::PlugInBranding()
{
}

PlugInBranding::~PlugInBranding()
{
}

const Filename* PlugInBranding::getSplashScreenImage() const
{
   return &mFilename;
}

void PlugInBranding::setSplashScreenImage(const Filename* pFilename)
{
   const FilenameImp* pImpParam = dynamic_cast<const FilenameImp*>(pFilename);
   if (pImpParam != NULL)
   {
      mFilename = *pImpParam;
   }
}

const string& PlugInBranding::getTitle() const
{
   return mTitle;
}

void PlugInBranding::setTitle(const string& title)
{
   mTitle = title;
}

const string& PlugInBranding::getDescription() const
{
   return mDescription;
}

void PlugInBranding::setDescription(const string& description)
{
   mDescription = description;
}

const string& PlugInBranding::getVersion() const
{
   return mVersion;
}

void PlugInBranding::setVersion(const string& version)
{
   mVersion = version;
}

const string& PlugInBranding::getLicense() const
{
   return mLicense;
}

void PlugInBranding::setLicense(const string& license)
{
   mLicense = license;
}

const string& PlugInBranding::getHelpTitle() const
{
   return mHelpTitle;
}

void PlugInBranding::setHelpTitle(const string& title)
{
   mHelpTitle = title;
}

const Filename* PlugInBranding::getHelpWebpage() const
{
   return &mHelpWebpage;
}

void PlugInBranding::setHelpWebpage(const Filename* pWebpage)
{
   const FilenameImp* pImpParam = dynamic_cast<const FilenameImp*>(pWebpage);
   if (pImpParam != NULL)
   {
      mHelpWebpage = *pImpParam;
   }
}

const vector<PlugInBranding>& PlugInBranding::getBrandings()
{
   static vector<PlugInBranding> sBrandings;
   static bool sParsedBranding = false;
   if (!sParsedBranding)
   {
      sParsedBranding = true; /* Since the parsed brandings are used during start-up to show
                                 the splash screen, we should only attempt to parse them once
                                 even if there is an error during the parse, so that the About
                                 dialog will be consistent with the splash screen display. */
      const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
      if (pSupportFilesPath != NULL)
      {
         string supportFiles = pSupportFilesPath->getFullPathAndName();
         FactoryResource<FileFinder> pFinder;
         pFinder->findFile(supportFiles + SLASH + "PlugInBranding", "*.brand", false);
         while (pFinder->findNextFile())
         {
            string brandingFile;
            if (!pFinder->getFullPath(brandingFile))
            {
               continue;
            }
            XmlReader reader(NULL, true);
            XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = NULL;
            try 
            {
               pDocument = reader.parse(brandingFile);
            }
            catch (XmlBase::XmlException&)
            {
               pDocument = NULL;
            }
            if (pDocument != NULL)
            {
               DOMElement* pRootElement = pDocument->getDocumentElement();
               if ((pRootElement != NULL) && (string(A(pRootElement->getTagName())) == "Branding"))
               {
                  PlugInBranding branding;
                  DOMNodeList* pSplashScreenNodes = pRootElement->getElementsByTagName(X("SplashScreen"));
                  if (pSplashScreenNodes != NULL && pSplashScreenNodes->getLength() == 1)
                  {
                     DOMElement* pSplashScreen = dynamic_cast<DOMElement*>(pSplashScreenNodes->item(0));
                     if (pSplashScreen != NULL)
                     {
                        string filePath = A(pSplashScreen->getAttribute(X("filePath")));
                        FactoryResource<Filename> pFilename;
                        pFilename->setFullPathAndName(StringUtilities::stripWhitespace(filePath));
                        branding.setSplashScreenImage(pFilename.get());
                     }
                  }
                  DOMNodeList* pTitleNodes = pRootElement->getElementsByTagName(X("Title"));
                  if (pTitleNodes != NULL && pTitleNodes->getLength() >= 1)
                  {
                     //only get the first Title, because according to xsd,
                     //we can only have 2 title elements, one here and one later under Help
                     DOMNode* pTitle = pTitleNodes->item(0);
                     if (pTitle != NULL)
                     {
                        string title = A(pTitle->getTextContent());
                        branding.setTitle(StringUtilities::stripWhitespace(title));
                     }
                  }
                  DOMNodeList* pDescNodes = pRootElement->getElementsByTagName(X("Description"));
                  if (pDescNodes != NULL && pDescNodes->getLength() == 1)
                  {
                     DOMNode* pDesc = pDescNodes->item(0);
                     if (pDesc != NULL)
                     {
                        string desc = A(pDesc->getTextContent());
                        branding.setDescription(StringUtilities::stripWhitespace(desc));
                     }
                  }
                  DOMNodeList* pVersionNodes = pRootElement->getElementsByTagName(X("Version"));
                  if (pVersionNodes != NULL && pVersionNodes->getLength() == 1)
                  {
                     DOMNode* pVersion = pVersionNodes->item(0);
                     if (pVersion != NULL)
                     {
                        string version = A(pVersion->getTextContent());
                        branding.setVersion(StringUtilities::stripWhitespace(version));
                     }
                  }
                  DOMNodeList* pLicenseNodes = pRootElement->getElementsByTagName(X("License"));
                  if (pLicenseNodes != NULL && pLicenseNodes->getLength() == 1)
                  {
                     DOMNode* pLicense = pLicenseNodes->item(0);
                     if (pLicense != NULL)
                     {
                        string license = A(pLicense->getTextContent());
                        branding.setLicense(StringUtilities::stripWhitespace(license));
                     }
                  }
                  DOMNodeList* pHelpNodes = pRootElement->getElementsByTagName(X("Help"));
                  if (pHelpNodes != NULL && pHelpNodes->getLength() == 1)
                  {
                     DOMElement* pHelpElement = dynamic_cast<DOMElement*>(pHelpNodes->item(0));
                     if (pHelpElement != NULL)
                     {
                        DOMNodeList* pHelpTitleNodes = pHelpElement->getElementsByTagName(X("Title"));
                        if (pHelpTitleNodes != NULL && pHelpTitleNodes->getLength() == 1)
                        {
                           DOMNode* pHelpTitle = pHelpTitleNodes->item(0);
                           if (pHelpTitle != NULL)
                           {
                              string helpTitle = A(pHelpTitle->getTextContent());
                              branding.setHelpTitle(StringUtilities::stripWhitespace(helpTitle));
                           }
                        }
                        DOMNodeList* pHelpWebpageNodes = pHelpElement->getElementsByTagName(X("Webpage"));
                        if (pHelpWebpageNodes != NULL && pHelpWebpageNodes->getLength() == 1)
                        {
                           DOMNode* pHelpWebpage = pHelpWebpageNodes->item(0);
                           if (pHelpWebpage != NULL)
                           {
                              string helpWebpage = A(pHelpWebpage->getTextContent());
                              FactoryResource<Filename> pFilename;
                              pFilename->setFullPathAndName(StringUtilities::stripWhitespace(helpWebpage));
                              branding.setHelpWebpage(pFilename.get());
                           }
                        }
                     }
                  }

                  sBrandings.push_back(branding);
               }
            }
         }
      }
   }

   return sBrandings;
}
