/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>

#include "AppVerify.h"
#include "DynamicObject.h"
#include "MessageLogImp.h"
#include "MessageLogMgr.h"
#include "MessageLogWindow.h"
#include "SessionManager.h"

#include <memory>
#include <string>
using namespace std;

MessageLogWindow::MessageLogWindow(const string& id, QWidget* pParent) :
   DockWindowAdapter(id, "Message Log Window", pParent),
   mpLogCombo(NULL),
   mpModel(NULL),
   mpMsgLogMgr(Service<MessageLogMgr>().get())
{
   QWidget* pWidget = new QWidget(this);

   // Message log item model
   mpModel = new MessageLogWindowModel;

   // Message tree
   QTreeView* pMessageTree = new QTreeView(pWidget);
   pMessageTree->setModel(mpModel);
   pMessageTree->setSelectionMode(QAbstractItemView::NoSelection);
   pMessageTree->setRootIsDecorated(true);

   QHeaderView* pHeader = pMessageTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setSortIndicatorShown(false);
   }

   // Message log combo
   mpLogCombo = new QComboBox(pWidget);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(pWidget);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(pMessageTree, 10);
   pLayout->addWidget(mpLogCombo);

   // Initialization
   setIcon(QIcon(":/icons/MessageLogWindow"));
   setWidget(pWidget);

   vector<MessageLog*> logs = mpMsgLogMgr->getLogs();
   for (vector<MessageLog*>::const_iterator iter = logs.begin(); iter != logs.end(); ++iter)
   {
      MessageLog* pLog = *iter;
      if (pLog != NULL)
      {
         QString logName = getDisplayName(pLog);
         if (logName.isEmpty() == false)
         {
            mpLogCombo->addItem(logName);
         }
      }
   }

   setLog(mpLogCombo->currentText());

   // Connections
   VERIFYNR(connect(mpLogCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setLog(const QString&))));
   mpMsgLogMgr.addSignal(SIGNAL_NAME(MessageLogMgr, LogAdded), Slot(this, &MessageLogWindow::messageLogAdded));
   mpMsgLogMgr.addSignal(SIGNAL_NAME(MessageLogMgr, LogRemoved), Slot(this, &MessageLogWindow::messageLogRemoved));
}

MessageLogWindow::~MessageLogWindow()
{}

void MessageLogWindow::messageLogAdded(Subject& subject, const string& signal, const boost::any& data)
{
   MessageLog* pLog = boost::any_cast<MessageLog*>(data);
   if (pLog == NULL)
   {
      return;
   }

   QString logName = getDisplayName(pLog);
   if (logName.isEmpty() == false)
   {
      if (mpLogCombo->findText(logName) == -1)
      {
         mpLogCombo->addItem(logName);
         mpLogCombo->setCurrentIndex(mpLogCombo->count() - 1);
      }
   }
}

void MessageLogWindow::messageLogRemoved(Subject& subject, const string& signal, const boost::any& data)
{
   MessageLog* pLog = boost::any_cast<MessageLog*>(data);
   if (pLog == NULL)
   {
      return;
   }

   QString logName = getDisplayName(pLog);

   int index = mpLogCombo->findText(logName);
   if (index != -1)
   {
      mpLogCombo->removeItem(index);   // This will cause a new item to be activated, which will call
                                       // setLog() to update the current log in the item model
   }
}

void MessageLogWindow::setLog(const QString& logName)
{
   MessageLog* pLog = NULL;

   string realLogName = getLogName(logName);
   if (realLogName.empty() == false)
   {
      pLog = mpMsgLogMgr->getLog(realLogName);
   }

   mpModel->setMessageLog(pLog);
}

QString MessageLogWindow::getDisplayName(const MessageLog* pLog)
{
   if (pLog == NULL)
   {
      return QString();
   }

   string logName = pLog->getLogName();
   if (logName.empty() == true)
   {
      return QString();
   }

   if (logName == Service<SessionManager>()->getName())
   {
      logName = "Session Log";
   }

   return QString::fromStdString(logName);
}

