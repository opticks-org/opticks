/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONADAPTER_H
#define ANIMATIONADAPTER_H

#include "Animation.h"
#include "AnimationImp.h"

class AnimationAdapter : public Animation, public AnimationImp
{
public:
   AnimationAdapter(FrameType frameType, const std::string& id);
   ~AnimationAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ANIMATIONADAPTER_METHODS(AnimationImp)
};

#endif
