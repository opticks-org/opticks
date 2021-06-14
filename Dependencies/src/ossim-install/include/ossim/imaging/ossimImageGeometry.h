//***********************************************************************************************
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer (oscarkramer@yahoo.com)
//
// Description: Class declaration of ossimImageGeometry.
//
//***********************************************************************************************
// $Id: ossimImageGeometry.h 3102 2012-01-18 15:30:20Z oscar.kramer $

#ifndef ossimImageGeometry_HEADER
#define ossimImageGeometry_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/base/ossim2dTo2dTransform.h>
#include <vector>
#include <ossim/matrix/newmat.h>
#include <ossim/base/ossimDpt3d.h>
#include <ossim/base/ossimAdjustableParameterInterface.h>

// Forward class declarations:
class ossimDrect;
class ossimIrect;
class ossimGrect;

//***********************************************************************************************
//! Container class that holds both 2D transform and 3D projection information for an image
//! Only one instance of this is alive per image. This is  the object that is returned from
//! a call to ossimImageSource::getImageGeometry()
//! 
//! All images in OSSIM have at least a 2D transformation from local (relative to start of file)
//! to full-image coordinates. Often this transform is an identity transform (does nothing). In 
//! some cases however, the first pixel in the file does not correspond to the upper-left pixel of
//! the original, full image. Since the image's sensor model may only apply to full-image pixel 
//! coordinates, it is necessary to reference the latter when making computing ground point 
//! location.  Also the local image could be a reduced resolution level of the full image.  So
//! the image could be a decimated sub image of the full image.
//!
//! Additionally, images typically are accompanied by some form of metadata that defines the mapping
//! from 2D image coordinates to 3D world coordinates. This mapping may be in the form of a map
//! projection for orthorectified images, or a perspective projection such as a sensor model.
//! 
//! This object class maintains both 2D transform and 3D projection information for the associated
//! image. This object will typically be created and "owned" by the image handler object. Therefore,
//! only one copy per image will exist at a time.
//!
//! Notes:
//! 1) Shifts are relative to "full image space".  So if you have a sub image from r2 the shift
//!    given to the transform should be relative to "full image space".
//
//! 2) A decimation of 1.0 is the full image.  This may or may not be r0 as r0 can be decimated.
//!
//***********************************************************************************************
class OSSIM_DLL ossimImageGeometry : public ossimObject
{
public:
   typedef std::vector<ossim_float64> DeltaParamList;
   
   //! Default constructor defaults to unity transform with no projection.
   ossimImageGeometry();
   virtual ~ossimImageGeometry();
   
   //! Copy constructor -- performs a deep copy. This is needed when an imageSource in the chain
   //! needs to modify the image geometry at that point in the chain. The image geometry to the 
   //! left of that imageSource must remain unchanged. This constructor permits that imageSource to 
   //! maintain its own geometry based on the input geometry. All chained objects to the right
   //! would see this alternate geometry. See ossimScaleFilter for an example of this case.
   ossimImageGeometry(const ossimImageGeometry& copy_this);

   //! Constructs with projection and transform objects available for referencing. Either pointer
   //! can be NULL -- the associated mapping would be identity.
   ossimImageGeometry(ossim2dTo2dTransform* transform, ossimProjection* projection);

   //! Shortcut way of getting to an image's geometry when access to pixels is not needed.
   //! @param image Image to open
   //! @return true if open was successful and a valid geometry was initialized.
   bool open(const ossimFilename& image);

   //! rnToRn is a utility method that takes a rn resolution image point and maps it to the another
   //! rn resolution image point.
   //!
   //! @param inRnPt Is a point in resolution n.
   //! @param inResolutionLevel Is the resolution of the point inRnPt.
   //! @param outResolutionLevel Is the resolution of the point outRnPt.
   //! @param outRnPt Is the result of the transform.
   //!
   void rnToRn(const ossimDpt& inRnPt, ossim_uint32 inResolutionLevel,
               ossim_uint32 outResolutionLevel,ossimDpt& outRnPt) const;
   
