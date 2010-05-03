/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModelServicesImp.h"
#include "AnnotationElementAdapter.h"
#include "AnyAdapter.h"
#include "AoiElementAdapter.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DataDescriptorAdapter.h"
#include "DataElementAdapter.h"
#include "DataElementGroupAdapter.h"
#include "FileDescriptorImp.h"
#include "GcpListAdapter.h"
#include "ImportDescriptorImp.h"
#include "MessageLog.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptorAdapter.h"
#include "RasterElementAdapter.h"
#include "RasterFileDescriptorImp.h"
#include "SafePtr.h"
#include "SessionItemSerializer.h"
#include "SessionItemDeserializer.h"
#include "SignatureAdapter.h"
#include "SignatureLibraryAdapter.h"
#include "SignatureSetAdapter.h"
#include "switchOnEncoding.h"
#include "TiePointListAdapter.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <boost/bind.hpp>
#include <queue>

using namespace std;

ModelServicesImp* ModelServicesImp::spInstance = NULL;
bool ModelServicesImp::mDestroyed = false;

ModelServicesImp* ModelServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use ModelServices after "
            "destroying it.");
      }
      spInstance = new ModelServicesImp();
   }

   return spInstance;
}

void ModelServicesImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy ModelServices after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

ModelServicesImp::ModelServicesImp() : 
   SettableSessionItemAdapter("{543BF9C3-2861-4240-ADD7-6748C3BF4F90}")
{
   mElementTypes.push_back("AnnotationElement");
   mElementTypes.push_back("Any");
   mElementTypes.push_back("AoiElement");
   mElementTypes.push_back("DataElement");
   mElementTypes.push_back("DataElementGroup");
   mElementTypes.push_back("GcpList");
   mElementTypes.push_back("RasterElement");
   mElementTypes.push_back("Signature");
   mElementTypes.push_back("SignatureLibrary");
   mElementTypes.push_back("SignatureSet");
   mElementTypes.push_back("TiePointList");
}

ModelServicesImp::~ModelServicesImp()
{
   clear();
}

const string& ModelServicesImp::getObjectType() const
{
   static string sType = "ModelServicesImp";
   return sType;
}

bool ModelServicesImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ModelServices"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

DataDescriptor* ModelServicesImp::createDataDescriptor(const string& name, const string& type,
                                                       DataElement* pParent) const
{
   DataDescriptor* pDescriptor = NULL;

   if (isKindOfElement(type, "RasterElement") == true)
   {
      pDescriptor = new RasterDataDescriptorAdapter(name, type, pParent);
   }
   else if (isKindOfElement(type, "DataElement") == true)
   {
      pDescriptor = new DataDescriptorAdapter(name, type, pParent);
   }

   return pDescriptor;
}

/*
DataDescriptor* ModelServicesImp::createDataDescriptor(const string& name, const string& type,
                                                       const vector<string>& parent) const
{
   DataDescriptor* pDescriptor = NULL;
   if (isKindOfElement(type, "RasterElement"))
   {
      pDescriptor = new RasterDataDescriptorAdapter(name, type, parent);
   }
   else if (isKindOfElement(type, "DataElement"))
   {
      pDescriptor = new DataDescriptorAdapter(name, type, parent);
   }
   return pDescriptor;
}
*/

void ModelServicesImp::destroyDataDescriptor(DataDescriptor* pDescriptor) const
{
   DataDescriptorImp* pDescriptorImp = dynamic_cast<DataDescriptorImp*>(pDescriptor);
   if (pDescriptorImp != NULL)
   {
      delete pDescriptorImp;
   }
}

ImportDescriptor* ModelServicesImp::createImportDescriptor(const string& name, const string& type,
                                                           DataElement* pParent, bool bImported) const
{
   ImportDescriptor* pImportDescriptor = NULL;

   DataDescriptor* pDescriptor = createDataDescriptor(name, type, pParent);
   if (pDescriptor != NULL)
   {
      pImportDescriptor = createImportDescriptor(pDescriptor, bImported);
   }

   return pImportDescriptor;
}

