/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CgmObject.h"
#include "AppConfig.h"
#include "ConfigurationSettings.h"
#include "AppVerify.h"
#include "FileFinder.h"
#include "glCommon.h"
#include "GraphicObject.h"
#include "GraphicObjectImp.h"
#include "GraphicObjectFactory.h"
#include "FileResource.h"
#include "ObjectResource.h"
#include "SymbolManager.h"

#include <QtCore/QDir>
#include <QtGui/QImage>
#include <QtOpenGL/QGLPixelBuffer>
#include <QtOpenGL/QGLFormat>

using namespace std;

SymbolManager *SymbolManager::mpSingleton = NULL;

SymbolManager::SymbolManager()
{
}

SymbolManager::~SymbolManager()
{
}

SymbolManager *SymbolManager::instance()
{
   if (mpSingleton == NULL)
   {
      mpSingleton = new SymbolManager;
   }
   return mpSingleton;
}

GraphicObject *SymbolManager::getSymbol(const std::string &symbolName)
{
   GraphicObject* pObject = mSymbols[symbolName];
   if (pObject == NULL)
   {
      pObject = loadSymbol(symbolName);
   }

   return pObject;
}

GraphicObject* SymbolManager::loadSymbol(const string& symbolName)
{
   GraphicObject* pObject = NULL;
   string symbolPath;
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      symbolPath = pSupportFilesPath->getFullPathAndName() + SLASH + "Annotation" + SLASH + symbolName;
   }

   GraphicResource<CgmObject> pCgm(CGM_OBJECT);
   if (pCgm->deserializeCgm(symbolPath))
   {
      pObject = pCgm.release();
   }

   if (pObject != NULL)
   {
      mSymbols[symbolName] = pObject;
   }

   return pObject;
}

void SymbolManager::drawSymbols(const string& symbolName, const vector<LocationType>& points, 
      double screenSize, double zoomFactor, double rotation, double pitch, double xScale,
      double yScale, double objectRotation)
{
   GraphicObjectImp* pSymbol = dynamic_cast<GraphicObjectImp*>(getSymbol(symbolName));
   if (pSymbol == NULL)
   {
      return;
   }

   LocationType ll = pSymbol->getLlCorner();
   LocationType ur = pSymbol->getUrCorner();

   LocationType center = ll + ur;
   center.mX /= 2;
   center.mY /= 2;

   LocationType nativeSize = ur - ll;

   double maxSize = max(nativeSize.mX, nativeSize.mY);
   double scale = screenSize / maxSize;
   
   glMatrixMode(GL_MODELVIEW);

   for (vector<LocationType>::const_iterator iter = points.begin(); 
      iter != points.end(); ++iter)
   {
      glPushMatrix();
      glTranslated(iter->mX, iter->mY, 0.0);
      
      glRotated(-objectRotation, 0, 0, 1);
      glScaled(1/xScale, 1/yScale, 1);
      glRotated(rotation, 0, 0, 1);
      glRotated(pitch+90, 1, 0, 0);
      glScaled(1/zoomFactor, 1/zoomFactor, 1);
      glScaled(scale, scale, 1.0);
      glTranslated(center.mX, center.mY, 0.0);

      pSymbol->draw(scale);

      glPopMatrix();
   }
}

const QImage &SymbolManager::getSymbolImage(const std::string &symbolName)
{
   static QImage image;
   static const int SIZE = 128;

   QGLFormat format(QGL::Rgba | QGL::AlphaChannel);
   QGLPixelBuffer pixelBuffer(QSize(SIZE, SIZE), format);
   pixelBuffer.makeCurrent();

   glLoadIdentity();
   gluOrtho2D(-SIZE/2.0, SIZE/2.0, -SIZE/2.0, SIZE/2.0);
   glClearColor(0, 0, 0, 0);
   glClear(GL_COLOR_BUFFER_BIT);
   
   vector<LocationType> points(1, LocationType(0, 0));
   drawSymbols(symbolName, points, SIZE/2);

   image = pixelBuffer.toImage();
   return image;
}

const vector<string> &SymbolManager::getAvailableSymbolNames() const
{
   static vector<string> symbolNames;
   symbolNames.clear();

   string annoPath;
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      annoPath = pSupportFilesPath->getFullPathAndName();
   }

   QDir dir(QString::fromStdString(annoPath + SLASH + "Annotation"), "*.cgm");
   QStringList entryList = dir.entryList();
   
   for (QStringList::const_iterator iter = entryList.begin(); 
      iter != entryList.end(); ++iter)
   {
      symbolNames.push_back(iter->toStdString());
   }

   return symbolNames;
}
