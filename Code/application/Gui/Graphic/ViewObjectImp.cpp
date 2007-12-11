/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QMoveEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QVBoxLayout>

#include "AppVerify.h"
#include "ClassificationAdapter.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicViewWidget.h"
#include "PerspectiveViewImp.h"
#include "SessionManager.h"
#include "TextObjectAdapter.h"
#include "View.h"
#include "ViewImp.h"
#include "ViewObject.h"
#include "ViewObjectImp.h"
#include "xmlwriter.h"

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

ViewObjectImp::ViewObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                             LocationType pixelCoord) :
      RectangleObjectImp(id, type, pLayer, pixelCoord), mpView(NULL), mpInvalidText(NULL), mUpdateBounds(false)
{
   // Create the text object for no associated view
   mpInvalidText = new TextObjectAdapter(SessionItemImp::generateUniqueId(), TEXT_OBJECT, pLayer, pixelCoord);
   if (mpInvalidText != NULL)
   {
      mpInvalidText->setText("View not\navailable");
   }

   // Set the fill color
   ColorType fillColor = View::getSettingBackgroundColor();
   if (fillColor.isValid() == true)
   {
      setFillColor(fillColor);
   }

   // Set line properties
   setLineColor(ColorType(0, 0, 0));
   setLineWidth(1.0);
   setLineStyle(SOLID_LINE);
   setLineState(true);
}

ViewObjectImp::~ViewObjectImp()
{
   if (mpView != NULL)
   {
      delete mpView;
   }

   if (mpInvalidText != NULL)
   {
      delete mpInvalidText;
   }
}

void ViewObjectImp::setView(View* pView)
{
   if (pView == dynamic_cast<View*>(mpView))
   {
      return;
   }

   if (mpView != NULL)
   {
      VERIFYNR(disconnect(mpView, SIGNAL(backgroundColorChanged(const QColor&)), this,
         SLOT(setBackgroundColor(const QColor&))));
      VERIFYNR(disconnect(mpView, SIGNAL(refreshRegistered()), this, SLOT(refresh())));
      VERIFYNR(disconnect(mpView, SIGNAL(refreshed()), this, SLOT(redraw())));

      View* pOldView = dynamic_cast<View*>(mpView);
      if (pOldView != NULL)
      {
         notify(SIGNAL_NAME(ViewObject, ViewDeleted), boost::any(pOldView));
      }

      delete mpView;
      mpView = NULL;
   }

   ColorType color = View::getSettingBackgroundColor();
   QColor backgroundColor = COLORTYPE_TO_QCOLOR(color);

   if (pView != NULL)
   {
      ViewImp* pParentView = NULL;
      QGLContext* pParentContext = NULL;

      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         pParentView = dynamic_cast<ViewImp*>(pLayer->getView());
         if (pParentView != NULL)
         {
            pParentContext = const_cast<QGLContext*>(pParentView->context());
         }
      }

      ViewImp* pViewImp = dynamic_cast<ViewImp*>(pView);

      mpView = dynamic_cast<ViewImp*>(pViewImp->copy(pParentContext, pParentView));
      if (mpView != NULL)
      {
         PerspectiveViewImp *pPerspectiveView = dynamic_cast<PerspectiveViewImp*>(mpView);
         if (pPerspectiveView)
         {
            pPerspectiveView->setAllowZoomOnResize(true);
         }

         // Notify attached objects
         View* pNewView = dynamic_cast<View*>(mpView);
         if (pNewView != NULL)
         {
            notify(SIGNAL_NAME(ViewObject, ViewCreated), boost::any(pNewView));
         }

         // Initialization
         backgroundColor = mpView->getBackgroundColor();
         mpView->enableClassification(false);
         mpView->enableReleaseInfo(false);
         mpView->blockUndo();
         mpView->hide();

         // Update classification string.
         // If new object has a higher level than the existing product,
         // use the new classifications string.
         QString classTextNew = pViewImp->getClassificationText();

         // strip off everything but the level
         QString classTextLevelNew = classTextNew.left(classTextNew.indexOf('/'));

         ClassificationAdapter classNew;
         classNew.setLevel(classTextLevelNew.toStdString());

         if (pLayer != NULL && pParentView != NULL)
         {
            QString classTextOrig = pParentView->getClassificationText();

            // strip off everything but the level
            QString classTextLevelOrig = classTextOrig.left(classTextOrig.indexOf('/'));

            ClassificationAdapter classOrig;
            classOrig.setLevel(classTextLevelOrig.toStdString());
            if (classNew.hasGreaterLevel(&classOrig))
            {
               pParentView->setClassificationText(classTextNew);
            }
         }

         mUpdateBounds = true;

         // Connections
         VERIFYNR(connect(mpView, SIGNAL(backgroundColorChanged(const QColor&)), this,
            SLOT(setBackgroundColor(const QColor&))));
         VERIFYNR(connect(mpView, SIGNAL(refreshRegistered()), this, SLOT(refresh())));
         VERIFYNR(connect(mpView, SIGNAL(refreshed()), this, SLOT(redraw())));
      }
   }

   setBackgroundColor(backgroundColor);
   refresh();
}

