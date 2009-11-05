/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SUPPRESSIBLEMSGDLG_H
#define SUPPRESSIBLEMSGDLG_H

#include <QtGui/QDialog>

#include "TypesFile.h"

class QCheckBox;

/**
 *    This interface class manages and displays a suppressible message box that 
 *    can be disabled by the user.
 *
 *    A suppressible message dialog is a pop-up box that can be used to display a message to the user. 
 *    This type of dialog will display a message with a check box. 
 */
class SuppressibleMsgDlg : public QDialog
{
   Q_OBJECT
public:

   /**
   *   This method creates a message box with a "Don't show this again" 
   *   check box.
   *
   *   @param dialogTitle
   *          The message box title.
   *   @param dialogMsg
   *          The message that will be displayed in the message box.
   *   @param type
   *          The type of message.
   *   @param checkBoxState
   *          The state of the check box.
   *   @param pParent
   *          The parent widget.
   */
   SuppressibleMsgDlg(const std::string& dialogTitle, const std::string& dialogMsg, MessageType type, 
               bool checkBoxState = false, QWidget* pParent = NULL);

   /**
    *  Destroys the message box.
    */
   ~SuppressibleMsgDlg();

   /**
    *  This method will return a bool value indicating whether the dialog will be 
    *  visible to the user.
    *  
    *  @return \c false if the dialog check box is checked. If the check box hasn't been checked,
    *          then return \c true.
    */
   virtual bool getDontShowAgain() const;

   /**
    *  This method will set a bool value that will indicate whether the dialog will be 
    *  visible to the user.
    *
    *  @param bEnable
    *         \c False to uncheck the check box, \c true to check the check box.
    */
   virtual void setDontShowAgain(bool bEnable);

private:
   QCheckBox* mpDontShow;
};

#endif