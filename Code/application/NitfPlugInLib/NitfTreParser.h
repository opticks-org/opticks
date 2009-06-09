/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFTREPARSER_H
#define NITFTREPARSER_H

#include <iostream>

#include "EnumWrapper.h"

class DynamicObject;
class ossimNitfRegisteredTag;
class RasterDataDescriptor;
class RasterFileDescriptor;

namespace Nitf
{
  /**
   *  The state of a given TRE.
   *
   *  @see     TreParser::isTreValid()
   */
   enum TreStateEnum
   {
      VALID = 0,     /**< A TRE is valid if all fields are present (and the correct type) and
                          the values of each field are within their prescribed ranges. */
      SUSPECT = 1,   /**< A TRE is suspect if all of its fields are present (and the correct type)
                          but the values of one or more fields are outside their prescribed ranges. */
      INVALID = 2,   /**< A TRE is invalid if fields are missing. */
      UNTESTED = 3   /**< A TRE has not been tested. */
   };

   /**
    * @EnumWrapper Nitf::TreStateEnum.
    */
   typedef EnumWrapper<TreStateEnum> TreState;

   /**
    * The status of a TRE export.
    *
    * @see TreParser::exportMetadata()
    */
   enum TreExportStatusEnum
   {
      REMOVE,     /**< Remove any existing TRE of the name without adding any generated ones. */
      REPLACE,    /**< Remove any existing TRE of the name while adding the one populated by
                       TreParser::exportMetadata(). */
      UNCHANGED   /**< Let any TREs of the same name be exported without modifications. */
   };

   /**
    * @EnumWrapper Nitf::TreExportStatusEnum.
    */
   typedef EnumWrapper<TreExportStatusEnum> TreExportStatus;

   /**
    * A plug-in interface use to read and write TREs while importing or exporting a NITF file.
    *
    * In order for the NITF importer and exporter to find your plug-in, you should insure that the
    * plug-in name is the same 6-character string as the CETAG as the TRE.
    *
    * Implementors of this interface can use an existing OSSIM TRE plug-in by implementing 
    * toDynamicObject(ossimNitfRegisteredTag&, DynamicObject&) const and 
    * fromDynamicObject(const DynamicObject&, ossimNitfRegisteredTag&) const.  The ossimNitfRegisteredTag
    * object will be that registered with OSSIM.
    *
    * Implementors can also simply perform stream operations for TRE import and export.  In that case,
    * implement toDynamicObject(std::istream&, size_t, DynamicObject&) const and
    * fromDynamicObject(const DynamicObject&, std::ostream&, size_t&) const.
    *
    */
   class TreParser
   {
   public:
      /**
       * The type that should be returned from PlugIn::getType()
       * for TRE parser plug-ins.
       *
       * @return Returns the type used for TRE parser plug-ins.
       */
      static std::string Type()
      {
         return "NITF Tagged Record Extension Parser";
      }

      /**
       * The type that should be returned from PlugIn::getSubtype()
       * for TRE parser plug-ins which do not implement TreParser::exportMetadata().
       *
       * @return Returns the subtype used for TRE parser plug-ins which do not
       *         implement TreParser::exportMetadata().
       *
       * @see TreParser::exportMetadata()
       */
      static std::string NormalSubtype()
      {
         return "";
      }

      /**
       * The type that should be returned from PlugIn::getSubtype()
       * for TRE parser plug-ins which do implement TreParser::exportMetadata().
       *
       * @return Returns the subtype used for TRE parser plug-ins which do
       *         implement TreParser::exportMetadata().
       *
       * @see TreParser::exportMetadata()
       */
      static std::string CreateOnExportSubtype()
      {
         return "Create on export";
      }

      /**
       * Examine the TRE to determine if it is valid.
       *
       * @param tre
       *        The TRE to examine.
       * @param reporter
       *        Write to this ostream with information regarding a TreState::SUSPECT or TreState::INVALID
       *        TRE.
       *
       * @return The TreState for the given TRE.
       */
      virtual TreState isTreValid(const DynamicObject& tre, std::ostream& reporter) const = 0;

      /**
       * Load the TRE directly from an ossimNitfRegisteredTag.
       *
       * Implementors should implement either this function or toDynamicObject(std::istream&, size_t, DynamicObject&) const
       * to load the TRE.
       *
       * @param input
       *        The ossimNitfRegisteredTag to read from.
       * @param output
       *        The DynamicObject to write to.
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that imported TREs might be incomplete, missing, etc.
       *
       * @return True if the operation succeeded, false otherwise.
       */
      virtual bool toDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output, 
         std::string &errorMessage) const = 0;

