/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONMODEL_H
#define ANIMATIONMODEL_H

#include "AnimationServices.h"
#include "AnimationToolBar.h"
#include "AttachmentPtr.h"
#include "SessionItemModel.h"

class AnimationController;

class AnimationModel : public SessionItemModel
{
public:
   AnimationModel(QObject* pParent = 0);
   virtual ~AnimationModel();

   virtual Qt::DropActions supportedDragActions() const;
      
   virtual Qt::ItemFlags flags(const QModelIndex& index) const;
   virtual Qt::DropActions supportedDropActions() const;
   virtual bool dropMimeData(const QMimeData* pData, Qt::DropAction action, int row, int column,
      const QModelIndex& parentIndex);

   void addController(Subject& subject, const std::string& signal, const boost::any& value);
   void removeController(Subject& subject, const std::string& signal, const boost::any& value);
   void setCurrentController(Subject& subject, const std::string& signal, const boost::any& value);
   void addAnimation(Subject& subject, const std::string& signal, const boost::any& value);
   void removeAnimation(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   void addControllerItem(AnimationController* pController);
   void removeControllerItem(AnimationController* pController);

private:
   AnimationModel(const AnimationModel& rhs);
   AnimationModel& operator=(const AnimationModel& rhs);

   AttachmentPtr<AnimationServices> mpAnimationServices;
   AttachmentPtr<AnimationToolBar> mpAnimationToolBar;
};

#endif
