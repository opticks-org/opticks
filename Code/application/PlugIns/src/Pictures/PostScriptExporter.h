/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POSTSCRIPTEXPORTER_H
#define POSTSCRIPTEXPORTER_H

#include "ExporterShell.h"

#include <string>

class QImage;
class QPoint;
class QSize;

class PostScriptExporter : public ExporterShell
{
public:
   PostScriptExporter();
   ~PostScriptExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   void writePostScriptHeader(const std::string& fname, const QPoint& offset, const QSize& size);
   void writeImageSegment(const QImage& image, const QPoint& offset, double scale = 1.0);
   void writePostScriptFooter();

private:
   void init85();
   void put85(unsigned c);
   void cleanup85();
   void encode(unsigned long tuple, int count);

   FILE* mpOutFile;
   unsigned long mWidth;
   int mPos;
   int mTuple;
   int mCount;
};

#endif
