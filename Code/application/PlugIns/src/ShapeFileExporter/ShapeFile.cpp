/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "BitMask.h"
#include "BitMaskIterator.h"
#include "AppVerify.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "Feature.h"
#include "GraphicGroup.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PolygonObject.h"
#include "PolylineObject.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "ShapeFile.h"
#include "UtilityServices.h"

#include <QtGui/QMessageBox>

#include <algorithm>
#include <float.h>
#include <fstream>
#include <shapefil.h>
#include <sstream>

using namespace std;

namespace
{
   const string prjFileContents = "GEOGCS[\"GCS_WGS_1984\",DATUM["
      "\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],"
      "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]";
};

ShapeFile::ShapeFile() :
   mpAttributeFile(NULL),
   mpShapeFile(NULL),
   mShape(ShapefileTypes::MULTIPOINT_SHAPE)
{}

ShapeFile::~ShapeFile()
{
   removeAllFeatures();
   cleanup();
}

void ShapeFile::attached(Subject& subject, const string& signal, const Slot& slot)
{
   shapeAttached(subject, signal, boost::any());
}

void ShapeFile::shapeAttached(Subject& subject, const string& signal, const boost::any& v)
{
   Feature* pFeature = dynamic_cast<Feature*>(&subject);
   if (pFeature != NULL)
   {
      for (map<string, string>::iterator iter = mFields.begin(); iter != mFields.end(); ++iter)
      {
         string name = iter->first;
         string type = iter->second;
         if (!name.empty() && !type.empty())
         {
            pFeature->addField(name, type);
         }
      }
   }
}

void ShapeFile::shapeModified(Subject& subject, const string& signal, const boost::any& v)
{
   Feature* pFeature = dynamic_cast<Feature*>(&subject);
   if (pFeature != NULL)
   {
      vector<string> fieldNames = pFeature->getFieldNames();

      for (vector<string>::iterator iter = fieldNames.begin(); iter != fieldNames.end(); ++iter)
      {
         string name = *iter;
         if (!name.empty())
         {
            if (!hasField(name))
            {
               string type = pFeature->getFieldType(name);
               addField(name, type);
            }
         }
      }
   }
}

void ShapeFile::setFilename(const string& filename)
{
   mFilename = filename;
}

const string& ShapeFile::getFilename() const
{
   return mFilename;
}

void ShapeFile::setShape(ShapefileTypes::ShapeType eShape)
{
   mShape = eShape;
}

ShapefileTypes::ShapeType ShapeFile::getShape() const
{
   return mShape;
}

