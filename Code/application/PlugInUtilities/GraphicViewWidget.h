/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICVIEWWIDGET_H
#define GRAPHICVIEWWIDGET_H

#include <QtGui/QComboBox>
#include <QtGui/QWidget>

#include <vector>

class View;

class GraphicViewWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicViewWidget(QWidget* pParent = NULL);
   ~GraphicViewWidget();

   View* getView() const;

public slots:
   void setView(View* pView);

signals:
   void viewChanged(View* pView);

protected:
   void addView(View* pView);

protected slots:
   void notifyViewChange();

private:
   GraphicViewWidget(const GraphicViewWidget& rhs);
   GraphicViewWidget& operator=(const GraphicViewWidget& rhs);
   std::vector<View*> mViews;
   QComboBox* mpViewCombo;
};

#endif
