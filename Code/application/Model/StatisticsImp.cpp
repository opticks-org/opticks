/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AppVerify.h"
#include "BitMaskIterator.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "MathUtil.h"
#include "ModelServices.h"
#include "RasterElement.h"
#include "RasterElementImp.h"
#include "RasterDataDescriptor.h"
#include "StatisticsImp.h"
#include "switchOnEncoding.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <algorithm>
#include <limits>
using namespace mta;
XERCES_CPP_NAMESPACE_USE

StatisticsImp::StatisticsImp(const RasterElementImp* pRasterElement,
                             DimensionDescriptor band,
                             AoiElement* pAoi) :
   mpRasterElement(pRasterElement),
   mpAoi(NULL),
   mpOriginalAoi(pAoi),
   mStatisticsResolution(0)
{
   if (pAoi != NULL)
   {
      mpAoi = FactoryResource<BitMask>();
      const BitMask* pBitMask = pAoi->getSelectedPoints();
      ENSURE(pBitMask);
      mpAoi->merge(*(pBitMask));
   }
   mBands.push_back(band);
}

StatisticsImp::StatisticsImp(const RasterElementImp* pRasterElement,
                             const std::vector<DimensionDescriptor>& bands,
                             AoiElement* pAoi) :
   mpRasterElement(pRasterElement),
   mBands(bands),
   mpAoi(NULL),
   mpOriginalAoi(pAoi),
   mStatisticsResolution(0)
{
   if (pAoi != NULL)
   {
      mpAoi = FactoryResource<BitMask>();
      const BitMask* pBitMask = pAoi->getSelectedPoints();
      ENSURE(pBitMask);
      mpAoi->merge(*(pBitMask));
   }
}

StatisticsImp::~StatisticsImp()
{}

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
   std::map<ComplexComponent, double>::iterator iter;
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
   std::map<ComplexComponent, double>::iterator iter;
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
   std::map<ComplexComponent, double>::iterator iter;
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
   std::map<ComplexComponent, double>::iterator iter;
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

   std::vector<double> percentileValues;
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
   std::map<ComplexComponent, std::vector<double> >::iterator iter;
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

   std::vector<double>& percentileValues = mPercentileValues[component];
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

   std::vector<double> binCenterValues;
   std::vector<unsigned int> histogramValues;
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
   std::map<ComplexComponent, std::vector<double> >::iterator dIter;
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
   std::map<ComplexComponent, std::vector<unsigned int> >::iterator uiIter;
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

   std::vector<double>& binCenterValues = mBinCenterValues[component];
   std::vector<unsigned int>& histogramValues = mHistogramValues[component];

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

