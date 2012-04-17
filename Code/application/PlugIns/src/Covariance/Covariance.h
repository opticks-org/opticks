/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COVARIANCE_H
#define COVARIANCE_H

#include "AlgorithmShell.h"
#include "AlgorithmPattern.h"
#include "RasterElement.h"

#include <math.h>
#include <string>
#include <vector>

class AoiElement;
class Filename;
class CovarianceGui;
class Progress;

struct Input
{
   Input() : mRowFactor(1), mColumnFactor(1), mpAoi(NULL), 
      mRecalculate(false), mComputeInverse(true) {}
   int mRowFactor;
   int mColumnFactor;
   AoiElement* mpAoi;
   bool mRecalculate;
   bool mComputeInverse;
};

class CovarianceAlgorithm : public AlgorithmPattern
{
public:
   CovarianceAlgorithm(RasterElement* pRasterElement, Progress* pProgress, bool interactive);
   void setFile(const Filename* pCvmFile, bool loadIfExists);
   std::string getFilename() const;
   RasterElement* getCovarianceElement() const;
   RasterElement* getInverseCovarianceElement() const;
   RasterElement* getMeansElement() const;

private:
   CovarianceAlgorithm(const CovarianceAlgorithm& rhs);
   bool preprocess();                     // from AlgorithmPattern. Initializes Raster Elements to NULL.
   bool processAll();                     // from AlgorithmPattern. Controls computation of CVM.
   bool postprocess();                    // from AlgorithmPattern. Does nothing.
   bool initialize(void* pAlgorithmData); // from AlgorithmPattern. Sets the private data from the gui or arg-list
   bool canAbort() const;                 // from AlgorithmPattern. Returns true.
   bool doAbort();                        // from AlgorithmPattern. Aborts computation of the CVM.
   bool readMatrixFromDisk(std::string filename, RasterElement* pElement, ModelResource<RasterElement>& pMeans) const;
   bool writeMatrixToDisk(std::string filename, const RasterElement* pElement, const RasterElement* pMeans) const;

   Input mInput;
   std::string mCvmFile;
   RasterElement* mpNewRasterElement;
   RasterElement* mpNewInvRasterElement;
   RasterElement* mpNewMeansElement;
   bool mAbortFlag;
   bool mLoadIfExists;
   static const std::string mExpectedFileHeader;
   static const std::string mOldFileHeader;
};

class Covariance : public AlgorithmPlugIn
{
public:
   Covariance();
   virtual ~Covariance();

   bool hasAbort();

private:
   Covariance(const Covariance& rhs);
   bool canRunBatch() const;
   bool canRunInteractive() const;
   bool populateBatchInputArgList(PlugInArgList* pArgList);
   bool populateInteractiveInputArgList(PlugInArgList* pArgList);
   bool populateDefaultOutputArgList(PlugInArgList* pArgList);
   bool parseInputArgList(PlugInArgList* pArgList);
   bool setActualValuesInOutputArgList(PlugInArgList* pArgList);
   QDialog* getGui(void* pAlgData);
   void propagateAbort();
   bool extractFromGui();

   CovarianceAlgorithm* mpCovarianceAlg;
   CovarianceGui* mpCovarianceGui;
   Input mInput;
   RasterElement* mpRasterElement;
   bool mLoadIfExists;
};

#endif
