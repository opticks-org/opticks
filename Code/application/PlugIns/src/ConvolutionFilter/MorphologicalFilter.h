/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MORPHOLOGICALFILTER_H__
#define MORPHOLOGICALFILTER_H__

#include "AlgorithmShell.h"
#include <opencv2/opencv.hpp>

class Progress;

class MorphologicalFilter : public AlgorithmShell
{
public:
   MorphologicalFilter(const std::string& opName);
   virtual ~MorphologicalFilter();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool process() = 0;

   Progress* mpProgress;
   cv::Mat mData;
};

class Dilation : public MorphologicalFilter
{
public:
   Dilation();
   virtual ~Dilation();

private:
   virtual bool process();
};

class Erosion : public MorphologicalFilter
{
public:
   Erosion();
   virtual ~Erosion();

private:
   virtual bool process();
};

class Open : public MorphologicalFilter
{
public:
   Open();
   virtual ~Open();

private:
   virtual bool process();
};

class Close : public MorphologicalFilter
{
public:
   Close();
   virtual ~Close();

private:
   virtual bool process();
};

#endif
