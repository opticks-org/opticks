/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERCONVERSIONTYPECOMBOBOX_H
#define RASTERCONVERSIONTYPECOMBOBOX_H

#include "ModisUtilities.h"

#include <QtGui/QComboBox>

class RasterConversionTypeComboBox : public QComboBox
{
   Q_OBJECT

public:
   RasterConversionTypeComboBox(QWidget* pParent);
   virtual ~RasterConversionTypeComboBox();

   void setRasterConversion(ModisUtilities::RasterConversionType rasterConversion);
   ModisUtilities::RasterConversionType getRasterConversion() const;

signals:
   void rasterConversionChanged(ModisUtilities::RasterConversionType rasterConversion);

private slots:
   void translateIndexChanged(const QString& text);
};

#endif
