/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "IceRasterElementExporter.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksIce, IceRasterElementExporter);

IceRasterElementExporter::IceRasterElementExporter() :
   IceExporterShell(IceUtilities::RASTER_ELEMENT),
   mpCube(NULL),
   mpOutputDescriptor(NULL)
{
   setName("Ice Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setShortDescription("Exports Ice Raster Element Files");
   setDescriptorId("{B53C892E-DF8C-453a-A5E6-B6267977FE98}");
   setExtensions("Ice Raster Element Files (*.re.ice.h5)");
   setSubtype(TypeConverter::toString<RasterElement>());
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

IceRasterElementExporter::~IceRasterElementExporter()
{
}

bool IceRasterElementExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   DO_IF(IceExporterShell::getInputSpecification(pArgList) == false, return false);
   VERIFY(pArgList->addArg<RasterElement>(Exporter::ExportItemArg(), "Element to be exported."));
   VERIFY(pArgList->addArg<RasterFileDescriptor>(Exporter::ExportDescriptorArg(), "File descriptor for the element to be exported."));
   return true;
}

void IceRasterElementExporter::parseInputArgs(PlugInArgList* pInArgList)
{
   IceExporterShell::parseInputArgs(pInArgList);

   mpCube = pInArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   ICEVERIFY_MSG(mpCube != NULL, "No cube to export.");

   mpOutputDescriptor = pInArgList->getPlugInArgValue<RasterFileDescriptor>(Exporter::ExportDescriptorArg());
   ICEVERIFY_MSG(mpOutputDescriptor != NULL, "No output file descriptor provided.");
}

void IceRasterElementExporter::getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
   RasterFileDescriptor*& pOutputFileDescriptor)
{
   pOutputCube = mpCube;
   pOutputFileDescriptor = mpOutputDescriptor;
}
