/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtGui/QComboBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTextEdit>

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationFrame.h"
#include "AnimationServices.h"
#include "AnimationTest.h"
#include "AnimationToolBar.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"

#include <boost/any.hpp>
#include <math.h>
#include <time.h>
#include <vector>

using namespace std;

//////////////////////////////////////////////////////////////////////////
// AnimationTestPlugIn

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, AnimationTestPlugIn);

AnimationTestPlugIn::AnimationTestPlugIn() :
   mpDialog(NULL)
{
   setName("AnimationTest Dialog");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Tests various Animation-related functionality.");
   setShortDescription("Tests various Animation-related functionality.");
   setDescriptorId("{6FA89EF4-6029-47db-B883-E02A3F23B2D0}");
   setMenuLocation("[Demo]/Animation Test");
   setProductionStatus(false);
   setWizardSupported(false);
}

AnimationTestPlugIn::~AnimationTestPlugIn()
{}

bool AnimationTestPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (mpDialog == NULL)
   {
      Service<DesktopServices> pDesktop;
      mpDialog = new AnimationTestDlg(this, pDesktop->getMainWidget());
   }

   if (mpDialog != NULL)
   {
      mpDialog->show();
   }

   return mpDialog != NULL;
}

bool AnimationTestPlugIn::abort()
{
   return ViewerShell::abort();
}

QWidget* AnimationTestPlugIn::getWidget() const
{
   return mpDialog;
}

//////////////////////////////////////////////////////////////////////////
// AnimationTestDlg