string MessageLogWindow::getLogName(const QString& displayName)
{
   if (displayName.isEmpty() == true)
   {
      return string();
   }

   if (displayName == "Session Log")
   {
      return Service<SessionManager>()->getName();
   }

   return displayName.toStdString();
}

/*************************
 **  MessageLogWindowModel
 *************************/
MessageLogWindowModel::MessageLogWindowModel(QObject* pParent) :
   mpLog(NULL)
{
   mHeaderNames << "ID" << "Type" << "Message" << "Result" << "Reason" << "Time Stamp" << "Component" << "Key";
   mpLog.addSignal(SIGNAL_NAME(MessageLog, MessageAdded), Slot(this, &MessageLogWindowModel::messageAdded));
   mpLog.addSignal(SIGNAL_NAME(MessageLog, MessageHidden), Slot(this, &MessageLogWindowModel::messageFinalized));
}

MessageLogWindowModel::~MessageLogWindowModel()
{}

QModelIndex MessageLogWindowModel::index(int row, int column, const QModelIndex &parent) const
{
   if (mpLog.get() == NULL)
   {
      return QModelIndex();
   }

   const Message* pMessage = NULL;
   if (parent.isValid())
   {
      Subject* pSubject = reinterpret_cast<Subject*>(parent.internalPointer());
      Message* pParent = dynamic_cast<Message*>(pSubject);
      Step* pParentStep = dynamic_cast<Step*>(pParent);

      // the parent is a step
      if (pParentStep != NULL)
      {
         if (row < static_cast<int>(pParentStep->size()))
         {
            pMessage = (*pParentStep)[row];
         }
         else
         {
            const DynamicObject* pProperties = pParent->getProperties();
            mPropertyCache.insert(pProperties, pParent);
            return createIndex(row, column,
                           const_cast<Subject*>(static_cast<const Subject*>(pProperties)));
         }
      }
      // the parent is a message
      else if (pParent != NULL)
      {
         const DynamicObject* pProperties = pParent->getProperties();
         mPropertyCache.insert(pProperties, pParent);
         return createIndex(row, column, const_cast<Subject*>(static_cast<const Subject*>(pProperties)));
      }
   }
   else if (row < static_cast<int>(mpLog->size()))
   {
      pMessage = (*(mpLog.get()))[row];
   }
   return createIndex(row, column, const_cast<Subject*>(static_cast<const Subject*>(pMessage)));
}

QModelIndex MessageLogWindowModel::parent(const QModelIndex &index) const
{
   if (mpLog.get() == NULL || !index.isValid())
   {
      return QModelIndex();
   }

   const Subject* pSubject = reinterpret_cast<Subject*>(index.internalPointer());
   if (pSubject == NULL)
   {
      return QModelIndex();
   }

   // find the parent
   const Message* pParent = NULL;
   const DynamicObject* pProperties = dynamic_cast<const DynamicObject*>(pSubject);
   if (pProperties != NULL)
   {
      // this is a property index
      pParent = mPropertyCache[pProperties];
   }
   else
   {
      const MessageImp* pMessageImp = dynamic_cast<const MessageImp*>(pSubject);
      if (pMessageImp != NULL)
      {
         pParent = pMessageImp->getParent();
      }
   }

   // locate the row number
   const MessageImp* pParentImp = dynamic_cast<const MessageImp*>(pParent);
   if (pParentImp == NULL)
   {
      return QModelIndex();
   }
   int parentRow = 0;
   const Step* pGrandParent = pParentImp->getParent();
   if (pGrandParent != NULL)
   {
      for (Step::const_iterator gpIter = pGrandParent->begin(); gpIter != pGrandParent->end(); ++gpIter)
      {
         if (*gpIter == pParent)
         {
            break;
         }
         parentRow++;
      }
   }
   else
   {
      for (MessageLog::const_iterator lIter = mpLog->begin(); lIter != mpLog->end(); ++lIter)
      {
         if (*lIter == pParent)
         {
            break;
         }
         parentRow++;
      }
   }
   return createIndex(parentRow, index.column(), const_cast<Subject*>(static_cast<const Subject*>(pParent)));
}

