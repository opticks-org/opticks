/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "AppVerify.h"
#include "CustomColorButton.h"
#include "FontSizeComboBox.h"
#include "GraphicTextWidget.h"

GraphicTextWidget::GraphicTextWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Text
   QLabel* pTextLabel = new QLabel("Text:", this);
   mpTextEdit = new QTextEdit(this);

   // Alignment
   QLabel* pAlignmentLabel = new QLabel("Alignment:", this);
   mpAlignmentCombo = new QComboBox(this);
   mpAlignmentCombo->setEditable(false);
   mpAlignmentCombo->addItem("Left");
   mpAlignmentCombo->addItem("Center");
   mpAlignmentCombo->addItem("Right");

   // Font
   QLabel* pFontLabel = new QLabel("Font:", this);
   mpFontCombo = new QFontComboBox(this);
   mpFontCombo->setEditable(false);

   QLabel* pFontSizeLabel = new QLabel("Font Size:", this);
   mpFontSizeCombo = new FontSizeComboBox(this);

   mpBoldCheck = new QCheckBox("Bold", this);
   mpItalicsCheck = new QCheckBox("Italics", this);
   mpUnderlineCheck = new QCheckBox("Underline", this);

   // Color
   QLabel* pColorLabel = new QLabel("Color:", this);
   mpColorButton = new CustomColorButton(this);
   mpColorButton->usePopupGrid(true);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pTextLabel, 0, 0, Qt::AlignTop);
   pGrid->addWidget(mpTextEdit, 0, 1, 1, 3);
   pGrid->addWidget(pAlignmentLabel, 1, 0);
   pGrid->addWidget(mpAlignmentCombo, 1, 1, Qt::AlignLeft);
   pGrid->addWidget(mpBoldCheck, 1, 3, Qt::AlignLeft);
   pGrid->addWidget(pFontLabel, 2, 0);
   pGrid->addWidget(mpFontCombo, 2, 1);
   pGrid->addWidget(mpItalicsCheck, 2, 3, Qt::AlignLeft);
   pGrid->addWidget(pFontSizeLabel, 3, 0);
   pGrid->addWidget(mpFontSizeCombo, 3, 1, Qt::AlignLeft);
   pGrid->addWidget(mpUnderlineCheck, 3, 3, Qt::AlignLeft);
   pGrid->addWidget(pColorLabel, 4, 0);
   pGrid->addWidget(mpColorButton, 4, 1, Qt::AlignLeft);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnMinimumWidth(2, 15);
   pGrid->setColumnStretch(3, 10);

   // Connections
   VERIFYNR(connect(mpTextEdit, SIGNAL(textChanged()), this, SLOT(notifyTextChange())));
   VERIFYNR(connect(mpAlignmentCombo, SIGNAL(activated(int)), this, SLOT(notifyAlignmentChange())));
   VERIFYNR(connect(mpFontCombo, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(notifyFontChange())));
   VERIFYNR(connect(mpFontSizeCombo, SIGNAL(valueChanged(int)), this, SLOT(notifyFontChange())));
   VERIFYNR(connect(mpBoldCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
   VERIFYNR(connect(mpItalicsCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
   VERIFYNR(connect(mpUnderlineCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
   VERIFYNR(connect(mpColorButton, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(colorChanged(const QColor&))));
}

GraphicTextWidget::~GraphicTextWidget()
{
}

QString GraphicTextWidget::getText() const
{
   return mpTextEdit->toPlainText();
}

int GraphicTextWidget::getAlignment() const
{
   int alignment = 0;

   QString strAlignment = mpAlignmentCombo->currentText();
   if (strAlignment == "Left")
   {
      alignment = Qt::AlignLeft;
   }
   else if (strAlignment == "Center")
   {
      alignment = Qt::AlignHCenter;
   }
   else if (strAlignment == "Right")
   {
      alignment = Qt::AlignRight;
   }

   return alignment;
}

QFont GraphicTextWidget::getTextFont() const
{
   QFont textFont(mpFontCombo->currentText(), mpFontSizeCombo->getCurrentValue());
   textFont.setBold(mpBoldCheck->isChecked());
   textFont.setItalic(mpItalicsCheck->isChecked());
   textFont.setUnderline(mpUnderlineCheck->isChecked());

   return textFont;
}

QColor GraphicTextWidget::getColor() const
{
   return mpColorButton->getColor();
}

void GraphicTextWidget::setText(const QString& text)
{
   if (text != getText())
   {
      mpTextEdit->setText(text);
   }
}

void GraphicTextWidget::setAlignment(int alignment)
{
   if (alignment != getAlignment())
   {
      if (alignment & Qt::AlignLeft)
      {
         mpAlignmentCombo->setCurrentIndex(0);
      }
      else if (alignment & Qt::AlignHCenter)
      {
         mpAlignmentCombo->setCurrentIndex(1);
      }
      else if (alignment & Qt::AlignRight)
      {
         mpAlignmentCombo->setCurrentIndex(2);
      }
      else
      {
         mpAlignmentCombo->clearEditText();
      }
   }
}

void GraphicTextWidget::setTextFont(const QFont& textFont)
{
   if (textFont != getTextFont())
   {
      VERIFYNR(disconnect(mpFontCombo, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(notifyFontChange())));
      VERIFYNR(disconnect(mpFontSizeCombo, SIGNAL(valueChanged(int)), this, SLOT(notifyFontChange())));
      VERIFYNR(disconnect(mpBoldCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
      VERIFYNR(disconnect(mpItalicsCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
      VERIFYNR(disconnect(mpUnderlineCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));

      mpFontCombo->setCurrentFont(textFont);
      mpFontSizeCombo->setCurrentValue(textFont.pointSize());
      mpBoldCheck->setChecked(textFont.bold());
      mpItalicsCheck->setChecked(textFont.italic());
      mpUnderlineCheck->setChecked(textFont.underline());

      VERIFYNR(connect(mpFontCombo, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(notifyFontChange())));
      VERIFYNR(connect(mpFontSizeCombo, SIGNAL(valueChanged(int)), this, SLOT(notifyFontChange())));
      VERIFYNR(connect(mpBoldCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
      VERIFYNR(connect(mpItalicsCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));
      VERIFYNR(connect(mpUnderlineCheck, SIGNAL(toggled(bool)), this, SLOT(notifyFontChange())));

      emit fontChanged(textFont);
   }
}

void GraphicTextWidget::setColor(const QColor& color)
{
   if (color != getColor())
   {
      mpColorButton->setColor(color);
   }
}

void GraphicTextWidget::notifyTextChange()
{
   QString text = getText();
   emit textChanged(text);
}

void GraphicTextWidget::notifyAlignmentChange()
{
   int alignment = getAlignment();
   emit alignmentChanged(alignment);
}

void GraphicTextWidget::notifyFontChange()
{
   QFont textFont = getTextFont();
   emit fontChanged(textFont);
}

void GraphicTextWidget::setTextReadOnly(bool bTextReadOnly)
{
   mpTextEdit->setReadOnly(bTextReadOnly);
}