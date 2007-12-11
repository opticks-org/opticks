/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MULTI_LINE_TEXT_DIALOG_H
#define MULTI_LINE_TEXT_DIALOG_H

#include <QtGui/QDialog>

class QTextEdit;

class MultiLineTextDialog : public QDialog
{
   Q_OBJECT

public:
   MultiLineTextDialog(QWidget* pParent);
   QString getText() const; // return is empty() if user cancels
   void setText(const QString& text);

private:
   QTextEdit *mpEdit;
};

#endif
