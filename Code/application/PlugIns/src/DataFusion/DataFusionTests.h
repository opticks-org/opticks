/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef DATA_FUSION_TESTS_H
#define DATA_FUSION_TESTS_H

#include "Vector.h"
#include "ProgressTracker.h"

#include <ostream>

class Progress;
class RasterElement;

class Test
{
public:
   Test(std::ostream& output, ProgressTracker &tracker) : mOutputStream(output), mProgressTracker(tracker) {}
   virtual bool run(double pause = 0) = 0;
   const ProgressTracker::Stage& getStage() { return myStage; }
protected:
   std::vector<ProgressTracker::Stage> mStages;
   ProgressTracker::Stage myStage;

   std::ostream& mOutputStream;
   ProgressTracker &mProgressTracker;
};

class PolywarpTests : public Test
{
public:
   PolywarpTests(std::ostream& output, ProgressTracker &tracker);
   bool run(double pause = 0);

private:
   void setupInputMatrices(Vector<double>& XP, Vector<double>& YP, Vector<double>& XS, Vector<double>& YS,
                           Vector<double>& KX, Vector<double>& KY,
                           Vector<double>& ExpectedKX, Vector<double>& ExpectedKY);

   /*
    * Verifies if every value v in the vector fulfills abs(v) < SMALL_VALUE
    */
   bool verifyVector(ProgressTracker::Stage &s, const Vector<double>& results, std::string name);

   bool positiveShiftTest();
   bool negativeShiftTest();
   bool positiveShiftAndScaleTest();
   bool negativeShiftAndScaleTest();
   bool varyXShiftTest();
};

class Poly2DTests : public Test
{
 public:
   Poly2DTests(std::ostream& output, ProgressTracker &tracker);
   bool run(double pause = 0);

   bool identityTest();
   bool positiveShiftTest();
   bool positiveScaleTest();
   bool positiveShiftAndScaleTest();
   

   bool runTest(std::string inputFile, std::string outputFile, std::string testName,
                const Vector<double>& KX, const Vector<double>& KY,
                unsigned int nx, unsigned int ny, unsigned int newx, unsigned int newy);
      
   bool verifyMatrix (ProgressTracker::Stage& s, RasterElement* pResults, RasterElement* pExpected,
                      std::string name);
};

#endif
