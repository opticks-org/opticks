/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MULTILINETEXTDIALOG_H
#define MULTILINETEXTDIALOG_H

#include <QtGui/QDialog>

#include "ConfigurationSettings.h"

#include <string>

class QTextEdit;

class MultiLineTextDialog : public QDialog
{
   Q_OBJECT

public:
   SETTING(Geometry, MultiLineTextDialog, std::string, "")

   MultiLineTextDialog(QWidget* pParent);
   QString getText() const; // return is empty() if user cancels
   void setText(const QString& text);

public slots:
   void accept();

private:
   MultiLineTextDialog(const MultiLineTextDialog& rhs);
   MultiLineTextDialog& operator=(const MultiLineTextDialog& rhs);
   QTextEdit* mpEdit;
};

#endif
