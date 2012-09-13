/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEDESCRIPTOR_H
#define GEOREFERENCEDESCRIPTOR_H

#include "ConfigurationSettings.h"
#include "DynamicObject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

/**
 *  Algorithm parameters for performing georeference.
 *
 *  The georeference descriptor contains all parameters necessary to
 *  georeference raster data.  The class contains a number of fixed parameters
 *  that are common to all Georeference plug-ins.  It also inherits
 *  DynamicObject to allow Georeference plug-ins to specify custom parameters as
 *  DynamicObject attributes.
 *
 *  A GeoreferenceDescriptor instance is contained in a RasterDataDescriptor so
 *  that individual raster data sets can have unique parameter values.  The user
 *  edits the parameters of the georeference descriptor in the Import Options
 *  dialog and also in the %Georeference dialog.  When editing the georeference
 *  parameters, the user can switch between various Georeference plug-ins, and
 *  when doing so any custom attributes set by the plug-in are not deleted.
 *  Therefore custom attributes should be added in a parent DynamicObject
 *  attribute with the Georeference plug-in name as the attribute name.  For
 *  example, the GCP %Georeference plug-in will set a custom polynomial order
 *  attribute as follows:
 *  \code
 *  pGeorefDescriptor->setAttributeByPath("GCP Georeference/PolynomialOrder", order);
 *  \endcode
 *
 *  This subclass of Subject will notify upon the following conditions:
 *     - The following methods are called: setGeoreferenceOnImport(),
 *       setGeoreferencePlugInName(), setValidGeoreferencePlugIns(),
 *       resetValidGeoreferencePlugIns(), setCreateLayer(), setLayerName(),
 *       setDisplayLayer(), setGeocoordType(), and setLatLonFormat().
 *     - Everything else documented in DynamicObject.
 *
 *  @see     DynamicObject, RasterDataDescriptor::setGeoreferenceDescriptor(),
 *           DynamicObject::setAttributeByPath()
 */
class GeoreferenceDescriptor : public DynamicObject
{
public:
   SETTING(AutoGeoreference, GeoreferenceDescriptor, bool, true)
   SETTING(CreateLayer, GeoreferenceDescriptor, bool, true)
   SETTING(DisplayLayer, GeoreferenceDescriptor, bool, false)
   SETTING(GeocoordType, GeoreferenceDescriptor, GeocoordType, GEOCOORD_LATLON)
   SETTING(LatLonFormat, GeoreferenceDescriptor, DmsFormatType, DMS_FULL)

   /**
    *  Emitted when the georeference-on-import parameter changes with
    *  boost::any<bool> containing the new value indicating whether
    *  georeferencing should automatically be performed on import.
    *
    *  @see     setGeoreferenceOnImport()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, GeoreferenceOnImportChanged)

   /**
    *  Emitted when the georeference plug-in name changes with
    *  boost::any<std::string> containing the new name of the plug-in that
    *  should perform the georeference.
    *
    *  @see     setGeoreferencePlugInName()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, GeoreferencePlugInNameChanged)

   /**
    *  Emitted when the valid Georeference plug-ins change with
    *  boost::any<std::vector<std::string> > containing the names of the
    *  plug-ins that can perform the georeference.
    *
    *  @see     setValidGeoreferencePlugIns()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, ValidGeoreferencePlugInsChanged)

   /**
    *  Emitted when the create layer parameter changes with
    *  boost::any<bool> containing the new value indicating whether
    *  a results layer should be created after performing the georeference.
    *
    *  @see     setCreateLayer()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, CreateLayerChanged)

   /**
    *  Emitted when the georeference results layer name changes with
    *  boost::any<std::string> containing the new name of the layer that
    *  is created after performing the georeference.
    *
    *  @see     setLayerName()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, LayerNameChanged)

   /**
    *  Emitted when the display layer parameter changes with
    *  boost::any<bool> containing the new value indicating whether the created
    *  results layer should be displayed by default after performing the
    *  georeference.
    *
    *  @see     setDisplayLayer()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, DisplayLayerChanged)

   /**
    *  Emitted when the geographic coordinate type for the results layer changes
    *  with boost::any<\link ::GeocoordType GeocoordType\endlink> containing the
    *  new results layer coordinate type.
    *
    *  @see     setGeocoordType()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, GeocoordTypeChanged)

   /**
    *  Emitted when the latitude/longitude display format changes with
    *  boost::any<\link ::DmsFormatType DmsFormatType\endlink> containing the
    *  new latitude/longitude display format used by the results layer.
    *
    *  @see     setLatLonFormat()
    */
   SIGNAL_METHOD(GeoreferenceDescriptor, LatLonFormatChanged)

