/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDVIEWADAPTER_H
#define POINTCLOUDVIEWADAPTER_H

#include "PointCloudView.h"
#include "PointCloudViewImp.h"

class PointCloudViewAdapter : public PointCloudView, public PointCloudViewImp POINTCLOUDVIEWADAPTEREXTENSION_CLASSES
{
public:
   PointCloudViewAdapter(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~PointCloudViewAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POINTCLOUDVIEWADAPTER_METHODS(PointCloudViewImp)
};

#endif