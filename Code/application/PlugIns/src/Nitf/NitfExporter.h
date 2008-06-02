/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITF_EXPORTER_H
#define NITF_EXPORTER_H

#include "ApplicationServices.h"
#include "DesktopServices.h"
#include "ExporterShell.h"
#include "PlugInManagerServices.h"
#include "ModelServices.h"
#include "UtilityServices.h"

#include <ossim/base/ossimListener.h>

class Classification;
class OptionsNitfExporter;
class RasterElement;
class RasterLayer;
class PlugInArgList;
class Progress;
class RasterFileDescriptor;
class ossimNitfFileHeaderV2_1;
class ossimNitfImageHeaderV2_1;

namespace Nitf
{
   class NitfExporter : public ExporterShell, public ossimListener
   {
   public:
      NitfExporter(void);
      ~NitfExporter(void);

      bool getInputSpecification(PlugInArgList *&pArgList);
      bool getOutputSpecification(PlugInArgList *&pArgList);

      bool execute(PlugInArgList *pInParam, PlugInArgList *pOutParam);

      bool hasAbort();
      bool abort();
      bool abortRequested();

      Progress *getProgress();
      RasterElement *getRasterElement();

      void processEvent(ossimEvent &event);

      ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const;

      QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);

   private:
      bool validateExportDescriptor(const RasterFileDescriptor* pDescriptor, std::string& errorMessage) const;
      bool setBandRepresentation(const RasterChannelType& eColor,
         const std::string& representation,
         ossimNitfImageHeaderV2_1* pImageHeader);
      bool exportClassification(const PlugInArgList* pArgList, const Classification* pClassification,
         ossimNitfFileHeaderV2_1* pFileHeader, ossimNitfImageHeaderV2_1* pImageHeader, std::string& errorMessage);

      Service<DesktopServices> mpDesktopSvcs;
      Service<ModelServices> mpDataModel;
      Service<UtilityServices> mpUtilSvcs;
      Service<PlugInManagerServices> mpPlugInManager;
      Service<ApplicationServices> mpApplicationManager;
      std::auto_ptr<OptionsNitfExporter> mpOptionsWidget;

      RasterElement *mpRaster;
      RasterLayer *mpRasterLayer;
      Progress *mpProgress;
      RasterFileDescriptor *mpDestination;
      bool mbAborted;
   };

}
#endif
