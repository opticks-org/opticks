/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Hdf5Attribute.h"
#include "Hdf5Dataset.h"
#include "Hdf5File.h"
#include "Hdf5Group.h"
#include "Hdf5ImporterShell.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "ObjectResource.h"
#include "ProgressTracker.h"
#include "RasterElement.h"
#include <map>
using namespace std;

namespace
{
   typedef map<unsigned long, bool> ObjectIdMap;
   struct Hdf5OpData
   {
      explicit Hdf5OpData(Hdf5Group* pGroup, ObjectIdMap& objMap) :
         mMap(objMap),
         mpGroup(pGroup)
      {
      }

      inline void insertId(unsigned long id)
      {
         mMap.insert(ObjectIdMap::value_type(id, true));
      }

      // returns 0 if not found, -1 if found
      inline herr_t idNotFound(unsigned long id) const
      {
         ObjectIdMap::const_iterator found = mMap.find(id);
         return found != mMap.end() ? -1 : 0;
      }

      ObjectIdMap mMap;
      Hdf5Group* mpGroup;
   };

   /**
    *  PRIVATE, NON-MEMBER FUNCTION that is used with H5Aiterate1 to add attributes to an Hdf5Element
    *
    *  This method is used internally only while building the Hdf5File object for a given HDF5 file.
    *
    *  The function signature is defined by the HDF5 specification. "Multiple arguments" are
    *  supported by passing in a structure to the void* argument, though we do not do that here.
    *  It must also be a non-member function since that is what H5Aiterate1 requires.
    * 
    *  @param   loc_id
    *           The handle to a dataset or group, passed in from H5Giterate or H5Diterate.
    *  @param   name
    *           The name of the attribute to open.
    *  @param   pData
    *           pData is an Hdf5Element object which contains the API for adding an attribute.
    *           Since Hdf5Group and Hdf5Dataset both have attributes, this lets us treat groups
    *           and datasets as the same in functions such as these.
    *
    *  @return  0 if the operation succeeds, -1 otherwise.
    */
   herr_t populateAttributes(hid_t loc_id, const char* name, void* pData)
   {
      if (pData == NULL)
      {
         return -1;
      }

      Hdf5Element* pElement = reinterpret_cast<Hdf5Element*>(pData);
      Hdf5AttributeResource attrId(H5Aopen_name(loc_id, name));

      string strName(name);

      DataVariant var;
      HdfUtilities::readHdf5Attribute(*attrId, var);
      Hdf5Attribute* pAttr = pElement->addAttribute(strName, var);
      if (pAttr != NULL)
      {
         Hdf5TypeResource attrType(H5Aget_type(*attrId));
         Hdf5DataSpaceResource attrSpace(H5Aget_space(*attrId));
         pAttr->setTypeName(HdfUtilities::hdf5TypeToTypeString(*attrType));
         hsize_t sizeArray[H5S_MAX_RANK]; // contains an array of all of the dimensions
         int numDims = H5Sget_simple_extent_dims(*attrSpace, sizeArray, NULL);    
         vector<hsize_t> dimensionSizes;
         for (int i = 0; i < numDims; ++i)
         {
            dimensionSizes.push_back(sizeArray[i]);
         }
         pAttr->setDimensionSizes(dimensionSizes);
      }

      return 0;
   }

