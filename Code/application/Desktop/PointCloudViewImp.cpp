/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationWindow.h"
#include "DesktopServices.h"
#include "ColorMap.h"
#include "ContextMenu.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "glCommon.h"
#include "MessageLogResource.h"
#include "MouseModeImp.h"
#include "MultiThreadedAlgorithm.h"
#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudElement.h"
#include "PointCloudViewAdapter.h"
#include "PointCloudViewImp.h"
#include "PropertiesPointCloudView.h"

#include "Undo.h"

#include <QtCore/QFile>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtOpenGL/QGLBuffer>
#include <QtOpenGL/QGLShader>
#include <QtOpenGL/QGLShaderProgram>
#include <limits>
#include <GL/glew.h>

using namespace std;

#define MVERTEX_ATTRIB_NUM 0
#define MCOLOR_ATTRIB_NUM 1

namespace
{
   const string shortcutContext = "View/PointCloud";
}

PointCloudViewImp::PointCloudViewImp(const std::string& id, const std::string& viewName, QGLContext* drawContext,
      QWidget* parent) : PerspectiveViewImp(id, viewName, drawContext, parent),
      mpPrimaryPointCloud(),
      mpVertexBuffer(NULL),
      mVertexBufferUpToDate(false),
      mpColorizationBuffer(NULL),
      mColorizationBufferUpToDate(false),
      mpShaderProg(NULL),
      mColorMapTexture(0),
      mTotalPoints(0),
      mLowerStretch(0),
      mUpperStretch(0),
      mLowerColor(255, 0, 0),
      mUpperColor(225, 255, 255),
      mUsingColorMap(false),
      mShaderProgsUpToDate(false),
      mColorMapTextureUpToDate(false),
      mDecimation(0),
      mScaleFactor(0.01),
      mZExaggerationFactor(1.0),
      mPointSize(1.0),
      mMouseActive(false)
{
   // check the OpenGL version
   {
      GlContextSave contextSave(this);
      int maj=1,min=0;
      const char* pVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
      maj = pVer[0] - '0';
      if (maj >= 3)
      {
         // only works on OpenGL 3.x and newer
         glGetIntegerv(GL_MAJOR_VERSION, &maj);
         glGetIntegerv(GL_MINOR_VERSION, &min);
      }
      glewInit();
      if (maj < 3 || (maj == 3 && min < 2) || !GLEW_ARB_explicit_attrib_location) // requires 3.2 or newer
      {
         Service<DesktopServices>()->showSuppressibleMsgDlg("Invalid OpenGL version",
            QString("Point clouds require OpenGL v3.2 or newer and GL_ARB_explicit_attrib_location but v%1.%2 was detected."
                    "\nPoint clouds may not display.")
            .arg(maj).arg(min).toStdString(), MESSAGE_WARNING, "PointCloudOpenGLCheck");
      }
   }

   addPropertiesPage(PropertiesPointCloudView::getName());
   mpPrimaryPointCloud.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &PointCloudViewImp::elementDeleted));
   mpPrimaryPointCloud.addSignal(SIGNAL_NAME(PointCloudElement, DataModified), Slot(this, &PointCloudViewImp::elementModified));

   Service<DesktopServices> pDesktop;
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(pDesktop->getMainWidget());
   if (pAppWindow != NULL)
   {
      VERIFYNR(connect(this, SIGNAL(mouseModeAdded(const MouseMode*)),
         pAppWindow, SLOT(addMouseModeToGroup(const MouseMode*))));
   }

   setIcon(QIcon(":/icons/PointCloudView"));
   setWindowIcon(QIcon(":/icons/PointCloudView"));
   addMouseMode(new MouseModeImp("LayerMode", QCursor(Qt::ArrowCursor)));
   addMouseMode(new MouseModeImp("MeasurementMode", QCursor(QPixmap(":/icons/MeasurementCursor"), 1, 14)));
   addMouseMode(new MouseModeImp("PanMode", QCursor(Qt::OpenHandCursor)));
   addMouseMode(new MouseModeImp("RotateMode", QCursor(QPixmap(":/icons/FreeRotateCursor"), 7, 9)));
   addMouseMode(new MouseModeImp("ZoomInMode", QCursor(QPixmap(":/icons/ZoomInCursor"), 0, 0)));
   addMouseMode(new MouseModeImp("ZoomOutMode", QCursor(QPixmap(":/icons/ZoomOutCursor"), 0, 0)));
   addMouseMode(new MouseModeImp("ZoomBoxMode", QCursor(QPixmap(":/icons/ZoomRectCursor"), 0, 0)));

   // Stretch type menu
   string stretchTypeContext = shortcutContext + string("/Stretch Type");
   mpStretchTypeMenu = new QMenu("Stretch Type");

   QActionGroup* pStetchTypeGroup = new QActionGroup(mpStretchTypeMenu);
   pStetchTypeGroup->setExclusive(true);

   mpLinearStretchAction = pStetchTypeGroup->addAction("Linear");
   mpLinearStretchAction->setAutoRepeat(false);
   mpLinearStretchAction->setCheckable(true);
   mpLinearStretchAction->setShortcutContext(Qt::WidgetShortcut);
   Service<DesktopServices>()->initializeAction(mpLinearStretchAction, stretchTypeContext);

   mpLogStretchAction = pStetchTypeGroup->addAction("Logarithmic");
   mpLogStretchAction->setAutoRepeat(false);
   mpLogStretchAction->setCheckable(true);
   mpLogStretchAction->setShortcutContext(Qt::WidgetShortcut);
   Service<DesktopServices>()->initializeAction(mpLogStretchAction, stretchTypeContext);

   mpExpStretchAction = pStetchTypeGroup->addAction("Exponential");
   mpExpStretchAction->setAutoRepeat(false);
   mpExpStretchAction->setCheckable(true);
   mpExpStretchAction->setShortcutContext(Qt::WidgetShortcut);
   Service<DesktopServices>()->initializeAction(mpExpStretchAction, stretchTypeContext);

   mpStretchTypeMenu->addActions(pStetchTypeGroup->actions());

   VERIFYNR(connect(pStetchTypeGroup, SIGNAL(triggered(QAction*)),
      this, SLOT(setStretchType(QAction*))));

   // Color By menu
   string colorizationContext = shortcutContext + string("/Color By");
   mpColorizationMenu = new QMenu("Color By");

   QActionGroup* pColorizationGroup = new QActionGroup(mpColorizationMenu);
   pColorizationGroup->setExclusive(true);

   mpColorizeHeightAction = pColorizationGroup->addAction("Height");
   mpColorizeHeightAction->setAutoRepeat(false);
   mpColorizeHeightAction->setCheckable(true);
   mpColorizeHeightAction->setShortcutContext(Qt::WidgetShortcut);
   Service<DesktopServices>()->initializeAction(mpColorizeHeightAction, colorizationContext);

   mpColorizeIntensityAction = pColorizationGroup->addAction("Intensity");
   mpColorizeIntensityAction->setAutoRepeat(false);
   mpColorizeIntensityAction->setCheckable(true);
   mpColorizeIntensityAction->setShortcutContext(Qt::WidgetShortcut);
   Service<DesktopServices>()->initializeAction(mpColorizeIntensityAction, colorizationContext);

   mpColorizeClassificationAction = pColorizationGroup->addAction("Classification");
   mpColorizeClassificationAction->setAutoRepeat(false);
   mpColorizeClassificationAction->setCheckable(true);
   mpColorizeClassificationAction->setShortcutContext(Qt::WidgetShortcut);
   Service<DesktopServices>()->initializeAction(mpColorizeClassificationAction, colorizationContext);

   mpColorizationMenu->addActions(pColorizationGroup->actions());

   VERIFYNR(connect(pStetchTypeGroup, SIGNAL(triggered(QAction*)),
      this, SLOT(setStretchType(QAction*))));
   VERIFYNR(connect(pColorizationGroup, SIGNAL(triggered(QAction*)),
      this, SLOT(setPointColorizationType(QAction*))));

   setStretchType(LINEAR);
   setPointColorizationType(POINT_HEIGHT);
}

