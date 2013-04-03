/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __TYPAWROB_H
#define __TYPAWROB_H

#include <string>

/**
 *  Base class for objects to store class inheritance hierarchy.
 *
 *  TypeAwareObject is an interface used to provide RTTI-like
 *  capability without having to compile with RTTI overhead. It
 *  provides several methods that must be implemented by all
 *  classes derived from it, even if intermediate parent classes
 *  have already implemented the obligations.
 */
class TypeAwareObject
{
public:
   /** 
    *  Returns a string containing the class name.
    *
    *  Must be implemented by all subclasses.
    *
    *  @return  A string containing the name of the class.
    */
   virtual const std::string& getObjectType() const = 0;

   /**
    *  Compares an incoming argument string against either the object's
    *  true class name or one of its parent (inherited) class names.
    *
    *  @param   className
    *           Name of the class that is to be searched for in the object's
    *           inheritance hierarchy.
    *
    *  @return  True if the object's class, or something it can safely be
    *           cast to, matches the incoming argument.
    */
   virtual bool isKindOf(const std::string& className) const = 0;

protected:
   /**
    * This should not be deleted directly.  It should be deleted according to
    * the instructions provided for the relevant subclass.
    */
   virtual ~TypeAwareObject() {}
};

#endif
