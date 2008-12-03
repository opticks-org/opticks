/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>

#include "AppConfig.h"
#include "AppVerify.h"
#include "DesktopServicesImp.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "GpuProgramDescriptorAdapter.h"
#include "ImageFilterDescriptorAdapter.h"
#include "ImageFilterManager.h"
#include "MessageLogResource.h"
#include "xmlreader.h"

#if defined(CG_SUPPORTED)
#include "CgContext.h"
#endif

XERCES_CPP_NAMESPACE_USE
using namespace std;

template<>
ImageFilterManager* Service<ImageFilterManager>::get() const
{
   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   if (pDesktop != NULL)
   {
      return pDesktop->getImageFilterManager();
   }

   return NULL;
}

ImageFilterManager::ImageFilterManager()
{
   buildFilterList();
}

ImageFilterManager::~ImageFilterManager()
{
   clearFilters();
}

vector<string> ImageFilterManager::getAvailableFilters() const
{
   vector<string> filters;

   map<string, ImageFilterDescriptorImp*>::const_iterator iter;
   for (iter = mImageFilters.begin(); iter != mImageFilters.end(); ++iter)
   {
      string filterName = iter->first;
      if (filterName.empty() == false)
      {
         filters.push_back(filterName);
      }
   }

   return filters;
}

unsigned int ImageFilterManager::getNumAvailableFilters() const
{
   return mImageFilters.size();
}

ImageFilterDescriptor* ImageFilterManager::createFilterDescriptor(const string& filterName) const
{
   map<string, ImageFilterDescriptorImp*>::const_iterator iter = mImageFilters.find(filterName);
   if (iter != mImageFilters.end())
   {
      ImageFilterDescriptorImp* pDescriptor = iter->second;
      if (pDescriptor != NULL)
      {
         return pDescriptor->copy();
      }
   }

   return NULL;
}

void ImageFilterManager::buildFilterList(const string& filterPath)
{
#if defined(WIN_API)
   // Get the default filter path if necessary
   string imageFiltersPath = filterPath;
   if (filterPath.empty())
   {
      const Filename* pSupportFiles = ConfigurationSettings::getSettingSupportFilesPath();
      if (pSupportFiles != NULL)
      {
         imageFiltersPath = pSupportFiles->getFullPathAndName() + SLASH + "ImageFilters";
      }
   }

   // Load all filter descriptors from the path
   vector<ImageFilterDescriptorImp*> loadedDescriptors = loadFilterDescriptors(imageFiltersPath);

   // Remove existing filters that were not loaded
   map<string, ImageFilterDescriptorImp*>::iterator existingIter = mImageFilters.begin();
   while (existingIter != mImageFilters.end())
   {
      string existingName = existingIter->first;

      vector<ImageFilterDescriptorImp*>::iterator loadedIter;
      for (loadedIter = loadedDescriptors.begin(); loadedIter != loadedDescriptors.end(); ++loadedIter)
      {
         ImageFilterDescriptorImp* pLoadedDescriptor = *loadedIter;
         if (pLoadedDescriptor != NULL)
         {
            string loadedName = pLoadedDescriptor->getName();
            if (loadedName == existingName)
            {
               break;
            }
         }
      }

      if (loadedIter == loadedDescriptors.end())
      {
         delete existingIter->second;
         existingIter = mImageFilters.erase(existingIter);
      }
      else
      {
         // Remove the loaded filter from the vector of filters to add since it already exists
         loadedDescriptors.erase(loadedIter);
         ++existingIter;
      }
   }

   // Add the remaining loaded filters that do not already exist
   vector<ImageFilterDescriptorImp*>::iterator loadedIter;
   for (loadedIter = loadedDescriptors.begin(); loadedIter != loadedDescriptors.end(); ++loadedIter)
   {
      ImageFilterDescriptorImp* pLoadedDescriptor = *loadedIter;
      if (pLoadedDescriptor != NULL)
      {
         string loadedName = pLoadedDescriptor->getName();
         if (loadedName.empty() == false)
         {
            mImageFilters.insert(pair<string, ImageFilterDescriptorImp*>(loadedName, pLoadedDescriptor));
         }
      }
   }
#endif
}

