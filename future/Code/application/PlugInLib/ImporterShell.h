/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTERSHELL_H
#define IMPORTERSHELL_H

#include "EnumWrapper.h"
#include "ExecutableShell.h"
#include "Importer.h"

#include <string>

/**
 *  \ingroup ShellModule
 *  %Importer Shell
 *
 *  This class represents the shell for an importer plug-in.  %Importer
 *  developers would take this class and extend it to support thier 
 *  importer specific code.
 *
 *  @see     ExecutableShell, Importer
 */
class ImporterShell : public ExecutableShell, public Importer
{
public:
   /**
    *  Creates an importer plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::ImporterType().
    *
    *  @see     getType()
    */
   ImporterShell();

   /**
    *  Destroys the importer plug-in.
    */
   virtual ~ImporterShell();

   /**
    *  @copydoc Importer::getDefaultExtensions()
    *
    *  @default The default implementation returns the extension string that
    *           was passed into setExtensions().  If setExtensions() has not
    *           yet been called, an empty string is returned.
    */
   virtual std::string getDefaultExtensions() const;

   /**
    *  @copydoc Importer::isProcessingLocationSupported()
    *
    *  @default Only a processing location of ProcessingLocation::IN_MEMORY is
    *           supported.  If \em location is ProcessingLocation::IN_MEMORY,
    *           the default implementation returns \b true, otherwise \b false
    *           is returned.
    */
   virtual bool isProcessingLocationSupported(ProcessingLocation location) const;

   /**
    *  @copydoc Importer::getPreview()
    *
    *  @default The default implementation of this method does not create a
    *           preview and returns \b NULL.
    */
   virtual QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);

   /**
    *  @copydoc Importer::validate()
    *
    *  @default The default implementation calls getValidationTest() to get the
    *           tests that should be used to validate the data set and then
    *           performs each of the tests specified.
    *
    *  @warning This method should not be called from within execute() because
    *           one or more of the tests (e.g.
    *           \link ImporterShell::NO_EXISTING_DATA_ELEMENT
    *           NO_EXISTING_DATA_ELEMENT\endlink) may fail since the import
    *           process has already started.
    */
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

   /**
    *  @copydoc Importer::getImportOptionsWidget()
    *
    *  @default The default implementation returns \b NULL.
    */
   virtual QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);

   /**
    *  @copydoc Importer::polishDataDescriptor()
    *
    *  @default The default implementation does nothing.
    */
   virtual void polishDataDescriptor(DataDescriptor* pDescriptor);

