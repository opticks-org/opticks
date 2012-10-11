/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTICKSMETHODS_H
#define OPTICKSMETHODS_H

#include "TypesFile.h"
#include "XmlRpc.h"
#include "XmlRpcParam.h"

#include <list>

class XmlRpcCallback;
class XmlRpcServer;

namespace OpticksXmlRpcMethods
{
static int XML_RPC_INTERFACE_VERSION;

class Version : public XmlRpcMethodCallImp
{
public:
   Version() {}
   Version(const Version &other) {}
   virtual ~Version() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

namespace Annotation
{
   class Create : public XmlRpcMethodCallImp
   {
   public:
      Create() {}
      Create(const Create &other) {}
      virtual ~Create() {}
      virtual XmlRpcParam *operator()(const XmlRpcParams &params);
      virtual QString getHelp();
      virtual XmlRpcArrayParam *getSignature();

   protected:
      virtual QString getNamespace() const
      {
         return "Annotation";
      }

      virtual LayerType getLayerType() const
      {
         return ANNOTATION;
      }
   };

   class Delete : public XmlRpcMethodCallImp
   {
   public:
      Delete() {}
      Delete(const Delete &other) {}
      virtual ~Delete() {}
      virtual XmlRpcParam *operator()(const XmlRpcParams &params);
      virtual QString getHelp();
      virtual XmlRpcArrayParam *getSignature();

   protected:
      virtual QString getNamespace() const
      {
         return "Annotation";
      }

      virtual LayerType getLayerType() const
      {
         return ANNOTATION;
      }
   };
};

namespace Aoi
{
   class Create : public Annotation::Create
   {
   public:
      Create() {}
      Create(const Create &other) {}
      virtual ~Create() {}

   protected:
      QString getNamespace() const
      {
         return "AOI";
      }

      LayerType getLayerType() const
      {
         return AOI_LAYER;
      }
   };

   class Delete : public Annotation::Delete
   {
   public:
      Delete() {}
      Delete(const Delete &other) {}
      virtual ~Delete() {}

   protected:
      QString getNamespace() const
      {
         return "AOI";
      }

      LayerType getLayerType() const
      {
         return AOI_LAYER;
      }
   };

   class DeleteLayer : public XmlRpcMethodCallImp
   {
   public:
      DeleteLayer() {}
      DeleteLayer(const DeleteLayer &other) {}
      virtual ~DeleteLayer() {}
      virtual XmlRpcParam *operator()(const XmlRpcParams &params);
      virtual QString getHelp();
      virtual XmlRpcArrayParam *getSignature();
   };

   class GetBoundingBox : public XmlRpcMethodCallImp
   {
   public:
      GetBoundingBox() {}
      GetBoundingBox(const GetBoundingBox &other) {}
      virtual ~GetBoundingBox() {}
      virtual XmlRpcParam *operator()(const XmlRpcParams &params);
      virtual QString getHelp();
      virtual XmlRpcArrayParam *getSignature();
   };

   class SetMode : public XmlRpcMethodCallImp
   {
   public:
      SetMode() {}
      SetMode(const SetMode &other) {}
      virtual ~SetMode() {}
      virtual XmlRpcParam *operator()(const XmlRpcParams &params);
      virtual QString getHelp();
      virtual XmlRpcArrayParam *getSignature();
   };
};

class Close : public XmlRpcMethodCallImp
{
public:
   Close() {}
   Close(const Close &other) {}
   virtual ~Close() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class CloseAll : public XmlRpcMethodCallImp
{
public:
   CloseAll() {}
   CloseAll(const CloseAll &other) {}
   virtual ~CloseAll() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class CreateView : public XmlRpcMethodCallImp
{
public:
   CreateView() {}
   CreateView(const CreateView &other) {}
   virtual ~CreateView() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class ExportElement: public XmlRpcMethodCallImp
{
public:
   ExportElement() {}
   ExportElement(const ExportElement &other) {}
   virtual ~ExportElement() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class GetMetadata : public XmlRpcMethodCallImp
{
public:
   GetMetadata() {}
   GetMetadata(const GetMetadata &other) {}
   virtual ~GetMetadata() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class GetViewInfo : public XmlRpcMethodCallImp
{
public:
   GetViewInfo() {}
   GetViewInfo(const GetViewInfo &other) {}
   virtual ~GetViewInfo() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class GetViews : public XmlRpcMethodCallImp
{
public:
   GetViews() {}
   GetViews(const GetViews &other) {}
   virtual ~GetViews() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class LinkViews : public XmlRpcMethodCallImp
{
public:
   LinkViews() {}
   LinkViews(const LinkViews &other) {}
   virtual ~LinkViews() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class Open : public XmlRpcMethodCallImp
{
public:
   Open() {}
   Open(const Open &other) {}
   virtual ~Open() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class PanBy : public XmlRpcMethodCallImp
{
public:
   PanBy() {}
   PanBy(const PanBy &other) {}
   virtual ~PanBy() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class PanTo : public XmlRpcMethodCallImp
{
public:
   PanTo() {}
   PanTo(const PanTo &other) {}
   virtual ~PanTo() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class RegisterCallback : public XmlRpcMethodCallImp
{
public:
   RegisterCallback(XmlRpcServer &server) : mServer(server) {}
   RegisterCallback(const RegisterCallback &other) : mServer(other.mServer) {}
   virtual ~RegisterCallback();
   void unRegister(XmlRpcCallback *pCallback);
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();

private:
   RegisterCallback& operator=(const RegisterCallback& rhs);

   XmlRpcServer& mServer;
   std::list<XmlRpcCallback*> mCallbacks;
};

class RotateBy : public XmlRpcMethodCallImp
{
public:
   RotateBy() {}
   RotateBy(const RotateBy &other) {}
   virtual ~RotateBy() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class RotateTo : public XmlRpcMethodCallImp
{
public:
   RotateTo() {}
   RotateTo(const RotateTo &other) {}
   virtual ~RotateTo() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class SetWindowState : public XmlRpcMethodCallImp
{
public:
   SetWindowState() {}
   SetWindowState(const SetWindowState &other) {}
   virtual ~SetWindowState() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class UnlinkViews : public XmlRpcMethodCallImp
{
public:
   UnlinkViews() {}
   UnlinkViews(const UnlinkViews &other) {}
   virtual ~UnlinkViews() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

class Zoom : public XmlRpcMethodCallImp
{
public:
   Zoom() {}
   Zoom(const Zoom &other) {}
   virtual ~Zoom() {}
   virtual XmlRpcParam *operator()(const XmlRpcParams &params);
   virtual QString getHelp();
   virtual XmlRpcArrayParam *getSignature();
};

}

#endif
