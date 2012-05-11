/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationImagePaletteOptions.h"
#include "AnnotationImagePaletteWidget.h"
#include "AnnotationLayer.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "GraphicObject.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Undo.h"

#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QDirModel>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QFileIconProvider>
#include <QtGui/QIcon>
#include <QtGui/QImageReader>
#include <QtGui/QListView>
#include <QtGui/QMessageBox>

namespace
{
   class QImageFileIconProvider : public QFileIconProvider
   {
   public:
      QImageFileIconProvider() {}
      virtual ~QImageFileIconProvider() {}
      using QFileIconProvider::icon;
      virtual QIcon icon(const QFileInfo& info) const
      {
         return QIcon(info.absoluteFilePath());
      }
   private:
      QImageFileIconProvider(const QImageFileIconProvider& rhs);
      QImageFileIconProvider& operator=(const QImageFileIconProvider& rhs);
   };

   QString dndMimeType("text/x-annotation-image-filename");
}

PaletteModel::PaletteModel(QObject* pParent) : QDirModel(pParent)
{
}

PaletteModel::~PaletteModel()
{
}

QStringList PaletteModel::mimeTypes() const
{
   return QStringList() << dndMimeType;
}

QMimeData* PaletteModel::mimeData(const QModelIndexList& indexes) const
{
   QMimeData* pUrlData = QDirModel::mimeData(indexes);
   if (pUrlData == NULL)
   {
      return NULL;
   }
   QMimeData* pData = new QMimeData();
   QList<QUrl> urls = pUrlData->urls();
   if (!urls.empty())
   {
      pData->setData(dndMimeType, urls.front().toString().toAscii());
   }
   delete pUrlData;
   return pData;
}

AnnotationImagePaletteWidget::AnnotationImagePaletteWidget(QWidget* pParent) : QToolBox(pParent)
{
   mDesktopAttachments.addSignal(SIGNAL_NAME(DesktopServices, WindowAdded),
      Slot(this, &AnnotationImagePaletteWidget::windowAdded));
   mDesktopAttachments.addSignal(SIGNAL_NAME(DesktopServices, WindowRemoved),
      Slot(this, &AnnotationImagePaletteWidget::windowRemoved));
   mDesktopAttachments.reset(Service<DesktopServices>().get());
   std::vector<Window*> windows;
   Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (std::vector<Window*>::iterator window = windows.begin(); window != windows.end(); ++window)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*window);
      if (pWindow != NULL)
      {
         pWindow->getWidget()->installEventFilter(this);
         pWindow->getWidget()->setAcceptDrops(true);
         mWindows.insert(pWindow);
      }
   }
   setContextMenuPolicy(Qt::ActionsContextMenu);
   QAction* pRefreshAction = new QAction("Refresh", this);
   pRefreshAction->setAutoRepeat(false);
   addAction(pRefreshAction);
   VERIFYNR(connect(pRefreshAction, SIGNAL(triggered()), this, SLOT(refresh())));

   setMinimumHeight(50);
}

AnnotationImagePaletteWidget::~AnnotationImagePaletteWidget()
{
   for (std::set<Window*>::iterator window = mWindows.begin(); window != mWindows.end(); ++window)
   {
      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(*window);
      pWindow->getWidget()->removeEventFilter(this);
      // Don't disable drops in case another plug-in is expecting drops enabled
   }
}

void AnnotationImagePaletteWidget::windowAdded(Subject& subject, const std::string& signal, const boost::any& val)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(boost::any_cast<Window*>(val));
   if (pWindow != NULL)
   {
      pWindow->getWidget()->installEventFilter(this);
      pWindow->getWidget()->setAcceptDrops(true);
      mWindows.insert(pWindow);
   }
}

void AnnotationImagePaletteWidget::windowRemoved(Subject& subject, const std::string& signal, const boost::any& val)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(boost::any_cast<Window*>(val));
   if (pWindow != NULL)
   {
      pWindow->getWidget()->removeEventFilter(this);
      // Don't disable drops in case another plug-in is expecting drops enabled
   }
   mWindows.erase(boost::any_cast<Window*>(val));
}

