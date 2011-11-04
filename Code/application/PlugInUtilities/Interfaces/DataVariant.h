/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAVARIANT_H
#define DATAVARIANT_H

#include <string>
#include <typeinfo>
#include <vector>

#include "AppVerify.h"
#include "AppConfig.h"
#include "DataVariantValidator.h"
#include "EnumWrapper.h"
#include "XercesIncludes.h"

class DataValueWrapper;

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;
class XMLWriter;

/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<unsigned char> {};
template <> class VariantTypeValidator<signed char> {};
template <> class VariantTypeValidator<char> {};
template <> class VariantTypeValidator<unsigned short> {};
template <> class VariantTypeValidator<short> {};
template <> class VariantTypeValidator<unsigned int> {};
template <> class VariantTypeValidator<int> {};
template <> class VariantTypeValidator<unsigned long> {};
template <> class VariantTypeValidator<long> {};
#ifdef WIN_API
template <> class VariantTypeValidator<uint64_t> {};
template <> class VariantTypeValidator<int64_t> {};
#endif
template <> class VariantTypeValidator<float> {};
template <> class VariantTypeValidator<double> {};
template <> class VariantTypeValidator<bool> {};
template <> class VariantTypeValidator<std::string> {};
template <> class VariantTypeValidator<std::vector<unsigned char> > {};
template <> class VariantTypeValidator<std::vector<signed char> > {};
template <> class VariantTypeValidator<std::vector<char> > {};
template <> class VariantTypeValidator<std::vector<unsigned short> > {};
template <> class VariantTypeValidator<std::vector<short> > {};
template <> class VariantTypeValidator<std::vector<unsigned int> > {};
template <> class VariantTypeValidator<std::vector<int> > {};
template <> class VariantTypeValidator<std::vector<unsigned long> > {};
template <> class VariantTypeValidator<std::vector<long> > {};
#ifdef WIN_API
template <> class VariantTypeValidator<std::vector<uint64_t> > {};
template <> class VariantTypeValidator<std::vector<int64_t> > {};
#endif
template <> class VariantTypeValidator<std::vector<float> > {};
template <> class VariantTypeValidator<std::vector<double> > {};
template <> class VariantTypeValidator<std::vector<bool> > {};
template <> class VariantTypeValidator<std::vector<std::string> > {};
/// \endcond

/**
 *  A type-safe variant type that allows variants to be passed by value.
 *  The DataVariant class is a type-safe alternative to void pointers.
 *
 *  @warning For serialization and deserialization, use of \c long, \c unsigned \c long,
 *  \c int64_t, and \c uint64_t and their associated \c vector types is strongly discouraged
 *  due to cross-platform compatibility issues. Use Int64 or UInt64 instead.
 */
class DataVariant
{
public:
   enum StatusEnum
   {
      SUCCESS, /**< The conversion succeeded. */
      FAILURE, /**< The conversion failed. */
      NOT_SUPPORTED /**< The given conversion is not supported by the type contained in the DataVariant. */
   };

   /**
    * @EnumWrapper DataVariant::StatusEnum.
    */
   typedef EnumWrapper<StatusEnum> Status;

   /**
    * An exception that indicates that the type
    * contained with the DataVariant does not support
    * the given operation.
    */
   class UnsupportedOperation : public std::exception
   {
   public:
      /**
       * Constructor.
       *
       * @param message
       *        The string used to provide additional information about
       *        why the operation was not supported.
       */
      explicit UnsupportedOperation(const std::string& message) : std::exception(), mMessage() {}

      /**
       * Destructor.
       */
      virtual ~UnsupportedOperation() throw() {}

      /**
       * Query for additional information.
       *
       * @return Returns a string providing additional information on why
       *         the operation was unsupported.
       */
      const char* what() const throw()
      {
         return mMessage.c_str();
      }
   private:
      std::string mMessage;
   };

   /**
    *  Default constructor. Creates an empty variant object.
    */
   DataVariant() :
      mpValue(NULL)
   {
   }

