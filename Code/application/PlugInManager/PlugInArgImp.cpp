/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlugInArgImp.h"

#include <algorithm>
using namespace std;

class Filename;

vector<string> PlugInArgImp::mArgTypes;

PlugInArgImp::PlugInArgImp()
{
   mType = "";
   mName = "";
   mActualSet = false;
   mpActualValueShallowCopy = NULL;
   mDefaultSet = false;
   mpDefaultValueShallowCopy = NULL;
   initArgTypes();
}

PlugInArgImp::PlugInArgImp(const string& type, const string& name)
{
   mType = type.c_str();
   mName = name.c_str();
   mActualSet = false;
   mpActualValueShallowCopy = NULL;
   mDefaultSet = false;
   mpDefaultValueShallowCopy = NULL;
   initArgTypes();
}

PlugInArgImp::PlugInArgImp(const string& type, const string& name, const void *pDefaultValue) 
{
   mType = type.c_str();
   mName = name.c_str();
   mActualSet = false;
   mpActualValueShallowCopy = NULL;
   initArgTypes();

   setDefaultValue(pDefaultValue);
}

PlugInArgImp::PlugInArgImp(const string& type, const string& name, 
   const void *pDefaultValue, const void *pActualValue) 
{
   mType = type.c_str();
   mName = name.c_str();
   initArgTypes();

   setDefaultValue(pDefaultValue);
   setActualValue(pActualValue);
}

PlugInArgImp::~PlugInArgImp()
{
}

const string& PlugInArgImp::getName() const
{
   return mName;
}

const string& PlugInArgImp::getType() const
{
   return mType;
}

void* PlugInArgImp::getDefaultValue() const
{
   return mpDefaultValueShallowCopy;
}

bool PlugInArgImp::isDefaultSet() const
{
   return mDefaultSet;
}

void* PlugInArgImp::getActualValue() const
{
   return mpActualValueShallowCopy;
}

bool PlugInArgImp::isActualSet() const
{
   return mActualSet;
}

static void setValue(const void *pValue, void **pShallowValue, DataVariant& deepValue, std::string type,
                     bool tryDeepCopy)
{
   if (pValue == *pShallowValue)
   {
      return;
   }

   //clear any current values
   *pShallowValue = NULL;
   deepValue = DataVariant();

   // Perform a deep copy on the types that DataVariant can hold, shallow on all others
   if (pValue != NULL)
   {
      if (tryDeepCopy)
      {
         deepValue = DataVariant(type, pValue, false);
         if (deepValue.isValid())
         {
            *pShallowValue = deepValue.getPointerToValueAsVoid();
         }
      }
      
      if (*pShallowValue == NULL)
      {
         *pShallowValue = const_cast<void*>(pValue);
      }
   }
}

void PlugInArgImp::setDefaultValue(const void* defValue, bool tryDeepCopy)
{
   setValue(defValue, &mpDefaultValueShallowCopy, mDefaultValueDeepCopy, mType, tryDeepCopy);

   mDefaultSet = true;
}

void PlugInArgImp::setActualValue(const void* value, bool tryDeepCopy)
{
   setValue(value, &mpActualValueShallowCopy, mActualValueDeepCopy, mType, tryDeepCopy);

   mActualSet = true;
}

void PlugInArgImp::setName(const string& value)
{
   mName = value.c_str();
}

bool PlugInArgImp::setType(const string& value)
{
   // Make sure the type is valid
   if (isArgType(value) == false)
   {
      mArgTypes.push_back(value);
   }

   mType = value.c_str();
   return true;
}

void PlugInArgImp::setDescription(const std::string &description)
{
   mDescription = description;
}

const std::string &PlugInArgImp::getDescription() const
{
   return mDescription;
}

const string& PlugInArgImp::getObjectType() const
{
   static string type("PlugInArgImp");
   return type;
}

bool PlugInArgImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlugInArg"))
   {
      return true;
   }

   return false;
}

void PlugInArgImp::initArgTypes()
{
   if (mArgTypes.empty())
   {
      mArgTypes.push_back("char");
      mArgTypes.push_back("unsigned char");
      mArgTypes.push_back("short");
      mArgTypes.push_back("unsigned short");
      mArgTypes.push_back("int");
      mArgTypes.push_back("unsigned int");
      mArgTypes.push_back("long");
      mArgTypes.push_back("unsigned long");
      mArgTypes.push_back("float");
      mArgTypes.push_back("double");
      mArgTypes.push_back("bool");
      mArgTypes.push_back("string");
      mArgTypes.push_back("vector<char>");
      mArgTypes.push_back("vector<unsigned char>");
      mArgTypes.push_back("vector<short>");
      mArgTypes.push_back("vector<unsigned short>");
      mArgTypes.push_back("vector<int>");
      mArgTypes.push_back("vector<unsigned int>");
      mArgTypes.push_back("vector<long>");
      mArgTypes.push_back("vector<unsigned long>");
      mArgTypes.push_back("vector<float>");
      mArgTypes.push_back("vector<double>");
      mArgTypes.push_back("vector<bool>");
      mArgTypes.push_back("vector<string>");
      mArgTypes.push_back("vector<Filename>");
      mArgTypes.push_back("Animation");
      mArgTypes.push_back("AnimationController");
      mArgTypes.push_back("AnnotationElement");
      mArgTypes.push_back("Any");
      mArgTypes.push_back("AoiElement");
      mArgTypes.push_back("CartesianPlot");
      mArgTypes.push_back("DataDescriptor");
      mArgTypes.push_back("DataElement");
      mArgTypes.push_back("DateTime");
      mArgTypes.push_back("DisplayMode");
      mArgTypes.push_back("EncodingType");
      mArgTypes.push_back("EndianType");
      mArgTypes.push_back("FileDescriptor");
      mArgTypes.push_back("Filename");
      mArgTypes.push_back("GcpList");
      mArgTypes.push_back("GeocoordType");
      mArgTypes.push_back("GraphicElement");
      mArgTypes.push_back("HistogramPlot");
      mArgTypes.push_back("InterleaveFormatType");
      mArgTypes.push_back("Layer");
      mArgTypes.push_back("OrthographicView");
      mArgTypes.push_back("PassArea");
      mArgTypes.push_back("PerspectiveView");
      mArgTypes.push_back("PlotView");
      mArgTypes.push_back("PlotWidget");
      mArgTypes.push_back("PolarPlot");
      mArgTypes.push_back("ProcessingLocation");
      mArgTypes.push_back("ProductView");
      mArgTypes.push_back("Progress");
      mArgTypes.push_back("RasterElement");
      mArgTypes.push_back("RasterDataDescriptor");
      mArgTypes.push_back("RasterFileDescriptor");
      mArgTypes.push_back("Signature");
      mArgTypes.push_back("SignatureSet");
      mArgTypes.push_back("SpatialDataView");
      mArgTypes.push_back("SignaturePlot");
      mArgTypes.push_back("UnitType");
      mArgTypes.push_back("View");
      mArgTypes.push_back("WizardObject");
   }
}

bool PlugInArgImp::isArgType(const string& type)
{
   return std::find(mArgTypes.begin(), mArgTypes.end(), type) != mArgTypes.end();
}

const vector<string>& PlugInArgImp::getArgTypes()
{
   return mArgTypes;
}

