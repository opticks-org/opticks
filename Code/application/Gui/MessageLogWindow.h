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
#include <QtCore/QHash>
#include <QtCore/QModelIndex>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QComboBox>

#include "AttachmentPtr.h"
#include "DockWindowAdapter.h"
#include "MessageLog.h"
#include "MessageLogMgr.h"

#include <string>

class DynamicObject;
class Message;
class MessageLogWindowModel;
class Step;

class MessageLogWindow : public DockWindowAdapter
{
   Q_OBJECT

public:
   MessageLogWindow(const std::string& id, QWidget* pParent = NULL);
   virtual ~MessageLogWindow();

protected:
   void messageLogAdded(Subject& subject, const std::string& signal, const boost::any& value);
   void messageLogRemoved(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void setLog(const QString& logName);

private:
   MessageLogWindow(const MessageLogWindow& rhs);
   MessageLogWindow& operator=(const MessageLogWindow& rhs);
   static QString getLogDisplayName(const MessageLog* pLog);
   static std::string getLogName(const QString& displayName);

   QComboBox* mpLogCombo;
   MessageLogWindowModel* mpModel;
   AttachmentPtr<MessageLogMgr> mpMsgLogMgr;
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

   private:
      DataElement(const DataElement& rhs);
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

   void messageFinalized(Subject& subject, const std::string& signal, const boost::any& value);
   void messageAdded(Subject& subject, const std::string& signal, const boost::any& value);

public slots:
   void setMessageLog(MessageLog* pLog);

protected:
   int findIndex(const Step *pParent, const Message *pMessage) const;

private:
   MessageLogWindowModel(const MessageLogWindowModel& rhs);
   MessageLogWindowModel& operator=(const MessageLogWindowModel& rhs);
   QStringList mHeaderNames;
   AttachmentPtr<MessageLog> mpLog;
   mutable QHash<const DynamicObject*, const Message*> mPropertyCache;
};

#endif // MESSAGELOGWINDOW_H
