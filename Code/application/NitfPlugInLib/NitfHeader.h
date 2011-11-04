/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFHEADER_H
#define NITFHEADER_H

#include <ossim/base/ossimDateProperty.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimPropertyInterface.h>

#include "ObjectResource.h"
#include "StringUtilities.h"
#include "NitfUtilities.h"

#include <string>
#include <vector>

class DateTime;
class DynamicObject;
class RasterDataDescriptor;

class ossimContainerProperty;
class ossimPropertyInterface;

namespace Nitf
{
   /**
    * The Header class contains all knowledge needed to succesfully
    * import or export a given NITF header.
    *
    * To be useful, this class must be subclassed to provide behavior
    * for a specific header or subheader.  Subclass behavior must
    * include a public import method, which calls the protected one on
    * this class.
    *
    * When importing or exporting, the Header iterates over the contents
    * of Header::mElements, calling the appropriate functions to import/export.
    */
   class Header
   {
   public:
      /**
       * Destructs the object.
       */
      virtual ~Header();

      /**
       * Import the metadata from OSSIM into a given DynamicObject.
       *
       * Subclasses must provide a public import method, which eventually
       * calls this one.
       *
       * Imports according to the specification in Header::mElements.
       *
       * @param pProperties
       *        The header to import from.
       * @param pDescriptor
       *        The Descriptor to import to.
       * @param pDynObj
       *        The DynamicObject to import according to the specification.
       *
       * @return True if the operation succeeded, false otherwise.
       */
      bool importMetadata(const ossimPropertyInterface* pProperties,
         RasterDataDescriptor* pDescriptor, DynamicObject* pDynObj);

      /**
       * Export all metadata from the given descriptor to the export header.
       *
       * This is performed by exporting two DynamicObjects.  First,
       * the DynamicObject created by createDefaultsDynamicObject is
       * exported.  This should contain any defaults to be exported.
       * This is most useful if the RasterElement wasn't originally a NITF file.
       *
       * Second, the DynamicObject for this header from the DataDescriptor's metadata 
       * object is exported.  This should contain metadata as imported from a NITF.
       *
       * @param pDescriptor
       *        The descriptor to export.
       * @param pExportHeader
       *        The OSSIM header to export to.
       *
       * @return True if the operation was successful, false otherwise.
       */
      bool exportMetadata(const RasterDataDescriptor *pDescriptor, 
         ossimContainerProperty *pExportHeader);

   protected:
      /**
       * Construct a Header with a given name.
       *
       * @param fileVersion
       *        The version of this NITF file.
       */
      Header(const std::string &fileVersion);

      /**
       * Returns the metadata location.
       *
       * @return The name to place and retrieve the main metadata DynamicObject for this header.
       */
      virtual std::string getMetadataPath() const = 0;

      /**
       * Create defaults to export from the given DataDescriptor.
       *
       * @param pDescriptor
       *        Descriptor which will be exported.  May be needed to
       *        create defaults.
       *
       * @return The defaults DynamicObject, wrapped in a FactoryResource to prevent leaks.
       */
      virtual FactoryResource<DynamicObject> createDefaultsDynamicObject(
         const RasterDataDescriptor *pDescriptor) = 0;

      /**
       * Exports a provided DynamicObject according to the specification in Header::mElements.
       *
       * @param pDescriptor
       *        The Descriptor to export
       * @param pDynObj
       *        The DynamicObject to export from.
       * @param pProperties
       *        The OSSIM header to export to.
       *
       * @return True if the operation succeeded, false otherwise.
       */
      bool exportMetadata(const RasterDataDescriptor *pDescriptor,
         const DynamicObject *pDynObj, 
         ossimContainerProperty *pProperties);

      /**
       * Typedef for function pointer to use to import a specific piece of
       * metadata.
       *
       * Write functions which follow this typedef to add custom behavior
       * for import.  See importMetadataValue for an example.
       */
      typedef bool(*ImportFunction)(const ossimPropertyInterface *pProperties,
         RasterDataDescriptor *pDescriptor, 
         DynamicObject *pDynObj, const std::string& appName, 
         const std::string& ossimName);

