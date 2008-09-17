/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SYMBOLTYPEGRID_H
#define SYMBOLTYPEGRID_H

#include "TypesFile.h"

#include "PixmapGrid.h"
#include "PixmapGridButton.h"

#include <map>

class SymbolTypeGrid : public PixmapGrid
{
   Q_OBJECT

public:
   SymbolTypeGrid(QWidget* pParent);
   void setCurrentValue(SymbolType value);
   SymbolType getCurrentValue() const;
   void setBorderedSymbols(bool show);

signals: 
   void valueChanged(SymbolType value);

private slots:
   void translateChange(const QString&);

private:
   QPixmap getSymbolPixmap(SymbolType eSymbol);

   std::map<SymbolType, QPixmap> mPixmaps;
};

class SymbolTypeButton : public PixmapGridButton
{
   Q_OBJECT

public:
   SymbolTypeButton(QWidget* pParent);

   void setCurrentValue(SymbolType value);
   SymbolType getCurrentValue() const;
   void setBorderedSymbols(bool show);

signals:
   void valueChanged(SymbolType value);

private slots:
   void translateChange();
};

#endif
