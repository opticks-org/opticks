/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ObjectFactoryImp.h"

// For maintenance ease, please keep the following
// list alphabetized.
#include "BadValuesAdapter.h"
#include "BitMaskImp.h"
#include "ClassificationAdapter.h"
#include "AppVerify.h"
#include "DataRequestImp.h"
#include "DataVariantAnyData.h"
#include "DateTimeImp.h"
#include "DynamicObjectAdapter.h"
#include "ExecutableAgentAdapter.h"
#include "ExportAgentAdapter.h"
#include "FileDescriptorAdapter.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "FontImp.h"
#include "GeoreferenceDescriptorAdapter.h"
#include "GraphicObject.h"
#include "ImportAgentAdapter.h"
#include "Layer.h"
#include "PointCloudDataRequestImp.h"
#include "PointCloudFileDescriptorAdapter.h"
#include "RasterFileDescriptorAdapter.h"
#include "SettableSessionItemAdapter.h"
#include "SignatureFileDescriptorAdapter.h"
#include "TypeConverter.h"
#include "UnitsAdapter.h"
#include "WavelengthsImp.h"
#include "WizardObjectAdapter.h"

#include <stdexcept>
#include <list>
#include <vector>
#include <map>
using namespace std;

namespace
{
   void* CreateVectorGraphicObject();
   void DestroyVectorGraphicObject(void* pObj);

   BadValues* CreateBadValues();
   void DestroyBadValues(void* pObj);
   void* CreateVectorBadValues();
   void DestroyVectorBadValues(void* pObj);

   BitMask* CreateBitMask();
   void DestroyBitMask(void* pObj);
   void* CreateVectorBitMask();
   void DestroyVectorBitMask(void* pObj);

   Classification* CreateClassification();
   void DestroyClassification(void* pObj);
   void* CreateVectorClassification();
   void DestroyVectorClassification(void* pObj);

   DataRequest* CreateDataRequest();
   void DestroyDataRequest(void* pObj);

   PointCloudDataRequest* CreatePointCloudDataRequest();
   void DestroyPointCloudDataRequest(void* pObj);

   DataVariantAnyData* CreateDataVariantAnyData();
   void DestroyDataVariantAnyData(void* pObj);

   DateTime* CreateDateTime();
   void DestroyDateTime(void* pObj);
   void* CreateVectorDateTime();
   void DestroyVectorDateTime(void* pObj);

   DynamicObject* CreateDynamicObject();
   void DestroyDynamicObject(void* pObj);
   void* CreateVectorDynamicObject();
   void DestroyVectorDynamicObject(void* pObj);

   FileFinder* CreateFileFinder();
   void DestroyFileFinder(void* pObj);
   void* CreateVectorFileFinder();
   void DestroyVectorFileFinder(void* pObj);

   static FileDescriptor* CreateFileDescriptor();
   static void DestroyFileDescriptor(void* pObj);
   static void* CreateVectorFileDescriptor();
   static void DestroyVectorFileDescriptor(void* pObj);

   Filename* CreateFilename();
   void DestroyFilename(void* pObj);
   void* CreateVectorFilename();
   void DestroyVectorFilename(void* pObj);

   Font* CreateFontObject();
   void DestroyFontObject(void* pObj);

   GeoreferenceDescriptor* CreateGeoreferenceDescriptor();
   void DestroyGeoreferenceDescriptor(void* pObj);
   void* CreateVectorGeoreferenceDescriptor();
   void DestroyVectorGeoreferenceDescriptor(void* pObj);

   void* CreateVectorLayer();
   void DestroyVectorLayer(void* pObj);

   static PointCloudFileDescriptor* CreatePointCloudFileDescriptor();
   static void DestroyPointCloudFileDescriptor(void* pObj);
   static void* CreateVectorPointCloudFileDescriptor();
   static void DestroyVectorPointCloudFileDescriptor(void* pObj);

   static RasterFileDescriptor* CreateRasterFileDescriptor();
   static void DestroyRasterFileDescriptor(void* pObj);
   static void* CreateVectorRasterFileDescriptor();
   static void DestroyVectorRasterFileDescriptor(void* pObj);

   static SignatureFileDescriptor* CreateSignatureFileDescriptor();
   static void DestroySignatureFileDescriptor(void* pObj);
   static void* CreateVectorSignatureFileDescriptor();
   static void DestroyVectorSignatureFileDescriptor(void* pObj);

   SettableSessionItem* CreateSettableSessionItem();
   void DestroySettableSessionItem(void* pObj);

   ExecutableAgent* CreateExecutableAgent();
   void DestroyExecutableAgent(void* pObj);

   ImportAgent* CreateImportAgent();
   void DestroyImportAgent(void* pObj);

   ExportAgent* CreateExportAgent();
   void DestroyExportAgent(void* pObj);

   Units* CreateUnits();
   void DestroyUnits(void* pObj);
   void* CreateVectorUnits();
   void DestroyVectorUnits(void* pObj);

   Wavelengths* CreateWavelengths();
   void DestroyWavelengths(void* pObj);
   void* CreateVectorWavelengths();
   void DestroyVectorWavelengths(void* pObj);

   static WizardObject* CreateWizardObject();
   static void DestroyWizardObject(void* pObj);
   static void* CreateVectorWizardObject();
   static void DestroyVectorWizardObject(void* pObj);

