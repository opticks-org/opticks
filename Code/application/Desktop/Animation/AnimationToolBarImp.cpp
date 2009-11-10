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
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationControllerImp.h"
#include "AnimationCycleButton.h"
#include "AnimationImp.h"
#include "AnimationToolBar.h"
#include "AnimationToolBarImp.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "PixmapGrid.h"
#include "StringUtilities.h"

#include <math.h>

#include <boost/rational.hpp>
#include <string>

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
   mpDropFramesAction(NULL),
   mpCycle(NULL),
   mpTimestampLabel(NULL),
   mpController(NULL),
   mPrevAnimationState(STOP),
   mHideTimestamp(false)
{
   Service<DesktopServices> pDesktop;
   std::string shortcutContext = windowTitle().toStdString();

   // Animation buttons
   mpStopAction = addAction(QIcon(":/icons/Stop"), QString(), this, SLOT(stop()));
   mpStopAction->setAutoRepeat(false);
   mpStopAction->setToolTip("Stop");
   mpStopAction->setCheckable(true);
   pDesktop->initializeAction(mpStopAction, shortcutContext);

   mpPlayPauseAction = addAction(QIcon(":/icons/PlayForward"), QString(), this, SLOT(playPause()));
   mpPlayPauseAction->setAutoRepeat(false);
   mpPlayPauseAction->setShortcut(QKeySequence(Qt::Key_Space));
   mpPlayPauseAction->setToolTip("Play//Pause");
   pDesktop->initializeAction(mpPlayPauseAction, shortcutContext);

   mpStepBackwardAction = addAction(QIcon(":/icons/AdvanceBackward"), QString(), this, SLOT(stepBackward()));
   mpStepBackwardAction->setAutoRepeat(true);
   mpStepBackwardAction->setToolTip("Step backward");
   pDesktop->initializeAction(mpStepBackwardAction, shortcutContext);

   mpStepForwardAction = addAction(QIcon(":/icons/AdvanceForward"), QString(), this, SLOT(stepForward()));
   mpStepForwardAction->setAutoRepeat(true);
   mpStepForwardAction->setToolTip("Step forward");
   pDesktop->initializeAction(mpStepForwardAction, shortcutContext);

   addSeparator();

   mpSlowDownAction = addAction(QIcon(":/icons/SlowDown"), QString(), this, SLOT(slowDown()));
   mpSlowDownAction->setAutoRepeat(false);
   mpSlowDownAction->setToolTip("Slow Down");
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

   mpSpeedUpAction = addAction(QIcon(":/icons/SpeedUp"), QString(), this, SLOT(speedUp()));
   mpSpeedUpAction->setAutoRepeat(false);
   mpSpeedUpAction->setToolTip("Speed Up");
   pDesktop->initializeAction(mpSpeedUpAction, shortcutContext);

   addSeparator();

   mpChangeDirectionAction = addAction(QIcon(":/icons/DirectionForward"), QString(), this, SLOT(changeDirection()));
   mpChangeDirectionAction->setAutoRepeat(false);
   mpChangeDirectionAction->setToolTip("Change Direction");
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

   mpFrameSlider = new WheelEventSlider(Qt::Horizontal, pSliderWidget);
   mpFrameSlider->setFixedWidth(200);
   mpFrameSlider->setToolTip("The current position in the animation");
   mpFrameSlider->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
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

   // Current frame and time labels
   mpTimestampLabel = new QLabel(this);
   mpTimestampLabel->setAlignment(Qt::AlignCenter);
   addWidget(mpTimestampLabel);

   addSeparator();

   // Drop frames
   mpDropFramesAction = addAction(QIcon(":/icons/Clock"), QString());
   mpDropFramesAction->setAutoRepeat(false);
   mpDropFramesAction->setToolTip("Drop Frames");
   mpDropFramesAction->setCheckable(true);
   pDesktop->initializeAction(mpDropFramesAction, shortcutContext);
   VERIFYNR(connect(mpDropFramesAction, SIGNAL(toggled(bool)), this, SLOT(setCanDropFrames(bool))));

   // Initialization
   setFocusPolicy(Qt::ClickFocus);     // Required to set the frame speed if the user edits the value
                                       // and clicks the Play button without pressing the return key
   updateAnimationCycle(PLAY_ONCE);
   updateAnimationControls();
}

AnimationToolBarImp::~AnimationToolBarImp()
{
}

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
         mpPlayPauseAction->setToolTip("Pause");
      }
      else
      {
         mpPlayPauseAction->setIcon(QIcon(":/icons/PlayForward"));
         mpPlayPauseAction->setToolTip("Play");
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
}

