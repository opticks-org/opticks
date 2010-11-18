/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESPLUGINDESCRIPTOR_H
#define PROPERTIESPLUGINDESCRIPTOR_H

#include <QtGui/QLabel>
#include <QtGui/QTextEdit>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

#include "LabeledSectionGroup.h"

#include <string>

class LabeledSection;

class PropertiesPlugInDescriptor : public LabeledSectionGroup
{
public:
   PropertiesPlugInDescriptor();
   ~PropertiesPlugInDescriptor();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

   static const std::string& getName();
   static const std::string& getPropertiesName();
   static const std::string& getDescription();
   static const std::string& getShortDescription();
   static const std::string& getCreator();
   static const std::string& getCopyright();
   static const std::string& getVersion();
   static const std::string& getDescriptorId();
   static bool isProduction();

private:
   void initializeArgTree(const PlugInArgList* pArgList, bool bInputArgs, QTreeWidgetItem* pParentItem);

   // PlugIn
   QLabel* mpNameLabel;
   QLabel* mpVersionLabel;
   QLabel* mpProductionStatusLabel;
   QLabel* mpCreatorLabel;
   QLabel* mpCopyrightLabel;
   QLabel* mpDescriptionLabel;
   QLabel* mpShortDescriptionLabel;
   QLabel* mpTypeLabel;
   QLabel* mpSubtypeLabel;
   QLabel* mpMultipleInstancesLabel;
   QLabel* mpRunningInstancesLabel;
   LabeledSection* mpPlugInSection;

   // Executable
   QLabel* mpStartupLabel;
   QLabel* mpDestroyedLabel;
   QLabel* mpMenuLocationLabel;
   QLabel* mpMenuIconLabel;
   QLabel* mpAbortLabel;
   QLabel* mpWizardLabel;
   QLabel* mpBatchModeLabel;
   QLabel* mpInteractiveModeLabel;
   QTreeWidget* mpArgsTree;
   LabeledSection* mpExecutableSection;

   // Importer
   QTextEdit* mpImportExtensionsEdit;
   LabeledSection* mpImporterSection;

   // Exporter
   QTextEdit* mpExportExtensionsEdit;
   LabeledSection* mpExporterSection;

   // Interpreter
   QTextEdit* mpInterpreterExtensionsEdit;
   LabeledSection* mpInterpreterSection;

   // Testable
   QLabel* mpTestableLabel;
   LabeledSection* mpTestableSection;
};

#endif
