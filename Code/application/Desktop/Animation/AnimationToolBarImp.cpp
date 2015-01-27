/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtGui/QApplication>
#include <QtGui/QHelpEvent>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionSlider>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>
#include <QtGui/QWidgetAction>

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationControllerImp.h"
#include "AnimationCycleButton.h"
#include "AnimationServicesImp.h"
#include "AnimationToolBar.h"
#include "AnimationToolBarImp.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "PixmapGrid.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "StringUtilities.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <math.h>

#include <boost/rational.hpp>
#include <string>

XERCES_CPP_NAMESPACE_USE

AnimationToolBarImp::AnimationToolBarImp(const std::string& id, QWidget* parent) :
   ToolBarImp(id, "Animation", parent),
   mpChangeDirectionAction(NULL),
   mpStopAction(NULL),
   mpPlayPauseAction(NULL),
   mpSlowDownAction(NULL),
   mpSpeedUpAction(NULL),
   mpFrameSpeedCombo(NULL),
   mpStepForwardAction(NULL),
   mpStepBackwardAction(NULL),
   mpFrameSlider(NULL),
   mpBumperButton(NULL),
   mpLeftBumperToFrameAction(NULL),
   mpRightBumperToFrameAction(NULL),
   mpAdjustBumpersAction(NULL),
   mpResetBumpersAction(NULL),
   mpStoreBumpersAction(NULL),
   mpRestoreBumpersAction(NULL),
   mpCycle(NULL),
   mpTimestampLabel(NULL),
   mpController(NULL),
   mPrevAnimationState(STOP),
   mHideTimestamp(false)
{
   Service<DesktopServices> pDesktop;
   std::string shortcutContext = windowTitle().toStdString();

   // Animation buttons
   mpStopAction = addAction(QIcon(":/icons/Stop"), "Stop", this, SLOT(stop()));
   mpStopAction->setAutoRepeat(false);
   mpStopAction->setCheckable(true);
   pDesktop->initializeAction(mpStopAction, shortcutContext);
   QWidget* pMainWidget = pDesktop->getMainWidget();
   pMainWidget->addAction(mpStopAction); // Want this shortcut available, even when toolbar is floated and not active.

   mpPlayPauseAction = addAction(QIcon(":/icons/PlayForward"), "Play//Pause", this, SLOT(playPause()));
   mpPlayPauseAction->setAutoRepeat(false);
   mpPlayPauseAction->setShortcut(QKeySequence(Qt::Key_Space));
   pDesktop->initializeAction(mpPlayPauseAction, shortcutContext);
   pMainWidget->addAction(mpPlayPauseAction); // Want this shortcut available, even when toolbar is floated and not active.

   mpStepBackwardAction = addAction(QIcon(":/icons/AdvanceBackward"), "Step Backward", this, SLOT(stepBackward()));
   mpStepBackwardAction->setAutoRepeat(true);
   pDesktop->initializeAction(mpStepBackwardAction, shortcutContext);
   pMainWidget->addAction(mpStepBackwardAction); // Want this shortcut available, even when toolbar is floated and not active.

   mpStepForwardAction = addAction(QIcon(":/icons/AdvanceForward"), "Step Forward", this, SLOT(stepForward()));
   mpStepForwardAction->setAutoRepeat(true);
   pDesktop->initializeAction(mpStepForwardAction, shortcutContext);
   pMainWidget->addAction(mpStepForwardAction); // Want this shortcut available, even when toolbar is floated and not active.

   addSeparator();

   mpSlowDownAction = addAction(QIcon(":/icons/SlowDown"), "Slow Down", this, SLOT(slowDown()));
   mpSlowDownAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpSlowDownAction, shortcutContext);

   mpFrameSpeedCombo = new QComboBox(this);

   std::vector<double> frameSpeeds = AnimationToolBar::getSettingFrameSpeeds();
   for (std::vector<double>::iterator iter = frameSpeeds.begin(); iter != frameSpeeds.end(); ++iter)
   {
      mpFrameSpeedCombo->addItem(QString::number(*iter, 'g', 3));
   }
   mpFrameSpeedCombo->setInsertPolicy(QComboBox::NoInsert);
   mpFrameSpeedCombo->setToolTip("Speed (X)");
   mpFrameSpeedCombo->setEditable(true);
   mpFrameSpeedCombo->setCompleter(NULL);
   QDoubleValidator* pValidator = new QDoubleValidator(mpFrameSpeedCombo);
   pValidator->setDecimals(3);
   pValidator->setBottom(0.0);
   mpFrameSpeedCombo->setValidator(pValidator);
   VERIFYNR(connect(mpFrameSpeedCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setFrameSpeed(const QString&))));
   VERIFYNR(connect(mpFrameSpeedCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setFocus())));

   QLineEdit* pSpeedEdit = mpFrameSpeedCombo->lineEdit();
   if (pSpeedEdit != NULL)
   {
      VERIFYNR(connect(pSpeedEdit, SIGNAL(editingFinished()), this, SLOT(setFrameSpeed())));
      VERIFYNR(connect(pSpeedEdit, SIGNAL(returnPressed()), this, SLOT(setFocus())));
   }

   addWidget(mpFrameSpeedCombo);

   mpSpeedUpAction = addAction(QIcon(":/icons/SpeedUp"), "Speed Up", this, SLOT(speedUp()));
   mpSpeedUpAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpSpeedUpAction, shortcutContext);

   addSeparator();

   mpChangeDirectionAction = addAction(QIcon(":/icons/DirectionForward"), "Change Direction", this,
      SLOT(changeDirection()));
   mpChangeDirectionAction->setAutoRepeat(false);
   pDesktop->initializeAction(mpChangeDirectionAction, shortcutContext);

   // Animation cycle
   mpCycle = new AnimationCycleButton(this);
   mpCycle->setStatusTip("Specifies the play behavior when the end of the animation is reached");
   mpCycle->setToolTip("Animation Cycle");
   VERIFYNR(connect(mpCycle, SIGNAL(valueChanged(AnimationCycle)), this, SLOT(updateAnimationCycle(AnimationCycle))));
   addWidget(mpCycle);
   addSeparator();

   // Animation Bumper button
   mpBumperButton = new QToolButton(this);
   mpBumperButton->setCheckable(true);
   mpBumperButton->setIcon(QIcon(":/icons/AnimationBumpers"));
   mpBumperButton->setPopupMode(QToolButton::MenuButtonPopup);
   addWidget(mpBumperButton);
   QAction* pEnabled = new QAction(QIcon(":/icons/AnimationBumpers"), "Bumpers Enabled", this);
   pEnabled->setCheckable(true);
   pEnabled->setChecked(false);
   VERIFYNR(connect(pEnabled, SIGNAL(toggled(bool)), this, SLOT(bumpersEnabled(bool))));
   pDesktop->initializeAction(pEnabled, shortcutContext + "/Bumpers");
   mpBumperButton->setDefaultAction(pEnabled);
   QMenu* pMenu = new QMenu(mpBumperButton);
   VERIFYNR(connect(pMenu, SIGNAL(aboutToShow()), this, SLOT(updateBumperMenu())));
   mpBumperButton->setMenu(pMenu);
   mpBumperButton->setToolTip("Enable Bumpers");

   // menu actions will be connected/disconnected in setAnimationController
   mpLeftBumperToFrameAction = pMenu->addAction("Snap Left Bumper to Current Frame");
   pDesktop->initializeAction(mpLeftBumperToFrameAction, shortcutContext + "/Bumpers");
   mpRightBumperToFrameAction = pMenu->addAction("Snap Right Bumper to Current Frame");
   pDesktop->initializeAction(mpRightBumperToFrameAction, shortcutContext + "/Bumpers");
   mpAdjustBumpersAction = pMenu->addAction("Adjust Bumpers...");
   pDesktop->initializeAction(mpAdjustBumpersAction, shortcutContext + "/Bumpers");
   mpResetBumpersAction = pMenu->addAction("Reset Bumpers");
   pDesktop->initializeAction(mpResetBumpersAction, shortcutContext + "/Bumpers");
   pMenu->addSeparator();
   mpStoreBumpersAction = pMenu->addAction("Store Bumpers");
   pDesktop->initializeAction(mpStoreBumpersAction, shortcutContext + "/Bumpers");
   mpRestoreBumpersAction = pMenu->addAction("Restore Bumpers");
   mpRestoreBumpersAction->setEnabled(false);  // will be updated in updateBumperMenu prior to display
   pDesktop->initializeAction(mpRestoreBumpersAction, shortcutContext + "/Bumpers");

   // Current frame slider
   QWidget* pSliderWidget = new QWidget(this);

   mpFrameSlider = new WheelEventSlider(this, Qt::Horizontal, pSliderWidget);
   VERIFYNR(connect(mpFrameSlider, SIGNAL(sliderPressed()), this, SLOT(activateSlider())));
   VERIFYNR(connect(mpFrameSlider, SIGNAL(sliderReleased()), this, SLOT(releaseSlider())));
   VERIFYNR(connect(mpFrameSlider, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int))));

   QHBoxLayout* pSliderLayout = new QHBoxLayout(pSliderWidget);
   pSliderLayout->setMargin(0);
   pSliderLayout->setSpacing(0);
   pSliderLayout->addSpacing(5);
   pSliderLayout->addWidget(mpFrameSlider);
   pSliderLayout->addSpacing(5);
   addWidget(pSliderWidget);

   // Slider context menu
   mpSliderMenu = new QMenu(this);

   mpMoveToAction = new QAction("Move To Frame", this);
   mpMoveToAction->setAutoRepeat(false);
   VERIFYNR(connect(mpMoveToAction, SIGNAL(triggered()), this, SLOT(moveToFrame())));

   // Current frame and time labels
   mpTimestampLabel = new QLabel(this);
   mpTimestampLabel->setAlignment(Qt::AlignCenter);
   addWidget(mpTimestampLabel);

   // Initialization
   setFocusPolicy(Qt::ClickFocus);     // Required to set the frame speed if the user edits the value
                                       // and clicks the Play button without pressing the return key
   updateAnimationCycle(PLAY_ONCE);
   updateAnimationControls();
}