   /**
    *  Constructor. 
    *
    *  It performs a deep copy on the value and stores the result
    *  in the variant. The deep copy is performed by the DataVariantFactory. 
    *  The valid types are as follows:
    *  - unsigned char
    *  - signed char
    *  - char
    *  - unsigned short
    *  - short
    *  - unsigned int
    *  - int
    *  - unsigned long
    *  - long
    *  - uint64_t (Windows only)
    *  - int64_t (Windows only)
    *  - UInt64
    *  - Int64
    *  - float
    *  - double
    *  - bool
    *  - std::string
    *  - std::vector<unsigned char>
    *  - std::vector<signed char>
    *  - std::vector<char>
    *  - std::vector<unsigned short>
    *  - std::vector<short>
    *  - std::vector<unsigned int>
    *  - std::vector<int>
    *  - std::vector<unsigned long>
    *  - std::vector<long>
    *  - std::vector<uint64_t> (Windows only)
    *  - std::vector<int64_t> (Windows only)
    *  - std::vector<UInt64>
    *  - std::vector<Int64>
    *  - std::vector<float>
    *  - std::vector<double>
    *  - std::vector<bool>
    *  - std::vector<std::string>
    *  - #AnimationCycle
    *  - #AnimationState
    *  - #ArcRegion
    *  - ColorType
    *  - #ComplexComponent
    *  - #DataOrigin
    *  - DateTime (const and non-const)
    *  - #DisplayMode
    *  - #DistanceUnits
    *  - #DmsFormatType
    *  - DynamicObject (const and non-const)
    *  - #EncodingType
    *  - #EndianType
    *  - Filename (const and non-const)
    *  - std::vector<Filename*> (const and non-const pointers)
    *  - #FillStyle
    *  - FloatComplex
    *  - #GcpSymbol
    *  - #GeocoordType
    *  - #GraphicObjectType
    *  - #InsetZoomMode
    *  - IntegerComplex
    *  - #InterleaveFormatType
    *  - #LatLonStyle
    *  - #LayerType
    *  - #LineStyle
    *  - #LocationType
    *  - #PanLimitType
    *  - #PassArea
    *  - #PlotObjectType
    *  - Point::PointSymbolType
    *  - #PositionType
    *  - #ProcessingLocation
    *  - #SessionSaveType
    *  - #RasterChannelType
    *  - #RegionUnits
    *  - #ReleaseType
    *  - #StretchType
    *  - #SymbolType
    *  - #UnitSystem
    *  - #UnitType
    *  - #WavelengthUnitsType
    *  - #WindowSizeType
    *  - #WindowType
    *
    *  @param   value
    *           The value to copy into the variant. If the type of value isn't
    *           one of the supported types, an empty variant is created. In the
    *           case of pointers, a deep copy of the object pointed to is
    *           performed, unless the pointer is NULL, in which case an empty
    *           variant is created.
    */
#if defined(WIN_API)
#pragma warning( push )
#pragma warning( disable: 4101 )
#endif
   template <typename T>
   DataVariant(const T& value) :
      mpValue(NULL)
   {
      VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;

      mpValue = createWrapper(static_cast<const void*>(&value), typeid(T).name());
   }

   /**
    *  Copy Constructor. It performs a deep copy on the value and stores the result
    *  in the new variant.
    *
    *  @param   value
    *       The source variant to copy. If the source variant is empty, the new
    *       copy will also be empty.
    */
   DataVariant(const DataVariant& value);

   /**
    *  Constructor. 
    *
    *  It performs a deep copy on the value and stores the result
    *  in the variant. The deep copy is performed by the DataVariantFactory.
    *  
    *  @param   type
    *       The type of the object to wrap in the variant. The format of the 
    *       string should match that used by getTypeName().
    *
    *  @param   pValue
    *       A pointer to the object to copy into the variant. If this is NULL,
    *       a default initialized object of the specified type will be created.
    *
    *  @param   strict
    *       If true, a verification error message will be generated if
    *       a DataVariant of the given type cannot be constructed.  If
    *       false, no verification error message will be generated.
    */
   DataVariant(const std::string& type, const void *pValue, bool strict = true) :
      mpValue(NULL)
   {
      mpValue = createWrapper(pValue, type.c_str(), strict);
   }

   /**
    *  Destructor. Deletes the wrapped object.
    */
   ~DataVariant();

   /**
    *  Indicates if the DataVariant is empty or not. On an empty variant, any
    *  attempt to extract its value will fail.
    *
    *  @return  true if the variant contains a wrapped object, or false otherwise
    */
   bool isValid() const;

   /**
    *  Gets the type of the object wrapped in the variant.
    *
    *  @return  The type_info of the wrapped object, if the variant is not empty,
    *           or typeid(void) otherwise.
    */
   const std::type_info &getType() const;

