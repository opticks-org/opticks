/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtGui/QApplication>

#include "AnnotationLayerImp.h"
#include "GraphicLayer.h"
#include "GraphicObjectImp.h"
#include "Icons.h"
#include "PropertiesAnnotationLayer.h"

#include <string>

XERCES_CPP_NAMESPACE_USE
using namespace std;

AnnotationLayerImp::AnnotationLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   GraphicLayerImp(id, layerName, pElement), mSnapToGrid(false)
{
   addAcceptableGraphicType(LINE_OBJECT);
   addAcceptableGraphicType(HLINE_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(VLINE_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(TEXT_OBJECT);
   addAcceptableGraphicType(FRAME_LABEL_OBJECT);
   addAcceptableGraphicType(ARROW_OBJECT);
   addAcceptableGraphicType(RECTANGLE_OBJECT);
   addAcceptableGraphicType(ELLIPSE_OBJECT);
   addAcceptableGraphicType(ROUNDEDRECTANGLE_OBJECT);
   addAcceptableGraphicType(ARC_OBJECT);
   addAcceptableGraphicType(SCALEBAR_OBJECT);
   addAcceptableGraphicType(GROUP_OBJECT);
   addAcceptableGraphicType(POLYLINE_OBJECT);
   addAcceptableGraphicType(POLYGON_OBJECT);
   addAcceptableGraphicType(CGM_OBJECT);
   addAcceptableGraphicType(RAW_IMAGE_OBJECT);
   addAcceptableGraphicType(FILE_IMAGE_OBJECT);
   addAcceptableGraphicType(WIDGET_IMAGE_OBJECT);
   addAcceptableGraphicType(LATLONINSERT_OBJECT);
   addAcceptableGraphicType(NORTHARROW_OBJECT);
   addAcceptableGraphicType(EASTARROW_OBJECT);
   addAcceptableGraphicType(VIEW_OBJECT);
   addAcceptableGraphicType(MULTIPOINT_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(TRAIL_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(TRIANGLE_OBJECT);
   addAcceptableGraphicType(ROW_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(COLUMN_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(BITMASK_OBJECT); // does not provide a gui to create
   addAcceptableGraphicType(MEASUREMENT_OBJECT); // does not provide a gui to create

   setSnapToGrid(false);

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mAnnotation);
   }

   addPropertiesPage(PropertiesAnnotationLayer::getName());

   connect(this, SIGNAL(objectAdded(GraphicObject *)), this, SLOT(objectWasAdded(GraphicObject *)));
}

AnnotationLayerImp::~AnnotationLayerImp()
{
}

AnnotationLayerImp &AnnotationLayerImp::operator = (const AnnotationLayerImp &annotationLayer)
{
   if (this != &annotationLayer)
   {
      GraphicLayerImp::operator =(annotationLayer);
      mSnapToGrid = annotationLayer.mSnapToGrid;
   }
   return *this;
}

void AnnotationLayerImp::setDefaultFont(const QFont& font)
{
   QString face = font.family();
   int iSize = font.pointSize();
   bool bBold = font.bold();
   bool bItalic = font.italic();
   bool bUnderline = font.underline();

   GraphicLayer::setSettingTextFont(face.toStdString());
   GraphicLayer::setSettingTextFontSize(static_cast<unsigned int>(iSize));
   GraphicLayer::setSettingTextBold(bBold);
   GraphicLayer::setSettingTextItalics(bItalic);
   GraphicLayer::setSettingTextUnderline(bUnderline);
}

QFont AnnotationLayerImp::getDefaultFont()
{
   QString face = QString::fromStdString(GraphicLayer::getSettingTextFont());
   if (face.isEmpty() == true)
   {
      face = QApplication::font().family();
   }

   int iSize = GraphicLayer::getSettingTextFontSize();
   bool bBold = GraphicLayer::getSettingTextBold();
   bool bItalic = GraphicLayer::getSettingTextItalics();
   bool bUnderline = GraphicLayer::getSettingTextUnderline();

   QFont font(face, iSize);
   font.setBold(bBold);
   font.setItalic(bItalic);
   font.setUnderline(bUnderline);

   return font;
}

void AnnotationLayerImp::reset()
{
   double dAlpha = GraphicLayer::getSettingAlpha();
   double dStartAngle = GraphicLayer::getSettingStartAngle();
   double dStopAngle = GraphicLayer::getSettingStopAngle();
   double dApex = GraphicLayer::getSettingApex();
   ArcRegion arcRegion = GraphicLayer::getSettingArcRegion();
   ColorType fillColor = GraphicLayer::getSettingFillColor();
   bool bFillState = GraphicLayer::getSettingFill();
   FillStyle fillStyle = GraphicLayer::getSettingFillStyle();
   QFont font = AnnotationLayerImp::getDefaultFont();
   SymbolType hatchStyle = GraphicLayer::getSettingHatchStyle(); 
   ColorType lineColor = GraphicLayer::getSettingLineColor();
   bool bLineState = GraphicLayer::getSettingBorder();
   LineStyle lineStyle = GraphicLayer::getSettingLineStyle();
   double dLineWidth = GraphicLayer::getSettingLineWidth();
   double dRotation = GraphicLayer::getSettingRotation();
   ColorType textColor = GraphicLayer::getSettingTextColor();
   bool lineScaled = GraphicLayer::getSettingLineScaled();
   string symbolName = GraphicLayer::getSettingSymbolName();
   unsigned int symbolSize = GraphicLayer::getSettingSymbolSize();

   const list<GraphicObject*>& objects = getGroup()->getObjects();

   list<GraphicObject*>::const_iterator iter = objects.begin();
   while (iter != objects.end())
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         pObject->setAlpha(dAlpha);
         pObject->setAngles(dStartAngle, dStopAngle);
         pObject->setApex(dApex);
         pObject->setArcRegion(arcRegion);
         pObject->setFillColor(fillColor);
         pObject->setFillState(bFillState);
         pObject->setFillStyle(fillStyle);
         pObject->setFont(font);
         pObject->setHatchStyle(hatchStyle);
         pObject->setLineColor(lineColor);
         pObject->setLineState(bLineState);
         pObject->setLineStyle(lineStyle);
         pObject->setLineWidth(dLineWidth);
         pObject->setRotation(dRotation);
         pObject->setTextColor(textColor);
         pObject->setLineScaled(lineScaled);
         pObject->setSymbolName(symbolName);
         pObject->setSymbolSize(symbolSize);
      }

      ++iter;
   }
   setSnapToGrid(false);
}


LayerType AnnotationLayerImp::getLayerType() const
{
   return ANNOTATION;
}

QCursor AnnotationLayerImp::getMouseCursor() const
{
   QCursor mouseCursor;

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      mouseCursor = QCursor(pIcons->mAnnoCursor, pIcons->mAnnoMask, 1, 15);
   }

   return mouseCursor;
}

LocationType AnnotationLayerImp::correctCoordinate(const LocationType &coord) const
{
   LocationType loc = coord;
   if (mSnapToGrid)
   {
      loc.mX = static_cast<int>(coord.mX+0.5);
      loc.mY = static_cast<int>(coord.mY+0.5);
   }
   return loc;
}

void AnnotationLayerImp::setSnapToGrid(bool snap)
{
   if (snap != mSnapToGrid)
   {
      mSnapToGrid = snap;
      emit snapToGridChanged(snap);
   }
}

void AnnotationLayerImp::objectWasAdded(GraphicObject *pObj)
{
   if (pObj->isVisible())
   {
      deselectAllObjects();
      selectObject(pObj);
   }
}

void AnnotationLayerImp::completeInsertion(bool bValidObject)
{
   static bool completing = false;
   GraphicLayerImp::completeInsertion(bValidObject);
   if (!completing) // prevent recursion -- setCurrentGraphicObjectType calls completeInsertion()
   {
      completing = true;
      setCurrentGraphicObjectType(MOVE_OBJECT);
      completing = false;
   }
}

QColor AnnotationLayerImp::getLabelColor(const GraphicObjectImp *pObj)
{
   ColorType color = pObj->getLineColor();
   if (color.isValid() == false)
   {
      color = GraphicLayer::getSettingLineColor();
   }

   return QColor(color.mRed, color.mGreen, color.mBlue);
}

bool AnnotationLayerImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL || !GraphicLayerImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("snapToGrid", mSnapToGrid);
   return true;
}

bool AnnotationLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !GraphicLayerImp::fromXml(pDocument, version))
   {
      return false;
   }
   mSnapToGrid = StringUtilities::fromXmlString<bool>(
      A(static_cast<DOMElement*>(pDocument)->getAttribute(X("snapToGrid"))));
   return true;
}