View* ViewObjectImp::getView() const
{
   return dynamic_cast<View*>(mpView);
}

void ViewObjectImp::draw(double zoomFactor) const
{
   RectangleObjectImp::draw(zoomFactor);

   // Get the current matrices
   double modelMatrix[16], projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   // Get the current location, width, and height in screen coordinates
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   if (llCorner == urCorner)
   {
      return;
   }

   double dScreenStartX = 0.0;
   double dScreenStartY = 0.0;
   double dScreenEndX = 0.0;
   double dScreenEndY = 0.0;
   GLdouble winZ;

   gluProject(llCorner.mX, llCorner.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
      &dScreenStartX, &dScreenStartY, &winZ);
   gluProject(urCorner.mX, urCorner.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
      &dScreenEndX, &dScreenEndY, &winZ);

   if (dScreenStartX > dScreenEndX)
   {
      double dTemp = dScreenStartX;
      dScreenStartX = dScreenEndX;
      dScreenEndX = dTemp;
   }

   if (dScreenStartY > dScreenEndY)
   {
      double dTemp = dScreenStartY;
      dScreenStartY = dScreenEndY;
      dScreenEndY = dTemp;
   }

   double dWidth = fabs(dScreenEndX - dScreenStartX);
   double dHeight = fabs(dScreenEndY - dScreenStartY);

   // Set the scissor box to only draw within the object
   glEnable(GL_SCISSOR_TEST);
   glScissor(dScreenStartX, dScreenStartY, dWidth, dHeight);

   // Draw the view or the invalid text
   if (mpView != NULL)
   {
      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         ViewImp* pView = dynamic_cast<ViewImp*>(pLayer->getView());
         if (pView != NULL)
         {
            // Create events to update the position and dimensions of the view
            // since Qt does not do this for hidden widgets
            QPoint newPos(dScreenStartX, pView->height() - dScreenEndY);
            QSize newSize(dWidth, dHeight);
            if (mUpdateBounds || newPos != mpView->pos() || newSize != mpView->size())
            {
               mUpdateBounds = false;
               QMoveEvent viewMoveEvent(newPos, mpView->pos());
               QResizeEvent viewResizeEvent(newSize, mpView->size());

               // Set the view widget position to the bounding box
               mpView->setGeometry(dScreenStartX, pView->height() - dScreenEndY, dWidth, dHeight);

               // Send the events to the view to update the matrices
               QApplication::sendEvent(mpView, &viewMoveEvent);
               QApplication::sendEvent(mpView, &viewResizeEvent);
            }

            // Set the viewport to draw to the object
            glViewport(dScreenStartX, dScreenStartY, dWidth, dHeight);

            // Draw the view
            mpView->draw();
         }
      }
   }
   else if (mpInvalidText != NULL)
   {
      mpInvalidText->draw(zoomFactor);
   }

   glDisable(GL_SCISSOR_TEST);

   // Restore matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
}

