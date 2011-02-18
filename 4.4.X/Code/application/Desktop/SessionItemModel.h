/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONITEMMODEL_H
#define SESSIONITEMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include <boost/any.hpp>
#include <map>
#include <vector>

class SessionItem;
class Subject;

class SessionItemModel : public QAbstractItemModel
{
public:
   enum ItemDataRole
   {
      SessionItemRole = Qt::UserRole,
   };
   SessionItemModel(QObject* pParent = 0);
   ~SessionItemModel();

   QModelIndex index(SessionItem* pItem) const;
   QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
   QModelIndex parent(const QModelIndex& index) const;
   int rowCount(const QModelIndex& parent = QModelIndex()) const;
   int columnCount(const QModelIndex& parent = QModelIndex()) const;
   Qt::ItemFlags flags(const QModelIndex& index) const;
   bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
   QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
   void updateData(SessionItem* pItem);

protected:
   class SessionItemWrapper
   {
   public:
      SessionItemWrapper(SessionItemModel* pModel, SessionItem* pItem, SessionItemWrapper* pParent = NULL);
      ~SessionItemWrapper();

      SessionItem* getSessionItem() const;
      void setCheckState(Qt::CheckState checkState);
      Qt::CheckState getCheckState() const;
      void setDisplayName(const QString& itemName);
      QString getDisplayName() const;
      void setDisplayFont(const QFont& itemFont);
      QFont getDisplayFont() const;
      void setDisplayColor(const QColor& itemColor);
      QColor getDisplayColor() const;
      void setBackgroundColor(const QColor& itemColor);
      QColor getBackgroundColor() const;
      SessionItemWrapper* getParent() const;
      const std::vector<SessionItemWrapper*>& getChildren() const;

      void addChild(SessionItemWrapper* pWrapper);
      SessionItemWrapper* addChild(SessionItem* pItem);
      SessionItemWrapper* getChild(SessionItem* pItem) const;
      void removeChild(SessionItemWrapper* pWrapper);
      void removeChild(SessionItem* pItem);
      void clear();

   private:
      SessionItemModel* mpModel;
      SessionItem* mpSessionItem;
      Qt::CheckState mCheckState;
      QString mDisplayName;
      QFont mDisplayFont;
      QColor mDisplayColor;
      QColor mBackgroundColor;
      SessionItemWrapper* mpParent;
      std::vector<SessionItemWrapper*> mChildren;
   };

   SessionItemWrapper* getWrapper(SessionItem* pItem) const;
   SessionItemWrapper* getRootWrapper() const;
   void activateItem(SessionItem* pItem);
   void clear();

private:
   SessionItemWrapper* mpRootWrapper;
   std::map<SessionItem*, SessionItemWrapper*> mSessionItems;
};

#endif