   void* CreateUnsignedChar();
   void* CreateChar();
   void* CreateUnsignedShort();
   void* CreateShort();
   void* CreateUnsignedInt();
   void* CreateInt();
   void* CreateUnsignedLong();
   void* CreateLong();
   void* CreateFloat();
   void* CreateDouble();
   void* CreateBool();
   void* CreateString();

   void DestroyUnsignedChar(void*);
   void DestroyChar(void*);
   void DestroyUnsignedShort(void*);
   void DestroyShort(void*);
   void DestroyUnsignedInt(void*);
   void DestroyInt(void*);
   void DestroyUnsignedLong(void*);
   void DestroyLong(void*);
   void DestroyFloat(void*);
   void DestroyDouble(void*);
   void DestroyBool(void*);
   void DestroyString(void*);

   void* CreateVectorUnsignedChar();
   void* CreateVectorChar();
   void* CreateVectorUnsignedShort();
   void* CreateVectorShort();
   void* CreateVectorUnsignedInt();
   void* CreateVectorInt();
   void* CreateVectorUnsignedLong();
   void* CreateVectorLong();
   void* CreateVectorFloat();
   void* CreateVectorDouble();
   void* CreateVectorBool();
   void* CreateVectorString();
   void* CreateVectorVoidPtr();
   void* CreateVectorUnsignedCharPtr();
   void* CreateVectorCharPtr();
   void* CreateVectorUnsignedShortPtr();
   void* CreateVectorShortPtr();
   void* CreateVectorUnsignedIntPtr();
   void* CreateVectorIntPtr();
   void* CreateVectorUnsignedLongPtr();
   void* CreateVectorLongPtr();
   void* CreateVectorFloatPtr();
   void* CreateVectorDoublePtr();
   void* CreateVectorBoolPtr();
   void* CreateVectorStringPtr();

   void DestroyVectorUnsignedChar(void*);
   void DestroyVectorChar(void*);
   void DestroyVectorUnsignedShort(void*);
   void DestroyVectorShort(void*);
   void DestroyVectorUnsignedInt(void*);
   void DestroyVectorInt(void*);
   void DestroyVectorUnsignedLong(void*);
   void DestroyVectorLong(void*);
   void DestroyVectorFloat(void*);
   void DestroyVectorDouble(void*);
   void DestroyVectorBool(void*);
   void DestroyVectorString(void*);
   void DestroyVectorVoidPtr(void*);
   void DestroyVectorUnsignedCharPtr(void*);
   void DestroyVectorCharPtr(void*);
   void DestroyVectorUnsignedShortPtr(void*);
   void DestroyVectorShortPtr(void*);
   void DestroyVectorUnsignedIntPtr(void*);
   void DestroyVectorIntPtr(void*);
   void DestroyVectorUnsignedLongPtr(void*);
   void DestroyVectorLongPtr(void*);
   void DestroyVectorFloatPtr(void*);
   void DestroyVectorDoublePtr(void*);
   void DestroyVectorBoolPtr(void*);
   void DestroyVectorStringPtr(void*);
}

typedef map<string,void*(*)()> ObjectMapType;
typedef map<string,void*(*)()> VectorMapType;
typedef map<string,void(*)(void*)> ObjectMapType2;
typedef map<string,void(*)(void*)> VectorMapType2;
static ObjectMapType sCreateObjectMap;
static ObjectMapType2 sDestroyObjectMap;
static VectorMapType sCreateObjectVectorMap;
static VectorMapType2 sDestroyObjectVectorMap;

ObjectFactoryImp* ObjectFactoryImp::spInstance = NULL;
bool ObjectFactoryImp::mDestroyed = false;

bool ObjectFactoryImp::parseClassString(const std::string& className, std::string &vectorClass)
{
   static const std::string strVector = "vector<";
   static const int strVectorLength = strVector.length();
   int classNameLen = className.length();
   if ( (className.substr(0, strVectorLength) == strVector)
      && (className[classNameLen-1] == '>') )
   {
      // className is of the form "vector<int>"
      vectorClass = className.substr(strVectorLength, classNameLen-strVectorLength-1);
      return true;

   }
   return false;
}

