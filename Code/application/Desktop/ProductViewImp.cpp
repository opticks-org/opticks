/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "glCommon.h"
#include "AnnotationElementAdapter.h"
#include "AnnotationLayerAdapter.h"
#include "ApplicationWindow.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "ClassificationLayerAdapter.h"
#include "ConfigurationSettingsImp.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DataDescriptorAdapter.h"
#include "DesktopServices.h"
#include "FileResource.h"
#include "GraphicLayerUndo.h"
#include "GraphicObject.h"
#include "GraphicProperty.h"
#include "ModelServicesImp.h"
#include "MouseModeImp.h"
#include "ProductViewAdapter.h"
#include "ProductViewImp.h"
#include "ProductViewUndo.h"
#include "PropertiesProductView.h"
#include "SessionManager.h"
#include "StatusBar.h"
#include "StringUtilities.h"
#include "TextObject.h"
#include "TextObjectImp.h"
#include "Undo.h"
#include "UtilityServicesImp.h"
#include "View.h"
#include "ViewImp.h"
#include "xmlwriter.h"

#include <list>
#include <map>
#include <string>

#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>

using namespace std;
XERCES_CPP_NAMESPACE_USE

ProductViewImp::ProductViewImp(const string& id, const string& viewName, QGLContext* drawContext,
                               QWidget* parent) :
   PerspectiveViewImp(id, viewName, drawContext, parent),
   mpExplorer(Service<SessionExplorer>().get(), SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &ProductViewImp::updateContextMenu)),
   mPaperWidth(0.0),
   mPaperHeight(0.0),
   mPaperColor(Qt::white),
   mDpi(100),
   mpLayoutLayer(NULL),
   mpClassificationLayer(NULL),
   mpActiveLayer(NULL),
   mpEditObject(NULL),
   mbViewEvent(false),
   mbZoomInitialized(false)
{
   DataDescriptorAdapter layoutDescriptor("Layout", "AnnotationElement", NULL);
   mpLayoutLayer = new AnnotationLayerAdapter(SessionItemImp::generateUniqueId(), "Layout",
      new AnnotationElementAdapter(layoutDescriptor, SessionItemImp::generateUniqueId()));
   mpLayoutLayer->setView(this);

   DataDescriptorAdapter classificationDescriptor("Classification", "AnnotationElement", NULL);
   mpClassificationLayer = new ClassificationLayerAdapter(SessionItemImp::generateUniqueId(), "Classification",
      new AnnotationElementAdapter(classificationDescriptor, SessionItemImp::generateUniqueId()));
   mpClassificationLayer->setView(this);

   // Initialization
   blockUndo();
   TextObject* pTopText = mpClassificationLayer->getTopText();
   if (pTopText != NULL)
   {
      pTopText->setTextAlignment(Qt::AlignHCenter);
   }

   TextObject* pBottomText = mpClassificationLayer->getBottomText();
   if (pBottomText != NULL)
   {
      pBottomText->setTextAlignment(Qt::AlignHCenter);
   }
   VERIFYNR(connect(this, SIGNAL(classificationChanged(const QString&)), this, SLOT(updateClassificationMarks(const QString&))));
   updateClassificationMarks(getClassificationText()); // Sets the text in the classification layer
   setClassificationFont(QApplication::font());
   setClassificationColor(Qt::black);

   enableClassification(false);
   enableReleaseInfo(false);
   enableCrossHair(false);
   setActiveLayer(dynamic_cast<GraphicLayer*>(mpLayoutLayer));
   addPropertiesPage(PropertiesProductView::getName());
   unblockUndo();

   // Separator
   QAction* pPropertiesSeparatorAction = new QAction(this);
   pPropertiesSeparatorAction->setSeparator(true);
   addContextMenuAction(ContextMenuAction(pPropertiesSeparatorAction, APP_PRODUCTVIEW_PROPERTIES_SEPARATOR_ACTION));
   setIcon(QIcon(":/icons/SpectralData"));
   setWindowIcon(QIcon(":/icons/SpectralData"));
   addMouseMode(new MouseModeImp("LayerMode", QCursor(Qt::ArrowCursor)));
   addMouseMode(new MouseModeImp("PanMode", QCursor(Qt::OpenHandCursor)));
   addMouseMode(new MouseModeImp("ZoomInMode", QCursor(QPixmap(":/icons/ZoomInCursor", 0, 0))));
   addMouseMode(new MouseModeImp("ZoomOutMode", QCursor(QPixmap(":/icons/ZoomOutCursor", 0, 0))));
   addMouseMode(new MouseModeImp("ZoomBoxMode", QCursor(QPixmap(":/icons/ZoomRectCursor", 0, 0))));

   // Connections
   VERIFYNR(connect(this, SIGNAL(mouseModeChanged(const MouseMode*)), this, SLOT(updateMouseCursor(const MouseMode*))));
   VERIFYNR(connect(mpLayoutLayer, SIGNAL(extentsModified()), this, SLOT(updateExtents())));
   connectLayers();

   // Initialization after connections so that slot methods are called
   setMouseMode("LayerMode");
}

void ProductViewImp::connectLayers()
{
   connect(mpLayoutLayer, SIGNAL(modified()), this, SLOT(refresh()));
   connect(mpClassificationLayer, SIGNAL(fontChanged(const QFont&)),
      this, SLOT(setClassificationFont(const QFont&)));
   connect(mpClassificationLayer, SIGNAL(colorChanged(const QColor&)), this,
      SLOT(setClassificationColor(const QColor&)));
   connect(mpClassificationLayer, SIGNAL(modified()), this, SLOT(refresh()));

   TextObject* pTopText = mpClassificationLayer->getTopText();
   if (pTopText != NULL)
   {
      connect(dynamic_cast<TextObjectImp*>(pTopText), 
         SIGNAL(propertyModified(GraphicProperty*)), this, SLOT(updateClassificationLocation()));
   }

   TextObject* pBottomText = mpClassificationLayer->getBottomText();
   if (pBottomText != NULL)
   {
      connect(dynamic_cast<TextObjectImp*>(pBottomText),
         SIGNAL(propertyModified(GraphicProperty*)), this, SLOT(updateClassificationLocation()));
   }
}

ProductViewImp::~ProductViewImp()
{
   // Delete the layers
   if (mpLayoutLayer != NULL)
   {
      // Destroy the element because the layer will not delete it since it does not exist in the model
      DataElementImp* pElement = dynamic_cast<DataElementImp*>(mpLayoutLayer->getDataElement());

      delete pElement;
      delete mpLayoutLayer;
   }

   if (mpClassificationLayer != NULL)
   {
      // Destroy the element because the layer will not delete it since it does not exist in the model
      DataElementImp* pElement = dynamic_cast<DataElementImp*>(mpClassificationLayer->getDataElement());

      delete pElement;
      delete mpClassificationLayer;
   }

   // Destroy the mouse modes
   deleteMouseMode(getMouseMode("LayerMode"));
   deleteMouseMode(getMouseMode("PanMode"));
   deleteMouseMode(getMouseMode("ZoomInMode"));
   deleteMouseMode(getMouseMode("ZoomOutMode"));
   deleteMouseMode(getMouseMode("ZoomBoxMode"));
}