protected:
   /**
    *  Sets the default file extensions recognized by the importer.
    *
    *  @param   extensions
    *           The file extensions recognized by the importer.  The string
    *           should consist of a description followed by one or more
    *           extensions separated by a space.  Multiple file types may
    *           be specified with a double semicolon.  Examples include
    *           "ENVI Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   void setExtensions(const std::string& extensions);

   /**
    *  Identifies the test that should be performed to validate the import.
    *
    *  @see     validate(), getValidationTest(), getValidationError()
    */
   enum ValidationTestEnum
   {
      NO_VALIDATION = 0x00000000,             /**< 0x00000000 - No validation is performed. */
      EXISTING_FILE = 0x00000001,             /**< 0x00000001 - Checks that the filename contained in the file
                                                   descriptor is a valid file that exists on the disk. */
      NO_EXISTING_DATA_ELEMENT = 0x00000002,  /**< 0x00000002 - Checks that an existing DataElement does not already
                                                   exist by calling ModelServices::getElement(). */
      VALID_CLASSIFICATION = 0x00000004,      /**< 0x00000004 - Checks for the existence of classification markings by
                                                   checking the return value of DataDescriptor::getClassification() for
                                                   a non-\c NULL pointer.\   Also reports a warning if the
                                                   classification level of the data being imported is greater than the
                                                   overall classification level of the system. */
      VALID_METADATA = 0x00000008,            /**< 0x00000008 - Checks for the existence of metadata by checking the
                                                   return value of DataDescriptor::getMetadata() for a non-\c NULL
                                                   pointer. */
      VALID_PROCESSING_LOCATION = 0x00000010, /**< 0x00000010 - Checks that the processing location set in the data
                                                   descriptor is valid by calling isProcessingLocationSupported(). */
      RASTER_SIZE = 0x00000020,               /**< 0x00000020 - Checks for at least one data pixel by checking for a
                                                   non-zero number of rows, columns, bands, and bits per element. */
      VALID_DATA_TYPE = 0x00000040,           /**< 0x00000040 - Checks that the data type set in the raster data
                                                   descriptor is one of the valid data types returned by
                                                   RasterDataDescriptor::getValidDataTypes(). */
      NO_HEADER_BYTES = 0x00000080,           /**< 0x00000080 - Checks for no header bytes set on the raster file
                                                   descriptor. */
      NO_PRE_POST_LINE_BYTES = 0x00000100,    /**< 0x00000100 - Checks for no preline or postline bytes set on the
                                                   raster file descriptor. */
      NO_PRE_POST_BAND_BYTES = 0x00000200,    /**< 0x00000200 - Checks for no preband or postband bytes set on the
                                                   raster file descriptor. */
      NO_TRAILER_BYTES = 0x00000400,          /**< 0x00000400 - Checks for no trailer bytes set on the raster file
                                                   descriptor. */
      FILE_SIZE = 0x00000800 | EXISTING_FILE, /**< 0x00000800 - Checks that the size of the file set in the raster file
                                                   descriptor (in bytes) is greater than or equal to required file size
                                                   determined by calling RasterUtilities::calculateFileSize(). */
      NO_BAND_FILES = 0x00001000,             /**< 0x00001000 - Checks that the number of band files is zero.\   Should
                                                   not be combined with \em EXISTING_BAND_FILES. */
      EXISTING_BAND_FILES = 0x00002000,       /**< 0x00002000 - Checks that number of band files is equal to the number
                                                   of bands in the raster file descriptor and that each of the band
                                                   files exists on the disk.\   Should not be combined with
                                                   \em NO_BAND_FILES. */
      BAND_FILE_SIZES = 0x00004000 | EXISTING_BAND_FILES, /**< 0x00004000 - Checks that the size of each band file (in
                                                   bytes) is greater than or equal to the required file size determined
                                                   by calling RasterUtilities::calculateFileSize(). */
      VALID_BAND_NAMES = 0x00008000 | VALID_METADATA, /**< 0x00008000 | \em VALID_METADATA - Checks that the number of
                                                   band names set in the metadata is equal to the number of bands in
                                                   the raster file descriptor.\   This test will succeed if band names
                                                   are not present in the metadata. */
      VALID_WAVELENGTHS = 0x00010000 | VALID_METADATA, /**< 0x00010000 | \em VALID_METADATA - Checks that the number of
                                                   wavelengths set in the metadata is equal to the number of bands in
                                                   the raster file descriptor by calling
                                                   Wavelengths::getNumWavelengths().\   This test will succeed if
                                                   wavelengths are not present in the metadata. */
      NO_INTERLEAVE_CONVERSIONS = 0x00020000, /**< 0x00020000 - Checks that the interleave format in the raster data
                                                   descriptor matches the interleave format in the raster file
                                                   descriptor.\   No check is performed if
                                                   RasterFileDescriptor::getBandCount() returns 1. */
      NO_ROW_SKIP_FACTOR = 0x00040000,        /**< 0x00040000 - Checks for no skip factor in the raster data descriptor
                                                   rows by calling RasterDataDescriptor::getRowSkipFactor(). */
      NO_COLUMN_SKIP_FACTOR = 0x00080000,     /**< 0x00080000 - Checks for no skip factor in the raster data descriptor
                                                   columns by calling RasterDataDescriptor::getColumnSkipFactor(). */
      NO_SKIP_FACTORS = NO_ROW_SKIP_FACTOR | NO_COLUMN_SKIP_FACTOR, /**< \em NO_ROW_SKIP_FACTOR |
                                                   \em NO_COLUMN_SKIP_FACTOR - Convenience value that performs both
                                                   \em NO_ROW_SKIP_FACTOR and \em NO_COLUMN_SKIP_FACTOR checks. */
      NO_ROW_SUBSETS = 0x00100000,            /**< 0x00100000 - Checks that the number of rows to import in the raster
                                                   data descriptor matches the number of rows in the raster file
                                                   descriptor. */
      NO_COLUMN_SUBSETS = 0x00200000,         /**< 0x00200000 - Checks that the number of columns to import in the
                                                   raster data descriptor matches the number of columns in the raster
                                                   file descriptor. */
      NO_BAND_SUBSETS = 0x00400000,           /**< 0x00400000 - Checks that the number of bands to import in the raster
                                                   data descriptor matches the number of bands in the raster file
                                                   descriptor. */
      NO_SUBSETS = NO_ROW_SUBSETS | NO_COLUMN_SUBSETS | NO_BAND_SUBSETS, /**< \em NO_ROW_SUBSETS |
                                                   \em NO_COLUMN_SUBSETS | \em NO_BAND_SUBSETS - Convenience value that
                                                   performs all \em NO_ROW_SUBSETS, \em NO_COLUMN_SUBSETS, and
                                                   \em NO_BAND_SUBSETS checks. */
      AVAILABLE_MEMORY = 0x00800000           /**< 0x00800000 - Checks that the amount of required memory calculated
                                                   from the rows, columns, bands, and bytes per element set in the
                                                   raster data descriptor can be successfully allocated. */
   };

   /**
    *  @EnumWrapper  ImporterShell::ValidationTestEnum.
    */
   typedef EnumWrapper<ValidationTestEnum> ValidationTest;

   /**
    *  Returns the test that should be performed when validating the given data
    *  set for import.
    *
    *  This method is called by validate() to determine which tests should be
    *  performed to validate the import.  This method should be overridden by
    *  derived importers to add additional tests or remove default tests.
    *
    *  @param   pDescriptor
    *           The data descriptor for the data set that is being imported.
    *
    *  @return  Returns the test that should be used to validate the import.
    *           The value should be an OR'd combination of
    *           \link ImporterShell::ValidationTest ValidationTest \endlink
    *           values.
    *
    *  @default The default implementation of this method returns the OR'd
    *           combination of the following tests:
    *           - \link ImporterShell::EXISTING_FILE EXISTING_FILE \endlink
    *           - \link ImporterShell::NO_EXISTING_DATA_ELEMENT NO_EXISTING_DATA_ELEMENT \endlink
    *           - \link ImporterShell::VALID_PROCESSING_LOCATION VALID_PROCESSING_LOCATION \endlink
    */
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

   /**
    *  Returns the test that failed during the last call to validate().
    *
    *  This method can be called by importers from within an override of the
    *  default implementation of validate() to determine which test failed the
    *  validation process while calling the base class ImporterShell::validate()
    *  method.  This allows the importer to report custom error messages or
    *  to continue importing by converting an error to a warning and changing
    *  the validate() return value to \c true.
    *
    *  This method should not be called for importers that do not call this
    *  base class implementation.
    *
    *  @return  Returns the validation test that failed during the last call to
    *           ImporterShell::validate().  If this base class method has not
    *           been called or returned \c true, then an invalid value is
    *           returned.  An invalid value is also returned if the data
    *           descriptor passed into validate() or its file descriptor are
    *           \c NULL.
    */
   ValidationTest getValidationError() const;

private:
   std::string mExtension;
   mutable ValidationTest mValidationError;
};

#endif