ObjectFactoryImp* ObjectFactoryImp::instance()
{
   if (spInstance == NULL) 
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use ObjectFactory after "
            "destroying it.");
      }
      spInstance = new ObjectFactoryImp;

      sCreateObjectMap.insert(ObjectMapType::value_type("unsigned char", (void*(*)())CreateUnsignedChar));
      sCreateObjectMap.insert(ObjectMapType::value_type("char", (void*(*)())CreateChar));
      sCreateObjectMap.insert(ObjectMapType::value_type("unsigned short", (void*(*)())CreateUnsignedShort));
      sCreateObjectMap.insert(ObjectMapType::value_type("short", (void*(*)())CreateShort));
      sCreateObjectMap.insert(ObjectMapType::value_type("unsigned int", (void*(*)())CreateUnsignedInt));
      sCreateObjectMap.insert(ObjectMapType::value_type("int", (void*(*)())CreateInt));
      sCreateObjectMap.insert(ObjectMapType::value_type("unsigned long", (void*(*)())CreateUnsignedLong));
      sCreateObjectMap.insert(ObjectMapType::value_type("long", (void*(*)())CreateLong));
      sCreateObjectMap.insert(ObjectMapType::value_type("float", (void*(*)())CreateFloat));
      sCreateObjectMap.insert(ObjectMapType::value_type("double", (void*(*)())CreateDouble));
      sCreateObjectMap.insert(ObjectMapType::value_type("bool", (void*(*)())CreateBool));
      sCreateObjectMap.insert(ObjectMapType::value_type("string", (void*(*)())CreateString));
      sCreateObjectMap.insert(ObjectMapType::value_type("BadValues", (void*(*)())CreateBadValues));
      sCreateObjectMap.insert(ObjectMapType::value_type("BitMask", (void*(*)())CreateBitMask));
      sCreateObjectMap.insert(ObjectMapType::value_type("Classification", (void*(*)())CreateClassification));
      sCreateObjectMap.insert(ObjectMapType::value_type("DataRequest", (void*(*)())CreateDataRequest));
      sCreateObjectMap.insert(ObjectMapType::value_type("DataVariantAnyData", (void*(*)())CreateDataVariantAnyData));
      sCreateObjectMap.insert(ObjectMapType::value_type("DateTime", (void*(*)())CreateDateTime));
      sCreateObjectMap.insert(ObjectMapType::value_type("DynamicObject", (void*(*)())CreateDynamicObject));
      sCreateObjectMap.insert(ObjectMapType::value_type("FileDescriptor", (void*(*)())CreateFileDescriptor));
      sCreateObjectMap.insert(ObjectMapType::value_type("FileFinder", (void*(*)())CreateFileFinder));
      sCreateObjectMap.insert(ObjectMapType::value_type("Filename", (void*(*)())CreateFilename));
      sCreateObjectMap.insert(ObjectMapType::value_type("Font", (void*(*)())CreateFontObject));
      sCreateObjectMap.insert(ObjectMapType::value_type("PointCloudDataRequest", (void*(*)())CreatePointCloudDataRequest));
      sCreateObjectMap.insert(ObjectMapType::value_type("PointCloudFileDescriptor",
         (void*(*)())CreatePointCloudFileDescriptor));
      sCreateObjectMap.insert(ObjectMapType::value_type("GeoreferenceDescriptor",
         (void*(*)())CreateGeoreferenceDescriptor));
      sCreateObjectMap.insert(ObjectMapType::value_type("RasterFileDescriptor",
         (void*(*)())CreateRasterFileDescriptor));
      sCreateObjectMap.insert(ObjectMapType::value_type("SignatureFileDescriptor",
         (void*(*)())CreateSignatureFileDescriptor));
      sCreateObjectMap.insert(ObjectMapType::value_type("SettableSessionItem", (void*(*)())CreateSettableSessionItem));
      sCreateObjectMap.insert(ObjectMapType::value_type("ExecutableAgent", (void*(*)())CreateExecutableAgent));
      sCreateObjectMap.insert(ObjectMapType::value_type("ImportAgent", (void*(*)())CreateImportAgent));
      sCreateObjectMap.insert(ObjectMapType::value_type("ExportAgent", (void*(*)())CreateExportAgent));
      sCreateObjectMap.insert(ObjectMapType::value_type("Units", (void*(*)())CreateUnits));
      sCreateObjectMap.insert(ObjectMapType::value_type("Wavelengths", (void*(*)())CreateWavelengths));
      sCreateObjectMap.insert(ObjectMapType::value_type("WizardObject", (void*(*)())CreateWizardObject));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned char).name(), (void*(*)())CreateUnsignedChar));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(char).name(), (void*(*)())CreateChar));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned short).name(),
         (void*(*)())CreateUnsignedShort));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(short).name(), (void*(*)())CreateShort));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned int).name(), (void*(*)())CreateUnsignedInt));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(int).name(), (void*(*)())CreateInt));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned long).name(), (void*(*)())CreateUnsignedLong));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(long).name(), (void*(*)())CreateLong));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(float).name(), (void*(*)())CreateFloat));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(double).name(), (void*(*)())CreateDouble));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(bool).name(), (void*(*)())CreateBool));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(string).name(), (void*(*)())CreateString));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(BadValues*).name(), (void*(*)())CreateBadValues));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(BitMask*).name(), (void*(*)())CreateBitMask));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(Classification*).name(),
         (void*(*)())CreateClassification));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(DateTime*).name(), (void*(*)())CreateDateTime));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(DynamicObject*).name(),
         (void*(*)())CreateDynamicObject));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(FileFinder*).name(), (void*(*)())CreateFileFinder));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(Filename*).name(), (void*(*)())CreateFilename));
      sCreateObjectMap.insert(ObjectMapType::value_type(typeid(Units*).name(), (void*(*)())CreateUnits));

      sDestroyObjectMap.insert(ObjectMapType2::value_type("unsigned char", (void(*)(void*))DestroyUnsignedChar));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("char", (void(*)(void*))DestroyChar));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("unsigned short", (void(*)(void*))DestroyUnsignedShort));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("short", (void(*)(void*))DestroyShort));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("unsigned int", (void(*)(void*))DestroyUnsignedInt));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("int", (void(*)(void*))DestroyInt));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("unsigned long", (void(*)(void*))DestroyUnsignedLong));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("long", (void(*)(void*))DestroyLong));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("float", (void(*)(void*))DestroyFloat));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("double", (void(*)(void*))DestroyDouble));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("bool", (void(*)(void*))DestroyBool));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("string", (void(*)(void*))DestroyString));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("BadValues", (void(*)(void*))DestroyBadValues));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("BitMask", (void(*)(void*))DestroyBitMask));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("Classification", (void(*)(void*))DestroyClassification));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("DataRequest", (void(*)(void*))DestroyDataRequest));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("DataVariantAnyData",
         (void(*)(void*))DestroyDataVariantAnyData));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("DateTime", (void(*)(void*))DestroyDateTime));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("DynamicObject", (void(*)(void*))DestroyDynamicObject));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("FileDescriptor", (void(*)(void*))DestroyFileDescriptor));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("FileFinder", (void(*)(void*))DestroyFileFinder));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("Filename", (void(*)(void*))DestroyFilename));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("Font", (void(*)(void*))DestroyFontObject));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("PointCloudDataRequest", (void(*)(void*))DestroyPointCloudDataRequest));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("PointCloudFileDescriptor",
         (void(*)(void*))DestroyPointCloudFileDescriptor));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("GeoreferenceDescriptor",
         (void(*)(void*))DestroyGeoreferenceDescriptor));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("RasterFileDescriptor",
         (void(*)(void*))DestroyRasterFileDescriptor));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("SignatureFileDescriptor",
         (void(*)(void*))DestroySignatureFileDescriptor));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("SettableSessionItem",
         (void(*)(void*))DestroySettableSessionItem));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("ExecutableAgent", (void(*)(void*))DestroyExecutableAgent));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("ImportAgent", (void(*)(void*))DestroyImportAgent));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("ExportAgent", (void(*)(void*))DestroyExportAgent));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("Units", (void(*)(void*))DestroyUnits));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("Wavelengths", (void(*)(void*))DestroyWavelengths));
      sDestroyObjectMap.insert(ObjectMapType2::value_type("WizardObject", (void(*)(void*))DestroyWizardObject));

      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned char", (void*(*)())CreateVectorUnsignedChar));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("char", (void*(*)())CreateVectorChar));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned short", (void*(*)())CreateVectorUnsignedShort));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("short", (void*(*)())CreateVectorShort));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned int", (void*(*)())CreateVectorUnsignedInt));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("int", (void*(*)())CreateVectorInt));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned long", (void*(*)())CreateVectorUnsignedLong));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("long", (void*(*)())CreateVectorLong));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("float", (void*(*)())CreateVectorFloat));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("double", (void*(*)())CreateVectorDouble));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("bool", (void*(*)())CreateVectorBool));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("string", (void*(*)())CreateVectorString));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("void*", (void*(*)())CreateVectorVoidPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned char*",
         (void*(*)())CreateVectorUnsignedCharPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("char*", (void*(*)())CreateVectorCharPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned short*",
         (void*(*)())CreateVectorUnsignedShortPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("short*", (void*(*)())CreateVectorShortPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned int*", (void*(*)())CreateVectorUnsignedIntPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("int*", (void*(*)())CreateVectorIntPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("unsigned long*",
         (void*(*)())CreateVectorUnsignedLongPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("long*", (void*(*)())CreateVectorLongPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("float*", (void*(*)())CreateVectorFloatPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("double*", (void*(*)())CreateVectorDoublePtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("bool*", (void*(*)())CreateVectorBoolPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("string*", (void*(*)())CreateVectorStringPtr));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("GraphicObject", (void*(*)())CreateVectorGraphicObject));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("BadValues", (void*(*)())CreateVectorBadValues));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("BitMask", (void*(*)())CreateVectorBitMask));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("Classification",
         (void*(*)())CreateVectorClassification));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("DateTime", (void*(*)())CreateVectorDateTime));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("DynamicObject", (void*(*)())CreateVectorDynamicObject));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("FileDescriptor",
         (void*(*)())CreateVectorFileDescriptor));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("FileFinder", (void*(*)())CreateVectorFileFinder));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("Filename", (void*(*)())CreateVectorFilename));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("GeoreferenceDescriptor",
         (void*(*)())CreateVectorGeoreferenceDescriptor));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("Layer", (void*(*)())CreateVectorLayer));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("PointCloudFileDescriptor",
         (void*(*)())CreateVectorPointCloudFileDescriptor));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("RasterFileDescriptor",
         (void*(*)())CreateVectorRasterFileDescriptor));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("SignatureFileDescriptor",
         (void*(*)())CreateVectorSignatureFileDescriptor));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("Units", (void*(*)())CreateVectorUnits));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("Wavelengths", (void*(*)())CreateVectorWavelengths));
      sCreateObjectVectorMap.insert(VectorMapType::value_type("WizardObject", (void*(*)())CreateVectorWizardObject));

      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned char",
         (void(*)(void*))DestroyVectorUnsignedChar));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("char", (void(*)(void*))DestroyVectorChar));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned short",
         (void(*)(void*))DestroyVectorUnsignedShort));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("short", (void(*)(void*))DestroyVectorShort));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned int",
         (void(*)(void*))DestroyVectorUnsignedInt));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("int", (void(*)(void*))DestroyVectorInt));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned long",
         (void(*)(void*))DestroyVectorUnsignedLong));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("long", (void(*)(void*))DestroyVectorLong));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("float", (void(*)(void*))DestroyVectorFloat));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("double", (void(*)(void*))DestroyVectorDouble));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("bool", (void(*)(void*))DestroyVectorBool));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("string", (void(*)(void*))DestroyVectorString));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("void*", (void(*)(void*))DestroyVectorVoidPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned char*",
         (void(*)(void*))DestroyVectorUnsignedCharPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("char*", (void(*)(void*))DestroyVectorCharPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned short*",
         (void(*)(void*))DestroyVectorUnsignedShortPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("short*", (void(*)(void*))DestroyVectorShortPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned int*",
         (void(*)(void*))DestroyVectorUnsignedIntPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("int*", (void(*)(void*))DestroyVectorIntPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned long*",
         (void(*)(void*))DestroyVectorUnsignedLongPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("long*", (void(*)(void*))DestroyVectorLongPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("float*", (void(*)(void*))DestroyVectorFloatPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("double*", (void(*)(void*))DestroyVectorDoublePtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("bool*", (void(*)(void*))DestroyVectorBoolPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("string*", (void(*)(void*))DestroyVectorStringPtr));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("GraphicObject",
         (void(*)(void*))DestroyVectorGraphicObject));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("BadValues", (void(*)(void*))DestroyVectorBadValues));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("BitMask", (void(*)(void*))DestroyVectorBitMask));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("Classification",
         (void(*)(void*))DestroyVectorClassification));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("DateTime", (void(*)(void*))DestroyVectorDateTime));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("DynamicObject",
         (void(*)(void*))DestroyVectorDynamicObject));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("FileDescriptor",
         (void(*)(void*))DestroyVectorFileDescriptor));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("FileFinder", (void(*)(void*))DestroyVectorFileFinder));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("Filename", (void(*)(void*))DestroyVectorFilename));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("GeoreferenceDescriptor",
         (void(*)(void*))DestroyVectorGeoreferenceDescriptor));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("Layer", (void(*)(void*))DestroyVectorLayer));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("PointCloudFileDescriptor",
         (void(*)(void*))DestroyVectorPointCloudFileDescriptor));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("RasterFileDescriptor",
         (void(*)(void*))DestroyVectorRasterFileDescriptor));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("SignatureFileDescriptor",
         (void(*)(void*))DestroyVectorSignatureFileDescriptor));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("Wavelengths",
         (void(*)(void*))DestroyVectorWavelengths));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("Units", (void(*)(void*))DestroyVectorUnits));
      sDestroyObjectVectorMap.insert(VectorMapType2::value_type("WizardObject",
         (void(*)(void*))DestroyVectorWizardObject));
   }

   return spInstance;
}

void ObjectFactoryImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy ObjectFactory after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

void* ObjectFactoryImp::createObject(const std::string& className)
{
   // call createObjectVector if it is a vector
   string vectorName;
   if (parseClassString(className, vectorName))
   {
      return createObjectVector(vectorName);
   }

   map<string, void*(*)()>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = sCreateObjectMap.find(className);
   if (itr != sCreateObjectMap.end())
   {
      return ((*itr).second) ();
   }
  
   string msg = "ObjectFactory::createObject given unknown object type: '" + className + "'";
   VERIFYRV_MSG(false, NULL, msg.c_str());
}

void ObjectFactoryImp::destroyObject(void* pObject, const std::string& className)
{
   VERIFYNRV (pObject != NULL);

   // call destroyObjectVector if it is a vector
   string vectorName;
   if (parseClassString(className, vectorName))
   {
      destroyObjectVector(pObject, vectorName);
      return;
   }

   map<string, void(*)(void*)>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = sDestroyObjectMap.find(className);
   if (itr != sDestroyObjectMap.end())
   {
      ((*itr).second) (pObject);
      return;
   }

   string msg = "ObjectFactory::destroyObject given unknown object type: '" + className + "'";
   VERIFYNRV_MSG(false, msg.c_str());
}

void* ObjectFactoryImp::createObjectVector(const string& className)
{
   map<string, void*(*)()>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = sCreateObjectVectorMap.find (className);
   if (itr != sCreateObjectVectorMap.end())
   {
      return ((*itr).second) ();
   }

   string msg = "ObjectFactory::createObjectVector given unknown object type: '" + className + "'";
   VERIFYRV_MSG(false, NULL, msg.c_str());
}

