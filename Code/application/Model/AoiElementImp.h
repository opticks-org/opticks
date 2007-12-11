/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOIELEMENTIMP_H
#define AOIELEMENTIMP_H

#include "GraphicElementImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"

class BitMask;
class Progress;

class AoiElementImp : public GraphicElementImp
{
public:
   AoiElementImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~AoiElementImp();

   virtual void clearPoints();

   virtual void toggleAllPoints();

   virtual size_t getPixelCount() const;
   virtual const BitMask *getSelectedPoints() const;

   bool getAllPointsToggled() const;

   virtual GraphicObject *addPoints(const std::vector<LocationType> &points);
   virtual GraphicObject *addPoints(const BitMask *pPoints);
   virtual GraphicObject *addPoint(LocationType point);
   virtual GraphicObject *removePoints(const std::vector<LocationType> &points);
   virtual GraphicObject *removePoints(const BitMask *pPoints);
   virtual GraphicObject *removePoint(LocationType point);
   virtual GraphicObject *togglePoints(const std::vector<LocationType> &points);
   virtual GraphicObject *togglePoints(const BitMask *pPoints);
   virtual GraphicObject *togglePoint(LocationType point);

   ModeType correctedDrawMode(ModeType mode);

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   void groupModified(Subject &subject, const std::string &signal, const boost::any &data);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getElementTypes(std::vector<std::string>& classList);
   static bool isKindOfElement(const std::string& className);

   bool setGeocentric(bool geocentric);

private:
   mutable FactoryResource<BitMask> mpBitMask;
   mutable bool mBitMaskDirty;
   bool mToggledAllPoints;
};

#define AOIELEMENTADAPTER_METHODS(impClass) \
   GRAPHICELEMENTADAPTER_METHODS(impClass) \
   void clearPoints() \
   { \
      return impClass::clearPoints(); \
   } \
   void toggleAllPoints() \
   { \
      return impClass::toggleAllPoints(); \
   } \
   size_t getPixelCount() const \
   { \
      return impClass::getPixelCount(); \
   } \
   const BitMask *getSelectedPoints() const \
   { \
      return impClass::getSelectedPoints(); \
   } \
   bool getAllPointsToggled() const \
   { \
      return impClass::getAllPointsToggled(); \
   } \
   GraphicObject *addPoints(const std::vector<LocationType> &points) \
   { \
      return impClass::addPoints(points); \
   } \
   GraphicObject *addPoints(const BitMask *pPoints) \
   { \
      return impClass::addPoints(pPoints); \
   } \
   GraphicObject *addPoint(LocationType point) \
   { \
      return impClass::addPoint(point); \
   } \
   GraphicObject *removePoints(const std::vector<LocationType> &points) \
   { \
      return impClass::removePoints(points); \
   } \
   GraphicObject *removePoints(const BitMask *pPoints) \
   { \
      return impClass::removePoints(pPoints); \
   } \
   GraphicObject *removePoint(LocationType point) \
   { \
      return impClass::removePoint(point); \
   } \
   GraphicObject *togglePoints(const std::vector<LocationType> &points) \
   { \
      return impClass::togglePoints(points); \
   } \
   GraphicObject *togglePoints(const BitMask *pPoints) \
   { \
      return impClass::togglePoints(pPoints); \
   } \
   GraphicObject *togglePoint(LocationType point) \
   { \
      return impClass::togglePoint(point); \
   }

#endif
