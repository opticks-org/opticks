/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ALGORITHMDIALOG_H
#define ALGORITHMDIALOG_H

#include <QtCore/QEventLoop>
#include <QtGui/QDialog>

/**
 *  Modeless dialog support for a plug-in.
 *
 *  The algorithm dialog provides a means for plug-ins to display modeless dialogs where
 *  an event loop can be executed to prevent control from returning to the caller until
 *  the dialog is accepted or rejected.
 *
 *  After calling show(), call enterLoop() to begin the event loop.  The enterLoop()
 *  method will not return until the dialog is accepted or rejected.
 *
 *  WARNING:  If another instance of an algorithm dialog is instantiated while the event
 *  loop of this instance is running, accepting or rejecting this dialog will not cause
 *  enterLoop() to return until control has returned from the other dialog instance.
 */
class AlgorithmDialog : public QDialog
{
   Q_OBJECT

public:
   /**
    *  Creates the algorithm dialog.
    *
    *  By default, the dialog is created as a modeless dialog.
    *
    *  @param   pParent
    *           The parent widget.
    */
   AlgorithmDialog(QWidget* pParent = 0);

   /**
    *  Destroys the algorithm dialog.
    *
    *  If the event loop is running and the dialog has not been accepted or rejected,
    *  the event loop is exited with a QDialog::Rejected value.
    */
   ~AlgorithmDialog();

   /**
    *  Begins the local event loop.
    *
    *  This method begins the local event loop, and the method will not return until
    *  the dialog is accepted or rejected.
    *
    *  @return  QDialog::Accepted is returned if the dialog is accepted either by the
    *           user or programmatically.  QDialog::Rejected is returned if the dialog
    *           is rejected or if the dialog is destroyed without being accepted or
    *           rejected.
    */
   int enterLoop();

public slots:
   /**
    *  Closes the dialog, accepting the changes.
    *
    *  This method accepts the dialog and exits the local event loop, thereby returning
    *  control to the caller of eventLoop().
    *
    *  WARNING:  If overriding this method, the base class must be called to ensure
    *  proper execution of the application.
    */
   void accept();

   /**
    *  Closes the dialog, rejecting the changes.
    *
    *  This method rejects the dialog and exits the local event loop, thereby returning
    *  control to the caller of eventLoop().
    *
    *  WARNING:  If overriding this method, the base class must be called to ensure
    *  proper execution of the application.
    */
   void reject();

private:
   AlgorithmDialog(const AlgorithmDialog& rhs);
   AlgorithmDialog& operator=(const AlgorithmDialog& rhs);
   QEventLoop* mpEventLoop;
};

#endif
