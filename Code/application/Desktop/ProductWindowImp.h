/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRODUCTWINDOWIMP_H
#define PRODUCTWINDOWIMP_H

#include "WorkspaceWindowImp.h"
#include "TypesFile.h"

class ProductView;

class ProductWindowImp : public WorkspaceWindowImp
{
public:
   ProductWindowImp(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~ProductWindowImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WindowType getWindowType() const;
   using WorkspaceWindowImp::setName;

   View* createView(const QString& strViewName, const ViewType& viewType);
   void setWidget(QWidget* pWidget);
   void print(bool bSetupDialog = false);

   ProductView* getProductView() const;
};

#define PRODUCTWINDOWADAPTEREXTENSION_CLASSES \
   WORKSPACEWINDOWADAPTEREXTENSION_CLASSES

#define PRODUCTWINDOWADAPTER_METHODS(impClass) \
   WORKSPACEWINDOWADAPTER_METHODS(impClass) \
   ProductView* getProductView() const \
   { \
      return impClass::getProductView(); \
   }

#endif