const string& ProductViewImp::getObjectType() const
{
   static string type("ProductViewImp");
   return type;
}

bool ProductViewImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ProductView"))
   {
      return true;
   }

   return PerspectiveViewImp::isKindOf(className);
}

bool ProductViewImp::isKindOfView(const string& className)
{
   if ((className == "ProductViewImp") || (className == "ProductView"))
   {
      return true;
   }

   return PerspectiveViewImp::isKindOfView(className);
}

void ProductViewImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("ProductView");
   PerspectiveViewImp::getViewTypes(classList);
}

ProductViewImp& ProductViewImp::operator= (const ProductViewImp& productView)
{
   if (this != &productView)
   {
      PerspectiveViewImp::operator= (productView);

      mPaperWidth = productView.mPaperWidth;
      mPaperHeight = productView.mPaperHeight;
      mPaperColor = productView.mPaperColor;
      mDpi = productView.mDpi;
      *mpLayoutLayer = *(productView.mpLayoutLayer);
      *mpClassificationLayer = *(productView.mpClassificationLayer);

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

View* ProductViewImp::copy(QGLContext* drawContext, QWidget* parent) const
{
   string viewName = getName();

   ProductViewAdapter* pView = new ProductViewAdapter(SessionItemImp::generateUniqueId(), viewName,
      drawContext, parent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *(static_cast<ProductViewImp*>(pView)) = *this;
   }

   return static_cast<View*>(pView);
}

bool ProductViewImp::copy(View *pView) const
{
   ProductViewImp* pViewImp = dynamic_cast<ProductViewImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

ViewType ProductViewImp::getViewType() const
{
   return PRODUCT_VIEW;
}

void ProductViewImp::getPaperSize(double& dWidth, double& dHeight) const
{
   dWidth = mPaperWidth;
   dHeight = mPaperHeight;
}

QColor ProductViewImp::getPaperColor() const
{
   return mPaperColor;
}

QImage ProductViewImp::getPaperImage()
{
   // Save matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   // Prevent undo actions from being registered
   ProductView* pView = dynamic_cast<ProductView*>(this);
   UndoLock lock(pView);

   // Zoom extents to ensure all paper contents are drawn
   double dZoomPercent = getZoomPercentage();
   LocationType center = getVisibleCenter();

   zoomExtents();

   // Update view object zoom percentages to maintain the same view to paper zoom ratio
   map<GraphicObject*, double> viewZoomPercents;
   double dExtentsPercent = getZoomPercentage();

   list<GraphicObject*> viewObjects;
   if (mpLayoutLayer != NULL)
   {
      viewObjects = mpLayoutLayer->getObjects(VIEW_OBJECT);
   }

   list<GraphicObject*>::iterator iter = viewObjects.begin();
   while (iter != viewObjects.end())
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         PerspectiveView* pPerspectiveView = static_cast<PerspectiveView*>(pObject->getObjectView());
         if (pPerspectiveView != NULL)
         {
            double dViewPercent = pPerspectiveView->getZoomPercentage();
            viewZoomPercents[pObject] = dViewPercent;

            UndoLock viewLock(pPerspectiveView);
            pPerspectiveView->zoomTo(dViewPercent * (dExtentsPercent / dZoomPercent));
         }
      }

      ++iter;
   }

   // Get the paper location in screen coordinates
   double dPaperScreenStartX = 0.0;
   double dPaperScreenStartY = 0.0;
   double dPaperScreenEndX = 0.0;
   double dPaperScreenEndY = 0.0;
   translateWorldToScreen(0.0, 0.0, dPaperScreenStartX, dPaperScreenStartY);
   translateWorldToScreen(mPaperWidth * mDpi, mPaperHeight * mDpi, dPaperScreenEndX, dPaperScreenEndY);

   // handle case where OpenGL and the app view origins are different
   if (dPaperScreenStartY > dPaperScreenEndY)
   {
      double temp = dPaperScreenStartY;
      dPaperScreenStartY = dPaperScreenEndY;
      dPaperScreenEndY = temp;
   }

   int iWidth = abs(static_cast<int>(dPaperScreenEndX - dPaperScreenStartX));
   int iHeight = abs(static_cast<int>(dPaperScreenEndY - dPaperScreenStartY));

   // Draw the layers in the back buffer
   glEnable(GL_SCISSOR_TEST);
   glScissor(dPaperScreenStartX, dPaperScreenStartY, iWidth, iHeight);
   glDrawBuffer(GL_BACK);
   glReadBuffer(GL_BACK);
   glFlush();
   qglClearColor(mPaperColor);
   glClear(GL_COLOR_BUFFER_BIT);
   drawLayers();
   glDisable(GL_SCISSOR_TEST);

   // Restore the zoom to the original values
   zoomToPoint(center, dZoomPercent);

   iter = viewObjects.begin();
   while (iter != viewObjects.end())
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         map<GraphicObject*, double>::iterator valueIter = viewZoomPercents.find(pObject);
         if (valueIter != viewZoomPercents.end())
         {
            PerspectiveView* pPerspectiveView = static_cast<PerspectiveView*>(pObject->getObjectView());
            if (pPerspectiveView != NULL)
            {
               double dViewPercent = valueIter->second;

               UndoLock viewLock(pPerspectiveView);
               pPerspectiveView->zoomTo(dViewPercent);
            }
         }
      }

      ++iter;
   }

   // Restore matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

   // Read the pixels from the draw buffer
   vector<unsigned int> pixels(iWidth * iHeight);
   glReadPixels(dPaperScreenStartX, dPaperScreenStartY, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE,
      reinterpret_cast<GLvoid*>(&pixels[0]));

   ViewImp::reorderImage(&pixels[0], iWidth, iHeight);

   // Create the image from the pixel data
   QImage image(iWidth, iHeight, QImage::Format_ARGB32);

   unsigned char* pBits = NULL;
   pBits = image.bits();
   if (pBits != NULL)
   {
      memcpy(pBits, &pixels[0], iWidth * iHeight * 4);
   }

   return image;
}

unsigned int ProductViewImp::getDpi() const
{
   return mDpi;
}

