/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPSYMBOLGRID_H
#define GCPSYMBOLGRID_H

#include "TypesFile.h"

#include "PixmapGrid.h"
#include "PixmapGridButton.h"

#include <map>

class GcpSymbolGrid : public PixmapGrid
{
   Q_OBJECT

public:
   GcpSymbolGrid(QWidget* pParent);
   void setCurrentValue(GcpSymbol value);
   GcpSymbol getCurrentValue() const;

signals: 
   void valueChanged(GcpSymbol value);

private slots:
   void translateChange(const QString&);
};

class GcpSymbolButton : public PixmapGridButton
{
   Q_OBJECT

public:
   GcpSymbolButton(QWidget* pParent);

   void setCurrentValue(GcpSymbol value);
   GcpSymbol getCurrentValue() const;

signals:
   void valueChanged(GcpSymbol value);

private slots:
   void translateChange();
};

#endif
