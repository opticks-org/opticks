/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICERASTERELEMENTEXPORTER_H
#define ICERASTERELEMENTEXPORTER_H

#include "IceExporterShell.h"

class PlugInArgList;
class RasterElement;
class RasterFileDescriptor;

class IceRasterElementExporter : public IceExporterShell
{
public:
   IceRasterElementExporter();
   virtual ~IceRasterElementExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);

private:
   void parseInputArgs(PlugInArgList* pInArgList);
   void getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
      RasterFileDescriptor*& pOutputFileDescriptor);

   RasterElement* mpCube;
   RasterFileDescriptor* mpOutputDescriptor;
};

#endif