AnimationToolBarImp::~AnimationToolBarImp()
{}

const std::string& AnimationToolBarImp::getObjectType() const
{
   static std::string type("AnimationToolBarImp");
   return type;
}

bool AnimationToolBarImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationToolBar"))
   {
      return true;
   }

   return ToolBarImp::isKindOf(className);
}

bool AnimationToolBarImp::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml("AnimationToolBar");
   if (toXml(&xml) == false)
   {
      return false;
   }

   if (mpController != NULL)
   {
      xml.addAttr("controllerId", mpController->getId());
   }

   xml.addAttr("animationState", mPrevAnimationState);
   xml.addAttr("hideTimestamp", mHideTimestamp);

   if (mpFrameSlider->toXml(&xml) == false)
   {
      return false;
   }

   return serializer.serialize(xml);
}

bool AnimationToolBarImp::deserialize(SessionItemDeserializer& deserializer)
{
   XmlReader reader(NULL, false);

   DOMElement* pRoot = deserializer.deserialize(reader, "AnimationToolBar");
   if ((pRoot == NULL) || (fromXml(pRoot, XmlBase::VERSION) == false))
   {
      return false;
   }

   AnimationController* pController = NULL;
   if (pRoot->hasAttribute(X("controllerId")))
   {
      Service<SessionManager> pManager;
      pController = dynamic_cast<AnimationController*>(
         pManager->getSessionItem(A(pRoot->getAttribute(X("controllerId")))));
   }

   // Restore the animation controller by calling setAnimationController() to perform all of the necessary
   // connections.  This also resets the state of the slider, so the slider must be restored after this
   // call in case the animation controller has not yet been restored.
   setAnimationController(pController);

   mPrevAnimationState = StringUtilities::fromXmlString<AnimationState>(A(pRoot->getAttribute(X("animationState"))));
   mHideTimestamp = StringUtilities::fromXmlString<bool>(A(pRoot->getAttribute(X("hideTimestamp"))));

   return mpFrameSlider->fromXml(pRoot, XmlBase::VERSION);
}