/*
ImportDescriptor* ModelServicesImp::createImportDescriptor(const string& name, const string& type,
                                                           const vector<string> &parent, bool bImported) const
{
   ImportDescriptor* pImportDescriptor = NULL;

   DataDescriptor* pDescriptor = createDataDescriptor(name, type, parent);
   if (pDescriptor != NULL)
   {
      pImportDescriptor = createImportDescriptor(pDescriptor, bImported);
   }

   return pImportDescriptor;
}
*/

ImportDescriptor* ModelServicesImp::createImportDescriptor(DataDescriptor* pDescriptor, bool bImported) const
{
   ImportDescriptor* pImportDescriptor = NULL;
   if (pDescriptor != NULL)
   {
      pImportDescriptor = new ImportDescriptorImp(pDescriptor);
      if (pImportDescriptor != NULL)
      {
         pImportDescriptor->setImported(bImported);
      }
   }

   return pImportDescriptor;
}

void ModelServicesImp::destroyImportDescriptor(ImportDescriptor* pImportDescriptor) const
{
   ImportDescriptorImp* pDescriptorImp = dynamic_cast<ImportDescriptorImp*>(pImportDescriptor);
   if (pDescriptorImp != NULL)
   {
      delete pDescriptorImp;
   }
}

bool ModelServicesImp::addElementType(const string& elementType)
{
   if (elementType.empty() == false)
   {
      if (hasElementType(elementType) == false)
      {
         mElementTypes.push_back(elementType);
         return true;
      }
   }

   return false;
}

bool ModelServicesImp::hasElementType(const string& elementType) const
{
   vector<string>::const_iterator iter = find(mElementTypes.begin(), mElementTypes.end(), elementType);
   return (iter != mElementTypes.end());
}

const vector<string>& ModelServicesImp::getValidElementTypes() const
{
   return mElementTypes;
}

namespace
{
   struct ElementDepthComparitor : less<vector<string>::size_type >
   {
      bool operator()(DataDescriptor* pA, DataDescriptor* pB)
      {
         if (pA == NULL || pB == NULL)
         {
            return false;
         }
         return less<vector<string>::size_type>::operator()(pA->getParentDesignator().size(), pB->getParentDesignator().size());
      }
   };
};

vector<DataElement*> ModelServicesImp::createElements(const vector<DataDescriptor*> &descriptors)
{
   vector<DataElement*> elements;

   vector<DataDescriptor*> sortedDescriptors = descriptors;
   stable_sort(sortedDescriptors.begin(), sortedDescriptors.end(), ElementDepthComparitor());

   for (vector<DataDescriptor*>::iterator it = sortedDescriptors.begin(); it != sortedDescriptors.end(); ++it)
   {
      DataElement* pElement = createElement(*it);
      if (pElement != NULL)
      {
         elements.push_back(pElement);
      }
   }
   return elements;
}

