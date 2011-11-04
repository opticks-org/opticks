/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModelServices.h"
#include "AnnotationLayer.h"
#include "AoiLayer.h"
#include "AppVersion.h"
#include "GraphicObject.h"
#include "OpticksMethods.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "Filename.h"
#include "GraphicElement.h"
#include "GraphicGroup.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "LayerList.h"
#include "ObjectFactory.h"
#include "OpticksCallbacks.h"
#include "PerspectiveView.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "RectangleObject.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "WorkspaceWindow.h"
#include "XmlRpcArrayParam.h"
#include "XmlRpcCallback.h"
#include "XmlRpcStructParam.h"

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QtDebug>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QWidget>
#include <string>
#include <vector>

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove qDebug() calls " \
//   "after debugging is complete (tclarke)")

using namespace OpticksXmlRpcMethods;

int XML_RPC_INTERFACE_VERSION = 1;

namespace // utility functions
{
   PerspectiveView* getView(const XmlRpcParams& params, int paramNumber)
   {
      Service<DesktopServices> pDesktop;
      WorkspaceWindow* pWindow = NULL;
      if (params.size() < (paramNumber + 1))
      {
         pWindow = pDesktop->getCurrentWorkspaceWindow();
      }
      else
      {
         const XmlRpcParam* pViewId = params[paramNumber];
         if ((pViewId == NULL) || (pViewId->type() != STRING_PARAM))
         {
            throw XmlRpcMethodFault(200);
         }
         pWindow = dynamic_cast<WorkspaceWindow*>(
            Service<SessionManager>()->getSessionItem(pViewId->value().toString().toStdString()));
      }
      PerspectiveView* pView = NULL;
      if (pWindow != NULL)
      {
         pDesktop->setCurrentWorkspaceWindow(pWindow);
         pView = dynamic_cast<PerspectiveView*>(pWindow->getView());
      }
      if (pView == NULL)
      {
         throw XmlRpcMethodFault(300);
      }
      return pView;
   }

   void setAnnotationProperties(GraphicObject& object, const XmlRpcStructParam& properties)
   {
      for (XmlRpcStructParam::type::const_iterator it = properties.begin(); it != properties.end(); ++it)
      {
         const XmlRpcParam* pValue = it.value();
         const QString type(pValue->type());
         const QString val(pValue->value().toString());

         if (pValue == NULL)
         {
            continue;
         }
         if (it.key() == "Alpha")
         {
            object.setAlpha(pValue->value().toDouble());
         }
         else if (it.key() == "FillColor")
         {
            ColorType newColor = StringUtilities::fromXmlString<ColorType>(pValue->value().toString().toStdString());
            if (newColor.isValid())
            {
               object.setFillColor(newColor);
            }
         }
         else if (it.key() == "FillStyle")
         {
            FillStyle newFill = StringUtilities::fromXmlString<FillStyle>(pValue->value().toString().toStdString());
            if (newFill.isValid())
            {
               object.setFillStyle(newFill);
            }
         }
         else if (it.key() == "LineColor")
         {
            ColorType newColor = StringUtilities::fromXmlString<ColorType>(pValue->value().toString().toStdString());
            if (newColor.isValid())
            {
               object.setLineColor(newColor);
            }
            else
            {
               qDebug() << "Could not convert" << pValue;
            }
         }
         else if (it.key() == "Position" && pValue->type() == "array")
         {
            const XmlRpcArrayParam* pArray = static_cast<const XmlRpcArrayParam*>(pValue);
            if (pArray->size() == 2)
            {
               LocationType ll((*pArray)[0]->value().toDouble(), (*pArray)[1]->value().toDouble());
               LocationType sz(object.getUrCorner() - object.getLlCorner());
               LocationType ur(ll + sz);
               object.setBoundingBox(ll, ur);
            }
         }
         else if (it.key() == "Text")
         {
            object.setText(pValue->value().toString().toStdString());
         }
         else if (it.key() == "TextColor")
         {
            ColorType newColor = StringUtilities::fromXmlString<ColorType>(pValue->value().toString().toStdString());
            if (newColor.isValid())
            {
               object.setTextColor(newColor);
            }
         }
         else if (it.key() == "Points" && pValue->type() == "array")
         {
            const XmlRpcArrayParam* pArray = static_cast<const XmlRpcArrayParam*>(pValue);
            if (pArray->size() % 2 == 0)
            {
               RectangleObject* pRectObject = dynamic_cast<RectangleObject*>(&object);
               if (pRectObject != NULL)
               {
                  VERIFYNR(pArray->size() == 4);
                  LocationType ll((*pArray)[0]->value().toDouble(), (*pArray)[1]->value().toDouble());
                  LocationType ur((*pArray)[2]->value().toDouble(), (*pArray)[3]->value().toDouble());
                  object.setBoundingBox(ll, ur);
               }
               else
               {
                  std::vector<LocationType> points;

                  for (int i = 0; i < pArray->size(); i += 2)
                  {
                     LocationType pos((*pArray)[i]->value().toDouble(), (*pArray)[i+1]->value().toDouble());
                     points.push_back(pos);
                  }
                  object.addVertices(points);
               }
            }
         }
      }
   }
}

