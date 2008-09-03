/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>

#include "MessageLogWindow.h"

#include "AppAssert.h"
#include "DynamicObject.h"
#include "Icons.h"
#include "MessageLogMgrImp.h"
#include "MessageLogAdapter.h"
#include "SessionManager.h"
#include "SessionManagerImp.h"

#include <memory>
#include <string>
using namespace std;

// MessageLogWindow

MessageLogWindow::MessageLogWindow(const string& id, QWidget* parent) :
   DockWindowAdapter(id, "Message Log Window", parent),
   mpLogs(NULL),
   mpModel(NULL)
{
   QFrame *pTopFrame = new QFrame(this);
   mpLogs = new QComboBox(pTopFrame);
   mpModel = new MessageLogWindowModel;
   QTreeView *pTreeView = new QTreeView;
   pTreeView->setModel(mpModel);
   pTreeView->setSelectionMode(QAbstractItemView::NoSelection);
   pTreeView->setRootIsDecorated(true);
   QHeaderView* pHeader = pTreeView->header();
   if(pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setSortIndicatorShown(false);
   }

   QVBoxLayout *pTopLayout = new QVBoxLayout(pTopFrame);
   pTopLayout->addWidget(pTreeView, 10);
   pTopLayout->addWidget(mpLogs, 0);

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setIcon(pIcons->mMessageLog);

   setWidget(pTopFrame);

   Service<MessageLogMgr> pMsgLogMgr;
   mpMsgLogMgr.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &MessageLogWindow::messageLogAdded));
   mpMsgLogMgr.reset(pMsgLogMgr.get());
   mpAppSrvcs.addSignal(SIGNAL_NAME(ApplicationServices, SessionClosed), Slot(this, &MessageLogWindow::sessionClosed));
   mpAppSrvcs.reset(Service<ApplicationServices>().get());

   connect(mpLogs, SIGNAL(activated(const QString &)), this, SLOT(setLog(const QString &)));

   Service<SessionManager> pSessionMgr;
   pMsgLogMgr->getLog(pSessionMgr->getName()); //force the creation of the session log
   setLogs(pMsgLogMgr->getLogs());
   setLog(QString::fromStdString(pSessionMgr->getName()));
}

MessageLogWindow::~MessageLogWindow()
{
}

void MessageLogWindow::messageLogAdded(Subject &subject, const string &signal, const boost::any &data)
{
   setLogs(mpMsgLogMgr->getLogs());
}

void MessageLogWindow::sessionClosed(Subject &subject, const string &signal, const boost::any &data)
{
   setLogs(vector<MessageLog*>(0));
}

void MessageLogWindow::setLogs(const vector<MessageLog*> &logs)
{
   QString currentName;
   if(mpLogs->count() > 0)
   {
      currentName = mpLogs->currentText();
   }
   mpLogs->clear();
   if(logs.empty())
   {
      mpModel->setMessageLog(NULL);
      return;
   }
   Service<SessionManager> pSessionMgr;
   string sessionLogName = pSessionMgr->getName();
   bool setLog = true;
   for(vector<MessageLog*>::const_iterator log = logs.begin(); log != logs.end(); ++log)
   {
      MessageLog *pLog = *log;
      if(pLog == NULL)
      {
         continue;
      }

      // Get the log name
      string logName = pLog->getLogName();
      if(logName.empty())
      {
         continue;
      }
      QString strName = QString::fromStdString(logName);
      if (logName == sessionLogName)
      {
         strName = "Session Log";
      }
      
      if(strName == currentName)
      {
         setLog = false;
      }
      mpLogs->addItem(strName);
   }
   if(setLog)
   {
      mpModel->setMessageLog(logs.front());
      mpLogs->setCurrentIndex(0);
   }
   else
   {
      mpLogs->setCurrentIndex(mpLogs->findText(currentName));
   }
}

