/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _CLASSIFICATION
#define _CLASSIFICATION

#include "DynamicObject.h"

#include <string>

class DateTime;

/**
 *  Classification information for data elements and views.
 *
 *  The Classification serves as an attribute for other data objects. It
 *  captures the security classification data needed to handle and mark
 *  the other data objects. There currently is no straight-forward mechanism
 *  to maintain the classification of the classification itself.
 *
 *  Most of the interface is based upon the NITF 2.1 classification attributes
 *  collection, with some modifications for flexibility or complexity reduction.
 *  Additional attributes can be added using the DynamicObject inherited interfaces.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *     - The following methods are called: setLevel(), setSystem(), setCodewords(),
 *         setFileControl(), setFileReleasing(), setDeclassificationDate(),
 *         setDeclassificationExemption(), setFileDowngrade(), setCountryCode(),
 *         setDowngradeDate(), setDescription(), setAuthority(),
 *         setAuthorityType(), setSecuritySourceDate(), setSecurityControlNumber(),
 *         setFileCopyNumber(), setFileNumberOfCopies(), setFileNumberOfCopies(),
 *         deserialize().
 *     - Everything else documented in DynamicObject.
 *
 *  @see        DynamicObject, DataElement
 */
class Classification : public DynamicObject
{
public:
   /**
    *  Sets all classification fields and attributes to those contained in a
    *  given text string.
    *
    *  %Any additional attributes contained in this classification object will
    *  be lost.
    *
    *  @param   classificationText
    *           The classification text from which to set this object's values.
    *           The string format should be identical to the format generated
    *           by getClassificationText().
    *
    *  @return  Returns \c true if the classification was successfully set from
    *           the given string.  Returns \c false if any of the following
    *           conditions are met:
    *           - \em classificationText is empty.
    *           - One or more field entries specified in \em classificationText
    *             is not contained in the security markings files in the
    *             SupportFiles directory.
    *           - The "REL TO" file releasing is specified without valid
    *             country codes.
    *           .
    *           If \c false is returned, the classification fields and
    *           attributes of this classification object are not changed.
    *
    *  @notify  This method notifies Subject::signalModified() if successful.
    *
    *  @see     setClassification(const Classification*)
    */
   virtual bool setClassification(const std::string& classificationText) = 0;

   /**
    *  Sets all classification fields and attributes to that of another classification
    *  object.
    *
    *  @param   pClassification
    *           The classification object from which to set this object's values.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     setClassification(const std::string&)
    */
   virtual void setClassification(const Classification* pClassification) = 0;

   /**
    *  Returns the classification level.
    *
    *  @return  A string reference with the data's classification level.  Valid values
    *           are T (Top Secret), S (Secret), R (Restricted), C (Confidential), and
    *           U (Unclassified).
    *
    *  @see     hasGreaterLevel()
    */
   virtual const std::string& getLevel() const = 0;

   /**
    *  Sets the classification level for the data represented by this object.
    *
    *  @param   myLevel
    *           This shall contain a valid value representing
    *           the classification level of the entire data. Valid values are
    *           T (Top Secret), S (Secret), R (Restricted), C (Confidential), and
    *           U (Unclassified).
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setLevel(const std::string& myLevel) = 0;

   /**
    *  Compares the level of another classification object with this object.
    *
    *  The order of classification levels is as follows, from greatest to least:
    *  T (Top Secret), S (Secret), R (Restricted), C (Confidential), and
    *  U (Unclassified).
    *
    *  @param   pClassification
    *           The classification object for which to compare against this object.
    *
    *  @return  True is returned if this object has a level that is greater that the given
    *           object.  False is returned if this object has a classification level less
    *           than or equal to the given object.
    */
   virtual bool hasGreaterLevel(const Classification* pClassification) const = 0;

   /**
    *  Access the security classification.  This contains valid values indicating the national or
    *  multinational security system used to classify the file.
    *
    *  @return  A string indicating the national or multinational system used to classify the data.
    */
   virtual const std::string& getSystem() const = 0;

   /**
    *  Set the security classification, indicating the national or
    *  multinational security system used to classify the file.
    *
    *  @param   mySystem
    *           This contains valid values indicating the national or
    *           multinational security system used to classify the file.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setSystem(const std::string& mySystem) = 0;

   /**
    *  Access the security compartments associated with the data.
    *
    *  @return  The security compartments associated with the data.
    */
   virtual const std::string& getCodewords() const = 0;