   /**
    *  Gets the name of the type of the object wrapped in the variant. The 
    *  format of the string will match that of TypeConverter. If T is a value
    *  type, it will be \#T. For example if T is vector<int> it will return 
    *  "vector<int>". If T is a pointer type, it will strip the '*' from the
    *  type before creating the string. For example, if T is Filename*, it will
    *  return "Filename". If T is vector<Filename*> it will return
    *  "vector<Filename>".
    *
    *  @return  The name of the type of the object or "void" if invalid.
    */
   std::string getTypeName() const;

   /**
    *  Converts the value in the variant to a string.
    *
    *  @param   pStatus
    *           If this value is non-NULL, it will be populated
    *           with a value of DataVariant::SUCCESS,
    *           DataVariant::FAILURE or DataVariant::NOT_SUPPORTED
    *           depending on the result of the toXmlString operation.
    *
    *  @return  The value in the variant converted to a string. If the variant
    *           is invalid, an empty string will be returned. If pStatus
    *           is set to DataVariant::FAILURE or DataVariant::NOT_SUPPORTED,
    *           empty string will be returned.
    */
   std::string toXmlString(Status* pStatus = NULL) const;

   /**
    *  Initializes the variant from input text.
    *
    *  @param   type
    *           The type of object represented by the text.
    *
    *  @param   text
    *           The textual representation of the object.
    *
    *  @return  Returns DataVariant::SUCCESS if the variant was successfully
    *           initialized from the text, DataVariant::FAILURE if the variant
    *           was not successfully initialized from the
    *           text or DataVariant::NOT_SUPPORTED if the given type does not support
    *           being initialized from text.
    */
   Status fromXmlString(const std::string &type, const std::string &text);

   /**
    *  Converts the value in the variant to a string.
    *
    *  @param   pStatus
    *           If this value is non-NULL, it will be populated
    *           with a value of DataVariant::SUCCESS,
    *           DataVariant::FAILURE or DataVariant::NOT_SUPPORTED
    *           depending on the result of the toXmlString operation.
    *
    *  @return  The value in the variant converted to a string. If the variant
    *           is invalid, an empty string will be returned. If pStatus
    *           is set to DataVariant::FAILURE or DataVariant::NOT_SUPPORTED,
    *           empty string will be returned.
    */
   std::string toDisplayString(Status* pStatus = NULL) const;

   /**
    *  Initializes the variant from input text.
    *
    *  @param   type
    *           The type of object represented by the text.
    *
    *  @param   text
    *           The textual representation of the object.
    *
    *  @return  Returns DataVariant::SUCCESS if the variant was successfully
    *           initialized from the text, DataVariant::FAILURE if the variant
    *           was not successfully initialized from the
    *           text or DataVariant::NOT_SUPPORTED if the given type does not support
    *           being initialized from text.
    */
   Status fromDisplayString(const std::string &type, const std::string &text);

   /**
    *  Converts the contents of this object to XML data.
    *
    *  @param   pWriter
    *           Pointer to an XMLWriter object in which the object's contents
    *           are written.
    *
    *  @throw   XmlBase::XmlException
    *           This exception (or a subclass) is thrown if there is a problem
    *           serializing the object.
    *
    *  @return  Returns true if the object was successfully converted to XML
    *           data.  Returns false if the object cannot be represented in
    *           XML format.
    */
   bool toXml(XMLWriter* pWriter) const;

   /**
    *  Sets the contents of this object from given XML data.
    *
    *  @param   pDocument
    *           The Xerces DOM node.
    *
    *  @param   version
    *           This is the version of the XML which is being deserialized.
    *
    *  @throw   XmlBase::XmlException
    *           This exception (or a subclass) is thrown if there is a problem
    *           deserializing the object.
    *
    *  @return  Returns true if the object's data values were successfully
    *           set from the given XML data.  Returns false if the object
    *           cannot be represented in XML format.
    */
   bool fromXml(DOMNode* pDocument, unsigned int version);

