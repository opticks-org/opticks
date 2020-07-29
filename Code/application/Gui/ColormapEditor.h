/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLORMAPEDITOR_H
#define COLORMAPEDITOR_H

#include "ColorMap.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
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
   ColormapEditor(const ColormapEditor& rhs);
   ColormapEditor& operator=(const ColormapEditor& rhs);
   void histogramDeleted(Subject &subject, const std::string &signal, const boost::any& v);
   ColorMap::Gradient makeGradient() const;
   std::vector<ColorType> makeCubeHelix() const;

   QTabWidget* mpTabWidget;
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

   QDoubleSpinBox* mpCHStart;
   QDoubleSpinBox* mpCHRotations;
   QDoubleSpinBox* mpCHHue;
   QDoubleSpinBox* mpCHGamma;
   QCheckBox* mpCHReverse;

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
   ColorMap mColormap;
   std::string mName;
   bool mIsApplied;
   bool mNeedDetach;
};
#endif