   /**
    *  Set the security compartments associated with the data.
    *
    *  @param   myCodewords
    *           The security codewords associated with the data. Values include
    *           one or more of the tri/digraphs found in DIAM 65-19. Multiple entries
    *           shall be separated by a single space.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setCodewords(const std::string& myCodewords) = 0;

   /**
    *  Return the additional security control and/or handling instructions.
    *
    *  @return  The security control and/or handling instructions associated with the data.
    */
   virtual const std::string& getFileControl() const = 0;

   /**
    *  Set the additional security control and/or handling instructions.
    *
    *  @param   myFileControl
    *           The security control and/or handling instructions associated with the data.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setFileControl(const std::string& myFileControl) = 0;

   /**
    *  Return a string of country and/or multilateral entity codes to which countries and/or 
    *  multilateral entities the data is authorized for release. Valid items in the list are 
    *  one or more country codes found in FIPS 10-4 and/or codes identifying multilateral 
    *  entries as found in DIAM 65-19.
    *
    *  @return  A string documenting the countries and/or groups to which the data may be released.
    */
   virtual const std::string& getFileReleasing() const = 0;

   /**
    *  Set the string of valid country and/or multilateral entity codes to which countries and/or 
    *  multilateral entities the data is authorized for release. This method should be called after
    *  calling the setCountryCode() method.
    *
    *  @param   myFileReleasing
    *           A string documenting the countries and/or groups to which the data 
    *           may be released. Valid items in the list are one or more country codes
    *           found in FIPS 10-4 and/or codes identifying multilateral entries as
    *           found in DIAM 65-19. If "NOFORN" is specified, then text set in setCountryCode()
    *           is removed from the classification. If "NOFORN" and "REL\\ TO" are specified, then
    *           text set in setCountryCode() and "REL\\ TO" are removed from the classification.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see setCountryCode()
    */
   virtual void setFileReleasing(const std::string& myFileReleasing) = 0;

   /**
    *  Return a string indicating the reason for classification.
    *  Valid values are "A" through "G", as defined by E.O. 12598, Section 1.5 (a) to (g).
    *
    *  @return  A string indicating the reason for classification.
    */
   virtual const std::string& getClassificationReason() const = 0;

   /**
    *  Set a string indicating the reason for classification.
    *  Valid values are "A" through "G", as defined by E.O. 12598, Section 1.5 (a) to (g).
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setClassificationReason(const std::string& myClassificationReason) = 0;

   /**
    *  Return a string indicating how this file should be declassified or downgraded.
    *  DD (Declassify on a specific date)
    *  DE (Declassify upon occurrence of an event)
    *  GD (Downgrade on a specific date)
    *  GE (Downgrade upon occurrence of an event)
    *  O  (OADR)
    *  X  (Exempt)
    *
    *  @return  A string indicating how this file should be declassified or downgraded.
    */
   virtual const std::string& getDeclassificationType() const = 0;

   /**
    *  Set a string indicating how this file should be declassified or downgraded.
    *
    *  @param   myDeclassificationType
    *           A string indicating how this file should be declassified or downgraded.
    *           DD (Declassify on a specific date)
    *           DE (Declassify upon occurrence of an event)
    *           GD (Downgrade on a specific date)
    *           GE (Downgrade upon occurrence of an event)
    *           O  (OADR)
    *           X  (Exempt)
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setDeclassificationType(const std::string& myDeclassificationType) = 0;

   /**
    *  Return the date on which the data is to be declassified. This value is only 
    *  meaningful if the declassification type is DD. Otherwise, NULL is returned.
    *
    *  @return  A pointer to a date object, giving the date (if any) when the data
    *           can be declassified. 
    */
   virtual const DateTime* getDeclassificationDate() const = 0;

   /**
    *  Set the date on which the data is to be declassified.
    *
    *  @param   myDeclassificationDate
    *           A pointer to a date object, giving the date (if any) when the data
    *           can be declassified. Passing NULL means the data is not to be automatically
    *           declassified on any given date.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setDeclassificationDate(const DateTime* myDeclassificationDate) = 0;

   /**
    *  Returns the reason the data is exempt from automatic declassification. Valid values
    *  are X1 through X8 and X251 through X259. X1 through X8 correspond to the 
    *  declassification exemptions found in DOD 5200.1-R, paragraphs 4-202b(1) through (8)
    *  for material emempt from the 10-year rule. X251 through X259 correspond to the
    *  declassification exemptions found in DOD 5200.1-R, paragraphs 4-301a(1) through
    *  (9) for permanently valuable material exempt from the 25-year declassification
    *  system. If this field is all spaces, it implies that a file declassification
    *  exemption does not apply.
    *
    *  @return  The declassification exemption reason, if any.
    */
   virtual const std::string& getDeclassificationExemption() const = 0;

