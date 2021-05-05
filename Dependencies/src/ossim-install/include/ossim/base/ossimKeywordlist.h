//---
//
// License: MIT
//
// Author: Ken Melero
// 
// Description: This class provides capabilities for keywordlists.
//
//---
// $Id$

#ifndef ossimKeywordlist_HEADER
#define ossimKeywordlist_HEADER 1

#include <ossim/base/ossimErrorStatusInterface.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <map>
#include <vector>
#include <algorithm>

static const char DEFAULT_DELIMITER = ':';

class ossimFilename;

/**
 * Represents serializable keyword/value map. The format is
 *
 *   [<prefix>.]<keyword>: value [value ...]
 *
 * The map is not a multimap, i.e., the keywords must be unique. Only the last occurrence of
 * identical keywords will be saved in the map. Methods are provided for reading from and writing
 * to an ascii file. Methods are also provided for merging multiple maps (a.k.a. "lists" or "KWLs")
 * as well as assorted operations for pruning and counting.
 *
 * Disk files representing a KWL can use the C-style "#include <filename>" preprocessor directive,
 * where <filename> specifies another external KWL file that will be merged with the current list.
 * This is convenient for sourcing common settings needed by multiple KWL files. Instead of
 * duplicating all common keywords/value pairs, the various KWL files can all specify, for example,
 *
 *      #include common_prefs.kwl
 *      #include "common config.kwl"
 *
 * The second form with quotes can be used, especially if the filename has spaces.
 */