DataElement* ModelServicesImp::createElement(const DataDescriptor* pDescriptor, const string& id)
{
   const DataDescriptorImp* pDescriptorImp = dynamic_cast<const DataDescriptorImp*>(pDescriptor);
   if (pDescriptorImp == NULL)
   {
      return NULL;
   }
   if (pDescriptorImp->getParent() == NULL && !pDescriptorImp->getParentDesignator().empty())
   {
      // const_cast since the designator and parent pointer are usually in synch except before creation
      // this synchs them back up...this is an implementation detail
      const_cast<DataDescriptorImp*>(pDescriptorImp)->setParent(getElement(pDescriptorImp->getParentDesignator(),
         string()));
   }

   // Check if an element with the same name, type, and parent already exists
   const string& name = pDescriptorImp->getName();
   const string& type = pDescriptorImp->getType();
   DataElement* pParent = pDescriptorImp->getParent();

   if (getElement(name, type, pParent) != NULL)
   {
      return NULL;
   }

   // Create the element
   DataElement* pElement = NULL;
   if ((type == "AnnotationElement") || (type == "AnnotationElementAdapter"))
   {
      pElement = new AnnotationElementAdapter(*pDescriptorImp, id);
   }
   else if ((type == "AoiElement") || (type == "AoiElementAdapter"))
   {
      pElement = new AoiElementAdapter(*pDescriptorImp, id);
   }
   else if ((type == "DataElement") || (type == "DataElementAdapter"))
   {
      pElement = new DataElementAdapter(*pDescriptorImp, id);
   }
   else if ((type == "DataElementGroup") || (type == "DataElementGroupAdapter"))
   {
      pElement = new DataElementGroupAdapter(*pDescriptorImp, id);
   }
   else if ((type == "GcpList") || (type == "GcpListAdapter"))
   {
      pElement = new GcpListAdapter(*pDescriptorImp, id);
   }
   else if ((type == "RasterElement") || (type == "RasterElementAdapter"))
   {
      const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
      if (pRasterDescriptor != NULL)
      {
         RasterElement* pRaster = new RasterElementAdapter(*pDescriptorImp, id);

         if (pRaster != NULL)
         {
            if (pDescriptorImp->getProcessingLocation() == IN_MEMORY)
            {
               if (!pRaster->createInMemoryPager())
               {
                  delete dynamic_cast<RasterElementImp*>(pRaster);
                  pRaster = NULL;
               }
            }
         }
         pElement = pRaster;
      }
   }
   else if ((type == "Signature") || (type == "SignatureAdapter"))
   {
      pElement = new SignatureAdapter(*pDescriptorImp, id);
   }
   else if ((type == "SignatureLibrary") || (type == "SignatureLibraryAdapter"))
   {
      pElement = new SignatureLibraryAdapter(*pDescriptorImp, id);
   }
   else if ((type == "SignatureSet") || (type == "SignatureSetAdapter"))
   {
      pElement = new SignatureSetAdapter(*pDescriptorImp, id);
   }
   else if ((type == "TiePointList") || (type == "TiePointListAdapter"))
   {
      pElement = new TiePointListAdapter(*pDescriptorImp, id);
   }
   else if ((type == "Any") || (type == "AnyAdapter") || (hasElementType(type) == true))
   {
      pElement = new AnyAdapter(type, *pDescriptorImp, id);
   }

   if (pElement != NULL)
   {
      // Add the element to the session
      addElement(pElement);

      // Notify of change
      notify(SIGNAL_NAME(ModelServices, ElementCreated), boost::any(pElement));
   }

   return pElement;
}

DataElement* ModelServicesImp::createElement(const DataDescriptor *pDescriptor)
{
   return createElement(pDescriptor, SessionItemImp::generateUniqueId());
}

DataElement* ModelServicesImp::createElement(const string& name, const string& type, DataElement* pParent)
{
   // Create the data descriptor
   DataDescriptor* pDescriptor = createDataDescriptor(name, type, pParent);
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   // Create the element
   DataElement* pElement = createElement(pDescriptor);

   // Destroy the data descriptor since creating the element does a deep copy
   destroyDataDescriptor(pDescriptor);

   return pElement;
}

multimap<ModelServicesImp::Key, DataElement*>::iterator ModelServicesImp::findElement(DataElement* pElement)
{
   if (pElement == NULL)
   {
      return mElements.end();
   }

   Key key(*pElement);
   return findElement(key, pElement->getObjectType());
}

multimap<ModelServicesImp::Key, DataElement*>::iterator ModelServicesImp::findElement(const Key& key,
                                                                                      const string& type)
{
   multimap<Key, DataElement*>::iterator iter = mElements.find(key);
   if (type.empty() == true)
   {
      return iter;
   }
   else
   {
      while (iter != mElements.end() && key == iter->first)
      {
         if (iter->second->isKindOf(type) == true)
         {
            return iter;
         }

         ++iter;
      }
   }

   return mElements.end();
}

multimap<ModelServicesImp::Key, DataElement*>::const_iterator ModelServicesImp::findElement(const Key& key,
                                                                                            const string& type) const
{
   multimap<Key, DataElement*>::const_iterator iter = mElements.find(key);
   if (type.empty() == true)
   {
      return iter;
   }
   else
   {
      while (iter != mElements.end() && key == iter->first)
      {
         if (iter->second->isKindOf(type) == true)
         {
            return iter;
         }

         ++iter;
      }
   }

   return mElements.end();
}

