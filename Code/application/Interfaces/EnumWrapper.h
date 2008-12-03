/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENUMWRAPPER_H
#define ENUMWRAPPER_H

/**
 * \page EnumWrapperHowTo How To Use EnumWrapper
 * The EnumWrapper is a templated class that
 * allows enumeration values to be used safely.
 * The EnumWrapper class contains both the enumerated
 * value being wrapped and a boolean that indicates
 * whether the contained enumerated value is valid and
 * can be used.  The following examples show usage
 * of the class:
 *
 * \code
 * enum TestEnumeration { ENUM_VALUE1, ENUM_VALUE2, ENUM_VALUE2 };
 * EnumWrapper<TestEnumeration> test1;
 * test1.isValid(); //this will return false
 * //The following line is bad, it will convert EnumWrapper to TestEnumeration
 * //but the enumVal1 value will be undefined since the EnumWrapper.isValid()
 * //method returns false.
 * TestEnumeration enumVal1 = test1;
 *
 * test1 < ENUM_VALUE1; //false because test1 is invalid
 * ENUM_VALUE1 < test1; //true because test1 is invalid
 *
 * test1 == ENUM_VALUE1; //false because test1 is invalid
 * ENUM_VALUE1 == test1; //false because test1 is invalid
 *
 * EnumWrapper<TestEnumeration> test2(ENUM_VALUE1);
 * test2.isValid(); //this will return true
 * TestEnumeration enumVal2 = test2; //enumVal2 == ENUM_VALUE1;
 *
 * EnumWrapper<TestEnumeration> test3(test2);
 * TestEnumeration enumVal3 = test3; //enumVal3 == ENUM_VALUE1;
 *
 * EnumWrapper<TestEnumeration> test4;
 * EnumWrapper<TestEnumeration> test5 = ENUM_VALUE2;
 * TestEnumeration enumVal4 = test5; //enumVal4 == ENUM_VALUE2;
 * test4.isValid(); //this will return false
 * test4 = test5;
 * test4.isValid(); //this will return true
 * TestEnumeration enumVal5 = test4; //enumVal5 = ENUM_VALUE2;
 *
 * //It is generally recommended to create a typedef for your specific
 * //usage of EnumWrapper, as shown below
 *
 * typedef EnumWrapper<TestEnumeration> TestEnum;
 *
 * //You can then do the following:
 *
 * bool testEnumValue(TestEnum val)
 * {
 *    if (!val.isValid())
 *    {
 *       return false;
 *    }
 *    if ((val == ENUM_VALUE1) || (val == ENUM_VALUE3))
 *    {
 *       return true;
 *    }
 *    return false;
 * }
 *
 * TestEnum getValue()
 * {
 *    return ENUM_VALUE2;
 * }
 *
 * TestEnum getInvalidValue()
 * {
 *    TestEnum retVal;
 *    return retVal;
 * }
 *
 * testEnumValue(ENUM_VALUE1); //the function returns true
 * testEnumValue(ENUM_VALUE2); //the function returns false
 * TestEnum test6 = ENUM_VALUE3;
 * testEnumValue(test6); //the function returns true
 * TestEnum test7;
 * testEnumValue(test7); //the function returns false because test7.isValid() returns false
 *
 * TestEnum test8 = getValue();
 * (test8 == ENUM_VALUE2); //evaluates to true
 * test8.isValid(); //returns true
 *
 * TestEnum test9 = getInvalidValue();
 * test9.isValid(); //returns false
 * TestEnumeration enumVal6 = test9; //DON'T DO THIS.  The value of enumVal6 is undefined.
 *
 * \endcode
 */

/**
 * \cond INTERNAL
 * This class is protected by the INTERNAL condition which hides the documentation of this class.
 * This is being done because in version 1.5.1-p1 of Doxygen any links to typedefs using
 * this templated class would point directly to the documentation of this class, but would
 * not show the template argument used in the typedef.  For instance, if the following
 * typedef were present:
 *
 * typedef EnumWrapper<SymbolTypeEnum> SymbolType
 *
 * Then when a user clicked on SymbolType in the doxygen generated documentation they
 * would be taken to the EnumWrapper class documentation, but with no indicator that
 * SymbolTypeEnum was the template argument.  In order to fix, this the documentation
 * of this class is hidden using the condition support of Doxygen.  When this is done
 * when clicking on SymbolType, the user will be taken to the documentation of the typedef
 * statement which does show the user both the use of the EnumWrapper class and the
 * template argument to the EnumWrapper class.
 */
template<typename EnumValue>
class EnumWrapper
{
public:
   /**
    * The enumerated type this EnumWapper is wrapping.  Use this when
    * the raw enumerated type is required.
    */
   typedef EnumValue EnumType;

   /**
    * Default construct so that isValid() will return false.
    */
   EnumWrapper() : mIsValid(false), mValue()
   {
   }

