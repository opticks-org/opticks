/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORTERSHELL_H
#define EXPORTERSHELL_H

#include "Exporter.h"
#include "ExecutableShell.h"

#include <string>
#include <vector>

class PlugInArgList;

/**
 *  \ingroup ShellModule
 *  %Exporter Shell
 *
 *  This class represents the shell for an exporter plug-in.  %Exporter
 *  developers would take this class and extend it to support their
 *  exporter specific code.
 *
 *  @see     ExecutableShell, Exporter
 */
class ExporterShell : public ExecutableShell, public Exporter
{
public:
   /**
    *  Creates an exporter plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::ExporterType().
    *
    *  @see     getType()
    */
   ExporterShell();

   /**
    *  Destroys the exporter plug-in.
    */
   ~ExporterShell();

   /**
    *  @copydoc Executable::getInputSpecification()
    *
    *  @default The default implementation adds the following args:
    *  <table><tr><th>Name</th>           <th>Type</th></tr>
    *         <tr><td>File Descriptor</td><td>FileDescriptor</td></tr>
    *         <tr><td>Progress</td>       <td>Progress</td></tr></table>
    */
   bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default The default implementation does not set any args in the arg
    *           list and returns \b true.
    */
   bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Exporter::getDefaultExtensions()
    *
    *  @default The default implementation returns the extension string that
    *           was passed into setExtensions().  If setExtensions() has not
    *           yet been called, an empty string is returned.
    */
   std::string getDefaultExtensions() const;

   /**
    *  @copydoc Exporter::validate()
    *
    *  @default The default implementation checks for the existence of a valid
    *           FileDescriptor argument in the arg list.
    */
   ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const;

   /**
    *  @copydoc Exporter::getExportOptionsWidget()
    *
    *  @default The default implementation returns \b NULL.
    */
   QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);

protected:
   /**
    *  Sets the default file extensions recognized by the exporter.
    *
    *  @param   extensions
    *           The file extensions recognized by the exporter.  The string
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
