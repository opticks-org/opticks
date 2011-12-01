/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ADVANCEDOPTIONSWIDGET_H
#define ADVANCEDOPTIONSWIDGET_H

#include <QtGui/QWidget>

#include <string>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;

class AdvancedOptionsWidget : public QWidget
{
public:
   AdvancedOptionsWidget(QWidget* pParent = NULL);
   ~AdvancedOptionsWidget();

   std::string getMeMethod() const;
   void setMeMethod(const std::string& method);
   int getGopSize() const;
   void setGopSize(int size);
   float getQCompress() const;
   void setQCompress(float val);
   float getQBlur() const;
   void setQBlur(float val);
   int getQMinimum() const;
   void setQMinimum(int val);
   int getQMaximum() const;
   void setQMaximum(int val);
   int getQDiffMaximum() const;
   void setQDiffMaximum(int val);
   int getMaxBFrames() const;
   void setMaxBFrames(int val);
   float getBQuantFactor() const;
   void setBQuantFactor(float val);
   float getBQuantOffset() const;
   void setBQuantOffset(float val);
   int getDiaSize() const;
   void setDiaSize(int val);
   int getOutputBufferSize() const;
   void setOutputBufferSize(int val);
   int getFlags() const;
   void setFlags(int val);

private:
   AdvancedOptionsWidget(const AdvancedOptionsWidget& rhs);
   AdvancedOptionsWidget& operator=(const AdvancedOptionsWidget& rhs);
   QComboBox* mpMeMethod;
   QSpinBox* mpGopSize;
   QDoubleSpinBox* mpQCompress;
   QDoubleSpinBox* mpQBlur;
   QSpinBox* mpQMinimum;
   QSpinBox* mpQMaximum;
   QSpinBox* mpQDiffMaximum;
   QSpinBox* mpMaxBFrames;
   QDoubleSpinBox* mpBQuantFactor;
   QDoubleSpinBox* mpBQuantOffset;
   QSpinBox* mpDiaSize;
   QSpinBox* mpOutputBufferSize;
   QCheckBox* mpQScale;
   QCheckBox* mpQPel;
   QCheckBox* mpGmc;
   QCheckBox* mpNormalizeAqp;
   QCheckBox* mpTrellis;
   QCheckBox* mpAcPred;
   QCheckBox* mpCbpRd;
   QCheckBox* mpQpRd;
   QCheckBox* mpObmc;
   QCheckBox* mpClosedGop;
};

#endif