bool ProductViewImp::loadTemplate(const QString& strTemplateFile)
{
   if (strTemplateFile.isEmpty() == true)
   {
      return false;
   }

   UndoGroup group(dynamic_cast<View*>(this), "Load Template");

   bool bSuccess = false;
   XmlReader xml(Service<MessageLogMgr>()->getLog());
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(strTemplateFile.toStdString());
   if (pDocument != NULL)
   {
      DOMElement* pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         // Clear the layout layer
         mpLayoutLayer->selectAllObjects();
         mpLayoutLayer->deleteSelectedObjects();

         try
         {
            // Older template files (4.3.X and prior) had "AnnotationLayer", not "ProductTemplate"
            if (XMLString::equals(pRootElement->getNodeName(), X("ProductTemplate")))
            {
               setPaperSize(StringUtilities::fromXmlString<double>(A(pRootElement->getAttribute(X("paperWidth")))),
                  StringUtilities::fromXmlString<double>(A(pRootElement->getAttribute(X("paperHeight")))));
               setPaperColor(COLORTYPE_TO_QCOLOR(
                  StringUtilities::fromXmlString<ColorType>(A(pRootElement->getAttribute(X("paperColor"))))));
               setDpi(StringUtilities::fromXmlString<unsigned int>(A(pRootElement->getAttribute(X("dpi")))));
            }

            bSuccess = mpLayoutLayer->fromXml(pRootElement, atoi(A(pRootElement->getAttribute(X("version")))));
         }
         catch (XmlReader::DomParseException& exc)
         {
            QMessageBox::critical(this, APP_NAME, QString::fromStdString(exc.str()));
            return false;
         }

      }
   }

   if (bSuccess == false)
   {
      QMessageBox::critical(this, APP_NAME, "The file '" + strTemplateFile + "' could not be loaded!");
      return false;
   }

   // Perform a zoom to fit
   zoomExtents();

   // Get the classification text object
   TextObjectImp* pClassificationObject = NULL;

   list<GraphicObject*> textObjects = mpLayoutLayer->getObjects(TEXT_OBJECT);

   list<GraphicObject*>::iterator iter = textObjects.begin();
   while (iter != textObjects.end())
   {
      TextObject* pObject = NULL;
      pObject = dynamic_cast<TextObject*>(*iter);
      if (pObject != NULL)
      {
         string objectText = pObject->getText();
         if (objectText == "ClassificationLabel")
         {
            pClassificationObject = dynamic_cast<TextObjectImp*>(pObject);
            break;
         }
      }

      ++iter;
   }

   // Update the classification properties
   if (pClassificationObject != NULL)
   {
      // Alignment
      int iAlignment = pClassificationObject->getTextAlignment();

      TextObject* pTopText = mpClassificationLayer->getTopText();
      if (pTopText != NULL)
      {
         pTopText->setTextAlignment(iAlignment);
      }

      TextObject* pBottomText = mpClassificationLayer->getBottomText();
      if (pBottomText != NULL)
      {
         pBottomText->setTextAlignment(iAlignment);
      }

      // Font
      QFont classificationFont = pClassificationObject->getFont();
      mpClassificationLayer->setClassificationFont(classificationFont);

      // Color
      ColorType classificationColor = pClassificationObject->getTextColor();
      QColor clrClassification(classificationColor.mRed, classificationColor.mGreen, classificationColor.mBlue);
      mpClassificationLayer->setClassificationColor(clrClassification);

      // Remove the classification object from the layout layer
      mpLayoutLayer->removeObject(dynamic_cast<GraphicObject*>(pClassificationObject), true);
   }

   return true;
}

bool ProductViewImp::saveTemplate(const QString& strTemplateFile) const
{
   if ((strTemplateFile.isEmpty() == true) || (mpLayoutLayer == NULL))
   {
      return false;
   }

   string filename = strTemplateFile.toStdString();
   FileResource pFile(filename.c_str(), "wt", true);
   if (pFile.get() == NULL)
   {
      QString strError = "Could not open the template file for writing:\n" + strTemplateFile;
      QMessageBox::critical(const_cast<ProductViewImp*>(this), APP_NAME, strError);
      return false;
   }

   // Add a text object to the layout layer to store the classification text font
   TextObjectImp* pClassificationObject = 
      dynamic_cast<TextObjectImp*>(mpLayoutLayer->addObject(TEXT_OBJECT, LocationType()));
   if (pClassificationObject != NULL)
   {
      // Set unique text to identify it on load
      pClassificationObject->setText("ClassificationLabel");

      // Alignment
      TextObject* pTopText = mpClassificationLayer->getTopText();
      if (pTopText != NULL)
      {
         int iAlignment = pTopText->getTextAlignment();
         pClassificationObject->setTextAlignment(iAlignment);
      }

      // Font
      QFont classificationFont = mpClassificationLayer->getClassificationFont();
      pClassificationObject->setFont(classificationFont);

      // Color
      QColor classificationColor = mpClassificationLayer->getClassificationColor();
      ColorType textColor(classificationColor.red(), classificationColor.green(), classificationColor.blue());
      pClassificationObject->setTextColor(textColor);

      // Update the texture and reset to bounding box to hide the object
      LocationType noSize;
      pClassificationObject->updateTexture();
      pClassificationObject->setBoundingBox(noSize, noSize);
   }

   XMLWriter xml("ProductTemplate");
   bool success = xml.addAttr("paperWidth", mPaperWidth) && xml.addAttr("paperHeight", mPaperHeight) &&
      xml.addAttr("paperColor", QCOLOR_TO_COLORTYPE(mPaperColor)) && xml.addAttr("dpi", mDpi) &&
      mpLayoutLayer->toXml(&xml);

   // Remove the classification label
   if (pClassificationObject != NULL)
   {
      mpLayoutLayer->removeObject(dynamic_cast<GraphicObject*>(pClassificationObject), true);
   }

   if (success)
   {
      xml.writeToFile(pFile);
      pFile.setDeleteOnClose(false);
      return true;
   }

   QMessageBox::critical(const_cast<ProductViewImp*>(this), APP_NAME, "Could not save the template file.");
   return false;
}

AnnotationLayer* ProductViewImp::getLayoutLayer() const
{
   return dynamic_cast<AnnotationLayer*>(mpLayoutLayer);
}

ClassificationLayer* ProductViewImp::getClassificationLayer() const
{
   return dynamic_cast<ClassificationLayer*>(mpClassificationLayer);
}

GraphicLayer* ProductViewImp::getActiveLayer() const
{
   return mpActiveLayer;
}

GraphicObject* ProductViewImp::getActiveEditObject() const
{
   return mpEditObject;
}

View* ProductViewImp::getActiveEditView() const
{
   View* pEditView = NULL;

   GraphicObject* pObject = NULL;
   pObject = getActiveEditObject();
   if (pObject != NULL)
   {
      pEditView = pObject->getObjectView();
   }

   return pEditView;
}

bool ProductViewImp::enableInset(bool bEnable)
{
   // Do not support an inset
   return false;
}

