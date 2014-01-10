/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include "AnimationControllerAdapter.h"
#include "AnimationFrame.h"
#include "AnimationServicesImp.h"
#include "AnimationToolBar.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "SessionExplorer.h"
#include "StringUtilities.h"

#include <boost/bind.hpp>
#include <math.h>

using namespace std;

AnimationServicesImp* AnimationServicesImp::spInstance = NULL;
bool AnimationServicesImp::mDestroyed = false;

AnimationServicesImp* AnimationServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed == true)
      {
         throw std::logic_error("Attempting to use AnimationServices after destroying it.");
      }

      spInstance = new AnimationServicesImp();
   }

   return spInstance;
}

void AnimationServicesImp::destroy()
{
   if (mDestroyed == true)
   {
      throw std::logic_error("Attempting to destroy AnimationServices after destroying it.");
   }

   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

AnimationServicesImp::AnimationServicesImp() :
   mpExplorer(SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &AnimationServicesImp::updateContextMenu))
{
   Service<DesktopServices> pDesktop;
   pDesktop->attach(SIGNAL_NAME(DesktopServices, AboutToShowContextMenu),
      Slot(this, &AnimationServicesImp::updateContextMenu));
}

AnimationServicesImp::~AnimationServicesImp()
{
   Service<DesktopServices> pDesktop;
   pDesktop->detach(SIGNAL_NAME(DesktopServices, AboutToShowContextMenu),
      Slot(this, &AnimationServicesImp::updateContextMenu));

   // Notify of destruction
   notify(SIGNAL_NAME(Subject, Deleted));

   // Destroy all animation controllers
   clear();
}

void AnimationServicesImp::clear()
{
   vector<AnimationController*> controllers(mControllers);
   for_each(controllers.begin(), controllers.end(),
      boost::bind(&AnimationServicesImp::destroyAnimationController, this, _1));
   mControllers.clear();
}

const string& AnimationServicesImp::getObjectType() const
{
   static string sType("AnimationServicesImp");
   return sType;
}

bool AnimationServicesImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationServices"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

AnimationController* AnimationServicesImp::createAnimationController(const string& name, FrameType frameType)
{
   return createAnimationController(name, frameType, string());
}

AnimationController* AnimationServicesImp::createAnimationController(const string& name, FrameType frameType,
                                                                     const string& id)
{
   if (name.empty() == true)
   {
      return NULL;
   }

   if (hasAnimationController(name) == true)
   {
      return NULL;
   }

   // Connect to the session explorer now that the window has been created
   if (mpExplorer.get() == NULL)
   {
      Service<SessionExplorer> pExplorer;
      mpExplorer.reset(pExplorer.get());
   }

   if (frameType.isValid() == false)
   {
      frameType = AnimationServices::getSettingNewControllerType();
   }

   // Create the controller
   AnimationController* pController = new AnimationControllerAdapter(frameType,
      (id.empty()) ? SessionItemImp::generateUniqueId() : id);
   if (pController != NULL)
   {
      AnimationControllerImp* pControllerImp = dynamic_cast<AnimationControllerImp*>(pController);
      if (pControllerImp != NULL)
      {
         pControllerImp->setName(name);
      }

      mControllers.push_back(pController);
      notify(SIGNAL_NAME(AnimationServices, ControllerCreated), boost::any(pController));
   }

   return pController;
}

bool AnimationServicesImp::hasAnimationController(const string& name) const
{
   AnimationController* pController = getAnimationController(name);
   return (pController != NULL);
}

AnimationController* AnimationServicesImp::getAnimationController(const string& name) const
{
   if (name.empty() == true)
   {
      return NULL;
   }

   vector<AnimationController*>::const_iterator iter;
   for (iter = mControllers.begin(); iter != mControllers.end(); ++iter)
   {
      AnimationController* pController = *iter;
      if (pController != NULL)
      {
         const string& currentName = pController->getName();
         if (currentName == name)
         {
            return pController;
         }
      }
   }

   return NULL;
}

const vector<AnimationController*>& AnimationServicesImp::getAnimationControllers() const
{
   return mControllers;
}

unsigned int AnimationServicesImp::getNumAnimationControllers() const
{
   return mControllers.size();
}

void AnimationServicesImp::setCurrentAnimationController(AnimationController* pController)
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      pToolBar->setAnimationController(pController);
   }
}

AnimationController* AnimationServicesImp::getCurrentAnimationController() const
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      return pToolBar->getAnimationController();
   }

   return NULL;
}

bool AnimationServicesImp::renameAnimationController(AnimationController* pController, const string& newName)
{
   if ((pController == NULL) || (newName.empty() == true))
   {
      return false;
   }

   vector<AnimationController*>::iterator iter;
   for (iter = mControllers.begin(); iter != mControllers.end(); ++iter)
   {
      AnimationController* pCurrentController = *iter;
      if (pCurrentController != NULL)
      {
         if (pCurrentController->getName() == newName)
         {
            return false;
         }
      }
   }

   AnimationControllerImp* pControllerImp = dynamic_cast<AnimationControllerImp*>(pController);
   if (pControllerImp != NULL)
   {
      pControllerImp->setName(newName);
   }

   return false;
}