void AnnotationImagePaletteWidget::addPalette(const QString& path)
{
   QListView* pPalette = new QListView(this);
   pPalette->setWrapping(true);
   pPalette->setLayoutMode(QListView::Batched);
   pPalette->setBatchSize(10);
   pPalette->setMovement(QListView::Static);
   pPalette->setFlow(QListView::LeftToRight);
   pPalette->setIconSize(QSize(32, 32));
   pPalette->setViewMode(QListView::IconMode);
   pPalette->setDragEnabled(true);
   pPalette->setDragDropMode(QAbstractItemView::DragOnly);
   pPalette->setSelectionMode(QAbstractItemView::SingleSelection);
   pPalette->setResizeMode(QListView::Adjust);

   if (QFileInfo(path).isDir())
   {
      QDirModel* pModel = new PaletteModel(this);
      pModel->setFilter(QDir::Files | QDir::Readable);
      QStringList formats;
      QList<QByteArray> rawFormats = QImageReader::supportedImageFormats();
      foreach (const QByteArray& rawFormat, rawFormats)
      {
         formats << "*." + QString(rawFormat);
      }
      pModel->setNameFilters(formats);
      pModel->setIconProvider(new QImageFileIconProvider());
      pPalette->setModel(pModel);
      pPalette->setRootIndex(pModel->index(path));
   }
   // If this ends in a slash, QFileInfo will report an empty string basename
   QString tmpPath(path);
   if (tmpPath.endsWith('/') || tmpPath.endsWith('\\'))
   {
      tmpPath.remove(tmpPath.size() - 1, 1);
   }
   tmpPath = QFileInfo(tmpPath).baseName();
   if (tmpPath.isEmpty())
   {
      // use the whole path...this can happen on Windows if the path is the root of a disk (c:/ for example)
      tmpPath = path;
   }
   int idx = addItem(pPalette, tmpPath);
   setItemToolTip(idx, path);
   mPalettes[path] = pPalette;
}

void AnnotationImagePaletteWidget::refresh()
{
   while(count() > 0)
   {
      removeItem(0);
   }
   mPalettes.clear();

   std::vector<Filename*> paths = AnnotationImagePaletteOptions::getSettingPaletteDirs();
   for (std::vector<Filename*>::iterator path = paths.begin(); path != paths.end(); ++path)
   {
      if (*path == NULL)
      {
         continue;
      }
      QString strPath = QString::fromStdString((*path)->getFullPathAndName());
      if (!mPalettes.contains(strPath))
      {
         addPalette(strPath);
      }
   }
}

void AnnotationImagePaletteWidget::showEvent(QShowEvent* pEvent)
{
   refresh();
}

bool AnnotationImagePaletteWidget::eventFilter(QObject* pObj, QEvent* pEvent)
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pObj);
   switch(pEvent->type())
   {
   case QEvent::DragEnter:
      windowDragEnterEvent(pView, static_cast<QDragEnterEvent*>(pEvent));
      return pEvent->isAccepted();
   case QEvent::Drop:
      windowDropEvent(pView, static_cast<QDropEvent*>(pEvent));
      return pEvent->isAccepted();
   default:
      ; // nothing
   }
   return false;
}

void AnnotationImagePaletteWidget::windowDragEnterEvent(SpatialDataView* pView, QDragEnterEvent* pEvent)
{
   if (pEvent != NULL && pEvent->mimeData()->hasFormat(dndMimeType))
   {
      pEvent->acceptProposedAction();
   }
}

void AnnotationImagePaletteWidget::windowDropEvent(SpatialDataView* pView, QDropEvent* pEvent)
{
   VERIFYNRV(pEvent && pView);

   QString filename = QUrl(pEvent->mimeData()->data(dndMimeType)).toLocalFile();
   if (filename.isEmpty())
   {
      return;
   }

   UndoGroup undoGroup(pView, "Drop Annotation Image");

   // Find an annotation layer.
   //  if the active layer is an AL, use it
   //  elif there is a top-most AL, activate and use it
   //  else create a new AL and use it
   bool layerCreated = false;

   AnnotationLayer* pLayer = dynamic_cast<AnnotationLayer*>(pView->getActiveLayer());
   if (pLayer == NULL)
   {
      pLayer = static_cast<AnnotationLayer*>(pView->getTopMostLayer(ANNOTATION));
   }
   if (pLayer == NULL)
   {
      pLayer = static_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL));
      layerCreated = true;
   }
   VERIFYNRV(pLayer);
   pView->setActiveLayer(pLayer);
   pView->setMouseMode("LayerMode");
   pLayer->setCurrentGraphicObjectType(ROTATE_OBJECT);

   GraphicObject* pObject = pLayer->addObject(FILE_IMAGE_OBJECT);
   VERIFYNRV(pObject);
   if (!pObject->setImageFile(filename.toAscii()))
   {
      pLayer->removeObject(pObject, true);
      if (layerCreated == true)
      {
         pView->deleteLayer(pLayer);
      }

      QMessageBox::critical(this, "Annotation Image Palette", "The image could not be loaded from the file.");

      //#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : The undo group is still added to the view.  This " \
      //   "should be removed when capability exists to delay adding undo group actions to the stack. (dsulgrov)")
      return;
   }
   LocationType corner;
   double yCoord = pView->getWidget()->height() - pEvent->pos().y();
   pLayer->translateScreenToData(pEvent->pos().x(), yCoord, corner.mX, corner.mY);
   pView->refresh();
   LocationType sz = pObject->getUrCorner() - pObject->getLlCorner();
   int width = AnnotationImagePaletteOptions::getSettingInitialWidth();
   if (width > 0)
   {
      double aspect = sz.mY / sz.mX;
      sz.mX = width;
      sz.mY = width * aspect;
   }
   sz.mX = abs(sz.mX) / 2.0;
   sz.mY = -abs(sz.mY) / 2.0;
   pObject->setBoundingBox(corner - sz, corner + sz);
   pView->refresh();
   pEvent->accept();
}