void AnimationToolBarImp::speedUp()
{
   // change the speed to the next value in the list
   int itemPos = mpFrameSpeedCombo->currentIndex();
   int maxItems = mpFrameSpeedCombo->count();
   if ((itemPos + 1) < maxItems)
   {
      mpFrameSpeedCombo->setCurrentIndex(itemPos + 1);
   }
}

void AnimationToolBarImp::slowDown()
{
   // change the speed to the previous value in the list
   int itemPos = mpFrameSpeedCombo->currentIndex();
   if (itemPos > 0)
   {
      mpFrameSpeedCombo->setCurrentIndex(itemPos - 1);
   }
}

void AnimationToolBarImp::playPause()
{
   if (mpController != NULL)
   {
      AnimationState state = mpController->getAnimationState();
      if (state == PLAY_FORWARD || state == PLAY_BACKWARD)
      {
         mpController->pause();
      }
      else
      {
         mpController->play();
      }
   }
}

void AnimationToolBarImp::changeDirection()
{
   if (mpController != NULL)
   {
      AnimationState state = mpController->getAnimationState();
      if (state == PLAY_FORWARD)
      {
         mpController->setAnimationState(PLAY_BACKWARD);
      } 
      else if (state == PAUSE_FORWARD)
      {
         mpController->setAnimationState(PAUSE_BACKWARD);
      } 
      else if (state == PLAY_BACKWARD)
      {
         mpController->setAnimationState(PLAY_FORWARD);
      } 
      else if (state == PAUSE_BACKWARD)
      {
         mpController->setAnimationState(PAUSE_FORWARD);
      }
   }
}

void AnimationToolBarImp::setPlayButtonState(AnimationState state)
{
   if (mpController != NULL)
   {
      if (state == PLAY_FORWARD || state == PLAY_BACKWARD)
      {
         mpPlayPauseAction->setIcon(QIcon(":/icons/Pause"));
         mpPlayPauseAction->setText("Pause");
      }
      else
      {
         mpPlayPauseAction->setIcon(QIcon(":/icons/PlayForward"));
         mpPlayPauseAction->setText("Play");
      }

      if (mpController->getNumAnimations() > 0)
      {
         mpStepForwardAction->setEnabled((state == PAUSE_FORWARD) || (state == PAUSE_BACKWARD) || (state == STOP));
         mpStepBackwardAction->setEnabled((state == PAUSE_FORWARD) || (state == PAUSE_BACKWARD) || (state == STOP));
      }
   }
}

void AnimationToolBarImp::setChangeDirectionButtonState(AnimationState state)
{
   if (state == PLAY_FORWARD)
   {
      mpChangeDirectionAction->setIcon(QIcon(":/icons/DirectionForward"));
   }
   else if (state == PAUSE_FORWARD)
   {
      mpChangeDirectionAction->setIcon(QIcon(":/icons/DirectionForward"));
   } 
   else if (state == PLAY_BACKWARD)
   {
      mpChangeDirectionAction->setIcon(QIcon(":/icons/DirectionBackward"));
   } 
   else if (state == PAUSE_BACKWARD)
   {
      mpChangeDirectionAction->setIcon(QIcon(":/icons/DirectionBackward"));
   } 
}

