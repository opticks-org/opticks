/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef BANDBADVALUESDLG_H
#define BANDBADVALUESDLG_H

#include <QtGui/QDialog>

#include <vector>

class QLineEdit;
class QCheckBox;

class BandBadValuesDlg : public QDialog
{
   Q_OBJECT

public:
   BandBadValuesDlg(QWidget* parent = 0);
   ~BandBadValuesDlg();

   void setBadValues(const std::vector<int> & badValues);
   void getBadValues(std::vector<int>& badValues) const;
   bool getChangeForAllBands() const;
   void setChangeForAllBands(bool& value);

protected slots:
   void okPressed();

private:
   BandBadValuesDlg(const BandBadValuesDlg& rhs);
   BandBadValuesDlg& operator=(const BandBadValuesDlg& rhs);
   QLineEdit* mpBandBadValue;
   QCheckBox* mpChkBoxAllBands;
   std::vector<int> mBadValues;
};

#endif
