/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BADVALUESDLG_H
#define BADVALUESDLG_H

#include "BadValuesAdapter.h"

#include <QtCore/QString>
#include <QtGui/QDialog>
#include <QtGui/QStyledItemDelegate>

#include <string>

class QCheckBox;
class QLineEdit;
class QModelIndex;
class QTreeWidget;

class BadValuesDlg : public QDialog
{
   Q_OBJECT

public:
   BadValuesDlg(QWidget* pParent = 0, const QString& statName = QString(), bool enableSetAll = false);
   virtual ~BadValuesDlg();

   void setBadValues(const BadValues* pBadValues);
   const BadValues* getBadValues() const;
   std::string getBadValuesString() const;
   bool getSetAll() const;

public slots:
   void accept();

protected slots:
   void initialize();
   void addBadValue();
   void deleteBadValue();
   void addRange();
   void deleteRange();

private:
   BadValuesDlg(const BadValuesDlg& rhs);
   BadValuesDlg& operator=(const BadValuesDlg& rhs);
   BadValuesAdapter mBadValues;
   QCheckBox* mpEnableUpperThreshold;
   QLineEdit* mpUpperThreshold;
   QCheckBox* mpEnableLowerThreshold;
   QLineEdit* mpLowerThreshold;
   QTreeWidget* mpValues;
   QTreeWidget* mpRanges;
   QLineEdit* mpTolerance;
   QCheckBox* mpSetAll;
};

class BadValueInputDlg : public QDialog
{
   Q_OBJECT

public:
   BadValueInputDlg(QWidget* pParent = NULL);
   virtual ~BadValueInputDlg();

   QString getBadValue() const;

public slots:
   void accept();

private:
   QLineEdit* mpBadValue;
};

class RangeInputDlg : public QDialog
{
   Q_OBJECT

public:
   RangeInputDlg(QWidget* pParent = NULL);
   virtual ~RangeInputDlg();

   QString getRangeStart() const;
   QString getRangeEnd() const;

public slots:
   void accept();

private:
   QLineEdit* mpRangeStart;
   QLineEdit* mpRangeEnd;
};

class BadValueItemDelegate : public QStyledItemDelegate
{
public:
   BadValueItemDelegate(QObject* pParent = NULL);
   virtual ~BadValueItemDelegate();

   QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   void setEditorData(QWidget* pEditor, const QModelIndex& index) const;
   void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const;
   void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
   BadValueItemDelegate(const BadValueItemDelegate& rhs);
   BadValueItemDelegate& operator=(const BadValueItemDelegate& rhs);
};

#endif
