/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITF_AIMIDB_PARSER_H
#define NITF_AIMIDB_PARSER_H

#include "NitfTreParserShell.h"

namespace Nitf
{
   class AimidbParser : public TreParserShell
   {
   public:
      AimidbParser();

      bool toDynamicObject(std::istream& input, size_t numBytes, DynamicObject& output, 
         std::string &errorMessage) const;
      bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten, 
         std::string &errorMessage) const;

      virtual TreState isTreValid(const DynamicObject& tre, std::ostream& reporter) const;

      // inherited from Testable
      bool runAllTests(Progress* pProgress, std::ostream& failure);
   };
}

#endif
