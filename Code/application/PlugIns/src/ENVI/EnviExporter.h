/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENVIEXPORTER_H
#define ENVIEXPORTER_H

#include "ExporterShell.h"
#include "TypesFile.h"

#include <string>

class Progress;
class QRadioButton;
class QWidget;
class RasterElement;
class RasterFileDescriptor;
class Step;

class EnviExporter : public ExporterShell
{
public:
   EnviExporter();
   ~EnviExporter();

   QWidget* getExportOptionsWidget(const PlugInArgList* pArgList);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(const PlugInArgList* pArgList);
   bool exportHeaderFile() const;
   bool exportDataFile() const;

private:
   Progress* mpProgress;
   RasterElement* mpRaster;
   RasterFileDescriptor* mpFileDescriptor;
   bool mExportDataFile;

   QWidget* mpOptionsWidget;
   QRadioButton* mpDataFileRadio;
   Step* mpStep;

   std::string convertInterleaveToText(InterleaveFormatType interleave) const;
};

#endif
