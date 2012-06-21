/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTVALIDATOR_H
#define INTVALIDATOR_H

#include <QtGui/QValidator>

/**
 *  An integer validator for text strings.
 *
 *  The IntValidator class extends the QValidator class to provide a validator
 *  for integer data types, including large integer types.  The following data
 *  types are supported:
 *  - char
 *  - signed char
 *  - unsigned char
 *  - short
 *  - unsigned short
 *  - int
 *  - unsigned int
 *  - long
 *  - unsigned long
 *  - int64_t
 *  - uint64_t
 *  - Int64
 *  - UInt64
 *
 *  The behavior of IntValidator is similar to that of QIntValidator, except
 *  integer types larger than int are supported.  The class defines a validation
 *  range bounded by a minimum and maximum value that is checked during the
 *  validation sequence.
 */
template <typename T>
class IntValidator : public QValidator
{
public:
   /**
    *  Creates an IntValidator object with a default range.
    *
    *  This constructor creates a default validator with the default minimum
    *  and maximum values set to std::numeric_limits<T>::min() and
    *  std::numeric_limits<T>::max().
    *
    *  @param   pParent
    *           The parent object.
    *
    *  @see     IntValidator(T, T, QObject*)
    */
   IntValidator(QObject* pParent);

   /**
    *  Creates an IntValidator object with a given range.
    *
    *  This constructor creates a validator with the validation range set to
    *  the given minimum and maximum values.
    *
    *  @param   minimum
    *           The minimum value for the validation range.
    *  @param   maximum
    *           The maximum value for the validation range.
    *  @param   pParent
    *           The parent object.
    *
    *  @see     IntValidator(QObject*)
    */
   IntValidator(T minimum, T maximum, QObject* pParent);

   /**
    *  Destroys the validator object.
    */
   virtual ~IntValidator();

   /**
    *  Sets the minimum value of the validation range.
    *
    *  @param   minimum
    *           The minimum value.
    *
    *  @see     setMaximum(), setRange()
    */
   void setMinimum(T minimum);

   /**
    *  Returns the minimum value of the validation range.
    *
    *  @return  The minimum value.
    *
    *  @see     maximum()
    */
   T minimum() const;

   /**
    *  Sets the maximum value of the validation range.
    *
    *  @param   maximum
    *           The maximum value.
    *
    *  @see     setMinimum(), setRange()
    */
   void setMaximum(T maximum);

   /**
    *  Returns the maximum value of the validation range.
    *
    *  @return  The maximum value.
    *
    *  @see     minimum()
    */
   T maximum() const;

   /**
    *  Sets the minimum and maximum values of the validation range.
    *
    *  @param   minimum
    *           The minimum value.
    *  @param   maximum
    *           The maximum value.
    *
    *  @see     setMinimum(), setMaximum()
    */
   void setRange(T minimum, T maximum);

   /**
    *  Validates the given text against the minimum and maximum integer values.
    *
    *  This method is typically called automatically by the widget containing
    *  the validator.  The implementation converts the given string to an
    *  integer using the QString text conversion methods (e.g. QString::toInt())
    *  and returns whether the string is or is not a valid integer within the
    *  validation range or if the string could be a valid integer with
    *  additional input.
    *
    *  @param   input
    *           The text to validate.
    *  @param   pos
    *           The cursor position.  This parameter is ignored.
    *
    *  @return  Returns QValidator::Intermediate if the input string is empty
    *           or if the input string contains only a positive (+) sign or
    *           negative (-) sign and a positive or negative value is allowed
    *           in the range. Returns QValidator::Acceptable if the string
    *           converts to a valid integer and is contained within the
    *           validation range. Returns QValidator::Invalid in all other
    *           cases.
    */
   virtual QValidator::State validate(QString& input, int& pos) const;

private:
   IntValidator(const IntValidator& rhs);
   IntValidator& operator=(const IntValidator& rhs);

   T mMinimum;
   T mMaximum;
};

#endif
