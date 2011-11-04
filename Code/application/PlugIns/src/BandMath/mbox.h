/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MBOX_H
#define MBOX_H

#define MB_OK_ONLY           1
#define MB_CANCEL            2
#define MB_OK_CANCEL         3
#define MB_OK_CANCEL_ALWAYS  4

#include <QtGui/QCheckBox>
#include <QtGui/QDialog>

class MBox : public QDialog
{
    Q_OBJECT

public:
    MBox(const QString& strTitle, const QString& strMessage, int type, QWidget* parent = 0);
    ~MBox();

    QCheckBox* cbAlways;

private:
   MBox(const MBox& rhs);
   MBox& operator=(const MBox& rhs);
};

#endif
