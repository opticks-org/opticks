/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LABELEDSECTION_H
#define LABELEDSECTION_H

#include <QtGui/QWidget>
#include <QtCore/QString>

class QLabel;

/**
 *  Used to organize complex widgets into smaller sections.
 *
 *  This widget should be used within complex layouts which have logical
 *  sections which can be split up.  By placing each section within a labeled
 *  section, a these complex widgets can have a consistent appearance
 *  throughout the application.
 *
 *  When a section widget is added, an expand/collapse indicator appears in the
 *  header next to the text label.  When the user clicks on the indicator the
 *  section widget is shown or hidden, giving the appearance of expanding or
 *  collapsing the labeled section.  This is especially useful when multiple
 *  labeled sections are contained in a LabeledSectionGroup, which provides a
 *  common layout in a scroll area.
 *
 *  @see        setSectionWidget()
 */
class LabeledSection : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates a labeled section with no header text and no section widget.
    *
    *  @param   pParent
    *           The parent widget.
    */
   LabeledSection(QWidget* pParent = NULL);

   /**
    *  Creates a labeled section with header text but no section widget.
    *
    *  This constructor may be used to create a simple separator between other
    *  widgets.  With no section widget, the expand/collapse indicator is not
    *  shown.
    *
    *  @param   text
    *           The text to dispay in the header.
    *  @param   pParent
    *           The parent widget.
    */
   LabeledSection(const QString& text, QWidget* pParent = NULL);

   /**
    *  Creates a labeled section with header text and a section widget.
    *
    *  @param   pSectionWidget
    *           The widget to display below the header.  When a valid section
    *           widget is given, the widget is shown and the labeled section
    *           assumes ownership of the new section widget.  The labeled
    *           section will delete the section widget when the labeled section
    *           itself is deleted.
    *  @param   text
    *           The text to dispay in the header.
    *  @param   pParent
    *           The parent widget.
    */
   LabeledSection(QWidget* pSectionWidget, const QString& text, QWidget* pParent = NULL);

   /**
    *  Destroys the labeled section and the section widget.
    */
   ~LabeledSection();

   /**
    *  Returns the header text.
    *
    *  @return  The header text.
    *
    *  @see     setText()
    */
   QString getText() const;

   /**
    *  Sets the text in the header.
    *
    *  @param   newText
    *           The new text to use in the header.
    *
    *  @see     getText()
    */
   void setText(const QString &newText);

   /**
    *  Returns the contained section widget.
    *
    *  @return  The contained section widget, which may be \c NULL.
    *
    *  @see     setSectionWidget()
    */
   QWidget* getSectionWidget() const;

   /**
    *  Sets the contained section widget.
    *
    *  If the widget already contains a section widget, the parent of the old
    *  section widget is set to \c NULL and the widget is not deleted.  The
    *  caller of this function then assumes ownership of the old section
    *  widget and is responsible for deleting it.
    *
    *  When a valid section widget is given, an expand/collapse indicator
    *  appears in the header next to the text label.  When the user clicks on
    *  the indicator the section widget is shown or hidden, giving the
    *  appearance of expanding or collapsing the labeled section.
    *
    *  @param   pNewSectionWidget
    *           The new section widget to display.  The labeled section assumes
    *           ownership of the new section widget and will delete the section
    *           widget when the labeled section itself is deleted.
    *
    *  @see     getSectionWidget()
    */
   void setSectionWidget(QWidget* pNewSectionWidget);

public slots:
   /**
    *  Collapses the section widget to show just the header.
    *
    *  If the widget does not have a section widget, this method does nothing.
    */
   void collapse();

   /**
    *  Expands the section widget to show both the header and the section
    *  widget.
    *
    *  If the widget does not have a section widget, this method does nothing.
    */
   void expand();

signals:
   /**
    *  Emitted when the section widget is hidden.
    *
    *  This signal is emitted when the section widget is hidden either
    *  programmatically by calling collapse() or when the user clicks the
    *  collapse indicator (-) next to the header text.
    */
   void collapsed();

   /**
    *  Emitted when the section widget is shown.
    *
    *  This signal is emitted when the section widget is shown either
    *  programmatically by calling expand() or when the user clicks the expand
    *  indicator (+) next to the header text.
    */
   void expanded();

protected:
   /**
    *  Emits the collapsed() and expanded() signals when the section widget is
    *  hidden or shown.
    *
    *  By default, when the labeled section contains a section widget, an event
    *  filter is installed on the section widget to emit either the collapsed()
    *  or expanded() signal.  If another event filter is installed on the
    *  labeled section and the given object is not the section widget, this
    *  method is just a pass-through to the default QWidget implementation.
    *
    *  @param   pObject
    *           The object prompting the event.
    *  @param   pEvent
    *           The event invoked by the object.
    *
    *  @return  Returns the value returned by the default QWidget
    *           implementation.
    */
   bool eventFilter(QObject* pObject, QEvent* pEvent);

   /**
    *  Expands or collapses the section widget.
    *
    *  This method is called by Qt when the user clicks inside the section
    *  widget.  If the user clicked on the expand/collapse indicator in the
    *  header, the section is shown or hidden and the expanded() or collapsed()
    *  signal is emitted accordingly.
    *
    *  @param   pEvent
    *           The mouse event associated with the mouse press.
    */
   void mousePressEvent(QMouseEvent* pEvent);

   /**
    *  Expands or collapses the section widget.
    *
    *  This method is called by Qt when the user double clicks inside the
    *  section widget.  If the user clicked on the header text or the
    *  horizontal line, the section is shown or hidden and the expanded() or
    *  collapsed() signal is emitted accordingly.
    *
    *  @param   pEvent
    *           The mouse event associated with the mouse double click.
    */
   void mouseDoubleClickEvent(QMouseEvent* pEvent);

private:
   void initialize(const QString& text = QString(), QWidget* pSectionWidget = NULL);
   void updateIndicator();

   QLabel* mpExpandLabel;
   QLabel* mpTextLabel;
   QWidget* mpSectionWidget;
};

#endif
