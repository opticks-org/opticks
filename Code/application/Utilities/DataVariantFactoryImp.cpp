/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataValueWrapper.h"
#include "DataVariant.h"
#include "DataVariantFactoryImp.h"
#include "DateTime.h"
#include "DateTimeImp.h"
#include "DynamicObjectAdapter.h"
#include "Filename.h"
#include "FilenameImp.h"
#include "Int64.h"
#include "ObjectResource.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "TypeConverter.h"
#include "UInt64.h"
#include "XercesIncludes.h"
#include "Point.h"

#include <sstream>
#include <vector>

XERCES_CPP_NAMESPACE_USE
using namespace std;
using namespace StringUtilities;

DataVariantFactoryImp* DataVariantFactoryImp::spInstance = NULL;
bool DataVariantFactoryImp::mDestroyed = false;

namespace
{
   template<typename N>
   bool fromXmlVectorHelper(DOMNode* pDocument, unsigned int version, vector<N>& values)
   {
      VERIFY(pDocument != NULL);
      values.clear();
      bool bDone = false;
      DOMNode* pVectorChild = NULL;
      for (pVectorChild = pDocument->getFirstChild(); 
         pVectorChild != NULL;
         pVectorChild = pVectorChild->getNextSibling())
      {
         bDone = XMLString::equals(pVectorChild->getNodeName(), X("vector"));
         if (bDone == true)
         {
            break;
         }
      }
      if (bDone == true && pVectorChild != NULL) // 2.3.7+
      {
         for (DOMNode* pVectorElement = pVectorChild->getFirstChild();
            pVectorElement != NULL;
            pVectorElement = pVectorElement->getNextSibling())
         {
            if (XMLString::equals(pVectorElement->getNodeName(), X("value")))
            {
               values.push_back(StringUtilities::fromXmlString<N>(A(pVectorElement->getTextContent())));
            }
         }
         return true;
      }
      else
      {
         return false;
      }
   }

   template<typename T>
   bool toXmlVectorHelper(XMLWriter* pWriter, const vector<T>& values)
   {
      DOMElement* pElement = pWriter->addElement("vector");
      VERIFY(pElement != NULL);

      pWriter->pushAddPoint(pElement);
      for (vector<T>::const_iterator vit = values.begin(); vit != values.end(); ++vit)
      {
         stringstream sstr;
         pElement = pWriter->addElement("value");
         VERIFY(pElement != NULL);
         pWriter->pushAddPoint(pElement);
         pWriter->addText(StringUtilities::toXmlString(*vit).c_str(), pElement);
         pWriter->popAddPoint();
      }
      pWriter->popAddPoint();
      return true;
   }
}

// For all value types and shallow pointers: wherever the default copy-ctor 
// and dtor are good enough.
template <typename T>
class DataVariantValue : public DataValueWrapper
{
public:
   DataVariantValue(const T* pValue)
   {
      if (pValue != NULL)
      {
         mValue = *pValue;
      }
   }

   static DataValueWrapper* createWrapper(const void* pInitialValue)
   {
      return new DataVariantValue<T>(reinterpret_cast<const T*>(pInitialValue));
   }

   static std::vector<std::string> getSupportedTypes()
   {
      std::vector<std::string> types;
      types.push_back(typeid(T).name());
      types.push_back(TypeConverter::toString<T>());
      return types;
   }

   const std::type_info& getType() const
   {
      return typeid(T);
   }

   string getTypeName() const
   {
      T* pT = NULL;
      return TypeConverter::toString(pT);
   }

