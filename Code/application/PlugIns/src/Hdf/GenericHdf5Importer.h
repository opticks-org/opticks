/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GENERIC_HDF5_IMPORTER_H
#define GENERIC_HDF5_IMPORTER_H

#include "Hdf5ImporterShell.h"
#include "Testable.h"

#include <string>
#include <vector>

class DatasetParameters;
class ObjectFactory;
class Subject;

/**
 * A basic HDF5 importer that reads 1 HDF dataset based on user input
 * and loads it into memory. Behaves similarly to GenericImporter.
 *
 * This plug-in is designed to illustrate the ability of sharing information
 * between a GUI used in the Options Dialog (HdfBrowserWidget) and an importer.
 * It also shows how to import any kind of HDF 5 Dataset as a data cube.
 *
 * See help in Hdf5ImporterShell on how to build this sample plugin.
 */
class GenericHdf5Importer : public Hdf5ImporterShell, public Testable
{
public:
   GenericHdf5Importer();
   ~GenericHdf5Importer();

   bool runOperationalTests(Progress* pProgress, std::ostream& failure);
   bool runAllTests(Progress* pProgress, std::ostream& failure);

   unsigned char getFileAffinity(const std::string& filename);

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
}; 

#endif
