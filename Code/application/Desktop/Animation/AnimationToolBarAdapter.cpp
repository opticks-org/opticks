/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationToolBarAdapter.h"

using namespace std;

AnimationToolBarAdapter::AnimationToolBarAdapter(const string& id, QWidget* parent) :
   AnimationToolBarImp(id, parent)
{
}

AnimationToolBarAdapter::~AnimationToolBarAdapter()
{
   SubjectImp::notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& AnimationToolBarAdapter::getObjectType() const
{
   static string type("AnimationToolBarAdapter");
   return type;
}

bool AnimationToolBarAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationToolBar"))
   {
      return true;
   }

   return AnimationToolBarImp::isKindOf(className);
}
