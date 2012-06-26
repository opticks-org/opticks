/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOITOOLBARADAPTER_H
#define AOITOOLBARADAPTER_H

#include "AoiToolBar.h"
#include "AoiToolBarImp.h"

class AoiToolBarAdapter : public AoiToolBar, public AoiToolBarImp AOITOOLBARADAPTEREXTENSION_CLASSES
{
public:
   AoiToolBarAdapter(const std::string& id, QWidget* pParent = NULL);
   virtual ~AoiToolBarAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   AOITOOLBARADAPTER_METHODS(AoiToolBarImp)

private:
   AoiToolBarAdapter(const AoiToolBarAdapter& rhs);
   AoiToolBarAdapter& operator=(const AoiToolBarAdapter& rhs);
};

#endif
