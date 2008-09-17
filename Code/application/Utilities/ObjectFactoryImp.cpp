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
#include "GraphicObject.h"
#include "ImportAgentAdapter.h"
#include "Layer.h"
#include "RasterFileDescriptorAdapter.h"
#include "SettableSessionItemAdapter.h"
#include "TypeConverter.h"
#include "UnitsImp.h"
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

   void* CreateVectorLayer();
   void DestroyVectorLayer(void* pObj);

   static RasterFileDescriptor* CreateRasterFileDescriptor();
   static void DestroyRasterFileDescriptor(void* pObj);
   static void* CreateVectorRasterFileDescriptor();
   static void DestroyVectorRasterFileDescriptor(void* pObj);

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

   static WizardObject* CreateWizardObject();
   static void DestroyWizardObject(void* pObj);
   static void* CreateVectorWizardObject();
   static void DestroyVectorWizardObject(void* pObj);

   void* CreateUnsignedChar ();
   void* CreateChar ();
   void* CreateUnsignedShort ();
   void* CreateShort ();
   void* CreateUnsignedInt ();
   void* CreateInt ();
   void* CreateUnsignedLong ();
   void* CreateLong ();
   void* CreateFloat ();
   void* CreateDouble ();
   void* CreateBool ();
   void* CreateString ();

   void DestroyUnsignedChar (void*);
   void DestroyChar (void*);
   void DestroyUnsignedShort (void*);
   void DestroyShort (void*);
   void DestroyUnsignedInt (void*);
   void DestroyInt (void*);
   void DestroyUnsignedLong (void*);
   void DestroyLong (void*);
   void DestroyFloat (void*);
   void DestroyDouble (void*);
   void DestroyBool (void*);
   void DestroyString (void*);

   void* CreateVectorUnsignedChar ();
   void* CreateVectorChar ();
   void* CreateVectorUnsignedShort ();
   void* CreateVectorShort ();
   void* CreateVectorUnsignedInt ();
   void* CreateVectorInt ();
   void* CreateVectorUnsignedLong ();
   void* CreateVectorLong ();
   void* CreateVectorFloat ();
   void* CreateVectorDouble ();
   void* CreateVectorBool ();
   void* CreateVectorString ();
   void* CreateVectorVoidPtr ();
   void* CreateVectorUnsignedCharPtr ();
   void* CreateVectorCharPtr ();
   void* CreateVectorUnsignedShortPtr ();
   void* CreateVectorShortPtr ();
   void* CreateVectorUnsignedIntPtr ();
   void* CreateVectorIntPtr ();
   void* CreateVectorUnsignedLongPtr ();
   void* CreateVectorLongPtr ();
   void* CreateVectorFloatPtr ();
   void* CreateVectorDoublePtr ();
   void* CreateVectorBoolPtr ();
   void* CreateVectorStringPtr ();

   void DestroyVectorUnsignedChar (void*);
   void DestroyVectorChar (void*);
   void DestroyVectorUnsignedShort (void*);
   void DestroyVectorShort (void*);
   void DestroyVectorUnsignedInt (void*);
   void DestroyVectorInt (void*);
   void DestroyVectorUnsignedLong (void*);
   void DestroyVectorLong (void*);
   void DestroyVectorFloat (void*);
   void DestroyVectorDouble (void*);
   void DestroyVectorBool (void*);
   void DestroyVectorString (void*);
   void DestroyVectorVoidPtr (void*);
   void DestroyVectorUnsignedCharPtr (void*);
   void DestroyVectorCharPtr (void*);
   void DestroyVectorUnsignedShortPtr (void*);
   void DestroyVectorShortPtr (void*);
   void DestroyVectorUnsignedIntPtr (void*);
   void DestroyVectorIntPtr (void*);
   void DestroyVectorUnsignedLongPtr (void*);
   void DestroyVectorLongPtr (void*);
   void DestroyVectorFloatPtr (void*);
   void DestroyVectorDoublePtr (void*);
   void DestroyVectorBoolPtr (void*);
   void DestroyVectorStringPtr (void*);
}

