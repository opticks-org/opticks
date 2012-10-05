/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RESOLUTIONWIDGET_H
#define RESOLUTIONWIDGET_H

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>

class LabeledSection;

class ResolutionWidget : public QWidget
{
   Q_OBJECT

public:
   ResolutionWidget(QWidget* pParent = NULL);
   ~ResolutionWidget();

   void getResolution(unsigned int& width, unsigned int& height) const;
   void setResolution(unsigned int width, unsigned int height);
   bool getUseViewResolution();
   bool getAspectRatioLock();

public slots:
   void setUseViewResolution(bool state);
   void setAspectRatioLock(bool state);

private slots:
   void checkResolutionX(bool ignoreAspectRatio = false);
   void checkResolutionY(bool ignoreAspectRatio = false);

public:
   QCheckBox* mpUseViewResolution;
   QLabel* mpWidthLabel;
   QLabel* mpHeightLabel;
   QLineEdit* mpResolutionX;
   QLineEdit* mpResolutionY;
   QGroupBox* mpAspectSizeGroup;
   QRadioButton* mpFixedSizeRadio;
   QRadioButton* mpBoundingBoxRadio;
   unsigned int mCurrentResolutionX;
   unsigned int mCurrentResolutionY;
   LabeledSection* mpSettingsSection;

private:
   ResolutionWidget(const ResolutionWidget& rhs);
   ResolutionWidget& operator=(const ResolutionWidget& rhs);
};
#endif


