/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef LOCATIONTYPE_H
#define LOCATIONTYPE_H

#include "DataVariantValidator.h"
#include "Location.h"

/**
 *  An X-Y pair.
 *  @deprecated This generic X-Y pair is deprecated in favor of more specific types
 *             such as PixelLocation and PixelOffset.
 */
typedef Opticks::Location<double,2> LocationType;

/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<LocationType> {};
/// \endcond

#endif
