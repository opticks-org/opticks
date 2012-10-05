/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "WizardItemImp.h"
#include "WizardNodeImp.h"
#include "xmlwriter.h"
#include "xmlreader.h"

#include <boost/any.hpp>
#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

WizardItemImp::WizardItemImp(WizardObject* pWizard, string name, string type) :
   mpWizard(pWizard),
   mName(name),
   mType(type),
   mbBatch(true),
   mbModeSupported(true)
{
   // Initialize the wizard nodes by setting interactive mode since most items will be added in interactive mode
   setBatchMode(false);
}

WizardItemImp::WizardItemImp(WizardObject* pWizard, string name, const DataVariant& value) :
   mpWizard(pWizard),
   mName(name),
   mType("Value"),
   mbBatch(true),
   mbModeSupported(true)
{
   // Add an output node initialized to the given value
   string nodeType = value.getTypeName();
   if (nodeType.empty() == false)
   {
      WizardNodeImp* pNode = static_cast<WizardNodeImp*>(addNode(mName, nodeType, string(), false));
      if ((pNode != NULL) && (value.isValid() == true))
      {
         pNode->setValue(value.getPointerToValueAsVoid());
      }
   }
}

WizardItemImp::~WizardItemImp()
{
   notify(SIGNAL_NAME(Subject, Deleted)); // Call before clearing the nodes so that connected objects can access them
   clearNodes(true);
   clearNodes(false);
}

bool WizardItemImp::copyItem(const WizardItemImp* pItem)
{
   if ((pItem == NULL) || (pItem == this))
   {
      return false;
   }

   mName = pItem->mName;
   mType = pItem->mType;
   mbBatch = pItem->mbBatch;
   mbModeSupported = pItem->mbModeSupported;
   mCoord = pItem->mCoord;

   // Nodes
   clearNodes(true);
   clearNodes(false);

   vector<WizardNode*>::size_type numNodes = pItem->mInputNodes.size() + pItem->mOutputNodes.size();
   for (vector<WizardNode*>::size_type i = 0; i < numNodes; ++i)
   {
      WizardNodeImp* pExistNode = NULL;
      if (i < pItem->mInputNodes.size())
      {
         pExistNode = static_cast<WizardNodeImp*>(pItem->mInputNodes[i]);
      }
      else
      {
         pExistNode = static_cast<WizardNodeImp*>(pItem->mOutputNodes[i - pItem->mInputNodes.size()]);
      }

      if (pExistNode != NULL)
      {
         const string& nodeName = pExistNode->getName();
         const string& nodeType = pExistNode->getOriginalType();
         const string& nodeDescription = pExistNode->getDescription();

         WizardNodeImp* pNode = static_cast<WizardNodeImp*>(addNode(nodeName, nodeType, nodeDescription,
            i < pItem->mInputNodes.size()));
         if (pNode != NULL)
         {
            pNode->copyNode(pExistNode);
         }
      }
   }

   return true;
}

WizardObject* WizardItemImp::getWizard()
{
   return mpWizard;
}

const WizardObject* WizardItemImp::getWizard() const
{
   return mpWizard;
}

