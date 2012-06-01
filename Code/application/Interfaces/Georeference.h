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

#include <string>

class QWidget;
class RasterDataDescriptor;

/**
 *  Interface specific to georeference plug-ins.
 *
 *  Defines the interface required to georeference raster data. This interface
 *  contains the following georeference-specific operations:
 *
 *  <table>
 *    <tr><td><b>Operation</b></td><td><b>Available Methods</b></td></tr>
 *    <tr><td>Query raster data to check if it can be georeferenced</td>
 *      <td>getGeoreferenceAffinity()</td></tr>
 *    <tr><td>Create a GUI for the user to modify georeference parameters</td>
 *      <td>getWidget()</td></tr>
 *    <tr><td>Verify that user-specified georeference parameters are acceptable
 *      for georeferencing</td><td>validate()</td></tr>
 *    <tr><td>Perform coordinate transformations</td><td>pixelToGeo()<br>
 *      pixelToGeoQuick()<br>geoToPixel()<br>geoToPixelQuick()</td></tr>
 *  </table>
 *
 *  A Georeference plug-in must implement SessionItem::serialize() and
 *  SessionItem::deserialize() to save and restore properly as part of a
 *  session.
 */
class Georeference
{
public:
   /**
    *  The name for a GcpList argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  GcpList pointer when the plug-in is executed as part of the
    *  auto-georeferencing capability of RasterElementImporterShell.  Arguments
    *  with this name should be of the type GcpList.
    */
   static std::string GcpListArg()
   {
      return std::string("GCP List");
   }

   /**
    *  Determines if this georeferencing algorithm can be used for a given
    *  raster data descriptor.
    *
    *  A Georeference plug-in must implement this method to provide information
    *  about whether it supports a given raster data set, which is similar to
    *  the functionality of Importer::getFileAffinity().
    *
    *  This method is called automatically on import if auto-georeferencing is
    *  enabled and after import when the user manually georeferences the raster
    *  data.
    *
    *  The following code demonstrates how a georeference plug-in can optionally
    *  not support georeferencing on import but support georeferencing after
    *  import:
    *  \code
    *  unsigned char MyGeoreferencePlugIn::getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const
    *  {
    *     // Check if the raster element is loaded, which indicates georeferencing after import.
    *     if (Service<ModelServices>()->getElement(pDescriptor) != NULL)
    *     {
    *        return Georeference::CAN_GEOREFERENCE;
    *     }
    *
    *     // The raster element is not loaded, which indicates georeferencing on import.
    *     return Georeference::CAN_NOT_GEOREFERENCE;
    *  }
    *  \endcode
    *
    *  @param   pDescriptor
    *           The raster data descriptor to test.
    *
    *  @return  Typically returns one of the following values, however any
    *           unsigned char value can be returned if a specific Georeference
    *           plug-in requires more granularity.
    *           <ul>
    *             <li>Georeference::CAN_NOT_GEOREFERENCE</li>
    *             <li>Georeference::CAN_GEOREFERENCE_WITH_USER_INPUT</li>
    *             <li>Georeference::CAN_GEOREFERENCE_DATA_TYPE</li>
    *             <li>Georeference::CAN_GEOREFERENCE</li>
    *           </ul>
    *           A Georeference plug-in can return a value greater than
    *           Georeference::CAN_GEOREFERENCE if it wishes to override another
    *           plug-in that can georeference the raster data.  For example, the
    *           RPC %Georeference plug-in may override the GCP %Georeference
    *           plug-in if the georeferencing will be more accurate.
    *
    *  @see     validate()
    */
   virtual unsigned char getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const = 0;

   /**
    *  When a Georeference plug-in returns this value from
    *  getGeoreferenceAffinity() it means the plug-in cannot georeference the
    *  given raster data.
    */
   static const unsigned char CAN_NOT_GEOREFERENCE = 0;

   /**
    *  When a Georeference plug-in returns this value from
    *  getGeoreferenceAffinity() it means the plug-in does not detect any
    *  supported geographic information in the raster data, but may still be
    *  able to georeference the raster data if provided additional user input.
    *  For example, the IGM %Georeference plug-in can georeference any raster
    *  data set, but will still require additional information (i.e. the IGM
    *  filename) from the user.
    */
   static const unsigned char CAN_GEOREFERENCE_WITH_USER_INPUT = 64;

   /**
    *  When a Georeference plug-in returns this value from
    *  getGeoreferenceAffinity() it means that the plug-in recognizes the
    *  geographic information in the raster data but will still require
    *  additional user input to successfully georeference the data.
    */
   static const unsigned char CAN_GEOREFERENCE_DATA_TYPE = 128;

   /**
    *  When a Georeference plug-in returns this value from
    *  getGeoreferenceAffinity() it means the plug-in can georeference the given
    *  raster data.
    */
   static const unsigned char CAN_GEOREFERENCE = 192;