DataElement* ModelServicesImp::getElement(const string& name, const string& type, DataElement* pParent) const
{
   if (name.empty() == true)
   {
      return false;
   }

   Key key(name, pParent);

   multimap<Key, DataElement*>::const_iterator iter = findElement(key, type);
   if (iter != mElements.end())
   {
      return iter->second;
   }

   return NULL;
}

DataElement* ModelServicesImp::getElement(const vector<string>& designator, const string &type) const
{
   if (designator.empty())
   {
      return false;
   }
   DataElement* pElement = NULL;
   for (vector<string>::const_iterator part = designator.begin(); part != designator.end(); ++part)
   {
      pElement = getElement(*part, string(), pElement);
   }
   return pElement;
}

vector<DataElement*> ModelServicesImp::getElements(const string& type) const
{
   vector<DataElement*> elements;

   multimap<Key, DataElement*>::const_iterator iter;
   for (iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      DataElement* pElement = iter->second;
      if (pElement != NULL)
      {
         if ((type.empty() == true) || (pElement->isKindOf(type) == true))
         {
            elements.push_back(pElement);
         }
      }
   }

   return elements;
}

vector<DataElement*> ModelServicesImp::getElements(DataElement* pParent, const string& type) const
{
   vector<DataElement*> elements;

   multimap<Key, DataElement*>::const_iterator iter;
   for (iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      DataElement* pElement = iter->second;
      if (pElement != NULL)
      {
         DataElement* pCurrentParent = pElement->getParent();
         if (pCurrentParent == pParent)
         {
            if ((type.empty() == true) || (pElement->isKindOf(type) == true))
            {
               elements.push_back(pElement);
            }
         }
      }
   }

   return elements;
}

vector<DataElement*> ModelServicesImp::getElements(const string& filename, const string& type) const
{
   vector<DataElement*> elements;

   multimap<Key, DataElement*>::const_iterator iter;
   for (iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      DataElement* pElement = iter->second;
      if (pElement != NULL)
      {
         string currentFilename = pElement->getFilename();
         string lowerCurrentFilename = StringUtilities::toLower(currentFilename);
         string lowerElementFilename = StringUtilities::toLower(filename);

         if (lowerCurrentFilename == lowerElementFilename)
         {
            if ((type.empty() == true) || (pElement->isKindOf(type) == true))
            {
               elements.push_back(pElement);
            }
         }
      }
   }

   return elements;
}

vector<string> ModelServicesImp::getElementNames(const string& type) const
{
   vector<string> elementNames;

   multimap<Key, DataElement*>::const_iterator iter;
   for (iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      DataElement* pElement = iter->second;
      if (pElement != NULL)
      {
         if ((type.empty() == true) || (pElement->isKindOf(type) == true))
         {
            elementNames.push_back(pElement->getName());
         }
      }
   }

   return elementNames;
}

vector<string> ModelServicesImp::getElementNames(DataElement* pParent, const string& type) const
{
   vector<string> elementNames;

   multimap<Key, DataElement*>::const_iterator iter;
   for (iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      DataElement* pElement = iter->second;
      if (pElement != NULL)
      {
         DataElement* pCurrentParent = pElement->getParent();
         if (pCurrentParent == pParent)
         {
            if ((type.empty() == true) || (pElement->isKindOf(type) == true))
            {
               elementNames.push_back(pElement->getName());
            }
         }
      }
   }

   return elementNames;
}

vector<string> ModelServicesImp::getElementNames(const string& filename, const string& type) const
{
   vector<string> elementNames;

   multimap<Key, DataElement*>::const_iterator iter;
   for (iter = mElements.begin(); iter != mElements.end(); ++iter)
   {
      DataElement* pElement = iter->second;
      if (pElement != NULL)
      {
         string currentFilename = pElement->getFilename();
         string lowerCurrentFilename = StringUtilities::toLower(currentFilename);
         string lowerElementFilename = StringUtilities::toLower(filename);

         if (lowerCurrentFilename == lowerElementFilename)
         {
            if ((type.empty() == true) || (pElement->isKindOf(type) == true))
            {
               elementNames.push_back(pElement->getName());
            }
         }
      }
   }

   return elementNames;
}