vector<Feature*> ShapeFile::addFeatures(DataElement* pElement, RasterElement* pGeoref, string& message)
{
   vector<Feature*> features;
   message.clear();

   if (pElement == NULL)
   {
      message = "Features cannot be added because the data element is invalid!";
      return features;
   }

   if (pGeoref == NULL || !pGeoref->isGeoreferenced())
   {
      message = "No georeferencing information can be found.";
      return features;
   }

   // Create the features
   string elementName = pElement->getName();

   AoiElement* pAoi = dynamic_cast<AoiElement*>(pElement);
   if (pAoi != NULL)
   {
      switch (mShape)
      {
      case ShapefileTypes::POINT_SHAPE:
         {
            // The BitMaskIterator does not support negative extents and
            // the BitMask does not correctly handle the outside flag so
            // the BitMaskIterator is used for cases when the outside flag is true and
            // the BitMask is used for cases when the outside flag is false
            const BitMask* pMask = pAoi->getSelectedPoints();
            if (pMask != NULL)
            {
               BitMaskIterator maskIt(pMask, pGeoref);
               if ((maskIt.getCount() > 0 && pMask->isOutsideSelected() == true) ||
                  (pMask->getCount() > 0 && pMask->isOutsideSelected() == false))
               {
                  // Add features for each selected pixel
                  int startColumn = 0;
                  int endColumn = 0;
                  int startRow = 0;
                  int endRow = 0;
                  if (pMask->isOutsideSelected() == true)
                  {
                     maskIt.getBoundingBox(startColumn, startRow, endColumn, endRow);
                  }
                  else
                  {
                     pMask->getBoundingBox(startColumn, startRow, endColumn, endRow);
                  }
                  LocationType pixel;
                  for (int i = startColumn; i <= endColumn; i++)
                  {
                     for (int j = startRow; j <= endRow; j++)
                     {
                        if ((maskIt.getPixel(i, j) && pMask->isOutsideSelected() == true) ||
                              (pMask->getPixel(i, j) && pMask->isOutsideSelected() == false))
                        {
                           // Add the feature
                           Feature* pFeature = new Feature(mShape);
                           if (pFeature != NULL)
                           {
                              features.push_back(pFeature);
                              mFeatures.push_back(pFeature);
                              pFeature->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &ShapeFile::shapeModified));

                              pixel.mX = i + 0.5;
                              pixel.mY = j + 0.5;

                              // Fields
                              pFeature->addField("Name", string());
                              pFeature->addField("Pixel", string());

                              if (!elementName.empty())
                              {
                                 pFeature->setFieldValue("Name", elementName);
                              }

                              string pixelName = "";
                              if (!pixelName.empty())
                              {
                                 pFeature->setFieldValue("Pixel", pixelName);
                              }

                              // Vertex
                              pixel = pGeoref->convertPixelToGeocoord(pixel);
                              pFeature->addVertex(pixel.mY, pixel.mX);    // Longitude as x-coord
                           }
                        }
                     }
                  }
               }
            }
         }
         break;

      case ShapefileTypes::POLYLINE_SHAPE:
         {
            GraphicGroup* pGroup = pAoi->getGroup();
            if (pGroup != NULL)
            {
               const list<GraphicObject*>& objects = pGroup->getObjects();
               if (objects.empty())
               {
                  message = "Error Shape File 101: Cannot create a shape file from an empty AOI.";
                  return features;
               }
               for (std::list<GraphicObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
               {
                  const PolylineObject* pObj = dynamic_cast<const PolylineObject*>(*it);
                  if (pObj == NULL)
                  {
                     continue;
                  }
                  const std::vector<LocationType>& vertices = pObj->getVertices();
                  Feature* pFeature = new Feature(mShape);
                  if (pFeature != NULL)
                  {
                     features.push_back(pFeature);
                     mFeatures.push_back(pFeature);
                     pFeature->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &ShapeFile::shapeModified));

                     pFeature->addField("Name", string());
                     if (!elementName.empty())
                     {
                        pFeature->setFieldValue("Name", elementName + ": " + pObj->getName());
                     }

                     for (vector<LocationType>::const_iterator iter = vertices.begin(); iter != vertices.end(); ++iter)
                     {
                        LocationType geo = pGeoref->convertPixelToGeocoord(*iter);
                        pFeature->addVertex(geo.mY, geo.mX);
                     }
                  }
               }
            }
         }
         break;

      case ShapefileTypes::POLYGON_SHAPE:
         {
            GraphicGroup* pGroup = pAoi->getGroup();
            if (pGroup != NULL)
            {
               const list<GraphicObject*>& objects = pGroup->getObjects();
               if (objects.empty())
               {
                  message = "Error Shape File 101: Cannot create a shape file from an empty AOI.";
                  return features;
               }

               for (std::list<GraphicObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
               {
                  const PolygonObject* pObj = dynamic_cast<const PolygonObject*>(*it);
                  if (pObj == NULL)
                  {
                     continue;
                  }
                  if (pObj->getDrawMode() == ERASE)
                  {
                     // currently the exporter can't handle holes, so skip.
                     message = elementName + " contains polygons with holes. Currently this exporter only "
                        "exports the outer polygons.";
                     continue;
                  }
                  const std::vector<LocationType>& vertices = pObj->getVertices();
                  Feature* pFeature = new Feature(mShape);
                  if (pFeature != NULL)
                  {
                     features.push_back(pFeature);
                     mFeatures.push_back(pFeature);
                     pFeature->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &ShapeFile::shapeModified));

                     pFeature->addField("Name", string());
                     if (!elementName.empty())
                     {
                        pFeature->setFieldValue("Name", elementName + ": " + pObj->getName());
                     }

                     for (vector<LocationType>::const_iterator iter = vertices.begin(); iter != vertices.end(); ++iter)
                     {
                        LocationType geo = pGeoref->convertPixelToGeocoord(*iter);
                        pFeature->addVertex(geo.mY, geo.mX);
                     }
                  }
               }
            }
         }

         break;

      case ShapefileTypes::MULTIPOINT_SHAPE:
         {
            // The BitMaskIterator does not support negative extents and
            // the BitMask does not correctly handle the outside flag so
            // the BitMaskIterator is used for cases when the outside flag is true and
            // the BitMask is used for cases when the outside flag is false
            const BitMask* pMask = pAoi->getSelectedPoints();
            if (pMask != NULL)
            {
               BitMaskIterator maskIt(pMask, pGeoref);
               if ((maskIt.getCount() > 0 && pMask->isOutsideSelected() == true) ||
                  (pMask->getCount() > 0 && pMask->isOutsideSelected() == false))
               {
                  // Add the feature
                  Feature* pFeature = new Feature(mShape);
                  if (pFeature != NULL)
                  {
                     features.push_back(pFeature);
                     mFeatures.push_back(pFeature);
                     pFeature->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &ShapeFile::shapeModified));

                     // Fields
                     pFeature->addField("Name", string());
                     if (!elementName.empty())
                     {
                        pFeature->setFieldValue("Name", elementName);
                     }

                     // Vertices
                     int startColumn = 0;
                     int endColumn = 0;
                     int startRow = 0;
                     int endRow = 0;
                     if (pMask->isOutsideSelected() == true)
                     {
                        maskIt.getBoundingBox(startColumn, startRow, endColumn, endRow);
                     }
                     else
                     {
                        pMask->getBoundingBox(startColumn, startRow, endColumn, endRow);
                     }
                     LocationType pixel;
                     for (int i = startColumn; i <= endColumn; i++)
                     {
                        for (int j = startRow; j <= endRow; j++)
                        {
                           if ((maskIt.getPixel(i, j) && pMask->isOutsideSelected() == true) ||
                              (pMask->getPixel(i, j) && pMask->isOutsideSelected() == false))
                           {
                              pixel.mX = i + 0.5;
                              pixel.mY = j + 0.5;
                              pixel = pGeoref->convertPixelToGeocoord(pixel);

                              pFeature->addVertex(pixel.mY, pixel.mX);    // Longitude as x-coord
                           }
                        }
                     }
                  }
               }
            }
         }
         break;

      default:
         message = "Shape type not recognized.";
         return features;
      }
   }

   if (features.empty())
   {
      message = elementName + " has no selected points, so no features can be added.";
   }

   return features;
}

