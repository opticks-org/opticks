/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BANDBINNINGUTILITIES_H
#define BANDBINNINGUTILITIES_H

#include "DimensionDescriptor.h"

#include <string>
#include <utility>
#include <vector>

class Filename;
class RasterDataDescriptor;

namespace BandBinningUtilities
{
   bool preprocessGroupedBands(std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands);

   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > readFile(const Filename* pFilename,
      const RasterDataDescriptor* pDescriptor);

   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > readFile(const std::string& filename,
      const RasterDataDescriptor* pDescriptor);

   bool writeFile(const std::string& filename,
      const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands);
};

#endif
