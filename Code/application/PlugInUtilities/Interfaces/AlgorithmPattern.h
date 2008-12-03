/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ALGORITHMPATTERN_H
#define ALGORITHMPATTERN_H

#include "AlgorithmShell.h"
#include "ApplicationServices.h"
#include "DesktopServices.h"
#include "Location.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "Resource.h"
#include "TypesFile.h"
#include "UtilityServices.h"

class BitMask;
class QDialog;
class RasterElement;
class SpatialDataView;
class Step;

#include <boost/any.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>

class AlgorithmReporter
{
public:
   AlgorithmReporter(Progress* pProgress);
   ~AlgorithmReporter();

   void reportProgress(ReportingLevel rptLevel, int progress, std::string message) const;
   int getProgressPercent() const;
   void addStage(std::string stageMessage, int weight);
   void nextStage();
   void clearStages();

protected:
   Progress* getProgress() const;
   Step* mpStep;

private:
   AlgorithmReporter(const AlgorithmReporter& reporter) {}

   virtual void updateProgress(ReportingLevel rptLevel, int progress, std::string message) const;
   Progress* mpProgress;
   class Stage
   {
   public:
      std::string mMessage;
      int mWeight;
      int mWeightSum;
      Stage(std::string message, int weight, int weightSum = 0);
      Stage(const Stage& stage);
   };
   std::vector<Stage> mStages;
   size_t mCurrentStage;
   int mWeightSum;
};

class AlgorithmPattern : public AlgorithmReporter
{
public:
   AlgorithmPattern(RasterElement* pRasterElement, Progress* pProgress, bool interactive, const BitMask* pRoi);
   virtual ~AlgorithmPattern();

   bool runPreprocess();
   bool runProcess();
   bool runPostprocess();
   bool hasAbort() const;
   bool abort();

   void elementDeleted(Subject& subject, const std::string& signal, const boost::any& v);

   RasterElement* getRasterElement() const;
   int getRowOffset() const;
   bool setGuiData(void* pAlgorithmData);
   bool isInteractive() const;

protected:
   bool determinePixelsToProcess() const;
   const BitMask* getPixelsToProcess() const;
   void setRoi(const BitMask* pRoi);
   void displayThresholdResults(RasterElement* pRasterElement, ColorType color, PassArea passArea,
      double firstThreshold, double secondThreshold, Opticks::PixelOffset offset = Opticks::PixelOffset());
   void displayPseudocolorResults(RasterElement* pRasterElement, std::vector<std::string>& sigNames,
      Opticks::PixelOffset offset = Opticks::PixelOffset());
   template <class ValueRange>
   void mergePseudocolorResults(RasterElement* pRasterElement, float* pCurrentData, int id, ValueRange range)
   {
      float* pPseudocolorData = static_cast<float*>(pRasterElement->getData());
      int numRows = pMatrix->getNumRows();
      int numColumns = pMatrix->getNumColumns();
      if (pPseudocolorData != NULL)
      {
         for (int j = 0; j < numRows; j++)
         {
            for (int k = 0; k < numColumns; k++)
            {
               if (range.contains(pCurrentData[(j * numColumns) + k]))
               {
                  if (pPseudocolorData[(j * numColumns) + k] == 0.0f)
                  {
                     pPseudocolorData[(j * numColumns) + k] = id;
                  }
                  else
                  {
                     pPseudocolorData[(j * numColumns) + k] = -1;
                  }
               }
            }
         }

         pMatrix->setData(pPseudocolorData);
      }
   }

   Service<ApplicationServices> mpApplicationServices;
   Service<DesktopServices> mpDesktopServices;
   Service<PlugInManagerServices> mpPlugInManagerServices;
   Service<ModelServices> mpModelServices;
   Service<UtilityServices> mpUtilityServices;
   ObjectFactory* mpObjFact;

private:
   virtual bool preprocess() = 0;
   virtual bool processAll() = 0;
   virtual bool postprocess() = 0;
   virtual bool initialize(void* pAlgorithmData) = 0;
   virtual bool canAbort() const = 0;
   virtual bool doAbort() = 0;

   int getSubCube(int startRow, int numRows);

   RasterElement* mpRasterElement;
   RasterElement* mpSubCube;
   const BitMask* mpRoi;
   mutable BitMask* mpPixelsToProcess;
   int mRowOffset;
   bool mInteractive;
};

class AlgorithmRunner
{
public:
   bool runAlgorithmFromGuiInputs();

private:
   virtual bool extractFromGui() = 0; // populate mpAlgorithmData from Gui. do-nothing if non-modal
   virtual bool setupAndRunAlgorithm() = 0;
   virtual bool needToRunAlgorithm()
   {
      return true;
   }
};

class AlgorithmPlugIn : public AlgorithmShell, public AlgorithmRunner
{
public:
   // Inherited obligations
   ~AlgorithmPlugIn();// will delete mpAlgorithm
   bool setBatch();
   bool setInteractive();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   AlgorithmPlugIn(void* pAlgData); // derived class ctor must call setAlgorithmPattern
   void setAlgorithmPattern(Resource<AlgorithmPattern> pAlgorithm);
   bool isInteractive() const;
   bool setupAndRunAlgorithm();

   Service<ApplicationServices> mpApplicationServices;
   Service<DesktopServices> mpDesktopServices;
   Service<PlugInManagerServices> mpPlugInManagerServices;
   Service<ModelServices> mpModelServices;
   Service<UtilityServices> mpUtilityServices;
   ObjectFactory* mpObjFact;

private:
   virtual bool canRunBatch() const = 0;
   virtual bool canRunInteractive() const = 0;
   virtual bool populateBatchInputArgList(PlugInArgList*) = 0;
   virtual bool populateInteractiveInputArgList(PlugInArgList*) = 0;
   virtual bool populateDefaultOutputArgList(PlugInArgList*) = 0;
   virtual bool setActualValuesInOutputArgList(PlugInArgList*) = 0;
   virtual bool parseInputArgList(PlugInArgList*) = 0; // populate mpAlgorithmData from argList
   virtual void propagateAbort() = 0;
   virtual QDialog* getGui(void* pAlgorithmData) = 0; // return NULL if no gui
   bool runDialog(QDialog* pDialog);

   Resource<AlgorithmPattern> mpAlgorithm; // owned by this class
   bool mInteractive;
   void* mpAlgorithmData;
};

#endif
