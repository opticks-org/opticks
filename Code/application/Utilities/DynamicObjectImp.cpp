/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DynamicObjectImp.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DateTimeImp.h"
#include "DynamicObject.h"
#include "DynamicObjectAdapter.h"
#include "FilenameImp.h"
#include "ObjectResource.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "TypeConverter.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

template <class T>
void copyValue(T* pValue, void*& pCopy)
{
   if (pValue == NULL)
   {
      return;
   }

   T* pNewValue = new T;
   if (pNewValue != NULL)
   {
      *pNewValue = *pValue;
   }

   pCopy = pNewValue;
}

template <class T>
void copyVector(vector<T>* pValue, void*& pCopy)
{
   if (pValue == NULL)
   {
      return;
   }

   vector<T>* pVector = NULL;
   pVector = new vector<T>;
   if (pVector != NULL)
   {
      unsigned int uiCount = 0;
      uiCount = pValue->size();
      for (unsigned int i = 0; i < uiCount; i++)
      {
         T newValue = pValue->at(i);
         pVector->push_back(newValue);
      }
   }

   pCopy = pVector;
}

DynamicObjectImp::DynamicObjectImp() :
   mpParent(NULL)
{
}

DynamicObjectImp::~DynamicObjectImp()
{
   clear();
}

DynamicObjectImp& DynamicObjectImp::operator=(const DynamicObjectImp& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   clear();
   map<string, DataVariant>::const_iterator pPair;
   for (pPair = rhs.mVariantAttributes.begin(); pPair != rhs.mVariantAttributes.end(); ++pPair)
   {
      setAttribute(pPair->first, const_cast<DataVariant&>(pPair->second), false);
   }

   return *this;
}

void DynamicObjectImp::merge(const DynamicObject* pObject)
{
   if (pObject == NULL)
   {
      return;
   }

   vector<string> attributes;
   pObject->getAttributeNames(attributes);

   vector<string>::const_iterator iter;
   for (iter = attributes.begin(); iter != attributes.end(); iter++)
   {
      string name = *iter;
      if (name.empty() == false)
      {
         const DataVariant& var = pObject->getAttribute(name);
         string type = var.getTypeName();
         if (var.isValid())
         {
            DataVariant& myVar = getAttribute(name);
            string myType = myVar.getTypeName();
            if (type == "DynamicObject" && myVar.isValid() && myType == "DynamicObject") // both are DOs
            {
               dv_cast<DynamicObject>(myVar).merge(&dv_cast<DynamicObject>(var));
            }
            else
            {
               setAttribute(name, const_cast<DataVariant&>(var), false);
            }
         }
      }
   }
}

void DynamicObjectImp::adoptiveMerge(DynamicObject* pObject)
{
   if (pObject == NULL)
   {
      return;
   }

   vector<string> attributes;
   pObject->getAttributeNames(attributes);

   vector<string>::const_iterator iter;
   for (iter = attributes.begin(); iter != attributes.end(); ++iter)
   {
      string name = *iter;
      if (name.empty() == false)
      {
         DataVariant& var = pObject->getAttribute(name);
         string type = var.getTypeName();
         if (var.isValid())
         {
            DataVariant& myVar = getAttribute(name);
            string myType = myVar.getTypeName();
            if (type == "DynamicObject" && myVar.isValid() && myType == "DynamicObject") // both are DOs
            {
               dv_cast<DynamicObject>(myVar).adoptiveMerge(&dv_cast<DynamicObject>(var));
            }
            else
            {
               adoptAttribute(name, var);
            }
         }
      }
   }
}

bool DynamicObjectImp::adoptAttribute(const string& name, DataVariant& value)
{
   return setAttribute(name, value, true);
}

bool DynamicObjectImp::adoptAttributeByPath(QStringList pathComponents, DataVariant& value)
{
   return setAttributeByPath(pathComponents, value, true);
}

bool DynamicObjectImp::adoptAttributeByPath(const string& path, DataVariant& value)
{
   return setAttributeByPath(path, value, true);
}