bool ModelServicesImp::setElementName(DataElement* pElement, const string& name)
{
   DataDescriptorImp* pDescriptor = dynamic_cast<DataDescriptorImp*>(pElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   if (getElement(name, pDescriptor->getType(), pDescriptor->getParent()) != NULL)
   {
      return false;
   }

   VERIFY(removeElement(pElement));
   dynamic_cast<DataElementImp*>(pElement)->setName(name);
   VERIFY(addElement(pElement));
   
   return true;
}

bool ModelServicesImp::setElementParent(DataElement* pElement, DataElement *pParent)
{
   DataDescriptorImp* pDescriptor = dynamic_cast<DataDescriptorImp*>(pElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   if (getElement(pDescriptor->getName(), pDescriptor->getType(), pParent) != NULL)
   {
      return false;
   }

   VERIFY(removeElement(pElement));
   pDescriptor->setParent(pParent);
   VERIFY(addElement(pElement));

   notify(SIGNAL_NAME(ModelServices, ElementReparented), boost::any(pElement));
   return true;
}

bool ModelServicesImp::removeElement(DataElement* pElement)
{
   if (pElement == NULL)
   {
      return false;
   }

   multimap<Key, DataElement*>::iterator iter = findElement(pElement);
   if (iter != mElements.end())
   {
      mElements.erase(iter);
      return true;
   }

   return false;
}

bool ModelServicesImp::destroyElement(DataElement* pElement)
{
   if (pElement == NULL)
   {
      return false;
   }

   SafePtr<DataElement> pParent(pElement);

   // Destroy the element's children
   vector<DataElement*> children = getElements(pParent.get(), string());
   while ((pParent.get() != NULL) && (children.empty() == false))
   {
      DataElement* pChild = children.front();
      if (pChild != NULL)
      {
         destroyElement(pChild);
      }

      children = getElements(pParent.get(), string());
   }

   // Check if the parent element was destroyed as a result of destroying its children
   if (pParent.get() == NULL)
   {
      return true;
   }

   // Destroy the element
   bool bSuccess = removeElement(pElement);
   if (bSuccess == true)
   {
      DataElementImp* pElementImp = dynamic_cast<DataElementImp*>(pElement);
      if (pElementImp != NULL)
      {
         notify(SIGNAL_NAME(ModelServices, ElementDestroyed), boost::any(pElement));
         delete pElementImp;
      }
   }

   return bSuccess;
}

void ModelServicesImp::clear()
{
   // Destroy only the top-level elements since they will destroy all child elements
   while (mElements.empty() == false)
   {
      for (multimap<Key, DataElement*>::iterator iter = mElements.begin(); iter != mElements.end(); ++iter)
      {
         DataElement* pElement = iter->second;
         if ((pElement != NULL) && (pElement->getParent() == NULL))
         {
            destroyElement(pElement);
            break;
         }
      }
   }

   mElements.clear();
}

char* ModelServicesImp::getMemoryBlock(size_t size)
{
   if (size == 0)
   {
      return NULL;
   }

   char* pBlock = new (nothrow) char[size];
   if (pBlock != NULL)
   {
      memset(pBlock, 0, size);
   }

   return pBlock;
}

void ModelServicesImp::deleteMemoryBlock(char* memory)
{
   delete memory;
}

double ModelServicesImp::getDataValue(EncodingType type, void* pData, int iIndex) const
{
   return getDataValue(type, pData, COMPLEX_MAGNITUDE, iIndex);
}

double ModelServicesImp::getDataValue(EncodingType type, void* pData, ComplexComponent component, int iIndex) const
{
   double dValue = 0.0;
   switchOnComplexEncoding(type, ModelServices::getDataValue, pData, component, iIndex, dValue);

   return dValue;
}

bool ModelServicesImp::isKindOfElement(const string& className, const string& elementName) const
{
   bool bSuccess = false;

   if ((className == "AnnotationElement") || (className == "AnnotationElementAdapter") ||
      (className == "AnnotationElementImp"))
   {
      bSuccess = AnnotationElementImp::isKindOfElement(elementName);
   }
   else if ((className == "Any") || (className == "AnyAdapter") || (className == "AnyImp"))
   {
      bSuccess = AnyImp::isKindOfElement(elementName);
   }
   else if ((className == "AoiElement") || (className == "AoiElementAdapter") || (className == "AoiElementImp"))
   {
      bSuccess = AoiElementImp::isKindOfElement(elementName);
   }
   else if ((className == "DataElement") || (className == "DataElementAdapter") || (className == "DataElementImp"))
   {
      bSuccess = DataElementImp::isKindOfElement(elementName);
   }
   else if ((className == "DataElementGroup") || (className == "DataElementGroupAdapter") ||
      (className == "DataElementGroupImp"))
   {
      bSuccess = DataElementGroupImp::isKindOfElement(elementName);
   }
   else if ((className == "GcpList") || (className == "GcpListAdapter") || (className == "GcpListImp"))
   {
      bSuccess = GcpListImp::isKindOfElement(elementName);
   }
   else if ((className == "GraphicElement") || (className == "GraphicElementAdapter") ||
      (className == "GraphicElementImp"))
   {
      bSuccess = GraphicElementImp::isKindOfElement(elementName);
   }
   else if ((className == "RasterElement") || (className == "RasterElementAdapter") ||
      (className == "RasterElementImp"))
   {
      bSuccess = RasterElementImp::isKindOfElement(elementName);
   }
   else if ((className == "Signature") || (className == "SignatureAdapter") || (className == "SignatureImp"))
   {
      bSuccess = SignatureImp::isKindOfElement(elementName);
   }
   else if ((className == "SignatureLibrary") || (className == "SignatureLibraryAdapter") ||
      (className == "SignatureLibraryImp"))
   {
      bSuccess = SignatureLibraryImp::isKindOfElement(elementName);
   }
   else if ((className == "SignatureSet") || (className == "SignatureSetAdapter") || (className == "SignatureSetImp"))
   {
      bSuccess = SignatureSetImp::isKindOfElement(elementName);
   }
   else if ((className == "TiePointList") || (className == "TiePointListAdapter") || (className == "TiePointListImp"))
   {
      bSuccess = TiePointListImp::isKindOfElement(elementName);
   }
   else if (hasElementType(className) == true)
   {
      if (className == elementName)
      {
         return true;
      }

      bSuccess = AnyImp::isKindOfElement(elementName);
   }

   return bSuccess;
}

void ModelServicesImp::getElementTypes(const string& className, vector<string>& classList) const
{
   if ((className == "AnnotationElement") || (className == "AnnotationElementAdapter")
      || (className == "AnnotationElementImp"))
   {
      AnnotationElementImp::getElementTypes(classList);
   }
   else if ((className == "Any") || (className == "AnyAdapter") || (className == "AnyImp"))
   {
      AnyImp::getElementTypes(classList);
   }
   else if ((className == "AoiElement") || (className == "AoiElementAdapter")
      || (className == "AoiElementImp"))
   {
      AoiElementImp::getElementTypes(classList);
   }
   else if ((className == "DataElement") || (className == "DataElementAdapter") || (className == "DataElementImp"))
   {
      DataElementImp::getElementTypes(classList);
   }
   else if ((className == "DataElementGroup") || (className == "DataElementGroupAdapter") ||
      (className == "DataElementGroupImp"))
   {
      DataElementGroupImp::getElementTypes(classList);
   }
   else if ((className == "GcpList") || (className == "GcpListAdapter") || (className == "GcpListImp"))
   {
      GcpListImp::getElementTypes(classList);
   }
   else if ((className == "GraphicElement") || (className == "GraphicElementAdapter") ||
      (className == "GraphicElementImp"))
   {
      GraphicElementImp::getElementTypes(classList);
   }
   else if ((className == "RasterElement") || (className == "RasterElementAdapter") ||
      (className == "RasterElementImp"))
   {
      RasterElementImp::getElementTypes(classList);
   }
   else if ((className == "Signature") || (className == "SignatureAdapter") || (className == "SignatureImp"))
   {
      SignatureImp::getElementTypes(classList);
   }
   else if ((className == "SignatureLibrary") || (className == "SignatureLibraryAdapter") ||
      (className == "SignatureLibraryImp"))
   {
      SignatureLibraryImp::getElementTypes(classList);
   }
   else if ((className == "SignatureSet") || (className == "SignatureSetAdapter") ||
      (className == "SignatureSetImp"))
   {
      SignatureSetImp::getElementTypes(classList);
   }
   else if ((className == "TiePointList") || (className == "TiePointListAdapter") || (className == "TiePointListImp"))
   {
      TiePointListImp::getElementTypes(classList);
   }
   else if (hasElementType(className) == true)
   {
      classList.push_back(className);
      AnyImp::getElementTypes(classList);
   }
}

bool ModelServicesImp::isKindOfDataDescriptor(const string& className, const string& descriptorName) const
{
   bool bSuccess = false;

   if ((className == "DataDescriptor") || (className == "DataDescriptorAdapter") || (className == "DataDescriptorImp"))
   {
      bSuccess = DataDescriptorImp::isKindOfDataDescriptor(descriptorName);
   }
   else if ((className == "RasterDataDescriptor") || (className == "RasterDataDescriptorAdapter") ||
      (className == "RasterDataDescriptorImp"))
   {
      bSuccess = RasterDataDescriptorImp::isKindOfDataDescriptor(descriptorName);
   }

   return bSuccess;
}

void ModelServicesImp::getDataDescriptorTypes(const string& className, vector<string>& classList) const
{
   if ((className == "DataDescriptor") || (className == "DataDescriptorAdapter") || (className == "DataDescriptorImp"))
   {
      DataDescriptorImp::getDataDescriptorTypes(classList);
   }
   else if ((className == "RasterDataDescriptor") || (className == "RasterDataDescriptorAdapter") ||
      (className == "RasterDataDescriptorImp"))
   {
      RasterDataDescriptorImp::getDataDescriptorTypes(classList);
   }
}

bool ModelServicesImp::isKindOfFileDescriptor(const string& className, const string& descriptorName) const
{
   bool bSuccess = false;

   if ((className == "FileDescriptor") || (className == "FileDescriptorAdapter") || (className == "FileDescriptorImp"))
   {
      bSuccess = FileDescriptorImp::isKindOfFileDescriptor(descriptorName);
   }
   else if ((className == "RasterFileDescriptor") || (className == "RasterFileDescriptorAdapter") ||
      (className == "RasterFileDescriptorImp"))
   {
      bSuccess = RasterFileDescriptorImp::isKindOfFileDescriptor(descriptorName);
   }

   return bSuccess;
}

void ModelServicesImp::getFileDescriptorTypes(const string& className, vector<string>& classList) const
{
   if ((className == "FileDescriptor") || (className == "FileDescriptorAdapter") || (className == "FileDescriptorImp"))
   {
      FileDescriptorImp::getFileDescriptorTypes(classList);
   }
   else if ((className == "RasterFileDescriptor") || (className == "RasterFileDescriptorAdapter") ||
      (className == "RasterFileDescriptorImp"))
   {
      RasterFileDescriptorImp::getFileDescriptorTypes(classList);
   }
}

bool ModelServicesImp::addElement(DataElement* pElement)
{
   if (pElement == NULL)
   {
      return false;
   }

   Key key(*pElement);
   mElements.insert(pair<const Key, DataElement*>(key, pElement));
   return true;
}

bool ModelServicesImp::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter writer("ModelServices");
   XML_ADD_CONTAINER(writer, elementTypes, mElementTypes.begin(), mElementTypes.end());
   return serializer.serialize(writer);
}

bool ModelServicesImp::deserialize(SessionItemDeserializer& deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, "ModelServices");
   if (pRootElement)
   {
      mElementTypes.clear();
      readContainerElements(pRootElement, "elementTypes", back_inserter(mElementTypes));
      return true;
   }
   return false;
}
