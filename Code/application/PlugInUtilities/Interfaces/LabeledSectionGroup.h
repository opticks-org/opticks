/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LABELEDSECTIONGROUP_H
#define LABELEDSECTIONGROUP_H

#include <QtCore/QMap>
#include <QtGui/QLayout>
#include <QtGui/QScrollArea>

#include <vector>

class LabeledSection;

/**
 *  A widget containing multiple LabeledSection widgets that provides a common
 *  layout in a scroll area.
 *
 *  This widget subclasses QScrollArea and organizes multiple LabeledSection
 *  widgets in a QVBoxLayout.  Sections are added with optional parameters that
 *  would normally be passed into a QVBoxLayout when adding child widgets. Each
 *  section in the layout contains extra spacing to make it easier to visually
 *  separate the sections.
 *
 *  For size purposes, each LabeledSection can be expanded or collapsed to
 *  display the entire section widget or just its header label. For adjacent sections,
 *  call addSection() with a very large stretch value (e.g.: 1000) for each section
 *  followed by addStretch() with a relatively small value (e.g.: 1) to obtain optimal results.
 *
 *  @see     LabeledSection
 */
class LabeledSectionGroup : public QScrollArea
{
   Q_OBJECT

public:
   /**
    *  Creates an empty labeled section group.
    *
    *  @param   pParent
    *           The parent widget for the group.
    */
   LabeledSectionGroup(QWidget* pParent = NULL);

   /**
    *  Destroys the labeled section group.
    *
    *  When the group is destroyed, all labeled section widgets contained in
    *  the group are also destroyed.  Call clear() prior to destroying the
    *  group to take ownership of the section widgets.
    */
   ~LabeledSectionGroup();

   /**
    *  Adds a labeled section to the group.
    *
    *  @param   pSection
    *           The labeled section widget to add to the group.  This method
    *           does nothing if \c NULL is passed in.
    *  @param   stretch
    *           The stretch factor to assign to the added section.  The stretch
    *           factor is passed into QVBoxLayout::addWidget().
    *  @param   alignment
    *           The layout alignment to assign to the added section.  The
    *           alignment is passed into QVBoxLayout::addWidget().
    *
    *  @see     addStretch()
    */
   void addSection(LabeledSection* pSection, int stretch = 0, Qt::Alignment alignment = 0);

   /**
    *  Adds a stretch item to the end of the layout.
    *
    *  @param   stretch
    *           The stretch factor to add to the layout.  The stretch factor is
    *           passed into QVBoxLayout::addStretch().
    */
   void addStretch(int stretch = 0);

   /**
    *  Removes a labeled section from the group.
    *
    *  This method removes the section from the group but does not delete it.
    *  Ownership is transferred to the calling object.
    *
    *  @param   pSection
    *           The labeled section widget to remove from the group.  This
    *           method does nothing if \c NULL is passed in or if the given
    *           section does not exist in the group.
    */
   void removeSection(LabeledSection* pSection);

   /**
    *  Queries whether a labeled section exists in the group.
    *
    *  @param   pSection
    *           The labeled section widget to query for its existance in the
    *           group.
    *
    *  @return  Returns \c true if the group contains the given labeled
    *           section; otherwise returns \c false.
    */
   bool hasSection(LabeledSection* pSection) const;

   /**
    *  Returns all labeled sections contained in the group.
    *
    *  @return  A vector containing the labeled sections widgets in the group.
    */
   std::vector<LabeledSection*> getSections() const;

   /**
    *  Expands a labeled section within the group.
    *
    *  This method visually expands the labeled section by showing its section
    *  widget.
    *
    *  @param   pSection
    *           The labeled section widget to expand.  This method does nothing
    *           if \c NULL is passed in or if the given section does not exist
    *           in the group.
    *
    *  @see     hasSection(), LabeledSection::expand()
    */
   void expandSection(LabeledSection* pSection);

   /**
    *  Collapses a labeled section within the group.
    *
    *  This method visually collapses the labeled section by hiding its section
    *  widget.
    *
    *  @param   pSection
    *           The labeled section widget to collapse.  This method does
    *           nothing if \c NULL is passed in or if the given section does
    *           not exist in the group.
    *
    *  @see     hasSection(), LabeledSection::collapse()
    */
   void collapseSection(LabeledSection* pSection);

   /**
    *  Removes all sections and spacer items from the layout.
    *
    *  This method removes all sections from the group but does not delete
    *  them.  Ownership for all section widgets is transferred to the calling
    *  object.
    */
   void clear();

   /**
    *  Returns the preferred size of the group.
    *
    *  @return  The preferred size of the group that was set by calling
    *           setSizeHint().  If setSizeHint() has not been called, the value
    *           returned from the base class QScrollArea::sizeHint()
    *           implementation is returned.
    */
   QSize sizeHint() const;

signals:
   /**
    *  Emitted when one of the contained section widgets is hidden.
    *
    *  This signal is emitted when one of the contained section widgets is
    *  hidden either programmatically by calling collapseSection() or when the
    *  user clicks the collapse indicator (-) next to the header text.
    *
    *  @param   pSection
    *           The labeled section that was collapsed.
    *
    *  @see     LabeledSection::collapsed()
    */
   void sectionCollapsed(LabeledSection* pSection);

   /**
    *  Emitted when one of the contained section widgets is shown.
    *
    *  This signal is emitted when one of the contained section widgets is
    *  hidden either programmatically by calling expandSection() or when the
    *  user clicks the expand indicator (+) next to the header text.
    *
    *  @param   pSection
    *           The labeled section that was expanded.
    *
    *  @see     LabeledSection::expanded()
    */
   void sectionExpanded(LabeledSection* pSection);

protected:
   /**
    *  Sets the preferred size of the group.
    *
    *  This is a convenience method that creates a QSize from the given width
    *  and height and calls setSizeHint(const QSize&).
    *
    *  @param   width
    *           The preferred group width.
    *  @param   height
    *           The preferred group height.
    */
   void setSizeHint(int width, int height);

   /**
    *  Sets the preferred size of the group.
    *
    *  This method sets a custom preferred size of the group widget that may
    *  need to be called based on the contents of the labeled sections
    *  contained in the group.
    *
    *  @param   size
    *           The preferred group size.
    *
    *  @see     setSizeHint(int, int)
    */
   void setSizeHint(const QSize& size);

private slots:
   void enableSectionStretch();
   void disableSectionStretch();

private:
   LabeledSectionGroup(const LabeledSectionGroup& rhs);
   LabeledSectionGroup& operator=(const LabeledSectionGroup& rhs);

   QVBoxLayout* mpLayout;
   QSize mPreferredSize;
   QMap<LabeledSection*, int> mSections;
};

#endif
