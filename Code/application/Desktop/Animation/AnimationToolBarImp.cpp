/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtCore/QDateTime>
#include <QtCore/QEvent>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include "Animation.h"
#include "AnimationImp.h"
#include "AnimationController.h"
#include "AnimationControllerImp.h"
#include "AnimationToolBar.h"
#include "AnimationToolBarImp.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "Icons.h"
#include "PixmapGrid.h"
#include "StringUtilities.h"

#include <boost/rational.hpp>
#include <string>
using namespace std;

AnimationToolBarImp::WheelEventSlider::WheelEventSlider(
   Qt::Orientation orientation, QWidget *parent)
   : QSlider(orientation, parent)
{
}

void AnimationToolBarImp::WheelEventSlider::wheelEvent(QWheelEvent *e)
{
   QSlider::wheelEvent(e);
   triggerAction(QAbstractSlider::SliderMove);
}

AnimationToolBarImp::AnimationToolBarImp(const string& id, QWidget* parent) :
   ToolBarImp(id, "Animation", parent),
   mpStopAction(NULL),
   mpPlayPauseAction(NULL),
   mpBackwardAction(NULL),
   mpForwardAction(NULL),
   mpFrameSpeedCombo(NULL),
   mpStepForwardAction(NULL),
   mpStepBackwardAction(NULL),
   mpFrameSlider(NULL),
   mpDropFramesAction(NULL),
   mpCycle(NULL),
   mpTimestampLabel(NULL),
   mpController(NULL),
   mPrevAnimationState(STOP),
   mHideTimestamp(false)
{
   Service<DesktopServices> pDesktop;
   string shortcutContext = windowTitle().toStdString();

   // Animation buttons
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);

   mpStopAction = addAction(pIcons->mAnimationStop, QString(), this, SLOT(stop()));
   mpStopAction->setAutoRepeat(false);
   mpStopAction->setToolTip("Stop");
   mpStopAction->setCheckable(true);
   pDesktop->initializeAction(mpStopAction, shortcutContext);

   mpPlayPauseAction = addAction(pIcons->mAnimationPlayForward, QString(), this, SLOT(playPause()));
   mpPlayPauseAction->setAutoRepeat(false);
   mpPlayPauseAction->setToolTip("Play");
   pDesktop->initializeAction(mpPlayPauseAction, shortcutContext);

   mpBackwardAction = addAction(pIcons->mAnimationFastRewind, QString(), this, SLOT(backward()));
   mpBackwardAction->setAutoRepeat(false);
   mpBackwardAction->setToolTip("Rewind");
   pDesktop->initializeAction(mpBackwardAction, shortcutContext);

   mpForwardAction = addAction(pIcons->mAnimationFastForward, QString(), this, SLOT(forward()));
   mpForwardAction->setAutoRepeat(false);
   mpForwardAction->setToolTip("Fast forward");
   pDesktop->initializeAction(mpForwardAction, shortcutContext);

   mpFrameSpeedCombo = new QComboBox(this);
   mpFrameSpeedCombo->addItem("0.1");
   mpFrameSpeedCombo->addItem("0.25");
   mpFrameSpeedCombo->addItem("0.5");
   mpFrameSpeedCombo->addItem("0.75");
   mpFrameSpeedCombo->addItem("1");
   mpFrameSpeedCombo->addItem("2");
   mpFrameSpeedCombo->addItem("5");
   mpFrameSpeedCombo->addItem("10");
   mpFrameSpeedCombo->addItem("30");
   mpFrameSpeedCombo->addItem("60");
   mpFrameSpeedCombo->setInsertPolicy(QComboBox::NoInsert);
   mpFrameSpeedCombo->setToolTip("Speed (X)");
   mpFrameSpeedCombo->setEditable(true);
   mpFrameSpeedCombo->setCompleter(NULL);
   QDoubleValidator* pValidator = new QDoubleValidator(mpFrameSpeedCombo);
   pValidator->setDecimals(2);

   mpFrameSpeedCombo->setValidator(pValidator);
   VERIFYNR(connect(mpFrameSpeedCombo, SIGNAL(activated(const QString&)), this, SLOT(setFrameSpeed(const QString&))));

   QLineEdit* pSpeedEdit = mpFrameSpeedCombo->lineEdit();
   if (pSpeedEdit != NULL)
   {
      VERIFYNR(connect(pSpeedEdit, SIGNAL(editingFinished()), this, SLOT(setFrameSpeed())));
   }

   addWidget(mpFrameSpeedCombo);

   mpStepBackwardAction = addAction(pIcons->mAnimationAdvanceBackward, QString(), this, SLOT(stepBackward()));
   mpStepBackwardAction->setAutoRepeat(false);
   mpStepBackwardAction->setToolTip("Step backward");
   pDesktop->initializeAction(mpStepBackwardAction, shortcutContext);

   mpStepForwardAction = addAction(pIcons->mAnimationAdvanceForward, QString(), this, SLOT(stepForward()));
   mpStepForwardAction->setAutoRepeat(false);
   mpStepForwardAction->setToolTip("Step forward");
   pDesktop->initializeAction(mpStepForwardAction, shortcutContext);

   addSeparator();

   // Current frame slider
   QWidget *pSliderWidget = new QWidget(this);

   mpFrameSlider = new WheelEventSlider(Qt::Horizontal, pSliderWidget);
   mpFrameSlider->setFixedWidth(200);
   mpFrameSlider->setToolTip("The current position in the animation");
   mpFrameSlider->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
   VERIFYNR(connect(mpFrameSlider, SIGNAL(sliderPressed()), this, SLOT(activateSlider())));
   VERIFYNR(connect(mpFrameSlider, SIGNAL(sliderReleased()), this, SLOT(releaseSlider())));
   VERIFYNR(connect(mpFrameSlider, SIGNAL(actionTriggered(int)), this, SLOT(sliderActionTriggered(int))));

   QHBoxLayout *pSliderLayout = new QHBoxLayout(pSliderWidget);
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

   // Animation cycle
   mpCycle = new AnimationCycleButton(this);
   mpCycle->setStatusTip("Specifies the play behavior when the end of the animation is reached");
   mpCycle->setToolTip("Animation Cycle");
   VERIFYNR(connect(mpCycle, SIGNAL(valueChanged(AnimationCycle)), this, SLOT(updateAnimationCycle(AnimationCycle))));
   addWidget(mpCycle);

   // Drop frames
   mpDropFramesAction = addAction(pIcons->mClock, QString());
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

