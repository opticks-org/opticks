/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RDF_H__
#define RDF_H__

#include "EnumWrapper.h"
#include <raptor.h>

#include <map>
#include <string>
#include <vector>

class Rdf;

enum RdfFormatEnum { RDF_XML };
typedef EnumWrapper<RdfFormatEnum> RdfFormat;

class RdfParser
{
public:
   RdfParser(RdfFormat type = RDF_XML);
   ~RdfParser();

   Rdf* parseFile(const std::string& filename);
   Rdf* parseString(const std::string& uri, const std::string& content);

   class Problem
   {
   public:
      bool isWarning();
      bool isError();
      const std::string& getMessage();
      int getLine();
      int getColumn();
   private:
      Problem(const std::string& message, bool warning, int line, int column);

      std::string mMessage;
      bool mWarning;
      int mLine;
      int mColumn;

      friend RdfParser;
   };

   const std::vector<Problem> getParseWarnings() const;
   const std::vector<Problem> getParseErrors() const;

private:
   static void parseRaptorTriple(void *pUserData, const raptor_statement *pTriple);
   static void handleRaptorError(void *pUserData, raptor_locator *pLocator, const char *pMessage);
   static void handleRaptorWarning(void *pUserData, raptor_locator *pLocator, const char *pMessage);

   RdfFormat mFormat;
   std::vector<Problem> mWarnings;
   std::vector<Problem> mErrors;
};

class RdfSerializer
{
public:
   RdfSerializer(const Rdf &rdf, RdfFormat type = RDF_XML);
   ~RdfSerializer();

   bool serializeToFile(const std::string &filename);
   std::string serializeToString(const std::string &uri);

private:
   bool serializeAll(raptor_serializer* pSerializer);

   const Rdf &mRdf;
   RdfFormat mType;
};

class RdfObject;

class Rdf
{
public:
   Rdf();
   ~Rdf();

   std::vector<std::string> getAllSubjects() const;
   const RdfObject* getSubject(const std::string& subjectName) const;
   std::vector<std::string> getAllObjects(const std::string &subjectName, const std::string &predicateName) const;
   void addTriple(std::string subject, std::string predicate, std::string object);

private:
   std::map<std::string, RdfObject*> mSubjects; // indexed on subject

   friend RdfParser;
};

class RdfObject
{
public:
   ~RdfObject();
   std::vector<std::string> getAllPredicates() const;
   std::vector<RdfObject*> getObjectsForPredicate(const std::string& predicate) const;
   const std::string& getObjectName() const;
private:
   RdfObject(const std::string& objectName);
   void addPredicate(const std::string& predicate, RdfObject* pObject);
   std::string mObjectName;
   std::map<std::string, std::vector<RdfObject*> > mObjects;

   friend Rdf;
};


#endif