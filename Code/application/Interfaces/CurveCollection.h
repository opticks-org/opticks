/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef CURVECOLLECTION_H
#define CURVECOLLECTION_H

#include "ColorType.h"
#include "PlotObject.h"
#include "TypesFile.h"

#include <vector>

class Curve;

/**
 *  Curve plot objects grouped into a single plot object.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: addCurve(), deleteCurve(), clear(),
 *    setLineWidth(), setLineStyle().
 *  - Everything else documented in PlotObject.
 *
 *  @see     Curve
 *  @see     PlotObject
 */
class CurveCollection : public PlotObject
{
public:
   /**
    *  Emitted with any<Curve*> when a Curve is added.
    */
   SIGNAL_METHOD(CurveCollection, CurveAdded)
   /**
    *  Emitted with any<Curve*> when a Curve is removed.
    */
   SIGNAL_METHOD(CurveCollection, CurveDeleted)

   /**
    *  Adds a curve to the collection.
    *
    *  @return  A pointer to the newly created Curve.
    *
    *  @notify  signalCurveAdded with any<Curve*>
    */
   virtual Curve* addCurve() = 0;

   /**
    *  Retrieves all curves in the collection.
    *
    *  @return  A reference to a vector containing pointers to the curves in
    *           the collection.
    */
   virtual const std::vector<Curve*>& getCurves() const = 0;

   /**
    *  Returns the number of curves in the collection.
    *
    *  @return  A number of curves currently in the collection.
    */
   virtual unsigned int getNumCurves() const = 0;

   /**
    *  Removes a curve from the collection and deletes it.
    *
    *  @param   pCurve
    *           The curve to delete.  Cannot be NULL.
    *
    *  @return  TRUE if the curve was successfully removed from the collection
    *           and deleted.  FALSE is returned if the curve is not found in
    *           the collection.
    *
    *  @notify  signalCurveDeleted with any<Curve*>
    */
   virtual bool deleteCurve(Curve* pCurve) = 0;

   /**
    *  Removes all curves from the collection and deletes them.
    *
    *  @notify  signalCurveDeleted with any<Curve*> for each curve
    */
   virtual void clear() = 0;

   /**
    *  Sets the color of all curves in the collection.
    *
    *  This method applies a single color to all curves in the collection.
    *  The color of a single curve can also be changed directly on the
    *  Curve.
    *
    *  @param   collectionColor
    *           The new curve color.
    *
    *  @see     Curve::setColor()
    */
   virtual void setColor(const ColorType& collectionColor) = 0;

   /**
    *  Returns the color of the curves in the collection.
    *
    *  @return  The color of all curves in the collection.  If the curves
    *           have different colors, an invalid color is returned.
    *
    *  @see     ColorType::isValid()
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the line width of all curves in the collection.
    *
    *  This method applies a single line width to all curves in the collection.
    *  The line width of a single curve can also be changed directly on the
    *  Curve.
    *
    *  @param   iWidth
    *           The new curve line width.
    *
    *  @notify  Subject::signalModified
    * 
    *  @see     Curve::setLineWidth()
    */
   virtual void setLineWidth(int iWidth) = 0;

   /**
    *  Returns the line width of the curves in the collection.
    *
    *  @return  The line width of all curves in the collection.  If the curves
    *           have different line widths, a value of -1 is returned.
    */
   virtual int getLineWidth() const = 0;

   /**
    *  Sets the line style of all curves in the collection.
    *
    *  This method applies a single line style to all curves in the collection.
    *  The line style of a single curve can also be changed directly on the
    *  Curve.
    *
    *  @param   lineStyle
    *           The new curve line style.
    *
    *  @notify  Subject::signalModified
    *
    *  @see     LineStyle
    *  @see     Curve::setLineStyle()
    */
   virtual void setLineStyle(const LineStyle& lineStyle) = 0;

   /**
    *  Returns the line style of the curves in the collection.
    *
    *  @return  The line style of all curves in the collection.  If the curves
    *           have different line widths, the value is undefined.
    */
   virtual LineStyle getLineStyle() const = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~CurveCollection() {}
};

#endif