   /**
    * Construct an instance with the given wrapped enumeration value.
    * Calling isValid() will return true.
    *
    * @param value
    *        the enumerated value to be wrapped.
    */
   EnumWrapper(const EnumValue& value) : mIsValid(true), mValue(value)
   {
   }

   /**
    * Queries whether the contained enum value is
    * valid and can be used.
    * 
    * @return Returns true if the enumeration value being
    * wrapped is valid.  Returns false otherwise.
    * 
    * @see EnumWrapper()
    */
   bool isValid() const
   {
      return mIsValid;
   }

   /**
    * Converts the contained value to the
    * given enum type.
    *
    * @return Returns the wrapped enum value if isValid() would return true.
    *         Returns an undefined value if isValid() would return false.
    *
    * @warning UNDER NO CIRCUMSTANCES should you rely on a defined value being returned
    *          if isValid() would return false.
    */
   operator EnumValue() const
   {
      if (mIsValid)
      {
         return mValue;
      }
      else
      {
         //We are returning a value if the EnumWrapper is invalid
         //that is highly unlikely to be a valid value for all
         //enums.  This is to in the general case cause switch()
         //statements using an EnumWrapper to fall into the
         //default case if a call on isValid() was not made.
         //Without this, an invalid EnumWrapper would most likely
         //return 0 on a variety of compilers which would hide
         //the problem in a switch statement.
         return static_cast<EnumValue>(-1);
      }
   }

   /**
    * Determines if this EnumWrapper is equal to another.
    *
    * @param rhs
    *        The object being compared.
    * 
    * @return Returns true if both EnumWrappers are
    *         valid and their wrapped enum values are equal. Returns
    *         true if both EnumWrappers are invalid regardless of
    *         their wrapped (ie. undefined) values. Returns false in all other
    *         cases.
    */
   bool operator==(const EnumWrapper& rhs) const
   {
      return ( (mIsValid && rhs.mIsValid && mValue == rhs.mValue) ||
               (!mIsValid && !rhs.mIsValid) );
   }

   /**
    * Determines if this EnumWrapper is equal to an enum value.
    *
    * @param enumVal
    *        The value being compared.
    *
    * @return Returns true if the EnumWrapper is valid
    *         and the wrapped enum value is equal to enumVal, false
    *         otherwise.
    */
   bool operator==(const EnumValue& enumVal) const
   {
      return (mIsValid && mValue == enumVal);
   }

   /**
    * Determines if this EnumWrapper is not equal to another.
    *
    * @param rhs
    *        The object being compared.
    *
    * @return Returns true if the EnumWrappers are not
    * equal, false otherwise.
    *
    * @see operator==()
    */
   bool operator!=(const EnumWrapper& rhs) const
   {
      return !(operator==(rhs));
   }

   /**
    * Determines if this EnumWrapper is not equal to an enum value.
    *
    * @param enumVal
    *        The value being compared.
    *
    * @return Returns true if the EnumWrapper is valid
    *         and the wrapped enum value is not equal to than enumVal, false
    *         otherwise.
    */
   bool operator!=(const EnumValue& enumVal) const
   {
      return !(operator==(enumVal));
   }

   /**
    * Determines if this EnumWrapper is less than another.
    *
    * @param rhs
    *        The object being compared.
    * 
    * @return Returns true if both EnumWrappers are
    *         valid and the wrapped enum value is less than the
    *         wrapped enum value of rhs.  Returns true
    *         if this EnumWrapper is valid, but the rhs is invalid.
    *         Returns false otherwise.
    */
   bool operator<(const EnumWrapper& rhs) const
   {
      if (mIsValid)
      {
         if (rhs.mIsValid)
         {
            return mValue < rhs.mValue;
         }
         else
         {
            return true; //valid < invalid == true
         }
      }
      else
      {
         if (rhs.mIsValid)
         {
            return false; //invalid < valid == false
         }
         else
         {
            return false; //invalid < invalid == false
         }
      }
   }

   /**
    * Determines if this EnumWrapper is less than an enum value.
    *
    * @param enumVal
    *        The value being compared.
    *
    * @return Returns true if the EnumWrapper is valid
    *         and the wrapped enum value is less than enumVal, false
    *         otherwise.
    */
   bool operator<(const EnumValue& enumVal) const
   {
      if (mIsValid)
      {
         return mValue < enumVal;
      }
      else
      {
         return false;
      }
   }

   /**
    * Determines if this EnumWrapper is less than or equal to another.
    *
    * @param rhs
    *        The object being compared.
    *
    * @return Returns true if the EnumWrappers are less than or
    * equal, false otherwise. 
    *
    * @see operator==(), operator<()
    */
   bool operator<=(const EnumWrapper& rhs) const
   {
      return operator<(rhs) || operator==(rhs);
   }

