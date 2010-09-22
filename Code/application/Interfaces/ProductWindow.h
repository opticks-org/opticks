/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PRODUCTWINDOW_H
#define PRODUCTWINDOW_H

#include "WorkspaceWindow.h"

class ProductView;

/**
 *  A window containing a product view.
 *
 *  The product window is a type of workspace window that contains a product view.
 *  The class is provided for convenience when creating and using product views.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in WorkspaceWindow.
 *
 *  @see     WorkspaceWindow, ProductView
 */
class ProductWindow : public WorkspaceWindow
{
public:
   /**
    *  Returns the product view contained in the window.
    *
    *  @return  A pointer to the product view displayed in the window.
    *
    *  @see     ProductView
    */
   virtual ProductView* getProductView() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~ProductWindow() {}
};

#endif