typedef  map<string,void*(*)()> ObjectMapType;
typedef  map<string,void*(*)()> VectorMapType;
typedef  map<string,void(*)(void*)> ObjectMapType2;
typedef  map<string,void(*)(void*)> VectorMapType2;
static ObjectMapType CreateObjectMap;
static ObjectMapType2 DestroyObjectMap;
static VectorMapType CreateObjectVectorMap;
static VectorMapType2 DestroyObjectVectorMap;

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
      if(mDestroyed)
      {
         throw std::logic_error("Attempting to use ObjectFactory after "
            "destroying it.");
      }
      spInstance = new ObjectFactoryImp;

      CreateObjectMap.insert(ObjectMapType::value_type("unsigned char", (void*(*)())CreateUnsignedChar));
      CreateObjectMap.insert(ObjectMapType::value_type("char", (void*(*)())CreateChar));
      CreateObjectMap.insert(ObjectMapType::value_type("unsigned short", (void*(*)())CreateUnsignedShort));
      CreateObjectMap.insert(ObjectMapType::value_type("short", (void*(*)())CreateShort));
      CreateObjectMap.insert(ObjectMapType::value_type("unsigned int", (void*(*)())CreateUnsignedInt));
      CreateObjectMap.insert(ObjectMapType::value_type("int", (void*(*)())CreateInt));
      CreateObjectMap.insert(ObjectMapType::value_type("unsigned long", (void*(*)())CreateUnsignedLong));
      CreateObjectMap.insert(ObjectMapType::value_type("long", (void*(*)())CreateLong));
      CreateObjectMap.insert(ObjectMapType::value_type("float", (void*(*)())CreateFloat));
      CreateObjectMap.insert(ObjectMapType::value_type("double", (void*(*)())CreateDouble));
      CreateObjectMap.insert(ObjectMapType::value_type("bool", (void*(*)())CreateBool));
      CreateObjectMap.insert(ObjectMapType::value_type("string", (void*(*)())CreateString));
      CreateObjectMap.insert(ObjectMapType::value_type("BitMask", (void*(*)())CreateBitMask));
      CreateObjectMap.insert(ObjectMapType::value_type("Classification", (void*(*)())CreateClassification));
      CreateObjectMap.insert(ObjectMapType::value_type("DataRequest", (void*(*)())CreateDataRequest));
      CreateObjectMap.insert(ObjectMapType::value_type("DataVariantAnyData", (void*(*)())CreateDataVariantAnyData));
      CreateObjectMap.insert(ObjectMapType::value_type("DateTime", (void*(*)())CreateDateTime));
      CreateObjectMap.insert(ObjectMapType::value_type("DynamicObject", (void*(*)())CreateDynamicObject));
      CreateObjectMap.insert(ObjectMapType::value_type("FileDescriptor", (void*(*)())CreateFileDescriptor));
      CreateObjectMap.insert(ObjectMapType::value_type("FileFinder", (void*(*)())CreateFileFinder));
      CreateObjectMap.insert(ObjectMapType::value_type("Filename", (void*(*)())CreateFilename));
      CreateObjectMap.insert(ObjectMapType::value_type("Font", (void*(*)())CreateFontObject));
      CreateObjectMap.insert(ObjectMapType::value_type("RasterFileDescriptor", (void*(*)())CreateRasterFileDescriptor));
      CreateObjectMap.insert(ObjectMapType::value_type("SettableSessionItem", (void*(*)())CreateSettableSessionItem));
      CreateObjectMap.insert(ObjectMapType::value_type("ExecutableAgent", (void*(*)())CreateExecutableAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("ExecutableAgentCommon", (void*(*)())CreateExecutableAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("ExecutableAgentCommon1", (void*(*)())CreateExecutableAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("ImportAgent", (void*(*)())CreateImportAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("ImportAgentCommon", (void*(*)())CreateImportAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("ExportAgent", (void*(*)())CreateExportAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("ExportAgentCommon", (void*(*)())CreateExportAgent));
      CreateObjectMap.insert(ObjectMapType::value_type("Units", (void*(*)())CreateUnits));
      CreateObjectMap.insert(ObjectMapType::value_type("WizardObject", (void*(*)())CreateWizardObject));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned char).name(), (void*(*)())CreateUnsignedChar));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(char).name(), (void*(*)())CreateChar));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned short).name(), (void*(*)())CreateUnsignedShort));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(short).name(), (void*(*)())CreateShort));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned int).name(), (void*(*)())CreateUnsignedInt));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(int).name(), (void*(*)())CreateInt));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(unsigned long).name(), (void*(*)())CreateUnsignedLong));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(long).name(), (void*(*)())CreateLong));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(float).name(), (void*(*)())CreateFloat));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(double).name(), (void*(*)())CreateDouble));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(bool).name(), (void*(*)())CreateBool));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(string).name(), (void*(*)())CreateString));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(BitMask*).name(), (void*(*)())CreateBitMask));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(Classification*).name(), (void*(*)())CreateClassification));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(DateTime*).name(), (void*(*)())CreateDateTime));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(DynamicObject*).name(), (void*(*)())CreateDynamicObject));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(FileFinder*).name(), (void*(*)())CreateFileFinder));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(Filename*).name(), (void*(*)())CreateFilename));
      CreateObjectMap.insert(ObjectMapType::value_type(typeid(Units*).name(), (void*(*)())CreateUnits));

      DestroyObjectMap.insert(ObjectMapType2::value_type("unsigned char", (void(*)(void*))DestroyUnsignedChar));
      DestroyObjectMap.insert(ObjectMapType2::value_type("char", (void(*)(void*))DestroyChar));
      DestroyObjectMap.insert(ObjectMapType2::value_type("unsigned short", (void(*)(void*))DestroyUnsignedShort));
      DestroyObjectMap.insert(ObjectMapType2::value_type("short", (void(*)(void*))DestroyShort));
      DestroyObjectMap.insert(ObjectMapType2::value_type("unsigned int", (void(*)(void*))DestroyUnsignedInt));
      DestroyObjectMap.insert(ObjectMapType2::value_type("int", (void(*)(void*))DestroyInt));
      DestroyObjectMap.insert(ObjectMapType2::value_type("unsigned long", (void(*)(void*))DestroyUnsignedLong));
      DestroyObjectMap.insert(ObjectMapType2::value_type("long", (void(*)(void*))DestroyLong));
      DestroyObjectMap.insert(ObjectMapType2::value_type("float", (void(*)(void*))DestroyFloat));
      DestroyObjectMap.insert(ObjectMapType2::value_type("double", (void(*)(void*))DestroyDouble));
      DestroyObjectMap.insert(ObjectMapType2::value_type("bool", (void(*)(void*))DestroyBool));
      DestroyObjectMap.insert(ObjectMapType2::value_type("string", (void(*)(void*))DestroyString));
      DestroyObjectMap.insert(ObjectMapType2::value_type("BitMask", (void(*)(void*))DestroyBitMask));
      DestroyObjectMap.insert(ObjectMapType2::value_type("Classification", (void(*)(void*))DestroyClassification));
      DestroyObjectMap.insert(ObjectMapType2::value_type("DataRequest", (void(*)(void*))DestroyDataRequest));
      DestroyObjectMap.insert(ObjectMapType2::value_type("DataVariantAnyData", (void(*)(void*))DestroyDataVariantAnyData));
      DestroyObjectMap.insert(ObjectMapType2::value_type("DateTime", (void(*)(void*))DestroyDateTime));
      DestroyObjectMap.insert(ObjectMapType2::value_type("DynamicObject", (void(*)(void*))DestroyDynamicObject));
      DestroyObjectMap.insert(ObjectMapType2::value_type("FileDescriptor", (void(*)(void*))DestroyFileDescriptor));
      DestroyObjectMap.insert(ObjectMapType2::value_type("FileFinder", (void(*)(void*))DestroyFileFinder));
      DestroyObjectMap.insert(ObjectMapType2::value_type("Filename", (void(*)(void*))DestroyFilename));
      DestroyObjectMap.insert(ObjectMapType2::value_type("Font", (void(*)(void*))DestroyFontObject));
      DestroyObjectMap.insert(ObjectMapType2::value_type("RasterFileDescriptor", (void(*)(void*))DestroyRasterFileDescriptor));
      DestroyObjectMap.insert(ObjectMapType2::value_type("SettableSessionItem", (void(*)(void*))DestroySettableSessionItem));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ExecutableAgent", (void(*)(void*))DestroyExecutableAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ExecutableAgentCommon", (void(*)(void*))DestroyExecutableAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ExecutableAgentCommon1", (void(*)(void*))DestroyExecutableAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ImportAgent", (void(*)(void*))DestroyImportAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ImportAgentCommon", (void(*)(void*))DestroyImportAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ExportAgent", (void(*)(void*))DestroyExportAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("ExportAgentCommon", (void(*)(void*))DestroyExportAgent));
      DestroyObjectMap.insert(ObjectMapType2::value_type("Units", (void(*)(void*))DestroyUnits));
      DestroyObjectMap.insert(ObjectMapType2::value_type("WizardObject", (void(*)(void*))DestroyWizardObject));

      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned char", (void*(*)())CreateVectorUnsignedChar));
      CreateObjectVectorMap.insert(VectorMapType::value_type("char", (void*(*)())CreateVectorChar));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned short", (void*(*)())CreateVectorUnsignedShort));
      CreateObjectVectorMap.insert(VectorMapType::value_type("short", (void*(*)())CreateVectorShort));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned int", (void*(*)())CreateVectorUnsignedInt));
      CreateObjectVectorMap.insert(VectorMapType::value_type("int", (void*(*)())CreateVectorInt));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned long", (void*(*)())CreateVectorUnsignedLong));
      CreateObjectVectorMap.insert(VectorMapType::value_type("long", (void*(*)())CreateVectorLong));
      CreateObjectVectorMap.insert(VectorMapType::value_type("float", (void*(*)())CreateVectorFloat));
      CreateObjectVectorMap.insert(VectorMapType::value_type("double", (void*(*)())CreateVectorDouble));
      CreateObjectVectorMap.insert(VectorMapType::value_type("bool", (void*(*)())CreateVectorBool));
      CreateObjectVectorMap.insert(VectorMapType::value_type("string", (void*(*)())CreateVectorString));
      CreateObjectVectorMap.insert(VectorMapType::value_type("void*", (void*(*)())CreateVectorVoidPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned char*", (void*(*)())CreateVectorUnsignedCharPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("char*", (void*(*)())CreateVectorCharPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned short*", (void*(*)())CreateVectorUnsignedShortPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("short*", (void*(*)())CreateVectorShortPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned int*", (void*(*)())CreateVectorUnsignedIntPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("int*", (void*(*)())CreateVectorIntPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("unsigned long*", (void*(*)())CreateVectorUnsignedLongPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("long*", (void*(*)())CreateVectorLongPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("float*", (void*(*)())CreateVectorFloatPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("double*", (void*(*)())CreateVectorDoublePtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("bool*", (void*(*)())CreateVectorBoolPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("string*", (void*(*)())CreateVectorStringPtr));
      CreateObjectVectorMap.insert(VectorMapType::value_type("GraphicObject", (void*(*)())CreateVectorGraphicObject));
      CreateObjectVectorMap.insert(VectorMapType::value_type("BitMask", (void*(*)())CreateVectorBitMask));
      CreateObjectVectorMap.insert(VectorMapType::value_type("Classification", (void*(*)())CreateVectorClassification));
      CreateObjectVectorMap.insert(VectorMapType::value_type("DateTime", (void*(*)())CreateVectorDateTime));
      CreateObjectVectorMap.insert(VectorMapType::value_type("DynamicObject", (void*(*)())CreateVectorDynamicObject));
      CreateObjectVectorMap.insert(VectorMapType::value_type("FileDescriptor", (void*(*)())CreateVectorFileDescriptor));
      CreateObjectVectorMap.insert(VectorMapType::value_type("FileFinder", (void*(*)())CreateVectorFileFinder));
      CreateObjectVectorMap.insert(VectorMapType::value_type("Filename", (void*(*)())CreateVectorFilename));
      CreateObjectVectorMap.insert(VectorMapType::value_type("Layer", (void*(*)())CreateVectorLayer));
      CreateObjectVectorMap.insert(VectorMapType::value_type("RasterFileDescriptor", (void*(*)())CreateVectorRasterFileDescriptor));
      CreateObjectVectorMap.insert(VectorMapType::value_type("Units", (void*(*)())CreateVectorUnits));
      CreateObjectVectorMap.insert(VectorMapType::value_type("WizardObject", (void*(*)())CreateVectorWizardObject));

      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned char", (void(*)(void*))DestroyVectorUnsignedChar));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("char", (void(*)(void*))DestroyVectorChar));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned short", (void(*)(void*))DestroyVectorUnsignedShort));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("short", (void(*)(void*))DestroyVectorShort));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned int", (void(*)(void*))DestroyVectorUnsignedInt));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("int", (void(*)(void*))DestroyVectorInt));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned long", (void(*)(void*))DestroyVectorUnsignedLong));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("long", (void(*)(void*))DestroyVectorLong));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("float", (void(*)(void*))DestroyVectorFloat));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("double", (void(*)(void*))DestroyVectorDouble));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("bool", (void(*)(void*))DestroyVectorBool));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("string", (void(*)(void*))DestroyVectorString));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("void*", (void(*)(void*))DestroyVectorVoidPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned char*", (void(*)(void*))DestroyVectorUnsignedCharPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("char*", (void(*)(void*))DestroyVectorCharPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned short*", (void(*)(void*))DestroyVectorUnsignedShortPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("short*", (void(*)(void*))DestroyVectorShortPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned int*", (void(*)(void*))DestroyVectorUnsignedIntPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("int*", (void(*)(void*))DestroyVectorIntPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("unsigned long*", (void(*)(void*))DestroyVectorUnsignedLongPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("long*", (void(*)(void*))DestroyVectorLongPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("float*", (void(*)(void*))DestroyVectorFloatPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("double*", (void(*)(void*))DestroyVectorDoublePtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("bool*", (void(*)(void*))DestroyVectorBoolPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("string*", (void(*)(void*))DestroyVectorStringPtr));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("GraphicObject", (void(*)(void*))DestroyVectorGraphicObject));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("BitMask", (void(*)(void*))DestroyVectorBitMask));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("Classification", (void(*)(void*))DestroyVectorClassification));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("DateTime", (void(*)(void*))DestroyVectorDateTime));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("DynamicObject", (void(*)(void*))DestroyVectorDynamicObject));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("FileDescriptor", (void(*)(void*))DestroyVectorFileDescriptor));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("FileFinder", (void(*)(void*))DestroyVectorFileFinder));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("Filename", (void(*)(void*))DestroyVectorFilename));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("Layer", (void(*)(void*))DestroyVectorLayer));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("RasterFileDescriptor", (void(*)(void*))DestroyVectorRasterFileDescriptor));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("Units", (void(*)(void*))DestroyVectorUnits));
      DestroyObjectVectorMap.insert(VectorMapType2::value_type("WizardObject", (void(*)(void*))DestroyVectorWizardObject));
   }

   return spInstance;
}

