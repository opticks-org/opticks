/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BITMASKOBJECTIMP_H
#define BITMASKOBJECTIMP_H

#include "GraphicObjectImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"

class BitMaskObjectImp : public GraphicObjectImp
{
public:
   BitMaskObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~BitMaskObjectImp();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void draw(double zoomFactor) const;
   bool hit(LocationType pixelCoord) const;

   void setBitMask(const BitMask *pMask, bool copy = true);

   using GraphicObjectImp::getPixels;
   const BitMask *getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

private:
   const BitMask *getMask() const;

   FactoryResource<BitMask> mpMask;
   const BitMask *mpUnownedMask;
};

#define BITMASKOBJECTADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass) \
   void setBitMask(const BitMask *pMask, bool copy) \
   { \
      impClass::setBitMask(pMask, copy); \
   }


#endif
