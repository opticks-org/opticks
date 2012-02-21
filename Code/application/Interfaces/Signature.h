/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "DataElement.h"
#include "DataVariant.h"

#include <set>
#include <string>
#include <vector>

class Units;

/**
 * Signature with identifying attributes
 *
 * A signature is an object that essentially consists of a map of data sets, along with a map of
 * optional Units objects. Each data set is referred to as a component of the signature.
 * The unique name associated with each data set is called its component name and is the key for
 * the map of data sets. The optional Units object associated with a data set is stored in a map using the
 * data set component name as the key.
 *
 * This subclass of Subject will notify upon the following conditions:
 * - The following method is called: setData().
 * - Everything else documented in DataElement.
 *
 * @see    DataElement, Units
 */
class Signature : public DataElement
{
public:
   /**
    * Emitted with any<std::pair<string,DataVariant> > when an attribute is added or changed.
    */
   SIGNAL_METHOD(Signature, DataChanged)

   /**
    * Gets a data set from the signature.
    *
    * This method returns a data set from the signature in the form of a
    * DataVariant. 
    *
    * @param   name
    *          The name of the data set, as provided when the data set was
    *          added to the Signature.
    *
    * @return  A DataVariant containing the requested data set. If no data
    *          set exists in the signature with the specified name, it will 
    *          return an invalid DataVariant.
    *
    * @see  DataVariant::isValid()
    */
   virtual const DataVariant& getData(const std::string& name) const = 0;

   /**
    * Adds a data set to the signature.
    *
    * This method adds a data set to the signature. A deep copy of the
    * value will be performed. If a data set already exists in the
    * Signature with the name specified, it will be replaced with the new
    * value.
    *
    * This method is preferred to adoptData() unless
    * you are passing an already constructed DataVariant in
    * which case, adoptData() will be faster because
    * it avoids a deep copy.
    *
    * @param   name
    *          The name of the data set to be added to the signature.
    *
    * @param   data
    *          The data set to be added to the Signature. For example:
    *
    * @code
    * bool populateSignature(Signature *pSig, 
    *                        const vector<double> &wavelengths,
    *                        const vector<double> &reflectances)
    * {
    *    if (pSig == NULL) return false;
    *    pSig->setData("wavelengths", wavelengths);
    *    pSig->setData("reflectances", reflectances);
    *    return true;
    * }
    * @endcode
    *
    * @notify  This method will notify signalDataChanged with
    *          any<std::pair<string,DataVariant> > after the 
    *          data set is added to the Signature.
    */
   template<typename T>
   void setData(const std::string& name, const T& data)
   {
      DataVariant temp(data);
      adoptData(name, temp);
   }

   /**
    * Adds a data set to the signature.
    *
    * This method adds a data set to the signature. A deep copy of the
    * value will be performed. If a data set already exists in the
    * Signature with the name specified, it will be replaced with the new
    * value.
    *
    * This method should not be used; generally setData() is
    * preferred. This method and setData() have identical
    * performance characteristics and setData() is easier to
    * call.  This method is faster than setData() though if
    * you have an already constructed DataVariant and is 
    * preferred to setData() in this case.
    *
    * @param   name
    *          The name of the data set to be added to the signature.
    *
    * @param   data
    *          The data set to be added to the Signature. On return, this
    *          will contain the value previously stored. If the value did
    *          not previously exist, then this will contain an invalid
    *          DataVariant.
    *
    * @notify  This method will notify signalDataChanged with
    *          any<std::pair<string,DataVariant> > after the 
    *          data set is added to the Signature.
    */
   virtual void adoptData(const std::string& name, DataVariant& data) = 0;

   
   /**
    * A convenience method that returns the Units object returned from calling
    * SignatureDataDescriptor::getUnits.
    *
    * @param   name
    *          The component name of the Units object to get from the signature.
    *
    * @return  A pointer to the Units object with the specified component name. \c NULL
    *          will be returned if no Units object exists with the specified
    *          name.
    *
    * @note    This pointer should not be stored. It may become invalid at a later time.
    */
   virtual const Units* getUnits(const std::string& name) const = 0;

   /**
    * Gets the names associated with data in this Signature.
    *
    * @return set of names
    */
   virtual std::set<std::string> getDataNames() const = 0;

   /**
    * Gets the names associated with units in this Signature.
    *
    * @return set of names
    */
   virtual std::set<std::string> getUnitNames() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~Signature() {}
};

#endif   // SIGNATURE_H
