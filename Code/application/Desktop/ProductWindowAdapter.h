/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRODUCTWINDOWADAPTER_H
#define PRODUCTWINDOWADAPTER_H

#include "ProductWindow.h"
#include "ProductWindowImp.h"

class ProductWindowAdapter : public ProductWindow, public ProductWindowImp PRODUCTWINDOWADAPTEREXTENSION_CLASSES
{
public:
   ProductWindowAdapter(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~ProductWindowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PRODUCTWINDOWADAPTER_METHODS(ProductWindowImp)
};

#endif