void MessageLogWindow::setLog(const QString &logName)
{
   if(mpLogs->count() == 0)
   {
      mpModel->setMessageLog(NULL);
      return;
   }
   string realLogName = logName.toStdString();
   QString aliasedLogName = logName;
   if (aliasedLogName == QString::fromStdString(Service<SessionManager>()->getName()))
   {
      aliasedLogName = QString::fromStdString("Session Log");
   }
   if (realLogName == "Session Log")
   {
      realLogName = Service<SessionManager>()->getName();
   }
   vector<MessageLog*> logs = mpMsgLogMgr->getLogs();
   for(vector<MessageLog*>::const_iterator lIter = logs.begin(); lIter != logs.end(); ++lIter)
   {
      MessageLog *pLog = *lIter;
      if((pLog != NULL) && (realLogName == pLog->getLogName()))
      {
         int idx = mpLogs->findText(aliasedLogName);
         if(idx == -1)
         {
            setLogs(mpMsgLogMgr->getLogs());
            idx = mpLogs->findText(aliasedLogName);
         }
         if(idx != -1)
         {
            mpLogs->setCurrentIndex(idx);
            mpModel->setMessageLog(pLog);
         }
         return;
      }
   }
}

/*************************
 **  MessageLogWindowModel
 *************************/
MessageLogWindowModel::MessageLogWindowModel(QObject *pParent) : mpLog(NULL)
{
   mHeaderNames << "ID" << "Type" << "Message" << "Result" << "Reason" << "Time Stamp" << "Component" << "Key";
}

MessageLogWindowModel::~MessageLogWindowModel()
{
}

QModelIndex MessageLogWindowModel::index(int row, int column, const QModelIndex &parent) const
{
   if(mpLog == NULL)
   {
      return QModelIndex();
   }

   const Message *pMessage = NULL;
   if(parent.isValid())
   {
      Subject *pSubject = reinterpret_cast<Subject*>(parent.internalPointer());
      Message *pParent = dynamic_cast<Message*>(pSubject);
      Step *pParentStep = dynamic_cast<Step*>(pParent);

      // the parent is a step
      if(pParentStep != NULL)
      {
         if(row < static_cast<int>(pParentStep->size()))
         {
            pMessage = (*pParentStep)[row];
         }
         else
         {
            const DynamicObject *pProperties = pParent->getProperties();
            mPropertyCache.insert(pProperties, pParent);
            return createIndex(row, column,
                           const_cast<Subject*>(static_cast<const Subject*>(pProperties)));
         }
      }
      // the parent is a message
      else if(pParent != NULL)
      {
         const DynamicObject *pProperties = pParent->getProperties();
         mPropertyCache.insert(pProperties, pParent);
         return createIndex(row, column, const_cast<Subject*>(static_cast<const Subject*>(pProperties)));
      }
   }
   else if(row < static_cast<int>(mpLog->size()))
   {
      pMessage = (*mpLog)[row];
   }
   return createIndex(row, column, const_cast<Subject*>(static_cast<const Subject*>(pMessage)));
}