   /**
    *  Sets whether the data should automatically be georeferenced on import.
    *
    *  @param   georeference
    *           Set this value to \c true to automatically georeference the
    *           data on import or to \c false to require the user manually
    *           georeference the data after import.
    *
    *  @notify  This method notifies signalGeoreferenceOnImportChanged() when
    *           the value indicating whether the data should be automatically
    *           georeferenced on import changes.
    */
   virtual void setGeoreferenceOnImport(bool georeference) = 0;

   /**
    *  Returns whether the data should automatically be georeferenced on import.
    *
    *  The initial value for this parameter is set to the return value of
    *  getSettingAutoGeoreference() when the GeoreferenceDescriptor is created.
    *
    *  @return  Returns \c true if the data should automatically be
    *           georeferenced on import or \c false if the user will be required
    *           to manually georeference the data after import.
    */
   virtual bool getGeoreferenceOnImport() const = 0;

   /**
    *  Sets the name of the Georeference plug-in that should be used to
    *  georeference the data.
    *
    *  If this method has not been called by the time the raster data set is
    *  validated during the import process, a plug-in will be determined
    *  automatically based on the valid Georeference plug-ins by
    *  calling RasterDataDescriptor::setDefaultGeoreferencePlugIn().
    *
    *  @param   plugInName
    *           The name of the Georeference plug-in to use when georeferencing
    *           the data.  If \em plugInName is not empty and the plug-in is not
    *           a valid Georeference plug-in as determined by
    *           isValidGeoreferencePlugIn(), this method does nothing.
    *
    *  @notify  This method notifies signalGeoreferencePlugInNameChanged() when
    *           the plug-in name changes.
    *
    *  @see     Importer::validate(), Georeference::validate(),
    *           getValidGeoreferencePlugIns()
    */
   virtual void setGeoreferencePlugInName(const std::string& plugInName) = 0;

   /**
    *  Returns the name of the Georeference plug-in that should be used to
    *  georeference the data.
    *
    *  @return  Returns the name of the Georeference plug-in to use when
    *           georeferencing the data.  If setGeoreferencePlugInName() has not
    *           been called, or if the georeferencing plug-in has not yet been
    *           determined automatically during the import process, this method
    *           returns an empty string by default.
    *
    *  @see     setGeoreferencePlugInName()
    */
   virtual const std::string& getGeoreferencePlugInName() const = 0;

   /**
    *  Sets the Georeference plug-ins that can be selected by the user to
    *  georeference the data.
    *
    *  Only valid Georeference plug-ins will be displayed to the user when
    *  selecting a plug-in to perform georeference, either on import or after
    *  import.  An importer may call this method to restrict the available
    *  georeference plug-ins if the importer will perform the georeference
    *  itself with a certain Georeference plug-in.
    *
    *  By default all Georeference plug-ins are valid.
    *
    *  @param   plugInNames
    *           The name of the Georeference plug-ins to display to the user
    *           when selecting a plug-in to georeference the data.  If a name
    *           in the vector is empty or not a Georeference plug-in, it is
    *           ignored.
    *
    *  @notify  This method notifies signalValidGeoreferencePlugInsChanged()
    *           when the list of plug-ins changes.
    *
    *  @see     resetValidGeoreferencePlugIns()
    */
   virtual void setValidGeoreferencePlugIns(const std::vector<std::string>& plugInNames) = 0;