void ImageFilterManager::clearFilters()
{
   map<string, ImageFilterDescriptorImp*>::iterator iter;
   for (iter = mImageFilters.begin(); iter != mImageFilters.end(); ++iter)
   {
      ImageFilterDescriptorImp* pDescriptor = iter->second;
      if (pDescriptor != NULL)
      {
         delete pDescriptor;
      }
   }

   mImageFilters.clear();
}

vector<ImageFilterDescriptorImp*> ImageFilterManager::loadFilterDescriptors(const string& filterPath) const
{
   vector<ImageFilterDescriptorImp*> filters;

#if defined(WIN_API)
   FileFinderImp fileFinder;
   fileFinder.findFile(filterPath, "*.gic");

   // Get the filter descriptors from all files in the path
   while (fileFinder.findNextFile() == true)
   {
      string filename;
      fileFinder.getFullPath(filename);

      if (filename.empty() == false)
      {
         // Get the filter descriptors from the current file
         vector<ImageFilterDescriptorImp*> fileFilters = deserialize(filename);

         vector<ImageFilterDescriptorImp*>::iterator iter = fileFilters.begin();
         for (iter = fileFilters.begin(); iter != fileFilters.end(); ++iter)
         {
            ImageFilterDescriptorImp* pDescriptor = *iter;
            if (pDescriptor != NULL)
            {
               filters.push_back(pDescriptor);
            }
         }
      }
   }
#endif

   return filters;
}

