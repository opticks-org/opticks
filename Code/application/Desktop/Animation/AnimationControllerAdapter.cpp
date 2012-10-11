/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationControllerAdapter.h"

using namespace std;

AnimationControllerAdapter::AnimationControllerAdapter(FrameType frameType, const string& id) :
   AnimationControllerImp(frameType, id)
{
}

AnimationControllerAdapter::~AnimationControllerAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& AnimationControllerAdapter::getObjectType() const
{
   static string type("AnimationControllerAdapter");
   return type;
}

bool AnimationControllerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationController"))
   {
      return true;
   }

   return AnimationControllerImp::isKindOf(className);
}
