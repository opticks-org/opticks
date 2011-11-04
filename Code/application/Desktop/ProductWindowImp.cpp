/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QImage>
#include <QtGui/QPrinter>

#include "ProductWindowImp.h"
#include "PrintPixmap.h"
#include "ProductViewAdapter.h"

using namespace std;

ProductWindowImp::ProductWindowImp(const string& id, const string& windowName, QWidget* parent) :
   WorkspaceWindowImp(id, windowName, parent)
{
   createView(QString::fromStdString(windowName), PRODUCT_VIEW);
}

ProductWindowImp::~ProductWindowImp()
{
}

const string& ProductWindowImp::getObjectType() const
{
   static string type("ProductWindowImp");
   return type;
}

bool ProductWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ProductWindow"))
   {
      return true;
   }

   return WorkspaceWindowImp::isKindOf(className);
}

WindowType ProductWindowImp::getWindowType() const
{
   return PRODUCT_WINDOW;
}

View* ProductWindowImp::createView(const QString& strViewName, const ViewType& viewType)
{
   if (getView() == NULL)
   {
      return WorkspaceWindowImp::createView(strViewName, viewType);
   }

   return NULL;
}

void ProductWindowImp::setWidget(QWidget* pWidget)
{
   if (getWidget() == NULL)
   {
      WorkspaceWindowImp::setWidget(pWidget);
   }
}

struct PaperType
{
   double height;
   double width;
   QPrinter::PageSize size;
};

const PaperType types[] =
{
   { 11.0, 8.5, QPrinter::Letter },
   { 17.0, 11.0, QPrinter::Tabloid },
   { 10.0, 7.5, QPrinter::Executive },
   { 14.0, 8.5, QPrinter::Legal },
   { 330.0/25.4, 210.0/25.4, QPrinter::Folio },
   { 220.0/25.4, 110.0/25.4, QPrinter::DLE },
   { 1189.0/25.4, 841.0/25.4, QPrinter::A0 },
   { 841.0/25.4, 594.0/25.4, QPrinter::A1 },
   { 594.0/25.4, 420.0/25.4, QPrinter::A2 },
   { 420.0/25.4, 297.0/25.4, QPrinter::A3 },
   { 297.0/25.4, 210.0/25.4, QPrinter::A4 },
   { 210.0/25.4, 148.0/25.4, QPrinter::A5 },
   { 148.0/25.4, 105.0/25.4, QPrinter::A6 },
   { 105.0/25.4, 74.0/25.4, QPrinter::A7 },
   { 74.0/25.4, 52.0/25.4, QPrinter::A8 },
   { 52.0/25.4, 37.0/25.4, QPrinter::A9 },
   { 1456.0/25.4, 1030.0/25.4, QPrinter::B0 },
   { 1030.0/25.4, 728.0/25.4, QPrinter::B1 },
   { 728.0/25.4, 515.0/25.4, QPrinter::B2 },
   { 515.0/25.4, 364.0/25.4, QPrinter::B3 },
   { 364.0/25.4, 257.0/25.4, QPrinter::B4 },
   { 257.0/25.4, 182.0/25.4, QPrinter::B5 },
   { 182.0/25.4, 128.0/25.4, QPrinter::B6 },
   { 128.0/25.4, 91.0/25.4, QPrinter::B7 },
   { 91.0/25.4, 64.0/25.4, QPrinter::B8 },
   { 64.0/25.4, 45.0/25.4, QPrinter::B9 },
   { 45.0/25.4, 32.0/25.4, QPrinter::B10 }
};

void ProductWindowImp::print(bool bSetupDialog)
{
   ProductView* pView = NULL;
   pView = getProductView();
   if (pView == NULL)
   {
      return;
   }

   // Get the paper image
   QImage paperImage = (static_cast<ProductViewAdapter*> (pView))->getPaperImage();
   if (paperImage.isNull() == true)
   {
      return;
   }

   // Create the printer object
   QPrinter printer;

   // Set the orientation
   double dPaperWidth = 0.0;
   double dPaperHeight = 0.0;
   pView->getPaperSize(dPaperWidth, dPaperHeight);

   printer.setOrientation((dPaperWidth > dPaperHeight) ? QPrinter::Landscape : QPrinter::Portrait);

   // Set the page size
   QPrinter::PageSize pageSize = QPrinter::Letter;    // default to 8.5 x 11
   double dHeight = max(dPaperWidth, dPaperHeight);
   double dWidth = min(dPaperWidth, dPaperHeight);

   const double fudge = 0.05;
   for (unsigned int i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
   {
      if (fabs(dHeight - types[i].height) < fudge && fabs(dWidth - types[i].width) < fudge)
      {
         pageSize = types[i].size;
         break;
      }
   }

   printer.setPageSize(pageSize);

   // Print the image
   QPixmap paperPixmap = QPixmap::fromImage(paperImage);
   PrintPixmap(paperPixmap, bSetupDialog, this, &printer);
}

ProductView* ProductWindowImp::getProductView() const
{
   ProductView* pView = static_cast<ProductView*>(getView());
   return pView;
}
