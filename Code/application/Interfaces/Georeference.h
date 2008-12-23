/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCE_H
#define GEOREFERENCE_H

#include "LocationType.h"

class QWidget;
class RasterElement;

/**
 *  Interface specific to georeference plug-ins.
 *
 *  Defines the georeference specific interface to all algorithm plug-ins. 
 *  This interface contains all georeference specific operations.
 *
 *  For Georeference plug-ins to properly serialize and deserialize as part of
 *  a session, they need to implement SessionItem::serialize and 
 *  SessionItem::deserialize.
 */
class Georeference
{
public:
   /**
    *  Takes a scene pixel coordinate and returns the corresponding 
    *  geocoordinate value.
    *
    *  @param   pixel
    *           The scene pixel location as a LocationType
    *
    *  @return  The corresponding geocoordinate as a LocationType.
    */
   virtual LocationType pixelToGeo(LocationType pixel) const = 0;

   /**
    *  Takes a scene pixel coordinate and returns the approximate corresponding 
    *  geocoordinate value.  This function will accomplish the goal faster
    *  than pixelToGeo() if possible.
    *
    *  @param   pixel
    *           The scene pixel location as a LocationType
    *
    *  @return  The corresponding geocoordinate as a LocationType.
    */
   virtual LocationType pixelToGeoQuick(LocationType pixel) const = 0;

   /**
    *  Takes a geocoordinate and returns the corresponding pixel
    *  coordinate value.
    *
    *  @param   geo
    *           The geocoordinate as a LocationType
    *
    *  @return  The corresponding pixel as a LocationType.
    */
   virtual LocationType geoToPixel(LocationType geo) const = 0;

   /**
    *  Takes a  and returns the approximate corresponding geocoordinate
    *  scene pixel coordinate value.  This function will accomplish the goal faster
    *  than pixelToGeo() if possible.
    *
    *  @param   geo
    *           The geocoordinate location as a LocationType
    *
    *  @return  The corresponding pixel as a LocationType.
    */
   virtual LocationType geoToPixelQuick(LocationType geo) const = 0;

   /**
    *  Gets a QWidget to set all parameters needed by the georeferencing algorithm.
    *
    *  The calling method takes ownership of the returned widget.  The returned widget
    *  may be destroyed at any time after calling Executable::execute() on the plug-in.
    *
    *  @param   pRaster
    *           The RasterElement to create the GUI for.
    *
    *  @return  The widget with any appropriate controls, or \b NULL if interactive mode
    *           is not supported or no controls are needed.
    */
   virtual QWidget* getGui(RasterElement* pRaster) = 0;

   /**
    *  Determines if the user input through the GUI is valid.
    *
    *  @return  Returns \b true if the input is valid, otherwise returns
    *           \b false.
    *
    *  @see     getGui()
    */
   virtual bool validateGuiInput() const = 0;

   /**
    * Determine if this georeferencing algorithm can be used for the given 
    * RasterElement.
    *
    * @param pRaster
    *        The RasterElement to test.
    * @return \c true if the plugin can handle the RasterElement, \c false otherwise
    */
   virtual bool canHandleRasterElement(RasterElement *pRaster) const = 0;

   /**
    *  Specifies whether the plug-in can accurately extrapolate beyond the
    *  extents of its associated raster element.
    *
    *  E.g. if this method returns \c true, the status bar will display geocoordinates
    *  regardless of whether the cursor is within its associated raster element
    *  or not.
    *
    *  @return  Returns \c true if the plug-in can extrapolate, otherwise returns
    *           \c false.
    */
   virtual bool canExtrapolate() const = 0;

protected:
   /**
    *  Since the Georeference interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Georeference() {}
};

#endif
