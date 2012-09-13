/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <string>

#include "Hdf5Dataset.h"
#include "Hdf5Group.h"
#include "StringUtilities.h"

using namespace std;

Hdf5Group::Hdf5Group(Hdf5Node* pParent, const string& name)
: Hdf5Element(pParent, name)
{
}

Hdf5Group::~Hdf5Group()
{
   vector<Hdf5Element*>::iterator it = mElements.begin();
   while (it != mElements.end())
   {
      Hdf5Element* pElement = *it;
      delete pElement;
      ++it;
   }

   mElements.clear();
}

Hdf5Dataset* Hdf5Group::addDataset(const string& name)
{
   Hdf5Dataset* pDataset = new Hdf5Dataset(this, name);
   if (pDataset != NULL)
   {
      mElements.push_back(pDataset);
   }

   return pDataset;
}

const vector<Hdf5Element*>& Hdf5Group::getElements() const
{
   return mElements;
}

const Hdf5Element* Hdf5Group::getElement(const string& name) const
{
   if (name.empty() == true)
   {
      return NULL;
   }

   vector<Hdf5Element*>::const_iterator iter = mElements.begin();
   while (iter != mElements.end())
   {
      Hdf5Element* pElement = *iter;
      if (pElement != NULL)
      {
         string currentName = pElement->getName();
         if (currentName == name)
         {
            return pElement;
         }
      }

      ++iter;
   }

   return NULL;
}

const Hdf5Element* Hdf5Group::getElementByPath(const string& path) const
{
   string localPath = path;
   if (localPath.empty())
   {
      return NULL;
   }
   if (localPath[0] == '/')
   {
      localPath = localPath.substr(1);
   }
   vector<string> pathComponents = StringUtilities::split(localPath, '/');
   if (pathComponents.empty())
   {
      return NULL;
   }
   string finalName = pathComponents.back();
   pathComponents.pop_back();
   const Hdf5Group* pCurrentObj = this;
   for (vector<string>::iterator iter = pathComponents.begin();
        iter != pathComponents.end();
        ++iter)
   {
      if (pCurrentObj == NULL)
      {
         return NULL;
      }
      pCurrentObj = dynamic_cast<const Hdf5Group*>(pCurrentObj->getElement(*iter));
   }
   if (pCurrentObj == NULL)
   {
      return NULL;
   }
   return pCurrentObj->getElement(finalName);
}

size_t Hdf5Group::getNumElements() const
{
   return mElements.size();
}

bool Hdf5Group::removeElement(const Hdf5Element* pElement)
{
   if (pElement != NULL)
   {
      vector<Hdf5Element*>::iterator iter = mElements.begin();
      while (iter != mElements.end())
      {
         Hdf5Element* pCurrentElement = *iter;
         if (pElement == pCurrentElement)
         {
            mElements.erase(iter);
            return true;
         }

         ++iter;
      }
   }

   return false;
}

Hdf5Group* Hdf5Group::addGroup(const string& name)
{
   Hdf5Group* pGroup = new Hdf5Group(this, name);
   if (pGroup != NULL)
   {
      mElements.push_back(pGroup);
   }
   return pGroup;
}
