/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSMOVIEEXPORTER_H
#define OPTIONSMOVIEEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"

#include <string>

class AdvancedOptionsWidget;
class BitrateWidget;
class ViewResolutionWidget;

class OptionsMovieExporter : public LabeledSectionGroup
{
public:
   OptionsMovieExporter();
   ~OptionsMovieExporter();

   SETTING(Width, MovieExporter, unsigned int, 0);
   SETTING(Height, MovieExporter, unsigned int, 0);
   SETTING(Bitrate, MovieExporter, unsigned int, 0);
   SETTING(MeMethod, MovieExporter, std::string, std::string());
   SETTING(GopSize, MovieExporter, int, 0);
   SETTING(QCompress, MovieExporter, float, 0.0);
   SETTING(QBlur, MovieExporter, float, 0.0);
   SETTING(QMinimum, MovieExporter, int, 0);
   SETTING(QMaximum, MovieExporter, int, 0);
   SETTING(QDiffMaximum, MovieExporter, int, 0);
   SETTING(MaxBFrames, MovieExporter, int, 0);
   SETTING(BQuantFactor, MovieExporter, float, 0.0);
   SETTING(BQuantOffset, MovieExporter, float, 0.0);
   SETTING(DiaSize, MovieExporter, int, 0);
   SETTING(Flags, MovieExporter, int, 0);

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Movie Product Exporter Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Export/Movie Product";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display movie product exporter related options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display movie product exporter related options";
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
      static std::string var = "{77309BDE-ED51-4e83-B2AF-5532823AB9F0}";
      return var;
   }

private:
   ViewResolutionWidget* mpResolutionWidget;
   BitrateWidget* mpBitrateWidget;
   AdvancedOptionsWidget* mpAdvancedWidget;
};

#endif
