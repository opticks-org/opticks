/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "ColorType.h"
#include "PlotObject.h"

#include <vector>

/**
 *  Displays a histogram as a single plot object.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setHistogramData(), setColor(). 
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class Histogram : public PlotObject
{
public:
   /**
    *  Emitted when the histogram data changes.
    */
   SIGNAL_METHOD(Histogram, HistogramChanged)
   /**
    *  Emitted with any<ColorType> when the bin color changes.
    */
   SIGNAL_METHOD(Histogram, ColorChanged)

   /**
    *  Sets the histogram data values.
    *
    *  @param   binCount
    *           The number of data values in the histogram.
    *  @param   pBinCenters
    *           The histogram bin center values.
    *  @param   pValues
    *           The count values for each histogram bin.
    *  @param   pBinWidths
    *           The width of each histogram bin.  If this value is NULL, a default
    *           bin width is calculated.
    *  @param   bAbove
    *           An optional flag to display the count values as either positive or
    *           negative.  TRUE sets the values as positive.
    *
    *  @return  TRUE if the histogram data values were set successfully, otherwise FALSE.
    *
    *  @notify  This method will notify signalHistogramChanged.
    */
   virtual bool setHistogramData(unsigned int binCount, const double* pBinCenters, const double* pValues,
      const double* pBinWidths = NULL, bool bAbove = true) = 0;

   /**
    *  Retrieves the histogram data values.
    *
    *  @param   binCenters
    *           A vector populated with the location of each bin center.  The size of
    *           the vector indicates the number of bins.
    *  @param   binValues
    *           A vector populated with the count value for each bin.  The size of the
    *           vector indicates the number of bins.
    *  @param   binWidths
    *           A vector populated with the width of each bin.  The size of the vector
    *           indicates the number of bins.
    *
    *  @see     getNumBins()
    */
   virtual void getHistogramData(std::vector<double>& binCenters, std::vector<double>& binValues,
      std::vector<double>& binWidths) const = 0;

   /**
    *  Returns the number of bins in the histogram.
    *
    *  @return  The number of bins in the histogram.
    */
   virtual unsigned int getNumBins() const = 0;

   /**
    *  Sets the histogram color.
    *
    *  @param   histogramColor
    *           The new color for the histogram.  Must be a valid color.
    *
    *  @see     ColorType::isValid()
    *
    *  @notify  This method will notify signalColorChanged with any<ColorType>.
    */
   virtual void setColor(const ColorType& histogramColor) = 0;

   /**
    *  Returns the histogram color.
    *
    *  @return  The current histogram color.
    */
   virtual ColorType getColor() const = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~Histogram() {}
};

#endif