AnimationTestDlg::AnimationTestDlg(PlugIn* pPlugIn, QWidget* pParent) :
   QDialog(pParent),
   mpPlugIn(pPlugIn),
   mAnimationNumber(0)
{
   // Animation Controller
   QWidget* pControllerWidget = new QWidget(this);
   QMenu* pFrameTypeMenu = new QMenu(this);

   vector<string> frameTypes = StringUtilities::getAllEnumValuesAsDisplayString<FrameType>();
   for (vector<string>::const_iterator iter = frameTypes.begin(); iter != frameTypes.end(); ++iter)
   {
      pFrameTypeMenu->addAction(QString::fromStdString(*iter));
   }

   QPushButton* pCreateControllerButton = new QPushButton("Create", pControllerWidget);
   pCreateControllerButton->setMenu(pFrameTypeMenu);
   QPushButton* pDestroyControllerButton = new QPushButton("Destroy", pControllerWidget);

   QHBoxLayout* pControllerLayout = new QHBoxLayout(pControllerWidget);
   pControllerLayout->setMargin(0);
   pControllerLayout->setSpacing(5);
   pControllerLayout->addWidget(pCreateControllerButton);
   pControllerLayout->addWidget(pDestroyControllerButton);
   pControllerLayout->addStretch(10);

   LabeledSection* pControllerSection = new LabeledSection(pControllerWidget, "Animation Controller", this);

   // Create Animations
   QWidget* pCreateAnimationsWidget = new QWidget(this);

   QLabel* pNumAnimationsLabel = new QLabel("Number of Animations:", pCreateAnimationsWidget);
   mpNumAnimations = new QSpinBox(pCreateAnimationsWidget);
   mpNumAnimations->setRange(1, 10000);

   QLabel* pNumFramesLabel = new QLabel("Number of Frames:", pCreateAnimationsWidget);
   mpNumFrames = new QSpinBox(pCreateAnimationsWidget);
   mpNumFrames->setRange(1, 10000);

   mpStack = new QStackedWidget(pCreateAnimationsWidget);
   QLabel* pEmptyLabel = new QLabel(mpStack);

   QWidget* pFrameTimeWidget = new QWidget(mpStack);

   QLabel* pMinFrameTimeLabel = new QLabel("Min Frame Time:", pFrameTimeWidget);
   mpMinFrameTime = new QDateTimeEdit(pFrameTimeWidget);
   mpMinFrameTime->setMinimumDateTime(QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0)));   // Valid range for converting
   mpMinFrameTime->setMaximumDateTime(QDateTime(QDate(2106, 2, 7), QTime(6, 28, 14))); // the QDateTime to a time_t
   mpMinFrameTime->setCalendarPopup(true);

   QLabel* pMaxFrameTimeLabel = new QLabel("Max Frame Time:", pFrameTimeWidget);
   mpMaxFrameTime = new QDateTimeEdit(pFrameTimeWidget);
   mpMaxFrameTime->setMinimumDateTime(QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0)));
   mpMaxFrameTime->setMaximumDateTime(QDateTime(QDate(2106, 2, 7), QTime(6, 28, 14)));
   mpMaxFrameTime->setCalendarPopup(true);

   QGridLayout* pFrameTimeLayout = new QGridLayout(pFrameTimeWidget);
   pFrameTimeLayout->setMargin(0);
   pFrameTimeLayout->setSpacing(5);
   pFrameTimeLayout->addWidget(pMinFrameTimeLabel, 0, 0);
   pFrameTimeLayout->addWidget(mpMinFrameTime, 0, 1);
   pFrameTimeLayout->addWidget(pMaxFrameTimeLabel, 1, 0);
   pFrameTimeLayout->addWidget(mpMaxFrameTime, 1, 1);
   pFrameTimeLayout->setRowStretch(2, 10);

   QWidget* pFrameElapsedTimeWidget = new QWidget(mpStack);

   QLabel* pHoursLabel = new QLabel("Hours:", pFrameElapsedTimeWidget);
   mpHoursSpin = new QSpinBox(pFrameElapsedTimeWidget);
   mpHoursSpin->setRange(0, 1000);

   QLabel* pMinutesLabel = new QLabel("Minutes:", pFrameElapsedTimeWidget);
   mpMinutesSpin = new QSpinBox(pFrameElapsedTimeWidget);
   mpMinutesSpin->setRange(0, 59);

   QLabel* pSecondsLabel = new QLabel("Seconds:", pFrameElapsedTimeWidget);
   mpSecondsSpin = new QSpinBox(pFrameElapsedTimeWidget);
   mpSecondsSpin->setRange(0, 59);

   QGridLayout* pFrameElapsedTimeLayout = new QGridLayout(pFrameElapsedTimeWidget);
   pFrameElapsedTimeLayout->setMargin(0);
   pFrameElapsedTimeLayout->setSpacing(5);
   pFrameElapsedTimeLayout->addWidget(pHoursLabel, 0, 0);
   pFrameElapsedTimeLayout->addWidget(mpHoursSpin, 0, 1);
   pFrameElapsedTimeLayout->addWidget(pMinutesLabel, 1, 0);
   pFrameElapsedTimeLayout->addWidget(mpMinutesSpin, 1, 1);
   pFrameElapsedTimeLayout->addWidget(pSecondsLabel, 2, 0);
   pFrameElapsedTimeLayout->addWidget(mpSecondsSpin, 2, 1);

   mpStack->addWidget(pEmptyLabel);
   mpStack->addWidget(pFrameTimeWidget);
   mpStack->addWidget(pFrameElapsedTimeWidget);

   QPushButton* pCreateAnimationsButton = new QPushButton("Create", pCreateAnimationsWidget);

   QGridLayout* pCreateAnimationsLayout = new QGridLayout(pCreateAnimationsWidget);
   pCreateAnimationsLayout->setMargin(0);
   pCreateAnimationsLayout->setSpacing(5);
   pCreateAnimationsLayout->addWidget(pNumAnimationsLabel, 0, 0);
   pCreateAnimationsLayout->addWidget(mpNumAnimations, 0, 1);
   pCreateAnimationsLayout->addWidget(pNumFramesLabel, 1, 0);
   pCreateAnimationsLayout->addWidget(mpNumFrames, 1, 1);
   pCreateAnimationsLayout->addWidget(mpStack, 2, 0, 1, 2);
   pCreateAnimationsLayout->addWidget(pCreateAnimationsButton, 3, 0, Qt::AlignLeft);
   pCreateAnimationsLayout->setColumnStretch(2, 10);

   LabeledSection* pCreateAnimationsSection = new LabeledSection(pCreateAnimationsWidget, "Create Animations", this);

   // Existing Animations
   QWidget* pExistingAnimationsWidget = new QWidget(this);

   mpAnimationList = new QComboBox(pExistingAnimationsWidget);
   QPushButton* pClearAnimationButton = new QPushButton("Clear", pExistingAnimationsWidget);
   QPushButton* pDestroyAnimationButton = new QPushButton("Destroy", pExistingAnimationsWidget);
   QPushButton* pViewFramesButton = new QPushButton("View Frames", pExistingAnimationsWidget);
   QPushButton* pViewAllFramesButton = new QPushButton("View All Frames", pExistingAnimationsWidget);

   QGridLayout* pExistingAnimationsLayout = new QGridLayout(pExistingAnimationsWidget);
   pExistingAnimationsLayout->setMargin(0);
   pExistingAnimationsLayout->setSpacing(5);
   pExistingAnimationsLayout->addWidget(mpAnimationList, 0, 0, 1, 3);
   pExistingAnimationsLayout->addWidget(pClearAnimationButton, 1, 0);
   pExistingAnimationsLayout->addWidget(pDestroyAnimationButton, 2, 0);
   pExistingAnimationsLayout->addWidget(pViewFramesButton, 1, 1);
   pExistingAnimationsLayout->addWidget(pViewAllFramesButton, 2, 1);
   pExistingAnimationsLayout->setColumnStretch(2, 10);

   LabeledSection* pExistingAnimationsSection = new LabeledSection(pExistingAnimationsWidget,
      "Existing Animations", this);

   // Misc
   QWidget* pMiscWidget = new QWidget(this);

   QPushButton* pToggleTimeDisplayButton = new QPushButton("Toggle Time Display", pMiscWidget);
   QPushButton* pToggleCanDropFramesButton = new QPushButton("Toggle Can Drop Frames", pMiscWidget);
   QPushButton* pDestroyAnimationsButton = new QPushButton("Destroy All Animations", pMiscWidget);

   QGridLayout* pMiscLayout = new QGridLayout(pMiscWidget);
   pMiscLayout->setMargin(0);
   pMiscLayout->setSpacing(5);
   pMiscLayout->addWidget(pToggleTimeDisplayButton, 0, 0);
   pMiscLayout->addWidget(pToggleCanDropFramesButton, 1, 0);
   pMiscLayout->addWidget(pDestroyAnimationsButton, 2, 0);
   pMiscLayout->setColumnStretch(1, 10);

   LabeledSection* pMiscSection = new LabeledSection(pMiscWidget, "Miscellaneous", this);

   // Labeled section group
   LabeledSectionGroup* pSectionGroup = new LabeledSectionGroup(this);
   pSectionGroup->addSection(pControllerSection);
   pSectionGroup->addSection(pCreateAnimationsSection);
   pSectionGroup->addSection(pExistingAnimationsSection);
   pSectionGroup->addSection(pMiscSection);
   pSectionGroup->addStretch(1000);

   // Line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pCloseButton = new QPushButton("Close", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(pSectionGroup, 10);
   pLayout->addWidget(pHLine);
   pLayout->addWidget(pCloseButton, 0, Qt::AlignRight);

   // Initialization
   setWindowTitle("Animation Test");
   setModal(false);
   resize(300, 500);

   // Connections
   VERIFYNRV(connect(pFrameTypeMenu, SIGNAL(triggered(QAction*)), this, SLOT(createController(QAction*))));
   VERIFYNRV(connect(pCreateAnimationsButton, SIGNAL(clicked()), this, SLOT(createAnimations())));
   VERIFYNRV(connect(pDestroyControllerButton, SIGNAL(clicked()), this, SLOT(destroyController())));
   VERIFYNRV(connect(pClearAnimationButton, SIGNAL(clicked()), this, SLOT(clearAnimation())));
   VERIFYNRV(connect(pDestroyAnimationButton, SIGNAL(clicked()), this, SLOT(destroyAnimation())));
   VERIFYNRV(connect(pViewFramesButton, SIGNAL(clicked()), this, SLOT(viewFrames())));
   VERIFYNRV(connect(pViewAllFramesButton, SIGNAL(clicked()), this, SLOT(viewAllFrames())));
   VERIFYNRV(connect(pToggleTimeDisplayButton, SIGNAL(clicked()), this, SLOT(toggleTimeDisplay())));
   VERIFYNRV(connect(pToggleCanDropFramesButton, SIGNAL(clicked()), this, SLOT(toggleCanDropFrames())));
   VERIFYNRV(connect(pDestroyAnimationsButton, SIGNAL(clicked()), this, SLOT(destroyAnimations())));
   VERIFYNRV(connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close())));

   mpController.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &AnimationTestDlg::controllerDeleted));
   mpController.addSignal(SIGNAL_NAME(AnimationController, AnimationAdded),
      Slot(this, &AnimationTestDlg::animationAdded));
   mpController.addSignal(SIGNAL_NAME(AnimationController, AnimationRemoved),
      Slot(this, &AnimationTestDlg::animationRemoved));
}

