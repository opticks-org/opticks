/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHFILEPARSER_H
#define BATCHFILEPARSER_H

#include <QtXml/QDomDocument>

#include "Value.h"
#include "BatchWizard.h"
#include "BatchFileset.h"

#include <string>
#include <vector>
#include <map>

/**
 *  Batch File Parser
 *
 *  This class parses an XML file and extracts two main sets of tags: <fileset/> and
 *  <wizard/>.  There can be multiple tags with the same name (i.e. multiple wizards
 *  and or filesets).  All file sets are parsed when the XML file is set.  A wizard
 *  is parsed when the read() method is called.
 *
 *  @see    BatchFileset, BatchWizard
 */
class BatchFileParser
{
public:
   /**
    *  Creates a batch file parser.
    */
   BatchFileParser();

   /**
    *  Destroys the batch file parser.
    */
   ~BatchFileParser();

   /**
    *  Assigns an XML file to the parser.
    *
    *  This method checks the given filename to see if it a valid XML file.  If so, all  
    *  file sets in the file are parsed and stored.
    *
    *  @param   filename
    *           The name of the XML file containing the file sets and wizard filenames
    *           and values to execute.
    *
    *  @return  TRUE if the file is a valid XML file and was parsed successfully,
    *           otherwise FALSE.
    */
   bool setFile(const std::string& filename);

   /**
    *  Reads a set of wizard parameters from the batch file.
    *
    *  This method parses the batch file for a wizard and creates a batch wizard object
    *  to contain the values specified in the file.  If the XML file contains multiple
    *  wizards, each subsequent call to this method will return the next wizard in the
    *  batch file.
    *
    *  @return  A pointer to the batch wizard containing the name and value(s) of the
    *           inputs specified in the XML file.  NULL is returned if no filename
    *           has been set, if an error occurs, or if no more wizards are found in
    *           the batch file.
    */
   BatchWizard* read();

   /**
    *  Retrieves a vector of all file sets found in the batch file.
    *
    *  @param   filesets
    *           A vector which will be filled with pointers to the filesets found
    *           in the batch file.  If no file sets were found in the batch file,
    *           the vector is cleared.
    */
   void getFileSets(std::vector<BatchFileset*>& filesets) const;

   /**
    *  Returns the error message last encountered while parsing the XML file.
    *
    *  @return  The last encountered error message.
    */
   const std::string& getError() const;

protected:
   /**
    *  Parses the batch file for elements containing the <fileset .../> tag(s),
    *  and populates the member vector
    *
    *  @return  TRUE if the file was successfully parsed for the file sets,
    *           otherwise FALSE.
    */
   bool parseFilesets();

private:
   QDomDocument* mpDocument;
   unsigned int mCurrentWizard;
   std::string mErrorMessage;
   std::map<std::string, BatchFileset*> mFilesets;
};

#endif