PointCloudViewImp::~PointCloudViewImp()
{
   delete mpShaderProg;
   cleanupShaders();
   delete mpVertexBuffer;
   delete mpColorizationBuffer;
   if (mColorMapTexture != 0)
   {
      glDeleteTextures(1, &mColorMapTexture);
   }
   if (mpPrimaryPointCloud.get() != NULL)
   {
      PointCloudElement* pElement = mpPrimaryPointCloud.get();
      mpPrimaryPointCloud.reset(NULL);
      Service<ModelServices>()->destroyElement(pElement);
   }

   // Destroy the mouse modes
   deleteMouseMode(getMouseMode("LayerMode"));
   deleteMouseMode(getMouseMode("MeasurementMode"));
   deleteMouseMode(getMouseMode("PanMode"));
   deleteMouseMode(getMouseMode("RotateMode"));
   deleteMouseMode(getMouseMode("ZoomInMode"));
   deleteMouseMode(getMouseMode("ZoomOutMode"));
   deleteMouseMode(getMouseMode("ZoomBoxMode"));
}

void PointCloudViewImp::elementDeleted(Subject &subject, const std::string &signal, const boost::any &v)
{
   Service<DesktopServices> pDesktop;
   pDesktop->deleteView(dynamic_cast<View*>(this));
}

void PointCloudViewImp::elementModified(Subject &subject, const std::string &signal, const boost::any &v)
{
   const uint32_t fields = boost::any_cast<uint32_t>(v);
   if (dynamic_cast<PointCloudElement*>(&subject) == mpPrimaryPointCloud.get())
   {
      if (fields & PointCloudElement::UPDATE_LOCATION) // one or more location fields
      {
         mVertexBufferUpToDate = false;
      }
      switch (mCurrentColorization)
      {
      case POINT_HEIGHT:
         if (fields & PointCloudElement::UPDATE_Z)
         {
            mColorizationBufferUpToDate = false;
         }
         break;
      case POINT_INTENSITY:
         if (fields & PointCloudElement::UPDATE_INTENSITY)
         {
            mColorizationBufferUpToDate = false;
         }
         break;
      case POINT_CLASSIFICATION:
         if (fields & PointCloudElement::UPDATE_CLASSIFICATION)
         {
            mColorizationBufferUpToDate = false;
         }
         break;
      case POINT_AUX:
         if (fields & PointCloudElement::UPDATE_AUX)
         {
            mColorizationBufferUpToDate = false;
         }
         break;
      case POINT_USER: // Always update with user colorization
         mColorizationBufferUpToDate = false;
         break;
      }
      if (!mVertexBufferUpToDate || !mColorizationBufferUpToDate)
      {
         refresh();
      }
   }
}

PointCloudViewImp& PointCloudViewImp::operator=(const PointCloudViewImp& pointCloudView)
{
   if (this != &pointCloudView)
   {
      PerspectiveViewImp::operator= (pointCloudView);

      //set mpPrimaryPointCloud and update the ref count

      notify(SIGNAL_NAME(Subject, Modified));
   }
   return *this;
}

