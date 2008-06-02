/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFCHIPCONVERTER_H
#define NITFCHIPCONVERTER_H

#include "LocationType.h"
class RasterDataDescriptor;

namespace Nitf
{
   /**
    * Performs all computation combining ICHIPB and the application's chipping representations.
    */
   class ChipConverter
   {
   public:
      /**
       * Create a converter for a given descriptor.
       *
       * @param descriptor
       *        The descriptor for the element to perform conversions for.
       */
      ChipConverter(const RasterDataDescriptor &descriptor);

      /**
       * Create a converter for a given descriptor and chip coefficients.
       *
       * @param descriptor
       *        The descriptor for the element to perform conversions for.
       * @param coefficients
       *        The coefficients to use.  Must have 6 entries.
       */
      ChipConverter(const RasterDataDescriptor &descriptor, const std::vector<double> &coefficients);

      /**
       * Destructor.
       */
      ~ChipConverter();

      /**
       * Convert an original value to the active one.
       *
       * @param original
       *        The NITF original value.
       *
       * @return The active value.
       */
      LocationType originalToActive(LocationType original) const;

      /**
       * Convert an active value to the original one.
       *
       * @param active
       *        The active value.
       *
       * @return The NITF original value.
       */
      LocationType activeToOriginal(LocationType active) const;

      /**
       * Get the coefficients used.
       *
       * @return The coefficients used.
       */
      const std::vector<double> &getChipCoefficients() const;

   private:
      std::vector<double> mChipCoefficients;
      const RasterDataDescriptor &mDescriptor;

   };
}

#endif