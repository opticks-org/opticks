//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossimImageViewAffineTransform.h 23477 2015-08-26 14:39:12Z gpotts $
#ifndef ossimImageViewAffineTransform_HEADER
#define ossimImageViewAffineTransform_HEADER
#include <ossim/projection/ossimImageViewTransform.h>
#include <ossim/matrix/newmat.h>

class OSSIMDLLEXPORT ossimImageViewAffineTransform: public ossimImageViewTransform
{
public:
   /**
    * Initialize transform completely here. The image-to-view transform is a chain of 3x3 matrix
    * multiplies as: [translation] [view scale] [rotation (about pivot pt)] [image scale].
    * The view to image is stored as the inverse transform.
    * @param rotateDegrees
    * @param imageScaleX  Image-side scale, applied before rotation
    * @param imageScaleY  ditto
    * @param scaleXValue  View-side scale
    * @param scaleYValue  ditto
    * @param translateXValue View-side translation
    * @param translateYValue  ditto
    * @param pivotXValue Pivot point in image space
    * @param pivotYValue ditto
    */
   ossimImageViewAffineTransform(double rotateDegrees = 0.0,
                                 double imageScaleX = 1.0,
                                 double imageScaleY = 1.0,
                                 double scaleXValue = 1.0,
                                 double scaleYValue = 1.0,
                                 double translateXValue = 0.0,
                                 double translateYValue = 0.0,
                                 double pivotXValue = 0.0,
                                 double pivotYValue = 0.0 );
   virtual ~ossimImageViewAffineTransform();
   
   ossimImageViewAffineTransform(const ossimImageViewAffineTransform& src)
   :ossimImageViewTransform(src),
   m_transform(src.m_transform),
   m_inverseTransform(src.m_inverseTransform),
   m_rotation(src.m_rotation),
   m_imageSpaceScale(src.m_imageSpaceScale),
   m_scale(src.m_scale),
   m_translate(src.m_translate),
   m_pivot(src.m_pivot)
   {
   }
   virtual ossimObject* dup()const
   {
      return new ossimImageViewAffineTransform(*this);
   }
//   virtual void inverse(const ossimDpt& input,
//                        ossimDpt&       output) const
//   {
//      viewToImage(input, output);
 //  }
   
   virtual void imageToView(const ossimDpt& imagePoint,
                            ossimDpt&       viewPoint)const;
   virtual void viewToImage(const ossimDpt& viewPoint,
                            ossimDpt&       imagePoint)const;
   void setMatrix(NEWMAT::Matrix& matrix);
   const NEWMAT::Matrix& getMatrix()const;
   
   virtual bool isIdentity()const
   {
      return ((m_transform[0][0] == 1.0)&&
              (m_transform[0][1] == 0.0)&&
              (m_transform[0][2] == 0.0)&&
              (m_transform[1][0] == 0.0)&&
              (m_transform[1][1] == 1.0)&&
              (m_transform[1][2] == 0.0)&&
              (m_transform[2][0] == 0.0)&&
              (m_transform[2][1] == 0.0)&&
              (m_transform[2][2] == 1.0));
   }

   virtual bool isValid()const;
   virtual bool setView(ossimObject* obj);
   virtual ossimObject* getView();
   virtual const ossimObject* getView()const;
   
   /** @return (1, 1) ???????(drb) */
   virtual ossimDpt getInputMetersPerPixel()const;

   /** @return (nan, nan) ????????? (drb) */
   virtual ossimDpt getOutputMetersPerPixel()const;
   
   /*!
    * Translate in the x and y direction.
    */ 
   virtual void translate(double deltaX, double deltaY);
   
   /*!
    * Translate in the x direction.
    */
   virtual void translateX(double deltaX);
   
   /*!
    * Translate in the Y direction.
    */
   virtual void translateY(double deltaY);
   
   /*!
    * Translate the origin for rotation in the x and y direction.
    */
   virtual void pivot(double originX, double originY);
   
   /*!
    * Translate the origin for rotation in the x direction.
    */
   virtual void pivotX(double originX);
   
   /*!
    * Translate the origin for rotation in the y direction.
    */
   virtual void pivotY(double originY);

   /*!
    * Will allow you to specify an image scale
    * for both the x and y direction.
    */
   virtual void imageSpaceScale(double x, double y);
   
   /*!
    * Will allow you to specify an image scale along the X direction.
    */
   virtual void imageSpaceScaleX(double x);

   /*!
    * Will allow you to an image scale along the Y direction.
    */
   virtual void imageSpaceScaleY(double y);
    
   /*!
    * will allow you to specify a scale
    * for both the x and y direction.
    */
   virtual void scale(double x, double y);
    
   /*!
    * will alow you to specify a scale
    * along the X direction.
    */
   virtual void scaleX(double x);
   
   /*!
    * Will allow you to scale along the Y
    * direction.
    */
   virtual void scaleY(double y);
   
   /*!
    * Will apply a rotation
    */
   virtual void rotate(double degrees);
   
   ossim_float64 getRotation()const{return m_rotation;}
   const ossimDpt& getScale()const{return m_scale;}
   const ossimDpt& getTranslate()const{return m_translate;}
   const ossimDpt& getPivot()const{return m_pivot;}
   
   
   virtual bool isEqualTo(const ossimObject& obj, ossimCompareType compareType = OSSIM_COMPARE_FULL)const;
   
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix =0);
   
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix =0)const;
   
   
protected:
   void buildCompositeTransform();
   
   /*!
    * This is the transformation from image to
    * viewing coordinates.  If this matrix is
    * changed it will perform an inverse to solve
    * the inverse transform.
    */
   NEWMAT::Matrix m_transform;
   NEWMAT::Matrix m_inverseTransform;
   ossim_float64  m_rotation;
   ossimDpt       m_imageSpaceScale;
   ossimDpt       m_scale;
   ossimDpt       m_translate;
   ossimDpt       m_pivot;

TYPE_DATA
};

#endif
