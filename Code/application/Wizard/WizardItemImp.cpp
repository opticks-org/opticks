/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardItemImp.h"
#include "WizardNodeImp.h"
#include "xmlwriter.h"
#include "xmlreader.h"

#include <boost/any.hpp>
#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

WizardItemImp::WizardItemImp(string name, string type)
{
   mName = name;
   mType = type;
   mbBatch = false;
   mbModeSupported = false;
}

WizardItemImp::~WizardItemImp()
{
   notify(SIGNAL_NAME(Subject, Deleted)); // Call before clearing the nodes so that connected objects can access them
   clearInputNodes();
   clearOutputNodes();
}

WizardItemImp& WizardItemImp::operator =(const WizardItemImp& item)
{
   if (this != &item)
   {
      mName = item.mName.c_str();
      mType = item.mType.c_str();
      mbBatch = item.mbBatch;
      mbModeSupported = item.mbModeSupported;
      mCoord = item.mCoord;

      // Input nodes
      clearInputNodes();

      int i = 0;

      int iNodes = 0;
      iNodes = item.mInputNodes.size();
      for (i = 0; i < iNodes; i++)
      {
         WizardNode* pExistNode = NULL;
         pExistNode = item.mInputNodes.at(i);
         if (pExistNode != NULL)
         {
            string nodeName = pExistNode->getName().c_str();
            string nodeType = ((WizardNodeImp*) pExistNode)->getOriginalType().c_str();

            WizardNode* pNode = NULL;
            pNode = addInputNode(nodeName, nodeType);
            if (pNode != NULL)
            {
               *((WizardNodeImp*) pNode) = *((WizardNodeImp*) pExistNode);
            }
         }
      }

      // Output nodes
      clearOutputNodes();

      iNodes = item.mOutputNodes.size();
      for (i = 0; i < iNodes; i++)
      {
         WizardNode* pExistNode = NULL;
         pExistNode = item.mOutputNodes.at(i);
         if (pExistNode != NULL)
         {
            string nodeName = pExistNode->getName().c_str();
            string nodeType = ((WizardNodeImp*) pExistNode)->getOriginalType().c_str();

            WizardNode* pNode = NULL;
            pNode = addOutputNode(nodeName, nodeType);
            if (pNode != NULL)
            {
               *((WizardNodeImp*) pNode) = *((WizardNodeImp*) pExistNode);
            }
         }
      }
   }

   return *this;
}

void WizardItemImp::setName(const string& name)
{
   if (name != mName)
   {
      mName = "" + name;
      WizardItemChangeType eChange = ItemName;
      notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
   }
}

const string& WizardItemImp::getName() const
{
   return mName;
}

const string& WizardItemImp::getType() const
{
   return mType;
}

void WizardItemImp::setBatchMode(bool bBatch, bool bModeSupported)
{
   if ((bBatch == mbBatch) && (bModeSupported == mbModeSupported))
   {
      return;
   }

   mbBatch = bBatch;
   mbModeSupported = bModeSupported;
   WizardItemChangeType eChange = ItemBatchMode;
   notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
}

bool WizardItemImp::getBatchMode() const
{
   return mbBatch;
}

bool WizardItemImp::isCurrentModeSupported() const
{
   return mbModeSupported;
}

void WizardItemImp::setPosition(double dX, double dY)
{
   if ((dX != mCoord.mX) || (dY != mCoord.mY))
   {
      mCoord.mX = dX;
      mCoord.mY = dY;
      WizardItemChangeType eChange = ItemPosition;
      notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
   }
}

double WizardItemImp::getXPosition() const
{
   return mCoord.mX;
}

double WizardItemImp::getYPosition() const
{
   return mCoord.mY;
}

WizardNode* WizardItemImp::addInputNode(const string& name, const string& type)
{
   WizardNode* pNode = NULL;
   pNode = new WizardNodeImp(this, name, type);
   if (pNode != NULL)
   {
      mInputNodes.push_back(pNode);
      WizardItemChangeType eChange = ItemNodeAdded;
      notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
   }

   return pNode;
}

int WizardItemImp::getNumInputNodes() const
{
   int iCount = 0;
   iCount = mInputNodes.size();

   return iCount;
}

WizardNode* WizardItemImp::getInputNode(const string& name, const string& type) const
{
   if ((name.empty() == true) || (type.empty() == true))
   {
      return NULL;
   }

   vector<WizardNode*>::const_iterator iter;
   iter = mInputNodes.begin();
   while (iter != mInputNodes.end())
   {
      WizardNode* pNode = NULL;
      pNode = *iter;
      if (pNode != NULL)
      {
         string nodeName = "";
         string nodeType = "";

         nodeName = pNode->getName();
         nodeType = pNode->getType();

         if ((nodeName == name) && (nodeType == type))
         {
            return pNode;
         }
      }

      iter++;
   }

   return NULL;
}

