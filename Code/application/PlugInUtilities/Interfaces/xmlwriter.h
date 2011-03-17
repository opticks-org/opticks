/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "StringUtilities.h"
#include "XercesIncludes.h"
#include "xmlbase.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

class Font;

/** @file xmlwriter.h
 *  @brief XML utilities and functionality for writing/generating a DOM
 */

/**
 * This class generates a DOM tree and associated XML.
 * Many of the functions in this class use the \b current DOM node.
 * The \b current DOM node is determined as follows:
 * <ul>
 *  <li> If the function takes a \e pOwner argument and one is specified, this is the \b current DOM node.
 *  <li> If the \b addpoint stack is not empty, the top of the stack is the \b current DOM node.
 *  <li> Finally, the root element of the DOM tree is used as the \b current DOM node.
 * </ul>
 * This allows a caller to extend the depth of the DOM tree by pushing and popping DOM elements
 * to the \b addpoint stack. The following example code generates this DOM tree.
 *
 * @code
 *   <topLevel>
 *     <child a="42" b="This is an attribute">
 *       <grandchild>
 *          Here is some text.
 *       </grandchild>
 *     </child>
 *     <anotherchild val="1 2 3 4"/>
 *   </topLevel>
 * @endcode
 *
 * @code
 *   XMLWriter xml("topLevel");
 *   xml.pushAddPoint(xml.addElement("child"));
 *   xml.addAttr("a", 42);
 *   xml.addAttr("b", "This is an attribute");
 *   xml.pushAddPoint(xml.addElement("grandchild"));
 *   xml.addText("Here is some text.");
 *   xml.popAddPoint();
 *   xml.popAddPoint();
 *   vector<int> intVect;
 *   intVect.push_back(1);
 *   intVect.push_back(2);
 *   intVect.push_back(3);
 *   intVect.push_back(4);
 *   xml.pushAddPoint(xml.addElement("anotherchild"));
 *   xml.addAttr("val", intVect);
 *   string xmlString = xml.writeToString();
 * @endcode
 *
 * @ingroup app_xml
 *
 * @par requirements
 * Apache Xerces-C++ version 3.1.1
 */
class XMLWriter : public XmlBase
{
public:
   //@{
   /**
    * Create an %XMLWriter.
    *
    * @param pRootElementName
    *        The name of the top level element
    *
    * @param pLog
    *        Optional MessageLog to be passed to XmlBase
    *
    * @param useNamespace
    *        If \c true, the app namespace will be exported, if \c false, no namespace will be exported.
    *
    * @throw XmlBase::XmlException
    *        When unable to create the Xerces DOM document.
    */
   XMLWriter(const char* pRootElementName, MessageLog* pLog = NULL, bool useNamespace = true);

   /**
    * Create an %XMLWriter.
    *
    * @param rootElementName
    *        The name of the top level element
    *
    * @param xmlNamespace
    *        Use this namespace instead of the default one.
    *
    * @param pLog
    *        Optional MessageLog to be passed to XmlBase
    *
    * @param useNamespace
    *        If \c true, the app namespace will be exported, if \c false, no namespace will be exported.
    *
    * @throw XmlBase::XmlException
    *        When unable to create the Xerces DOM document.
    */
   XMLWriter(const std::string& rootElementName, const std::string& xmlNamespace, MessageLog* pLog = NULL,
      bool useNamespace = true);

   /**
    * Destroy and cleanup the %XMLWriter object.
    */
   ~XMLWriter();
   //@}

   //@{
   /**
    * Add an attribute to the current DOM node
    * This templated class converts \e value to
    * a string using a std::stringstream.
    *
    * @param pName
    *        The name of the attribute.
    *
    * @param value
    *        The value of the attribute. \b T
    *        must have \e operator<< defined for
    *        a std::stringstream.
    *
    * @return Returns \c true on success and \c false on failure.
    *
    * @see std::stringstream
    */
   template <class T>
   bool addAttr(const char* pName, T value)
   {
      return addAttr(pName, static_cast<std::string>(StringUtilities::toXmlString(value)), NULL);
   }

   /**
    * Add a vector attribute to the current DOM node
    * This templated class converts \e value to
    * an XML list of strings using a std::stringstream.
    *
    * @param pName
    *        The name of the attribute.
    *
    * @param value
    *        The value of the attribute. \b T
    *        must have \e operator<< defined for
    *        a std::stringstream.
    *
    * @return Returns \c true on success and \c false on failure.
    *
    * @see std::stringstream
    */
   template<class T>
   bool addAttr(const char* pName, std::vector<T> value)
   {
      std::stringstream buf;
      std::copy(value.begin(), value.end(), std::ostream_iterator<T>(buf, " "));
      return addAttr(pName, buf.str(), NULL);
   }

   /**
    * Add a string attribute to the specified DOM node.
    *
    * @param pName
    *        The name of the attribute.
    *
    * @param pValue
    *        The value of the attribute.
    *
    * @param pOwner
    *        The DOM node which will have the attribute.
    *
    * @return Returns \c true on success and \c false on failure.
    */
   bool addAttr(const char* pName, const char* pValue, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pOwner);

