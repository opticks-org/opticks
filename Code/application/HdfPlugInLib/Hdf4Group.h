/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF4GROUP_H
#define HDF4GROUP_H

#include <string>
#include <vector>

#include "Hdf4Element.h"
#include "TypesFile.h"

/**
 * The Hdf4Group class is a composite object to hierarchically store groups and datasets.
 */
class Hdf4Group : public Hdf4Element
{
public:
   /**
    * Adds a dataset to the HDF group.
    *
    * @param  name
    *         The name of the dataset to add.
    *
    * @return A pointer to the newly added HDF dataset. Returns NULL if the operation failed.
    *
    * @see Hdf4Dataset::Hdf4Dataset()
    */
   virtual Hdf4Dataset* addDataset(const std::string& name);

   /*
    * Adds a group to the %Hdf4Group.
    *
    * @return A pointer to the newly added HDF group object. Returns NULL if the operation failed.
    */
   virtual Hdf4Group* addGroup(const std::string& name);

   /**
    * Returns a vector of the group's elements.
    *
    * @return A vector of the group's elements.
    */
   virtual const std::vector<Hdf4Element*>& getElements() const;

   /**
    * Non-recursively searches the group's element for a dataset matching the input name.
    *
    * @param  name
    *         The name of the Hdf4Element to search for.
    *
    * @return A pointer to the element with the given name; NULL if it does not exist.
    */
   virtual const Hdf4Element* getElement(const std::string& name) const;

   /**
    * Returns the number of Hdf4Elements in the group.
    *
    * @return Returns the number of elements in the group. Equivalent to getElements().size().
    */
   virtual size_t getNumElements() const;

   /**
    * Removes an element from the group.
    *
    * @param  pElement
    *         A pointer to the dataset that will be removed.
    *
    * @return TRUE if the operation succeeded, otherwise FALSE.
    *
    * @see Hdf4Dataset::~Hdf4Dataset(), Hdf4Group::~Hdf4Group
    */
   virtual bool removeElement(const Hdf4Element* pElement);

protected:
   /**
    * Creates an empty HDF group.
    */
   explicit Hdf4Group(const std::string& name);

   /**
    * Destroys the HDF group.
    */
   virtual ~Hdf4Group();

   /* The Hdf4File is the only class that needs to create and destroy Hdf4Groups, so don't let anyone
      create or destroy them.
    */
   friend class Hdf4File;

private:
   std::vector<Hdf4Element*> mElements;
};

#endif
