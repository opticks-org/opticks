/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DYNAMICTYPES_H__
#define DYNAMICTYPES_H__

#include "AppConfig.h"

class DataVariant;
class PlugInArg;
class PlugInArgList;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
   * @file DynamicTypes.h
   * This file contains functions and type definitions for accessing Opticks dynamic types.
   */

   /**
    * Create a new DataVariant and deep copies the value.
    *
    * Sets an error message if the DataVariant can't be created.
    *
    * @param pType
    *        The type of the object to wrap in the variant. The format of the 
    *        string should match that used by DataVariant::getTypeName().
    * @param pValue
    *        A pointer to the object to copy into the variant. If this is \c NULL,
    *        a default initialized object of the specified type will be created.
    * @return A new DataVariant or \c NULL if an error occured.
    *         Must be freed with freeDataVariant() if ownership is not explicitly transfered.
    * @default Always calls the DataVariant constructor with strict=false.
    * @see DataVariant::DataVariant()
    */
   EXPORT_SYMBOL DataVariant* createDataVariant(const char* pType, const void* pValue);

   /**
    * Create a DataVariant from a string.
    *
    * Sets an error message if the DataVariant can't be created.
    *
    * @param pType
    *        The type string for the DataVariant.
    * @param pValue
    *        The string data to convert.
    * @param xml
    *        If 0, convert using the display string conversion, otherwise convert using the XML string conversion.
    * @return A new DataVariant or \c NULL if an error occured.
    *         Must be freed with freeDataVariant() if ownership is not explicitly transfered.
    */
   EXPORT_SYMBOL DataVariant* createDataVariantFromString(const char* pType, const char* pValue, int xml);

   /**
    * Free a DataVariant created with createDataVariant().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pDv
    *        The DataVariant to free. No error checking is performed on this value
    *        and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void freeDataVariant(DataVariant* pDv);

   /**
    * Does the DataVariant contain a valid value?
    *
    * Sets an error message if the DataVariant is \c NULL.
    * @param pDv
    *        The DataVariant to check.
    * @return 0 if the variant is invalid, non-zero if it is valid.
    */
   EXPORT_SYMBOL int isDataVariantValid(DataVariant* pDv);

   /**
    * Get the type name of the value stored in the DataVariant.
    *
    * @param pDv
    *        The DataVariant whose type should be accessed.
    * @param pType
    *        Buffer to store the type name. This will be \c NULL terminated.
    * @param typeSize
    *        The size of the Buffer. If the type name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the type name.
    */
   EXPORT_SYMBOL uint32_t getDataVariantTypeName(DataVariant* pDv, char* pType, uint32_t typeSize);

   /**
    * Get a pointer to the object stored in the DataVariant.
    *
    * Sets an error message if the value can not be accessed.
    *
    * @param pDv
    *        The DataVariant whose type should be accessed.
    * @return a pointer to the value or \c NULL if an error occurred. This pointer
    *         should not be modified.
    */
   EXPORT_SYMBOL void* getDataVariantValue(DataVariant* pDv);

   /**
    * Convert the variant value to a string.
    *
    * Sets an error if the inputs are invalid, the data can't be accessed, or another error occurs.
    *  SIMPLE_BAD_PARAMS - The passed in parameters are invalid.
    *  SIMPLE_WRONG_TYPE - The value has no string conversion for the specified string type.
    *  SIMPLE_BUFFER_SIZE - The value buffer is not large enough to hold the value string.
    *  SIMPLE_OTHER_FAILURE - The conversion to a string failed for an unknown reason.
    *
    * @param pDv
    *        The DaraVariant whose type should be accessed.
    * @param xml
    *        If 0, convert using the display string conversion, otherwise convert using the XML string conversion.
    * @param pValue
    *        A buffer to store the result.
    * @param valueSize
    *        The size of pValue.
    * @return The actual size of the converted value or 0 if an error occurred.
    *
    * @see DataVariant::toDisplayString, DataVariant::toXmlString()
    */
   EXPORT_SYMBOL uint32_t getDataVariantValueString(DataVariant* pDv, int xml, char* pValue, uint32_t valueSize);

   /**
    * Unwrap an std::vector.
    *
    * DataVariant can hold an std::vector of certain types but std::vector is a C++
    * class so manipulation can be difficult. This method returns a C array of the
    * underlying datatype given an std::vector. i.e. an std::vector<double> will
    * be turned into a double* of length std::vector<double>::size().
    *
    * @param pValue
    *        The void* that is a pointer to a vector.
    * @param pType
    *        The \c NULL terminated string which is a vector type. Must be std::vector<type>.
    * @param pOutValue
    *        An output parameter which will contain a C array of the underlying data type.
    *        This is equavalent to &value.front(). A deep copy is not performed so this
    *        is a borrow reference.
    * @return The length of the array in bytes. The caller must convert this to an element count.
    *         A 0 may indicate an empty array or an error so getLastError() should be checked.
    */
   EXPORT_SYMBOL uint32_t vectorToArray(void* pValue, const char* pType, void** pOutValue);

   /**
    * Free a PlugInArgList which has had ownership transfered to the caller.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pPial
    *        The PlugInArgList to free. No error checking is performed on this value
    *        and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void freePlugInArgList(PlugInArgList* pPial);

   /**
    * Get the number of PlugInArgs in a PlugInArgList.
    *
    * @param pPial
    *        The PlugInArgList.
    * @return The number of PlugInArgs. This will be 0 if an error occured but the
    *         error code must be checked as 0 may be a non-error return value as well.
    */
   EXPORT_SYMBOL uint32_t getPlugInArgCount(PlugInArgList* pPial);

   /**
    * Get a PlugInArg from a PlugInArgList.
    *
    * @param pPial
    *        The PlugInArgList.
    * @param pArgName
    *        The name of the PlugInArg.
    * @return The PlugInArg requested or \c NULL if an error occurred.
    */
   EXPORT_SYMBOL PlugInArg* getPlugInArgByName(PlugInArgList* pPial, const char* pArgName);

   /**
    * Get a PlugInArg from a PlugInArgList.
    *
    * @param pPial
    *        The PlugInArgList.
    * @param argNum
    *        The index of the PlugInArg.
    * @return The PlugInArg requested or \c NULL if an error occurred.
    */
   EXPORT_SYMBOL PlugInArg* getPlugInArgByIndex(PlugInArgList* pPial, uint32_t argNum);

   /**
    * Get the name of a PlugInArg.
    *
    * @param pPia
    *        The PlugInArg whose type should be accessed.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the Buffer. If the name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getPlugInArgName(PlugInArg* pPia, char* pName, uint32_t nameSize);

   /**
    * Get the type name of a PlugInArg.
    *
    * @param pPia
    *        The PlugInArg whose type should be accessed.
    * @param pType
    *        Buffer to store the type name. This will be \c NULL terminated.
    * @param typeSize
    *        The size of the Buffer. If the type name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the type name.
    */
   EXPORT_SYMBOL uint32_t getPlugInArgTypeName(PlugInArg* pPia, char* pType, uint32_t typeSize);

   /**
    * Get a description of a PlugInArg suitable for display to the user.
    *
    * @param pPia
    *        The PlugInArg whose description should be accessed.
    * @param pDesc
    *        Buffer to store the description. This will be \c NULL terminated.
    * @param descSize
    *        The size of the Buffer. If the description is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the description.
    */
   EXPORT_SYMBOL uint32_t getPlugInArgDescription(PlugInArg* pPia, char* pDesc, uint32_t descSize);

   /**
    * Does the PlugInArg have a default value?
    *
    * @param pPia
    *        The PlugInArg to check.
    * @return 0 if the default value is not set, non-zero if it is set.
    */
   EXPORT_SYMBOL int isPlugInArgDefaultSet(PlugInArg* pPia);

   /**
    * Does the PlugInArg have an actual value?
    *
    * @param pPia
    *        The PlugInArg to check.
    * @return 0 if the actual value is not set, non-zero if it is set.
    */
   EXPORT_SYMBOL int isPlugInArgActualSet(PlugInArg* pPia);

   /**
    * Get the default value of a PlugInArg.
    *
    * @param pPia
    *        The PlugInArg whose value should be accessed.
    * @return a pointer to the value or \c NULL if an error occurred. This pointer
    *         should not be modified. An error will be set if there is no default value.
    */
   EXPORT_SYMBOL void* getPlugInArgDefaultValue(PlugInArg* pPia);

   /**
    * Get the actual value of a PlugInArg.
    *
    * @param pPia
    *        The PlugInArg whose value should be accessed.
    * @return a pointer to the value or \c NULL if an error occurred. This pointer
    *         should not be modified. An error will be set if there is no actual value.
    */
   EXPORT_SYMBOL void* getPlugInArgActualValue(PlugInArg* pPia);

   /**
    * A convenience function which returns the actual value if set, otherwise
    * the default value if set, otherwise an error.
    *
    * @param pPia
    *        The PlugInArg whose value should be accessed.
    * @return a pointer to the value or \c NULL if an error occurred. This pointer
    *         should not be modified. An error will be set if there is no actual value
    *         and no default value.
    * @see getPlugInArgDefaultValue(), getPlugInArgActualValue()
    */
   EXPORT_SYMBOL void* getPlugInArgValue(PlugInArg* pPia);

   /**
    * Set the default value of a PlugInArg.
    *
    * @default Always sets tryDeepCopy to true.
    *
    * Sets the following errors:
    *  SIMPLE_BAD_PARAMS - \c NULL pPia
    *  SIMPLE_OTHER_FAILURE - After attempting to set the value, PlugInArg::isDefaultSet() is false.
    *
    * @param pPia
    *        The PlugInArg whose value should be set.
    * @param pValue
    *        A pointer to the value.
    * @see PlugInArg::setDefaultValue(const void*, bool)
    */
   EXPORT_SYMBOL void setPlugInArgDefaultValueFromVoid(PlugInArg* pPia, void* pValue);

   /**
    * Set the default value of a PlugInArg using a DataVariant.
    *
    * @default Always sets tryDeepCopy to true.
    *
    * Sets the following errors:
    *  SIMPLE_BAD_PARAMS - \c NULL pPia
    *  SIMPLE_OTHER_FAILURE - After attempting to set the value, PlugInArg::isDefaultSet() is false.
    *  SIMPLE_WRONG_TYPE - The DataVariant type does not match the PlugInArg type.
    *
    * @param pPia
    *        The PlugInArg whose value should be set.
    * @param pValue
    *        A pointer to a DataVariant which will be deep copied to the PlugInArg value.
    * @see PlugInArg::setDefaultValue(const void*, bool)
    */
   EXPORT_SYMBOL void setPlugInArgDefaultValueFromDataVariant(PlugInArg* pPia, DataVariant* pValue);

   /**
    * Set the actual value of a PlugInArg.
    *
    * @default Always sets tryDeepCopy to true.
    *
    * Sets the following errors:
    *  SIMPLE_BAD_PARAMS - \c NULL pPia
    *  SIMPLE_OTHER_FAILURE - After attempting to set the value, PlugInArg::isActualSet() is false.
    *
    * @param pPia
    *        The PlugInArg whose value should be set.
    * @param pValue
    *        A pointer to the value.
    * @see PlugInArg::setActualValue(const void*, bool)
    */
   EXPORT_SYMBOL void setPlugInArgActualValueFromVoid(PlugInArg* pPia, void* pValue);

   /**
    * Set the actual value of a PlugInArg using a DataVariant.
    *
    * @default Always sets tryDeepCopy to true.
    *
    * Sets the following errors:
    *  SIMPLE_BAD_PARAMS - \c NULL pPia
    *  SIMPLE_OTHER_FAILURE - After attempting to set the value, PlugInArg::isActualSet() is false.
    *  SIMPLE_WRONG_TYPE - The DataVariant type does not match the PlugInArg type.
    *
    * @param pPia
    *        The PlugInArg whose value should be set.
    * @param pValue
    *        A pointer to a DataVariant which will be deep copied to the PlugInArg value.
    * @see PlugInArg::setActualValue(const void*, bool)
    */
   EXPORT_SYMBOL void setPlugInArgActualValueFromDataVariant(PlugInArg* pPia, DataVariant* pValue);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif