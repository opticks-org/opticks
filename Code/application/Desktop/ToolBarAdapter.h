/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TOOLBARADAPTER_H
#define TOOLBARADAPTER_H

#include "ToolBar.h"
#include "ToolBarImp.h"

class ToolBarAdapter : public ToolBar, public ToolBarImp
{
public:
   ToolBarAdapter(const std::string& id, const std::string& name, QWidget* parent = 0);
   ~ToolBarAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   TOOLBARADAPTER_METHODS(ToolBarImp)
};

#endif
