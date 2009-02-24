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
class ApplicationServices;
class SpatialDataView;
class Step;

class PCA : public AlgorithmShell
{
public:
   PCA();
   ~PCA();

   bool setBatch();
   bool setInteractive();
   bool hasAbort();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   void m_CalculateEigenValues();
   bool extractInputArgs(const PlugInArgList* pArgList);
   bool m_CreatePCACube();
   bool m_ComputePCAwhole();
   bool m_ComputePCAaoi();
   bool m_CreatePCAView();

private:
   ExecutableResource mpSecondMoment;
   bool mbUseEigenValPlot;
   int m_MaxScaleValue;
   EncodingType m_OutputDataType;
   std::string mMessage;
   AoiElement* getAoiElement(const std::string& aoiName);
   bool m_WriteOutPCAtransform(QString filename);
   bool m_ReadInPCAtransform(QString filename);
   bool m_ComputeCovarianceMatrix(QString aoiName = "", int rowSkip = 1, int colSkip = 1);
   bool m_ReadMatrixFromFile(QString filename, double **pData, int numBands, const std::string &caption);
   bool m_WriteMatrixToFile(QString filename, const double **pData, int numBands, const std::string &caption);
   bool m_GetStatistics(std::vector<std::string> aoiList);
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
   Step* mpStep;

   enum CalcMethodTypeEnum { SECONDMOMENT, COVARIANCE, CORRCOEF };

   /**
    * @EnumWrapper PCA::CalcMethodTypeEnum.
    */
   typedef EnumWrapper<CalcMethodTypeEnum> CalcMethodType;
   CalcMethodType m_CalcMethod;
   bool mbInteractive;
   bool mbAbort;
   std::vector<unsigned int> mSelectedBands;
};

#endif   // PCA_H
