/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONNECTIONPARAMETERSWIDGET_H
#define CONNECTIONPARAMETERSWIDGET_H

#include <QtGui/QWidget>
#include "ConnectionParameters.h"

class QCheckBox;
class QLineEdit;
class QPushButton;
class QRadioButton;

class ConnectionParametersWidget : public QWidget
{
   Q_OBJECT

public:
   ConnectionParametersWidget(QWidget *pParent = NULL);
   ~ConnectionParametersWidget();

   ArcProxyLib::ConnectionParameters getConnectionParameters() const;
   void setConnectionParameters(const ArcProxyLib::ConnectionParameters &connection);

   void setAvailableConnectionTypes(
      const std::vector<ArcProxyLib::ConnectionType> &types);

   bool isModified() const;

public slots:
   void setModified(bool modified = true);
   void selectFileClicked();
   void updateWidgets();

private:
   QRadioButton* mpSdeButton;
   QRadioButton* mpShpButton;
   QCheckBox* mpUseArcCheck;

   QLineEdit* mpFilenameEdit;
   QPushButton* mpSelectFileButton;

   QLineEdit* mpUserEdit;
   QLineEdit* mpPasswordEdit;
   QLineEdit* mpDatabaseEdit;
   QLineEdit* mpServerEdit;
   QLineEdit* mpInstanceEdit;
   QLineEdit* mpVersionEdit;
   QLineEdit* mpFeatureClassEdit;

   std::vector<QWidget*> mFileWidgets;
   std::vector<QWidget*> mDatabaseWidgets;

   bool mModified;
};

#endif
