/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

/**
 *  A widget to provide information to the user.
 *
 *  The InfoBar widget is a gorizontal, rectangular widget that can give information
 *  to the user.  The widget has three major components: title, descrption, and
 *  identifying icon.
 *
 *  The title is a string displayed at the far left end of the widget.  The text color
 *  and font can be set using setTitleColor() and setTitleFont().  The title can also
 *  have an associated popup menu for the user to select between multiple titles.  Call
 *  the setMenu() method to set the desired menu.  Internally, the title label is replaced
 *  with a flat push button using the setTitleButton() method.
 *
 *  The description is a string displayed in the center of the widget.  Like the title,
 *  the text color and font can be set using setDescriptionColor() and setDescriptionFont().
 *
 *  The icon is displayed at the far right end of the widget.  It can be set with the
 *  setInfoIcon() method.
 *
 *  In addition to the title, description, and icon, the background color of the widget
 *  can be set with the setBackgroundColor() method.
 */
class InfoBar : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates the info bar.
    *
    *  @param    parent
    *            The parent widget.  This value may be NULL.
    */
   InfoBar(QWidget* parent = 0);

   /**
    *  Destroys the info bar.
    *
    *  The destructor destroys the info bar widget and any associated menu.
    */
   ~InfoBar();

   /**
    *  Sets the background color for the entire widget.
    *
    *  This method sets the background color for the entire widget, including the title,
    *  description, and icon mask.
    *
    *  @param    clrBackground
    *            The new background color.  Must be a valid QColor.
    */
   virtual void setBackgroundColor(const QColor& clrBackground);

   // Title

   /**
    *  Returns the current title text.
    *
    *  @return  The current title.  An empty string is returned if no title has been set.
    *
    *  @see     InfoBar::setTitle
    */
   QString getTitle() const;

   /**
    *  Returns the current title color.
    *
    *  @return  The current title text color.  A valid color is returned even if no
    *           title has been set.
    *
    *  @see     InfoBar::setTitleColor
    *  @see     InfoBar::setTitle
    */
   QColor getTitleColor() const;

   /**
    *  Returns the current title font.
    *
    *  @return  The current title text font.  A valid font is returned even if no
    *           title has been set.
    *
    *  @see     InfoBar::setTitleFont
    *  @see     InfoBar::setTitle
    */
   QFont getTitleFont() const;

   // Popup menu

   /**
    *  Sets the menu to use when selecting from multiple titles.
    *
    *  @param   pMenu
    *           A pointer to a menu containing multiple titles.  The info bar assumes
    *           ownership of the menu and will delete it when a new menu is passed in
    *           or when the info bar itself is deleted.  Passing in NULL disables
    *           multiple titles and only a single title may be used.
    *
    *  @see     getMenu()
    */
   void setMenu(QMenu* pMenu);

   /**
    *  Returns a pointer to the menu containing multiple titles.
    *
    *  @return  A pointer to the menu containing multiple titles.  NULL is returned
    *           if no menu has been set and only a single title is displayed.
    *
    *  @see     setMenu()
    */
   QMenu* getMenu() const;

   // Description

   /**
    *  Returns the current description text.
    *
    *  @return  The current description.  An empty string is returned if no description
    *           has been set.
    *
    *  @see     InfoBar::setDescription
    */
   QString getDescription() const;

   /**
    *  Returns the current description color.
    *
    *  @return  The current description text color.  A valid color is returned even if
    *           no description has been set.
    *
    *  @see     InfoBar::setDescriptionColor
    *  @see     InfoBar::setDescription
    */
   QColor getDescriptionColor() const;

   /**
    *  Returns the current description font.
    *
    *  @return  The current description text font.  A valid font is returned even if no
    *           description has been set.
    *
    *  @see     InfoBar::setDescriptionFont
    *  @see     InfoBar::setDescription
    */
   QFont getDescriptionFont() const;

   /**
    *  Returns the description text alignment.
    *
    *  @return  The alignment flags for the current description.
    *
    *  @see     InfoBar::setDescriptionAlignment
    */
   int getDescriptionAlignment() const;

   /**
    *  Sets the description text alignment.
    *
    *  @param   iAlignment
    *           The alignment flags.  The value is a bitwise OR of Qt::AlignmentFlags
    *           and Qt::TextFlags values.
    *
    *  @see     InfoBar::getDescriptionFont
    */
   void setDescriptionAlignment(int iAlignment);

   // Icon

   /**
    *  Sets the current info icon.
    *
    *  @param   pixInfo
    *           A pixmap contining the icon to display.
    *
    *  @see     InfoBar::getInfoIcon
    */
   void setInfoIcon(const QPixmap& pixInfo);

   /**
    *  Returns the current info icon.
    *
    *  @return  A pointer to the icon pixmap.  NULL is returned if no icon has been set.
    *
    *  @see     InfoBar::setInfoIcon
    */
   const QPixmap* getInfoIcon() const;

   /**
    *  Returns a suggested size for the widget.
    *
    *  @return  The suggested size of the widget, based on the text, font, and icon.
    */
   virtual QSize sizeHint() const;

