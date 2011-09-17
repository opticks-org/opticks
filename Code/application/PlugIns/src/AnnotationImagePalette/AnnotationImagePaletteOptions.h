/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONIMAGEPALETTEOPTIONS_H
#define ANNOTATIONIMAGEPALETTEOPTIONS_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"
#include <vector>

class CustomTreeWidget;
class Filename;
class QCheckBox;
class QSpinBox;

class AnnotationImagePaletteOptions : public LabeledSectionGroup
{
   Q_OBJECT

public:
   AnnotationImagePaletteOptions();
   ~AnnotationImagePaletteOptions();

   SETTING(PaletteDirs, AnnotationImagePalette, std::vector<Filename*>, std::vector<Filename*>());
   SETTING(InitialWidth, AnnotationImagePalette, int, 0);

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Annotation Image Palette Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Annotation Image Palette";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display options for the Annotation Image Palette";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display options for the Annotation Image Palette";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT;
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
      static std::string var = "{F23CD792-DD28-4190-AD84-073DA5FC3639}";
      return var;
   }

protected slots:
   void addEntry();
   void removeEntry();
   void setDirty(bool isDirty=true);

protected:
   void setPalettePaths(const std::vector<Filename*>& paths);
   std::vector<Filename*> getPalettePaths() const;
   void setInitialWidth(int width);

private:
   CustomTreeWidget* mpDirWidget;
   QSpinBox* mpInitialWidth;
   QCheckBox* mpFullSize;
   bool mDirty;
};

#endif