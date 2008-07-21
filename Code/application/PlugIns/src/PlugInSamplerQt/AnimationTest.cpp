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
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationFrame.h"
#include "AnimationServices.h"
#include "AnimationTest.h"
#include "AnimationToolBar.h"
#include "DesktopServices.h"
#include "Executable.h"

#include <boost/any.hpp>
#include <math.h>
#include <time.h>
#include <vector>

using namespace std;

//////////////////////////////////////////////////////////////////////////
// AnimationTestPlugIn

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
}

AnimationTestPlugIn::~AnimationTestPlugIn()
{

}

bool AnimationTestPlugIn::execute(PlugInArgList *pInputArgList, PlugInArgList *pOutArgList)
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
   // Initialization
   setWindowTitle("Animation Test");
   setModal(false);
   createController();

   // Animation Controller
   QPushButton* pCreateControllerButton = new QPushButton("Create Controller", this);
   QPushButton* pDestroyControllerButton = new QPushButton("Destroy Controller", this);

   QHBoxLayout* pControllerLayout = new QHBoxLayout;
   pControllerLayout->addWidget(pCreateControllerButton);
   pControllerLayout->addWidget(pDestroyControllerButton);

   QGroupBox* pControllerGroup = new QGroupBox("Animation Controller", this);
   pControllerGroup->setLayout(pControllerLayout);

   // Create Animations
   QLabel* pNumAnimationsLabel = new QLabel("Number of Animations:", this);
   mpNumAnimations = new QSpinBox(this);
   mpNumAnimations->setRange(1, 10000);

   QLabel* pNumFramesLabel = new QLabel("Number of Frames:", this);
   mpNumFrames = new QSpinBox(this);
   mpNumFrames->setRange(1, 10000);

   QLabel* pMinFrameValueLabel = new QLabel("Min Frame Value:", this);
   mpMinFrameValue = new QDoubleSpinBox(this);
   mpMinFrameValue->setDecimals(10);
   mpMinFrameValue->setRange(0, 10000);

   QLabel* pMaxFrameValueLabel = new QLabel("Max Frame Value:", this);
   mpMaxFrameValue = new QDoubleSpinBox(this);
   mpMaxFrameValue->setDecimals(10);
   mpMaxFrameValue->setRange(0, 10000);
   mpMaxFrameValue->setValue(1);

   QPushButton* pCreateAnimationsButton = new QPushButton("Create Animations", this);

   QGridLayout* pCreateAnimationsLayout = new QGridLayout;
   pCreateAnimationsLayout->addWidget(pNumAnimationsLabel, 0, 0);
   pCreateAnimationsLayout->addWidget(mpNumAnimations, 0, 1);
   pCreateAnimationsLayout->addWidget(pNumFramesLabel, 1, 0);
   pCreateAnimationsLayout->addWidget(mpNumFrames, 1, 1);
   pCreateAnimationsLayout->addWidget(pMinFrameValueLabel, 2, 0);
   pCreateAnimationsLayout->addWidget(mpMinFrameValue, 2, 1);
   pCreateAnimationsLayout->addWidget(pMaxFrameValueLabel, 3, 0);
   pCreateAnimationsLayout->addWidget(mpMaxFrameValue, 3, 1);
   pCreateAnimationsLayout->addWidget(pCreateAnimationsButton, 4, 1);

   QGroupBox* pCreateAnimationsGroup = new QGroupBox("Create Animations", this);
   pCreateAnimationsGroup->setLayout(pCreateAnimationsLayout);

   // Existing Animations
   mpAnimationList = new QComboBox;
   QPushButton* pClearAnimationButton = new QPushButton("Clear Animation", this);
   QPushButton* pDestroyAnimationButton = new QPushButton("Destroy Animation", this);
   QPushButton* pViewFramesButton = new QPushButton("View Frames", this);
   QPushButton* pViewAllFramesButton = new QPushButton("View All Frames", this);

   QGridLayout* pExistingAnimationsLayout = new QGridLayout;
   pExistingAnimationsLayout->addWidget(mpAnimationList, 0, 0);
   pExistingAnimationsLayout->addWidget(pClearAnimationButton, 0, 1);
   pExistingAnimationsLayout->addWidget(pDestroyAnimationButton, 1, 1);
   pExistingAnimationsLayout->addWidget(pViewFramesButton, 2, 1);
   pExistingAnimationsLayout->addWidget(pViewAllFramesButton, 3, 1);

   QGroupBox* pExistingAnimationsGroup = new QGroupBox("Existing Animations", this);
   pExistingAnimationsGroup->setLayout(pExistingAnimationsLayout);

   // Misc
   QPushButton* pToggleTimeDisplayButton = new QPushButton("Toggle Time Display", this);
   QPushButton* pDestroyAnimationsButton = new QPushButton("Destroy All Animations", this);
   QPushButton* pCloseButton = new QPushButton("Close");

   QHBoxLayout* pMiscLayout = new QHBoxLayout;
   pMiscLayout->addWidget(pToggleTimeDisplayButton);
   pMiscLayout->addWidget(pDestroyAnimationsButton);
   pMiscLayout->addWidget(pCloseButton);

   QGroupBox* pMiscGroup = new QGroupBox("Miscellaneous", this);
   pMiscGroup->setLayout(pMiscLayout);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(pControllerGroup);
   pLayout->addWidget(pCreateAnimationsGroup);
   pLayout->addWidget(pExistingAnimationsGroup);
   pLayout->addWidget(pMiscGroup);

   // Connections
   VERIFYNRV(connect(pCreateControllerButton, SIGNAL(clicked()), this, SLOT(createController())));
   VERIFYNRV(connect(pCreateAnimationsButton, SIGNAL(clicked()), this, SLOT(createAnimations())));
   VERIFYNRV(connect(pDestroyControllerButton, SIGNAL(clicked()), this, SLOT(destroyController())));
   VERIFYNRV(connect(pClearAnimationButton, SIGNAL(clicked()), this, SLOT(clearAnimation())));
   VERIFYNRV(connect(pDestroyAnimationButton, SIGNAL(clicked()), this, SLOT(destroyAnimation())));
   VERIFYNRV(connect(pViewFramesButton, SIGNAL(clicked()), this, SLOT(viewFrames())));
   VERIFYNRV(connect(pViewAllFramesButton, SIGNAL(clicked()), this, SLOT(viewAllFrames())));
   VERIFYNRV(connect(pToggleTimeDisplayButton, SIGNAL(clicked()), this, SLOT(toggleTimeDisplay())));
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