void AnimationToolBarImp::stop()
{
   if (mpController != NULL)
   {
      if (mpController->getAnimationState() == STOP)
      {
         mpStopAction->setChecked(true);
      }
      else
      {
         mpController->stop();
      }
   }
}

void AnimationToolBarImp::sliderActionTriggered(int action)
{
   int start;
   int stop;
   mpFrameSlider->getPlaybackRange(start, stop);
   switch (action)
   {
   case QSlider::SliderSingleStepAdd:
   case QSlider::SliderSingleStepSub:
   case QSlider::SliderPageStepAdd:
   case QSlider::SliderPageStepSub:
   case QSlider::SliderMove: // all fall through
      {
         // convert bumper index to frame index
         int framePos = mpFrameSlider->sliderPosition();
         if (framePos < start)
         {
            framePos = start;
         }
         else if (framePos > stop)
         {
            framePos = stop;
         }
         mpFrameSlider->setSliderPosition(framePos);
         setCurrentFrame(framePos);
         break;
      }
   case QSlider::SliderToMinimum:
      if (mpController->getBumpersEnabled())
      {
         mpController->setCurrentFrame(mpController->getStartBumper());
      }
      else
      {
         mpController->setCurrentFrame(mpController->getStartFrame());
      }
      mpFrameSlider->setSliderPosition(start);
      break;
   case QSlider::SliderToMaximum:
      if (mpController->getBumpersEnabled())
      {
         mpController->setCurrentFrame(mpController->getStopBumper());
      }
      else
      {
         mpController->setCurrentFrame(mpController->getStopFrame());
      }
      mpFrameSlider->setSliderPosition(stop);
      break;
   default:
      return;
   }
}

void AnimationToolBarImp::setCurrentFrame(int frameIndex)
{
   if (mpController != NULL)
   {
      double newFrame = getSliderFrame(frameIndex);
      mpController->setCurrentFrame(newFrame);
   }
}

void AnimationToolBarImp::setFrameSpeed()
{
   QString frameSpeed = mpFrameSpeedCombo->currentText();
   setFrameSpeed(frameSpeed);
}

void AnimationToolBarImp::setFrameSpeed(const QString& strSpeed)
{
   // check for an empty string
   if (strSpeed.isEmpty() == true)
   {
      return;
   }

   // set speed in combo box
   if (mpController != NULL)
   {
      double intervalMultiplier = strSpeed.toDouble();

      // default to 1.0 if 0.0 is returned
      if (intervalMultiplier == 0.0)
      {
         intervalMultiplier = 1.0;
      }

      mpController->setIntervalMultiplier(intervalMultiplier);
   }
}

void AnimationToolBarImp::updateAnimationState(AnimationState state)
{
   mpStopAction->setChecked(state == STOP);
   setPlayButtonState(state);
   setChangeDirectionButtonState(state);
}

void AnimationToolBarImp::updateFrameRange()
{
   int numSteps = 1;
   int lineStep = 1;
   double currentFrame = -1.0;

   if (mpController != NULL)
   {
      // Get the range from the animation controller
      double startFrame = mpController->getStartFrame();
      double stopFrame = mpController->getStopFrame();
      const int frequency = mpController->getFrequency();
      currentFrame = mpController->getCurrentFrame();

      // Calculate the total number of steps, adding 0.5 to round to the nearest number
      double dNumSteps = (stopFrame - startFrame) * frequency;

      // make sure reasonable number and prevent very large number from becoming a negative on cast to int
      if (dNumSteps > 100000.0)
      {
         dNumSteps = 100000.0;
      }
      numSteps = static_cast<int>(dNumSteps + 0.5);

      // make sure there are a minimum number of steps so slider moves smoothly
      if (numSteps < 100)
      {
         numSteps = 100;
      }

      // Define the line step size as one frame or one second
      lineStep = frequency;
   }

   // Update the slider range
   mpFrameSlider->setRange(0, numSteps);
   mpFrameSlider->setSingleStep(std::max(1,lineStep));   // Page step is updated by the slider widget itself on resize

   // update bumpers
   updateBumpers();

   // Update the slider value
   updateCurrentFrame(currentFrame);
}

void AnimationToolBarImp::updateCurrentFrame(double frameValue)
{
   int currentStep = 0;
   QString strFrameText;

   if ((mpController != NULL) && (frameValue > -1.0))
   {
      // Get the current slider index value
      double startFrame = mpController->getStartFrame();
      double stopFrame = mpController->getStopFrame();

      // Add 0.5 to round to the nearest index
      currentStep = static_cast<int> (mpFrameSlider->maximum() *
         ((frameValue - startFrame) / (stopFrame - startFrame)) + 0.5);

      // Slider
      mpFrameSlider->setValue(currentStep);

      // Frame text
      if (mHideTimestamp == false)
      {
         strFrameText = getFrameText(frameValue) + " ";
      }
   }

   VERIFYNRV(mpTimestampLabel != NULL);
   mpTimestampLabel->setText(strFrameText);
}

