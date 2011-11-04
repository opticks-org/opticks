/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALDATAWINDOWADAPTER_H
#define SPATIALDATAWINDOWADAPTER_H

#include "SpatialDataWindow.h"
#include "SpatialDataWindowImp.h"

class SpatialDataWindowAdapter : public SpatialDataWindow, public SpatialDataWindowImp SPATIALDATAWINDOWADAPTEREXTENSION_CLASSES
{
public:
   SpatialDataWindowAdapter(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~SpatialDataWindowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SPATIALDATAWINDOWADAPTER_METHODS(SpatialDataWindowImp)

private:
   SpatialDataWindowAdapter(const SpatialDataWindowAdapter& rhs);
   SpatialDataWindowAdapter& operator=(const SpatialDataWindowAdapter& rhs);
};

#endif