   /**
    * Add a string attribute to the specified DOM node.
    *
    * @param pName
    *        The name of the attribute.
    *
    * @param value
    *        The value of the attribute.
    *
    * @param pOwner
    *        The DOM node which will have the attribute.
    *
    * @return Returns \c true on success and \c false on failure.
    */
   bool addAttr(const char* pName, const std::string value, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pOwner)
   {
      return addAttr(pName, value.c_str(), pOwner);
   }
   //@}

   //@{
   /**
    * Add an element DOM node.
    *
    * @param pName
    *        The name of the element.
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @return The new DOM element or \c NULL on failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* addElement(const char* pName,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL);
   //@}

   //@{
   /**
    * Add an element DOM node.
    *
    * @param name
    *        The name of the element.
    *
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @return The new DOM element or \c NULL on failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* addElement(const std::string& name,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL)
   {
      return addElement(name.c_str(), pOwner);
   }
   //@}

   //@{
   /**
    * Add an element DOM node.
    *
    * @param pName
    *        The name of the element.
    *
    * @param font
    *        The font specification to be written in the XMLWriter.
    *
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @return The new DOM element or \c NULL on failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* addFontElement(const char* pName, const Font& font,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL);
   //@}

   //@{
   /**
    * Add a value as a text DOM node.
    * This templated class converts \e value to
    * a string using a std::stringstream.
    *
    * @param value
    *        The value of the text node. \b T must
    *        have \e operator<< defined on std::stringstream.
    *
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @see std::stringstream
    *
    * @return The new DOM text node or \c NULL on failure.
    */
   template<class T>
   XERCES_CPP_NAMESPACE_QUALIFIER DOMText* addText(T value, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL)
   {
      std::stringstream buf;
      buf << value;
      return addText(buf.str(), pOwner);
   }

   /**
    * Add a vector of values as a text DOM node.
    * This templated class converts \e value to
    * an XML list of strings using a std::stringstream.
    *
    * @param value
    *        The value of the text node. \b T must
    *        have \e operator<< defined on std::stringstream.
    *
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @return The new DOM text node or \c NULL on failure.
    *
    * @see std::stringstream
    */
   template<class T>
   XERCES_CPP_NAMESPACE_QUALIFIER DOMText* addText(std::vector<T> value,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL)
   {
      std::stringstream buf;
      std::copy(value.begin(), value.end(), std::ostream_iterator<T>(buf, " "));
      return addText(buf.str(), pOwner);
   }

   /**
    * Add a string value as a text DOM node.
    *
    * @param pValue
    *        The value of the text node.
    *
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @return The new DOM text node or \c NULL on failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMText* addText(const char* pValue,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL);

   /**
    * Add a string value as a text DOM node.
    *
    * @param value
    *        The value of the text node.
    *
    * @param pOwner
    *        The optional DOM node which will be the parent
    *        of this element. If not specified, the current
    *        DOM node is used as the parent.
    *
    * @return The new DOM text node or \c NULL on failure.
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMText* addText(const std::string& value,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL)
   {
      return addText(value.c_str(),pOwner);
   }
   //@}

   //@{
   /**
    * Determine if a named element exists.
    * This will check descendents of \e pOwner, if
    * specified or descendents of the current DOM node.
    *
    * @param pName
    *        The name of the element to find.
    *
    * @param pOwner
    *        The optional DOM node which will be searched.
    *        If not specified, the current DOM node is used
    *        as the parent.
    *
    * @return Returns \c true if the element exists and \c false if it does not exist.
    */
   bool elementExists(const char* pName, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL);

   /**
    * Remove a named element from a DOM tree.
    * This will remove an element if it is a child of
    * \e pOwner, if specified or if it is a child of
    * the current DOM node otherwise.
    *
    * @param pName
    *        The name of the element to remove.
    *
    * @param pOwner
    *        The optional DOM node which will be searched.
    *        If not specified, the current DOM node is used
    *        as the parent.
    */
   void removeElement(const char* pName, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL);

   /**
    * Remove a specified node from a DOM tree.
    * This will remove a node if it is a child of
    * \e pOwner, if specified or if it is a child of
    * the current DOM node otherwise.
    *
    * @param pChild
    *        The DOM node to remove.
    *
    * @param pOwner
    *        The optional DOM node which will be searched.
    *        If not specified, the current DOM node is used
    *        as the parent.
    */
   void removeChild(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pChild,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner = NULL);
   //@}

   //@{
   /**
    * This flag determines if only a single child DOM node
    * with a name can be added. If set, child DOM nodes with
    * the name of a newly added DOM node are removed. If clear,
    * multiple DOM nodes with the same name may be added.
    *
    * @param v
    *        The new value of the flag.
    *
    * @return The old value of the flag.
    */
   bool setSingleChildInstance(bool v)
   {
      bool r(mSingleChildInstance);
      mSingleChildInstance = v;
      return r;
   }
   //@}

