/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataDescriptorImp.h"
#include "DataDescriptor.h"
#include "FileDescriptorAdapter.h"
#include "ModelServicesImp.h"
#include "RasterFileDescriptorAdapter.h"
#include "SessionManager.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

DataDescriptorImp::DataDescriptorImp(const string& name, const string& type, DataElement* pParent) :
   mName(name),
   mType(type),
   mpParent(pParent),
   mProcessingLocation(IN_MEMORY),
   mpFileDescriptor(NULL)
{
   // copy non-NULL parent classification
   if (pParent != NULL)
   {
      setClassification(pParent->getClassification());
   }

   // Attach to the metadata and classification objects to notify when the they change
   mMetadata.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DataDescriptorImp::notifyModified));
   mClassification.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DataDescriptorImp::notifyModified));

   generateParentDesignator();
}

DataDescriptorImp::DataDescriptorImp(const string& name, const string& type, const vector<string>& parent) :
   mName(name),
   mType(type),
   mpParent(NULL),
   mParentDesignator(parent),
   mProcessingLocation(IN_MEMORY),
   mpFileDescriptor(NULL)
{
   // Attach to the metadata and classification objects to notify when the they change
   mMetadata.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DataDescriptorImp::notifyModified));
   mClassification.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DataDescriptorImp::notifyModified));
}

DataDescriptorImp::~DataDescriptorImp()
{
   if (mpFileDescriptor != NULL)
   {
      delete mpFileDescriptor;
   }
}

void DataDescriptorImp::notifyModified(Subject &subject, const string &signal, const boost::any &data)
{
   if (&subject == &mMetadata || &subject == &mClassification)
   {
      notify(SIGNAL_NAME(Subject, Modified), data);
   }
}

void DataDescriptorImp::setName(const string& name)
{
   if (name != mName)
   {
      mName = name;
      notify(SIGNAL_NAME(DataDescriptor, Renamed), boost::any(mName));
   }
}

const string& DataDescriptorImp::getName() const
{
   return mName;
}

const string& DataDescriptorImp::getType() const
{
   return mType;
}

DataElement* DataDescriptorImp::getParent() const
{
   return const_cast<DataElement*>(mpParent.get());
}

vector<string> DataDescriptorImp::getParentDesignator() const
{
   return mParentDesignator;
}