AnimationTestDlg::~AnimationTestDlg()
{
   destroyController();
}

void AnimationTestDlg::animationAdded(Subject& subject, const std::string& signal, const boost::any& value)
{
   VERIFYNRV(&subject == mpController.get());
   Animation* pAnimation = boost::any_cast<Animation*>(value);
   VERIFYNRV(pAnimation != NULL);

   mpAnimationList->addItem(QString::fromStdString(pAnimation->getName()));
}

void AnimationTestDlg::animationRemoved(Subject& subject, const std::string& signal, const boost::any& value)
{
   VERIFYNRV(&subject == mpController.get());
   Animation* pAnimation = boost::any_cast<Animation*>(value);
   VERIFYNRV(pAnimation != NULL);

   const int index = mpAnimationList->findText(QString::fromStdString(pAnimation->getName()));
   VERIFYNRV(index != -1);

   mpAnimationList->removeItem(index);
}

void AnimationTestDlg::controllerDeleted(Subject& subject, const std::string& signal, const boost::any& value)
{
   mpAnimationList->clear();
}

void AnimationTestDlg::createController(QAction* pAction)
{
   VERIFYNR(pAction != NULL);

   if (mpController.get() != NULL)
   {
      FrameType frameType = mpController->getFrameType();
      string frameTypeText = StringUtilities::toDisplayString(frameType);

      QString message = "An animation controller ";
      if (frameTypeText.empty() == false)
      {
         message += "with the " + QString::fromStdString(frameTypeText) + " frame type ";
      }

      message += "already exists.  The controller must first be destroyed before a new controller can be created.";
      QMessageBox::critical(this, windowTitle(), message);
      return;
   }

   QString frameTypeText = pAction->text();
   bool error = false;

   FrameType frameType = StringUtilities::fromDisplayString<FrameType>(frameTypeText.toStdString(), &error);
   if ((error == true) || (frameType.isValid() == false))
   {
      QMessageBox::critical(this, windowTitle(), "Could not determine the frame type for the animation controller.");
      return;
   }

   // Create the controller
   mAnimationNumber = 0;

   mpController.reset(Service<AnimationServices>()->createAnimationController("AnimationTest", frameType));
   VERIFYNRV(mpController.get() != NULL);

   // Make the created controller the active controller on the Animation toolbar
   AnimationToolBar* pToolBar = dynamic_cast<AnimationToolBar*>
      (Service<DesktopServices>()->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      pToolBar->setAnimationController(mpController.get());
   }

   // Update the stacked widget to display the appropriate animation options
   switch (frameType)
   {
   case FRAME_ID:    // Fall through
   default:
      mpStack->setCurrentIndex(0);
      break;

   case FRAME_TIME:
      mpStack->setCurrentIndex(1);
      break;

   case FRAME_ELAPSED_TIME:
      mpStack->setCurrentIndex(2);
      break;
   }
}