      /**
       * Load the TRE from an istream.
       *
       * Implementors should implement either this function or toDynamicObject(ossimNitfRegisteredTag&, DynamicObject&) const
       * to load the TRE.  This function will only be called if the other returned false.
       *
       * @param input
       *        The istream to read from.  The data within does not include the CETAG or CEL fields.
       * @param numBytes
       *        The number of bytes to read.
       * @param output
       *        The DynamicObject to write to.
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that imported TREs might be incomplete, missing, etc.
       *
       * @return True if the operation succeeded, false otherwise.
       */
      virtual bool toDynamicObject(std::istream& input, size_t numBytes, DynamicObject& output,
         std::string &errorMessage) const = 0;

      /**
       * Write the TRE directly to an ossimNitfRegisteredTag.
       *
       * Implementors should implement either this function or fromDynamicObject(const DynamicObject&, std::ostream&, size_t&) const
       * to write the TRE.
       *
       * @param input
       *        The DynamicObject to read from.
       * @param tre
       *        The ossimNitfRegisteredTag to write to.
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that exported TREs might be incomplete, missing, etc.
       *
       * @return True if the operation succeeded, false otherwise.
       */
      virtual bool fromDynamicObject(const DynamicObject& input, ossimNitfRegisteredTag& tre, 
         std::string &errorMessage) const = 0;
 
      /**
       * Write the TRE directly to an ossimNitfRegisteredTag.
       *
       * Implementors should implement either this function or fromDynamicObject(const DynamicObject&, ossimNitfRegisteredTag&) const
       * to write the TRE.  This function will only be called if the other returned false.
       *
       * @param input
       *        The DynamicObject to read from.
       * @param output
       *        The ostream to write to.  Do not write the CETAG or CEL.
       * @param numBytesWritten
       *        The number of bytes written.
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that exported TREs might be incomplete, missing, etc.
       *
       * @return True if the operation succeeded, false otherwise.
       */
      virtual bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten, 
         std::string &errorMessage) const = 0;

      /**
       * Import contents of the TRE to specific fields within the descriptor.
       *
       * For example, a TRE might contain wavelength data which would be
       * appropriately represented in the \ref specialmetadata.
       *
       * @param tre
       *        The parsed TRE to place copies of data within the descriptor.
       * @param descriptor
       *        The descriptor to place copies of data within.
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that imported TREs might be incomplete, missing, etc.
       *
       * @return \c true if the operation successfully populated the descriptor,
       *         \c false otherwise.  This method should return \c false if there
       *         is no need to import metadata into the descriptor.
       */
      virtual bool importMetadata(const DynamicObject &tre,
         RasterDataDescriptor &descriptor, std::string &errorMessage) const = 0;

      /**
       * Export contents descriptor into a tre.
       *
       * For example, a TRE might contain wavelength data which would be
       * appropriately represented in the \ref specialmetadata.
       *
       * This method can also be used to modify or create new TREs on export.
       * This might be desired if the TRE must be modified when chipping,
       * or to create a TRE for a dataset not initially loaded as a NITF.
       *
       * @param descriptor
       *        The data descriptor to export from.
       * @param exportDescriptor
       *        The descriptor which specifies which portion is to be exported.
       * @param tre
       *        On return, this should contain the TRE to export with a
       *        future call to fromDynamicObject().
       * @param ownerIndex
       *        When returning TreExportStatus::REPLACE, specifies the index of the owner of this tag.
       *        This should be set to 0 for the file header and 1 for the image subheader.
       *        If this value is unspecified, it will default to the image subheader.
       * @param tagType
       *        When returning TreExportStatus::REPLACE, specifies the type of this tag.
       *        Valid values are "UDHD" "UDID", "XHD", and "IXSHD".
       *        If this value is unspecified, it will default to "IXSHD".
       * @param errorMessage
       *        If this is modified by the function, it will be displayed to the
       *        user as a warning that exported TREs might be incomplete, missing, etc.
       *
       * @return A TreExportStatus which indicates what to do with existing and created
       *         TREs that this parser handles.
       *
       * @see CreateOnExportSubtype()
       */
      virtual TreExportStatus exportMetadata(const RasterDataDescriptor &descriptor, 
         const RasterFileDescriptor &exportDescriptor, 
         DynamicObject &tre, unsigned int &ownerIndex, std::string &tagType, std::string &errorMessage) const = 0;
   };
}

#endif
