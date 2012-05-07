/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Rdf.h"

#include <raptor.h>

#include <stdlib.h>

RdfParser::RdfParser(RdfFormat type) : mFormat(type)
{
}

RdfParser::~RdfParser()
{
}

void RdfParser::handleRaptorError(void *pUserData, raptor_locator *pLocator, const char *pMessage)
{
   if (pUserData == NULL)
   {
      return;
   }
   RdfParser* pParser = reinterpret_cast<RdfParser*>(pUserData);
   pParser->mErrors.push_back(RdfParser::Problem(pMessage, false, raptor_locator_line(pLocator), raptor_locator_column(pLocator)));
}

void RdfParser::handleRaptorWarning(void *pUserData, raptor_locator *pLocator, const char *pMessage)
{
   if (pUserData == NULL)
   {
      return;
   }
   RdfParser* pParser = reinterpret_cast<RdfParser*>(pUserData);
   pParser->mWarnings.push_back(RdfParser::Problem(pMessage, true, raptor_locator_line(pLocator), raptor_locator_column(pLocator)));
}

void RdfParser::parseRaptorTriple(void* pUserData, const raptor_statement* pTriple)
{
   if (pUserData == NULL)
   {
      return;
   }
   Rdf* pRdf = reinterpret_cast<Rdf*>(pUserData);
   char *pSubject = reinterpret_cast<char*>(raptor_statement_part_as_string(pTriple->subject, pTriple->subject_type, NULL, NULL));
   char *pPredicate = reinterpret_cast<char*>(raptor_statement_part_as_string(pTriple->predicate, pTriple->predicate_type, NULL, NULL));
   char *pObject = reinterpret_cast<char*>(raptor_statement_part_as_string(pTriple->object, pTriple->object_type, NULL, NULL));
   pRdf->addTriple(pSubject, pPredicate, pObject);
   raptor_free_memory(pSubject);
   raptor_free_memory(pPredicate);
   raptor_free_memory(pObject);
}

Rdf* RdfParser::parseFile(const std::string& filename)
{
   mWarnings.clear();
   mErrors.clear();
   std::string parserName;
   if (mFormat == RDF_XML)
   {
      parserName = "rdfxml";
   }
   else
   {
      return NULL;
   }
   raptor_init();
   raptor_parser *pParser = raptor_new_parser(parserName.c_str());
   raptor_set_feature(pParser, RAPTOR_FEATURE_NO_NET, 1);

   Rdf* pRdf = new Rdf();
   raptor_set_statement_handler(pParser, pRdf, &RdfParser::parseRaptorTriple);
   raptor_set_error_handler(pParser, this, &RdfParser::handleRaptorError);
   raptor_set_warning_handler(pParser, this, &RdfParser::handleRaptorWarning);

   unsigned char* pUriString = raptor_uri_filename_to_uri_string(filename.c_str());
   raptor_uri *pUri = raptor_new_uri(pUriString);
   raptor_parse_file(pParser, pUri, NULL);

   raptor_free_uri(pUri);
   raptor_free_memory(pUriString);
   raptor_free_parser(pParser);
   raptor_finish();

   if (!mErrors.empty())
   {
      delete pRdf;
      pRdf = NULL;
   }

   return pRdf;
}

Rdf* RdfParser::parseString(const std::string& uri, const std::string& content)
{
   mWarnings.clear();
   mErrors.clear();
   std::string parserName;
   if (mFormat == RDF_XML)
   {
      parserName = "rdfxml";
   }
   else
   {
      return NULL;
   }
   raptor_init();
   raptor_parser *pParser = raptor_new_parser(parserName.c_str());
   raptor_set_feature(pParser, RAPTOR_FEATURE_NO_NET, 1);

   Rdf* pRdf = new Rdf();
   raptor_set_statement_handler(pParser, pRdf, &RdfParser::parseRaptorTriple);
   raptor_set_error_handler(pParser, this, &RdfParser::handleRaptorError);
   raptor_set_warning_handler(pParser, this, &RdfParser::handleRaptorWarning);

   raptor_uri *pUri = raptor_new_uri(reinterpret_cast<const unsigned char*>(uri.c_str()));
   raptor_start_parse(pParser, pUri);
   raptor_parse_chunk(pParser, reinterpret_cast<const unsigned char*>(content.c_str()), content.size(), 1);

   raptor_free_uri(pUri);
   raptor_free_parser(pParser);
   raptor_finish();

   if (!mErrors.empty())
   {
      delete pRdf;
      pRdf = NULL;
   }

   return pRdf;
}