void AnimationToolBarImp::setCurrentFrame(int frameIndex)
{
   if (mpController != NULL)
   {
      double startFrame = mpController->getStartFrame();
      double range = mpController->getStopFrame() - startFrame;
      double fraction = static_cast<double>(frameIndex) / static_cast<double>(mpFrameSlider->maximum());
      double newFrame = startFrame + range * fraction;
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
      updateFrameRange();
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
   int pageStep = 1;
   double currentFrame = -1.0;
   int timestampWidth = 0;

   if (mpController != NULL)
   {
      // Get the range from the animation controller
      double startFrame = mpController->getStartFrame();
      double stopFrame = mpController->getStopFrame();
      double intervalMultiplier = mpController->getIntervalMultiplier();
      const int frequency = mpController->getFrequency();
      currentFrame = mpController->getCurrentFrame();

      if (intervalMultiplier == 0.0)
      {
         intervalMultiplier = 1.0;
      }

      // Calculate the total number of steps, adding 0.5 to round to the nearest number
      double dNumSteps = (stopFrame - startFrame) * frequency / intervalMultiplier;

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
      lineStep = frequency / intervalMultiplier;

      // Define the page step size so ten steps for the entire range
      pageStep = numSteps / 10;

      // Determine the maximum width of the timestamp
      FrameType frameType = mpController->getFrameType();
      QString sample;
      if (frameType == FRAME_ID)
      {
         int digits = 0;
         if (stopFrame > -1)
         {
            digits = log10(stopFrame+1) + 1;
         }
         sample = QString("Frame: %1/%2").arg(0, digits, 10, QChar('0')).arg(static_cast<int>(stopFrame + 1));
      }
      else
      {
         sample = "yyyy/MM/dd hh:mm:ss.zzzZ";
      }

      QFontMetrics metric(mpTimestampLabel->font());
      timestampWidth = metric.width(sample);
   }

   // Update the slider range
   mpFrameSlider->setRange(0, numSteps);
   mpFrameSlider->setSingleStep(lineStep);
   mpFrameSlider->setPageStep(pageStep);

   // update bumpers
   updateBumpers();

   mpTimestampLabel->setFixedWidth(timestampWidth);

   // Update the slider value
   updateCurrentFrame(currentFrame);

   // Make sure the toolbar updates its size with the label width changing
   adjustSize();
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
      double intervalMultiplier = mpController->getIntervalMultiplier();
      const int frequency = mpController->getFrequency();

      // Add 0.5 to round to the nearest index
      currentStep = static_cast<int> (mpFrameSlider->maximum() * ((frameValue - startFrame) / (stopFrame - startFrame)) + 0.5);

      // Slider
      mpFrameSlider->setValue(currentStep);

      // Frame text
      if (mHideTimestamp == false)
      {
         strFrameText = AnimationImp::frameToQString(frameValue, mpController->getFrameType(),
            static_cast<unsigned int> (stopFrame - startFrame + 1.0));
      }
   }

   VERIFYNRV(mpTimestampLabel != NULL);
   mpTimestampLabel->setText(strFrameText);
}

void AnimationToolBarImp::cleanUpItems()
{
   std::vector<double> frameSpeeds = AnimationToolBar::getSettingFrameSpeeds();

   if (AnimationToolBar::getSettingFrameSpeeds().size() != mpFrameSpeedCombo->count())
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

   updateFrameRange();
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
      VERIFYNR(disconnect(mpController, SIGNAL(canDropFramesChanged(bool)), this,
         SLOT(setCanDropFrames(bool))));
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
      VERIFYNR(connect(mpController, SIGNAL(canDropFramesChanged(bool)), this,
         SLOT(setCanDropFrames(bool))));
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
      mpDropFramesAction->setChecked(mpController->getCanDropFrames());
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
   mpDropFramesAction->setEnabled(bEnable);
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

void AnimationToolBarImp::setCanDropFrames(bool canDropFrames)
{
   if (mpController != NULL)
   {
      mpController->setCanDropFrames(canDropFrames);
   }
   mpDropFramesAction->setChecked(canDropFrames);
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

int AnimationToolBarImp::getSliderIndex(double frameValue)
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

AnimationToolBarImp::WheelEventSlider::WheelEventSlider(
                                   Qt::Orientation orientation, QWidget *parent)
                                   : QSlider(orientation, parent),
                                   mLeftBumper(-1),
                                   mRightBumper(-1),
                                   mBumpersEnabled(false)
{
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
