/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GETSUBSETDIALOG_H__
#define GETSUBSETDIALOG_H__

#include <QtGui/QDialog>
#include <vector>

class QComboBox;
class QLineEdit;
class QListWidget;
class QStringList;

class GetSubsetDialog : public QDialog
{
   Q_OBJECT

public:
   GetSubsetDialog(const QString& plotName,
                   const QStringList& aoiNames,
                   const QStringList& bands,
                   const std::vector<int>& defaultSelection,
                   QWidget* pParent = NULL);
   virtual ~GetSubsetDialog();

   QString getPlotName() const;
   QString getSelectedAoi() const;
   std::vector<int> getBandSelectionIndices() const;

private:
   GetSubsetDialog(const GetSubsetDialog& rhs);
   GetSubsetDialog& operator=(const GetSubsetDialog& rhs);
   QLineEdit* mpName;
   QComboBox* mpAoiSelect;
   QListWidget* mpBandSelect;
};

#endif