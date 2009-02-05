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

WizardItemImp::WizardItemImp(string name, string type) :
   mName(name),
   mType(type),
   mbBatch(false),
   mbModeSupported(false)
{}

WizardItemImp::~WizardItemImp()
{
   notify(SIGNAL_NAME(Subject, Deleted)); // Call before clearing the nodes so that connected objects can access them
   clearInputNodes();
   clearOutputNodes();
}

WizardItemImp& WizardItemImp::operator=(const WizardItemImp& item)
{
   if (this != &item)
   {
      mName = item.mName;
      mType = item.mType;
      mbBatch = item.mbBatch;
      mbModeSupported = item.mbModeSupported;
      mCoord = item.mCoord;

      // Input nodes
      clearInputNodes();

      vector<WizardNode*>::const_iterator iter;
      for (iter = item.mInputNodes.begin(); iter != item.mInputNodes.end(); ++iter)
      {
         WizardNodeImp* pExistNode = static_cast<WizardNodeImp*>(*iter);
         if (pExistNode != NULL)
         {
            const string& nodeName = pExistNode->getName();
            const string& nodeType = pExistNode->getOriginalType();
            string nodeDescription = "";

            WizardNodeExt1* pExistNodeExt1 = dynamic_cast<WizardNodeExt1*>(pExistNode);
            if (pExistNodeExt1 != NULL)
            {
               nodeDescription = pExistNodeExt1->getDescription();
            }

            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(addInputNode(nodeName, nodeType, nodeDescription));
            if (pNode != NULL)
            {
               *pNode = *pExistNode;
            }
         }
      }

      // Output nodes
      clearOutputNodes();

      for (iter = item.mOutputNodes.begin(); iter != item.mOutputNodes.end(); ++iter)
      {
         WizardNodeImp* pExistNode = static_cast<WizardNodeImp*>(*iter);
         if (pExistNode != NULL)
         {
            const string& nodeName = pExistNode->getName();
            const string& nodeType = pExistNode->getOriginalType();
            string nodeDescription = "";

            WizardNodeExt1* pExistNodeExt1 = dynamic_cast<WizardNodeExt1*>(pExistNode);
            if (pExistNodeExt1 != NULL)
            {
               nodeDescription = pExistNodeExt1->getDescription();
            }

            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(addOutputNode(nodeName, nodeType, nodeDescription));
            if (pNode != NULL)
            {
               *pNode = *pExistNode;
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

WizardNode* WizardItemImp::addInputNode(const string& name, const string& type, const string& description)
{
   WizardNode* pNode = new WizardNodeImp(this, name, type, description);
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
   int iCount = mInputNodes.size();
   return iCount;
}

WizardNode* WizardItemImp::getInputNode(const string& name, const string& type) const
{
   if ((name.empty() == true) || (type.empty() == true))
   {
      return NULL;
   }

   vector<WizardNode*>::const_iterator iter = mInputNodes.begin();
   while (iter != mInputNodes.end())
   {
      WizardNode* pNode = *iter;
      if (pNode != NULL)
      {
         string nodeName = pNode->getName();
         string nodeType = pNode->getType();

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

   for (unsigned int i = 0; i < mInputNodes.size(); ++i)
   {
      WizardNode* pCurrentNode = mInputNodes.at(i);
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

   vector<WizardNode*>::iterator iter = mInputNodes.begin();
   while (iter != mInputNodes.end())
   {
      WizardNode* pCurrentNode = *iter;
      if (pCurrentNode == pNode)
      {
         WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(pNode);
         if (pNodeImp != NULL)
         {
            if (bDeleteValue == true)
            {
               pNodeImp->deleteValue();
            }

            delete pNodeImp;
         }

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

WizardNode* WizardItemImp::addOutputNode(const string& name, const string& type, const string& description)
{
   WizardNode* pNode = new WizardNodeImp(this, name, type, description);
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
   int iCount = mOutputNodes.size();
   return iCount;
}

WizardNode* WizardItemImp::getOutputNode(const string& name, const string& type) const
{
   if ((name.empty() == true) || (type.empty() == true))
   {
      return NULL;
   }

   vector<WizardNode*>::const_iterator iter = mOutputNodes.begin();
   while (iter != mOutputNodes.end())
   {
      WizardNode* pNode = *iter;
      if (pNode != NULL)
      {
         string nodeName = pNode->getName();
         string nodeType = pNode->getType();

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

   for (unsigned int i = 0; i < mOutputNodes.size(); ++i)
   {
      WizardNode* pCurrentNode = mOutputNodes.at(i);
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

   vector<WizardNode*>::iterator iter = mOutputNodes.begin();
   while (iter != mOutputNodes.end())
   {
      WizardNode* pCurrentNode = *iter;
      if (pCurrentNode == pNode)
      {
         WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(pNode);
         if (pNodeImp != NULL)
         {
            if (bDeleteValue == true)
            {
               pNodeImp->deleteValue();
            }

            delete pNodeImp;
         }

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

   if (static_cast<WizardItemImp*>(pItem) == this)
   {
      return true;
   }

   vector<WizardItem*> connectedItems;
   getConnectedItems(bInputNode, connectedItems);

   for (unsigned int i = 0; i < connectedItems.size(); ++i)
   {
      WizardItemImp* pConnectedItem = static_cast<WizardItemImp*>(connectedItems.at(i));
      if (pConnectedItem != NULL)
      {
         bool bConnected = pConnectedItem->isItemConnected(pItem, bInputNode);
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

   vector<WizardNode*>::iterator iter = nodes.begin();
   while (iter != nodes.end())
   {
      WizardNode* pNode = *iter;
      if (pNode != NULL)
      {
         const vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();
         for (unsigned int i = 0; i < connectedNodes.size(); ++i)
         {
            WizardNodeImp* pConnectedNode = static_cast<WizardNodeImp*>(connectedNodes.at(i));
            if (pConnectedNode != NULL)
            {
               WizardItem* pConnectedItem = pConnectedNode->getItem();
               if (pConnectedItem != NULL)
               {
                  bool bContains = false;
                  for (unsigned int j = 0; j < connectedItems.size(); ++j)
                  {
                     WizardItem* pCurrentConnectedItem = connectedItems.at(j);
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

   int iCount = items.size();
   for (int i = 0; i < iCount; ++i)
   {
      WizardItem* pOutputItem = items.at(i);
      if (pOutputItem == NULL)
      {
         continue;
      }

      vector<WizardNode*> outputNodes = pOutputItem->getOutputNodes();

      int iOutputNodes = outputNodes.size();
      for (int j = 0; j < iOutputNodes; ++j)
      {
         WizardNode* pOutputNode = outputNodes.at(j);
         if (pOutputNode == NULL)
         {
            continue;
         }

         vector<WizardNode*> connectedNodes = pOutputNode->getConnectedNodes();

         int iConnectedNodes = connectedNodes.size();
         for (int k = 0; k < iConnectedNodes; ++k)
         {
            WizardNode* pConnectedNode = connectedNodes.at(k);
            if (pConnectedNode == NULL)
            {
               continue;
            }

            WizardItem* pConnectedItem = static_cast<WizardNodeImp*>(pConnectedNode)->getItem();
            if (pConnectedItem == NULL)
            {
               continue;
            }

            for (int l = 0; l < iCount; ++l)
            {
               WizardItem* pInputItem = items.at(l);
               if (pInputItem != pConnectedItem)
               {
                  continue;
               }

               vector<WizardNode*> inputNodes = pConnectedItem->getInputNodes();

               int iInputNodes = inputNodes.size();
               for (int m = 0; m < iInputNodes; ++m)
               {
                  WizardNode* pInputNode = inputNodes.at(m);
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
   int iConnections = connections.size();
   for (int i = 0; i < iConnections; ++i)
   {
      WizardConnection connection = connections.at(i);
      int iInputItemIndex = connection.miInputItemIndex;
      int iInputNodeIndex = connection.miInputNodeIndex;
      int iOutputItemIndex = connection.miOutputItemIndex;
      int iOutputNodeIndex = connection.miOutputNodeIndex;

      int iItems = items.size();
      if ((iInputItemIndex >= iItems) || (iOutputItemIndex >= iItems))
      {
         continue;
      }

      WizardItem* pOutputItem = items.at(iOutputItemIndex);
      WizardItem* pInputItem = items.at(iInputItemIndex);

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

      WizardNodeImp* pOutputNode = static_cast<WizardNodeImp*>(outputNodes.at(iOutputNodeIndex));
      WizardNodeImp* pInputNode = static_cast<WizardNodeImp*>(inputNodes.at(iInputNodeIndex));

      if ((pOutputNode == NULL) || (pInputNode == NULL))
      {
         continue;
      }

      pOutputNode->addConnectedNode(pInputNode);
      pInputNode->addConnectedNode(pOutputNode);
   }
}

bool WizardItemImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("name", mName);
   pXml->addAttr("type", mType);
   pXml->addAttr("batch", StringUtilities::toXmlString<bool>(mbBatch));
   pXml->addAttr("batchSupported", StringUtilities::toXmlString<bool>(mbModeSupported));

   pXml->addText(StringUtilities::toXmlString(mCoord), pXml->addElement("location"));

   vector<WizardNode*>::const_iterator iter;
   for (iter = mInputNodes.begin(); iter != mInputNodes.end(); ++iter)
   {
      WizardNode* pNode(*iter);
      if (pNode == NULL)
      {
         continue;
      }

      pXml->pushAddPoint(pXml->addElement("input"));
      bool rval(pNode->toXml(pXml));
      pXml->popAddPoint();
      if (!rval)
      {
         return false;
      }
   }

   for (iter = mOutputNodes.begin(); iter != mOutputNodes.end(); ++iter)
   {
      WizardNode* pNode(*iter);
      if (pNode == NULL)
      {
         continue;
      }

      pXml->pushAddPoint(pXml->addElement("output"));
      bool rval(pNode->toXml(pXml));
      pXml->popAddPoint();
      if (!rval)
      {
         return false;
      }
   }

   return true;
}

bool WizardItemImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));

   mName = A(elmnt->getAttribute(X("name")));
   mType = A(elmnt->getAttribute(X("type")));

   string v(A(elmnt->getAttribute(X("batch"))));
   mbBatch = StringUtilities::fromXmlString<bool>(v);
   v = A(elmnt->getAttribute(X("batchSupported")));
   mbModeSupported = StringUtilities::fromXmlString<bool>(v);

   mInputNodes.clear();
   mOutputNodes.clear();
   for (DOMNode* chld = pDocument->getFirstChild(); chld != NULL; chld = chld->getNextSibling())
   {
      if (XMLString::equals(chld->getNodeName(), X("location")))
      {
         mCoord = StringUtilities::fromXmlString<LocationType>(A(chld->getTextContent()));
      }
      else if (XMLString::equals(chld->getNodeName(), X("input"))
           || XMLString::equals(chld->getNodeName(), X("output")))
      {
         WizardNode* pNode(new WizardNodeImp(this, "", "", ""));
         if (pNode->fromXml(chld, version))
         {
            if (XMLString::equals(chld->getNodeName(), X("input")))
            {
               mInputNodes.push_back(pNode);
            }
            else
            {
               mOutputNodes.push_back(pNode);
            }
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
   static string sType("WizardItemImp");
   return sType;
}

bool WizardItemImp::isKindOf(const string& className) const
{
   if ((className == getType()) || (className == "WizardItem"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}