void ObjectFactoryImp::destroyObjectVector(void* pVector, const string& className)
{
   VERIFYNRV (pVector!= NULL);

   map<string, void(*)(void*)>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = sDestroyObjectVectorMap.find (className);
   if (itr != sDestroyObjectVectorMap.end())
   {
      ((*itr).second) (pVector);
   }
   else
   {
      string msg = "ObjectFactory::destroyObjectVector given unknown object type: '" + className + "'";
      VERIFYNRV_MSG(false, msg.c_str());
   }
}

namespace
{
//=============================
// GraphicObject
void* CreateVectorGraphicObject()
{
   return new vector<GraphicObject*>();
}

void DestroyVectorGraphicObject(void* pObj)
{
   delete reinterpret_cast<vector<GraphicObject*>*>(pObj);
}

//=============================
// BadValues
BadValues* CreateBadValues()
{
   return new BadValuesAdapter();
}

void DestroyBadValues(void* pObj)
{
   delete reinterpret_cast<BadValuesAdapter*>(pObj);
}

void* CreateVectorBadValues()
{
   return new vector<BadValues*>();
}

void DestroyVectorBadValues(void* pObj)
{
   delete reinterpret_cast<vector<BadValues*>*>(pObj);
}

//=============================
// BitMask
BitMask* CreateBitMask()
{
   return new BitMaskImp();
}

void DestroyBitMask(void* pObj)
{
   delete reinterpret_cast<BitMaskImp*>(pObj);
}

void* CreateVectorBitMask()
{
   return new vector<BitMask*>();
}

void DestroyVectorBitMask(void* pObj)
{
   delete reinterpret_cast<vector<BitMask*>*>(pObj);
}

//=============================
// Classification
Classification* CreateClassification()
{
   return new ClassificationAdapter();
}

void DestroyClassification(void* pObj)
{
   delete reinterpret_cast<ClassificationAdapter*>(pObj);
}

void* CreateVectorClassification()
{
   return new vector<Classification*>();
}

void DestroyVectorClassification(void* pObj)
{
   delete reinterpret_cast<vector<Classification*>*>(pObj);
}

//=============================
// DataRequest
DataRequest* CreateDataRequest()
{
   return new DataRequestImp();
}

void DestroyDataRequest(void* pObj)
{
   delete reinterpret_cast<DataRequestImp*>(pObj);
}

//=============================
// PointCloudDataRequest
PointCloudDataRequest* CreatePointCloudDataRequest()
{
   return new PointCloudDataRequestImp();
}

void DestroyPointCloudDataRequest(void* pObj)
{
   delete reinterpret_cast<PointCloudDataRequestImp*>(pObj);
}

//=============================
// DataVariantAnyData
DataVariantAnyData* CreateDataVariantAnyData()
{
   return new DataVariantAnyData();
}

void DestroyDataVariantAnyData(void* pObj)
{
   delete reinterpret_cast<DataVariantAnyData*>(pObj);
}

//=============================
// DateTime
DateTime* CreateDateTime()
{
   return new DateTimeImp();
}

void DestroyDateTime(void* pObj)
{
   delete reinterpret_cast<DateTimeImp*>(pObj);
}

void* CreateVectorDateTime()
{
   return new vector<DateTime*>();
}

void DestroyVectorDateTime(void* pObj)
{
   delete reinterpret_cast<vector<DateTime*>*>(pObj);
}

//=============================
// DynamicObject
DynamicObject* CreateDynamicObject()
{
   return new DynamicObjectAdapter();
}

void DestroyDynamicObject(void* pObj)
{
   delete reinterpret_cast<DynamicObjectAdapter*>(pObj);
}

void* CreateVectorDynamicObject()
{
   return new vector<DynamicObject*>();
}

void DestroyVectorDynamicObject(void* pObj)
{
   delete reinterpret_cast<vector<DynamicObject*>*>(pObj);
}

//=============================
// FileDescriptor
FileDescriptor* CreateFileDescriptor()
{
   return new FileDescriptorAdapter();
}

void DestroyFileDescriptor(void* pObj)
{
   delete reinterpret_cast<FileDescriptorAdapter*>(pObj);
}

void* CreateVectorFileDescriptor()
{
   return new vector<FileDescriptor*>();
}

void DestroyVectorFileDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<FileDescriptor*>*>(pObj);
}

