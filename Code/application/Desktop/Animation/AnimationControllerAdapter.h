/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONCONTROLLERADAPTER_H
#define ANIMATIONCONTROLLERADAPTER_H

#include "AnimationController.h"
#include "AnimationControllerImp.h"

class AnimationControllerAdapter : public AnimationController, public AnimationControllerImp ANIMATIONCONTROLLERADAPTEREXTENSION_CLASSES
{
public:
   AnimationControllerAdapter(FrameType frameType, const std::string& id);
   ~AnimationControllerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ANIMATIONCONTROLLERADAPTER_METHODS(AnimationControllerImp)
};

#endif