bool DynamicObjectImp::adoptAttributeByPath(const string pComponents[], DataVariant& value)
{
   return setAttributeByPath(pComponents, value, true);
}

bool DynamicObjectImp::setAttribute(const string& name, DataVariant& value, bool swap)
{
   if ((name.empty() == true) || (value.isValid() == false))
   {
      return false;
   }

   pair<map<string, DataVariant>::iterator, bool> location = 
      mVariantAttributes.insert(pair<string, DataVariant>(name, DataVariant()));
   DataVariant& mapVariant(location.first->second);

   if (swap)
   {
      mapVariant.swap(value);
      DynamicObjectImp* pValue = dynamic_cast<DynamicObjectImp*>(dv_cast<DynamicObject>(&value));
      if (pValue != NULL && pValue->mpParent != NULL)
      {
         //since value is no longer in the map it should
         //not be notifying this object when it changes.
         pValue->detach(SIGNAL_NAME(Subject, Modified), Signal(dynamic_cast<Subject*>(pValue->mpParent),
            SIGNAL_NAME(Subject, Modified)));
      }
   }
   else
   {
      mapVariant = value;
   }

   if (location.second)
   {
      notify(SIGNAL_NAME(DynamicObject, AttributeAdded), boost::any(pair<string, DataVariant*>(name, &mapVariant)));
   }
   else
   {
      notify(SIGNAL_NAME(DynamicObject, AttributeModified),
         boost::any(pair<string, DataVariant*>(name, &mapVariant)));
   }

   DynamicObjectImp* pValue = dynamic_cast<DynamicObjectImp*>(dv_cast<DynamicObject>(&mapVariant));
   if (pValue != NULL)
   {
      if (pValue->mpParent != NULL)
      {
         pValue->detach(SIGNAL_NAME(Subject, Modified), Signal(dynamic_cast<Subject*>(pValue->mpParent),
            SIGNAL_NAME(Subject, Modified)));
      }
      pValue->attach(SIGNAL_NAME(Subject, Modified), Signal(dynamic_cast<Subject*>(this),
         SIGNAL_NAME(Subject, Modified)));
      pValue->mpParent = this;
   }

   return true;
}

bool DynamicObjectImp::setAttributeByPath(QStringList pathComponents, DataVariant& value, bool swap)
{
   if (!value.isValid())
   {
      return false;
   }

   QString finalName = pathComponents.back();
   pathComponents.pop_back();

   string loopType = "DynamicObject";
   DynamicObject* pLoopObj = dynamic_cast<DynamicObject*>(this);
   DynamicObject* pCurObj = pLoopObj;
   for (QStringList::const_iterator iter = pathComponents.begin();
      iter != pathComponents.end(); ++iter)
   {
      if (pLoopObj == NULL || loopType != "DynamicObject")
      {
         return false;
      }

      pCurObj = pLoopObj;
      DataVariant& attrValue = pCurObj->getAttribute(iter->toStdString());
      loopType = attrValue.getTypeName();
      pLoopObj = attrValue.getPointerToValue<DynamicObject>();

      if ((pLoopObj != NULL) && (loopType != "DynamicObject"))
      {
         return false;
      }

      if (pLoopObj == NULL)
      {
         FactoryResource<DynamicObject> pNewObj;
         if (pCurObj != NULL)
         {
            pCurObj->setAttribute(iter->toStdString(), *pNewObj.get());
            DataVariant& currentValue = pCurObj->getAttribute(iter->toStdString());
            loopType = currentValue.getTypeName();
            pLoopObj = currentValue.getPointerToValue<DynamicObject>();
         }
      }
   }

   if (pLoopObj == NULL || loopType != "DynamicObject")
   {
      return false;
   }

   pCurObj = pLoopObj;
   DynamicObjectImp* const pCurObjImp = dynamic_cast<DynamicObjectImp*>(pCurObj);
   VERIFY(pCurObjImp != NULL);
   return pCurObjImp->setAttribute(finalName.toStdString(), value, swap);
}