const std::vector<RdfParser::Problem> RdfParser::getParseWarnings() const
{
   return mWarnings;
}

const std::vector<RdfParser::Problem> RdfParser::getParseErrors() const
{
   return mErrors;
}

bool RdfParser::Problem::isWarning()
{
   return mWarning;
}
bool RdfParser::Problem::isError()
{
   return !mWarning;
}

const std::string& RdfParser::Problem::getMessage()
{
   return mMessage;
}

int RdfParser::Problem::getLine()
{
   return mLine;
}

int RdfParser::Problem::getColumn()
{
   return mColumn;
}

RdfParser::Problem::Problem(const std::string& message, bool warning, int line, int column)
: mMessage(message), mWarning(warning), mLine(line), mColumn(column)
{
}

RdfSerializer::RdfSerializer(const Rdf &rdf, RdfFormat type) : mRdf(rdf), mType(type)
{
}

RdfSerializer::~RdfSerializer()
{
}

bool RdfSerializer::serializeToFile(const std::string &filename)
{
   raptor_init();
   raptor_serializer* pSerializer = NULL;
   switch(mType)
   {
   case RDF_XML:
      pSerializer = raptor_new_serializer("rdfxml-abbrev");
      break;
   default:
      return false;
   }
   VERIFY(pSerializer != NULL);
   
   raptor_serialize_start_to_filename(pSerializer, filename.c_str());
   bool rval = serializeAll(pSerializer);
   raptor_serialize_end(pSerializer);
   
   raptor_free_serializer(pSerializer);
   raptor_finish();

   return rval;
}

std::string RdfSerializer::serializeToString(const std::string &uri)
{
   raptor_init();
   raptor_serializer* pSerializer = NULL;
   switch(mType)
   {
   case RDF_XML:
      pSerializer = raptor_new_serializer("rdfxml-abbrev");
      break;
   default:
      return std::string();
   }
   VERIFYRV(pSerializer != NULL, std::string());

   raptor_uri *pUri = raptor_new_uri(reinterpret_cast<const unsigned char*>(uri.c_str()));
   void *pBuf = NULL;
   size_t length = 0;
   raptor_serialize_start_to_string(pSerializer, pUri, &pBuf, &length);
   bool rval = serializeAll(pSerializer);
   raptor_serialize_end(pSerializer);

   raptor_free_uri(pUri);

   raptor_free_serializer(pSerializer);
   raptor_finish();

   if(rval)
   {
      std::string result(reinterpret_cast<char*>(pBuf), length);
      return result;
   }
   return std::string();
}

bool RdfSerializer::serializeAll(raptor_serializer* pSerializer)
{
   std::vector<std::string> subjects = mRdf.getAllSubjects();
   for(std::vector<std::string>::const_iterator subject = subjects.begin(); subject != subjects.end(); ++subject)
   {
      const RdfObject *pSubj = mRdf.getSubject(*subject);
      std::vector<std::string> predicates = pSubj->getAllPredicates();
      for(std::vector<std::string>::const_iterator predicate = predicates.begin(); predicate != predicates.end(); ++predicate)
      {
         std::vector<RdfObject*> objects = pSubj->getObjectsForPredicate(*predicate);
         for(std::vector<RdfObject*>::const_iterator object = objects.begin(); object != objects.end(); ++object)
         {
            raptor_uri *pSubjUri = NULL;
            raptor_statement *pTriple = (raptor_statement*)malloc(sizeof(raptor_statement));
            if(subject->substr(0, 2) == "_:")
            {
               pTriple->subject = subject->c_str();
               pTriple->subject_type = RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
            }
            else
            {
               pSubjUri = raptor_new_uri(reinterpret_cast<const unsigned char*>(subject->c_str()));
               pTriple->subject = reinterpret_cast<void*>(pSubjUri);
               pTriple->subject_type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            }
            raptor_uri *pPredUri = raptor_new_uri(reinterpret_cast<const unsigned char*>(predicate->c_str()));
            pTriple->predicate = reinterpret_cast<void*>(pPredUri);
            pTriple->predicate_type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            std::string objname = (*object)->getObjectName();
            pTriple->object = objname.c_str();
            if(objname.substr(0, 2) == "_:")
            {
               pTriple->object_type = RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
            }
            else
            {
               pTriple->object_type = RAPTOR_IDENTIFIER_TYPE_LITERAL;
            }
            pTriple->object_literal_datatype = NULL;
            pTriple->object_literal_language = reinterpret_cast<const unsigned char*>("en");
            raptor_serialize_statement(pSerializer, pTriple);
            if(pSubjUri != NULL)
            {
               raptor_free_uri(pSubjUri);
            }
            raptor_free_uri(pPredUri);
            free(pTriple);
         }
      }
   }
   return true;
}

