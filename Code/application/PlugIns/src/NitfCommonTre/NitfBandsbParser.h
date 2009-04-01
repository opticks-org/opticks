/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFBANDSBPARSER_H
#define NITFBANDSBPARSER_H

#include "NitfTreParserShell.h"

#include <vector>

namespace Nitf
{
   class BandsbParser : public TreParserShell
   {
   public:
      BandsbParser();

      bool toDynamicObject(std::istream& input, size_t numBytes, DynamicObject& output,
         std::string &errorMessage) const;
      bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten,
         std::string &errorMessage) const;

      TreExportStatus exportMetadata(const RasterDataDescriptor &descriptor, 
         const RasterFileDescriptor &exportDescriptor, DynamicObject &tre, 
         unsigned int & ownerIndex, std::string & tagType, std::string &errorMessage) const;

      virtual TreState isTreValid(const DynamicObject& tre, std::ostream& reporter) const;

      bool runAllTests(Progress* pProgress, std::ostream& failure);

      bool importMetadata(const DynamicObject& tre, RasterDataDescriptor& descriptor, std::string& errorMessage) const;

   private:
      mutable std::vector<double> mCenterWavelengths;
      mutable std::vector<double> mStartWavelengths;
      mutable std::vector<double> mEndWavelengths;
      mutable std::vector<double> mFwhms;
      mutable bool mWavelengthsInInverseCentimeters;
   };
}

#endif
