/*
 * The information in this file is
 * Copyright(c) 2014 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LASIMPORTER_H__
#define LASIMPORTER_H__

#include "ImporterShell.h"
#include "OptionLasImporter.h"
#include <boost/atomic.hpp>

namespace liblas
{
   class Header;
   class Reader;
}

class PointCloudDataDescriptor;
class PointCloudElement;
class ProgressTracker;

class LasImporter : public ImporterShell
{
public:
   LasImporter();
   virtual ~LasImporter();

   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual bool isProcessingLocationSupported(ProcessingLocation location) const;
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);
   virtual void polishDataDescriptor (DataDescriptor *pDescriptor);
   virtual bool validate(const DataDescriptor* pDescriptor,
      const std::vector<const DataDescriptor*>& importedDescriptors, std::string& errorMessage) const;
   virtual QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);

   enum ThinningMethod
   {
      THIN_NONE,
      THIN_MAX_POINTS
   };

protected:
   int maxPointsThinning(const unsigned int maxPoints,
                         const PointCloudDataDescriptor* pDesc,
                         liblas::Reader& reader,
                         liblas::Header const& header,
                         PointCloudElement* pElement,
                         ProgressTracker& progress,
                         bool* pAborted);
   PointCloudDataDescriptor* generatePointCloudDataDescriptor(const std::string& name, DataElement* pParent,
                                                              InterleaveFormatType interleave, EncodingType encoding,
                                                              EncodingType intensityEncoding, EncodingType classEncoding,
                                                              ProcessingLocation location);

private:
   OptionLasImporter mCustomOptions;

   unsigned int mTotalPoints;
   double mMinX;
   double mMinY;
   double mMinZ;
   double mMaxX;
   double mMaxY;
   double mMaxZ;
   double mScaleX;
   double mScaleY;
   double mScaleZ;
   double mOffsetX;
   double mOffsetY;
   double mOffsetZ;

   boost::atomic<bool> mPolishEntered; // used to prevent infinite recursion in polishDataDescriptor
};

#endif
