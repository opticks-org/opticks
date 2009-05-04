/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BACKGROUNDPLUGINWINDOW_H
#define BACKGROUNDPLUGINWINDOW_H

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>

#include "DockWindowAdapter.h"

class PlugIn;
class PlugInCallback;
class Progress;
class QLabel;
class QProgressBar;
class QPushButton;

/**
 *  This is a QListWidgetItem which displays status messages
 *  and a progress bar for PlugIns running in the background.
 */
class BackgroundPluginItem : public QListWidgetItem
{
public:
   BackgroundPluginItem(const QString& strPlugInName, Progress* pProgress, PlugIn* pPlugIn,
      QListWidget* pListWidget = NULL);
   ~BackgroundPluginItem();

   void setData(int role, const QVariant& value);
   void setStatus(QString statusStr);

   Progress* getProgress() const;
   PlugIn* getPlugIn() const;

   void update();
   void finish();
   bool isFinished() const;

private:
   QLabel* mpNameLabel;
   QProgressBar* mpProgressBar;
   QLabel* mpStatusLabel;

   PlugIn* mpPlugIn;
   Progress* mpProgress;
   bool mFinished;
};


/**
 *  BackgroudnPluginWindow
 *  A DockWindow which displays progress and status information
 *  for PlugIns running in the background
 */
class BackgroundPluginWindow : public DockWindowAdapter
{
   Q_OBJECT

public:
   BackgroundPluginWindow(const std::string& id, QWidget* parent = 0);
   ~BackgroundPluginWindow();

public: // API
   /**
    *  addItem
    *  adds a new PlugIn to the list
    *
    *  @param pPlugIn
    *         The plugin to add.
    *
    *  @param pProgress
    *         This is the Progress object associated with
    *         the PlugIn. The progress bar and status message
    *         are updated when this Progress sends a notify(SIGNAL_NAME(Subject, Modified))
    *         This should be a thread-safe Progress.
    *
    *  @return The BackgroundPluginItem created.
    */
   BackgroundPluginItem* addItem(PlugIn* pPlugIn, Progress* pProgress);

   /**
    *  getItem
    *  locates the BackgroundPluginItem associated with a Progress object
    *
    *  @param pProgress
    *         The Progress object to search for.
    *
    *  @return The BackgroundPluginItem which is associated with pProgress
    *          or NULL if there is no such BackgroundPluginItem
    */
   BackgroundPluginItem* getItem(Progress* pProgress);

   /**
    *  Receive updates from other objects.
    *  This is a thread safe function.
    *  Therefore, do NOT make any direct changes
    *  to the Qt elements. Typically, this function
    *  injects an event into the Qt event
    *  loop which is a thread safe Qt operation.
    *  This event performs the necessary processing.
    *
    *  @param subject
    *         The subject sending the notify()
    *
    *  @param signal
    *         The update type
    *
    *  @param v
    *         An argument for the update
    */
   void progressChanged(Subject &subject, const std::string &signal, const boost::any &v);

   /**
    *  Adds a new PlugInCallback to the event queue for cleanup in the main thread
    *
    *  @param callback
    *         The PlugInCallback to cleanup
    *
    *  @return TRUE for success
    *          FALSE otherwise
    */
   bool addCallback(PlugInCallback* callback);

public slots:
   /**
    *  changeSelection
    *  Called when the selected item's are changed.
    */
   void changeSelection();

   /**
    *  dismissClicked
    *  Called when the Dismiss button is clicked. This function dismisses finised PlugIns
    */
   void dismissClicked();

   /**
    *  abortClicked
    *  Called when the Abort button is clicked. This function aborts PlugIns.
    */
   void abortClicked();

protected: // custom events
   static const int BPW_PMODIFIED;
   static const int BPW_CALLBACK;

   class ModifiedEvent : public QEvent
   {
   public:
      ModifiedEvent(Progress* pProgress) :
         QEvent(QEvent::Type(BPW_PMODIFIED)),
         mpProgress(pProgress)
      {
      }

      Progress* getProgress() const
      {
         return mpProgress;
      }

   private:
      Progress* mpProgress;
   };

   class CallbackEvent : public QEvent
   {
   public:
      CallbackEvent(PlugInCallback* pCallback) :
         QEvent(QEvent::Type(BPW_CALLBACK)),
         mpCallback(pCallback)
      {
      }

      PlugInCallback* getCallback() const
      {
         return mpCallback;
      }

   private:
      PlugInCallback* mpCallback;
   };

   void customEvent(QEvent* pEvent);

private:
   QListWidget* mpStatusList;
   QPushButton* mpDismissButton;
   QPushButton* mpAbortButton;

   friend class ModifiedEvent;
   friend class CallbackEvent;
};

#endif