bool DynamicObjectImp::setAttributeByPath(const string& path, DataVariant& value, bool swap)
{
   QStringList pathComponents = getPathComponents(path);
   return setAttributeByPath(pathComponents, value, swap);
}

bool DynamicObjectImp::setAttributeByPath(const string pComponents[], DataVariant& value, bool swap)
{
   VERIFY(pComponents != NULL);
   QStringList pathComponents;

   for (unsigned int i = 0; pComponents[i] != END_METADATA_NAME; ++i)
   {
      pathComponents.push_back(QString::fromStdString(pComponents[i]));
   }

   return setAttributeByPath(pathComponents, value, swap);
}

QStringList DynamicObjectImp::getPathComponents(const string& path) const
{
   if (path.empty() == true)
   {
      return QStringList();
   }

   QString qpath = QString::fromStdString(path);
   QStringList pathComponents;

   int startPos = 0;
   int pos = qpath.indexOf("/", startPos);

   while (pos != -1)
   {
      int newStartPos = pos + 1;
      if (pos < qpath.length() - 1)
      {
         if (qpath.at(newStartPos) == '/')
         {
            newStartPos++;
         }
         else
         {
            QString component = qpath.mid(startPos, pos - startPos);
            if (component.isEmpty() == false)
            {
               component.replace("//", "/");
               pathComponents.append(component);
            }

            startPos = newStartPos;
         }
      }
      else
      {
         // The path ends in a single slash, so just remove it
         qpath.truncate(pos);
      }

      pos = qpath.indexOf("/", newStartPos);
   }

   QString attribute = qpath.mid(startPos);
   if (attribute.isEmpty() == false)
   {
      attribute.replace("//", "/");
      pathComponents.append(attribute);
   }

   return pathComponents;
}

const DataVariant& DynamicObjectImp::getAttribute(const string& name) const
{
   static DataVariant sEmptyVariant;

   map<string, DataVariant>::const_iterator pPair = mVariantAttributes.find(name);
   if (pPair != mVariantAttributes.end())
   {
      return pPair->second;
   }

   sEmptyVariant = DataVariant();
   return sEmptyVariant;
}

DataVariant& DynamicObjectImp::getAttribute(const string& name)
{
   return const_cast<DataVariant&>(const_cast<const DynamicObjectImp*>(this)->getAttribute(name));
}

const DataVariant& DynamicObjectImp::getAttributeByPath(QStringList pathComponents) const
{
   static DataVariant sEmptyVariant;

   QString finalName = pathComponents.back();
   pathComponents.pop_back();

   string loopType = "DynamicObject";
   const DynamicObject* pCurrentObj = dynamic_cast<const DynamicObject*>(this);
   for (QStringList::const_iterator iter = pathComponents.begin();
      iter != pathComponents.end(); ++iter)
   {
      if ( (pCurrentObj == NULL) || loopType != "DynamicObject")
      {
         sEmptyVariant = DataVariant();
         return sEmptyVariant;
      }

      const DataVariant& attrValue = pCurrentObj->getAttribute(iter->toStdString());
      loopType = attrValue.getTypeName();
      pCurrentObj = attrValue.getPointerToValue<DynamicObject>();
   }

   if (pCurrentObj == NULL)
   {
      sEmptyVariant = DataVariant();
      return sEmptyVariant;
   }

   return pCurrentObj->getAttribute(finalName.toStdString());
}

DataVariant& DynamicObjectImp::getAttributeByPath(QStringList pathComponents)
{
   return const_cast<DataVariant&>(const_cast<const DynamicObjectImp*>(this)->getAttributeByPath(pathComponents));
}

const DataVariant& DynamicObjectImp::getAttributeByPath(const string& path) const
{
   QStringList pathComponents = getPathComponents(path);
   return getAttributeByPath(pathComponents);
}

DataVariant& DynamicObjectImp::getAttributeByPath(const string& path)
{
   return const_cast<DataVariant&>(const_cast<const DynamicObjectImp*>(this)->getAttributeByPath(path));
}

