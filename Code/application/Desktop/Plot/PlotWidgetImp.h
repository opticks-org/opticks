/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTWIDGETIMP_H
#define PLOTWIDGETIMP_H

#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>

#include "ColorType.h"
#include "FontImp.h"
#include "LocationType.h"
#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

class AnnotationToolBar;
class Axis;
class AxisImp;
class Classification;
class ElidedLabel;
class FloatingLabel;
class Legend;
class MouseMode;
class PlotObject;
class PlotSet;
class PlotView;
class PlotViewImp;
class QSplitter;
class SessionItemDeserializer;
class SessionItemSerializer;

class PlotWidgetImp : public QWidget, public SessionItemImp, public SubjectImp
{
   Q_OBJECT

public:
   PlotWidgetImp(const std::string& id, const std::string& plotName, PlotType plotType, PlotSet* pPlotSet,
      QWidget* parent = 0);
   ~PlotWidgetImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PlotSet* getPlotSet() const;
   PlotView* getPlot() const;

   void setName(const std::string& name);
   std::list<ContextMenuAction> getContextMenuActions() const;
   std::vector<std::string> getPropertiesPages() const;

   QImage getCurrentImage();
   bool getCurrentImage(QImage& image);
   QColor getBackgroundColor() const;

   // Classification
   void setClassificationPosition(PositionType ePosition);
   PositionType getClassificationPosition() const;
   void setClassificationText(const Classification* pClassification);
   QString getClassificationText() const;
   QFont getClassificationFont() const;
   QColor getClassificationColor() const;

   // Organization
   void setOrganizationPosition(PositionType ePosition);
   PositionType getOrganizationPosition() const;
   void setOrganizationText(const QString& strOrganization);
   QString getOrganizationText() const;
   void setOrganizationFont(const QFont& ftOrganization);
   QFont getOrganizationFont() const;
   void setOrganizationColor(const QColor& clrOrganization);
   QColor getOrganizationColor() const;

   // Title
   QString getTitle() const;
   QFont getTitleFont() const;

   // Axes
   void showAxis(AxisPosition axis, bool bShow);
   bool isAxisShown(AxisPosition axis) const;
   Axis* getAxis(AxisPosition axis) const;

   // Legend
   bool isLegendShown() const;
   Legend* getLegend() const;
   QColor getLegendBackgroundColor() const;

   // Session
   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer); 
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

   bool canRename() const;
   bool rename(const std::string &newName, std::string &errorMessage);

   QSplitter* getSplitter();

public slots:
   void setBackgroundColor(const QColor& backgroundColor);
   void setClassificationText(const QString& strClassification);
   void setClassificationFont(const QFont& ftClassification);
   void setClassificationColor(const QColor& clrClassification);
   void setTitle(const QString& strTitle);
   void setTitleFont(const QFont& ftTitle);
   void setTitleElideMode(Qt::TextElideMode mode);
   void showLegend(bool bShow);
   void setLegendBackgroundColor(const QColor& backgroundColor);
   void print(bool bDialog = true);

protected:
   bool event(QEvent* pEvent);
   void initialize(PlotViewImp *pPlotView, const std::string& plotName, PlotType plotType);
   bool eventFilter(QObject* o, QEvent* e);
   void contextMenuEvent(QContextMenuEvent* pEvent);

   const FontImp& getClassificationFontImp() const;
   const FontImp& getOrganizationFontImp() const;
   const FontImp& getTitleFontImp() const;

protected slots:
   void updateName(const QString& strName);
   void enableAnnotationToolBar(const MouseMode* pMouseMode);
   void updateScaleRange();
   void updateMouseLabel(const QString& strTextX, const QString& strTextY);
   void selectPlotObject(PlotObject* pObject, bool bSelect);

private:
   QAction* mpPrintAction;
   QAction* mpSeparatorAction;
   QAction* mpLegendAction;
   QAction* mpSeparator2Action;

   AnnotationToolBar* mpAnnotationToolBar;
   QWidget* mpPlotWidget;

   QLabel* mpTopLeftLabel;
   QLabel* mpTopCenterLabel;
   QLabel* mpTopRightLabel;
   QLabel* mpBottomLeftLabel;
   QLabel* mpBottomCenterLabel;
   QLabel* mpBottomRightLabel;

   FontImp mClassificationFont;
   FontImp mOrganizationFont;

   ElidedLabel* mpTitleLabel;
   FontImp mTitleFont;

   PlotSet* mpPlotSet;
   PlotViewImp* mpPlot;
   FloatingLabel* mpXMouseLabel;
   FloatingLabel* mpYMouseLabel;
   AxisImp* mpXAxis;
   AxisImp* mpYAxis;
   Legend* mpLegend;
   PositionType mClassificationPosition;
   PositionType mOrganizationPosition;

   QSplitter* mpSplitter;

   QString mClassificationText;
   QString mOrganizationText;

   QColor mClassificationColor;
   QColor mOrganizationColor;

   void setLabelText(const QString& strClassification, const QString& strOrganization);
   void setLabelFont(const QFont& ftClassification, const QFont& ftOrganization);
   void setLabelColor(const QColor& clrClassification, const QColor& clrOrganization);
};

