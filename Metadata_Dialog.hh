/*	Metadata_Dialog

HiROC CVS ID: $Id: Metadata_Dialog.hh,v 1.12 2012/08/31 07:18:02 castalia Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/

#ifndef Metadata_Dialog_hh
#define Metadata_Dialog_hh

#include	<QDialog>

//	Forward references.
namespace idaeim {
namespace PVL {
class Parameter;
class Aggregate;
}}

class QWidget;
class QTreeView;
class QTextEdit;
class QItemSelection;
class QMenuBar;
class QString;
class QFileDialog;
class QErrorMessage;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Parameter_Tree_View;

/**	A <i>Metadata_Dialog</i> provides a dialog box to display metadata
	contained in an <i>idaeim::PVL::Aggregate</i> structure.

	@author		Parker Snell, UA/HiROC
	@version	$Revision: 1.12 $
*/
class Metadata_Dialog
:	public QDialog
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

//!	Default size of the dialog.
static const int
	DEFAULT_WIDTH,
	DEFAULT_HEIGHT;

/*==============================================================================
	Constructor
*/
/**	Constructs a Metadata_Dialog.

	<b>>>> WARNING <<<</b> The Metadata_Dialog does not take ownership
	of the Aggregate. The Aggregate must not be deleted during the
	lifespan of the dialog. Changes to the Aggregate may not be noticed
	right away.

	@param	parameters	A pointer to an Aggregate which contains all the
		metadata parameters to be shown in the dialog.
	@param	parent	A pointer to the QWidget that takes ownership of this
		Metadata_Dialog object, the parent will delete its children when it
		is deleted.
	@param	flags	A set of optional Qt::WindowFlags that can be used to
		control the appearance of the dialog window.
*/
explicit Metadata_Dialog (idaeim::PVL::Aggregate* parameters,
	QWidget* parent = NULL, Qt::WindowFlags flags = 0);

/**	Constructs an empty Metadata_Dialog.

	<b>N.B.</b>: The {@link parameters() parameters} will be NULL.

	@param	parent	A pointer to the QWidget that takes ownership of this
		Metadata_Dialog object, the parent will delete its children when it
		is deleted.
	@param	flags	A set of optional Qt::WindowFlags that can be used to
		control the appearance of the dialog window.
*/
explicit Metadata_Dialog (QWidget* parent = NULL, Qt::WindowFlags flags = 0);

/*==============================================================================
	Accessors
*/
/**	Get the parameters being displayed.

	@return	A pointer to an Aggregate which contains all the
		metadata parameters being shown in the dialog.
*/
idaeim::PVL::Aggregate* parameters () const;

/**	Get the Parameter_Tree_View used to display the Parameter tree.

	@return	A pointer to the Parameter_Tree_View owned by this Metadata_Dialog.
*/
inline Parameter_Tree_View* tree_view () const
	{return Tree_View;}

/**	Test if the root of the tree hierarchy is visible.

	@return	true if the top level root Aggregate node of the tree is
		visible; false if the root node is not visible.
*/
bool root_visible () const;

/**	Enable or disable visibility of the root node of the tree hierarchy.

	@param	enable	If true, the top level root Aggregate node of the
		tree is to be visible; if false, the root node is not to be
		visible.
*/
void root_visible (bool enable);

/**	Get the default name of the root tree node.

	@return	A String with the default name of the root node.
	@see	root_name() const
*/
static QString default_root_name ();

/**	Get the name of the root tree node.

	@return	A String with the name of the root node. This will be the
		{@link default_root_name() default root name} if the {@link
		root_name(const QString&) root name} has not been set.
*/
QString root_name () const;

/**	Set the name of the root tree node.

	<b>N.B.</b>: The root node name will only appear in the tree viewer
	when the {@link root_visible(bool) root node is visible}.

	@param	name	A Qstring providing the name of the root node.
	@see	root_name()
*/
void root_name (const QString& new_name);

/**	Test if alternating row colors are enabled.

	@return	true if alternating row colors are enabled; false otherwise.
*/
bool alternating_row_colors () const;

