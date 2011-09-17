/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNALBLOCKER_H
#define SIGNALBLOCKER_H

#include "SafePtr.h"
#include "Subject.h"

/**
 * SignalEnabler is an RAII class for enabling/disabling signals on a Subject,
 * guaranteeing that the previous state is restored when the SignalEnabler
 * goes out of scope.
 *
 * @warning  Disabling signals can be very dangerous as it hides details from
 *           any attached slots which may depend on being notified to
 *           function properly.
 *
 * @see Subject, Subject::signalsEnabled
 */
class SignalEnabler
{
public:
   /**
    * Creates the RAII object and enables or disables signals on the specified
    * Subject.
    *
    * @param subject
    *         The Subject to enable/disable
    *
    * @param enable
    *         The new enabled state
    *
    * @warning  Disabling signals can be very dangerous as it hides details from
    *           any attached slots which may depend on being notified to
    *           function properly.
    */
   SignalEnabler(Subject& subject, bool enable) :
      mpSubject(&subject),
      mWereEnabled(mpSubject->signalsEnabled())
   {
      mpSubject->enableSignals(enable);
   }

   /**
    * Creates the RAII object, leaving the enabled state unchanged.
    *
    * @param subject
    *         The Subject to restore upon destruction of the SignalEnabler.
    */
   explicit SignalEnabler(Subject& subject) :
      mpSubject(&subject),
      mWereEnabled(mpSubject->signalsEnabled())
   {
   }

   /**
    * Destroys the SignalEnabler, restoring the enabled state of the Subject to 
    * what it was when the SignalEnabler was created.
    */
   ~SignalEnabler()
   {
      if (mpSubject.get() != NULL)
      {
         mpSubject->enableSignals(mWereEnabled);
      }
   }

   /**
    * Returns the enabled state the Subject had when the SignalEnabler was
    * created.
    */
   bool wereEnabled() const
   {
      return mWereEnabled;
   }

private:
   SignalEnabler& operator=(const SignalEnabler&); // prevents assignment
   SignalEnabler(const SignalEnabler&); // prevents copying

   SafePtr<Subject> mpSubject;
   bool mWereEnabled;
};

/**
 * Blocks signals on the given Subject.  The given Subject will have it's
 * signals blocked when SignalBlocker is constructed.  When SignalBlocker is
 * destructed, the state of signal blocking will return to the state it had prior
 * to the construction of SignalBlocker.
 *
 * A convenience class. The same as <code>SignalEnabler(subject, false)</code>.
 *
 * @warning  Disabling signals can be very dangerous as it hides details from
 *           any attached slots which may be depending on being notified to
 *           function properly.
 *
 * @see SignalEnabler, Subject, Subject::signalsEnabled
 */
class SignalBlocker : SignalEnabler
{
public:
   /**
    * Creates the RAII object. Disables signals on the specified Subject.
    *
    * @param subject
    *         The Subject to disable
    *
    * @warning  Disabling signals can be very dangerous as it hides details from
    *           any attached slots which may depend on being notified to
    *           function properly.
    */
   explicit SignalBlocker(Subject& subject) :
      SignalEnabler(subject, false)
   {
   }
};

#endif