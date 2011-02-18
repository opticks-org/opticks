/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTLIST_H
#define TIEPOINTLIST_H

#include "DataElement.h"

#include <string>
#include <vector>

/**
 *  Ties a geographical position between two scenes.
 *
 *  A %TiePoint stores the geographical relationship between a point in one
 *  scene and the same geographical point in another scene. The relationship
 *  is stored in terms of pixel coordinates.
 *
 *  @see     TiePointList
 */
class TiePoint
{
public:
   struct
   {
      int mX, mY;
   } mReferencePoint;
   struct
   {
      float mX, mY;
   } mMissionOffset;
   int mConfidence;
   int mPhi;

   bool operator== (const TiePoint& tiePoint) const
   {
      if ((tiePoint.mReferencePoint.mX == mReferencePoint.mX) &&
         (tiePoint.mReferencePoint.mY == mReferencePoint.mY) &&
         (tiePoint.mMissionOffset.mX == mMissionOffset.mX) &&
         (tiePoint.mMissionOffset.mY == mMissionOffset.mY) &&
         (tiePoint.mConfidence == mConfidence) &&
         (tiePoint.mPhi == mPhi))
      {
         return true;
      }

      return false;
   }
};

/**
 *  Stores a set of tie points.
 *
 *  A %TiePointList is a DataElement that stores a set of tie points.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: adoptTiePoints().
 *  - Everything else documented in DataElement.
 *
 *  @see     DataElement
 */
class TiePointList : public DataElement
{
public:
   /**
    *  Sets the name of the mission dataset on the tie point list.
    *
    *  Sets the name of the mission dataset of the tie point list.
    *  A tie point list has two associated datasets. This method
    *  allows the specification of the second one. The name of the
    *  reference dataset is the name that is used when the tie point
    *  list is created.
    *
    *  @param   missionName
    *           The name of the mission dataset.
    *
    *  @see     ModelServices::createElement()
    */
   virtual void setMissionDatasetName(std::string missionName) = 0;

   /**
    *  Gets the name of the mission dataset of the tie point list.
    *
    *  Gets the name of the mission dataset of the tie point list.
    *  A tie point list has two associated datasets. This method
    *  allows querying of the second one. The normal
    *  DataElement::getName() method gets the name of the reference
    *  dataset.
    *
    *  @return   The name of the mission dataset.
    */
   virtual const std::string &getMissionDatasetName() const = 0;

   /**
    *  Gets a reference to the tie points in the list.
    *
    *  @return   The tie points in the list.
    */
   virtual const std::vector<TiePoint>& getTiePoints() const = 0;

   /**
    *  Sets the tie points in the list.
    *
    *  This method sets the tie points in the list and overwrites whatever
    *  tie points have already been to the list.  It replaces rather than
    *  appends to the list.
    *
    *  The implementation of this method calls swap() on the given vector
    *  to avoid allocating potentially large amounts of memory.  After
    *  calling this method, the contents of the given vector will be empty.
    *
    *  @param   points
    *           The points to set in the list.  This is a non-const vector so
    *           that swap() can be called on the vector to avoid allocating
    *           potentially large amounts of memory.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void adoptTiePoints(std::vector<TiePoint>& points) = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~TiePointList() {}
};

#endif