void ProductViewImp::updateClassificationMarks(const QString &newClassification)
{
   QString strTopText = newClassification;
   QString strBottomText = newClassification;

   Service<ConfigurationSettings> pConfigSettings;
   if (!strTopText.isEmpty())
   {
      strTopText.append("\n");
   }
   strTopText.append(QString::fromStdString(StringUtilities::toDisplayString(pConfigSettings->getReleaseType())));

   QString strReleaseDescription = QString::fromStdString(pConfigSettings->getReleaseDescription());
   if (!strReleaseDescription.isEmpty())
   {
      if (!strTopText.isEmpty())
      {
         strTopText.append("\n");
      }

      strTopText.append(strReleaseDescription);
   }

   if (!pConfigSettings->isProductionRelease())
   {
      strBottomText.prepend("Not for Production Use\n");
   }

   TextObject* pTopText = mpClassificationLayer->getTopText();
   if (pTopText != NULL)
   {
      pTopText->setText(strTopText.toStdString());
   }

   TextObject* pBottomText = mpClassificationLayer->getBottomText();
   if (pBottomText != NULL)
   {
      pBottomText->setText(strBottomText.toStdString());
   }
}

void ProductViewImp::setClassificationFont(const QFont& classificationFont)
{
   PerspectiveViewImp::setClassificationFont(classificationFont);
   mpClassificationLayer->setClassificationFont(classificationFont);
}

void ProductViewImp::setClassificationColor(const QColor& clrClassification)
{
   PerspectiveViewImp::setClassificationColor(clrClassification);
   mpClassificationLayer->setClassificationColor(clrClassification);
}

void ProductViewImp::rotateTo(double dDegrees)
{
   // Do nothing to disable rotating
}

void ProductViewImp::flipTo(double dDegrees)
{
   // Do nothing to disable flipping
}

void ProductViewImp::setPaperSize(double dWidth, double dHeight)
{
   if ((dWidth != mPaperWidth) || (dHeight != mPaperHeight))
   {
      addUndoAction(new SetPaperSize(dynamic_cast<ProductView*>(this), mPaperWidth, mPaperHeight, dWidth, dHeight));

      mPaperWidth = dWidth;
      mPaperHeight = dHeight;

      UndoLock lock(dynamic_cast<View*>(this));

      // Update the classification object position
      updateClassificationLocation();

      // Update the view extents
      updateExtents();
      zoomExtents();

      // Notify connected and attached objects
      emit paperSizeChanged(mPaperWidth, mPaperHeight);
      notify(SIGNAL_NAME(ProductView, PaperSizeChanged), boost::any(pair<double, double>(mPaperWidth, mPaperHeight)));
   }
}

void ProductViewImp::setPaperColor(const QColor& clrPaper)
{
   if (clrPaper.isValid() == false)
   {
      return;
   }

   if (clrPaper != mPaperColor)
   {
      addUndoAction(new SetPaperColor(dynamic_cast<ProductView*>(this), QCOLOR_TO_COLORTYPE(mPaperColor),
         QCOLOR_TO_COLORTYPE(clrPaper)));

      mPaperColor = clrPaper;

      emit paperColorChanged(mPaperColor);
      notify(SIGNAL_NAME(ProductView, PaperColorChanged), boost::any(
         ColorType(mPaperColor.red(), mPaperColor.green(), mPaperColor.blue())));
   }
}

void ProductViewImp::setDpi(unsigned int dpi)
{
   if (dpi != mDpi)
   {
      mDpi = dpi;
      emit dpiChanged(mDpi);
      notify(SIGNAL_NAME(ProductView, DpiChanged), boost::any(mDpi));
   }
}

void ProductViewImp::setActiveLayer(GraphicLayer* pLayer)
{
   if (pLayer == mpActiveLayer)
   {
      return;
   }

   if ((pLayer == dynamic_cast<GraphicLayer*>(mpLayoutLayer)) ||
      (pLayer == dynamic_cast<GraphicLayer*>(mpClassificationLayer)) || (pLayer == NULL))
   {
      mpActiveLayer = pLayer;
      emit layerActivated(mpActiveLayer);
      notify(SIGNAL_NAME(ProductView, LayerActivated), boost::any(static_cast<Layer*>(mpActiveLayer)));
   }
}

bool ProductViewImp::setActiveEditObject(GraphicObject* pObject)
{
   if (pObject == mpEditObject)
   {
      return false;
   }

   if (pObject != NULL)
   {
      // Ensure the object is in the layout layer
      if (mpLayoutLayer->hasObject(pObject) == false)
      {
         return false;
      }

      // Ensure the object is a view object
      if (pObject->getGraphicObjectType() != VIEW_OBJECT)
      {
         return false;
      }

      // Ensure the view is a spatial data view
      View* pView = NULL;
      pView = pObject->getObjectView();
      if (pView != NULL)
      {
         if (pView->isKindOf("SpatialDataView") == false)
         {
            return false;
         }
      }
   }

   // Update the member object and notify connected objects
   mpEditObject = pObject;

   View* pView = NULL;
   if (mpEditObject != NULL)
   {
      pView = mpEditObject->getObjectView();
   }

   emit editObjectChanged(mpEditObject);
   emit editViewChanged(pView);

   // Make sure all objects in the layout layer are not selected
   list<GraphicObject*> objects = mpLayoutLayer->getObjects();

   list<GraphicObject*>::iterator iter = objects.begin();
   while (iter != objects.end())
   {
      GraphicObject* pCurrentObject = NULL;
      pCurrentObject = *iter;
      if (pCurrentObject != NULL)
      {
         if (pCurrentObject == mpEditObject)
         {
            mpLayoutLayer->selectObject(pCurrentObject);
         }
         else
         {
            mpLayoutLayer->deselectObject(pCurrentObject);
         }
      }

      ++iter;
   }

   // Reset the active layer
   if (mpEditObject != NULL)
   {
      setActiveLayer(NULL);
   }
   else
   {
      setActiveLayer(dynamic_cast<GraphicLayer*>(mpLayoutLayer));
   }

   return true;
}

bool ProductViewImp::setActiveEditView(View* pView)
{
   GraphicObject* pEditObject = NULL;

   if (pView != NULL)
   {
      list<GraphicObject*> objects = mpLayoutLayer->getObjects(VIEW_OBJECT);

      list<GraphicObject*>::iterator iter = objects.begin();
      while (iter != objects.end())
      {
         GraphicObject* pObject = NULL;
         pObject = *iter;
         if (pObject != NULL)
         {
            View* pCurrentView = NULL;
            pCurrentView = pObject->getObjectView();
            if (pCurrentView == pView)
            {
               pEditObject = pObject;
               break;
            }
         }

         ++iter;
      }
   }

   return setActiveEditObject(pEditObject);
}

