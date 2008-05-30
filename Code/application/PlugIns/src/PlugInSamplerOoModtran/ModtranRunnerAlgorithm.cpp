/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#include "ApplicationServices.h"
#include "ConfigurationSettings.h"
#include "AppAssert.h"
#include "MessageLogResource.h"
#include "ModtranRunnerAlgorithm.h"
#ifdef MICRON
#undef MICRON
#endif
#include "OoModtran.H"
#include "UtilityServices.h"

#include <sstream>
#include <string>

ModtranRunnerAlgorithm::ModtranRunnerAlgorithm(RasterElement* pRasterElement, Progress* pProgress, bool interactive) :
   AlgorithmPattern(pRasterElement, pProgress, interactive, NULL)
{
}

bool ModtranRunnerAlgorithm::initialize(void *pAlgorithmData)
{
   return true;
}

bool ModtranRunnerAlgorithm::preprocess()
{
   return true;
}

bool ModtranRunnerAlgorithm::processAll()
{
   try
   {
      // get path to the modtran band model data
      string modtranData;
      const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
      if (pSupportFilesPath != NULL)
      {
#if defined(SOLARIS)
         modtranData = pSupportFilesPath->getFullPathAndName() + "/Modtran/DATA-sun/";
#else
         modtranData = pSupportFilesPath->getFullPathAndName() + "/Modtran/DATA/";
#endif
      }

      // Unfortunately the OoModtran API takes a char* so we have to cast away
      // const. It's ok since OoModtran doesn't modify the input char data. 
      OoModtran modtran(const_cast<char*>(modtranData.c_str()), false, false, false, false);

      int MODEL = 7;
      int ITYPE = 2;
      int IEMSC = 2;
      int IMULT = 1;
      int M1 = 0;
      int M2 = 0;
      int M3 = 0;
      int M4 = 0;
      int M5 = 0;
      int M6 = 0;
      int MDEF = 0;
      int IM = 1;
      int NOPRNT = 0;
      float TPTEMP = 0.0f;

      modtran.setCard1(OoModtran::MODTRAN, ' ', false, MODEL, ITYPE, IEMSC, IMULT,
                       M1, M2, M3, M4, M5, M6, MDEF, IM, false, NOPRNT, TPTEMP, "1.00");

      char DIS = 'F';
      char DISAZM = 'F';
      char DISALB = 'F';
      int NSTR = 4;
      float SFWHM = 0.0f;
      float CO2MX = 355.0f;
      char LSUNFL = 'F';
      char LBMNAM = 'T';   // get root name of band model from card 1A2
      char LFLTNM = 'F';
      char H2OAER = 'F';
      char CDTDIR = 'T';   // get path to data directory from card 1A4



      modtran.setCard1A(DIS, DISAZM, DISALB, NSTR, SFWHM, CO2MX, " ", " ", 
                        LSUNFL, LBMNAM, LFLTNM, H2OAER, CDTDIR, 0.0f, ' ', 
                        0.0f, 0.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f);

      modtran.setCard1A2("01_2004");

      int IHAZE = 2;
      int ISEASN = 1;
      int IVULCN = 2;
      int ICSTL = 0;
      int ICLD = 0;
      float VIS = 23.0f;
      float GNDALT = 0.243f;

      modtran.setCard2(" ", IHAZE, ' ', ISEASN, " ", IVULCN, ICSTL, ICLD, 0, 
                       VIS, 0.0f, 0.0f, 0.0f, GNDALT);

      const int ML = 33;
      int IRD1 = 0;
      int IRD2 = 0;
      float REE = 0.0f;
      int NMOLY = 0;

      modtran.setCard2C(ML, IRD1, IRD2, "created in AERO.f", REE, NMOLY);

      float zm[ML] = {0.243f, 1.009f, 2.007f, 3.005f, 4.004f, 5.002f, 6.0f, 7.0f, 8.0f, 9.0f,
                      10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 
                      20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 30.0f, 35.0f, 40.0f, 50.0f, 
                      60.0f, 70.0f, 100.0f};

      float p[ML] = {985.966f, 901.0f, 801.3f, 709.5f, 627.7f, 553.9f, 487.0f, 426.0f, 372.0f, 
                     324.0f, 281.0f, 243.0f, 209.0f, 179.0f, 153.0f, 130.0f, 111.0f, 95.0f, 
                     81.2f, 69.5f, 59.5f, 51.0f, 43.7f, 36.7f, 32.2f, 27.7f, 13.2f, 6.52f, 
                     3.33f, 0.95f, 0.27f, 0.07f, 0.001f};

      float t[ML] = {293.106f, 289.66f, 285.16f, 279.17f, 273.18f, 267.19f, 261.2f, 254.7f, 
                     248.2f, 241.7f, 235.3f, 228.8f, 222.3f, 215.8f, 215.7f, 215.7f, 215.7f, 
                     215.7f, 216.8f, 217.9f, 219.2f, 220.4f, 221.6f, 222.8f, 223.9f, 225.1f, 
                     233.7f, 245.2f, 257.5f, 275.7f, 257.1f, 218.1f, 190.5f};

      float wmol2c1_0[ML]={36.431f, 32.232f, 26.948f, 22.119f, 19.077f, 15.355f, 14.657f, 
                           14.817f, 14.486f, 14.737f, 14.393f, 9.524f, 5.228f, 2.652f, 1.434f, 
                           0.829f, 0.687f, 0.57f, 0.419f, 0.318f, 1.82f, 0.92f, 0.5f, 0.33f, 
                           0.24f, 0.16f, 0.03f, 0.01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

      float wmol2c1_1[ML]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
                           0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
                           0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

      float wmol2c1_2[ML]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
                           0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
                           0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

      float *wmol2c1[3] = {wmol2c1_0,wmol2c1_1,wmol2c1_2};

      char pType[ML] = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 
                        'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 
                        'A', 'A', 'A', 'A', 'A', '2', '2'};

      char tType[ML] = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 
                        'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 
                        'A', 'A', 'A', 'A', 'A', '2', '2'};

      char mType_0[ML]={'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 
                        'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 
                        'H', 'H', '2', '2', '2', '2', '2'};

      char mType_1[ML]={'2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2'};

      char mType_2[ML]={'2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2'};

      char mType_3[ML]={'2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2'};

      char mType_4[ML]={'2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2'};

      char mType_5[ML]={'2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2'};

      char mType_6[ML]={'2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', 
                        '2', '2', '2', '2', '2', '2', '2'};

      char mType_7[ML]={'6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                        '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                        '6', '6', '6', '6', '6', '6', '6'};

      char mType_8[ML]={'6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                        '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                        '6', '6', '6', '6', '6', '6', '6'};

      char mType_9[ML]={'6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                        '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                        '6', '6', '6', '6', '6', '6', '6'};

      char mType_10[ML]={'6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                         '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                         '6', '6', '6', '6', '6', '6', '6'};

      char mType_11[ML]={'6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                         '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', '6', 
                         '6', '6', '6', '6', '6', '6', '6'};

      char *mType[12] = {mType_0,mType_1,mType_2,mType_3,mType_4,mType_5,mType_6,mType_7,
                         mType_8,mType_9,mType_10,mType_11};

      modtran.setCard2C1(ML, zm, p, t, wmol2c1, pType, tType, mType);

      float H1 = 1.573f;
      float H2 = 0.243f;
      float ANGLE = 180.0f;
      float RANGE = 0.0f;

      modtran.setCard3(H1, H2, ANGLE, RANGE, 0.0f, 0.0f, 0, 0.0f, 0.0f);

      int IPARM = 1;
      int IPH = 2;
      int IDAY = 177;
      int ISOURC = 0;

      modtran.setCard3a1(IPARM, IPH, IDAY, ISOURC);

      float PARM1 = 32.965f;
      float PARM2 = 114.269f;
      float PARM3 = 0.0f;
      float PARM4 = 0.0f;
      float TIME = 15.867f;
      float PSIPO = 0.0f;
      float ANGLEM = 0.0f;
      float HenyeyGreen = 0.0f;

      modtran.setCard3a2(PARM1, PARM2, PARM3, PARM4, TIME, PSIPO, ANGLEM, HenyeyGreen);

      float V1 = 3770.0f;
      float V2 = 28320.0f;
      float DV = 2.0f;
      float FWHM = 2.0f;

      modtran.setCard4(V1, V2, DV, FWHM, ' ', ' ', " ", "       ", 0);

      StepResource pStep("MODTRAN run", "app", "C1F8C62A-E85E-4FC5-A2C2-FE2AD1DF9BB8");
      OoModtran::ExitStatus result;
      result = modtran.compute();
      if (result == OoModtran::SUCCESS && pStep.get() != NULL)
      {
         int n = 0;
         float *pFrequency = modtran.getFrequency(n);
         float *pWavelength = modtran.getWavelength(n);
         float *pTrans = modtran.getTotalTransmission(n);
         float *pRad = modtran.getRadiance(OoModtran::INVCM,n);

         if (pFrequency == NULL || pWavelength == NULL || pTrans == NULL || pRad == NULL)
         {
            pStep->finalize(Message::Failure);
            return false;
         }

         stringstream outputStr;
         pStep->addProperty("Line_Number = Frequency, Wavelength, Transmittance, Radiance","");
         for (int i=0; (i<n && i<5); ++i)
         {
            outputStr.str("");
            outputStr << "ResultsLine" << i+1 << " = " << pFrequency[i] << ", " << 
               pWavelength[i] << ", " << pTrans[i] << ", " << pRad[i];
            pStep->addProperty(outputStr.str(), "");
         }
         pStep->finalize();

         return true;
      }
   }
   catch(const AssertException &)
   {
      // getting a service failed
      return false;
   }

   return false;
}

bool ModtranRunnerAlgorithm::postprocess()
{
   return true;
}

bool ModtranRunnerAlgorithm::canAbort() const
{
   return false;
}

bool ModtranRunnerAlgorithm::doAbort()
{
   return false;
}