   /**
    *  Extracts the value of the variant.
    *
    *  If the type of the supplied argument matches the type of the wrapped
    *  object, the method extracts the value of the wrapped object into the
    *  supplied argument.
    *
    *  @code
    *  int i = 5;
    *  DataVariant variant(i);
    *  int j = 0;
    *  bool success = variant.getValue(j); 
    *  // j == 5 and success == true at this point
    *  float f = 0.0;
    *  success = variant.getValue(f);
    *  // f == 0.0 and success == false at this point
    *  @endcode
    *
    *  @return  true if the value was successfully extracted, or false otherwise.
    *         If the variant is empty, this will always return false.
    */
   template <typename T>
   bool getValue(T& value) const
   {
      VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
      T* pValue = const_cast<DataVariant*>(this)->getPointerToValue<T>();
      if (pValue != NULL)
      {
         value = *pValue;
         return true;
      }

      return false;
   }

   /**
    *  Returns a pointer to the wrapped object.
    *
    *  If the type used in calling the method matches the type of the wrapped
    *  object, the method returns a pointer to the wrapped object.
    *
    *  @code
    *  int i = 5;
    *  DataVariant variant(i);
    *  int *pIntValue = variant.getPointerToValue<int>(); 
    *  int j = pIntValue ? *pIntValue : 0;
    *  // j == 5 at this point;
    *  float *pFloatValue = variant.getPointerToValue<float>();
    *  // pFloatValue == NULL at this point
    *  @endcode
    *
    *  @return  A pointer to the wrapped value if the type used in calling the
    *         method matches the type of the wrapped object, or NULL otherwise.
    *         If the variant is empty, this will always return NULL.
    */
   template <typename T>
   const T* getPointerToValue() const
   {
      VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
      if (getType() == typeid(T))
      {
         return reinterpret_cast<T*> (getPointerToValueAsVoid());
      }

      return NULL;
   }

   /**
    *  Returns a pointer to the wrapped object.
    *
    *  If the type used in calling the method matches the type of the wrapped
    *  object, the method returns a pointer to the wrapped object.
    *
    *  @code
    *  int i = 5;
    *  DataVariant variant(i);
    *  int *pIntValue = variant.getPointerToValue<int>(); 
    *  int j = pIntValue ? *pIntValue : 0;
    *  // j == 5 at this point;
    *  float *pFloatValue = variant.getPointerToValue<float>();
    *  // pFloatValue == NULL at this point
    *  @endcode
    *
    *  @return  A pointer to the wrapped value if the type used in calling the
    *         method matches the type of the wrapped object, or NULL otherwise.
    *         If the variant is empty, this will always return NULL.
    */
   template <typename T>
   T* getPointerToValue()
   {
      VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
      return const_cast<T*>(const_cast<const DataVariant*>(this)->getPointerToValue<T>());
   }

   /**
    *  Assignment operator. 
    *
    *  Performs a deep copy of the object
    *  on the right-hand side of the = into the variant on the left-hand side of
    *  the =. If the variant on the left-hand side is not empty prior to the
    *  assignment, its existing wrapped object will be deleted.
    *
    *  @code
    *  int i = 5;
    *  float f = 6.0;
    *  DataVariant variant(i);
    *  variant = f;
    *  i = 0;
    *  bool success = variant.getValue(i);
    *  // success == false and i == 0 at this point
    *  float g = 0.0;
    *  success = variant.getValue(g);
    *  // success == true and g == 6.0 at this point
    *  @endcode
    *
    *  @param   rhs
    *         The object on the right-hand side of the assignment
    *
    *  @return  A reference to the variant on the left-hand side of the assignment.
    */
   template<typename ValueType>
   DataVariant & operator=(const ValueType & rhs)
   {
      DataVariant(rhs).swap(*this);
      return *this;
   }

   /**
    *  Swaps the internal contents of two variants.
    *
    *  @param   rhs
    *         The variant to swap
    *
    *  @return  A reference to the variant on which the method was invoked.
    */
   DataVariant& swap(DataVariant & rhs);

   /**
    *  Assignment operator. 
    *
    *  Performs a deep copy of the contents of the variant
    *  on the right-hand side of the = into the variant on the left-hand side of
    *  the =. If the variant on the left-hand side is not empty prior to the
    *  assignment, its existing wrapped object will be deleted.
    *
    *  @param   rhs
    *         The variant on the right-hand side of the assignment
    *
    *  @return  A reference to the variant on the left-hand side of the assignment.
    */
   DataVariant & operator=(const DataVariant & rhs);

