/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RESULTSEXPORTER_H
#define RESULTSEXPORTER_H

#include "DesktopServices.h"
#include "ExporterShell.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "Testable.h"
#include "TypesFile.h"

#include <string>
#include <ostream>

class Progress;
class RasterElement;
class ResultsOptionsWidget;
class RasterFileDescriptor;
class Step;

class ResultsExporter : public ExporterShell, public Testable
{
public:
   ResultsExporter();
   ~ResultsExporter();

   QWidget* getExportOptionsWidget(const PlugInArgList *pInArgList);

   bool setBatch();
   bool setInteractive();
   bool hasAbort();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

   bool runOperationalTests(Progress* pProgress, std::ostream& failure);
   bool runAllTests(Progress *pProgress, std::ostream& failure);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);
   bool isValueExported(double dValue, const std::vector<int>& badValues) const;
   std::string getLocationString(unsigned int uiRow, unsigned int uiColumn, RasterElement *pGeo) const;
   RasterElement* getGeoreferencedRaster() const;
   bool writeOutput(std::ostream &stream);

private:
   bool mbInteractive;
   bool mbAbort;

   Progress* mpProgress;
   RasterElement* mpResults;
   RasterFileDescriptor* mpFileDescriptor;
   double mFirstThreshold;
   double mSecondThreshold;
   PassArea mPassArea;
   GeocoordType mGeocoordType;
   bool mbMetadata;
   bool mbAppendFile;

   ResultsOptionsWidget* mpOptionsWidget;

   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpModel;
   Service<PlugInManagerServices> mpPlugInManager;
   Step* mpStep;
   std::string mMessage;
};

#endif
