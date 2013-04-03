/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ADDQUERIESDLG_H
#define ADDQUERIESDLG_H

#include <QtGui/QDialog>

#include <string>
#include <vector>

class QComboBox;
class QCheckBox;

class AddQueriesDlg : public QDialog
{
   Q_OBJECT
public:
   AddQueriesDlg(const std::vector<std::string>& fields, QWidget* pParent = NULL);
   virtual ~AddQueriesDlg();

   std::string getAttribute() const;
   bool isFillColorUnique() const;
   bool isLineColorUnique() const;

private:
   QComboBox* mpAttributeCombo;
   QCheckBox* mpFillColorBox;
   QCheckBox* mpLineColorBox;
};

#endif
