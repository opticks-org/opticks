/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONIMAGEPALETTEWIDGET_H
#define ANNOTATIONIMAGEPALETTEWIDGET_H

#include "AttachmentPtr.h"
#include "DesktopServices.h"
#include <boost/any.hpp>
#include <QtCore/QString>
#include <QtGui/QDirModel>
#include <QtGui/QToolBox>
#include <set>
#include <string>

class QListView;
class QShowEvent;
class SpatialDataView;
class Subject;
class Window;

class PaletteModel : public QDirModel
{
   Q_OBJECT

public:
   PaletteModel(QObject* pParent = NULL);
   virtual ~PaletteModel();
   QStringList mimeTypes() const;
   QMimeData* mimeData(const QModelIndexList& indexes) const;
};

class AnnotationImagePaletteWidget : public QToolBox
{
   Q_OBJECT

public:
   AnnotationImagePaletteWidget(QWidget* pParent = NULL);
   virtual ~AnnotationImagePaletteWidget();

   void windowAdded(Subject& subject, const std::string& signal, const boost::any& val);
   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& val);

public slots:
   void addPalette(const QString& path);
   void refresh();

protected:
   void showEvent(QShowEvent* pEvent);
   bool eventFilter(QObject* pObj, QEvent* pEvent);
   void windowDragEnterEvent(SpatialDataView* pWindow, QDragEnterEvent* pEvent);
   void windowDropEvent(SpatialDataView* pWindow, QDropEvent* pEvent);

private:
   AttachmentPtr<DesktopServices> mDesktopAttachments;
   std::set<Window*> mWindows;
   QMap<QString,QWidget*> mPalettes;
};

#endif