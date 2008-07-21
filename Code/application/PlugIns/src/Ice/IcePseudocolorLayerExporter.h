/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICE_PSEUDOCOLOR_LAYER_EXPORTER_H
#define ICE_PSEUDOCOLOR_LAYER_EXPORTER_H

#include "IceExporterShell.h"

class FileDescriptor;
class PseudocolorLayer;

class IcePseudocolorLayerExporter : public IceExporterShell
{
public:
   IcePseudocolorLayerExporter();
   virtual ~IcePseudocolorLayerExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);

private:
   void parseInputArgs(PlugInArgList* pInArgList);
   void getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
      RasterFileDescriptor*& pOutputFileDescriptor);
   void finishWriting(IceWriter& writer);

   PseudocolorLayer* mpLayer;
   FileDescriptor* mpOutputDescriptor;
};

#endif
