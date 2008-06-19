/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataElement.h"
#include "DataElementGroup.h"
#include "DataElementGroupImp.h"
#include "ModelServicesImp.h"
#include "SessionManager.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

DataElementGroupImp::DataElementGroupImp(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id),
   mEnforceUniformity(false),
   mNotificationEnabled(true)
{
}

DataElementGroupImp::~DataElementGroupImp()
{
   mNotificationEnabled = false;
   clear(true);
}

bool DataElementGroupImp::insertElement(DataElement* pElement)
{
   VERIFY(pElement != NULL);

   if (mEnforceUniformity && !mElements.empty())
   {
      if (pElement->getObjectType() != mElements.front()->getObjectType())
      {
         return false;
      }
   }
   pElement->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataElementGroupImp::elementDeleted));

   mElements.push_back(pElement);
   if (mNotificationEnabled) 
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
   return true;
}

bool DataElementGroupImp::insertElements(const std::vector<DataElement*>& elements)
{
   vector<DataElement*>::const_iterator ppElement;
   string type;
   if (mEnforceUniformity == true)
   {
      if (mElements.empty() == false)
      {
         type = mElements.front()->getObjectType();
      }
      else if (elements.empty() == false)
      {
         type = elements.front()->getObjectType();
      }
   }
   for (ppElement=elements.begin(); ppElement!=elements.end(); ++ppElement)
   {
      if (!NN(*ppElement) || (mEnforceUniformity && (*ppElement)->getObjectType() != type)) 
      {
         return false;
      }
   }
   for (ppElement=elements.begin(); ppElement!=elements.end(); ++ppElement)
   {
      (*ppElement)->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataElementGroupImp::elementDeleted));
      mElements.push_back(*ppElement);
   }
   if (!elements.empty())
   {
      if (mNotificationEnabled)    
      {
         notify(SIGNAL_NAME(Subject, Modified));
      }
   }
   return true;
}

bool DataElementGroupImp::hasElement(DataElement* pElement) const
{
   return find(mElements.begin(), mElements.end(), pElement) != mElements.end();
}

unsigned int DataElementGroupImp::getNumElements() const
{
   return mElements.size();
}

const std::vector<DataElement*>& DataElementGroupImp::getElements() const
{
   return mElements;
}

bool DataElementGroupImp::removeElement(DataElement* pElement, bool bDelete)
{
   ModelServicesImp *pModel = ModelServicesImp::instance();
   vector<DataElement*>::iterator ppElement = find(mElements.begin(), mElements.end(), pElement);
   if (ppElement != mElements.end())
   {
      if (NN(*ppElement))
      {
         (*ppElement)->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataElementGroupImp::elementDeleted));
         if (bDelete && pModel)
         {
            pModel->destroyElement(*ppElement);
         }
      }
      mElements.erase(ppElement);
      if (mNotificationEnabled)    
      {
         notify(SIGNAL_NAME(Subject, Modified));
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool DataElementGroupImp::removeElements(const std::vector<DataElement*>& elements, bool bDelete)
{
   if (elements.empty())
   {
      return false;
   }

   bool modified(false);
   bool success(true);
   vector<DataElement*>::const_iterator ppElement;
   vector<DataElement*>::iterator ppRemoval;
   ModelServicesImp *pModel = ModelServicesImp::instance();

   for (ppElement=elements.begin(); ppElement!=elements.end(); ++ppElement)
   {
      ppRemoval = find(mElements.begin(), mElements.end(), *ppElement);
      if (ppRemoval != mElements.end())
      {
         if (NN(*ppRemoval))
         {
            (*ppRemoval)->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataElementGroupImp::elementDeleted));
            if (bDelete)
            {
               pModel->destroyElement(*ppRemoval);
            }
            mElements.erase(ppRemoval);
            modified = true;
         }
      }
      else
      {
         success = false;
      }
   }

   if (modified)
   {
      if (mNotificationEnabled)    
      {
         notify(SIGNAL_NAME(Subject, Modified));
      }
   }

   return success;
}

void DataElementGroupImp::clear(bool bDelete)
{
   if (mElements.empty())
   {
      return;
   }

   ModelServicesImp *pModel = ModelServicesImp::instance();
   vector<DataElement*>::iterator ppElement;
   for (ppElement=mElements.begin(); ppElement!=mElements.end(); ++ppElement)
   {
      if (NN(*ppElement))
      {
         (*ppElement)->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataElementGroupImp::elementDeleted));
         if (bDelete && pModel)
         {
            pModel->destroyElement(*ppElement);
         }
      }
   }

   mElements.clear();
   if (mNotificationEnabled)    
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void DataElementGroupImp::elementDeleted(Subject &subject, const string &signal, const boost::any &data)
{
   vector<DataElement*>::iterator ppElement;
   for (ppElement=mElements.begin(); ppElement!=mElements.end(); ++ppElement)
   {
      VERIFYNRV(*ppElement != NULL);
      if (*ppElement == &subject)
      {
         mElements.erase(ppElement);
         if (mNotificationEnabled)
         {
            notify(SIGNAL_NAME(DataElementGroup, ElementDeleted), boost::any(dynamic_cast<DataElement*>(&subject)));
         }
         break;
      }
   }
}

DataElement* DataElementGroupImp::copy(const string& name, DataElement *pParent) const
{
   return NULL;
}

bool DataElementGroupImp::enforceUniformity(bool enforce)
{
   if (enforce == mEnforceUniformity)
   {
      return true;
   }

   if (enforce == false)
   {
      mEnforceUniformity = enforce;
      return true;
   }
   else
   {
      if (!isUniform())
      {
         return false;
      }
      mEnforceUniformity = enforce;
      return true;
   }
}

bool DataElementGroupImp::isUniform() const
{
   if (mEnforceUniformity == true)
   {
      return true;
   }

   if (mElements.size() > 1)
   {
      vector<DataElement*>::const_iterator ppElement = mElements.begin();
      VERIFY(*ppElement!= NULL);
      const string &type = (*ppElement)->getObjectType();
      ++ppElement;
      for (; ppElement!=mElements.end(); ++ppElement)
      {
         VERIFY(*ppElement!= NULL);
         if ((*ppElement)->getObjectType() != type)
         {
            return false;
         }
      }
   }
   return true;
}

bool DataElementGroupImp::toXml(XMLWriter* pXml) const
{
   if(!DataElementImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("enforceUniformity", mEnforceUniformity);
   for(vector<DataElement*>::const_iterator it = mElements.begin(); it != mElements.end(); ++it)
   {
      pXml->addAttr("id", (*it)->getId(), pXml->addElement("element"));
   }
   return true;
}

bool DataElementGroupImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if(pDocument == NULL)
   {
      return false;
   }
   mEnforceUniformity = StringUtilities::fromXmlString<bool>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("enforceUniformity"))));
   for(DOMNode *pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if(XMLString::equals(pChld->getNodeName(), X("element")))
      {
         DataElement *pElement = dynamic_cast<DataElement*>(
            Service<SessionManager>()->getSessionItem(A(static_cast<DOMElement*>(pChld)->getAttribute(X("id")))));
         if(pElement == NULL)
         {
            return false;
         }
         mElements.push_back(pElement);
      }
   }
   return true;
}

const string& DataElementGroupImp::getObjectType() const
{
   static string type("DataElementGroupImp");
   return type;
}

bool DataElementGroupImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DataElementGroup"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

void DataElementGroupImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("DataElementGroup");
   DataElementImp::getElementTypes(classList);
}

bool DataElementGroupImp::isKindOfElement(const string& className)
{
   if ((className == "DataElementGroupImp") || (className == "DataElementGroup"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}