void AnimationTestDlg::createController()
{
   if (mpController.get() == NULL)
   {
      mAnimationNumber = 0;
      mpController.reset(Service<AnimationServices>()->createAnimationController("AnimationTest", FRAME_TIME));
      VERIFYNRV(mpController.get() != NULL);

      AnimationToolBar* pToolBar = dynamic_cast<AnimationToolBar*>
         (Service<DesktopServices>()->getWindow("Animation", TOOLBAR));
      if (pToolBar != NULL)
      {
         pToolBar->setAnimationController(mpController.get());
      }
   }
}

void AnimationTestDlg::destroyController()
{
   if (mpController.get() != NULL)
   {
      Service<AnimationServices>()->destroyAnimationController(mpController.get());
   }
}

void AnimationTestDlg::createAnimations()
{
   createController();
   const double minValue = mpMinFrameValue->value();
   const double maxValue = mpMaxFrameValue->value();
   const double range = maxValue - minValue;
   if (range <= 0)
   {
      QMessageBox::warning(this, "Error", "Minimum frame value must be less than or equal to maximum frame value.");
      return;
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

         // Get a random value between 0 and 1. Scale the value by range and offset it by minValue.
         double frameTime = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) * range) + minValue;
         frames.push_back(AnimationFrame(animationFrameName.toStdString(), i, frameTime));
         if (j == 0) // insert a duplicate frame
         {
            frames.push_back(AnimationFrame(animationFrameName.toStdString() + "_dup", i, frameTime));
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
      QDialog* pDialog = new QDialog(this);
      QTextEdit* pInfo = new QTextEdit(frameInfo, pDialog);
      pInfo->setReadOnly(true);

      QVBoxLayout* pLayout = new QVBoxLayout(pDialog);
      pLayout->addWidget(pInfo);

      pDialog->setWindowTitle("Animation Frames -- Sorted by Frame Time");
      pDialog->setLayout(pLayout);
      pDialog->resize(350, 350);
      pDialog->exec();
      delete pDialog;
   }
}

void AnimationTestDlg::toggleTimeDisplay()
{
   AnimationToolBar* pToolBar = dynamic_cast<AnimationToolBar*>
      (Service<DesktopServices>()->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      pToolBar->setHideTimestamp(pToolBar->getHideTimestamp() == false);
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
