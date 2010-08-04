/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTICKSCALLBACKS_H
#define OPTICKSCALLBACKS_H

#include "OpticksMethods.h"
#include "XmlRpcCallback.h"
#include "XmlRpcParam.h"
#include <boost/any.hpp>
#include <QtCore/QBuffer>
#include <QtCore/QObject>

class GraphicLayer;
class GraphicObject;
class SpatialDataView;
class XmlRpcStructParam;

class XmlRpcAnnotationCallback : public XmlRpcCallback
{
public:
   XmlRpcAnnotationCallback(LayerType layerType, const QString& url, const QString& method,
      const XmlRpcStructParam& params, OpticksXmlRpcMethods::RegisterCallback& callbackRegistrar,
      QObject* pParent = NULL);
   virtual ~XmlRpcAnnotationCallback();

   virtual void processModified(Subject& subject, const std::string& signal, const boost::any& val);

   static bool getGraphicLayerAndObject(LayerType layerType, const QString& name, GraphicLayer*& pLayer,
      GraphicObject*& pObject, SpatialDataView* pView, bool createNewObject = false,
      GraphicObjectType newObjectType = TEXT_OBJECT);

private:
   QString mObjectName;
   GraphicLayer* mpLayer;
   GraphicObject* mpObject;
   OpticksXmlRpcMethods::RegisterCallback& mRegistrar;
};

#endif