const vector<WizardNode*>& WizardItemImp::getInputNodes() const
{
   return mInputNodes;
}

bool WizardItemImp::isInputNode(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return false;
   }

   for (unsigned int i = 0; i < mInputNodes.size(); i++)
   {
      WizardNode* pCurrentNode = NULL;
      pCurrentNode = mInputNodes.at(i);
      if (pCurrentNode == pNode)
      {
         return true;
      }
   }

   return false;
}

bool WizardItemImp::removeInputNode(WizardNode* pNode, bool bDeleteValue)
{
   if (pNode == NULL)
   {
      return false;
   }

   vector<WizardNode*>::iterator iter;
   iter = mInputNodes.begin();
   while (iter != mInputNodes.end())
   {
      WizardNode* pCurrentNode = NULL;
      pCurrentNode = *iter;
      if (pCurrentNode == pNode)
      {
         if (bDeleteValue == true)
         {
            ((WizardNodeImp*) pNode)->deleteValue();
         }

         delete ((WizardNodeImp*) pNode);
         mInputNodes.erase(iter);
         WizardItemChangeType eChange = ItemNodeRemoved;
         notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
         return true;
      }

      iter++;
   }

   return false;
}

void WizardItemImp::clearInputNodes()
{
   while (mInputNodes.empty() == false)
   {
      WizardNode* pNode = mInputNodes.front();
      if (pNode != NULL)
      {
         bool bDelete = false;
         if (mType == "Value")
         {
            bDelete = true;
         }

         bool bSuccess = removeInputNode(pNode, bDelete);
         if (bSuccess == false)
         {
            break;
         }
      }
   }
}

WizardNode* WizardItemImp::addOutputNode(const string& name, const string& type)
{
   WizardNode* pNode = NULL;
   pNode = new WizardNodeImp(this, name, type);
   if (pNode != NULL)
   {
      mOutputNodes.push_back(pNode);
      WizardItemChangeType eChange = ItemNodeAdded;
      notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
   }

   return pNode;
}

int WizardItemImp::getNumOutputNodes() const
{
   int iCount = 0;
   iCount = mOutputNodes.size();

   return iCount;
}

WizardNode* WizardItemImp::getOutputNode(const string& name, const string& type) const
{
   if ((name.empty() == true) || (type.empty() == true))
   {
      return NULL;
   }

   vector<WizardNode*>::const_iterator iter;
   iter = mOutputNodes.begin();
   while (iter != mOutputNodes.end())
   {
      WizardNode* pNode = NULL;
      pNode = *iter;
      if (pNode != NULL)
      {
         string nodeName = "";
         string nodeType = "";

         nodeName = pNode->getName();
         nodeType = pNode->getType();

         if ((nodeName == name) && (nodeType == type))
         {
            return pNode;
         }
      }

      iter++;
   }

   return NULL;
}

const vector<WizardNode*>& WizardItemImp::getOutputNodes() const
{
   return mOutputNodes;
}

bool WizardItemImp::isOutputNode(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return false;
   }

   for (unsigned int i = 0; i < mOutputNodes.size(); i++)
   {
      WizardNode* pCurrentNode = NULL;
      pCurrentNode = mOutputNodes.at(i);
      if (pCurrentNode == pNode)
      {
         return true;
      }
   }

   return false;
}

bool WizardItemImp::removeOutputNode(WizardNode* pNode, bool bDeleteValue)
{
   if (pNode == NULL)
   {
      return false;
   }

   vector<WizardNode*>::iterator iter;
   iter = mOutputNodes.begin();
   while (iter != mOutputNodes.end())
   {
      WizardNode* pCurrentNode = NULL;
      pCurrentNode = *iter;
      if (pCurrentNode == pNode)
      {
         if (bDeleteValue == true)
         {
            ((WizardNodeImp*) pNode)->deleteValue();
         }

         delete ((WizardNodeImp*) pNode);
         mOutputNodes.erase(iter);
         WizardItemChangeType eChange = ItemNodeRemoved;
         notify(SIGNAL_NAME(Subject, Modified), boost::any(&eChange));
         return true;
      }

      iter++;
   }

   return false;
}

void WizardItemImp::clearOutputNodes()
{
   while (mOutputNodes.empty() == false)
   {
      WizardNode* pNode = mOutputNodes.front();
      if (pNode != NULL)
      {
         bool bDelete = false;
         if (mType == "Value")
         {
            bDelete = true;
         }

         bool bSuccess = removeOutputNode(pNode, bDelete);
         if (bSuccess == false)
         {
            break;
         }
      }
   }
}

