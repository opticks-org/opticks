/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODIFIERWIDGET_H
#define MODIFIERWIDGET_H

#include "AppVerify.h"
#include "Modifier.h"

#include <QtGui/QWidget>

/**
 *  Tracks modifications to an widget.
 *
 *  This class stores a modified flag that can be set with setModified() and
 *  queried with isModified().  The modified() signal is emitted whenever
 *  setModified() is called with a value of \c true.  The attachSignal() method
 *  can be called to setup a signal/slot connection where the setModified()
 *  method is called whenever the given signal is emitted.
 *
 *  A custom widget can subclass this class to store a modification state and to
 *  provide a common modified() signal that can be automatically emitted when
 *  child widgets are modified.
 *
 *  @see        Modifier
 */
class ModifierWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates the modifier widget.
    *
    *  The constructor creates the ModifierWidget in an unmodified state (i.e.
    *  isModified() returns \c false).
    */
   ModifierWidget(QWidget* pParent = NULL) :
      QWidget(pParent)
   {
      VERIFYNR(connect(&mModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   }

   /**
    *  Destroys the modifier widget.
    */
   virtual ~ModifierWidget()
   {}

   /**
    *  Attaches a signal to the widget that sets the widget in a modified
    *  state.
    *
    *  This method connects the given signal to the setModified() slot method
    *  to automatically set the widget in a modified state whenever the signal
    *  is emitted.
    *
    *  @param   pObject
    *           The object that emits the signal.
    *  @param   pSignal
    *           The signal to connect to the setModified() slot.
    *
    *  @return  Returns \c true if the signal was successfully connected to the
    *           setModified() slot; otherwise returns \c false.
    *
    *  @see     isModified()
    */
   bool attachSignal(QObject* pObject, const char* pSignal)
   {
      return mModifier.attachSignal(pObject, pSignal);
   }

   /**
    *  Queries whether the widget is in a modified state.
    *
    *  @return  Returns \c true if the widget has been modified as a result of
    *           calling setModified() with a value of \c true.  Returns
    *           \c false if the object has not been modified since creation or
    *           since the last call to setModified() with a value of \c false.
    *
    *  @see     setModified()
    */
   bool isModified() const
   {
      return mModifier.isModified();
   }

public slots:
   /**
    *  Sets whether the widget is in a modified state.
    *
    *  This method can be called directly to set or reset the modified state.
    *  It can also be called as a result of an emitted signal that was connected
    *  by calling attachSignal().
    *
    *  @param   modified
    *           Set this parameter to \c true to indicate that the widget is in
    *           a modified state or \c false to indicate that the widget is no
    *           longer in a modified state.
    *
    *  @see     isModified()
    */
   virtual void setModified(bool modified = true)
   {
      mModifier.setModified(modified);
   }

signals:
   /**
    *  Emitted when the widget has been modified.
    *
    *  This signal is automatically emitted when setModified() is called with a
    *  value of \c true.
    */
   void modified();

private:
   Modifier mModifier;
};

#endif