   /**
    *  Returns a widget in which the user can modify parameters needed by the
    *  georeferencing algorithm.
    *
    *  This method provides an interface for which specialized georeference
    *  parameters for a data set can be displayed to the user.  The method
    *  returns a Qt widget that is added to either the Import Options dialog
    *  during import, or the %Georeference dialog after import.  The
    *  Georeference plug-in should create the widget with a \c NULL parent, and
    *  should destroy the widget when the plug-in itself is destroyed.
    *
    *  @param   pDescriptor
    *           The raster data for which the create the widget containing
    *           custom georeference parameters.
    *
    *  @return  A QWidget containing widgets to edit custom georeference
    *           parameters, or \c NULL if the plug-in does not have any custom
    *           parameters to display to the user.
    */
   virtual QWidget* getWidget(RasterDataDescriptor* pDescriptor) = 0;

   /**
    *  Queries whether a given raster data descriptor can be successfully
    *  georeferenced by the plug-in.
    *
    *  This method is called automatically for the plug-in to parse the current
    *  settings in the raster data descriptor to see if it supports
    *  georeferencing the data as currently specified in the raster data
    *  descriptor.  During import, this method is called each time the user
    *  changes a value in the Import Options dialog.  After import, it is called
    *  when the user clicks the OK button in the %Georeference dialog.
    *
    *  @param   pDescriptor
    *           The raster data descriptor to query if it can be successfully
    *           georeferenced.
    *  @param   errorMessage
    *           An error message string that is populated with the reason why
    *           the plug-in cannot successfully georeference the given raster
    *           data descriptor.  This message will be displayed to the user
    *           either in the Import Options dialog or in a message box when
    *           the user clicks the OK button in the %Georeference dialog.
    *           If this method returns \c true, this message will be displayed
    *           to the user as a warning.  If this method returns \c false, this
    *           message will be displayed to the user as an error.
    *
    *  @return  Returns \c true if the plug-in can successfully georeference the
    *           given raster data descriptor; otherwise returns \c false.
    *
    *  @see     getGeoreferenceAffinity()
    */
   virtual bool validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const = 0;

   /**
    *  Takes a scene pixel coordinate and returns the corresponding 
    *  geocoordinate value.
    *
    *  @param   pixel
    *           The scene pixel location as a LocationType
    *  @param   pAccurate
    *           Output indicator of conversion accuracy. Georeference plug-ins that
    *           can not accurately extrapolate should return \c false when \c pixel is
    *           outside the extents of the reference points. When \c NULL, no accuracy
    *           check is performed. 
    *
    *  @return  The corresponding geocoordinate as a LocationType.
    */
   virtual LocationType pixelToGeo(LocationType pixel, bool* pAccurate = NULL) const = 0;

   /**
    *  Takes a scene pixel coordinate and returns the approximate corresponding 
    *  geocoordinate value.  This function will accomplish the goal faster
    *  than pixelToGeo() if possible.
    *
    *  @param   pixel
    *           The scene pixel location as a LocationType
    *  @param   pAccurate
    *           Output indicator of conversion accuracy. Georeference plug-ins that
    *           can not accurately extrapolate should return \c false when \c pixel is
    *           outside the extents of the reference points. When \c NULL, no accuracy
    *           check is performed. 
    *
    *  @return  The corresponding geocoordinate as a LocationType.
    */
   virtual LocationType pixelToGeoQuick(LocationType pixel, bool* pAccurate = NULL) const = 0;

   /**
    *  Takes a geocoordinate and returns the corresponding pixel
    *  coordinate value.
    *
    *  @param   geo
    *           The geocoordinate as a LocationType
    *  @param   pAccurate
    *           Output indicator of conversion accuracy. Georeference plug-ins that
    *           can not accurately extrapolate should return \c false when \c geo is
    *           outside the extents of the reference points. When \c NULL, no accuracy
    *           check is performed. 
    *
    *  @return  The corresponding pixel as a LocationType.
    */
   virtual LocationType geoToPixel(LocationType geo, bool* pAccurate = NULL) const = 0;

   /**
    *  Takes a  and returns the approximate corresponding geocoordinate
    *  scene pixel coordinate value.  This function will accomplish the goal faster
    *  than pixelToGeo() if possible.
    *
    *  @param   geo
    *           The geocoordinate location as a LocationType
    *  @param   pAccurate
    *           Output indicator of conversion accuracy. Georeference plug-ins that
    *           can not accurately extrapolate should return \c false when \c geo is
    *           outside the extents of the reference points. When \c NULL, no accuracy
    *           check is performed. 
    *
    *  @return  The corresponding pixel as a LocationType.
    */
   virtual LocationType geoToPixelQuick(LocationType geo, bool* pAccurate = NULL) const = 0;

protected:
   /**
    *  Since the Georeference interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Georeference()
   {}
};

#endif
