/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "ClassificationImp.h"
#include "Classification.h"
#include "AppVerify.h"
#include "DateTimeImp.h"
#include "StringUtilities.h"
#include "UtilityServicesImp.h"

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

static string trimString(string text)
{
   unsigned int i, j;
   const char *pText = text.c_str();

   if (text.size() == 0)
   {
      return text;
   }

   for (i = 0; i < text.size(); i++)
   {
      if (!isspace(pText[i]))
      {
         break;
      }
   }

   for (j = text.size() - 1; j > 0; j--)
   {
      if (!isspace(pText[j]))
      {
         break;
      }
   }

   return text.substr(i, j + 1);
}

/**
 * Expands escaped whitespace and fills nonescaped whitespace with the contents
 * of the fill string.
 *
 * @param text
 *        String to expand and fill
 * @param fill
 *        String to fill unescaped spaces with
 * @return The expanded and filled string
 */
static string expandAndFill(string text, string fill)
{
   string retval;
   bool escaped = false;
   bool inWhite = false;
   for (string::size_type pos = 0; pos < text.size(); ++pos)
   {
      char ch = text[pos];
      if (!escaped)
      {
         if (isspace(ch))
         {
            if (inWhite == false)
            {
               inWhite = true;
               retval += fill;
            }
         }
         else
         {
            inWhite = false;
         }

         if (ch == '\\')
         {
            escaped = true;
         }
         else if (inWhite == false)
         {
            retval.push_back(ch);
         }
      }
      else
      {
         escaped = false;
         retval.push_back(ch);
      }
   }
   return retval;
}

ClassificationImp::ClassificationImp() : mDeclassificationDate(new DateTimeImp()),
   mDowngradeDate(new DateTimeImp()), mSecuritySourceDate(new DateTimeImp()),
   mCodewordsDefaulted(false)
{
   setLevel(string());
}

ClassificationImp::~ClassificationImp()
{
   delete mDeclassificationDate;
   delete mDowngradeDate;
   delete mSecuritySourceDate;
}