void AnimationToolBarImp::cleanUpItems()
{
   std::vector<double> frameSpeeds = AnimationToolBar::getSettingFrameSpeeds();

   if (static_cast<int>(frameSpeeds.size()) != mpFrameSpeedCombo->count())
   {
      for (int i = 0; i < mpFrameSpeedCombo->count(); ++i)
      {
         if (std::find(frameSpeeds.begin(), frameSpeeds.end(), 
             mpFrameSpeedCombo->itemText(i).toDouble()) == frameSpeeds.end())
         {
            mpFrameSpeedCombo->removeItem(i);
            break;
         }
      }
   }
}

void AnimationToolBarImp::updateFrameSpeed(double speed)
{
   mpFrameSpeedCombo->blockSignals(true);
   cleanUpItems();

   QString strSpeed = QString::number(speed);
   int index = mpFrameSpeedCombo->findText(strSpeed);
   if (index != -1)
   {
      mpFrameSpeedCombo->setCurrentIndex(index);
   }
   else
   {
      int i;
      for (i = 0; i < mpFrameSpeedCombo->count(); ++i)
      {
         if (mpFrameSpeedCombo->itemText(i).toDouble() > speed)
         {
            break;
         }
      }

      mpFrameSpeedCombo->insertItem(i, strSpeed);
      mpFrameSpeedCombo->setCurrentIndex(i);
   }

   mpFrameSpeedCombo->blockSignals(false);
}

void AnimationToolBarImp::updateAnimationCycle(AnimationCycle cycle)
{
   mpCycle->setCurrentValue(cycle);
   if (mpController != NULL)
   {
      mpController->setAnimationCycle(cycle);
   }
}

void AnimationToolBarImp::setAnimationController(AnimationController* pController)
{
   AnimationControllerImp* pImp = dynamic_cast<AnimationControllerImp*>(pController);
   if (pImp == mpController)
   {
      return;
   }

   AnimationState state = STOP;
   double intervalMultiplier = 1.0;
   AnimationCycle cycle = PLAY_ONCE;

   if (mpController != NULL)
   {
      VERIFYNR(disconnect(mpController, SIGNAL(frameRangeChanged()), this, SLOT(updateFrameRange())));
      VERIFYNR(disconnect(mpController, SIGNAL(frameChanged(double)), this, SLOT(updateCurrentFrame(double))));
      VERIFYNR(disconnect(mpController, SIGNAL(animationStateChanged(AnimationState)), this,
         SLOT(updateAnimationState(AnimationState))));
      VERIFYNR(disconnect(mpController, SIGNAL(animationCycleChanged(AnimationCycle)), this,
         SLOT(updateAnimationCycle(AnimationCycle))));
      VERIFYNR(disconnect(mpController, SIGNAL(intervalMultiplierChanged(double)), this,
         SLOT(updateFrameSpeed(double))));
      VERIFYNR(disconnect(mpController, SIGNAL(animationAdded(Animation*)), this,
         SLOT(updateAnimationControls())));
      VERIFYNR(disconnect(mpController, SIGNAL(animationRemoved(Animation*)), this,
         SLOT(updateAnimationControls())));
      VERIFYNR(disconnect(mpController, SIGNAL(bumpersEnabledChanged(bool)), this,
         SLOT(bumpersEnabled(bool))));
      VERIFYNR(disconnect(mpController, SIGNAL(bumperStartChanged(double)), this,
         SLOT(setStartBumper(double))));
      VERIFYNR(disconnect(mpController, SIGNAL(bumperStopChanged(double)), this,
         SLOT(setStopBumper(double))));
      VERIFYNR(disconnect(mpLeftBumperToFrameAction, SIGNAL(triggered()), mpController,
         SLOT(snapStartBumperToFrame())));
      VERIFYNR(disconnect(mpRightBumperToFrameAction, SIGNAL(triggered()), mpController,
         SLOT(snapStopBumperToFrame())));
      VERIFYNR(disconnect(mpAdjustBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(adjustBumpers())));
      VERIFYNR(disconnect(mpResetBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(resetBumpers())));
      VERIFYNR(disconnect(mpStoreBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(storeBumpers())));
      VERIFYNR(disconnect(mpRestoreBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(restoreBumpers())));
   }

   mpController = pImp;

   if (mpController != NULL)
   {
      // Get current values for the widgets
      state = mpController->getAnimationState();
      intervalMultiplier = mpController->getIntervalMultiplier();
      cycle = mpController->getAnimationCycle();

      VERIFYNR(connect(mpController, SIGNAL(frameRangeChanged()), this, SLOT(updateFrameRange())));
      VERIFYNR(connect(mpController, SIGNAL(frameChanged(double)), this, SLOT(updateCurrentFrame(double))));
      VERIFYNR(connect(mpController, SIGNAL(animationStateChanged(AnimationState)), this,
         SLOT(updateAnimationState(AnimationState))));
      VERIFYNR(connect(mpController, SIGNAL(animationCycleChanged(AnimationCycle)), this,
         SLOT(updateAnimationCycle(AnimationCycle))));
      VERIFYNR(connect(mpController, SIGNAL(intervalMultiplierChanged(double)), this,
         SLOT(updateFrameSpeed(double))));
      VERIFYNR(connect(mpController, SIGNAL(animationAdded(Animation*)), this, SLOT(updateAnimationControls())));
      VERIFYNR(connect(mpController, SIGNAL(animationRemoved(Animation*)), this, SLOT(updateAnimationControls())));
      VERIFYNR(connect(mpController, SIGNAL(bumpersEnabledChanged(bool)), this,
         SLOT(bumpersEnabled(bool))));
      VERIFYNR(connect(mpController, SIGNAL(bumperStartChanged(double)), this,
         SLOT(setStartBumper(double))));
      VERIFYNR(connect(mpController, SIGNAL(bumperStopChanged(double)), this,
         SLOT(setStopBumper(double))));
      VERIFYNR(connect(mpLeftBumperToFrameAction, SIGNAL(triggered()), mpController,
         SLOT(snapStartBumperToFrame())));
      VERIFYNR(connect(mpRightBumperToFrameAction, SIGNAL(triggered()), mpController,
         SLOT(snapStopBumperToFrame())));
      VERIFYNR(connect(mpAdjustBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(adjustBumpers())));
      VERIFYNR(connect(mpResetBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(resetBumpers())));
      VERIFYNR(connect(mpStoreBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(storeBumpers())));
      VERIFYNR(connect(mpRestoreBumpersAction, SIGNAL(triggered()), mpController,
         SLOT(restoreBumpers())));

      mpBumperButton->defaultAction()->setChecked(mpController->getBumpersEnabled());
   }

   updateAnimationControls();

   // Update the widget values
   updateFrameRange();
   updateAnimationState(state);
   updateFrameSpeed(intervalMultiplier);
   updateAnimationCycle(cycle);

   // notify current attached observers that the animation controller has been changed
   notify(SIGNAL_NAME(AnimationToolBar, ControllerChanged), boost::any(pController));
}

