/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "UnitsImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

UnitsImp::UnitsImp() :
   mUnitType(DIGITAL_NO),
   mUnitName("Digital Number"),
   mRangeMin(0.0),
   mRangeMax(0.0),
   mScaleFromStandard(1.0)
{
}

UnitsImp::~UnitsImp()
{
}

UnitType UnitsImp::getUnitType() const
{
   return mUnitType;
}

void UnitsImp::setUnitType(UnitType myType)
{
   if (myType == mUnitType)
   {
      return;
   }

   string unitName = mUnitName;
   string typeName = StringUtilities::toDisplayString(mUnitType);

   mUnitType = myType;

   if (unitName == typeName)
   {
      typeName = StringUtilities::toDisplayString(mUnitType);
      setUnitName(typeName);
   }
}

const string& UnitsImp::getUnitName() const
{
   return mUnitName;
}

void UnitsImp::setUnitName(const string& unitName)
{
   if (unitName.empty() == true)
   {
      return;
   }

   if (unitName != mUnitName)
   {
      mUnitName = unitName;
   }
}

double UnitsImp::getRangeMin() const
{
   return mRangeMin;
}

void UnitsImp::setRangeMin(double myRangeMin)
{
   mRangeMin = myRangeMin;
}

double UnitsImp::getRangeMax() const
{
   return mRangeMax;
}

void UnitsImp::setRangeMax(double myRangeMax)
{
   mRangeMax = myRangeMax;
}

double UnitsImp::getScaleFromStandard() const
{
   return mScaleFromStandard;
}

void UnitsImp::setScaleFromStandard(double myScaleFromStandard)
{
   mScaleFromStandard = myScaleFromStandard;
}

UnitsImp& UnitsImp::operator =(const UnitsImp& units)
{
   if (this != &units)
   {
      mUnitType = units.mUnitType;
      mUnitName = units.mUnitName;
      mRangeMin = units.mRangeMin;
      mRangeMax = units.mRangeMax;
      mScaleFromStandard = units.mScaleFromStandard;
   }

   return *this;
}

bool UnitsImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("type", mUnitType);
   pXml->addAttr("name", mUnitName);
   vector<double> range;
   range.push_back(mRangeMin);
   range.push_back(mRangeMax);
   pXml->addAttr("range", range);
   pXml->addAttr("scale", mScaleFromStandard);

   return true;
}

bool UnitsImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement *pElmnt(static_cast<DOMElement*>(pDocument));

   mUnitType = StringUtilities::fromXmlString<UnitType>(
      A(pElmnt->getAttribute(X("type"))));

   mUnitName = A(pElmnt->getAttribute(X("name")));

   vector<double> *pRange(static_cast<vector<double>*>(
            XmlReader::StrToVector<double,
                                   XmlReader::StringStreamAssigner<double> >(
                                          pElmnt->getAttribute(X("range")))));
   try
   {
      mRangeMin = pRange->at(0);
      mRangeMax = pRange->at(1);
   }
   catch(...)
   {
      return false;
   }
   XmlReader::StringStreamAssigner<double> parser;
   mScaleFromStandard = parser(A(pElmnt->getAttribute(X("scale"))));

   return true;
}

const string& UnitsImp::getObjectType() const
{
   static string type("UnitsImp");
   return type;
}

bool UnitsImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Units"))
   {
      return true;
   }

   return false;
}