ClassificationImp& ClassificationImp::operator =(const ClassificationImp& rhs)
{
   if (this != &rhs)
   {
      DynamicObjectImp::operator =(rhs);

      mLevel = rhs.mLevel.c_str();
      mSystem = rhs.mSystem.c_str();
      mCodewords = rhs.mCodewords.c_str();
      mFileControl = rhs.mFileControl.c_str();
      mFileReleasing = rhs.mFileReleasing.c_str();
      mClassificationReason = rhs.mClassificationReason.c_str();
      mDeclassificationType = rhs.mDeclassificationType.c_str();
      *mDeclassificationDate = *(rhs.mDeclassificationDate);
      mDeclassificationExemption = rhs.mDeclassificationExemption.c_str();
      mFileDowngrade = rhs.mFileDowngrade.c_str();
      mCountryCode = rhs.mCountryCode.c_str();
      *mDowngradeDate = *(rhs.mDowngradeDate);
      mDescription = rhs.mDescription.c_str();
      mAuthority = rhs.mAuthority.c_str();
      mAuthorityType = rhs.mAuthorityType.c_str();
      *mSecuritySourceDate = *(rhs.mSecuritySourceDate);
      mSecurityControlNumber = rhs.mSecurityControlNumber.c_str();
      mFileCopyNumber = rhs.mFileCopyNumber.c_str();
      mFileNumberOfCopies = rhs.mFileNumberOfCopies.c_str();
      mCodewordsDefaulted = rhs.mCodewordsDefaulted;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& ClassificationImp::getLevel() const 
{
  return mLevel;
}

void ClassificationImp::setLevel(const string& myLevel)
{
   if (mCodewordsDefaulted == true)
   {
      mCodewords.clear();
      mCodewordsDefaulted = false;
   }
   if (myLevel.empty() == true)
   {
      mLevel.clear();
      Service<UtilityServices> pUtil;
      string defaultLevelAndCodewords = pUtil->getDefaultClassification();
      if (!defaultLevelAndCodewords.empty())
      {
         mLevel = defaultLevelAndCodewords[0];
         string::size_type slashpos = defaultLevelAndCodewords.find("//");
         if (slashpos != string::npos)
         {
            mCodewords = defaultLevelAndCodewords.substr(slashpos+2);
            std::replace(mCodewords.begin(), mCodewords.end(), '/', ' ');
            mCodewords = trimString(mCodewords);
            mCodewordsDefaulted = true;
         }
      }
      if (mLevel.empty() == true)
      {
         mLevel = "T";
         mCodewords = "SI TK";
         mCodewordsDefaulted = true;
      }
   }
   else
   {
      mLevel = "" + myLevel;
   }
   mLevel = trimString(mLevel);

   // Store level as one character
   if (mLevel == "UNCLASSIFIED")
   {
      mLevel = "U";
   }
   else if (mLevel == "CONFIDENTIAL")
   {
      mLevel = "C";
   }
   else if (mLevel == "RESTRICTED")
   {
      mLevel = "R";
   }
   else if (mLevel == "SECRET")
   {
      mLevel = "S";
   }
   else if (mLevel == "TOP SECRET")
   {
      mLevel = "T";
   }

   if ((mLevel != "U") && (mLevel != "C") && (mLevel != "R") &&
      (mLevel != "S") && (mLevel != "T"))
   {
      setLevel(string());
      return;
   }

   if (mLevel == "U")
   {
      mSystem.clear();
      mCodewords.clear();
      mFileControl.clear();
      mFileReleasing.clear();
      mClassificationReason.clear();
      mDeclassificationType.clear();
      *mDeclassificationDate = DateTimeImp();
      mDeclassificationExemption.clear();
      mFileDowngrade.clear();
      mCountryCode.clear();
      *mDowngradeDate = DateTimeImp();
      mDescription.clear();
      mAuthority.clear();
      mAuthorityType.clear();
      *mSecuritySourceDate = DateTimeImp();
      mSecurityControlNumber.clear();
      mFileCopyNumber.clear();
      mFileNumberOfCopies.clear();
      mCodewordsDefaulted = false;
   }

   notify(SIGNAL_NAME(Subject, Modified));
}

bool ClassificationImp::hasGreaterLevel(const Classification* pClassification) const
{
   if (pClassification == NULL)
   {
      return true;
   }

   char* levels[] = {"U", "R", "C", "S", "T"};
   const unsigned int numLevels = sizeof(levels) / sizeof(levels[0]);
   const string& compareLevel = pClassification->getLevel();

   int iThisLevel = -1;
   int iCompareLevel = -1;

   for (unsigned int i = 0; i < numLevels; i++)
   {
      if (mLevel == levels[i])
      {
         iThisLevel = i;
      }

      if (compareLevel == levels[i])
      {
         iCompareLevel = i;
      }
   }

   return (iThisLevel > iCompareLevel);
}

const string& ClassificationImp::getSystem() const
{
   return mSystem;
}

void ClassificationImp::setSystem(const string& mySystem)
{
   if (mySystem != mSystem)
   {
      mSystem = "" + mySystem;
      mSystem = trimString(mSystem);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getCodewords() const
{
   return mCodewords;
}

void ClassificationImp::setCodewords(const string& myCodewords)
{
   mCodewordsDefaulted = false;
   if (myCodewords != mCodewords)
   {
      mCodewords = "" + myCodewords;
      mCodewords = trimString(mCodewords);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getFileControl() const
{
   return mFileControl;
}

void ClassificationImp::setFileControl(const string& myFileControl)
{
   if (myFileControl != mFileControl)
   {
      mFileControl = "" + myFileControl;
      mFileControl = trimString(mFileControl);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getFileReleasing() const
{
   return mFileReleasing;
}

void ClassificationImp::setFileReleasing(const string& myFileReleasing)
{
   if (myFileReleasing != mFileReleasing)
   {
      string fileReleasing = myFileReleasing;
      string relTo("REL\\ TO");
      if (fileReleasing.find("NOFORN") != string::npos)
      {
         int relToPos = fileReleasing.find(relTo);
         if (relToPos != string::npos) // can't have both NOFORN & REL TO
         {
            int len = relTo.length();
            if (relToPos != 0)
            {
               --relToPos;
               ++len;
            }
            fileReleasing.erase(relToPos, len);
         }
      }
      if (fileReleasing.find(relTo) == string::npos)
      {
         mCountryCode.clear();
      }
      mFileReleasing = "" + fileReleasing;
      mFileReleasing = trimString(mFileReleasing);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getClassificationReason() const
{
   return mClassificationReason;
}

void ClassificationImp::setClassificationReason(const string& myClassificationReason)
{
   if (myClassificationReason != mClassificationReason)
   {
      mClassificationReason = "" + myClassificationReason;
      mClassificationReason = trimString(mClassificationReason);
      mClassificationReason.resize(1);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getDeclassificationType() const
{
   return mDeclassificationType;
}

void ClassificationImp::setDeclassificationType(const string& myDeclassificationType)
{
   if (myDeclassificationType != mDeclassificationType)
   {
      mDeclassificationType = "" + myDeclassificationType;
      mDeclassificationType = trimString(mDeclassificationType);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const DateTime* ClassificationImp::getDeclassificationDate() const
{
   return getDate(mDeclassificationDate);
}

void ClassificationImp::setDeclassificationDate(const DateTime* myDeclassificationDate)
{
   setDate(mDeclassificationDate, myDeclassificationDate);
}

const string& ClassificationImp::getDeclassificationExemption() const
{
   return mDeclassificationExemption;
}

void ClassificationImp::setDeclassificationExemption(const string&
   myDeclassificationExemption)
{
   if (myDeclassificationExemption != mDeclassificationExemption)
   {
      mDeclassificationExemption = "" + myDeclassificationExemption;
      mDeclassificationExemption = trimString(mDeclassificationExemption);
      if (mDeclassificationExemption.empty() == false)
      {
         if (mDeclassificationDate != NULL)
         {
            *mDeclassificationDate = DateTimeImp();
         }
      }
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getFileDowngrade() const
{
   return mFileDowngrade;
}

void ClassificationImp::setFileDowngrade(const string& myFileDowngrade)
{
   if (myFileDowngrade != mFileDowngrade)
   {
      mFileDowngrade = "" + myFileDowngrade;
      mFileDowngrade = trimString(mFileDowngrade);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getCountryCode() const
{
   return mCountryCode;
}

void ClassificationImp::setCountryCode(const string& myCountryCode)
{
   if (myCountryCode != mCountryCode)
   {
      string countryCode = myCountryCode;
      if (countryCode.empty() == false && countryCode.find("USA") == string::npos)
      {
         countryCode = "USA " + countryCode;
      }
      mCountryCode = "" + countryCode;
      mCountryCode = trimString(mCountryCode);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const DateTime* ClassificationImp::getDowngradeDate() const
{
   return getDate(mDowngradeDate);
}

void ClassificationImp::setDowngradeDate(const DateTime* myDowngradeDate)
{
   setDate(mDowngradeDate, myDowngradeDate);
}

const string& ClassificationImp::getDescription() const
{
   return mDescription;
}

void ClassificationImp::setDescription(const string& myDescription)
{
   if (myDescription != mDescription)
   {
      mDescription = "" + myDescription;
      mDescription = trimString(mDescription);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getAuthority() const
{
   return mAuthority;
}

void ClassificationImp::setAuthority(const string& myAuthority)
{
   if (myAuthority != mAuthority)
   {
      mAuthority = "" + myAuthority;
      mAuthority = trimString(mAuthority);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getAuthorityType() const
{
   return mAuthorityType;
}

void ClassificationImp::setAuthorityType(const string& myAuthorityType)
{
   if (myAuthorityType != mAuthorityType)
   {
      mAuthorityType = "" + myAuthorityType;
      mAuthorityType = trimString(mAuthorityType);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const DateTime* ClassificationImp::getSecuritySourceDate() const
{
   return getDate(mSecuritySourceDate);
}
    
void ClassificationImp::setSecuritySourceDate(const DateTime* mySecuritySourceDate)
{
   setDate(mSecuritySourceDate, mySecuritySourceDate);
}

const string& ClassificationImp::getSecurityControlNumber() const
{
   return mSecurityControlNumber;
}

void ClassificationImp::setSecurityControlNumber(const string&
   mySecurityControlNumber)
{
   if (mySecurityControlNumber != mSecurityControlNumber)
   {
      mSecurityControlNumber = "" + mySecurityControlNumber;
      mSecurityControlNumber = trimString(mSecurityControlNumber);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getFileCopyNumber() const
{
   return mFileCopyNumber;
}

void ClassificationImp::setFileCopyNumber(const string& myFileCopyNumber)
{
   if (myFileCopyNumber != mFileCopyNumber)
   {
      mFileCopyNumber = "" + myFileCopyNumber;
      mFileCopyNumber = trimString(mFileCopyNumber);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const string& ClassificationImp::getFileNumberOfCopies() const
{
   return mFileNumberOfCopies;
}

void ClassificationImp::setFileNumberOfCopies(const string& myFileNumberOfCopies)
{
   if (myFileNumberOfCopies != mFileNumberOfCopies)
   {
      mFileNumberOfCopies = "" + myFileNumberOfCopies;
      mFileNumberOfCopies = trimString(mFileNumberOfCopies);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void ClassificationImp::getClassificationText(string& classificationText) const
{
   classificationText = "UNCLASSIFIED";

   string field = "";
   string escaped;
   field = getLevel();
   if (field.empty() == false)
   {
      switch (field[0])
      {
         case 'C':
            classificationText = "CONFIDENTIAL";
            break;

         case 'R':
            classificationText = "RESTRICTED";
            break;

         case 'S':
            classificationText = "SECRET";
            break;

         case 'T':
            classificationText = "TOP SECRET";
            break;

         case 'U':
            classificationText = "UNCLASSIFIED";
            break;

         default:
            break;
      }
   }

   field = getCodewords();
   escaped = expandAndFill(field, "/");
   if (escaped.empty() == false)
   {
      classificationText += "//" + escaped;
   }

   field = getSystem();
   escaped = expandAndFill(field, "/");
   if (escaped.empty() == false)
   {
      classificationText += "//" + escaped;
   }

   field = getFileReleasing();
   escaped = expandAndFill(field, "/");
   string relTo("REL TO");
   string slashRelTo = "/" + relTo;
   int relToLen = relTo.size();
   if (escaped.empty() == false)
   {
      int relToPos = escaped.find(relTo);
      if (relToPos != string::npos)
      {
         field = getCountryCode();
         string countries = expandAndFill(field, ", ");
         if (countries.empty()) // don't display 'REL TO' if no countries specified
         {
            int len = relToLen;
            if (relToPos != 0)
            {
               --relToPos; // strip the preceding '/'
               ++len;
            }
            escaped.erase(relToPos, len);
         }
         else
         {
            countries = " " + countries;
            escaped.insert(relToPos + relToLen, countries);
         }
      }
      if (escaped.empty() == false)
      {
         classificationText += "//" + escaped;
      }
   }

   field = getDeclassificationExemption();
   escaped = expandAndFill(field, "/");
   if (escaped.empty() == false)
   {
      classificationText += "//" + escaped;
   }
   else
   {
      const DateTime *pDeclassDate = getDeclassificationDate();
      if (pDeclassDate != NULL && pDeclassDate->isValid())
      {
         string date = pDeclassDate->getFormattedUtc("%Y%m%d");
         if (date.empty() == false && date != "20000101")
         {
            classificationText += "//" + date;
         }
      }
   }
}

void ClassificationImp::setClassification(const Classification* pClassification)
{
   if (pClassification == NULL)
   {
      return;
   }

   const ClassificationImp* pClassificationImp = dynamic_cast<const ClassificationImp*> (pClassification);
   if (pClassificationImp != NULL)
   {
      *this = *pClassificationImp;
   }
}

bool ClassificationImp::toXml(XMLWriter* xml) const
{
   string dateString;
   string dateFormat = "%Y-%m-%dT%H:%M:%SZ";

   VERIFY(xml != NULL);

   DOMElement *e(xml->addElement("classification"));
   xml->pushAddPoint(e);
   DynamicObjectImp::toXml(xml);
   if(mLevel == "UNCLASSIFIED" ||
      mLevel == "CONFIDENTIAL" ||
      mLevel == "SECRET" ||
      mLevel == "TOP SECRET" ||
      mLevel == "U" ||
      mLevel == "C" ||
      mLevel == "S" ||
      mLevel == "T")
   {
      xml->addAttr("level",mLevel.c_str());
   }
   else
   {
      // unknown level, get default from UtilityServices
      // BTW, this should NEVER happen unless you have
      // as the default is set elsewhere in this class
      // however, it's here just in case
      xml->addAttr("level",string(
                     UtilityServicesImp::instance()->getDefaultClassification()));
   }
   if(mSystem.size() > 0)
      xml->addAttr("system",mSystem.c_str());
   if(mCodewords.size() > 0)
      xml->addAttr("codewords",mCodewords.c_str());
   if(mFileControl.size() > 0)
      xml->addAttr("fileControl",mFileControl.c_str());
   if(mFileReleasing.size() > 0)
      xml->addAttr("fileReleasing",mFileReleasing.c_str());
   if(mClassificationReason.size() > 0)
      xml->addAttr("classificationReason", mClassificationReason.c_str());
   if(mDeclassificationType.size() > 0)
      xml->addAttr("declassificationType", mDeclassificationType.c_str());
   if(mDeclassificationDate->isValid())
   {
      dateString = mDeclassificationDate->getFormattedUtc(dateFormat);
      xml->addAttr("declassificationDate",dateString);
   }
   if(mDeclassificationExemption.size() > 0)
      xml->addAttr("declassificationExemption",mDeclassificationExemption.c_str());
   if(mFileDowngrade.size() > 0)
      xml->addAttr("fileDowngrade",mFileDowngrade.c_str());
   if(mCountryCode.size() > 0)
      xml->addAttr("countryCode",mCountryCode.c_str());
   if(mDowngradeDate->isValid())
   {
      dateString = mDowngradeDate->getFormattedUtc(dateFormat);
      xml->addAttr("downgradeDate",dateString);
   }
   if(mDescription.size() > 0)
      xml->addAttr("description",mDescription.c_str());
   if(mAuthority.size() > 0)
      xml->addAttr("authority",mAuthority.c_str());
   if(mAuthorityType.size() > 0)
      xml->addAttr("authorityType",mAuthorityType.c_str());
   if(mSecuritySourceDate->isValid())
   {
      dateString = mSecuritySourceDate->getFormattedUtc(dateFormat);
      xml->addAttr("securitySourceDate",dateString);
   }
   if(mSecurityControlNumber.size() > 0)
      xml->addAttr("securityControlNumber",mSecurityControlNumber.c_str());
   if(mFileCopyNumber.size() > 0)
      xml->addAttr("fileCopyNumber",mFileCopyNumber.c_str());
   if(mFileNumberOfCopies.size() > 0)
      xml->addAttr("fileNumberOfCopies",mFileNumberOfCopies.c_str());
   xml->popAddPoint();

   return true;
}

// this should be a DOMElement
// if called as a sub-deserialization (from DataElement)
// this is guarenteed. there is usually
// no other time when this is called
bool ClassificationImp::fromXml(DOMNode* document, unsigned int version)
{
   // we don't check the return value
   // the schema will determine if the
   // dynamic object is formatted properly
   // other instantiation issues (fail new, etc.)
   // will either throw an exception or are
   // ignored so we can deserialize as much as possible
   DynamicObjectImp::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement *>(document));
   VERIFY(elmnt != NULL);

   // we don't need to check that level exists
   // as it is required by the language as defined
   // in the .xsd file
   setLevel(A(elmnt->getAttribute(X("level"))));
   if(elmnt->hasAttribute(X("system")))
      mSystem = A(elmnt->getAttribute(X("system")));
   if(elmnt->hasAttribute(X("codewords")))
      mCodewords = A(elmnt->getAttribute(X("codewords")));
   if(elmnt->hasAttribute(X("fileControl")))
      mFileControl = A(elmnt->getAttribute(X("fileControl")));
   if(elmnt->hasAttribute(X("fileReleasing")))
      mFileReleasing = A(elmnt->getAttribute(X("fileReleasing")));
   if(elmnt->hasAttribute(X("classificationReason")))
      mClassificationReason = A(elmnt->getAttribute(X("classificationReason")));
   if(elmnt->hasAttribute(X("declassificationType")))
      mDeclassificationType = A(elmnt->getAttribute(X("declassificationType")));
   if(elmnt->hasAttribute(X("declassificationDate")))
   {
      if(mDeclassificationDate)
         delete mDeclassificationDate;
      mDeclassificationDate = new DateTimeImp(A(elmnt->getAttribute(
                                                          X("declassificationDate"))));
   }
   if(elmnt->hasAttribute(X("declassificationExemption")))
      mDeclassificationExemption = A(elmnt->getAttribute(X("declassificationExemption")));
   if(elmnt->hasAttribute(X("fileDowngrade")))
      mFileDowngrade = A(elmnt->getAttribute(X("fileDowngrade")));
   if(elmnt->hasAttribute(X("countryCode")))
      mCountryCode = A(elmnt->getAttribute(X("countryCode")));
   if(elmnt->hasAttribute(X("downgradeDate")))
   {
      if(mDowngradeDate)
         delete mDowngradeDate;
      mDowngradeDate = new DateTimeImp(A(elmnt->getAttribute(
                                                 X("downgradeDate"))));
   }
   if(elmnt->hasAttribute(X("description")))
      mDescription = A(elmnt->getAttribute(X("description")));
   if(elmnt->hasAttribute(X("authority")))
      mAuthority = A(elmnt->getAttribute(X("authority")));
   if(elmnt->hasAttribute(X("authorityType")))
      mAuthorityType = A(elmnt->getAttribute(X("authorityType")));
   if(elmnt->hasAttribute(X("securitySourceDate")))
   {
      if(mSecuritySourceDate)
         delete mSecuritySourceDate;
      mSecuritySourceDate = new DateTimeImp(A(elmnt->getAttribute(
                                                       X("securitySourceDate"))));
   }
   if(elmnt->hasAttribute(X("securityControlNumber")))
      mSecurityControlNumber = A(elmnt->getAttribute(X("securityControlNumber")));
   if(elmnt->hasAttribute(X("fileCopyNumber")))
      mFileCopyNumber = A(elmnt->getAttribute(X("fileCopyNumber")));
   if(elmnt->hasAttribute(X("fileNumberOfCopies")))
      mFileNumberOfCopies = A(elmnt->getAttribute(X("fileNumberOfCopies")));

   return true;
}

const string& ClassificationImp::getObjectType() const
{
   static string type("ClassificationImp");
   return type;
}

bool ClassificationImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Classification"))
   {
      return true;
   }

   return false;
}

bool ClassificationImp::isValid(string &errorMessage) const
{
   if (mLevel == "T" && mCodewords.empty())
   {
      errorMessage = "The classification cannot be Top Secret without codewords.";
      return false;
   }

   return true;
}

void ClassificationImp::setDate(DateTimeImp* pOldDateTime, const DateTime* pNewDateTime)
{
   if (pNewDateTime != NULL)
   {
      const DateTimeImp* pDateTimeImp = static_cast<const DateTimeImp*>(pNewDateTime);
      if (pOldDateTime != pDateTimeImp)
      {
         *pOldDateTime = *pDateTimeImp;
         notify(SIGNAL_NAME(Subject, Modified));
      }
   }
}

const DateTime* ClassificationImp::getDate(DateTimeImp* pDateTimeImp) const
{
   const DateTime* pDateTime = NULL;
   if (pDateTimeImp != NULL)
   {
      pDateTime = static_cast<const DateTime*>(pDateTimeImp);
   }

   return pDateTime;
}
