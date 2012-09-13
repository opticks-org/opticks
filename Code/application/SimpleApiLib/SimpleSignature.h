/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLESIGNATURE_H__
#define SIMPLESIGNATURE_H__

#include "AppConfig.h"

class Signature;
class SignatureSet;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
    * @file SimpleSignature.h
    * This file contains API utilities for accessing signatures.
    */

   /**
    * Access the number of data sets in a signature.
    *
    * @param pSig
    *        The signature DataElement to access.
    * @return The number of data sets in the signature. A zero may indicate failure.
    */
   EXPORT_SYMBOL uint32_t getSignatureDataSetCount(DataElement* pSig);

   /**
    * Get the name of the indexed data set.
    *
    * The index to name correspondence should not be considered static. The index
    * is used to access an std::set which sorted alphanumerically by name. Adding or removing
    * a data set to a signature may result in a reodering of the names.
    *
    * This only accesses the data names (via Signature::getDataNames()). The unit names are
    * not accessible but generally there are no units without a corresponding data set so this
    * should not cause problems.
    *
    * @param pSig
    *        The signature DataElement to access.
    * @param index
    *        The index of the data set.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getSignatureDataSetName(DataElement* pSig, uint32_t index, char* pName, uint32_t nameSize);

   /**
    * Get the signature data set with the specified name.
    *
    * @param pSig
    *        The signature DataElement to access.
    * @param pName
    *        The \c NULL terminated name of the dataset to access.
    * @return The value for the named dataset. This is a shared DataVariant so freeDataVariant() should not be called.
    *         \c NULL will be returned on error.
    */
   EXPORT_SYMBOL DataVariant* getSignatureDataSet(DataElement* pSig, const char* pName);

   /**
    * Set the signature data set with the specified name.
    *
    * @param pSig
    *        The signature DataElement to mutate.
    * @param pName
    *        The \c NULL terminated name of the dataset to mutate.
    * @param pValue
    *        The DataVariant with the new value. After returning, pValue
    *        will contain the data previously in the data set named by pName.
    *        If the data set did not exist, pValue will be an invalid DataVariant.
    * @return A non-zero on error or a zero on success.
    */
   EXPORT_SYMBOL int setSignatureDataSet(DataElement* pSig, const char* pName, DataVariant* pValue);

   /**
    * Access the number of signatures in a signature set.
    *
    * @param pSet
    *        The signature set to access.
    * @return The number of signatures in the signature set. A zero may indicate failure.
    */
   EXPORT_SYMBOL uint32_t getSignatureSetCount(DataElement* pSet);

   /**
    * Get the indexed signature.
    *
    * @param pSet
    *        The signature set to access.
    * @param index
    *        The index of the signature.
    * @return the signature as a DataElement or \c NULL on error.
    */
   EXPORT_SYMBOL DataElement* getSignatureSetSignature(DataElement* pSet, uint32_t index);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif
