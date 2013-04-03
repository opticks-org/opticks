/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURE_H
#define FEATURE_H

#include "ObjectResource.h"
#include "SubjectAdapter.h"

#include <string>
#include <vector>

class DataVariant;
class DynamicObject;
class SessionItem;

class Feature : public SubjectAdapter
{
public:
   /**
    *  Emitted with no data when a Feature is cleared.
    */
   SIGNAL_METHOD(Feature, Cleared)
   /**
    *  Emitted with any<FeatureVertex> when a vertex is added.
    */
   SIGNAL_METHOD(Feature, VertexAdded)
   /**
    *  Emitted with any<FeatureVertex> when a vertex is removed.
    */
   SIGNAL_METHOD(Feature, VertexRemoved)

   struct FeatureVertex
   {
      FeatureVertex() : mX(0.0), mY(0.0), mZ(0.0) {};
      FeatureVertex(double dX, double dY, double dZ) : mX(dX), mY(dY), mZ(dZ) {};

      double mX;
      double mY;
      double mZ;
   };

   Feature(SessionItem* pSessionItem);
   ~Feature();

   SessionItem* getSessionItem() const;

   /**
    * @notify  signalVertexAdded with any<FeatureVertex>
    */
   int addVertex(double dX, double dY, double dZ = 0.0);
   /**
    * @notify  signalVertexRemoved with any<FeatureVertex>
    */
   bool removeVertex(int iIndex);
   FeatureVertex getVertex(int iIndex) const;
   unsigned int getNumVertices() const;
   const std::vector<FeatureVertex>& getVertices() const;
   /**
    * @notify  signalVertexRemoved
    */
   void clearVertices();
   int getPart(int iIndex) const;
   unsigned int getNumParts() const;

   /**
    * @notify  Subject::signalModified
    */
   bool addField(const std::string& name, const DataVariant& defaultValue);
   /**
    * @notify  Subject::signalModified
    */
   bool removeField(const std::string& name);
   unsigned int getNumFields() const;
   std::vector<std::string> getFieldNames() const;
   std::string getFieldType(const std::string& name) const;
   /**
    * @notify  Subject::signalModified
    */
   bool setFieldValue(const std::string& name, const DataVariant& var);
   const DataVariant& getFieldValue(const std::string& name) const;
   bool hasField(const std::string& name) const;
   /**
    * @notify  Subject::signalModified
    */
   void clearFields();

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

private:
   SessionItem* mpSessionItem;
   std::vector<FeatureVertex> mVertices;
   FactoryResource<DynamicObject> mpFields;
};

#endif
