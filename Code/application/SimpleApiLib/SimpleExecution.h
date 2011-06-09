/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLEEXECUTION__
#define SIMPLEEXECUTION__

#include "AppConfig.h"

class ExecutableAgent;
class PlugInArgList;
class WizardItem;
class WizardNode;
class WizardObject;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
   * @file SimpleExecution.h
   * This file contains functions and type definitions for executing plugins and wizards.
   */

   /**
    * Create a new PlugIn handle.
    *
    * @param pName
    *        The \c NULL terminated name of the plug-in to create.
    * @param batch
    *        Create an interactive plug-in if this is 0 otherwise create a batch plug-in.
    * @return A new PlugIn or \c NULL if an error occured.
    *         Must be freed with freePlugIn() if ownership is not explicitly transfered.
    * @see ExecutableResource
    */
   EXPORT_SYMBOL ExecutableAgent* createPlugIn(const char* pName, int batch);

   /**
    * Free a PlugIn handle created with createPlugIn().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pPlugin
    *        The PlugIn to free. No error checking is performed on this value
    *        and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void freePlugIn(ExecutableAgent* pPlugin);

   /**
    * Access the input argument list for a PlugIn.
    *
    * A SIMPLE_OTHER_FAILURE indicates std::logic_error was caught.
    *
    * @param pPlugin
    *        The PlugIn whose argument list will be returned.
    * @return an input argument list or \c NULL if an error occurs.
    * @see ExecutableAgent::getInArgList()
    */
   EXPORT_SYMBOL PlugInArgList* getPlugInInputArgList(ExecutableAgent* pPlugin);

   /**
    * Access the output argument list for a PlugIn.
    *
    * A SIMPLE_OTHER_FAILURE indicates std::logic_error was caught.
    *
    * @param pPlugin
    *        The PlugIn whose argument list will be returned.
    * @return an output argument list or \c NULL if an error occurs.
    * @see ExecutableAgent::getOutArgList()
    */
   EXPORT_SYMBOL PlugInArgList* getPlugInOutputArgList(ExecutableAgent* pPlugin);

   /**
    * Execute a PlugIn.
    *
    * A SIMPLE_OTHER_FAILURE indicates std::logic_error was caught.
    *
    * @param pPlugin
    *        The PlugIn to execute.
    * @return 0 if execution failed, non-zero if execution succeeded.
    *         The only simple API error set is SIMPLE_OTHER_FAILURE if
    *         std::logic_error was caught. All other 0 returns indicate an
    *         error in the PlugIn execution.
    * @see ExecutableAgent::execute()
    */
   EXPORT_SYMBOL int executePlugIn(ExecutableAgent* pPlugin);

   /**
    * Load a wizard from a file.
    *
    * @param pFilename
    *        The \c NULL terminated filename of the wizard to load.
    *        An attempt will be made to determine if this is an absolute
    *        or relative path by first opening pFilename directly. If this
    *        fails, the wizard path setting will be prepended to the filename
    *        and this will be opened.
    * @return A new WizardObject or \c NULL if an error occured.
    *         Must be freed with freeWizard().
    */
   EXPORT_SYMBOL WizardObject* loadWizard(const char* pFilename);

   /**
    * Free a WizardObject created with loadWizard().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pWizard
    *        The wizard to free. No error checking is performed on this value
    *        and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void freeWizard(WizardObject* pWizard);
   
   /**
    * Get the name of a wizard.
    *
    * @param pWizard
    *        The WizardObject to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the Buffer. If the name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getWizardName(WizardObject* pWizard, char* pName, uint32_t nameSize);

   /**
    * Get the number of input nodes in a wizard.
    *
    * Input nodes correspond to value items in the wizard.
    *
    * @param pWizard
    *        The WizardObject to query.
    * @return the number of input nodes. 0 may indicate an error.
    */
   EXPORT_SYMBOL uint32_t getWizardInputNodeCount(WizardObject* pWizard);

   /**
    * Get the number of output nodes in a wizard.
    *
    * Output nodes for value items are not counted.
    *
    * @param pWizard
    *        The WizardObject to query.
    * @return the number of output nodes. 0 may indicate an error.
    */
   EXPORT_SYMBOL uint32_t getWizardOutputNodeCount(WizardObject* pWizard);

   /**
    * Get an input node.
    *
    * Input nodes correspond to value items in the wizard.
    *
    * @param pWizard
    *        The WizardObject to query.
    * @param idx
    *        The 0 based index of the input node. Must be less than the return value
    *        of getWizardInputNodeCount().
    * @return A WizardNode handle.
    */
   EXPORT_SYMBOL WizardNode* getWizardInputNodeByIndex(WizardObject* pWizard, uint32_t idx);

   /**
    * Get an input node with a given name.
    *
    * Input nodes correspond to value items in the wizard. The name is the name
    * associated with the value item.
    *
    * @param pWizard
    *        The WizardObject to query.
    * @param pName
    *        The \c NULL terminated name of the input node.
    * @return A WizardNode handle.
    */
   EXPORT_SYMBOL WizardNode* getWizardInputNodeByName(WizardObject* pWizard, const char* pName);

   /**
    * Get an output node.
    *
    * Output nodes for value items are not counted.
    *
    * @param pWizard
    *        The WizardObject to query.
    * @param idx
    *        The 0 based index of the output node. Must be less than the return value
    *        of getWizardOutputNodeCount().
    * @return A WizardNode handle.
    */
   EXPORT_SYMBOL WizardNode* getWizardOutputNodeByIndex(WizardObject* pWizard, uint32_t idx);

   /**
    * Get an output node with a given name.
    *
    * Output nodes for value items are not counted. The name is the name
    * of the wizard item followed by a | and the name of the output node.
    * Wizard item names which contain a | can't be accessed with the method, use
    * getWizardOutputNodeByIndex() instead. If there are multiple items with the
    * same name, the first one (ordered by execution order) will be selected. Use
    * getWizardOutputNodeByIndex() to access the other items.
    * For example:
    *    Principal Component Analysis|Corrected Data Cube
    *
    * @param pWizard
    *        The WizardObject to query.
    * @param pName
    *        The \c NULL terminated name of the output node.
    * @return A WizardNode handle.
    */
   EXPORT_SYMBOL WizardNode* getWizardOutputNodeByName(WizardObject* pWizard, const char* pName);

   /**
    * Execute a wizard.
    *
    * @param pWizard
    *        The wizard to execute.
    * @return 0 if execution failed, non-zero if execution succeeded.
    *         The only simple API error set is SIMPLE_BAD_PARAMS.
    *         All other 0 returns indicate an
    *         error in the wizard execution.
    */
   EXPORT_SYMBOL int executeWizard(WizardObject* pWizard);
   
   /**
    * Get the name of a wizard node.
    *
    * @param pNode
    *        The WizardNode to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the Buffer. If the name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getWizardNodeName(WizardNode* pNode, char* pName, uint32_t nameSize);
   
   /**
    * Get the type string of a wizard node.
    *
    * @param pNode
    *        The WizardNode to query.
    * @param pType
    *        Buffer to store the type. This will be \c NULL terminated.
    * @param typeSize
    *        The size of the Buffer. If the type is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the type.
    */
   EXPORT_SYMBOL uint32_t getWizardNodeType(WizardNode* pNode, char* pType, uint32_t typeSize);

   /**
    * Get the value of a wizard node.
    *
    * @param pNode
    *        The WizardNode to query.
    * @return A void* to the value. The type of this data is determined by getWizardNodeType().
    *         This value should not be directly modified. If you want to change the value, call
    *         setWizardNodeValue().
    */
   EXPORT_SYMBOL void* getWizardNodeValue(WizardNode* pNode);

   /**
    * Set the value of a wizard node.
    *
    * @param pNode
    *        The WizardNode to query.
    * @param pValue
    *        A void* to the value. The type of this data is determined by getWizardNodeType().
    * @return A 0 if a failure occurs, a non-zero otherwise.
    */
   EXPORT_SYMBOL int setWizardNodeValue(WizardNode* pNode, void* pValue);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif