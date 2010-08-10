/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef METADATA_H__
#define METADATA_H__

#include "AppConfig.h"

class DataElement;
class DynamicObject;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
   * @file Metadata.h
   * This file contains functions and type definitions for manipulating DataElement metadata.
   */

   /**
    * Create a new, blank DynamicObject.
    *
    * @return A DynamicObject which the caller owns and must be freed.
    * @see freeDynamicObject()
    */
   EXPORT_SYMBOL DynamicObject* createDynamicObject();

   /**
    * Free a DynamicObject.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pDynamicObject
    *        The DynamicObject to free. No error checking is performed on this value
    *        and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void freeDynamicObject(DynamicObject* pDynamicObject);

   /**
    * Get the metadata for a DataElement.
    *
    * @param pElement
    *        The DataElement to access.
    * @return Pointer to the metadata object or a \c NULL if an error occurred.
    *         This is a borrowed reference.
    */
   EXPORT_SYMBOL DynamicObject* getDataElementMetadata(DataElement* pElement);

   /**
    * Get the number of immediate child attributes.
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @return The number of immediate child elements in the specified DynamicObject.
    * @see DynamicObject::getNumAttributes()
    */
   EXPORT_SYMBOL uint32_t getMetadataAttributeCount(DynamicObject* pMeta);

   /**
    * Get the name of the name of a child attribute.
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param attributeIndex
    *        The index of the attribute.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the Buffer. If the name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getMetadataAttributeName(DynamicObject* pMeta,
      uint32_t attributeIndex, char* pName, uint32_t nameSize);

   /**
    * Get a child attribute.
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param pName
    *        The name of the attribute to access. Should be \c NULL terminated.
    * @return The attribute value as a DataVariant or \c NULL if an error occurred.
    * @see DynamicObject::getAttribute(const std::string&)
    */
   EXPORT_SYMBOL DataVariant* getMetadataAttribute(DynamicObject* pMeta, const char* pName);

   /**
    * Get an attribute from the entire sub-tree of a DynamicObject.
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param pPath
    *        The full path name of the attribute to access. Should be \c NULL terminated.
    * @return The attribute value as a DataVariant or \c NULL if an error occurred.
    * @see DynamicObject::getAttributeByPath(const std::string&)
    */
   EXPORT_SYMBOL DataVariant* getMetadataAttributeByPath(DynamicObject* pMeta, const char* pPath);

   /**
    * Set an attribute by taking ownership of a DataVariant.
    *
    * Will set:
    *   SIMPLE_BAD_PARAMS any of the input values is \c NULL
    *   SIMPLE_OTHER_FAILURE if the attribute set fails
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param pName
    *        The name of the attribute to access. Should be \c NULL terminated.
    * @param pValue
    *        The new value of the attribute. This will be modified to contain the previous
    *        value of the attribute or an invalid DataVariant if no previous value exists.
    * @see DynamicObject::adoptAttribute(const std::string&, DataVariant&)
    */
   EXPORT_SYMBOL void setMetadataAttribute(DynamicObject* pMeta, const char* pName, DataVariant* pValue);

   /**
    * Set an attribute by taking ownership of a DataVariant.
    *
    * Will set:
    *   SIMPLE_BAD_PARAMS any of the input values is \c NULL
    *   SIMPLE_OTHER_FAILURE if the attribute set fails
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param pPath
    *        The full path name of the attribute to access. Should be \c NULL terminated.
    * @param pValue
    *        The new value of the attribute. This will be modified to contain the previous
    *        value of the attribute or an invalid DataVariant if no previous value exists.
    * @see DynamicObject::adoptAttributeByPath(const std::string&, DataVariant*)
    */
   EXPORT_SYMBOL void setMetadataAttributeByPath(DynamicObject* pMeta, const char* pPath, DataVariant* pValue);

   /**
    * Remove an immediate child attribute.
    *
    * Will set:
    *   SIMPLE_BAD_PARAMS any of the input values is \c NULL
    *   SIMPLE_OTHER_FAILURE if the attribute remove fails
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param pName
    *        The name of the attribute to remove. Should be \c NULL terminated.
    * @see DynamicObject::removeAttribute(const std::string&)
    */
   EXPORT_SYMBOL void removeMetadataAttribute(DynamicObject* pMeta, const char* pName);

   /**
    * Remove an attribute from the sub-tree of a DynamicObject.
    *
    * Will set:
    *   SIMPLE_BAD_PARAMS any of the input values is \c NULL
    *   SIMPLE_OTHER_FAILURE if the attribute remove fails
    *
    * @param pMeta
    *        The DynamicObject to access.
    * @param pPath
    *        The full path name of the attribute to remove. Should be \c NULL terminated.
    * @see DynamicObject::removeAttributeByPath(const std::string&)
    */
   EXPORT_SYMBOL void removeMetadataAttributeByPath(DynamicObject* pMeta, const char* pPath);

   /**
    * Remove all attributes from a DynamicObject.
    *
    * @param pMeta
    *        The DynamicObject to clear.
    * @see DynamicObject::clear()
    */
   EXPORT_SYMBOL void clearMetadata(DynamicObject* pMeta);

   /**
    * Access ConfigurationSettings values.
    *
    * @param pSettingKey
    *        The \c NULL terminated setting key to access.
    * @return A DataVariant with the setting value or \c NULL if there was an error.
    *         This is a borrowed reference and should be copied if changes will be made.
    * @see setConfigurationSetting(), ConfigurationSettings::getSetting()
    */
   EXPORT_SYMBOL DataVariant* getConfigurationSetting(const char* pSettingKey);

   /**
    * Change ConfigurationSettings values.
    *
    * @param pSettingKey
    *        The \c NULL terminated setting key to mutate.
    * @param pValue
    *        The value to set. This value will be adopted and after
    *        this function returned, pValue will contain the previous setting
    *        or an invalid DataVariant if there was no previous setting.
    * @return a zero on success or a non-zero on error.
    * @see getConfigurationSetting(), ConfiguationSettings::adoptSetting()
    */
   EXPORT_SYMBOL int setConfigurationSetting(const char* pSettingKey, DataVariant* pValue);

   /**
    * Add the current value of a setting to a DynamicObject for serialization to a defaults file.
    *
    * @param pSettingKey
    *        The \c NULL terminated setting key.
    * @param pDynamicObject
    *        The DynamicObject which will hold the copy.
    * @return a zero on success or a non-zero on error.
    * @see ConfigurationSettings::copySetting()
    */
   EXPORT_SYMBOL int copyConfigurationSetting(const char* pSettingKey, DynamicObject* pDynamicObject);

   /**
    * Serialize a DynamicObject to a file suitable for use as a setting defaults file.
    *
    * @param pFilename
    *        The \c NULL terminated filename. If this is relative, the application's current working directory
    *        will be used as the base path.
    * @param pDynamicObject
    *        The DynamicObject to serialize.
    * @return a zero on success or a non-zero on error.
    * @see ConfigurationSettings::serializeAsDefaults()
    */
   EXPORT_SYMBOL int serializeConfigurationSettingDefaults(const char* pFilename, DynamicObject* pDynamicObject);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif