/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSUPPRESSIBLEMSGDLG_H
#define OPTIONSUPPRESSIBLEMSGDLG_H

#include "LabeledSectionGroup.h"

class CustomTreeWidget;

class OptionsSuppressibleMsgDlg : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsSuppressibleMsgDlg();
   ~OptionsSuppressibleMsgDlg();

   static const std::string& getName()
   {
      static std::string var = "Example Suppressible Dialog Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Plug-in Sampler/Example Suppressible Dialog Options";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Plug-in Sampler Example Suppressible Dialog Options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Plug-in Sampler Example Suppressible Dialog Options";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Opticks Community";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = "Copyright (C) 2009, Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getVersion()
   {
      static std::string var = "Sample";
      return var;
   }

   static bool isProduction()
   {
      return false;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{9BB15179-A23E-461d-95F2-3E60E92C0D4C}";
      return var;
   }

   void applyChanges();

private:
   OptionsSuppressibleMsgDlg(const OptionsSuppressibleMsgDlg& rhs);
   OptionsSuppressibleMsgDlg& operator=(const OptionsSuppressibleMsgDlg& rhs);

   std::vector<std::pair<std::string, std::string> > mDialogList;
   CustomTreeWidget* mpDialogTree;
};

#endif