/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef GRAPHIC_OBJECT_FACTORY
#define GRAPHIC_OBJECT_FACTORY

#include "MoveObjectImp.h"
#include "Resource.h"
#include "TypesFile.h"

class GraphicLayer;

// include GraphicObject/Imp here rather than forward declare since the 
// GraphicObjectObject requires that both be fully declared, so do it here 
// rather than putting the obligation of correct include order on everyone 
// who uses it.
#include "GraphicObject.h"

/**
 *  Since the GraphicGroup had to change to no longer be static, the factory
 *  became necessary to produce the annotation objects that the group once created
 */
class GraphicObjectFactory
{
public:
   static GraphicObject* createObject(GraphicObjectType eType,
                                            GraphicLayer* pLayer,
                                            LocationType point = LocationType());
};

template <typename T>
class GraphicObjectObject
{
public:
   class Args
   {
   public:
      GraphicObjectType mType;
      GraphicLayer *mpLayer;
      LocationType mPixel;
      Args(GraphicObjectType type, GraphicLayer *pLayer, LocationType pixel) :
      mType(type), mpLayer(pLayer), mPixel(pixel) {}
   };

   T *obtainResource(const Args &args) const
   {
      return dynamic_cast<T*>(
         GraphicObjectFactory::createObject(args.mType, args.mpLayer, args.mPixel));
   }

   void releaseResource(const Args &args, T *pObject) const
   {
      delete dynamic_cast<GraphicObjectImp*>(pObject);
   }
};

template<typename T = GraphicObject>
class GraphicResource : public Resource<T, GraphicObjectObject<T> >
{
public:
   GraphicResource(GraphicObjectType type, GraphicLayer *pLayer = NULL, LocationType pixel = LocationType()) : 
      Resource<T, GraphicObjectObject<T> >(GraphicObjectObject<T>::Args(type, pLayer, pixel))
   {
   }
};


#endif