const string& AnimationToolBarImp::getObjectType() const
{
   static string type("AnimationToolBarImp");
   return type;
}

bool AnimationToolBarImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationToolBar"))
   {
      return true;
   }

   return ToolBarImp::isKindOf(className);
}

void AnimationToolBarImp::forward()
{
   if (mpController != NULL)
   {
      AnimationState state = mpController->getAnimationState();
      if (state == PLAY_BACKWARD || state == PAUSE_FORWARD || state == PAUSE_BACKWARD || state == STOP)
      {
         mpController->setAnimationState(PLAY_FORWARD);
         mpController->play();
      }
      else
      {
         // change the speed to the next value in the list
         int itemPos = mpFrameSpeedCombo->currentIndex();
         int maxItems = mpFrameSpeedCombo->count();
         if ((itemPos + 1) < maxItems)
         {
            itemPos++;
         }
         else
         {
            itemPos = 0;
         }

         QString speedMultiplier = mpFrameSpeedCombo->itemText(itemPos);
         double multiplier = speedMultiplier.toDouble();

         // use multiplier to change the speed at which the animation is playing
         mpController->fastForward(multiplier);
      }
   }
}

void AnimationToolBarImp::backward()
{
   if (mpController != NULL)
   {
      AnimationState state = mpController->getAnimationState();

      if (state == PLAY_FORWARD || state == PAUSE_FORWARD || state == PAUSE_BACKWARD || state == STOP)
      {
         mpController->setAnimationState(PLAY_BACKWARD);
         mpController->play();
      }
      else
      {
         // change the speed to the next value in the list
         int itemPos = mpFrameSpeedCombo->currentIndex();
         int maxItems = mpFrameSpeedCombo->count();
         if ((itemPos + 1) < maxItems)
         {
            itemPos++;
         }
         else
         {
            itemPos = 0;
         }

         QString speedMultiplier = mpFrameSpeedCombo->itemText(itemPos);
         double multiplier = speedMultiplier.toDouble();

         // use multiplier to change the speed at which the animation is playing
         mpController->fastRewind(multiplier);
      }
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

void AnimationToolBarImp::setPlayButtonState(AnimationState state)
{
   if (mpController != NULL)
   {
      Icons* pIcons = Icons::instance();
      REQUIRE(pIcons != NULL);
      if (state == PLAY_FORWARD || state == PLAY_BACKWARD)
      {
         mpPlayPauseAction->setIcon(pIcons->mAnimationPause);
         mpPlayPauseAction->setToolTip("Pause");
      }
      else
      {
         mpPlayPauseAction->setIcon(pIcons->mAnimationPlayForward);
         mpPlayPauseAction->setToolTip("Play");
      }

      if (mpController->getNumAnimations() > 0)
      {
         mpStepForwardAction->setEnabled((state == PAUSE_FORWARD) || (state == PAUSE_BACKWARD) || (state == STOP));
         mpStepBackwardAction->setEnabled((state == PAUSE_FORWARD) || (state == PAUSE_BACKWARD) || (state == STOP));
      }
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
   setCurrentFrame(mpFrameSlider->sliderPosition());
}

void AnimationToolBarImp::setCurrentFrame(int frameIndex)
{
   if (mpController != NULL)
   {
      double startFrame = mpController->getStartFrame();
      double intervalMultiplier = mpController->getIntervalMultiplier();
      const int frequency = mpController->getFrequency();
      double newFrame = startFrame + (intervalMultiplier / frequency * frameIndex);
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
      numSteps = static_cast<int> (((stopFrame - startFrame) * frequency / intervalMultiplier) + 0.5);

      // Define the line step size as one frame or one second
      lineStep = frequency / intervalMultiplier;

      // Define the page step size
      if ((stopFrame - startFrame) < 10.0)
      {
         // The range is less than ten, so use the line step size of one frame or one second
         pageStep = lineStep;
      }
      else
      {
         // Ten steps for the entire range
         pageStep = numSteps / 10;
      }

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
         sample = QString("Frame: %1/%2")
            .arg(int(0), digits, 10, QChar('0'))
            .arg(static_cast<int>(stopFrame+1));
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

   mpTimestampLabel->setFixedWidth(timestampWidth);

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
      double intervalMultiplier = mpController->getIntervalMultiplier();
      const int frequency = mpController->getFrequency();

      // Add 0.5 to round to the nearest index
      currentStep = static_cast<int> (((frameValue - startFrame) * frequency / intervalMultiplier) + 0.5);

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

void AnimationToolBarImp::updateFrameSpeed(double speed)
{
   QString strSpeed = QString::number(speed);
   int index = mpFrameSpeedCombo->findText(strSpeed);
   if (index != -1)
   {
      mpFrameSpeedCombo->setCurrentIndex(index);
   }
   else
   {
      mpFrameSpeedCombo->setEditText(strSpeed);
   }

   updateFrameRange();
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
   if (pController == mpController)
   {
      return;
   }

   AnimationState state = STOP;
   double intervalMultiplier = 1.0;
   AnimationCycle cycle = PLAY_ONCE;

   if (mpController != NULL)
   {
      AnimationControllerImp* pControllerImp = dynamic_cast<AnimationControllerImp*>(mpController);
      if (pControllerImp != NULL)
      {
         VERIFYNR(disconnect(pControllerImp, SIGNAL(frameRangeChanged()), this, SLOT(updateFrameRange())));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(frameChanged(double)), this, SLOT(updateCurrentFrame(double))));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(animationStateChanged(AnimationState)), this,
            SLOT(updateAnimationState(AnimationState))));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(animationCycleChanged(AnimationCycle)), this,
            SLOT(updateAnimationCycle(AnimationCycle))));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(intervalMultiplierChanged(double)), this,
            SLOT(updateFrameSpeed(double))));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(animationAdded(Animation*)), this,
            SLOT(updateAnimationControls())));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(animationRemoved(Animation*)), this,
            SLOT(updateAnimationControls())));
         VERIFYNR(disconnect(pControllerImp, SIGNAL(canDropFramesChanged(bool)), this,
            SLOT(setCanDropFrames(bool))));
      }
   }

   mpController = pController;

   if (mpController != NULL)
   {
      // Get current values for the widgets
      state = mpController->getAnimationState();
      intervalMultiplier = mpController->getIntervalMultiplier();
      cycle = mpController->getAnimationCycle();

      AnimationControllerImp* pControllerImp = dynamic_cast<AnimationControllerImp*>(mpController);
      if (pControllerImp != NULL)
      {
         VERIFYNR(connect(pControllerImp, SIGNAL(frameRangeChanged()), this, SLOT(updateFrameRange())));
         VERIFYNR(connect(pControllerImp, SIGNAL(frameChanged(double)), this, SLOT(updateCurrentFrame(double))));
         VERIFYNR(connect(pControllerImp, SIGNAL(animationStateChanged(AnimationState)), this,
            SLOT(updateAnimationState(AnimationState))));
         VERIFYNR(connect(pControllerImp, SIGNAL(animationCycleChanged(AnimationCycle)), this,
            SLOT(updateAnimationCycle(AnimationCycle))));
         VERIFYNR(connect(pControllerImp, SIGNAL(intervalMultiplierChanged(double)), this,
            SLOT(updateFrameSpeed(double))));
         VERIFYNR(connect(pControllerImp, SIGNAL(animationAdded(Animation*)), this, SLOT(updateAnimationControls())));
         VERIFYNR(connect(pControllerImp, SIGNAL(animationRemoved(Animation*)), this, SLOT(updateAnimationControls())));
         VERIFYNR(connect(pControllerImp, SIGNAL(canDropFramesChanged(bool)), this,
            SLOT(setCanDropFrames(bool))));

         mpDropFramesAction->setChecked(pControllerImp->getCanDropFrames());
     }
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
   mpBackwardAction->setEnabled(bEnable);
   mpForwardAction->setEnabled(bEnable);
   mpStepBackwardAction->setEnabled(bEnable);
   mpStepForwardAction->setEnabled(bEnable);
   mpTimestampLabel->setHidden(!bEnable || mHideTimestamp);
   mpFrameSlider->setEnabled(bEnable);
   mpFrameSpeedCombo->setEnabled(bEnable);
   mpCycle->setEnabled(bEnable);
   mpDropFramesAction->setEnabled(bEnable);
}

