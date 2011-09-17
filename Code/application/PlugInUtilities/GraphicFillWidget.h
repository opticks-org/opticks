/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICFILLWIDGET_H
#define GRAPHICFILLWIDGET_H

#include <QtGui/QWidget>

#include "TypesFile.h"

class CustomColorButton;
class FillStyleComboBox;
class SymbolTypeButton;

class GraphicFillWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicFillWidget(QWidget* pParent = NULL);
   ~GraphicFillWidget();

   void setFillColor(const QColor& color);
   QColor getFillColor() const;

   void setFillStyle(FillStyle style);
   FillStyle getFillStyle() const;

   void setHatchStyle(SymbolType hatch);
   SymbolType getHatchStyle() const;

signals:
   void colorChanged(const QColor& color);
   void styleChanged(FillStyle style);
   void hatchChanged(SymbolType hatch);

private:
   FillStyleComboBox* mpFillStyleCombo;
   SymbolTypeButton* mpHatchStyle;
   CustomColorButton* mpFillColorButton;
};

#endif