bool ProductViewImp::event(QEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return false;
   }

   if (pEvent->type() == QEvent::ToolTip)
   {
      QHelpEvent* pHelp = static_cast<QHelpEvent*>(pEvent);
      toolTipEvent(pHelp);
      return true;
   }

   // Call the base class first to update the view matrices
   bool bReturn = PerspectiveViewImp::event(pEvent);

   // Initialize the paper size in the Polish event instead of the constructor since
   // a virtual function is called in the classification layer's text objects
   Service<SessionManager> pManager;
   if ((pManager->isSessionLoading() == false) && (pEvent->type() == QEvent::Polish))
   {
      if ((mPaperWidth == 0.0) || (mPaperHeight == 0.0))
      {
         UndoLock lock(dynamic_cast<View*>(this));
         setPaperSize(11.0, 8.5);
      }
   }

   return bReturn;
}

void ProductViewImp::showEvent(QShowEvent* pEvent)
{
   PerspectiveViewImp::showEvent(pEvent);

   if (mbZoomInitialized == false)
   {
      UndoLock lock(dynamic_cast<View*>(this));
      resetZoom();
      mbZoomInitialized = true;
   }
}

void ProductViewImp::toolTipEvent(QHelpEvent* pEvent)
{
   QPoint ptMouse = pEvent->pos();
   ptMouse.setY(height() - pEvent->pos().y());

   GraphicObject* pEditObject = getActiveEditObject();
   if (pEditObject != NULL)
   {
      // Get the pixel coodinate for the current screen position
      LocationType worldCoord;
      translateScreenToWorld(ptMouse.x(), ptMouse.y(), worldCoord.mX, worldCoord.mY);

      // Set the active edit mouse event flag based on whether the mouse is over the active edit view object
      bool bViewEvent = dynamic_cast<GraphicObjectImp*>(pEditObject)->hit(worldCoord);
      ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
      if ( (pEditView != NULL) && (bViewEvent) )
      {
         QPoint ptView = pEditView->mapFromParent(pEvent->pos());
         QHelpEvent helpEvent(QEvent::ToolTip, ptView, pEvent->globalPos());
         pEvent->accept();
         QApplication::sendEvent(pEditView, &helpEvent);
      }
   }
}

void ProductViewImp::keyPressEvent(QKeyEvent* e)
{
   ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
   if (pEditView != NULL)
   {
      QApplication::sendEvent(pEditView, e);
      updateGL();
      return;
   }

   if (e != NULL)
   {
      if (e->key() == Qt::Key_Delete)
      {
         const MouseMode* pMouseMode = getCurrentMouseMode();
         if (pMouseMode != NULL)
         {
            string mouseMode = "";
            pMouseMode->getName(mouseMode);
            if ((mouseMode == "LayerMode") && (mpActiveLayer != NULL))
            {
               dynamic_cast<AnnotationLayerImp*>(mpActiveLayer)->deleteSelectedObjects();
            }
         }
      }
   }

   PerspectiveViewImp::keyPressEvent(e);
}

void ProductViewImp::mousePressEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   // Reset the flag that specifies whether mouse events should be passed to the active edit view
   mbViewEvent = false;

   // Get the current mouse position
   QPoint ptMouse = e->pos();
   ptMouse.setY(height() - e->pos().y());

   // Check if a view object is being edited
   GraphicObjectImp* pEditObject = dynamic_cast<GraphicObjectImp*>(getActiveEditObject());
   if (pEditObject != NULL)
   {
      // Get the pixel coodinate for the current screen position
      LocationType worldCoord;
      translateScreenToWorld(ptMouse.x(), ptMouse.y(), worldCoord.mX, worldCoord.mY);

      // Set the active edit mouse event flag based on whether the mouse is over the active edit view object
      mbViewEvent = pEditObject->hit(worldCoord);
   }

   // Process the mouse event
   QCursor mouseCursor(Qt::ArrowCursor);
   if (mbViewEvent == true)
   {
      ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
      if (pEditView != NULL)
      {
         // Send the event to the edit view
         QPoint ptView = pEditView->mapFromParent(e->pos());
         QMouseEvent mouseEvent(QEvent::MouseButtonPress, ptView, e->globalPos(), e->button(), e->buttons(),
            e->modifiers());
         QApplication::sendEvent(pEditView, &mouseEvent);
         updateGL();

         // Get the edit view's cursor
         mouseCursor = pEditView->cursor();
      }
   }
   else if (pEditObject == NULL)
   {
      // Send the event to the active layer
      bool bEventProcessed = false;

      const MouseModeImp* pMouseMode = dynamic_cast<const MouseModeImp*>(getCurrentMouseMode());
      if (pMouseMode != NULL)
      {
         QString strModeName = pMouseMode->getName();
         if ((strModeName == "LayerMode") && (mpActiveLayer != NULL))
         {
            bEventProcessed = (dynamic_cast<AnnotationLayerImp*>(mpActiveLayer))->processMousePress(ptMouse,
               e->button(), e->buttons(), e->modifiers());
         }
      }

      // If the event is not processed by the active layer, send it to the base class for processing
      if (bEventProcessed == false)
      {
         PerspectiveViewImp::mousePressEvent(e);
      }
      else
      {
         refresh();
      }

      // Get the current view cursor
      mouseCursor = cursor();
      if ((mouseCursor.shape() == Qt::ArrowCursor) && (pMouseMode != NULL))
      {
         mouseCursor = pMouseMode->getCursor();
      }
   }

   // Update the mouse cursor
   setCursor(mouseCursor);
}

void ProductViewImp::mouseMoveEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   // Get the current mouse position
   QPoint ptMouse = e->pos();
   ptMouse.setY(height() - e->pos().y());

   // Update the flag that specifies whether mouse events should be passed to the active edit view
   GraphicObjectImp* pEditObject = dynamic_cast<GraphicObjectImp*>(getActiveEditObject());
   if (pEditObject != NULL)
   {
      // Only need to update the flag if the user id not pressing any buttons
      if (e->buttons() == Qt::NoButton)
      {
         // Get the pixel coodinate for the current screen position
         LocationType worldCoord;
         translateScreenToWorld(ptMouse.x(), ptMouse.y(), worldCoord.mX, worldCoord.mY);

         // Set the active edit mouse event flag based on whether the mouse is over the active edit view object
         mbViewEvent = dynamic_cast<GraphicObjectImp*>(pEditObject)->hit(worldCoord);
      }
   }

   // Process the mouse event
   QCursor mouseCursor(Qt::ArrowCursor);
   if (mbViewEvent == true)
   {
      ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
      if (pEditView != NULL)
      {
         // Send the event to the edit view
         QPoint ptView = pEditView->mapFromParent(e->pos());
         QMouseEvent mouseEvent(QEvent::MouseMove, ptView, e->globalPos(), e->button(), e->buttons(), e->modifiers());
         QApplication::sendEvent(pEditView, &mouseEvent);
         updateGL();

         // Get the edit view's cursor
         mouseCursor = pEditView->cursor();
      }
   }
   else if (pEditObject == NULL)
   {
      // Send the event to the active layer
      bool bEventProcessed = false;

      const MouseModeImp* pMouseMode = dynamic_cast<const MouseModeImp*>(getCurrentMouseMode());
      if (pMouseMode != NULL)
      {
         QString strMode = pMouseMode->getName();
         if ((strMode == "LayerMode") && (mpActiveLayer != NULL))
         {
            bEventProcessed = (dynamic_cast<AnnotationLayerImp*>(mpActiveLayer))->processMouseMove(ptMouse,
               e->button(), e->buttons(), e->modifiers());
         }
      }

      // If the event is not processed by the active layer, send it to the base class for processing
      if (bEventProcessed == false)
      {
         PerspectiveViewImp::mouseMoveEvent(e);
      }
      else
      {
         updateStatusBar(ptMouse);
         updateGL();
      }

      // Get the current view cursor
      mouseCursor = cursor();
      if ((mouseCursor.shape() == Qt::ArrowCursor) && (pMouseMode != NULL))
      {
         mouseCursor = pMouseMode->getCursor();
      }
   }
   else
   {
      // Clear the status bar
      Service<DesktopServices> pDesktop;

      ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(pDesktop->getMainWidget());
      if (pAppWindow != NULL)
      {
         StatusBar* pBar = static_cast<StatusBar*>(pAppWindow->statusBar());
         if (pBar != NULL)
         {
            pBar->clearValues();
         }
      }
   }

   // Update the mouse cursor
   setCursor(mouseCursor);
}

