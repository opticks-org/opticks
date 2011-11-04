/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SECONDMOMENT_H
#define SECONDMOMENT_H

#include "AlgorithmShell.h"
#include "AlgorithmPattern.h"
#include "RasterElement.h"

#include <math.h>
#include <string>
#include <vector>

class AoiElement;
class Filename;
class SecondMomentGui;
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

class SecondMomentAlgorithm : public AlgorithmPattern
{
public:
   SecondMomentAlgorithm(RasterElement* pRasterElement, Progress* pProgress, bool interactive);
   void setFile(const Filename* pSmmFile, bool loadIfExists);
   std::string getFilename() const;
   RasterElement* getSecondMomentElement() const;
   RasterElement* getInverseSecondMomentElement() const;

private:
   SecondMomentAlgorithm(const SecondMomentAlgorithm& rhs);

   bool preprocess();                     // from AlgorithmPattern. Does nothing.
   bool processAll();                     // from AlgorithmPattern. Controls computation of SMM.
   bool postprocess();                    // from AlgorithmPattern. Does nothing.
   bool initialize(void* pAlgorithmData); // from AlgorithmPattern. Sets the private data from the gui or arg-list
   bool canAbort() const;                 // from AlgorithmPattern. Returns true.
   bool doAbort();                        // from AlgorithmPattern. Aborts computation of the SMM.
   bool readMatrixFromDisk(std::string filename, RasterElement* pElement) const;
   bool writeMatrixToDisk(std::string filename, const RasterElement* pElement) const;

   Input mInput;
   std::string mSmmFile;
   RasterElement* mpNewRasterElement;
   RasterElement* mpNewInvRasterElement;
   bool mAbortFlag;
   bool mLoadIfExists;
   static const std::string mExpectedFileHeader;
};

class SecondMoment : public AlgorithmPlugIn
{
public:
   SecondMoment();
   ~SecondMoment();

   bool hasAbort();

private:
   SecondMoment(const SecondMoment& rhs);

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

   SecondMomentAlgorithm* mpSecondMomentAlg;
   SecondMomentGui* mpSecondMomentGui;
   Input mInput;
   RasterElement* mpRasterElement;
   bool mLoadIfExists;
};

#endif