bool ViewObjectImp::setProperty(const GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   bool bSuccess = RectangleObjectImp::setProperty(pProperty);
   if (bSuccess == true)
   {
      string propertyName = pProperty->getName();
      if (propertyName == "BoundingBox")
      {
         if (mpInvalidText != NULL)
         {
            // Get the current text center location
            LocationType llCorner = mpInvalidText->getLlCorner();
            LocationType urCorner = mpInvalidText->getUrCorner();
            LocationType center((llCorner.mX + urCorner.mX) / 2.0, (llCorner.mY + urCorner.mY) / 2.0);

            // Calculate the distance from the new object center to the old text center
            LocationType newLlCorner = getLlCorner();
            LocationType newUrCorner = getUrCorner();
            LocationType newCenter((newLlCorner.mX + newUrCorner.mX) / 2.0, (newLlCorner.mY + newUrCorner.mY) / 2.0);

            LocationType delta = newCenter - center;

            // Move the text object to the center of the view object
            mpInvalidText->move(delta);
         }
      }
      else if (propertyName == "FillColor")
      {
         // Update the text color if the background color changed
         updateTextColor();
      }
   }

   return bSuccess;
}

bool ViewObjectImp::replicateObject(const GraphicObject* pObject)
{
   const ViewObjectImp *pObjectView = dynamic_cast<const ViewObjectImp*>(pObject);
   if ((pObjectView == NULL) || (pObjectView == this))
   {
      return false;
   }

   bool bSuccess = RectangleObjectImp::replicateObject(pObject);
   if (bSuccess == true)
   {
      TextObjectImp *pText = pObjectView->mpInvalidText;
      if (pText != NULL)
      {
         bSuccess = mpInvalidText->replicateObject(dynamic_cast<GraphicObject*>(pText));
      }

      View* pView = pObjectView->getView();
      setView(pView);
   }

   return bSuccess;
}

void ViewObjectImp::setBackgroundColor(const QColor& clrBackground)
{
   if (clrBackground.isValid() == false)
   {
      return;
   }

   ColorType currentColor = getFillColor();
   ColorType backgroundColor(clrBackground.red(), clrBackground.green(), clrBackground.blue());

   if (backgroundColor != currentColor)
   {
      setFillColor(backgroundColor);
   }
}

void ViewObjectImp::updateTextColor()
{
   if (mpInvalidText == NULL)
   {
      return;
   }

   // Get the fill color
   ColorType fillColor = getFillColor();
   if (fillColor.isValid() == false)
   {
      return;
   }

   // Set the text color as the inverse of the fill color
   QColor clrText(fillColor.mRed, fillColor.mGreen, fillColor.mBlue);

   int h, s, v;
   clrText.getHsv(&h, &s, &v);
   if (h == -1)
   {
      v = (v + 180) % 360;
   }
   else
   {
      h = (h + 180) % 360;
   }

   clrText.setHsv(h, s, v);

   ColorType textColor(clrText.red(), clrText.green(), clrText.blue());
   mpInvalidText->setTextColor(textColor);
}

void ViewObjectImp::refresh()
{
   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}

void ViewObjectImp::redraw()
{
   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      ViewImp* pView = dynamic_cast<ViewImp*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->updateGL();
      }
   }
}

