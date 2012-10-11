/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BADVALUES_H
#define BADVALUES_H

#include "ConfigurationSettings.h"
#include "Serializable.h"
#include "Subject.h"

#include <string>
#include <vector>

/**
 *  Bad values for raster elements.
 *
 *  The BadValues class is used to maintain the bad values specific for a
 *  spatial dimension of a data set.  A separate BadValues object is available
 *  for each band of a RasterElement object.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *    - The object is deleted.
 *    - The following methods are called: addRange(), addBadValue(),
 *      setLowerBadValueThreshold(), setUpperBadValueThreshold(), setBadValueTolerance() and clear().
 *    - Other notifications documented in the Subject class.
 *
 *  @see    RasterDataDescriptor::getBadValues(), Statistics::getBadValues()
 */
class BadValues : public Serializable, public Subject
{
public:
   SETTING(Tolerance, BadValues, std::string, "0.000001")

   /**
    *  Adds a range of bad data values.
    *
    *  Adds a range of values that will be considered bad values. Multiple ranges
    *  can be added, but overlapping ranges will be combined. A data value that is greater than or equal
    *  to the start value and less than or equal to the end value will be considered bad.
    *
    *  @param   startValStr
    *           The start value string for the range of bad values.
    *  @param   endValStr
    *           The end value string for the range of bad values.
    *
    *  @return  Returns \c true if the bad value range was successfully added; otherwise,
    *           returns \c false.
    *
    *  @note   Call method getLastErrorMsg() to retrieve the reason for a failure.
    *
    *  @see    getLastErrorMsg()
    */
   virtual bool addRange(const std::string& startValStr, const std::string& endValStr) = 0;

   /**
    *  Adds a bad data value.
    *
    *  Adds a value that will be considered a bad value.
    *
    *  @param   valueStr
    *           The bad data value string.
    *
    *  @return  Returns \c true if the bad value was successfully added; otherwise,
    *           returns \c false.
    *
    *  @note   Call method getLastErrorMsg() to retrieve the reason for a failure.
    *
    *  @see    getLastErrorMsg()
    */
   virtual bool addBadValue(const std::string& valueStr) = 0;

   /**
    *  Add list of bad values.
    *
    *  Add individual bad values from a vector of integer values.
    *
    *  @param   badValues
    *           The vector of values to add as bad values.
    *
    *  @note    This convenience method is provided for backward compatibility where simple integer bad values
    *           are used (e.g. importers that set 0 as default bad value).
    */
   virtual void addBadValues(const std::vector<int>& badValues) = 0;

   /**
    *  Queries whether the value is a bad data value.
    *
    *  @param   value
    *           The value to be evaluated as bad or valid. The bad value tolerance will be used to evaluate
    *           whether or not the data value matches the bad value criteria. The tolerance is the minimum
    *           difference between two values to consider them different.
    *
    *  @return  Returns \c true if the value is a bad data value;
    *           otherwise returns \c false.
    *
    *  @see     setBadValueTolerance(), getBadValueTolerance()
    */
   virtual bool isBadValue(double value) const = 0;

   /**
    *  Sets the lower threshold for bad values.
    *
    *  @param   thresholdStr
    *           The data value string that marks the lower threshold for bad data values. %Any data value
    *           less than or equal to this value will be considered a bad value. If the value string is empty,
    *           the lower threshold check will be disabled.
    *
    *  @return  Returns \c true if the lower threshold was successfully set from the input string; otherwise,
    *           returns \c false.
    *
    *  @note   Call method getLastErrorMsg() to retrieve the reason for a failure.
    *
    *  @see    getLastErrorMsg()
    */
   virtual bool setLowerBadValueThreshold(const std::string& thresholdStr) = 0;

   /**
    *  Sets the upper threshold for bad values.
    *
    *  @param   thresholdStr
    *           The data value string that marks the upper threshold for bad data values. %Any data value
    *           greater than or equal to this value will be considered a bad value. If the value string is empty,
    *           the upper threshold check will be disabled.
    *
    *  @return  Returns \c true if the upper threshold was successfully set from the input string; otherwise,
    *           returns \c false.
    *
    *  @note   Call method getLastErrorMsg() to retrieve the reason for a failure.
    *
    *  @see    getLastErrorMsg()
    */
   virtual bool setUpperBadValueThreshold(const std::string& thresholdStr) = 0;

   /**
    *  Returns the lower threshold for bad values.
    *
    *  @return  Returns the data value string that marks the lower threshold for bad data values. %Any data value
    *           less than or equal to this value will be considered a bad value.
    */
   virtual std::string getLowerBadValueThreshold() const = 0;

   /**
    *  Returns the upper threshold for bad values.
    *
    *  @return  Returns the data value string that marks the upper threshold for bad data values. %Any data value
    *           greater than or equal to this value will be considered a bad value.
    */
   virtual std::string getUpperBadValueThreshold() const = 0;

   /**
    *  Returns the ranges of bad values.
    *
    *  @return  Returns a vector of bad value string pairs that mark the ranges for bad data values. %Any data value
    *           greater than or equal to the start of a range and less than or equal to the end of a range
    *           is considered a bad value.
    */
   virtual const std::vector<std::pair<std::string, std::string> > getBadValueRanges() const = 0;

   /**
    *  Returns a vector of individual bad values.
    *
    *  @return  Returns a vector of string values that mark the individual bad data values. %Any data value
    *           equal to one of the values in the vector is considered a bad value.
    */
   virtual const std::vector<std::string> getIndividualBadValues() const = 0;

