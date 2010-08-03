/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FRAMERATEWIDGET_H
#define FRAMERATEWIDGET_H

#include <QtGui/QWidget>

#include <boost/rational.hpp>
#include <vector>

class QComboBox;
class QSpinBox;

class FramerateWidget : public QWidget
{
   Q_OBJECT

public:
   FramerateWidget(QWidget* pParent = NULL);
   ~FramerateWidget();

   void setFramerates(const std::vector<boost::rational<int> >& framerates);
   void setFramerate(boost::rational<int> framerate);
   boost::rational<int> getFramerate() const;

protected slots:
   void framerateChanged(const QString& framerate);

private:
   QSpinBox* mpNumeratorSpin;
   QSpinBox* mpDenominatorSpin;
   QComboBox* mpFramerateCombo;
};

#endif
