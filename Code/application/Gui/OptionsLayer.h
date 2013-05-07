/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSLAYER_H
#define OPTIONSLAYER_H

#include "AppVersion.h"

#include <QtGui/QWidget>

class QCheckBox;

class OptionsLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsLayer();
   virtual ~OptionsLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display options common to all layers";
      return var;
   }

   static const std::string& getShortDescription()
   {
      return getDescription();
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
      static std::string var = "{CE71F51B-724B-42D1-8DD5-F9A522E1DE2B}";
      return var;
   }

private:
   OptionsLayer(const OptionsLayer& rhs);
   OptionsLayer& operator=(const OptionsLayer& rhs);

   QCheckBox* mpRenameElementCheck;
   QCheckBox* mpWarnRenameCheck;
};

#endif
