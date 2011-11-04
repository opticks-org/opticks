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

/**
 *  A widget used to get text along with other customizable properties, such as font and color, from the user.
 *
 *  The graphic text widget will allow users to customize text, specifying alignment,
 *  font, and color. In addition, users may be able to modify the text. This widget is not
 *  meant to display the text but rather to allow the user to customize text which is displayed
 *  elsewhere, such as a Measurement or Plot.
 */
class GraphicTextWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates a graphic text widget.
    *
    *  @param   pParent
    *           The parent widget.
    */
   GraphicTextWidget(QWidget* pParent = NULL);

   /**
    *  Destroys the graphic text widget and all child items.
    */
   virtual ~GraphicTextWidget();

   /**
    * Returns the text to be displayed to the user.
    *
    *  @return  The text to be displayed to the user. By default, an empty string will be returned.
    */
   QString getText() const;

   /**
    * Returns the alignment which should be used to display the text.
    * This will correspond to one or more of the values in Qt::AlignmentFlag.
    *
    *  @return  The alignment to use when displaying the text. By default, the alignment is Qt::AlignLeft.
    */
   int getAlignment() const;

   /**
    * Returns the font which should be used to display the text.
    *
    *  @return  The font to use when displaying the text.
    *           By default, the QFontComboBox default font is used without bold, italics, or underlining.
    *           By default, the FontSizeComboBox default font size is used.
    */
   QFont getTextFont() const;

   /**
    * Returns the color which should be used to display the text.
    *
    *  @return  The color to use when displaying the text. By default, the CustomColorButton default color is used.
    */
   QColor getColor() const;

public slots:
   /**
    * Sets the text to be displayed to the user.
    *
    *  @param   text
    *           The text to display.
    */
   void setText(const QString& text);

   /**
    * Sets the alignment which should be used to display the text.
    * By default, the alignment is Qt::AlignLeft.
    *
    *  @param   alignment
    *           The alignment to use.
    *           The only supported values are Qt::AlignLeft, Qt::AlignHCenter, and Qt::AlignRight.
    */
   void setAlignment(int alignment);

   /**
    * Sets the font which should be used to display the text.
    * By default, the QFontComboBox default font is used without bold, italics, or underlining.
    * By default, the FontSizeComboBox default font size is used.
    *
    *  @param   textFont
    *           The font to use.
    */
   void setTextFont(const QFont& textFont);

   /**
    * Sets the color which should be used to display the text.
    * By default, the CustomColorButton default color is used.
    *
    *  @param   color
    *           The color to use.
    */
   void setColor(const QColor& color);

   /**
    * Sets whether the user can modify the text.
    * By default, the text can be edited by the user.
    *
    *  @param   bTextReadOnly
    *           \b True to disable editing, \b false otherwise.
    */
   void setTextReadOnly(bool bTextReadOnly);

signals:
   /**
    *  Emitted when the text is changed.
    *
    *  @param   text
    *           The newly set text.
    */
   void textChanged(const QString& text);

   /**
    *  Emitted when the alignment is changed.
    *
    *  @param   alignment
    *           The newly set alignment.
    */
   void alignmentChanged(int alignment);

   /**
    *  Emitted when the font is changed.
    *
    *  @param   textFont
    *           The newly set font.
    */
   void fontChanged(const QFont& textFont);

   /**
    *  Emitted when the color is changed.
    *
    *  @param   color
    *           The newly set color.
    */
   void colorChanged(const QColor& color);

protected slots:
   /**
    *  Causes textChanged() to be emitted.
    */
   void notifyTextChange();

   /**
    *  Causes alignmentChanged() to be emitted.
    */
   void notifyAlignmentChange();

   /**
    *  Causes fontChanged() to be emitted.
    */
   void notifyFontChange();

private:
   GraphicTextWidget(const GraphicTextWidget& rhs);
   GraphicTextWidget& operator=(const GraphicTextWidget& rhs);
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
