/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ALPHANUMERICSORTFILTERPROXYMODEL_H
#define ALPHANUMERICSORTFILTERPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

class AlphaNumericSortFilterProxyModel : public QSortFilterProxyModel
{
public:
   AlphaNumericSortFilterProxyModel(QObject* pParent = 0);
   virtual ~AlphaNumericSortFilterProxyModel();

protected:
   virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

#endif