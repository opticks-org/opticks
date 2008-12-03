/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "FileDescriptor.h"
#include "IcePseudocolorLayerExporter.h"
#include "IceWriter.h"
#include "PlugInArgList.h"
#include "PseudocolorLayer.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"

using namespace std;

IcePseudocolorLayerExporter::IcePseudocolorLayerExporter() :
   IceExporterShell(IceUtilities::PSEUDOCOLOR_LAYER),
   mpLayer(NULL),
   mpOutputDescriptor(NULL)
{
   setName("Ice PseudocolorLayer Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setShortDescription("Exports Ice Pseudocolor Layer Files");
   setDescriptorId("{D404DACF-82B9-4c98-BBEC-E3BEAA6CB09A}");
   setExtensions("Ice Pseudocolor Layers Files (*.psl.ice.h5)");
   setSubtype(TypeConverter::toString<PseudocolorLayer>());
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

IcePseudocolorLayerExporter::~IcePseudocolorLayerExporter()
{
}

bool IcePseudocolorLayerExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   DO_IF(IceExporterShell::getInputSpecification(pArgList) == false, return false);
   VERIFY(pArgList->addArg<PseudocolorLayer>(ExportItemArg()));
   VERIFY(pArgList->addArg<FileDescriptor>(ExportDescriptorArg()));
   return true;
}

void IcePseudocolorLayerExporter::parseInputArgs(PlugInArgList* pInArgList)
{
   IceExporterShell::parseInputArgs(pInArgList);

   mpLayer = pInArgList->getPlugInArgValue<PseudocolorLayer>(ExportItemArg());
   ICEVERIFY_MSG(mpLayer != NULL, "No pseudocolor layer to export.");

   mpOutputDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
   ICEVERIFY_MSG(mpOutputDescriptor != NULL, "No output file descriptor provided.");
}

void IcePseudocolorLayerExporter::getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
   RasterFileDescriptor*& pOutputFileDescriptor)
{
   pOutputCube = dynamic_cast<RasterElement*>(mpLayer->getDataElement());
   ICEVERIFY_MSG(pOutputCube != NULL, "Layer does not contain a Raster Element.");

   pOutputFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
      RasterUtilities::generateFileDescriptorForExport(pOutputCube->getDataDescriptor(),
      mpOutputDescriptor->getFilename().getFullPathAndName()));
   ICEVERIFY_MSG(pOutputFileDescriptor != NULL, "Unable to create an output file descriptor.");
}

void IcePseudocolorLayerExporter::finishWriting(IceWriter& writer)
{
   writer.writeLayer("/Layers/PseudocolorLayer1", outputCubePath(), mpLayer, mpProgress);
}
