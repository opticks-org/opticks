/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTERSHELL_H
#define IMPORTERSHELL_H

#include "ExecutableShell.h"
#include "Importer.h"

#include <string>

/**
 *  %Importer Shell
 *
 *  This class represents the shell for an importer plug-in.  %Importer
 *  developers would take this class and extend it to support thier 
 *  importer specific code.
 *
 *  @see     ExecutableShell, Importer
 */
class ImporterShell : public ExecutableShell, public Importer
{
public:
   /**
    *  Creates an importer plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::ImporterType().
    *
    *  @see     getType()
    */
   ImporterShell();

   /**
    *  Destroys the importer plug-in.
    */
   ~ImporterShell();

   /**
    *  @copydoc Importer::getDefaultExtensions()
    *
    *  @default The default implementation returns the extension string that
    *           was passed into setExtensions().  If setExtensions() has not
    *           yet been called, an empty string is returned.
    */
   std::string getDefaultExtensions() const;

   /**
    *  @copydoc Importer::isProcessingLocationSupported()
    *
    *  @default Only a processing location of ProcessingLocation::IN_MEMORY is
    *           supported.  If \em location is ProcessingLocation::IN_MEMORY,
    *           the default implementation returns \b true, otherwise \b false
    *           is returned.
    */
   bool isProcessingLocationSupported(ProcessingLocation location) const;

   /**
    *  @copydoc Importer::getPreview()
    *
    *  @default The default implementation of this method does not create a
    *           preview and returns \b NULL.
    */
   QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);

   /**
    *  @copydoc Importer::validate()
    *
    *  @default The default implementation checks the following criteria in the
    *           specified order:
    *           - Non-NULL data descriptor
    *           - Non-NULL file descriptor
    *           - Valid filename
    *           - Existing file
    *           - A DataElement based on the given data descriptor using
    *             ModelServices::getElement() does not already exist
    *
    *  @warning This method should not be called from within execute() because
    *           the check for a non-existing data element should fail since the
    *           data element has already been created.
    */
   bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

   /**
    *  @copydoc Importer::getImportOptionsWidget()
    *
    *  @default The default implementation returns \b NULL.
    */
   QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);

   /**
    *  @copydoc Importer::polishDataDescriptor()
    *
    *  @default The default implementation does nothing.
    */
   void polishDataDescriptor(DataDescriptor* pDescriptor);

protected:
   /**
    *  Sets the default file extensions recognized by the importer.
    *
    *  @param   extensions
    *           The file extensions recognized by the importer.  The string
    *           should consist of a description followed by one or more
    *           extensions separated by a space.  Multiple file types may
    *           be specified with a double semicolon.  Examples include
    *           "Landsat Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   void setExtensions(const std::string& extensions);

private:
   std::string mExtension;
};

#endif
