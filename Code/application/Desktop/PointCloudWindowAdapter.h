/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDWINDOWADAPTER_H
#define POINTCLOUDWINDOWADAPTER_H

#include "PointCloudWindow.h"
#include "PointCloudWindowImp.h"

class PointCloudWindowAdapter : public PointCloudWindow, public PointCloudWindowImp POINTCLOUDWINDOWADAPTEREXTENSION_CLASSES
{
public:
   PointCloudWindowAdapter(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~PointCloudWindowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POINTCLOUDWINDOWADAPTER_METHODS(PointCloudWindowImp)
};


#endif