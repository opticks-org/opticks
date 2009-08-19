/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLORMAPEDITOR_H
#define COLORMAPEDITOR_H

#include "ColorMap.h"
#include "ColorType.h"

#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <string>
#include <vector>

class CustomColorButton;
class HistogramPlotImp;
class Subject;

namespace boost
{
   class any;
}

class ColormapEditor : public QDialog
{
   Q_OBJECT

public:
   ColormapEditor(HistogramPlotImp &parent);
   ~ColormapEditor();

public slots:
   void accept();
   void distributeUniformly();
   void primaryPositionChanged(int newValue);
   void rangePositionChanged(int newValue);
   void saveColormap();
   void loadColormap();
   void applyColormap();

private slots:
   void updateColormap();
   void numPrimariesChanged(int newCount);
   void primaryColorChanged(const QColor &clrNew);

private:
   void histogramDeleted(Subject &subject, const std::string &signal, const boost::any& v);
   ColorMap::Gradient makeGradient() const;

   QVBoxLayout* mpVboxLayout;
   QHBoxLayout* mpHboxLayout;
   QLabel* mpPrimariesLabel;
   QSpinBox* mpPrimariesSpinBox;
   QSpacerItem* mpSpacerItem;
   QLabel* mpIndicesLabel;
   QSpinBox* mpIndicesSpinBox;
   QFrame* mpPrimaryView;
   QSlider* mpRangeMinSlider;
   QLabel* mpDisplay;
   QSlider* mpRangeMaxSlider;
   QHBoxLayout* mpHboxLayout1;
   QPushButton* mpSaveButton;
   QPushButton* mpApplyButton;
   QPushButton* mpLoadButton;
   QSpacerItem* mpSpacerItem1;
   QPushButton* mpOkButton;
   QPushButton* mpCloseButton;
   QGridLayout* mpPrimaryLayout;
   QPushButton* mpUniformButton;

   struct Primary
   {
      CustomColorButton* mpButton;
      QSlider* mpSlider;
      int mValue;
      QColor mColor;
   };
   std::vector<Primary> mPrimaries;
   HistogramPlotImp& mHistogram;
   static const int mInitialIndices = 250;
   static const int mSliderRange = 10000;
   std::vector<ColorType> mColormap;
   std::string mName;
   bool mIsApplied;
   bool mNeedDetach;
};
#endif
