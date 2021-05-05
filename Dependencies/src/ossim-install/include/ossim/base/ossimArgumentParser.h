//-------------------------------------------------------------------------
//
// This code was taken from Open Scene Graph and incorporated from into
// OSSIM.
//
//-------------------------------------------------------------------------
// $Id: ossimArgumentParser.h 22491 2013-11-26 18:17:29Z dburken $
#ifndef ossimArgumentParser_HEADER
#define ossimArgumentParser_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimString.h>
#include <map>
#include <string>
#include <iosfwd>

class ossimApplicationUsage;

class OSSIMDLLEXPORT ossimArgumentParser
{
public:
   
   class ossimParameter
   {
   public:
      enum ossimParameterType
      {
         OSSIM_FLOAT_PARAMETER,
         OSSIM_DOUBLE_PARAMETER,
         OSSIM_INT_PARAMETER,
         OSSIM_UNSIGNED_INT_PARAMETER,
         OSSIM_STRING_PARAMETER,
      };
      
      union ossimValueUnion
      {
         float*          theFloat;
         double*         theDouble;
         int*            theInt;
         unsigned int*   theUint;
         std::string*    theString;
      };
      
      ossimParameter(float& value)
      {
         theType = OSSIM_FLOAT_PARAMETER; theValue.theFloat = &value;
      }
      
      ossimParameter(double& value)
      {
         theType = OSSIM_DOUBLE_PARAMETER; theValue.theDouble = &value;
      }
      
      ossimParameter(int& value)
      {
         theType = OSSIM_INT_PARAMETER; theValue.theInt = &value;
      }
      
      ossimParameter(unsigned int& value)
      {
         theType = OSSIM_UNSIGNED_INT_PARAMETER; theValue.theUint = &value;
      }
      
      ossimParameter(std::string& value)
      {
         theType = OSSIM_STRING_PARAMETER; theValue.theString = &value;
      }
      
      ossimParameter(ossimString& value)
      {
         theType = OSSIM_STRING_PARAMETER; theValue.theString =
                                              &(value.string());
      }
      
      bool valid(const char* str) const;
      bool assign(const char* str);

   protected:
      
      ossimParameterType   theType;
      ossimValueUnion      theValue;
   };
   
   /** return return true if specified string is an option in the form of
    * -option or --option .
    */
   static bool isOption(const char* str);
   
   /** return return true if string is any other string apart from an option.*/
   static bool isString(const char* str);
   
   /** return return true if specified parameter is an number.*/
   static bool isNumber(const char* str);
   
public:
   
   ossimArgumentParser(int* argc,char **argv);
   ossimArgumentParser(const ossimString& commandLine);

   ~ossimArgumentParser();

   /** @brief Initialize from command arguments. */
   void initialize(int* argc, const char **argv); 
   
   void setApplicationUsage(ossimApplicationUsage* usage) { theUsage = usage; }
   ossimApplicationUsage* getApplicationUsage() { return theUsage; }
   const ossimApplicationUsage* getApplicationUsage() const { return theUsage; }
   
   /** return the argument count.*/
   int& argc() { return *theArgc; }
   
   /** return the argument array.*/
   char** argv() { return theArgv; }
   
   /** return char* argument at specificed position.*/
   char* operator [] (int pos) { return theArgv[pos]; }
   
   /** return const char* argument at specificed position.*/
   const char* operator [] (int pos) const { return theArgv[pos]; }
   
   /** return the application name, as specified by argv[0] */
   std::string getApplicationName() const;
   
   /** return the position of an occurence of a string in the argument list.
    * return -1 when no string is found.*/      
   int find(const std::string& str) const;
   
   /** return return true if specified parameter is an option in the form of -option or --option .*/
   bool isOption(int pos) const;
   
   /** return return true if specified parameter is an string, which can be any other string apart from an option.*/
   bool isString(int pos) const;
   
   /** return return true if specified parameter is an number.*/
   bool isNumber(int pos) const;
   
   bool containsOptions() const;
   