bool ViewObjectImp::processMouseRelease(LocationType screenCoord, 
                                        Qt::MouseButton button,
                                        Qt::MouseButtons buttons,
                                        Qt::KeyboardModifiers modifiers)
{
   bool used = false;
   if (button == Qt::LeftButton)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      setBoundingBox(getLlCorner(), screenCoord);
      GraphicLayerImp *pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayerImp != NULL)
      {
         QDialog dialog(dynamic_cast<ViewImp*>(pLayerImp->getView()));

         GraphicViewWidget* pViewWidget = new GraphicViewWidget(&dialog);
         QDialogButtonBox* pButtonBox = new QDialogButtonBox(&dialog);
         pButtonBox->setOrientation(Qt::Horizontal);
         pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

         QVBoxLayout* pLayout = new QVBoxLayout(&dialog);
         pLayout->setMargin(10);
         pLayout->setSpacing(10);
         pLayout->addWidget(pViewWidget);
         pLayout->addStretch();
         pLayout->addWidget(pButtonBox, 0, Qt::AlignRight);

         dialog.setWindowTitle("Select View Image");
         dialog.resize(300, 65);

         VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &dialog, SLOT(accept())));
         VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &dialog, SLOT(reject())));

         if (dialog.exec() == QDialog::Rejected)
         {
            pLayerImp->completeInsertion(false);
            return false;
         }

         View* pView = pViewWidget->getView();
         if (pView != NULL)
         {
            setView(pView);
         }

         pLayerImp->completeInsertion();
      }
      used = true;
   }

   return used;
}

const string& ViewObjectImp::getObjectType() const
{
   static string type("ViewObjectImp");
   return type;
}

bool ViewObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ViewObject"))
   {
      return true;
   }

   return RectangleObjectImp::isKindOf(className);
}

bool ViewObjectImp::toXml(XMLWriter *pWriter) const
{
   if (pWriter == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::toXml(pWriter);
   if (bSuccess == true)
   {
      if (mpView != NULL)
      {
         Service<SessionManager> pSession;
         if (pSession->isSessionSaving() == true)
         {
            pWriter->addAttr("viewId", mpView->getId());
         }
      }
      else if (mpInvalidText != NULL)
      {
         pWriter->pushAddPoint(pWriter->addElement("objects"));
         pWriter->pushAddPoint(pWriter->addElement("Graphic"));

         bSuccess = mpInvalidText->toXml(pWriter);

         pWriter->popAddPoint();
         pWriter->popAddPoint();
      }
   }

   return bSuccess;
}

bool ViewObjectImp::fromXml(DOMNode *pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::fromXml(pDocument, version);
   if (bSuccess == false)
   {
      return false;
   }

   if (mpInvalidText != NULL)
   {
      DOMNode* pObjectNode = NULL;
      for (pObjectNode = pDocument->getFirstChild(); pObjectNode != NULL; pObjectNode = pObjectNode->getNextSibling())
      {
         if (XMLString::equals(pObjectNode->getNodeName(), X("objects")))
         {
            DOMNode* pTextNode = NULL;
            for (pTextNode = pObjectNode->getFirstChild(); pTextNode != NULL; pTextNode = pTextNode->getNextSibling())
            {
               if (XMLString::equals(pTextNode->getNodeName(), X("Graphic")))
               {
                  DOMElement* pElement(static_cast<DOMElement*>(pTextNode));
                  string type(A(pElement->getAttribute(X("type"))));

                  GraphicObjectType objectType = StringUtilities::fromXmlString<GraphicObjectType>(type);
                  if (objectType == TEXT_OBJECT)
                  {
                     bSuccess = mpInvalidText->fromXml(pTextNode, version);
                     break;
                  }
               }
            }
         }
      }
   }

   if (bSuccess == true)
   {
      Service<SessionManager> pSession;
      if (pSession->isSessionLoading() == true)
      {
         bSuccess = false;

         DOMElement *pElem = dynamic_cast<DOMElement*>(pDocument);
         if (pElem)
         {
            string id(A(pElem->getAttribute(X("viewId"))));
            if (id.empty() == false)
            {
               View* pView = dynamic_cast<View*>(pSession->getSessionItem(id));
               if (pView != NULL)
               {
                  setView(pView);
                  delete dynamic_cast<ViewImp*>(pView);
                  bSuccess = true;
               }
            }
         }
      }
   }

   return bSuccess;
}
