/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSANIMATION_H
#define OPTIONSANIMATION_H

#include <QtGui/QListWidget>
#include <QtGui/QWidget>

#include "AppVersion.h"

#include <vector>

class AnimationCycleButton;
class QCheckBox;

class OptionsAnimation : public QWidget
{
   Q_OBJECT

public:
   OptionsAnimation();
   virtual ~OptionsAnimation();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Animation Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Animation";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display animation related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display animation related options for the application";
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
      static std::string var = "{F764406C-7726-4ed7-A823-BA7B803EB075}";
      return var;
   }

protected slots:
   void addFrameSpeed();
   void removeFrameSpeed();
   void editFrameSpeedFinished(QListWidgetItem *pCurrentItem);

private:
   OptionsAnimation(const OptionsAnimation& rhs);
   OptionsAnimation& operator=(const OptionsAnimation& rhs);

   QCheckBox* mpCanDropFrames;
   QCheckBox* mpResetOnStop;
   QCheckBox* mpConfirmDelete;
   AnimationCycleButton* mpCycle;
   QListWidget* mpFrameSpeedList;
   std::vector<double> mFrameSpeeds;
};

#endif 
