/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHFILESET_H
#define BATCHFILESET_H

#include "BatchFile.h"
#include "EnumWrapper.h"

#include <string>
#include <vector>
#include <map>

class ObjectFactory;

/**
 *  This class represents a set of files to be used in batch wizard processing.  These
 *  files are selected explicitly or with wildcards by adding requirements.  The batch
 *  wizard uses this class to select files when executing the wizards.
 *
 *  @see    BatchFile
 */
class BatchFileset
{
public:
   /**
    *  Creates an empty file set.
    */
   BatchFileset();

   /**
    *  Creates a file set with initial values.
    *
    *  @param   name
    *           The file set name.
    *  @param   directory
    *           The file set directory.  The directory represents the location
    *           where all file set files are stored.
    *  @param   fileset
    *           The files to include in the file set.
    */
   BatchFileset(const std::string& name, const std::string& directory,
      const std::vector<BatchFile*>& fileset);

   /**
    *  Destroys the file set.
    */
   ~BatchFileset();

   /**
    *  Sets the name of the file set.
    *
    *  @param   filesetName
    *           The file set name.
    */
   void setName(const std::string& filesetName);

   /**
    *  Returns the name of the file set.
    *
    *  @return  The file set name.
    */
   const std::string& getName() const;

   /**
    *  Sets the directory where the file set files are stored.
    *
    *  @param   directory
    *           The file set directory.
    */
   void setDirectory(const std::string& directory);

   /**
    *  Returns the directory of the file set where the file set files are stored.
    *
    *  @return  The file set directory.
    */
   const std::string& getDirectory() const;

   /**
    *  Adds a single batch file to the file set.
    *
    *  This method creates a batch file based on the given filename and adds
    *  it to the file set.
    *
    *  @param   pBatchFile
    *           The batch file to add to the file set.  Cannot be NULL.
    */
   void addFile(const std::string& filename);

   /**
    *  Adds a single batch file to the file set.
    *
    *  @param   pBatchFile
    *           The batch file to add to the file set.  Cannot be NULL.
    */
   void addFile(BatchFile* pBatchFile);

   /**
    *  Adds multiple batch files to the file set.
    *
    *  This method adds multiple batch files to the file set.  Any existing files
    *  in the file set are replaced.
    *
    *  @param   fileset
    *           The batch files to add to the file set.
    */
   void addFiles(const std::vector<BatchFile*>& fileset);

   /**
    *  Indicates whether files specified by a file set requirement should be
    *  included or excluded from the file set.
    */
   enum FilesetRequirementTypeEnum { INCLUDE = 1, EXCLUDE = 2 };

   /**
    * @EnumWrapper BatchFileset::FilesetRequirementTypeEnum.
    */
   typedef EnumWrapper<FilesetRequirementTypeEnum> FilesetRequirementType;

   /**
    *  Include or exclude multiple files in the fileset.
    * 
    *  This method adds a file set requirement consisting of a wild card specification
    *  of files.  The multiple files specified by the requirement are then added or
    *  removed from the file set based on the requirement type.  This method must be
    *  called before calling the updateFileset() method
    * 
    *  @param   eType
    *           The type siginifies if the requirement is adding or removing
    *           files from the fileset.
    *  @param   requirement
    *           A wild card specification of files.  For example "*.hdr" specifies all 
    *           files ending with a "hdr" extension.
    */
   void addFilesetRequirement(FilesetRequirementType eType, const std::string& requirement);

   /**
    *  Clears the list of file set requirements.
    */
   void clearFilesetRequirements();

   /**
    *  Returns the list of file set requirements.
    * 
    *  This method adds a file set requirement consisting of a wild card specification
    *  of files.  The multiple files specified by the requirement are then added or
    *  removed from the file set based on the requirement type.  This method must be
    *  called before calling the updateFileset() method
    * 
    *  @return  A map of file set requirements.  The first value in the map is the
    *           requirement and the second value is the inclusion string, which can be
    *           either "include" or "exclude".  The return map should not be modified.
    */
   const std::multimap<std::string, std::string>& getFilesetRequirements() const;

   /**
    *  Checks if a file exists in the fileset.
    *
    *  @param   filename
    *           The filename to query in the fileset.
    *
    *  @return  Returns true if the file exists in the fileset, otherwise it returns
    *           false.
    */
   bool doesFileExist(const std::string& filename) const;

   /**
    *  Updates the file set based on the file set requirements
    * 
    *  This method forces an update of the files in the file set.  The method reads the
    *  target directory and excludes and includes files based on the current requirement
    *  information.  All previously existing files in the file set are cleared.
    *
    *  @param   pObjFact
    *           A pointer to the object factory.  This is needed to create a FileFinder
    *           object with which to iterate over all files specified by a wild card
    *           requirement.
    *
    *  @see     FileFinder
    */
   void updateFileset(ObjectFactory* pObjFact);

   /**
    *  Returns the filename of the first file in the file set.
    *
    *  @return  The first file set filename.
    */
   std::string getFirstFile();

   /**
    *  Returns the current filename being used in the file set.
    *
    *  @return  The current file set filename.  If no files have been added to the file
    *           set, or if the end if the file set has neem reached, an empty string is
    *           returned.
    */
   std::string getCurrentFile();

   /**
    *  Returns the next file to use in the file set.
    *
    *  @return  The next file set filename.  If the end if the file set is reached,
    *           an empty string is returned.
    */
   std::string getNextFile();

   /**
    *  Queries whether all files in the file set have been processed.
    *
    *  @return  TRUE if all files have been processed, otherwise false.
    */
   bool isComplete() const;

private:
   std::string mName;
   std::string mDirectory; 
   std::vector<BatchFile*> mFiles;
   std::multimap<std::string, std::string> mFilesetReq;
   BatchFile* mpCurrentFile;
};

#endif
