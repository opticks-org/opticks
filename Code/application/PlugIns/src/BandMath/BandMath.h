/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifdef BandMath_EXPORTS
#define BandMath_API __declspec(dllexport)
#else
#define BandMath_API __declspec(dllimport)
#endif

#include <math.h>
#include "bmathfuncs.h"

#include "AlgorithmShell.h"
#include "ApplicationServices.h"
#include "BitMask.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlugInManagerServices.h"
#include "RasterElement.h"
#include "UtilityServices.h"

#include <string>
#include <vector>

class Step;

class BandMath : public AlgorithmShell
{
private:
   Step *mpStep;
   bool mbInteractive;   //default to interactive mode
   bool mbDisplayResults;

   std::string mstrProgressString;
   ReportingLevel meGabbiness;
   int mCubeRows, mCubeColumns, mCubeBands;

   bool mbGuiIsNeeded;
   bool mbDegrees;
   bool mbCubeMath;
   bool mbAsLayerOnExistingView;

   std::vector<RasterElement*> mCubesList;

   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpDataModel;
   Service<PlugInManagerServices> mpPluginManager;
   Service<ApplicationServices> mpApplication;
   Service<ObjectFactory> mpObjFact;
   Service<UtilityServices> mpUtilities;

   // Source data
   RasterElement *mpCube;
   RasterElement *mpCube2;
   RasterElement *mpCube3;
   RasterElement *mpCube4;
   RasterElement *mpCube5;

   Progress *mpProgress; 
   std::string mResultsName;
   std::string mExpression;

   RasterElement* mpResultData; 

   bool mbError;

   bool parse(PlugInArgList *, PlugInArgList *);
   void displayErrorMessage();
   bool createReturnValue(std::string partialResultsName);
   bool createReturnGuiElement();

public:
   BandMath();
   ~BandMath();

   bool isInputValid(PlugInArgList &);
   bool setBatch();
   bool setInteractive();
   bool getInputSpecification(PlugInArgList *&);
   bool getOutputSpecification(PlugInArgList *&);
   bool execute(PlugInArgList *, PlugInArgList *);
   bool initialize();
};
