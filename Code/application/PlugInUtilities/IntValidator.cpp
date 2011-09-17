/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "Int64.h"
#include "IntValidator.h"
#include "UInt64.h"

#include <limits>

template<typename T>
IntValidator<T>::IntValidator(QObject* pParent) :
   QValidator(pParent),
   mMinimum(std::numeric_limits<T>::min()),
   mMaximum(std::numeric_limits<T>::max())
{}

template<typename T>
IntValidator<T>::IntValidator(T minimum, T maximum, QObject* pParent) :
   QValidator(pParent),
   mMinimum(minimum <= maximum ? minimum : maximum),
   mMaximum(minimum <= maximum ? maximum : minimum)
{}

template<typename T>
IntValidator<T>::~IntValidator()
{}

template<typename T>
void IntValidator<T>::setMinimum(T minimum)
{
   setRange(minimum, mMaximum);
}

template<typename T>
T IntValidator<T>::minimum() const
{
   return mMinimum;
}

template<typename T>
void IntValidator<T>::setMaximum(T maximum)
{
   setRange(mMinimum, maximum);
}

template<typename T>
T IntValidator<T>::maximum() const
{
   return mMaximum;
}

template<typename T>
void IntValidator<T>::setRange(T minimum, T maximum)
{
   if (minimum > maximum)
   {
      T tempValue = minimum;
      minimum = maximum;
      maximum = tempValue;
   }

   mMinimum = minimum;
   mMaximum = maximum;
}

template<typename T>
QValidator::State IntValidator<T>::validate(QString& input, int& pos) const
{
   // Tentatively allow an empty string
   if (input.isEmpty() == true)
   {
      return QValidator::Intermediate;
   }

   // Disallow a positive or negative sign based on the min and max
   if ((mMinimum >= 0 && input[0] == '-') || (mMaximum < 0 && input[0] == '+'))
   {
      return QValidator::Invalid;
   }

   // Tentatively allow only a positive or negative sign
   if ((input.size() == 1) && (input[0] == '+' || input[0] == '-'))
   {
      return QValidator::Intermediate;
   }

   // Convert the string to a value and disallow text that is not a valid value
   bool success = false;

   T value = textToValue(input, &success);
   if (success == false)
   {
      return QValidator::Invalid;
   }

   // Disallow a value outside the set range
   if (value < mMinimum || value > mMaximum)
   {
      return QValidator::Invalid;
   }

   // Allow the value
   return QValidator::Acceptable;
}

template<>
char IntValidator<char>::textToValue(const QString& input, bool* pSuccess) const
{
   short value = input.toShort(pSuccess);
   if (pSuccess != NULL && *pSuccess == false)
   {
      return 0;
   }

   if ((value >= std::numeric_limits<char>::min()) && (value <= std::numeric_limits<char>::max()))
   {
      return static_cast<char>(value);
   }

   if (pSuccess != NULL)
   {
      *pSuccess = false;
   }

   return 0;
}

template<>
signed char IntValidator<signed char>::textToValue(const QString& input, bool* pSuccess) const
{
   short value = input.toShort(pSuccess);
   if (pSuccess != NULL && *pSuccess == false)
   {
      return 0;
   }

   if ((value >= std::numeric_limits<signed char>::min()) && (value <= std::numeric_limits<signed char>::max()))
   {
      return static_cast<signed char>(value);
   }

   if (pSuccess != NULL)
   {
      *pSuccess = false;
   }

   return 0;
}

template<>
unsigned char IntValidator<unsigned char>::textToValue(const QString& input, bool* pSuccess) const
{
   unsigned short value = input.toUShort(pSuccess);
   if (pSuccess != NULL && *pSuccess == false)
   {
      return 0;
   }

   if (value <= std::numeric_limits<unsigned char>::max())
   {
      return static_cast<unsigned char>(value);
   }

   if (pSuccess != NULL)
   {
      *pSuccess = false;
   }

   return 0;
}

template<>
short IntValidator<short>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toShort(pSuccess);
}

template<>
unsigned short IntValidator<unsigned short>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toUShort(pSuccess);
}

template<>
int IntValidator<int>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toInt(pSuccess);
}

template<>
unsigned int IntValidator<unsigned int>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toUInt(pSuccess);
}

template<>
long IntValidator<long>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toLong(pSuccess);
}

template<>
unsigned long IntValidator<unsigned long>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toULong(pSuccess);
}

#ifdef WIN_API
template<>
int64_t IntValidator<int64_t>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toLongLong(pSuccess);
}

template<>
uint64_t IntValidator<uint64_t>::textToValue(const QString& input, bool* pSuccess) const
{
   return input.toULongLong(pSuccess);
}
#endif

template<>
Int64 IntValidator<Int64>::textToValue(const QString& input, bool* pSuccess) const
{
   return Int64(input.toLongLong(pSuccess));
}

template<>
UInt64 IntValidator<UInt64>::textToValue(const QString& input, bool* pSuccess) const
{
   return UInt64(input.toULongLong(pSuccess));
}

template class IntValidator<char>;
template class IntValidator<signed char>;
template class IntValidator<unsigned char>;
template class IntValidator<short>;
template class IntValidator<unsigned short>;
template class IntValidator<int>;
template class IntValidator<unsigned int>;
template class IntValidator<long>;
template class IntValidator<unsigned long>;
#ifdef WIN_API
template class IntValidator<int64_t>;
template class IntValidator<uint64_t>;
#endif
template class IntValidator<Int64>;
template class IntValidator<UInt64>;
