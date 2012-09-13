/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODIFIER_H
#define MODIFIER_H

#include <QtCore/QObject>

/**
 *  Tracks modifications to an object.
 *
 *  This class stores a modified flag that can be set with setModified() and
 *  queried with isModified().  The modified() signal is emitted whenever
 *  setModified() is called with a value of \c true.  The attachSignal() method
 *  can be called to setup a signal/slot connection where the setModified()
 *  method is called whenever the given signal is emitted.
 *
 *  This class can be used to track modifications in two ways:
 *  -# An object can subclass this class to store a modification state and to
 *     provide a common modified() signal that can be automatically emitted
 *     when other GUI widgets are modified.
 *  -# This class can be created by an object to track modifications of another
 *     object that does not track its own modifications.
 */
class Modifier : public QObject
{
   Q_OBJECT

public:
   /**
    *  Creates the modifier object.
    *
    *  The constructor creates the Modifier object in an unmodified state.
    */
   Modifier() :
      mModified(false)
   {
   }

   /**
    *  Attaches a signal to the object that sets the object in a modified
    *  state.
    *
    *  This method connects the given signal to the setModified() slot method
    *  to automatically set the object in a modified state whenever the signal
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
      return connect(pObject, pSignal, this, SLOT(setModified()));
   }

   /**
    *  Queries whether the object is in a modified state.
    *
    *  @return  Returns \c true if the object has been modified as a result of
    *           calling setModified() with a value of \c true.  Returns
    *           \c false if the object has not been modified since creation or
    *           since the last call to setModified() with a value of \c false.
    *
    *  @see     setModified()
    */
   bool isModified() const
   {
      return mModified;
   }

public slots:
   /**
    *  Sets whether the object is in a modified state.
    *
    *  This method can be called directly to set or reset the modified state.
    *  It can also be called as a result of the signal that is passed into
    *  attachSignal() being emitted.
    *
    *  @param   bModified
    *           Set this parameter to \c true to indicate that the object is in
    *           a modified state or \c false to indicate that the object is no
    *           longer in a modified state.
    *
    *  @see     isModified()
    */
   void setModified(bool bModified = true)
   {
      mModified = bModified;
      if (mModified == true)
      {
         emit modified();
      }
   }

signals:
   /**
    *  Emitted when the object has been modified.
    *
    *  This signal is automatically emitted when setModified() is called with a
    *  value of \c true.
    */
   void modified();

private:
   Modifier(const Modifier& rhs);
   Modifier& operator=(const Modifier& rhs);
   bool mModified;
};

#endif
