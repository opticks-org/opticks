/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include "AppVersion.h"
#include "IceRasterElementImporter.h"
#include "PlugInRegistration.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksIce, IceRasterElementImporter);

IceRasterElementImporter::IceRasterElementImporter() :
   IceImporterShell(IceUtilities::RASTER_ELEMENT)
{
   setName("Ice Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setShortDescription("Imports Ice Raster Element Files");
   // the .re.ice.h5 extension is to allow standard HDF5 tools to recognize the file as HDF5
   setExtensions("Ice Raster Element Files (*.re.ice.h5 *.ice.h5)");
   setDescriptorId("{9404083C-0ACF-4fdb-9736-73C1C0AD59AA}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

IceRasterElementImporter::~IceRasterElementImporter()
{
}
