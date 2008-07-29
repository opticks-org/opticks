/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "StatisticsImp.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "MathUtil.h"
#include "ModelServices.h"
#include "RasterElementImp.h"
#include "RasterDataDescriptor.h"
#include "switchOnEncoding.h"
#include "UtilityServicesImp.h"

#include <algorithm>
using namespace std;
using namespace mta;
XERCES_CPP_NAMESPACE_USE

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

StatisticsImp::StatisticsImp(const RasterElementImp* pRasterElement, DimensionDescriptor band) :
   mpRasterElement(pRasterElement),
   mBand(band),
   mStatisticsResolution(0)
{
}

StatisticsImp::~StatisticsImp()
{
}

void StatisticsImp::setMin(double dMin)
{
   setMin(dMin, COMPLEX_MAGNITUDE);
}

void StatisticsImp::setMin(double dMin, ComplexComponent component)
{
   if (dMin == -99999.9)
   {
      mMinValues.erase(component);
      return;
   }

   mMinValues[component] = dMin;
}

double StatisticsImp::getMin()
{
   return getMin(COMPLEX_MAGNITUDE);
}

double StatisticsImp::getMin(ComplexComponent component)
{
   map<ComplexComponent, double>::iterator iter;
   iter = mMinValues.find(component);
   if (iter == mMinValues.end())
   {
      calculateStatistics(component);

      iter = mMinValues.find(component);
      if (iter == mMinValues.end())
      {
         return -99999.9;
      }
   }

   return mMinValues[component];
}

void StatisticsImp::setMax(double dMax)
{
   setMax(dMax, COMPLEX_MAGNITUDE);
}

void StatisticsImp::setMax(double dMax, ComplexComponent component)
{
   if (dMax == -99999.9)
   {
      mMaxValues.erase(component);
      return;
   }

   mMaxValues[component] = dMax;
}

double StatisticsImp::getMax()
{
   return getMax(COMPLEX_MAGNITUDE);
}

double StatisticsImp::getMax(ComplexComponent component)
{
   map<ComplexComponent, double>::iterator iter;
   iter = mMaxValues.find(component);
   if (iter == mMaxValues.end())
   {
      calculateStatistics(component);

      iter = mMaxValues.find(component);
      if (iter == mMaxValues.end())
      {
         return -99999.9;
      }
   }

   return mMaxValues[component];
}

void StatisticsImp::setAverage(double dAverage)
{
   setAverage(dAverage, COMPLEX_MAGNITUDE);
}

void StatisticsImp::setAverage(double dAverage, ComplexComponent component)
{
   if (dAverage == -99999.9)
   {
      mAverageValues.erase(component);
      return;
   }

   mAverageValues[component] = dAverage;
}

double StatisticsImp::getAverage()
{
   return getAverage(COMPLEX_MAGNITUDE);
}

double StatisticsImp::getAverage(ComplexComponent component)
{
   map<ComplexComponent, double>::iterator iter;
   iter = mAverageValues.find(component);
   if (iter == mAverageValues.end())
   {
      calculateStatistics(component);

      iter = mAverageValues.find(component);
      if (iter == mAverageValues.end())
      {
         return -99999.9;
      }
   }

   return mAverageValues[component];
}

void StatisticsImp::setStandardDeviation(double dStdDev)
{
   setStandardDeviation(dStdDev, COMPLEX_MAGNITUDE);
}

void StatisticsImp::setStandardDeviation(double dStdDev, ComplexComponent component)
{
   if (dStdDev == -99999.9)
   {
      mStandardDeviationValues.erase(component);
      return;
   }

   mStandardDeviationValues[component] = dStdDev;
}

double StatisticsImp::getStandardDeviation()
{
   return getStandardDeviation(COMPLEX_MAGNITUDE);
}

double StatisticsImp::getStandardDeviation(ComplexComponent component)
{
   map<ComplexComponent, double>::iterator iter;
   iter = mStandardDeviationValues.find(component);
   if (iter == mStandardDeviationValues.end())
   {
      calculateStatistics(component);

      iter = mStandardDeviationValues.find(component);
      if (iter == mStandardDeviationValues.end())
      {
         return -99999.9;
      }
   }

   return mStandardDeviationValues[component];
}

