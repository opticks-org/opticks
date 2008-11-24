/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAMPLEHDF4IMPORTER_H
#define SAMPLEHDF4IMPORTER_H

#include "AppConfig.h"
#if defined(HDF4_SUPPORT)

#include "Hdf4ImporterShell.h"

#include <string>
#include <vector>

class DataDescriptor;

/**
 * A basic HDF4 importer that reads 1 HDF dataset from the NASA MODIS sensor
 * and loads it into memory. This is one of the most basic importers
 * to load a single HDF dataset into a RasterElement.
 *
 * See help in Hdf4ImporterShell on how to build this sample plugin.
 */
class SampleHdf4Importer : public Hdf4ImporterShell
{
public:
   SampleHdf4Importer();
   unsigned char getFileAffinity(const std::string& filename);

private:
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
};

#endif

#endif
