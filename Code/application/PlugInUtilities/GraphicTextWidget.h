/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICTEXTWIDGET_H
#define GRAPHICTEXTWIDGET_H

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

class CustomColorButton;
class FontSizeComboBox;

class GraphicTextWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicTextWidget(QWidget* pParent = NULL);
   ~GraphicTextWidget();

   QString getText() const;
   int getAlignment() const;
   QFont getTextFont() const;
   QColor getColor() const;

public slots:
   void setText(const QString& text);
   void setAlignment(int alignment);
   void setTextFont(const QFont& textFont);
   void setColor(const QColor& color);
   void setTextReadOnly(bool bTextReadOnly);

signals:
   void textChanged(const QString& text);
   void alignmentChanged(int alignment);
   void fontChanged(const QFont& textFont);
   void colorChanged(const QColor& color);

protected slots:
   void notifyTextChange();
   void notifyAlignmentChange();
   void notifyFontChange();

private:
   QTextEdit* mpTextEdit;
   QComboBox* mpAlignmentCombo;
   QFontComboBox* mpFontCombo;
   FontSizeComboBox* mpFontSizeCombo;
   CustomColorButton* mpColorButton;
   QCheckBox* mpBoldCheck;
   QCheckBox* mpItalicsCheck;
   QCheckBox* mpUnderlineCheck;
};

#endif