bool WizardItemImp::isItemConnected(WizardItem* pItem, bool bInputNode) const
{
   if (pItem == NULL)
   {
      return false;
   }

   if (pItem == (WizardItem*) this)
   {
      return true;
   }

   vector<WizardItem*> connectedItems;
   getConnectedItems(bInputNode, connectedItems);

   unsigned int uiCount = 0;
   uiCount = connectedItems.size();
   for (unsigned int i = 0; i < uiCount; i++)
   {
      WizardItem* pConnectedItem = NULL;
      pConnectedItem = connectedItems.at(i);
      if (pConnectedItem != NULL)
      {
         bool bConnected = false;
         bConnected = ((WizardItemImp*) pConnectedItem)->isItemConnected(pItem, bInputNode);
         if (bConnected == true)
         {
            return true;
         }
      }
   }

   return false;
}

void WizardItemImp::getConnectedItems(bool bInputNode, vector<WizardItem*>& connectedItems) const
{
   connectedItems.clear();

   vector<WizardNode*> nodes;
   if (bInputNode == true)
   {
      nodes = mInputNodes;
   }
   else if (bInputNode == false)
   {
      nodes = mOutputNodes;
   }

   vector<WizardNode*>::iterator iter;
   iter = nodes.begin();
   while (iter != nodes.end())
   {
      WizardNode* pNode = NULL;
      pNode = *iter;
      if (pNode != NULL)
      {
         const vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();

         int iCount = 0;
         iCount = connectedNodes.size();
         for (int i = 0; i < iCount; i++)
         {
            WizardNode* pConnectedNode = NULL;
            pConnectedNode = connectedNodes.at(i);
            if (pConnectedNode != NULL)
            {
               WizardItem* pConnectedItem = NULL;
               pConnectedItem = ((WizardNodeImp*) pConnectedNode)->getItem();
               if (pConnectedItem != NULL)
               {
                  bool bContains = false;

                  int iConnectedCount = 0;
                  iConnectedCount = connectedItems.size();
                  for (int j = 0; j < iConnectedCount; j++)
                  {
                     WizardItem* pCurrentConnectedItem = NULL;
                     pCurrentConnectedItem = connectedItems.at(j);
                     if (pCurrentConnectedItem == pConnectedItem)
                     {
                        bContains = true;
                     }
                  }

                  if (bContains == false)
                  {
                     connectedItems.push_back(pConnectedItem);
                  }
               }
            }
         }
      }

      iter++;
   }
}

vector<WizardConnection> WizardItemImp::getConnections(const vector<WizardItem*>& items)
{
   vector<WizardConnection> connections;

   int iCount = 0;
   iCount = items.size();
   for (int i = 0; i < iCount; i++)
   {
      WizardItem* pOutputItem = NULL;
      pOutputItem = items.at(i);
      if (pOutputItem == NULL)
      {
         continue;
      }

      vector<WizardNode*> outputNodes = pOutputItem->getOutputNodes();

      int iOutputNodes = 0;
      iOutputNodes = outputNodes.size();
      for (int j = 0; j < iOutputNodes; j++)
      {
         WizardNode* pOutputNode = NULL;
         pOutputNode = outputNodes.at(j);
         if (pOutputNode == NULL)
         {
            continue;
         }

         vector<WizardNode*> connectedNodes = pOutputNode->getConnectedNodes();

         int iConnectedNodes = 0;
         iConnectedNodes = connectedNodes.size();
         for (int k = 0; k < iConnectedNodes; k++)
         {
            WizardNode* pConnectedNode = NULL;
            pConnectedNode = connectedNodes.at(k);
            if (pConnectedNode == NULL)
            {
               continue;
            }

            WizardItem* pConnectedItem = NULL;
            pConnectedItem = ((WizardNodeImp*) pConnectedNode)->getItem();
            if (pConnectedItem == NULL)
            {
               continue;
            }

            for (int l = 0; l < iCount; l++)
            {
               WizardItem* pInputItem = NULL;
               pInputItem = items.at(l);
               if (pInputItem != pConnectedItem)
               {
                  continue;
               }

               vector<WizardNode*> inputNodes = pConnectedItem->getInputNodes();

               int iInputNodes = 0;
               iInputNodes = inputNodes.size();
               for (int m = 0; m < iInputNodes; m++)
               {
                  WizardNode* pInputNode = NULL;
                  pInputNode = inputNodes.at(m);
                  if (pInputNode != pConnectedNode)
                  {
                     continue;
                  }

                  WizardConnection connection;
                  connection.mpInputNode = pInputNode;
                  connection.mpOutputNode = pOutputNode;
                  connection.miInputItemIndex = l;
                  connection.miInputNodeIndex = m;
                  connection.miOutputItemIndex = i;
                  connection.miOutputNodeIndex = j;

                  connections.push_back(connection);
               }
            }
         }
      }
   }

   return connections;
}

