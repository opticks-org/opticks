/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef SAMPLEGEOREFGUI_H
#define SAMPLEGEOREFGUI_H

#include <QtGui/QWidget>

class QCheckBox;
class QRadioButton;
class QSpinBox;

class SampleGeorefGui : public QWidget
{
public:
   SampleGeorefGui(void);
   ~SampleGeorefGui(void);

   int getXSize() const;
   int getYSize() const;
   bool getAnimated() const;
   bool getRotate() const;
   bool getExtrapolate() const;

private:
   QSpinBox* mpXSpin;
   QSpinBox* mpYSpin;
   QCheckBox* mpExtrapolateCheck;
   QCheckBox* mpAnimatedCheck;
   QRadioButton* mpRotateButton;
};

#endif