void ObjectFactoryImp::destroy()
{
   if(mDestroyed)
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
   if (parseClassString(className, vectorName)) {
      return createObjectVector(vectorName);
   }

   map<string,void*(*)()>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = CreateObjectMap.find(className);
   if (itr != CreateObjectMap.end())
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
   if (parseClassString(className, vectorName)) {
      destroyObjectVector(pObject, vectorName);
      return;
   }

   map<string,void(*)(void*)>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = DestroyObjectMap.find(className);
   if (itr != DestroyObjectMap.end())
   {
      ((*itr).second) (pObject);
      return;
   }

   string msg = "ObjectFactory::destroyObject given unknown object type: '" + className + "'";
   VERIFYNRV_MSG(false, msg.c_str());
}

void* ObjectFactoryImp::createObjectVector (const string& className)
{
   map<string,void*(*)()>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = CreateObjectVectorMap.find (className);
   if (itr != CreateObjectVectorMap.end())
   {
      return ((*itr).second) ();
   }

   string msg = "ObjectFactory::createObjectVector given unknown object type: '" + className + "'";
   VERIFYRV_MSG(false, NULL, msg.c_str());
}

void ObjectFactoryImp::destroyObjectVector (void* pVector, const string& className)
{
   VERIFYNRV (pVector!= NULL);

   map<string,void(*)(void*)>::iterator itr;

   // Fabricate the entry point name, using the class name argument
   itr = DestroyObjectVectorMap.find (className);
   if (itr != DestroyObjectVectorMap.end())
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
// For maintenance ease, please keep the following 
// alphabetized by object class name.
//=============================
void* 
CreateVectorGraphicObject()
{
   return new vector<GraphicObject*>;
}

void
DestroyVectorGraphicObject(void* pObj)
{
    vector<GraphicObject*>* myObj = (vector<GraphicObject*>*) pObj;
   delete myObj;
}
//=============================
BitMask* 
CreateBitMask()
{
   return new BitMaskImp;
}

void
DestroyBitMask(void* pObj)
{
   delete (BitMaskImp*) pObj;
}

void* 
CreateVectorBitMask()
{
   return new vector<BitMask*>;
}

void
DestroyVectorBitMask(void* pObj)
{
    vector<BitMask*>* myObj = (vector<BitMask*>*) pObj;
   delete myObj;
}
//=============================
Classification* 
CreateClassification()
{
   return new ClassificationAdapter;
}

void
DestroyClassification(void* pObj)
{
   delete (ClassificationAdapter*) pObj;
}

void* 
CreateVectorClassification()
{
   return new vector<Classification*>;
}

void
DestroyVectorClassification(void* pObj)
{
    vector<Classification*>* myObj = (vector<Classification*>*) pObj;
   delete myObj;
}

//=============================
// DataRequest
DataRequest* 
CreateDataRequest()
{
   return new DataRequestImp;
}

void
DestroyDataRequest(void* pObj)
{
   delete reinterpret_cast<DataRequestImp*>(pObj);
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
DateTime*
CreateDateTime()
{
   return new DateTimeImp;
}

void
DestroyDateTime(void* pObj)
{
   delete (DateTimeImp*) pObj;
}

void*
CreateVectorDateTime()
{
   return new vector<DateTime*>;
}

void
DestroyVectorDateTime(void* pObj)
{
   vector<DateTime*>* myObj = (vector<DateTime*>*) pObj;
   delete myObj;
}
//=============================
DynamicObject*
CreateDynamicObject()
{
   return new DynamicObjectAdapter;
}

void
DestroyDynamicObject(void* pObj)
{
   delete (DynamicObjectAdapter*) pObj;
}

void*
CreateVectorDynamicObject()
{
   return new vector<DynamicObject*>;
}

void
DestroyVectorDynamicObject(void* pObj)
{
   vector<DynamicObject*>* myObj = (vector<DynamicObject*>*) pObj;
   delete myObj;
}
//=============================
// FileDescriptor
FileDescriptor* CreateFileDescriptor()
{
   return new FileDescriptorAdapter;
}

void DestroyFileDescriptor(void* pObj)
{
   delete reinterpret_cast<FileDescriptorAdapter*>(pObj);
}

void* CreateVectorFileDescriptor()
{
   return new vector<FileDescriptor*>;
}

void DestroyVectorFileDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<FileDescriptor*>*>(pObj);
}
//=============================
FileFinder* 
CreateFileFinder()
{
   return new FileFinderImp;
}

void
DestroyFileFinder(void* pObj)
{
   delete (FileFinderImp*) pObj;
}

void* 
CreateVectorFileFinder()
{
   return new vector<FileFinder*>;
}

void
DestroyVectorFileFinder(void* pObj)
{
    vector<FileFinder*>* myObj = (vector<FileFinder*>*) pObj;
   delete myObj;
}
//=============================
Filename* 
CreateFilename()
{
   return new FilenameImp;
}

void
DestroyFilename(void* pObj)
{
   delete (FilenameImp*) pObj;
}

void* 
CreateVectorFilename()
{
   return new vector<Filename*>;
}

void
DestroyVectorFilename(void* pObj)
{
    vector<Filename*>* myObj = (vector<Filename*>*) pObj;
   delete myObj;
}
//=============================
Font* CreateFontObject()
{
   return new FontImp;
}

void DestroyFontObject(void* pObj)
{
   delete reinterpret_cast<FontImp*>(pObj);
}
//=============================
void* 
CreateVectorLayer()
{
   return new vector<Layer*>;
}

void
DestroyVectorLayer(void* pObj)
{
    vector<Layer*>* myObj = (vector<Layer*>*) pObj;
   delete myObj;
}
//=============================
// RasterFileDescriptor
RasterFileDescriptor* CreateRasterFileDescriptor()
{
   return new RasterFileDescriptorAdapter;
}

void DestroyRasterFileDescriptor(void* pObj)
{
   delete reinterpret_cast<RasterFileDescriptorAdapter*>(pObj);
}

void* CreateVectorRasterFileDescriptor()
{
   return new vector<RasterFileDescriptor*>;
}

void DestroyVectorRasterFileDescriptor(void* pObj)
{
   delete reinterpret_cast<vector<RasterFileDescriptor*>*>(pObj);
}
//=============================
SettableSessionItem* CreateSettableSessionItem()
{
   return new SettableSessionItemAdapter(SessionItemImp::generateUniqueId());
}

void DestroySettableSessionItem(void* pObj)
{
   delete reinterpret_cast<SettableSessionItemAdapter*>(pObj);
}
//=============================
ExecutableAgent* CreateExecutableAgent()
{
   return new ExecutableAgentAdapter();
}

void DestroyExecutableAgent(void* pObj)
{
   delete reinterpret_cast<ExecutableAgentAdapter*>(pObj);
}
//=============================
ImportAgent* CreateImportAgent()
{
   return new ImportAgentAdapter();
}

void DestroyImportAgent(void* pObj)
{
   delete reinterpret_cast<ImportAgentAdapter*>(pObj);
}
//=============================
ExportAgent* CreateExportAgent()
{
   return new ExportAgentAdapter();
}

void DestroyExportAgent(void* pObj)
{
   delete reinterpret_cast<ExportAgentAdapter*>(pObj);
}
//=============================
Units*  
CreateUnits()
{
   return new UnitsImp;
}

void
DestroyUnits(void* pObj)
{
   delete (UnitsImp*) pObj;
}

void* 
CreateVectorUnits()
{
   return new vector<Units*>;
}

void
DestroyVectorUnits(void* pObj)
{
    vector<Units*>* myObj = (vector<Units*>*) pObj;
   delete myObj;
}
//=============================
// WizardObject
WizardObject* CreateWizardObject()
{
   return new WizardObjectAdapter;
}

void DestroyWizardObject(void* pObj)
{
   delete reinterpret_cast<WizardObjectAdapter*>(pObj);
}

static void* CreateVectorWizardObject()
{
   return new vector<WizardObject*>;
}

void DestroyVectorWizardObject(void* pObj)
{
   delete reinterpret_cast<vector<WizardObject*>*>(pObj);
}
//=============================

void* CreateUnsignedChar () { return new unsigned char; }
void* CreateChar () { return new char; }
void* CreateUnsignedShort () { return new unsigned short; }
void* CreateShort () { return new short; }
void* CreateUnsignedInt () { return new unsigned int; }
void* CreateInt () { return new int; }
void* CreateUnsignedLong () { return new unsigned long; }
void* CreateLong () { return new long; }
void* CreateFloat () { return new float; }
void* CreateDouble () { return new double; }
void* CreateBool () { return new bool; }
void* CreateString () { return new string; }

void DestroyUnsignedChar (void* ptr) { delete (unsigned char*) ptr;}
void DestroyChar (void* ptr) { delete (char*) ptr;}
void DestroyUnsignedShort (void* ptr) { delete (unsigned short*) ptr;}
void DestroyShort (void* ptr) { delete (short*) ptr;}
void DestroyUnsignedInt (void* ptr) { delete (unsigned int*) ptr;}
void DestroyInt (void* ptr) { delete (int*) ptr;}
void DestroyUnsignedLong (void* ptr) { delete (unsigned long*) ptr;}
void DestroyLong (void* ptr) { delete (long*) ptr;}
void DestroyFloat (void* ptr) { delete (float*) ptr;}
void DestroyDouble (void* ptr) { delete (double*) ptr;}
void DestroyBool (void* ptr) { delete (bool*) ptr;}
void DestroyString (void* ptr) { delete (string*) ptr;}

void* CreateVectorUnsignedChar () {return new vector<unsigned char>;}
void* CreateVectorChar () {return new vector<char>;}
void* CreateVectorUnsignedShort () {return new vector<unsigned short>;}
void* CreateVectorShort () {return new vector<short>;}
void* CreateVectorUnsignedInt () {return new vector<unsigned int>;}
void* CreateVectorInt () {return new vector<int>;}
void* CreateVectorUnsignedLong () {return new vector<unsigned long>;}
void* CreateVectorLong () {return new vector<long>;}
void* CreateVectorFloat () {return new vector<float>;}
void* CreateVectorDouble () {return new vector<double>;}
void* CreateVectorBool () {return new vector<bool>;}
void* CreateVectorString () {return new vector<string>;}
void* CreateVectorVoidPtr () {return new vector<void*>;}
void* CreateVectorUnsignedCharPtr () {return new vector<unsigned char*>;}
void* CreateVectorCharPtr () {return new vector<char*>;}
void* CreateVectorUnsignedShortPtr () {return new vector<unsigned short*>;}
void* CreateVectorShortPtr () {return new vector<short*>;}
void* CreateVectorUnsignedIntPtr () {return new vector<unsigned int*>;}
void* CreateVectorIntPtr () {return new vector<int*>;}
void* CreateVectorUnsignedLongPtr () {return new vector<unsigned long*>;}
void* CreateVectorLongPtr () {return new vector<long*>;}
void* CreateVectorFloatPtr () {return new vector<float*>;}
void* CreateVectorDoublePtr () {return new vector<double*>;}
void* CreateVectorBoolPtr () {return new vector<bool*>;}
void* CreateVectorStringPtr () {return new vector<string*>;}

void DestroyVectorUnsignedChar (void* ptr) {delete (vector<unsigned char>*)ptr;}
void DestroyVectorChar (void* ptr) {delete (vector<char>*) ptr;}
void DestroyVectorUnsignedShort (void* ptr) {delete (vector<unsigned short>*)ptr;}
void DestroyVectorShort (void* ptr) {delete (vector<short>*)ptr;}
void DestroyVectorUnsignedInt (void* ptr) {delete (vector<unsigned int>*)ptr;}
void DestroyVectorInt (void* ptr) {delete (vector<int>*)ptr;}
void DestroyVectorUnsignedLong (void* ptr) {delete (vector<unsigned long>*)ptr;}
void DestroyVectorLong (void* ptr) {delete (vector<long>*)ptr;}
void DestroyVectorFloat (void* ptr) {delete (vector<float>*)ptr;}
void DestroyVectorDouble (void* ptr) {delete (vector<double>*)ptr;}
void DestroyVectorBool (void* ptr) {delete (vector<bool>*)ptr;}
void DestroyVectorString (void* ptr) {delete (vector<string>*)ptr;}
void DestroyVectorVoidPtr (void* ptr) {delete (vector<void*>*)ptr;}
void DestroyVectorUnsignedCharPtr (void* ptr) {delete (vector<unsigned char*>*)ptr;}
void DestroyVectorCharPtr (void* ptr) {delete (vector<char*>*) ptr;}
void DestroyVectorUnsignedShortPtr (void* ptr) {delete (vector<unsigned short*>*)ptr;}
void DestroyVectorShortPtr (void* ptr) {delete (vector<short*>*)ptr;}
void DestroyVectorUnsignedIntPtr (void* ptr) {delete (vector<unsigned int*>*)ptr;}
void DestroyVectorIntPtr (void* ptr) {delete (vector<int*>*)ptr;}
void DestroyVectorUnsignedLongPtr (void* ptr) {delete (vector<unsigned long*>*)ptr;}
void DestroyVectorLongPtr (void* ptr) {delete (vector<long*>*)ptr;}
void DestroyVectorFloatPtr (void* ptr) {delete (vector<float*>*)ptr;}
void DestroyVectorDoublePtr (void* ptr) {delete (vector<double*>*)ptr;}
void DestroyVectorBoolPtr (void* ptr) {delete (vector<bool*>*)ptr;}
void DestroyVectorStringPtr (void* ptr) {delete (vector<string*>*)ptr;}

} // end of anonymous namespace
