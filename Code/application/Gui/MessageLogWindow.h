/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGELOGWINDOW_H
#define MESSAGELOGWINDOW_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QEvent>
#include <QtCore/QHash>
#include <QtCore/QModelIndex>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QComboBox>
#include <QtGui/QTreeView>

#include "ApplicationServices.h"
#include "AttachmentPtr.h"
#include "DockWindowAdapter.h"
#include "MessageLogMgr.h"

#include <map>

class DynamicObject;
class Message;
class MessageLog;
class MessageLogWindowModel;
class Step;

class MessageLogWindow : public DockWindowAdapter
{
   Q_OBJECT

public:
   MessageLogWindow(const std::string& id, QWidget* parent = 0);
   ~MessageLogWindow();

   void sessionClosed(Subject &subject, const std::string &signal, const boost::any &data);
   void messageLogAdded(Subject &subject, const std::string &signal, const boost::any &data);

protected slots:
   void setLogs(const std::vector<MessageLog*> &logs);
   void setLog(const QString &logName);

private:
   std::map<MessageLog*, QTreeView*> mLogs;
   QComboBox* mpLogs;
   MessageLogWindowModel* mpModel;
   AttachmentPtr<MessageLogMgr> mpMsgLogMgr;
   AttachmentPtr<ApplicationServices> mpAppSrvcs;
};


class MessageLogWindowModel : public QAbstractItemModel
{
   Q_OBJECT

   class DataElement
   {
   public:
      Message* mpMessage;
      bool mIsProperties;

      DataElement(Message* pMessage, bool isProperties = false) :
         mpMessage(pMessage),
         mIsProperties(isProperties)
      {
      }
   };

public:
   MessageLogWindowModel(QObject *pParent = 0);
   ~MessageLogWindowModel();

   // from QAbstractItemModel
   virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
   virtual QModelIndex parent(const QModelIndex &index) const;
   virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
   virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

   void messageFinalized(Subject &subject, const std::string &signal, const boost::any &data);
   void messageAdded(Subject &subject, const std::string &signal, const boost::any &data);

public slots:
   void setMessageLog(const MessageLog *pLog);

protected:
   int findIndex(const Step *pParent, const Message *pMessage) const;

private:
   QStringList mHeaderNames;
   const MessageLog* mpLog;
   mutable QHash<const DynamicObject*, const Message*> mPropertyCache;
};

#endif // MESSAGELOGWINDOW_H
