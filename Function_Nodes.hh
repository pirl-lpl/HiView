/*	Function_Nodes

HiROC CVS ID: $Id: Function_Nodes.hh,v 1.3 2012/09/06 09:00:46 castalia Exp $

Copyright (C) 2010-2011  Arizona Board of Regents on behalf of the
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

#ifndef Function_Nodes_hh
#define Function_Nodes_hh

#include	<qwt.h>
#include <qwt_series_store.h>
#include <qwt_compat.h>
#include	<QPolygon>
#include	<QPoint>

#include	<iosfwd>

//	Forward references.
template<typename T> class QVector;



namespace UA
{
namespace HiRISE
{
/**	<i>Function_Nodes</i> contains the node points of a function that
	defines the contents of a Data_Map.

	A Function_Nodes object contains a QPolygon of data map function node
	points. The node points are used to define the values of the Data_Map
	vector to which the the Function_Nodes are bound.

	Each node is a coordinate in which the x value is a Data_Map index
	and the y value is the Data_Map entry at the index. The node points
	are maintained in index order. Data_Map values between the node points
	are interpolated. Currently only a linear interpolation algorithm is
	provided. Two required nodes are always provided: lower and upper
	anchor nodes for the first (zero) and last (highest) Data_Map index
	locations.

	Whenever the nodes list is changed the Data_Map entries are modified
	accordingly.

	Lower and upper bound nodes are recognized. The lower bound node
	immediately preceeds the first (lowest x index) node with a non-zero
	Data_Map entry value, unless the first (lower anchor) node has a
	non-zero entry (y) in which case it is the lower bound node.
	Similarly, the upper bound node is found and the upper end of the
	nodes list. When the lower or upper bound node value is change

	Function_Nodes implements the QwtData class


	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.3 $
*/
class Function_Nodes
:	public QwtSeriesStore<QPointF>
{
public:
/*==============================================================================
	Types
*/
//!	A lookup table that maps source data values to display image values.
typedef QVector<quint8>		Data_Map;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/**	Node list change code values returned from methods that may change
	the node list.
<dl>
<dt>REMOVED_NODES_MASK
	<dd>That portion of the code value that contains the count of nodes
		that were removed from the list.
<dt>ADDED_NODE
	<dd>Flag indicating that a node was added to the list.
<dt>SHIFTED_NODE
	<dd>Flag indicating that a node was shifted - its Data_Map entry (y)
		value was changed without changing its index location (x).
*/
enum
	{
	REMOVED_NODES_MASK	= 0xFFFF,
	ADDED_NODE			= (1 << 16),
	SHIFTED_NODE		= (1 << 17)
	};

/*==============================================================================
	Constructor
*/
/**	Construct a Function_Nodes object bound to a Data_Map.

	<b>N.B.</b>: The Data_Map is {@link reset() reset}.

	@param	data_map	A pointer to the Data_Map to which the
		Function_Nodes object is to be bound. This should not be NULL;
		if it is no nodes will be present and the {@link max_x() maximum
		x index} will be -1.
*/
explicit Function_Nodes (Data_Map* data_map);

/*==============================================================================
	Accessors
*/
/**	Get the list of function node points.

	<b>>>> WARNING <<<</b> The function node list should not be modified.
	This can result in a node list that does not correctly reflect the
	{@link data_map() data map} contents that is expected to be defined
	by the nodes list.

	@param	A pointer to the QPolygon containing the function nodes in
		data map index order.
*/
inline QPolygon* nodes ()
	{return &Nodes;}

/**	Get the Data_Map to which this Function_Nodes is bound.

	<b>>>> WARNING <<<</b> The data map contents should not be modified.
	This can result in a data map that does not correctly reflect the
	{@link nodes() nodes list} that is expected to define the contents of
	the data map.

	@return	A pointer to the Data_Map to which the Function_Nodes object
		bound.
*/
inline Data_Map* data_map ()
	{return Map;}

/**	Set the Data_Map to which this Function_Nodes is bound.

	<b>N.B.</b>: The Data_Map is {@link reset() reset}.

	@param	data_map	A pointer to the Data_Map to which the
		Function_Nodes object is to be bound. If NULL the current
		data map remains unchanged.
*/
void data_map (Data_Map* data_map);

/**	Get the maximum {@link data_map() data map} index value.

	The data map index value corresponds to a node point x location.

	@return	The maximum data map index value; this is one minus the size
		of the Data_Map.
*/
inline int max_x () const
	{return Map->size () - 1;}

/**	Get the maximum {@link data_map() data map} entry value.

	The data map entry value corresponds to a node point y position.

	@return	The maximum data map entry value; this is determined by the
		data precision of the Data_Map values, which are expected to be
		8-bit values.
*/
inline int max_y () const;

/**	Get the {@link nodes() node list} index of the node with a
	specified data map index.

	@param	x	A data map index.
	@return The nodes list index of the node with the data map index;
		or -1 if no node is at the specified data map index location.
*/
int node_index_of (int index) const;

/**	Get the {@link nodes() node list} index of the last node in the list.

	@return	The nodes list index of the last node in the list. This
		is the same as one minus the size of the nodes list.
*/
inline int last_node_index () const
	{return Nodes.size () - 1;}

/**	Get the index of the lower bound node.

	The lower bound node immediately preceeds the first (lowest x index)
	node with a non-zero Data_Map entry (y) value, unless the first (lower
	anchor at index zero) node has a non-zero entry value in which case it
	is the lower bound node.

	@return	The {@link data_map() data map} index of the lower bound node.
	@see	lower_bound(int)
*/
int lower_bound () const;

/**	Set the data map x index location of the lower bound node.

	The current {@link lower_bound() lower bound} node is {@link
	move_node{const QPoint&, const QPoint&) moved} to the specified data
	map x index location with data map entry value zero. Any nodes
	between the lower anchor node and the new lower bound node location
	will be removed.

	Note that the lower bound node may be the lower anchor node and
	have a non-zero data map entry (y) value, but this method will
	set such a node to a zero entry value.

	@param	x	A data map x index location for the lower bound node.
	@return A node list change code. This will have the SHIFTED_NODE flag
		set if the lower anchor node entry value was changed or an
		existing node at the x index location was not the lower bound
		node. It will have the ADDED_NODE flag set if a new node was
		added at the specified x index location. The REMOVED_NODES_MASK
		portion of the code value will contain the number of nodes
		between the lower anchor node and the new lower bound node
		location that were removed.
	@see	lower_bound()
*/
int lower_bound (int x);

/**	Get the index of the upper bound node.

	The upper bound node immediately follows the last (highest x index)
	node with a non-zero Data_Map entry (y) value, unless the last (upper
	anchor at the (@link max_x() maximum index}) node has a non-zero
	entry value in which case it is the upper bound node.

	@return	The {@link data_map() data map} index of the upper bound node.
	@see	upper_bound(int)
*/
int upper_bound () const;

/**	Set the data map x index location of the upper bound node.

	The current {@link upper_bound() upper bound} node is {@link
	move_node{const QPoint&, const QPoint&) moved} to the specified
	data map x index location with the {@link max_y() maximum data map
	entry value}. Any nodes between the upper anchor node and the new
	upper bound node location will be removed.

	Note that the upper bound node may be the upper anchor node and
	have a non-maximum data map entry (y) value, but this method will
	set such a node to the maximum entry value.

	@param	x	A data map x index location for the upper bound node.
	@return A node list change code. This will have the SHIFTED_NODE flag
		set if the upper anchor node entry value was changed or an
		existing node at the x index location was not the upper bound
		node. It will have the ADDED_NODE flag set if a new node was
		added at the specified x index location. The REMOVED_NODES_MASK
		portion of the code value will contain the number of nodes
		between the upper anchor node and the new upper bound node
		location that were removed.
	@see	upper_bound()
*/
int upper_bound (int x);

/**	Print the list of {@link nodes() function node points}.

	@param	stream	The output stream on which to print the list.
*/
void print (std::ostream& stream) const;

/*==============================================================================
	Manipulators
*/
/**	Add a node at a specified function location.

	If there is any change to the function nodes list the data map is
	{@link interpolate{int) interpolated} around the new or changed node.

	@param	x	The Data_Map index of the node.
	@param	y	The Data_Map entry value of the node.
	@return	A node list change code. This will be zero if there was no
		change to the nodes list. It will be SHIFTED_NODE if an existing
		node had its Data_Map entry (y) value shifted to the new entry
		value. It will be ADDED_NODE if a new node was added to the list.
*/
int add_node (int x, int y);

/**	Add a node at a specified function point.

	If there is any change to the function nodes list the data map is
	{@link interpolate{int) interpolated} around the new or changed node.

	@param	node	A QPoint in which the x value is the Data_Map
		index of the node and the y value is the Data_Map entry value.
	@return	A node list change code. This will be zero if there was no
		change to the nodes list. It will be SHIFTED_NODE if an existing
		node had its Data_Map entry (y) value shifted to the new entry
		value. It will be ADDED_NODE if a new node was added to the list.
*/
inline bool add_node (const QPoint& node)
	{return add_node (node.x (), node. y ());}

/**	Move a node from one function position to another.

	Any existing nodes at the from point x index location up to, but not
	including the to point x index location are removed if the from and
	to point x index values are different. A node does not need to exist
	with the from point x index location for other nodes between this
	point and the to point location to be removed. Effectively, all nodes
	from the from point to the to point are swept away.

	If a node exists with the to point x index location its Data_Map
	entry (y) value is set to the to point y value. Otherwise a node is
	added with the to point values. Note that the from point y value is
	not used.

	@param	from	A QPoint specifying the staring point of the node
		move. A node need not exist at this point.
	@param	to	A QPoint specifying the ending point of the node
		move. A node need not exist at this point.
	@return A node list change code. This will have the SHIFTED_NODE flag
		if a node with the to point x index location had its Data_Map
		entry (y) value shifted to the new value. It will have the
		ADDED_NODE flag set if a new node was added for the to point
		position. The REMOVED_NODES_MASK portion of the code value will
		contain the number of nodes from the from point location up to,
		but not including, the to point location that were removed.
*/
int move_node (const QPoint& from, const QPoint& to);

/**	Remove the node at the specified Data_Map x index location.

	<b>N.B.</b>: Anchor nodes at x index zero and {@link max_x() maximum
	index} locations will not be removed. If either of these locations
	are specifed nothing is done.

	If a node is found with the specified Data_Map x index it is removed
	from the list and the data map is interpolated between the next lower
	and next upper nodes.

	@param	x	The Data_Map index for the node.
	@return true if an existing node was removed; false otherwise.
*/
bool remove_node (int x);

/**	Reset the {@link nodes() nodes list}.

	All existing nodes other than the anchor nodes are removed and the
	anchor nodes set to the minimum (zero) and {@link max_y() maximum}
	Data_Map entry values, respectively. The {@link data_map() data map}
	is then {@link interpolate() interpolated}.

	@return true if the nodes list and data map were changed; false
		otherwise.
*/
bool reset ();

/**	Interpolate all {@link data_map() data map} values from the
	{@link nodes() function nodes}.

	Beginning with the node following the lower anchor node, the data map
	values between this node and previous node are {@link
	linear_interpolation(const QPoint&, const QPoint&, Data_Map*)
	interpolated} between the Data_Map entry (y) values of the nodes.
	This is repeated for all the following nodes in the list.
*/
void interpolate ();

/**	Interpolate the {@link data_map() data map} values around a specific
	function node.

	If the specified function node is not the lower anchor, the data map
	values between the lower anchor node and the specified node are
	{@link linear_interpolation(const QPoint&, const QPoint&, Data_Map*)
	interpolated} between the Data_Map entry (y) values of the nodes.

	If the specified function node is not the upper anchor, the data map
	values between the upper anchor node and the specified node are
	{@link linear_interpolation(const QPoint&, const QPoint&, Data_Map*)
	interpolated} between the Data_Map entry (y) values of the nodes.

	@param	node_index	The index of a node in the {@link nodes()
		function nodes list}.
*/
void interpolate (int node_index);

/*==============================================================================
	QwtData implementation
*/

inline virtual void dataChanged() {}

/**	Pretends to provide a copy of this Function_Nodes object.

	@return	A pointer to this Function_Nodes object. No copy is made.
*/
virtual QwtSeriesStore* copy () const;

/**	Gets the number of nodes in the {@link nodes() functions nodes} list.

	@return	The number of nodes in the functions nodes list.
*/
virtual size_t size () const;

/**	The Data_Map x index value of a node.

	@param	node_index	A {@link nodes() functions nodes} list index.
	@return	The x index value of the node as a double type, or -1 if
		there is no such node.
*/
double x (size_t node_index) const;

/**	The Data_Map entry (y) value of a node.

	@param	node_index	A {@link nodes() functions nodes} list index.
	@return	The y value of the node as a double type, or -1 if there is
		no such node.
*/
double y (size_t node_index) const;

virtual QPointF sample(size_t index) const;

/**	The bounding rectangle of all function nodes.

	@return	A QwtDoubleRect (this is a typedef for a QRectF) rectangle in
		which the left side is 0, the top is the {@link max_y() maximum
		Data_Map entry value}, the width is the {@link max_x() maximum
		Data_Map index value}, and the height is the maximum Data_Map
		entry value. The is the area of the function graph that
		encompasses the entire data map.
*/
virtual QwtDoubleRect boundingRect () const;

/*==============================================================================
	Utilities
*/
/**	Apply a linear interpolation of values to a section of a Data_Map.

	The data map entries from x0 to x1, inclusive, are given values
	linearly interpolated between the y0 and y1 values (rounded to
	the nearest integer).

	If x0 > x1 their values are swapped as well as the y0 and y1 values.
	The x0 and x1 values are clipped to the range 0 - max_x, where max_x
	is the size of the data map minus one. The calculated interpolation values
	are cast to the Data_Map data type.

	@param	x0	The starting Data_Map index.
	@param	y0	The Data_Map entry value at index x0.
	@param	x1	The ending Data_Map index.
	@param	y1	The Data_Map entry value at index x1.
	@param	data_map	The Data_Map to receive the interpolated entry
		values.
	@see	linear_interpolation(const QPoint&, const QPoint&, Data_Map*)
*/
static void linear_interpolation
	(int x0, int y0, int x1, int y1, Data_Map* data_map);

/**	Apply a linear interpolation of values to a section of a Data_Map.

	@param	start	A QPoint specifying the starting Data_Map position
		where the x value of the point  is a Data_Map index value and the
		y value of the point is a Data_Map entry value.
	@param	end	A QPoint specifying the ending Data_Map position.
	@param	data_map	The Data_Map to receive the interpolated entry
		values.
	@see	linear_interpolation(int, int, int, int, Data_Map*)
*/
inline static void linear_interpolation
	(const QPoint& start, const QPoint& end, Data_Map* data_map)
	{linear_interpolation
		(start.x (), start.y (), end.x (), end.y (), data_map);}

/*==============================================================================
	Data
*/
private:

QPolygon
	Nodes;
Data_Map
	*Map;

};	//	Function_Nodes class.


}	//	namespace HiRISE
}	//	namespace UA
#endif
