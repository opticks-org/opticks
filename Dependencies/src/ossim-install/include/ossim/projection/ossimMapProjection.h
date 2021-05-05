//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Base class for all map projections.
// 
//*******************************************************************
//  $Id: ossimMapProjection.h 22938 2014-11-01 11:43:35Z okramer $

#ifndef ossimMapProjection_HEADER
#define ossimMapProjection_HEADER 1

#include <ossim/projection/ossimProjection.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimKeyword.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimDrect.h>
#include <iostream>

#include <ossim/base/ossimMatrix4x4.h>

class ossimKeywordlist;

class OSSIMDLLEXPORT ossimMapProjection : public ossimProjection
{
public:
   ossimMapProjection(const ossimEllipsoid& ellipsoid = ossimEllipsoid(),
                      const ossimGpt& origin=ossimGpt());

   ossimMapProjection(const ossimMapProjection& src);

   virtual ossimGpt origin()const;

   /**
    * This method will convert the world coordinate to  model coordinates (easting,
    * northing in meters). It will be necessary then to transform the map coordinates returned by
    * this method into to line, sample by calling eastingNorthingToLineSample(). Alternatively,
    * just use worldToLineSample() to skip the intermediate model coordinates.
    */
   virtual ossimDpt forward(const ossimGpt &worldPoint) const = 0;

   /**
    * Will take a point in meters and convert it to ground.
    * This methid will convert the model coordinates (easting, northing in meters) to world
    * coordinates. Note that the projectedPoint is not line, sample on the image. It is necessary to
    * first transform the image coordinates into to map easting, northing by calling
    * lineSampleToEastingNorthing(). Alternatively, just use lineSampleToWorld() to skip the
    * intermediate model coordinates.
    */
   virtual ossimGpt inverse(const ossimDpt &projectedPoint)const = 0;

   virtual ossimDpt worldToLineSample(const ossimGpt &worldPoint)const;
   virtual void     worldToLineSample(const ossimGpt &worldPoint,
                                      ossimDpt&       lineSample)const;
   
   virtual ossimGpt lineSampleToWorld(const ossimDpt &projectedPoint)const;
   virtual void     lineSampleToWorld(const ossimDpt &projectedPoint,
                                      ossimGpt& gpt)const;
   /**
    * This is the virtual that projects the image point to the given
    * elevation above ellipsoid, thereby bypassing reference to a DEM. Useful
    * for projections that are sensitive to elevation.
    */
   virtual void lineSampleHeightToWorld(const ossimDpt& lineSampPt,
                                        const double&   heightAboveEllipsoid,
                                        ossimGpt&       worldPt) const;

   /** Performs image to model coordinate transformation. */
   virtual void lineSampleToEastingNorthing(const ossimDpt& lineSample,
                                            ossimDpt& eastingNorthing) const;

   /** Performs model to image coordinate transformation. */
   virtual void eastingNorthingToLineSample(const ossimDpt& eastingNorthing,
                                            ossimDpt& lineSample) const;

   virtual void eastingNorthingToWorld(const ossimDpt& eastingNorthing,
                                       ossimGpt&       worldPt)const;

   /** @return The false easting. */
   virtual double getFalseEasting() const;

   /** @return The false northing. */
   virtual double getFalseNorthing() const;
   
   /**
    * Derived classes should implement as needed.
    * This implementation returns 0.0.
    * 
    * @return The first standard parallel.
    */
   virtual double getStandardParallel1() const;

   /**
    * Derived classes should implement as needed.
    * This implementation returns 0.0.
    * 
    * @return The second standard parallel.
    */
   virtual double getStandardParallel2() const;
   
   virtual void update();

   virtual void setPcsCode(ossim_uint32 pcsCode);

   //! Returns the EPSG PCS code or 32767 if the projection is a custom (non-EPSG) projection
   virtual ossim_uint32 getPcsCode()const;

   /**
    *  Returns the projection name.
    */
   virtual ossimString getProjectionName() const;
   
   /**
    * ACCESS METHODS: 
    */
   virtual double getA() const;
   virtual double getB() const;
   virtual double getF() const;
   
   virtual ossimDpt getMetersPerPixel() const;

   /**
    * Returns decimal degrees per pixel as an ossimDpt with
    * "x" representing longitude and "y" representing latitude.
    */
   virtual const ossimDpt& getDecimalDegreesPerPixel() const;
   
