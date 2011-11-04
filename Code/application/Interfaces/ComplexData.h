/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COMPLEXDATA_H
#define COMPLEXDATA_H

#include <complex>
#include "DataVariantValidator.h"
#include "EnumWrapper.h"

/**
 * Specifies the various components of complex data.
 */
enum ComplexComponentEnum { COMPLEX_MAGNITUDE, COMPLEX_PHASE, COMPLEX_INPHASE, COMPLEX_QUADRATURE };

/**
 * @EnumWrapper ::ComplexComponentEnum.
 */
typedef EnumWrapper<ComplexComponentEnum> ComplexComponent;

/**
 *  Contains integer real and imaginary values.
 *
 *  @see     FloatComplex
 */
class IntegerComplex
{
public:
   /**
    *  Creates an default IntegerComplex object.
    */
   IntegerComplex() : mReal(0), mImaginary(0) {}

   /**
    *  Creates an IntegerComplex object with initial values.
    *
    *  @param   real
    *           The real or in-phase component value.
    *  @param   imaginary
    *           The imaginary or quadrature component value.
    */
   IntegerComplex(short real, short imaginary) : mReal(real), mImaginary(imaginary) {}
   IntegerComplex(double real) : mReal((short)real), mImaginary(0) {}

   double operator[] (ComplexComponent component) const
   {
      double dValue = 0.0;
      switch (component)
      {
         case COMPLEX_MAGNITUDE:
            dValue = (double) getMagnitude();
            break;

         case COMPLEX_PHASE:
            dValue = (double) getPhase();
            break;

         case COMPLEX_INPHASE:
            dValue = (double) mReal;
            break;

         case COMPLEX_QUADRATURE:
            dValue = (double) mImaginary;
            break;

         default:
            break;
      }

      return dValue;
   }

   /**
    *  Returns the magnitude of the data.
    *
    *  @return  The magnitude.
    */
   float getMagnitude() const
   {
      std::complex<float> complexData(mReal, mImaginary);
      return std::abs(complexData);
   }

   /**
    *  Returns the phase angle of the data.
    *
    *  @return  The phase angle in radians.
    */
   float getPhase() const
   {
      std::complex<float> complexData(mReal, mImaginary);
      return std::arg(complexData);
   }

   short mReal;
   short mImaginary;
};

/**
 *  Contains floating point real and imaginary values.
 *
 *  @see     IntegerComplex
 */
class FloatComplex
{
public:
   /**
    *  Creates an default FloatComplex object.
    */
   FloatComplex() : mReal(0.0f), mImaginary(0.0f) {}

   /**
    *  Creates an FloatComplex object with initial values.
    *
    *  @param   real
    *           The real or in-phase component value.
    *  @param   imaginary
    *           The imaginary or quadrature component value.
    */
   FloatComplex(float real, float imaginary) : mReal(real), mImaginary(imaginary) {}
   FloatComplex(double real) : mReal((float)real), mImaginary(0.0f) {}

   double operator[] (ComplexComponent component) const
   {
      double dValue = 0.0;
      switch (component)
      {
         case COMPLEX_MAGNITUDE:
            dValue = (double) getMagnitude();
            break;

         case COMPLEX_PHASE:
            dValue = (double) getPhase();
            break;

         case COMPLEX_INPHASE:
            dValue = (double) mReal;
            break;

         case COMPLEX_QUADRATURE:
            dValue = (double) mImaginary;
            break;

         default:
            break;
      }

      return dValue;
   }

   /**
    *  Returns the magnitude of the data.
    *
    *  @return  The magnitude.
    */
   float getMagnitude() const
   {
      std::complex<float> complexData(mReal, mImaginary);
      return std::abs(complexData);
   }

   /**
    *  Returns the phase angle of the data.
    *
    *  @return  The phase angle in radians.
    */
   float getPhase() const
   {
      std::complex<float> complexData(mReal, mImaginary);
      return std::arg(complexData);
   }

   float mReal;
   float mImaginary;
};

/**
 * \cond INTERNAL
 * This template specialization is required to allow this type to be put into a DataVariant.
 */
template <> class VariantTypeValidator<ComplexComponent> {};
template <> class VariantTypeValidator<IntegerComplex> {};
template <> class VariantTypeValidator<FloatComplex> {};
/// \endcond

#endif   // COMPLEXDATA_H
