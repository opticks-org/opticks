/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAVARIANTVALIDATOR_H
#define DATAVARIANTVALIDATOR_H

/**
 *  \cond INTERNAL
 *  A validator for types used to initialize a DataVariant. All supported types
 *  are specialized. A VariantTypeValidator is created in the DataVariant 
 *  constructor. If an unsupported type is used, the default template will be
 *  instantiated giving a compile error. Supported types will instantiate the
 *  appropriate specialization rather than the default template.
 */
template <class T>
class VariantTypeValidator
{
   class UnsupportedDataVariantType;
   static const int i = sizeof(UnsupportedDataVariantType); // can only get here on an unsupported type
};
/// \endcond

#endif