/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <QtWidgets/QDialog>

class AboutDlg : public QDialog
{
   Q_OBJECT

public:
   AboutDlg(QWidget* parent = 0);
   ~AboutDlg();

private:
   AboutDlg(const AboutDlg& rhs);
   AboutDlg& operator=(const AboutDlg& rhs);
};

#endif
