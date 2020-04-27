/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataPlotterDlg.h"
#include "DataPlotterPlugIn.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "ModelServices.h"
#include "PlugInRegistration.h"
#include "Signature.h"

#include <vector>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, DataPlotterPlugIn);

DataPlotterPlugIn::DataPlotterPlugIn() :
   mInteractive(true)
{
   AlgorithmShell::setName("Data Plotter");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Demonstrates use of portions of the plotting API.");
   setMenuLocation("[Demo]\\Data Plotter");
   setDescriptorId("{69DD0E12-8D6B-4e23-A742-469447E4EC2F}");
   allowMultipleInstances(true);
   setWizardSupported(false);
}

DataPlotterPlugIn::~DataPlotterPlugIn()
{}

bool DataPlotterPlugIn::setBatch()
{
   mInteractive = false;
   return false;
}

bool DataPlotterPlugIn::setInteractive()
{
   mInteractive = true;
   return true;
}

bool DataPlotterPlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return mInteractive;
}

bool DataPlotterPlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return mInteractive;
}

namespace
{
double sBandNumbers[] = {
1.0,
2.0,
3.0,
4.0,
5.0,
6.0,
7.0,
8.0,
9.0,
10.0,
11.0,
12.0,
13.0,
14.0,
15.0,
16.0,
17.0,
18.0,
19.0,
20.0,
21.0,
22.0,
23.0,
24.0,
25.0,
26.0,
27.0,
28.0,
29.0,
30.0,
31.0,
32.0,
33.0,
34.0,
35.0,
36.0,
37.0,
38.0,
39.0,
40.0,
41.0,
42.0,
43.0,
44.0,
45.0,
46.0,
47.0,
48.0,
49.0,
50.0,
51.0,
52.0,
53.0,
54.0,
55.0,
56.0,
57.0,
58.0,
59.0,
60.0,
61.0,
62.0,
63.0,
64.0,
65.0,
66.0,
67.0,
68.0,
69.0,
70.0,
71.0,
72.0,
73.0,
74.0,
75.0,
76.0,
77.0,
78.0,
79.0,
80.0,
81.0,
82.0,
83.0,
84.0,
85.0,
86.0,
87.0,
88.0,
89.0,
90.0,
91.0,
92.0,
93.0,
94.0,
95.0,
96.0,
97.0,
98.0,
99.0,
100.0,
101.0,
102.0,
103.0,
104.0,
105.0,
106.0,
107.0,
108.0,
109.0,
110.0,
111.0,
112.0,
113.0,
114.0,
115.0,
116.0,
117.0,
118.0,
119.0,
120.0,
121.0,
122.0,
123.0,
124.0,
125.0,
126.0,
127.0,
128.0,
129.0,
130.0,
131.0,
132.0,
133.0,
134.0,
135.0,
136.0,
137.0,
138.0,
139.0,
140.0,
141.0,
142.0,
143.0,
144.0,
145.0,
146.0,
147.0,
148.0,
149.0,
150.0,
151.0,
152.0,
153.0,
154.0,
155.0,
156.0,
157.0,
158.0,
159.0,
160.0,
161.0,
162.0,
163.0,
164.0,
165.0,
166.0,
167.0,
168.0 };

double sWavelengths[] = {
0.413967,
0.417388,
0.420847,
0.424348,
0.427893,
0.431485,
0.435128,
0.438823,
0.442574,
0.446385,
0.450257,
0.454195,
0.458201,
0.462279,
0.466431,
0.470620,
0.474975,
0.479374,
0.483862,
0.488443,
0.493121,
0.497901,
0.502786,
0.507781,
0.512890,
0.518119,
0.523472,
0.528953,
0.534568,
0.540323,
0.546222,
0.552270,
0.558474,
0.564839,
0.571371,
0.578075,
0.584957,
0.592024,
0.599281,
0.606734,
0.614388,
0.622250,
0.630324,
0.638616,
0.647131,
0.655874,
0.664849,
0.674060,
0.683511,
0.693204,
0.703141,
0.713326,
0.723758,
0.734437,
0.745364,
0.756537,
0.767954,
0.779612,
0.791507,
0.803634,
0.815988,
0.828563,
0.841351,
0.854344,
0.867534,
0.880912,
0.894467,
0.908191,
0.922072,
0.936100,
0.950263,
0.964550,
0.978950,
0.993452,
1.008000,
1.022720,
1.037460,
1.052260,
1.067100,
1.081990,
1.096900,
1.111830,
1.126770,
1.141720,
1.156660,
1.171580,
1.186490,
1.201370,
1.216210,
1.231020,
1.245790,
1.260510,
1.275180,
1.289790,
1.304340,
1.318830,
1.459760,
1.473420,
1.487000,
1.500500,
1.513920,
1.527260,
1.540510,
1.553690,
1.566780,
1.579780,
1.592710,
1.605560,
1.618320,
1.631010,
1.643620,
1.656140,
1.668590,
1.680960,
1.693260,
1.705470,
1.717620,
1.729680,
1.741670,
1.753590,
1.765440,
1.777210,
1.788920,
1.988270,
1.998760,
2.009200,
2.019580,
2.029910,
2.040180,
2.050400,
2.060560,
2.070670,
2.080730,
2.090740,
2.100700,
2.110610,
2.120470,
2.130280,
2.140040,
2.149750,
2.159420,
2.169030,
2.178610,
2.188130,
2.197610,
2.207050,
2.216440,
2.225790,
2.235090,
2.244360,
2.253580,
2.262750,
2.271890,
2.280980,
2.290040,
2.299050,
2.308020,
2.316960,
2.325850,
2.334710,
2.343530,
2.352310,
2.361050,
2.369760,
2.378430,
2.387060,
2.395660,
2.404220 };

double sRadiances[] = {
1000.0/2357.0,
1000.0/2220.0,
1000.0/2184.0,
1000.0/2158.0,
1000.0/2127.0,
1000.0/2214.0,
1000.0/2187.0,
1000.0/2173.0,
1000.0/2158.0,
1000.0/2203.0,
1000.0/2204.0,
1000.0/2212.0,
1000.0/2220.0,
1000.0/2212.0,
1000.0/2206.0,
1000.0/2191.0,
1000.0/2223.0,
1000.0/2236.0,
1000.0/2248.0,
1000.0/2351.0,
1000.0/2403.0,
1000.0/2534.0,
1000.0/2590.0,
1000.0/2568.0,
1000.0/2588.0,
1000.0/2653.0,
1000.0/2694.0,
1000.0/2727.0,
1000.0/2764.0,
1000.0/2816.0,
1000.0/2882.0,
1000.0/2966.0,
1000.0/3088.0,
1000.0/3274.0,
1000.0/3481.0,
1000.0/3727.0,
1000.0/3929.0,
1000.0/4191.0,
1000.0/4154.0,
1000.0/4183.0,
1000.0/4293.0,
1000.0/4293.0,
1000.0/4274.0,
1000.0/4271.0,
1000.0/4274.0,
1000.0/4272.0,
1000.0/4279.0,
1000.0/4261.0,
1000.0/4208.0,
1000.0/4260.0,
1000.0/4247.0,
1000.0/4177.0,
1000.0/4182.0,
1000.0/4265.0,
1000.0/4164.0,
1000.0/4033.0,
1000.0/4192.0,
1000.0/4075.0,
1000.0/4004.0,
1000.0/3962.0,
1000.0/3922.0,
1000.0/4004.0,
1000.0/3954.0,
1000.0/3897.0,
1000.0/3870.0,
1000.0/3792.0,
1000.0/3720.0,
1000.0/3775.0,
1000.0/3720.0,
1000.0/3712.0,
1000.0/4004.0,
1000.0/3958.0,
1000.0/3794.0,
1000.0/3720.0,
1000.0/3707.0,
1000.0/3671.0,
1000.0/3726.0,
1000.0/3575.0,
1000.0/3509.0,
1000.0/3585.0,
1000.0/3371.0,
1000.0/3305.0,
1000.0/3163.0,
1000.0/3346.0,
1000.0/3442.0,
1000.0/3394.0,
1000.0/3297.0,
1000.0/3253.0,
1000.0/3257.0,
1000.0/3227.0,
1000.0/3187.0,
1000.0/3122.0,
1000.0/3166.0,
1000.0/3146.0,
1000.0/3077.0,
1000.0/3128.0,
1000.0/3036.0,
1000.0/2917.0,
1000.0/2986.0,
1000.0/2933.0,
1000.0/2867.0,
1000.0/2844.0,
1000.0/2828.0,
1000.0/2796.0,
1000.0/2773.0,
1000.0/2769.0,
1000.0/2778.0,
1000.0/2757.0,
1000.0/2746.0,
1000.0/2720.0,
1000.0/2690.0,
1000.0/2762.0,
1000.0/2787.0,
1000.0/2760.0,
1000.0/2730.0,
1000.0/2685.0,
1000.0/2680.0,
1000.0/2679.0,
1000.0/2688.0,
1000.0/2692.0,
1000.0/2677.0,
1000.0/2661.0,
1000.0/2639.0,
1000.0/2486.0,
1000.0/2199.0,
1000.0/2097.0,
1000.0/2368.0,
1000.0/2219.0,
1000.0/2213.0,
1000.0/2386.0,
1000.0/2222.0,
1000.0/2296.0,
1000.0/2358.0,
1000.0/2325.0,
1000.0/2268.0,
1000.0/2267.0,
1000.0/2324.0,
1000.0/2358.0,
1000.0/2339.0,
1000.0/2417.0,
1000.0/2396.0,
1000.0/2401.0,
1000.0/2399.0,
1000.0/2353.0,
1000.0/2326.0,
1000.0/2257.0,
1000.0/2178.0,
1000.0/2213.0,
1000.0/2157.0,
1000.0/2149.0,
1000.0/2211.0,
1000.0/2287.0,
1000.0/2157.0,
1000.0/2043.0,
1000.0/1958.0,
1000.0/2064.0,
1000.0/2111.0,
1000.0/2009.0,
1000.0/1999.0,
1000.0/2071.0,
1000.0/1994.0,
1000.0/1900.0,
1000.0/2080.0,
1000.0/1982.0,
1000.0/1842.0,
1000.0/1992.0,
1000.0/2056.0,
1000.0/1970.0 };

double sReflectances[] = {
2357.0/10000.0,
2223.0/10000.0,
2184.0/10000.0,
2158.0/10000.0,
2127.0/10000.0,
2214.0/10000.0,
2187.0/10000.0,
2173.0/10000.0,
2158.0/10000.0,
2203.0/10000.0,
2204.0/10000.0,
2212.0/10000.0,
2223.0/10000.0,
2212.0/10000.0,
2206.0/10000.0,
2191.0/10000.0,
2223.0/10000.0,
2236.0/10000.0,
2248.0/10000.0,
2351.0/10000.0,
2403.0/10000.0,
2534.0/10000.0,
2593.0/10000.0,
2568.0/10000.0,
2588.0/10000.0,
2653.0/10000.0,
2694.0/10000.0,
2727.0/10000.0,
2764.0/10000.0,
2816.0/10000.0,
2882.0/10000.0,
2966.0/10000.0,
3088.0/10000.0,
3274.0/10000.0,
3481.0/10000.0,
3727.0/10000.0,
3929.0/10000.0,
4191.0/10000.0,
4154.0/10000.0,
4183.0/10000.0,
4293.0/10000.0,
4293.0/10000.0,
4274.0/10000.0,
4271.0/10000.0,
4274.0/10000.0,
4272.0/10000.0,
4279.0/10000.0,
4261.0/10000.0,
4208.0/10000.0,
4263.0/10000.0,
4247.0/10000.0,
4177.0/10000.0,
4182.0/10000.0,
4265.0/10000.0,
4164.0/10000.0,
4033.0/10000.0,
4192.0/10000.0,
4075.0/10000.0,
4004.0/10000.0,
3962.0/10000.0,
3922.0/10000.0,
4004.0/10000.0,
3954.0/10000.0,
3897.0/10000.0,
3873.0/10000.0,
3792.0/10000.0,
3723.0/10000.0,
3775.0/10000.0,
3723.0/10000.0,
3712.0/10000.0,
4004.0/10000.0,
3958.0/10000.0,
3794.0/10000.0,
3720.0/10000.0,
3707.0/10000.0,
3674.0/10000.0,
3726.0/10000.0,
3575.0/10000.0,
3509.0/10000.0,
3585.0/10000.0,
3374.0/10000.0,
3305.0/10000.0,
3163.0/10000.0,
3346.0/10000.0,
3442.0/10000.0,
3394.0/10000.0,
3297.0/10000.0,
3253.0/10000.0,
3257.0/10000.0,
3227.0/10000.0,
3187.0/10000.0,
3122.0/10000.0,
3166.0/10000.0,
3146.0/10000.0,
3077.0/10000.0,
3128.0/10000.0,
3036.0/10000.0,
2917.0/10000.0,
2986.0/10000.0,
2933.0/10000.0,
2867.0/10000.0,
2844.0/10000.0,
2828.0/10000.0,
2796.0/10000.0,
2773.0/10000.0,
2769.0/10000.0,
2778.0/10000.0,
2757.0/10000.0,
2746.0/10000.0,
2720.0/10000.0,
2690.0/10000.0,
2762.0/10000.0,
2787.0/10000.0,
2760.0/10000.0,
2730.0/10000.0,
2685.0/10000.0,
2680.0/10000.0,
2679.0/10000.0,
2688.0/10000.0,
2692.0/10000.0,
2677.0/10000.0,
2664.0/10000.0,
2639.0/10000.0,
2486.0/10000.0,
2199.0/10000.0,
2097.0/10000.0,
2368.0/10000.0,
2219.0/10000.0,
2213.0/10000.0,
2386.0/10000.0,
2225.0/10000.0,
2296.0/10000.0,
2358.0/10000.0,
2325.0/10000.0,
2268.0/10000.0,
2267.0/10000.0,
2324.0/10000.0,
2358.0/10000.0,
2339.0/10000.0,
2417.0/10000.0,
2396.0/10000.0,
2401.0/10000.0,
2399.0/10000.0,
2353.0/10000.0,
2326.0/10000.0,
2257.0/10000.0,
2178.0/10000.0,
2213.0/10000.0,
2157.0/10000.0,
2149.0/10000.0,
2211.0/10000.0,
2287.0/10000.0,
2157.0/10000.0,
2043.0/10000.0,
1958.0/10000.0,
2064.0/10000.0,
2111.0/10000.0,
2009.0/10000.0,
1999.0/10000.0,
2071.0/10000.0,
1994.0/10000.0,
1900.0/10000.0,
2080.0/10000.0,
1985.0/10000.0,
1845.0/10000.0,
1995.0/10000.0,
2056.0/10000.0,
1970.0/10000.0 };
}

bool DataPlotterPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (mInteractive == false)
   {
      return false;
   }

   Service<ModelServices> pModel;
   Signature* pSig = static_cast<Signature*>(pModel->getElement("DataPlotter", "Signature", NULL));
   if (!pSig)
   {
      DataDescriptor* pDescriptor = pModel->createDataDescriptor("DataPlotter", "Signature", NULL);
      pSig = static_cast<Signature*>(pModel->createElement(pDescriptor));

      if (!pSig) 
      {
         return false;
      }

      vector<double> vData;

      copy(&sBandNumbers[0], &sBandNumbers[168], back_inserter(vData));
      pSig->setData("Band Number", vData);

      copy(&sWavelengths[0], &sWavelengths[168], vData.begin());
      pSig->setData("Wavelength", vData);

      copy(&sReflectances[0], &sReflectances[168], vData.begin());
      pSig->setData("Reflectance", vData);

      copy(&sRadiances[0], &sRadiances[168], vData.begin());
      pSig->setData("Radiance", vData);
   }

   DataPlotterDlg dlg(*pSig, Service<DesktopServices>()->getMainWidget());
   dlg.exec();

   return true;
}