   /**
    *  Returns a std::string representation of all bad data values.
    *
    *  @return  Returns a std::string representation of all the individual bad values, the bad value ranges
    *           and the bad value thresholds using a comma delimiter.
    *           Examples of bad value strings:
    *             - Lower threshold of -0.5, upper threshold +2.0, individual value of 0.0 returns "<-0.5,0.0,>2.0".
    *             - Lower threshold of -0.5, upper threshold +2.0, range of +0.5 to +0.75 returns "<-0.5,0.5<>0.75,>2.0".
    */
   virtual std::string getBadValuesString() const = 0;

   /**
    *  Returns a default bad value.
    *
    *  @return  Returns a value that will result in call to isBadValue() returning \c false.
    *
    *  @note    As an example, the band binning algorithm computes an average value for the bands being binned.
    *           If all of the band values in a bin are bad values, the algorithm needs to set the computed
    *           average to a value that will be evaluated as bad by the BadValues object associated with
    *           the band binning output. This method returns a value that will evaluate as bad.
    */
   virtual double getDefaultBadValue() const = 0;

   /**
    *  Sets bad values.
    *
    *  Set the bad value thresholds, ranges and values from another instance of BadValues.
    *
    *  @param   pBadValues
    *           The BadValues instance to use in setting the bad values.
    *
    *  @see     setBadValues(const std::string&, const std::string&)
    */
   virtual void setBadValues(const BadValues* pBadValues) = 0;

   /**
    *  Sets bad values from strings.
    *
    *  Set the bad value thresholds, ranges and values from a string representation for bad values.
    *
    *  @param   badValStr
    *           The string representation to use in setting the bad values. If the string is empty, the 
    *           bad values criteria will be cleared, i.e., same result as calling method clear() except
    *           the tolerance value will not be set to the default value.
    *  @param   toleranceStr
    *           The string representation to use in setting the bad values tolerance.  If the string is empty, 
    *           the existing tolerance will not be changed.
    *
    *  @return  Returns \c true if the bad values were successfully set from the input strings; otherwise, returns
    *           \c false.
    *
    *  @note   Call method getLastErrorMsg() to retrieve the reason for a failure.
    *
    *  @see    setBadValues(const BadValues*), getLastErrorMsg()
    */
   virtual bool setBadValues(const std::string& badValStr, const std::string& toleranceStr = std::string()) = 0;

   /**
    *  Gets the stored error message associated with the last BadValues method called that attempted to make
    *  changes in the bad values.
    *
    *  @return  Returns the last error message associated with changing the bad values and clears
    *           the stored messaged. Returns an empty string if no error has occurred.
    */
   virtual std::string getLastErrorMsg() = 0;

   /**
    *  Clears all bad values.
    *
    *  Clears the bad value thresholds, ranges and values. All data values will be considered valid.
    */
   virtual void clear() = 0;

   /**
    *  Queries whether any bad data values have been set.
    *
    *  @return  Returns \c true if any bad data values have been set;
    *           otherwise returns \c false.
    */
   virtual bool empty() const = 0;

   /**
    *  Get the bad values tolerance.
    *
    *  Get the tolerance that will be used to evaluate if a data value is bad. The tolerance is the
    *  minimum difference between two values to consider them different.
    *
    *  @return  The bad values tolerance in string format.
    */
   virtual std::string getBadValueTolerance() const = 0;

   /**
    *  Set the bad values tolerance.
    *
    *  Set the tolerance that will be used to evaluate if a data value is bad. The tolerance is the
    *  minimum difference between two values to consider them different.
    *
    *  @param   badValToleranceStr
    *           The value string to use for the bad values tolerance. The method will return \c false for an
    *           empty value string.
    *
    *  @return  Returns \c true if the bad values tolerance was successfully set from the input string; otherwise,
    *           returns \c false.
    *
    *  @note   Call method getLastErrorMsg() to retrieve the reason for a failure.
    *
    *  @see    getLastErrorMsg()
    */
   virtual bool setBadValueTolerance(const std::string& badValToleranceStr) = 0;

   /**
    *  Suppress notification of changes while making multiple changes to the bad values.
    *
    *  This method will prevent the notification of Subject::signalModified. It will continue to
    *  block notification until method endBadValuesUpdate() is called.
    *
    *  @see     endBadValuesUpdate()
    */
   virtual void startBadValuesUpdate() = 0;

   /**
    *  Enable notification of changes to the bad values.
    *
    *  This method will re-enable the notification of Subject::signalModified after a call to startBadValuesUpdate().
    *
    *  @see     startBadValuesUpdate()
    */
   virtual void endBadValuesUpdate() = 0;

   /**
    *  Compares all values in this BadValues object with those of another BadValues
    *  object.
    *
    *  @param   pBadValues
    *           The BadValues object with which to compare values in this BadValues
    *           object.  This method does nothing and returns \c false if
    *           \c NULL is passed in.
    *
    *  @return  Returns \c true if all values in \em pBadValues are the same as the
    *           values in this BadValues object; otherwise returns \c false.
    */
   virtual bool compare(const BadValues* pBadValues) const = 0;

protected:
   /**
    *  The BadValues objects contained in RasterDataDescriptor and Statistics
    *  will be destroyed automatically when those objects are destroyed.\ %Any
    *  other BadValues object should be destroyed either as part of a
    *  FactoryResource object or by calling ObjectFactory::destroyObject().
    */
   virtual ~BadValues() {}
};

#endif
