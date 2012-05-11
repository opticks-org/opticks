/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREFILEDESCRIPTOR_H
#define SIGNATUREFILEDESCRIPTOR_H

#include "FileDescriptor.h"

#include <set>
#include <string>

class Units;

/**
 *  Describes how a signature data element is stored in a file on disk.
 *
 *  In addition to the ancillary information stored in the FileDescriptor base
 *  class, this class contains information pertinent to how signature data elements
 *  are stored in files on disk.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: setUnits()
 *  - All notifications documented in FileDescriptor.
 *
 *  @see        Signature, SignatureDataDescriptor, Units
 */
class SignatureFileDescriptor : public FileDescriptor
{
public:
   /**
    * Emitted with boost::any<std::pair<std::string, const \link Units\endlink*> > when a Units object is added or changed.
    */
   SIGNAL_METHOD(SignatureFileDescriptor, UnitsChanged)

   /**
    *  Sets the units for the values in the signature element.
    *
    *  @param   name
    *           The component name of the data set to which the given units should be associated.
    *           This method does nothing if an empty name is passed in.
    *
    *  @param   pUnits
    *           The units of the values in the signature element.
    *
    * @notify  This method will notify signalUnitsChanged() with
    *          boost::any<std::pair<std::string, const \link Units\endlink*> > after the 
    *          Units object is added to the Signature.
    *
    * @note    This method makes a deep copy of \em pUnits. It does not take ownership of \em pUnits.
    *          A change in the associated Units instance does not change the data values,
    *          it only affects how the data is interpreted by plug-ins that use the signature.
    */
   virtual void setUnits(const std::string& name, const Units* pUnits) = 0;

   /**
    *  Returns read-only access to the element's Units object.
    *
    *  @param   name
    *           The component name associated with the Units instance to be returned.
    *
    *  @return  A const pointer to the element's Units object.  The units
    *           represented by the returned pointer should not be modified.
    *           The values can be modified using the following code:
    *  @code
    *     const Units* pUnits = pSignatureFileDescriptor->getUnits(name);
    *     FactoryResource<Units> pModifiedUnits;
    *
    *     // make deep copy of existing Units object values except for the item/s to be modified
    *     pModifiedUnits->setUnitName(pUnits->getUnitName());
    *     pModifiedUnits->setUnitType(pUnits->getUnitType());
    *     pModifiedUnits->setScaleFromStandard(0.1);             // value to be changed
    *     pModifiedUnits->setRangeMin(pUnits->getRangeMin());
    *     pModifiedUnits->setRangeMax(pUnits->getRangeMax());
    *
    *     // apply the changes
    *     pSignatureFileDescriptor->setUnits(name, pModifiedUnits.get());
    *  @endcode
    *
    * @note    This pointer should not be stored. It may become invalid at a later time.
    */
   virtual const Units* getUnits(const std::string& name) const = 0;

   /**
    *  Returns the component names for the Units associated with the signature's data.
    *
    *  @return  A set containing the component names for all the Units instances
    *           associated with the signature's data.
    */
   virtual std::set<std::string> getUnitNames() const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~SignatureFileDescriptor() {}
};

#endif
