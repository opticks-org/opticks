/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICLINEWIDGET_H
#define GRAPHICLINEWIDGET_H

#include <QtGui/QWidget>
#include "ColorType.h"
#include "TypesFile.h"

class CustomColorButton;
class LineStyleComboBox;
class LineWidthComboBox;
class QCheckBox;

class GraphicLineWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicLineWidget(QWidget* pParent = NULL);
   ~GraphicLineWidget();

   void setHideLineState(bool hide);
   bool getHideLineState() const;

   void setLineState(bool lineState);
   bool getLineState() const;

   void setLineStyle(LineStyle style);
   LineStyle getLineStyle() const;

   void setLineWidth(unsigned int width);
   unsigned int getLineWidth() const;

   void setLineColor(const ColorType& color);
   ColorType getLineColor() const;

   void setLineScaled(bool scaled);
   bool getLineScaled() const;

signals:
   void stateChanged(bool state);
   void styleChanged(LineStyle style);
   void widthChanged(unsigned int width);
   void colorChanged(const QColor& color);
   void scaledChanged(bool scaled);

private:
   GraphicLineWidget(const GraphicLineWidget& rhs);
   GraphicLineWidget& operator=(const GraphicLineWidget& rhs);
   QCheckBox* mpBorderCheck;
   LineStyleComboBox* mpLineStyleCombo;
   LineWidthComboBox* mpLineWidthCombo;
   CustomColorButton* mpLineColorButton;
   QCheckBox* mpScaledCheck;

};

#endif
