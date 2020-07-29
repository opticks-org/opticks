/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TESTBEDTESTUTILITIES_H
#define TESTBEDTESTUTILITIES_H

#include <string>
#include <vector>

class DataElement;
class RasterElement;
class SpatialDataWindow;
class TestSuiteFactory;
class ThresholdLayer;
class WorkspaceWindow;

namespace TestUtilities
{
   bool runGeoRef(RasterElement* pRasterElement, const std::string& pluginName = "GCP Georeference");
   ThresholdLayer* createThresholdLayer(RasterElement* pRasterElement, unsigned char threshold, int valuesOver);
   SpatialDataWindow* loadDataSet(const std::string& cubeName, const std::string& importerName,
      SpatialDataWindow* pWindow = NULL);
   std::vector<DataElement*> loadDataSet(const std::string& cubeName, const std::string& importerName,
      const unsigned int& minNumCubes);
   RasterElement* getStandardRasterElement(bool cleanLoad = false, bool loadFromTempDir = false);
   bool destroyWorkspaceWindow(WorkspaceWindow* pWindow);
   std::vector<TestSuiteFactory*>& getFactoryVector();
}

#endif
