/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXTENSIONLISTDIALOG_H__
#define EXTENSIONLISTDIALOG_H__

#include <QtGui/QDialog>

class QListWidget;

class ExtensionListDialog : public QDialog
{
   Q_OBJECT

public:
   ExtensionListDialog(QWidget* pParent = NULL);
   virtual ~ExtensionListDialog();

private slots:
   void install();
   void reloadExtensions();

private:
   QListWidget* mpExtensionList;
};

#endif