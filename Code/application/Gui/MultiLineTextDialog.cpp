/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>

#include "AppVerify.h"
#include "MultiLineTextDialog.h"

using namespace std;

MultiLineTextDialog::MultiLineTextDialog(QWidget *pParent) :
   QDialog(pParent)
{
   // Text edit
   mpEdit = new QTextEdit(this);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOK = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(mpEdit, 0, 0, 1, 3);
   pGrid->addWidget(pLine, 1, 0, 1, 3);
   pGrid->addWidget(pOK, 2, 1);
   pGrid->addWidget(pCancel, 2, 2);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   setWindowTitle("Enter Text");
   setModal(true);

   setWhatsThis("Metadata fields may be included by specifying a metadata field using $M(specification) where "
      "specification is:<ul>"
      "<li>a metadata path which will be loaded from a default location"
      "<li>//<i>dataset_name</i>//<i>path</i> -- where <i>dataset_name</i> is the name of the element"
      "<li>//[<i>view_object_name</i>]//<i>path</i> -- where <i>view_object_name</i> is the name of a view object "
      "in a product view</ul>"
      "Examples:<ul>"
      "<li>$M(NITF/File Header/ONAME)"
      "<li>$M(//C:/Data/mydata.ntf-I1//NITF/File Header/ONAME)"
      "<li>$M(//[View 1]//NITF/File Header/ONAME)</ul><hr/>"
      "Other special replacements are specified with $S(specification) where "
      "specification uses the same rules as $M() but instead of a metadata "
      "path, use a variable name. The only variable currently supported is:<ul>"
      "<li>CLASSIFICATION - Replace with the classification string of the specified dataset</ul>");

   const string geometry = MultiLineTextDialog::getSettingGeometry();
   if (geometry.empty() == false)
   {
      VERIFYNR(restoreGeometry(QByteArray::fromBase64(QByteArray(geometry.c_str(), geometry.length()))));
   }

   // Connections
   VERIFYNR(connect(pOK, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancel, SIGNAL(clicked()), this, SLOT(reject())));
}

void MultiLineTextDialog::accept()
{
   MultiLineTextDialog::setSettingGeometry(QString(saveGeometry().toBase64()).toStdString());
   QDialog::accept();
}

QString MultiLineTextDialog::getText() const
{
   QString text;
   if (result() == QDialog::Accepted)
   {
      text = mpEdit->toPlainText();
   }

   return text;
}

void MultiLineTextDialog::setText(const QString& text)
{
   mpEdit->setText(text);
}