void AnimationTestDlg::destroyController()
{
   if (mpController.get() != NULL)
   {
      Service<AnimationServices>()->destroyAnimationController(mpController.get());
      mpStack->setCurrentIndex(0);
   }
}

void AnimationTestDlg::createAnimations()
{
   if (mpController.get() == NULL)
   {
      QMessageBox::critical(this, windowTitle(), "The animation controller must be created "
         "before creating animations.");
      return;
   }

   double range = 0.0;
   double minValue = 0.0;

   FrameType frameType = mpController->getFrameType();
   if (frameType != FRAME_ID)
   {
      if (frameType == FRAME_TIME)
      {
         minValue = mpMinFrameTime->dateTime().toTime_t();
         double maxValue = mpMaxFrameTime->dateTime().toTime_t();
         range = maxValue - minValue;

      }
      else if (frameType == FRAME_ELAPSED_TIME)
      {
         int hours = mpHoursSpin->value();
         int minutes = mpMinutesSpin->value();
         int seconds = mpSecondsSpin->value();
         range = (hours * 3600.0) + (minutes * 60.0) + seconds;
      }

      if (range <= 0.0)
      {
         QMessageBox::critical(this, windowTitle(), "The minimum frame value must be less than or "
            "equal to the maximum frame value.");
         return;
      }
   }

   srand(static_cast<unsigned int>(time(NULL)));
   const unsigned int numValues = static_cast<unsigned int>(mpNumFrames->value());
   const unsigned int numAnimations = static_cast<unsigned int>(mpNumAnimations->value());
   int animWidth = QString::number(numAnimations-1).length();
   int valWidth = QString::number(numValues-1).length();
   for (unsigned int i = 0; i < numAnimations; ++i)
   {
      QString animationName = QString("Animation_%1").arg(++mAnimationNumber, animWidth, 10, QChar('0'));
      Animation* pAnimation = mpController->createAnimation(animationName.toStdString());
      VERIFYNRV(pAnimation != NULL);

      vector<AnimationFrame> frames;
      for (unsigned int j = 0; j < numValues; j++)
      {
         QString animationFrameName = QString("AnimationFrame_%1").arg(j, valWidth, 10, QChar('0'));

         if (frameType == FRAME_ID)
         {
            frames.push_back(AnimationFrame(animationFrameName.toStdString(), j));
         }
         else if (frameType == FRAME_TIME)
         {
            // Get a random value between 0 and 1. Scale the value by range and offset it by minValue.
            double frameTime = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) * range) + minValue;
            frames.push_back(AnimationFrame(animationFrameName.toStdString(), j, frameTime));
            if (j == 0) // insert a duplicate frame
            {
               frames.push_back(AnimationFrame(animationFrameName.toStdString() + "_dup", j, frameTime));
            }
         }
         else if (frameType == FRAME_ELAPSED_TIME)
         {
            double frameTime = minValue;
            if (numValues > 1)
            {
               frameTime += j * range / (numValues - 1);
            }

            frames.push_back(AnimationFrame(animationFrameName.toStdString(), j, frameTime));
         }
      }

      pAnimation->setFrames(frames);
   }
}