XmlRpcParam* Version::operator()(const XmlRpcParams& params)
{
   if (!params.empty())
   {
      throw XmlRpcMethodFault(200);
   }
   XmlRpcArrayParam* pRval = new XmlRpcArrayParam;
   *pRval << new XmlRpcParam(INT_PARAM, XML_RPC_INTERFACE_VERSION) << new XmlRpcParam(STRING_PARAM, APP_VERSION_NUMBER);
   return pRval;
}

QString Version::getHelp()
{
   return QString("Returns version information. "
                  "The first return value is the version of the XML-RPC interface. "
                  "The second return value is the version of "APP_NAME".");
}

XmlRpcArrayParam* Version::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "[XML-RPC version, "APP_NAME" version]");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Annotation::Create::operator()(const XmlRpcParams& params)
{
   if ((params.size() < 1 || params.size() > 4))
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 3));
   if (pView == NULL)
   {
      throw XmlRpcMethodFault(300);
   }
   QString annotationId;
   const XmlRpcParam* pAnnotationId = params[0];
   if (pAnnotationId != NULL)
   {
      annotationId = pAnnotationId->value().toString();
   }
   GraphicObjectType newObjectType;
   if (params.size() >= 2)
   {
      const XmlRpcParam* pAnnotationType = params[1];
      if (pAnnotationType != NULL)
      {
         newObjectType =
            StringUtilities::fromXmlString<GraphicObjectType>(pAnnotationType->value().toString().toStdString());
      }
   }
   GraphicLayer* pLayer = NULL;
   GraphicObject* pObject = NULL;
   if (!XmlRpcAnnotationCallback::getGraphicLayerAndObject(getLayerType(), annotationId, pLayer, pObject,
      pView, true, newObjectType))
   {
      if (pLayer == NULL)
      {
         throw XmlRpcMethodFault(305);
      }
      if (pObject != NULL)
      {
         throw XmlRpcMethodFault(304, getNamespace());
      }
      throw XmlRpcMethodFault(200);
   }
   pView->setActiveLayer(pLayer);
   if (params.size() >= 3 && params[2]->type() == "struct")
   {
      const XmlRpcStructParam* pProperties = static_cast<const XmlRpcStructParam*>(params[2]);
      setAnnotationProperties(*pObject, *pProperties);
      pView->refresh();
   }
   return NULL;
}

QString Annotation::Create::getHelp()
{
   return QString("Create an %1. %1ID is either LayerName[ObjectName], LayerName[ObjectType:ObjectIndex], or "
      "just LayerName. Allowed properties: Alpha, FillColor, FillStyle, LineColor, Position, Text, TextColor, "
      "and Points.").arg(getNamespace());
}

XmlRpcArrayParam* Annotation::Create::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1ID").arg(getNamespace()));
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1ID").arg(getNamespace()));
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1Type").arg(getNamespace()));
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1ID").arg(getNamespace()));
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1Type").arg(getNamespace()));
   *pParams << new XmlRpcParam(STRING_PARAM, "struct InitialProperties");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1ID").arg(getNamespace()));
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1Type").arg(getNamespace()));
   *pParams << new XmlRpcParam(STRING_PARAM, "struct InitialProperties");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Annotation::Delete::operator()(const XmlRpcParams& params)
{
   if ((params.size() != 1) && (params.size() != 2))
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 1));
   if (pView == NULL)
   {
      throw XmlRpcMethodFault(300);
   }

   QString annotationId;
   const XmlRpcParam* pAnnotationId = params[0];
   if (pAnnotationId != NULL)
   {
      annotationId = pAnnotationId->value().toString();
      if (annotationId.isEmpty() || annotationId.isNull())
      {
         throw XmlRpcMethodFault(200);
      }
   }
   GraphicLayer* pLayer = NULL;
   GraphicObject* pObject = NULL;
   if (!XmlRpcAnnotationCallback::getGraphicLayerAndObject(getLayerType(), annotationId, pLayer, pObject, pView))
   {
      if (pLayer == NULL)
      {
         throw XmlRpcMethodFault(305);
      }
   }

   if (pObject != NULL)
   {
      pLayer->removeObject(pObject, true);
   }
   else
   {
      if (!pView->deleteLayer(pLayer))
      {
         throw XmlRpcMethodFault(308);
      }
   }
   
   return NULL;
}

QString Annotation::Delete::getHelp()
{
   return "Delete an %1. %1ID is either LayerName[ObjectName] or just LayerName.";
}

