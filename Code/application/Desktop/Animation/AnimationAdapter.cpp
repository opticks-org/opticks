/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationAdapter.h"

using namespace std;

AnimationAdapter::AnimationAdapter(FrameType frameType, const string& id) :
   AnimationImp(frameType, id)
{
}

AnimationAdapter::~AnimationAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& AnimationAdapter::getObjectType() const
{
   static string type("AnimationAdapter");
   return type;
}

bool AnimationAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Animation"))
   {
      return true;
   }

   return AnimationImp::isKindOf(className);
}
