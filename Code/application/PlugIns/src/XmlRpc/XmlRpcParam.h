/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLRPCPARAM_H
#define XMLRPCPARAM_H

#include "xmlwriter.h"
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#define INT_PARAM "int"
#define BOOLEAN_PARAM "boolean"
#define STRING_PARAM "string"
#define DOUBLE_PARAM "double"
#define DATE_PARAM "dateTime.iso8601"
#define BASE64_PARAM "base64"

class XmlRpcParam
{
public:
   XmlRpcParam() {}
   XmlRpcParam(const QString& type, const QVariant& value = QVariant()) :
      mType(type),
      mValue(value)
   {
   }

   XmlRpcParam(const XmlRpcParam& other) :
      mType(other.mType),
      mValue(other.mValue)
   {
   }

   virtual ~XmlRpcParam() {};

   const QString& type() const
   {
      return mType;
   }

   const QVariant& value() const
   {
      return mValue;
   }

   virtual bool isValid() const
   {
      return !mType.isEmpty() && mValue.isValid();
   }

   virtual bool toXml(XMLWriter& xml) const
   {
      xml.addText(mValue.toString().toAscii().constData(), xml.addElement(mType.toStdString()));
      return true;
   }

private:
   QString mType;
   QVariant mValue;
};

typedef QVector<const XmlRpcParam*> XmlRpcParams;

#endif
