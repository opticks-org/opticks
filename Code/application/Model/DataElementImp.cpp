/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataElementImp.h"
#include "DataElement.h"
#include "DataDescriptor.h"
#include "FileDescriptorImp.h"
#include "MessageLogResource.h"
#include "ModelServicesImp.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "RasterElement.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

DataElementImp::DataElementImp(const DataDescriptorImp& descriptor, const string& id) :
   SessionItemImp(id, descriptor.getName()),
   mpDataDescriptor(dynamic_cast<DataDescriptorImp*>(descriptor.copy()))
{
   if (mpDataDescriptor != NULL)
   {
      addPropertiesPage("Data Descriptor Properties");

      if (mpDataDescriptor->getFileDescriptor() != NULL)
      {
         addPropertiesPage("File Descriptor Properties");
      }

      addPropertiesPage("Classification Properties");
      addPropertiesPage("Metadata Properties");
   }

   // Attach to the data descriptor to notify when the descriptor changes
   if (mpDataDescriptor != NULL)
   {
      mpDataDescriptor->attach(SIGNAL_NAME(DataDescriptor, Renamed), Slot(this, &DataElementImp::descriptorRenamed));
      mpDataDescriptor->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DataElementImp::descriptorModified));
   }
}

DataElementImp::~DataElementImp()
{
   delete mpDataDescriptor;
}

void DataElementImp::descriptorRenamed(Subject &subject, const string &signal, const boost::any &data)
{
   if (dynamic_cast<DataDescriptor*>(mpDataDescriptor) == &subject)
   {
      string name = boost::any_cast<string>(data);
      setName(name);
   }
}

void DataElementImp::descriptorModified(Subject &subject, const string &signal, const boost::any &data)
{
   if (dynamic_cast<DataDescriptor*>(mpDataDescriptor) == &subject)
   {
      notify(signal, data);
   }
}

DataElementImp& DataElementImp::operator =(const DataElementImp& element)
{
   if (this != &element)
   {
      SessionItemImp::operator= (element);

      if (mpDataDescriptor != NULL)
      {
         delete mpDataDescriptor;
         mpDataDescriptor = NULL;
      }

      if (element.mpDataDescriptor != NULL)
      {
         mpDataDescriptor = dynamic_cast<DataDescriptorImp*>(element.mpDataDescriptor->copy());
      }
   }

   return *this;
}

DataDescriptorImp* DataElementImp::getDataDescriptor()
{
   return mpDataDescriptor;
}

const DataDescriptorImp* DataElementImp::getDataDescriptor() const
{
   return mpDataDescriptor;
}

