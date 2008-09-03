/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITF_ACFTA_PARSER_H
#define NITF_ACFTA_PARSER_H

#include "NitfTreParserShell.h"

// NOTE: 19 June 2006 - Larry Beck (MLB), Technology Service Corporation:
// The JITC tells me that there are no production systems that produce the ACFTA TAG.
// They also say that there were multiple versions produced during development with no
// good ways to tell the difference. This parser is coded to the most current version
// of the STDI-0002 (ver 2.1 16 Nov 2000)

namespace Nitf
{
   class AcftaParser : public TreParserShell
   {
   public:
      AcftaParser();

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
