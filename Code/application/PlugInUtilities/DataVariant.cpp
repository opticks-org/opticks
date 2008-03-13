/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "DataVariant.h"
#include "DataValueWrapper.h"
#include "DataVariantFactory.h"

DataVariant::DataVariant(const DataVariant& value) :
   mpValue(NULL)
{
   if (value.isValid() == true)
   {
      mpValue = value.mpValue->copy();
   }
}

DataVariant::~DataVariant()
{
   delete mpValue;
}

bool DataVariant::isValid() const
{
   return (mpValue != NULL);
}

const std::type_info &DataVariant::getType() const
{
   if (isValid())
   {
      return mpValue->getType();
   }

   return typeid(void);
}

std::string DataVariant::getTypeName() const
{
   if (!isValid())
   {
      return "void";
   }
   return mpValue->getTypeName();
}

std::string DataVariant::toXmlString(Status* pStatus) const
{
   if (!isValid())
   {
      if (pStatus != NULL)
      {
         *pStatus = DataVariant::SUCCESS;
      }
      return std::string();
   }
   return mpValue->toXmlString(pStatus);
}

DataVariant::Status DataVariant::fromXmlString(const std::string &type, const std::string &text)
{
   DataValueWrapper *pOldWrapper = mpValue;
   DataValueWrapper *pNewWrapper = Service<DataVariantFactory>()->createWrapper(NULL, type);
   if (pNewWrapper == NULL)
   {
      return DataVariant::NOT_SUPPORTED;
   }
   DataVariant::Status status = pNewWrapper->fromXmlString(text);
   if (status == DataVariant::SUCCESS)
   {
      mpValue = pNewWrapper;
      delete pOldWrapper;
   }
   return status;
}

std::string DataVariant::toDisplayString(Status* pStatus) const
{
   if (!isValid())
   {
      if (pStatus != NULL)
      {
         *pStatus = DataVariant::SUCCESS;
      }
      return std::string();
   }
   return mpValue->toDisplayString(pStatus);
}

DataVariant::Status DataVariant::fromDisplayString(const std::string &type, const std::string &text)
{
   DataValueWrapper *pOldWrapper = mpValue;
   DataValueWrapper *pNewWrapper = Service<DataVariantFactory>()->createWrapper(NULL, type);
   if (pNewWrapper == NULL)
   {
      return DataVariant::NOT_SUPPORTED;
   }
   DataVariant::Status status = pNewWrapper->fromDisplayString(text);
   if (status == DataVariant::SUCCESS)
   {
      mpValue = pNewWrapper;
      delete pOldWrapper;
   }
   return status;
}

bool DataVariant::toXml(XMLWriter* pWriter) const
{
   if (!isValid())
   {
      return false;
   }
   return mpValue->toXml(pWriter);
}

bool DataVariant::fromXml(DOMNode* pDocument, unsigned int version)
{
   DataValueWrapper *pOldWrapper = mpValue;

   DataValueWrapper *pNewWrapper = Service<DataVariantFactory>()->createWrapper(pDocument, version);
   if (pNewWrapper == NULL)
   {
      return false;
   }

   mpValue = pNewWrapper;
   delete pOldWrapper;

   return true;
}

DataVariant & DataVariant::swap(DataVariant & rhs)
{
   std::swap(mpValue, rhs.mpValue);
   return *this;
}

DataVariant & DataVariant::operator=(const DataVariant & rhs)
{
   DataVariant(rhs).swap(*this);
   return *this;
}

bool DataVariant::operator==(const DataVariant &rhs) const
{
   if (mpValue == NULL)
   {
      if (rhs.mpValue == NULL)
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      if (rhs.mpValue == NULL)
      {
         return false;
      }
      else
      {
         return *mpValue == *rhs.mpValue; //the wrapper perform the type equivalent test
      }
   }
}

/**
 *  Gets a pointer to the wrapped object.
 *
 *  If the variant isn't empty, this method gets a pointer to the internal
 *  wrapped object.
 *
 *  @return  A pointer to the wrapped object, or NULL if the variant is empty.
 */
void *DataVariant::getPointerToValueAsVoid() const
{
   if (isValid())
   {
      return mpValue->getValue();
   }
   else
   {
      return NULL;
   }
}

/**
 *  Creates a type-aware wrapper around an object.
 *
 *  @param   pValue
 *         A pointer to the object to be wrapped.
 *  @param   pTypeName
 *         The raw type name of the object to be wrapped. This should be in the
 *         format returned by typeid.name.
 *  @param   strict
 *         If true, a verification error message will be generated if
 *         a DataVariant of the given type cannot be constructed.  If
 *         false, no verification error message will be generated.
 *
 *  @return  A pointer to the wrapper object. If the type name isn't one supported
 *         by the data variant factory, NULL will be returned.
 */
DataValueWrapper *DataVariant::createWrapper(const void *pValue, const char *pTypeName, bool strict) const
{
   return Service<DataVariantFactory>()->createWrapper(pValue, pTypeName, strict);
}
