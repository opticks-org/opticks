/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DYNAMICOBJECTITEMMODEL_H
#define DYNAMICOBJECTITEMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMetaType>

#include "DataVariant.h"

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

class DynamicObject;
class Subject;

Q_DECLARE_METATYPE(DataVariant*)

class DynamicObjectItemModel : public QAbstractItemModel
{
public:
   DynamicObjectItemModel(QObject* pParent, DynamicObject* pDynamicObject);
   ~DynamicObjectItemModel();

   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   QModelIndex index(DynamicObject* pDynamicObject) const;
   QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
   QModelIndex parent(const QModelIndex& index) const;
   int rowCount(const QModelIndex& parent = QModelIndex()) const;
   int columnCount(const QModelIndex& parent = QModelIndex()) const;
   QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

   void setDynamicObject(DynamicObject* pDynamicObject);

protected:
   class AttributeWrapper
   {
   public:
      AttributeWrapper(DynamicObjectItemModel* pModel, const std::string& name, DataVariant* pValue,
         AttributeWrapper* pParent = NULL);
      ~AttributeWrapper();

      const std::string& getName() const;
      void setValue(DataVariant* pValue);
      DataVariant* getValue() const;
      AttributeWrapper* getParent() const;
      const std::vector<AttributeWrapper*>& getChildren() const;

      void addChild(AttributeWrapper* pWrapper);
      AttributeWrapper* addChild(const std::string& name, DataVariant* pValue);
      AttributeWrapper* getChild(const std::string& name, DataVariant* pValue) const;
      bool removeChild(AttributeWrapper* pWrapper);
      bool removeChild(const std::string& name, DataVariant* pValue);
      void clear();

   private:
      DynamicObjectItemModel* mpModel;
      std::string mName;
      DataVariant* mpValue;
      AttributeWrapper* mpParent;
      std::vector<AttributeWrapper*> mChildren;
   };

   void addAttribute(Subject& subject, const std::string& signal, const boost::any& value);
   void modifyAttribute(Subject& subject, const std::string& signal, const boost::any& value);
   void removeAttribute(Subject& subject, const std::string& signal, const boost::any& value);
   void removeAllAttributes(Subject& subject, const std::string& signal, const boost::any& value);
   void dynamicObjectDeleted(Subject& subject, const std::string& signal, const boost::any& value);

   AttributeWrapper* addAttributeItem(AttributeWrapper* pParentWrapper, const std::string& name,
      DataVariant* pValue);
   void removeAttributeItem(AttributeWrapper* pParentWrapper, const std::string& name, DataVariant* pValue);

   AttributeWrapper* getWrapper(DynamicObject* pDynamicObject) const;
   void clear();

private:
   void initializeDynamicObjectItem(AttributeWrapper* pWrapper, DynamicObject* pDynamicObject);
   void cleanupDynamicObjectItem(AttributeWrapper* pWrapper, DynamicObject* pDynamicObject);

   DynamicObject* mpDynamicObject;
   AttributeWrapper* mpRootWrapper;
   std::map<DynamicObject*, AttributeWrapper*> mDynamicObjects;
};

#endif