//=============================
// FileFinder
FileFinder* CreateFileFinder()
{
   return new FileFinderImp();
}

void DestroyFileFinder(void* pObj)
{
   delete reinterpret_cast<FileFinderImp*>(pObj);
}

void* CreateVectorFileFinder()
{
   return new vector<FileFinder*>();
}

void DestroyVectorFileFinder(void* pObj)
{
   delete reinterpret_cast<vector<FileFinder*>*>(pObj);
}

//=============================
// Filename
Filename* CreateFilename()
{
   return new FilenameImp();
}

void DestroyFilename(void* pObj)
{
   delete reinterpret_cast<FilenameImp*>(pObj);
}

void* CreateVectorFilename()
{
   return new vector<Filename*>();
}

void DestroyVectorFilename(void* pObj)
{
   delete reinterpret_cast<vector<Filename*>*>(pObj);
}

//=============================
// Font
Font* CreateFontObject()
{
   return new FontImp();
}

void DestroyFontObject(void* pObj)
{
   delete reinterpret_cast<FontImp*>(pObj);
}

//=============================
// GeoreferenceDescriptor
GeoreferenceDescriptor* CreateGeoreferenceDescriptor()
{
   return new GeoreferenceDescriptorAdapter();
}

void DestroyGeoreferenceDescriptor(void* pObj)
{
   delete reinterpret_cast<GeoreferenceDescriptorAdapter*>(pObj);
}

void* CreateVectorGeoreferenceDescriptor()
{
   return new vector<GeoreferenceDescriptor*>();
}

void DestroyVectorGeoreferenceDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<GeoreferenceDescriptor*>*>(pObj);
}

//=============================
// Layer
void* CreateVectorLayer()
{
   return new vector<Layer*>();
}

void DestroyVectorLayer(void* pObj)
{
   delete reinterpret_cast<vector<Layer*>*>(pObj);
}

//=============================
// PointCloudFileDescriptor
PointCloudFileDescriptor* CreatePointCloudFileDescriptor()
{
   return new PointCloudFileDescriptorAdapter();
}

void DestroyPointCloudFileDescriptor(void* pObj)
{
   delete reinterpret_cast<PointCloudFileDescriptorAdapter*>(pObj);
}

void* CreateVectorPointCloudFileDescriptor()
{
   return new vector<PointCloudFileDescriptor*>();
}

void DestroyVectorPointCloudFileDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<PointCloudFileDescriptor*>*>(pObj);
}


