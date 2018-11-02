/*	Value_Tree_Model

HiROC CVS ID: $Id: Value_Tree_Model.hh,v 1.10 2012/05/10 05:34:01 castalia Exp $

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

#ifndef Value_Tree_Model_hh
#define Value_Tree_Model_hh

#include	<QAbstractItemModel>

//	Forward references.
namespace idaeim {
namespace PVL {
class Array;
class Value;
}}


namespace UA
{
namespace HiRISE
{
/**	The <i>Value_Tree_Model</i> implements a QAbstractItemModel for use
	with an idaeim::PVL::Array.

	This implementation of the QAbstractItemModel interface enables an
	idaeim::PVL::Array tree of zero or more PVL::Value objects to be
	managed by Qt widgets such as a QTreeView.

	<b>N.B.</b>: The model is implemented as read-only; no editing
	operations are supported.

	@author		Parker Snell, UA/HiROC
	@version	$Revision: 1.10 $
*/
class Value_Tree_Model
:	public QAbstractItemModel
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
//! Class identification name with source code version and date.
static const char * const
	ID;

/*==============================================================================
	Constructors
*/
/**	Constructs a Value_Tree_Model on an Array.

	<b>N.B.</b>: The Value_Tree_Model does not take ownership of the
	Array. The Array must not be deleted during the lifespan of the
	model. The Array should not be changed as the model presumes
	read-only data with no editing operations supported.

	@param	root	A pointer to an idaeim::PVL::Array. If NULL an empty
		model is constructed.
	@param	parent	The parent QObject that will take ownership of
		the model.
*/
explicit Value_Tree_Model (idaeim::PVL::Array* root = 0,
	QObject* parent = NULL);

/**	Copy constructor.

	@param	model	The Value_Tree_Model to be copied. The existing
		{@link values() values} bound to the model are replaced, not
		deleted.
*/
Value_Tree_Model (const Value_Tree_Model& model);

/**	Assign another Value_Tree_Model to this Value_Tree_Model.

	@param	model	The Value_Tree_Model to be assigned. The existing
		{@link values() values} bound to the model are replaced, not
		deleted.
*/
Value_Tree_Model& operator= (const Value_Tree_Model& model);

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
virtual QModelIndex index
	(int row, int column, const QModelIndex& parent = QModelIndex ()) const;

/**	Get the data for the role of a tree item.

	If the role is not Qt::DisplayRole or the index is not valid an
	invalid QVariant is returned. Otherwise a QVariant for a QString is
	returned. If the Value from the index is an Array the string is the
	type name (translated) of the Array, otherwise it is the Value's
	string representation.

	@param	index	A reference to a QModelIndex for the item of interest.
	@param	role	A Qt::ItemDataRole indicating what kind of data to
		obtain.
	@return	A QVariant containing the requested data. This will be
		invalid if the role is not recognized or the index is invalid.
*/
virtual QVariant data
	(const QModelIndex& index, int role = Qt::DisplayRole) const;

/**	Get the index of the parent of an indexed tree item.

	@param	index	The tree item for which a parent index is desired.
	@return	A QModelIndex for the parent of the indexed item. This will
		be invalid if the index is invalid or the item has no parent.
*/
virtual QModelIndex parent (const QModelIndex& index) const;

/**	Returns the number of rows for a given <i>parent</i>. If the item at
	<i>parent</i> has no children, 0 is returned.
*/
virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

/**	Get the number of columns in the tree model.

	@param	A QModelIndex reference (unused and optional).
	@return Always returns 1.
*/
virtual int columnCount (const QModelIndex& = QModelIndex ()) const
	{return 1;}

/*==============================================================================
	Accessors
*/
/**	Get the values bound to this Value_Tree_Model.

	@return	A pointer to the idaeim::PVL::Array bound to this
		Value_Tree_Model.
*/
idaeim::PVL::Array* values () const
	{return Root;}

/**	Get the units for the Value at a model index.

	@param	index	A QModelIndex for the Value from which to get the
		units.
	@return	A QString containing the Value units. This may be the empty
		string if the Value has no units or if the index is invalid.
*/
QString units (const QModelIndex& index) const;

/*==============================================================================
	Helpers
*/
private:

/**	Get the row of a Value in its parent.

	@param	value	A pointer to a Value.
	@return	The Array entry index of the value in its parent. This will
		be -1 if the value is NULL. It will be zero if the value has no
		parent.
*/
int value_row (idaeim::PVL::Value* value) const;

/*==============================================================================
	Data
*/
private:

//	The root of the tree.
idaeim::PVL::Array*
	Root;

};

}	// end namespace HiRISE
}	// end namespace UA
#endif