int MessageLogWindowModel::rowCount(const QModelIndex &parent) const
{
   if (mpLog.get() == NULL)
   {
      return 0;
   }

   // if the parent is invalid, the request is for the number of top level items
   if (!parent.isValid())
   {
      return mpLog->size();
   }

   const Subject* pSubject = reinterpret_cast<Subject*>(parent.internalPointer());
   const Message* pParent = dynamic_cast<const Message*>(pSubject);
   if (pParent == NULL)
   {
      return 0;
   }
   const Step* pParentStep = dynamic_cast<const Step*>(pParent);

   // if the parent is a Step, return the count of subitems
   if (pParentStep != NULL)
   {
      return pParentStep->size() + pParent->getProperties()->getNumAttributes();
   }

   // return the count of properteis
   return pParent->getProperties()->getNumAttributes();
}

int MessageLogWindowModel::columnCount(const QModelIndex &parent) const
{
   if (mpLog.get() == NULL)
   {
      return 0;
   }
   return mHeaderNames.size();
}

QVariant MessageLogWindowModel::data(const QModelIndex &index, int role) const
{
   if (mpLog.get() == NULL || !index.isValid())
   {
      return QVariant();
   }

   const Subject* pSubject = reinterpret_cast<Subject*>(index.internalPointer());
   const DynamicObject* pProperties = dynamic_cast<const DynamicObject*>(pSubject);
   if (pProperties != NULL)
   {
      if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
      {
         const Message* pParent = mPropertyCache[pProperties];
         const Step* pParentStep = dynamic_cast<const Step*>(pParent);
         // this is a property index
         unsigned int propertyNumber = index.row();
         if (pParentStep != NULL)
         {
            propertyNumber -= pParentStep->size();
         }
         if (propertyNumber < pProperties->getNumAttributes())
         {
            vector<string> propertyNames;
            pProperties->getAttributeNames(propertyNames);
            string name = propertyNames[propertyNumber];

            QString header(headerData(index.column(), Qt::Horizontal).toString());
            if (header == "ID")
            {
               return QString::fromStdString(name);
            }
            else if (header == "Type")
            {
               string type;
               const DataVariant& attrValue = pProperties->getAttribute(name);
               if (attrValue.isValid())
               {
                  type = attrValue.getTypeName();
               }
               return QString::fromStdString(type);
            }
            else if (header == "Message")
            {
               string value = pProperties->getAttribute(name).toDisplayString();
               return QString::fromStdString(value);
            }
         }
         return QVariant();
      }
   }
   else
   {
      const MessageImp* pMessageImp = dynamic_cast<const MessageImp*>(pSubject);
      if (pMessageImp == NULL)
      {
         return QVariant();
      }
      switch (role)
      {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
         {
            QString header(headerData(index.column(), Qt::Horizontal).toString());
            if ((header == "ID") && (role == Qt::DisplayRole))
            {
               return QString::fromStdString(const_cast<MessageImp*>(pMessageImp)->getStringId());
            }
            else if ((header == "Type") && (role == Qt::DisplayRole))
            {
               if (dynamic_cast<const Step*>(pMessageImp) != NULL)
               {
                  return QString("Step");
               }
               else
               {
                  return QString("Message");
               }
            }
            else if (header == "Message")
            {
               return QString::fromStdString(pMessageImp->getAction());
            }
            else if ((header == "Result") && (role == Qt::DisplayRole))
            {
               const Step* pStep = dynamic_cast<const Step*>(pMessageImp);
               if (pStep != NULL)
               {
                  switch (pStep->getResult())
                  {
                     case Message::Success:
                        return QString("Success");
                     case Message::Failure:
                        return QString("Failure");
                     case Message::Abort:
                        return QString("Aborted");
                     default:
                        ; // blank
                  }
               }
            }
            else if (header == "Reason")
            {
               const Step* pStep = dynamic_cast<const Step*>(pMessageImp);
               if ((pStep != NULL) && ((pStep->getResult() == Message::Abort) ||
                                      (pStep->getResult() == Message::Failure)))
               {
                  return QString::fromStdString(pStep->getFailureMessage());
               }
            }
            else if (header == "Time Stamp")
            {
               string date;
               string time;
               pMessageImp->serializeDate(date, time);
               return QString::fromStdString(date) + "  " + QString::fromStdString(time);
            }
            else if ((header == "Component") && (role == Qt::DisplayRole))
            {
               return QString::fromStdString(pMessageImp->getComponent());
            }
            else if ((header == "Key") && (role == Qt::DisplayRole))
            {
               return QString::fromStdString(pMessageImp->getKey());
            }
            break;
         }
         case Qt::BackgroundRole:
         {
            if (pMessageImp->getComponent().empty() || pMessageImp->getKey().empty())
            {
               return QVariant(Qt::yellow);
            }
            if (pMessageImp->getResult() == Message::Failure)
            {
               return QVariant(Qt::red);
            }

            break;
         }
         case Qt::ForegroundRole:
         {
            if (pMessageImp->getResult() == Message::Failure)
            {
               return QVariant(Qt::white);
            }

            break;
         }
         default:
            ; // blank
      }
   }
   return QVariant();
}

