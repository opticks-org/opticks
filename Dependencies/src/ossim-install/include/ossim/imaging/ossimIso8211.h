/******************************************************************************
 * Copied from "gdal" project. See licence below.
 *
 * Project:  ISO 8211 Access
 * Purpose:  Main declarations for ISO 8211.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam <warmerdam@pobox.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 * $Id: ossimIso8211.h 7238 2005-02-13 16:08:04Z dburken $
 */

#ifndef ossimIso8211_HEADER
#define ossimIso8211_HEADER

#include <cstdio>
#include <cstdlib>


/**
  General data type
    */
typedef enum {
    DDFInt,
    DDFFloat,
    DDFString,
    DDFBinaryString
} DDFDataType;
  
/************************************************************************/
/*      These should really be private to the library ... they are      */
/*      mostly conveniences.                                            */
/************************************************************************/

long ossimDDFScanInt( const char *pszString, int nMaxChars );
int  ossimDDFScanVariable( const char * pszString,
                           int nMaxChars, int nDelimChar );
char *ossimDDFFetchVariable( const char *pszString, int nMaxChars,
                             int nDelimChar1, int nDelimChar2,
                             int *pnConsumedChars );

#ifndef OSSIM_DDF_FIELD_TERMINATOR
#  define OSSIM_DDF_FIELD_TERMINATOR    30
#endif
#ifndef OSSIM_DDF_UNIT_TERMINATOR
#  define OSSIM_DDF_UNIT_TERMINATOR     31
#endif

/************************************************************************/
/*                           Predeclarations                            */
/************************************************************************/

class ossimDDFFieldDefn;
class ossimDDFSubfieldDefn;
class ossimDDFRecord;
class ossimDDFField;

/************************************************************************/
/*                              DDFModule                               */
/************************************************************************/

/**
  The primary class for reading ISO 8211 files.  This class contains all
  the information read from the DDR record, and is used to read records
  from the file.           

*/  

class ossimDDFModule
{
  public:
                ossimDDFModule();
                ~ossimDDFModule();
                
    int         Open( const char * pszFilename, int bFailQuietly = false );
    int         Create( const char *pszFilename );
    void        Close();

    int         Initialize( char chInterchangeLevel = '3',
                            char chLeaderIden = 'L', 
                            char chCodeExtensionIndicator = 'E',
                            char chVersionNumber = '1',
                            char chAppIndicator = ' ',
                            const char *pszExtendedCharSet = " ! ",
                            int nSizeFieldLength = 3,
                            int nSizeFieldPos = 4,
                            int nSizeFieldTag = 4 );

    void        Dump( FILE * fp );

    ossimDDFRecord   *ReadRecord( void );
    void        Rewind( long nOffset = -1 );

    ossimDDFFieldDefn *FindFieldDefn( const char * );

    /** Fetch the number of defined fields. */

    int         GetFieldCount() { return nFieldDefnCount; }
    ossimDDFFieldDefn *GetField(int);
    void        AddField( ossimDDFFieldDefn *poNewFDefn );
    
    // This is really just for internal use.
    int         GetFieldControlLength() { return _fieldControlLength; }
    void        AddCloneRecord( ossimDDFRecord * );
    void        RemoveCloneRecord( ossimDDFRecord * );
    
    // This is just for DDFRecord.
    FILE        *GetFP() { return fpDDF; }
    
  private:
    FILE        *fpDDF;
    int         bReadOnly;
    long        nFirstRecordOffset;

    char        _interchangeLevel;
    char        _inlineCodeExtensionIndicator;
    char        _versionNumber;
    char        _appIndicator;
    int         _fieldControlLength;
    char        _extendedCharSet[4];

    long _recLength;
    char _leaderIden;
    long _fieldAreaStart;
    long _sizeFieldLength;
    long _sizeFieldPos;
    long _sizeFieldTag;

    // One DirEntry per field.  
    int         nFieldDefnCount;
    ossimDDFFieldDefn **papoFieldDefns;

