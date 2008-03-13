/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <string>

#include "Hdf4Dataset.h"
#include "Hdf4Group.h"

using namespace std;

Hdf4Group::Hdf4Group(const std::string& name) : Hdf4Element(name)
{
}

Hdf4Group::~Hdf4Group()
{
   vector<Hdf4Element*>::iterator it = mElements.begin();
   while (it != mElements.end())
   {
      Hdf4Element* pElement = *it;
      delete pElement;
      ++it;
   }

   mElements.clear();
}

Hdf4Dataset* Hdf4Group::addDataset(const string& name)
{
   Hdf4Dataset* pDataset = new Hdf4Dataset(name);
   if (pDataset != NULL)
   {
      mElements.push_back(pDataset);
   }

   return pDataset;
}

const vector<Hdf4Element*>& Hdf4Group::getElements() const
{
   return mElements;
}

const Hdf4Element* Hdf4Group::getElement(const string& name) const
{
   if (name.empty() == true)
   {
      return NULL;
   }

   vector<Hdf4Element*>::const_iterator iter = mElements.begin();
   while (iter != mElements.end())
   {
      Hdf4Element* pElement = *iter;
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

size_t Hdf4Group::getNumElements() const
{
   return mElements.size();
}

bool Hdf4Group::removeElement(const Hdf4Element* pElement)
{
   if (pElement != NULL)
   {
      vector<Hdf4Element*>::iterator iter = mElements.begin();
      while (iter != mElements.end())
      {
         Hdf4Element* pCurrentElement = *iter;
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

Hdf4Group* Hdf4Group::addGroup(const string& name)
{
   Hdf4Group* pGroup = new Hdf4Group(name);
   if (pGroup != NULL)
   {
      mElements.push_back(pGroup);
   }
   return pGroup;
}
