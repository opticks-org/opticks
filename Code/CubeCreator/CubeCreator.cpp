/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <tclap/CmdLine.h>

#include <direct.h>
#include <limits>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#define BIG_ENDIAN_BYTE_ORDER 4321
#define LITTLE_ENDIAN_BYTE_ORDER 1234

#if defined(_MSC_VER)
#define BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
#elif defined (__SUNPRO_CC)
#include <sys/isa_defs.h>

#ifdef _BIG_ENDIAN
#define BYTE_ORDER BIG_ENDIAN_BYTE_ORDER
#else
#define BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
#endif
#else
#error Unrecognized build platform
#endif


using namespace std;

struct IntegerComplex
{
   short mReal;
   short mImaginary;
};

struct FloatComplex
{
   float mReal;
   float mImaginary;
};

template<typename T>
static void writeBIP(const string& filenameBase,
                     const unsigned int& rows,
                     const unsigned int& columns,
                     const unsigned int& bands,
                     bool littleEndian,
                     const unsigned int& headerBytes,
                     const unsigned int& trailerBytes,
                     const unsigned int& preLineBytes,
                     const unsigned int& postLineBytes);

template<typename T>
static void writeBIL(const string& filenameBase,
                     const unsigned int& rows,
                     const unsigned int& columns,
                     const unsigned int& bands,
                     bool littleEndian,
                     const unsigned int& headerBytes,
                     const unsigned int& trailerBytes,
                     const unsigned int& preLineBytes,
                     const unsigned int& postLineBytes);

template<typename T>
static void writeBSQ(const string& filenameBase,
                     const unsigned int& rows,
                     const unsigned int& columns,
                     const unsigned int& bands,
                     bool littleEndian, 
                     const unsigned int& headerBytes,
                     const unsigned int& trailerBytes,
                     const unsigned int& preLineBytes,
                     const unsigned int& postLineBytes,
                     const unsigned int& preBandBytes,
                     const unsigned int& postBandBytes);

template<typename T>
static void writeBSQMulti(const string& filenameBase,
                          const unsigned int& rows,
                          const unsigned int& columns,
                          const unsigned int& bands,
                          bool littleEndian,
                          const unsigned int& headerBytes,
                          const unsigned int& trailerBytes,
                          const unsigned int& preLineBytes,
                          const unsigned int& postLineBytes,
                          const unsigned int& preBandBytes,
                          const unsigned int& postBandBytes);

template<typename T>
static void createCube(const string& interleave,
                       bool littleEndian,
                       const unsigned int& rows,
                       const unsigned int& columns,
                       const unsigned int& bands,
                       const unsigned int& headerBytes,
                       const unsigned int& trailerBytes,
                       const unsigned int& preBandBytes,
                       const unsigned int& postBandBytes,
                       const unsigned int& preLineBytes,
                       const unsigned int& postLineBytes);

static double getMin(unsigned char);
static double getMin(char);
static double getMin(unsigned short);
static double getMin(short);
static double getMin(unsigned int);
static double getMin(int);
static double getMin(float);
static double getMin(double);
static double getMax(unsigned char);
static double getMax(char);
static double getMax(unsigned short);
static double getMax(short);
static double getMax(unsigned int);
static double getMax(int);
static double getMax(float);
static double getMax(double);

static char* getName(unsigned char*);
static char* getName(char*);
static char* getName(unsigned short*);
static char* getName(short*);
static char* getName(unsigned int*);
static char* getName(int*);
static char* getName(float*);
static char* getName(double*);
static char* getName(IntegerComplex*);
static char* getName(FloatComplex*);

