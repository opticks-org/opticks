/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf5Node.h"

#include "Hdf5Element.h"
#include "Hdf5File.h"

using namespace std;

Hdf5Node::Hdf5Node(Hdf5Node* pParent, const string& name)
: mpParent(pParent), mName(name)
{
   if (mpParent != NULL)
   {
      string parentPath = mpParent->getFullPathAndName();
      if (parentPath == "/")
      {
         mFullPathAndName = parentPath + mName;
      }
      else
      {
         mFullPathAndName = parentPath + "/" + mName;
      }
   }
}

Hdf5Node* Hdf5Node::getParent() const
{
   return mpParent;
}

string Hdf5Node::getFullPathAndName() const
{
   return mFullPathAndName;
}

Hdf5File* Hdf5Node::getFile() const
{
   Hdf5Node* pParent = getParent();
   Hdf5Node* foundElement = NULL;
   while (pParent != NULL)
   {
      foundElement = pParent;
      pParent = pParent->getParent();
   }
   return dynamic_cast<Hdf5File*>(foundElement);
}

string Hdf5Node::getName() const
{
   return mName;
}
