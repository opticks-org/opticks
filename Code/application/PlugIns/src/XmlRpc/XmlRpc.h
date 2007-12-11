/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLRPC_H
#define XMLRPC_H

#include "XmlRpcParam.h"
#include <QtCore/QString>
#include <QtCore/QVariant>

/**
 * This file is based on the XML-RPC spec as of June 30, 2003.
 * See http://www.xmlrpc.com/spec for more information.
 */

class XmlRpcArrayParam;

/**
 * This is the implementation of a specific XML-RPC method call.
 *
 * @see XmlRpcMethodCall
 */
class XmlRpcMethodCallImp
{
public:
   XmlRpcMethodCallImp() {}
   XmlRpcMethodCallImp(const XmlRpcMethodCallImp &other) {}
   virtual ~XmlRpcMethodCallImp() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params) { return new XmlRpcParam; }
   virtual QString getHelp() { return ""; }
   virtual XmlRpcArrayParam *getSignature() { return NULL; }
};

/**
 * This contains the information in an XML-RPC method call.
 * It is a C++ object representation of the XML for the call.
 * This contains only the method call name and parameters compared
 * to XmlRpcMethodCallImp which is the implementation of a specific,
 * named method call.
 *
 * @see XmlRpcMethodCallImp
 */
class XmlRpcMethodCall
{
public:
   XmlRpcMethodCall(const QString &methodName, const XmlRpcParams &params) : mMethodName(methodName), mParams(params) {}
   explicit XmlRpcMethodCall(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *pRoot);

   QString getMethodName() const { return mMethodName; }
   const XmlRpcParams &getParams() const { return mParams; }

   QString toXml() const;

private:
   QString mMethodName;
   XmlRpcParams mParams;
};

class XmlRpcMethodResponse
{
public:
   explicit XmlRpcMethodResponse(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *pRoot);
   explicit XmlRpcMethodResponse(const XmlRpcParam *pReturnValue);
   ~XmlRpcMethodResponse() { delete mpReturnValue; }

   QString toXml() const;
   const XmlRpcParam *returnValue() { return mpReturnValue; }

private:
   const XmlRpcParam *mpReturnValue;
};

class XmlRpcMethodFault
{
public:
   XmlRpcMethodFault(unsigned int faultCode, const QString &otherInformation = QString());

   QString toXml() const;

   static void populateFaults();
   static QMap<unsigned int, QString> sFaults;

private:
   unsigned int mFaultCode;
   QString mOtherInformation;
};

#endif