const std::vector<int>& StatisticsImp::getBadValues() const
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
   std::map<ComplexComponent, double>::const_iterator iter;
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
   std::map<ComplexComponent, std::vector<double> >::const_iterator dIter;
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

   std::map<ComplexComponent, std::vector<unsigned int> >::const_iterator uiIter;
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
   for (std::map<ComplexComponent, double>::const_iterator it = mMinValues.begin(); it != mMinValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("minimum"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for (std::map<ComplexComponent, double>::const_iterator it = mMaxValues.begin(); it != mMaxValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("maximum"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for (std::map<ComplexComponent, double>::const_iterator it = mAverageValues.begin(); it != mAverageValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("average"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for (std::map<ComplexComponent, double>::const_iterator it = mStandardDeviationValues.begin();
      it != mStandardDeviationValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("stddev"));
      pXml->addAttr("component", it->first);
      pXml->addAttr("value", it->second);
      pXml->popAddPoint();
   }
   for (std::map<ComplexComponent, std::vector<double> >::const_iterator it = mPercentileValues.begin();
      it != mPercentileValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("percentile"));
      pXml->addAttr("component", it->first);
      pXml->addText(it->second);
      pXml->popAddPoint();
   }
   for (std::map<ComplexComponent, std::vector<double> >::const_iterator it = mBinCenterValues.begin();
      it != mBinCenterValues.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("center"));
      pXml->addAttr("component", it->first);
      pXml->addText(it->second);
      pXml->popAddPoint();
   }
   for (std::map<ComplexComponent, std::vector<unsigned int> >::const_iterator it = mHistogramValues.begin();
      it != mHistogramValues.end(); ++it)
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
   for (DOMNode *pNode = pDocument->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("minimum")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mMinValues[component] = value;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("maximum")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mMaxValues[component] = value;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("average")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mAverageValues[component] = value;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("stddev")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         double value = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("value"))));
         mStandardDeviationValues[component] = value;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("percentile")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         std::vector<double> values;
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(values, pElement->getTextContent());
         mPercentileValues[component] = values;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("center")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         std::vector<double> values;
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(values, pElement->getTextContent());
         mBinCenterValues[component] = values;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("histogram")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pNode);
         ComplexComponent component = StringUtilities::fromXmlString<ComplexComponent>(
            A(pElement->getAttribute(X("component"))));
         std::vector<unsigned int> values;
         XmlReader::StrToVector<unsigned int, XmlReader::StringStreamAssigner<unsigned int> >(values,
            pElement->getTextContent());
         mHistogramValues[component] = values;
      }
      else if (XMLString::equals(pNode->getNodeName(), X("badValues")))
      {
         mBadValues.clear();
         XmlReader::StrToVector<int, XmlReader::StringStreamAssigner<int> >(mBadValues, pNode->getTextContent());
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
   VERIFYNRV(pDescriptor);

   int rowNum = pDescriptor->getRowCount();
   int colNum = pDescriptor->getColumnCount();
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

   if (mpOriginalAoi.get() != NULL)
   {
      mpAoi->clear();
      mpAoi->merge(*(mpOriginalAoi->getSelectedPoints()));
   }

   StatisticsInput statInput(mBands, dynamic_cast<const RasterElement*>(mpRasterElement),
      component, mStatisticsResolution, mBadValues, mpAoi.get());
   StatisticsOutput statOutput;

   mta::StatusBarReporter barReporter("Computing statistics", "app", "CF884AA2-A1BF-468d-9609-795DE0F7B7A4");

   std::vector<int> phaseWeights;
   phaseWeights.push_back(20);
   phaseWeights.push_back(80);
   mta::MultiPhaseProgressReporter progressReporter(barReporter, phaseWeights);

   mta::MultiThreadedAlgorithm<StatisticsInput, StatisticsOutput, StatisticsThread> statisticsAlgorithm
      (getNumRequiredThreads(pDescriptor->getRowCount()), statInput, statOutput, &progressReporter);
   statisticsAlgorithm.run();

   bool bInteger = true;
   EncodingType encoding = pDescriptor->getDataType();
   if ((encoding == FLT4BYTES) || (encoding == FLT8COMPLEX) || (encoding == FLT8BYTES) ||
      ((encoding == INT4SCOMPLEX) && (component == COMPLEX_MAGNITUDE)) ||
      ((encoding == INT4SCOMPLEX) && (component == COMPLEX_PHASE)))
   {
      bInteger = false;
   }

   HistogramInput histInput(statInput, statOutput);
   HistogramOutput histOutput(bInteger, statOutput.mMaximum, statOutput.mMinimum);

   progressReporter.setCurrentPhase(1);

   mta::MultiThreadedAlgorithm<HistogramInput, HistogramOutput, HistogramThread> histogramAlgorithm
      (getNumRequiredThreads(pDescriptor->getRowCount()), histInput, histOutput, &progressReporter);

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

StatisticsThread::StatisticsThread(const StatisticsInput& input, int threadCount, int threadIndex,
                                   ThreadReporter& reporter) :
   AlgorithmThread(threadIndex, reporter),
   mInput(input),
   mRowRange(getThreadRange(threadCount, static_cast<const RasterDataDescriptor*>(
                                 input.mpRasterElement->getDataDescriptor())->getRowCount())),
   mMaximum(-std::numeric_limits<double>::max()),
   mMinimum(std::numeric_limits<double>::max()),
   mSum(0.0),
   mSumSquared(0.0),
   mCount(0)
{}

void StatisticsThread::run()
{
   const RasterDataDescriptor* pDescriptor = static_cast<const RasterDataDescriptor*>(
      mInput.mpRasterElement->getDataDescriptor());
   VERIFYNRV(pDescriptor != NULL);

   BitMaskIterator diter(mInput.mpAoi, 0, mRowRange.mFirst, pDescriptor->getColumnCount() - 1, mRowRange.mLast);

   bool maxMinSet = false;
   mSum = 0.0;
   mSumSquared = 0.0;
   mCount = 0;

   EncodingType encoding = pDescriptor->getDataType();
   ComplexComponent component = mInput.mComplexComponent;
   unsigned int badValueCount = mInput.mBadValues.size();
   int firstBadValue = 0;
   if (badValueCount != 0)
   {
      firstBadValue = mInput.mBadValues.front();
   }
   std::vector<int>::const_iterator badBegin = mInput.mBadValues.begin();
   std::vector<int>::const_iterator badEnd = mInput.mBadValues.end();

   int oldPercentDone = -1;

   bool isBip = pDescriptor->getInterleaveFormat() == BIP;
   // Outer band loop not for BIP, will break if BIP
   for (std::vector<DimensionDescriptor>::const_iterator bandIt = mInput.mBandsToCalculate.begin();
        bandIt != mInput.mBandsToCalculate.end(); ++bandIt)
   {
      FactoryResource<DataRequest> pRequest;
      pRequest->setRows(pDescriptor->getActiveRow(diter.getBoundingBoxStartRow()),
                        pDescriptor->getActiveRow(diter.getBoundingBoxEndRow()), 0);
      pRequest->setColumns(pDescriptor->getActiveColumn(diter.getBoundingBoxStartColumn()),
                           pDescriptor->getActiveColumn(diter.getBoundingBoxEndColumn()), 0);
      if (isBip)
      {
         // request native accessor for efficiency
         pRequest->setBands(pDescriptor->getActiveBand(0),
                            pDescriptor->getActiveBand(pDescriptor->getBandCount() - 1),
                            pDescriptor->getBandCount());
      }
      else
      {
         // BIL is really tricky to use with BitMaskIterator so we convert to BSQ
         pRequest->setBands(*bandIt, *bandIt, 1);
         pRequest->setInterleaveFormat(BSQ);
      }
      DataAccessor da(mInput.mpRasterElement->getDataAccessor(pRequest.release()));
      if (!da.isValid())
      {
         return;
      }

      // Iterate the band over the AOI or all bands in the case of BIP
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This should be changed to for (; fiter != diter.end(); diter += mInput.mResolution)  if/when BitMaskIterator is modified to be an STL iterator (tclarke)")
      while (diter != diter.end())
      {
         LocationType loc;
         diter.getPixelLocation(loc);
         int percentDone = mRowRange.computePercent(static_cast<int>(loc.mY));
         if (percentDone >= oldPercentDone + 25)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }

         da->toPixel(static_cast<int>(loc.mY), static_cast<int>(loc.mX));
         VERIFYNRV(da.isValid());

         // Inner band loop for BIP, will break for other interleaves
         for (std::vector<DimensionDescriptor>::const_iterator bipBandIt = mInput.mBandsToCalculate.begin();
              bipBandIt != mInput.mBandsToCalculate.end(); ++bipBandIt)
         {
            double temp = ModelServices::getDataValue(encoding,
               da->getColumn(), component, isBip ? bipBandIt->getActiveNumber() : 0);

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
                  if (temp < mMinimum)
                  {
                     mMinimum = temp;
                  }

                  if (temp > mMaximum)
                  {
                     mMaximum = temp;
                  }
               }

               mSumSquared += temp*temp;
               mSum += temp;
               mCount++;
               if (!isBip)
               {
                  // this inner band loop is only for BIP
                  break;
               }
            }
         }

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove this if BitMaskIterator becomes an STL iterator or change it if BitMaskIterator::operator+=(int) is created (tclarke)")
         for (int cnt = 0; diter != diter.end() && cnt < mInput.mResolution; ++cnt)
         {
            diter.nextPixel();
         }
      }
      if (isBip)
      {
         // this outer band loop is not for BIP
         break;
      }
   }
}