void AnimationToolBarImp::stepForward()
{
   if (mpController != NULL)
   {
      mpController->stepForward();
   }
}

void AnimationToolBarImp::stepBackward()
{
   if (mpController != NULL)
   {
      mpController->stepBackward();
   }
}

void AnimationToolBarImp::updateAnimationControls()
{
   bool bEnable = false;
   if (mpController != NULL)
   {
      bEnable = (mpController->getNumAnimations() > 0);
   }

   // Enable the widgets
   mpStopAction->setEnabled(bEnable);
   mpPlayPauseAction->setEnabled(bEnable);
   mpSlowDownAction->setEnabled(bEnable);
   mpSpeedUpAction->setEnabled(bEnable);
   mpStepBackwardAction->setEnabled(bEnable);
   mpStepForwardAction->setEnabled(bEnable);
   mpTimestampLabel->setHidden(!bEnable || mHideTimestamp);
   mpFrameSlider->setEnabled(bEnable);
   mpFrameSpeedCombo->setEnabled(bEnable);
   mpCycle->setEnabled(bEnable);
   mpChangeDirectionAction->setEnabled(bEnable);
   mpBumperButton->setEnabled(bEnable);
}

AnimationController* AnimationToolBarImp::getAnimationController() const
{
   return dynamic_cast<AnimationController*>(mpController);
}

void AnimationToolBarImp::activateSlider()
{
   if (mpController != NULL)
   {
      AnimationState state = mpController->getAnimationState();
      mPrevAnimationState = state;
      if (state == PLAY_FORWARD || state == PLAY_BACKWARD)
      {
         playPause();
      }
   }
}

void AnimationToolBarImp::releaseSlider()
{
   if (mPrevAnimationState == PLAY_FORWARD || mPrevAnimationState == PLAY_BACKWARD)
   {
      playPause();
   }
}

void AnimationToolBarImp::moveToFrame()
{
   bool success = false;
   int sliderIndex = mpMoveToAction->data().toInt(&success);
   if ((success == true) && (sliderIndex != -1))
   {
      mpFrameSlider->setSliderPosition(sliderIndex);
      setCurrentFrame(sliderIndex);
   }
}

bool AnimationToolBarImp::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::ToolTip)
      {
         QHelpEvent* pHelpEvent = dynamic_cast<QHelpEvent*>(pEvent);
         VERIFY(pHelpEvent != NULL);

         // Display a tool tip containing the frame text for the current mouse position
         const QPoint& globalPos = pHelpEvent->globalPos();

         int index = getSliderIndex(globalPos);
         if (index != -1)
         {
            double frameValue = getSliderFrame(index);
            QString frameText = getFrameText(frameValue);
            QToolTip::showText(globalPos, frameText, mpFrameSlider, mpFrameSlider->rect());
            return true;
         }
      }
   }

   return ToolBarImp::event(pEvent);
}