QModelIndex MessageLogWindowModel::parent(const QModelIndex &index) const
{
   if(mpLog == NULL || !index.isValid())
   {
      return QModelIndex();
   }

   const Subject *pSubject = reinterpret_cast<Subject*>(index.internalPointer());
   if(pSubject == NULL)
   {
      return QModelIndex();
   }

   // find the parent
   const Message *pParent = NULL;
   const DynamicObject *pProperties = dynamic_cast<const DynamicObject*>(pSubject);
   if(pProperties != NULL)
   {
      // this is a property index
      pParent = mPropertyCache[pProperties];
   }
   else
   {
      const MessageImp *pMessageImp = dynamic_cast<const MessageImp*>(pSubject);
      if(pMessageImp != NULL)
      {
         pParent = pMessageImp->getParent();
      }
   }

   // locate the row number
   const MessageImp *pParentImp = dynamic_cast<const MessageImp*>(pParent);
   if(pParentImp == NULL)
   {
      return QModelIndex();
   }
   int parentRow = 0;
   const Step *pGrandParent = pParentImp->getParent();
   if(pGrandParent != NULL)
   {
      for(Step::const_iterator gpIter = pGrandParent->begin(); gpIter != pGrandParent->end(); ++gpIter)
      {
         if(*gpIter == pParent)
         {
            break;
         }
         parentRow++;
      }
   }
   else
   {
      for(MessageLog::const_iterator lIter = mpLog->begin(); lIter != mpLog->end(); ++lIter)
      {
         if(*lIter == pParent)
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
   if(mpLog == NULL)
   {
      return 0;
   }

   // if the parent is invalid, the request is for the number of top level items
   if(!parent.isValid())
   {
      return mpLog->size();
   }

   const Subject *pSubject = reinterpret_cast<Subject*>(parent.internalPointer());
   const Message *pParent = dynamic_cast<const Message*>(pSubject);
   if(pParent == NULL)
   {
      return 0;
   }
   const Step *pParentStep = dynamic_cast<const Step*>(pParent);

   // if the parent is a Step, return the count of subitems
   if(pParentStep != NULL)
   {
      return pParentStep->size() + pParent->getProperties()->getNumAttributes();
   }

   // return the count of properteis
   return pParent->getProperties()->getNumAttributes();
}

int MessageLogWindowModel::columnCount(const QModelIndex &parent) const
{
   if(mpLog == NULL)
   {
      return 0;
   }
   return mHeaderNames.size();
}

QVariant MessageLogWindowModel::data(const QModelIndex &index, int role) const
{
   if(mpLog == NULL || !index.isValid())
   {
      return QVariant();
   }

   const Subject *pSubject = reinterpret_cast<Subject*>(index.internalPointer());
   const DynamicObject *pProperties = dynamic_cast<const DynamicObject*>(pSubject);
   if(pProperties != NULL)
   {
      if((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
      {
         const Message *pParent = mPropertyCache[pProperties];
         const Step *pParentStep = dynamic_cast<const Step*>(pParent);
         // this is a property index
         unsigned int propertyNumber = index.row();
         if(pParentStep != NULL)
         {
            propertyNumber -= pParentStep->size();
         }
         if(propertyNumber < pProperties->getNumAttributes())
         {
            vector<string> propertyNames;
            pProperties->getAttributeNames(propertyNames);
            string name = propertyNames[propertyNumber];

            QString header(headerData(index.column(), Qt::Horizontal).toString());
            if(header == "ID")
            {
               return QString::fromStdString(name);
            }
            else if(header == "Type")
            {
               string type;
               const DataVariant& attrValue = pProperties->getAttribute(name);
               if (attrValue.isValid())
               {
                  type = attrValue.getTypeName();
               }
               return QString::fromStdString(type);
            }
            else if(header == "Message")
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
      const MessageImp *pMessageImp = dynamic_cast<const MessageImp*>(pSubject);
      if(pMessageImp == NULL)
      {
         return QVariant();
      }
      switch(role)
      {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
         {
            QString header(headerData(index.column(), Qt::Horizontal).toString());
            if((header == "ID") && (role == Qt::DisplayRole))
            {
               return QString::fromStdString(const_cast<MessageImp*>(pMessageImp)->getStringId());
            }
            else if((header == "Type") && (role == Qt::DisplayRole))
            {
               if(dynamic_cast<const Step*>(pMessageImp) != NULL)
               {
                  return QString("Step");
               }
               else
               {
                  return QString("Message");
               }
            }
            else if(header == "Message")
            {
               return QString::fromStdString(pMessageImp->getAction());
            }
            else if((header == "Result") && (role == Qt::DisplayRole))
            {
               const Step *pStep = dynamic_cast<const Step*>(pMessageImp);
               if(pStep != NULL)
               {
                  switch(pStep->getResult())
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
            else if(header == "Reason")
            {
               const Step *pStep = dynamic_cast<const Step*>(pMessageImp);
               if((pStep != NULL) && ((pStep->getResult() == Message::Abort) ||
                                      (pStep->getResult() == Message::Failure)))
               {
                  return QString::fromStdString(pStep->getFailureMessage());
               }
            }
            else if(header == "Time Stamp")
            {
               string date, time;
               pMessageImp->serializeDate(date, time);
               return QString::fromStdString(date) + "  " + QString::fromStdString(time);
            }
            else if((header == "Component") && (role == Qt::DisplayRole))
            {
               return QString::fromStdString(pMessageImp->getComponent());
            }
            else if((header == "Key") && (role == Qt::DisplayRole))
            {
               return QString::fromStdString(pMessageImp->getKey());
            }
            break;
         }
         case Qt::BackgroundRole:
         {
            if(pMessageImp->getComponent().empty() || pMessageImp->getKey().empty())
            {
               return QVariant(Qt::yellow);
            }
            if(pMessageImp->getResult() == Message::Failure)
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
   if(role == Qt::DisplayRole)
   {
      if(orientation == Qt::Horizontal)
      {
         if(section < mHeaderNames.size())
         {
            return mHeaderNames[section];
         }
      }
   }
   return QVariant();
}

void MessageLogWindowModel::setMessageLog(const MessageLog *pLog)
{
   if(pLog == mpLog)
   {
      return;
   }
   if(mpLog != NULL)
   {
      const_cast<MessageLog*>(mpLog)->detach(SIGNAL_NAME(MessageLog, MessageAdded), Slot(this, &MessageLogWindowModel::messageAdded));
      const_cast<MessageLog*>(mpLog)->detach(SIGNAL_NAME(MessageLog, MessageHidden), Slot(this, &MessageLogWindowModel::messageFinalized));
   }
   mpLog = pLog;
   if(mpLog != NULL)
   {
      const_cast<MessageLog*>(mpLog)->attach(SIGNAL_NAME(MessageLog, MessageAdded), Slot(this, &MessageLogWindowModel::messageAdded));
      const_cast<MessageLog*>(mpLog)->attach(SIGNAL_NAME(MessageLog, MessageHidden), Slot(this, &MessageLogWindowModel::messageFinalized));
   }
   reset();
}

int MessageLogWindowModel::findIndex(const Step *pParent, const Message *pMessage) const
{
   int row = -1;
   if(pParent != NULL)
   {
      for(row = pParent->size() - 1; row >= 0; row--)
      {
         if((*pParent)[row] == pMessage)
         {
            break;
         }
      }
   }
   else
   {
      for(row = mpLog->size() - 1; row >= 0; row--)
      {
         if((*mpLog)[row] == pMessage)
         {
            break;
         }
      }
   }
   return row;
}

void MessageLogWindowModel::messageAdded(Subject &subject, const string &signal, const boost::any &data)
{
   MessageLog *pLog = dynamic_cast<MessageLog*>(&subject);
   Message *pMessage = boost::any_cast<Message*>(data);
   MessageImp *pMessageImp = dynamic_cast<MessageImp*>(pMessage);
   if (pMessageImp != NULL)
   {
      const Step *pParent = pMessageImp->getParent();
      int msgRow = findIndex(pParent, pMessage);
      if(pParent != NULL)
      {
         // get the numeric index (row) of the parent step in the grand-parent
         const MessageImp *pParentImp = dynamic_cast<const MessageImp*>(pParent);
         VERIFYNRV(pParentImp != NULL);
         int parentRow = findIndex(pParentImp->getParent(), pParent);

         // insert the new Message into the parent Step
         insertRow(pParent->size() - 1, createIndex(parentRow, 0, const_cast<Step*>(pParent)));
      }
      // if the parent is a MessageLog and the messagae is ADDED
      // than a new top-level Message has been added to the log
      else if(pLog != NULL)
      {
         // insert the new Message into the top-level
         insertRow(pLog->size() - 1);
      }
   }
}

void MessageLogWindowModel::messageFinalized(Subject &subject, const string &signal, const boost::any &data)
{
   MessageLog *pLog = dynamic_cast<MessageLog*>(&subject);
   Message *pMessage = boost::any_cast<Message*>(data);
   MessageImp *pMessageImp = dynamic_cast<MessageImp*>(pMessage);
   if (pMessageImp != NULL)
   {
      const Step *pParent = pMessageImp->getParent();
      int msgRow = findIndex(pParent, pMessage);
      if(msgRow >= 0)
      {
         // Inform the GUI that we've modified the Message/Step item so
         // we see the finalization message
         emit dataChanged(createIndex(msgRow, 0, pMessage),
            createIndex(msgRow, mHeaderNames.size(), pMessage));
      }
   }
}
