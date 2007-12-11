/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHFILE_H
#define BATCHFILE_H

#include <string>

/**
 *  This class represents a batch file to be processed within a batch wizard.  This
 *  can be one file within a batch file set.
 *
 *  @see    BatchFileset
 */
class BatchFile
{
public:
   /**
    *  Creates a batch file with no associated filename.
    */
   BatchFile();

   /**
    *  Creates a batch file based on the given filename.
    *
    *  @param   filename
    *           The filename to set in the batch file.
    */
   BatchFile(const std::string& filename);

   /**
    *  Destroys the batch file.
    */
   ~BatchFile();

   /**
    *  Sets the associated filename in the batch file.
    *
    *  @param   filename
    *           The filename to set in the batch file.
    */
   void setFileName(const std::string& filename);

   /**
    *  Returns the associated filename in the batch file.
    *
    *  @return  The associated filename.
    */
   const std::string& getFileName() const;

   /**
    *  Sets the state of the processed flag.
    *
    *  This method sets the state of an internal processing flag.  When executed
    *  in a batch wizard as part of a file set, setting this flag to TRUE will
    *  prevent the file from being used multiple times in the file set.
    *
    *  @param   bUsed
    *           Input TRUE if the file has already been processed, or FALSE to
    *           indicate that the file has not yet been processed.
    */
   void setUsed(bool bUsed = true);

   /**
    *  Queries whether this file has been processed.
    *
    *  When executed in a batch wizard, this flag indicates whether the batch file
    *  has already been processed in the file set.
    *
    *  @return  TRUE if the file has been processed, otherwise FALSE.
    */
   bool isUsed() const;

private:
   std::string mFilename;
   bool mbUsed;
};

#endif
