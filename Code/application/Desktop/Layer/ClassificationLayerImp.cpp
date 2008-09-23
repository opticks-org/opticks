/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayerImp.h"
#include "ClassificationLayerImp.h"
#include "ClassificationLayerAdapter.h"
#include "ConfigurationSettingsImp.h"
#include "AppVerify.h"
#include "GraphicElementAdapter.h"
#include "GraphicProperty.h"
#include "PerspectiveView.h"
#include "StringUtilities.h"
#include "TextObject.h"
#include "TextObjectImp.h"
#include "Undo.h"
#include "xmlreader.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

ClassificationLayerImp::ClassificationLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   AnnotationLayerImp(id, layerName, pElement),
   mpTopText(NULL),
   mpBottomText(NULL)
{
   mClassificationFont = AnnotationLayerImp::getDefaultFont();

   ColorType textColor = GraphicLayer::getSettingTextColor();
   mClassificationColor = QColor(textColor.mRed, textColor.mGreen, textColor.mBlue);

   GraphicObject *pTopObj = NULL;
   GraphicObject *pBottomObj = NULL;
   GraphicElement *pGraphicElement = dynamic_cast<GraphicElement*>(pElement);
   if (pGraphicElement)
   {
      GraphicGroup *pGroup = pGraphicElement->getGroup();
      if (pGroup)
      {
         const list<GraphicObject*>& objects = pGroup->getObjects();
         if (objects.size() == 2)
         {
            list<GraphicObject*>::const_iterator iter = objects.begin();
            pTopObj = *iter++;
            pBottomObj = *iter;
            mpTopText = dynamic_cast<TextObjectImp*>(pTopObj);
            mpBottomText = dynamic_cast<TextObjectImp*>(pBottomObj);
         }
      }
   }

   if (mpTopText == NULL)
   {
      // Top text
      pTopObj = GraphicLayerImp::addObject(TEXT_OBJECT, LocationType());
      mpTopText = dynamic_cast<TextObjectImp*>(pTopObj);
      if (mpTopText)
      {
         mpTopText->setFont(mClassificationFont.getQFont());
         mpTopText->setTextColor(textColor);
      }
   }

   if (mpBottomText == NULL)
   {
      // Bottom text
      pBottomObj = GraphicLayerImp::addObject(TEXT_OBJECT, LocationType());
      mpBottomText = dynamic_cast<TextObjectImp*>(pBottomObj);
      if (mpBottomText)
      {
         mpBottomText->setFont(mClassificationFont.getQFont());
         mpBottomText->setTextColor(textColor);
      }
   }

   // Initialization
   GraphicLayerImp::selectObject(pTopObj);
   GraphicLayerImp::selectObject(pBottomObj);
   setHideSelectionBox(true);

   // Connections
   if (mpTopText)
   {
      connect(mpTopText, SIGNAL(propertyModified(GraphicProperty*)), this,
         SLOT(updateProperties(GraphicProperty*)));
   }
   if (mpBottomText)
   {
      connect(mpBottomText, SIGNAL(propertyModified(GraphicProperty*)), this,
         SLOT(updateProperties(GraphicProperty*)));
   }
   connect(this, SIGNAL(fontChanged(const QFont&)), this, SIGNAL(modified()));
   connect(this, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified()));
}

ClassificationLayerImp::~ClassificationLayerImp()
{
}

const string& ClassificationLayerImp::getObjectType() const
{
   static string type("ClassificationLayerImp");
   return type;
}

bool ClassificationLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ClassificationLayer"))
   {
      return true;
   }

   return GraphicLayerImp::isKindOf(className);
}

ClassificationLayerImp &ClassificationLayerImp::operator =(
   const ClassificationLayerImp &classificationLayer)
{
   if (this != &classificationLayer)
   {
      AnnotationLayerImp::operator =(classificationLayer);
      mClassificationFont = classificationLayer.mClassificationFont;
      mClassificationColor = classificationLayer.mClassificationColor;

      GraphicGroup *pGroup = getGroup();
      VERIFYRV(pGroup != NULL, *this);

      const std::list<GraphicObject*> &objects = pGroup->getObjects();
      VERIFYRV(objects.size() == 2, *this);

      mpTopText = dynamic_cast<TextObjectImp*>(objects.front());
      mpBottomText = dynamic_cast<TextObjectImp*>(objects.back());
   }

   return *this;
}