   //! rnToFull is a utility method that takes a rn resolution image point and maps it to the full
   //! image point.
   //!
   //! @param rnPt Is a point in resolution n.
   //! @param resolutionLevel Is the resolution of the point rnPt.  a value of 0 is the local image
   //! @param fullPt Is the result of the transform
   //!
   void rnToFull(const ossimDpt& rnPt, ossim_uint32 resolutionLevel, ossimDpt& fullPt) const;

   //! @brief fullToRn is a utility method that takes a full image point and maps it to a rn
   //! resolution image point.
   //! 
   //! @param fullPt Is a point in full image space.
   //! @param resolutionLevel Is the resolution of the point rnPt. A value of 0 is the local image.
   //! @param fullPt Is the result of the transform
   void fullToRn(const ossimDpt& fullPt, ossim_uint32 resolutionLevel, ossimDpt& rnPt) const;

   //! rnToWorld is a utility method that takes a rn resolution image point and maps it to the 
   //! world point.
   //!
   //! @param rnPt Is a point in resolution n.
   //! @param resolutionLevel Is the resolution of the point rnPt.  a value of 0 is the local image
   //! @param wpt Is the result of the transform
   //!
   void rnToWorld(const ossimDpt& rnPt, ossim_uint32 resolutionLevel, ossimGpt& wpt) const;
   
   //! worldToRn is a utility method that takes a world point allows one to transform all the way back to
   //! an rn point.
   //!
   //! @param wpt Ground point.
   //! @param resolutionLevel Is the resolution of the point rnPt.  a value of 0 is the local image
   //! @param rnPt Is the resoltion point.
   //!
   void worldToRn(const ossimGpt& wpt, ossim_uint32 resolutionLevel, ossimDpt& rnPt) const;

   //! Exposes the 3D projection from image to world coordinates. The caller should verify that
   //! a valid projection exists before calling this method. Returns TRUE if a valid ground point
   //! is available in the ground_pt argument. This method depends on the existence of elevation
   //! information. If no DEM is available, the results will be incorrect or inaccurate.
   bool localToWorld(const ossimDpt& local_pt, ossimGpt& world_pt) const;
   bool localToWorld(const ossimDrect& local_rect, ossimGrect& world_rect) const;

   //! Exposes the 3D projection from image to world coordinates given a constant height above 
   //! ellipsoid. The caller should verify that a valid projection exists before calling this
   //! method. Returns TRUE if a valid ground point is available in the ground_pt argument.
   bool localToWorld(const ossimDpt& local_pt, const double& h_ellipsoid, ossimGpt& world_pt) const;

   //! Exposes the 3D world-to-local image coordinate reverse projection. The caller should verify 
   //! that a valid projection exists before calling this method. Returns TRUE if a valid image 
   //! point is available in the local_pt argument.
   bool worldToLocal(const ossimGpt& world_pt, ossimDpt& local_pt) const;
   bool worldToLocal(const ossimGrect& world_rect, ossimDrect& local_rect) const;

   //! Sets the transform to be used for local-to-full-image coordinate transformation
   void setTransform(ossim2dTo2dTransform* transform);

   //! Sets the projection to be used for local-to-world coordinate transformation
   void setProjection(ossimProjection* projection);

   //! Access methods for transform (may be NULL pointer).
   const ossim2dTo2dTransform* getTransform() const { return m_transform.get(); }
   ossim2dTo2dTransform*       getTransform()       { return m_transform.get(); }

   //! Access methods for projection (may be NULL pointer).
   const ossimProjection* getProjection() const { return m_projection.get(); }
   ossimProjection*       getProjection()       { return m_projection.get(); }

   /**
    * @brief Returns true if underlying projection is derived from
    * ossimMapProjection.
    * @return true if map projected; false, if not.
    */
   bool isMapProjected() const;
   
   /**
    * @return const ossimMapProjection* or NULL if projection not set or not
    * derived from ossimMapProjection.
    */
   const ossimMapProjection* getAsMapProjection() const
   { return dynamic_cast<const ossimMapProjection*>( m_projection.get() ); }

