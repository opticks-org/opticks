/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORTER_H
#define EXPORTER_H

#include "TypesFile.h"
#include <string>

class DataDescriptor;
class PlugInArgList;
class QWidget;

/**
 *  Interface specific to exporter plug-ins.
 *
 *  Defines the exporter specific interface to all exporter plug-ins.
 *  This interface contains all exporter specific operations.
 */
class Exporter
{
public:
   /**
    *  The name for an export item argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  SessionItem pointer when the plug-in is executed with an ExportAgent.
    *  Arguments with this name should be of the type SessionItem or one of its
    *  subclasses.
    *
    *  @see     ExportAgent::execute()
    */
   static std::string ExportItemArg() { return "Export Item"; }

   /**
    *  The name for an export file descriptor argument.
    *
    *  Input arguments with this name will be automatically populated with the
    *  FileDescriptor to use for export when the plug-in is executed with an
    *  ExportAgent.  Arguments with this name should be of the type
    *  FileDescriptor or one of its subclasses.
    *
    *  @see     ExportAgent::execute()
    */
   static std::string ExportDescriptorArg() { return "Export Descriptor"; }

   /**
    *  Returns the default file extensions recognized by the exporter.
    *
    *  @return  The file extensions recognized by the exporter as a string.
    *           The string consists of a description followed by one or more
    *           file extensions separated by a space.  Multiple file
    *           types may be specified with a double semicolon.  Examples
    *           include "Landsat Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   virtual std::string getDefaultExtensions() const = 0;

   /**
    *  Queries whether a given argument list can be successfully saved by
    *  the exporter.
    *
    *  This method is called for the exporter to parse the current settings in
    *  the plug-in argument list to see if it supports saving the data as currently
    *  specified.  This allows exporters that do not support certain combinations 
    *  of values and options to indicate as such.  This method is called each time
    *  the user changes a value in the export options dialog.
    *
    *  @param   pArgList
    *           The plug-in argument list as requested in getInputSpecification.
    *           This is populated as it would be in a call to execute.
    *  @param   errorMessage
    *           An error message that is populated with the reason why the
    *           exporter cannot save the given data descriptor. This message
    *           will be displayed to the user. When returning
    *           ValidationResultType::VALIDATE_INFO, a non-empty \em errorMessage
    *           is required.
    *
    *  @return  Returns a result based on the level of validation success.
    *           Either complete success, complete failure, a success with a caveat
    *           which will be reported to the user, or a potential success with
    *           additional input from the user.
    *
    *  @see     ValidationResultType
    */
   virtual ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const = 0;

   /**
    *  Returns a widget to display custom export option values.
    *
    *  This method provides an interface for which specialized export options
    *  can be displayed to the user.  The method returns a Qt widget that is
    *  added to the default export options dialog.  The exporter should create
    *  the widget with a \b NULL parent, and should destroy the widget when the
    *  exporter itself is destroyed.
    *
    *  Exporters should call QWidget::setWindowTitle() on the widget that is
    *  returned to set the name that appears on the tab in the export options
    *  dialog.  If the window title is not set, the exporter name is displayed.
    *  If the returned widget is the only widget to display, a tab widget is
    *  not used in the dialog.
    *
    *  @param   pInArgList
    *           The input argument list specified by
    *           Executable::getInputSpecification() and populated as if being
    *           passed to Executable::execute().
    *
    *  @return  A QWidget that will be displayed as an additional tab in the
    *           default export options dialog.  \b NULL should be returned if
    *           the exporter does not have custom options to display to the
    *           user.
    */
   virtual QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList)= 0;

protected:
   /**
    *  Since the Exporter interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Exporter() {}
};

#endif