XmlRpcArrayParam* Annotation::Delete::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1ID").arg(getNamespace()));
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, QString("string %1ID").arg(getNamespace()));
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Aoi::GetBoundingBox::operator()(const XmlRpcParams& params)
{
   if ((params.size() != 1) && (params.size() != 2))
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 1));
   if (pView == NULL)
   {
      throw XmlRpcMethodFault(300);
   }

   std::string aoiLayerName;
   const XmlRpcParam* pAoiLayerName = params[0];
   if (pAoiLayerName != NULL)
   {
      QString str = pAoiLayerName->value().toString();
      if (str.isEmpty() || str.isNull())
      {
         throw XmlRpcMethodFault(200);
      }
      aoiLayerName = str.toStdString();
   }

   LayerList* pLayerList = pView->getLayerList();
   std::vector<Layer*> aoiLayers;
   pLayerList->getLayers(AOI_LAYER, aoiLayers);
   AoiLayer* pAoiLayer = NULL;
   for (std::vector<Layer*>::iterator lit = aoiLayers.begin(); lit != aoiLayers.end(); ++lit)
   {
      Layer* pLayer = *lit;
      if (pLayer != NULL)
      {
         std::string layerName = pLayer->getName();
         if (layerName == aoiLayerName)
         {
            pAoiLayer = static_cast<AoiLayer*>(pLayer);
            break;
         }
      }
   }
   if (pAoiLayer == NULL)
   {
      throw XmlRpcMethodFault(305);
   }

   GraphicElement* pElement = static_cast<GraphicElement*>(pAoiLayer->getDataElement());
   if (pElement == NULL)
   {
      throw XmlRpcMethodFault(305);
   }
   GraphicGroup* pGroup = pElement->getGroup();
   if (pGroup == NULL)
   {
      throw XmlRpcMethodFault(305);
   }
   LocationType ll = pGroup->getLlCorner();
   LocationType ur = pGroup->getUrCorner();
   XmlRpcArrayParam* pRval = new XmlRpcArrayParam;
   if (pRval == NULL)
   {
      throw XmlRpcMethodFault(201);
   }
   XmlRpcArrayParam* pTuple = new XmlRpcArrayParam;
   if (pTuple == NULL)
   {
      throw XmlRpcMethodFault(201);
   }
   *pTuple << new XmlRpcParam(DOUBLE_PARAM, ll.mX)
           << new XmlRpcParam(DOUBLE_PARAM, ll.mY);
   *pRval << pTuple;
   pTuple = new XmlRpcArrayParam;
   if (pTuple == NULL)
   {
      throw XmlRpcMethodFault(201);
   }
   *pTuple << new XmlRpcParam(DOUBLE_PARAM, ur.mX)
           << new XmlRpcParam(DOUBLE_PARAM, ur.mY);
   *pRval << pTuple;

   return pRval;
}

QString Aoi::GetBoundingBox::getHelp()
{
   return "Get the minimum bounding box of an AOI in data coordinates. "
          "Returns an array of 2-tuples. [[x1, y1], [x2, y2]]";
}

XmlRpcArrayParam* Aoi::GetBoundingBox::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "array");
   *pParams << new XmlRpcParam(STRING_PARAM, "string AOIID");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "array");
   *pParams << new XmlRpcParam(STRING_PARAM, "string AOIID");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Aoi::SetMode::operator()(const XmlRpcParams& params)
{
   if ((params.size() != 2) && (params.size() != 3))
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 3));
   if (pView == NULL)
   {
      throw XmlRpcMethodFault(300);
   }

   GraphicObjectType aoiTool;
   const XmlRpcParam* pAoiTool = params[1];
   if (pAoiTool != NULL)
   {
      aoiTool = StringUtilities::fromXmlString<GraphicObjectType>(pAoiTool->value().toString().toStdString());
   }
   if (!aoiTool.isValid())
   {
      throw XmlRpcMethodFault(200);
   }
   ModeType aoiMode;
   const XmlRpcParam* pAoiMode = params[0];
   if (pAoiMode != NULL)
   {
      aoiMode = StringUtilities::fromXmlString<ModeType>(pAoiMode->value().toString().toStdString());
   }
   if (!aoiMode.isValid())
   {
      throw XmlRpcMethodFault(200);
   }
   Service<DesktopServices>()->setAoiSelectionTool(aoiTool, aoiMode);
   pView->setMouseMode("LayerMode");

   return NULL;
}

QString Aoi::SetMode::getHelp()
{
   return "Sets the AOI model and tool. "
      "AOI modes are: draw, erase, toggle, move.\n"
      "AOI tools are: pixel, rectangle, row, column, polygon, polyline, line, hline, vline.";
}

XmlRpcArrayParam* Aoi::SetMode::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string AOIMode");
   *pParams << new XmlRpcParam(STRING_PARAM, "string AOITool");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string AOIMode");
   *pParams << new XmlRpcParam(STRING_PARAM, "string AOITool");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Close::operator()(const XmlRpcParams& params)
{
   if (params.size() > 1)
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 0));
   if (pView == NULL)
   {
      throw XmlRpcMethodFault(300);
   }
   Service<DesktopServices> pDesktop;
   std::vector<Window*> windows;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (std::vector<Window*>::iterator wit = windows.begin(); wit != windows.end(); ++wit)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*wit);
      if (static_cast<SpatialDataView*>(pWindow->getView()) == pView)
      {
         pDesktop->deleteWindow(pWindow);
         break;
      }
   }
   return NULL;
}

QString Close::getHelp()
{
   return "Close a data set and the associated window.";
}

XmlRpcArrayParam* Close::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* CloseAll::operator()(const XmlRpcParams& params)
{
   Service<DesktopServices> pDesktop;
   pDesktop->deleteAllWindows();

   return NULL;
}

QString CloseAll::getHelp()
{
   return "Closes all data sets and associated windows.";
}

XmlRpcArrayParam* CloseAll::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pSignatures << pParams;
   return pSignatures;
}

