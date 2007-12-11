/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATION_TOOLBAR_ADAPTER_H
#define ANIMATION_TOOLBAR_ADAPTER_H

#include "AnimationToolBar.h"
#include "AnimationToolBarImp.h"

class AnimationToolBarAdapter : public AnimationToolBar, public AnimationToolBarImp
{
public:
   AnimationToolBarAdapter(const std::string& id, QWidget* parent = 0);
   ~AnimationToolBarAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ANIMATIONTOOLBARADAPTER_METHODS(AnimationToolBarImp)
};

#endif
