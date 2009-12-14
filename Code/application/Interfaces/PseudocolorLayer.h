/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PSEUDOCOLORLAYER_H
#define PSEUDOCOLORLAYER_H

#include "Layer.h"
#include "ConfigurationSettings.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class ColorType;

/**
 *  Adjusts the properties of a pseudocolor layer.
 *
 *  A pseudocolor layer consists of sets of markers identifying pixels in a scene.  Each
 *  set of markers is called a class, and each class has four properties: name, inclusion
 *  value, color, and a display flag.  The inclusion value defines which pixels are
 *  included in the class.  The value corresponds with the data value of the underlying
 *  spectral element.  Element values in float or double values are truncated for the
 *  inclusion value.  This class manages the pseudocolor classes and their properties,
 *  where each class is identified by a unique ID value.  The pixel marker is drawn with
 *  a symbol based on the layer properties.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: addClass(), removeClass(), clear().
 *  - An action on the PseudocolorLayer is undone.
 *  - Everything else documented in Layer.
 *
 *  @see     Layer
 */
class PseudocolorLayer : public Layer
{
public:
   SETTING(MarkerSymbol, PseudocolorLayer, SymbolType, SOLID)

   /**
    *  Emitted with any<SymbolType> when the symbol style is changed.
    */
   SIGNAL_METHOD(PseudocolorLayer, SymbolChanged)
   /**
    *  Emitted when the list of classes is cleared.
    */
   SIGNAL_METHOD(PseudocolorLayer, Cleared)

   /**
    *  Adds a new class to the current pseudocolor layer.
    *
    *  @return  The unique class ID on success. Otherwise, -1.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     addInitializedClass(), removeClass()
    */
   virtual int addClass() = 0;

   /**
    *  Adds a new class to the current pseudocolor layer.
    *
    *  @param   className
    *           The new class name.
    *  @param   iValue
    *           The new class value.
    *  @param   classColor
    *           The new class RGB color.
    *  @param   bDisplayed
    *           TRUE if the new class should initially be displayed, otherwise FALSE.
    *
    *  @return  The unique class ID on success. Otherwise, -1.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     addClass()
    */
   virtual int addInitializedClass(const std::string& className, int iValue, const ColorType& classColor,
      bool bDisplayed = true) = 0;

   /**
    *  Retrieves the unique IDs for all classes.
    *
    *  @param   classIds
    *           A reference to a vector in which to put the class IDs.
    *           This vector should be allocated by the studio. A plug-in can do
    *           this via the ObjectFactory.
    */
   virtual void getClassIDs(std::vector<int>& classIds) const = 0;

   /**
    *  Returns the number of classes in the pseudocolor layer.
    *
    *  @return  The number of classes.
    *
    *  @see     getClassIDs()
    */
   virtual unsigned int getClassCount() const = 0;

   /**
    *  Removes a class from the pseudocolor layer.
    *
    *  @param   iID
    *           The unique ID of the class to remove.
    *
    *  @return  TRUE if the class was successfully removed.  FALSE if no class exists
    *           with the given ID or if an error occurred.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     clear()
    */
   virtual bool removeClass(int iID) = 0;

   /**
    *  Removes all classes from the current pseudocolor layer.
    *
    *  @notify  This method will notify signalCleared.
    *
    *  @see     removeClass()
    */
   virtual void clear() = 0;

   /**
    *  Sets all properties for an existing class on the current pseudocolor layer.
    *
    *  @param   iID
    *           The unique ID of the class.
    *  @param   className
    *           The new class name.
    *  @param   iValue
    *           The new class value.
    *  @param   classColor
    *           The new class RGB color.
    *  @param   bDisplayed
    *           TRUE if the new class should be displayed, otherwise FALSE.
    *
    *  @return  TRUE if the properties were set successfully, otherwise FALSE.
    */
   virtual bool setClassProperties(int iID, const std::string& className, int iValue, const ColorType& classColor,
      bool bDisplayed) = 0;

   /**
    *  Sets the name of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *  @param   className
    *           The new name.
    *
    *  @return  TRUE if the class name was successfully set.  FALSE if no class exists
    *           with the given ID or if an error occurred.
    *
    *  @see     getClassName()
    */
   virtual bool setClassName(int iID, const std::string& className) = 0;

   /**
    *  Retrieves the name of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *  @param   className
    *           A string that is populated with the class name
    *
    *  @return  True if the class name was successfully populated, otherwise false.
    *
    *  @see     setClassName()
    */
   virtual bool getClassName(int iID, std::string& className) const = 0;

   /**
    *  Sets the value of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *  @param   iValue
    *           The new value.
    *
    *  @return  TRUE if the class value was successfully set.  FALSE if no class exists
    *           with the given ID or if an error occurred.
    *
    *  @see     getClassValue()
    */
   virtual bool setClassValue(int iID, int iValue) = 0;

   /**
    *  Retrieves the value of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *
    *  @return  The class value.  A value of -1 is returned if no class exists with the
    *           given ID or if an error occurred.
    *
    *  @see     setClassValue()
    */
   virtual int getClassValue(int iID) const = 0;

   /**
    *  Sets the color of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *  @param   classColor
    *           The new RGB color for the class.
    *
    *  @return  TRUE if the class color was successfully set.  FALSE if no class exists
    *           with the given ID or if an error occurred.
    *
    *  @see     getClassColor()
    */
   virtual bool setClassColor(int iID, const ColorType& classColor) = 0;

   /**
    *  Retrieves the color of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *
    *  @return  The class color.
    *
    *  @see     setClassColor()
    */
   virtual ColorType getClassColor(int iID) const = 0;

   /**
    *  Sets the display state of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *  @param   bDisplayed
    *           TRUE if the class should be displayed.  FALSE if the class should not
    *           be displayed.
    *
    *  @return  TRUE if the class display state was successfully set.  FALSE if no
    *           class exists with the given ID or if an error occurred.
    *
    *  @see     isClassDisplayed()
    */
   virtual bool setClassDisplayed(int iID, bool bDisplayed) = 0;

   /**
    *  Retrieves the display state of a pseudocolor class.
    *
    *  @param   iID
    *           The unique ID of the class.
    *
    *  @return  TRUE if the class if displayed.  FALSE if the class is not displayed
    *           or if no class exists with the given ID
    *
    *  @see     setClassDisplayed()
    */
  virtual bool isClassDisplayed(int iID) const = 0;

  virtual SymbolType getSymbol() const = 0;

  virtual void setSymbol(SymbolType symbol) = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~PseudocolorLayer() {}
};

#endif