void StatisticsImp::setPercentiles(const double* pPercentiles)
{
   setPercentiles(pPercentiles, COMPLEX_MAGNITUDE);
}

void StatisticsImp::setPercentiles(const double* pPercentiles, ComplexComponent component)
{
   if (pPercentiles == NULL)
   {
      mPercentileValues.erase(component);
      return;
   }

   vector<double> percentileValues;
   for (int i = 0; i < 1001; i++)
   {
      percentileValues.push_back(pPercentiles[i]);
   }

   mPercentileValues[component] = percentileValues;
}

const double* StatisticsImp::getPercentiles()
{
   return getPercentiles(COMPLEX_MAGNITUDE);
}

const double* StatisticsImp::getPercentiles(ComplexComponent component)
{
   map<ComplexComponent, vector<double> >::iterator iter;
   iter = mPercentileValues.find(component);
   if (iter == mPercentileValues.end())
   {
      calculateStatistics(component);

      iter = mPercentileValues.find(component);
      if (iter == mPercentileValues.end())
      {
         return NULL;
      }
   }

   vector<double>& percentileValues = mPercentileValues[component];
   return &percentileValues[0];
}

void StatisticsImp::setHistogram(const double* pBinCenters, const unsigned int* pHistogramCounts)
{
   setHistogram(pBinCenters, pHistogramCounts, COMPLEX_MAGNITUDE);
}

void StatisticsImp::setHistogram(const double* pBinCenters, const unsigned int* pHistogramCounts,
                                ComplexComponent component)
{
   if ((pBinCenters == NULL) || (pHistogramCounts == NULL))
   {
      mBinCenterValues.erase(component);
      mHistogramValues.erase(component);
      return;
   }

   vector<double> binCenterValues;
   vector<unsigned int> histogramValues;
   for (int i = 0; i < 256; i++)
   {
      binCenterValues.push_back(pBinCenters[i]);
      histogramValues.push_back(pHistogramCounts[i]);
   }

   mBinCenterValues[component] = binCenterValues;
   mHistogramValues[component] = histogramValues;
}

void StatisticsImp::getHistogram(const double*& pBinCenters, const unsigned int*& pHistogramCounts)
{
   getHistogram(pBinCenters, pHistogramCounts, COMPLEX_MAGNITUDE);
}

void StatisticsImp::getHistogram(const double*& pBinCenters, const unsigned int*& pHistogramCounts,
                                ComplexComponent component)
{
   pBinCenters = NULL;
   pHistogramCounts = NULL;

   // Bin centers
   map<ComplexComponent, vector<double> >::iterator dIter;
   dIter = mBinCenterValues.find(component);
   if (dIter == mBinCenterValues.end())
   {
      calculateStatistics(component);

      dIter = mBinCenterValues.find(component);
      if (dIter == mBinCenterValues.end())
      {
         return;
      }
   }

   // Counts
   map<ComplexComponent, vector<unsigned int> >::iterator uiIter;
   uiIter = mHistogramValues.find(component);
   if (uiIter == mHistogramValues.end())
   {
      calculateStatistics(component);

      uiIter = mHistogramValues.find(component);
      if (uiIter == mHistogramValues.end())
      {
         return;
      }
   }

   vector<double>& binCenterValues = mBinCenterValues[component];
   vector<unsigned int>& histogramValues = mHistogramValues[component];

   pBinCenters = &binCenterValues[0];
   pHistogramCounts = &histogramValues[0];
}

void StatisticsImp::setStatisticsResolution(int resolution)
{
   if (resolution != mStatisticsResolution && resolution > 0)
   {
      mStatisticsResolution = resolution;
      resetAll();
   }
}

int StatisticsImp::getStatisticsResolution() const
{
   return mStatisticsResolution;
}

void StatisticsImp::setBadValues(const std::vector<int>& badValues)
{
   mBadValues = badValues;
   std::sort(mBadValues.begin(), mBadValues.end());
   resetAll();
}

const vector<int>& StatisticsImp::getBadValues() const
{
   return mBadValues;
}

bool StatisticsImp::areStatisticsCalculated() const
{
   return areStatisticsCalculated(COMPLEX_MAGNITUDE);
}