/**	Enable or disable alternating row colors.

	@param	enableed	If true if alternating row colors are enabled;
		false disables alternating row colors.
*/
void alternating_row_colors (bool enable);

/**	Get the preferred size of the dialog window.

	@return	A QSize containing the {@link #DEFAULT_WIDTH} and {@link
		#DEFAULT_HEIGHT} values.
*/
virtual QSize sizeHint () const;

inline static QErrorMessage* error_message ()
	{return Error_Message;}

//	Ownership of the QErrorMesage is NOT transferred.
inline static void error_message (QErrorMessage* dialog)
	{Error_Message = dialog;}

/*==============================================================================
	Event Handler
*/
protected:

/**	Check for hide or show events.

	If the QEvent::Hide or QEvent::Show event type has occured the
	{@link visibility_changed(bool) visibility_changed signal} is emitted.

	@param	event	A pointer to the QEvent that has occured. The event
		is always propagated on to the QDialog base class.
*/
virtual bool event (QEvent *event);

/*==============================================================================
	Signals
*/
public:

signals:

/**	Signal when the dialog visibility has changed.

	@param	visible	true if the dialog has become visible (show);
		false if the diaglog has become invisible (hide).
*/
void visibility_changed (bool visible);

/*==============================================================================
	Slots
*/
public slots:

/**	Set the parameters to be displayed.

	<b>>>> WARNING <<<</b> The Metadata_Dialog does not take ownership
	of the Aggregate. The Aggregate must not be deleted during the
	lifespan of the dialog.

	@param	parameters	A pointer to an Aggregate which contains all the
		metadata parameters to be shown in the dialog.
*/
void parameters (idaeim::PVL::Aggregate* parameters);

/**	Save a metadata parameter to a file selected by a dialog.

	The specified metadata may be an Aggregate containing a list of
	parameters or a single Assignment Parameter.

	A file selection dialog is presented to choose the pathname of the
	file in which the metadata will be written. If no file is selected
	nothing is done.
 
	@param	metadata	A pointer to an idaeim::PVL::Parameter. If NULL all
		available metadata is used. However, if no metadata is available
		nothing is done.
	@return	true if the metadata was successfully saved; false otherwise.
	@see	save(idaeim::PVL::Parameter*, const QString&)
*/
bool save_metadata (idaeim::PVL::Parameter* metadata = NULL,
	const QString& title = QString ());

/**	Save the selected metadata to a file selected by a dialog.

	@return	true if the metadata was successfully saved; false otherwise.
	@see	save(idaeim::PVL::Parameter*)
*/
bool save_selected_metadata ();


private slots:

/**	Receives the QTreeView selection model selectionChanged signal.

	If at least one tree item is included in the current selection
	the first QModelIndex is used to obtain the text for the Comments
	pane. If nothing is selected the Comments pane is cleared.

	@param	current		A QItemSelection for the new selection.
	@param	previous	A QItemSelection for the old selection (not used).
*/
void selection_changed
	(const QItemSelection& current, const QItemSelection& previous);

/*==============================================================================
	Utilities
*/
public:

/**	Save metadata to a file.
	
	@param	parameters	A pointer to an Aggregate which contains all the
		metadata parameters to be written. If NULL nothing is done.
	@param	pathname	The pathname of the file to which the metadata is
		to be written. If the pathname is empty, or refers to an existing
		file that is not a regular file or is not writable, nothing is
		done.
*/
static bool save
	(const idaeim::PVL::Parameter* const parameters, const QString& pathname);

/*==============================================================================
	Helpers
*/
private:

//!	Initialize the widget.
void initialize ();

//!	Create the menus used by the dialog.
QMenuBar* create_menus ();

/*==============================================================================
	Data
*/
private:

Parameter_Tree_View*
	Tree_View;

QTextEdit*
	Comments;

QFileDialog* 
	Save_Dialog;

QAction*
	Save_Selected_Action;

static QErrorMessage
	*Error_Message;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