    ossimDDFRecord   *poRecord;

    int         nCloneCount;
    int         nMaxCloneCount;
    ossimDDFRecord   **papoClones;
};

/************************************************************************/
/*                             DDFFieldDefn                             */
/************************************************************************/

  typedef enum { dsc_elementary, dsc_vector, dsc_array, dsc_concatenated } DDF_data_struct_code;
  typedef enum { dtc_char_string, 
                 dtc_implicit_point, 
                 dtc_explicit_point, 
                 dtc_explicit_point_scaled, 
                 dtc_char_bit_string, 
                 dtc_bit_string, 
                 dtc_mixed_data_type } DDF_data_type_code;

/**
 * Information from the DDR defining one field.  Note that just because
 * a field is defined for a DDFModule doesn't mean that it actually occurs
 * on any records in the module.  DDFFieldDefns are normally just significant
 * as containers of the DDFSubfieldDefns.
 */

class ossimDDFFieldDefn
{
  public:
                ossimDDFFieldDefn();
                ~ossimDDFFieldDefn();

    int         Create( const char *pszTag, const char *pszFieldName,
                        const char *pszDescription,
                        DDF_data_struct_code eDataStructCode,
                        DDF_data_type_code   eDataTypeCode,
                        const char *pszFormat = NULL );
    void        AddSubfield( ossimDDFSubfieldDefn *poNewSFDefn,
                             int bDontAddToFormat = false );
    void        AddSubfield( const char *pszName, const char *pszFormat );
    int         GenerateDDREntry( char **ppachData, int *pnLength ); 
                            
    int         Initialize( ossimDDFModule * poModule, const char *pszTag,
                            int nSize, const char * pachRecord );
    
    void        Dump( FILE * fp );

    /** Fetch a pointer to the field name (tag).
     * @return this is an internal copy and shouldn't be freed.
     */
    const char  *GetName() { return pszTag; }

    /** Fetch a longer descriptio of this field.
     * @return this is an internal copy and shouldn't be freed.
     */
    const char  *GetDescription() { return _fieldName; }

    /** Get the number of subfields. */
    int         GetSubfieldCount() { return nSubfieldCount; }
    
    ossimDDFSubfieldDefn *GetSubfield( int i );
    ossimDDFSubfieldDefn *FindSubfieldDefn( const char * );

    /**
     * Get the width of this field.  This function isn't normally used
     * by applications.
     *
     * @return The width of the field in bytes, or zero if the field is not
     * apparently of a fixed width.
     */
    int         GetFixedWidth() { return nFixedWidth; }

    /**
     * Fetch repeating flag.
     * @see DDFField::GetRepeatCount()
     * @return TRUE if the field is marked as repeating.
     */
    int         IsRepeating() { return bRepeatingSubfields; }

    static char       *ExpandFormat( const char * );

    /** this is just for an S-57 hack for swedish data */
    void SetRepeatingFlag( int n ) { bRepeatingSubfields = n; }

    char        *GetDefaultValue( int *pnSize );
    
  private:

    static char       *ExtractSubstring( const char * );

    ossimDDFModule * poModule;
    char *      pszTag;

    char *      _fieldName;
    char *      _arrayDescr;
    char *      _formatControls;

    int         bRepeatingSubfields;
    int         nFixedWidth;    // zero if variable. 

    int         BuildSubfields();
    int         ApplyFormats();

    DDF_data_struct_code _data_struct_code;

    DDF_data_type_code   _data_type_code;

    int         nSubfieldCount;
    ossimDDFSubfieldDefn **papoSubfields;
};

/************************************************************************/
/*                           DDFSubfieldDefn                            */
/*                                                                      */
/*      Information from the DDR record for one subfield of a           */
/*      particular field.                                               */
/************************************************************************/

/**
 * Information from the DDR record describing one subfield of a DDFFieldDefn.
 * All subfields of a field will occur in each occurance of that field
 * (as a DDFField) in a DDFRecord.  Subfield's actually contain formatted
 * data (as instances within a record).
 */