bool StatisticsImp::areStatisticsCalculated(ComplexComponent component) const
{
   // Min
   map<ComplexComponent, double>::const_iterator iter;
   iter = mMinValues.find(component);
   if (iter == mMinValues.end())
   {
      return false;
   }

   // Max
   iter = mMaxValues.find(component);
   if (iter == mMaxValues.end())
   {
      return false;
   }

   // Average
   iter = mAverageValues.find(component);
   if (iter == mAverageValues.end())
   {
      return false;
   }

   // Standard deviation
   iter = mStandardDeviationValues.find(component);
   if (iter == mStandardDeviationValues.end())
   {
      return false;
   }

   // Percentiles
   map<ComplexComponent, vector<double> >::const_iterator dIter;
   dIter = mPercentileValues.find(component);
   if (dIter == mPercentileValues.end())
   {
      return false;
   }

   // Histogram
   dIter = mBinCenterValues.find(component);
   if (dIter == mBinCenterValues.end())
   {
      return false;
   }

   map<ComplexComponent, vector<unsigned int> >::const_iterator uiIter;
   uiIter = mHistogramValues.find(component);
   if (uiIter == mHistogramValues.end())
   {
      return false;
   }

   return true;
}

void StatisticsImp::reset(ComplexComponent component)
{
   mMinValues.erase(component);
   mMaxValues.erase(component);
   mAverageValues.erase(component);
   mStandardDeviationValues.erase(component);
   mPercentileValues.erase(component);
   mHistogramValues.erase(component);
   mBinCenterValues.erase(component);
}

void StatisticsImp::resetAll()
{
   mMinValues.clear();
   mMaxValues.clear();
   mAverageValues.clear();
   mStandardDeviationValues.clear();
   mPercentileValues.clear();
   mHistogramValues.clear();
   mBinCenterValues.clear();
}

bool StatisticsImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("resolution", mStatisticsResolution);
   for(map<ComplexComponent, double>::const_iterator it = mMinValues.begin(); it != mMinValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("minimum"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for(map<ComplexComponent, double>::const_iterator it = mMaxValues.begin(); it != mMaxValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("maximum"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for(map<ComplexComponent, double>::const_iterator it = mAverageValues.begin(); it != mAverageValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("average"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for(map<ComplexComponent, double>::const_iterator it = mStandardDeviationValues.begin(); it != mStandardDeviationValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("stddev"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for(map<ComplexComponent, std::vector<double> >::const_iterator it = mPercentileValues.begin(); it != mPercentileValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("percentile"));
      pXml->addAttr("component", it->first);
      pXml->addText(it->second);
      pXml->popAddPoint();
   }
   for(map<ComplexComponent, std::vector<double> >::const_iterator it = mBinCenterValues.begin(); it != mBinCenterValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("center"));
      pXml->addAttr("component", it->first);
      pXml->addText(it->second);
      pXml->popAddPoint();
   }
   for(map<ComplexComponent, std::vector<unsigned int> >::const_iterator it = mHistogramValues.begin(); it != mHistogramValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("histogram"));
      pXml->addAttr("component", it->first);
      pXml->addText(it->second);
      pXml->popAddPoint();
   }
   pXml->addText(mBadValues, pXml->addElement("badValues"));
   return true;
}

bool StatisticsImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   mStatisticsResolution = StringUtilities::fromXmlString<int>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("resolution"))));
   mMinValues.clear();
   mMaxValues.clear();
   mAverageValues.clear();
   mStandardDeviationValues.clear();
   mPercentileValues.clear();
   mBinCenterValues.clear();
   mHistogramValues.clear();
   for(DOMNode *pNode = pDocument->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if(XMLString::equals(pNode->getNodeName(), X("minimum")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mMinValues[component] = value;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("maximum")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mMaxValues[component] = value;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("average")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mAverageValues[component] = value;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("stddev")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mStandardDeviationValues[component] = value;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("percentile")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         vector<double> values;
         XmlReader::StrToVector<double,XmlReader::StringStreamAssigner<double> >(values, pElement->getTextContent());
         mPercentileValues[component] = values;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("center")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         vector<double> values;
         XmlReader::StrToVector<double,XmlReader::StringStreamAssigner<double> >(values, pElement->getTextContent());
         mBinCenterValues[component] = values;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("histogram")))
      {
         DOMElement *pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         vector<unsigned int> values;
         XmlReader::StrToVector<unsigned int,XmlReader::StringStreamAssigner<unsigned int> >(values, pElement->getTextContent());
         mHistogramValues[component] = values;
      }
      else if(XMLString::equals(pNode->getNodeName(), X("badValues")))
      {
         mBadValues.clear();
         XmlReader::StrToVector<int,XmlReader::StringStreamAssigner<int> >(mBadValues, pNode->getTextContent());
      }
   }
   return true;
}

