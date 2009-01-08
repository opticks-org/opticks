/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONSERVICESIMP_H
#define ANIMATIONSERVICESIMP_H

#include <QtCore/QObject>

#include "AnimationServices.h"
#include "AttachmentPtr.h"
#include "SessionExplorer.h"
#include "SubjectImp.h"

#include <vector>

class AnimationController;

class AnimationServicesImp : public QObject, public AnimationServices, public SubjectImp
{
   Q_OBJECT

public:
   static AnimationServicesImp* instance();
   static void destroy();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Subject
   SUBJECTADAPTER_METHODS(SubjectImp)

   // AnimationServices
   AnimationController* createAnimationController(const std::string& name, FrameType frameType,
                                                   const std::string& id);
   AnimationController* createAnimationController(const std::string& name, FrameType frameType)
   {
      return createAnimationController(name, frameType, std::string());
   }
   bool hasAnimationController(const std::string& name) const;
   AnimationController* getAnimationController(const std::string& name) const;
   const std::vector<AnimationController*>& getAnimationControllers() const;
   unsigned int getNumAnimationControllers() const;
   void setCurrentAnimationController(AnimationController* pController);
   AnimationController* getCurrentAnimationController() const;
   bool renameAnimationController(AnimationController* pController, const std::string& newName);
   void destroyAnimationController(AnimationController* pController);
   void clear();

   // AnimationServicesImp
   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   AnimationServicesImp();
   ~AnimationServicesImp();

protected slots:
   void activateSelectedController();
   void destroySelectedControllers();

private:
   static AnimationServicesImp* spInstance;
   static bool mDestroyed;

   AttachmentPtr<SessionExplorer> mpExplorer;
   std::vector<AnimationController*> mControllers;
};

#endif