Rdf::~Rdf()
{
   std::map<std::string, RdfObject*>::iterator iter;
   for (iter = mSubjects.begin(); iter != mSubjects.end(); ++iter)
   {
      delete iter->second;
   }
}

std::vector<std::string> Rdf::getAllSubjects() const
{
   std::vector<std::string> subjects;
   std::map<std::string, RdfObject*>::const_iterator iter;
   for (iter = mSubjects.begin(); iter != mSubjects.end(); ++iter)
   {
      subjects.push_back(iter->first);
   }
   return subjects;
}

const RdfObject* Rdf::getSubject(const std::string& subjectName) const
{
   std::map<std::string, RdfObject*>::const_iterator iter = mSubjects.find(subjectName);
   if (iter != mSubjects.end())
   {
      return iter->second;
   }
   return NULL;
}

std::vector<std::string> Rdf::getAllObjects(const std::string &subjectName, const std::string &predicateName) const
{
   std::vector<std::string> allObjects;
   const RdfObject *pSubject = getSubject(subjectName);
   if(pSubject != NULL)
   {
      std::vector<RdfObject*> objs = pSubject->getObjectsForPredicate(predicateName);
      for(std::vector<RdfObject*>::const_iterator obj = objs.begin(); obj != objs.end(); ++obj)
      {
         allObjects.push_back((*obj)->getObjectName());
      }
   }
   return allObjects;
}

Rdf::Rdf()
{
}

void Rdf::addTriple(std::string subject, std::string predicate, std::string object)
{
   // strip <> and "" if preset...they are added by the RDF parser but are superfluous
   if(*subject.begin() == '<' && *subject.rbegin() == '>')
   {
      subject = subject.substr(1, subject.size() - 2);
   }
   if(*predicate.begin() == '<' && *predicate.rbegin() == '>')
   {
      predicate = predicate.substr(1, predicate.size() - 2);
   }
   if(*object.begin() == '"' && *object.rbegin() == '"')
   {
      object = object.substr(1, object.size() - 2);
   }
   std::map<std::string, RdfObject*>::iterator iter = mSubjects.find(subject);
   RdfObject* pSubject = NULL;
   if (iter == mSubjects.end())
   {
      pSubject = new RdfObject(subject);
      mSubjects.insert(make_pair(subject, pSubject));
   }
   else
   {
      pSubject = iter->second;
   }
   RdfObject* pObject = NULL;
   iter = mSubjects.find(object);
   if (iter == mSubjects.end())
   {
      pObject = new RdfObject(object);
      mSubjects.insert(make_pair(object, pObject));
   }
   else
   {
      pObject = iter->second;
   }
   pSubject->addPredicate(predicate, pObject);
}

RdfObject::~RdfObject()
{
   // Rdf holds pointers to all Rdf objects so nothing special needs to happen here
}

std::vector<std::string> RdfObject::getAllPredicates() const
{
   std::vector<std::string> predicates;
   std::map<std::string, std::vector<RdfObject*> >::const_iterator iter;
   for (iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      predicates.push_back(iter->first);
   }
   return predicates;
}
 
std::vector<RdfObject*> RdfObject::getObjectsForPredicate(const std::string& predicate) const
{
   std::vector<RdfObject*> objects;
   std::map<std::string, std::vector<RdfObject*> >::const_iterator iter;
   iter = mObjects.find(predicate);
   if (iter != mObjects.end())
   {
      return iter->second;
   }
   else
   {
      return objects;
   }
}

const std::string& RdfObject::getObjectName() const
{
   return mObjectName;
}

RdfObject::RdfObject(const std::string& objectName) : mObjectName(objectName)
{
}

void RdfObject::addPredicate(const std::string& predicate, RdfObject* pObject)
{
   std::map<std::string, std::vector<RdfObject*> >::iterator iter = mObjects.find(predicate);
   if (iter == mObjects.end())
   {
      std::vector<RdfObject*> tempVec;
      tempVec.push_back(pObject);
      mObjects.insert(make_pair(predicate, tempVec));
   }
   else
   {
      iter->second.push_back(pObject);
   }
}