   /**
    * @return ossimMapProjection* or NULL if projection not set or not
    * derived from ossimMapProjection.
    */
   ossimMapProjection*       getAsMapProjection()
   { return dynamic_cast<ossimMapProjection*>( m_projection.get() ); }

   //! Returns TRUE if valid projection defined
   bool hasProjection() const { return m_projection.valid(); }
   
   //! Returns TRUE if valid transform defined
   bool hasTransform() const { return m_transform.valid(); }

   //! Returns TRUE if this geometry is sensitive to elevation
   bool isAffectedByElevation() const;

   //! Returns the GSD associated with this image in the active projection. Note that this only
   //! makes sense if there is a projection associated with the image. Returns NaNs if no 
   //! projection defined.
   ossimDpt getMetersPerPixel() const;

   /**
    * @brief Get the ground sample distance(GSD) associated with this image
    * in the active projection.
    *
    * GSD is taken from projection if there is no transform set; else,
    * taken from three localToWorld calls (one pixel apart) at the image
    * center.  If the projection or the image size(if needed) is not set the
    * point will be set to NaNs.
    * 
    * @param gsd Point to intialize with GSD.
    * 
    * @note This only makes sense if there is a projection associated with
    * the image.
    *
    * @note Result should be checked for NaNs after call.
    */
   void getMetersPerPixel( ossimDpt& gsd ) const;

   //! Returns the resolution of this image in degrees/pixel. Note that this only
   //! makes sense if there is a projection associated with the image. Returns NaNs if no 
   //! projection defined.
   ossimDpt getDegreesPerPixel() const;

   /**
    * @brief Get the resolution of this image in degrees/pixel.
    *
    * Degrees/pixel is taken from projection if there is no transform set;
    * else, taken from three localToWorld calls (one pixel apart) at the image
    * center.  If the projection or the image size(if needed) is not set the
    * point will be set to NaNs.
    * 
    * @param dpp Point to intialize with degrees/pixel.
    * 
    * @note This only makes sense if there is a projection associated with
    * the image.
    *
    * @note Result should be checked for NaNs after call.
    */
   void getDegreesPerPixel( ossimDpt& dpp ) const;

   //! Assigns the ossimGpts with the ground coordinates of the four corresponding image 
   //! corner points. Returns true if points are valid.
   bool getCornerGpts(ossimGpt& ul, ossimGpt& ur, ossimGpt& lr, ossimGpt& ll) const;

   /**
    * @brief Get the latitude, longitude of the tie point.
    *
    * This is the bounding upper left point of the image which is not
    * necessarily the image (0, 0) point.
    * 
    * @param tie ossimGpt to assign.
    *
    * @param edge If true the tie point is shifted up and to the right by
    * half the gsd.
    *
    * @note Requires projection and image size to be initialized or the
    * latitude and longitue will be set to nan.
    *
    * @note The height parameter of the ossimGpt is not touched by this method.
    */
   void getTiePoint(ossimGpt& tie, bool edge) const;

   /**
    * @brief Get the easting, northing of the tie point.
    *
    * This is the bounding upper left point of the image which is not
    * necessarily the image (0, 0) point.
    * 
    * @param tie ossimDpt to assign.
    *
    * @param edge If true the tie point is shifted up and to the right by
    * half the gsd.
    * 
    * @note Requires projection and image size to be initialized or the
    * easting and northing will be set to nan.
    */
   void getTiePoint(ossimDpt& tie, bool edge) const;

   //! Prints contents to output stream.
   std::ostream& print(std::ostream& out) const;

   //! Returns TRUE if argument geometry has exactly the same instances for transform and 
   //! m_projection. This should be expanded to permit different instances of same transforms
   bool operator==(const ossimImageGeometry& compare_this) const;

   //! Replaces any existing transform and projection with those in the copy_this object
   const ossimImageGeometry& operator=(const ossimImageGeometry& copy_this);

   //! Returns the decimation factor from R0 for the resolution level specified. For r_index=0, the 
   //! decimation factor is by definition 1.0. For the non-discrete case, r_index=1 returns a 
   //! decimation of 0.5. If the vector of discrete decimation factors (m_decimationFactors) is 
   //! empty, the factor will be computed as f=1/2^n
   ossimDpt decimationFactor(ossim_uint32 r_index) const;