class OSSIM_DLL ossimKeywordlist : public ossimErrorStatusInterface,
   public ossimReferenced
{
public:

   typedef std::map<std::string, std::string> KeywordMap;

   ossimKeywordlist(const ossimKeywordlist& src);
   ossimKeywordlist(const std::map<std::string, std::string>& keywordMap);
   ossimKeywordlist(char delimiter = DEFAULT_DELIMITER,
                    bool expandEnvVars = false);

   ossimKeywordlist(const char* file,
                    char        delimiter = DEFAULT_DELIMITER,
                    bool        ignoreBinaryChars = false,
                    bool        expandEnvVars = false );

   ossimKeywordlist(const ossimFilename& fileName,
                    char                 delimiter = DEFAULT_DELIMITER,
                    bool                 ignoreBinaryChars = false,
                    bool                 expandEnvVars = false);

   ~ossimKeywordlist();

   static const std::string NULL_KW;

   /*!
    *  Reads file and adds keywords to the KeywordMap.
    *  Returns true if file was parsed, false on error.
    */
   bool addFile(const char* file);

   /*!
    *  Reads file and adds keywords to the KeywordMap.
    *  Returns true if file was parsed, false on error.
    */
   bool addFile(const ossimFilename& file); 

   /*!
    *  Method to change default delimiter.  Handy when parsing
    *  files similar to a ossimKeywordlist.  (DEFAULT = ':')
    */
   void change_delimiter(char del);

   ossimString delimiter_str() const;

   /*!
    * If set to true, then strings found having the format
    * "$(env_var_name)" are expanded in place.
    */
   void setExpandEnvVarsFlag( bool flag );
   /*!
    * Returns the flag that determines whether or not
    * environment variables are expanded.
    */
   bool getExpandEnvVarsFlag( void ) const;

   void add(const char* prefix,
            const ossimKeywordlist& kwl,
            bool overwrite=true);

   /**
    * This is a generic find method that takes a comparator type and iterates through 
    * the map executing the overloaded operator ().
    * Typical code example format
    <pre>
    typedef std::unary_function<std::pair<ossimString, ossimString>, bool> KwlCompareFunctionType;
    
    class KwlKeyCaseInsensitiveEquals : public KwlCompareFunctionType
    {
    public:
       KwlKeyCaseInsensitiveEquals(const ossimString& key):m_key(key){}
       virtual bool operator()(const KwlComparePairType& rhs)const
       {
          return (m_key == rhs.first.downcase());
       }
       ossimString m_key;
    };

    // now for use case example:
    kwl.findValue(value, KwlKeyCaseInsensitiveEquals("foo"));
    </pre>
    
    This example shows how to supplly your own comparator and do a case insensitive
    search for the key foo and the value is set to the variable value.
    *
    */
   template<class CompareType>
   bool findValue(ossimString& value, const CompareType& compare)const
   {
      KeywordMap::const_iterator iter = std::find_if(m_map.begin(), m_map.end(), compare);
      bool result = (iter != m_map.end());
      if(result) value = iter->second;
      return result;
   }
   
   std::string& operator[](const std::string& key)
   {
      return m_map[key];
   }
   std::string operator[](const std::string& key)const
   {
      ossimString result = find(key.c_str());
      
      return result.c_str();
   }

   // Methods to add keywords to list.
   void addPair(const std::string& key,
                const std::string& value,
                bool               overwrite = true);

   void addPair(const std::string& prefix,
                const std::string& key,
                const std::string& value,
                bool               overwrite = true);
   
   /*!
    * Allows you to extract out a sub keywordlist from another
    * you can also collapse the hieracrchy by setting
    * strip prefix to true.
    */
   void add(const ossimKeywordlist& kwl,
            const char* prefix=0,
            bool stripPrefix=true);
  
   void add(const char*   key,
            const char*   value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            const char*   value,
            bool          overwrite = true);

   void add(const char*   key,
            char          value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            char          value,
            bool          overwrite = true);

   void add(const char*   key,
            ossim_int16   value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            ossim_int16   value,
            bool          overwrite = true);

   void add(const char*   key,
            ossim_uint16  value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            ossim_uint16  value,
            bool          overwrite = true);

   void add(const char*   key,
            ossim_int32   value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            ossim_int32   value,
            bool          overwrite = true);

   void add(const char*   key,
            ossim_uint32  value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            ossim_uint32  value,
            bool          overwrite = true);

   void add(const char*   key,
            ossim_int64   value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            ossim_int64   value,
            bool          overwrite = true);

   void add(const char*   key,
            ossim_uint64  value,
            bool          overwrite = true);

   void add(const char*   prefix,
            const char*   key,
            ossim_uint64  value,
            bool          overwrite = true);


   /**
    * @param key Key for key-value pair.
    *
    * @param value Value to pair with key.  Note this will be stored as a
    * string.
    * 
    * @param precision Decimal point precision of the output. (default = 8)
    *
    * @param trimZeroFlag If true trailing '0's and any trailing '.' will
    * be trimmed from the converted string.  (default = false)
    *
    * @param scientific If true output will be in scientific notation else
    * fixed is used. (default = false)
    */
   void add(const char*   key,
            ossim_float32 value,
            bool          overwrite    = true,
            int           precision    = 8);

   /**
    * @param key Key for key-value pair.
    *
    * @param value Value to pair with key.  Note this will be stored as a
    * string.
    * 
    * @param precision Decimal point precision of the output. (default = 8)
    *
    * @param trimZeroFlag If true trailing '0's and any trailing '.' will
    * be trimmed from the converted string.  (default = false)
    *
    * @param scientific If true output will be in scientific notation else
    * fixed is used. (default = false)
    */
   void add(const char*   prefix,
            const char*   key,
            ossim_float32 value,
            bool          overwrite    = true,
            int           precision    = 8);

   /**
    * @param key Key for key-value pair.
    *
    * @param value Value to pair with key.  Note this will be stored as a
    * string.
    * 
    * @param precision Decimal point precision of the output. (default = 15)
    *
    * @param trimZeroFlag If true trailing '0's and any trailing '.' will
    * be trimmed from the converted string.  (default = false)
    *
    * @param scientific If true output will be in scientific notation else
    * fixed is used. (default = false)
    */
   void add(const char*   key,
            ossim_float64 value,
            bool          overwrite    = true,
            int           precision    = 15);

   /**
    * @param key Key for key-value pair.
    *
    * @param value Value to pair with key.  Note this will be stored as a
    * string.
    * 
    * @param precision Decimal point precision of the output. (default = 15)
    *
    * @param trimZeroFlag If true trailing '0's and any trailing '.' will
    * be trimmed from the converted string.  (default = false)
    *
    * @param scientific If true output will be in scientific notation else
    * fixed is used. (default = false)
    */
   void add(const char*   prefix,
            const char*   key,
            ossim_float64 value,
            bool          overwrite    = true,
            int           precision    = 15);

   /**
    * @brief Checks for key in map.
    *
    * Note that "find" and findKey will alway return an empty string even if
    * the key in not in the map.
    *
    * @return true if key is in map even if value is empty; false, if not.
    */
   bool hasKey( const std::string& key ) const;
   
   /**
    *  @brief Find methods that take std::string(s).
    *  Searches the map for key(/prefix) and returns the resulting value
    *  or an empty string if the key was not found.
    *  @param key e.g. "number_line"
    *  @param prefix e..g "image0."
    *  @return Reference to string.  This will be empty if not found or
    *  if value is empty.
    */
   const std::string& findKey(const std::string& key) const;
   const std::string& findKey(const std::string& prefix,
                              const std::string& key) const;
   
   const char* find(const char* key) const;
   const char* find(const char* prefix,
                    const char* key) const;

   void remove(const char * key);
   void remove(const char* prefix, const char * key);

   /*!
    *  Searches the map for the number of keys containing the string.
    */
   ossim_uint32 numberOf(const char* str) const;

   /*!
    *  Searches the map for the number of keys containing the prefix+key.
    *  
    *  Given the keyword list contains:
    *
    *  source.type1: foo
    *  source.type2: you
    *
    *  This:
    *
    *  int number_of_sources = numberOf("source", "type");
    *
    *  number_of_sources equals 2
    */
   ossim_uint32 numberOf(const char* prefix, const char* key) const;

   /**
    * Methods to dump the ossimKeywordlist to a file on disk.
    *
    * @param file Name of output file.
    * @param comment Optional string that will be written to line 1
    * as a C++-style comment. A "//" is prepended to the input string.
    *
    * @return true on success, false on error.
    */
   virtual bool write(const char* file, const char* comment = 0) const;

   virtual ossimString toString()const;
   virtual void toString(ossimString& result)const;

   virtual void writeToStream(std::ostream &out)const;

   /**
    * Outputs in xml format.
    * @param out Stream to write to.
    * @param rootTag name of the root XML element/tag
    */
   void toXML(std::ostream &out, const std::string& rootTag="info")const;
   
   /**
    * Outputs in json format.
    * @param out Stream to write to.
    * @param rootTag name of the root json element/tag
    */
   void toJSON(std::ostream &out, const std::string& rootTag="info")const;
   
   virtual std::ostream& print(std::ostream& os) const;
   OSSIMDLLEXPORT friend std::ostream& operator<<(std::ostream& os,
                                                  const ossimKeywordlist& kwl);
   bool operator ==(ossimKeywordlist& kwl)const;
   bool operator !=(ossimKeywordlist& kwl)const;

    /*!
     * Clear all contents out of the ossimKeywordlist.
     */

    void clear();

    /*!
     * Add contents of another keyword list to this one.
     *
     * @param src the keyword list to copy items from.
     * @param overwrite true if keys existing in this and src should have
     * their value overwritten by the src value, otherwise false to preserve
     * the original value.  Defaults to true.
     */

   void addList( const ossimKeywordlist &src, bool overwrite = true );

   /** deprecated method */
   virtual bool parseStream(ossim::istream& is,
                            bool ignoreBinaryChars);
   
   virtual bool parseStream(ossim::istream& is);
   virtual bool parseString(const std::string& inString);

   /**
   * This return the sorted keys if you have a list.
   * Example:
   * @code
   * // given a keywordlist called kwl with contents: 
   * // my.list.element1.prop
   * // my.list.element345.prop
   * // my.list.element22.prop
   * std::vector<ossimString> sortedPrefixValues;
   * kwl.getSortedList(sortedPrefixValues, "my.list.element");
   * if(sortedPrefixValues.size())
   * {
   * // contents should be my.list.element1, my.list.element22, my.list.element345
   *
   * }
   * @endcode
   *
   */
   void getSortedList(std::vector<ossimString>& prefixValues,
                      const ossimString &prefixKey)const;
   /*!
    *  Will return a list of keys that contain the string passed in.
    *  Later we will need to allow a user to specify regular expresion
    *  searches.
    */
   std::vector<ossimString> findAllKeysThatContains(
      const ossimString &searchString)const;

   /**
    * @brief Finds keys that match regular expression.
    *
    * Note: This does not clear vector passed to it.
    *
    * @param result Initialized by this.
    * @param regularExpression e.g. "image[0-9]*\\.file"
    */
   void findAllKeysThatMatch( std::vector<ossimString>& result,
                              const ossimString &regularExpression ) const;

   /**
    * @brief Gets number keys that match regular expression.
    * @param regularExpression e.g. "image[0-9]*\\.file"
    * @return Number of keys matching regular expression.
    */
   ossim_uint32 getNumberOfKeysThatMatch(
      const ossimString &regularExpression ) const;

   void extractKeysThatMatch(ossimKeywordlist& kwl,
                             const ossimString &regularExpression)const;

   void removeKeysThatMatch(const ossimString &regularExpression);

   /*!
    * Will return only the portion of the key that
    * matches the regular expression.
    *
    * example:
    *
    *  source1.source1.a:
    *  source1.source2.a:
    *  source1.source3.a:
    *  source1.source4.a:
    *  source1.source10.a:
    *
    *  kwl.getSubstringKeyList("source1.source[0-9]*\\.");
    *
    *  will return:
    *
    *  source1.source1.
    *  source1.source2.
    *  source1.source3.
    *  source1.source4.
    *  source1.source10.
    *
    */
   std::vector<ossimString> getSubstringKeyList(const ossimString& regularExpression)const;
   void getSubstringKeyList(std::vector<ossimString>& result,
                            const ossimString& regularExpression)const;

   ossim_uint32 getNumberOfSubstringKeys(
      const ossimString& regularExpression)const;

   void addPrefixToAll(const ossimString& prefix);
   void addPrefixToKeysThatMatch(const ossimString& prefix,
                                 const ossimString& regularExpression);
   void stripPrefixFromAll(const ossimString& regularExpression);

   /*!
    * Returns the number of elements.
    */
   ossim_uint32 getSize()const;

   const ossimKeywordlist::KeywordMap& getMap()const;
   ossimKeywordlist::KeywordMap& getMap();
   
   ossimKeywordlist& downcaseKeywords();
   ossimKeywordlist& upcaseKeywords();
   
   ossimKeywordlist& trimAllValues(const ossimString& valueToTrim= ossimString(" \t\n\r"));
   ossimKeywordlist trimAllValues(const ossimString& valueToTrim= ossimString(" \t\n\r"))const;


   //! [OLK, Aug/2008]
   //! Sets the boolean  <rtn_val> depending on value associated with keyword for values = 
   //! (yes|no|true|false|1|0). Returns TRUE if keyword found, otherwise false. Also returns false
   //! if none of the above permitted values are specified (rtn_val left unchanged in this case).
   bool getBoolKeywordValue(bool& rtn_val, 
                            const char* keyword, 
                            const char* prefix=0) const;

protected:
   enum KeywordlistParseState
   {
      KeywordlistParseState_OK         = 0,
      
      // Used to say this set of token has failed the rules.
      KeywordlistParseState_FAIL       = 1,

      // Means an error occured that is a mal formed stream for Keywordlist.
      KeywordlistParseState_BAD_STREAM = 2, 
   };
   /*!
    *  Method to parse files to initialize the list.  Method will error on
    *  binary characters if "ignoreBinaryChars = false".  This is used by
    *  ImageHandler factories that can be passed a binary file inadvertently
    *  by a user.  The "ignoreBinaryChars" flag should be set to true if
    *  a text file contains mixed ascii/binary values.
    *  Returns true if file was parsed, false on error.
    */
   bool parseFile(const ossimFilename& file,
                  bool  ignoreBinaryChars = false);

   bool isValidKeywordlistCharacter(ossim_uint8 c)const;
   void skipWhitespace(ossim::istream& in)const;
   KeywordlistParseState readComments(ossimString& sequence, ossim::istream& in)const;
   KeywordlistParseState readPreprocDirective(ossim::istream& in);
   KeywordlistParseState readKey(ossimString& sequence, ossim::istream& in)const;
   KeywordlistParseState readValue(ossimString& sequence, ossim::istream& in)const;
   KeywordlistParseState readKeyAndValuePair(ossimString& key,
                                             ossimString& value, ossim::istream& in)const;
   
   // Method to see if keyword exists in list.
   KeywordMap::iterator getMapEntry(const std::string& key);
   KeywordMap::iterator getMapEntry(const ossimString& key);
   KeywordMap::iterator getMapEntry(const char* key);

   // For toXML method lifted from oms::DataInfo.
   bool isSpecialXmlCharacters(const ossimString& value) const;
   bool isValidTag(const std::string& value)const;
   void replaceSpecialCharacters(ossimString& value)const;

   /**
    * @return true if a == b, false if not.
    */
   bool isSame( const std::vector<ossimString>& a,
                const std::vector<ossimString>& b ) const;

   KeywordMap               m_map;
   char                     m_delimiter;

   // enables preserving empty field values, multi lines, ... etc
   bool                     m_preserveKeyValues; 

   bool                     m_expandEnvVars;

   // enables relative paths in #include directive
   ossimFilename            m_currentlyParsing; 
};

#endif /* #ifndef ossimKeywordlist_HEADER */
