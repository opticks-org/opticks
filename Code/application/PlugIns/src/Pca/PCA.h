/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PCA_H
#define PCA_H

#include <QtCore/QString>

#include "AlgorithmShell.h"
#include "ApplicationServices.h"
#include "BitMask.h"
#include "DesktopServices.h"
#include "EnumWrapper.h"
#include "MessageLogMgr.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterElement.h"
#include "UtilityServices.h"

#include <vector>
#include <string>

class AlgorithmResource;
class AoiElement;
class ApplicationServices;
class SpatialDataView;
class Step;

class PCA : public AlgorithmShell
{
public:
   PCA();
   virtual ~PCA();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   void calculateEigenValues();
   bool extractInputArgs(const PlugInArgList* pArgList);
   bool createPCACube();
   bool computePCAwhole();
   bool computePCAaoi();
   bool createPCAView();

private:
   ExecutableResource mpSecondMoment;
   ExecutableResource mpCovariance;
   bool mUseEigenValPlot;
   int mMaxScaleValue;
   int mMinScaleValue;
   EncodingType mOutputDataType;
   std::string mMessage;
   AoiElement* getAoiElement(const std::string& aoiName);
   bool writeOutPCAtransform(QString filename);
   bool readInPCAtransform(QString filename);
   bool computeCovarianceMatrix(QString aoiName = "", int rowSkip = 1, int colSkip = 1);
   bool readMatrixFromFile(QString filename, double **pData, int numBands, const std::string &caption);
   bool writeMatrixToFile(QString filename, const double **pData, int numBands, const std::string &caption);
   bool getStatistics(std::vector<std::string> aoiList);
   BitMask* mpAoiBitMask;
   bool mUseAoi;
   bool mDisplayResults;

   unsigned int mNumRows;
   unsigned int mNumColumns;
   unsigned int mNumBands;
   double** mpMatrixValues;
   QString mRoiName;
   unsigned int mNumComponentsToUse;
   Service<PlugInManagerServices> mpPlugInMgr;
   Service<ModelServices> mpModel;
   Service<ObjectFactory> mpObjFact;
   Service<DesktopServices> mpDesktop;
   Service<UtilityServices> mpUtilities;
   Service<ApplicationServices> mpAppSvcs;
   Progress* mpProgress;
   SpatialDataView* mpView;
   RasterElement* mpRaster;
   RasterElement* mpPCARaster;
   RasterElement* mpSecondMomentMatrix;
   RasterElement* mpCovarianceMatrix;
   Step* mpStep;

   enum CalcMethodTypeEnum { SECONDMOMENT, COVARIANCE, CORRCOEF };

   /**
    * @EnumWrapper PCA::CalcMethodTypeEnum.
    */
   typedef EnumWrapper<CalcMethodTypeEnum> CalcMethodType;
   CalcMethodType mCalcMethod;
   std::vector<unsigned int> mSelectedBands;
};

#endif   // PCA_H
