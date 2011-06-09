/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include "Serializable.h"
#include "ComplexData.h"

#include <vector>

/**
 *  Statistics for raster elements.
 *
 *  The Statistics class is used to maintain the information specific for a
 *  spatial dimension of a data set.  A separate Statistics object is available
 *  for each band of a RasterElement object.
 *
 *  The data used in the calculation of the statistics is set automatically by
 *  the  RasterElement.  The values are not calculated until one of the get
 *  methods are called.  If the specific statistics values for the data are
 *  known, the set methods can be called to avoid calculating the
 *  values, which could take some time.  All values in the object need to be
 *  set to avoid calculations when calling one of the get methods.  Typically
 *  the values would only be directly set by an importer.
 *
 *  @warning Be careful when dealing with RasterElements that have NaN(Not a number) 
 *           values. Before doing anything with the dataset, be sure to sanitize the data
 *           first. Failure to do this may lead to Opticks crashing when you attempt to 
 *           calculate the statistics.
 *
 *  @see    RasterElement::getStatistics()
 */
class Statistics : public Serializable
{
public:
   /**
    *  Sets the minimum value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dMin
    *           The minimum value for the data.
    */
   virtual void setMin(double dMin) = 0;

   /**
    *  Sets the minimum value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dMin
    *           The minimum value for the data.
    *  @param   component
    *           The complex data component represented by the given minimum.
    */
   virtual void setMin(double dMin, ComplexComponent component) = 0;

   /**
    *  Returns the minimum value for the data.
    *
    *  @return  The minimum value detected in the data, or -99999.9 if the
    *           statistics could not be calculated successfully.
    */
   virtual double getMin() = 0;

   /**
    *  Returns the minimum value for the data.
    *
    *  @param   component
    *           The complex data component for which to get its minimum value.
    *
    *  @return  The minimum value detected in the data, or -99999.9 if the
    *           statistics could not be calculated successfully.
    */
   virtual double getMin(ComplexComponent component) = 0;

   /**
    *  Sets the maximum value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dMax
    *           The maximum value for the data.
    */
   virtual void setMax(double dMax) = 0;

   /**
    *  Sets the maximum value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dMax
    *           The maximum value for the data.
    *  @param   component
    *           The complex data component represented by the given maximum.
    */
   virtual void setMax(double dMax, ComplexComponent component) = 0;

   /**
    *  Returns the maximum value for the data.
    *
    *  @return  The maximum value detected in the data, or -99999.9 if the
    *           statistics could not be calculated successfully.
    */
   virtual double getMax() = 0;

   /**
    *  Returns the maximum value for the data.
    *
    *  @param   component
    *           The complex data component for which to get its maximum value.
    *
    *  @return  The maximum value detected in the data, or -99999.9 if the
    *           statistics could not be calculated successfully.
    */
   virtual double getMax(ComplexComponent component) = 0;

   /**
    *  Sets the average value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dAverage
    *           The average value for the data.
    */
   virtual void setAverage(double dAverage) = 0;

   /**
    *  Sets the average value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dAverage
    *           The average value for the data.
    *  @param   component
    *           The complex data component represented by the given average.
    */
   virtual void setAverage(double dAverage, ComplexComponent component) = 0;

   /**
    *  Returns the average value for the data.
    *
    *  @return  The average of all values in the data, or -99999.9 if the
    *           statistics could not be calculated successfully.
    */
   virtual double getAverage() = 0;

   /**
    *  Returns the average value for the data.
    *
    *  @param   component
    *           The complex data component for which to get its average value.
    *
    *  @return  The average of all values in the data, or -99999.9 if the
    *           statistics could not be calculated successfully.
    */
   virtual double getAverage(ComplexComponent component) = 0;

   /**
    *  Sets the standard deviation value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dStdDev
    *           The standard deviation value for the data.
    */
   virtual void setStandardDeviation(double dStdDev) = 0;

   /**
    *  Sets the standard deviation value for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   dStdDev
    *           The standard deviation value for the data.
    *  @param   component
    *           The complex data component represented by the given standard
    *           deviation.
    */
   virtual void setStandardDeviation(double dStdDev, ComplexComponent component) = 0;

   /**
    *  Returns the standard deviation value for the data.
    *
    *  @return  The standard deviation of all values in the band, or -99999.9
    *           if the statistics could not be calculated successfully.
    */
   virtual double getStandardDeviation() = 0;

   /**
    *  Returns the standard deviation value for the data.
    *
    *  @param   component
    *           The complex data component for which to get its standard
    *           deviation value.
    *
    *  @return  The standard deviation of all values in the data, or -99999.9
    *           if the statistics could not be calculated successfully.
    */
   virtual double getStandardDeviation(ComplexComponent component) = 0;

   /**
    *  Sets the percentile values for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   pPercentiles
    *           The percentile values for the data.
    */
   virtual void setPercentiles(const double* pPercentiles) = 0;

   /**
    *  Sets the percentile values for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   pPercentiles
    *           The percentile values for the data.
    *  @param   component
    *           The complex data component represented by the given
    *           percentiles.
    */
   virtual void setPercentiles(const double* pPercentiles, ComplexComponent component) = 0;