void ProductViewImp::mouseReleaseEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   // Get the current mouse position
   QPoint ptMouse = e->pos();
   ptMouse.setY(height() - e->pos().y());

   // Determine whether the mouse is over the active edit view object
   bool bHit = false;

   GraphicObjectImp* pEditObject = dynamic_cast<GraphicObjectImp*>(getActiveEditObject());
   if (pEditObject != NULL)
   {
      LocationType worldCoord;
      translateScreenToWorld(ptMouse.x(), ptMouse.y(), worldCoord.mX, worldCoord.mY);

      bHit = pEditObject->hit(worldCoord);
   }

   // Process the mouse event
   QCursor mouseCursor(Qt::ArrowCursor);
   if (mbViewEvent == true)
   {
      ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
      if (pEditView != NULL)
      {
         // Send the event to the edit view
         QPoint ptView = pEditView->mapFromParent(e->pos());
         QMouseEvent mouseEvent(QEvent::MouseButtonRelease, ptView, e->globalPos(), e->button(), e->buttons(),
            e->modifiers());
         QApplication::sendEvent(pEditView, &mouseEvent);
         updateGL();

         // Get the edit view's cursor
         if (bHit == true)
         {
            mouseCursor = pEditView->cursor();
         }
      }
   }
   else if (pEditObject == NULL)
   {
      // Send the event to the active layer
      bool bEventProcessed = false;

      const MouseModeImp* pMouseMode = dynamic_cast<const MouseModeImp*>(getCurrentMouseMode());
      if (pMouseMode != NULL)
      {
         QString strMode = pMouseMode->getName();
         if ((strMode == "LayerMode") && (mpActiveLayer != NULL))
         {
            bEventProcessed = (dynamic_cast<AnnotationLayerImp*>(mpActiveLayer))->processMouseRelease(ptMouse,
               e->button(), e->buttons(), e->modifiers());
         }
      }

      // If the event is not processed by the active layer, send it to the base class for processing
      if (bEventProcessed == false)
      {
         PerspectiveViewImp::mouseReleaseEvent(e);
      }
      else
      {
         refresh();
      }

      // Get the current view cursor
      mouseCursor = cursor();
      if ((mouseCursor.shape() == Qt::ArrowCursor) && (pMouseMode != NULL))
      {
         mouseCursor = pMouseMode->getCursor();
      }
   }

   // Clear the status bar
   if (bHit == false)
   {
      Service<DesktopServices> pDesktop;

      ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(pDesktop->getMainWidget());
      if (pAppWindow != NULL)
      {
         StatusBar* pBar = static_cast<StatusBar*>(pAppWindow->statusBar());
         if (pBar != NULL)
         {
            pBar->clearValues();
         }
      }
   }

   // Update the mouse cursor
   setCursor(mouseCursor);

   // Reset the flag that specifies whether mouse events should be passed to the active edit view
   mbViewEvent = false;
}

void ProductViewImp::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   // Reset the flag that specifies whether mouse events should be passed to the active edit view
   mbViewEvent = false;

   // Get the current mouse position
   QPoint ptMouse = pEvent->pos();
   ptMouse.setY(height() - pEvent->pos().y());

   // Check if a view object is being edited
   GraphicObjectImp* pEditObject = dynamic_cast<GraphicObjectImp*>(getActiveEditObject());
   if (pEditObject != NULL)
   {
      // Get the pixel coodinate for the current screen position
      LocationType worldCoord;
      translateScreenToWorld(ptMouse.x(), ptMouse.y(), worldCoord.mX, worldCoord.mY);

      // Set the active edit mouse event flag based on whether the mouse is over the active edit view object
      mbViewEvent = pEditObject->hit(worldCoord);
   }

   // Process the mouse event
   QCursor mouseCursor(Qt::ArrowCursor);
   if (mbViewEvent == true)
   {
      ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
      if (pEditView != NULL)
      {
         // Send the event to the edit view
         QPoint ptView = pEditView->mapFromParent(pEvent->pos());
         QMouseEvent mouseEvent(QEvent::MouseButtonDblClick, ptView, pEvent->globalPos(), pEvent->button(),
            pEvent->buttons(), pEvent->modifiers());
         QApplication::sendEvent(pEditView, &mouseEvent);
         updateGL();

         // Get the edit view's cursor
         mouseCursor = pEditView->cursor();
      }
   }
   else if (pEditObject == NULL)
   {
      // Send the event to the active layer
      bool bEventProcessed = false;

      const MouseModeImp* pMouseMode = dynamic_cast<const MouseModeImp*>(getCurrentMouseMode());
      if (pMouseMode != NULL)
      {
         QString strModeName = pMouseMode->getName();
         if ((strModeName == "LayerMode") && (mpActiveLayer != NULL))
         {
            bEventProcessed = (dynamic_cast<AnnotationLayerImp*>(mpActiveLayer))->processMouseDoubleClick(ptMouse,
               pEvent->button(), pEvent->buttons(), pEvent->modifiers());
         }
      }

      // If the event is not processed by the active layer, send it to the base class for processing
      if (bEventProcessed == false)
      {
         PerspectiveViewImp::mouseDoubleClickEvent(pEvent);
      }
      else
      {
         refresh();
      }

      // Get the current view cursor
      mouseCursor = cursor();
      if ((mouseCursor.shape() == Qt::ArrowCursor) && (pMouseMode != NULL))
      {
         mouseCursor = pMouseMode->getCursor();
      }
   }

   // Update the mouse cursor
   setCursor(mouseCursor);

   // Reset the flag that specifies whether mouse events should be passed to the active edit view
   mbViewEvent = false;
}

