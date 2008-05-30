/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BatchFile.h"

using namespace std;

BatchFile::BatchFile()
{
   mbUsed = false;
}

BatchFile::BatchFile(const string& filename)
{
   mFilename = filename.c_str();
   mbUsed = false;
}

BatchFile::~BatchFile()
{
}

void BatchFile::setFileName(const string& filename)
{
   mFilename = filename.c_str();
}

const string& BatchFile::getFileName() const
{
   return mFilename;
}

void BatchFile::setUsed(bool bUsed)
{
   mbUsed = bUsed;
}

bool BatchFile::isUsed() const
{
   return mbUsed;
}