   /**
    *  Comparison operator. 
    *
    *  Compares a %DataVariant object with a value object. It will return true 
    *  if the types are the same and the internal value of the DataVariant compares 
    *  equal with the value object and false otherwise. If the types are the 
    *  same but the type doesn't support comparison, DataVariant::UnsupportedOperation will be 
    *  thrown.
    *
    *  @param   rhs
    *         The value object to compare with
    *
    *  @return  true if the types are the same and the internal value of the 
    *         DataVariant compares equal with the value object and false 
    *         otherwise.
    */
   template<typename ValueType>
   bool operator==(const ValueType & rhs) const
   {
      VariantTypeValidator<ValueType> validator HIDE_UNUSED_VARIABLE_WARNING;
      DataVariant rhsVariant(rhs);
      return *this == rhsVariant;
   }

   /**
    *  Comparison operator. 
    *
    *  Compares two %DataVariant objects. It will return true if the types are
    *  the same and the internal values compare equal and false otherwise. If
    *  the types are the same but the type doesn't support comparison, 
    *  DataVariant::UnsupportedOperation will be thrown.
    *
    *  @param   rhs
    *         The variant to compare with
    *
    *  @return  true if the types are the same and the internal values compare 
    *         equal and false otherwise.
    */
   bool operator==(const DataVariant& rhs) const;

   /**
    *  Returns a pointer to the wrapped object.
    *
    *  @return  A pointer to the wrapped value. If the variant is empty, this 
    *          will return NULL.
    */
   void *getPointerToValueAsVoid() const;

private:
   DataValueWrapper* mpValue;

   DataValueWrapper *createWrapper(const void *pValue, const char *pTypeName, bool strict = true) const;
};

/**
 *  Extracts an object from a variant.
 *
 *  @code
 *  Filename *pFilename = (Filename*)pObjFact->createObject("Filename");
 *  DataVariant variant(pFilename);
 *  const Filename *pFilename2 = NULL;
 *  const DateTime *pDateTime = NULL;
 *  int i = 5;
 *  DataVariant ivariant(i);
 *  j = 0;
 *  try
 *  {
 *     j = dv_cast<int>(ivariant);
 *     pFilename2 = &dv_cast<Filename>(variant);
 *     pDateTime = &dv_cast<DateTime>(variant); // will throw
 *  }
 *  catch (std::bad_cast &e)
 *  {
 *  }
 *  // j == 5 at this point
 *  // pFilename2 != NULL at this point
 *  // pDateTime == NULL at this point
 *  @endcode
 *
 *  @param   variant
 *         The variant from which to extract
 *
 *  @throw   std::bad_cast
 *         If the type of the 'cast' and the type of the DataVariant do not
 *         match, this method will throw std::bad_cast.
 *
 *  @return  A reference to the wrapped object. 
 *
 *  @relates DataVariant
 */
template<typename T>
const T &dv_cast(const DataVariant &variant)
{
   VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
   const T* pT = variant.getPointerToValue<T>();
   if (pT == NULL)
   {
      throw std::bad_cast();
   }
   return *pT;
}

/**
 *  Extracts an object from a variant.
 *
 *  @code
 *  Filename *pFilename = (Filename*)pObjFact->createObject("Filename");
 *  DataVariant variant(pFilename);
 *  const Filename *pFilename2 = NULL;
 *  const DateTime *pDateTime = NULL;
 *  int i = 5;
 *  DataVariant ivariant(i);
 *  j = 0;
 *  try
 *  {
 *     j = dv_cast<int>(ivariant);
 *     pFilename2 = &dv_cast<Filename>(variant);
 *     pDateTime = &dv_cast<DateTime>(variant); // will throw
 *  }
 *  catch (std::bad_cast &e)
 *  {
 *  }
 *  // j == 5 at this point
 *  // pFilename2 != NULL at this point
 *  // pDateTime == NULL at this point
 *  @endcode
 *
 *  @param   variant
 *         The variant from which to extract
 *
 *  @throw   std::bad_cast
 *         If the type of the 'cast' and the type of the DataVariant do not
 *         match, this method will throw std::bad_cast.
 *
 *  @return  A reference to the wrapped object. 
 *
 *  @relates DataVariant
 */
template<typename T>
T &dv_cast(DataVariant &variant)
{
   VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
   T* pT = variant.getPointerToValue<T>();
   if (pT == NULL)
   {
      throw std::bad_cast();
   }
   return *pT;
}