#define PLOTWIDGETADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define PLOTWIDGETADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   const QWidget* getWidget() const \
   { \
      return static_cast<const QWidget*>(this); \
   } \
   QWidget* getWidget() \
   { \
      return static_cast<QWidget*>(this); \
   } \
   PlotSet* getPlotSet() const \
   { \
      return impClass::getPlotSet(); \
   } \
   PlotView* getPlot() const \
   { \
      return impClass::getPlot(); \
   } \
   void setBackgroundColor(const ColorType& backgroundColor) \
   { \
      impClass::setBackgroundColor(COLORTYPE_TO_QCOLOR(backgroundColor)); \
   } \
   ColorType getBackgroundColor() const \
   { \
      return QCOLOR_TO_COLORTYPE(impClass::getBackgroundColor()); \
   } \
   void setClassificationPosition(PositionType ePosition) \
   { \
      impClass::setClassificationPosition(ePosition); \
   } \
   PositionType getClassificationPosition() const \
   { \
      return impClass::getClassificationPosition(); \
   } \
   void setClassificationText(const Classification* pClassification) \
   { \
      impClass::setClassificationText(pClassification); \
   } \
   void setClassificationText(const std::string& classificationText) \
   { \
      impClass::setClassificationText(QString::fromStdString(classificationText)); \
   } \
   std::string getClassificationText() const \
   { \
      return impClass::getClassificationText().toStdString(); \
   } \
   void setClassificationFont(const Font& font) \
   { \
      impClass::setClassificationFont(font.getQFont()); \
   } \
   const Font& getClassificationFont() const \
   { \
      return impClass::getClassificationFontImp(); \
   } \
   void setClassificationColor(const ColorType& classificationColor) \
   { \
      QColor clrClassification; \
      if (classificationColor.isValid() == true) \
      { \
         clrClassification.setRgb(classificationColor.mRed, classificationColor.mGreen, classificationColor.mBlue); \
      } \
      \
      impClass::setClassificationColor(clrClassification); \
   } \
   ColorType getClassificationColor() const \
   { \
      ColorType classificationColor; \
      \
      QColor clrClassification = impClass::getClassificationColor(); \
      if (clrClassification.isValid() == true) \
      { \
         classificationColor.mRed = clrClassification.red(); \
         classificationColor.mGreen = clrClassification.green(); \
         classificationColor.mBlue = clrClassification.blue(); \
      } \
      \
      return classificationColor; \
   } \
   void setOrganizationPosition(PositionType ePosition) \
   { \
      impClass::setOrganizationPosition(ePosition); \
   } \
   PositionType getOrganizationPosition() const \
   { \
      return impClass::getOrganizationPosition(); \
   } \
   void setOrganizationText(const std::string& strOrganization) \
   { \
      impClass::setOrganizationText(QString::fromStdString(strOrganization)); \
   } \
   std::string getOrganizationText() const \
   { \
      return impClass::getOrganizationText().toStdString(); \
   } \
   void setOrganizationFont(const Font& font) \
   { \
      impClass::setOrganizationFont(font.getQFont()); \
   } \
   const Font& getOrganizationFont() const \
   { \
      return impClass::getOrganizationFontImp(); \
   } \
   void setOrganizationColor(const ColorType& clrOrganization) \
   { \
      QColor clrOrg; \
      if (clrOrganization.isValid() == true) \
      { \
         clrOrg.setRgb(clrOrganization.mRed, clrOrganization.mGreen, clrOrganization.mBlue); \
      } \
      \
      impClass::setOrganizationColor(clrOrg); \
   } \
   ColorType getOrganizationColor() const \
   { \
      ColorType orgColor; \
      \
      QColor clrOrg = impClass::getOrganizationColor(); \
      if (clrOrg.isValid() == true) \
      { \
         orgColor.mRed = clrOrg.red(); \
         orgColor.mGreen = clrOrg.green(); \
         orgColor.mBlue = clrOrg.blue(); \
      } \
      \
      return orgColor; \
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
   void showAxis(AxisPosition axis, bool bShow) \
   { \
      impClass::showAxis(axis, bShow); \
   } \
   bool isAxisShown(AxisPosition axis) const \
   { \
      return impClass::isAxisShown(axis); \
   } \
   Axis* getAxis(AxisPosition axis) const \
   { \
      return impClass::getAxis(axis); \
   } \
   void showLegend(bool bShow) \
   { \
      impClass::showLegend(bShow); \
   } \
   bool isLegendShown() const \
   { \
      return impClass::isLegendShown(); \
   } \
   void setLegendBackgroundColor(const ColorType& backgroundColor) \
   { \
      impClass::setLegendBackgroundColor(COLORTYPE_TO_QCOLOR(backgroundColor)); \
   } \
   ColorType getLegendBackgroundColor() const \
   { \
      return QCOLOR_TO_COLORTYPE(impClass::getLegendBackgroundColor()); \
   } \
   bool getCurrentImage(QImage& image) \
   { \
      return impClass::getCurrentImage(image); \
   } \
   void print(bool bPrintDialog = true) \
   { \
      impClass::print(bPrintDialog); \
   }

#endif
