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
#include "LocationType.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"

class GcpGui;

#include <vector>

#define COEFFS_FOR_ORDER(order) (((order) + 1) * ((order) + 2) / 2)
#define MAX_ORDER 6
#define INTERACTIVE_MAX_ORDER 3

class GcpGeoreference : public GeoreferenceShell
{
public:
   GcpGeoreference();
   ~GcpGeoreference();

   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool setInteractive();

   LocationType pixelToGeo(LocationType pixel) const;
   LocationType geoToPixel(LocationType geocoord) const;
   LocationType geoToPixelQuick(LocationType geo) const;
   bool canHandleRasterElement(RasterElement *pRaster) const;
   QWidget *getGui(RasterElement *pRaster);
   bool validateGuiInput() const;

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

protected:
   LocationType evaluatePolynomial(LocationType position, const double pXCoeffs[], 
      const double pYCoeffs[], int order) const;

   void computeAnchor(int corner);
   void setCubeSize(unsigned int numRows, unsigned int numColumns);

private:
   GcpGui* mpGui;

   RasterElement* mpRaster;
   int mOrder;
   unsigned short mReverseOrder;
   double mLatCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)];
   double mLonCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)];
   double mXCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)];
   double mYCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)];
   double computePolynomial(LocationType pixel, int order, std::vector<double> &coeffs);
   bool computeFit(const std::vector<LocationType> &points,
      const std::vector<LocationType> &values, int which, std::vector<double> &coefficients);
   void basisFunction(const LocationType& pixelCoord, double* pBasisValues, int numBasisValues);

   int mNumRows;
   int mNumColumns;

   Service<ModelServices> mpDataModel;
   Service<PlugInManagerServices> mpPlugInManager;
   Service<ApplicationServices> mpApplication;
   Service<UtilityServices> mpUtilities;

   Progress* mpProgress;
   std::string mMessageText;
};

#endif // GCPGEOREFERENCE_H
