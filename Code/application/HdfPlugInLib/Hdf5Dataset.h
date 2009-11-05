/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5DATASET_H
#define HDF5DATASET_H

#include "EnumWrapper.h"
#include "Hdf5CustomReader.h"
#include "Hdf5Data.h"
#include "Hdf5Element.h"
#include "Hdf5Resource.h"
#include "TypesFile.h"

#include <hdf5.h>

#include <memory>
#include <string>

class Hdf5Dataset : public Hdf5Element, public Hdf5Data
{
public:
   /**
    * Gets whether the dataset is compound or not.
    *
    * Compound datasets have no data encoding (UNKNOWN), 0 per element, and 0 count.
    * If this function returns true, HdfPlugInLib will not have parsed the dataset because
    * the HdfPlugInLib does not support loading compound datasets.
    *
    * @return Returns whether the dataset is a compound dataset (ie. vdata) or not.
    */
   bool isCompoundDataset() const;

   /**
    * Gets the data encoding type for the HDF dataset.
    *
    * For non-native types or compound datasets, EncodingType will not be valid (EncodingType.isValid()).
    *
    * @param  encoding
    *         The data encoding type that corresponds to the member string.
    *         For those types that do not correspond to an EncodingType (ie. a compound dataset),
    *         the EncodingType will not be valid.
    *
    */
   void getDataEncoding(EncodingType& encoding) const;

protected:
   // The Hdf5Group must be a friend so the Hdf5Group can delete and create Hdf5Datasets
   friend class Hdf5Group;

   /**
    * Creates an Hdf5Dataset object that represents the data set object of an HDF file.
    *
    * To create a dataset, use Hdf5Group::addDataset().
    */
   explicit Hdf5Dataset(Hdf5Element* pParent, const std::string& name);

   /**
    * Destroys the Hdf5Dataset object.
    *
    * To destroy an %Hdf5Dataset from a group, call Hdf5Group::removeDataset().
    */
   virtual ~Hdf5Dataset();

   Hdf5Data::DataReader* createReader() const;
};

#endif
