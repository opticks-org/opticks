/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFSENSRBPARSER_H__
#define NITFSENSRBPARSER_H__

#include "NitfTreParserShell.h"

namespace Nitf
{
   class SensrbParser : public TreParserShell
   {
   public:
      SensrbParser();

      virtual bool toDynamicObject(std::istream& input, size_t numBytes, DynamicObject& output, 
         std::string &errorMessage) const;
      virtual bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten, 
         std::string &errorMessage) const;

      virtual TreState isTreValid(const DynamicObject& tre, std::ostream& reporter) const;

      // inherited from Testable
      virtual bool runAllTests(Progress* pProgress, std::ostream& failure);
   };
}

#endif
