/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "ConfigurationSettings.h"
#include "ExecutableAgent.h"
#include "ObjectFactory.h"
#include "SimpleApiErrors.h"
#include "SimpleExecution.h"
#include "TypeConverter.h"
#include "WizardItem.h"
#include "WizardNode.h"
#include "WizardObject.h"
#include "WizardUtilities.h"
#include <stdexcept>

extern "C"
{
   ExecutableAgent* createPlugIn(const char* pName, int batch)
   {
      std::string name(pName == NULL ? "" : pName);
      ObjectFactory* pFact = Service<ApplicationServices>()->getObjectFactory();
      ExecutableAgent* pPlugin =
         reinterpret_cast<ExecutableAgent*>(pFact->createObject(TypeConverter::toString<ExecutableAgent>()));
      if (pPlugin == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      pPlugin->instantiate(name, std::string(), NULL, (batch != 0));
      if (pPlugin->getPlugIn() == NULL)
      {
         freePlugIn(pPlugin);
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      pPlugin->createProgressDialog(true);
      pPlugin->setAutoArg(false);
      setLastError(SIMPLE_NO_ERROR);
      return pPlugin;
   }

   void freePlugIn(ExecutableAgent* pPlugin)
   {
      ObjectFactory* pFact = Service<ApplicationServices>()->getObjectFactory();
      if (pFact != NULL)
      {
         pFact->destroyObject(pPlugin, TypeConverter::toString<ExecutableAgent>());
      }
   }

   PlugInArgList* getPlugInInputArgList(ExecutableAgent* pPlugin)
   {
      try
      {
         if (pPlugin == NULL)
         {
            setLastError(SIMPLE_BAD_PARAMS);
            return NULL;
         }
         PlugInArgList* pPial = &pPlugin->getInArgList();
         setLastError(SIMPLE_NO_ERROR);
         return pPial;
      }
      catch(std::logic_error&)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }
   }

   PlugInArgList* getPlugInOutputArgList(ExecutableAgent* pPlugin)
   {
      try
      {
         if (pPlugin == NULL)
         {
            setLastError(SIMPLE_BAD_PARAMS);
            return NULL;
         }
         PlugInArgList* pPial = &pPlugin->getOutArgList();
         setLastError(SIMPLE_NO_ERROR);
         return pPial;
      }
      catch(std::logic_error&)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }
   }

   int executePlugIn(ExecutableAgent* pPlugin)
   {
      try
      {
         if (pPlugin == NULL)
         {
            setLastError(SIMPLE_BAD_PARAMS);
            return NULL;
         }
         setLastError(SIMPLE_NO_ERROR);
         return pPlugin->execute();
      }
      catch(std::logic_error&)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 0;
      }
   }

   WizardObject* loadWizard(const char* pFilename)
   {
      if (pFilename == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string filename(pFilename);
      WizardObject* pWizard = WizardUtilities::readWizard(filename);
      if (pWizard == NULL)
      {
         // open using a path relative to the default wizard path
         const Filename* pFilename = Service<ConfigurationSettings>()->getSettingWizardPath();
         if (pFilename != NULL)
         {
            filename = pFilename->getFullPathAndName() + "/" + filename;
            pWizard = WizardUtilities::readWizard(filename);
         }
      }
      if (pWizard == NULL)
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pWizard;
   }

   void freeWizard(WizardObject* pWizard)
   {
      ObjectFactory* pFact = Service<ApplicationServices>()->getObjectFactory();
      if (pFact != NULL)
      {
         pFact->destroyObject(pWizard, TypeConverter::toString<WizardObject>());
      }
   }

   uint32_t getWizardName(WizardObject* pWizard, char* pName, uint32_t nameSize)
   {
      if (pWizard == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pWizard->getName();
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   uint32_t getWizardInputNodeCount(WizardObject* pWizard)
   {
      if (pWizard == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      uint32_t cnt = 0;
      const std::vector<WizardItem*>& items = pWizard->getItems();
      for (std::vector<WizardItem*>::const_iterator item = items.begin(); item != items.end(); ++item)
      {
         WizardItem* pItem = *item;
         if (pItem->getType() == "Value")
         {
            ++cnt;
         }
      }
      setLastError(SIMPLE_NO_ERROR);
      return cnt;
   }

   uint32_t getWizardOutputNodeCount(WizardObject* pWizard)
   {
      if (pWizard == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      uint32_t cnt = 0;
      const std::vector<WizardItem*>& items = pWizard->getItems();
      for (std::vector<WizardItem*>::const_iterator item = items.begin(); item != items.end(); ++item)
      {
         WizardItem* pItem = *item;
         if (pItem->getType() != "Value")
         {
            cnt += pItem->getOutputNodes().size();
         }
      }
      setLastError(SIMPLE_NO_ERROR);
      return cnt;
   }

   WizardNode* getWizardInputNodeByIndex(WizardObject* pWizard, uint32_t idx)
   {
      if (pWizard == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      uint32_t cnt = 0;
      const std::vector<WizardItem*>& items = pWizard->getItems();
      for (std::vector<WizardItem*>::const_iterator item = items.begin(); item != items.end(); ++item)
      {
         WizardItem* pItem = *item;
         if (pItem->getType() == "Value")
         {
            if (cnt == idx)
            {
               setLastError(SIMPLE_NO_ERROR);
               return pItem->getOutputNodes().front();
            }
            ++cnt;
         }
      }
      setLastError(SIMPLE_NOT_FOUND);
      return NULL;
   }

   WizardNode* getWizardInputNodeByName(WizardObject* pWizard, const char* pName)
   {
      if (pWizard == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string name(pName);
      const std::vector<WizardItem*>& items = pWizard->getItems();
      for (std::vector<WizardItem*>::const_iterator item = items.begin(); item != items.end(); ++item)
      {
         WizardItem* pItem = *item;
         if (pItem->getType() == "Value" && pItem->getName() == name)
         {
            setLastError(SIMPLE_NO_ERROR);
            return pItem->getOutputNodes().front();
         }
      }
      setLastError(SIMPLE_NOT_FOUND);
      return NULL;
   }

   WizardNode* getWizardOutputNodeByIndex(WizardObject* pWizard, uint32_t idx)
   {
      if (pWizard == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      uint32_t cnt = 0;
      const std::vector<WizardItem*>& items = pWizard->getItems();
      for (std::vector<WizardItem*>::const_iterator item = items.begin(); item != items.end(); ++item)
      {
         WizardItem* pItem = *item;
         if (pItem->getType() != "Value")
         {
            const std::vector<WizardNode*>& nodes = pItem->getOutputNodes();
            if (idx >= cnt && idx < (cnt + nodes.size()))
            {
               setLastError(SIMPLE_NO_ERROR);
               return nodes[idx - cnt];
            }
            cnt += nodes.size();
         }
      }
      setLastError(SIMPLE_NOT_FOUND);
      return NULL;
   }

   WizardNode* getWizardOutputNodeByName(WizardObject* pWizard, const char* pName)
   {
      if (pWizard == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string name(pName);
      std::string::size_type idx = name.find_first_of("|");
      if (idx == std::string::npos)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string itemName = name.substr(0, idx);
      std::string nodeName = name.substr(idx + 1);

      const std::vector<WizardItem*>& items = pWizard->getItems();
      for (std::vector<WizardItem*>::const_iterator item = items.begin(); item != items.end(); ++item)
      {
         WizardItem* pItem = *item;
         if (pItem->getType() != "Value" && pItem->getName() == itemName)
         {
            const std::vector<WizardNode*>& nodes = pItem->getOutputNodes();
            for (std::vector<WizardNode*>::const_iterator node = nodes.begin(); node != nodes.end(); ++node)
            {
               WizardNode* pNode = *node;
               if (pNode->getName() == nodeName)
               {
                  setLastError(SIMPLE_NO_ERROR);
                  return pNode;
               }
            }
         }
      }
      setLastError(SIMPLE_NOT_FOUND);
      return NULL;
   }

   int executeWizard(WizardObject* pWizard)
   {
      if (pWizard == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      WizardUtilities::runWizard(pWizard);
      return 1;
   }

   uint32_t getWizardNodeName(WizardNode* pNode, char* pName, uint32_t nameSize)
   {
      if (pNode == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pNode->getName();
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   uint32_t getWizardNodeType(WizardNode* pNode, char* pType, uint32_t typeSize)
   {
      if (pNode == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type = pNode->getType();
      if (typeSize == 0)
      {
         return type.size() + 1;
      }
      else if (type.size() > typeSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pType, type.c_str(), type.size());
      pType[type.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return type.size();
   }

   void* getWizardNodeValue(WizardNode* pNode)
   {
      if (pNode == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pNode->getValue();
   }

   int setWizardNodeValue(WizardNode* pNode, void* pValue)
   {
      if (pNode == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      pNode->setValue(pValue);
      setLastError(SIMPLE_NO_ERROR);
      return 1;
   }
}
