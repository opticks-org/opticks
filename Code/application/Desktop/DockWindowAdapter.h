/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DOCKWINDOWADAPTER_H
#define DOCKWINDOWADAPTER_H

#include "DockWindow.h"
#include "DockWindowImp.h"

class DockWindowAdapter : public DockWindow, public DockWindowImp DOCKWINDOWADAPTEREXTENSION_CLASSES
{
public:
   DockWindowAdapter(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~DockWindowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   DOCKWINDOWADAPTER_METHODS(DockWindowImp)

private:
   DockWindowAdapter(const DockWindowAdapter& rhs);
   DockWindowAdapter& operator=(const DockWindowAdapter& rhs);
};

#endif
