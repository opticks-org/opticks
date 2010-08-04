/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEASUREMENTOBJECTIMP_H
#define MEASUREMENTOBJECTIMP_H

#include "AttachmentPtr.h"
#include "DrawUtil.h"
#include "LineObjectImp.h"
#include "RasterElement.h"
#include "TypesFile.h"

class GraphicLayer;

/**
 * This class is used to draw a measurement object onto the screen.  It is derived from annotation object,
 * however, it is not used through the annotation toolbar / UI.
 */
class MeasurementObjectImp : public LineObjectImp
{
public:

   /**
   * Constructor for MeasurementObject
   * 
   * This method constructs a MeasurementObject
   * 
   *   @param id
   *      The unique ID for the object.
   *   @param pLayer
   *      The annotation layer to construct the object in
   *   @param
   *      the location to draw the measurement object
   */
   MeasurementObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~MeasurementObjectImp();

   /**
   * Draw object to screen
   *
   * This method is called by the application to draw the MeasurementObject to the
   * screen
   */
   void draw(double zoomFactor) const;

   bool setProperty(const GraphicProperty* pProp);

   bool replicateObject(const GraphicObject* pObject);

   void setDistancePrecision(int precision);
   int getDistancePrecision() const;
   void setBearingPrecision(int precision);
   int getBearingPrecision() const;
   void setEndPointsPrecision(int precision);
   int getEndPointsPrecision() const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected:

private:
   int mDistancePrecision;
   int mBearingPrecision;
   int mEndPointsPrecision;
   mutable bool mUsingInaccurateGeocoords;

   void refreshGeoInformation() const;
   std::string generateGeoStrings() const;

   double mArrowHeadOffset;   //!< The half-width of the arrow head (in pixels, this value will be scaled)
   double mBarEndLength;      //!< The length of the end-bars (in pixels, this value will be scaled)
   double mArrowRelStartLoc;  //!< The relative start location along the arrow line to start drawing the text.
                              //!< This value is a percentage in decimal form.  Min value is 0.0, Max value is 1.0.
   double mArrowRelEndLoc;    //!< The relative end location along the arrow line to stop drawing the text.
                              //!< This value is a percentage in decimal form.  Min value is 0.0, Max value is 1.0
   double mArrowOffset;       //!< Offset from main line to arrow line (in pixels, this value will be scaled)
   double mTextOffset;        //!< Offset from a location to where text is drawn (in pixels, this value will be scaled)
   
   void updateGeoreferenceAttachment();
   void georeferenceModified(Subject &subject, const std::string &signal, const boost::any &v);

   AttachmentPtr<RasterElement> mpGeoreference;
   
   // cached data generated from bounding box and unit selection
   mutable QString mBearingText;            //!< The bearing text
   mutable QString mDistanceText;           //!< The distance text
   mutable QString mStartLocText;           //!< The start location text
   mutable QString mEndLocText;             //!< The end location text

   mutable DrawUtil::TextTexture mBearingTextTexture;    //!< The stored texture info for bearing text
   mutable DrawUtil::TextTexture mDistanceTextTexture;   //!< The stored texture info for distance text
   mutable DrawUtil::TextTexture mStartLocTextTexture;   //!< The stored texture info for start location text
   mutable DrawUtil::TextTexture mEndLocTextTexture;     //!< The stored texture info for end location text
   
   mutable DistanceUnits mDrawnDistanceUnit;
   mutable DmsFormatType mDrawnDmsFormat;
   mutable GeocoordType mDrawnGeocoord;

};

#define MEASUREMENTOBJECTADAPTEREXTENSION_CLASSES \
   LINEOBJECTADAPTEREXTENSION_CLASSES

#define MEASUREMENTOBJECTADAPTER_METHODS(impClass) \
   LINEOBJECTADAPTER_METHODS(impClass)

#endif   // MEASUREMENTOBJECT_H
