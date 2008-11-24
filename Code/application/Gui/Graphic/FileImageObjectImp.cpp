/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "FileImageObjectImp.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicProperty.h"
#include "View.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <QtGui/QImageReader>
#include <QtCore/QLibraryInfo>
using namespace std;

#if defined(WIN_API)
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

FileImageObjectImp::FileImageObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                       LocationType pixelCoord) :
   ImageObjectImp(id, type, pLayer, pixelCoord),
   mLoading(false)
{
   addProperty("Filename");
}

bool FileImageObjectImp::setProperty(const GraphicProperty *pProp)
{
   if (pProp == NULL)
   {
      return false;
   }

   bool bSuccess = ImageObjectImp::setProperty(pProp);
   if ((bSuccess == true) && (pProp->getName() == "Filename"))
   {
      QString lib = QLibraryInfo::location(QLibraryInfo::PluginsPath);
      const string& filename = static_cast<const FileNameProperty*>(pProp)->getFileName();
      if (filename.empty() == false)
      {
         QImage image(QString::fromStdString(filename));
         setImageData(image);

         if (mLoading == false)
         {
            updateBoundingBox();
         }
      }
   }

   return bSuccess;
}

bool FileImageObjectImp::processMousePress(LocationType screenCoord, 
                               Qt::MouseButton button,
                               Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers)
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);

   QWidget* pWidget = NULL;
   View* pView = pLayer->getView();
   if (pView != NULL)
   {
      pWidget = pView->getWidget();
   }

   char pwd[256];
   GETCWD(pwd, 256);
   QString filename = QFileDialog::getOpenFileName(pWidget, QString(), pwd,
      "Image Files (*.bmp; *.jpg);;All Files (*.*)");

   if (filename.isEmpty() == true)
   {
      pLayer->completeInsertion(false);
      return false;
   }

   if (setImageFile(filename.toStdString().c_str()) == false)
   {
      QMessageBox::critical(pWidget, "Image Error", "Unrecognized image format, or invalid image file");
      pLayer->completeInsertion(false);
      return false;
   }

   pLayer->completeInsertion();
   return true;
}

bool FileImageObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   mLoading = true;
   bool bSuccess = ImageObjectImp::fromXml(pDocument, version);
   mLoading = false;

   return bSuccess;
}

const string& FileImageObjectImp::getObjectType() const
{
   static string type("FileImageObjectImp");
   return type;
}

bool FileImageObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "FileImageObject"))
   {
      return true;
   }

   return ImageObjectImp::isKindOf(className);
}