void ProductViewImp::wheelEvent(QWheelEvent* e)
{
   if (e != NULL)
   {
      ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
      if (pEditView != NULL)
      {
         QPoint ptMouse = pEditView->mapFromParent(e->pos());
         QWheelEvent wheelEvent(ptMouse, e->globalPos(), e->delta(), e->buttons(), e->modifiers(), e->orientation());
         QApplication::sendEvent(pEditView, &wheelEvent);
         updateGL();
         return;
      }
   }

   PerspectiveViewImp::wheelEvent(e);
}

void ProductViewImp::leaveEvent(QEvent* e)
{
   ViewImp* pEditView = dynamic_cast<ViewImp*>(getActiveEditView());
   if (pEditView != NULL)
   {
      // Send the event to the view but do not return to ensure the product view also handles the event
      QApplication::sendEvent(pEditView, e);
   }

   PerspectiveViewImp::leaveEvent(e);
}

void ProductViewImp::drawContents()
{
   drawPaper();
   drawLayers();
}

void ProductViewImp::drawPaper()
{
   setupWorldMatrices();

   double vertices[4][2];
   vertices[0][0] = 0.0;
   vertices[0][1] = 0.0;
   vertices[1][0] = mPaperWidth * mDpi;
   vertices[1][1] = 0.0;
   vertices[2][0] = mPaperWidth * mDpi;
   vertices[2][1] = mPaperHeight * mDpi;
   vertices[3][0] = 0.0;
   vertices[3][1] = mPaperHeight * mDpi;

   DataOrigin dataOrigin = getDataOrigin();
   if (dataOrigin == UPPER_LEFT)
   {
      glTranslated(0.1 * mDpi, 0.1 * mDpi, 0.0);
   }
   else if (dataOrigin == LOWER_LEFT)
   {
      glTranslated(0.1 * mDpi, -0.1 * mDpi, 0.0);
   }

   glColor3ub(0, 0, 0);
   drawRectangle(GL_QUADS, vertices);

   if (dataOrigin == UPPER_LEFT)
   {
      glTranslated(-0.1 * mDpi, -0.1 * mDpi, 0.0);
   }
   else if (dataOrigin == LOWER_LEFT)
   {
      glTranslated(-0.1 * mDpi, 0.1 * mDpi, 0.0);
   }

   glColor3ub(mPaperColor.red(), mPaperColor.green(), mPaperColor.blue());
   drawRectangle(GL_QUADS, vertices);

   glLineWidth(1.0);
   glColor3ub(0, 0, 0);
   drawRectangle(GL_LINE_LOOP, vertices);
}

void ProductViewImp::drawLayers()
{
   VERIFYNRV(mpLayoutLayer != NULL);
   VERIFYNRV(mpClassificationLayer != NULL);

   setupWorldMatrices();

   mpLayoutLayer->draw();
   mpClassificationLayer->draw();
}

void ProductViewImp::drawRectangle(GLenum type, double vertices[4][2])
{
   glBegin(type);
   for (unsigned int i = 0; i < 4; i++)
   {
      glVertex2dv(vertices[i]);
   }

   glEnd();
}

void ProductViewImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   QObject* pParent = pMenu->getActionParent();

   // Check if the user clicked in the session explorer
   if (dynamic_cast<SessionExplorer*>(&subject) != NULL)
   {
      // Add the layer actions
      vector<SessionItem*> items = pMenu->getSessionItems();
      if (items.size() == 1)
      {
         LayerImp* pLayerImp = dynamic_cast<LayerImp*>(items.front());
         if ((pLayerImp != NULL) && ((pLayerImp == mpLayoutLayer) || (pLayerImp == mpClassificationLayer)))
         {
            Layer* pLayer = dynamic_cast<Layer*>(items.front());
            if ((pLayer != getActiveLayer()) && (pLayerImp->acceptsMouseEvents() == true))
            {
               // Activate
               QAction* pActivateAction = new QAction("Activate", pParent);
               pActivateAction->setAutoRepeat(false);
               pActivateAction->setData(QVariant::fromValue(pLayer));
               connect(pActivateAction, SIGNAL(triggered()), this, SLOT(setActiveLayer()));
               pMenu->addActionBefore(pActivateAction, APP_PRODUCTVIEW_LAYER_ACTIVATE_ACTION,
                  APP_LAYER_DISPLAYED_ACTION);
            }

            // Remove default layer actions
            pMenu->removeAction(APP_LAYER_DISPLAYED_ACTION);
         }
      }
   }
}

void ProductViewImp::updateClassificationLocation()
{
   TextObjectImp* pTopText = dynamic_cast<TextObjectImp*>(mpClassificationLayer->getTopText());
   if (pTopText != NULL)
   {
      // Disconnect the object signal to prevent recursive calls
      disconnect(pTopText, SIGNAL(propertyModified(GraphicProperty*)), this, SLOT(updateClassificationLocation()));

      // Ensure the text bounding box is the correct size
      pTopText->updateTexture();

      // Get the current text center location
      LocationType llCorner = pTopText->getLlCorner();
      LocationType urCorner = pTopText->getUrCorner();
      LocationType center((llCorner.mX + urCorner.mX) / 2.0, (llCorner.mY + urCorner.mY) / 2.0);

      double dTextWidth = urCorner.mX - llCorner.mX;
      double dTextHeight = urCorner.mY - llCorner.mY;

      // Calculate the distance from the new text center to the old text center
      LocationType newCenter(mPaperWidth * mDpi / 2.0, (mPaperHeight * mDpi) - (mDpi * 0.1) - (dTextHeight / 2.0));
      LocationType delta = newCenter - center;

      // Move the text object to the center of the view object
      pTopText->move(delta);

      // Reconnect the object signal
      connect(pTopText, SIGNAL(propertyModified(GraphicProperty*)), this, SLOT(updateClassificationLocation()));
   }

   TextObjectImp* pBottomText = dynamic_cast<TextObjectImp*>(mpClassificationLayer->getBottomText());
   if (pBottomText != NULL)
   {
      // Disconnect the object signal to prevent recursive calls
      disconnect(pBottomText, SIGNAL(propertyModified(GraphicProperty*)), this, SLOT(updateClassificationLocation()));

      // Ensure the text bounding box is the correct size
      pBottomText->updateTexture();

      // Get the current text center location
      LocationType llCorner = pBottomText->getLlCorner();
      LocationType urCorner = pBottomText->getUrCorner();
      LocationType center((llCorner.mX + urCorner.mX) / 2.0, (llCorner.mY + urCorner.mY) / 2.0);

      double dTextWidth = urCorner.mX - llCorner.mX;
      double dTextHeight = urCorner.mY - llCorner.mY;

      // Calculate the distance from the new text center to the old text center
      LocationType newCenter(mPaperWidth * mDpi / 2.0, (mDpi * 0.1) + (dTextHeight / 2.0));
      LocationType delta = newCenter - center;

      // Move the text object to the center of the view object
      pBottomText->move(delta);

      // Reconnect the object signal
      connect(pBottomText, SIGNAL(propertyModified(GraphicProperty*)), this, SLOT(updateClassificationLocation()));
   }
}