void StatisticsImp::calculateStatistics(ComplexComponent component)
{
   reset(component);

   if (mpRasterElement == NULL)
   {
      return;
   }

   const RasterDataDescriptor* pDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(mpRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   EncodingType eType = pDescriptor->getDataType();

   int rowNum = pDescriptor->getRowCount();
   int colNum = pDescriptor->getColumnCount();
   int bandNum = pDescriptor->getBandCount();

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This is deprecated and should be changed (tclarke)")
   mta::Cube cube((void*)NULL, eType, rowNum, colNum, bandNum);

   if (mStatisticsResolution == 0)
   {
      if (rowNum < colNum)
      {
         mStatisticsResolution = rowNum/500;
      }
      else
      {
         mStatisticsResolution = colNum/500;
      }

      if (mStatisticsResolution < 1)
      {
         mStatisticsResolution = 1;
      }
   }

   unsigned int originalNumber = 0;
   unsigned int activeNumber = 0;
   if (mBand.isValid())
   {
      originalNumber = mBand.getOriginalNumber();
      activeNumber = mBand.getActiveNumber();
   }

   StatisticsInput statInput(cube, activeNumber, mpRasterElement, component, mStatisticsResolution, mBadValues);
   StatisticsOutput statOutput;

   char buffer[256];
   sprintf(buffer, "Computing statistics of band %d", originalNumber + 1);
   mta::StatusBarReporter barReporter(buffer, "app", "CF884AA2-A1BF-468d-9609-795DE0F7B7A4");

   vector<int> phaseWeights;
   phaseWeights.push_back(20);
   phaseWeights.push_back(80);
   mta::MultiPhaseProgressReporter progressReporter(barReporter, phaseWeights);

   unsigned int threadCount = ConfigurationSettings::getSettingThreadCount();
   // if there are more threads than rows in the data set, we need to clamp
   // the number of threads so we don't have idle threads.
   if(pDescriptor != NULL)
   {
      threadCount = std::min(threadCount, pDescriptor->getRowCount());
   }

   mta::MultiThreadedAlgorithm<StatisticsInput,StatisticsOutput,StatisticsThread> statisticsAlgorithm
      (threadCount, statInput, statOutput, &progressReporter);
   statisticsAlgorithm.run();

   bool bInteger = true;
   if ((eType == FLT4BYTES) || (eType == FLT8COMPLEX) || (eType == FLT8BYTES) ||
      ((eType == INT4SCOMPLEX) && (component == COMPLEX_MAGNITUDE)) ||
      ((eType == INT4SCOMPLEX) && (component == COMPLEX_PHASE)))
   {
      bInteger = false;
   }

   HistogramInput histInput(statInput, statOutput);
   HistogramOutput histOutput(bInteger, statOutput.mMaximum, statOutput.mMinimum);

   progressReporter.setCurrentPhase(1);

   mta::MultiThreadedAlgorithm<HistogramInput, HistogramOutput, HistogramThread> histogramAlgorithm
      (threadCount, histInput, histOutput, &progressReporter);

   if (histogramAlgorithm.run() == mta::SUCCESS)
   {
      setMin(statOutput.mMinimum, component);
      setMax(statOutput.mMaximum, component);
      setAverage(statOutput.mAverage, component);
      setStandardDeviation(statOutput.mStandardDeviation, component);
      setPercentiles(histOutput.getPercentiles(), component);
      setHistogram(histOutput.getBinCenters(), histOutput.getBinCounts(), component);
   }
}

StatisticsThread::StatisticsThread(const StatisticsInput &input, int threadCount, int threadIndex,
   ThreadReporter &reporter) : AlgorithmThread(threadIndex, reporter), mCube(input.mCube),
   mBandToCalculate(input.mBandToCalculate), mpRasterElement(input.mpRasterElement), 
   mComplexComponent(input.mComplexComponent),  mResolution(input.mResolution),
   mRowRange(getThreadRange(threadCount, mCube.getRowCount())), mMaximum(-1e38), mMinimum(1e38), mSum(0.0),
   mSumSquared(0.0), mCount(0), mBadValues(input.mBadValues)
{
}

void StatisticsThread::run()
{
   switchOnComplexEncoding(mCube.getType(), StatisticsThread::calculateStatistics, mCube.getData(),
      mComplexComponent);
}

template<class T>
void StatisticsThread::calculateStatistics(T* pData, ComplexComponent component)
{
   int oldPercentDone = -1;

   unsigned int posY = mRowRange.mFirst;
   unsigned int posX = 0;
   unsigned int geomSizeY = mRowRange.mLast - mRowRange.mFirst + 1;
   unsigned int geomSizeX = mCube.getColumnCount();

   // TODO: Remove the const_cast when a const DataAccessor is available
   const RasterDataDescriptor *pDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      mpRasterElement->getDataDescriptor());
   VERIFYNRV(pDescriptor != NULL);

   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(pDescriptor->getActiveRow(posY), 
      pDescriptor->getActiveRow(posY+geomSizeY-1), 1);
   pRequest->setColumns(pDescriptor->getActiveColumn(posX), 
      pDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
   pRequest->setBands(pDescriptor->getActiveBand(mBandToCalculate),
      pDescriptor->getActiveBand(mBandToCalculate), 1);
   DataAccessor da = mpRasterElement->getDataAccessor(pRequest.release());
   if (!da.isValid())
   {
      return;
   }

   T* source = static_cast<T*>(da->getColumn());
   bool maxMinSet = false;

   mSum = 0.0;
   mSumSquared = 0.0;
   mCount = 0;

   unsigned int badValueCount = mBadValues.size();
   int firstBadValue = 0;
   if (badValueCount != 0)
   {
      firstBadValue = mBadValues.front();
   }
   std::vector<int>::iterator badBegin=mBadValues.begin(), badEnd=mBadValues.end();

   for (unsigned int y1 = 0; y1 < geomSizeY; y1 += mResolution)
   {
      int percentDone = mRowRange.computePercent(y1 + mRowRange.mFirst);
      if (percentDone >= oldPercentDone + 25)
      {
         oldPercentDone = percentDone;
         getReporter().reportProgress(getThreadIndex(), percentDone);
      }

      VERIFYNRV(da.isValid());
      for (unsigned int x1 = 0; x1<geomSizeX; x1+=mResolution)
      {
         source = static_cast<T*>(da->getColumn());
         double temp = ModelServices::getDataValue(*source, component);

         int tempInt = roundDouble(temp);
         if (badValueCount == 0 || (badValueCount == 1 && tempInt != firstBadValue) || 
            !std::binary_search(badBegin, badEnd, tempInt))
         {
            if (!maxMinSet)
            {
               mMinimum = mMaximum = temp;
               maxMinSet = true;
            }
            else
            {
               if (temp < mMinimum) { mMinimum = temp; }
               if (temp > mMaximum) { mMaximum = temp; }
            }

            mSumSquared += temp*temp;
            mSum += temp;
            mCount++;
         }

         da->nextColumn(mResolution);
      }

      da->nextRow(mResolution);
   }
   if (!maxMinSet)
   {
      mMinimum = mMaximum = 0.0;
   }
}

bool StatisticsOutput::compileOverallResults(const vector<StatisticsThread*>& threads)
{
   mMaximum = -1e38;
   mMinimum = 1e38;
   mAverage = 0.0;
   mStandardDeviation = 0.0;

   if (threads.size() == 0) { return false; }

   double totalSum = 0.0;
   double totalSquaredSum = 0.0;
   unsigned int pointCount = 0;

   vector<StatisticsThread*>::const_iterator iter;
   for (iter = threads.begin(); iter != threads.end(); ++iter)
   {
      StatisticsThread* pThread = NULL;
      pThread = *iter;
      if (pThread != NULL)
      {
         mMaximum = MAX(mMaximum, pThread->getMaximum());
         mMinimum = MIN(mMinimum, pThread->getMinimum());
         totalSum += pThread->getSum();
         totalSquaredSum += pThread->getSumSquared();
         pointCount += pThread->getCount();
      }
   }

   if (pointCount > 0)
   {
      mAverage = totalSum / pointCount;
   }

   if (pointCount > 1)
   {
      // the fabs() on the next line prevents roundoff error from giving sqrt a negative
      // when every pixel has the same value
      double numerator = fabs(pointCount * totalSquaredSum - totalSum * totalSum);
      mStandardDeviation = sqrt((numerator / pointCount) / (pointCount - 1));
   }

   return true;
}

HistogramThread::HistogramThread(const HistogramInput &input, int threadCount, int threadIndex,
   mta::ThreadReporter &reporter) :
   AlgorithmThread(threadIndex, reporter),
   mCube(input.mStatInput.mCube),
   mBandToCalculate(input.mStatInput.mBandToCalculate),
   mComplexComponent(input.mStatInput.mComplexComponent),
   mpRasterElement(input.mStatInput.mpRasterElement),
   mResolution(input.mStatInput.mResolution),
   mCount(0),
   mMinimum(input.mStatistics.mMinimum),
   mMaximum(input.mStatistics.mMaximum),
   mRowRange(getThreadRange(threadCount, mCube.getRowCount())),
   mBinCounts(HISTOGRAM_SIZE),
   mBadValues(input.mStatInput.mBadValues)
{
}

void HistogramThread::run()
{
   switchOnComplexEncoding(mCube.getType(), HistogramThread::calculateHistogram, mCube.getData(),
      mComplexComponent);
}

template<class T>
void HistogramThread::calculateHistogram(T* pData, ComplexComponent component)
{
   double range = 1.0;
   double toBin = 0.0;
   double minimum;

   if (getMaximum() != getMinimum())
   {
      range = getMaximum() - getMinimum();
      toBin = 0.999999999 * (HISTOGRAM_SIZE)/range;
   }

   std::vector<unsigned int> &binCounts = getBinCounts();

   int oldPercentDone = -1;

   unsigned int posY = mRowRange.mFirst;
   unsigned int posX = 0;
   unsigned int geomSizeY = mRowRange.mLast - mRowRange.mFirst + 1;
   unsigned int geomSizeX = mCube.getColumnCount();

   const RasterDataDescriptor *pDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      mpRasterElement->getDataDescriptor());
   VERIFYNRV(pDescriptor != NULL);

   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(pDescriptor->getActiveRow(posY), 
      pDescriptor->getActiveRow(posY+geomSizeY-1), 1);
   pRequest->setColumns(pDescriptor->getActiveColumn(posX), 
      pDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
   pRequest->setBands(pDescriptor->getActiveBand(mBandToCalculate),
      pDescriptor->getActiveBand(mBandToCalculate), 1);

   DataAccessor da = mpRasterElement->getDataAccessor(pRequest.release());
   if (!da.isValid())
   {
      return;
   }

   unsigned int badValueCount = mBadValues.size();
   int firstBadValue = 0;
   if (badValueCount != 0)
   {
      firstBadValue = mBadValues.front();
   }
   std::vector<int>::iterator badBegin=mBadValues.begin(), badEnd=mBadValues.end();

   minimum = getMinimum();
   for (unsigned int y1 = 0; y1 < geomSizeY; y1+=mResolution)
   {
      VERIFYNRV(da.isValid());
      int percentDone = mRowRange.computePercent(y1 + mRowRange.mFirst);
      if (percentDone >= oldPercentDone + 25)
      {
         oldPercentDone = percentDone;
         getReporter().reportProgress(getThreadIndex(), percentDone);
      }

      for (unsigned int x1 = 0; x1 < geomSizeX; x1 += mResolution)
      {
         T* source = static_cast<T*>(da->getColumn());

         double temp = ModelServices::getDataValue(*source, component);

         int tempInt = roundDouble(temp);
         if (badValueCount == 0 || (badValueCount == 1 && tempInt != firstBadValue) || 
            !std::binary_search(badBegin, badEnd, tempInt))
         {
            int bin = static_cast<int>((temp-minimum) * toBin);
            if (bin >= HISTOGRAM_SIZE)
            {
               bin = HISTOGRAM_SIZE - 1;
            }
            else if (bin < 0)
            {
               bin = 0;
            }

            binCounts[bin]++;
            mCount++;
         }

         da->nextColumn(mResolution);
      }

      da->nextRow(mResolution);
   }
}

bool HistogramOutput::compileOverallResults(const vector<HistogramThread*>& threads)
{
   vector<unsigned int> totalBinCounts(HISTOGRAM_SIZE);

   sumAllThreads(threads, totalBinCounts);
   computeBinCenters();
   computeResultHistogram(totalBinCounts);
   computePercentiles(totalBinCounts);

   return true;
}

void HistogramOutput::sumAllThreads(const vector<HistogramThread*>& threads,
                                    vector<unsigned int>& totalBinCounts)
{
   memset(&totalBinCounts[0], 0, HISTOGRAM_SIZE * sizeof(unsigned int));

   vector<HistogramThread*>::const_iterator iter;
   for (iter = threads.begin(); iter != threads.end(); ++iter)
   {
      HistogramThread* pThread = NULL;
      pThread = *iter;
      if (pThread != NULL)
      {
         const vector<unsigned int>& threadBinCounts = pThread->getBinCounts();

         transform(totalBinCounts.begin(), totalBinCounts.end(), 
            threadBinCounts.begin(), totalBinCounts.begin(), plus<unsigned int>());
      }
   }
}

void HistogramOutput::computeBinCenters()
{
   double width = 0.0;
   double range = mMaximum - mMinimum;
   bool oneBin = false;

   if (range == 0.0)
   {
      range = 1.0;
      oneBin = true;
   }

   if (mIsInteger)
   {
      double binCount = range + 0.5;
      width = ceil(binCount / 256.0);
   }
   else
   {
      width = range / 256.0;
   }

   int bin;
   for (bin = 0; bin < 256; ++bin)
   {
      if (mIsInteger)
      {
         mBinCenters[bin] = mMinimum + bin * width + (width - 1.0) / 2.0;
      }
      else if (oneBin)
      {
         mBinCenters[bin] = mMinimum + bin * width;
      }
      else
      {
         mBinCenters[bin] = mMinimum + bin * width + width / 2.0;
      }
   }
}

void HistogramOutput::computeResultHistogram(const vector<unsigned int>& totalHistogram)
{
   memset(mBinCounts, 0, 256 * sizeof(unsigned int));

   double overallRange = mMaximum - mMinimum;
   double resultRange = mBinCenters[255] - mBinCenters[0];

   if (mIsInteger == false)
   {
      resultRange += overallRange / 256.0;
   }

   double binConversion = (256.0 / resultRange) * (overallRange / HISTOGRAM_SIZE);

   int sourceBin = 0;
   int destBin = 0;

   for (sourceBin = 0; sourceBin < HISTOGRAM_SIZE; ++sourceBin)
   {
      destBin = static_cast<int>(sourceBin * binConversion);
      mBinCounts[destBin] += totalHistogram[sourceBin];
   }
}

void HistogramOutput::computePercentiles(const vector<unsigned int>& totalHistogram)
{
   int bin;
   int pointCount = accumulate(totalHistogram.begin(), totalHistogram.end(), 0);

   double range = mMaximum - mMinimum;
   int percentile;
   int count = 0;
   bin = -1;
   int prevPercentile = 0;
   bool hit = false;
   mPercentiles[0] = mMinimum;
   for (percentile = 1; percentile < 1001; ++percentile)
   {
      hit = false;
      int cutoff = (int) (0.001 * percentile * pointCount);
      while (count < cutoff && bin < HISTOGRAM_SIZE - 1)
      {
         bin++;
         count += totalHistogram[bin];
         hit = true;
      }
      mPercentiles[percentile] = range * bin / HISTOGRAM_SIZE + mMinimum;
      if (hit == true)
      {
         int j;
         for (j = prevPercentile + 1; j < percentile; ++j) // linearly interpolate over gaps
         {
            mPercentiles[j] = mPercentiles[prevPercentile] + 
               (mPercentiles[percentile] - mPercentiles[prevPercentile]) *
               (j - prevPercentile) / (percentile - prevPercentile);
         }

         prevPercentile = percentile;
      }
   }
}
