/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "MessageLogAdapter.h"

using namespace std;

MessageLogAdapter::MessageLogAdapter(const char* name, const char* path, QFile *journal) :
   MessageLogImp(name, path, journal)
{}

MessageLogAdapter::~MessageLogAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& MessageLogAdapter::getObjectType() const
{
   static string sType("MessageLogAdapter");
   return sType;
}

bool MessageLogAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MessageLog"))
   {
      return true;
   }

   return MessageLogImp::isKindOf(className);
}


MessageAdapter::MessageAdapter(const string &action,
                               const string &component,
                               const string &key,
                               DateTime *timestamp,
                               Step *pParent) : MessageImp(action, component, key, timestamp, pParent)
{}

MessageAdapter::~MessageAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}


StepAdapter::StepAdapter(const string &action,
                         const string &component,
                         const string &key,
                         DateTime *timestamp,
                         Step *pParent) : StepImp(action, component, key, timestamp, pParent)
{}

StepAdapter::~StepAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}
