/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FONTSIZECOMBOBOX_H
#define FONTSIZECOMBOBOX_H

#include <QtGui/QComboBox>

/**
 *  A widget used to choose a font size from an editable combo box.
 */
class FontSizeComboBox : public QComboBox
{
   Q_OBJECT

public:
   /**
    *  Creates a font size combo box.
    *  The QFontDatabase will be queried to determine valid font sizes.
    *
    *  @param   pParent
    *           The parent widget.
    */
   FontSizeComboBox(QWidget* pParent);

   /**
    *  Destroys the font size combo box and all child items.
    */
   virtual ~FontSizeComboBox();

   /**
    * Sets the font size.
    *
    *  @param   fontSize
    *           The font size to use.
    *           If this value is greater than zero and does not exist in the combo box, it will be added and selected.
    *           If this value is not greater than zero, the combo box will be cleared and set to an invalid index.
    */
   void setCurrentValue(int fontSize);

   /**
    * Returns the selected font size.
    *
    *  @return   The selected font size or -1 if no font size is selected.
    */
   int getCurrentValue() const;

signals:
   /**
    * Emitted when the user presses the \b Enter key with the combo box active.
    *
    *  @param   fontSize
    *           The selected font size.
    */
   void valueEdited(int fontSize);

   /**
    * Emitted when a new combo box item is activated by the user.
    *
    *  @param   fontSize
    *           The selected font size.
    */
   void valueActivated(int fontSize);

   /**
    * Emitted when the user changes the value of the combo box.
    *
    *  @param   fontSize
    *           The selected font size.
    */
   void valueChanged(int fontSize);

private slots:
   void translateEdited();
   void translateActivated();
   void translateChanged();
};

#endif
