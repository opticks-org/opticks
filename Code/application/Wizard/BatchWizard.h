/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHWIZARD_H
#define BATCHWIZARD_H

#include "Serializable.h"

#include <string>
#include <vector>

class BatchFileset;
class DataVariant;
class ObjectFactory;
class Value;

class BatchWizard : Serializable
{
public:
   BatchWizard();
   ~BatchWizard();

   void setWizardFilename(const std::string& filename);
   const std::string& getWizardFilename() const;
   void setRepeatFileset(BatchFileset* pFileset);
   bool isRepeating(std::string& repeatName) const;
   void setCleanup(bool bCleanup = true);
   bool doesCleanup() const;

   // File sets
   void addFileset(BatchFileset* pFileset);
   BatchFileset* getFileset(const std::string& filesetName) const;
   const std::vector<BatchFileset*>& getFilesets() const;
   int getNumFilesets() const;
   bool containsFileset(BatchFileset* pFileset) const;
   bool removeFileset(BatchFileset* pFileset);

   // Input values
   Value* setInputValue(const std::string& itemName, const std::string& nodeName, const std::string& nodeType,
      const DataVariant& value);
   const std::vector<Value*>& getInputValues() const;

   // Execution
   void initializeFilesets(ObjectFactory* pObjFact);
   void updateFilesets();
   void getCurrentRepeatFile(std::string& currentFile) const;
   void getCurrentFilesetFile(const std::string& filesetName, std::string& currentFile) const;
   bool isComplete() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   std::string mWizardFilename;
   bool mbClean;

   std::vector<BatchFileset*> mFilesets;
   BatchFileset* mpRepeatFileset;
   std::vector<Value*> mInputValues;
};

#endif
