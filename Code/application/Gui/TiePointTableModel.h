/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTTABLEMODEL_H
#define TIEPOINTTABLEMODEL_H

#include <QtCore/QAbstractTableModel>

#include "TiePointList.h"

#include <boost/any.hpp>

#include <string>
#include <vector>

class Subject;
class TiePointList;

class TiePointTableModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   TiePointTableModel(QObject* pParent);
   ~TiePointTableModel();

   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   int rowCount(const QModelIndex& parent = QModelIndex()) const;
   int columnCount(const QModelIndex& parent = QModelIndex()) const;
   Qt::ItemFlags flags(const QModelIndex& index) const;
   bool setData(const QModelIndex& index, const QVariant& value, int role);
   QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

   void setTiePointList(TiePointList* pTiePointList);

signals:
   void pointsModified(const std::vector<TiePoint>& oldPoints, const std::vector<TiePoint>& newPoints);

protected:
   void tiePointListModified(Subject& subject, const std::string& signal, const boost::any& value);
   void tiePointListDeleted(Subject& subject, const std::string& signal, const boost::any& value);
   void addTiePoints();
   void clear();

private:
   TiePointTableModel(const TiePointTableModel& rhs);
   TiePointTableModel& operator=(const TiePointTableModel& rhs);
   TiePointList* mpTiePointList;    // This is not an AttachmentPtr because the signal needs to be detached
                                    // and then reattached when updating the tie points in the model element
   int mRows;
};

#endif
