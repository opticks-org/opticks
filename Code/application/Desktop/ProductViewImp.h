/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRODUCTVIEWIMP_H
#define PRODUCTVIEWIMP_H

#include <QtGui/QImage>

#include "AttachmentPtr.h"
#include "ColorType.h"
#include "PerspectiveViewImp.h"
#include "SessionExplorer.h"
#include "TypesFile.h"

#include <string>

class AnnotationLayer;
class AnnotationLayerImp;
class ClassificationLayer;
class ClassificationLayerImp;
class GraphicLayer;
class GraphicObject;
class Layer;
class QHelpEvent;

class ProductViewImp : public PerspectiveViewImp
{
   Q_OBJECT

public:
   ProductViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~ProductViewImp();

   using PerspectiveViewImp::setIcon;
   using PerspectiveViewImp::setName;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   View* copy(QGLContext* drawContext = 0, QWidget* parent = 0) const;
   bool copy(View *pView) const;

   ViewType getViewType() const;

   void getPaperSize(double& dWidth, double& dHeight) const;
   QColor getPaperColor() const;
   QImage getPaperImage();
   unsigned int getDpi() const;

   bool loadTemplate(const QString& strTemplateFile);
   bool saveTemplate(const QString& strTemplateFile) const;

   AnnotationLayer* getLayoutLayer() const;
   ClassificationLayer* getClassificationLayer() const;
   GraphicLayer* getActiveLayer() const;

   GraphicObject* getActiveEditObject() const;
   View* getActiveEditView() const;

   bool enableInset(bool bEnable);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setClassificationFont(const QFont& classificationFont);
   void setClassificationColor(const QColor& clrClassification);
   void rotateTo(double dDegrees);
   void flipTo(double dDegrees);
   void setPaperSize(double dWidth, double dHeight);
   void setPaperColor(const QColor& clrPaper);
   void setDpi(unsigned int dpi);
   void setActiveLayer(GraphicLayer* pLayer);
   bool setActiveEditObject(GraphicObject* pObject);
   bool setActiveEditView(View* pView);
   void updateExtents();
   virtual void enableClassification(bool bEnable);

signals:
   void paperSizeChanged(double dWidth, double dHeight);
   void paperColorChanged(const QColor& clrPaper);
   void dpiChanged(unsigned int dpi);
   void layerActivated(Layer* pLayer);
   void editObjectChanged(GraphicObject* pObject);
   void editViewChanged(View* pView);

protected:
   ProductViewImp& operator=(const ProductViewImp& productView);
   bool event(QEvent* pEvent);
   void showEvent(QShowEvent* pEvent);
   void toolTipEvent(QHelpEvent* pEvent);
   void keyPressEvent(QKeyEvent* e);
   void mousePressEvent(QMouseEvent* e);
   void mouseMoveEvent(QMouseEvent* e);
   void mouseReleaseEvent(QMouseEvent* e);
   void mouseDoubleClickEvent(QMouseEvent* pEvent);
   void wheelEvent(QWheelEvent* e);
   void leaveEvent(QEvent* e);

   void drawContents();
   void drawPaper();
   void drawLayers();
   void drawRectangle(GLenum type, double vertices[4][2]);

   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void updateClassificationLocation();
   void updateMouseCursor(const MouseMode* pMouseMode);
   void setActiveLayer();
   void updateClassificationMarks(const QString& classificationText);
   virtual void toggleMousePanByKey();

private:
   ProductViewImp(const ProductViewImp& rhs);

   void connectLayers();

   AttachmentPtr<SessionExplorer> mpExplorer;

   double mPaperWidth;
   double mPaperHeight;
   QColor mPaperColor;
   unsigned int mDpi;

   AnnotationLayerImp* mpLayoutLayer;
   ClassificationLayerImp* mpClassificationLayer;
   GraphicLayer* mpActiveLayer;

   GraphicObject* mpEditObject;
   bool mbViewEvent;
   bool mbZoomInitialized;
   bool mProductClassificationEnabled;
};

#define PRODUCTVIEWADAPTEREXTENSION_CLASSES \
   PERSPECTIVEVIEWADAPTEREXTENSION_CLASSES

#define PRODUCTVIEWADAPTER_METHODS(impClass) \
   PERSPECTIVEVIEWADAPTER_METHODS(impClass) \
   void setPaperSize(double dWidth, double dHeight) \
   { \
      return impClass::setPaperSize(dWidth, dHeight); \
   } \
   void getPaperSize(double& dWidth, double& dHeight) const \
   { \
      return impClass::getPaperSize(dWidth, dHeight); \
   } \
   void setPaperColor(const ColorType& paperColor) \
   { \
      QColor clrPaper; \
      if (paperColor.isValid() == true) \
      { \
         clrPaper.setRgb(paperColor.mRed, paperColor.mGreen, paperColor.mBlue); \
      } \
      \
      impClass::setPaperColor(clrPaper); \
   } \
   ColorType getPaperColor() const \
   { \
      ColorType paperColor; \
      \
      QColor clrPaper = impClass::getPaperColor(); \
      if (clrPaper.isValid() == true) \
      { \
         paperColor.mRed = clrPaper.red(); \
         paperColor.mGreen = clrPaper.green(); \
         paperColor.mBlue = clrPaper.blue(); \
      } \
      \
      return paperColor; \
   } \
   void setDpi(unsigned int dpi) \
   { \
      return impClass::setDpi(dpi); \
   } \
   unsigned int getDpi() const \
   { \
      return impClass::getDpi(); \
   } \
   bool loadTemplate(const std::string& filename) \
   { \
      QString strFilename; \
      if (filename.empty() == false) \
      { \
         strFilename = QString::fromStdString(filename); \
      } \
      \
      return impClass::loadTemplate(strFilename); \
   } \
   bool saveTemplate(const std::string& filename) const \
   { \
      QString strFilename; \
      if (filename.empty() == false) \
      { \
         strFilename = QString::fromStdString(filename); \
      } \
      \
      return impClass::saveTemplate(strFilename); \
   } \
   AnnotationLayer* getLayoutLayer() const \
   { \
      return impClass::getLayoutLayer(); \
   } \
   ClassificationLayer* getClassificationLayer() const \
   { \
      return impClass::getClassificationLayer(); \
   } \
   void setActiveLayer(GraphicLayer* pLayer) \
   { \
      return impClass::setActiveLayer(pLayer); \
   } \
   GraphicLayer* getActiveLayer() const \
   { \
      return impClass::getActiveLayer(); \
   } \
   bool setActiveEditObject(GraphicObject* pObject) \
   { \
      return impClass::setActiveEditObject(pObject); \
   } \
   GraphicObject* getActiveEditObject() const \
   { \
      return impClass::getActiveEditObject(); \
   } \
   bool setActiveEditView(View* pView) \
   { \
      return impClass::setActiveEditView(pView); \
   } \
   View* getActiveEditView() const \
   { \
      return impClass::getActiveEditView(); \
   }

#endif