XmlRpcParam* CreateView::operator()(const XmlRpcParams& params)
{
   if ((params.size() != 1) && (params.size() != 2))
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 1));
   
   std::string newViewName;
   const XmlRpcParam* pNewViewName = params[0];
   if (pNewViewName != NULL)
   {
      QString str = pNewViewName->value().toString();
      if (str.isEmpty() || str.isNull())
      {
         throw XmlRpcMethodFault(200);
      }
      newViewName = str.toStdString();
   }

   Service<DesktopServices> pDesktop;
   SpatialDataWindow* pNewWindow = static_cast<SpatialDataWindow*>(
      pDesktop->createWindow(newViewName, SPATIAL_DATA_WINDOW));
   if (pNewWindow == NULL)
   {
      throw XmlRpcMethodFault(309);
   }

   SpatialDataView* pNewView = pNewWindow->getSpatialDataView();
   if (pNewView == NULL)
   {
      pDesktop->deleteWindow(pNewWindow);
      throw XmlRpcMethodFault(309);
   }
   pView->copy(pNewView);

   return NULL;
}

QString CreateView::getHelp()
{
   return "Create a new view on an existing data set. The arguments are a new view name and an optional "
      "existing view name.";
}

XmlRpcArrayParam* CreateView::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string NewViewName");
   *pSignatures << pParams;
   
   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string NewViewName");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* ExportElement::operator()(const XmlRpcParams& params)
{
   if (params.size() < 1)
   {
      throw XmlRpcMethodFault(200);
   }

   const XmlRpcParam* pExporterName = params[0];
   if ((pExporterName == NULL) || (pExporterName->type() != STRING_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }

   const XmlRpcParam* pFilename = params[1];
   if ((pFilename == NULL) || (pFilename->type() != STRING_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }

   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView(params, 2));
   if (pView == NULL)
   {
      throw XmlRpcMethodFault(300);
   }

   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList == NULL)
   {
      throw XmlRpcMethodFault(305);
   }

   RasterElement* pElement = pLayerList->getPrimaryRasterElement();
   if (pElement == NULL)
   {
      throw XmlRpcMethodFault(303);
   }

   FactoryResource<FileDescriptor> pFileDescriptor(
      RasterUtilities::generateFileDescriptorForExport(pElement->getDataDescriptor(),
      pFilename->value().toString().toStdString()));
   if (pFileDescriptor.get() == NULL)
   {
      throw XmlRpcMethodFault(307);
   }

   ExporterResource exporter(pExporterName->value().toString().toStdString(), pElement, pFileDescriptor.get());
   if (exporter->getPlugIn() == NULL)
   {
      throw XmlRpcMethodFault(307);
   }

   if (exporter->execute() == false)
   {
      throw XmlRpcMethodFault(313);
   }

   return NULL;
}

QString ExportElement::getHelp()
{
   return "Export the primary raster element of a data set. "
      "The arguments are the name of the exporter, output file and, optionally, the id of the view to use.";
}

