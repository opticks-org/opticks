/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURE_LIBRARY_H
#define SIGNATURE_LIBRARY_H

#include "SignatureSet.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class RasterElement;

/**
 *  A container for multiple signatures.
 *
 *  This class provides encapsulation for multiple signatures into a single object.  The
 *  SignatureLibrary is also a SignatureSet, so it can contain metadata, a description,
 *  and acquisition values.  The Signature Library's set of Signatures is largely immutable, 
 *  in that, except for an empty library, new signatures cannot be added to it, and, 
 *  for the most part, signatures cannot be removed from it.
 *
 *  When a SignatureLibrary is deleted, Signatures contained in the set are automatically
 *  deleted. It owns its signatures.
 *
 *  The primary benefit of a SignatureLibrary over a standard SignatureSet is that the
 *  SignatureLibrary can store much larger collections of signatures and can more 
 *  efficiently and conveniently resample them. Additionally, a SignatureLibrary
 *  provides faster, more convenient access to its primary ordinate data.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: 
 *    resample(), desample(), import(), insertSignature(), insertSignatures(), clear().
 *  - Everything else documented in SignatureSet.
 *
 *  @see    SignatureSet
 */
class SignatureLibrary : public SignatureSet
{
public:
   /**
    *  Returns the values that the signature ordinate data is currently sampled
    *  to.
    *
    *  Returns the values that the signature ordinate data is currently sampled
    *  to. If the library has not been resampled, this will return an empty
    *  vector.
    *
    *  @return  The active abscissa of the library.
    */
   virtual const std::vector<double>& getAbscissa() const = 0;

   /**
    *  Returns the values that the raw signature library ordinate data is 
    *  sampled to.
    *
    *  @return  The original abscissa of the library.
    */
   virtual const std::vector<double>& getOriginalAbscissa() const = 0;

   /**
    *  Returns the raw ordinate data.
    *
    *  The raw ordinate data is returned as an on-disk RasterElement.
    *
    *  @return  The raw ordinate data.
    */
   virtual const RasterElement *getOriginalOrdinateData() const = 0;

   /**
    *  Returns the raw ordinate data.
    *
    *  The raw ordinate data is returned as an on-disk RasterElement.
    *
    *  @return  The raw ordinate data.
    */
   virtual RasterElement *getOriginalOrdinateData() = 0;

   /**
    *  Returns the resampled ordinate data.
    *
    *  The resampled ordinate data is returned as a single array of doubles
    *  with each signature's ordinate data being contiguous. The number of 
    *  ordinate samples for each signature will be the same as the number of
    *  abscissa values returned from getAbscissa(). If the library has not been
    *  resampled, this returns NULL.
    *
    *  @return  The entire resampled signature library's ordinate data.
    */
   virtual const double *getOrdinateData() const = 0;

   /**
    *  Returns the resampled ordinate data of a specified signature.
    *
    *  The resampled ordinate data is returned as a single array of doubles. 
    *  The number of ordinate samples for the signature will be the same as the
    *  number of abscissa values returned from getAbscissa(). If the library 
    *  has not been resampled, this returns NULL.
    *
    *  @param   index
    *           The index of the signature for which the ordinate data is to be
    *           retrieved.
    *
    *  @return  The signature's resampled ordinate data, or NULL of the library
    *         has not been resampled or the index is invalid.
    */
   virtual const double *getOrdinateData(unsigned int index) const = 0;

   /**
    *  Returns the names of all of the signatures in the library.
    *
    *  @return  The names of all of the signatures in the library.
    */
   virtual std::set<std::string> getSignatureNames() const = 0;

   /**
    *  Returns the name of the specified signature.
    *
    *  @param   index
    *           The index of the signature for which the name is to be 
    *           retrieved.
    *
    *  @return  The name of the specified signature or an empty string if the
    *           index is invalid.
    */
   virtual std::string getSignatureName(unsigned int index) const = 0;

   /**
    *  Returns the specified signature.
    *
    *  @param   index
    *           The index of the signature to be retrieved.
    *
    *  @return  The specified signature, or NULL if the index is invalid.
    */
   virtual const Signature *getSignature(unsigned int index) const = 0;

   /**
    *  Returns the specified signature.
    *
    *  @param   name
    *           The name of the signature to be retrieved.
    *
    *  @return  The specified signature or NULL if the specified signature does
    *           not exist in the library.
    */
   virtual const Signature* getSignature(const std::string& name) const = 0;

   /**
    *  Returns the name of the library's abscissa.
    *
    *  @return  The abscissa name.
    */
   const std::string& getAbscissaName() const;

