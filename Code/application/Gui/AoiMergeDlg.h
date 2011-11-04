/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef AOIMERGEDLG_H
#define AOIMERGEDLG_H

#include <QtCore/QStringList>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QRadioButton>

class AoiMergeDlg : public QDialog
{
   Q_OBJECT

public:
   AoiMergeDlg(QWidget* parent = 0);
   ~AoiMergeDlg();

   QStringList getMergeAoiNames();
   QString getOutputAoiName();
   bool combinePixels();

   QSize sizeHint() const;

protected slots:
   void accept();

private:
   AoiMergeDlg(const AoiMergeDlg& rhs);
   AoiMergeDlg& operator=(const AoiMergeDlg& rhs);
   QListWidget* mpMergeList;
   QRadioButton* mpCombineRadio;
   QComboBox* mpOutputCombo;
};

#endif
