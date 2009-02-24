/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICEEXPORTERSHELL_H
#define ICEEXPORTERSHELL_H

#include "ExporterShell.h"
#include "IceUtilities.h"
#include "PlugInManagerServices.h"

#include <string>

class IceWriter;
class OptionsIceExporter;
class PlugInArgList;
class Progress;
class RasterElement;
class RasterFileDescriptor;

class IceExporterShell : public ExporterShell
{
public:
   IceExporterShell(IceUtilities::FileType fileType);
   virtual ~IceExporterShell();

   bool abort();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);

protected:
   Progress* mpProgress;

   virtual void parseInputArgs(PlugInArgList* pInArgList);
   virtual void getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
      RasterFileDescriptor*& pOutputFileDescriptor) = 0;
   virtual void finishWriting(IceWriter& writer);
   void abortIfNecessary();

   static std::string outputCubePath();

private:
   Service<PlugInManagerServices> mpPlugInMgr;
   IceWriter* mpWriter;
   const IceUtilities::FileType mFileType;
   std::auto_ptr<OptionsIceExporter> mpOptionsWidget;
};

#endif
