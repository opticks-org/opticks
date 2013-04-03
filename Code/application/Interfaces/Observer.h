/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __OBSERVER_H
#define __OBSERVER_H

#include <string>

class Subject;
class Slot;

/**
 *  Base class for objects to receive notification of attachment and
 *  detachment.
 *
 *  If an object needs to be notified when one of its slot methods is attached
 *  or detached from a subject, it needs to derive from %Observer and provide
 *  implementations of either or both of the attached and detached methods.
 *
 *  @see        Subject
 */
class Observer
{
public:
   /**
    *  Method to call when a subject is attached to an observer.
    *
    *  @param   subject
    *           The subject being attached to.
    *  @param   signal
    *           The signal being attached to.
    *  @param   slot
    *           The slot being attached to.
    */
   virtual void attached(Subject &subject, const std::string &signal, 
      const Slot &slot) { }

   /**
    *  Method to call when a subject is detached from an observer.
    *
    *  @param   subject
    *           The subject being detached from.
    *  @param   signal
    *           The signal being detached from.
    *  @param   slot
    *           The slot being detached from.
    */
   virtual void detached(Subject &subject, const std::string &signal, 
      const Slot &slot) { }

protected:
   /**
    * This should not be deleted directly.  It should be deleted according to
    * the instructions provided for the relevant subclass.
    */
   virtual ~Observer() {}
};

#endif