   virtual const ossimDpt&   getUlEastingNorthing() const;
   virtual const ossimGpt&   getUlGpt() const;
   virtual const ossimDatum* getDatum() const;
   const ossimEllipsoid&     getEllipsoid() const { return theEllipsoid; }
   const ossimGpt& getOrigin() const;
   virtual bool isGeographic()const;

   /**
    * Applies scale to theDeltaLonPerPixel, theDeltaLatPerPixel and theMetersPerPixel data members
    * (eg: theDeltaLonPerPixel *= scale.x). The image-to-model transform is recomputed.
    *
    * @param scale Multiplier to be applied to theDeltaLonPerPixel,
    * theDeltaLatPerPixel and theMetersPerPixel
    *
    * @param recenterTiePoint If true the will adjust the tie point by
    * shifting the original tie to the upper left corner, applying scale,
    * then shifting back by half of either the new
    * theDeltaLat/lon or theMetersPerPixel depending on if projection
    * isGeographic.
    */
   virtual void applyScale(const ossimDpt& scale, bool recenterTiePoint);

   /**
    * Applies clockwise rotation to the image-to-map coordinates. Scale and offset are preserved.
    */
   virtual void applyRotation(const double& azimuth_degrees);
   bool isRotated() const { return (theImageToModelAzimuth != 0.0); }
   const double& getRotation() const { return theImageToModelAzimuth; }
   /**
    * SET METHODS: 
    */
   virtual void setEllipsoid(const ossimEllipsoid& ellipsoid);
   virtual void setAB(double a, double b);

   /**
    * Sets theDatum to datum.
    * Sets theEllipsiod to datum's ellipsiod.
    * Calls update().
    */
   virtual void setDatum(const ossimDatum* datum);

   /**
    * Sets theOrigin to origin.
    * Sets theDatum to origin's datum.
    * Sets theEllipsiod to datum's ellipsiod.
    * Calls update().
    *
    * @param origin The projection origin.
    */
   virtual void setOrigin(const ossimGpt& origin);
   
   virtual void setMetersPerPixel(const ossimDpt& gsd);
   virtual void setDecimalDegreesPerPixel(const ossimDpt& gsd);
   virtual void setUlTiePoints(const ossimGpt& gpt);
   virtual void setUlTiePoints(const ossimDpt& eastingNorthing);
   virtual void setUlEastingNorthing(const  ossimDpt& ulEastingNorthing);
   virtual void setUlGpt(const  ossimGpt& ulGpt);
   virtual void assign( const  ossimProjection &aProjection);
   
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0) const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Prints data members to stream.  Returns stream&.
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * Compares this to arg projection and returns TRUE if the same. NOTE: As currently implemented,
    * in OSSIM, map projections also contain image geometry information like tiepoint and scale.
    * This operator is only concerned with the map specification and ignores image geometry
    * differences. I.e., theModelTransform is not compared.
    */
   virtual bool operator==(const ossimProjection& projection) const;

   /**
    * Utility method to snap the tie point to some multiple.
    *
    * This is used to overcome inaccuracies in formats like NITF's rpc tag
    * witch has a four digit decimal degree precision for the lat/lon offset.
    *
    * @param multiple Value to snap tie point to like 1.0.
    *
    * @param unitType OSSIM_METERS or OSSIM_DEGREES.
    *
    * EXAMPLE1:
    * projection:    ossimUtmProjection
    * tie point is:  551634.88, 4492036.16
    * multiple:      1.0
    * unitType:      OSSIM_METERS
    * new tie point: 551635.00, 4492036.00
    *
    * EXAMPLE2:
    * projection:    ossimEquDistCylProjection
    * tie point is:  -90.3246343      42.0297589
    * multiple:      0.0001
    * unitType:      OSSIM_DEGREES
    * new tie point: -90.3246 42.0298
    */
   void snapTiePointTo(ossim_float64 multiple, ossimUnitType unitType);
   void snapTiePointToOrigin();
                      
   void setElevationLookupFlag(bool flag);
   bool getElevationLookupFlag()const;

   const ossimMatrix4x4& getModelTransform() const { return theModelTransform; }

   /**
    * Sets the map-to-image rotation, scale and offset. In other words, converts easting, northing
    * in meters to x, y pixels on the image. 
    * @param transform
    */
   void setModelTransform(const ossimMatrix4x4& transform);

   /**
    * @brief Implementation of pure virtual
    * ossimProjection::isAffectedByElevation method.
    * @return false.
    */
   virtual bool isAffectedByElevation() const { return false; }
   
   void setProjectionUnits(ossimUnitType units) { theProjectionUnits = units; }

   /**
    * OSSIM considers all map projection coordinates (including false eastings and northings) to
    * be in meters. However, some standard projection specifications (cf EPSG spec) indicate
    * coordinates in feet (US and Intl). Since the methods forward() and inverse() always deal in
    * meters, the user of this projection may need to convert their map coordinates to meters prior
    * to calling inverse() (and after calling forward() if the coordinates need to be in the
    * original unit. This class keeps track of the original units so that a user can query and
    * convert the map point coordinates to meters as needed.
    */
   ossimUnitType getProjectionUnits() const { return theProjectionUnits; }
   
   virtual bool isEqualTo(const ossimObject& obj, ossimCompareType compareType = OSSIM_COMPARE_FULL)const;