bool MessageLogWindowModel::insertRows(int row, int count, const QModelIndex &parent)
{
   beginInsertRows(parent, row, row + count - 1);
   endInsertRows();
   return true;
}

QVariant MessageLogWindowModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role == Qt::DisplayRole)
   {
      if (orientation == Qt::Horizontal)
      {
         if (section < mHeaderNames.size())
         {
            return mHeaderNames[section];
         }
      }
   }
   return QVariant();
}

void MessageLogWindowModel::setMessageLog(MessageLog* pLog)
{
   if (pLog != mpLog.get())
   {
      mpLog.reset(pLog);
      reset();
   }
}

int MessageLogWindowModel::findIndex(const Step *pParent, const Message *pMessage) const
{
   int row = -1;
   if (pParent != NULL)
   {
      for (row = pParent->size() - 1; row >= 0; row--)
      {
         if ((*pParent)[row] == pMessage)
         {
            break;
         }
      }
   }
   else
   {
      for (row = mpLog->size() - 1; row >= 0; row--)
      {
         if ((*(mpLog.get()))[row] == pMessage)
         {
            break;
         }
      }
   }
   return row;
}

void MessageLogWindowModel::messageAdded(Subject &subject, const string &signal, const boost::any &data)
{
   MessageLog* pLog = dynamic_cast<MessageLog*>(&subject);
   Message* pMessage = boost::any_cast<Message*>(data);
   MessageImp* pMessageImp = dynamic_cast<MessageImp*>(pMessage);
   if (pMessageImp != NULL)
   {
      const Step* pParent = pMessageImp->getParent();
      int msgRow = findIndex(pParent, pMessage);
      if (pParent != NULL)
      {
         // get the numeric index (row) of the parent step in the grand-parent
         const MessageImp* pParentImp = dynamic_cast<const MessageImp*>(pParent);
         VERIFYNRV(pParentImp != NULL);
         int parentRow = findIndex(pParentImp->getParent(), pParent);

         // insert the new Message into the parent Step
         insertRow(pParent->size() - 1, createIndex(parentRow, 0, const_cast<Step*>(pParent)));
      }
      // if the parent is a MessageLog and the messagae is ADDED
      // than a new top-level Message has been added to the log
      else if (pLog != NULL)
      {
         // insert the new Message into the top-level
         insertRow(pLog->size() - 1);
      }
   }
}

void MessageLogWindowModel::messageFinalized(Subject &subject, const string &signal, const boost::any &data)
{
   MessageLog* pLog = dynamic_cast<MessageLog*>(&subject);
   Message* pMessage = boost::any_cast<Message*>(data);
   MessageImp* pMessageImp = dynamic_cast<MessageImp*>(pMessage);
   if (pMessageImp != NULL)
   {
      const Step* pParent = pMessageImp->getParent();
      int msgRow = findIndex(pParent, pMessage);
      if (msgRow >= 0)
      {
         // Inform the GUI that we've modified the Message/Step item so
         // we see the finalization message
         emit dataChanged(createIndex(msgRow, 0, pMessage),
            createIndex(msgRow, mHeaderNames.size(), pMessage));
      }
   }
}
