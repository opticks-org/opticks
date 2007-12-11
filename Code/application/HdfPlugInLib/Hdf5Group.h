/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef HDF5_GROUP_H
#define HDF5_GROUP_H

#include <string>
#include <vector>

#include "Hdf5Element.h"
#include "TypesFile.h"

class Hdf5Dataset;
class Hdf5Node;

/**
 * The %Hdf5Group class is a composite object to hierarchically store groups and datasets.
 */
class Hdf5Group : public Hdf5Element
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
    * @see Hdf5Dataset::Hdf5Dataset()
    */
   virtual Hdf5Dataset* addDataset(const std::string& name);

   /*
    * Adds a group to the %Hdf5Group.
    *
    * @return A pointer to the newly added HDF group object. Returns NULL if the operation failed.
    */
   virtual Hdf5Group* addGroup(const std::string& name);

   /**
    * Returns a vector of the group's elements.
    *
    * @return A vector of the group's elements.
    */
   virtual const std::vector<Hdf5Element*>& getElements() const;

   /**
    * Non-recursively searches the group's element for a dataset matching the input name.
    *
    * @param  name
    *         The name of the Hdf5Element to search for.
    *
    * @return A pointer to the element with the given name; NULL if it does not exist.
    */
   virtual const Hdf5Element* getElement(const std::string& name) const;

   /**
    * Locates the given element with the given path.
    *
    * @param  path
    *         The path to use to locate the element.  The elements in the path
    *         should be separted with "/". A leading slash may be present
    *         and will be ignored. The element names cannot themselves contain
    *         a slash, this should be enforced by the HDF5 library.
    *
    * @return A pointer to the element with the given name; NULL if it does not exist.
    */
   virtual const Hdf5Element* getElementByPath(const std::string& path) const;

   /**
    * Returns the number of Hdf5Elements in the group.
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
    * @see Hdf5Dataset::~Hdf5Dataset(), Hdf5Group::~Hdf5Group
    */
   virtual bool removeElement(const Hdf5Element* pElement);

protected:
   /**
    * Creates an empty HDF group.
    */
   explicit Hdf5Group(Hdf5Node* pParent, const std::string& name);

   /**
    * Destroys the HDF group.
    */
   virtual ~Hdf5Group();

   /* The Hdf5File is the only class that needs to create and destroy Hdf5Groups, so don't let anyone
      create or destroy them.
    */
   friend class Hdf5File;

private:
   std::vector<Hdf5Element*> mElements;
};

#endif