class ossimDDFSubfieldDefn
{
public:

                ossimDDFSubfieldDefn();
                ~ossimDDFSubfieldDefn();

    void        SetName( const char * pszName );

    /** Get pointer to subfield name. */
    const char  *GetName() { return pszName; }
    
    /** Get pointer to subfield format string */
    const char  *GetFormat() { return pszFormatString; }
    int         SetFormat( const char * pszFormat );

    /**
     * Get the general type of the subfield.  This can be used to
     * determine which of ExtractFloatData(), ExtractIntData() or
     * ExtractStringData() should be used.
     * @return The subfield type.  One of DDFInt, DDFFloat, DDFString or
     * DDFBinaryString.
     */
      
    DDFDataType GetType() { return eType; }

    double      ExtractFloatData( const char *pachData, int nMaxBytes,
                                  int * pnConsumedBytes );
    int         ExtractIntData( const char *pachData, int nMaxBytes,
                                int * pnConsumedBytes );
    const char  *ExtractStringData( const char *pachData, int nMaxBytes,
                                    int * pnConsumedBytes );
    int         GetDataLength( const char *, int, int * );
    void        DumpData( const char *pachData, int nMaxBytes, FILE * fp );

    int         FormatStringValue( char *pachData, int nBytesAvailable, 
                                   int *pnBytesUsed, const char *pszValue, 
                                   int nValueLength = -1 );

    int         FormatIntValue( char *pachData, int nBytesAvailable, 
                                int *pnBytesUsed, int nNewValue );

    int         FormatFloatValue( char *pachData, int nBytesAvailable, 
                                  int *pnBytesUsed, double dfNewValue );

    /** Get the subfield width (zero for variable). */
    int         GetWidth() { return nFormatWidth; } // zero for variable.

    int         GetDefaultValue( char *pachData, int nBytesAvailable, 
                                 int *pnBytesUsed );
    
    void        Dump( FILE * fp );

/**
  Binary format: this is the digit immediately following the B or b for
  binary formats. 
  */
typedef enum {
    NotBinary=0,
    UInt=1,
    SInt=2,
    FPReal=3,
    FloatReal=4,
    FloatComplex=5
} DDFBinaryFormat;

    DDFBinaryFormat GetBinaryFormat(void) const { return eBinaryFormat; }
    

private:

  char      *pszName;   // a.k.a. subfield mnemonic
  char      *pszFormatString; 

  DDFDataType           eType;
  DDFBinaryFormat       eBinaryFormat;

/* -------------------------------------------------------------------- */
/*      bIsVariable determines whether we using the                     */
/*      chFormatDelimeter (TRUE), or the fixed width (false).           */
/* -------------------------------------------------------------------- */
  int        bIsVariable;
  
  char       chFormatDelimeter;
  int        nFormatWidth;

/* -------------------------------------------------------------------- */
/*      Fetched string cache.  This is where we hold the values         */
/*      returned from ExtractStringData().                              */
/* -------------------------------------------------------------------- */
  int        nMaxBufChars;
  char       *pachBuffer;
};

/************************************************************************/
/*                              DDFRecord                               */
/*                                                                      */
/*      Class that contains one DR record from a file.  We read into    */
/*      the same record object repeatedly to ensure that repeated       */
/*      leaders can be easily preserved.                                */
/************************************************************************/

/**
 * Contains instance data from one data record (DR).  The data is contained
 * as a list of DDFField instances partitioning the raw data into fields.
 */

class ossimDDFRecord
{
  public:
                ossimDDFRecord( ossimDDFModule * );
                ~ossimDDFRecord();

    ossimDDFRecord  *Clone();
    ossimDDFRecord  *CloneOn( ossimDDFModule * );
    
    void        Dump( FILE * );

    /** Get the number of DDFFields on this record. */
    int         GetFieldCount() { return nFieldCount; }

    ossimDDFField    *FindField( const char *, int = 0 );
    ossimDDFField    *GetField( int );