AnimationController* AnimationToolBarImp::getAnimationController() const
{
   return mpController;
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

AnimationCycleGrid::AnimationCycleGrid(QWidget* pParent)
: PixmapGrid(pParent)
{
   setNumRows(1);
   setNumColumns(3);

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setPixmap(0, 0, pIcons->mPlayOnce,
      QString::fromStdString(StringUtilities::toXmlString(PLAY_ONCE)),
      QString::fromStdString(StringUtilities::toDisplayString(PLAY_ONCE)));
   setPixmap(0, 1, pIcons->mBouncePlay,
      QString::fromStdString(StringUtilities::toXmlString(BOUNCE)),
      QString::fromStdString(StringUtilities::toDisplayString(BOUNCE)));
   setPixmap(0, 2, pIcons->mRepeatPlay,
      QString::fromStdString(StringUtilities::toXmlString(REPEAT)),
      QString::fromStdString(StringUtilities::toDisplayString(REPEAT)));

   // Set the current symbol
   setSelectedPixmap(QString::fromStdString(StringUtilities::toXmlString(PLAY_ONCE)));

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void AnimationCycleGrid::setCurrentValue(AnimationCycle value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

AnimationCycle AnimationCycleGrid::getCurrentValue() const
{
   AnimationCycle retValue;
   string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<AnimationCycle>(curText);
   }
   return retValue;
}

void AnimationCycleGrid::translateChange(const QString& strText)
{
   AnimationCycle curType = StringUtilities::fromXmlString<AnimationCycle>(strText.toStdString());
   emit valueChanged(curType);
}

AnimationCycleButton::AnimationCycleButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   AnimationCycleGrid* pGrid = new AnimationCycleGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(pGrid, SIGNAL(valueChanged(AnimationCycle)), this, SIGNAL(valueChanged(AnimationCycle))));
}

void AnimationCycleButton::setCurrentValue(AnimationCycle value)
{
   AnimationCycleGrid* pGrid = dynamic_cast<AnimationCycleGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

AnimationCycle AnimationCycleButton::getCurrentValue() const
{
   AnimationCycle retValue;
   AnimationCycleGrid* pGrid = dynamic_cast<AnimationCycleGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}
