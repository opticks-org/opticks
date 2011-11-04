/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONTIMINGTEST_H
#define ANIMATIONTIMINGTEST_H

#include "AnimationController.h"
#include "AttachmentPtr.h"
#include "RasterLayer.h"
#include "ViewerShell.h"

#include <QtGui/QDialog>
#include <QtCore/QTime>

class QLabel;

class AnimationTimingTestPlugIn : public ViewerShell
{
public:
   AnimationTimingTestPlugIn();
   ~AnimationTimingTestPlugIn();

   bool execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList);

protected:
   QWidget* getWidget() const;

private:
   AnimationTimingTestPlugIn(const AnimationTimingTestPlugIn& rhs);
   AnimationTimingTestPlugIn& operator=(const AnimationTimingTestPlugIn& rhs);
   QDialog* mpDialog;
};

class AnimationTimingTestDlg : public QDialog
{
   Q_OBJECT

public:
   AnimationTimingTestDlg(Executable* pPlugIn, QWidget* pParent = 0);
   ~AnimationTimingTestDlg();

   void closeEvent(QCloseEvent *pEvent);

private:
   void displayedBandChanged(Subject& subject, const std::string& signal, const boost::any& value);
   void animationStateChanged(Subject& subject, const std::string& signal, const boost::any& value);
   void updateFrameRateLabel();
   void updateStateLabel(QString state);

private:
   AnimationTimingTestDlg(const AnimationTimingTestDlg& rhs);
   AnimationTimingTestDlg& operator=(const AnimationTimingTestDlg& rhs);
   Executable* mpPlugIn;
   AttachmentPtr<AnimationController> mpController;
   AttachmentPtr<RasterLayer> mpRasterLayer;
   QLabel* mpAnimationStateLabel;
   QLabel* mpRasterLayerNameLabel;
   QLabel* mpFrameRateLabel;
   int mFrameCount;
   QTime mStartTime;
   int mUpdatePeriod;
   int mPrevFrameCount;
};

#endif