void AnimationToolBarImp::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (pEvent != NULL)
   {
      // Only display the context menu if the mouse is over the slider
      const QPoint& globalPos = pEvent->globalPos();

      int sliderIndex = getSliderIndex(globalPos);
      if (sliderIndex != -1)
      {
         mpSliderMenu->clear();

         if ((mpController != NULL) && (mpController->getAnimationState() != STOP))
         {
            mpSliderMenu->addAction(mpStopAction);
         }

         mpSliderMenu->addAction(mpPlayPauseAction);
         mpSliderMenu->addSeparator();

         if (mpStepBackwardAction->isEnabled() == true)
         {
            mpSliderMenu->addAction(mpStepBackwardAction);
         }

         if (mpStepForwardAction->isEnabled() == true)
         {
            mpSliderMenu->addAction(mpStepForwardAction);
         }

         double frameValue = getSliderFrame(sliderIndex);
         QString actionText = "Move to '" + getFrameText(frameValue) + "'";
         mpMoveToAction->setText(actionText);
         mpMoveToAction->setData(QVariant(sliderIndex));

         mpSliderMenu->addAction(mpMoveToAction);
         mpSliderMenu->popup(globalPos);
         return;
      }
   }

   ToolBarImp::contextMenuEvent(pEvent);
}

QString AnimationToolBarImp::getFrameText(double frameValue) const
{
   if (mpController == NULL)
   {
      return QString();
   }

   AnimationServicesImp* pAnimationServices = AnimationServicesImp::instance();
   if (pAnimationServices == NULL)
   {
      return QString();
   }

   FrameType frameType = mpController->getFrameType();
   QString frameText = QString::fromStdString(pAnimationServices->frameToString(frameValue, frameType));

   if (frameType == FRAME_ID)
   {
      double startFrame = mpController->getStartFrame();
      double stopFrame = mpController->getStopFrame();

      frameText = "Frame: " + frameText + "/" +
         QString::number(static_cast<unsigned int>(stopFrame - startFrame + 1.0));
   }

   return frameText;
}

void AnimationToolBarImp::setHideTimestamp(bool hideTimestamp)
{
   if (mHideTimestamp != hideTimestamp)
   {
      mHideTimestamp = hideTimestamp;
      if (mpController != NULL)
      {
         updateCurrentFrame(mpController->getCurrentFrame());
      }
      updateAnimationControls();
   }
}

bool AnimationToolBarImp::getHideTimestamp() const
{
   return mHideTimestamp;
}

void AnimationToolBarImp::bumpersEnabled(bool enabled)
{
   if (mpController != NULL && mpFrameSlider->getBumpersEnabled() != enabled)
   {
      // need to set button since change may have been from controller context menu action
      mpBumperButton->defaultAction()->setChecked(enabled);
      mpFrameSlider->setBumpersEnabled(enabled);
      mpFrameSlider->repaint();
      mpController->setBumpersEnabled(enabled);
      if (enabled)
      {
         mpBumperButton->setToolTip("Disable Bumpers");
      }
      else
      {
         mpBumperButton->setToolTip("Enable Bumpers");
      }
   }
}

void AnimationToolBarImp::setStartBumper(double newValue)
{
   mpFrameSlider->setLeftBumper(getSliderIndex(newValue));
   mpFrameSlider->repaint();
}

void AnimationToolBarImp::setStopBumper(double newValue)
{
   mpFrameSlider->setRightBumper(getSliderIndex(newValue));
   mpFrameSlider->repaint();
}

void AnimationToolBarImp::updateBumperMenu()
{
   Service<ConfigurationSettings> pSettings;
   std::string bumperPath("AnimationController/Bumpers/BumpersActive");
   mpRestoreBumpersAction->setEnabled(dv_cast<bool>(pSettings->getSetting(bumperPath), false));
}

int AnimationToolBarImp::getSliderIndex(const QPoint& globalPos) const
{
   QPoint sliderPos = mpFrameSlider->mapFromGlobal(globalPos);
   QRect sliderRect = mpFrameSlider->rect();

   if (sliderRect.contains(sliderPos) == false)
   {
      return -1;
   }

   QStyleOptionSlider option;
   option.initFrom(mpFrameSlider);
   option.subControls = QStyle::SC_All;

   QRect handleRect = mpFrameSlider->style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle,
      mpFrameSlider);

   int index = QStyle::sliderValueFromPosition(mpFrameSlider->minimum(), mpFrameSlider->maximum(),
      sliderPos.x() - (handleRect.width() / 2), mpFrameSlider->width() - handleRect.width());
   return index;
}

int AnimationToolBarImp::getSliderIndex(double frameValue) const
{
   if (frameValue < 0.0)  // unset bumper value
   {
      return -1;
   }

   double index(0.0);

   if (mpController != NULL)
   {
      double startFrame = mpController->getStartFrame();
      double stopFrame = mpController->getStopFrame();
      double sliderRange = mpFrameSlider->maximum() - mpFrameSlider->minimum();
      index = ((frameValue - startFrame) / (stopFrame - startFrame)) * sliderRange + mpFrameSlider->minimum() + 0.5;
   }

   return static_cast<int>(index);
}

double AnimationToolBarImp::getSliderFrame(int sliderIndex) const
{
   if (mpController == NULL)
   {
      return 0.0;
   }

   double startFrame = mpController->getStartFrame();
   double range = mpController->getStopFrame() - startFrame;
   double fraction = static_cast<double>(sliderIndex) / static_cast<double>(mpFrameSlider->maximum());
   long maximum = mpFrameSlider->maximum();
   double frame = startFrame + range * sliderIndex / maximum;
   return frame;
}