void AnimationServicesImp::destroyAnimationController(AnimationController* pController)
{
   if (pController == NULL)
   {
      return;
   }

   vector<AnimationController*>::iterator iter;
   for (iter = mControllers.begin(); iter != mControllers.end(); ++iter)
   {
      AnimationController* pCurrentController = *iter;
      if (pCurrentController == pController)
      {
         // Reset the animation controller on the toolbar if destroying the current controller
         Service<DesktopServices> pDesktop;

         AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
         if (pToolBar != NULL)
         {
            if (pToolBar->getAnimationController() == pController)
            {
               pToolBar->setAnimationController(NULL);
            }
         }

         // Destroy the controller
         mControllers.erase(iter);
         notify(SIGNAL_NAME(AnimationServices, ControllerDestroyed), boost::any(pController));
         delete (dynamic_cast<AnimationControllerImp*>(pController));
         break;
      }
   }
}

string AnimationServicesImp::frameToString(const AnimationFrame& frame, FrameType frameType) const
{
   if (frameType == FRAME_ID)
   {
      return frameToString(frame.mFrameNumber, frameType);
   }

   return frameToString(frame.mTime, frameType);
}

string AnimationServicesImp::frameToString(double frameValue, FrameType frameType) const
{
   QString frameText;
   if (frameType == FRAME_ID)
   {
      frameText = QString::number(static_cast<unsigned int>(frameValue + 1.0));
   }
   else if (frameValue >= 0.0)
   {
      unsigned int seconds = static_cast<unsigned int>(floor(frameValue));
      int milliseconds = static_cast<int>((frameValue - static_cast<double>(seconds)) * 1000.0);

      if (frameType == FRAME_TIME)
      {
         QDateTime dateTime;
         dateTime.setTime_t(seconds);
         dateTime.setTime(dateTime.time().addMSecs(milliseconds));
         dateTime = dateTime.toUTC();

         frameText = dateTime.toString("yyyy/MM/dd hh:mm:ss.zzz");
      }
      else if (frameType == FRAME_ELAPSED_TIME)
      {
         unsigned int hourSeconds = seconds % 60;
         unsigned int minutes = (seconds / 60) % 60;
         unsigned int hours = seconds / 3600;

         frameText = QString("%1:%2:%3.%4").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10,
            QChar('0')).arg(hourSeconds, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));
      }
   }

   return frameText.toStdString();
}

void AnimationServicesImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   QObject* pParent = pMenu->getActionParent();

   // Check if the Session Explorer's menu is invoked without items selected
   vector<SessionExplorer*> sessionItems = pMenu->getSessionItems<SessionExplorer>();
   if (sessionItems.empty() == false)
   {
      VERIFYNRV(sessionItems.size() == 1);

      SessionExplorer* pExplorer = sessionItems.front();
      VERIFYNRV(pExplorer != NULL);

      if (pExplorer->getItemViewType() == SessionExplorer::ANIMATION_ITEMS)
      {
         // Separator
         QAction* pSeparatorAction = new QAction(pParent);
         pSeparatorAction->setSeparator(true);
         pMenu->addAction(pSeparatorAction, APP_ANIMATIONSERVICES_SEPARATOR_ACTION);

         // Add an action to create a new animation controller
         QAction* pNewAction = new QAction(QIcon(":/icons/New"), "New Animation Player...", pParent);
         pNewAction->setAutoRepeat(false);
         pNewAction->setStatusTip("Creates and activates a new animation controller");
         VERIFYNR(connect(pNewAction, SIGNAL(triggered()), this, SLOT(newController())));
         pMenu->addAction(pNewAction, APP_ANIMATIONSERVICES_NEW_CONTROLLER_ACTION);
      }

      return;
   }

   // Get the selected controllers
   vector<AnimationController*> selectedControllers = pMenu->getSessionItems<AnimationController>();
   if (selectedControllers.empty() == false)
   {
      // Separator
      QAction* pSeparatorAction = new QAction(pParent);
      pSeparatorAction->setSeparator(true);
      pMenu->addAction(pSeparatorAction, APP_ANIMATIONSERVICES_SEPARATOR_ACTION);

      // If only one controller is selected, add an activate action if it is not the current controller
      if (selectedControllers.size() == 1)
      {
         AnimationController* pController = selectedControllers.front();
         if (pController != NULL)
         {
            Service<DesktopServices> pDesktop;

            AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
            if (pToolBar != NULL)
            {
               if (pController != pToolBar->getAnimationController())
               {
                  QAction* pActivateAction = new QAction("&Activate", pParent);
                  pActivateAction->setAutoRepeat(false);
                  pActivateAction->setStatusTip("Activates the selected animation controller on the "
                     "Animation toolbar");
                  connect(pActivateAction, SIGNAL(triggered()), this, SLOT(activateSelectedController()));
                  pMenu->addAction(pActivateAction, APP_ANIMATIONSERVICES_ACTIVATE_ACTION);
               }
            }
         }
      }

      // Add a delete action
      QAction* pDeleteAction = new QAction("&Delete", pParent);
      pDeleteAction->setIcon(QIcon(":/icons/Delete"));
      pDeleteAction->setAutoRepeat(false);
      pDeleteAction->setStatusTip("Destroys the selected animation controller(s)");
      connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(destroySelectedControllers()));
      pMenu->addAction(pDeleteAction, APP_ANIMATIONSERVICES_DELETE_ACTION);
   }
}

