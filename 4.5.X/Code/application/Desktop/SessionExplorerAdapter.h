/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONEXPLORERADAPTER_H
#define SESSIONEXPLORERADAPTER_H

#include "SessionExplorer.h"
#include "SessionExplorerImp.h"

class SessionExplorerAdapter : public SessionExplorer, public SessionExplorerImp SESSIONEXPLORERADAPTEREXTENSION_CLASSES
{
public:
   SessionExplorerAdapter(const std::string& id, QWidget* pParent = 0);
   ~SessionExplorerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SESSIONEXPLORERADAPTER_METHODS(SessionExplorerImp)
};

#endif
