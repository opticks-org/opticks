/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLRPCSTRUCTPARAM_H
#define XMLRPCSTRUCTPARAM_H

#include "XmlRpcParam.h"
#include "xmlwriter.h"
#include <QtCore/QMap>

class XmlRpcStructParam : public XmlRpcParam
{
public:
   typedef QMap<QString, const XmlRpcParam*> type;

   XmlRpcStructParam() :
      XmlRpcParam("struct") {}

   XmlRpcStructParam(const XmlRpcStructParam& other) :
      XmlRpcParam(other),
      mStruct(other.mStruct) {}

   ~XmlRpcStructParam()
   {
      for (QMap<QString, const XmlRpcParam*>::const_iterator it = mStruct.begin(); it != mStruct.end(); ++it)
      {
         delete it.value();
      }
   }

   XmlRpcStructParam &insert(const QString &name, const XmlRpcParam *pValue)
   {
      mStruct.insert(name, pValue);
      return *this;
   }

   const XmlRpcParam *operator[](const QString &key) const
   {
      return mStruct[key];
   }

   type::size_type size() const
   {
      return mStruct.size();
   }

   type::const_iterator begin() const
   {
      return mStruct.begin();
   }

   type::const_iterator end() const
   {
      return mStruct.end();
   }

   virtual bool isValid() const
   {
      return !mStruct.isEmpty();
   }

   virtual bool toXml(XMLWriter& xml) const
   {
      xml.pushAddPoint(xml.addElement("struct"));
      for (type::const_iterator it = mStruct.begin(); it != mStruct.end(); ++it)
      {
         xml.pushAddPoint(xml.addElement("member"));
         const QString& name = it.key();
         const XmlRpcParam* pParam = it.value();
         if (name.isEmpty() || (pParam == NULL))
         {
            xml.popAddPoint();
            continue;
         }
         xml.pushAddPoint(xml.addElement("name"));
         xml.addText(name.toStdString());
         xml.popAddPoint();
         xml.pushAddPoint(xml.addElement("value"));
         if (!pParam->toXml(xml))
         {
            return false;
         }
         xml.popAddPoint();
         xml.popAddPoint();
      }
      xml.popAddPoint();
      return true;
   }

private:
   type mStruct;
};

#endif
