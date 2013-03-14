/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPGEOREFERENCE_H
#define GCPGEOREFERENCE_H

#include "ApplicationServices.h"
#include "GeoreferenceShell.h"
#include "GeoreferenceUtilities.h"
#include "LocationType.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"

class GcpGui;
class RasterElement;

#include <vector>

#define MAX_ORDER 6
#define INTERACTIVE_MAX_ORDER 3

class GcpGeoreference : public GeoreferenceShell
{
public:
   GcpGeoreference();
   virtual ~GcpGeoreference();

   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool getInputSpecification(PlugInArgList*& pArgList);

   unsigned char getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const;
   QWidget* getWidget(RasterDataDescriptor* pDescriptor);
   bool validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const;
   LocationType pixelToGeo(LocationType pixel, bool* pAccurate = NULL) const;
   LocationType geoToPixel(LocationType geocoord, bool* pAccurate = NULL) const;

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

protected:
   void computeAnchor(int corner);
   void setCubeSize(unsigned int numRows, unsigned int numColumns);

private:
   GcpGui* mpGui;

   RasterElement* mpRaster;
   int mOrder;
   unsigned short mReverseOrder;
   std::vector<double> mLatCoefficients;
   std::vector<double> mLonCoefficients;
   std::vector<double> mXCoefficients;
   std::vector<double> mYCoefficients;

   int mNumRows;
   int mNumColumns;

   Service<ModelServices> mpDataModel;
   Service<PlugInManagerServices> mpPlugInManager;
   Service<ApplicationServices> mpApplication;
   Service<UtilityServices> mpUtilities;

   Progress* mpProgress;
   std::string mMessageText;
};

#endif
