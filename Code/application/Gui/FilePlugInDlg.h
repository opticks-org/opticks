/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEPLUGINDLG_H
#define FILEPLUGINDLG_H

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

#include "TypesFile.h"

class PlugInDescriptor;

class QLabel;

class FilePlugInDlg : public QFileDialog
{
   Q_OBJECT

public:
   FilePlugInDlg(const std::vector<PlugInDescriptor*>& availablePlugins, const std::string plugInKey,
      QWidget* parent = 0);
   ~FilePlugInDlg();

   void setSelectedPlugIn(const QString& strPlugIn);
   QString getSelectedPlugIn() const;
   bool isDefaultPlugIn() const;

public slots:
   void enableOptions(bool bEnable);
   void accept();

signals:
   void plugInSelected(const QString& strPlugIn);
   void optionsClicked();

protected:
   QWidget* getPlugInWidget() const;
   bool isDefaultExtension(const QString& strExtension) const;
   void setPlugInLabel(const QString &label);
   static std::vector<PlugInDescriptor*> getPlugInNames(const std::string &plugInType, std::string subtype);

protected slots:
   void updateFileFilters(const QString& strPlugIn);

private:
   FilePlugInDlg(const FilePlugInDlg& rhs);
   FilePlugInDlg& operator=(const FilePlugInDlg& rhs);
   static std::map<std::string, QString> mLastPlugIns;
   QMap<QString, QStringList> mPlugInFilters;
   QWidget* mpPlugInWidget;
   QComboBox* mpPlugInCombo;
   QPushButton* mpOptionsButton;
   QLabel* mpPlugInLabel;
   std::string mPlugInKey;
};

#endif
