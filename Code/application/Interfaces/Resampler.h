/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RESAMPLER_H
#define RESAMPLER_H

/**
 *  Resamples data based on wavelength values.
 *
 *  The Resampler class provides the capability to resample data from one
 *  set of wavelengths to another.  When the resampler is executed, the
 *  resampling algorithm is the current algorithm selected in the user options.
 *
 *  To use the Resampler, PlugIns should create a PlugInResource
 *  with Resampler::ResamplerPlugInName() as its input argument.
 *
 *  @code
 *  PlugInResource pPlugIn(Resampler::ResamplerPlugInName());
 *  Resampler* pResampler = dynamic_cast<Resampler*>(pPlugIn.get());
 *  @endcode
 */
class Resampler
{
public:
   /**
    *  Resamples data to a set of given wavelengths.
    *
    *  @param   fromData
    *           The data values to resample.
    *  @param   toData
    *           This vector is populated with the resampled data values.
    *  @param   fromWavelengths
    *           The wavelength values that correspond with the given data values
    *           to resample.
    *  @param   toWavelengths
    *           The wavelength values to which to resample the data values.
    *  @param   toFwhm
    *           The full width half max values corresponding to the wavelength
    *           values to which to resample the data values.  If this vector is
    *           empty, a default value is used as defined in the user options.
    *  @param   toBands
    *           This vector is populated with the zero-based index values corresponding
    *           to the positions in the destination wavelengths vector to which 
    *           data values were actually resampled.
    *  @param   errorMessage
    *           This string is populated with an error message if the resampling
    *           fails.
    *
    *  @return  TRUE if the data was successfully resampled, otherwise FALSE.
    */
   virtual bool execute(const std::vector<double>& fromData, std::vector<double>& toData,
      const std::vector<double>& fromWavelengths, const std::vector<double>& toWavelengths, 
      const std::vector<double>& toFwhm, std::vector<int>& toBands, std::string& errorMessage) = 0;
};

#endif