void DataElementImp::setName(const string& name)
{
   if (mpDataDescriptor == NULL)
   {
      return;
   }

   const string& oldName = getName();
   if (name != oldName)
   {
      StepResource pStep(NULL);
      if (oldName.empty() == false)
      {
         pStep = StepResource("Change " + getObjectType(), "app", "7C4015B0-938E-44CE-918D-ABC6AB70489D");
         pStep->addProperty("attribute", "name");
         pStep->addProperty("oldValue", oldName);
         pStep->addProperty("newValue", name);
      }

      SessionItemImp::setName(name);
      mpDataDescriptor->setName(name);

      if (pStep.get() != NULL)
      {
         pStep->finalize(Message::Success);
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }
}

string DataElementImp::getType() const
{
   string type = "";
   if (mpDataDescriptor != NULL)
   {
      type = mpDataDescriptor->getType();
   }

   return type;
}

string DataElementImp::getFilename() const
{
   string filename = "";
   if (mpDataDescriptor != NULL)
   {
      const FileDescriptorImp* pFileDescriptor = mpDataDescriptor->getFileDescriptor();
      if (pFileDescriptor != NULL)
      {
         filename = pFileDescriptor->getFilename();
      }
   }

   return filename;
}

DataElement* DataElementImp::getParent() const
{
   DataElement* pParent = NULL;
   if (mpDataDescriptor != NULL)
   {
      pParent = mpDataDescriptor->getParent();
   }

   return pParent;
}

vector<string> DataElementImp::getParentDesignator() const
{
   vector<string> parentDesignator;
   if (mpDataDescriptor != NULL)
   {
      parentDesignator = mpDataDescriptor->getParentDesignator();
   }

   return parentDesignator;
}

Classification* DataElementImp::getClassification()
{
   if (mpDataDescriptor != NULL)
   {
      return mpDataDescriptor->getClassification();
   }

   return NULL;
}

const Classification* DataElementImp::getClassification() const
{
   const Classification* pClassification = NULL;
   if (mpDataDescriptor != NULL)
   {
      pClassification = mpDataDescriptor->getClassification();
   }

   return pClassification;
}

void DataElementImp::setClassification(const Classification* pClassification)
{
   if (mpDataDescriptor != NULL)
   {
       mpDataDescriptor->setClassification(pClassification);
   }
}

void DataElementImp::copyClassification(const DataElement* pElement)
{
   if (mpDataDescriptor != NULL && pElement != NULL)
   {
      mpDataDescriptor->setClassification(pElement->getClassification());
   }
}

DynamicObject* DataElementImp::getMetadata()
{
   DynamicObject* pMetadata = NULL;
   if (mpDataDescriptor != NULL)
   {
      pMetadata = mpDataDescriptor->getMetadata();
   }

   return pMetadata;
}

const DynamicObject* DataElementImp::getMetadata() const
{
   const DynamicObject* pMetadata = NULL;
   if (mpDataDescriptor != NULL)
   {
      pMetadata = mpDataDescriptor->getMetadata();
   }

   return pMetadata;
}

bool DataElementImp::canRename() const
{
   return true;
}

bool DataElementImp::rename(const string& newName, string& errorMessage)
{
   DataElement* pElement = dynamic_cast<DataElement*>(this);
   if (pElement == NULL)
   {
      return false;
   }

   Service<ModelServices> pModel;
   return pModel->setElementName(pElement, newName);
}

DataElement* DataElementImp::copy(const string& name, DataElement* pParent) const
{
   if (dynamic_cast<DataElementImp*>(pParent) == this)
   {
      return NULL;
   }

   DataElement* pElement = NULL;

   ModelServicesImp* pModel = ModelServicesImp::instance();
   if (pModel != NULL)
   {
      const DataDescriptorImp* pDescriptor = getDataDescriptor();
      if (pDescriptor != NULL)
      {
         const DataDescriptor* pNewDescriptor = pDescriptor->copy(name, pParent);
         if (pNewDescriptor != NULL)
         {
            pElement = pModel->createElement(pNewDescriptor);
         }
      }
   }

   return pElement;
}

bool DataElementImp::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml(getObjectType().c_str());
   if (!SessionItemImp::toXml(&xml))
   {
      return false;
   }
   DataElement* pParent = getParent();
   if (pParent != NULL)
   {
      xml.addAttr("parentId", pParent->getId());
   }
   return toXml(&xml) && serializer.serialize(xml);
}

bool DataElementImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMDocument* pDoc = deserializer.deserialize(reader);
   if (pDoc == NULL)
   {
      return false;
   }
   DOMElement* pRoot = pDoc->getDocumentElement();
   VERIFY(pRoot != NULL);
   if (pRoot->hasAttribute(X("parentId")))
   {
      DataElement* pParent =
         dynamic_cast<DataElement*>(Service<SessionManager>()->getSessionItem(A(pRoot->getAttribute(X("parentId")))));
      if (pParent == NULL)
      {
         return false;
      }

      Service<ModelServices>()->setElementParent(dynamic_cast<DataElement*>(this), pParent);
   }
   Service<ModelServices>()->setElementName(dynamic_cast<DataElement*>(this), A(pRoot->getAttribute(X("name"))));
   unsigned int formatVersion = 0;
   stringstream str(A(pRoot->getAttribute(X("version"))));
   str >> formatVersion;
   return SessionItemImp::fromXml(pRoot, formatVersion) && fromXml(pRoot, formatVersion);
}

bool DataElementImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   pXml->addAttr("name", getName());
   string elementType;
   if (mpDataDescriptor != NULL)
   {
      elementType = mpDataDescriptor->getType();
   }

   pXml->addAttr("type", elementType);
   pXml->addAttr("version", XmlBase::VERSION);

   // Data descriptor
   bool bSuccess = false;

   DOMElement* pElement = pXml->addElement("DataDescriptor");
   if (pElement != NULL)
   {
      pXml->pushAddPoint(pElement);
      if (mpDataDescriptor != NULL)
      {
         bSuccess = mpDataDescriptor->toXml(pXml);
      }

      pXml->popAddPoint();
   }

   return bSuccess;
}

bool DataElementImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("DataDescriptor")))
      {
         if (mpDataDescriptor == NULL)
         {
            throw XmlReader::DomParseException("Cannot access the data descriptor", pDocument);
         }

         if (mpDataDescriptor->fromXml(pChild, version) == false)
         {
            throw XmlReader::DomParseException("Cannot deserialize data descriptor", pDocument);
         }
      }
   }

   return true;
}

const string& DataElementImp::getObjectType() const
{
   static string sType("DataElementImp");
   return sType;
}

bool DataElementImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DataElement"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void DataElementImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("DataElement");
}

bool DataElementImp::isKindOfElement(const string& className)
{
   if ((className == "DataElementImp") || (className == "DataElement"))
   {
      return true;
   }

   return false;
}
