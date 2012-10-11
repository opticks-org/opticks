/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURESET_H
#define SIGNATURESET_H

#include "Signature.h"

#include <vector>

/**
 *  A container for multiple signatures.
 *
 *  This class provides encapsulation for multiple signatures into a single object.  The
 *  SignatureSet is also a type of signature, so it can contain metadata, a description,
 *  and acquisition values.  Signatures can be inserted into and removed from the set.
 *
 *  When a SignatureSet is deleted, Signatures contained in the set are not automatically
 *  deleted unless the SignatureSet is an ancestor of the Signature.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: insertSignature(), removeSignature(),
 *    clearSignatures().
 *  - Everything else documented in Signature.
 *
 *  @see    Signature
 */
class SignatureSet : public Signature
{
public:
   /**
    *  Adds an existing signature into the set.
    *
    *  @param   pSignature
    *           The signature to add to the set.
    *
    *  @return  TRUE if the signature was successfully added to the set, otherwise FALSE.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool insertSignature(Signature* pSignature) = 0;

   /**
    *  Add several signatures to the set. It is possible to add the same sig
    *  to the set multiple times.
    *
    *  This method adds several sigs to the set. If any of the sigs to
    *  add are NULL, the method will fail, having done nothing. 
    *
    *  @param   signatures
    *           A vector of signatures to add to the set
    *
    *  @return  true if the signatures were added, otherwise false.
    *
    *  @notify  This method will notify Subject::signalModified after the 
    *           sigs are done being added to the list. Only one notification 
    *           will be done. No notification will be done if signatures is empty.
    */
   virtual bool insertSignatures(const std::vector<Signature*>& signatures) = 0;

   /**
    *  Checks to see if a signature is in the set.
    *
    *  @param   pSignature
    *           The signature to check for.
    *
    *  @return  true if the specified signature is in the set, otherwise false.
    */
   virtual bool hasSignature(Signature* pSignature) const = 0;

   /**
    *  Counts the signatures in the set.
    *
    *  @return  the number of signatures in the set.
    */
   virtual unsigned int getNumSignatures() const = 0;

   /**
    *  Returns a vector of pointers to the signatures in the set.
    *
    *  @return  The vector of signature pointers.
    */
   virtual std::vector<Signature*> getSignatures() const = 0;

   /**
    *  Removed a signature from the set.
    *
    *  @param   pSignature
    *           The signature to remove from the set.
    *
    *  @param   bDelete
    *           TRUE if the signature should be deleted when removed from the set,
    *           otherwise FALSE.
    *
    *  @return  TRUE if the signature successfully removed from the set, otherwise
    *           FALSE.  FALSE is also returned if the given signature does not exist
    *           in the set.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool removeSignature(Signature* pSignature, bool bDelete = false) = 0;

   /**
    *  Removes several signatures from the set.
    *
    *  This method removes several sigs from the set, optionally deleting
    *  them in the process. If the list of sigs to remove contains sigs
    *  not in the set, those sigs will be ignored.
    *
    *  @param   signatures
    *           A vector of sigs to remove from the set
    *
    *  @param   bDelete
    *           Specifies if the signatures should be destroyed as well as being 
    *           removed from the set. Signatures not found in the set will
    *           not be deleted regardless of the value of this parameter.
    *
    *  @return  Returns \c true if the list of signatures to remove was not
    *           empty and all of the specified signatures were found in the
    *           set; otherwise returns \c false.
    *
    *  @notify  This method will notify Subject::signalModified after the 
    *           signatures are done being removed from the list. Only one 
    *           notification will be done.
    */
   virtual bool removeSignatures(const std::vector<Signature*>& signatures, bool bDelete = false) = 0;

   /**
    *  Removes all of the signatures from the set.
    *
    *  This method removes all of the signatures from the set, optionally deleting
    *  them in the process.
    *
    *  @param   bDelete
    *           Specifies if the signatures should be destroyed as well as being 
    *           removed from the set.
    *
    *  @notify  This method will notify Subject::signalModified after the signatures are
    *           done being removed from the set if the set was not empty.
    *           At most one notification will be done.
    */
   virtual void clear(bool bDelete = false) = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~SignatureSet() {}
};

#endif
