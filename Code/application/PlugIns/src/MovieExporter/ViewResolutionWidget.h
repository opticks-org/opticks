/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWRESOLUTIONWIDGET_H
#define VIEWRESOLUTIONWIDGET_H

#include <QtGui/QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;

class ViewResolutionWidget : public QWidget
{
   Q_OBJECT

public:
   ViewResolutionWidget(QWidget* pParent = NULL);
   ~ViewResolutionWidget();

   void setResolution(const QSize& size);
   QSize getResolution() const;

protected:
   void updateResolution();

protected slots:
   void viewResolutionToggled(bool useViewResolution);
   void widthEdited();
   void heightEdited();

private:
   QCheckBox* mpViewResolutionCheck;
   QLineEdit* mpWidthEdit;
   QLineEdit* mpHeightEdit;
   QPushButton* mpAspectLockButton;

   QSize mResolution;
};

#endif