//=============================
// RasterFileDescriptor
RasterFileDescriptor* CreateRasterFileDescriptor()
{
   return new RasterFileDescriptorAdapter();
}

void DestroyRasterFileDescriptor(void* pObj)
{
   delete reinterpret_cast<RasterFileDescriptorAdapter*>(pObj);
}

void* CreateVectorRasterFileDescriptor()
{
   return new vector<RasterFileDescriptor*>();
}

void DestroyVectorRasterFileDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<RasterFileDescriptor*>*>(pObj);
}

//=============================
// SettableSessionItem
SettableSessionItem* CreateSettableSessionItem()
{
   return new SettableSessionItemAdapter(SessionItemImp::generateUniqueId());
}

void DestroySettableSessionItem(void* pObj)
{
   delete reinterpret_cast<SettableSessionItemAdapter*>(pObj);
}

//=============================
// SignatureFileDescriptor
SignatureFileDescriptor* CreateSignatureFileDescriptor()
{
   return new SignatureFileDescriptorAdapter();
}

void DestroySignatureFileDescriptor(void* pObj)
{
   delete reinterpret_cast<SignatureFileDescriptorAdapter*>(pObj);
}

void* CreateVectorSignatureFileDescriptor()
{
   return new vector<SignatureFileDescriptor*>();
}

void DestroyVectorSignatureFileDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<SignatureFileDescriptor*>*>(pObj);
}

//=============================
// ExecutableAgent
ExecutableAgent* CreateExecutableAgent()
{
   return new ExecutableAgentAdapter();
}

void DestroyExecutableAgent(void* pObj)
{
   delete reinterpret_cast<ExecutableAgentAdapter*>(pObj);
}

//=============================
// ImportAgent
ImportAgent* CreateImportAgent()
{
   return new ImportAgentAdapter();
}

void DestroyImportAgent(void* pObj)
{
   delete reinterpret_cast<ImportAgentAdapter*>(pObj);
}

//=============================
// ExportAgent
ExportAgent* CreateExportAgent()
{
   return new ExportAgentAdapter();
}

void DestroyExportAgent(void* pObj)
{
   delete reinterpret_cast<ExportAgentAdapter*>(pObj);
}

//=============================
// Units
Units* CreateUnits()
{
   return new UnitsAdapter();
}

void DestroyUnits(void* pObj)
{
   delete reinterpret_cast<UnitsAdapter*>(pObj);
}

void* CreateVectorUnits()
{
   return new vector<Units*>();
}

void DestroyVectorUnits(void* pObj)
{
   delete reinterpret_cast<vector<Units*>*>(pObj);
}

//=============================
// Wavelengths
Wavelengths* CreateWavelengths()
{
   return new WavelengthsImp();
}

void DestroyWavelengths(void* pObj)
{
   delete reinterpret_cast<WavelengthsImp*>(pObj);
}

void* CreateVectorWavelengths()
{
   return new vector<Wavelengths*>();
}

void DestroyVectorWavelengths(void* pObj)
{
   delete reinterpret_cast<vector<Wavelengths*>*>(pObj);
}

//=============================
// WizardObject
WizardObject* CreateWizardObject()
{
   return new WizardObjectAdapter();
}

void DestroyWizardObject(void* pObj)
{
   delete reinterpret_cast<WizardObjectAdapter*>(pObj);
}

static void* CreateVectorWizardObject()
{
   return new vector<WizardObject*>();
}

void DestroyVectorWizardObject(void* pObj)
{
   delete reinterpret_cast<vector<WizardObject*>*>(pObj);
}

//=============================
// Basic types
void* CreateUnsignedChar()
{
   return new unsigned char();
}

void* CreateChar()
{
   return new char();
}

void* CreateUnsignedShort()
{
   return new unsigned short();
}

void* CreateShort()
{
   return new short();
}

void* CreateUnsignedInt()
{
   return new unsigned int();
}

void* CreateInt()
{
   return new int();
}

void* CreateUnsignedLong()
{
   return new unsigned long();
}

void* CreateLong()
{
   return new long();
}

void* CreateFloat()
{
   return new float();
}

void* CreateDouble()
{
   return new double();
}

void* CreateBool()
{
   return new bool();
}

void* CreateString()
{
   return new string();
}

void DestroyUnsignedChar(void* pObj)
{
   delete reinterpret_cast<unsigned char*>(pObj);
}

void DestroyChar(void* pObj)
{
   delete reinterpret_cast<char*>(pObj);
}

void DestroyUnsignedShort(void* pObj)
{
   delete reinterpret_cast<unsigned short*>(pObj);
}

void DestroyShort(void* pObj)
{
   delete reinterpret_cast<short*>(pObj);
}

void DestroyUnsignedInt(void* pObj)
{
   delete reinterpret_cast<unsigned int*>(pObj);
}

void DestroyInt(void* pObj)
{
   delete reinterpret_cast<int*>(pObj);
}

void DestroyUnsignedLong(void* pObj)
{
   delete reinterpret_cast<unsigned long*>(pObj);
}

void DestroyLong(void* pObj)
{
   delete reinterpret_cast<long*>(pObj);
}

void DestroyFloat(void* pObj)
{
   delete reinterpret_cast<float*>(pObj);
}

void DestroyDouble(void* pObj)
{
   delete reinterpret_cast<double*>(pObj);
}

void DestroyBool(void* pObj)
{
   delete reinterpret_cast<bool*>(pObj);
}

