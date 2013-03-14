/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEDLG_H
#define GEOREFERENCEDLG_H

#include <QtGui/QDialog>

class GeoreferenceWidget;
class RasterDataDescriptor;

class GeoreferenceDlg : public QDialog
{
   Q_OBJECT

public:
   GeoreferenceDlg(QWidget* pParent = NULL);
   virtual ~GeoreferenceDlg();

   void setDataDescriptor(RasterDataDescriptor* pDescriptor);
   RasterDataDescriptor* getDataDescriptor();
   const RasterDataDescriptor* getDataDescriptor() const;

public slots:
   virtual void accept();

private:
   GeoreferenceDlg(const GeoreferenceDlg& rhs);
   GeoreferenceDlg& operator=(const GeoreferenceDlg& rhs);

   GeoreferenceWidget* mpGeorefWidget;
};

#endif