   /**
    *  Set the declassification exemption reason. Valid values
    *  are X1 through X8 and X251 through X259. X1 through X8 correspond to the 
    *  declassification exemptions found in DOD 5200.1-R, paragraphs 4-202b(1) through (8)
    *  for material emempt from the 10-year rule. X251 through X259 correspond to the
    *  declassification exemptions found in DOD 5200.1-R, paragraphs 4-301a(1) through
    *  (9) for permanently valuable material exempt from the 25-year declassification
    *  system. If this field is all spaces, it implies that a file declassification
    *  exemption does not apply.
    *
    *  @param   myDeclassificationExemption
    *           The declassification exemption reason.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setDeclassificationExemption(const std::string& myDeclassificationExemption) = 0;

   /**
    *  Returns the classification level to which the data is to be downgraded.
    *
    *  @return  A string with the classification level to which the data can be
    *           downgraded. Valid values are S (Secret), C (Confidential), and R
    *           (Restricted). If all spaces, then security downgrading does not apply.
    */
   virtual const std::string& getFileDowngrade() const = 0;

   /**
    *  Set the classification level to which the data is to be downgraded.
    *
    *  @param   myFileDowngrade
    *           A string with the classification level to which the data can be
    *           downgraded. Valid values are S (Secret), C (Confidential), and R
    *           (Restricted). If all spaces, then security downgrading does not apply.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setFileDowngrade(const std::string& myFileDowngrade) = 0;

   /**
    *  Return the country code. Used in conjunction with "REL TO" under file 
    *  releasing attributes.
    *
    * @return  Returns the list of country codes that will be shown in the 
    *            "REL TO" section of the classification text
    *
    * @see  getClassificationText()
    */
   virtual const std::string& getCountryCode() const = 0;

   /**
    *  Set the country code. Used in conjunction with "REL TO" under file releasing attributes.
    *  This method should be called prior to calling the setFileReleasing() method.
    *
    *  @param   myCountryCode
    *           The country code. Each country code should be separated by a space. If the country code
    *           is not empty and does not contain "USA", then "USA" will be prepended to the country code.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see setFileReleasing()
    */
   virtual void setCountryCode(const std::string& myCountryCode) = 0;

   /**
    *  Return the date on which the data is to be downgraded, if applicable. If not,
    *  then NULL is returned.
    *
    *  @return  Pointer to a DateTime interface object giving the downgrade date for the data.
    */
   virtual const DateTime* getDowngradeDate() const = 0;

   /**
    *  Set the date on which the data is to be downgraded, if applicable. If not,
    *  then NULL should be used.
    *
    *  @param   myDowngradeDate
    *           Pointer to a DateTime interface object giving the downgrade date for the data.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setDowngradeDate(const DateTime* myDowngradeDate) = 0;

   /**
    *  Return a description regarding any classification issues. This includes additional
    *  information about identification of a declassification or downgrading event or
    *  identifying multiple classification sources and/or other special handling rules.
    *
    *  @return  A free-form string with any additional classification description.
    */
   virtual const std::string& getDescription() const = 0;

   /**
    *  Set the free-form text description for the classification.
    *
    *  @param   myDescription
    *           Description regarding any classification issues. This includes additional
    *           information about identification of a declassification or downgrading event
    *           or identifying multiple classification sources and/or other special handling
    *           rules.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setDescription(const std::string& myDescription) = 0;

   /**
    *  Access the classification authority for the file dependent upon the value in 
    *  the Authority Type. Values are free-form text which should contain the following
    *  information: original classification authority name and position or personal
    *  identifier if the value in Authority Type is O; title of the document or
    *  security classification guide used to classify the file if the value in Authority
    *  Type is D; and Derive-Multiple if the file classification was derived from
    *  multiple sources. In the latter case, the file originator will maintain a record
    *  of the source used in accordance with existing security directives. One of the 
    *  multiple sources may also be identified in File Classification Text if desired.
    *
    *  @return  A string identifying the classification authority.
    */
   virtual const std::string& getAuthority() const = 0;

   /**
    *  Set the classification authority for the file dependent upon the value in 
    *  the Authority Type. Values are free-form text which should contain the following
    *  information: original classification authority name and position or personal
    *  identifier if the value in Authority Type is O; title of the document or
    *  security classification guide used to classify the file if the value in Authority
    *  Type is D; and Derive-Multiple if the file classification was derived from
    *  multiple sources. In the latter case, the file originator will maintain a record
    *  of the source used in accordance with existing security directives. One of the 
    *  multiple sources may also be identified in Description if desired.
    *
    *  @param   myAuthority
    *           A string giving the authorities, if any.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setAuthority(const std::string& myAuthority) = 0;

   /**
    *  Return the indication of the type of authority used to classify the file.
    *  O (original classification authority), D (derivative from a single source),
    *  and M (derivative from multiple sources).
    *
    *  @return  A string containing the code for the authority type.
    */
   virtual const std::string& getAuthorityType() const = 0;