   //@{
   /**
    * Write a DOM tree to a file as XML.
    *
    * @param pFp
    *        File pointer that the DOM tree will be written to.
    *
    * @throw XmlBase::XmlException
    *        When Xerces is not able to generate the XML.
    */
   void writeToFile(FILE* pFp);

   /**
    * Write a DOM tree to a string as XML.
    *
    * @return The DOM tree as a string of XML.
    *
    * @throw XmlBase::XmlException
    *        When Xerces is not able to generate the XML;
    *        or when the \e char type is not 1 byte in size.
    *        (this should not occur on any modern computer)
    */
   std::string writeToString();
   //@}

   //@{
   /**
    * Push a new current DOM node to the add point stack.
    * See the class documentation for XMLWriter for further
    * information on add points.
    *
    * @param pNode
    *        The new current DOM node. If this is \c NULL, the document
    *        root is pushed onto the stack.
    *
    * @see XMLWriter
    */
   void pushAddPoint(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode = NULL);

   /**
    * Pop the current DOM node from the add point stack.
    * See the class documentation for XMLWriter for further
    * information on add points.
    *
    * @return The DOM node which was just popped from the stack.
    *
    * @see XMLWriter
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* popAddPoint();

   /**
    * Find out what DOM node is on top of the add point stack without removing the node.
    * See the class documentation for XMLWriter for further
    * information on add points.
    *
    * @return The DOM node which is on the top of the stack.
    *
    * @see XMLWriter
    */
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* peekAddPoint();
   //@}

private:
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* mpDoc;
   std::stack<XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*> mpAddPoint;
   XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* mpImpl;
   bool mSingleChildInstance;
   XMLCh* mpWithNamespace;
};

/** Explicit template specializations can not go in the class declaration
    as this is not supported in Studio 11.
 **/

template<> bool XMLWriter::addAttr<std::string>(const char* pName, std::string value);
template<> bool XMLWriter::addAttr<char*>(const char* pName, char* pValue);
template<> bool XMLWriter::addAttr<const char*>(const char* pName, const char* pValue);

/**
 *  This class is intended to be used by the XML_ADD_POINT macro and should not
 *  be used directly.
 */
class XmlAddPoint
{
public:
   XmlAddPoint(XMLWriter& writer, const std::string& name) :
      mWriter(writer),
      mFirstTime(true)
   {
      writer.pushAddPoint(writer.addElement(name.c_str()));
   }
   ~XmlAddPoint()
   {
      mWriter.popAddPoint();
   }
   bool isFirstTime()
   {
      bool firstTime = mFirstTime;
      mFirstTime = false;
      return firstTime;
   }
private:
   XMLWriter& mWriter;
   bool mFirstTime;
};

/**
 *  Adds an add-point to the XML and to the C++ stack.
 * 
 *  This macro causes an add-point to be inserted into the writer, and 
 *  simultaneously added to an RAII object on the C++ stack. This causes the
 *  indentation of the C++ writing code to match the indentation of the 
 *  resulting XML. This in turn makes it easier to visualize the structure of
 *  the resulting XML and makes it impossible mismatch the pushing and popping
 *  of add points.
 *
 *  @code
 *  XML_ADD_POINT (writer, my_add_point) // pushes an add point on the writer
 *  {
 *     writer.addAttribute("my_attr", myValue);
 *  } // pops the add point from the writer
 *  @endcode
 * 
 *  @param writer
 *             The XMLWriter to push the new element / add-point on to.
 *
 *  @param name
 *             The name for the new element.
 */
#define XML_ADD_POINT(writer, name) \
   for (XmlAddPoint name(writer, #name); name.isFirstTime(); )

/**
 *  This method is intended to be called from the XML_ADD_CONTAINER macro and 
 *  should not be called directly.
 */
template <class Iter>
void writeContainerElements(XMLWriter& writer, Iter pStartIter, Iter pStopIter)
{
   for (Iter pIter = pStartIter; pIter != pStopIter; ++pIter)
   {
      XML_ADD_POINT (writer, element)
      {
         writer.addAttr("value", *pIter);
      }
   }
}

/**
 *  Writes a container of data to the specified XMLWriter
 * 
 *  @code
 *  XML_ADD_POINT (writer, my_data)
 *  {
 *     vector<int> v(12, 65); // a vector of 12 65's
 *     XML_ADD_CONTAINER(writer, value, v.begin(), v.end());
 *  }
 *  @endcode
 *  The above code will write out the vector of 12 values to the specified
 *  XMLWriter. The data can be read back in later with readContainerElements.
 *
 *  @param writer
 *             The XMLWriter to write the data to.
 *
 *  @param name
 *             The name to give each value.
 *
 *  @param startIter
 *             A container iterator specifying the first element to write.
 *
 *  @param stopIter
 *             A container iterator specifying where to stop writing.
 */
#define XML_ADD_CONTAINER(writer, name, startIter, stopIter) \
   XML_ADD_POINT (writer, name) \
   { \
      writeContainerElements(writer, startIter, stopIter); \
   }

#endif