void ProductViewImp::updateMouseCursor(const MouseMode* pMouseMode)
{
   const MouseModeImp* pMouseModeImp = static_cast<const MouseModeImp*>(pMouseMode);
   if (pMouseModeImp != NULL)
   {
      QString strModeName = pMouseModeImp->getName();
      if (strModeName == "LayerMode")
      {
         QCursor mouseCursor = pMouseModeImp->getCursor();

         LayerImp* pActiveLayerImp = dynamic_cast<LayerImp*>(getActiveLayer());
         if (pActiveLayerImp != NULL)
         {
            mouseCursor = pActiveLayerImp->getMouseCursor();
         }

         setCursor(mouseCursor);
      }
   }
}

void ProductViewImp::setActiveLayer()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction != NULL)
   {
      GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(pAction->data().value<Layer*>());
      if (pLayer != NULL)
      {
         setMouseMode("LayerMode");
         setActiveLayer(pLayer);
      }
   }
}

bool ProductViewImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL || PerspectiveViewImp::toXml(pXml) == false)
   {
      return false;
   }

   pXml->addAttr("paperWidth", mPaperWidth);
   pXml->addAttr("paperHeight", mPaperHeight);
   pXml->addAttr("paperColor", mPaperColor.name().toStdString());
   pXml->addAttr("dpi", mDpi);
   if (mpLayoutLayer != NULL)
   {
      DOMElement* pElement = pXml->addElement("LayoutLayer");
      pXml->pushAddPoint(pElement);
      pXml->addAttr("id", mpLayoutLayer->getId());
      mpLayoutLayer->toXml(pXml);
      pXml->popAddPoint();

      // LayerImp::toXml does not save the data elements during a session save
      // This kludge saves the DataElement manually because it is not stored in ModelServices
      DataElement* pAnno = mpLayoutLayer->getDataElement();
      if (pAnno != NULL)
      {
         pElement = pXml->addElement("LayoutElement");
         pXml->pushAddPoint(pElement);
         pXml->addAttr("id", pAnno->getId());
         pAnno->toXml(pXml);
         pXml->popAddPoint();
      }
   }
   if (mpClassificationLayer != NULL)
   {
      DOMElement* pElement = pXml->addElement("ClassificationLayer");
      pXml->pushAddPoint(pElement);
      pXml->addAttr("id", mpClassificationLayer->getId());
      mpClassificationLayer->toXml(pXml);
      pXml->popAddPoint();

      // LayerImp::toXml does not save the data elements during a session save
      // This kludge saves the DataElement manually because it is not stored in ModelServices
      DataElement* pAnno = mpClassificationLayer->getDataElement();
      if (pAnno != NULL)
      {
         pElement = pXml->addElement("ClassificationElement");
         pXml->pushAddPoint(pElement);
         pXml->addAttr("id", pAnno->getId());
         pAnno->toXml(pXml);
         pXml->popAddPoint();
      }
   }

   return true;
}

bool ProductViewImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!PerspectiveViewImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   mPaperWidth = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("paperWidth"))));
   mPaperHeight = StringUtilities::fromXmlString<double>(A(pElem->getAttribute(X("paperHeight"))));
   mPaperColor = QColor(A(pElem->getAttribute(X("paperColor"))));
   mDpi = StringUtilities::fromXmlString<unsigned int>(A(pElem->getAttribute(X("dpi"))));

   AnnotationElement* pAnnoElem = dynamic_cast<AnnotationElement*>(mpClassificationLayer->getDataElement());
   mpClassificationLayer->setView(this);
   // restore classification text objects
   DOMNode* pNode = findChildNode(pElem, "ClassificationElement/group/objects");
   bool bTopText(true);
   for (DOMNode* pChld = pNode->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      string nodeName = A(pChld->getNodeName());
      if (nodeName=="Graphic" && bTopText)
      {
         mpClassificationLayer->getTopText()->fromXml(pChld, version);
         bTopText = false;
      }
      if (nodeName=="Graphic" && !bTopText)
      {
         mpClassificationLayer->getBottomText()->fromXml(pChld, version);
      }
   }

   DataDescriptorAdapter layoutDescriptor("Layout", "AnnotationElement", NULL);
   delete mpLayoutLayer;
   AnnotationElementAdapter* pAdapter = new AnnotationElementAdapter(layoutDescriptor, 
      findAttribute(pElem, "LayoutElement/id"));
   mpLayoutLayer = new AnnotationLayerAdapter(findAttribute(pElem, "LayoutLayer/id"), "Layout",
      pAdapter);
   mpLayoutLayer->getGroup(); // force the layer to be set into the element
   mpLayoutLayer->setView(this);
   pAdapter->fromXml(dynamic_cast<DOMElement*>(findChildNode(pElem, "LayoutElement")), version);

   bool bLocked = StringUtilities::fromXmlString<bool>(
      findAttribute(pElem, "LayoutLayer/layerLocked"));
   mpLayoutLayer->setLayerLocked(bLocked);
   bool bSnap = StringUtilities::fromXmlString<bool>(
      findAttribute(pElem, "LayoutLayer/snapToGrid"));
   mpLayoutLayer->setSnapToGrid(bSnap);

   mpActiveLayer = NULL;

   connectLayers();
   return true;
}

void ProductViewImp::updateExtents()
{
   double dPaperMinX = 0.0;
   double dPaperMinY = 0.0;
   double dPaperMaxX = mPaperWidth * mDpi;
   double dPaperMaxY = mPaperHeight * mDpi;

   double dLayerMinX = 0.0;
   double dLayerMinY = 0.0;
   double dLayerMaxX = 0.0;
   double dLayerMaxY = 0.0;

   mpLayoutLayer->getExtents(dLayerMinX, dLayerMinY, dLayerMaxX, dLayerMaxY);

   double dMinX = min(dLayerMinX, dPaperMinX);
   double dMinY = min(dLayerMinY, dPaperMinY);
   double dMaxX = max(dLayerMaxX, dPaperMaxX);
   double dMaxY = max(dLayerMaxY, dPaperMaxY);

   double dMarginX = (dMaxX - dMinX) * 0.03;
   double dMarginY = (dMaxY - dMinY) * 0.03;

   setExtents(dMinX - dMarginX, dMinY - dMarginY, dMaxX + dMarginX, dMaxY + dMarginY);
}
