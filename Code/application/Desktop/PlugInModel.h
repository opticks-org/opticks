/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINMODEL_H
#define PLUGINMODEL_H

#include "AttachmentPtr.h"
#include "PlugInManagerServices.h"
#include "SessionItemModel.h"

class ModuleDescriptor;

class PlugInModel : public SessionItemModel
{
public:
   PlugInModel(QObject* pParent = 0);
   virtual ~PlugInModel();

   virtual Qt::ItemFlags flags(const QModelIndex& index) const;
   virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

   void addPlugIn(Subject& subject, const std::string& signal, const boost::any& value);
   void removePlugIn(Subject& subject, const std::string& signal, const boost::any& value);
   void addModule(Subject& subject, const std::string& signal, const boost::any& value);
   void removeModule(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   void addModuleItem(ModuleDescriptor* pModule);
   void removeModuleItem(ModuleDescriptor* pModule);

private:
   PlugInModel(const PlugInModel& rhs);
   PlugInModel& operator=(const PlugInModel& rhs);

   AttachmentPtr<PlugInManagerServices> mpManager;
};

#endif
