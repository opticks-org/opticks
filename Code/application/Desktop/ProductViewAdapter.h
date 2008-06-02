/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRODUCTVIEWADAPTER_H
#define PRODUCTVIEWADAPTER_H

#include "ProductView.h"
#include "ProductViewImp.h"

class ProductViewAdapter : public ProductView, public ProductViewImp PRODUCTVIEWADAPTEREXTENSION_CLASSES
{
public:
   ProductViewAdapter(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~ProductViewAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PRODUCTVIEWADAPTER_METHODS(ProductViewImp)
};

#endif