GraphicObject* ClassificationLayerImp::addObject(const GraphicObjectType& objectType, LocationType point)
{
   return NULL;
}

bool ClassificationLayerImp::removeObject(GraphicObject* pObject, bool bDelete)
{
   return false;
}

list<GraphicObject*> ClassificationLayerImp::getObjects() const
{
   return list<GraphicObject*>();
}

list<GraphicObject*> ClassificationLayerImp::getObjects(const GraphicObjectType& objectType) const
{
   return list<GraphicObject*>();
}

bool ClassificationLayerImp::selectObject(GraphicObject* pObject)
{
   return false;
}

void ClassificationLayerImp::selectAllObjects()
{
}

bool ClassificationLayerImp::isObjectSelected(GraphicObject* pObject) const
{
   return false;
}

void ClassificationLayerImp::getSelectedObjects(std::list<GraphicObject*>& selectedObjects) const
{
   selectedObjects.clear();
}

void ClassificationLayerImp::getSelectedObjects(const GraphicObjectType& objectType,
                                                list<GraphicObject*>& selectedObjects) const
{
   selectedObjects.clear();
}

unsigned int ClassificationLayerImp::getNumSelectedObjects() const
{
   return 0;
}

unsigned int ClassificationLayerImp::getNumSelectedObjects(const GraphicObjectType& objectType) const
{
   return 0;
}

bool ClassificationLayerImp::deselectObject(GraphicObject* pObject)
{
   return false;
}

void ClassificationLayerImp::deselectAllObjects()
{
}

void ClassificationLayerImp::groupSelection()
{
}

void ClassificationLayerImp::ungroupSelection()
{
}

void ClassificationLayerImp::clearSelection()
{
}

void ClassificationLayerImp::popFront()
{
}

void ClassificationLayerImp::pushBack()
{
}

bool ClassificationLayerImp::setBoundingBox(LocationType llCorner, LocationType urCorner)
{
   return false;
}

bool ClassificationLayerImp::setText(const char* pText)
{
   return false;
}

const char* ClassificationLayerImp::getText() const
{
   static string classificationText;
   classificationText.clear();

   // Get the top and bottom text
   QString strTopText = QString::fromStdString(mpTopText->getText());
   QString strBottomText = QString::fromStdString(mpBottomText->getText());

   // Remove any release info
   Service<ConfigurationSettings> pConfigSettings;
   string releaseType = StringUtilities::toDisplayString(pConfigSettings->getReleaseType());
   int iPos = strTopText.lastIndexOf("\n" + QString::fromStdString(releaseType));
   if (iPos != -1)
   {
      strTopText.truncate(iPos);
   }

   QString strReleaseText = "Not for Production Use\n";

   iPos = strBottomText.indexOf(strReleaseText);
   if (iPos != -1)
   {
      strBottomText = strBottomText.mid(strReleaseText.length());
   }

   // Set the text only if the top and bottom text is the same
   if (strTopText == strBottomText)
   {
      classificationText = strTopText.toStdString();
   }

   if (classificationText.empty() == true)
   {
      return NULL;
   }

   return classificationText.c_str();
}

bool ClassificationLayerImp::setRotation(double angle)
{
   return false;
}

bool ClassificationLayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button,
                                               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool ClassificationLayerImp::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button,
                                              Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool ClassificationLayerImp::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button,
                                                 Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   return false;
}

QFont ClassificationLayerImp::getClassificationFont() const
{
   return mClassificationFont.toQFont();
}

QColor ClassificationLayerImp::getClassificationColor() const
{
   return mClassificationColor;
}

TextObject* ClassificationLayerImp::getTopText() const
{
   return dynamic_cast<TextObject*>(mpTopText);
}

TextObject* ClassificationLayerImp::getBottomText() const
{
   return dynamic_cast<TextObject*>(mpBottomText);
}

