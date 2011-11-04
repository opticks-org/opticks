/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLRPCARRAYPARAM_H
#define XMLRPCARRAYPARAM_H

#include "XmlRpcParam.h"
#include "xmlwriter.h"
#include <QtCore/QVector>

class XmlRpcArrayParam : public XmlRpcParam
{
public:
   XmlRpcArrayParam() : XmlRpcParam("array") {}
   XmlRpcArrayParam(const XmlRpcArrayParam &other) : XmlRpcParam(other), mArray(other.mArray) {}
   virtual ~XmlRpcArrayParam()
   {
      for (QVector<const XmlRpcParam*>::const_iterator it = mArray.begin(); it != mArray.end(); ++it)
      {
         delete *it;
      }
   }

   XmlRpcArrayParam& operator<<(const XmlRpcParam* pValue)
   {
      mArray.push_back(pValue);
      return *this;
   }

   const XmlRpcParam* operator[](QVector<const XmlRpcParam*>::size_type i) const
   {
      return mArray[i];
   }

   QVector<const XmlRpcParam*>::size_type size() const
   {
      return mArray.size();
   }

   virtual bool isValid() const
   {
      return !mArray.isEmpty();
   }

   virtual bool toXml(XMLWriter &xml) const
   {
      xml.pushAddPoint(xml.addElement("array"));
      xml.pushAddPoint(xml.addElement("data"));
      for (QVector<const XmlRpcParam*>::const_iterator it = mArray.begin(); it != mArray.end(); ++it)
      {
         xml.pushAddPoint(xml.addElement("value"));
         const XmlRpcParam* pParam = *it;
         if ((pParam == NULL) || !pParam->toXml(xml))
         {
            return false;
         }
         xml.popAddPoint();
      }
      xml.popAddPoint();
      xml.popAddPoint();
      return true;
   }

private:
   QVector<const XmlRpcParam*> mArray;
};

#endif