   /**
    *  Set the code for the type of authority used to classify the file.
    *
    *  @param   myAuthorityType
    *           Indication of the type of authority used to classify the file.
    *           O(original classification authority), D(derivative from a single source),
    *           and M(derivative from multivple sources)
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setAuthorityType(const std::string& myAuthorityType) = 0;

   /**
    *  Return the date of the source used to derive the classification of the data.
    *
    *  @return  The date of the source used to derive the classification of the data.
    *           In the case of multiple sources, the date of the most recent source is
    *           used.
    */
   virtual const DateTime* getSecuritySourceDate() const = 0;

   /**
    *  Set the date of the source used to derive the classification of the data.
    *  In the case of multiple sources, the date of the most recent source should
    *  be used.
    *
    *  @param   mySecuritySourceDate
    *           The date of the source used to derive the classification of the data.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setSecuritySourceDate(const DateTime* mySecuritySourceDate) = 0;

   /**
    *  Return the security control number associated with the data.
    *
    *  @return  The security control number associated with the data. This
    *           value shall be accordance with the regulations governing the
    *           appropriate security channel(s).
    */
   virtual const std::string& getSecurityControlNumber() const = 0;

   /**
    *  Set the security control number associated with the data.
    *
    *  @param   mySecurityControlNumber
    *           The security control number associated with the data. This
    *           value shall be accordance with the regulations governing the
    *           appropriate security channel(s).
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setSecurityControlNumber(const std::string& mySecurityControlNumber) = 0;

   /**
    *  Return the file copy number associated with the data.
    *
    *  @return  The file copy number associated with the data. If zero, then
    *           the data is not subject to file copy tracking.
    */
   virtual const std::string& getFileCopyNumber() const = 0;

   /**
    *  Set the file copy number associated with the data.
    *
    *  @param   myFileCopyNumber
    *           The file copy number associated with the data. If zero, then
    *           the data is not subject to file copy tracking.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setFileCopyNumber(const std::string& myFileCopyNumber) = 0;

   /**
    *  Return the number of copies associated with the data.
    *
    *  @return  The number of copiesassociated with the data. If zero, then
    *           the data is not subject to file copy tracking.
    */
   virtual const std::string& getFileNumberOfCopies() const = 0;

   /**
    *  Return the number of copies associated with the data.
    *
    *  @param   myFileNumberOfCopies
    *           The number of copies associated with the data. If zero, then
    *           the data is not subject to file copy tracking.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setFileNumberOfCopies(const std::string& myFileNumberOfCopies) = 0;

   /**
    *  Retrieves a text string containing the classification settings.
    *
    *  @param   classificationText
    *           The string to contain the classification text.  %Any text
    *           existing in the string is erased.  The string is formatted as
    *           follows:
    *           - The classification level is the first field in the string.
    *           - The "//" delimiter separates fields.
    *           - The "/"  delimiter separates entries within a single field.
    *           - If applicable, the "REL TO" file releasing is followed by a
    *             space and then the country codes.
    *           - The ", " delimiter separates country codes.
    *           - If applicable, the declassification date is in "%Y%m%d"
    *             format, as used by DateTime::getFormattedUtc().
    */
   virtual void getClassificationText(std::string& classificationText) const = 0;

   /**
    *  Compares all values and attribues in this Classification object with
    *  those of another Classification object.
    *
    *  This method compares both Classification field values and DynamicObject
    *  attributes.
    *
    *  @param   pClassification
    *           The Classification object with which to compare the values and
    *           attributes in this Classification object.  This method does
    *           nothing and returns \c false if \c NULL is passed in.
    *
    *  @return  Returns \c true if all values and attributes in
    *           \em pClassification are the same as the values and attributes in
    *           this Classification object; otherwise returns \c false.
    *
    *  @see     DynamicObject::compare()
    */
   virtual bool compare(const Classification* pClassification) const = 0;

   /**
    *  Queries whether the classification is valid.
    *
    *  Currently this method only ensures that a TS classification has codewords.
    *
    *  @param   errorMessage
    *           A string which will be populated with the reason why the classification
    *           is invalid.  This string is presentable to the user and will be left
    *           unchanged if the classification is valid.
    *
    *  @return  True if the classification is valid, otherwise false.
    */
   virtual bool isValid(std::string& errorMessage) const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~Classification() {}
};

#endif   // _CLASSIFICATION
