/*	Parameter_Tree_Model

HiROC CVS ID: $Id: Parameter_Tree_Model.hh,v 1.19 2012/08/31 06:50:04 castalia Exp $

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

#ifndef Parameter_Tree_Model_hh
#define Parameter_Tree_Model_hh

#include	<QAbstractItemModel>

#include	<iosfwd>

//	Forward references.
namespace idaeim {
namespace PVL {
class Parameter;
class Aggregate;
}}


namespace UA
{
namespace HiRISE
{
/**	The <i>Parameter_Tree_Model</i> implements a QAbstractItemModel for
	use with an idaeim::PVL::Aggregate.

	This implementation of the QAbstractItemModel interface enables an
	idaeim::PVL::Aggregate tree of zero or more idaeim::PVL::Parameter
	objects to be managed by Qt widgets such as a Parameter_Tree_View
	subclass of QTreeView.

	<b>N.B.</b>: The model is implemented as read-only; no editing
	operations are supported.

	@author		Parker Snell, UA/HiROC
	@version	$Revision: 1.19 $
*/
class Parameter_Tree_Model
:	public QAbstractItemModel
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
//! Class identification name with source code version and date.
static const char* const
	ID;


//!	Default {@link root_name() root name}.
static const char* const
	DEFAULT_ROOT_NAME;

/*==============================================================================
	Constructors
*/
/**	Constructs a Parameter_Tree_Model on an Aggregate.

	<b>N.B.</b>: The Parameter_Tree_Model does not take ownership of the
	Aggregate. The Aggregate must not be deleted during the lifespan of
	the model. The Aggregate should not be changed as the model presumes
	read-only data with no editing operations supported.

	@param	root	A pointer to an idaeim::PVL::Aggregate. If NULL an
		empty model is constructed.
	@param	parent	The parent QObject that will take ownership of
		the model.
*/
explicit Parameter_Tree_Model (idaeim::PVL::Aggregate* root = NULL,
	QObject* parent = NULL);

/**	Copy constructor.

	The root {@link root_visible() visibility} and {@link root_name() name}
	are copied along with the {@link values() parameters} binding.

	@param	model	The Parameter_Tree_Model to be copied. The existing
		{@link values() parameters} bound to the model are replaced, not
		deleted.
*/
Parameter_Tree_Model (const Parameter_Tree_Model& model);

/**	Assign another Parameter_Tree_Model to this Parameter_Tree_Model.

	The root {@link root_visible() visibility} and {@link root_name() name}
	are assigned along with the {@link values() parameters} binding.

	@param	model	The Parameter_Tree_Model to be assigned. The existing
		{@link values() parameters} bound to the model are replaced, not
		deleted.
*/
Parameter_Tree_Model& operator= (const Parameter_Tree_Model& model);

/*==============================================================================
	QAbstractItemModel interface implementation
*/
/**	Get the index of an item in the tree at the row and column location
	of a parent item.

	@param	row		The row within the parent item of the desired item.
	@param	column	The column within the parent item of the desired item.
		<b>N.B.</b>: This must be 0, for a Parameter name, or 1, for a
		Paramater Value.
	@param	parent_index	A reference to a QModelIndex for the parent
		item to which the row and column are relative. An invalid parent
		index refers to the root of the tree.
	@return	A QModelIndex that locates the desired item within the tree.
*/
virtual QModelIndex index (int row, int column,
	const QModelIndex& parent_index = QModelIndex ()) const;

/**	Get the data for the role of a tree item.

	Recognized roles are:
<dl>
<dt>Qt::DisplayRole
<dd>If the index is not valid an invalid QVariant is returned. Otherwise
	a QVariant for a QString is returned.
	<dl>
	<dt>For an index of column 0 - the "Name" column
	<dd>If the index refers to the root node the {@link root_name()
		root name} will be returned; otherwise the name of the parameter
		refered to by the index is returned.
	<dt>For an index of column 1 - the "Value" column
	<dd>If the index refers to an Aggregate parameter the type name
		(translated) of the Aggregate is returned. Otherwise, the Value
		of the Parameter as a string representation is returned. If the
		Value is an Array the string is the type name (translated) of the
		Array, otherwise it is the Value's string representation.
	</dl>
<dt>Qt::TextAlignmentRole
<dd>A QVariant for the the Qt::Alignment flags Qt::AlignTop | Qt::AlignLeft
	is returned.
</dl>

	@param	index	A reference to a QModelIndex for the item of interest.
	@param	role	A Qt::ItemDataRole indicating what kind of data to
		obtain.
	@return	A QVariant containing the requested data. This will be
		invalid if the role is not recognized or the index is invalid
		for a Qt::DisplayRole.
*/
virtual QVariant data
	(const QModelIndex& index, int role = Qt::DisplayRole) const;

