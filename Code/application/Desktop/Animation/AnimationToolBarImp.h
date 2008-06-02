/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATION_TOOLBAR_IMP_H
#define ANIMATION_TOOLBAR_IMP_H

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QSlider>
#include <QtCore/QString>

#include "ToolBarImp.h"

#include "PixmapGrid.h"
#include "PixmapGridButton.h"
#include "TypesFile.h"

#include <vector>

class AnimationController;
class AnimationCycleButton;

class AnimationToolBarImp : public ToolBarImp
{
   Q_OBJECT

public:
   AnimationToolBarImp(const std::string& id, QWidget* parent = 0);
   ~AnimationToolBarImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void setAnimationController(AnimationController* pController);
   AnimationController* getAnimationController() const;

   void setHideTimestamp(bool hideTimestamp);
   bool getHideTimestamp() const;

protected slots:
   // animation control buttons
   void backward();
   void forward();
   void stop();
   void playPause();
   void stepForward();
   void stepBackward();

   void setCurrentFrame(int frameIndex);
   void sliderActionTriggered(int action);
   void setFrameSpeed();
   void setFrameSpeed(const QString& strSpeed);

   // Methods to update the widgets from the animation controller
   void updateAnimationState(AnimationState state);
   void updateFrameRange();
   void updateCurrentFrame(double frameValue);
   void updateFrameSpeed(double speed);
   void updateAnimationCycle(AnimationCycle cycle);

   void updateAnimationControls();

   void activateSlider();
   void releaseSlider();

   void setCanDropFrames(bool canDropFrames);

protected:
   void removeAnimationController(AnimationController* pController);

private:
   void setPlayButtonState(AnimationState state);

   /**
    * WheelEventSlider is a subclass of QSlider to avoid
    * a bug in QSlider's mouse wheel event handling.
    */
   class WheelEventSlider : public QSlider
   {
   public:
      WheelEventSlider(Qt::Orientation orientation, QWidget *parent = 0);

   protected:
      /**
       * Since QSlider's wheelEvent() method does not
       * call triggerAction, we shall have to do it ourselves.
       */
      void wheelEvent(QWheelEvent *e);
   };

   QAction* mpStopAction;              // button to stop playing movie and return to beginning of movie
   QAction* mpPlayPauseAction;         // pause/play button
   QAction* mpBackwardAction;          // button changes the direction in which the movie is playing to backward
                                       // button also changes the speed if movie already playing backwards
   QAction* mpForwardAction;           // button changes the direction in which the movie is playing to forward
                                       // button also changes the speed if movie already playing backwards

   QComboBox* mpFrameSpeedCombo;       // button changes the speed at which the movie is playing
   QAction* mpStepBackwardAction;      // button steps the movie one frame backward
   QAction* mpStepForwardAction;       // button steps the movie one frame forward
   QSlider* mpFrameSlider;             // slider is used to change the frame the movie is displaying
   QAction* mpDropFramesAction;        // button to determine show if frames can be dropped or not
   AnimationCycleButton* mpCycle;
   QLabel* mpTimestampLabel;

   AnimationController* mpController;       // current animation controller to play attached movies

   AnimationState mPrevAnimationState; // stores the previous animation state to be used after the slider is released
   bool mHideTimestamp;
};

class AnimationCycleGrid : public PixmapGrid
{
   Q_OBJECT

public:
   AnimationCycleGrid(QWidget* pParent);
   void setCurrentValue(AnimationCycle value);
   AnimationCycle getCurrentValue() const;

signals: 
   void valueChanged(AnimationCycle value);

private slots:
   void translateChange(const QString&);
};

class AnimationCycleButton : public PixmapGridButton
{
   Q_OBJECT

public:
   AnimationCycleButton(QWidget* pParent);

   void setCurrentValue(AnimationCycle value);
   AnimationCycle getCurrentValue() const;

signals:
   void valueChanged(AnimationCycle value);
};

#define ANIMATIONTOOLBARADAPTEREXTENSION_CLASSES \
   TOOLBARADAPTEREXTENSION_CLASSES

#define ANIMATIONTOOLBARADAPTER_METHODS(impClass) \
   TOOLBARADAPTER_METHODS(impClass) \
   void setAnimationController(AnimationController* pController) \
   { \
      return impClass::setAnimationController(pController); \
   } \
   AnimationController* getAnimationController() const \
   { \
      return impClass::getAnimationController(); \
   } \
   void setHideTimestamp(bool hideTimestamp) \
   { \
      return impClass::setHideTimestamp(hideTimestamp); \
   } \
   bool getHideTimestamp() const \
   { \
      return impClass::getHideTimestamp(); \
   }

#endif
