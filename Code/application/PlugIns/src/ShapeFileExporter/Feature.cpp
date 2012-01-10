/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "DynamicObject.h"
#include "Feature.h"

using namespace std;

Feature::Feature(ShapefileTypes::ShapeType eShape)
{}

ShapefileTypes::ShapeType Feature::getShape() const
{
   return mShape;
}

Feature::~Feature()
{}

int Feature::addVertex(double dX, double dY, double dZ)
{
   FeatureVertex vertex(dX, dY, dZ);
   mVertices.push_back(vertex);
   notify(SIGNAL_NAME(Feature, VertexAdded), boost::any(vertex));

   int iIndex = static_cast<int>(mVertices.size()) - 1;
   return iIndex;
}

bool Feature::removeVertex(int iIndex)
{
   int i = 0;
   for (vector<FeatureVertex>::iterator iter = mVertices.begin(); iter != mVertices.end(); ++iter, ++i)
   {
      if (i == iIndex)
      {
         FeatureVertex oldVertex = *iter;
         mVertices.erase(iter);
         notify(SIGNAL_NAME(Feature, VertexRemoved), boost::any(oldVertex));
         return true;
      }
   }

   return false;
}

Feature::FeatureVertex Feature::getVertex(int iIndex) const
{
   FeatureVertex vertex;

   unsigned int uiVertices = getNumVertices();
   if ((iIndex >= 0) || (iIndex < static_cast<int>(uiVertices)))
   {
      vertex = mVertices[iIndex];
   }

   return vertex;
}

unsigned int Feature::getNumVertices() const
{
   return mVertices.size();
}

const vector<Feature::FeatureVertex>& Feature::getVertices() const
{
   return mVertices;
}

void Feature::clearVertices()
{
   bool emit = !mVertices.empty();
   mVertices.clear();
   if (emit)
   {
      notify(SIGNAL_NAME(Feature, Cleared));
   }
}

bool Feature::addField(const string& name, const DataVariant& defaultValue)
{
   if (hasField(name))
   {
      return false;
   }

   if (!mpFields->setAttribute(name, defaultValue))
   {
      return false;
   }
   notify(SIGNAL_NAME(Subject, Modified));

   return true;
}

bool Feature::removeField(const string& name)
{
   if (!hasField(name))
   {
      return false;
   }

   if (!mpFields->removeAttribute(name))
   {
      return false;
   }
   notify(SIGNAL_NAME(Subject, Modified));

   return true;
}

unsigned int Feature::getNumFields() const
{
   return mpFields->getNumAttributes();
}

vector<string> Feature::getFieldNames() const
{
   vector<string> fieldNames;
   mpFields->getAttributeNames(fieldNames);

   return fieldNames;
}

string Feature::getFieldType(const string& name) const
{
   return mpFields->getAttribute(name).getTypeName();
}

bool Feature::setFieldValue(const string& name, const DataVariant &var)
{
   if (mpFields->getAttribute(name).isValid())
   {
      if (!mpFields->setAttribute(name, var))
      {
         notify(SIGNAL_NAME(Subject, Modified));
         return true;
      }
   }

   return false;
}

const DataVariant &Feature::getFieldValue(const string& name) const
{
   return mpFields->getAttribute(name);
}

bool Feature::hasField(const string& name) const
{
   return mpFields->getAttribute(name).isValid();
}

void Feature::clearFields()
{
   mpFields->clear();
   notify(SIGNAL_NAME(Subject, Modified));
}

const string& Feature::getObjectType() const
{
   static string type("Feature");
   return type;
}

bool Feature::isKindOf(const string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return SubjectAdapter::isKindOf(className);
}

int Feature::getPart(int iIndex) const
{
   return 0;
}

unsigned int Feature::getNumParts() const
{
   return 1;
}