XmlRpcArrayParam* ExportElement::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string Exporter");
   *pParams << new XmlRpcParam(STRING_PARAM, "string OutputFilename");
   *pSignatures << pParams;
   
   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string Exporter");
   *pParams << new XmlRpcParam(STRING_PARAM, "string OutputFilename");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewId");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* GetMetadata::operator()(const XmlRpcParams& params)
{
   if (params.size() != 2)
   {
      throw XmlRpcMethodFault(200);
   }

   const XmlRpcParam* pInputFilename = params[0];
   if ((pInputFilename == NULL) || (pInputFilename->type() != STRING_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }

   const std::string inputFilename = XmlBase::URLtoPath(X(pInputFilename->value().toString().toAscii()));
   if (inputFilename.empty())
   {
      throw XmlRpcMethodFault(200);
   }

   const XmlRpcParam* pOutputFilename = params[1];
   if ((pOutputFilename == NULL) || (pOutputFilename->type() != STRING_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }

   const std::string outputFilename = XmlBase::URLtoPath(X(pOutputFilename->value().toString().toAscii()));
   if (outputFilename.empty())
   {
      throw XmlRpcMethodFault(200);
   }

   ImporterResource pImporter("Auto Importer", inputFilename, NULL, false);
   Importer* pImp = dynamic_cast<Importer*>(pImporter->getPlugIn());
   if (pImp == NULL)
   {
      throw XmlRpcMethodFault(302);
   }

   std::string errorMessage;
   std::vector<ImportDescriptor*> descriptors = pImporter->getImportDescriptors();
   if (descriptors.size() != 1 || descriptors[0] == NULL || descriptors[0]->getDataDescriptor() == NULL)
   {
      errorMessage = "The file must contain exactly 1 image";
   }
   else
   {
      if (!pImp->validate(descriptors[0]->getDataDescriptor(), errorMessage) && errorMessage.empty())
      {
         errorMessage = "Validate returned false";
      }
   }

   // If there were validation warnings or errors, return now
   if (!errorMessage.empty())
   {
      return new XmlRpcParam("string", QString::fromStdString(errorMessage));
   }

   DataDescriptorResource<DataDescriptor> pDescriptor(descriptors[0]->getDataDescriptor()->copy());
   if (!pDescriptor.get())
   {
      throw XmlRpcMethodFault(307);
   }

   // Mark this as on disk so that no memory is allocated
   pDescriptor->setProcessingLocation(ON_DISK);
   ModelResource<DataElement> pElement(pDescriptor.release());
   if (!pElement.get())
   {
      throw XmlRpcMethodFault(307);
   }

   FactoryResource<FileDescriptor> pFileDescriptor(
      RasterUtilities::generateFileDescriptorForExport(pElement->getDataDescriptor(), outputFilename));
   if (pFileDescriptor.get() == NULL)
   {
      throw XmlRpcMethodFault(307);
   }

   // This is not a generic export method because the raster data is not present
   // This is ok for this particular exporter
   ExporterResource exporter("Metadata Exporter", pElement.get(), pFileDescriptor.get());
   if (exporter->getPlugIn() == NULL)
   {
      throw XmlRpcMethodFault(307);
   }

   if (exporter->execute() == false)
   {
      throw XmlRpcMethodFault(313);
   }

   return new XmlRpcParam("string", QString::fromStdString("Success"));
}

QString GetMetadata::getHelp()
{
   return "Retrieve metadata from a file without importing the data.\n";
}

XmlRpcArrayParam* GetMetadata::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "string");
   *pParams << new XmlRpcParam(STRING_PARAM, "string InputFilename");
   *pParams << new XmlRpcParam(STRING_PARAM, "string OutputFilename");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* GetViewInfo::operator()(const XmlRpcParams& params)
{
   if (params.size() > 1)
   {
      throw XmlRpcMethodFault(200);
   }
   PerspectiveView* pView = getView(params, 0);

   XmlRpcStructParam* pRval = new XmlRpcStructParam;

   QString backgroundColor =
      QString::fromStdString(StringUtilities::toXmlString<ColorType>(pView->getBackgroundColor()));
   pRval->insert("background color", new XmlRpcParam(STRING_PARAM, backgroundColor));

   double minX(0.0);
   double minY(0.0);
   double maxX(0.0);
   double maxY(0.0);
   pView->getExtents(minX, minY, maxX, maxY);
   XmlRpcArrayParam* pExtents = new XmlRpcArrayParam;
   *pExtents << new XmlRpcParam(DOUBLE_PARAM, minX) << new XmlRpcParam(DOUBLE_PARAM, minY)
             << new XmlRpcParam(DOUBLE_PARAM, maxX) << new XmlRpcParam(DOUBLE_PARAM, maxY);
   pRval->insert("extents", pExtents);

   XmlRpcArrayParam* pVisibleCenter = new XmlRpcArrayParam;
   *pVisibleCenter << new XmlRpcParam(DOUBLE_PARAM, pView->getVisibleCenter().mX)
                   << new XmlRpcParam(DOUBLE_PARAM, pView->getVisibleCenter().mY);
   pRval->insert("visible center", pVisibleCenter);
   pRval->insert("rotation", new XmlRpcParam(DOUBLE_PARAM, pView->getRotation()));
   pRval->insert("zoom percentage", new XmlRpcParam(DOUBLE_PARAM, pView->getZoomPercentage()));
   SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
   if (pSpatialDataView != NULL)
   {
      RasterElement* pRasterElement =
         dynamic_cast<RasterElement*>(pSpatialDataView->getLayerList()->getPrimaryRasterElement());
      if (pRasterElement != NULL)
      {
         FileDescriptor* pFileDescriptor = pRasterElement->getDataDescriptor()->getFileDescriptor();
         if (pFileDescriptor != NULL)
         {
            QString filename = QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName());
            if (!filename.isEmpty())
            {
               pRval->insert("filename", new XmlRpcParam(STRING_PARAM, filename));
            }
         }
      }
   }

   return pRval;
}

QString GetViewInfo::getHelp()
{
   return "Retrieve information about a view.\n"
      "Returned structure contains: \n"
      "background color = string containing #AARRGGBB; \n"
      "extents = array of doubles [minX, minY, maxX, maxY]; \n"
      "visible center = array of doubles [centerX, centerY]; \n"
      "rotation = double indicating rotation in degrees; \n"
      "zoom percentage = double indicating zoom in percent; \n"
      "filename = string indicating the filename of the loaded data (not present if there is no on-disk file)";
}

XmlRpcArrayParam* GetViewInfo::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "struct");
   *pSignatures << pParams;
   
   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "struct");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* GetViews::operator()(const XmlRpcParams& params)
{
   Service<DesktopServices> pDesktop;
   if (params.size() != 0)
   {
      throw XmlRpcMethodFault(200);
   }

   XmlRpcArrayParam* pRval = new XmlRpcArrayParam;
   std::vector<Window*> windows;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (std::vector<Window*>::const_iterator wit = windows.begin(); wit != windows.end(); ++wit)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*wit);
      if (pWindow != NULL)
      {
         XmlRpcStructParam* pStruct = new XmlRpcStructParam;
         pStruct->insert("name", new XmlRpcParam(STRING_PARAM, QString::fromStdString(pWindow->getName())));
         pStruct->insert("id", new XmlRpcParam(STRING_PARAM, QString::fromStdString(pWindow->getId())));
         *pRval << pStruct;
      }
   }

   return pRval;
}

QString GetViews::getHelp()
{
   return "Retrieve an array of available view names and IDs.";
}