void WizardItemImp::setName(const string& name)
{
   if (name != mName)
   {
      mName = name;
      notify(SIGNAL_NAME(WizardItemImp, Renamed), boost::any(mName));
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

void WizardItemImp::setBatchMode(bool bBatch)
{
   if (bBatch == mbBatch)
   {
      return;
   }

   mbBatch = bBatch;
   mbModeSupported = true;

   if (mType != "Value")
   {
      Service<PlugInManagerServices> pManager;

      PlugInDescriptor* pDescriptor = pManager->getPlugInDescriptor(mName);
      if (pDescriptor != NULL)
      {
         const PlugInArgList* pInArgList = NULL;
         const PlugInArgList* pOutArgList = NULL;

         if (mbBatch == true)
         {
            mbModeSupported = pDescriptor->hasBatchSupport();
            pInArgList = pDescriptor->getBatchInputArgList();
            pOutArgList = pDescriptor->getBatchOutputArgList();
         }
         else
         {
            mbModeSupported = pDescriptor->hasInteractiveSupport();
            pInArgList = pDescriptor->getInteractiveInputArgList();
            pOutArgList = pDescriptor->getInteractiveOutputArgList();
         }

         clearNodes(true);
         clearNodes(false);

         unsigned short numInputArgs = 0;
         if (pInArgList != NULL)
         {
            numInputArgs = pInArgList->getCount();
         }

         unsigned short numOutputArgs = 0;
         if (pOutArgList != NULL)
         {
            numOutputArgs = pOutArgList->getCount();
         }

         for (unsigned short i = 0; i < numInputArgs + numOutputArgs; ++i)
         {
            // Get the current plug-in arg
            PlugInArg* pArg = NULL;
            if (i < numInputArgs)
            {
               pInArgList->getArg(i, pArg);
            }
            else
            {
               pOutArgList->getArg(i - numInputArgs, pArg);
            }

            if (pArg == NULL)
            {
               continue;
            }

            string argName = pArg->getName();
            string argType = pArg->getType();
            string argDescription = pArg->getDescription();

            if (argType.empty() == true)
            {
               argType = "Unknown";
            }

            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(addNode(argName, argType, argDescription,
               i < numInputArgs));
            if (pNode != NULL)
            {
               Service<ModelServices> pModel;

               vector<string> validTypes;
               pModel->getElementTypes(argType, validTypes);

               if (validTypes.empty() == true)
               {
                  pModel->getDataDescriptorTypes(argType, validTypes);
               }

               if (validTypes.empty() == true)
               {
                  pModel->getFileDescriptorTypes(argType, validTypes);
               }

               Service<DesktopServices> pDesktop;
               if (validTypes.empty() == true)
               {
                  pDesktop->getViewTypes(argType, validTypes);
               }

               if (validTypes.empty())
               {
                  pDesktop->getLayerTypes(argType, validTypes);
               }

               pNode->setValidTypes(validTypes);
               pNode->setValue(pArg->getDefaultValue());
            }
         }
      }
   }

   notify(SIGNAL_NAME(WizardItemImp, BatchModeChanged), boost::any(mbBatch));
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
      notify(SIGNAL_NAME(WizardItemImp, PositionChanged), boost::any(mCoord));
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

int WizardItemImp::getNodeIndex(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return -1;
   }

   for (int i = 0; i < static_cast<int>(mInputNodes.size()); ++i)
   {
      WizardNode* pCurrentNode = mInputNodes.at(i);
      if (pCurrentNode == pNode)
      {
         return i;
      }
   }

   for (int i = 0; i < static_cast<int>(mOutputNodes.size()); ++i)
   {
      WizardNode* pCurrentNode = mOutputNodes.at(i);
      if (pCurrentNode == pNode)
      {
         return i;
      }
   }

   return -1;
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

   clearNodes(true);
   clearNodes(false);

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

            static_cast<WizardNodeImp*>(pNode)->attach(SIGNAL_NAME(WizardNodeImp, NodeConnected),
               Slot(this, &WizardItemImp::nodeConnected));
            static_cast<WizardNodeImp*>(pNode)->attach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected),
               Slot(this, &WizardItemImp::nodeDisconnected));
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

void WizardItemImp::nodeConnected(Subject& subject, const string& signal, const boost::any& data)
{
   WizardNode* pConnectedNode = boost::any_cast<WizardNode*>(data);
   if (pConnectedNode != NULL)
   {
      WizardItem* pConnectedItem = pConnectedNode->getItem();
      if (pConnectedItem != NULL)
      {
         notify(SIGNAL_NAME(WizardItemImp, ItemConnected), boost::any(pConnectedItem));
      }
   }
}

void WizardItemImp::nodeDisconnected(Subject& subject, const string& signal, const boost::any& data)
{
   WizardNode* pDisconnectedNode = boost::any_cast<WizardNode*>(data);
   if (pDisconnectedNode != NULL)
   {
      WizardItem* pDisconnectedItem = pDisconnectedNode->getItem();
      if (pDisconnectedItem != NULL)
      {
         notify(SIGNAL_NAME(WizardItemImp, ItemDisconnected), boost::any(pDisconnectedItem));
      }
   }
}

WizardNode* WizardItemImp::addNode(const string& name, const string& type, const string& description, bool inputNode)
{
   WizardNode* pNode = new WizardNodeImp(this, name, type, description);
   if (inputNode == true)
   {
      mInputNodes.push_back(pNode);
   }
   else
   {
      mOutputNodes.push_back(pNode);
   }

   static_cast<WizardNodeImp*>(pNode)->attach(SIGNAL_NAME(WizardNodeImp, NodeConnected),
      Slot(this, &WizardItemImp::nodeConnected));
   static_cast<WizardNodeImp*>(pNode)->attach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected),
      Slot(this, &WizardItemImp::nodeDisconnected));

   notify(SIGNAL_NAME(WizardItemImp, NodeAdded), boost::any(pNode));
   return pNode;
}

void WizardItemImp::clearNodes(bool inputNodes)
{
   vector<WizardNode*>* pNodes = NULL;
   if (inputNodes == true)
   {
      pNodes = &mInputNodes;
   }
   else
   {
      pNodes = &mOutputNodes;
   }

   if (pNodes == NULL)
   {
      return;
   }

   while (pNodes->empty() == false)
   {
      vector<WizardNode*>::iterator iter = pNodes->begin();

      WizardNode* pNode = *iter;
      if (pNode != NULL)
      {
         bool bDelete = false;
         if (mType == "Value")
         {
            bDelete = true;
         }

         pNodes->erase(iter);
         notify(SIGNAL_NAME(WizardItemImp, NodeRemoved), boost::any(pNode));

         WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(pNode);
         if (pNodeImp != NULL)
         {
            pNodeImp->detach(SIGNAL_NAME(WizardNodeImp, NodeConnected), Slot(this, &WizardItemImp::nodeConnected));
            pNodeImp->detach(SIGNAL_NAME(WizardNodeImp, NodeDisconnected),
               Slot(this, &WizardItemImp::nodeDisconnected));

            if (bDelete == true)
            {
               pNodeImp->deleteValue();
            }

            delete pNodeImp;
         }
      }
   }
}