   /**
    *  Queries whether a Georeference plug-in is available to the user to
    *  georeference the data.
    *
    *  @return  Returns \c true if the given Georeference plug-in name is
    *           displayed to the user when selecting a plug-in to georeference
    *           the data; otherwise returns \c false.
    *
    *  @see     setValidGeoreferencePlugIns()
    */
   virtual bool isValidGeoreferencePlugIn(const std::string& plugInName) const = 0;

   /**
    *  Returns the Georeference plug-ins that are available for the user to
    *  select when georeferencing the data.
    *
    *  @return  Returns the names of the Georeference plug-ins that are
    *           displayed to the user when georeferencing the data.
    *
    *  @see     setValidGeoreferencePlugIns()
    */
   virtual const std::vector<std::string>& getValidGeoreferencePlugIns() const = 0;

   /**
    *  Resets the Georeference plug-ins that can be selected by the user to
    *  georeference the data.
    *
    *  This method resets the valid Georeference plug-ins to the default state
    *  of all Georeference plug-ins.
    *
    *  @notify  This method notifies signalValidGeoreferencePlugInsChanged()
    *           if the list of valid Georeference plug-ins changes.
    *
    *  @see     setValidGeoreferencePlugIns()
    */
   virtual void resetValidGeoreferencePlugIns() = 0;

   /**
    *  Sets whether a geographic results layer should be created after
    *  georeferencing the data.
    *
    *  When set to create a results layer, a LatLonLayer is created to display
    *  the georeferencing results to the user.  The layer is initially shown or
    *  hidden based on the return value of getDisplayLayer().
    *
    *  @param   createLayer
    *           Set this value to \c true to create a geographic results layer
    *           or to \c false to not create the layer.  To set the initial
    *           display state of the created layer, see setDisplayLayer().
    *
    *  @note    Regardless of whether a results layer will be created,
    *           performing georeference deletes any existing results layers
    *           displaying the georeferenced RasterElement.  This method only
    *           sets whether to create a new layer after all other existing
    *           results layers have been deleted.
    *
    *  @notify  This method notifies signalCreateLayerChanged() when the value
    *           indicating whether to create the geographic results layer
    *           changes.
    */
   virtual void setCreateLayer(bool createLayer) = 0;

   /**
    *  Returns whether a geographic results layer should be created after
    *  georeferencing the data.
    *
    *  The initial value for this parameter is set to the return value of
    *  getSettingCreateLayer() when the GeoreferenceDescriptor is created.
    *
    *  @return  Returns \c true if a geographic results layer should be created
    *           after georeferencing the data or \c false if the layer should
    *           not be created.
    *
    *  @note    Regardless of whether a results layer will be created,
    *           performing georeference deletes any existing results layers
    *           displaying the georeferenced RasterElement.  This method only
    *           indicates whether a new layer will be created after all other
    *           existing results layers have been deleted.
    */
   virtual bool getCreateLayer() const = 0;

   /**
    *  Sets the default name for the geographic results layer that is created
    *  after georeferencing the data.
    *
    *  @param   layerName
    *           The name to use for the results layer that is created after
    *           georeferencing the data.
    *
    *  @notify  This method notifies signalLayerNameChanged() when the name for
    *           the results layer changes.
    */
   virtual void setLayerName(const std::string& layerName) = 0;

   /**
    *  Returns the name for the geographic results layer that is created after
    *  georeferencing the data.
    *
    *  The initial value for this parameter is set to "GEO_RESULTS" when the
    *  GeoreferenceDescriptor is created.
    *
    *  @return  Returns name to use for the results layer that is created after
    *           georeferencing the data.
    */
   virtual const std::string& getLayerName() const = 0;

   /**
    *  Sets whether the geographic results layer should be displayed by default
    *  after georeferencing the data.
    *
    *  @param   displayLayer
    *           Set this value to \c true to display the geographic results
    *           layer or to \c false to not display the layer.
    *
    *  @notify  This method notifies signalDisplayLayerChanged() when the value
    *           indicating whether to display the geographic results layer
    *           changes.
    *
    *  @warning This parameter is ignored if getCreateLayer() returns \c false.
    */
   virtual void setDisplayLayer(bool displayLayer) = 0;