   /**
    *  PRIVATE, NON-MEMBER FUNCTION that is used with H5Giterate to add attributes to an Hdf5Element
    *
    *  Populates the internal data structure that represents the "file system"
    *  contained within the HDF5 file. pStructure is really an Hdf5Element object
    *  so we can populate group and dataset information as we go.
    *
    *  The function signature is defined by the HDF5 specification. "Multiple arguments" are
    *  supported by passing in a structure to the void* argument, though we do not do that here.
    *  It must also be a non-member function since that is what H5Aiterate1 requires.
    * 
    *  @param   loc_id
    *           The handle to a dataset or group, passed in from H5Giterate
    *  @param   name
    *           The name of the group or dataset to open.
    *  @param   pStructure
    *           pStructure is an Hdf5Element object which contains the API for adding an attribute.
    *           Since Hdf5Group and Hdf5Dataset both have attributes, this lets us treat groups
    *           and datasets as the same in functions such as these.
    *
    *  @return  0 if the operation succeeds, -1 otherwise.
    */
   herr_t populateHdfFile(hid_t loc_id, const char *name, void *pStructure)
   {
      H5G_stat_t statbuf;

      if (pStructure == NULL)
      {
         return -1;
      }

      Hdf5OpData* pOpData = reinterpret_cast<Hdf5OpData*>(pStructure);

      H5Gget_objinfo(loc_id, name, 1, &statbuf);
      string strName(name);

      herr_t status = pOpData->idNotFound(statbuf.objno[0]);
      if (status >= 0)
      {
         Hdf5Group* pGroup = reinterpret_cast<Hdf5Group*>(pOpData->mpGroup);
         switch (statbuf.type)
         {
         case H5G_GROUP: 
            {
               if (pGroup != NULL)
               {
                  Hdf5GroupResource innerGroupId(H5Gopen1(loc_id, name));

                  // since groups are non-terminals, we need to make sure there are no
                  // recursive links and no recursive references

                  Hdf5Group* pNewGroup = pGroup->addGroup(strName); // add new group to group

                  pOpData->insertId(statbuf.objno[0]);

                  Hdf5OpData newOpData(pNewGroup, pOpData->mMap);

                  // perform bitwise or so that if one of these returns -1, that value is preserved
                  status = H5Aiterate1(*innerGroupId, NULL, populateAttributes, pNewGroup) |
                           H5Giterate(loc_id, name, NULL, populateHdfFile, &newOpData);
               }
               return status;
            }
         case H5G_DATASET: 
            {
               string fullPathAndName;
               if (pGroup != NULL) // parent is a group
               {
                  Hdf5Element* pDataset = pGroup->addDataset(strName); // add dataset to group
                  if (pDataset != NULL)
                  {
                     fullPathAndName = pDataset->getFullPathAndName();
                  }

                  Hdf5DataSetResource dataId(H5Dopen1(loc_id, fullPathAndName.c_str()));

                  Hdf5TypeResource dtype(H5Dget_type(*dataId));
                  Hdf5DataSpaceResource dspace(H5Dget_space(*dataId));
                  H5T_class_t dataType = H5Tget_class(*dtype); // integer or floating point data?

                  if (pDataset != NULL)
                  {
                     string type = HdfUtilities::hdf5TypeToTypeString(*dtype);
                     Hdf5Dataset* pDs = static_cast<Hdf5Dataset*>(pDataset);
                     hsize_t sizeArray[H5S_MAX_RANK]; // contains an array of all of the dimensions
                     int numDims = H5Sget_simple_extent_dims(*dspace, sizeArray, NULL);    
                     pDs->setTypeName(type);
                     vector<hsize_t> dimensionSizes;
                     for (int i = 0; i < numDims; ++i)
                     {
                        dimensionSizes.push_back(sizeArray[i]);
                     }
                     pDs->setDimensionSizes(dimensionSizes);
                  }
                  else
                  {
                     return -1; // failure
                  }

                  pOpData->insertId(statbuf.objno[0]);

                  status = H5Aiterate1(*dataId, NULL, populateAttributes, pDataset);
               }
               break;
            }
         case H5G_TYPE: 
            {
               // no support currently for "named data types"
               break;
            }
         default:
            break;
         }
      }

      return status;
   }
}

Hdf5File::Hdf5File(const string& filename, hid_t fileHandle) :
   Hdf5Node(NULL, ""),
   mFileHandle(fileHandle),
   mFilename(filename)
{
   mpRootGroup = new Hdf5Group(this, "");
   mFullPathAndName = "";
}

Hdf5File::~Hdf5File()
{
   delete mpRootGroup;
}

Hdf5Group* Hdf5File::getRootGroup() const
{
   return mpRootGroup;
}

Hdf5File* Hdf5File::getFile() const
{
   return const_cast<Hdf5File*>(this);
}

string Hdf5File::getFilename() const
{
   return mFilename;
}

hid_t Hdf5File::getFileHandle() const
{
   return mFileHandle;
}

bool Hdf5File::readFileData(const string& groupPath)
{
   hid_t fileHandle = mFileHandle;
   Hdf5FileResource fileResource;
   if (fileHandle < 0)
   {
      herr_t status = H5Fis_hdf5(mFilename.c_str());
      DO_IF(status <= 0, return false);

      fileResource = Hdf5FileResource(mFilename.c_str());
      DO_IF(*fileResource < 0, return false);
      fileHandle = *fileResource;
   }

   Hdf5Group* pRootGroup = getRootGroup();
   DO_IF(pRootGroup == NULL, return false);

   ObjectIdMap objMap;
   Hdf5OpData opData(pRootGroup, objMap);

   Hdf5GroupResource groupId;
   {  //Turn off error handling while we check for the group, since it may not exist
      Hdf5ErrorHandlerResource errHandler(NULL, NULL);
      groupId = Hdf5GroupResource(H5Gopen1(fileHandle, groupPath.c_str()));
   }
   DO_IF(*groupId < 0, return false);

   //  Read over the root file system and find out all the groups
   // and populate an internal structure used to populate the file info
   return (H5Giterate(*groupId, "/", NULL, populateHdfFile, &opData) >= 0);
}