void AnimationToolBarImp::updateBumpers()
{
   if (mpController != NULL)
   {
      mpFrameSlider->setLeftBumper(getSliderIndex(mpController->getStartBumper()));
      mpFrameSlider->setRightBumper(getSliderIndex(mpController->getStopBumper()));
      if (mpController->getNumAnimations() > 0)
      {
         mpFrameSlider->setBumpersEnabled(mpController->getBumpersEnabled());
      }
      else
      {
         mpController->setBumpersEnabled(false);
      }
   }
   else
   {
      mpFrameSlider->resetBumpers();
   }

   mpFrameSlider->repaint();
}

AnimationToolBarImp::WheelEventSlider::WheelEventSlider(AnimationToolBarImp* pToolbar, Qt::Orientation orientation, QWidget *parent) :
                                   QSlider(orientation, parent),
                                   mpToolbar(pToolbar),
                                   mBumpersEnabled(false),
                                   mLeftBumper(-1),
                                   mRightBumper(-1)
{
   setStyle(new WheelEventSliderStyle());
}

void AnimationToolBarImp::WheelEventSlider::wheelEvent(QWheelEvent* pEvent)
{
   QSlider::wheelEvent(pEvent);
   triggerAction(QAbstractSlider::SliderMove);
}

void AnimationToolBarImp::WheelEventSlider::paintEvent(QPaintEvent* pEvent)
{
   // only paint bumpers if bumpers are enabled and have been set to valid frame values, i.e. > -1
   if (mBumpersEnabled)
   {
      QPainter painter(this);
      QBrush brush(palette().mid());
      int x1 = rect().left() -1 ;
      int y1 = rect().top() + 5;
      int x2 = rect().right() + 1;
      int y2 = rect().bottom() - 5;
      if (mLeftBumper > -1)
      {
         QRect rect;
         rect.setCoords(x1, y1, mLeftBumper, y2);
         painter.fillRect(rect, brush);
      }
      if (mRightBumper > -1)
      {
         QRect rect2;
         rect2.setCoords(mRightBumper, y1, x2, y2);
         painter.fillRect(rect2, brush);
      }
   }

   QSlider::paintEvent(pEvent);
}

void AnimationToolBarImp::WheelEventSlider::resizeEvent(QResizeEvent* pEvent)
{
   // Set the page step based on a fixed number of screen pixels
   const int stepSize = 20;
   int minValue = minimum();
   int maxValue = maximum();

   const QSize& newSize = pEvent->size();
   int newWidth = newSize.width();

   double pixelRatio = 1.0;
   if (stepSize < newWidth)
   {
      pixelRatio = static_cast<double>(stepSize) / static_cast<double>(newWidth);
   }

   int pageSize = static_cast<int>((maxValue - minValue) * pixelRatio);
   setPageStep(pageSize);

   mpToolbar->updateBumpers();
}

void AnimationToolBarImp::WheelEventSlider::setLeftBumper(int index)
{
   // convert index to widget rectangle coordinates
   mLeftBumper = (index * (rect().width() - 1)) / (maximum() - minimum());
   if (mBumpersEnabled)
   {
      repaint();
   }
}

void AnimationToolBarImp::WheelEventSlider::setRightBumper(int index)
{
   // convert index to widget rectangle coordinates
   mRightBumper = (index * (rect().width() - 1)) / (maximum() - minimum());
   if (mBumpersEnabled)
   {
      repaint();
   }
}

void AnimationToolBarImp::WheelEventSlider::resetBumpers()
{
   mLeftBumper = rect().left();
   mRightBumper = rect().right();
   repaint();
}

void AnimationToolBarImp::WheelEventSlider::setBumpersEnabled(bool enabled)
{
   mBumpersEnabled = enabled;
   repaint();
}

bool AnimationToolBarImp::WheelEventSlider::getBumpersEnabled() const
{
   return mBumpersEnabled;
}

void AnimationToolBarImp::WheelEventSlider::getPlaybackRange(int& start, int& stop) const
{
   // return in units of slider index
   int sliderWidth = width() - 1;
   if (mBumpersEnabled && sliderWidth > 0)
   {
      int sliderRange = maximum() - minimum();
      if (mLeftBumper > -1)
      {
         start = (mLeftBumper * sliderRange) / sliderWidth;
      }
      else
      {
         start = minimum();
      }
      if (mRightBumper > -1)
      {
         stop = (mRightBumper * sliderRange) / sliderWidth;
      }
      else
      {
         stop = maximum();
      }
   }
   else
   {
      start = minimum();
      stop = maximum();
   }
}

bool AnimationToolBarImp::WheelEventSlider::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   pXml->addAttr("bumpersEnabled", mBumpersEnabled);
   pXml->addAttr("leftBumper", mLeftBumper);
   pXml->addAttr("rightBumper", mRightBumper);

   return true;
}

bool AnimationToolBarImp::WheelEventSlider::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   mBumpersEnabled = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("bumpersEnabled"))));
   mLeftBumper = StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("leftBumper"))));
   mRightBumper = StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("rightBumper"))));

   return true;
}
