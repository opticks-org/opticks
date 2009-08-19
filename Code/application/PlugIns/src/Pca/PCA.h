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
   ~PCA();

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
   bool mbUseEigenValPlot;
   int m_MaxScaleValue;
   EncodingType m_OutputDataType;
   std::string mMessage;
   AoiElement* getAoiElement(const std::string& aoiName);
   bool writeOutPCAtransform(QString filename);
   bool readInPCAtransform(QString filename);
   bool computeCovarianceMatrix(QString aoiName = "", int rowSkip = 1, int colSkip = 1);
   bool readMatrixFromFile(QString filename, double **pData, int numBands, const std::string &caption);
   bool writeMatrixToFile(QString filename, const double **pData, int numBands, const std::string &caption);
   bool getStatistics(std::vector<std::string> aoiList);
   BitMask* mp_AOIbitmask;
   bool mb_UseAoi;
   bool mDisplayResults;

   unsigned int m_NumRows;
   unsigned int m_NumColumns;
   unsigned int m_NumBands;
   double** mp_MatrixValues;
   QString m_ROIname;
   unsigned int m_NumComponentsToUse;
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
   CalcMethodType m_CalcMethod;
   std::vector<unsigned int> mSelectedBands;
};

#endif   // PCA_H
