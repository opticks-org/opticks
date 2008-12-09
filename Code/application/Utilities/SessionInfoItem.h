/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONINFOITEM_H
#define SESSIONINFOITEM_H

#include "SettableSessionItemAdapter.h"
#include "SubjectImp.h"
#include "xmlwriter.h"

#include "XercesIncludes.h"

#include <string>

class SessionItemDeserializer;
class SessionItemSerializer;

class SessionInfoItem : public SettableSessionItemAdapter
{
public:
   SessionInfoItem(const std::string& id, const std::string& name);
   ~SessionInfoItem();

   const std::string& getObjectType() const;
   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

};

#endif