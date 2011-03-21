/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationServices.h"
#include "AnimationTimingTest.h"
#include "DesktopServices.h"
#include "LayerList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "SpatialDataView.h"

#include <boost/any.hpp>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>
#include <time.h>
#include <vector>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, AnimationTimingTestPlugIn);

AnimationTimingTestPlugIn::AnimationTimingTestPlugIn() :
   mpDialog(NULL)
{
   setName("Animation Timing Test Dialog");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Measures the animation frame rate.");
   setShortDescription("Measures the animation frame rate.");
   setDescriptorId("{65FB5539-B7EB-402a-9B7C-1F19F660F02D}");
   setMenuLocation("[Demo]/Animation Timing Test");
   setProductionStatus(false);
   setWizardSupported(false);
}

AnimationTimingTestPlugIn::~AnimationTimingTestPlugIn()
{
}

bool AnimationTimingTestPlugIn::execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList)
{
   try
   {
      if (mpDialog == NULL)
      {
         Service<DesktopServices> pDesktop;
         mpDialog = new AnimationTimingTestDlg(this, pDesktop->getMainWidget());
      }
   }
   catch (QString &e)
   {
      QMessageBox::critical(NULL, "Timing Test Error", "Unable to initialize the timing test. \n" + e);
      return false;
   }

   if (mpDialog != NULL)
   {
      mpDialog->show();
   }

   return mpDialog != NULL;
}

QWidget* AnimationTimingTestPlugIn::getWidget() const
{
   return mpDialog;
}

namespace
{
   const QString animationStateTag("Animation State: ");
   const QString rasterLayerNameTag("Raster Layer Name: ");
   const QString frameRateTag("Average Frame Rate: ");
}

AnimationTimingTestDlg::AnimationTimingTestDlg(Executable* pPlugIn, QWidget* pParent) :
   QDialog(pParent),
   mpPlugIn(pPlugIn),
   mpController(SIGNAL_NAME(AnimationController, AnimationStateChanged), 
                        Slot(this, &AnimationTimingTestDlg::animationStateChanged)),
   mpRasterLayer(SIGNAL_NAME(RasterLayer, DisplayedBandChanged), 
               Slot(this, &AnimationTimingTestDlg::displayedBandChanged)),
   mpAnimationStateLabel(NULL),
   mpRasterLayerNameLabel(NULL),
   mpFrameRateLabel(NULL),
   mFrameCount(0),
   mUpdatePeriod(5),
   mPrevFrameCount(0)
{
   // Initialization
   setWindowTitle("Animation Timing Test");
   setModal(false);

   mpAnimationStateLabel = new QLabel(animationStateTag + "Stopped", this);
   mpRasterLayerNameLabel = new QLabel(rasterLayerNameTag + "N/A", this);
   mpFrameRateLabel = new QLabel(frameRateTag + "0", this);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpAnimationStateLabel);
   pLayout->addWidget(mpRasterLayerNameLabel);
   pLayout->addWidget(mpFrameRateLabel);

   Service<DesktopServices> pDesktop;
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
   if (pView != NULL)
   {
      AnimationController* pController = NULL;
      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL)
      {
         RasterLayer* pLayer = static_cast<RasterLayer*>(pLayerList->getLayer(RASTER,
            pLayerList->getPrimaryRasterElement()));
         if (pLayer != NULL)
         {
            mpRasterLayerNameLabel->setText(rasterLayerNameTag + QString::fromStdString(pLayer->getName()));
            mpRasterLayer.reset(pLayer);
            Animation* pAnimation = pLayer->getAnimation();
            Service<AnimationServices> pAnimations;
            const vector<AnimationController*>& animationControllers = pAnimations->getAnimationControllers();
            vector<AnimationController*>::const_iterator ppAnimationController;
            for (ppAnimationController = animationControllers.begin();
               ppAnimationController != animationControllers.end();
               ++ppAnimationController)
            {
               const vector<Animation*>& animations = (*ppAnimationController)->getAnimations();
               vector<Animation*>::const_iterator ppAnimation;
               for (ppAnimation = animations.begin(); ppAnimation != animations.end(); ++ppAnimation)
               {
                  if (*ppAnimation == pAnimation)
                  {
                     pController = *ppAnimationController;
                     mpController.reset(*ppAnimationController);
                     animationStateChanged(*(*ppAnimationController), 
                        SIGNAL_NAME(AnimationController, AnimationStateChanged), boost::any());
                     break;
                  }
               }
               if (ppAnimation != animations.end())
               {
                  break;
               }
            }
         }
      }
      if (pController == NULL)
      {
         throw QString("Unable to find an appropriate animation controller.");
      }
   }
   else
   {
      throw QString("There is no current spatial data view.");
   }
}

AnimationTimingTestDlg::~AnimationTimingTestDlg()
{
}

void AnimationTimingTestDlg::displayedBandChanged(Subject& subject, const string &signal, const boost::any &value)
{
   ++mFrameCount;
   if (mFrameCount - mPrevFrameCount >= mUpdatePeriod)
   {
      updateFrameRateLabel();
   }
}

void AnimationTimingTestDlg::animationStateChanged(Subject& subject, const string &signal, const boost::any &value)
{
   AnimationController* pController = dynamic_cast<AnimationController*>(&subject);
   if (pController)
   {
      AnimationState state = pController->getAnimationState();
      if (state == PLAY_FORWARD || state == PLAY_BACKWARD)
      {
         mFrameCount = 0;
         mPrevFrameCount = 0;
         mUpdatePeriod = 5;
         mStartTime = QTime::currentTime();
         updateFrameRateLabel();
         updateStateLabel("Play");
      }
      else if (state == STOP)
      {
         updateStateLabel("Stopped");
      }
      else if (state == PAUSE_FORWARD || state == PAUSE_BACKWARD)
      {
         updateStateLabel("Paused");
      }
   }
}

void AnimationTimingTestDlg::updateFrameRateLabel()
{
   QTime currentTime (QTime::currentTime());

   int diffTime = mStartTime.msecsTo(currentTime);
   if (diffTime == 0)
   {
      mpFrameRateLabel->setText(frameRateTag + "0");
      mUpdatePeriod = 5;
   }
   else
   {
      double frameRate = 1000.0 * mFrameCount / diffTime;
      QString text = frameRateTag + QString::number(frameRate);
      mpFrameRateLabel->setText(text);
      if (frameRate >= 6.0)
      {
         mUpdatePeriod = static_cast<int>(frameRate);
      }
      else
      {
         mUpdatePeriod = 5;
      }
   }
   mPrevFrameCount = mFrameCount;
}

void AnimationTimingTestDlg::updateStateLabel(QString state)
{
   mpAnimationStateLabel->setText(animationStateTag + state);
}

void AnimationTimingTestDlg::closeEvent(QCloseEvent *pEvent)
{
   if (mpPlugIn)
   {
      mpPlugIn->abort();
   }
}
