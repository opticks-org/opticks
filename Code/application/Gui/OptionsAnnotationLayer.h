/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSANNOTATIONLAYER_H
#define OPTIONSANNOTATIONLAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class ArcRegionComboBox;
class CustomColorButton;
class FillStyleComboBox;
class FontSizeComboBox;
class GraphicUnitsWidget;
class LineStyleComboBox;
class LineWidthComboBox;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFontComboBox;
class QSpinBox;
class SymbolTypeButton;

class OptionsAnnotationLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsAnnotationLayer();
   ~OptionsAnnotationLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Annotation Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Annotation";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display annotation layer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display annotation layer related options for the application";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT_MSG;
      return var;
   }

   static const std::string& getVersion()
   {
      static std::string var = APP_VERSION_NUMBER;
      return var;
   }

   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{463B4D84-BDC7-496a-AC05-6CDBC172C7D2}";
      return var;
   }

private:
   OptionsAnnotationLayer(const OptionsAnnotationLayer& rhs);
   OptionsAnnotationLayer& operator=(const OptionsAnnotationLayer& rhs);
   CustomColorButton* mpTextColor;
   QDoubleSpinBox* mpStartAngle;
   QDoubleSpinBox* mpStopAngle;
   QCheckBox* mpObjectFill;
   QCheckBox* mpObjectBorder;
   LineWidthComboBox* mpLineWidth;
   LineStyleComboBox* mpLineStyle;
   CustomColorButton* mpLineColor;
   SymbolTypeButton* mpHatchStyle;
   FontSizeComboBox* mpTextFontSize;
   QFontComboBox* mpTextFont;
   QCheckBox* mpBoldCheck;
   QCheckBox* mpItalicsCheck;
   QCheckBox* mpUnderlineCheck;
   FillStyleComboBox* mpFillStyle;
   CustomColorButton* mpFillColor;
   ArcRegionComboBox* mpArcRegion;
   QSpinBox* mpApex;
   QSpinBox* mpAlpha;
   GraphicUnitsWidget* mpUnitsWidget;
};

#endif