   /**
    *  Returns the percentile boundaries for the data.
    *
    *  @return  An array of 1001 values that contain the percentile boundaries
    *           for this band, or \b NULL if the statistics could not be
    *           calculated successfully.
    */
   virtual const double* getPercentiles() = 0;

   /**
    *  Returns the percentile boundaries for the data.
    *
    *  @param   component
    *           The complex data component for which to get its percentile
    *           values.
    *
    *  @return  An array of 1001 values that contain the percentile boundaries
    *           for the data, or \b NULL if the statistics could not be
    *           calculated successfully.
    */
   virtual const double* getPercentiles(ComplexComponent component) = 0;

   /**
    *  Sets the histogram values for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   pBinCenters
    *           The bin center values for the histogram.
    *  @param   pHistogramCounts
    *           The histogram values for the data.
    */
   virtual void setHistogram(const double* pBinCenters, const unsigned int* pHistogramCounts) = 0;

   /**
    *  Sets the histogram values for the data.
    *
    *  Calling this method and the set methods for the other statistics values,
    *  calculations will not be performed when calling the get methods, which
    *  could save some time.
    *
    *  @param   pBinCenters
    *           The bin center values for the histogram.
    *  @param   pHistogramCounts
    *           The histogram values for the data.
    *  @param   component
    *           The complex data component represented by the given histogram.
    */
   virtual void setHistogram(const double* pBinCenters, const unsigned int* pHistogramCounts,
      ComplexComponent component) = 0;

   /**
    *  Returns the histogram values for the data.
    *
    *  @param   pBinCenters
    *           Populated with an array of 256 values that specify the center
    *           location of the histogram bins.  This value is set to \b NULL
    *           if the histogram cannot be computed successfully.
    *  @param   pHistogramCounts
    *           Populated with an array of 256 values that specify the number
    *           of values contained in each histogram bin.  This value is set
    *           to \b NULL if the histogram cannot be computed successfully.
    */
   virtual void getHistogram(const double*& pBinCenters, const unsigned int*& pHistogramCounts) = 0;

   /**
    *  Returns the histogram values for the data.
    *
    *  @param   pBinCenters
    *           Populated with an array of 256 values that specify the center
    *           location of the histogram bins.  This value is set to \b NULL
    *           if the histogram cannot be computed successfully.
    *  @param   pHistogramCounts
    *           Populated with an array of 256 values that specify the number
    *           of values contained in each histogram bin.  This value is set
    *           to \b NULL if the histogram cannot be computed successfully.
    *  @param   component
    *           The complex data component for which to get its histogram
    *           values.
    */
   virtual void getHistogram(const double*& pBinCenters, const unsigned int*& pHistogramCounts,
      ComplexComponent component) = 0;

   /**
    *  Sets the step size used when computing the statistics for the data.
    *
    *  This method sets the number of rows and columns that will be stepped
    *  over while looping through the data to compute the statistics. 
    *
    *  @param   resolution
    *           The step size for the data. Must be at least 1.
    */
   virtual void setStatisticsResolution(int resolution) = 0;

   /**
    *  Gets the step size used when computing the statistics for the data.
    *
    *  This method gets the number of rows and columns that will be stepped
    *  over while looping through the data to compute the statistics. 
    *
    *  @return   The step size for the data. Will be at least 1.
    */
   virtual int getStatisticsResolution() const = 0;

   /**
    *  Sets values that will be excluded from statistics computation.
    *
    *  Certain layers (e.g. ThresholdLayer) also take these values into
    *  account.
    *
    *  This method is intended to be used to change the bad values in an
    *  existing RasterElement.  To set the initial bad values that should be
    *  used to populate a RasterElement when it is created, use the
    *  RasterDataDescriptor::setBadValues() method instead.
    *
    *  @param   badValues
    *           The values that should be ignored when computing statistics.
    */
   virtual void setBadValues(const std::vector<int>& badValues) = 0;

   /**
    *  Gets the values that will be excluded from statistics computation.
    *  These values will always be sorted.
    *
    *  @return  The values that should be ignored when computing statistics.
    */
   virtual const std::vector<int>& getBadValues() const = 0;

   /**
    *  Queries whether the statistics have been calculated for the data.
    *
    *  @return  Returns \b true if the statistics have been calculated;
    *           otherwise returns \b false.
    */
   virtual bool areStatisticsCalculated() const = 0;

   /**
    *  Queries whether the complex data statistics have been calculated for the
    *  data.
    *
    *  @param   component
    *           The complex data component for which to query its statistics.
    *
    *  @return  Returns \b true if the statistics have been calculated;
    *           otherwise returns \b false.
    */
   virtual bool areStatisticsCalculated(ComplexComponent component) const = 0;

protected:
   /**
    * A plug-in cannot create this object, it can only retrieve an already existing
    * object from RasterElement::getStatistics and the RasterElement will manage 
    * any instances of this object.
    */
   virtual ~Statistics() {}
};

#endif