XmlRpcArrayParam* GetViews::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "array");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* LinkViews::operator()(const XmlRpcParams& params)
{
   if (params.size() != 2)
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pPrimaryView = dynamic_cast<SpatialDataView*>(getView(params, 0));
   SpatialDataView* pLinkedView = dynamic_cast<SpatialDataView*>(getView(params, 1));

   if ((pPrimaryView == NULL) || (pLinkedView == NULL))
   {
      throw XmlRpcMethodFault(300);
   }

   if (!pPrimaryView->linkView(pLinkedView, AUTOMATIC_LINK))
   {
      throw XmlRpcMethodFault(310);
   }

   return NULL;
}

QString LinkViews::getHelp()
{
   return "Link two views.";
}

XmlRpcArrayParam* LinkViews::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string PrimaryViewID");
   *pParams << new XmlRpcParam(STRING_PARAM, "string LinkedViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Open::operator()(const XmlRpcParams& params)
{
   std::string filename;
   if (params.size() < 1)
   {
      throw XmlRpcMethodFault(200);
   }
   const XmlRpcParam* pFilenameParam = params[0];
   if ((pFilenameParam == NULL) || (pFilenameParam->type() != STRING_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }
   filename = XmlBase::URLtoPath(X(pFilenameParam->value().toString().toAscii()));
   if (filename.empty())
   {
      throw XmlRpcMethodFault(200);
   }

   ProcessingLocation location;
   if (params.size() >= 2)
   {
      const XmlRpcParam* pProcessingLocation = params[1];
      if ((pProcessingLocation == NULL) || (pProcessingLocation->type() != STRING_PARAM))
      {
         throw XmlRpcMethodFault(200);
      }
      location =
         StringUtilities::fromXmlString<ProcessingLocation>(pProcessingLocation->value().toString().toStdString());
   }

   // See if the cube has already been imported
   std::vector<Window*> windows;
   Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);

   for (std::vector<Window*>::const_iterator wit = windows.begin(); wit != windows.end(); ++wit)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*wit);
      if (pWindow != NULL)
      {
         RasterElement* pRasterElement =
            dynamic_cast<RasterElement*>(pWindow->getSpatialDataView()->getLayerList()->getPrimaryRasterElement());
         if (pRasterElement != NULL)
         {
            FileDescriptor* pFileDescriptor = pRasterElement->getDataDescriptor()->getFileDescriptor();
            if (pFileDescriptor != NULL)
            {
               QString pathname = QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName());
               if ((pathname.toStdString()) == filename)
               {
                  return new XmlRpcParam("string", QString::fromStdString(pWindow->getId()));
               }
            }
         }
      }
   }

   ImporterResource pImporter("Auto Importer", filename, NULL, false);
   pImporter->createProgressDialog(true);

   Importer* pImp = dynamic_cast<Importer*>(pImporter->getPlugIn());
   if (pImp == NULL)
   {
      return NULL;
   }
   QString errors;
   bool cubePresent = false;
   std::vector<ImportDescriptor*> descriptors = pImporter->getImportDescriptors();
   for (std::vector<ImportDescriptor*>::iterator descriptor = descriptors.begin();
      descriptor != descriptors.end(); ++descriptor)
   {
      if (*descriptor != NULL && (*descriptor)->getDataDescriptor() != NULL)
      {
         if (dynamic_cast<RasterDataDescriptor*>((*descriptor)->getDataDescriptor()) != NULL)
         {
            cubePresent = true;
         }
         if (location.isValid() && (*descriptor)->getDataDescriptor()->getProcessingLocation() != location)
         {
            (*descriptor)->getDataDescriptor()->setProcessingLocation(location);
         }
         std::string errorMessage;
         if (!pImp->validate((*descriptor)->getDataDescriptor(), errorMessage) && !errorMessage.empty())
         {
            if (!errors.isEmpty())
            {
               errors += "\n";
            }
            errors += QString::fromStdString(errorMessage);
         }
      }
   }
   if (!cubePresent)
   {
      throw XmlRpcMethodFault(200, "File does not contain a data cube");
   }

   if (!pImporter->execute())
   {
      throw XmlRpcMethodFault(302, errors);
   }

   std::vector<DataElement*> elements = pImporter->getImportedElements();
   if (!elements.empty())
   {
      Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
      for (std::vector<Window*>::const_iterator wit = windows.begin(); wit != windows.end(); ++wit)
      {
         SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*wit);
         if (pWindow != NULL && static_cast<SpatialDataView*>(pWindow->getSpatialDataView())->
            getLayerList()->getPrimaryRasterElement() == elements.front())
         {
            return new XmlRpcParam("string", QString::fromStdString(pWindow->getId()));
         }
      }
   }

   return NULL;
}

QString Open::getHelp()
{
   return "Open a data file in a new view.\n"
          "The file path is specified as a file:// URL. Returns the ID of the new view.";
}

XmlRpcArrayParam* Open::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "string");
   *pParams << new XmlRpcParam(STRING_PARAM, "string URL");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "string");
   *pParams << new XmlRpcParam(STRING_PARAM, "string URL");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ProcessingLocation");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* PanBy::operator()(const XmlRpcParams& params)
{
   if (params.size() < 2 || params.size() > 3)
   {
      throw XmlRpcMethodFault(200);
   }
   PerspectiveView* pView = getView(params, 2);

   const XmlRpcParam* pPanXAmount = params[0];
   const XmlRpcParam* pPanYAmount = params[1];
   if ((pPanXAmount == NULL) || (pPanYAmount == NULL) ||
                               ((pPanXAmount->type() != DOUBLE_PARAM) &&
                                (pPanXAmount->type() != INT_PARAM)) ||
                               ((pPanYAmount->type() != DOUBLE_PARAM) &&
                                (pPanYAmount->type() != INT_PARAM)))
   {
      throw XmlRpcMethodFault(200);
   }
   double panX = pPanXAmount->value().toDouble();
   double panY = pPanYAmount->value().toDouble();

   pView->panBy(panX, panY);
   pView->refresh();

   return NULL;
}