const DataVariant& DynamicObjectImp::getAttributeByPath(const string pComponents[]) const
{
   QStringList pathComponents;

   for (unsigned int i = 0; pComponents != NULL && pComponents[i] != END_METADATA_NAME; ++i)
   {
      pathComponents.push_back(QString::fromStdString(pComponents[i]));
   }

   return getAttributeByPath(pathComponents);
}

DataVariant& DynamicObjectImp::getAttributeByPath(const string pComponents[])
{
   return const_cast<DataVariant&>(const_cast<const DynamicObjectImp*>(this)->getAttributeByPath(pComponents));
}

void DynamicObjectImp::getAttributeNames(vector<string>& attributeNames) const
{
   attributeNames.clear();

   map<string, DataVariant>::const_iterator pPair;
   for (pPair = mVariantAttributes.begin(); pPair != mVariantAttributes.end(); ++pPair)
   {
      attributeNames.push_back(pPair->first);
   }
}

unsigned int DynamicObjectImp::getNumAttributes() const
{
   return mVariantAttributes.size();
}

const DataVariant& DynamicObjectImp::findFirstOf(const QRegExp& name, const QRegExp& value) const
{
   static DataVariant sEmptyVariant;

   if ((name.isEmpty() == true) && (value.isEmpty() == true))
   {
      sEmptyVariant = DataVariant();
      return sEmptyVariant;
   }

   for (map<string, DataVariant>::const_iterator iter = mVariantAttributes.begin();
      iter != mVariantAttributes.end();
      ++iter)
   {
      string attributeName = iter->first;
      if (attributeName.empty() == false)
      {
         // Name
         bool nameMatch = true;
         if (name.isEmpty() == false)
         {
            nameMatch = name.exactMatch(QString::fromStdString(attributeName));
         }

         // Value
         bool valueMatch = true;
         if (value.isEmpty() == false)
         {
            if (iter->second.isValid() == true)
            {
               QString valueText = QString::fromStdString(iter->second.toDisplayString());
               valueMatch = value.exactMatch(valueText);
            }
            else
            {
               valueMatch = false;
            }
         }

         if ((nameMatch == true) && (valueMatch == true))
         {
            return iter->second;
         }
      }
   }

   sEmptyVariant = DataVariant();
   return sEmptyVariant;
}

DataVariant& DynamicObjectImp::findFirstOf(const QRegExp& name, const QRegExp& value)
{
   return const_cast<DataVariant&>(const_cast<const DynamicObjectImp*>(this)->findFirstOf(name, value));
}

bool DynamicObjectImp::removeAttribute(const string& name)
{
   map<string, DataVariant>::iterator iter = mVariantAttributes.find(name);
   if (iter != mVariantAttributes.end())
   {
      // Since the map stores attributes by value, emit the signal before removing
      // the attribute to ensure that the value is still created and valid
      notify(SIGNAL_NAME(DynamicObject, AttributeRemoved),
         boost::any(pair<string, DataVariant*>(iter->first, &(iter->second))));
      mVariantAttributes.erase(iter);
      return true;
   }

   return false;
}

bool DynamicObjectImp::removeAttributeByPath(const string pComponents[])
{
   QStringList pathComponents;

   for (unsigned int i = 0; pComponents != NULL && pComponents[i] != END_METADATA_NAME; ++i)
   {
      pathComponents.push_back(QString::fromStdString(pComponents[i]));
   }

   return removeAttributeByPath(pathComponents);
}

bool DynamicObjectImp::removeAttributeByPath(const string& path)
{
   QStringList pathComponents = getPathComponents(path);
   return removeAttributeByPath(pathComponents);
}

bool DynamicObjectImp::removeAttributeByPath(QStringList pathComponents)
{
   if (pathComponents.size() == 0)
   {
      return false;
   }
   DynamicObject* pParent = dynamic_cast<DynamicObject*>(this);
   string attributeToRemove = pathComponents.last().toStdString();
   if (pathComponents.size() > 1)
   {
      pathComponents.removeLast();
      DataVariant& parentVar = getAttributeByPath(pathComponents);
      pParent = dv_cast<DynamicObject>(&parentVar);
   }
   if (pParent == NULL)
   {
      return false;
   }
   return pParent->removeAttribute(attributeToRemove);
}