   /**
    *  Returns whether the geographic results layer should be displayed after
    *  georeferencing the data.
    *
    *  The initial value for this parameter is set to the return value of
    *  getSettingDisplayLayer() when the GeoreferenceDescriptor is created.
    *
    *  @return  Returns \c true if a geographic results layer should be
    *           displayed after georeferencing the data or \c false if the layer
    *           should not be displayed.
    *
    *  @warning This parameter is ignored if getCreateLayer() returns \c false.
    */
   virtual bool getDisplayLayer() const = 0;

   /**
    *  Sets the coordinate type to be used when creating the geographic results
    *  layer.
    *
    *  @param   geocoordType
    *           The coordinate type for the geographic results layer.
    *
    *  @notify  This method notifies signalGeocoordTypeChanged() when the
    *           coordinate type changes.
    */
   virtual void setGeocoordType(GeocoordType geocoordType) = 0;

   /**
    *  Returns the coordinate type to be used when creating the geographic
    *  results layer.
    *
    *  The initial value for this parameter is set to the return value of
    *  getSettingGeocoordType() when the GeoreferenceDescriptor is created.
    *
    *  @return  Returns the coordinate type to use when creating the geographic
    *           results layer.
    */
   virtual GeocoordType getGeocoordType() const = 0;

   /**
    *  Sets the display format for latitude/longitude values in the geographic
    *  results layer.
    *
    *  This value only applies if the coordinate type for the layer is set to
    *  \link GeocoordType::GEOCOORD_LATLON GEOCOORD_LATLON\endlink.
    *
    *  @param   latLonFormat
    *           The display format for the latitude/longitude values in the
    *           geographic results layer.
    *
    *  @notify  This method notifies signalLatLonFormatChanged() when the
    *           display format for the latitude/longitude values changes.
    */
   virtual void setLatLonFormat(DmsFormatType latLonFormat) = 0;

   /**
    *  Returns the display format for latitude/longitude values in the
    *  geographic results layer.
    *
    *  The initial value for this parameter is set to the return value of
    *  getSettingLatLonFormat() when the GeoreferenceDescriptor is created.
    *
    *  @return  Returns the display format for the latitude/longitude values in
    *           the geographic results layer.  This value only applies if the
    *           coordinate type for the layer is set to
    *           \link GeocoordType::GEOCOORD_LATLON GEOCOORD_LATLON\endlink.
    */
   virtual DmsFormatType getLatLonFormat() const = 0;

   /**
    *  Sets all values and attributes in this georeference descriptor to those
    *  of another georeference descriptor.
    *
    *  @param   pGeorefDescriptor
    *           The georeference descriptor from which to set all values and
    *           attributes in this georeference descriptor.  No signal/slot
    *           attachments currently defined in \em pGeorefDescriptor are set
    *           into this descriptor.  This method does nothing and returns
    *           \c false if \c NULL is passed in.
    *
    *  @return  Returns \c true if all values and attributes in this
    *           georeference descriptor were successfully set to the values and
    *           attribute values in the given georeference descriptor; otherwise
    *           returns \c false.
    *
    *  @notify  This method notifies one or more signals defined in this class
    *           based on whether their values actually change.
    *
    *  @see     compare()
    */
   virtual bool clone(const GeoreferenceDescriptor* pGeorefDescriptor) = 0;

protected:
   /**
    *  The GeoreferenceDescriptor object contained in a RasterDataDescriptor
    *  will be destroyed by the RasterDataDescriptor itself and does not need to
    *  be destroyed explicitly.\   %Any other GeoreferenceDescriptor should be
    *  destroyed either as part of a FactoryResource object or by calling
    *  ObjectFactory::destroyObject().
    */
   virtual ~GeoreferenceDescriptor()
   {}
};

#endif
