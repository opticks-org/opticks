/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFRPCPARSER_H
#define NITFRPCPARSER_H

#include "NitfTreParserShell.h"

namespace Nitf
{
   class RpcParser : public TreParserShell
   {
   public:
      RpcParser();

      bool toDynamicObject(std::istream&, size_t, DynamicObject&, 
         std::string &errorMessage) const;
      bool ossimTagToDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output, 
         std::string &errorMessage) const;
      bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten, 
         std::string &errorMessage) const;

      virtual TreState isTreValid(const DynamicObject& tre, std::ostream& reporter) const;

      // inherited from Testable
      bool runAllTests(Progress* pProgress, std::ostream& failure);
   };
}

#endif
