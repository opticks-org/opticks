/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEPLUGIN_H  //Required for current class
#define GEOREFERENCEPLUGIN_H

#include "AlgorithmShell.h"
#include "DesktopServices.h"
#include "Georeference.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"

using namespace std;

class Georeference;
class PlugInArgList;
class RasterElement;

class GeoreferencePlugIn : public AlgorithmShell
{
public:
   GeoreferencePlugIn();
   ~GeoreferencePlugIn();

   bool execute(PlugInArgList* pInParam, PlugInArgList* pOutParam);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);

protected:
   void preparePluginVectors();
   void cleanUpPluginVectors();

private:
   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpDataModel;
   Service<PlugInManagerServices> mpPluginManager;
   std::string mMessageText;

   bool mCreateLayer;
   bool mDisplayLayer;
   Progress* mpProgress;
   RasterElement* mpRaster;
   SpatialDataView* mpView;
   string mResultsName;

   std::vector<PlugIn*> mlstPlugins;
   std::vector<std::string> mlstPluginNames;
   std::vector<QWidget*> mlstPluginWidgets;
};

#endif // GEOREFERENCEPLUGIN_H