void AnimationTestDlg::closeEvent(QCloseEvent* pEvent)
{
   if (mpPlugIn != NULL)
   {
      Executable* pExecutable = dynamic_cast<Executable*>(mpPlugIn);
      if (pExecutable != NULL)
      {
         pExecutable->abort();
      }
   }

   QDialog::closeEvent(pEvent);
}

void AnimationTestDlg::clearAnimation()
{
   if (mpController.get() != NULL)
   {
      string animationName = mpAnimationList->currentText().toStdString();
      Animation* pAnimation = mpController->getAnimation(animationName);
      if (pAnimation != NULL)
      {
         pAnimation->setFrames(vector<AnimationFrame>());
      }
   }
}

void AnimationTestDlg::destroyAnimation()
{
   if (mpController.get() != NULL)
   {
      string animationName = mpAnimationList->currentText().toStdString();
      Animation* pAnimation = mpController->getAnimation(animationName);
      mpController->destroyAnimation(pAnimation);
   }
}

void AnimationTestDlg::viewFrames()
{
   if (mpController.get() != NULL)
   {
      string animationName = mpAnimationList->currentText().toStdString();
      vector<Animation*> animations;
      animations.push_back(mpController->getAnimation(animationName));
      viewFrames(animations);
   }
}

void AnimationTestDlg::viewAllFrames()
{
   if (mpController.get() != NULL)
   {
      viewFrames(mpController->getAnimations());
   }
}

void AnimationTestDlg::viewFrames(const std::vector<Animation*>& animations)
{
   vector<AnimationFrame> frames;
   for (vector<Animation*>::const_iterator animationIter = animations.begin();
      animationIter != animations.end(); ++animationIter)
   {
      const Animation* pAnimation = *animationIter;
      if (pAnimation != NULL)
      {
         const vector<AnimationFrame>& currentFrames = pAnimation->getFrames();
         for (vector<AnimationFrame>::const_iterator frameIter = currentFrames.begin();
            frameIter != currentFrames.end(); ++frameIter)
         {
            frames.push_back(*frameIter);
         }
      }
   }

   sort(frames.begin(), frames.end());

   QString frameInfo;
   for (vector<AnimationFrame>::const_iterator frameIter = frames.begin();
      frameIter != frames.end(); ++frameIter)
   {
      frameInfo += QString("Frame Name: %1, Frame Time: %2<br>").arg(
         QString::fromStdString(frameIter->mName)).arg(frameIter->mTime);
   }

   if (frameInfo.isEmpty() == false)
   {
      QDialog dialog(this);
      QTextEdit* pInfo = new QTextEdit(frameInfo, &dialog);
      pInfo->setReadOnly(true);

      QVBoxLayout* pLayout = new QVBoxLayout(&dialog);
      pLayout->addWidget(pInfo);

      dialog.setWindowTitle("Animation Frames -- Sorted by Frame Time");
      dialog.setLayout(pLayout);
      dialog.resize(350, 350);
      dialog.exec();
   }
}

void AnimationTestDlg::toggleTimeDisplay()
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = dynamic_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      pToolBar->setHideTimestamp(pToolBar->getHideTimestamp() == false);
   }
}

void AnimationTestDlg::toggleCanDropFrames()
{
   if (mpController.get() != NULL)
   {
      mpController->setCanDropFrames(!mpController->getCanDropFrames());
   }
}

void AnimationTestDlg::destroyAnimations()
{
   if (mpController.get() != NULL)
   {
      // pAnimations is a copy and not a reference because calling destroyAnimation on a referenced vector crashes.
      const vector<Animation*> pAnimations = mpController->getAnimations();
      for (vector<Animation*>::const_iterator iter = pAnimations.begin(); iter != pAnimations.end(); ++iter)
      {
         mpController->destroyAnimation(*iter);
      }

      // Since all animations have been destroyed, reset mAnimationNumber.
      mAnimationNumber = 0;
   }
}
