/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFMTIMSAPARSER_H
#define NITFMTIMSAPARSER_H

#include "NitfTreParserShell.h"

namespace Nitf
{
   class MtimsaParser : public TreParserShell
   {
   public:
      MtimsaParser();

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

