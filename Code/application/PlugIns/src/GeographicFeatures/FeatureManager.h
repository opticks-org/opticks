/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATUREMANAGER_H
#define FEATUREMANAGER_H

#include "AlgorithmShell.h"
#include "AttachmentPtr.h"
#include "ConfigurationSettings.h"
#include "ConnectionParameters.h"
#include "DynamicObject.h"
#include "SessionExplorer.h"

#include <QtCore/QObject>
#include <QtGui/QAction>
#include <vector>
#include <boost/any.hpp>

class FeatureClass;
class FeatureProxyConnector;
class MenuBar;

class FeatureManager : public QObject, public AlgorithmShell
{
   Q_OBJECT

public:
   FeatureManager();
   ~FeatureManager();

   SETTING_PTR(Options, FeatureManager, DynamicObject);
   SETTING(OptionsVersion, FeatureManager, unsigned int, 0);
   SETTING(ArcProxyExecutable, FeatureManager, std::string, "ArcProxy");

   bool execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList);

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

   bool refresh();

   static const std::string PLUGIN_NAME;
   static const unsigned int mCurrentVersion;

   FeatureProxyConnector *getProxy();

   void updateContextMenu(Subject &subject, const std::string &signal, const boost::any &data);
   void updatePropertiesDialog(Subject& subject, const std::string& signal, const boost::any& data);

   static std::vector<FeatureClass*> getFeatureClasses(const std::vector<SessionItem*> sessionItems,
      std::vector<SessionItem*> *pFilteredSessionItems = NULL);

protected slots:
   void menuItemTriggered(QAction *pAction);
   void openEditDlg();
   void proxyInitialized();
   void importGeodatabase();
   void refreshFeatureClass();
   void exportFeatureClass();
   void displayFeatureClassProperties();

private:
   FeatureManager(const FeatureManager& rhs);
   FeatureManager& operator=(const FeatureManager& rhs);
   AttachmentPtr<SessionExplorer> mpExplorer;

   bool refresh(MenuBar &menuBar);
   bool addMenuItems(MenuBar &menuBar);

   static const std::string MENU_NAME;
   static const std::string EDIT;
   static const std::string IMPORT;

   std::vector<QAction*> mImportActions;
   QAction* mpSeparator;

   FeatureProxyConnector* mpProxy;
};

#endif
