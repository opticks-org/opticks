/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "Any.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "ApiUtilities.h"
#include "AppConfig.h"
#include "assert.h"
#include "BadValues.h"
#include "ConnectionManager.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataElement.h"
#include "DataElementGroup.h"
#include "DesktopServices.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "GraphicElement.h"
#include "LatLonLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "LocationType.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "PseudocolorLayer.h"
#include "RasterData.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SessionItemImp.h"
#include "Signature.h"
#include "SignatureLibrary.h"
#include "SignatureSet.h"
#include "SimpleApiErrors.h"
#include "SimpleViews.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"
#include "TiePointLayer.h"
#include "TiePointList.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TypeConverter.h"
#include "View.h"

#include <math.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace
{
   template<class T>
   bool testDataElement(const DataElement* pElement)
   {
      bool success = true;
      if (pElement != NULL)
      {
         // Get the full name of the element
         string name = pElement->getName();
         DataElement* pParent = pElement->getParent();
         while (pParent != NULL)
         {
            name = pParent->getName() + "|" + name;
            pParent = pParent->getParent();
         }

         const char* const pElementName = name.c_str();
         const char* const pElementId = pElement->getId().c_str();
         const RasterElement* const pRasterElement = dynamic_cast<const RasterElement*>(pElement);

         // Positive tests
         vector<char> filename(pElement->getFilename().length() + 1, 0);
         getDataElementFilename(const_cast<DataElement*>(pElement), &filename[0], filename.size());
         issearf(pElement->getFilename() == string(&filename[0]));

         issearf(dynamic_cast<T*>(getDataElement(pElementId, NULL, 0)) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getDataElement(pElementName, NULL, 0)) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         issearf(getDataElement(pElementId, "", 0) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(getDataElement(pElementName, "", 0) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         issearf(dynamic_cast<T*>(getDataElement(pElementId, TypeConverter::toString(pElement), 0)) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getDataElement(pElementName, TypeConverter::toString(pElement), 0)) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         issearf(dynamic_cast<T*>(getDataElement(pElementId, TypeConverter::toString<T>(), 0)) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getDataElement(pElementName, TypeConverter::toString<T>(), 0)) == pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         int owns = 0;
         if (pRasterElement != NULL)
         {
            issearf(createDataPointer(const_cast<DataElement*>(pElement), NULL, &owns) == pRasterElement->getRawData());
         }

         // Negative tests
         issearf(getDataElement(NULL, NULL, 0) == NULL);
         issearf(getLastError() == SIMPLE_BAD_PARAMS);
         issearf(getDataElement(NULL, "", 0) == NULL);
         issearf(getLastError() == SIMPLE_BAD_PARAMS);
         issearf(getDataElement("", NULL, 0) == NULL);
         issearf(getLastError() == SIMPLE_NOT_FOUND);

         const string fakeId = SessionItemImp::generateUniqueId();
         issearf(getDataElement(fakeId.c_str(), NULL, 0) == NULL);
         issearf(getLastError() == SIMPLE_NOT_FOUND);

         issearf(dynamic_cast<T*>(getDataElement(pElementId, "wrongType", 0)) == NULL);
         issearf(getLastError() == SIMPLE_WRONG_TYPE);
         issearf(dynamic_cast<T*>(getDataElement(pElementName, "wrongType", 0)) == NULL);
         issearf(getLastError() == SIMPLE_NOT_FOUND);
      }

      return success;
   }

   template<class T>
   bool testView(const View* pView)
   {
      bool success = true;
      if (pView != NULL)
      {
         const SpatialDataView* pSpatialDataView = dynamic_cast<const SpatialDataView*>(pView);
         const char* const pViewId = pView->getId().c_str();
         const char* const pViewName = pView->getName().c_str();
         const char* const pViewDisplayName = pView->getDisplayName().c_str();
         const View* const pCurrentView = Service<DesktopServices>()->getCurrentWorkspaceWindowView();
         issearf(pCurrentView != NULL);

         // Positive tests
         issearf(getView(NULL, NULL) == pCurrentView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(getView("", NULL) == pCurrentView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(getView(NULL, "") == pCurrentView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(getView("", "") == pCurrentView);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         issearf(dynamic_cast<T*>(getView(pViewId, NULL)) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getView(pViewName, NULL)) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getView(pViewDisplayName, NULL)) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         issearf(dynamic_cast<T*>(getView(pViewId, TypeConverter::toString<T>())) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getView(pViewName, TypeConverter::toString<T>())) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getView(pViewDisplayName, TypeConverter::toString<T>())) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         issearf(dynamic_cast<T*>(getView(pViewId, TypeConverter::toString(pView))) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getView(pViewName, TypeConverter::toString(pView))) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         issearf(dynamic_cast<T*>(getView(pViewDisplayName, TypeConverter::toString(pView))) == pView);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         // Special tests for SpatialDataView.
         if (pSpatialDataView != NULL)
         {
            LayerList* pLayerList = pSpatialDataView->getLayerList();
            issearf(pLayerList != NULL);
            vector<Layer*> layers;
            pLayerList->getLayers(layers);
            for (vector<Layer*>::const_iterator iter = layers.begin(); iter != layers.end(); ++iter)
            {
               const Layer* const pLayer = *iter;
               issearf(pLayer != NULL);
               issearf(testLayer<Layer>(pLayer));
               issearf(testLayer<AnnotationLayer>(dynamic_cast<const AnnotationLayer*>(pLayer)));
               issearf(testLayer<AoiLayer>(dynamic_cast<const AoiLayer*>(pLayer)));
               issearf(testLayer<GcpLayer>(dynamic_cast<const GcpLayer*>(pLayer)));
               issearf(testLayer<GraphicLayer>(dynamic_cast<const GraphicLayer*>(pLayer)));
               issearf(testLayer<LatLonLayer>(dynamic_cast<const LatLonLayer*>(pLayer)));
               issearf(testLayer<RasterLayer>(dynamic_cast<const RasterLayer*>(pLayer)));
               issearf(testLayer<PseudocolorLayer>(dynamic_cast<const PseudocolorLayer*>(pLayer)));
               issearf(testLayer<ThresholdLayer>(dynamic_cast<const ThresholdLayer*>(pLayer)));
               issearf(testLayer<TiePointLayer>(dynamic_cast<const TiePointLayer*>(pLayer)));
            }
         }

         // Negative tests
         const string fakeId = SessionItemImp::generateUniqueId();
         issearf(getView(fakeId.c_str(), NULL) == NULL);
         issearf(getLastError() == SIMPLE_NOT_FOUND);

         issearf(dynamic_cast<T*>(getView(pViewId, "wrongType")) == NULL);
         issearf(getLastError() == SIMPLE_WRONG_TYPE);
         issearf(dynamic_cast<T*>(getView(pViewName, "wrongType")) == NULL);
         issearf(getLastError() == SIMPLE_WRONG_TYPE);
      }

      return success;
   }

   template<class T>
   bool testLayer(const Layer* pLayer)
   {
      bool success = true;
      if (pLayer != NULL)
      {
         const char* const pLayerId = pLayer->getId().c_str();
         const SpatialDataView* const pView = dynamic_cast<SpatialDataView*>(pLayer->getView());

         // Positive tests
         issearf(dynamic_cast<T*>(getLayer(pLayerId, TypeConverter::toString<T>())) == pLayer);
         issearf(getLastError() == SIMPLE_NO_ERROR);

         if (pView != NULL)
         {
            const string name(pView->getName() + "|" + pLayer->getName());
            issearf(dynamic_cast<T*>(getLayer(name.c_str(), TypeConverter::toString<T>())) == pLayer);
            issearf(getLastError() == SIMPLE_NO_ERROR);
         }

         // Negative tests
         const string fakeId = SessionItemImp::generateUniqueId();
         issearf(getLayer(fakeId.c_str(), NULL) == NULL);
         issearf(getLastError() == SIMPLE_NOT_FOUND);

         issearf(dynamic_cast<T*>(getLayer(pLayerId, "wrongType")) == NULL);
         issearf(getLastError() == SIMPLE_WRONG_TYPE);
      }

      return success;
   }

   class RasterElementResource
   {
   public:
      RasterElementResource(const string& name, RasterElementArgs args) :
         mpRasterElement(dynamic_cast<RasterElement*>(createRasterElement(name.c_str(), args)))
      {}

      ~RasterElementResource()
      {
         destroyDataElement(mpRasterElement);
      }

      operator RasterElement*()
      {
         return mpRasterElement;
      }

   private:
      RasterElement* const mpRasterElement;
      RasterElementResource(const RasterElementResource&);
      RasterElementResource& operator=(const RasterElementResource&);
   };

   class BadValuesResource
   {
   public:
      BadValuesResource(const string& badValuesStr) :
         mpBadValues(dynamic_cast<BadValues*>(createBadValues(badValuesStr.c_str())))
         {}

         ~BadValuesResource()
         {
            Service<ObjectFactory>()->destroyObject(mpBadValues, TypeConverter::toString<BadValues>());
         }

         operator BadValues*()
         {
            return mpBadValues;
         }

   private:
      BadValues* const mpBadValues;
      BadValuesResource(const BadValuesResource&);
      BadValuesResource& operator=(const BadValuesResource&);
   };

   class DataInfoResource
   {
   public:
      DataInfoResource(RasterElement* pElement) :
         mpDataInfo(createDataInfo(pElement))
      {}

      ~DataInfoResource()
      {
         destroyDataInfo(mpDataInfo);
      }

      operator DataInfo*()
      {
         return mpDataInfo;
      }

   private:
      DataInfo* const mpDataInfo;
      DataInfoResource(const DataInfoResource&);
      DataInfoResource& operator=(const DataInfoResource&);
   };

   class DataPointerResource
   {
   public:
      DataPointerResource(DataElement* pElement, DataPointerArgs* pArgs)
      {
         mpData = createDataPointer(pElement, pArgs, &mOwns);
      }

      ~DataPointerResource()
      {
         if (mOwns != 0)
         {
            destroyDataPointer(mpData);
         }
      }

      operator char*()
      {
         return reinterpret_cast<char*>(mpData);
      }

   private:
      void* mpData;
      int mOwns;
      DataPointerResource(const DataPointerResource&);
      DataPointerResource& operator=(const DataPointerResource&);
   };

   class DataAccessorResource
   {
   public:
      DataAccessorResource(RasterElement* pElement, DataAccessorArgs* pArgs) :
         mpDataAccessor(createDataAccessor(pElement, pArgs))
      {}

      ~DataAccessorResource()
      {
         destroyDataAccessor(mpDataAccessor);
      }

      operator DataAccessorImpl*()
      {
         return mpDataAccessor;
      }

   private:
      DataAccessorImpl* mpDataAccessor;
      DataAccessorResource(const DataAccessorResource&);
      DataAccessorResource& operator=(const DataAccessorResource&);
   };
}

class CreateRasterElementTestCase : public TestCase
{
public:
   CreateRasterElementTestCase(const string& name = "CreateRasterElement") : TestCase(name) { }
   bool run()
   {
      bool success = true;

      RasterElementArgs args;
      args.encodingType = INT2UBYTES;
      args.interleaveFormat = BSQ;
      args.location = 1;
      args.numRows = 6;
      args.numColumns = 5;
      args.numBands = 3;
      string badValuesStr("<-10, 0, 1.5<>3.5, >200");
      BadValuesResource pArgsBadValues(badValuesStr);
      args.pBadValues = pArgsBadValues;
      args.pParent = NULL;
      const string name = getName() + "_temp";

      // Check creation and deletion of a RasterElement with arbitrary dimensions and data type.
      {
         RasterElementResource rasterElementResource(name, args);
         RasterElement* pElement = rasterElementResource;
         issearf(pElement != NULL);
         issearf(getDataElement(name.c_str(), TypeConverter::toString<RasterElement>(), 0) == pElement);

         DataInfoResource dataInfoResource(pElement);
         DataInfo* pDataInfo = dataInfoResource;
         issearf(pDataInfo != NULL);
         issearf(pDataInfo->encodingType == args.encodingType);
         issearf(pDataInfo->encodingTypeSize ==
            RasterUtilities::bytesInEncoding(static_cast<EncodingTypeEnum>(args.encodingType)));
         issearf(pDataInfo->numRows == args.numRows);
         issearf(pDataInfo->numColumns == args.numColumns);
         issearf(pDataInfo->numBands == args.numBands);
         issearf(pDataInfo->interleaveFormat == args.interleaveFormat);
         issearf(pDataInfo->pBadValues != NULL);
         issearf(pDataInfo->pBadValues->getBadValuesString() == badValuesStr);
      }

      // Check deletion
      issearf(getDataElement(name.c_str(), NULL, 0) == NULL);
      return success;
   }
};

class RasterElementTestCase : public TestCase
{
public:
   RasterElementTestCase(const string& name = "RasterElement") : TestCase(name) { }
   bool run()
   {
      bool success = true;
      RasterElement* pElement = TestUtilities::getStandardRasterElement();
      issearf(pElement != NULL);
      issearf(testDataElement<DataElement>(pElement));
      issearf(testDataElement<RasterElement>(pElement));

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issearf(pDescriptor != NULL);

      // Check that DataInfo can be obtained successfully and contains correct values.
      DataInfoResource dataInfoResource(pElement);
      DataInfo* pDataInfo = dataInfoResource;
      issearf(pDataInfo != NULL);
      issearf(pDataInfo->encodingTypeSize == pDescriptor->getBytesPerElement());
      issearf(pDataInfo->encodingType == pDescriptor->getDataType());
      issearf(pDataInfo->numRows == pDescriptor->getRowCount());
      issearf(pDataInfo->numColumns == pDescriptor->getColumnCount());
      issearf(pDataInfo->numBands == pDescriptor->getBandCount());
      issearf(pDataInfo->interleaveFormat == pDescriptor->getInterleaveFormat());

      const BadValues* pBadValues = pDescriptor->getBadValues();
      if (pDataInfo->pBadValues != NULL)
      {
         issearf(pDataInfo->pBadValues->compare(pBadValues));
      }
      else
      {
         issearf(pBadValues == NULL);
      }

      DataAccessorArgs dataAccessorArgs;
      dataAccessorArgs.rowStart = 0;
      dataAccessorArgs.rowEnd = pDescriptor->getRowCount() - 1;
      dataAccessorArgs.concurrentRows = 1;
      dataAccessorArgs.columnStart = 0;
      dataAccessorArgs.columnEnd = pDescriptor->getColumnCount() - 1;
      dataAccessorArgs.concurrentColumns = 0;
      dataAccessorArgs.bandStart = 0;
      dataAccessorArgs.bandEnd = 1;
      dataAccessorArgs.concurrentBands = 0;
      dataAccessorArgs.interleaveFormat = pDescriptor->getInterleaveFormat();
      dataAccessorArgs.writable = false;
      DataAccessorResource pAccessor(pElement, &dataAccessorArgs);
      issearf(pAccessor != NULL && isDataAccessorValid(pAccessor));

      DataAccessor dataAccessor = pElement->getDataAccessor(NULL);
      issearf(dataAccessor.isValid());

      // Check a few arbitrary points to make sure they match using toPixel.
      vector<LocationType> points;
      points.push_back(LocationType(0, 0));
      points.push_back(LocationType(0, dataAccessorArgs.rowEnd));
      points.push_back(LocationType(dataAccessorArgs.columnEnd, 0));
      points.push_back(LocationType(dataAccessorArgs.columnEnd, dataAccessorArgs.rowEnd));
      points.push_back(LocationType(dataAccessorArgs.columnEnd / 2, dataAccessorArgs.rowEnd / 2));

      for (vector<LocationType>::const_iterator iter = points.begin(); iter != points.end(); ++iter)
      {
         toDataAccessorPixel(pAccessor, iter->mY, iter->mX);
         issearf(isDataAccessorValid(pAccessor) == 1);
         dataAccessor->toPixel(iter->mY, iter->mX);
         issearf(dataAccessor.isValid() == true);
         issearf(memcmp(getDataAccessorColumn(pAccessor), dataAccessor->getColumn(), pDescriptor->getBytesPerElement()) == 0);
      }

      // Check a few arbitrary points to make sure they match using nextRow/nextColumn.
      const int32_t rowStart = dataAccessorArgs.rowEnd / 10;
      const int32_t rowEnd = rowStart * 5;
      const int32_t rowSkip = 13;
      const int32_t columnStart = dataAccessorArgs.columnEnd / 10;
      const int32_t columnEnd = columnStart * 7;
      const int32_t columnSkip = 11;
      for (int32_t row = rowStart; row < rowEnd; row += rowSkip)
      {
         toDataAccessorPixel(pAccessor, row, columnStart);
         dataAccessor->toPixel(row, columnStart);
         for (int32_t column = columnStart; column < columnEnd; column += columnSkip)
         {
            issearf(isDataAccessorValid(pAccessor) == 1);
            issearf(dataAccessor.isValid() == true);
            issearf(memcmp(getDataAccessorColumn(pAccessor), dataAccessor->getColumn(), pDescriptor->getBytesPerElement()) == 0);
            nextDataAccessorColumn(pAccessor, columnSkip);
            dataAccessor->nextColumn(columnSkip);
         }
      }

      {
         DataPointerArgs dataPointerArgs;
         dataPointerArgs.rowStart = dataAccessorArgs.rowStart;
         dataPointerArgs.rowEnd = dataAccessorArgs.rowEnd;
         dataPointerArgs.columnStart = dataAccessorArgs.columnStart;
         dataPointerArgs.columnEnd = dataAccessorArgs.columnEnd;
         dataPointerArgs.bandStart = dataAccessorArgs.bandStart;
         dataPointerArgs.bandEnd = dataPointerArgs.bandStart;  // Only request a single band.
         dataPointerArgs.interleaveFormat = dataAccessorArgs.interleaveFormat;

         DataPointerResource dataPointerResource(pElement, &dataPointerArgs);
         char* const pData = dataPointerResource;
         issearf(pData != NULL);

         // Check NULL argument list.
         {
            DataPointerResource dataPointerResourceNullArgs(pElement, NULL);
            char* pDataNullArgs = dataPointerResourceNullArgs;
            issearf(pDataNullArgs != NULL);
            issearf(pData != pDataNullArgs);

            DataPointerResource dataPointerResourceNullArgs2(pElement, NULL);
            char* pDataNullArgs2 = dataPointerResourceNullArgs2;
            issearf(pDataNullArgs2 != NULL);
            issearf(pDataNullArgs == pDataNullArgs2);
         }

         // Check a few arbitrary points to make sure they match using createDataPointer/destroyDataPointer.
         for (vector<LocationType>::const_iterator iter = points.begin(); iter != points.end(); ++iter)
         {
            dataAccessor->toPixel(iter->mY, iter->mX);
            issearf(dataAccessor.isValid() == true);

            const unsigned int rowOffset = (iter->mY - dataPointerArgs.rowStart) *
               (dataPointerArgs.columnEnd - dataPointerArgs.columnStart + 1);
            const unsigned int columnOffset = (iter->mX - dataPointerArgs.columnStart);
            const unsigned int offset = (rowOffset + columnOffset) * pDescriptor->getBytesPerElement();
            issearf(memcmp(pData + offset, dataAccessor->getColumn(), pDescriptor->getBytesPerElement()) == 0);

            // Modify the point and confirm that it only affects the copy.
            memset(pData + offset, 0, pDescriptor->getBytesPerElement());
            issearf(memcmp(pData + offset, dataAccessor->getColumn(), pDescriptor->getBytesPerElement()) != 0);
         }

         // Update the original data.
         copyDataToRasterElement(pElement, &dataPointerArgs, pData);
         updateRasterElement(pElement);
         issearf(getLastError() == SIMPLE_NO_ERROR);
         for (vector<LocationType>::const_iterator iter = points.begin(); iter != points.end(); ++iter)
         {
            dataAccessor->toPixel(iter->mY, iter->mX);
            issearf(dataAccessor.isValid() == true);

            const unsigned int rowOffset = (iter->mY - dataPointerArgs.rowStart) *
               (dataPointerArgs.columnEnd - dataPointerArgs.columnStart + 1);
            const unsigned int columnOffset = (iter->mX - dataPointerArgs.columnStart);
            const unsigned int offset = (rowOffset + columnOffset) * pDescriptor->getBytesPerElement();
            issearf(memcmp(pData + offset, dataAccessor->getColumn(), pDescriptor->getBytesPerElement()) == 0);
         }
      }

      return success;
   }
};

class NestedDataElementTestCase : public TestCase
{
public:
   NestedDataElementTestCase(const string& name = "NestedDataElement") : TestCase(name) { }
   bool run()
   {
      string fullName;
      bool success = true;
      Service<ModelServices> pModel;
      const unsigned int numElements(5);
      vector<DataElement*> elements(numElements);
      for (vector<DataElement*>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
      {
         if (iter == elements.begin())
         {
            *iter = TestUtilities::getStandardRasterElement();
         }
         else
         {
            *iter = pModel->createElement(SessionItemImp::generateUniqueId(),
               TypeConverter::toString<DataElement*>(), *(iter - 1));
         }

         issearf(*iter != NULL);
      }

      for (vector<DataElement*>::const_iterator iter = elements.begin(); iter != elements.end(); ++iter)
      {
         issearf(testDataElement<DataElement>(*iter));
      }

      return success;
   }
};

class TreeDataElementTestCase : public TestCase
{
public:
   TreeDataElementTestCase(const string& name = "TreeDataElement") : TestCase(name) { }
   bool run()
   {
      string fullName;
      bool success = true;
      Service<ModelServices> pModel;

      // Create lots of DataElement subclasses
      vector<string> elementTypes;
      elementTypes.push_back(TypeConverter::toString<AnnotationElement>());
      elementTypes.push_back(TypeConverter::toString<Any>());
      elementTypes.push_back(TypeConverter::toString<AoiElement>());
      elementTypes.push_back(TypeConverter::toString<DataElement>());
      elementTypes.push_back(TypeConverter::toString<DataElementGroup>());
      elementTypes.push_back(TypeConverter::toString<GcpList>());
      elementTypes.push_back(TypeConverter::toString<Signature>());
      elementTypes.push_back(TypeConverter::toString<SignatureLibrary>());
      elementTypes.push_back(TypeConverter::toString<SignatureSet>());
      elementTypes.push_back(TypeConverter::toString<TiePointList>());

      const double width(2.0);
      const double height(5.0);
      const unsigned int numElements(static_cast<unsigned int>(pow(width, height) - 1));
      vector<DataElement*> elements(numElements);
      for (unsigned int i = 0; i < elements.size(); ++i)
      {
         if (i == 0)
         {
            elements[i] = TestUtilities::getStandardRasterElement();
         }
         else
         {
            elements[i] = pModel->createElement(SessionItemImp::generateUniqueId(),
               elementTypes[i % elementTypes.size()], elements[(i - 1) / static_cast<unsigned int>(width)]);
         }

         issearf(elements[i] != NULL);
      }

      for (vector<DataElement*>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
      {
         const DataElement* const pElement = *iter;
         issearf(testDataElement<DataElement>(pElement));
         issearf(testDataElement<AnnotationElement>(dynamic_cast<const AnnotationElement*>(pElement)));
         issearf(testDataElement<Any>(dynamic_cast<const Any*>(pElement)));
         issearf(testDataElement<AoiElement>(dynamic_cast<const AoiElement*>(pElement)));
         issearf(testDataElement<DataElementGroup>(dynamic_cast<const DataElementGroup*>(pElement)));
         issearf(testDataElement<GcpList>(dynamic_cast<const GcpList*>(pElement)));
         issearf(testDataElement<GraphicElement>(dynamic_cast<const GraphicElement*>(pElement)));
         issearf(testDataElement<RasterElement>(dynamic_cast<const RasterElement*>(pElement)));
         issearf(testDataElement<Signature>(dynamic_cast<const Signature*>(pElement)));
         issearf(testDataElement<SignatureLibrary>(dynamic_cast<const SignatureLibrary*>(pElement)));
         issearf(testDataElement<SignatureSet>(dynamic_cast<const SignatureSet*>(pElement)));
         issearf(testDataElement<TiePointList>(dynamic_cast<const TiePointList*>(pElement)));
      }

      return success;
   }
};

class SpatialDataViewTestCase : public TestCase
{
public:
   SpatialDataViewTestCase(const string& name = "SpatialDataView") : TestCase(name) { }
   bool run()
   {
      bool success = true;
      RasterElement* const pElement = TestUtilities::getStandardRasterElement();
      issearf(pElement != NULL);
      SpatialDataView* const pView = dynamic_cast<SpatialDataView*>(
         Service<DesktopServices>()->getCurrentWorkspaceWindowView());
      issearf(pView != NULL);

      vector<const Layer*> layers;
      vector<pair<LayerType, DataElement*> > layerTypes;
      layerTypes.push_back(pair<LayerType, DataElement*>(ANNOTATION, reinterpret_cast<DataElement*>(NULL)));
      layerTypes.push_back(pair<LayerType, DataElement*>(AOI_LAYER, reinterpret_cast<DataElement*>(NULL)));
      layerTypes.push_back(pair<LayerType, DataElement*>(GCP_LAYER, reinterpret_cast<DataElement*>(NULL)));
      layerTypes.push_back(pair<LayerType, DataElement*>(LAT_LONG, pElement));
      layerTypes.push_back(pair<LayerType, DataElement*>(RASTER, pElement));
      layerTypes.push_back(pair<LayerType, DataElement*>(PSEUDOCOLOR, pElement));
      layerTypes.push_back(pair<LayerType, DataElement*>(THRESHOLD, pElement));
      layerTypes.push_back(pair<LayerType, DataElement*>(TIEPOINT_LAYER, reinterpret_cast<DataElement*>(NULL)));
      for (vector<pair<LayerType, DataElement*> >::const_iterator iter = layerTypes.begin();
         iter != layerTypes.end();
         ++iter)
      {
         const Layer* const pLayer = pView->createLayer(iter->first, iter->second, SessionItemImp::generateUniqueId());
         issearf(pLayer != NULL);
         layers.push_back(pLayer);
      }

      // View tests
      issearf(testView<View>(pView));
      issearf(testView<SpatialDataView>(pView));

      // Layer tests
      for (vector<const Layer*>::const_iterator iter = layers.begin(); iter != layers.end(); ++iter)
      {
         const Layer* const pLayer = *iter;
         issearf(testLayer<Layer>(pLayer));
         issearf(testLayer<AnnotationLayer>(dynamic_cast<const AnnotationLayer*>(pLayer)));
         issearf(testLayer<AoiLayer>(dynamic_cast<const AoiLayer*>(pLayer)));
         issearf(testLayer<GcpLayer>(dynamic_cast<const GcpLayer*>(pLayer)));
         issearf(testLayer<GraphicLayer>(dynamic_cast<const GraphicLayer*>(pLayer)));
         issearf(testLayer<LatLonLayer>(dynamic_cast<const LatLonLayer*>(pLayer)));
         issearf(testLayer<RasterLayer>(dynamic_cast<const RasterLayer*>(pLayer)));
         issearf(testLayer<PseudocolorLayer>(dynamic_cast<const PseudocolorLayer*>(pLayer)));
         issearf(testLayer<ThresholdLayer>(dynamic_cast<const ThresholdLayer*>(pLayer)));
         issearf(testLayer<TiePointLayer>(dynamic_cast<const TiePointLayer*>(pLayer)));
      }

      return success;
   }
};

class ProductViewTestCase : public TestCase
{
public:
   ProductViewTestCase(const string& name = "ProductView") : TestCase(name) { }
   bool run()
   {
      bool success = true;
      Service<DesktopServices> pDesktop;
      const RasterElement* const pElement = TestUtilities::getStandardRasterElement();
      issearf(pElement != NULL);
      SpatialDataView* const pView = dynamic_cast<SpatialDataView*>(
         pDesktop->getCurrentWorkspaceWindowView());
      issearf(pView != NULL);

      const ProductWindow* const pProductWindow = pDesktop->deriveProduct(pView);
      issearf(pProductWindow != NULL);
      const ProductView* const pProductView = pProductWindow->getProductView();
      issearf(pProductView != NULL);

      issearf(testView<View>(pView));
      issearf(testView<SpatialDataView>(pView));
      issearf(testView<View>(pProductView));
      issearf(testView<ProductView>(pProductView));

      return success;
   }
};

class SimpleApiTestSuite : public TestSuiteNewSession
{
public:
   SimpleApiTestSuite() : TestSuiteNewSession("SimpleApi")
   {
      setHandle(ConnectionManager::instance());
      addTestCase(new CreateRasterElementTestCase);
      addTestCase(new RasterElementTestCase);
      addTestCase(new NestedDataElementTestCase);
      addTestCase(new TreeDataElementTestCase);
      addTestCase(new SpatialDataViewTestCase);
      addTestCase(new ProductViewTestCase);
   }
};

REGISTER_SUITE(SimpleApiTestSuite)
