/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "AppVerify.h"
#include "BandBinningUtilities.h"
#include "Filename.h"
#include "RasterDataDescriptor.h"

#include <algorithm>

namespace BandBinningUtilities
{
   // Predicate to be used with std::remove_if.
   class BandChecker
   {
   public:
      BandChecker()
      {}

      bool operator()(std::pair<DimensionDescriptor, DimensionDescriptor> bandGroup)
      {
         return bandGroup.first.isValid() == false || bandGroup.second.isValid() == false;
      }
   };

   bool preprocessGroupedBands(std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands)
   {
      bool modified = false;

      // Remove bands which are not valid.
      std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >::iterator iter =
         std::remove_if(groupedBands.begin(), groupedBands.end(), BandChecker());
      if (iter != groupedBands.end())
      {
         groupedBands.erase(iter, groupedBands.end());
         modified = true;
      }

      // Swap first and last bands if necessary.
      // This is an implementation detail for DataAccessorImpl since it requires start and stop bands to be ordered.
      // It also provides a nice way to display the grouped bands to the user.
      // This is performed after the removal of invalid bands so that invalid bands are not checked.
      for (std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >::iterator iter = groupedBands.begin();
         iter != groupedBands.end();
         ++iter)
      {
         if (iter->first > iter->second)
         {
            std::swap(iter->first, iter->second);
            modified = true;
         }
      }

      return modified;
   }

   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > readFile(const Filename* pFilename,
      const RasterDataDescriptor* pDescriptor)
   {
      return pFilename == NULL ? std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >() :
         readFile(pFilename->getFullPathAndName(), pDescriptor);
   }

   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > readFile(const std::string& filename,
      const RasterDataDescriptor* pDescriptor)
   {
      std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > groupedBands;
      VERIFYRV(pDescriptor != NULL, groupedBands);

      QRegExp whitespace("\\s+");
      QFile file(QString::fromStdString(filename));
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      QTextStream stream(&file);
      for (QString line = stream.readLine(); line.isNull() == false; line = stream.readLine())
      {
         if (line.isEmpty())
         {
            continue;
         }

         QStringList parts = line.split(whitespace, QString::SkipEmptyParts);
         if (parts.size() != 2)
         {
            continue;
         }

         bool success;
         unsigned int first = parts[0].toUInt(&success);
         if (success == false)
         {
            continue;
         }
         DimensionDescriptor firstBand = pDescriptor->getOriginalBand(first);
         if (firstBand.isValid() == false)
         {
            continue;
         }

         unsigned int last = parts[1].toUInt(&success);
         if (success == false)
         {
            continue;
         }
         DimensionDescriptor lastBand = pDescriptor->getOriginalBand(last);
         if (lastBand.isValid() == false)
         {
            continue;
         }

         groupedBands.push_back(std::make_pair(firstBand, lastBand));
      }

      file.close();
      preprocessGroupedBands(groupedBands);  // Ignore return value.
      return groupedBands;
   }

   bool writeFile(const std::string& filename,
      const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands)
   {
      QFile file(QString::fromStdString(filename));
      if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
      {
         return false;
      }

      QTextStream stream(&file);
      for (
         std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >::const_iterator iter = groupedBands.begin();
         iter != groupedBands.end();
         ++iter)
      {
         if (iter->first.isOriginalNumberValid() && iter->second.isOriginalNumberValid())
         {
            stream << iter->first.getOriginalNumber() << " " << iter->second.getOriginalNumber() << endl;
         }
      }

      file.close();
      return true;
   }
}
