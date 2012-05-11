/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STATISTICSIMP_H
#define STATISTICSIMP_H

#include "AoiElement.h"
#include "BitMask.h"
#include "ComplexData.h"
#include "DimensionDescriptor.h"
#include "MultiThreadedAlgorithm.h"
#include "ObjectResource.h"
#include "SafePtr.h"
#include "Statistics.h"

#include <map>
#include <vector>

class RasterElement;
class RasterElementImp;

class StatisticsImp : public Statistics
{
public:
   StatisticsImp(const RasterElementImp* pRasterElement, DimensionDescriptor band, AoiElement* pAoi = NULL);
   StatisticsImp(const RasterElementImp* pRasterElement,
                 const std::vector<DimensionDescriptor>& bands,
                 AoiElement* pAoi = NULL);
   ~StatisticsImp();

   void setMin(double dMin);
   void setMin(double dMin, ComplexComponent component);
   double getMin();
   double getMin(ComplexComponent component);

   void setMax(double dMax);
   void setMax(double dMax, ComplexComponent component);
   double getMax();
   double getMax(ComplexComponent component);

   void setAverage(double dAverage);
   void setAverage(double dAverage, ComplexComponent component);
   double getAverage();
   double getAverage(ComplexComponent component);

   void setStandardDeviation(double dStdDev);
   void setStandardDeviation(double dStdDev, ComplexComponent component);
   double getStandardDeviation();
   double getStandardDeviation(ComplexComponent component);

   void setPercentiles(const double* pPercentiles);
   void setPercentiles(const double* pPercentiles, ComplexComponent component);
   const double* getPercentiles();
   const double* getPercentiles(ComplexComponent component);

   void setHistogram(const double* pBinCenters, const unsigned int* pHistogramCounts);
   void setHistogram(const double* pBinCenters, const unsigned int* pHistogramCounts, ComplexComponent component);
   void getHistogram(const double*& pBinCenters, const unsigned int*& pHistogramCounts);
   void getHistogram(const double*& pBinCenters, const unsigned int*& pHistogramCounts,
      ComplexComponent component);

   void setStatisticsResolution(int resolution);
   int getStatisticsResolution() const;

   void setBadValues(const std::vector<int>& badValues);
   const std::vector<int>& getBadValues() const;

   bool areStatisticsCalculated() const;
   bool areStatisticsCalculated(ComplexComponent component) const;
   void reset(ComplexComponent component);
   void resetAll();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected:
   void calculateStatistics(ComplexComponent component);

private:
   StatisticsImp(const StatisticsImp& rhs);
   StatisticsImp& operator=(const StatisticsImp& rhs);

   // NOTE: this has to be a RasterElementImp instead of RasterElement as it is populated
   // in the RasterElementImp constructor. At that point, a dynamic_cast to RasterElement
   // is not possible.
   const RasterElementImp* mpRasterElement;
   std::vector<DimensionDescriptor> mBands;
   FactoryResource<BitMask> mpAoi;
   SafePtr<AoiElement> mpOriginalAoi; // a refresh will update mpAoi if this still exists

   std::map<ComplexComponent, double> mMinValues;
   std::map<ComplexComponent, double> mMaxValues;
   std::map<ComplexComponent, double> mAverageValues;
   std::map<ComplexComponent, double> mStandardDeviationValues;
   std::map<ComplexComponent, std::vector<double> > mPercentileValues;
   std::map<ComplexComponent, std::vector<double> > mBinCenterValues;
   std::map<ComplexComponent, std::vector<unsigned int> > mHistogramValues;

   int mStatisticsResolution;
   std::vector<int> mBadValues;
};

class StatisticsInput
{
public:
   StatisticsInput(const std::vector<DimensionDescriptor>& bandsToCalculate, const RasterElement* pRaster,
                   ComplexComponent component, int resolution = 1,
                   const std::vector<int>& badValues = std::vector<int>(),
                   const BitMask* pAoi = NULL) :
      mBandsToCalculate(bandsToCalculate),
      mpRasterElement(pRaster),
      mComplexComponent(component),
      mResolution(resolution),
      mBadValues(badValues),
      mpAoi(pAoi)
   {
   }

   const std::vector<DimensionDescriptor>& mBandsToCalculate;
   const RasterElement* mpRasterElement;
   ComplexComponent mComplexComponent;
   int mResolution;
   std::vector<int> mBadValues;
   const BitMask* mpAoi;

private:
   StatisticsInput& operator=(const StatisticsInput& rhs);
};

class StatisticsThread;
class StatisticsOutput
{
public:
   StatisticsOutput();

   bool mMaxMinSet;
   double mMaximum;
   double mMinimum;
   double mAverage;
   double mStandardDeviation;
   bool compileOverallResults(const std::vector<StatisticsThread*>& threads);
};

class StatisticsThread : public mta::AlgorithmThread
{
public:
   StatisticsThread(const StatisticsInput &input, int threadCount, int threadIndex, mta::ThreadReporter &reporter);
   virtual ~StatisticsThread() {};

   virtual void run();

   bool isMaxMinSet() const;
   double getMaximum() const;
   double getMinimum() const;
   double getSum() const;
   double getSumSquared() const;
   unsigned int getCount() const;

private:
   StatisticsThread& operator=(const StatisticsThread& rhs);

   const StatisticsInput& mInput;

   Range mRowRange;
   bool mMaxMinSet;
   double mMaximum;
   double mMinimum;
   double mSum;
   double mSumSquared;
   unsigned int mCount;
};

class HistogramInput
{
public:
   HistogramInput(const StatisticsInput& statInput, const StatisticsOutput& statOutput) :
      mStatInput(statInput), mStatistics(statOutput) {}
   StatisticsInput mStatInput;
   StatisticsOutput mStatistics;

private:
   HistogramInput& operator=(const HistogramInput& rhs);
};

class HistogramThread;
const int HISTOGRAM_SIZE = 128 * 1024;
class HistogramOutput
{
public:
   HistogramOutput(bool isInteger, double maximum, double minimum) : 
      mIsInteger(isInteger), mMaximum(maximum), mMinimum(minimum) {}

   bool compileOverallResults(const std::vector<HistogramThread*>& threads);
   const double* getBinCenters() const;
   const unsigned int* getBinCounts() const;
   const double* getPercentiles() const;

private:
   void sumAllThreads(const std::vector<HistogramThread*>& threads, std::vector<unsigned int>& totalBinCounts);
   void computeBinCenters();
   void computeResultHistogram(const std::vector<unsigned int>& totalHistogram);
   void computePercentiles(const std::vector<unsigned int>& totalHistogram);

   double mBinCenters[256];
   unsigned int mBinCounts[256];
   double mPercentiles[1001];
   bool mIsInteger;
   double mMaximum;
   double mMinimum;
};

class HistogramThread : public mta::AlgorithmThread
{
public:
   HistogramThread(const HistogramInput& input, int threadCount, int threadIndex, mta::ThreadReporter& reporter);
   virtual ~HistogramThread() {};

   virtual void run();

   std::vector<unsigned int>& getBinCounts();

private:
   HistogramThread& operator=(const HistogramThread& rhs);

   const HistogramInput& mInput;
   unsigned int mCount;

   Range mRowRange;
   std::vector<unsigned int> mBinCounts;
};

#endif