   /**
    * Determines if this EnumWrapper is less than or equal to an enum value.
    *
    * @param enumVal
    *        The value being compared.
    *
    * @return Returns true if the EnumWrapper is valid
    *         and the wrapped enum value is less than or equal to enumVal,
    *         false otherwise.
    */
   bool operator<=(const EnumValue& enumVal) const
   {
      return operator<(enumVal) || operator==(enumVal);
   }

   /**
    * Determines if this EnumWrapper is greater than another.
    *
    * @param rhs
    *        The object being compared.
    *
    * @return Returns true if the EnumWrappers are greater than
    * each other, false otherwise.
    *
    * @see operator==(), operator<()
    */
   bool operator>(const EnumWrapper& rhs) const
   {
      return !(operator<=(rhs));
   }

   /**
    * Determines if this EnumWrapper is greater than an enum value.
    *
    * @param enumVal
    *        The value being compared.
    *
    * @return Returns true if the EnumWrapper is valid
    *         and the wrapped enum value is greater than enumVal,
    *         false otherwise.
    */
   bool operator>(const EnumValue& enumVal) const
   {
      return !(operator<=(enumVal));
   }

   /**
    * Determines if this EnumWrapper is greater than or equal to another.
    *
    * @param rhs
    *        The object being compared.
    *
    * @return Returns true if the EnumWrappers are greater than or
    * equal, false otherwise. 
    *
    * @see operator==(), operator<()
    */
   bool operator>=(const EnumWrapper& rhs) const
   {
      return !(operator<(rhs));
   }

   /**
    * Determines if this EnumWrapper is greater than or equal to an enum value.
    *
    * @param enumVal
    *        The value being compared.
    *
    * @return Returns true if the EnumWrapper is valid
    *         and the wrapped enum value is greater than or equal to enumVal,
    *         false otherwise.
    */
   bool operator>=(const EnumValue& enumVal) const
   {
      return !(operator<(enumVal));
   }

private:
   bool mIsValid;
   EnumValue mValue;
};

/**
 * Determines if a enum value and EnumWrapper are equal.
 *
 * @param lhs
 *        The value being compared.
 * @param rhs
 *        The object being compared.
 *
 * @return Returns true if the enum value and EnumWrapper are equal.
 *
 * @see EnumWrapper::operator==()
 */
template <typename T>
bool operator==(const T& lhs, const EnumWrapper<T>& rhs)
{
   return EnumWrapper<T>(lhs) == rhs;
}

/**
 * Determines if a enum value and EnumWrapper are not equal.
 *
 * @param lhs
 *        The value being compared.
 * @param rhs
 *        The object being compared.
 *
 * @return Returns true if the enum value and EnumWrapper are not equal.
 *
 * @see EnumWrapper::operator!=()
 */
template <typename T>
bool operator!=(const T& lhs, const EnumWrapper<T>& rhs)
{
   return EnumWrapper<T>(lhs) != rhs;
}

/**
 * Determines if a enum value is less than the EnumWrapper.
 *
 * @param lhs
 *        The value being compared.
 * @param rhs
 *        The object being compared.
 *
 * @return Returns true if the enum value is less than the EnumWrapper.
 *
 * @see EnumWrapper::operator<()
 */
template <typename T>
bool operator<(const T& lhs, const EnumWrapper<T>& rhs)
{
   return EnumWrapper<T>(lhs) < rhs;
}

/**
 * Determines if a enum value is less than or equal to the EnumWrapper.
 *
 * @param lhs
 *        The value being compared.
 * @param rhs
 *        The object being compared.
 *
 * @return Returns true if the enum value is less than or equal to the EnumWrapper.
 *
 * @see EnumWrapper::operator<=()
 */
template <typename T>
bool operator<=(const T& lhs, const EnumWrapper<T>& rhs)
{
   return EnumWrapper<T>(lhs) <= rhs;
}

/**
 * Determines if a enum value is greater than the EnumWrapper.
 *
 * @param lhs
 *        The value being compared.
 * @param rhs
 *        The object being compared.
 *
 * @return Returns true if the enum value is greater than the EnumWrapper.
 *
 * @see EnumWrapper::operator>()
 */
template <typename T>
bool operator>(const T& lhs, const EnumWrapper<T>& rhs)
{
   return EnumWrapper<T>(lhs) > rhs;
}

/**
 * Determines if a enum value is greater than or equal to the EnumWrapper.
 *
 * @param lhs
 *        The value being compared.
 * @param rhs
 *        The object being compared.
 *
 * @return Returns true if the enum value is greater than or equal to the EnumWrapper.
 *
 * @see EnumWrapper::operator>=()
 */
template <typename T>
bool operator>=(const T& lhs, const EnumWrapper<T>& rhs)
{
   return EnumWrapper<T>(lhs) >= rhs;
}

/// \endcond

#endif