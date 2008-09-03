/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ELEMENTMODEL_H
#define ELEMENTMODEL_H

#include "SessionItemModel.h"

class DataElement;

class ElementModel : public SessionItemModel
{
public:
   ElementModel(QObject* pParent = 0);
   ~ElementModel();

   Qt::ItemFlags flags(const QModelIndex& index) const;
   QStringList mimeTypes() const;
   QMimeData *mimeData(const QModelIndexList &indexes) const;
   void addElement(Subject& subject, const std::string& signal, const boost::any& value);
   void removeElement(Subject& subject, const std::string& signal, const boost::any& value);
   void updateElementParent(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   void addElementItem(DataElement* pElement);
   void removeElementItem(DataElement* pElement);
};

#endif