View* PointCloudViewImp::copy(QGLContext* drawContext, QWidget* parent) const
{
   std::string viewName = getName();

   PointCloudViewAdapter* pView = new PointCloudViewAdapter(SessionItemImp::generateUniqueId(), viewName,
      drawContext, parent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *dynamic_cast<PointCloudViewImp*>(pView) = *this;
   }

   return static_cast<View*>(pView);
}

bool PointCloudViewImp::copy(View *pView) const
{
   PointCloudViewImp* pViewImp = dynamic_cast<PointCloudViewImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

void PointCloudViewImp::setStretchType(const StretchType& stretch)
{
   if (stretch == mStretchType)
   {
      return;
   }
   mStretchType = stretch;
   repaint();
   notify(SIGNAL_NAME(Subject, Modified));
}

void PointCloudViewImp::setStretchType(QAction* pAction)
{
   StretchType stretch;
   if (pAction == mpLinearStretchAction)
   {
      stretch = LINEAR;
   }
   else if (pAction == mpLogStretchAction)
   {
      stretch = LOGARITHMIC;
   }
   else if (pAction == mpExpStretchAction)
   {
      stretch = EXPONENTIAL;
   }

   setStretchType(stretch);
}

void PointCloudViewImp::setPointColorizationType(QAction* pAction)
{
   PointColorizationType colorize;
   if (pAction == mpColorizeHeightAction)
   {
      colorize = POINT_HEIGHT;
   }
   else if (pAction == mpColorizeIntensityAction)
   {
      colorize = POINT_INTENSITY;
   }
   else if (pAction == mpColorizeClassificationAction)
   {
      colorize = POINT_CLASSIFICATION;
   }

   setPointColorizationType(colorize);
}

std::list<ContextMenuAction> PointCloudViewImp::getContextMenuActions() const
{
   std::list<ContextMenuAction> menuActions = PerspectiveViewImp::getContextMenuActions();

   menuActions.push_front(ContextMenuAction(mpStretchTypeMenu->menuAction(), APP_POINTCLOUDVIEW_STRETCH_MENU_ACTION));
   switch (mStretchType)
   {
   case LINEAR:
      mpLinearStretchAction->setChecked(true);
      break;
   case LOGARITHMIC:
      mpLogStretchAction->setChecked(true);
      break;
   case EXPONENTIAL:
      mpExpStretchAction->setChecked(true);
      break;
   default:
      ;// pass
   }

   menuActions.push_front(ContextMenuAction(mpColorizationMenu->menuAction(), APP_POINTCLOUDVIEW_COLORBY_MENU_ACTION));
   switch (mCurrentColorization)
   {
   case POINT_HEIGHT:
      mpColorizeHeightAction->setChecked(true);
      break;
   case POINT_INTENSITY:
      mpColorizeIntensityAction->setChecked(true);
      break;
   case POINT_CLASSIFICATION:
      mpColorizeClassificationAction->setChecked(true);
      break;
   default:
      ;// pass
   }

   return menuActions;
}

ViewType PointCloudViewImp::getViewType() const
{
   return POINT_CLOUD_VIEW;
}

bool PointCloudViewImp::setPrimaryPointCloud(PointCloudElement* pPointCloud)
{
   if (pPointCloud == NULL)
   {
      return false;
   }
   if (mpPrimaryPointCloud.get() != NULL)
   {
      return false; //primary can only set once per instance
   }
   mpPrimaryPointCloud.reset(pPointCloud);
   mVertexBufferUpToDate = false;
   updateVertexBufferIfNeeded();
   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

PointCloudElement* PointCloudViewImp::getPrimaryPointCloud()
{
   return mpPrimaryPointCloud.get();
}

void PointCloudViewImp::setPointColorizationType(PointColorizationType type)
{
   if (type != mCurrentColorization)
   {
      if (type == POINT_INTENSITY)
      {
         if (mpPrimaryPointCloud.get() != NULL)
         {
            PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(mpPrimaryPointCloud.get()->getDataDescriptor());
            if (pDescriptor != NULL && !pDescriptor->hasIntensityData())
            {
               return; //don't accept setting to POINT_INTENSITY if PointCloud doesn't have intensity data
            }
         }
      }
      else if (type == POINT_CLASSIFICATION)
      {
         if (mpPrimaryPointCloud.get() != NULL)
         {
            PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(mpPrimaryPointCloud.get()->getDataDescriptor());
            if (pDescriptor != NULL && !pDescriptor->hasClassificationData())
            {
               return; //don't accept setting to POINT_CLASSIFICATION if PointCloud doesn't have classification data
            }
         }
      }
      else if (type == POINT_HEIGHT)
      {
         double scale = 1.0;
         double offset = 0.0;
         if (mpPrimaryPointCloud.get() != NULL)
         {
            PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(mpPrimaryPointCloud->getDataDescriptor());
            if (pDescriptor != NULL)
            {
               scale = pDescriptor->getZScale();
               offset = pDescriptor->getZOffset();
            }
         }
         mLowerStretch = mMinZ * scale + offset;
         mUpperStretch = mMaxZ * scale + offset;
      }
      mCurrentColorization = type;
      mColorizationBufferUpToDate = false;
      mShaderProgsUpToDate = false;
   }
   repaint();
   notify(SIGNAL_NAME(Subject, Modified));
}

void PointCloudViewImp::zoomExtents()
{
   // reset the pitch so zoom extents works properly
   double newPitch = 90.0;
   DataOrigin dataOrigin = getDataOrigin();
   if (dataOrigin == UPPER_LEFT)
   {
      newPitch = -90.0;
   }
   flipTo(newPitch);
   
   // now zoom to extents
   PerspectiveViewImp::zoomExtents();
}

PointColorizationType PointCloudViewImp::getPointColorizationType() const
{
   return mCurrentColorization;
}

void PointCloudViewImp::setDecimation(uint32_t decimation)
{
   if (decimation != mDecimation)
   {
      mDecimation = decimation;
      mVertexBufferUpToDate = false;
      mColorizationBufferUpToDate = false;
      repaint();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

uint32_t PointCloudViewImp::getDecimation() const
{
   return mDecimation;
}

void PointCloudViewImp::setPointSize(float pointsize)
{
   if (pointsize >= 1. && pointsize != mPointSize)
   {
      mPointSize = pointsize;
      repaint();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

float PointCloudViewImp::getPointSize() const
{
   return mPointSize;
}

void PointCloudViewImp::setLowerStretch(double value)
{
   mLowerStretch = value;
   notify(SIGNAL_NAME(Subject, Modified));
}

double PointCloudViewImp::getLowerStretch() const
{
   return mLowerStretch;
}

void PointCloudViewImp::setUpperStretch(double value)
{
   mUpperStretch = value;
   notify(SIGNAL_NAME(Subject, Modified));
}

double PointCloudViewImp::getUpperStretch() const
{
   return mUpperStretch;
}

void PointCloudViewImp::setColorMap(const ColorMap& colorMap)
{
   if (colorMap == mColorMap)
   {
      return;
   }
   mColorMap = colorMap;
   mColorMapTextureUpToDate = false;
   notify(SIGNAL_NAME(Subject, Modified));
}

const ColorMap& PointCloudViewImp::getColorMap() const
{
   return mColorMap;
}

void PointCloudViewImp::setLowerStretchColor(ColorType lower)
{
   if (mLowerColor != lower)
   {
      mLowerColor = lower;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void PointCloudViewImp::setUpperStretchColor(ColorType upper)
{
   if (mUpperColor != upper)
   {
      mUpperColor = upper;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void PointCloudViewImp::setColorStretch(ColorType lower, ColorType upper)
{
   setLowerStretchColor(lower);
   setUpperStretchColor(upper);
}

ColorType PointCloudViewImp::getLowerStretchColor() const
{
   return mLowerColor;
}

ColorType PointCloudViewImp::getUpperStretchColor() const
{
   return mUpperColor;
}

bool PointCloudViewImp::isUsingColorMap() const
{
   return mUsingColorMap;
}

void PointCloudViewImp::setUsingColorMap(bool usingMap)
{
   if (usingMap == mUsingColorMap)
   {
      return;
   }
   mUsingColorMap = usingMap;
   mShaderProgsUpToDate = false;
   notify(SIGNAL_NAME(Subject, Modified));
}

void PointCloudViewImp::setZExaggeration(double value)
{
   mZExaggerationFactor = value;
   notify(SIGNAL_NAME(Subject, Modified));
}

double PointCloudViewImp::getZExaggeration()
{
   return mZExaggerationFactor;
}

double PointCloudViewImp::limitZoomPercentage(double dPercent)
{
   return dPercent;
}

void PointCloudViewImp::initializeDrawing()
{
   delete mpVertexBuffer;
   mpVertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
   mpVertexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
   mpVertexBuffer->create();
   mVertexBufferUpToDate = false;
   delete mpColorizationBuffer;
   mpColorizationBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
   mpColorizationBuffer->setUsagePattern(QGLBuffer::StaticDraw);
   mpColorizationBuffer->create();
   mColorizationBufferUpToDate = false;

   delete mpShaderProg;
   if (initShaders())
   {
      mpShaderProg = new QGLShaderProgram();
      mpShaderProg->bindAttributeLocation("vertex", MVERTEX_ATTRIB_NUM);
      mpShaderProg->bindAttributeLocation("colorAttribute", MCOLOR_ATTRIB_NUM);
   }

   updateMatrices();
   setupWorldMatrices();
}

void PointCloudViewImp::cleanupShaders()
{
   for (std::map<std::string, QGLShader*>::iterator iter = mShaders.begin();
        iter != mShaders.end();
        ++iter)
   {
      delete iter->second;
   }
   mShaders.clear();
}

bool PointCloudViewImp::initShaders()
{
   StepResource step("Compiling Shaders", "PointCloudView", "2cd9def6-c298-4579-b5c8-76bd358b8346", "Failure compiling shaders");
   cleanupShaders();
   std::auto_ptr<QGLShader> pVtxHeightColored(new QGLShader(QGLShader::Vertex));
   QFile shader(":/shaders/pointcloud/vertex_height");
   if (!shader.open(QFile::ReadOnly))
   {
      step->addProperty("Error", "VtxHeightColored Unable to load shader source");
      return false;
   }
   pVtxHeightColored->compileSourceCode(shader.readAll());
   shader.close();
   if (!pVtxHeightColored->isCompiled())
   {
      step->addProperty("VtxHeightColored Compilation Failure", pVtxHeightColored->log().toStdString());
      return false;
   }
   std::auto_ptr<QGLShader> pVtxAttributeColored(new QGLShader(QGLShader::Vertex));
   shader.setFileName(":/shaders/pointcloud/vertex_attribute");
   if (!shader.open(QFile::ReadOnly))
   {
      step->addProperty("Error", "VtxAttributeColored Unable to load shader source");
      return false;
   }
   pVtxAttributeColored->compileSourceCode(shader.readAll());
   shader.close();
   if (!pVtxAttributeColored->isCompiled())
   {
      step->addProperty("VtxAttributeColored Compilation Failure", pVtxAttributeColored->log().toStdString());
      return false;
   }
   std::auto_ptr<QGLShader> pFragColorStretch(new QGLShader(QGLShader::Fragment));
   shader.setFileName(":/shaders/pointcloud/fragment_color_stretch");
   if (!shader.open(QFile::ReadOnly))
   {
      step->addProperty("Error", "FragColorStretch Unable to load shader source");
      return false;
   }
   pFragColorStretch->compileSourceCode(shader.readAll());
   shader.close();
   if (!pFragColorStretch->isCompiled())
   {
      step->addProperty("FragColorStretch Compilation Failure", pFragColorStretch->log().toStdString());
      return false;
   }
   std::auto_ptr<QGLShader> pFragColorMap(new QGLShader(QGLShader::Fragment));
   shader.setFileName(":/shaders/pointcloud/fragment_color_map");
   if (!shader.open(QFile::ReadOnly))
   {
      step->addProperty("Error", "FragColorMap Unable to load shader source");
      return false;
   }
   pFragColorMap->compileSourceCode(shader.readAll());
   shader.close();
   if (!pFragColorMap->isCompiled())
   {
      step->addProperty("FragColorMap Compilation Failure", pFragColorMap->log().toStdString());
      return false;
   }
   mShaders.insert(make_pair("VtxHeightColored", pVtxHeightColored.release()));
   mShaders.insert(make_pair("VtxAttributeColored", pVtxAttributeColored.release()));
   mShaders.insert(make_pair("FragColorStretch", pFragColorStretch.release()));
   mShaders.insert(make_pair("FragColorMap", pFragColorMap.release()));
   step->finalize(Message::Success);
   return true;
}

void PointCloudViewImp::updateVertexBufferIfNeeded()
{
   if (mVertexBufferUpToDate)
   {
      return;
   }

   mTotalPoints = 0;
   if (mpVertexBuffer == NULL)
   {
      return;
   }
   PointCloudDataDescriptor* pDesc = dynamic_cast<PointCloudDataDescriptor*>(mpPrimaryPointCloud->getDataDescriptor());
   VERIFYNRV(pDesc != NULL);

   uint32_t pointCount = pDesc->getPointCount();
   GLfloat minX = std::numeric_limits<float>::max();
   GLfloat minY = std::numeric_limits<float>::max();
   GLfloat minZ = std::numeric_limits<float>::max();
   GLfloat maxX = -1.0 * minX;
   GLfloat maxY = -1.0 * minY;
   GLfloat maxZ = -1.0 * minZ;

   mta::StatusBarReporter queryReporter("Querying points", "app", "75711F5F-7286-4B5B-8F46-6E1EF33CAA19");
   int oldPercent = 0, curPercent = 0;
   PointCloudAccessor pAccessor = mpPrimaryPointCloud->getPointCloudAccessor();
   uint32_t validPointCount = 0;
   uint32_t decimation = mDecimation + 1;
   if (!pAccessor.isValid())
   {
      return;
   }
   if (!pAccessor->isPointValid())
   {
      pAccessor->nextValidPoint();
   }
   for (unsigned int i = 0; i < pointCount; i += decimation)
   {
      curPercent = i * 100 / pointCount;
      if (curPercent - oldPercent >= 1) 
      {
         queryReporter.reportProgress(min(curPercent, 99));
      }
      oldPercent = curPercent;
      if (!pAccessor.isValid())
      {
         return;
      }

      GLfloat xValue = pAccessor->getXAsDouble();
      minX = std::min(xValue, minX);
      maxX = std::max(xValue, maxX);
      GLfloat yValue = pAccessor->getYAsDouble();
      minY = std::min(yValue, minY);
      maxY = std::max(yValue, maxY);
      GLfloat zValue = pAccessor->getZAsDouble();
      minZ = std::min(zValue, minZ);
      maxZ = std::max(zValue, maxZ);
      validPointCount++;
      for (unsigned int skip = 0; (skip < decimation) && (i + skip + 1 < pointCount); ++skip)
      {
         pAccessor->nextValidPoint();
      }
   }
   queryReporter.reportProgress(100);

   mpVertexBuffer->bind();
   mpVertexBuffer->allocate(sizeof(GLfloat)*3*validPointCount);
   GLfloat* pBuffer = reinterpret_cast<GLfloat*>(mpVertexBuffer->map(QGLBuffer::ReadWrite));
   uint32_t origValidPointCount = validPointCount * decimation;
   while (pBuffer == NULL && validPointCount > 0 && decimation < 100)
   {
      // wasn't enough room, try to reallocate with more decimation
      decimation++;
      validPointCount = origValidPointCount / decimation;
      mpVertexBuffer->allocate(sizeof(GLfloat)*3*validPointCount);
      pBuffer = reinterpret_cast<GLfloat*>(mpVertexBuffer->map(QGLBuffer::ReadWrite));
   }
   if (pBuffer == NULL)
   {
      MessageResource("Unable to allocate space for point cloud vertex buffer.", "app", "{FC41CDE0-D1D2-4C29-BF98-58F81FA1A00F}");
      return;
   }
   if (decimation != (mDecimation + 1))
   {
      MessageResource msg("Point cloud is too large, decimation adjusted to compensate.", "app", "{1AF741FD-3C95-4361-A430-EF34F9E274F4}");
      mDecimation = decimation - 1;
   }
   double maxSpan = max(maxX - minX, maxY - minY);
   int count = 0;
   while (maxSpan > 100000)
   {
      maxSpan /= 10;
      count--;
   }
   mScaleFactor = pow(10.0, count);
   setExtents(0, 0, (maxX - minX) * mScaleFactor, (maxY - minY) * mScaleFactor);

   mta::StatusBarReporter barReporter("Transferring points", "app", "75711F5F-7286-4B5B-8F46-6E1EF33CAA19");
   oldPercent = 0;
   curPercent = 0;
   pAccessor->toIndex(0);
   if (!pAccessor.isValid())
   {
      return;
   }
   if (!pAccessor->isPointValid())
   {
      pAccessor->nextValidPoint();
   }
   GLfloat minZcalc = std::numeric_limits<float>::max();
   GLfloat maxZcalc = -1.0 * minZcalc;
   for (unsigned int i = 0; i < pointCount; i += decimation)
   {
      curPercent = i * 100 / pointCount;
      if (curPercent - oldPercent >= 1) 
      {
         barReporter.reportProgress(min(curPercent, 99));
      }
      oldPercent = curPercent;
      if (!pAccessor.isValid())
      {
         mpVertexBuffer->unmap();
         return;
      }

      *pBuffer = pAccessor->getXAsDouble() - minX;
      pBuffer++;
      *pBuffer = maxY - pAccessor->getYAsDouble();
      pBuffer++;
      GLfloat zValue;
      if (minZ < 0.0)
      {
         zValue = pAccessor->getZAsDouble() - minZ;
      }
      else
      {
         zValue = maxZ - pAccessor->getZAsDouble();
      }

      minZcalc = std::min(zValue, minZcalc);
      maxZcalc = std::max(zValue, maxZcalc);
      *pBuffer = zValue;
      pBuffer++;
      for (unsigned int skip = 0; (skip < decimation) && (i + skip + 1 < pointCount); ++skip)
      {
         pAccessor->nextValidPoint();
      }
   }
   mpVertexBuffer->unmap();
   mpShaderProg->setAttributeBuffer(MVERTEX_ATTRIB_NUM, GL_FLOAT, 0, 3, 0);
   double scale = pDesc->getZScale();
   double offset = pDesc->getZOffset();
   mUpperStretch = maxZ * scale + offset;
   mLowerStretch = minZ * scale + offset;
   mMaxZ = maxZ;
   mMinZ = minZ;
   // This should check x, y, and z as well as update dynamically
   // but for now, these should be better estimates than the hard-coded
   // values until we have proper statistics support for point clouds
   //mFrontPlane = std::min(mFrontPlane,mMinZ * 0.8);
   //mBackPlane = std::max(mBackPlane,mMaxZ * 1.5);
   mTotalPoints = validPointCount;
   barReporter.reportProgress(100);
   mVertexBufferUpToDate = true;
}

void PointCloudViewImp::updateColorMapTextureIfNeeded()
{
   if (mColorMapTextureUpToDate)
   {
      return;
   }
   mColorMapTextureUpToDate = true;
   if (mColorMapTexture != 0)
   {
      glDeleteTextures(1, &mColorMapTexture);
   }
   glGenTextures(1, &mColorMapTexture);
   if (mColorMapTexture == 0)
   {
      return;
   }
   glBindTexture(GL_TEXTURE_2D, mColorMapTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
   const vector<ColorType>& colors = mColorMap.getTable();
   unsigned char* colorMapBuffer = new unsigned char[1 * colors.size() * 4]; //RGBA texture with 1 column and colors.size() rows
   for (vector<ColorType>::size_type index = 0; index < colors.size(); ++index)
   {
      colorMapBuffer[index * 4 + 0] = colors[index].mRed;
      colorMapBuffer[index * 4 + 1] = colors[index].mGreen;
      colorMapBuffer[index * 4 + 2] = colors[index].mBlue;
      colorMapBuffer[index * 4 + 3] = colors[index].mAlpha;
   }
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, colors.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, colorMapBuffer);
   glBindTexture(GL_TEXTURE_2D, 0);
   delete [] colorMapBuffer;
}

void PointCloudViewImp::updateColorizationBufferIfNeeded()
{
   if (mColorizationBufferUpToDate)
   {
      return;
   }
   mColorizationBufferUpToDate = true;
   PointCloudDataDescriptor* pDesc = dynamic_cast<PointCloudDataDescriptor*>(mpPrimaryPointCloud->getDataDescriptor());
   VERIFYNRV(pDesc != NULL);
   if ((mCurrentColorization == POINT_INTENSITY && !pDesc->hasIntensityData()) ||
       (mCurrentColorization == POINT_CLASSIFICATION && !pDesc->hasClassificationData()))
   {
      mCurrentColorization = POINT_HEIGHT;
      mShaderProgsUpToDate = false;
   }
   if (mCurrentColorization == POINT_HEIGHT)
   {
      mpColorizationBuffer->bind();
      mpColorizationBuffer->allocate(0);
      mpColorizationBuffer->unmap();
      return;
   }

   uint32_t pointCount = pDesc->getPointCount();

   int oldPercent = 0, curPercent = 0;
   PointCloudAccessor pAccessor = mpPrimaryPointCloud->getPointCloudAccessor();
   uint32_t decimation = mDecimation + 1;
   uint32_t validPointCount = pointCount / decimation;
   if (!pAccessor.isValid())
   {
      return;
   }
   if (!pAccessor->isPointValid())
   {
      pAccessor->nextValidPoint();
   }

   mpColorizationBuffer->bind();
   mpColorizationBuffer->allocate(sizeof(GLfloat)*1*validPointCount);
   GLfloat* pBuffer = reinterpret_cast<GLfloat*>(mpVertexBuffer->map(QGLBuffer::ReadWrite));
   if (pBuffer == NULL)
   {
      return;
   }

   mta::StatusBarReporter barReporter("Transferring colorization data", "app", "58AC5F1A-BEFE-44E9-B292-D2E0D390A084");
   oldPercent = 0;
   curPercent = 0;
   if (!pAccessor.isValid())
   {
      return;
   }
   if (!pAccessor->isPointValid())
   {
      pAccessor->nextValidPoint();
   }
   GLfloat minCalc = std::numeric_limits<float>::max();
   GLfloat maxCalc = -1.0 * minCalc;
   for (unsigned int i = 0; i < pointCount; i += decimation)
   {
      curPercent = i * 100 / pointCount;
      if (curPercent - oldPercent >= 1) 
      {
         barReporter.reportProgress(min(curPercent, 99));
      }
      oldPercent = curPercent;
      if (!pAccessor.isValid())
      {
         mpColorizationBuffer->unmap();
         return;
      }

      GLfloat value;
      switch (mCurrentColorization)
      {
      case POINT_INTENSITY:
         value = pAccessor->getIntensityAsDouble();
         break;
      case POINT_CLASSIFICATION:
         value = pAccessor->getClassificationAsDouble();
         break;
      default:
         mpColorizationBuffer->unmap();
         return;
      }
      minCalc = std::min(value, minCalc);
      maxCalc = std::max(value, maxCalc);
      *pBuffer = value;
      pBuffer++;
      for (unsigned int skip = 0; (skip < decimation) && (i + skip + 1 < pointCount); ++skip)
      {
         pAccessor->nextValidPoint();
      }
   }
   mpColorizationBuffer->unmap();
   mpShaderProg->setAttributeBuffer(MCOLOR_ATTRIB_NUM, GL_FLOAT, 0, 1, 0);
   mUpperStretch = maxCalc;
   mLowerStretch = minCalc;
   if (mUpperStretch == mLowerStretch)
   {
      mUpperStretch += 1e-10f; // add epsilon to avoid dbz errors
   }
   barReporter.reportProgress(100);
}

void PointCloudViewImp::drawContents()
{
   glEnable(GL_DEPTH_TEST);
   if (mPointSize > 1.)
   {
      glEnable(GL_POINT_SMOOTH);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }
   else
   {
      glDisable(GL_POINT_SMOOTH);
   }
   glPointSize(mPointSize);

   setupWorldMatrices();
   if (mpShaderProg == NULL)
   {
      initializeDrawing();
   }
   if (mpShaderProg == NULL || mpPrimaryPointCloud.get() == NULL)
   {
      return;
   }
   glClear(GL_DEPTH_BUFFER_BIT); // this should really be in ViewImp::draw() but it'll work here for now
   updateVertexBufferIfNeeded();
   updateColorMapTextureIfNeeded();
   updateColorizationBufferIfNeeded();
   if (mTotalPoints == 0)
   {
      return;
   }
   bool success = true;
   mpShaderProg->enableAttributeArray(MVERTEX_ATTRIB_NUM);
   if (mCurrentColorization == POINT_HEIGHT)
   {
      mpShaderProg->disableAttributeArray(MCOLOR_ATTRIB_NUM);
   }
   else
   {
      mpShaderProg->enableAttributeArray(MCOLOR_ATTRIB_NUM);
   }
   if (!mShaderProgsUpToDate)
   {
      mpShaderProg->removeAllShaders();
      if (mCurrentColorization == POINT_HEIGHT)
      {
         mpShaderProg->addShader(mShaders.find("VtxHeightColored")->second);
      }
      else
      {
         mpShaderProg->addShader(mShaders.find("VtxAttributeColored")->second);         
      }
      if (mUsingColorMap)
      {
         mpShaderProg->addShader(mShaders.find("FragColorMap")->second);
      }
      else
      {
         mpShaderProg->addShader(mShaders.find("FragColorStretch")->second);
      }
      mpShaderProg->link();
      mShaderProgsUpToDate = true;
   }
   success = mpVertexBuffer->bind();
   mpShaderProg->setAttributeBuffer(MVERTEX_ATTRIB_NUM, GL_FLOAT, 0, 3, 0);
   if (mCurrentColorization != POINT_HEIGHT)
   {
      success = success && mpColorizationBuffer->bind();
      mpShaderProg->setAttributeBuffer(MCOLOR_ATTRIB_NUM, GL_FLOAT, 0, 1, 0);
   }
   success = success && mpShaderProg->bind();
   if (mUsingColorMap)
   {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, mColorMapTexture);
      mpShaderProg->setUniformValue("colorMap", 0);
   }
   else
   {
      mpShaderProg->setUniformValue("color1", COLORTYPE_TO_QCOLOR(mLowerColor));
      mpShaderProg->setUniformValue("color2", COLORTYPE_TO_QCOLOR(mUpperColor));
   }
   if (mCurrentColorization == POINT_HEIGHT)
   {
      double scale = 1.0;
      double offset = 0.0;
      if (mpPrimaryPointCloud.get() != NULL)
      {
         PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(mpPrimaryPointCloud->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            scale = pDescriptor->getZScale();
            offset = pDescriptor->getZOffset();
         }
      }
      GLfloat upper = ((mUpperStretch - offset) / scale);
      GLfloat lower = ((mLowerStretch - offset) / scale);
      if (mMinZ < 0.0)
      {
         mpShaderProg->setUniformValue("upper", upper - mMinZ); //need to fix this via model matrix
         mpShaderProg->setUniformValue("lower", lower - mMinZ);
      }
      else
      {
         mpShaderProg->setUniformValue("upper", mMaxZ - upper); //need to fix this via model matrix
         mpShaderProg->setUniformValue("lower", mMaxZ - lower);
      }
   }
   else
   {
      mpShaderProg->setUniformValue("upper", mUpperStretch);
      mpShaderProg->setUniformValue("lower", mLowerStretch);
   }
   mpShaderProg->setUniformValue("scaleFactor", static_cast<GLfloat>(mScaleFactor));
   mpShaderProg->setUniformValue("zExaggeration", static_cast<GLfloat>(mZExaggerationFactor));
   // Do not use uniform uint. Qt seems to have a bug where it transfers uint scalars glUniform1i() instead of glUniform1ui().
   GLint stretchTypeUni = -1; // invalid value which is easy to see in a GLSL debugger
   switch (mStretchType)
   {
   case LINEAR:
      stretchTypeUni = 0;
      break;
   case LOGARITHMIC:
      stretchTypeUni = 1;
      break;
   case EXPONENTIAL:
      stretchTypeUni = 2;
      break;
   default:
      ; // pass
   }
   mpShaderProg->setUniformValue("stretchType", stretchTypeUni); 
   if (success)
   {
      glDrawArrays(GL_POINTS, 0, mTotalPoints);
   }
   if (mCurrentColorization == POINT_INTENSITY)
   {
      mpColorizationBuffer->release();
   }
   mpVertexBuffer->release();
   mpShaderProg->release();
}

const std::string& PointCloudViewImp::getObjectType() const
{
   static std::string sType("PointCloudViewImp");
   return sType;
}

bool PointCloudViewImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudView"))
   {
      return true;
   }

   return PerspectiveViewImp::isKindOf(className);
}

bool PointCloudViewImp::isKindOfView(const std::string& className)
{
   if ((className == "PointCloudViewImp") || (className == "PointCloudView"))
   {
      return true;
   }

   return PerspectiveViewImp::isKindOfView(className);
}

void PointCloudViewImp::getViewTypes(std::vector<std::string>& classList)
{
   classList.push_back("PointCloudView");
   PerspectiveViewImp::getViewTypes(classList);
}

void PointCloudViewImp::mousePressEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      Qt::MouseButtons buttons = pEvent->buttons();
      if (buttons & Qt::MidButton)
      {
         mMouseActive = true;
         mMouseY = ptMouse.y();
      }
      PerspectiveViewImp::mousePressEvent(pEvent);
   }
}

void PointCloudViewImp::mouseMoveEvent(QMouseEvent* pEvent)
{
   setupWorldMatrices();
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setX(width() - pEvent->pos().x());
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      Qt::MouseButtons buttons = pEvent->buttons();
      // "flop" rotate if middle button down and moving up/down
      if (mMouseActive && buttons & Qt::MidButton)
      {
         const int dy = -(ptMouse.y() - mMouseY);
         flipBy(-dy);
         mMouseY = ptMouse.y();
         repaint();

         // now remove the y component and process zoom
         mMouseCurrent.setY(ptMouse.y());
      }
      else
      {
         PerspectiveViewImp::mouseMoveEvent(pEvent);
      }
   }
}

void PointCloudViewImp::mouseReleaseEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      mMouseActive = false;
      PerspectiveViewImp::mouseReleaseEvent(pEvent);
   }
}

void PointCloudViewImp::updateMatrices(int width, int height)
{
   if ((width <= 0) || (height <= 0))
   {
      return;
   }

   GlContextSave contextSave(this);

   // Save current matrices
   double modelMatrix[16];
   double projectionMatrix[16];
   int viewPort[4];
   glGetIntegerv(GL_VIEWPORT, viewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   // Update eyepoint
   PointType eyepoint;
   eyepoint.x = (mDist * cos((mHeading - 90.0) * PI / 180.0)) * cos(mPitch * PI / 180.0) + mCenter.mX;
   eyepoint.y = (mDist * sin((mHeading - 90.0) * PI / 180.0)) * cos(mPitch * PI / 180.0) + mCenter.mY;
   eyepoint.z = mDist * sin(mPitch * PI / 180.0);
   eyepoint.pitch = -mPitch;
   eyepoint.heading = 360.0 + mHeading;
   eyepoint.roll = 0.0;

   // Viewport
   glViewport(0, 0, width, height);

   // Projection matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(mFov, static_cast<double>(width) / static_cast<double>(height), mFrontPlane, mBackPlane);

   // Model matrix
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glRotatef(-90.0, 1, 0, 0);                   // now the y-axis is forward and z-axis points up
   glRotatef(-1 * eyepoint.roll, 0, 1, 0);      // roll about the y-axis
   glRotatef(-1 * eyepoint.pitch, 1, 0, 0);     // pitch about the x-axis degrees
   glRotatef(-1 * eyepoint.heading, 0, 0, 1);   // heading changes about the z-axis

   if (mPixelAspect > 1)
   {
      glScalef(mPixelAspect, 1, 1);
   }
   else
   {
      glScalef(1, 1/mPixelAspect, 1);
   }
   glTranslatef(-1 * eyepoint.x, -1 * eyepoint.y, -1 * eyepoint.z);

   // Update the member matrix values
   glGetIntegerv(GL_VIEWPORT, mViewPort);
   glGetDoublev(GL_PROJECTION_MATRIX, mProjMatrix);
   glGetDoublev(GL_MODELVIEW_MATRIX, mModelMatrix);

   // Restore current matrices
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(projectionMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixd(modelMatrix);
   glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);

   emit displayAreaChanged();
   notify(SIGNAL_NAME(PerspectiveView, DisplayAreaChanged), boost::any());
}

void PointCloudViewImp::updateStatusBar(const QPoint& screenCoord)
{
   // no special status displayed right now so do nothing
}
