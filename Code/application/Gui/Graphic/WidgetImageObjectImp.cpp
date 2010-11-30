/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>

#include "WidgetImageObjectImp.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "GraphicImageWidget.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "PlotWidgetImp.h"
#include "ViewImp.h"

#include <vector>
using namespace std;

WidgetImageObjectImp::WidgetImageObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                           LocationType pixelCoord) :
   ImageObjectImp(id, type, pLayer, pixelCoord)
{
}

bool WidgetImageObjectImp::setWidget(QWidget* pWidget)
{
   if (pWidget == NULL)
   {
      return false;
   }

   QPixmap pixmap = QPixmap::grabWidget(pWidget);
   return setWidgetImage(pixmap.toImage());
}

bool WidgetImageObjectImp::setWidgetImage(const QImage& image)
{
   if (image.isNull() == true)
   {
      return false;
   }

   setImageData(image);
   updateBoundingBox();
   return true;
}

bool WidgetImageObjectImp::processMousePress(LocationType screenCoord, Qt::MouseButton button,
                                             Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayer != NULL)
      {
         QDialog dialog(dynamic_cast<ViewImp*>(pLayer->getView()));

         GraphicImageWidget* pImageWidget = new GraphicImageWidget(&dialog);
         pImageWidget->setImageSource(GraphicImageWidget::WIDGET);
         pImageWidget->setOpacityVisible(false);

         QDialogButtonBox* pButtonBox = new QDialogButtonBox(&dialog);
         pButtonBox->setOrientation(Qt::Horizontal);
         pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

         QVBoxLayout* pLayout = new QVBoxLayout(&dialog);
         pLayout->setMargin(10);
         pLayout->setSpacing(10);
         pLayout->addWidget(pImageWidget);
         pLayout->addStretch();
         pLayout->addWidget(pButtonBox, 0, Qt::AlignRight);

         dialog.setWindowTitle("Select Plot Image");
         dialog.resize(300, 65);

         VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &dialog, SLOT(accept())));
         VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &dialog, SLOT(reject())));

         if (dialog.exec() == QDialog::Rejected)
         {
            pLayer->completeInsertion(false);
            return false;
         }

         PlotWidgetImp* pWidget = dynamic_cast<PlotWidgetImp*>(pImageWidget->getImageWidget());
         if (pWidget == NULL)
         {
            pLayer->completeInsertion(false);
            return false;
         }

         if (setWidgetImage(pWidget->getCurrentImage()) == false)
         {
            QMessageBox::critical(dynamic_cast<ViewImp*>(pLayer->getView()), QString::fromStdString(APP_NAME),
               "Could not get the image of the current plot!  The object cannot be created.");
            pLayer->completeInsertion(false);
            return false;
         }

         pLayer->completeInsertion();
         return true;
      }
   }

   return false;
}

bool WidgetImageObjectImp::toXml(XMLWriter* pXml) const
{
   return false;
}

bool WidgetImageObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   return false;
}

const string& WidgetImageObjectImp::getObjectType() const
{
   static string type("WidgetImageObjectImp");
   return type;
}

bool WidgetImageObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WidgetImageObject"))
   {
      return true;
   }

   return ImageObjectImp::isKindOf(className);
}
