/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDWINDOWIMP_H
#define POINTCLOUDWINDOWIMP_H

#include "WorkspaceWindowImp.h"

class PointCloudView;

class PointCloudWindowImp : public WorkspaceWindowImp
{
   Q_OBJECT

public:
   PointCloudWindowImp(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~PointCloudWindowImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WindowType getWindowType() const;
   using WorkspaceWindowImp::setName;

   void setWidget(QWidget* pWidget);

   PointCloudView* getPointCloudView() const;

};

#define POINTCLOUDWINDOWADAPTEREXTENSION_CLASSES \
   WORKSPACEWINDOWADAPTEREXTENSION_CLASSES

#define POINTCLOUDWINDOWADAPTER_METHODS(impClass) \
   WORKSPACEWINDOWADAPTER_METHODS(impClass) \
   PointCloudView* getPointCloudView() const \
   { \
      return impClass::getPointCloudView(); \
   } \

#endif