/**
 *  Extracts an object from a variant.
 *
 *  @code
 *  int i = 5;
 *  DataVariant ivariant(i);
 *  int j, l;
 *  double k;
 *  j = dv_cast_with_verification<int>(ivariant);
 *  k = dv_cast_with_verification<double>(ivariant, 10.0);
 *  l = dv_cast_with_verification<int>(ivariant, 25);
 *  // j == 5 at this point
 *  // k == 10.0 at this point, because ivariant is of type int, not type double so default value used.
 *  // l == 5 at this point, because ivariant is valid and of type int, so default value ignored.
 *  @endcode
 *
 *  @param variant
 *         The variant from which to extract
 *  @param defaultValue
 *         The default value to return from dv_cast_with_verification if the variant provided is invalid
 *         or if the type of the 'cast' and the type of the DataVariant do not match.
 *         If the dv_cast_with_verification uses the default value it is considered a defect in the code
 *         calling dv_cast_with_verification and therefore a VERIFY message will be logged
 *         to the message log.
 *
 *  @return  A copy of the type requested.
 *
 *  @relates DataVariant
 */
template<typename T>
T dv_cast_with_verification(const DataVariant &variant, const T& defaultValue)
{
   VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
   T retValue = defaultValue;
   bool success = variant.getValue(retValue);
   if (!success)
   {
      VERIFYNR_MSG(false, std::string("Invalid dv_cast when provided variant is of type " +
         variant.getTypeName()).c_str());
   }
   return retValue;
}

/**
 *  Extracts an object from a variant.
 *
 *  @code
 *  int i = 5;
 *  DataVariant ivariant(i);
 *  int j, l;
 *  double k;
 *  j = dv_cast<int>(ivariant);
 *  k = dv_cast<double>(ivariant, 10.0);
 *  l = dv_cast<int>(ivariant, 25);
 *  // j == 5 at this point
 *  // k == 10.0 at this point, because ivariant is of type int, not type double so default value used.
 *  // l == 5 at this point, because ivariant is valid and of type int, so default value ignored.
 *  @endcode
 *
 *  @param variant
 *         The variant from which to extract
 *  @param defaultValue
 *         The default value to return from dv_cast if the variant provided is invalid
 *         or if the type of the 'cast' and the type of the DataVariant do not match.
 *         Unlike dv_cast_with_verification(), if the dv_cast uses the default value it is not
*          considered a defect in the code.
 *
 *  @return  A copy of the type requested.
 *
 *  @relates DataVariant
 *  @see dv_cast_with_verification
 */
template<typename T>
T dv_cast(const DataVariant &variant, const T& defaultValue)
{
   VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
   T retValue = defaultValue;
   variant.getValue(retValue);
   return retValue;
}

/**
 *  Extracts a pointer to the object wrapped in a variant.
 *
 *  @code
 *  int i = 5;
 *  DataVariant variant(i);
 *  DataVariant *pVariant(&variant);
 *  int *pInt = dv_cast<int>(pVariant);
 *  int j = pInt?*pInt:0;
 *  float *pFloat = dv_cast<float>(pVariant);
 *  // j == 5 at this point;
 *  // pFloat == NULL at this point
 *  @endcode
 *
 *  @param   pVariant
 *         A pointer to the variant from which to extract
 *
 *  @return  A pointer to the wrapped object or NULL if the
 *         types don't match.
 *
 *  @relates DataVariant
 */
template<typename T>
const T* dv_cast(const DataVariant *pVariant)
{
   VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
   if (pVariant == NULL)
   {
      return NULL;
   }
   return pVariant->getPointerToValue<T>();
}

/**
 *  Extracts a pointer to the object wrapped in a variant.
 *
 *  @code
 *  int i = 5;
 *  DataVariant variant(i);
 *  DataVariant *pVariant(&variant);
 *  int *pInt = dv_cast<int>(pVariant);
 *  int j = pInt?*pInt:0;
 *  float *pFloat = dv_cast<float>(pVariant);
 *  // j == 5 at this point;
 *  // pFloat == NULL at this point
 *  @endcode
 *
 *  @param   pVariant
 *         A pointer to the variant from which to extract
 *
 *  @return  A pointer to the wrapped object or NULL if the
 *         types don't match.
 *
 *  @relates DataVariant
 */
template<typename T>
T* dv_cast(DataVariant *pVariant)
{
   VariantTypeValidator<T> validator HIDE_UNUSED_VARIABLE_WARNING;
   if (pVariant == NULL)
   {
      return NULL;
   }
   return pVariant->getPointerToValue<T>();
}
#if defined(WIN_API)
#pragma warning( pop )
#endif

#endif