public slots:
   /**
    *  Sets the title text.
    *
    *  @param   strTitle
    *           The new title text.
    *
    *  @see     InfoBar::getTitle
    */
   void setTitle(const QString& strTitle);

   /**
    *  Sets the title text color.
    *
    *  @param   clrTitle
    *           The new title text color.
    *
    *  @see     InfoBar::getTitleColor
    */
   void setTitleColor(QColor clrTitle);

   /**
    *  Sets the title text font.
    *
    *  @param   fntTitle
    *           The new title text font.
    *
    *  @see     InfoBar::getTitleFont
    */
   void setTitleFont(QFont fntTitle);

   /**
    *  Sets the description text.
    *
    *  @param   strDescription
    *           The new description text.
    *
    *  @see     InfoBar::getDescription
    */
   void setDescription(const QString& strDescription);

   /**
    *  Sets the description text color.
    *
    *  @param   clrDescription
    *           The new description text color.
    *
    *  @see     InfoBar::getDescriptionColor
    */
   void setDescriptionColor(QColor clrDescription);

   /**
    *  Sets the description text font.
    *
    *  @param   fntDescription
    *           The new description text font.
    *
    *  @see     InfoBar::getDescriptionFont
    */
   void setDescriptionFont(QFont fntDescription);

signals:
   /**
    *  Indicates a change in the title text.
    *
    *  This signal is emitted when the title text changes.
    *
    *  @param   strTitle
    *           The new title text.
    *
    *  @see     InfoBar::setTitle
    */
   void titleChanged(const QString& strTitle);

   /**
    *  Indicates a change in the title text.
    *
    *  This signal is emitted when the user selects a new title command from the menu.
    *
    *  @param   pAction
    *           The action of the newly selected title.
    *
    *  @see     setTitle(), setMenu()
    */
   void titleChanged(QAction* pAction);

protected:
   /**
    *  Toggles the title widget between a label and a push button.
    *
    *  @param   bButton
    *           TRUE if the title widget should be a push button.  FALSE if the title
    *           widget should be a label.
    */
   void setTitleButton(bool bButton);

   /**
    *  Resizes the title push button based on its font.
    *
    *  @param   fntTitle
    *           The font used for the title text.  The font metrics are obtained to set
    *           an appropriate size for the push button.
    */
   void resizeTitleButton(QFont fntTitle);

protected slots:
   /**
    *  Sets the title text based on the given menu action.
    *
    *  This method is called automatically when the user selects a title action from the
    *  menu.
    *
    *  @param   pAction
    *           The action containing the new title text.
    */
   void setTitle(QAction* pAction);

private:
   QLabel* mpTitle;
   QPushButton* mpButton;
   QLabel* mpDescription;
   QLabel* mpIconLabel;
};

#endif
