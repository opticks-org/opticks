/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPECIALMETADATA_H
#define SPECIALMETADATA_H

#include <string>

/**
 * Placeholder name for the end of DynamicObject::getAttributeByPath() and
 * DynamicObject::setAttributeByPath().
 */
#define END_METADATA_NAME (std::string("END_METADATA_NAME"))

/**
 * The key for a DynamicObject which contains all metadata that is treated specially.
 *
 * @see \ref specialmetadata
 */
#define SPECIAL_METADATA_NAME (std::string("Special"))

/**
 * The key for a DynamicObject used to contain row-oriented metadata.
 *
 * %Any items contained within this dynamic object will be chipped when the
 * RasterElement is chipped.  For more details see RasterUtilities::chipMetadata().  %Any
 * vectors contained in this item, should be indexed using DimensionDescriptor::getActiveNumber().
 */
#define ROW_METADATA_NAME (std::string("Row"))

/**
 * The key for a DynamicObject used to contain column-oriented metadata.
 *
 * %Any items contained within this dynamic object will be chipped when the
 * RasterElement is chipped.  For more details see RasterUtilities::chipMetadata().  %Any
 * vectors contained in this item, should be indexed using DimensionDescriptor::getActiveNumber().
 */
#define COLUMN_METADATA_NAME (std::string("Column"))

/**
 * The key for a DynamicObject used to contain band-oriented metadata.
 *
 * %Any items contained within this dynamic object will be chipped when the
 * RasterElement is chipped.  For more details see RasterUtilities::chipMetadata().  %Any
 * vectors contained in this item, should be indexed using DimensionDescriptor::getActiveNumber().
 */
#define BAND_METADATA_NAME (std::string("Band"))

/**
 * The key for a vector<double> which contains the center wavelengths of the associated 
 * RasterElement's bands.  The wavelengths should be in microns.
 *
 * This vector should always be the size of RasterDataDescriptor::getBandCount().  An importer
 * should specifically populate this vector with RasterFileDescriptor::getBandCount() number
 * of items.  The center wavelengths should be populated with the center spectral wavelength for
 * the given band represented by DimensionDescriptor::getActiveNumber().
 */
#define CENTER_WAVELENGTHS_METADATA_NAME (std::string("CenterWavelengths"))

/**
 * The complete path for a vector<double> which contains the center wavelengths of the associated 
 * RasterElement's bands.  The wavelengths should be in microns.
 *
 * @see CENTER_WAVELENGTHS_METADATA_NAME
 */
#define CENTER_WAVELENGTHS_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + CENTER_WAVELENGTHS_METADATA_NAME)

/**
 * The key for a vector<double> which contains the start wavelengths of the associated 
 * RasterElement's bands.  The wavelengths should be in microns.
 *
 * This vector should always be the size of RasterDataDescriptor::getBandCount().  An importer
 * should specifically populate this vector with RasterFileDescriptor::getBandCount() number
 * of items.  The start wavelengths should be populated with the starting point of the spectral wavelength for
 * the given band represented by DimensionDescriptor::getActiveNumber().
 */
#define START_WAVELENGTHS_METADATA_NAME (std::string("StartWavelengths"))

/**
 * The complete path for a vector<double> which contains the start wavelengths of the associated 
 * RasterElement's bands.  The wavelengths should be in microns.
 *
 * @see START_WAVELENGTHS_METADATA_NAME
 */
#define START_WAVELENGTHS_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + START_WAVELENGTHS_METADATA_NAME)

/**
 * The key for a vector<double> which contains the end wavelengths of the associated 
 * RasterElement's bands.  The wavelengths should be in microns.
 *
 * This vector should always be the size of RasterDataDescriptor::getBandCount().  An importer
 * should specifically populate this vector with RasterFileDescriptor::getBandCount() number
 * of items.  The end wavelengths should be populated with the stopping point of the  spectral wavelength for
 * the given band represented by DimensionDescriptor::getActiveNumber().
 */
#define END_WAVELENGTHS_METADATA_NAME (std::string("EndWavelengths"))

/**
 * The complete path for a vector<double> which contains the end wavelengths of the associated 
 * RasterElement's bands.  The wavelengths should be in microns.
 *
 * @see END_WAVELENGTHS_METADATA_NAME
 */
#define END_WAVELENGTHS_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + END_WAVELENGTHS_METADATA_NAME)

/**
 * The key for a vector<string> which contains the names of the associated 
 * RasterElement's bands.
 *
 * This vector should always be the size of RasterDataDescriptor::getBandCount().  An importer
 * should specifically populate this vector with RasterFileDescriptor::getBandCount() number
 * of items.  The band names should be populated with the names of the band that should
 * be displayed to the user.  If this value is present, 
 * SPECIAL_METADATA_NAME / BAND_NAME_PREFIX_METADATA_NAME will be ignored.
 */
#define NAMES_METADATA_NAME (std::string("Names"))

/**
 * The key for a string which contains a common prefix for the names of the associated
 * RasterElement's bands.
 * 
 * If provided, this text will be prepended to the DimensionDescriptor::getOriginalNumber()
 * of a given band before displaying it to the user.  This value will not be used if 
 * SPECIAL_METADATA_NAME / NAMES_METADATA_NAME has been populated.
 */
#define BAND_NAME_PREFIX_METADATA_NAME (std::string("BandNamePrefix"))

/**
 * The key for a DateTime which specifies when the associated DataElement was
 * captured.
 *
 * This key should not be used when there is no clear single 
 * date/time associated with the DataElement.
 */
#define COLLECTION_DATE_TIME_METADATA_NAME (std::string("CollectionDateTime"))

/**
 * The full path for a DateTime which specifies when the associated DataElement was
 * captured.
 *
 * @see COLLECTION_DATE_TIME_METADATA_NAME
 */
#define COLLECTION_DATE_TIME_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + COLLECTION_DATE_TIME_METADATA_NAME)

/**
* The key for a double which specifies the altitude of the sensor when the associated DataElement was
* captured.
*/
#define SENSOR_ALTITUDE_METADATA_NAME (std::string("SensorAltitude"))

/**
* The full path for a double which specifies the altitude of the sensor when the associated DataElement was
* captured.
*
* @see SENSOR_ALTITUDE_METADATA_NAME
*/
#define SENSOR_ALTITUDE_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + SENSOR_ALTITUDE_METADATA_NAME)

/**
* The key for a double which specifies the altitude of the target when the associated DataElement was
* captured.
*/
#define TARGET_ALTITUDE_METADATA_NAME (std::string("TargetAltitude"))

/**
* The full path for a double which specifies the altitude of the target when the associated DataElement was
* captured.
*
* @see TARGET_ALTITUDE_METADATA_NAME
*/
#define TARGET_ALTITUDE_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + TARGET_ALTITUDE_METADATA_NAME)

/**
* The key for a double which specifies the off-nadir angle of the sensor when the associated DataElement was
* captured.
*/
#define OFF_NADIR_ANGLE_METADATA_NAME (std::string("OffNadirAngle"))

/**
* The full path for a double which specifies the off-nadir angle of the sensor when the associated DataElement was
* captured.
*
* @see OFF_NADIR_ANGLE_METADATA_NAME
*/
#define OFF_NADIR_ANGLE_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + OFF_NADIR_ANGLE_METADATA_NAME)

#endif
