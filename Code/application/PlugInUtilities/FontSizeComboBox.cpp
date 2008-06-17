/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFontDatabase>
#include <QtGui/QLineEdit>
#include <QtGui/QIntValidator>

#include "AppVerify.h"
#include "FontSizeComboBox.h"

FontSizeComboBox::FontSizeComboBox(QWidget* pParent) :
   QComboBox(pParent)
{
   // Initialization
   setEditable(true);
   setAutoCompletion(false);
   setInsertPolicy(QComboBox::NoInsert);
   setMinimumWidth(50);

   QIntValidator* pValidator = new QIntValidator(this);
   pValidator->setRange(1, 256);
   setValidator(pValidator);

   QList<int> fontSizes = QFontDatabase::standardSizes();
   for (int i = 0; i < fontSizes.count(); ++i)
   {
      addItem(QString::number(fontSizes[i]));
   }

   // Connections
   QLineEdit* pLineEdit = lineEdit();
   VERIFYNR(connect(pLineEdit, SIGNAL(returnPressed()), this, SLOT(translateEdited())));
   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated())));
   VERIFYNR(connect(this, SIGNAL(editTextChanged(const QString&)), this, SLOT(translateChanged())));
}

FontSizeComboBox::~FontSizeComboBox()
{
}

void FontSizeComboBox::setCurrentValue(int fontSize)
{
   QList<int> fontSizes = QFontDatabase::standardSizes();

   int index = fontSizes.indexOf(fontSize);
   if (index != -1)
   {
      setCurrentIndex(index);
   }
   else if (fontSize > 0)
   {
      setCurrentIndex(-1);
      setEditText(QString::number(fontSize));
   }
   else
   {
      setCurrentIndex(-1);
      clearEditText();
   }
}

int FontSizeComboBox::getCurrentValue() const
{
   int fontSize = -1;

   QString fontSizeText = currentText();
   if (fontSizeText.isEmpty() == false)
   {
      fontSize = fontSizeText.toInt();
   }

   return fontSize;
}

void FontSizeComboBox::translateEdited()
{
   int fontSize = getCurrentValue();
   emit valueEdited(fontSize);
}

void FontSizeComboBox::translateActivated()
{
   int fontSize = getCurrentValue();
   emit valueActivated(fontSize);
}

void FontSizeComboBox::translateChanged()
{
   int fontSize = getCurrentValue();
   emit valueChanged(fontSize);
}