protected:
   //! Computes the approximate resolution in degrees/pixel
   virtual void computeDegreesPerPixel();

   /**
    * This will go from the ground point and give
    * you an approximate meters per pixel. the Delta Lat
    * and delta lon will be in degrees.
    */
   virtual void computeMetersPerPixel();

   /**
    * Recomputes the image-to-model transform given GSD and UL corner parameters
    */
   virtual void updateTransform();

   /** Extracts tiepoint, scale and rotation info from transform */
   virtual void updateFromTransform();

   virtual ~ossimMapProjection();

   /** This static method converts the image-to-model transform matrix from degrees
    *  into meters as required by the base class and GeoTrans code. It uses the geographic
    *  origin given as the offset terms in the input transform to compute scaling */
   void convertImageModelTransformToMeters();

   /**
    * This method verifies that the projection parameters match the current
    * pcs code.  If not this will set the pcs code to 0.
    *
    * @return true if match, false if not.
    */
   //bool verifyPcsCodeMatches() const;
   
   ossimEllipsoid   theEllipsoid;
   ossimGpt         theOrigin;

   /**
    * This is only set if we want to have built in datum shifting
    */
   const ossimDatum* theDatum;
   
   /**
    * Holds the number of meters per pixel.
    * x = easting
    * y = northing
    */
   ossimDpt          theMetersPerPixel;

   /**
    * Hold the decimal degrees per pixel.
    * x = longitude
    * y = latitude
    */
   ossimDpt          theDegreesPerPixel;

   /** Hold tie point in decimal degrees. */
   ossimGpt          theUlGpt;

   /**
    * Hold tie point as easting northing.
    * x = easting,
    * y = northing
    */
   ossimDpt          theUlEastingNorthing;

   /**
    * Hold the false easting northing.
    * x = false easting,
    * y = false northing
    */
   ossimDpt          theFalseEastingNorthing;

   //! Projection Coordinate System(PCS) code. Mutable because they may update automatically
   //! after the projection is initialized, even in a const method. Normally set to 0 until the
   //! corresponding EPSG code can be determined. If the projection is NOT represented by any
   //! EPSG code, then the PCS is set to 32767.
   mutable ossim_uint32      thePcsCode;

   bool              theElevationLookupFlag;

   /**
    * Will always be a 4x4 matrix. Provides affine scaling, rotation, and offset to the image line,
    * sample (x, y) to arrive at the map coordinates (easting, northing). The latter are then
    * projected to the ground given specific map projection equations. Note: only the first 2 rows
    * are used as follows.
    *                      [ e, n ]t = M(4-cols x 2-rows) * [ x, y, 0, 1 ]t   (t = transpose)
    * See GeoTIFF tag 34264 specification.
    */
   ossimMatrix4x4 theModelTransform; // goes from image to model
   ossimMatrix4x4 theInverseModelTransform; //goes from model back to image

   //! Linear units of the projection as indicated in the projection's specification. All projections
   //! internal to OSSIM use meters. The EPSG spec may indicate otherwise so users can check if
   //! they need to convert original map coordinates to meters by checking this:
   ossimUnitType theProjectionUnits;

   /** Image azimuth relative to map model coordinates. Applies to image-to-model transform */
   double theImageToModelAzimuth;

TYPE_DATA
};


#endif