      /**
       * Typedef for function pointer to use to export a specific piece of
       * metadata.
       *
       * Write functions which follow this typedef to add custom behavior
       * for import.  See exportMetadataValue for an example.
       */
      typedef bool(*ExportFunction)(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop, 
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * Generic function to import most metadata.
       *
       * This function is templated on type to import to the DynamicObject.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      template<typename T>
      static bool importMetadataValue(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName)
      {
         if (pPropertyInterface == NULL)
         {
            return false;
         }

         ossimRefPtr<ossimProperty> pProperty = pPropertyInterface->getProperty(ossimName);
         ossimProperty* pProp = pProperty.get();
         if (pProp == NULL)
         {
            return false;
         }

         DataVariant var;
         string value = pProp->valueToString();
         if (var.fromXmlString(TypeConverter::toString<T>(), value) == DataVariant::SUCCESS)
         {
            return pDynObj->setAttribute(appName, var);
         }

         return false;
      }

      /**
       * Generic function to export most metadata.
       *
       * This function is templated on type stored in the DynamicObject.
       *
       * @param pDescriptor
       *        Descriptor to export from.
       * @param prop
       *        DataVariant with stored metadata.
       * @param pProperties
       *        ossimContainerProperty to export the metadata to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully exported, false otherwise.
       */
      template<typename T>
      static bool exportMetadataValue(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop, 
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName)
      {
         const T *pT = prop.getPointerToValue<T>();
         if (pT == NULL)
         {
            return false;
         }

         string stringValue = StringUtilities::toXmlString(*pT);
         if (!stringValue.empty())
         {
            ossimRefPtr<ossimProperty> pProperty = pProperties->getProperty(ossimName);
            if (pProperty.get() != NULL)
            {
               return pProperty->setValue(stringValue);
            }
         }

         return false;
      }

      /**
       * Import an ossimBinaryDataProperty object as a vector<unsigned char>.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      static bool importBinaryData(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * Export vector<unsigned char> to an ossimBinaryDataProperty object.
       *
       * @param pDescriptor
       *        Descriptor to export from.
       * @param prop
       *        DataVariant with stored metadata.
       * @param pProperties
       *        ossimContainerProperty to export the metadata to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully exported, false otherwise.
       */
      static bool exportBinaryData(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * Import ossimColorProperty objects as ColorType objects.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      static bool importColor(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * Export ColorType objects to ossimColorProperty objects.
       *
       * @param pDescriptor
       *        Descriptor to export from.
       * @param prop
       *        DataVariant with stored metadata.
       * @param pProperties
       *        ossimContainerProperty to export the metadata to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully exported, false otherwise.
       */
      static bool exportColor(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * Import dates stored in CCYYMMDDhhmmss format.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      static bool importDateCCYYMMDDhhmmss(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor,
         DynamicObject *pDynObj, const std::string& appName,
         const std::string& ossimName);

      /**
       * Export dates stored in CCYYMMDDhhmmss format.
       *
       * @param pDescriptor
       *        Descriptor to export from.
       * @param prop
       *        DataVariant with stored metadata.
       * @param pProperties
       *        ossimContainerProperty to export the metadata to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully exported, false otherwise.
       */
      static bool exportDateCCYYMMDDhhmmss(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * Import dates stored in CCYYMMDD format.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      static bool importDateCCYYMMDD(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor, 
         DynamicObject *pDynObj, const std::string& appName, 
         const std::string& ossimName);

      /**
       * Export dates stored in CCYYMMDD format.
       *
       * @param pDescriptor
       *        Descriptor to export from.
       * @param prop
       *        DataVariant with stored metadata.
       * @param pProperties
       *        ossimContainerProperty to export the metadata to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully exported, false otherwise.
       */
      static bool exportDateCCYYMMDD(const RasterDataDescriptor *pDescriptor, 
         const DataVariant &prop,
         ossimContainerProperty *pProperties, const std::string& appName,
         const std::string& ossimName);

      /**
       * Import dates stored in DDHHMMSSZMONYY format.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      static bool importDateDDHHMMSSZMONYY(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor, 
         DynamicObject *pDynObj, const std::string& appName, 
         const std::string& ossimName);

      /**
       * Import dates stored in YYMMDD format.
       *
       * @param pPropertyInterface
       *        OSSIM property interface to import from.
       * @param pDescriptor
       *        Descriptor to import to.
       * @param pDynObj
       *        DynamicObject to import to.
       * @param appName
       *        The name which is used internally.
       * @param ossimName
       *        The name which OSSIM uses to access the property.
       *
       * @return True if the metadata was successfully imported, false otherwise.
       */
      static bool importDateYYMMDD(const ossimPropertyInterface *pPropertyInterface,
         RasterDataDescriptor *pDescriptor, 
         DynamicObject *pDynObj, const string& appName, 
         const string& ossimName);

      /**
       * An Element contains all information needed to import
       * and export a piece of metadata.
       */
      struct Element
      {
         /**
          * The name used internally.
          */
         std::string mAppName;

         /**
          * The name used in OSSIM.
          */
         std::string mOssimName;

         /**
          * The function used to import the metadata.
          */
         ImportFunction mImportFunction;

         /**
          * The function used to export the metadata.
          */
         ExportFunction mExportFunction;

         /**
          * Constructor to fully populate the object.
          */
         Element(const std::string &appName, const std::string &ossimName,
            ImportFunction importFunction, ExportFunction exportFunction);

         /**
          * Constructor to populate the object with only an ImportFunction and ExportFunction.
          */
         Element(ImportFunction importFunction, ExportFunction exportFunction);
      };

      /**
       * Contains all Element objects for import and/or export.
       */
      std::vector<Element> mElements;

      /**
       * The version of the NITF file - either Nitf::VERSION_02_00 or Nitf::VERSION_02_10.
       */
      const std::string mFileVersion;

   private:
      Header& operator=(const Header& rhs);
   };
}

#endif