   string toXmlString(DataVariant::Status* pStatus) const
   {
      bool error;
      string retValue = StringUtilities::toXmlString(mValue, &error);
      if (pStatus != NULL)
      {
         *pStatus = (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
      }
      return retValue;
   }

   DataVariant::Status fromXmlString(const std::string& text)
   {
      bool error;
      T tempValue = StringUtilities::fromXmlString<T>(text, &error);
      if (!error)
      {
         mValue = tempValue;
      }
      return (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
   }

   string toDisplayString(DataVariant::Status* pStatus) const
   {
      bool error;
      string retValue = StringUtilities::toDisplayString(mValue, &error);
      if (pStatus != NULL)
      {
         *pStatus = (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
      }
      return retValue;
   }

   DataVariant::Status fromDisplayString(const std::string& text)
   {
      bool error;
      T tempValue = StringUtilities::fromDisplayString<T>(text, &error);
      if (!error)
      {
         mValue = tempValue;
      }
      return (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
   }

   bool toXml(XMLWriter* pWriter) const
   {
      DOMElement* pElement(pWriter->addElement("value"));
      VERIFY(pElement != NULL);
      pWriter->pushAddPoint(pElement);
      pWriter->addText(toXmlString(NULL).c_str(), pElement);
      pWriter->popAddPoint();
      return true;
   }

   bool fromXml(DOMNode* pDocument, unsigned int version)
   {
      for (DOMNode* pNode = pDocument->getFirstChild();
         pNode != NULL;
         pNode = pNode->getNextSibling())
      {
         if (XMLString::equals(pNode->getNodeName(), X("value")))
         {
            string str = A(pNode->getTextContent());
            mValue = StringUtilities::fromXmlString<T>(str);
            return true;
         }
      }
      return false;
   }

   DataValueWrapper* copy() const
   {
      DataValueWrapper* pData = new DataVariantValue(&mValue);
      return pData;
   }

   void* getValue()
   {
      return &mValue;
   }

   bool operator==(const DataValueWrapper& rhs) const
   {
      if (getType() == rhs.getType())
      {
         return mValue == static_cast<const DataVariantValue<T>&>(rhs).mValue;
      }
      return false;
   }

protected:
   T mValue;
};

template <typename T>
class DataVariantVectorValue : public DataVariantValue<vector<T> >
{
public:
   DataVariantVectorValue(const vector<T>* pValue) : DataVariantValue(pValue)
   {
   }

   static DataValueWrapper* createWrapper(const void* pInitialValue)
   {
      return new DataVariantVectorValue<T>(reinterpret_cast<const vector<T>*>(pInitialValue));
   }

   bool toXml(XMLWriter* pWriter) const
   {
      return toXmlVectorHelper<T>(pWriter, mValue);
   }

   bool fromXml(DOMNode* pDocument, unsigned int version)
   {
      return fromXmlVectorHelper<T>(pDocument, version, mValue);
   }

   DataValueWrapper* copy() const
   {
      DataValueWrapper* pData = new DataVariantVectorValue<T>(&mValue);
      return pData;
   }

   bool operator==(const DataValueWrapper& rhs) const
   {
      if (getType() == rhs.getType())
      {
         return mValue == static_cast<const DataVariantVectorValue<T>&>(rhs).mValue;
      }
      return false;
   }
};

// For all deep pointers: wherever we need to use ObjectFactory to
// create a duplicate, deep-copied object. E.g. T* could be Filename*
// or DateTime* (with U* being FilenameImp* or DateTimeImp* 
// respectively).
template <typename T, typename U>
class DataVariantPointer : public DataValueWrapper
{
public:
   DataVariantPointer(const T* pValue)
   {
      const U* pSrc = dynamic_cast<const U*>(pValue);
      if (pSrc != NULL)
      {
         mValue = *pSrc;
      }
   }

   ~DataVariantPointer()
   {
   }

   static std::vector<std::string> getSupportedTypes()
   {
      std::vector<std::string> types;
      types.push_back(typeid(T).name());
      types.push_back(typeid(const T).name());
      types.push_back(TypeConverter::toString<T>());
      return types;
   }

   const std::type_info& getType() const
   {
      return typeid(T);
   }

   string getTypeName() const
   {
      T* pT = NULL;
      return TypeConverter::toString(pT);
   }

   string toXmlString(DataVariant::Status* pStatus) const
   {
      if (pStatus != NULL)
      {
         *pStatus = DataVariant::NOT_SUPPORTED;
      }
      return "";
   }

   DataVariant::Status fromXmlString(const std::string& text)
   {
      return DataVariant::NOT_SUPPORTED;
   }

   string toDisplayString(DataVariant::Status* pStatus) const
   {
      if (pStatus != NULL)
      {
         *pStatus = DataVariant::NOT_SUPPORTED;
      }
      return "";
   }

   DataVariant::Status fromDisplayString(const std::string& text)
   {
      return DataVariant::NOT_SUPPORTED;
   }

   bool toXml(XMLWriter* pWriter) const
   {
      return false;
   }

   bool fromXml(DOMNode* pDocument, unsigned int version)
   {
      return false;
   }

   void* getValue()
   {
      return dynamic_cast<T*>(&mValue);
   }

   bool operator==(const DataValueWrapper& rhs) const
   {
      string message = std::string(TypeConverter::toString<T>()) + " doesn't support comparison";
      throw DataVariant::UnsupportedOperation(message);
      return false;
   }

protected:
   U mValue;
};

template<class T, class U>
class DataVariantPointerSupportString : public DataVariantPointer<T, U>
{
public:
   DataVariantPointerSupportString(const T* pValue) : DataVariantPointer(pValue)
   {
   }

   static DataValueWrapper* createWrapper(const void* pInitialValue)
   {
      return new DataVariantPointerSupportString<T, U>(reinterpret_cast<const T*>(pInitialValue));
   }

   string toXmlString(DataVariant::Status* pStatus) const
   {
      bool error;
      string retValue = StringUtilities::toXmlString(dynamic_cast<const T*>(&mValue), &error);
      if (pStatus != NULL)
      {
         *pStatus = (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
      }
      return retValue;
   }

   DataVariant::Status fromXmlString(const std::string& text)
   {
      bool error;
      FactoryResource<T> pTempValue(StringUtilities::fromXmlString<T*>(text, &error));
      if (!error && pTempValue.get() != NULL)
      {
         mValue = *(dynamic_cast<U*>(pTempValue.get()));
      }
      return (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
   }

   string toDisplayString(DataVariant::Status* pStatus) const
   {
      bool error;
      string retValue = StringUtilities::toDisplayString(dynamic_cast<const T*>(&mValue), &error);
      if (pStatus != NULL)
      {
         *pStatus = (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
      }
      return retValue;
   }

   DataVariant::Status fromDisplayString(const std::string& text)
   {
      bool error;
      FactoryResource<T> pTempValue(StringUtilities::fromDisplayString<T*>(text, &error));
      if (!error && pTempValue.get() != NULL)
      {
         mValue = *(dynamic_cast<U*>(pTempValue.get()));
      }
      return (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
   }

   bool toXml(XMLWriter* pWriter) const
   {
      if (pWriter == NULL)
      {
         return false;
      }

      DOMElement* pElement(pWriter->addElement("value"));
      VERIFY(pElement != NULL);
      pWriter->pushAddPoint(pElement);
      string text = StringUtilities::toXmlString(dynamic_cast<const T*>(&mValue));
      pWriter->addText(text.c_str(), pElement);
      pWriter->popAddPoint();
      return true;
   }

   bool fromXml(DOMNode* pDocument, unsigned int version)
   {
      for (DOMNode* pNode = pDocument->getFirstChild();
         pNode != NULL;
         pNode = pNode->getNextSibling())
      {
         if (XMLString::equals(pNode->getNodeName(), X("value")))
         {
            string str = A(pNode->getTextContent());
            DataVariant::Status status = fromXmlString(str);
            return (status == DataVariant::SUCCESS);
         }
      }
      return false;
   }

   DataValueWrapper* copy() const
   {
      DataValueWrapper* pData = new DataVariantPointerSupportString<T, U>(&mValue);
      return pData;
   }

   bool operator==(const DataValueWrapper& rhs) const
   {
      if (getType() == rhs.getType())
      {
         return mValue == dynamic_cast<const DataVariantPointerSupportString<T, U>&>(rhs).mValue;
      }
      return false;
   }
};

class DataVariantDynamicObject : public DataVariantPointer<DynamicObject, DynamicObjectAdapter>
{
public:
   DataVariantDynamicObject(const DynamicObject* pValue) : DataVariantPointer(pValue)
   {
   }

   static DataValueWrapper* createWrapper(const void* pInitialValue)
   {
      return new DataVariantDynamicObject(reinterpret_cast<const DynamicObject*>(pInitialValue));
   }

   bool toXml(XMLWriter* pWriter) const
   {
      return mValue.toXml(pWriter);
   }

   bool fromXml(DOMNode* pDocument, unsigned int version)
   {
      return mValue.fromXml(pDocument, version);
   }

   DataValueWrapper* copy() const
   {
      DataValueWrapper* pData = new DataVariantDynamicObject(&mValue);
      return pData;
   }
};

// For all vectors of deep types. E.g. T* could be Filename* 
// (with U* being FilenameImp*).
template <typename T, typename U>
class DataVariantVectorPtr : public DataValueWrapper
{
public:
   DataVariantVectorPtr(const vector<T*>* pValue)
   {
      if (pValue != NULL)
      {
         const vector<T*>& localValue = *pValue;
         mValue.reserve(localValue.size());
         vector<T*>::const_iterator ppItem;
         for (ppItem = localValue.begin(); ppItem != localValue.end(); ++ppItem)
         {
            if (*ppItem != NULL)
            {
               U* pSrc = dynamic_cast<U*>(*ppItem);
               U* pDst = new U;
               if (pDst != NULL && pSrc != NULL)
               {
                  *pDst = *pSrc;
                  mValue.push_back(dynamic_cast<T*>(pDst));
               }
            }
         }
      }
   }

   ~DataVariantVectorPtr()
   {
      for (unsigned int i = 0; i < mValue.size(); ++i)
      {
         U* pItem = dynamic_cast<U*>(mValue[i]);
         if (pItem != NULL)
         {
            delete pItem;
         }
      }
   }

   static DataValueWrapper* createWrapper(const void* pInitialValue)
   {
      return new DataVariantVectorPtr<T, U>(reinterpret_cast<const vector<T*>*>(pInitialValue));
   }

   static std::vector<std::string> getSupportedTypes()
   {
      std::vector<std::string> types;
      types.push_back(typeid(vector<T*>).name());
      types.push_back(typeid(vector<const T*>).name());
      types.push_back(TypeConverter::toString<vector<T> >());
      return types;
   }

   const std::type_info& getType() const
   {
      return typeid(vector<T*>);
   }

   string getTypeName() const
   {
      return TypeConverter::toString(&mValue);
   }

   string toXmlString(DataVariant::Status* pStatus) const
   {
      bool error;
      string retValue = StringUtilities::toXmlString(mValue, &error);
      if (pStatus != NULL)
      {
         *pStatus = (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
      }
      return retValue;
   }

   DataVariant::Status fromXmlString(const std::string& text)
   {
      bool error;
      vector<T*> tempValue = StringUtilities::fromXmlString<vector<T*> >(text, &error);
      if (!error)
      {
         mValue = tempValue;
      }
      return (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
   }

   string toDisplayString(DataVariant::Status* pStatus) const
   {
      bool error;
      string retValue = StringUtilities::toDisplayString(mValue, &error);
      if (pStatus != NULL)
      {
         *pStatus = (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
      }
      return retValue;
   }

   DataVariant::Status fromDisplayString(const std::string& text)
   {
      bool error;
      vector<T*> tempValue = StringUtilities::fromDisplayString<vector<T*> >(text, &error);
      if (!error)
      {
         mValue = tempValue;
      }
      return (error == true ? DataVariant::FAILURE : DataVariant::SUCCESS);
   }

   bool toXml(XMLWriter* pWriter) const
   {
      return toXmlVectorHelper<T*>(pWriter, mValue);
   }

   bool fromXml(DOMNode* pDocument, unsigned int version)
   {
      return fromXmlVectorHelper<T*>(pDocument, version, mValue);
   }

   DataValueWrapper* copy() const
   {
      DataValueWrapper* pData = new DataVariantVectorPtr<T, U>(&mValue);
      return pData;
   }

   void* getValue()
   {
      return &mValue;
   }

   bool operator==(const DataValueWrapper& rhs) const
   {
      if (getType() == rhs.getType())
      {
         const vector<T*>& rhsValue = static_cast<const DataVariantVectorPtr<T, U>&>(rhs).mValue;
         if (mValue.size() != rhsValue.size())
         {
            return false;
         }
         vector<T*>::const_iterator ppT, ppTrhs;
         for (ppT = mValue.begin(), ppTrhs = rhsValue.begin(); 
            ppT != mValue.end() && ppTrhs != rhsValue.end(); 
            ++ppT, ++ppTrhs)
         {
            if (*dynamic_cast<U*>(*ppT) != *dynamic_cast<U*>(*ppTrhs))
            {
               return false;
            }
         }
         return true;
      }
      return false;
   }

private:
   vector<T*> mValue;
};

void DataVariantFactoryImp::registerType(const std::string type, WrapperCreatorProc creationFunc)
{
   mCreateWrapperMap.insert(ObjectMapType3::value_type(type, creationFunc));
}

void DataVariantFactoryImp::initializeMaps()
{
   registerType<DataVariantVectorPtr<Filename, FilenameImp> >();
   registerType<DataVariantPointerSupportString<DateTime, DateTimeImp> >();
   registerType<DataVariantPointerSupportString<Filename, FilenameImp> >();
   registerType<DataVariantDynamicObject>();
   registerType<DataVariantValue<char> >();
   registerType<DataVariantVectorValue<char> >();
   registerType<DataVariantValue<unsigned char> >();
   registerType<DataVariantVectorValue<unsigned char> >();
   registerType<DataVariantValue<short> >();
   registerType<DataVariantVectorValue<short> >();
   registerType<DataVariantValue<unsigned short> >();
   registerType<DataVariantVectorValue<unsigned short> >();
   registerType<DataVariantValue<int> >();
   registerType<DataVariantVectorValue<int> >();
   registerType<DataVariantValue<unsigned int> >();
   registerType<DataVariantVectorValue<unsigned int> >();
   registerType<DataVariantValue<Int64> >();
   registerType<DataVariantVectorValue<Int64> >();
   registerType<DataVariantValue<UInt64> >();
   registerType<DataVariantVectorValue<UInt64> >();
   registerType<DataVariantValue<int64_t> >();
   registerType<DataVariantVectorValue<int64_t> >();
   registerType<DataVariantValue<uint64_t> >();
   registerType<DataVariantVectorValue<uint64_t> >();
   registerType<DataVariantValue<long> >();
   registerType<DataVariantVectorValue<long> >();
   registerType<DataVariantValue<unsigned long> >();
   registerType<DataVariantVectorValue<unsigned long> >();
   registerType<DataVariantValue<float> >();
   registerType<DataVariantVectorValue<float> >();
   registerType<DataVariantValue<double> >();
   registerType<DataVariantVectorValue<double> >();
   registerType<DataVariantValue<bool> >();
   registerType<DataVariantVectorValue<bool> >();
   registerType<DataVariantValue<string> >();
   registerType<DataVariantVectorValue<string> >();

   registerType<DataVariantValue<AnimationCycle> >();
   registerType<DataVariantValue<AnimationState> >();
   registerType<DataVariantValue<ArcRegion> >();
   registerType<DataVariantValue<ColorType> >();
   registerType<DataVariantValue<ComplexComponent> >();
   registerType<DataVariantValue<DataOrigin> >();
   registerType<DataVariantValue<DisplayMode> >();
   registerType<DataVariantValue<DistanceUnits> >();
   registerType<DataVariantValue<DmsFormatType> >();
   registerType<DataVariantValue<EncodingType> >();
   registerType<DataVariantValue<EndianType> >();
   registerType<DataVariantValue<FillStyle> >();
   registerType<DataVariantValue<GcpSymbol> >();
   registerType<DataVariantValue<GeocoordType> >();
   registerType<DataVariantValue<GraphicObjectType> >();
   registerType<DataVariantValue<InsetZoomMode> >();
   registerType<DataVariantValue<InterleaveFormatType> >();
   registerType<DataVariantValue<LatLonStyle> >();
   registerType<DataVariantValue<LayerType> >();
   registerType<DataVariantValue<LineStyle> >();
   registerType<DataVariantValue<LocationType> >();
   registerType<DataVariantValue<PanLimitType> >();
   registerType<DataVariantValue<PassArea> >();
   registerType<DataVariantValue<PlotObjectType> >();
   registerType<DataVariantValue<PositionType> >();
   registerType<DataVariantValue<ProcessingLocation> >();
   registerType<DataVariantValue<RasterChannelType> >();
   registerType<DataVariantValue<SessionSaveType> >();
   registerType<DataVariantValue<ReleaseType> >();
   registerType<DataVariantValue<StretchType> >();
   registerType<DataVariantValue<SymbolType> >();
   registerType<DataVariantValue<Point::PointSymbolType> >();
   registerType<DataVariantValue<RegionUnits> >();
   registerType<DataVariantValue<UnitType> >();
   registerType<DataVariantValue<WindowSizeType> >();
   registerType<DataVariantValue<WindowType> >();
}

DataVariantFactoryImp::~DataVariantFactoryImp()
{
}

DataVariantFactoryImp* DataVariantFactoryImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use DataVariantFactory after "
            "destroying it.");
      }
      spInstance = new DataVariantFactoryImp;
   }
   return spInstance;
}

void DataVariantFactoryImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy DataVariantFactory after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

DataValueWrapper* DataVariantFactoryImp::createWrapper(const void* pObject, const std::string& className, bool strict)
{
   ObjectMapType3::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = mCreateWrapperMap.find(className);
   if (itr != mCreateWrapperMap.end())
   {
      return ((*itr).second) (pObject);
   }

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : WBS9.1: stack overflow if we get here " \
   "via CS::deserialize (TJOHNSON)")
   if (strict)
   {
      string msg = "DataVariantFactory::createWrapper given unknown object type: '" + className + "'";
      VERIFYRV_MSG(false, NULL, msg.c_str()); 
   }
   return NULL;
}

DataValueWrapper* DataVariantFactoryImp::createWrapper(DOMNode* pDocument, int version)
{
   DOMElement* pElement(static_cast<DOMElement*>(pDocument));
   std::string type = A(pElement->getAttribute(X("type")));
   DataValueWrapper* pWrapper = createWrapper(NULL, type);
   if (pWrapper)
   {
      if (pWrapper->fromXml(pDocument, version) == false)
      {
         delete pWrapper;
         pWrapper = NULL;
      }
   }
   return pWrapper;
}
