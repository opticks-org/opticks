/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SYMBOLMANAGER_H
#define SYMBOLMANAGER_H

#include "LocationType.h"

#include <map>
#include <string>

class GraphicObject;

class SymbolManager
{
public:
   static SymbolManager *instance();

   void drawSymbols(const std::string &symbolName, const std::vector<LocationType> &points, 
      double screenSize, double zoomFactor = 1.0, double rotation = 0.0, 
      double pitch = 90.0, double xScale = 1.0, double yScale = 1.0, double objectRotation = 0.0);

   const QImage &getSymbolImage(const std::string &symbolName);

   const std::vector<std::string> &getAvailableSymbolNames() const;

private:
   SymbolManager(void);
   ~SymbolManager(void);

   static SymbolManager* mpSingleton;

   GraphicObject *getSymbol(const std::string &symbolName);
   GraphicObject *loadSymbol(const std::string &symbolName);

   std::map<std::string, GraphicObject*> mSymbols;

};

#endif
