/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLRPCCALLBACK_H
#define XMLRPCCALLBACK_H

#include "XmlRpcParam.h"

#include <QtCore/QBuffer>
#include <QtCore/QObject>
#include <QtCore/QString>

class GraphicLayer;
class GraphicObject;
class QHttp;
class QHttpResponseHeader;
class SpatialDataView;
class XmlRpcStructParam;

class XmlRpcCallback : public QObject
{
   Q_OBJECT

public:
   virtual ~XmlRpcCallback();

protected:
   XmlRpcCallback(const QString& url, const QString& method, const XmlRpcStructParam& params, QObject* pParent = NULL);
   void call(const XmlRpcParams& params) const;
   virtual void callComplete() {}

protected slots:
   void processRequestFinished(int id, bool error);
   void processResponseHeader(const QHttpResponseHeader& header);

private:
   QString mUrl;
   QString mMethod;
   mutable QHttp* mpHttpConnection;
};

#endif