bool ShapeFile::removeFeature(Feature* pFeature)
{
   if (pFeature == NULL)
   {
      return false;
   }

   for (vector<Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
   {
      Feature* pCurrentFeature = *iter;
      if (pCurrentFeature == pFeature)
      {
         mFeatures.erase(iter);
         pFeature->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &ShapeFile::shapeModified));
         delete pFeature;
         return true;
      }
   }

   return false;
}

const vector<Feature*>& ShapeFile::getFeatures() const
{
   return mFeatures;
}

unsigned int ShapeFile::getNumFeatures() const
{
   return mFeatures.size();
}

void ShapeFile::removeAllFeatures()
{
   for (vector<Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
   {
      Feature* pFeature = *iter;
      if (pFeature != NULL)
      {
         pFeature->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &ShapeFile::shapeModified));
         delete pFeature;
      }
   }

   mFeatures.clear();
}

bool ShapeFile::addField(const string& name, const string& type)
{
   if (hasField(name))
   {
      return false;
   }

   mFields[name] = type;

   for (vector<Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
   {
      Feature* pFeature = *iter;
      if (pFeature != NULL)
      {
         pFeature->addField(name, type);
      }
   }

   return true;
}

bool ShapeFile::removeField(const string& name)
{
   if (!hasField(name))
   {
      return false;
   }

   mFields.erase(name);

   for (vector<Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
   {
      Feature* pFeature = *iter;
      if (pFeature != NULL)
      {
         pFeature->removeField(name);
      }
   }

   return true;
}

unsigned int ShapeFile::getNumFields() const
{
   return mFields.size();
}

vector<string> ShapeFile::getFieldNames() const
{
   vector<string> fieldNames;

   for (map<string, string>::const_iterator iter = mFields.begin(); iter != mFields.end(); ++iter)
   {
      string name = iter->first;
      if (!name.empty())
      {
         fieldNames.push_back(name);
      }
   }

   return fieldNames;
}

string ShapeFile::getFieldType(const string& name) const
{
   string type = "";

   map<string, string>::const_iterator iter = mFields.find(name);
   if (iter != mFields.end())
   {
      type = iter->second;
   }

   return type;
}

bool ShapeFile::hasField(const string& name) const
{
   map<string, string>::const_iterator iter = mFields.find(name);
   if (iter != mFields.end())
   {
      return true;
   }

   return false;
}

void ShapeFile::removeAllFields()
{
   mFields.clear();

   for (vector<Feature*>::iterator iter = mFeatures.begin(); iter != mFeatures.end(); ++iter)
   {
      Feature* pFeature = *iter;
      if (pFeature != NULL)
      {
         pFeature->clearFields();
      }
   }
}

bool ShapeFile::save(Progress* pProgress, string& errorMessage)
{
   errorMessage.clear();
   int iFeatures = mFeatures.size();

   if (iFeatures == 0)
   {
      errorMessage = "There are no features to save!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(errorMessage, 0, ERRORS);
      }

      return false;
   }

   // Open the files
   SHPHandle pShapeFile = NULL;
   DBFHandle pAttributeFile = NULL;

   int iType = -1;
   switch (mShape)
   {
   case ShapefileTypes::POINT_SHAPE:
      iType = SHPT_POINT;
      break;

   case ShapefileTypes::POLYLINE_SHAPE:
      iType = SHPT_ARC;
      break;

   case ShapefileTypes::POLYGON_SHAPE:
      iType = SHPT_POLYGON;
      break;

   case ShapefileTypes::MULTIPOINT_SHAPE:
      iType = SHPT_MULTIPOINT;
      break;

   default:
      errorMessage = "Can not save invalid shape type";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(errorMessage, 0, ERRORS);
      }
      return false;
   }

   pShapeFile = SHPCreate(mFilename.c_str(), iType);
   if (pShapeFile == NULL)
   {
      errorMessage = "Cannot create the SHP and SHX files!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(errorMessage, 0, ERRORS);
      }

      return false;
   }

   pAttributeFile = DBFCreate(mFilename.c_str());
   if (pAttributeFile == NULL)
   {
      SHPClose(pShapeFile);

      errorMessage = "Cannot create the DBF file!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(errorMessage, 0, ERRORS);
      }

      return false;
   }

   // Add the fields to the attribute file
   map<string, int> fieldIds;
   int iFields = mFields.size();
   int i = 0;

   for (map<string, string>::iterator iter = mFields.begin(); iter != mFields.end(); ++iter, ++i)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Writing fields...", i * 33 / iFields, NORMAL);
      }

      string name = iter->first;
      string type = iter->second;

      if (!name.empty())
      {
         DBFFieldType eFieldType = FTInvalid;
         int iWidth = 0;
         int iDecimalPlaces = 0;

         if ((type == "char") || (type == "short") || (type == "int"))
         {
            eFieldType = FTInteger;
            iWidth = 18;
         }
         else if ((type == "float") || (type == "double"))
         {
            eFieldType = FTDouble;
            iWidth = 19;
            iDecimalPlaces = 8;
         }
         else if (type == "string")
         {
            eFieldType = FTString;
            iWidth = 254;
         }

         int iFieldId = DBFAddField(pAttributeFile, name.c_str(), eFieldType, iWidth, iDecimalPlaces);
         if (iFieldId != -1)
         {
            fieldIds[name] = iFieldId;
         }
      }
   }

   // Save the features and attributes
   for (i = 0; i < iFeatures; i++)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Writing features...", 33 + (i * 67 / iFeatures), NORMAL);
      }

      Feature* pFeature = mFeatures[i];
      if (pFeature != NULL)
      {
         // Features
         const vector<Feature::FeatureVertex>& vertices = pFeature->getVertices();
         int iVertices = vertices.size();

         vector<double> dX(iVertices);
         vector<double> dY(iVertices);
         vector<double> dZ(iVertices);

         for (int j = 0; j < iVertices; j++)
         {
            Feature::FeatureVertex vertex = vertices[j];
            dX[j] = vertex.mX;
            dY[j] = vertex.mY;
            dZ[j] = vertex.mZ;
         }

         if (mShape == ShapefileTypes::POLYGON_SHAPE)
         {
            // make sure there are no collinear segments by calculating
            // the area of the triangle defined by each point triplet
            // if the area is 0, the points are collinear so we remove the
            // middle point and continue
            for (int a = 0; a < (iVertices - 2); a++)
            {
               int b = a + 1;
               int c = a + 2;
               double area = dX[a] * (dY[b] - dY[c]) + dX[b] * (dY[c] - dY[a]) + dX[c] * (dY[a] - dY[b]);
               if (fabs(area) < 1e-15) // equals zero
               {
                  dX.erase(dX.begin() + b);
                  dY.erase(dY.begin() + b);
                  dZ.erase(dZ.begin() + b);
                  a--;
                  iVertices--;
               }
            }
         }

         SHPObject* pObject = SHPCreateObject(iType, i, 0, NULL, NULL, iVertices, &dX.front(), &dY.front(),
            &dZ.front(), NULL);
         if (pObject != NULL)
         {
            SHPRewindObject(pShapeFile, pObject);
            SHPWriteObject(pShapeFile, -1, pObject);
            SHPDestroyObject(pObject);
         }

         // Attributes
         for (map<string, int>::iterator iter = fieldIds.begin(); iter != fieldIds.end(); ++iter)
         {
            string name = iter->first;
            int iFieldId = iter->second;

            if (!name.empty())
            {
               const DataVariant& var = pFeature->getFieldValue(name);
               if (var.isValid())
               {
                  string fieldType = pFeature->getFieldType(name);
                  if (fieldType == var.getTypeName())
                  {
                     if (fieldType == "int")
                     {
                        DBFWriteIntegerAttribute(pAttributeFile, i, iFieldId, dv_cast<int>(var));
                     }
                     else if (fieldType == "double")
                     {
                        DBFWriteDoubleAttribute(pAttributeFile, i, iFieldId, dv_cast<double>(var));
                     }
                     else if (fieldType == "string")
                     {
                        DBFWriteStringAttribute(pAttributeFile, i, iFieldId, dv_cast<string>(var).c_str());
                     }
                  }
               }
               else
               {
                  DBFWriteNULLAttribute(pAttributeFile, i, iFieldId);
               }
            }
         }
      }
   }

   SHPClose(pShapeFile);
   DBFClose(pAttributeFile);

   string prjFilename = mFilename + ".prj";
   ofstream prjFile(prjFilename.c_str());
   prjFile << prjFileContents;
   prjFile.close();
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Shape files saved!", 100, NORMAL);
   }

   return true;
}

void ShapeFile::cleanup()
{
   if (mpAttributeFile != NULL)
   {
      DBFClose(mpAttributeFile);
      mpAttributeFile = NULL;
   }
   if (mpShapeFile != NULL)
   {
      SHPClose(mpShapeFile);
      mpShapeFile = NULL;
   }
}