    int         GetIntSubfield( const char *, int, const char *, int,
                                int * = NULL );
    double      GetFloatSubfield( const char *, int, const char *, int,
                                  int * = NULL );
    const char *GetStringSubfield( const char *, int, const char *, int,
                                   int * = NULL );

    int         SetIntSubfield( const char *pszField, int iFieldIndex, 
                                const char *pszSubfield, int iSubfieldIndex,
                                int nValue );
    int         SetStringSubfield( const char *pszField, int iFieldIndex, 
                                   const char *pszSubfield, int iSubfieldIndex,
                                   const char *pszValue, int nValueLength=-1 );
    int         SetFloatSubfield( const char *pszField, int iFieldIndex, 
                                  const char *pszSubfield, int iSubfieldIndex,
                                  double dfNewValue );

    /** Fetch size of records raw data (GetData()) in bytes. */
    int         GetDataSize() { return nDataSize; }

    /**
     * Fetch the raw data for this record.  The returned pointer is effectively
     * to the data for the first field of the record, and is of size 
     * GetDataSize().
     */
    const char  *GetData() { return pachData; }

    /**
     * Fetch the DDFModule with which this record is associated.
     */

    ossimDDFModule * GetModule() { return poModule; }

    int ResizeField( ossimDDFField *poField, int nNewDataSize );
    int DeleteField( ossimDDFField *poField );
    ossimDDFField* AddField( ossimDDFFieldDefn * );

    int CreateDefaultFieldInstance( ossimDDFField *poField, int iIndexWithinField );

    int SetFieldRaw( ossimDDFField *poField, int iIndexWithinField, 
                     const char *pachRawData, int nRawDataSize );
    int UpdateFieldRaw( ossimDDFField *poField, int iIndexWithinField, 
                        int nStartOffset, int nOldSize,
                        const char *pachRawData, int nRawDataSize );

    int         Write();
    
    // This is really just for the DDFModule class.
    int         Read();
    void        Clear();
    int         ResetDirectory();
    
  private:

    int         ReadHeader();
    
    ossimDDFModule   *poModule;

    int         nReuseHeader;   

    int         nFieldOffset;   // field data area, not dir entries.

    int         _sizeFieldTag;
    int         _sizeFieldPos;
    int         _sizeFieldLength;

    int         nDataSize;      // Whole record except leader with header
    char        *pachData;

    int         nFieldCount;
    ossimDDFField    *paoFields;

    int         bIsClone;
};

/************************************************************************/
/*                               DDFField                               */
/*                                                                      */
/*      This object represents one field in a DDFRecord.                */
/************************************************************************/

/**
 * This object represents one field in a DDFRecord.  This
 * models an instance of the fields data, rather than it's data definition
 * which is handled by the DDFFieldDefn class.  Note that a DDFField
 * doesn't have DDFSubfield children as you would expect.  To extract
 * subfield values use GetSubfieldData() to find the right data pointer and
 * then use ExtractIntData(), ExtractFloatData() or ExtractStringData().
 */

class ossimDDFField
{
  public:
    void                Initialize( ossimDDFFieldDefn *, const char *pszData,
                                    int nSize );

    void                Dump( FILE * fp );

    const char         *GetSubfieldData( ossimDDFSubfieldDefn *,
                                         int * = NULL, int = 0 );

    const char         *GetInstanceData( int nInstance, int *pnSize );

    /**
     * Return the pointer to the entire data block for this record. This
     * is an internal copy, and shouldn't be freed by the application.
     */
    const char         *GetData() { return pachData; }

    /** Return the number of bytes in the data block returned by GetData(). */
    int                 GetDataSize() { return nDataSize; }

    int                 GetRepeatCount();

    /** Fetch the corresponding DDFFieldDefn. */
    ossimDDFFieldDefn        *GetFieldDefn() { return poDefn; }
    
  private:
    ossimDDFFieldDefn        *poDefn;

    int                 nDataSize;

    const char          *pachData;
};

#endif /* ifndef ossimIso8211_HEADER */
