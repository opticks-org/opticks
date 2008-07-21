/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTROSPECTIONMETHODS_H
#define INTROSPECTIONMETHODS_H

#include "XmlRpc.h"
#include "XmlRpcParam.h"
#include "XmlRpcArrayParam.h"
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

class SystemListMethodsCallImp : public XmlRpcMethodCallImp
{
   const QMap<QString, XmlRpcMethodCallImp*> &mMethods;

public:
   SystemListMethodsCallImp(const QMap<QString, XmlRpcMethodCallImp*> &methods) : mMethods(methods) {}
   SystemListMethodsCallImp(const SystemListMethodsCallImp &other) : mMethods(other.mMethods) {}
   virtual ~SystemListMethodsCallImp() {}
   virtual XmlRpcArrayParam *operator()(const XmlRpcParams &params)
   {
      QStringList methodNames = mMethods.keys();
      XmlRpcArrayParam *pRval = new XmlRpcArrayParam;
      for(QStringList::const_iterator lit = methodNames.begin(); lit != methodNames.end(); ++lit)
      {
         *pRval << new XmlRpcParam(STRING_PARAM, *lit);
      }
      return pRval;
   }
   virtual QString getHelp() { return "Return a list of methods supported by this interface."; }
   virtual XmlRpcArrayParam *getSignature()
   {
      XmlRpcArrayParam *pParams = new XmlRpcArrayParam;
      *pParams << new XmlRpcParam(STRING_PARAM, "array");
      XmlRpcArrayParam *pSignatures = new XmlRpcArrayParam;
      *pSignatures << pParams;
      return pSignatures;
   }
};

class SystemMethodHelpCallImp : public XmlRpcMethodCallImp
{
   const QMap<QString, XmlRpcMethodCallImp*> &mMethods;

public:
   SystemMethodHelpCallImp(const QMap<QString, XmlRpcMethodCallImp*> &methods) : mMethods(methods) {}
   SystemMethodHelpCallImp(const SystemMethodHelpCallImp &other) : mMethods(other.mMethods) {}
   virtual ~SystemMethodHelpCallImp() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params)
   {
      if(params.size() != 1)
      {
         throw XmlRpcMethodFault(200);
      }
      const XmlRpcParam *pMethod = params[0];
      if((pMethod == NULL) || (pMethod->type() != STRING_PARAM))
      {
         throw XmlRpcMethodFault(200);
      }
      QString methodName = pMethod->value().toString();
      if(!mMethods.contains(methodName) || (mMethods[methodName] == NULL))
      {
         throw XmlRpcMethodFault(100, methodName);
      }
      return new XmlRpcParam(STRING_PARAM, mMethods[methodName]->getHelp());
   }
   virtual QString getHelp() { return "Return a help string for the requested method."; }
   virtual XmlRpcArrayParam *getSignature()
   {
      XmlRpcArrayParam *pParams = new XmlRpcArrayParam;
      *pParams << new XmlRpcParam(STRING_PARAM, "string");
      *pParams << new XmlRpcParam(STRING_PARAM, "string");
      XmlRpcArrayParam *pSignatures = new XmlRpcArrayParam;
      *pSignatures << pParams;
      return pSignatures;
   }
};

class SystemMethodSignatureCallImp : public XmlRpcMethodCallImp
{
   const QMap<QString, XmlRpcMethodCallImp*> &mMethods;

public:
   SystemMethodSignatureCallImp(const QMap<QString, XmlRpcMethodCallImp*> &methods) : mMethods(methods) {}
   SystemMethodSignatureCallImp(const SystemMethodSignatureCallImp &other) : mMethods(other.mMethods) {}
   virtual ~SystemMethodSignatureCallImp() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params)
   {
      if(params.size() != 1)
      {
         throw XmlRpcMethodFault(200);
      }
      const XmlRpcParam *pMethod = params[0];
      if((pMethod == NULL) || (pMethod->type() != STRING_PARAM))
      {
         throw XmlRpcMethodFault(200);
      }
      QString methodName = pMethod->value().toString();
      if(!mMethods.contains(methodName) || (mMethods[methodName] == NULL))
      {
         throw XmlRpcMethodFault(100, methodName);
      }
      XmlRpcParam *pSignatures = mMethods[methodName]->getSignature();
      if(pSignatures == NULL)
      {
         // makes sure something is returned, even if it is empty
         pSignatures = new XmlRpcArrayParam;
      }
      return pSignatures;
   }
   virtual QString getHelp() { return "Return the signatures for the requested method as an array.\n"
                                      "Each signature is an array of strings containing the types in that "
                                      "signature. The first type is the return type."; }
   virtual XmlRpcArrayParam *getSignature()
   {
      XmlRpcArrayParam *pParams = new XmlRpcArrayParam;
      *pParams << new XmlRpcParam(STRING_PARAM, "array");
      *pParams << new XmlRpcParam(STRING_PARAM, "string");
      XmlRpcArrayParam *pSignatures = new XmlRpcArrayParam;
      *pSignatures << pParams;
      return pSignatures;
   }
};

#endif
