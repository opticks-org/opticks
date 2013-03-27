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

#include "OptionsMovieExporter.h"
#include <QtGui/QWidget>

class QLineEdit;
class QPushButton;
class QRadioButton;

class ViewResolutionWidget : public QWidget
{
   Q_OBJECT

public:
   ViewResolutionWidget(QWidget* pParent = NULL);
   virtual ~ViewResolutionWidget();

   void setResolution(const QSize& size, OptionsMovieExporter::ResolutionType resType);
   QSize getResolution() const;
   OptionsMovieExporter::ResolutionType getResolutionType() const;

protected:
   void updateResolution();

protected slots:
   void resolutionChanged();
   void widthEdited();
   void heightEdited();

private:
   ViewResolutionWidget(const ViewResolutionWidget& rhs);
   ViewResolutionWidget& operator=(const ViewResolutionWidget& rhs);
   QRadioButton* mpViewResolution;
   QRadioButton* mpFullResolution;
   QRadioButton* mpFixedResolution;
   QLineEdit* mpWidthEdit;
   QLineEdit* mpHeightEdit;
   QPushButton* mpAspectLockButton;

   QSize mResolution;
};

#endif
