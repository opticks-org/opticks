/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARCPROXY_H__
#define ARCPROXY_H__

#include "ArcSDK.h"
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QTextStream>
#include <QtCore/QUuid>

namespace ArcProxyLib
{
   class ConnectionParameters;
}
class QString;
class QStringList;

/**
 *  This class is used to wrap the COM services provided by ArcEngine.
 *
 *  The ArcEngine COM classes define operator&() which requires a valid, initialized
 *  COM object or it will crash. When adding a COM "pointer" to a QMap, a default constructed
 *  object is created and returned as a reference. The default constructor creates
 *  an invalid COM object with will cause a crash when the operator&() is called. This
 *  wrapper class provides the operators necessary for the QMap but not an operator&()
 *  allowing storage of a default constructed (and invalid) COM object.
 */
template <typename T>
class ComWrap
{
public:
   ComWrap() {}
   ComWrap(T value) : mValue(value) {}
   ComWrap(const ComWrap &other) : mValue(other.mValue) {}
   ~ComWrap() {}
   ComWrap &operator=(const ComWrap &other)
   {
      mValue = other.mValue;
      return *this;
   }
   T &operator=(T &value)
   {
      mValue = value;
      return value;
   }
   operator T() { return mValue; }
   operator bool() { return mValue != 0; }
   T mValue;
};

class ArcProxy : public QObject
{
   Q_OBJECT

public:
   ArcProxy(QObject *pParent = NULL);
   virtual ~ArcProxy();

protected slots:
   void connectToServer();
   void processData();
   void serverExited();

protected:
   void initialize(const QString &data);
   void handleRequest(const QString &request, QStringList &args);
   void queryFeatureClass(IFeatureClassPtr pFeatures, QStringList &args);
   const IFeatureClassPtr getFeatureClass(const ArcProxyLib::ConnectionParameters &params, std::string &errorMessage);
   bool writeFeature(IFeaturePtr pFeature, const std::string &labelFormat);
   QString convertWhereClause(IFeatureClassPtr pFeatures, const QString &original);

private:
   ISpatialReferencePtr mpDestinationReference;
   enum NfaState { INITIALIZE, REQUEST } mState;
   QTextStream mStream;
   QMap<QUuid, ComWrap<IFeatureClassPtr> > mFeatureClasses;
   QString mPartialRequest;
};

#endif