   /**
    * @brief Method to get the decimation factor for a given resolution
    * level.
    *
    * If the array of decimations is not initialized by owner, the default is:
    * r_index=0 is by definition 1.0.
    * r_index=n Where n is some level the factor will be computed as f=1/2^n.
    *
    * @param resLevel Reduced resolution set for requested decimation.
    *
    * @param result ossimDpt to initialize with requested decimation.
    */
   void decimationFactor(ossim_uint32 r_index, ossimDpt& result) const;

   /**
    * @brief Gets array of all decimation levels.
    * @param decimations Array to initialiaze.
    */
   void decimationFactors(std::vector<ossimDpt>& decimations) const;

   //! Sets the decimation scheme to a discrete list of decimation factors.
   void setDiscreteDecimation(const std::vector<ossimDpt>& decimation_list) 
      { m_decimationFactors = decimation_list; }

   //! @return The number of decimation factors
   ossim_uint32 getNumberOfDecimations()const
   {
      return (ossim_uint32)m_decimationFactors.size();
   }
   void setImageSize(const ossimIpt& size)
   {
      m_imageSize = size;
   }
   const ossimIpt& getImageSize()const
   {
      return m_imageSize;
   }

   bool getCrossesDateline()const;
   

   void getImageEdgePoints(std::vector<ossimDpt>& result, ossim_uint32 partitions = 25)const;

   /**
   * This is the first stage implementation.   It will determine if it crosses the dateline
   * and do a special multi polygon for the ossimPolyArea2d result.  For each edge
   * of the image we walk "partitions" number of points.   If we cross a dateline we calculate
   * the crossing lat by using the parametric form of the equation.   For example, if we
   * are crossing from positive to negative that means we are going through the 180 degree lon
   * location and we will need to solve for the paramtetric parameter t and plug back into the parametric
   * equation to solve for the latitude.  Basic form:  start + (end-start)*t = 180.  If we are coming from
   * negative to positive then we solve basic form: start + (end-start)*t = -180.  Where deltaPoint is (end-start) 
   *
   *                   ossim_float64 t = ((180-start.x)/deltaPoint.x);
   *                   ossim_float64 lat = (start.y+deltaPoint.y*t);
   *
   * If we do not cross the dateline then if the image is affected by elevation (i.e. a sensor model) then we use the partitions
   * parameter to calculate that number of partitions to sample along each edge.
   *
   * if The geometry is not affected by elevation then we just use the corner points and ignore the partitions parameter
   *
   * @param poly holds the resulting polygon.  This could be a MultiPolygon depending on the geometry
   * @param partitions These are the number of steps you want when walking the border.
   */
   void calculatePolyBounds(ossimPolyArea2d& result, ossim_uint32 partitions = 25)const;

   /**
    * @brief Get the bounding rect of (0, 0) to (imageSize.x-1, imageSize.y-1).
    *
    * Relies on image size being initialized.
    *
    * @param bounding_rect Initialized by this method.  Will do a
    * ossimIrect::makeNan() if the image size is not initialized.
    */
   void getBoundingRect(ossimIrect& bounding_rect) const;
   void getBoundingRect(ossimDrect& bounding_rect) const; // Overloaded for drect.
   
   void getBoundingGroundRect(ossimGrect& bounding_grect) const;

   //! Creates a new instance of ossimImageGeometry with the same transform and projection.
   //! Overrides base-class version requiring loadState() and saveState() (not implemented yet)
   virtual ossimObject* dup() const { return new ossimImageGeometry(*this); }

