/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5NODE_H
#define HDF5NODE_H

class Hdf5File;

#include <string>

/**
 * This class serves as a base class for the rest of the Hdf5File,
 * Hdf5Group, etc. classes. It provides basic properties and traversal capability
 * that should be available to all of these classes.
 */
class Hdf5Node
{
public:
   virtual ~Hdf5Node() {}

   /**
    * Returns the parent element. The root Hdf5Group object will have a parent of Hdf5File.
    * The Hdf5File will have a NULL parent.
    *
    * @return returns the parent element of this element or NULL if no parent can be found.
    */
   virtual Hdf5Node* getParent() const;

   /**
    * Returns the full path to this node from the root group of the HDF5 file.
    *
    * @return Returns the full path to this node.
    */
   virtual std::string getFullPathAndName() const;

   /**
    * Returns a pointer to the Hdf5File object that this object belongs to.
    *
    * @return Returns a pointer to the owning Hdf5File object.
    */
   virtual Hdf5File* getFile() const;

   /**
    * Gets the node name.
    *
    * @return  Returns the name of the node.
    */
   virtual std::string getName() const;

protected:
   Hdf5Node(Hdf5Node* pParent, const std::string& name);

   Hdf5Node* mpParent;
   std::string mFullPathAndName;
   std::string mName;
};

#endif