   /** remove one or more arguments from the argv argument list, and decrement the argc respectively.*/
   void remove(int pos,int num=1);
   
   /** Inserts string into the argv argument list, and increment the argc respectively.
    * If string contains spaces, it will be split up into component simple strings. */
   void insert(int pos, const ossimString& arg);

   /** return true if specified argument matches string.*/        
   bool match(int pos, const std::string& str) const;
   
   /**
    * search for an occurance of a string in the argument list, on sucess
    * remove that occurance from the list and return true, otherwise
    * return false.
    */
   bool read(const std::string& str);
   bool read(const std::string& str, ossimParameter value1);
   bool read(const std::string& str, ossimParameter value1,
             ossimParameter value2);
   bool read(const std::string& str, ossimParameter value1,
             ossimParameter value2, ossimParameter value3);
   bool read(const std::string& str, ossimParameter value1,
             ossimParameter value2, ossimParameter value3,
             ossimParameter value4);
   bool read(const std::string& str, ossimParameter value1,
             ossimParameter value2, ossimParameter value3,
             ossimParameter value4, ossimParameter value5);
   bool read(const std::string& str, ossimParameter value1,
             ossimParameter value2, ossimParameter value3,
             ossimParameter value4, ossimParameter value5,
             ossimParameter value6);
   
   /**
    * Alternate form for reading variable length arguments (must be comma-separated), e.g.,
    *
    *    --input_files file1, file2, file3,file4 next_arg
    *
    * Note that spaces between arguments are optional. The next_arg entry will not be considered
    * part of the list since there's no comma separator and will be left on the argument array.
    * @param str The option string (with "-" or "--")
    * @param param_list Vector to contain results as strings. Always cleared before populating
    * @return True if option found (param_list may be empty f no args followed).
    */
   bool read(const std::string& str, std::vector<ossimString>& param_list);

   /**
    * @return The number of parameters of type value associated with specified
    * option, or -1 if option not found
    */
   int numberOfParams(const std::string& str,
                      const ossimParameter value) const;
   
   /**
    * if the argument value at the position pos matches specified string, and
    * subsequent paramters are also matched then set the paramter values and
    * remove the from the list of arguments.
    */
   bool read(int pos, const std::string& str);
   bool read(int pos, const std::string& str, ossimParameter value1);
   bool read(int pos, const std::string& str, ossimParameter value1,
             ossimParameter value2);
   bool read(int pos, const std::string& str, ossimParameter value1,
             ossimParameter value2, ossimParameter value3);
   bool read(int pos, const std::string& str, ossimParameter value1,
             ossimParameter value2, ossimParameter value3,
             ossimParameter value4);
   
   
   enum ossimErrorSeverity
   {
      OSSIM_BENIGN = 0,
      OSSIM_CRITICAL = 1
   };
   
   typedef std::map<std::string,ossimErrorSeverity> ossimErrorMessageMap;
   
   /**
    * @return The error flag, true if an error has occured when
    * reading arguments.
    */
   bool errors(ossimErrorSeverity severity=OSSIM_BENIGN) const;
   
   /** report an error message by adding to the ErrorMessageMap.*/
   void reportError(const std::string& message,
                    ossimErrorSeverity severity=OSSIM_CRITICAL);
   
   /** for each remaining option report it as an unrecongnized.*/
   void reportRemainingOptionsAsUnrecognized(
      ossimErrorSeverity severity=OSSIM_BENIGN);
   
   /** @return The error message, if any has occured.*/
   ossimErrorMessageMap& getErrorMessageMap();
   
   /** @return The error message, if any has occured.*/
   const ossimErrorMessageMap& getErrorMessageMap() const;
   
   /** write out error messages at an above specified .*/
   void writeErrorMessages(std::ostream& output,
                           ossimErrorSeverity sevrity=OSSIM_BENIGN);
   
   
protected:
   
   int*                     theArgc;
   char**                   theArgv;
   ossimErrorMessageMap     theErrorMessageMap;
   ossimApplicationUsage*   theUsage;
   bool                     theMemAllocated;
        
};

#endif