vector<ImageFilterDescriptorImp*> ImageFilterManager::deserialize(const string& filename) const
{
   vector<ImageFilterDescriptorImp*> filterDescriptors;
   if (filename.empty())
   {
      return filterDescriptors;
   }

#if defined(CG_SUPPORTED)
   if (CgContext::instance() == NULL)
   {
      return filterDescriptors;
   }
#endif

   XmlReader xmlReader(NULL, false);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDomDoc(NULL);
   if (filename.empty() == false)
   {
      if (QFile::exists(QString::fromStdString(filename)))
      {
         FilenameImp filepath;
         filepath.setFullPathAndName(filename);
         pDomDoc = xmlReader.parse(dynamic_cast<Filename*>(&filepath));
      }
   }

   if (pDomDoc == NULL)
   {
      return filterDescriptors;
   }

   try 
   {
      DOMElement* pRoot(NULL);
      if (pDomDoc != NULL)
      {
         pRoot = pDomDoc->getDocumentElement();

         if (pRoot == NULL)
         {
            return filterDescriptors;
         }

         for (DOMNode* pNode = pRoot->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
         {
            if (XMLString::equals(pNode->getNodeName(), X("filter")))
            {
               // Parse the Filter XML Element
               DOMElement* pElement(static_cast<DOMElement*>(pNode));

               ImageFilterDescriptorAdapter* pFilterDescriptor = new ImageFilterDescriptorAdapter();
               pFilterDescriptor->setName(A(pElement->getAttribute(X("name"))));
               pFilterDescriptor->setDescription(A(pElement->getAttribute(X("description"))));
               pFilterDescriptor->setType(ImageFilterDescriptor::IMAGE_FILTER);

               if (XMLString::equals(pElement->getAttribute(X("type")), X("FeedbackFilter")))
               {
                  pFilterDescriptor->setType(ImageFilterDescriptor::FEEDBACK_FILTER);
               }

               // Parse all the Programs
               for (DOMNode* pProgram = pNode->getFirstChild(); pProgram != NULL; pProgram = pProgram->getNextSibling())
               {
                  if (XMLString::equals(pProgram->getNodeName(), X("parameter")))
                  {
                     DOMElement* pParamElement(static_cast<DOMElement*>(pProgram));
                     if (pParamElement != NULL)
                     {
                        string parameterName(A(pParamElement->getAttribute(X("name"))));
                        string parameterType(A(pParamElement->getAttribute(X("type"))));
                        string parameterValue(A(pParamElement->getAttribute(X("value"))));
                        DataVariant parameterVariant(parameterType, NULL);
                        parameterVariant.fromXmlString(parameterType, parameterValue);
                        pFilterDescriptor->setParameter(parameterName, parameterVariant);
                     }
                  }
                  if (XMLString::equals(pProgram->getNodeName(), X("program")))
                  {
                     DOMElement* pProgramElement(static_cast<DOMElement*>(pProgram));
                     if (pProgramElement != NULL)
                     {
                        string gpuProgramName(A(pProgramElement->getAttribute(X("name"))));
                        string gpuProgramType(A(pProgramElement->getAttribute(X("type"))));
                        GpuProgramDescriptor::GpuProgramType programType = GpuProgramDescriptor::VERTEX_PROGRAM;

                        GpuProgramDescriptorAdapter* pGpuDescriptor = new GpuProgramDescriptorAdapter();
                        VERIFYRV(pGpuDescriptor != NULL, vector<ImageFilterDescriptorImp*>());
                        if (gpuProgramName.empty() == false)
                        {
                           pGpuDescriptor->setName(gpuProgramName);
                           if (gpuProgramType == "Fragment")
                           {
                              programType = GpuProgramDescriptor::FRAGMENT_PROGRAM;
                           }
                           pGpuDescriptor->setType(programType);

                           // get input parameters specific to Gpu program
                           for (DOMNode* pParameter = pProgram->getFirstChild();
                              pParameter != NULL;
                              pParameter = pParameter->getNextSibling())
                           {
                              if (XMLString::equals(pParameter->getNodeName(), X("parameter")))
                              {
                                 DOMElement* pParamElement(static_cast<DOMElement*>(pParameter));
                                 if (pParamElement != NULL)
                                 {
                                    string parameterName(A(pParamElement->getAttribute(X("name"))));
                                    string parameterType(A(pParamElement->getAttribute(X("type"))));
                                    string parameterValue(A(pParamElement->getAttribute(X("value"))));
                                    DataVariant parameterVariant(parameterType, NULL);
                                    parameterVariant.fromXmlString(parameterType, parameterValue);
                                    pGpuDescriptor->setParameter(parameterName, parameterVariant);
                                 }
                              }
                           }
                           bool compiled = true;
#if defined(CG_SUPPORTED)
                           compiled = CgContext::instance()->isCgVertexProgram(pGpuDescriptor->getName()) ||
                              CgContext::instance()->isCgFragmentProgram(pGpuDescriptor->getName());
#endif
                           if (compiled)
                           {
                              pFilterDescriptor->addGpuProgram(pGpuDescriptor);
                           }
                           else
                           {
                              MessageResource msg("Unable to load GPU filter", "app",
                                 "7FA00287-DF55-46A9-AA82-D3CBFA34842F");
                              msg->addProperty("filter name", pGpuDescriptor->getName());
#if defined(CG_SUPPORTED)
                              msg->addProperty("error message", CgContext::instance()->getLastCgErrorMessage());
#endif
                              // make sure this filter descriptor does not go in the list
                              pFilterDescriptor->setName("");
                              delete pGpuDescriptor;
                              pGpuDescriptor = NULL;
                           }
                        }
                     }
                  }
               }

               if (pFilterDescriptor->getName().empty() != true)
               {
                  filterDescriptors.push_back(pFilterDescriptor);
               }
            } 
         }
      }
   }
   catch (XmlReader::DomParseException& parseException)
   {
      VERIFYNR_MSG(false == true, parseException.str().c_str());
   }

   return filterDescriptors;
}