void DynamicObjectImp::clear()
{
   // Since the map stores attributes by value, emit the signal before clearing
   // the attributes to ensure that the values are still created and valid
   notify(SIGNAL_NAME(DynamicObject, Cleared));
   mVariantAttributes.clear();
}

const string& DynamicObjectImp::getObjectType() const
{
   static string sType = "DynamicObjectImp";
   return sType;
}

bool DynamicObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DynamicObject"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

namespace
{
   template <class T>
   bool serializeVector(XMLWriter* pXml, const DataVariant& var)
   {
      vector<T>* pValue = *dv_cast<vector<T> >(&var);
      VERIFY(pValue != NULL);
      for (typename vector<T>::iterator vit = pValue->begin(); vit != pValue->end(); ++vit)
      {
         stringstream sstr;
         DOMElement* pElement = pXml->addElement("value");
         VERIFY(pElement != NULL);
         pXml->pushAddPoint(pElement);
         sstr << *vit;
         pXml->addText(sstr.str().c_str(), pElement);
         pXml->popAddPoint();
      }
      return true;
   }
}

bool DynamicObjectImp::toXml(XMLWriter* pWriter) const
{
   VERIFY(pWriter != NULL);

   pWriter->addAttr("version", XmlBase::VERSION);

   if (mVariantAttributes.size() == 0)
   {
      return true; // don't write any output
   }

   bool addedTopLevel(false);
   if (pWriter->peekAddPoint() == NULL)
   {
      addedTopLevel = true;
      pWriter->pushAddPoint(pWriter->addElement("DynamicObject"));
   }
   
   map<string, DataVariant>::const_iterator pPair;
   for (pPair = mVariantAttributes.begin(); pPair != mVariantAttributes.end(); ++pPair)
   {
      const DataVariant& var = pPair->second;
      if (var.isValid())
      {
         bool bSuccess = true;
         DOMElement* pAttribute(pWriter->addElement("attribute"));
         pWriter->pushAddPoint(pAttribute);
         string type = var.getTypeName();
         pWriter->addAttr("name", pPair->first.c_str());
         pWriter->addAttr("type", type.c_str());
         bSuccess = pPair->second.toXml(pWriter);
         pWriter->popAddPoint();
         if (!bSuccess) 
         {
            break;
         }
      }
   }

   if (addedTopLevel) 
   {
      pWriter->popAddPoint();
   }

   return true;
}

bool DynamicObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   clear();

   for (DOMNode* pNode = pDocument->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("attribute")))
      {
         string name;
         DataVariant var;
         try
         {
            DOMElement* pElement(static_cast<DOMElement*>(pNode));
            name = A(pElement->getAttribute(X("name")));
            if (!var.fromXml(pNode, version))
            {
               return false;
            }
         }
         catch (XmlReader::DomParseException& exc)
         {
            throw exc;
         }

         setAttribute(name, var, true);
      }
   }

   return true;
}

bool DynamicObjectImp::isParentOf(const DynamicObjectImp* pObject) const
{
   if (pObject == NULL)
   {
      return false;
   }

   if (pObject == this)
   {
      return true;
   }

   // recursive, exhaustive, depth-first search
   map<string, DataVariant>::const_iterator iter;
   for (iter = mVariantAttributes.begin(); iter != mVariantAttributes.end(); ++iter)
   {
      const DataVariant& var = iter->second;
      const DynamicObjectImp* pCurrentObject = dynamic_cast<const DynamicObjectImp*>(dv_cast<DynamicObject>(&var));
      if (pCurrentObject != NULL)
      {
         if (pCurrentObject->isParentOf(pObject) == true)
         {
            return true;
         }
      }
   }
   return false;
}
