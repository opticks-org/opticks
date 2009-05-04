/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SHAPEFILE_H
#define SHAPEFILE_H

#include "Observer.h"
#include "ShapeFileTypes.h"

#include <shapefil.h>

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

class DataElement;
class Feature;
class Progress;
class RasterElement;

class ShapeFile: public Observer
{
public:
   ShapeFile();
   ~ShapeFile();

   void shapeModified(Subject &subject, const std::string &signal, const boost::any &v);
   void shapeAttached(Subject &subject, const std::string &signal, const boost::any &v);
   void attached(Subject &subject, const std::string &signal, const Slot &slot);

   void setFilename(const std::string& filename);
   const std::string& getFilename() const;

   void setShape(ShapeType eShape);
   ShapeType getShape() const;

   std::vector<Feature*> addFeatures(DataElement* pElement, RasterElement* pGeoref, std::string& message);
   bool removeFeature(Feature* pFeature);
   const std::vector<Feature*>& getFeatures() const;
   unsigned int getNumFeatures() const;
   void removeAllFeatures();

   bool addField(const std::string& name, const std::string& type);
   bool removeField(const std::string& name);
   unsigned int getNumFields() const;
   std::vector<std::string> getFieldNames() const;
   std::string getFieldType(const std::string& name) const;
   bool hasField(const std::string& name) const;
   void removeAllFields();

   bool save(Progress* pProgress, std::string& errorMessage);

private:
   void cleanup();
   DBFHandle mpAttributeFile;
   SHPHandle mpShapeFile;
   std::string mFilename;
   ShapeType mShape;
   std::vector<Feature*> mFeatures;
   std::map<std::string, std::string> mFields;
};

#endif
