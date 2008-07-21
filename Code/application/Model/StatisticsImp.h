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

#include "Statistics.h"
#include "ComplexData.h"
#include "DimensionDescriptor.h"
#include "MultiThreadedAlgorithm.h"

#include <map>
#include <vector>

class RasterElementImp;

class StatisticsImp : public Statistics
{
public:
   StatisticsImp(const RasterElementImp* pSensorData, DimensionDescriptor band);
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
   const RasterElementImp* mpRasterElement;
   DimensionDescriptor mBand;

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
   StatisticsInput(const mta::Cube& cube, int bandToCalculate, const RasterElementImp* pRaster, ComplexComponent component,
      int resolution=1, const std::vector<int> &badValues=std::vector<int>()) :
      mCube(cube), mBandToCalculate(bandToCalculate), mpRasterElement(pRaster), mComplexComponent(component),
      mResolution(resolution), mBadValues(badValues) {}
   mta::Cube mCube;
   int mBandToCalculate;
   const RasterElementImp* mpRasterElement;
   ComplexComponent mComplexComponent;
   int mResolution;
   std::vector<int> mBadValues;
};

class StatisticsThread;
class StatisticsOutput
{
public:
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

   void run();

   const mta::Cube& getCube() const { return mCube; }
   double getMaximum() const { return mMaximum; }
   double getMinimum() const { return mMinimum; }
   double getSum() const { return mSum; }
   double getSumSquared() const { return mSumSquared; }
   int getResolution() const { return mResolution; }
   unsigned int getCount() const { return mCount; }

protected:
   template<class T> void calculateStatistics(T* pData, ComplexComponent component);

private:
   mta::Cube mCube;
   int mBandToCalculate;
   ComplexComponent mComplexComponent;
   const RasterElementImp* mpRasterElement;
   int mResolution;

   Range mRowRange;
   double mMaximum;
   double mMinimum;
   double mSum;
   double mSumSquared;
   unsigned int mCount;
   std::vector<int> mBadValues;
};

class HistogramInput
{
public:
   HistogramInput(const StatisticsInput& statInput, const StatisticsOutput& statOutput) :
      mStatInput(statInput), mStatistics(statOutput) {}
   StatisticsInput mStatInput;
   StatisticsOutput mStatistics;
};

class HistogramThread;
const int HISTOGRAM_SIZE = 128 * 1024;
class HistogramOutput
{
public:
   HistogramOutput(bool isInteger, double maximum, double minimum) : 
      mIsInteger(isInteger), mMaximum(maximum), mMinimum(minimum) {}

   bool compileOverallResults(const std::vector<HistogramThread*>& threads);
   const double *getBinCenters() const { return mBinCenters; }
   const unsigned int *getBinCounts() const { return mBinCounts; }
   const double *getPercentiles() const { return mPercentiles; }

private:
   void sumAllThreads(const std::vector<HistogramThread*>& threads, std::vector<unsigned int>& totalHistogram);
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
   HistogramThread(const HistogramInput &input, int threadCount, int threadIndex, mta::ThreadReporter &reporter);

   void run();

   const mta::Cube& getCube() const { return mCube; }
   std::vector<unsigned int> &getBinCounts() { return mBinCounts; }
   double getMaximum() const { return mMaximum; }
   double getMinimum() const { return mMinimum; }

protected:
   template<class T> void calculateHistogram(T* pData, ComplexComponent component);

private:
   mta::Cube mCube;
   int mBandToCalculate;
   ComplexComponent mComplexComponent;
   const RasterElementImp* mpRasterElement;
   int mResolution;
   unsigned int mCount;

   double mMinimum;
   double mMaximum;
   Range mRowRange;
   std::vector<unsigned int> mBinCounts;
   std::vector<int> mBadValues;
};

#endif
