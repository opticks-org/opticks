/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEUTILITIES_H__
#define GEOREFERENCEUTILITIES_H__

#include "LocationType.h"
#include <vector>

#define COEFFS_FOR_ORDER(order) (((order) + 1) * ((order) + 2) / 2)

namespace GeoreferenceUtilities
{
void basisFunction(const LocationType& pixelCoord, double* pBasisValues, int numBasisValues);

bool computeFit(const std::vector<LocationType>& points,
   const std::vector<LocationType>& values, int which,
   std::vector<double>& coefficients);

double computePolynomial(LocationType pixel, int order, std::vector<double>& coeffs);

LocationType evaluatePolynomial(LocationType position,
                                const std::vector<double>& pXCoeffs,
                                const std::vector<double>& pYCoeffs,
                                int order);
}

#endif