void DataDescriptorImp::setParent(DataElement *pParent)
{
   if (pParent != mpParent.get())
   {
      mpParent.reset(pParent);
      generateParentDesignator();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void DataDescriptorImp::setClassification(const Classification* pClassification)
{
   if ((pClassification != NULL) && (pClassification != &mClassification))
   {
      mClassification = *(dynamic_cast<const ClassificationAdapter*>(pClassification));
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

Classification* DataDescriptorImp::getClassification()
{
   return &mClassification;
}

const Classification* DataDescriptorImp::getClassification() const
{
   return &mClassification;
}

void DataDescriptorImp::setMetadata(const DynamicObject* pMetadata)
{
   if (pMetadata == NULL)
   {
      mMetadata.clear();
      notify(SIGNAL_NAME(Subject, Modified));
   }
   else if (pMetadata != &mMetadata)
   {
      mMetadata = *(dynamic_cast<const DynamicObjectAdapter*>(pMetadata));
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

DynamicObject* DataDescriptorImp::getMetadata()
{
   return &mMetadata;
}

const DynamicObject* DataDescriptorImp::getMetadata() const
{
   return &mMetadata;
}

void DataDescriptorImp::setProcessingLocation(ProcessingLocation processingLocation)
{
   if (processingLocation != mProcessingLocation)
   {
      mProcessingLocation = processingLocation;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

ProcessingLocation DataDescriptorImp::getProcessingLocation() const
{
   return mProcessingLocation;
}

void DataDescriptorImp::setFileDescriptor(const FileDescriptorImp* pFileDescriptor)
{
   if (pFileDescriptor != mpFileDescriptor)
   {
      if (mpFileDescriptor != NULL)
      {
         delete mpFileDescriptor;
         mpFileDescriptor = NULL;
      }

      if (pFileDescriptor != NULL)
      {
         const FileDescriptor* pDescriptor = dynamic_cast<const FileDescriptor*> (pFileDescriptor);
         if (pDescriptor != NULL)
         {
            mpFileDescriptor = dynamic_cast<FileDescriptorImp*>(pDescriptor->copy());
         }
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }
}

FileDescriptorImp* DataDescriptorImp::getFileDescriptor()
{
   return mpFileDescriptor;
}

DataDescriptor* DataDescriptorImp::copy() const
{
   if ((mpParent.get() == NULL) && (!mParentDesignator.empty()))
   {
      return copy(mName, mParentDesignator);
   }

   return copy(mName, const_cast<DataElement*>(mpParent.get()));
}

DataDescriptor* DataDescriptorImp::copy(const string& name, DataElement* pParent) const
{
   ModelServicesImp* pModel = ModelServicesImp::instance();
   if (pModel == NULL)
   {
      return NULL;
   }

   DataDescriptor* pDescriptor = pModel->createDataDescriptor(name, mType, pParent);
   if (pDescriptor != NULL)
   {
      pDescriptor->setClassification(dynamic_cast<const Classification*>(&mClassification));
      pDescriptor->setMetadata(dynamic_cast<const DynamicObject*>(&mMetadata));
      pDescriptor->setProcessingLocation(mProcessingLocation);
      pDescriptor->setFileDescriptor(dynamic_cast<FileDescriptor*>(mpFileDescriptor));
   }

   return pDescriptor;
}

DataDescriptor* DataDescriptorImp::copy(const string& name, const vector<string>& parent) const
{
   ModelServicesImp* pModel = ModelServicesImp::instance();
   if (pModel == NULL)
   {
      return NULL;
   }

   DataDescriptor* pDescriptor = pModel->createDataDescriptor(name, mType, parent);
   if (pDescriptor != NULL)
   {
      pDescriptor->setClassification(dynamic_cast<const Classification*>(&mClassification));
      pDescriptor->setMetadata(dynamic_cast<const DynamicObject*>(&mMetadata));
      pDescriptor->setProcessingLocation(mProcessingLocation);
      pDescriptor->setFileDescriptor(dynamic_cast<FileDescriptor*>(mpFileDescriptor));
   }

   return pDescriptor;
}

void DataDescriptorImp::addToMessageLog(Message* pMessage) const
{
   if (pMessage == NULL)
   {
      return;
   }

   // Name
   pMessage->addProperty("Name", mName);

   // Type
   pMessage->addProperty("Type", mType);

   // Parent element
   string parentElement;
   if (mpParent.get() != NULL)
   {
      parentElement = mpParent->getName();
   }
   else
   {
      for (vector<string>::const_iterator part = mParentDesignator.begin(); part != mParentDesignator.end(); ++part)
      {
         if (part != mParentDesignator.begin())
         {
            parentElement += "/" + *part;
         }
         else
         {
            parentElement = *part;
         }
      }
   }

   pMessage->addProperty("Parent Element", parentElement);

   // Classification
   string classificationText;
   mClassification.getClassificationText(classificationText);

   pMessage->addProperty("Classification", classificationText);

   // TODO: Metadata

   // Processing location
   pMessage->addProperty("Processing Location", mProcessingLocation);

   // File descriptor
   if (mpFileDescriptor != NULL)
   {
      mpFileDescriptor->addToMessageLog(pMessage);
   }
}

const FileDescriptorImp* DataDescriptorImp::getFileDescriptor() const
{
   return mpFileDescriptor;
}

bool DataDescriptorImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   // Name, type, and XML version
   pXml->addAttr("name", mName);
   pXml->addAttr("type", mType);
   pXml->addAttr("version", XmlBase::VERSION);

   // Classification
   bool bSuccess = mClassification.toXml(pXml);

   // Metadata
   if (bSuccess == true)
   {
      pXml->pushAddPoint(pXml->addElement("metadata"));
      bSuccess = mMetadata.toXml(pXml);
      pXml->popAddPoint();
   }

   // Processing location
   if (bSuccess == true)
   {
      pXml->addAttr("processingLocation", mProcessingLocation);
   }

   // File descriptor
   if ((bSuccess == true) && (mpFileDescriptor != NULL))
   {
      pXml->pushAddPoint(pXml->addElement("FileDescriptor"));
      if (mpFileDescriptor != NULL)
      {
         bSuccess = mpFileDescriptor->toXml(pXml);
      }
      pXml->popAddPoint();
   }

   // Importer name
   if (bSuccess == true)
   {
      pXml->addAttr("importerName", mImporterName);
   }

   return bSuccess;
}

bool DataDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);

   if (mName != A(pElement->getAttribute(X("name"))))
   {
      throw XmlBase::XmlException("Element name does not match this element");
   }

   if (!Service<SessionManager>()->isSessionLoading() && mType != A(pElement->getAttribute(X("type"))))
   {
      throw XmlBase::XmlException("Element type does not match this element");
   }
   mProcessingLocation = StringUtilities::fromXmlString<ProcessingLocation>(
      A(pElement->getAttribute(X("processingLocation"))));

   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("classification")))
      {
         if (mClassification.fromXml(pChild, version) == false)
         {
            throw XmlReader::DomParseException("Can't deserialize classification", pDocument);
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("metadata")))
      {
         if (mMetadata.fromXml(pChild, version) == false)
         {
            throw XmlReader::DomParseException("Can't deserialize metadata", pDocument);
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("FileDescriptor")))
      {
         // create a new FileDescriptor to make sure we have the correct type of FileDescriptor
         delete mpFileDescriptor;
         DOMElement* pChildElement = static_cast<DOMElement*>(pChild);
         string fileDescriptorType = A(pChildElement->getAttribute(X("type")));

         ModelServicesImp* pModel = ModelServicesImp::instance();
         if (pModel != NULL)
         {
            if (pModel->isKindOfFileDescriptor(fileDescriptorType, "RasterFileDescriptor") == true)
            {
               mpFileDescriptor = new RasterFileDescriptorAdapter();
            }
            else if (pModel->isKindOfFileDescriptor(fileDescriptorType, "FileDescriptor") == true)
            {
               mpFileDescriptor = new FileDescriptorAdapter();
            }
         }

         if (mpFileDescriptor == NULL)
         {
            throw XmlReader::DomParseException("Cannot create file descriptor", pDocument);
         }

         if (mpFileDescriptor->fromXml(pChild, version) == false)
         {
            throw XmlReader::DomParseException("Cannot deserialize file descriptor", pDocument);
         }
      }
   }

   if (pElement->hasAttribute(X("importerName")) == true)
   {
      mImporterName = A(pElement->getAttribute(X("importerName")));
   }
   else
   {
      mImporterName.clear();
   }

   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

const string& DataDescriptorImp::getObjectType() const
{
   static string sType("DataDescriptorImp");
   return sType;
}

bool DataDescriptorImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DataDescriptor"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void DataDescriptorImp::getDataDescriptorTypes(vector<string>& classList)
{
   classList.push_back("DataDescriptor");
}

bool DataDescriptorImp::isKindOfDataDescriptor(const string& className)
{
   if ((className == "DataDescriptorImp") || (className == "DataDescriptor"))
   {
      return true;
   }

   return false;
}

void DataDescriptorImp::generateParentDesignator()
{
   mParentDesignator.clear();
   if (mpParent.get() != NULL)
   {
      mParentDesignator = mpParent->getParentDesignator();
      mParentDesignator.push_back(mpParent->getName());
   }
}

void DataDescriptorImp::setImporterName(const string& name)
{
   mImporterName = name;
}

string DataDescriptorImp::getImporterName() const
{
   return mImporterName;
}
