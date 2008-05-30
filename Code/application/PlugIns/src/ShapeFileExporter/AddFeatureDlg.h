/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ADDFEATUREDLG_H
#define ADDFEATUREDLG_H

#include <QtCore/QMap>
#include <QtGui/QDialog>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include <vector>

class DataElement;

class AddFeatureDlg : public QDialog
{
   Q_OBJECT

public:
   AddFeatureDlg(QWidget* parent = 0);
   ~AddFeatureDlg();

   std::vector<DataElement*> getDataElements() const;

protected:
   void accept();

private:
   QTreeWidget* mpElementTree;
   QMap<QTreeWidgetItem*, DataElement*> mElements;
};

#endif
