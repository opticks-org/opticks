/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AXISIMP_H
#define AXISIMP_H

#include <QtGui/QWidget>
#include <qwt_scale_engine.h>
#include <qwt_scale_draw.h>

#include "ColorType.h"
#include "FontImp.h"
#include "Serializable.h"
#include "TypesFile.h"

#include <vector>

class AxisImp : public QWidget, public Serializable
{
   Q_OBJECT

public:
   AxisImp(AxisPosition position, QWidget* pParent = 0);
   ~AxisImp();

   // Position
   AxisPosition getPosition() const;

   // Title
   QString getTitle() const;
   QFont getTitleFont() const;
   QColor getTitleColor() const;

   // Scale type
   ScaleType getScaleType() const;

   // Values
   void setValueRange(double dMin, double dMax);
   double getMinimumValue() const;
   double getMaximumValue() const;
   std::vector<double> getMajorTickLocations() const;
   std::vector<double> getMinorTickLocations() const;
   int getMaxNumMajorTicks() const;
   int getMaxNumMinorTicks() const;

   // Value labels
   void setLabelFormat(const QString& strFormat);
   QString getLabelFormat() const;

   QSize sizeHint() const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setTitle(const QString& title);
   void setTitleFont(const QFont& titleFont);
   void setTitleColor(const QColor& titleColor);
   void setScaleType(ScaleType scaleType);
   void setMaxNumMajorTicks(int numTicks);
   void setMaxNumMinorTicks(int numTicks);

signals:
   void titleChanged(const QString& title);
   void titleFontChanged(const QFont& titleFont);
   void titleColorChanged(const QColor& titleColor);
   void scaleTypeChanged(ScaleType scaleType);
   void rangeChanged(double dMin, double dMax);
   void maxNumMajorTicksChanged(int numTicks);
   void maxNumMinorTicksChanged(int numTicks);

protected:
   void paintEvent(QPaintEvent* e);
   void updateSize();
   void updateScale();

   const FontImp& getTitleFontImp() const;

private:
   class ScaleDraw : public QwtScaleDraw
   {
   public:
      ScaleDraw() {}
      ~ScaleDraw() {}

      void setLabelFormat(const QString& strFormat)
      {
         mFormat = strFormat;
         invalidateCache();
      }

      QString getLabelFormat() const
      {
         return mFormat;
      }

      QwtText label(double value) const
      {
         QString strValue;
         if (mFormat.isEmpty() == false)
         {
            strValue.sprintf(mFormat.toLatin1(), value);
         }

         if (strValue.isEmpty() == true)
         {
            strValue = QString::number(value);
         }

         return QwtText(strValue);
      }

   private:
      QString mFormat;
   };

   QwtLinearScaleEngine mLinearScale;
   QwtLog10ScaleEngine mLogScale;
   ScaleDraw mScaleDraw;
   int mMaxMajorTicks;
   int mMaxMinorTicks;

   ScaleType mScaleType;
   QString mTitle;
   FontImp mTitleFont;
   QColor mTitleColor;
};

#define AXISADAPTER_METHODS(impClass) \
   AxisPosition getPosition() const \
   { \
      return impClass::getPosition(); \
   } \
   void setTitle(const std::string& title) \
   { \
      impClass::setTitle(QString::fromStdString(title)); \
   } \
   std::string getTitle() const \
   { \
      return impClass::getTitle().toStdString(); \
   } \
   void setTitleFont(const Font& font) \
   { \
      impClass::setTitleFont(font.getQFont()); \
   } \
   const Font& getTitleFont() const \
   { \
      return impClass::getTitleFontImp(); \
   } \
   void setTitleColor(const ColorType& titleColor) \
   { \
      impClass::setTitleColor(QColor(titleColor.mRed, titleColor.mGreen, titleColor.mBlue)); \
   } \
   ColorType getTitleColor() const \
   { \
      QColor titleColor = impClass::getTitleColor(); \
      return ColorType(titleColor.red(), titleColor.green(), titleColor.blue()); \
   } \
   void setScaleType(ScaleType scaleType) \
   { \
      impClass::setScaleType(scaleType); \
   } \
   ScaleType getScaleType() const \
   { \
      return impClass::getScaleType(); \
   } \
   void setValueRange(double dMin, double dMax) \
   { \
      impClass::setValueRange(dMin, dMax); \
   } \
   double getMinimumValue() const \
   { \
      return impClass::getMinimumValue(); \
   } \
   double getMaximumValue() const \
   { \
      return impClass::getMaximumValue(); \
   } \
   std::vector<double> getMajorTickLocations() const \
   { \
      return impClass::getMajorTickLocations(); \
   } \
   std::vector<double> getMinorTickLocations() const \
   { \
      return impClass::getMinorTickLocations(); \
   } \
   void setMaxNumMajorTicks(int numTicks) \
   { \
      impClass::setMaxNumMajorTicks(numTicks); \
   } \
   void setMaxNumMinorTicks(int numTicks) \
   { \
      impClass::setMaxNumMinorTicks(numTicks); \
   } \
   int getMaxNumMajorTicks() const \
   { \
      return impClass::getMaxNumMajorTicks(); \
   } \
   int getMaxNumMinorTicks() const \
   { \
      return impClass::getMaxNumMinorTicks(); \
   } \
   void setLabelFormat(const std::string& labelFormat) \
   { \
      impClass::setLabelFormat(QString::fromStdString(labelFormat)); \
   } \
   std::string getLabelFormat() const \
   { \
      return impClass::getLabelFormat().toStdString(); \
   }

#endif
