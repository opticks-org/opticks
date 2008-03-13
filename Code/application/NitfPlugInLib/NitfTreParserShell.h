/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRE_PARSER_SHELL_H
#define TRE_PARSER_SHELL_H

#include "PlugInShell.h"
#include "Testable.h"
#include "NitfTreParser.h"

namespace Nitf
{

   /**
    *  This class is a shell for a tre parer plug-in.
    *
    *  TreParser developers should take this class and extend it to support thier 
    *  importer specific code.
    */
   class TreParserShell : public PlugInShell, public Testable, public TreParser
   {
   public:
      TreParserShell();
      ~TreParserShell();

      // PlugIn
      /**
       *  @copydoc PlugIn::getShortDescription()
       *
       *  @default The default implementation returns a
       *           general description based on the results of getName().
       */
      std::string getShortDescription() const;

      // Testable
      /**
       * @copydoc Testable::runOperationalTests()
       *
       * @default The default implementation returns true,
       *          since most TreParser plug-ins will not
       *          require operational tests.
       */
      bool runOperationalTests(Progress* pProgress, std::ostream& failure);

      // TreParser
      /**
       * @copydoc TreParser::isTreValid()
       *
       * @default The default implementation returns TreState::UNTESTED.
       */
      TreState isTreValid(const DynamicObject& tre, std::ostream& reporter) const;

      /**
       * @copydoc TreParser::toDynamicObject(const ossimNitfRegisteredTag&, DynamicObject&) const
       *
       * @default The default implementation returns false,
       *          so that developers only need to override
       *          those methods they wish to implement.
       */
      bool toDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output, 
         std::string &errorMessage) const;

      /**
       * @copydoc TreParser::toDynamicObject(std::istream&, size_t, DynamicObject&) const
       *
       * @default The default implementation returns false,
       *          so that developers only need to override
       *          those methods they wish to implement.
       */
      bool toDynamicObject(std::istream& input, size_t numBytes, DynamicObject& output, 
         std::string &errorMessage) const;

      /**
       * @copydoc TreParser::fromDynamicObject(const DynamicObject&, ossimNitfRegisteredTag&) const
       *
       * @default The default implementation returns false,
       *          so that developers only need to override
       *          those methods they wish to implement.
       */
      bool fromDynamicObject(const DynamicObject& input, ossimNitfRegisteredTag& tre, 
         std::string &errorMessage) const;

      /**
       * @copydoc TreParser::fromDynamicObject(const DynamicObject&, std::ostream&, size_t&) const
       *
       * @default The default implementation returns false,
       *          so that developers only need to override
       *          those methods they wish to implement.
       */
      bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten, 
         std::string &errorMessage) const;

      /**
       * @copydoc TreParser::importMetadata(const DynamicObject &, RasterDataDescriptor &) const
       *
       * @default The default implementation returns false,
       *          so that developers only need to override
       *          those methods they wish to implement.
       */
      bool importMetadata(const DynamicObject &tre, RasterDataDescriptor &descriptor, 
         std::string &errorMessage) const;

      /**
       * @copydoc TreParser::exportMetadata(const RasterDataDescriptor &, DynamicObject &) const
       *
       * @default The default implementation returns #UNCHANGED,
       *          so that developers only need to override
       *          those methods they wish to implement. Note that
       *          if a subclass wishes to reimplement this function,
       *          the subclass should use CreateOnExportSubtype().
       */
      TreExportStatus exportMetadata(const RasterDataDescriptor &descriptor, 
         const RasterFileDescriptor &exportDescriptor, DynamicObject &tre,
         unsigned int &ownerIndex, std::string &tagType, std::string &errorMessage) const;
   };
}

#endif