void AnimationServicesImp::newController()
{
   Service<DesktopServices> pDesktop;
   QDialog dlg(pDesktop->getMainWidget());
   dlg.setWindowTitle("New Animation Player");

   QLabel* pNameLabel = new QLabel("Name:", &dlg);
   QLineEdit* pNameEdit = new QLineEdit(&dlg);

   QLabel* pTypeLabel = new QLabel("Type:", &dlg);
   QComboBox* pTypeCombo = new QComboBox(&dlg);
   pTypeCombo->setEditable(false);
   pTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FRAME_ID)));
   pTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FRAME_TIME)));
   pTypeCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FRAME_ELAPSED_TIME)));

   QFrame* pLine = new QFrame(&dlg);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, &dlg);

   // Layout
   QGridLayout* pLayout = new QGridLayout(&dlg);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pNameLabel, 0, 0);
   pLayout->addWidget(pNameEdit, 0, 1);
   pLayout->addWidget(pTypeLabel, 1, 0);
   pLayout->addWidget(pTypeCombo, 1, 1, Qt::AlignLeft);
   pLayout->addWidget(pLine, 3, 0, 1, 2);
   pLayout->addWidget(pButtonBox, 4, 0, 1, 2);
   pLayout->setRowStretch(2, 10);
   pLayout->setColumnStretch(1, 10);

   // Initialization
   unsigned int controllerNumber = getNumAnimationControllers() + 1;
   QString controllerName = "Animation Player " + QString::number(controllerNumber);

   while (hasAnimationController(controllerName.toStdString()) == true)
   {
      controllerName = "Animation Player " + QString::number(++controllerNumber);
   }

   pNameEdit->setText(controllerName);

   FrameType frameType = AnimationServices::getSettingNewControllerType();
   if (frameType.isValid() == true)
   {
      QString frameTypeText = QString::fromStdString(StringUtilities::toDisplayString(frameType));
      if (frameTypeText.isEmpty() == false)
      {
         int typeIndex = pTypeCombo->findText(frameTypeText);
         pTypeCombo->setCurrentIndex(typeIndex);
      }
   }

   dlg.resize(275, 125);

   // Connections
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &dlg, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &dlg, SLOT(reject())));

   // Execute the dialog
   if (dlg.exec() == QDialog::Accepted)
   {
      QString name = pNameEdit->text();
      if (name.isEmpty() == true)
      {
         QMessageBox::critical(pDesktop->getMainWidget(), APP_NAME, "The animation player name is empty.  "
            "A new player will not be created.");
         return;
      }

      if (hasAnimationController(name.toStdString()) == true)
      {
         QMessageBox::critical(pDesktop->getMainWidget(), APP_NAME, "An animation player with the given name "
            "already exists.  A new player will not be created.");
         return;
      }

      QString type = pTypeCombo->currentText();
      FrameType frameType = StringUtilities::fromDisplayString<FrameType>(type.toStdString());
      VERIFYNRV(frameType.isValid() == true);

      AnimationController* pController = createAnimationController(name.toStdString(), frameType);
      if (pController != NULL)
      {
         AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
         if (pToolBar != NULL)
         {
            pToolBar->setAnimationController(pController);
         }
      }
   }
}

void AnimationServicesImp::activateSelectedController()
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar == NULL)
   {
      return;
   }

   Service<SessionExplorer> pExplorer;

   vector<SessionItem*> selectedItems = pExplorer->getSelectedSessionItems();
   if (selectedItems.size() == 1)
   {
      AnimationController* pController = dynamic_cast<AnimationController*>(selectedItems.front());
      if (pController != NULL)
      {
         pToolBar->setAnimationController(pController);
      }
   }
}

void AnimationServicesImp::destroySelectedControllers()
{
   // Get the selected controllers
   Service<SessionExplorer> pExplorer;
   vector<AnimationController*> selectedControllers = pExplorer->getSelectedSessionItems<AnimationController>();
   VERIFYNRV(selectedControllers.empty() == false);

   // Get confirmation from the user if necessary
   if (AnimationController::getSettingConfirmDelete() == true)
   {
      QMessageBox::StandardButton button = QMessageBox::question(Service<DesktopServices>()->getMainWidget(),
         "Confirm Delete", "Are you sure that you want to delete the selected animation controllers?",
         QMessageBox::Yes | QMessageBox::No);
      if (button == QMessageBox::No)
      {
         return;
      }
   }

   // Destroy the selected controllers
   vector<AnimationController*>::iterator iter;
   for (iter = selectedControllers.begin(); iter != selectedControllers.end(); ++iter)
   {
      AnimationController* pController = *iter;
      if (pController != NULL)
      {
         destroyAnimationController(pController);
      }
   }
}
