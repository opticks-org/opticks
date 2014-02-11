/*
* The information in this file is
* Copyright(c) 2011 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef POINTCLOUDVIEWIMP_H
#define POINTCLOUDVIEWIMP_H

#include "AttachmentPtr.h"
#include "ColorMap.h"
#include "PerspectiveViewImp.h"
#include "PointCloudElement.h"
#include "PointCloudView.h"

class QAction;
class QGLBuffer;
class QGLShader;
class QGLShaderProgram;
class QMenu;

class PointCloudViewImp : public PerspectiveViewImp, public Observer
{
   Q_OBJECT

public:
   PointCloudViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext = 0,
      QWidget* parent = 0);
   ~PointCloudViewImp();
   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void elementDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void elementModified(Subject &subject, const std::string &signal, const boost::any &v);

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   PointCloudViewImp& operator= (const PointCloudViewImp& pointCloudView);
   View* copy(QGLContext* drawContext = 0, QWidget* parent = 0) const;
   bool copy(View *pView) const;

   ViewType getViewType() const;
   using ViewImp::setName;

   virtual bool setPrimaryPointCloud(PointCloudElement* pPointCloud);
   virtual PointCloudElement* getPrimaryPointCloud();
   virtual void setPointColorizationType(PointColorizationType type);
   virtual PointColorizationType getPointColorizationType() const;
   virtual void setDecimation(uint32_t decimation);
   virtual uint32_t getDecimation() const;
   virtual void setPointSize(float pointsize);
   virtual float getPointSize() const;
   virtual void setLowerStretch(double value); 
   virtual double getLowerStretch() const;
   virtual void setUpperStretch(double value);
   virtual double getUpperStretch() const;
   virtual void setColorMap(const ColorMap& colorMap);
   virtual const ColorMap& getColorMap() const;
   virtual void setLowerStretchColor(ColorType lower);
   virtual void setUpperStretchColor(ColorType upper);
   virtual void setColorStretch(ColorType lower, ColorType upper);
   virtual ColorType getLowerStretchColor() const;
   virtual ColorType getUpperStretchColor()  const;
   virtual void setUsingColorMap(bool usingMap);
   virtual bool isUsingColorMap() const;
   virtual double limitZoomPercentage(double dPercent);
   virtual void setZExaggeration(double value);
   virtual double getZExaggeration();
   
   void setStretchType(const StretchType& stretch);
   virtual std::list<ContextMenuAction> getContextMenuActions() const;

protected slots:
   void setStretchType(QAction* pAction);
   void setPointColorizationType(QAction* pAction);
   virtual void zoomExtents();

protected:
   virtual void drawContents();
   virtual void mousePressEvent(QMouseEvent* pEvent);
   virtual void mouseMoveEvent(QMouseEvent* pEvent);
   virtual void mouseReleaseEvent(QMouseEvent* pEvent);
   virtual void updateMatrices(int width, int height);
   using ViewImp::updateMatrices;
   virtual void updateStatusBar(const QPoint& screenCoord);

private:
   void initializeDrawing();
   void updateVertexBufferIfNeeded();
   void updateColorizationBufferIfNeeded();
   void updateColorMapTextureIfNeeded();
   void cleanupShaders();
   bool initShaders();

   AttachmentPtr<PointCloudElement> mpPrimaryPointCloud;

   // Ideally, these members would belong to the layer containing each element.
   // Until support for that can be implemented, this data will be stored
   // for only whatever the primary element is.

   QMenu* mpStretchTypeMenu;
   QAction* mpLinearStretchAction;
   QAction* mpLogStretchAction;
   QAction* mpExpStretchAction;
   StretchType mStretchType;

   QMenu* mpColorizationMenu;
   QAction* mpColorizeHeightAction;
   QAction* mpColorizeIntensityAction;
   QAction* mpColorizeClassificationAction;
   PointColorizationType mCurrentColorization;
   
   QGLBuffer* mpVertexBuffer;
   bool mVertexBufferUpToDate;
   QGLBuffer* mpColorizationBuffer;
   bool mColorizationBufferUpToDate;

   QGLShaderProgram* mpShaderProg;
   GLuint mColorMapTexture;
   std::map<std::string, QGLShader*> mShaders;

   unsigned int mTotalPoints;
   GLfloat mLowerStretch;
   GLfloat mUpperStretch;
   ColorMap mColorMap;
   ColorType mLowerColor;
   ColorType mUpperColor;
   bool mUsingColorMap;
   bool mShaderProgsUpToDate;
   bool mColorMapTextureUpToDate;

   GLfloat mMinZ;
   GLfloat mMaxZ;

   uint32_t mDecimation;
   double mScaleFactor;
   double mZExaggerationFactor;
   GLfloat mPointSize;

   int mMouseY;
   bool mMouseActive;
};

#define POINTCLOUDVIEWADAPTEREXTENSION_CLASSES \
   PERSPECTIVEVIEWADAPTEREXTENSION_CLASSES

#define POINTCLOUDVIEWADAPTER_METHODS(impClass) \
   PERSPECTIVEVIEWADAPTER_METHODS(impClass) \
   bool setPrimaryPointCloud(PointCloudElement* pPointCloud) \
{ \
   return impClass::setPrimaryPointCloud(pPointCloud); \
} \
   PointCloudElement* getPrimaryPointCloud() \
{ \
   return impClass::getPrimaryPointCloud(); \
} \
void setPointColorizationType(PointColorizationType type) \
{ \
   impClass::setPointColorizationType(type); \
} \
PointColorizationType getPointColorizationType() const \
{ \
   return impClass::getPointColorizationType(); \
} \
void setDecimation(uint32_t decimation) \
{ \
   impClass::setDecimation(decimation); \
} \
uint32_t getDecimation() const \
{ \
   return impClass::getDecimation(); \
} \
void setPointSize(float pointsize) \
{ \
   impClass::setPointSize(pointsize); \
} \
float getPointSize() const \
{ \
   return impClass::getPointSize(); \
} \
void setLowerStretch(double value) \
{ \
   impClass::setLowerStretch(value); \
} \
double getLowerStretch() const \
{ \
   return impClass::getLowerStretch(); \
} \
void setUpperStretch(double value) \
{ \
   impClass::setUpperStretch(value); \
} \
double getUpperStretch() const \
{ \
   return impClass::getUpperStretch(); \
} \
void setColorMap(const ColorMap& colorMap) \
{ \
   impClass::setColorMap(colorMap); \
} \
const ColorMap& getColorMap() const \
{ \
   return impClass::getColorMap(); \
} \
void setLowerStretchColor(ColorType lower) \
{ \
   impClass::setLowerStretchColor(lower); \
} \
void setUpperStretchColor(ColorType upper) \
{ \
   impClass::setUpperStretchColor(upper); \
} \
void setColorStretch(ColorType lower, ColorType upper) \
{ \
   impClass::setColorStretch(lower, upper); \
} \
ColorType getLowerStretchColor() const \
{ \
   return impClass::getLowerStretchColor(); \
} \
ColorType getUpperStretchColor() const \
{ \
   return impClass::getUpperStretchColor(); \
} \
void setUsingColorMap(bool usingMap) \
{ \
   impClass::setUsingColorMap(usingMap); \
} \
bool isUsingColorMap() const \
{ \
   return impClass::isUsingColorMap(); \
} \
virtual void setZExaggeration(double value) \
{ \
   impClass::setZExaggeration(value); \
} \
double getZExaggeration() \
{ \
   return impClass::getZExaggeration(); \
} \

#endif
