/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVOLUTIONFILTERSHELL_H
#define CONVOLUTIONFILTERSHELL_H

#include "AlgorithmShell.h"
#include "MultiThreadedAlgorithm.h"
#include "ProgressTracker.h"

#include <ossim/matrix/newmat.h>

class AoiElement;
class BitMaskIterator;
class RasterDataDescriptor;
class RasterElement;

class ConvolutionFilterShell : public AlgorithmShell
{
public:
   ConvolutionFilterShell();
   virtual ~ConvolutionFilterShell();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   /**
    * Populate the kernel.
    *
    * This method should set mKernel to the convolution kernel.
    *
    * @return false on error, true on success
    */
   virtual bool populateKernel() = 0;
   virtual SpatialDataView* displayResult();

   struct ConvolutionFilterThreadInput
   {
      ConvolutionFilterThreadInput() :
            mpRaster(NULL),
            mpDescriptor(NULL),
            mpResult(NULL),
            mpAbortFlag(NULL),
            mpIterCheck(NULL)
      {}

      const RasterElement* mpRaster;
      const RasterDataDescriptor* mpDescriptor;
      RasterElement* mpResult;
      const bool* mpAbortFlag;
      const BitMaskIterator* mpIterCheck;
      std::vector<unsigned int> mBands;
      NEWMAT::Matrix mKernel;
   };

   ProgressTracker mProgress;
   ConvolutionFilterThreadInput mInput;
   AoiElement* mpAoi;
   std::string mResultName;

   class ConvolutionFilterThread : public mta::AlgorithmThread
   {
   public:
      ConvolutionFilterThread(const ConvolutionFilterThreadInput& input,
         int threadCount, int threadIndex, mta::ThreadReporter& reporter);
      virtual ~ConvolutionFilterThread() {}
      void run();

   private:
      template<typename T> void convolve(const T*);
      const ConvolutionFilterThreadInput& mInput;
      mta::AlgorithmThread::Range mRowRange;
   };

   struct ConvolutionFilterThreadOutput
   {
      bool compileOverallResults(const std::vector<ConvolutionFilterThread*>& threads);
   };
};

class GenericConvolution : public ConvolutionFilterShell
{
public:
   GenericConvolution();
   virtual ~GenericConvolution();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   virtual bool populateKernel();
};

#endif
