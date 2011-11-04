/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef GEOREFERENCEDLG_H
#define GEOREFERENCEDLG_H

#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QStackedWidget>

#include "TypesFile.h"

#include <string>
#include <vector>

class GeoreferenceDlg : public QDialog
{
   Q_OBJECT

public:
   GeoreferenceDlg(const QString& title, const std::vector<std::string>& geoPluginNameList,
      const std::vector<QWidget*>& geoPluginWidgetList, QWidget* parent = NULL);
   ~GeoreferenceDlg();

   void setResultsName(const std::string& name);
   std::string getResultsName() const;
   GeocoordType getGeocoordType() const;

   int getGeorefAlgorithmIndex() const;

protected slots:
   void setPlugin(int iPluginIndex);
   void accept();

private:
   GeoreferenceDlg(const GeoreferenceDlg& rhs);
   GeoreferenceDlg& operator=(const GeoreferenceDlg& rhs);

   QListWidget* mpGeoList;
   QStackedWidget* mpStack;

   QLineEdit* mpResultsEdit;
   QComboBox* mpCoordCombo;

   std::vector<QWidget*> mWidgets;
};

#endif // GEOREFERENCEDLG_H