vector<ColorType> ClassificationLayerImp::getColors() const
{
   vector<ColorType> colors;

   QColor currentColor = getClassificationColor();
   if (currentColor.isValid() == true)
   {
      ColorType color(currentColor.red(), currentColor.green(), currentColor.blue());
      colors.push_back(color);
   }

   return colors;
}

void ClassificationLayerImp::draw()
{
   PerspectiveView *pView = dynamic_cast<PerspectiveView*>(getView());
   double zoomPercent = 100;
   if (pView != NULL)
   {
      zoomPercent = pView->getZoomPercentage();
   }

   if (mpTopText)
   {
      mpTopText->draw(zoomPercent/100);
   }
   if (mpBottomText)
   {
      mpBottomText->draw(zoomPercent/100);
   }
}

void ClassificationLayerImp::setClassificationFont(const QFont& classificationFont)
{
   if (classificationFont == mClassificationFont.getQFont())
   {
      return;
   }

   if (mbLinking == false)
   {
      mClassificationFont = classificationFont;

      // Ensure the undo lock is removed before emitting the signal
      {
         UndoLock lock(getView());
         mpTopText->setFont(classificationFont);
         mpBottomText->setFont(classificationFont);
      }

      emit fontChanged(classificationFont);
      notify(SIGNAL_NAME(ClassificationLayer, FontChanged), boost::any(mClassificationFont));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         ClassificationLayerImp* pLayer = dynamic_cast<ClassificationLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setClassificationFont(classificationFont);
         }

         ++iter;
      }

      mbLinking = false;
   }
}

void ClassificationLayerImp::setClassificationColor(const QColor& classificationColor)
{
   if (classificationColor.isValid() == false)
   {
      return;
   }

   if (classificationColor == mClassificationColor)
   {
      return;
   }

   if (mbLinking == false)
   {
      mClassificationColor = classificationColor;

      // Ensure the undo lock is removed before emitting the signal
      ColorType textColor(mClassificationColor.red(), mClassificationColor.green(), mClassificationColor.blue());
      {
         UndoLock lock(getView());
         mpTopText->setTextColor(textColor);
         mpBottomText->setTextColor(textColor);
      }

      emit colorChanged(mClassificationColor);
      notify(SIGNAL_NAME(ClassificationLayer, ColorChanged), boost::any(textColor));

      mbLinking = true;

      vector<Layer*> linkedLayers = getLinkedLayers();

      vector<Layer*>::iterator iter = linkedLayers.begin();
      while (iter != linkedLayers.end())
      {
         ClassificationLayerImp* pLayer = dynamic_cast<ClassificationLayerImp*>(*iter);
         if (pLayer != NULL)
         {
            pLayer->setClassificationColor(classificationColor);
         }

         ++iter;
      }

      mbLinking = false;
   }
}

const FontImp& ClassificationLayerImp::getClassificationFontImp() const
{
   return mClassificationFont;
}

void ClassificationLayerImp::updateProperties(GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return;
   }

   string propertyName = pProperty->getName();
   if (propertyName == "Font")
   {
      QFont textFont = (dynamic_cast<FontProperty*>(pProperty))->getFont().toQFont();
      setClassificationFont(textFont);
   }
   else if (propertyName == "TextColor")
   {
      ColorType textColor = ((TextColorProperty*) pProperty)->getColor();
      QColor clrText(textColor.mRed, textColor.mGreen, textColor.mBlue);
      setClassificationColor(clrText);
   }
}

bool ClassificationLayerImp::getShowLabels() const
{
   // Classification does not need to do anything
   return false;
}

void ClassificationLayerImp::setShowLabels(bool bShowLabels)
{
   // Classification does not need to do anything
   return;
}

void ClassificationLayerImp::reset()
{
   // Classification has nothing to reset
   return;
}

bool ClassificationLayerImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   if (!AnnotationLayerImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("classificationColor", mClassificationColor.name().toStdString());

   pXml->addFontElement("ClassificationFont", mClassificationFont);

   return true;
}

bool ClassificationLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!AnnotationLayerImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement *pRootElement = dynamic_cast<DOMElement *>(pDocument);
   mClassificationColor = QColor(A(pRootElement->getAttribute(X("classificationColor"))));
   readFontElement("ClassificationFont", pRootElement, mClassificationFont);
   return true;
}