double StatisticsThread::getMaximum() const
{
   return mMaximum;
}

double StatisticsThread::getMinimum() const
{
   return mMinimum;
}

double StatisticsThread::getSum() const
{
   return mSum;
}

double StatisticsThread::getSumSquared() const
{
   return mSumSquared;
}

unsigned int StatisticsThread::getCount() const
{
   return mCount;
}

StatisticsOutput::StatisticsOutput() :
   mMaximum(-std::numeric_limits<double>::max()),
   mMinimum(std::numeric_limits<double>::max()),
   mAverage(0.0),
   mStandardDeviation(0.0)
{}

bool StatisticsOutput::compileOverallResults(const std::vector<StatisticsThread*>& threads)
{
   mMaximum = -std::numeric_limits<double>::max();
   mMinimum = std::numeric_limits<double>::max();
   mAverage = 0.0;
   mStandardDeviation = 0.0;

   if (threads.size() == 0)
   {
      return false;
   }

   double totalSum = 0.0;
   double totalSquaredSum = 0.0;
   unsigned int pointCount = 0;

   for (std::vector<StatisticsThread*>::const_iterator iter = threads.begin(); iter != threads.end(); ++iter)
   {
      StatisticsThread* pThread = *iter;
      if (pThread != NULL)
      {
         mMaximum = std::max(mMaximum, pThread->getMaximum());
         mMinimum = std::min(mMinimum, pThread->getMinimum());
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

HistogramThread::HistogramThread(const HistogramInput& input,
                                 int threadCount,
                                 int threadIndex,
                                 mta::ThreadReporter& reporter) :
   AlgorithmThread(threadIndex, reporter),
   mInput(input),
   mCount(0),
   mRowRange(getThreadRange(threadCount, static_cast<const RasterDataDescriptor*>(
                                 input.mStatInput.mpRasterElement->getDataDescriptor())->getRowCount())),
   mBinCounts(HISTOGRAM_SIZE)
{}

void HistogramThread::run()
{
   double range = 1.0;
   double toBin = 0.0;
   
   if (mInput.mStatistics.mMaximum != mInput.mStatistics.mMinimum)
   {
      range = mInput.mStatistics.mMaximum - mInput.mStatistics.mMinimum;
      toBin = 0.999999999 * (HISTOGRAM_SIZE)/range;
   }

   std::vector<unsigned int>& binCounts = getBinCounts();

   const RasterDataDescriptor* pDescriptor = static_cast<const RasterDataDescriptor*>(
      mInput.mStatInput.mpRasterElement->getDataDescriptor());
   VERIFYNRV(pDescriptor != NULL);

   BitMaskIterator diter(mInput.mStatInput.mpAoi, 0, mRowRange.mFirst,
      pDescriptor->getColumnCount() - 1, mRowRange.mLast);

   bool isBip = pDescriptor->getInterleaveFormat() == BIP;
   // Outer band loop not for BIP, will break if BIP
   for (std::vector<DimensionDescriptor>::const_iterator bandIt = mInput.mStatInput.mBandsToCalculate.begin();
      bandIt != mInput.mStatInput.mBandsToCalculate.end(); ++bandIt)
   {
      FactoryResource<DataRequest> pRequest;
      pRequest->setRows(pDescriptor->getActiveRow(diter.getBoundingBoxStartRow()),
         pDescriptor->getActiveRow(diter.getBoundingBoxEndRow()), 0);
      pRequest->setColumns(pDescriptor->getActiveColumn(diter.getBoundingBoxStartColumn()),
         pDescriptor->getActiveColumn(diter.getBoundingBoxEndColumn()), 0);
      if (isBip)
      {
         // request native accessor for efficiency
         pRequest->setBands(pDescriptor->getActiveBand(0),
                            pDescriptor->getActiveBand(pDescriptor->getBandCount() - 1),
                            pDescriptor->getBandCount());
      }
      else
      {
         // BIL is really tricky to use with BitMaskIterator so we convert to BSQ
         pRequest->setBands(*bandIt, *bandIt, 1);
         pRequest->setInterleaveFormat(BSQ);
      }
      DataAccessor da(mInput.mStatInput.mpRasterElement->getDataAccessor(pRequest.release()));
      if (!da.isValid())
      {
         return;
      }

      EncodingType encoding = pDescriptor->getDataType();
      ComplexComponent component = mInput.mStatInput.mComplexComponent;
      unsigned int badValueCount = mInput.mStatInput.mBadValues.size();
      int firstBadValue = 0;
      if (badValueCount != 0)
      {
         firstBadValue = mInput.mStatInput.mBadValues.front();
      }
      std::vector<int>::const_iterator badBegin = mInput.mStatInput.mBadValues.begin();
      std::vector<int>::const_iterator badEnd = mInput.mStatInput.mBadValues.end();

      int oldPercentDone = -1;
      // Iterate the band over the AOI or all bands in the case of BIP
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This should be changed to for (; fiter != diter.end(); diter += mInput.mResolution)  if/when BitMaskIterator is modified to be an STL iterator (tclarke)")
      while (diter != diter.end())
      {
         LocationType loc;
         diter.getPixelLocation(loc);
         int percentDone = mRowRange.computePercent(static_cast<int>(loc.mY));
         if (percentDone >= oldPercentDone + 25)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }

         da->toPixel(static_cast<int>(loc.mY), static_cast<int>(loc.mX));
         VERIFYNRV(da.isValid());

         // Inner band loop for BIP, will break for other interleaves
         for (std::vector<DimensionDescriptor>::const_iterator bipBandIt = mInput.mStatInput.mBandsToCalculate.begin();
              bipBandIt != mInput.mStatInput.mBandsToCalculate.end(); ++bipBandIt)
         {
            double temp = ModelServices::getDataValue(encoding,
               da->getColumn(), mInput.mStatInput.mComplexComponent,
               isBip ? bipBandIt->getActiveNumber() : 0);

            int tempInt = roundDouble(temp);
            if (badValueCount == 0 || (badValueCount == 1 && tempInt != firstBadValue) || 
               !std::binary_search(badBegin, badEnd, tempInt))
            {
               int bin = static_cast<int>((temp-mInput.mStatistics.mMinimum) * toBin);
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
            if (!isBip)
            {
               // this inner band loop is only for BIP
               break;
            }
         }
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove this if BitMaskIterator becomes an STL iterator or change it if BitMaskIterator::operator+=(int) is created (tclarke)")
         for (int cnt = 0; diter != diter.end() && cnt < mInput.mStatInput.mResolution; ++cnt)
         {
            diter.nextPixel();
         }
      }
      if (isBip)
      {
         // this outer band loop is not for BIP
         break;
      }
   }
}

std::vector<unsigned int>& HistogramThread::getBinCounts()
{
   return mBinCounts;
}

bool HistogramOutput::compileOverallResults(const std::vector<HistogramThread*>& threads)
{
   std::vector<unsigned int> totalBinCounts(HISTOGRAM_SIZE);

   sumAllThreads(threads, totalBinCounts);
   computeBinCenters();
   computeResultHistogram(totalBinCounts);
   computePercentiles(totalBinCounts);

   return true;
}

const double* HistogramOutput::getBinCenters() const
{
   return mBinCenters;
}

const unsigned int* HistogramOutput::getBinCounts() const
{
   return mBinCounts;
}

const double* HistogramOutput::getPercentiles() const
{
   return mPercentiles;
}

void HistogramOutput::sumAllThreads(const std::vector<HistogramThread*>& threads,
                                    std::vector<unsigned int>& totalBinCounts)
{
   memset(&totalBinCounts[0], 0, HISTOGRAM_SIZE * sizeof(unsigned int));

   std::vector<HistogramThread*>::const_iterator iter;
   for (iter = threads.begin(); iter != threads.end(); ++iter)
   {
      HistogramThread* pThread = NULL;
      pThread = *iter;
      if (pThread != NULL)
      {
         const std::vector<unsigned int>& threadBinCounts = pThread->getBinCounts();

         transform(totalBinCounts.begin(), totalBinCounts.end(), 
            threadBinCounts.begin(), totalBinCounts.begin(), std::plus<unsigned int>());
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

void HistogramOutput::computeResultHistogram(const std::vector<unsigned int>& totalHistogram)
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

   // If the RasterElement contains any data with a value of NaN (Not a Number), Opticks
   // will crash when the following lines of code are executed. The RasterElement's
   // sanitizeData method must be called prior to calling this method to prevent the
   // crash.
   for (sourceBin = 0; sourceBin < HISTOGRAM_SIZE; ++sourceBin)
   {
      destBin = static_cast<int>(sourceBin * binConversion);
      mBinCounts[destBin] += totalHistogram[sourceBin];
   }
}

void HistogramOutput::computePercentiles(const std::vector<unsigned int>& totalHistogram)
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
      int cutoff = static_cast<int>(0.001 * percentile * pointCount);
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
