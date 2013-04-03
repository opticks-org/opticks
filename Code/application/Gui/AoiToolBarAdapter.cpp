/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiToolBarAdapter.h"

AoiToolBarAdapter::AoiToolBarAdapter(const std::string& id, QWidget* pParent) :
   AoiToolBarImp(id, pParent)
{}

AoiToolBarAdapter::~AoiToolBarAdapter()
{
   SubjectImp::notify(SIGNAL_NAME(Subject, Deleted));
}

const std::string& AoiToolBarAdapter::getObjectType() const
{
   static std::string sType("AoiToolBarAdapter");
   return sType;
}

bool AoiToolBarAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "AoiToolBar"))
   {
      return true;
   }

   return AoiToolBarImp::isKindOf(className);
}