/**	Get the index of the parent of an indexed tree item.

	@param	index	The tree item for which a parent index is desired.
	@return	A QModelIndex for the parent of the indexed item. This will
		be invalid if the index is invalid or the item has no parent.
*/
virtual QModelIndex parent (const QModelIndex& index) const;

/**	Get the number of rows for an indexed tree item.

	@param	index	A reference to a QModelIndex for the item of
		interest. An invalid index refers to the root of the tree.
	@return	The number of rows contained within the item. This will be
		zero if the item has no children. It will be 1 if the index
		is invalid and the {@link root_visible() root is visible}.
*/
virtual int rowCount (const QModelIndex& index = QModelIndex ()) const;

/**	Get the number of columns in the tree model.

	@param	A QModelIndex reference (unused and optional).
	@return Always returns 2.
*/
virtual int columnCount (const QModelIndex& = QModelIndex ()) const
	{return 2;}

/**	Get column header data.

	The header data is a QString. For section (column) 0 the string is
	"Name"; for section 1 the string is "Value". These strings are
	translated.

	@param	section	The header section for which data is desired.
		<b>N.B.</b> A Parameter_Tree_Model only has two {@link
		columnCount(const QModelIndex&) columns}, so for any value other
		than 0 or 1 an invalid variant will be returned.
	@param	orientation	The orientation of the header to which the
		section applies. <b>N.B.</b>: A Parameter_Tree_Model only has
		column headers, so if this is not Qt::Horizontal an invalid
		variant will be returned.
	@param	role	The header data role for which data is desired. Only
		the Qt::DisplayRole is recognized; for any other role an invalid
		variant will be returned.
*/
virtual QVariant headerData (int section,
	Qt::Orientation orientation = Qt::Horizontal,
	int role = Qt::DisplayRole) const;

/*==============================================================================
	Accessors
*/
/**	Get the parameters bound to this Parameter_Tree_Model.

	@return	A pointer to the idaeim::PVL::Aggregate bound to this
		Parameter_Tree_Model.
*/
idaeim::PVL::Aggregate* parameters () const
	{return Root;}

/**	Test if the root of the tree hierarchy is visible.

	@return	true if the top level root Aggregate node of the tree model
		is visible; false if the root node is not visible.
*/
bool root_visible () const
	{return Root_Visible;}

/**	Enable or disable visibility of the root node of the tree hierarchy.

	If the visiblity of the root node will change a layoutAboutToBeChanged
	signal will be emitted before the change takes place, and a
	layoutChanged signal will be emitted after the change is applied.

	@param	enable	If true, the top level root Aggregate node of the
		tree model is to be visible; if false, the root node is not to be
		visible.
*/
void root_visible (bool enable);

/**	Get the default name of the root tree node.

	@return	A String with the default name of the root node.
	@see	root_name() const
*/
inline static QString default_root_name ()
	{return DEFAULT_ROOT_NAME;}

/**	Get the name of the root tree node.

	@return	A String with the name of the root node. This will be "Root"
		if the {@link root_name(const QString&) root name} has not been set.
*/
const QString& root_name () const
	{return Root_Name;}

/**	Set the name of the root tree node.

	<b>N.B.</b>: The root node name will only appear in a model viewer
	when the {@link root_visible(bool) root node is visible}.

	If the name of the root node changed and the root node is visible then
	a dataChanged signal will be emitted.

	@param	name	A Qstring providing the name of the root node.
	@see	root_name()
*/
void root_name (const QString& name);

/**	Get the comments for the Parameter at a model index.

	@param	index	A QModelIndex for the Parameter from which to get the
		comments.
	@return	A QString containing the Parameter comments. This may be
		the empty string if the Parameter has no comments or if the
		index is invalid.
*/
QString comments (const QModelIndex& index) const;

/*==============================================================================
	Helpers
*/
private:

/**	Get the row of a Parameter in its parent.

	@param	parameter	A pointer to a Parameter.
	@return	The Aggregate entry index of the parameter in its parent.
		This will be -1 if the parameter is NULL. If the parameter has no
		parent zero will be returned if the {@link root_visible() root is
		visible}, otherwise -1 will be returned.
*/
int parameter_row (idaeim::PVL::Parameter* parameter) const;

/*==============================================================================
	Data
*/
private:

//!	The root of the tree. <b/N.B.</b>: The Aggregate is owned externally.
idaeim::PVL::Aggregate*
	Root;

bool
	Root_Visible;

QString
	Root_Name;
};

/*==============================================================================
	Utility
*/
/**	Output operator to describe a QModelIndex.

	The description is a single, non-indented line with the index row,
	column, and internalPointer value, with no EOL.

	@param	stream	A reference to the stream where the description will
		be sent.
	@param	index	A reference to the QModelIndex to be described.
*/
std::ostream& operator<< (std::ostream& stream, const QModelIndex& index);


}	// end namespace HiRISE
}	// end namespace UA
#endif
