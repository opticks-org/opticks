/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef LINKDATASETPAGE_H
#define LINKDATASETPAGE_H

#include <QtGui/QComboBox>
#include <QtGui/QWidget>

class LinkDatasetPage : public QWidget
{
   Q_OBJECT

public:
   LinkDatasetPage(QWidget* parent = 0);
   ~LinkDatasetPage();

   QString getSelectedDatasetName() const;

private:
   QComboBox* mpDatasetCombo;
};

#endif