QString PanBy::getHelp()
{
   return "Pan a " APP_NAME " view by a certain amount.";
}

XmlRpcArrayParam* PanBy::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanByX");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanByY");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanByX");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanByY");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanByX");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanByY");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanByX");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanByY");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* PanTo::operator()(const XmlRpcParams& params)
{
   if (params.size() < 2 || params.size() > 3)
   {
      throw XmlRpcMethodFault(200);
   }
   PerspectiveView* pView = getView(params, 2);

   const XmlRpcParam* pPanXAmount = params[0];
   const XmlRpcParam* pPanYAmount = params[1];
   if ((pPanXAmount == NULL) || (pPanYAmount == NULL) ||
                               ((pPanXAmount->type() != DOUBLE_PARAM) &&
                                (pPanXAmount->type() != INT_PARAM)) ||
                               ((pPanYAmount->type() != DOUBLE_PARAM) &&
                                (pPanYAmount->type() != INT_PARAM)))
   {
      throw XmlRpcMethodFault(200);
   }
   LocationType location(pPanXAmount->value().toDouble(), pPanYAmount->value().toDouble());

   pView->panTo(location);
   pView->refresh();

   return NULL;
}

QString PanTo::getHelp()
{
   return "Pan a " APP_NAME " view to a location.";
}

XmlRpcArrayParam* PanTo::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanToX");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanToY");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanToX");
   *pParams << new XmlRpcParam(STRING_PARAM, "double PanToY");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanToX");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanToY");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanToX");
   *pParams << new XmlRpcParam(STRING_PARAM, "int PanToY");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

RegisterCallback::~RegisterCallback()
{
   for (std::list<XmlRpcCallback*>::iterator cit = mCallbacks.begin(); cit != mCallbacks.end(); ++cit)
   {
      delete *cit;
   }
}

void RegisterCallback::unRegister(XmlRpcCallback* pCallback)
{
   if (pCallback != NULL)
   {
      if (find(mCallbacks.begin(), mCallbacks.end(), pCallback) != mCallbacks.end())
      {
         mCallbacks.remove(pCallback);
         delete pCallback;
      }
   }
}

XmlRpcParam* RegisterCallback::operator()(const XmlRpcParams& params)
{
   if (params.size() < 3 || params.size() > 4)
   {
      throw XmlRpcMethodFault(200);
   }
   LayerType callbackType = StringUtilities::fromXmlString<LayerType>(params[0]->value().toString().toStdString());
   QString callbackUrl = params[1]->value().toString();
   QString callbackMethod = params[2]->value().toString();
   const XmlRpcStructParam* pArgs = NULL;
   if (params.size() == 4)
   {
      pArgs = dynamic_cast<const XmlRpcStructParam*>(params[3]);
   }
   mCallbacks.push_back(new XmlRpcAnnotationCallback(callbackType, callbackUrl, callbackMethod, *pArgs, *this));
   return NULL;
}

QString RegisterCallback::getHelp()
{
   return "Register a callback for " APP_NAME " events.<br/>"
      "The currently supported callback types, their registration parameters and the parameters passed to "
      "the callback are:<br/>"
      "<table><tr><th>Callback type</th><th>Registration parameters</th><th>Callback parameters</th></tr>"
      "<tr><td>annotation: Activated when certain events happen to annotation objects</td>"
      "<td>string name: an Annotation object specifier. LayerName[ObjectIndex], "
      "LayerName[ObjectType:TypeIndex], or LayerName[ObjectName]<br/>"
      "string event: deleted<br/>"
      "variable data...: The new annotation data</td></tr>"
      "<tr><td>aoi: Activated when certain events happen to AOIs</td>"
      "<td>string name: an AOI layer name.</td>"
      "string event: modified</td></tr>"
      "</table>";
}

XmlRpcArrayParam* RegisterCallback::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string CallbackType");
   *pParams << new XmlRpcParam(STRING_PARAM, "string CallbackURL");
   *pParams << new XmlRpcParam(STRING_PARAM, "string CallbackFunctionName");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string CallbackType");
   *pParams << new XmlRpcParam(STRING_PARAM, "string CallbackURL");
   *pParams << new XmlRpcParam(STRING_PARAM, "string CallbackFunctionName");
   *pParams << new XmlRpcParam(STRING_PARAM, "struct CallbackParameters");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* RotateBy::operator()(const XmlRpcParams& params)
{
   if (params.size() < 1 || params.size() > 2)
   {
      throw XmlRpcMethodFault(200);
   }
   PerspectiveView* pView = getView(params, 1);

   const XmlRpcParam* pRotationAmount = params[0];
   if ((pRotationAmount == NULL) || ((pRotationAmount->type() != DOUBLE_PARAM) &&
                                    (pRotationAmount->type() != INT_PARAM)))
   {
      throw XmlRpcMethodFault(200);
   }
   double degrees = pRotationAmount->value().toDouble();

   pView->rotateBy(degrees);
   pView->refresh();

   return NULL;
}