void DestroyString(void* pObj)
{
   delete reinterpret_cast<string*>(pObj);
}

void* CreateVectorUnsignedChar()
{
   return new vector<unsigned char>();
}

void* CreateVectorChar()
{
   return new vector<char>();
}

void* CreateVectorUnsignedShort()
{
   return new vector<unsigned short>();
}

void* CreateVectorShort()
{
   return new vector<short>();
}

void* CreateVectorUnsignedInt()
{
   return new vector<unsigned int>();
}

void* CreateVectorInt()
{
   return new vector<int>();
}

void* CreateVectorUnsignedLong()
{
   return new vector<unsigned long>();
}

void* CreateVectorLong()
{
   return new vector<long>();
}

void* CreateVectorFloat()
{
   return new vector<float>();
}

void* CreateVectorDouble()
{
   return new vector<double>();
}

void* CreateVectorBool()
{
   return new vector<bool>();
}

void* CreateVectorString()
{
   return new vector<string>();
}

void* CreateVectorVoidPtr()
{
   return new vector<void*>();
}

void* CreateVectorUnsignedCharPtr()
{
   return new vector<unsigned char*>();
}

void* CreateVectorCharPtr()
{
   return new vector<char*>();
}

void* CreateVectorUnsignedShortPtr()
{
   return new vector<unsigned short*>();
}

void* CreateVectorShortPtr()
{
   return new vector<short*>();
}

void* CreateVectorUnsignedIntPtr()
{
   return new vector<unsigned int*>();
}

void* CreateVectorIntPtr()
{
   return new vector<int*>();
}

void* CreateVectorUnsignedLongPtr()
{
   return new vector<unsigned long*>();
}

void* CreateVectorLongPtr()
{
   return new vector<long*>();
}

void* CreateVectorFloatPtr()
{
   return new vector<float*>();
}

void* CreateVectorDoublePtr()
{
   return new vector<double*>();
}

void* CreateVectorBoolPtr()
{
   return new vector<bool*>();
}

void* CreateVectorStringPtr()
{
   return new vector<string*>();
}

void DestroyVectorUnsignedChar(void* pObj)
{
   delete reinterpret_cast<vector<unsigned char>*>(pObj);
}

void DestroyVectorChar(void* pObj)
{
   delete reinterpret_cast<vector<char>*>(pObj);
}

void DestroyVectorUnsignedShort(void* pObj)
{
   delete reinterpret_cast<vector<unsigned short>*>(pObj);
}

void DestroyVectorShort(void* pObj)
{
   delete reinterpret_cast<vector<short>*>(pObj);
}

void DestroyVectorUnsignedInt(void* pObj)
{
   delete reinterpret_cast<vector<unsigned int>*>(pObj);
}

void DestroyVectorInt(void* pObj)
{
   delete reinterpret_cast<vector<int>*>(pObj);
}

void DestroyVectorUnsignedLong(void* pObj)
{
   delete reinterpret_cast<vector<unsigned long>*>(pObj);
}

void DestroyVectorLong(void* pObj)
{
   delete reinterpret_cast<vector<long>*>(pObj);
}

void DestroyVectorFloat(void* pObj)
{
   delete reinterpret_cast<vector<float>*>(pObj);
}

void DestroyVectorDouble(void* pObj)
{
   delete reinterpret_cast<vector<double>*>(pObj);
}

void DestroyVectorBool(void* pObj)
{
   delete reinterpret_cast<vector<bool>*>(pObj);
}

void DestroyVectorString(void* pObj)
{
   delete reinterpret_cast<vector<string>*>(pObj);
}

void DestroyVectorVoidPtr(void* pObj)
{
   delete reinterpret_cast<vector<void*>*>(pObj);
}

void DestroyVectorUnsignedCharPtr(void* pObj)
{
   delete reinterpret_cast<vector<unsigned char*>*>(pObj);
}

void DestroyVectorCharPtr(void* pObj)
{
   delete reinterpret_cast<vector<char*>*>(pObj);
}

void DestroyVectorUnsignedShortPtr(void* pObj)
{
   delete reinterpret_cast<vector<unsigned short*>*>(pObj);
}

void DestroyVectorShortPtr(void* pObj)
{
   delete reinterpret_cast<vector<short*>*>(pObj);
}

void DestroyVectorUnsignedIntPtr(void* pObj)
{
   delete reinterpret_cast<vector<unsigned int*>*>(pObj);
}

void DestroyVectorIntPtr(void* pObj)
{
   delete reinterpret_cast<vector<int*>*>(pObj);
}

void DestroyVectorUnsignedLongPtr(void* pObj)
{
   delete reinterpret_cast<vector<unsigned long*>*>(pObj);
}

void DestroyVectorLongPtr(void* pObj)
{
   delete reinterpret_cast<vector<long*>*>(pObj);
}

void DestroyVectorFloatPtr(void* pObj)
{
   delete reinterpret_cast<vector<float*>*>(pObj);
}

void DestroyVectorDoublePtr(void* pObj)
{
   delete reinterpret_cast<vector<double*>*>(pObj);
}

void DestroyVectorBoolPtr(void* pObj)
{
   delete reinterpret_cast<vector<bool*>*>(pObj);
}

void DestroyVectorStringPtr(void* pObj)
{
   delete reinterpret_cast<vector<string*>*>(pObj);
}

} // end of anonymous namespace
