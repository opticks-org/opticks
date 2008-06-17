/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"

#include "NitfAcftaParser.h"
#include "NitfAcftbParser.h"
#include "NitfAimidaParser.h"
#include "NitfAimidbParser.h"
#include "NitfBandsaParser.h"
#include "NitfBandsbParser.h"
#include "NitfBlockaParser.h"
#include "NitfCmetaaParser.h"
#include "NitfExoptaParser.h"
#include "NitfExpltaParser.h"
#include "NitfExpltbParser.h"
#include "NitfIchipbParser.h"
#include "NitfMensraParser.h"
#include "NitfMensrbParser.h"
#include "NitfMod26aParser.h"
#include "NitfMpd26aParser.h"
#include "NitfPatchaParser.h"
#include "NitfPatchbParser.h"
#include "NitfRadsdaParser.h"
#include "NitfReflnaParser.h"
#include "NitfRpc00aParser.h"
#include "NitfRpc00bParser.h"
#include "NitfSectgaParser.h"
#include "NitfSensraParser.h"
#include "NitfStdidbParser.h"
#include "NitfStdidcParser.h"
#include "NitfUse00aParser.h"
#include "NitfUse26aParser.h"

const char *ModuleManager::mspName = "Common Nitf TRE Parsers";
const char *ModuleManager::mspVersion = "1.0.0";
const char *ModuleManager::mspDescription = "Common Nitf TRE Parsers";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{B3AA207A-5367-49ea-A6B5-C8B289D8F2E5}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 28;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;

   switch (plugInNumber)
      {
      case 0:
         pPlugIn = new Nitf::AcftaParser;
         break;

      case 1:
         pPlugIn = new Nitf::AcftbParser;
         break;

      case 2:
         pPlugIn = new Nitf::AimidaParser;
         break;

      case 3:
         pPlugIn = new Nitf::AimidbParser;
         break;

      case 4:
         pPlugIn = new Nitf::BandsaParser;
         break;

      case 5:
         pPlugIn = new Nitf::BandsbParser;
         break;

      case 6:
         pPlugIn = new Nitf::BlockaParser;
         break;

      case 7:
         pPlugIn = new Nitf::CmetaaParser;
         break;

      case 8:
         pPlugIn = new Nitf::ExoptaParser;
         break;

      case 9:
         pPlugIn = new Nitf::ExpltaParser;
         break;

      case 10:
         pPlugIn = new Nitf::ExpltbParser;
         break;

      case 11:
         pPlugIn = new Nitf::IchipbParser;
         break;

      case 12:
         pPlugIn = new Nitf::MensraParser;
         break;

      case 13:
         pPlugIn = new Nitf::MensrbParser;
         break;

      case 14:
         pPlugIn = new Nitf::Mod26aParser;
         break;

      case 15:
         pPlugIn = new Nitf::Mpd26aParser;
         break;

      case 16:
         pPlugIn = new Nitf::PatchaParser;
         break;

      case 17:
         pPlugIn = new Nitf::PatchbParser;
         break;

      case 18:
         pPlugIn = new Nitf::RadsdaParser;
         break;

      case 19:
         pPlugIn = new Nitf::ReflnaParser;
         break;

      case 20:
         pPlugIn = new Nitf::Rpc00aParser;
         break;

      case 21:
         pPlugIn = new Nitf::Rpc00bParser;
         break;

      case 22:
         pPlugIn = new Nitf::SensraParser;
         break;

      case 23:
         pPlugIn = new Nitf::SectgaParser;
         break;

      case 24:
         pPlugIn = new Nitf::StdidbParser;
         break;

      case 25:
         pPlugIn = new Nitf::StdidcParser;
         break;

      case 26:
         pPlugIn = new Nitf::Use00aParser;
         break;

      case 27:
         pPlugIn = new Nitf::Use26aParser;
         break;

      default:
         break;
      }

   return pPlugIn;
}