QString RotateBy::getHelp()
{
   return "Rotate an " APP_NAME " view by a certain amount.";
}

XmlRpcArrayParam* RotateBy::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double RotateBy");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double RotateBy");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int RotateBy");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int RotateBy");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* RotateTo::operator()(const XmlRpcParams& params)
{
   if (params.size() < 1 || params.size() > 2)
   {
      throw XmlRpcMethodFault(200);
   }
   PerspectiveView* pView = getView(params, 1);

   const XmlRpcParam* pRotationAmount = params[0];
   if ((pRotationAmount == NULL) || ((pRotationAmount->type() != DOUBLE_PARAM) &&
                                    (pRotationAmount->type() != INT_PARAM)))
   {
      throw XmlRpcMethodFault(200);
   }
   double degrees = pRotationAmount->value().toDouble();

   pView->rotateTo(degrees);
   pView->refresh();

   return NULL;
}

QString RotateTo::getHelp()
{
   return "Rotate an " APP_NAME " view to a certain rotational position.";
}

XmlRpcArrayParam* RotateTo::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double RotateTo");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double RotateTo");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int RotateTo");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int RotateTo");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* SetWindowState::operator()(const XmlRpcParams& params)
{
   if (params.size() != 2)
   {
      throw XmlRpcMethodFault(200);
   }

   const XmlRpcParam* pClearFlags = params[0];
   if ((pClearFlags == NULL) || (pClearFlags->type() != INT_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }

   const XmlRpcParam* pSetFlags = params[1];
   if ((pSetFlags == NULL) || (pSetFlags->type() != INT_PARAM))
   {
      throw XmlRpcMethodFault(200);
   }

   QWidget* pWidget = Service<DesktopServices>()->getMainWidget();
   if (pWidget == NULL)
   {
      throw XmlRpcMethodFault(303);
   }

   Qt::WindowStates clearFlags(pClearFlags->value().toInt());
   Qt::WindowStates setFlags(pSetFlags->value().toInt());
   pWidget->setWindowState(pWidget->windowState() & ~clearFlags | setFlags);
   return new XmlRpcParam(INT_PARAM, QVariant(pWidget->windowState()));
}

QString SetWindowState::getHelp()
{
   return "Change the window state of "APP_NAME" by specifying new Qt::WindowStates flags. "
      "ClearFlags will be applied before SetFlags.";
}

XmlRpcArrayParam* SetWindowState::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "int");
   *pParams << new XmlRpcParam(STRING_PARAM, "int ClearFlags");
   *pParams << new XmlRpcParam(STRING_PARAM, "int SetFlags");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* UnlinkViews::operator()(const XmlRpcParams& params)
{
   if (params.size() != 2)
   {
      throw XmlRpcMethodFault(200);
   }
   SpatialDataView* pPrimaryView = dynamic_cast<SpatialDataView*>(getView(params, 0));
   SpatialDataView* pLinkedView = dynamic_cast<SpatialDataView*>(getView(params, 1));

   if ((pPrimaryView == NULL) || (pLinkedView == NULL))
   {
      throw XmlRpcMethodFault(300);
   }

   if (!pPrimaryView->unlinkView(pLinkedView))
   {
      throw XmlRpcMethodFault(310);
   }

   return NULL;
}

QString UnlinkViews::getHelp()
{
   return "Unlink two previously linked views.";
}

XmlRpcArrayParam* UnlinkViews::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "string PrimaryViewID");
   *pParams << new XmlRpcParam(STRING_PARAM, "string LinkedViewID");
   *pSignatures << pParams;

   return pSignatures;
}

XmlRpcParam* Zoom::operator()(const XmlRpcParams& params)
{
   if (params.size() < 1 || params.size() > 2)
   {
      throw XmlRpcMethodFault(200);
   }
   PerspectiveView* pView = getView(params, 1);

   const XmlRpcParam* pZoomLevel = params[0];
   if ((pZoomLevel == NULL) || ((pZoomLevel->type() != DOUBLE_PARAM) &&
                               (pZoomLevel->type() != INT_PARAM)))
   {
      throw XmlRpcMethodFault(200);
   }
   double zoomLevel = pZoomLevel->value().toDouble();

   pView->zoomTo(zoomLevel);
   pView->refresh();

   return NULL;
}

QString Zoom::getHelp()
{
   return "Change the zoom level of an " APP_NAME " view.";
}

XmlRpcArrayParam* Zoom::getSignature()
{
   XmlRpcArrayParam* pSignatures = new XmlRpcArrayParam;

   XmlRpcArrayParam* pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double ZoomLevel");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "double ZoomLevel");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int ZoomLevel");
   *pSignatures << pParams;

   pParams = new XmlRpcArrayParam;
   *pParams << new XmlRpcParam(STRING_PARAM, "null");
   *pParams << new XmlRpcParam(STRING_PARAM, "int ZoomLevel");
   *pParams << new XmlRpcParam(STRING_PARAM, "string ViewID");
   *pSignatures << pParams;

   return pSignatures;
}