   //! Attempts to initialize a transform and a projection given the KWL. Returns TRUE if no
   //! error encountered.
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   //! Saves the transform (if any) and projection (if any) states to the KWL.
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0) const;

   /**
    * @brief Set m_targetRrds data member.
    *
    * This is used by methods worldToRn and localToWorld that do not take a rrds
    * argument and methods rnToWorld and worldToRn.  If the target rrds is set to 2,
    * then the resulting point from worldToLocal is relative to reduced
    * resolution level 2.  Vice versa for localToWorld it is assumed the local
    * point is relative to the target
    * reduced resolution data set.
    *
    * @param rrds Target (zero based) reduced resolution data set.
    */
   void setTargetRrds(ossim_uint32 rrds);

   /**
    * @return The target zero based reduced resolution data set used for localToWorld and
    * worldToLocal.
    * @see setTargetRrds 
    */
   ossim_uint32 getTargetRrds() const;

   //! @brief  Changes the GSD and image size to reflect the scale provided.
   //! @param scale  The scale to be applied in x and y directions
   //! @param recenterTiePoint If true the will adjust the tie point by shifting the original tie 
   //! to the upper left corner, applying scale, then shifting back by half of either the new
   //! theDeltaLat/lon or theMetersPerPixel depending on if underlying projection isGeographic.
   void applyScale(const ossimDpt& scale, bool recenterTiePoint);

   virtual bool isEqualTo(const ossimObject& obj, ossimCompareType compareType = OSSIM_COMPARE_FULL)const;



   // If we have an adjustable parameter interface return one.
   //
   // @return a pointer to an ossimAdjustableParameterInterface or NULL if no
   //         adjustable paramters exist
   //
   virtual ossimAdjustableParameterInterface* getAdjustableParameterInterface();
   virtual const ossimAdjustableParameterInterface* getAdjustableParameterInterface()const;
   
   
   // Compute partials from image to ground with respect to the adjustable parameters.
   //
   //
   bool computeImageToGroundPartialsWRTAdjParam(ossimDpt& result,
                                                          const ossimGpt& gpt,
                                                          ossim_uint32 idx,
                                                          ossim_float64 paramDelta=1.0);
   bool computeImageToGroundPartialsWRTAdjParams(NEWMAT::Matrix& result,
                                                           const ossimGpt& gpt,
                                                           ossim_float64 paramDelta=1.0);
   bool computeImageToGroundPartialsWRTAdjParams(NEWMAT::Matrix& result,
                                                           const ossimGpt& gpt,
                                                           const DeltaParamList& deltas);
   
   bool computeGroundToImagePartials(NEWMAT::Matrix& result,
                                     const ossimGpt& gpt,
                                     const ossimDpt3d& deltaLlh);
   
   bool computeGroundToImagePartials(NEWMAT::Matrix& result,
                                     const ossimGpt& gpt);

   /**
    * @return Returns the angle to "up is up" in decimal degrees, 0.0 if image
    * is not affected by elevation, ossim::nan on error.  
    */
   ossim_float64 upIsUpAngle(const ossimDpt&) const;
   ossim_float64 upIsUpAngle() const;

   /**
    * @return Returns the angle to "north up" in decimal degrees, ossim::nan
    * on error.
    */
   ossim_float64 northUpAngle() const;
   
protected:
   //! @brief Method to back out decimation of a point.
   //! @param rnPt Is a point in resolution n.
   //! @param resolutionLevel Is the resolution of the point rnPt.
   //! @param outPt Is the result of the transform a non-decimated point.
   void undecimatePoint(const ossimDpt& rnPt,
                        ossim_uint32 resolutionLevel,
                        ossimDpt& outPt) const;

   //! @brief Method to apply decimation of a point.
   //! @param inPt Is a point with no decimation.
   //! @param resolutionLevel Is the resolution of the point rnPt.
   //! @param rnPt Is the result of the transform
   void decimatePoint(const ossimDpt& inPt,
                      ossim_uint32 resolutionLevel,
                      ossimDpt& rnPt) const;

   ossimRefPtr<ossim2dTo2dTransform> m_transform;   //!< Maintains local_image-to-full_image transformation 
   ossimRefPtr<ossimProjection>      m_projection;  //!< Maintains full_image-to-world_space transformation
   std::vector<ossimDpt>             m_decimationFactors; //!< List of decimation factors for R-levels
   ossimIpt                          m_imageSize; // Image width and height

   /** @brief Target rrds for localToWorld and worldToLocal methods. */
   ossim_uint32                      m_targetRrds; 

   TYPE_DATA
};

#endif /* #ifndef ossimImageGeometry_HEADER */
