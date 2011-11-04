/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICTRIANGLEWIDGET_H
#define GRAPHICTRIANGLEWIDGET_H

#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

class GraphicTriangleWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicTriangleWidget(QWidget* pParent = NULL);
   ~GraphicTriangleWidget();

   int getApex() const;

public slots:
   void setApex(int apex);

signals:
   void apexChanged(int apex);

private:
   GraphicTriangleWidget(const GraphicTriangleWidget& rhs);
   GraphicTriangleWidget& operator=(const GraphicTriangleWidget& rhs);
   QSpinBox* mpApexSpin;
};

#endif