   /**
    *  Resamples the raw ordinate data to the specified abscissa.
    *
    *  Resamples the entire raw ordinate data set to the specified abscissa.
    *  After calling this, the getOrdinateData() methods will return non-NULL
    *  pointers to the resampled ordinate data. If the range of the abscissa
    *  is greater than the range of the original abscissa, or if the resampling
    *  fails for any other reason, this method will fail and will leave the
    *  ordinate data unsampled.
    *
    *  @param   abscissa
    *           The abscissa values to resample the ordinate data to. If this 
    *           is empty, the ordinate data will not be resampled and 
    *           getOrdinateData() will return NULL.
    *
    *  @return  True if resampling was successful, or false otherwise.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool resample(const std::vector<double> &abscissa) = 0;

   /**
    *  Frees up all resources associated with resampling the library.
    *
    *  Frees up all resources associated with resampling the library. The 
    *  library is restored to the state it was in immediately after being
    *  imported, but before being resampled.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void desample() = 0;

   /**
    *  Imports a new signature library file into the library object.
    *
    *  This method clears the SignatureLibrary and then imports a new library
    *  file into it. The library will be imported as an on-disk RasterElement
    *  and will be available via getOriginalOrdinateData(). This method will
    *  fail if the library is not empty.
    *
    *  @param filename
    *         The full pathname of the file to import.
    *
    *  @param importerName
    *         The name of the importer to use to import the library as an on-
    *         disk RasterElement.
    *
    *  @return  True if the import succeeded and false otherwise.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see clear()
    *  @see getOriginalOrdinateData()
    */
   virtual bool import(const std::string &filename, const std::string &importerName) = 0;

   /**
    *  Adds an existing signature into the library. 
    *
    *  Adds an existing signature into an empty library. If the library is not
    *  empty, this method will fail.
    *
    *  @param   pSignature
    *           The signature to add to the library.
    *
    *  @return  TRUE if the signature was successfully added to the library, otherwise FALSE.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool insertSignature(Signature* pSignature) = 0;

   /**
    *  Adds several signatures to the library. 
    *
    *  Adds several signatures into any empty library. The signatures must be 
    *  unique. If any of the sigs to add are NULL, the method will fail, 
    *  having done nothing. If the library is not empty, this method will fail.
    *
    *  @param   signatures
    *           A vector of signatures to add to the library
    *
    *  @return  true if the signatures were added, otherwise false.
    *
    *  @notify  This method will notify Subject::signalModified after the 
    *           sigs are done being added to the list. Only one notification 
    *           will be done. No notification will be done if signatures is empty.
    */
   virtual bool insertSignatures(const std::vector<Signature*>& signatures) = 0;

   /**
    *  Removes a signature from the library.
    *
    *  Removes a signature from the library. This will only succeed if the 
    *  signature is the only signature in the library.
    *
    *  @param   pSignature
    *           The signature to remove from the library.
    *
    *  @param   bDelete
    *           This value is ignored. If the method succeeds, the signatures
    *           will be deleted.
    *
    *  @return  TRUE if the signature successfully removed from the library, otherwise
    *           FALSE.  FALSE is also returned if the given signature does not exist
    *           in the library.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool removeSignature(Signature* pSignature, bool bDelete = false) = 0;

   /**
    *  Removes several signatures from the library.
    *
    *  This method removes several sigs from the library. The list must contain
    *  all of the sigs in the library or the method will fail. If the list of 
    *  sigs to remove contains sigs not in the library, those sigs will be 
    *  ignored.
    *
    *  @param   signatures
    *           A vector of sigs to remove from the library
    *
    *  @param   bDelete
    *           This value is ignored. If the method succeeds, the signatures
    *           will be deleted.
    *
    *  @return  true if the the list of signaturess to remove was non-empty and 
    *           all of the specified sigs were found in the set, 
    *           otherwise false.
    *
    *  @notify  This method will notify Subject::signalModified after the 
    *           signatures are done being removed from the list. Only one 
    *           notification will be done.
    */
   virtual bool removeSignatures(const std::vector<Signature*>& signatures, bool bDelete = false) = 0;

   /**
    *  Removes all of the signatures from the library.
    *
    *  This method removes all of the signatures from the library. The 
    *  signatures will be deleted in the process.
    *
    *  @param   bDelete
    *           This value is ignored. The signatures will be deleted.
    *
    *  @notify  This method will notify Subject::signalModified after the signatures are
    *           done being removed from the library if the library was not empty.
    *           At most one notification will be done.
    */
   virtual void clear(bool bDelete = false) = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~SignatureLibrary() {}
};

#endif
