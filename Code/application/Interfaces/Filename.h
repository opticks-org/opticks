/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __FILENAME_H
#define __FILENAME_H

#include "Serializable.h"
#include "DataVariantValidator.h"

#include <string>
#include <vector>

/**
 * Provides capability for filename manipulation
 *
 * Filename provides a platform-independent interface for manipulating
 * operating system file paths and names. It allows developers to avoid
 * conditional compile directives yet still write portable code.
 *
 * @see      FileFinder
 */
class Filename : public Serializable
{
public:
   /**
    *  Set/change the full path specification (directory, title and extension,
    *  if any).
    *
    *  @param   pathAndName
    *           A string with the file path information.
    *
    */
   virtual void setFullPathAndName(const std::string& pathAndName) = 0;

   /**
    *  Get the full path specification (directory, title and extension,
    *  if any).
    *
    *  @return  A string with the file path information.
    */
   virtual std::string getFullPathAndName() const = 0;

   /**
    *  Get the directory specification.The appropriate
    *  separator(s) will be used regardless of the original string, so the
    *  client need not worry about the platform portability.
    *
    *  @return A  string with the file extension.
    */
   virtual std::string getPath() const = 0;

   /**
    *  Get the file name (title and extension, if any).
    *
    *  @return  A string with the file name, absent any path information.
    */
   virtual std::string getFileName() const = 0;

   /**
    *  Get the file title.
    *
    *  @return  A string with the file title.
    */
   virtual std::string getTitle() const = 0;

   /**
    *  Get the extension, if any.
    *
    *  @return  A string with the file extension.
    */
   virtual std::string getExtension() const = 0;

   /**
    *  Checks whether the current filename is actually a directory.
    *
    *  @return  TRUE if the filename is a directory, otherwise FALSE.
    */
   virtual bool isDirectory() const = 0;

   /**
    *  Set the filename from a string.
    *
    *  This convenience function is equivalent to Filename::setFullPathAndName().
    *
    *  @param path
    *         The string path to set.
    *
    *  @return the string passed in as an argument.
    */
   virtual const std::string& operator=(const std::string& path) = 0;

   /**
    *  Equality operator.
    *
    *  Checks if two Filenames have identical full path and name representations.
    *
    *  @param other
    *         The other Filename to compare against.
    *
    *  @return true if the Filename objects do contain equal filenames.
    */
   virtual bool operator==(const Filename& other) const = 0;

   /**
    *  Inequality operator.
    *
    *  Checks if two Filenames do not have identical full path and name representations.
    *
    *  @param other
    *         The other Filename to compare against.
    *
    *  @return true if the Filename objects do not contain equal filenames.
    */
   virtual bool operator!=(const Filename& other) const = 0;

   /**
    *  Obtain a string representation of the Filename.
    *
    *  This convenience function is equivalent to Filename::getFullPathAndName().
    *
    *  @return A string representation of the Filename.
    */
   virtual operator std::string() const = 0;
  
protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~Filename() {}
};

/**
 * \cond INTERNAL
 * These template specialization are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<Filename> {};
template <> class VariantTypeValidator<const Filename> {};
template <> class VariantTypeValidator<std::vector<Filename*> > {};
template <> class VariantTypeValidator<std::vector<const Filename*> > {};
/// \endcond

#endif