void main (int argc, char *argv[])
{
   vector<string> types;
   vector<string> interleave;
   string cubeType;
   string interleaveType;
   unsigned int rows;
   unsigned int columns;
   unsigned int bands;
   unsigned int headerBytes;
   unsigned int trailerBytes;
   unsigned int preLineBytes;
   unsigned int postLineBytes;
   unsigned int preBandBytes;
   unsigned int postBandBytes;
   unsigned int numRandomCubes;
   unsigned int maxRandomCubeSize;
   bool littleEndian;
   try
   {
      /**
       * The TCLAP (Templatized C++ Command Line Parser) Library is a third party open-source library.
       * This library is being used in the Cube Creator in order to parse command-line arguments
       * provided by the user.
       * Please see the tclap/ folder for the include files provided by this library as well
       * as the license it is governed by.
       */
      string ver = "1.1 - Built " + string(__DATE__);
      TCLAP::CmdLine cmd("Creates generic cubes according to provided options in the current directory.", ' ', ver);
      types.push_back("INT1SBYTE");
      types.push_back("INT1UBYTE");
      types.push_back("INT2SBYTES");
      types.push_back("INT2UBYTES");
      types.push_back("INT4SBYTES");
      types.push_back("INT4UBYTES");
      types.push_back("FLT4BYTES");
      types.push_back("FLT8BYTES");
      types.push_back("INT4SCOMPLEX");
      types.push_back("FLT8COMPLEX");
      TCLAP::ValuesConstraint<string> typeConstraint(types);
      TCLAP::ValueArg<string> typeArg("t", "type", "The data type of the cube",
         false, "INT1UBYTE", &typeConstraint, cmd);
      interleave.push_back("BIP");
      interleave.push_back("BIL");
      interleave.push_back("BSQ");
      interleave.push_back("BSQMulti");
      TCLAP::ValuesConstraint<string> interleaveConstraint(interleave);
      TCLAP::ValueArg<string> interleaveArg("i", "interleave", "The interleave of the cube",
         false, "BIP", &interleaveConstraint, cmd);

      TCLAP::ValueArg<unsigned int> rowArg("r", "rows", "The number of rows in the cube",
         false, 512, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> columnArg("c", "columns", "The number of columns in the cube",
         false, 512, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> bandArg("b", "bands", "The number of bands in the cube",
         false, 3, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> headerBytesArg("", "headerBytes",
         "The number of header bytes before the cube", false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> trailerBytesArg("", "trailerBytes",
         "The number of trailer bytes after the cube", false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> preLineBytesArg("", "preLineBytes",
         "The number of bytes before a line in the cube", false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> postLineBytesArg("", "postLineBytes",
         "The number of bytes after a line in the cube", false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> preBandBytesArg("", "preBandBytes",
         "The number of bytes before a band in a BSQ formatted cube", false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> postBandBytesArg("", "postBandBytes",
         "The number of bytes after a band in a BSQ formatted cube", false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> numRandomCubesArg("", "numRandomCubes",
         "The number of random cubes to generate. If this value is not zero, the specified number of random "
         "cubes will be generated instead of the default cube. Each random cube will be no larger than the specified "
         "number of rows, columns, and bands and will be of a random data type and interleave.",
         false, 0, "unsigned int", cmd);

      TCLAP::ValueArg<unsigned int> maxRandomCubeSizeArg("", "maxRandomCubeSize",
         "If specified, the maximum number of bytes that a randomly generated cube can contain.",
         false, 0, "unsigned int", cmd);

      TCLAP::SwitchArg bigEndianArg("", "bigEndian", "If present a big endian cube will be created. "
         "If this flag is not used a little endian cube will be created.", cmd, false);

      cmd.parse(argc, argv);
      cubeType = typeArg.getValue();
      interleaveType = interleaveArg.getValue();
      rows = rowArg.getValue();
      columns = columnArg.getValue();
      bands = bandArg.getValue();
      headerBytes = headerBytesArg.getValue();
      trailerBytes = trailerBytesArg.getValue();
      preLineBytes = preLineBytesArg.getValue();
      postLineBytes = postLineBytesArg.getValue();
      preBandBytes = preBandBytesArg.getValue();
      postBandBytes = postBandBytesArg.getValue();
      numRandomCubes = numRandomCubesArg.getValue();
      maxRandomCubeSize = maxRandomCubeSizeArg.getValue();
      littleEndian = !(bigEndianArg.getValue());
   }
   catch (TCLAP::ArgException &e)
   {
      cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
      exit(1);
   }

   if (numRandomCubes == 0)
   {
      if (cubeType == "INT1SBYTE")
      {
         createCube<char>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "INT1UBYTE")
      {
         createCube<unsigned char>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "INT2SBYTES")
      {
         createCube<short>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "INT2UBYTES")
      {
         createCube<unsigned short>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "INT4SBYTES")
      {
         createCube<int>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "INT4UBYTES")
      {
         createCube<unsigned int>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "FLT4BYTES")
      {
         createCube<float>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "FLT8BYTES")
      {
         createCube<double>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "INT4SCOMPLEX")
      {
         createCube<IntegerComplex>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
      else if (cubeType == "FLT8COMPLEX")
      {
         createCube<FloatComplex>(interleaveType, littleEndian, rows, columns, bands,
            headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
      }
   }
   else
   {
      srand(static_cast<unsigned int>(time(NULL)));
      while (numRandomCubes != 0)
      {
         unsigned int randomNumRows = rand() % rows + 1;
         unsigned int randomNumColumns = rand() % columns + 1;
         unsigned int randomNumBands = rand() % bands + 1;
         string randomCubeType = types[rand() % types.size()];
         string randomInterleaveType = interleave[rand() % interleave.size()];
         const unsigned int rowColBandSize = randomNumRows * randomNumColumns * randomNumBands;

         if (randomCubeType == "INT1SBYTE")
         {
            if (maxRandomCubeSize != 0 && sizeof(char) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<char>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "INT1UBYTE")
         {
            if (maxRandomCubeSize != 0 && sizeof(unsigned char) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<unsigned char>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "INT2SBYTES")
         {
            if (maxRandomCubeSize != 0 && sizeof(short) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<short>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "INT2UBYTES")
         {
            if (maxRandomCubeSize != 0 && sizeof(unsigned short) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<unsigned short>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "INT4SBYTES")
         {
            if (maxRandomCubeSize != 0 && sizeof(int) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<int>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "INT4UBYTES")
         {
            if (maxRandomCubeSize != 0 && sizeof(unsigned int) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<unsigned int>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "FLT4BYTES")
         {
            if (maxRandomCubeSize != 0 && sizeof(float) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<float>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "FLT8BYTES")
         {
            if (maxRandomCubeSize != 0 && sizeof(double) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<double>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "INT4SCOMPLEX")
         {
            if (maxRandomCubeSize != 0 && sizeof(IntegerComplex) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<IntegerComplex>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }
         else if (randomCubeType == "FLT8COMPLEX")
         {
            if (maxRandomCubeSize != 0 && sizeof(FloatComplex) * rowColBandSize > maxRandomCubeSize)
            {
               continue;
            }
            createCube<FloatComplex>(randomInterleaveType, littleEndian, randomNumRows, randomNumColumns,
               randomNumBands, headerBytes, trailerBytes, preBandBytes, postBandBytes, preLineBytes, postLineBytes);
         }

         --numRandomCubes;
      }
   }
}

template<typename T>
static void createCube(const string& interleave,
                       bool littleEndian,
                       const unsigned int& rows,
                       const unsigned int& columns,
                       const unsigned int& bands,
                       const unsigned int& headerBytes,
                       const unsigned int& trailerBytes,
                       const unsigned int& preBandBytes,
                       const unsigned int& postBandBytes,
                       const unsigned int& preLineBytes,
                       const unsigned int& postLineBytes)
{
   cout << "BAND Format: " << interleave << endl;
   cout << "DATASIZEBYTES: " << sizeof(T) << endl;
   cout << "ROWS: " << rows << endl;
   cout << "COLUMNS: " << columns << endl;
   cout << "BANDS: " << bands << endl;
   cout << "HEADERBYTES: " << headerBytes << endl;
   cout << "TRAILERBYTES: " << endl;
   cout << "PREBANDBYTES: " << preBandBytes << endl;
   cout << "POSTBANDBYTES: " << postBandBytes << endl;
   cout << "PRELINEBYTES: " << preLineBytes << endl;
   cout << "POSTLINEBYTES: " << postLineBytes << endl;

   string endianStr = (littleEndian ? "lsb" : "msb");
   ostringstream filenameBaseStream;
   filenameBaseStream << "cube" << rows << "x" << columns << "x" <<
      bands << "x" << getName(reinterpret_cast<T*>(NULL)) << endianStr;
   string filenameBase = filenameBaseStream.str();

   cout << "Writing cube: ";

   if (interleave == "BIP")
   {
      writeBIP<T>(filenameBase, rows, columns, bands,
         littleEndian, headerBytes, trailerBytes, preLineBytes, postLineBytes);
   }
   else if (interleave == "BIL")
   {
      writeBIL<T>(filenameBase, rows, columns, bands,
         littleEndian, headerBytes, trailerBytes, preLineBytes, postLineBytes);
   }
   else if (interleave == "BSQ")
   {
      writeBSQ<T>(filenameBase, rows, columns, bands,
         littleEndian, headerBytes, trailerBytes, preLineBytes, postLineBytes, preBandBytes, postBandBytes);
   }
   else if (interleave == "BSQMulti")
   {
      writeBSQMulti<T>(filenameBase, rows, columns, bands,
         littleEndian, headerBytes, trailerBytes, preLineBytes, postLineBytes, preBandBytes, postBandBytes);
   }
   else
   {
      cout << "Unknown format: " << interleave << endl;
   }
   cout << endl << "Done" << endl;
}

void swapBuffer(void* pBuffer, size_t dataSize, size_t count, int requestedType)
{
   if ((pBuffer == NULL) || (BYTE_ORDER == requestedType))
   {
      return;
   }

   for (size_t i = 0; i < count; ++i)
   {
      unsigned char* pData = reinterpret_cast<unsigned char*>(pBuffer) + (i * dataSize);
      for (size_t j = 0; j < dataSize / 2; ++j)
      {
         size_t index = dataSize - j - 1;

         unsigned char byteData = pData[j];
         pData[j] = pData[index];
         pData[index] = byteData;
      }
   }
}

template<typename T>
static void endianSwap(T* pSource, int numItems, bool littleEndianDesired)
{
   swapBuffer(pSource, sizeof(T), numItems,
      (littleEndianDesired ? LITTLE_ENDIAN_BYTE_ORDER : BIG_ENDIAN_BYTE_ORDER));
}

template<>
static void endianSwap<IntegerComplex>(IntegerComplex* pSource, int numItems, bool littleEndianDesired)
{
   for (int i = 0; i < numItems; i++)
   {
      swapBuffer(pSource + i, sizeof(short), 2,
         (littleEndianDesired ? LITTLE_ENDIAN_BYTE_ORDER : BIG_ENDIAN_BYTE_ORDER));
   }
}

template<>
static void endianSwap<FloatComplex>(FloatComplex* pSource, int numItems, bool littleEndianDesired)
{
   for (int i = 0; i < numItems; i++)
   {
      swapBuffer(pSource + i, sizeof(float), 2,
         (littleEndianDesired ? LITTLE_ENDIAN_BYTE_ORDER : BIG_ENDIAN_BYTE_ORDER));
   }
}

template<typename T>
static T getValue(const unsigned int& currentRow,
                  const unsigned int& currentColumn,
                  const unsigned int& currentBand,
                  const unsigned int& totalRows,
                  const unsigned int& totalColumns,
                  const unsigned int& totalBands,
                  bool useSin = true)
{
   static const double PI = 3.1415927;
   double x, y, val;
   x = static_cast<double>(currentBand+1)*1.0*PI*static_cast<double>(currentColumn)/static_cast<double>(totalColumns);
   y = static_cast<double>((currentRow*(currentBand+1))%totalRows)/static_cast<double>(totalRows);
   T defaultValue = 0;
   if (useSin)
   {
      x = sin(x);
   }
   else
   {
      x = cos(x);
   }
   x *= x;
   val = x*y*(getMax(defaultValue)-getMin(defaultValue)) + getMin(defaultValue);
   return static_cast<T>(val);
}

template<>
static IntegerComplex getValue<IntegerComplex>(const unsigned int& currentRow,
                                               const unsigned int& currentColumn,
                                               const unsigned int& currentBand,
                                               const unsigned int& totalRows,
                                               const unsigned int& totalColumns,
                                               const unsigned int& totalBands,
                                               bool useSin)
{
   IntegerComplex retVal;
   retVal.mReal = getValue<short>(currentRow, currentColumn,
      currentBand, totalRows, totalColumns, totalBands, true);

   retVal.mImaginary = getValue<short>(currentRow, currentColumn,
      currentBand, totalRows, totalColumns, totalBands, false);
   return retVal;
}

template<>
static FloatComplex getValue<FloatComplex>(const unsigned int& currentRow,
                                           const unsigned int& currentColumn,
                                           const unsigned int& currentBand,
                                           const unsigned int& totalRows,
                                           const unsigned int& totalColumns,
                                           const unsigned int& totalBands,
                                           bool useSin)
{
   FloatComplex retVal;
   retVal.mReal = getValue<float>(currentRow, currentColumn,
      currentBand, totalRows, totalColumns, totalBands, true);

   retVal.mImaginary = getValue<float>(currentRow, currentColumn,
      currentBand, totalRows, totalColumns, totalBands, false);

   return retVal;
}

static int getEnviDataType(unsigned char*)  {return 1;}
static int getEnviDataType(char*)           {return 1;}
static int getEnviDataType(unsigned short*) {return 12;}
static int getEnviDataType(short*)          {return 2;}
static int getEnviDataType(unsigned int*)   {return 13;}
static int getEnviDataType(int*)            {return 3;}
static int getEnviDataType(float*)          {return 4;}
static int getEnviDataType(double*)         {return 5;}
static int getEnviDataType(IntegerComplex*) {return 99;}
static int getEnviDataType(FloatComplex*)   {return 6;}

static void writeENVIHeader(const string& filenameBase,
                            const string& cubeFilename,
                            const unsigned int& rows,
                            const unsigned int& columns,
                            const unsigned int& bands,
                            bool littleEndian,
                            const unsigned int& headerBytes,
                            const unsigned int& trailerBytes,
                            const unsigned int& preLineBytes,
                            const unsigned int& postLineBytes,
                            const unsigned int& preBandBytes,
                            const unsigned int& postBandBytes,
                            const string& interleave,
                            const int enviDataType)
{
   if (preLineBytes != 0 || postLineBytes != 0 || preBandBytes != 0 || postBandBytes != 0)
   {
      cout << "Cannot create an ENVI header.  ENVI does not support pre-line bytes, "
         "post-line bytes, pre-band bytes or post-band bytes" << endl;
      return; //ENVI header doesn't support any of the following
   }
   if (enviDataType == -1)
   {
      cout << "Cannot create an ENVI header.  ENVI does not support loading the "
         "cube with the data type that you selected." << endl;
      return;
   }
   string filename = filenameBase + ".hdr";
   FILE* pStream = fopen(filename.c_str(), "w");
   fprintf(pStream, "ENVI\n");
   fprintf(pStream, "samples = %d\n", columns);
   fprintf(pStream, "lines = %d\n", rows);
   fprintf(pStream, "bands = %d\n", bands);
   fprintf(pStream, "header offset = %d\n", headerBytes);
   fprintf(pStream, "file type = ENVI Standard\n");
   fprintf(pStream, "data type = %d\n", enviDataType);
   fprintf(pStream, "interleave = %s\n", interleave.c_str());
   fprintf(pStream, "byte order = %d\n", !littleEndian);
   fclose(pStream);
}


template<typename T>
static void writeBIP(const string& filenameBase,
                     const unsigned int& rows,
                     const unsigned int& columns,
                     const unsigned int& bands,
                     bool littleEndian,
                     const unsigned int& headerBytes,
                     const unsigned int& trailerBytes,
                     const unsigned int& preLineBytes,
                     const unsigned int& postLineBytes)
{
   unsigned int i, j, k;
   vector<T> bufferRes(bands);
   T* pBuffer = &bufferRes.front();
   FILE* pStream;
   int offset = 0;
   char dummyValue = 1;

   string filename = filenameBase + ".bip";
   pStream = fopen(filename.c_str(), "wb");

   if (headerBytes)
   {
      fwrite(&dummyValue, headerBytes, 1, pStream);
   }

   int modVal = rows / 10;
   if (modVal == 0)
   {
      modVal = 1;
   }
   for (i=0; i<rows; i++)
   {
      if (!(i%modVal))
      {
         cout << 10*i/(modVal*10) << flush;
      }

      if (preLineBytes != 0)
      {
         fwrite(&dummyValue, preLineBytes, 1, pStream);
      }

      for (j=0; j<columns; j++)
      {
         for (k=0; k<bands; k++)
         {
            pBuffer[k] = getValue<T>(i, j, k, rows, columns, bands);
         }
         endianSwap(pBuffer, bands, littleEndian);
         fwrite(pBuffer, bands, sizeof(T), pStream);
      }

      if (postLineBytes != 0)
      {
         fwrite(&dummyValue, postLineBytes, 1, pStream);
      }
   }

   if (trailerBytes)
   {
      fwrite(&dummyValue, trailerBytes, 1, pStream);
   }

   fclose(pStream);

   writeENVIHeader(filenameBase, filename, rows,
      columns, bands, littleEndian, headerBytes, trailerBytes,
      preLineBytes, postLineBytes, 0, 0,
      "bip", getEnviDataType(reinterpret_cast<T*>(NULL)));
}

template<typename T>
static void writeBIL(const string& filenameBase,
                     const unsigned int& rows,
                     const unsigned int& columns,
                     const unsigned int& bands,
                     bool littleEndian,
                     const unsigned int& headerBytes,
                     const unsigned int& trailerBytes,
                     const unsigned int& preLineBytes,
                     const unsigned int& postLineBytes)
{
   unsigned int i, j, k;
   vector<T> bufferRes(columns);
   T* pBuffer = &bufferRes.front();
   FILE* pStream;
   char dummyValue = 1;

   string filename = filenameBase + ".bil";
   pStream = fopen(filename.c_str(), "wb");

   if (headerBytes)
   {
      fwrite(&dummyValue, headerBytes, 1, pStream);
   }

   int modVal = rows / 10;
   if (modVal == 0)
   {
      modVal = 1;
   }
   for (i=0; i<rows; i++)
   {
      if (!(i%modVal))
      {
         cout << 10*i/(modVal*10) << flush;
      }

      for (k=0; k<bands; k++)
      {
         if (preLineBytes != 0)
         {
            fwrite(&dummyValue, preLineBytes, 1, pStream);
         }

         for (j=0; j<columns; j++)
         {
            pBuffer[j] = getValue<T>(i, j, k, rows, columns, bands);
         }
         endianSwap(pBuffer, columns, littleEndian);
         fwrite(pBuffer, columns, sizeof(T), pStream);

         if (postLineBytes != 0)
         {
            fwrite(&dummyValue, postLineBytes, 1, pStream);
         }
      }
   }

   if (trailerBytes)
   {
      fwrite(&dummyValue, trailerBytes, 1, pStream);
   }

   fclose(pStream);

   writeENVIHeader(filenameBase, filename, rows,
      columns, bands, littleEndian, headerBytes, trailerBytes,
      preLineBytes, postLineBytes, 0, 0,
      "bil", getEnviDataType(reinterpret_cast<T*>(NULL)));
}

template<typename T>
static void writeBSQ(const string& filenameBase,
                     const unsigned int& rows,
                     const unsigned int& columns,
                     const unsigned int& bands,
                     bool littleEndian, 
                     const unsigned int& headerBytes,
                     const unsigned int& trailerBytes,
                     const unsigned int& preLineBytes,
                     const unsigned int& postLineBytes,
                     const unsigned int& preBandBytes,
                     const unsigned int& postBandBytes)
{
   unsigned int i, j, k;
   int modVal;
   vector<T> bufferRes(rows*columns);
   T* pBuffer = &bufferRes.front();
   FILE* pStream;
   char dummyValue = 1;

   string filename = filenameBase + ".bsq";
   pStream = fopen(filename.c_str(), "wb");

   if (headerBytes)
   {
      fwrite(&dummyValue, headerBytes, 1, pStream);
   }

   modVal = bands/10;
   if (modVal == 0)
   {
      modVal = 1;
   }
   for (k=0; k<bands; k++)
   {
      if (!(k%(modVal)))
      {
         cout << 10*k/(modVal*10) << flush;
      }

      if (preBandBytes != 0)
      {
         fwrite(&dummyValue, preBandBytes, 1, pStream);
      }

      for (i=0; i<rows; i++)
      {
         if (preLineBytes != 0)
         {
            fwrite(&dummyValue, preLineBytes, 1, pStream);
         }

         for (j=0; j<columns; j++)
         {
            pBuffer[j] = getValue<T>(i, j, k, rows, columns, bands);
         }
         endianSwap(pBuffer, columns, littleEndian);
         fwrite(pBuffer, columns, sizeof(T), pStream);

         if (postLineBytes != 0)
         {
            fwrite(&dummyValue, postLineBytes, 1, pStream);
         }
      }

      if (postBandBytes != 0)
      {
         fwrite(&dummyValue, postBandBytes, 1, pStream);
      }
   }

   if (trailerBytes)
   {
      fwrite(&dummyValue, trailerBytes, 1, pStream);
   }

   fclose(pStream);

   writeENVIHeader(filenameBase, filename, rows,
      columns, bands, littleEndian, headerBytes, trailerBytes,
      preLineBytes, postLineBytes, preBandBytes, postBandBytes,
      "bsq", getEnviDataType(reinterpret_cast<T*>(NULL)));
}

template<typename T>
static void writeBSQMulti(const string& filenameBase,
                          const unsigned int& rows,
                          const unsigned int& columns,
                          const unsigned int& bands,
                          bool littleEndian, 
                          const unsigned int& headerBytes,
                          const unsigned int& trailerBytes,
                          const unsigned int& preLineBytes,
                          const unsigned int& postLineBytes,
                          const unsigned int& preBandBytes,
                          const unsigned int& postBandBytes)
{
   unsigned int i, j, k;
   int modVal;
   vector<T> bufferRes(rows*columns);
   T* pBuffer = &bufferRes.front();
   FILE* pStream;
   char dummyValue = 1;

   string headerFilename = filenameBase + ".bsq.hdr";
   pStream = fopen(headerFilename.c_str(), "wt");

   fprintf(pStream, "BAND Format: %s\n", "BSQMulti");
   fprintf(pStream, "ROWS: %d\n", rows);
   fprintf(pStream, "COLUMNS: %d\n", columns);
   fprintf(pStream, "BANDS: %d\n", bands);
   fprintf(pStream, "HEADERBYTES: %d\n", headerBytes);
   fprintf(pStream, "TRAILERBYTES: %d\n", trailerBytes);
   fprintf(pStream, "PREBANDBYTES: %d\n", preBandBytes);
   fprintf(pStream, "POSTBANDBYTES: %d\n", postBandBytes);
   fprintf(pStream, "PRELINEBYTES: %d\n", preLineBytes);
   fprintf(pStream, "POSTLINEBYTES: %d\n", postLineBytes);

   fclose(pStream);

   modVal = bands/10;
   if (modVal == 0)
   {
      modVal = 1;
   }
   for (k=0; k<bands; k++)
   {
      ostringstream filenameStream;
      filenameStream << filenameBase << ".bsq.raw" << setw(3) << setfill('0') << k;
      pStream = fopen(filenameStream.str().c_str(), "wb");

      if (!(k%(modVal)))
      {
         cout << 10*k/(modVal*10) << flush;
      }

      if (headerBytes)
      {
         fwrite(&dummyValue, headerBytes, 1, pStream);
      }

      if (preBandBytes != 0)
      {
         fwrite(&dummyValue, preBandBytes, 1, pStream);
      }

      for (i=0; i<rows; i++)
      {
         if (preLineBytes != 0)
         {
            fwrite(&dummyValue, preLineBytes, 1, pStream);
         }

         for (j=0; j<columns; j++)
         {
            pBuffer[j] = getValue<T>(i, j, k, rows, columns, bands);
         }
         endianSwap(pBuffer, columns, littleEndian);
         fwrite(pBuffer, columns, sizeof(T), pStream);

         if (postLineBytes != 0)
         {
            fwrite(&dummyValue, postLineBytes, 1, pStream);
         }
      }

      if (postBandBytes != 0)
      {
         fwrite(&dummyValue, postBandBytes, 1, pStream);
      }

      if (trailerBytes)
      {
         fwrite(&dummyValue, trailerBytes, 1, pStream);
      }

      fclose(pStream);
   }

   char dir[256];
   getcwd(dir, 256);
   string txtFilename = filenameBase + ".bsq.txt";
   pStream = fopen(txtFilename.c_str(), "w");
   for (k=0; k<bands; k++)
   {
      fprintf(pStream, "%s\\%s.bsq.raw%03d\n", dir, filenameBase.c_str(), k);
   }

   fclose(pStream);
}

static inline double getMin(unsigned char v)   {return numeric_limits<unsigned char>::min();}
static inline double getMin(char v)            {return numeric_limits<char>::min();}
static inline double getMin(unsigned short v)  {return numeric_limits<unsigned short>::min();}
static inline double getMin(short v)           {return numeric_limits<short>::min();}
static inline double getMin(unsigned int v)    {return numeric_limits<unsigned int>::min();}
static inline double getMin(int v)             {return numeric_limits<int>::min();}
static inline double getMin(float v)           {return 0.0;}
static inline double getMin(double v)          {return 0.0;}
static inline double getMax(unsigned char v)   {return numeric_limits<unsigned char>::max();}
static inline double getMax(char v)            {return numeric_limits<char>::max();}
static inline double getMax(unsigned short v)  {return numeric_limits<unsigned short>::max();}
static inline double getMax(short v)           {return numeric_limits<short>::max();}
static inline double getMax(unsigned int v)    {return numeric_limits<unsigned int>::max();}
static inline double getMax(int v)             {return numeric_limits<int>::max();}
static inline double getMax(float v)           {return 10000.0;}
static inline double getMax(double v)          {return 1.0;}

static char* getName(unsigned char*)  {return "1u";}
static char* getName(char*)           {return "1s";}
static char* getName(unsigned short*) {return "2u";}
static char* getName(short*)          {return "2s";}
static char* getName(unsigned int*)   {return "4u";}
static char* getName(int*)            {return "4s";}
static char* getName(float*)          {return "4f";}
static char* getName(double*)         {return "8f";}
static char* getName(IntegerComplex*) {return "4complex";}
static char* getName(FloatComplex*)   {return "8complex";}
