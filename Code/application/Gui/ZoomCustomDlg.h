/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef ZOOMCUSTOMDLG_H
#define ZOOMCUSTOMDLG_H

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include "LocationType.h"

class ZoomCustomDlg : public QDialog
{
   Q_OBJECT

public:
   ZoomCustomDlg(QWidget* parent = 0);
   ~ZoomCustomDlg();

   void setZoomBox(const LocationType& llCorner, const LocationType& urCorner);
   void getZoomBox(LocationType& llCorner, LocationType& urCorner) const;

protected slots:
   void accept();

private:
   ZoomCustomDlg(const ZoomCustomDlg& rhs);
   ZoomCustomDlg& operator=(const ZoomCustomDlg& rhs);
   QLineEdit* mpMinXEdit;
   QLineEdit* mpMinYEdit;
   QLineEdit* mpMaxXEdit;
   QLineEdit* mpMaxYEdit;
};

#endif