void WizardItemImp::setConnections(vector<WizardItem*>& items, const vector<WizardConnection>& connections)
{
   int iConnections = 0;
   iConnections = connections.size();
   for (int i = 0; i < iConnections; i++)
   {
      WizardConnection connection = connections.at(i);
      int iInputItemIndex = connection.miInputItemIndex;
      int iInputNodeIndex = connection.miInputNodeIndex;
      int iOutputItemIndex = connection.miOutputItemIndex;
      int iOutputNodeIndex = connection.miOutputNodeIndex;

      int iItems = 0;
      iItems = items.size();
      if ((iInputItemIndex >= iItems) || (iOutputItemIndex >= iItems))
      {
         continue;
      }

      WizardItem* pOutputItem = NULL;
      WizardItem* pInputItem = NULL;
      pOutputItem = items.at(iOutputItemIndex);
      pInputItem = items.at(iInputItemIndex);

      if ((pOutputItem == NULL) || (pInputItem == NULL))
      {
         continue;
      }

      vector<WizardNode*> outputNodes = pOutputItem->getOutputNodes();
      vector<WizardNode*> inputNodes = pInputItem->getInputNodes();

      int iInputNodes = inputNodes.size();
      int iOutputNodes = outputNodes.size();

      if ((iInputNodeIndex >= iInputNodes) || (iOutputNodeIndex >= iOutputNodes))
      {
         continue;
      }

      WizardNode* pOutputNode = NULL;
      WizardNode* pInputNode = NULL;
      pOutputNode = outputNodes.at(iOutputNodeIndex);
      pInputNode = inputNodes.at(iInputNodeIndex);

      if ((pOutputNode == NULL) || (pInputNode == NULL))
      {
         continue;
      }

      ((WizardNodeImp*) pOutputNode)->addConnectedNode(pInputNode);
      ((WizardNodeImp*) pInputNode)->addConnectedNode(pOutputNode);
   }
}

bool WizardItemImp::toXml(XMLWriter* xml) const
{
   xml->addAttr("name",mName);
   xml->addAttr("type",mType);
   xml->addAttr("batch", StringUtilities::toXmlString<bool>(mbBatch));
   xml->addAttr("batchSupported", StringUtilities::toXmlString<bool>(mbModeSupported));

   xml->addText(StringUtilities::toXmlString(mCoord), xml->addElement("location"));

   vector<WizardNode*>::const_iterator iter;
   for(iter = mInputNodes.begin(); iter != mInputNodes.end(); iter++)
   {
      WizardNode* pNode(*iter);
      if(pNode == NULL)
         continue;
      xml->pushAddPoint(xml->addElement("input"));
      bool rval(pNode->toXml(xml));
      xml->popAddPoint();
      if(!rval)
         return false;
   }

   for(iter = mOutputNodes.begin(); iter != mOutputNodes.end(); iter++)
   {
      WizardNode* pNode(*iter);
      if(pNode == NULL)
         continue;
      xml->pushAddPoint(xml->addElement("output"));
      bool rval(pNode->toXml(xml));
      xml->popAddPoint();
      if(!rval)
         return false;
   }

   return true;
}

bool WizardItemImp::fromXml(DOMNode* document, unsigned int version)
{
   DOMElement *elmnt(static_cast<DOMElement*>(document));

   mName = A(elmnt->getAttribute(X("name")));
   mType = A(elmnt->getAttribute(X("type")));

   string v(A(elmnt->getAttribute(X("batch"))));
   mbBatch = StringUtilities::fromXmlString<bool>(v);
   v = A(elmnt->getAttribute(X("batchSupported")));
   mbModeSupported = StringUtilities::fromXmlString<bool>(v);

   mInputNodes.clear();
   mOutputNodes.clear();
   for(DOMNode *chld = document->getFirstChild();
                chld != NULL;
                chld = chld->getNextSibling())
   {
      if(XMLString::equals(chld->getNodeName(), X("location")))
      {
         mCoord = StringUtilities::fromXmlString<LocationType>(A(chld->getTextContent()));
      }
      else if(XMLString::equals(chld->getNodeName(), X("input"))
           || XMLString::equals(chld->getNodeName(), X("output")))
      {
         WizardNode *pNode(new WizardNodeImp(this, "", ""));
         if(pNode->fromXml(chld, version))
         {
            if(XMLString::equals(chld->getNodeName(), X("input")))
               mInputNodes.push_back(pNode);
            else
               mOutputNodes.push_back(pNode);
         }
         else
         {
            delete dynamic_cast<WizardNodeImp*>(pNode);
            return false;
         }
      }
   }

   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

const string& WizardItemImp::getObjectType() const
{
   static string type("WizardItemImp");
   return type;
}

bool WizardItemImp::isKindOf(const string& className) const
{
   if ((className == getType()) || (className == "WizardItem"))
   {
      return true;
      }

   return SubjectImp::isKindOf(className);
}
