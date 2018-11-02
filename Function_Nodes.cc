/*	Function_Nodes

HiROC CVS ID: $Id: Function_Nodes.cc,v 1.3 2012/03/09 02:13:55 castalia Exp $

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

#include	"Function_Nodes.hh"

#include	"HiView_Config.hh"
#include	"HiView_Utilities.hh"

#include	<iostream>
#include	<iomanip>
using std::endl;

#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define	DEBUG_MANIPULATORS		(1 << 2)
#define DEBUG_INTERPOLATORS		(1 << 3)

#define DEBUG_DEFAULT			DEBUG_CONSTRUCTORS

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	<iostream>
using std::clog;
using std::boolalpha;
using std::setw;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Function_Nodes::ID =
		"UA::HiRISE::Function_Nodes ($Revision: 1.3 $ $Date: 2012/03/09 02:13:55 $)";

/*==============================================================================
	Application configuration parameters
*/
#define MAX_DISPLAY_VALUE \
	HiView_Config::MAX_DISPLAY_VALUE

/*==============================================================================
	Constructor
*/
Function_Nodes::Function_Nodes
	(
	Data_Map*	data_map
	)
	:	QwtSeriesStore (),
		Map (data_map)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Function_Nodes @ " << (void*)this
		<< ": Data_Map @ " << (void*)Map << endl;
#endif
reset ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Function_Nodes" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
void
Function_Nodes::data_map
	(
	Data_Map*	data_map
	)
{
if (data_map)
	{
	Map = data_map;
	reset ();
	}
}


int
Function_Nodes::max_y () const
{return MAX_DISPLAY_VALUE;}


int
Function_Nodes::node_index_of
	(
	int		x
	)
	const
{
int
	index = Nodes.size ();
if (index &&
	x >= 0 &&
	x <= max_x ())
	while (index-- &&
		Nodes[index].x () != x) ;
else
	index = -1;
return index;
}


int
Function_Nodes::lower_bound () const
{
int
	index = -1,
	total_nodes = Nodes.size ();
while (++index < total_nodes &&
		Nodes[index].y () == 0) ;
if (index)
	index--;
return Nodes[index].x ();
}


int
Function_Nodes::lower_bound
	(
	int		x
	)
{
if (x < 0)
	x = 0;
else
if (x > max_x ())
	x = max_x ();

//	Find the index of the current lower bound node.
int
	changed = 0,
	index = -1,
	total_nodes = Nodes.size ();
while (++index < total_nodes &&
		Nodes[index].ry () == 0) ;
if (index)
	index--;
else
	{
	//	Move low end anchor to zero.
	Nodes[index].ry () = 0;
	changed = SHIFTED_NODE;
	}

if (Nodes[index].rx () != x)
	changed |= move_node (Nodes[index], QPoint (x, 0));
return changed;
}


int
Function_Nodes::upper_bound () const
{
int
	index = Nodes.size ();
while (index-- &&
		Nodes[index].y () == max_y ()) ;
if (index < last_node_index ())
	index++;
return Nodes[index].x ();
}


int
Function_Nodes::upper_bound
	(
	int		x
	)
{
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << ">>> Function_Nodes::upper_bound: " << x << endl;
#endif
if (x < 0)
	x = 0;
else
if (x > max_x ())
	x = max_x ();

int
	changed = 0,
	index = Nodes.size ();
while (index-- &&
		Nodes[index].ry () == max_y ()) ;
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << "    selected upper bound node index = " << index << endl
	 << "                    last_node_index = " << last_node_index () << endl;
#endif
if (index < last_node_index ())
	index++;
else
	{
	//	Move high end anchor to max_y.
	Nodes[index].ry () = max_y ();
	changed = SHIFTED_NODE;
	#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
	clog << "    repositioned anchor node[" << index << "] = "
			<< Nodes[index] << endl;
	#endif
	}

if (Nodes[index].rx () != x)
	changed |= move_node (Nodes[index], QPoint (x, max_y ()));
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << "<<< Function_Nodes::upper_bound: " << changed
		<< " (removed " << (changed & REMOVED_NODES_MASK)
		<< ", added " << ((changed & ADDED_NODE) ? '1' : '0')
		<< ", shifted " << ((changed & SHIFTED_NODE) ? '1' : '0')
		<< ')' <<endl;
#endif
return changed;
}


void
Function_Nodes::print
	(
	std::ostream&	stream
	)
	const
{
for (int
		index = 0;
		index < Nodes.size ();
		index++)
	stream << "        node " << index << " = " << Nodes.at (index) << endl;
}

/*==============================================================================
	Manipulators
*/
int
Function_Nodes::add_node
	(
	int		x,
	int		y
	)
{
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << ">>> Function_Nodes::add_node: " << x << "x, " << y << 'y' << endl;
#endif
int
	changed = 0;
if (x >= 0 &&
	x <= max_x () &&
	y >= 0 &&
	y <= max_y ())
	{	
	int
		index = Nodes.size ();
	while (index--)
		{
		if (Nodes[index].rx () > x)
			continue;

		if (Nodes[index].rx () == x)
			{
			#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
			clog << "    replace node " << index
					<< " = " << Nodes[index] << endl;
			#endif
			if (Nodes[index].ry () != y)
				{
				Nodes[index].ry () = y;
				changed = SHIFTED_NODE;
				}
			}
		else
			{
			#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
			clog << "    insert node " << index << endl;
			#endif
			Nodes.insert (index, QPoint (x, y));
			changed = ADDED_NODE;
			}
		break;
		}
	if (changed)
		//	Interpolate around the added node.
		interpolate (index);
	}
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << "<<< Function_Nodes::add_node: " << changed << " ("
		<< ((changed & ADDED_NODE) ? "added" :
		   ((changed & SHIFTED_NODE) ? "shifted" : "no change")) << ')' <<endl;
#endif
return changed;
}


int
Function_Nodes::move_node
	(
	const QPoint&	from,
	const QPoint&	to
	)
{
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << ">>> Function_Nodes::move_node: from " << from << " to " << to << endl;
#endif
int
	changed = 0;
int
	x0 = from.x (),
	x1 = to.x (),
	y1 = to.y ();
if (x0 >= 0 &&
	x0 <= max_x () &&
	x1 >= 0 &&
	x1 <= max_x () &&
	y1 >= 0 &&
	y1 <= max_y ())
	{
	int
		index = -1,
		total_nodes = Nodes.size ();
	#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
	clog << "    total_nodes = " << total_nodes << endl;
	#endif

	//	Find the node at or above the from node.
	while (++index < total_nodes &&
			Nodes[index].rx () < x0) ;

	if (Nodes[index].rx () == x0)
		{
		//	Move from existing node.
		#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
		clog << "      existing from node[" << index << "] = "
				<< Nodes[index] << endl;
		#endif
		if (x0 == x1)
			{
			//	The node does not move horizontally.
			if (Nodes[index].ry () != y1)
				{
				//	The node only moves vertically.
				Nodes[index].ry () = y1;
				changed = SHIFTED_NODE;
				#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
				clog << "              +from node[" << index << "] = "
						<< Nodes[index] << endl;
				#endif
				}
			goto Update_Data_Map;
			}
		else
		if (index &&
			index < last_node_index ())
			{
			#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
			clog << "            -remove node[" << index << "] = "
					<< Nodes[index] << endl;
			#endif
			Nodes.remove (index);
			++changed;
			}
		}
	#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
	else
		clog << "      next index = " << index << endl;
	#endif

	//	Remove all intervening nodes.
	if (x0 <= x1)
		{
		/*	>>> CAUTION <<<
			Nodes may be removed from the list during a bottom-up
			traversal. When a node is removed the next node up becomes
			the node at the index of the node that was removed, so in
			this case the traversal index does not increment; however, it
			does increment if no removal occurs.
		*/
		while (index < Nodes.size () &&
				Nodes[index].rx () < x1)
			{
			if (index &&
				index < last_node_index ())
				{
				#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
				clog << "            -remove node[" << index << "] = "
						<< Nodes[index] << endl;
				#endif
				Nodes.remove (index);
				++changed;
				}
			else
				++index;
			}
		}
	else
		{
		while (index-- &&
				Nodes[index].rx () > x1)
			{
			if (index &&
				index < last_node_index ())
				{
				#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
				clog << "            -remove node[" << index << "] = "
						<< Nodes[index] << endl;
				#endif
				Nodes.remove (index);
				++changed;
				}
			}
		}

	if (Nodes[index].rx () == x1)
		{
		//	Move to existing node.
		#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
		clog << "        existing to node[" << index << "] = "
				<< Nodes[index] << endl;
		#endif
		if (Nodes[index].ry () != y1)
			{
			Nodes[index].ry () = y1;
			changed |= SHIFTED_NODE;
			#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
			clog << "                +to node[" << index << "] = "
					<< Nodes[index] << endl;
			#endif
			}
		}
	else
		{
		//	Insert new node.
		if (x0 > x1)
			++index;
		Nodes.insert (index, to);
		changed |= ADDED_NODE;
		#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
		clog << "               +add node[" << index << "] = "
				<< Nodes[index] << endl;
		#endif
		}

	Update_Data_Map:
	#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
	clog << "    total_nodes = " << total_nodes << endl;
	#endif
	if (changed)
		{
		#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
		clog << "    interpolate around node[" << index << "] = "
				<< Nodes[index] << endl;
		#endif
		interpolate (index);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_MANIPULATORS)
clog << "<<< Function_Nodes::move_node: " << changed
		<< " (removed " << (changed & REMOVED_NODES_MASK)
		<< ", added " << ((changed & ADDED_NODE) ? '1' : '0')
		<< ", shifted " << ((changed & SHIFTED_NODE) ? '1' : '0')
		<< ')' <<endl;
#endif
return changed;
}


bool
Function_Nodes::remove_node
	(
	int		x
	)
{
bool
	changed = false;
int
	index = node_index_of (x);
//	Don't remove node end points.
if (index > 0 &&
	index < max_x ())
	{
	changed = true;

	//	Remap between the bounding nodes.
	linear_interpolation (Nodes[index - 1], Nodes[index + 1], Map);

	//	Remove the specified node.
	Nodes.remove (index);
	}
return changed;
}


bool
Function_Nodes::reset ()
{
QPoint
	lower_anchor,						//	Low end anchor.
	upper_anchor (max_x (), max_y ());	//	High end anchor.
if (Nodes.size () != 2 ||
	Nodes.at (0) != lower_anchor ||
	Nodes.at (1) != upper_anchor)
	{
	Nodes.clear ();
	Nodes
		<< lower_anchor
		<< upper_anchor;	
	interpolate ();
	return true;
	}
return false;
}


void
Function_Nodes::interpolate
	(
	int		node_index
	)
{
if (node_index)
	linear_interpolation (Nodes[node_index - 1], Nodes[node_index], Map);
if (node_index < last_node_index ())
	linear_interpolation (Nodes[node_index], Nodes[node_index + 1], Map);
}


void
Function_Nodes::interpolate ()
{
int
	index = 0,
	total_nodes = Nodes.size ();
while (++index < total_nodes)
	linear_interpolation (Nodes[index -1], Nodes[index], Map);
}

/*==============================================================================
	QwtData implementation
*/
//	N.B.: No copying; just return self.
QwtSeriesStore<QPointF>*
Function_Nodes::copy () const
{return const_cast<Function_Nodes*>(this);}


size_t
Function_Nodes::size () const
{return Nodes.size ();}


double
Function_Nodes::x
	(
	size_t	node_index
	) const
{
if (node_index < (size_t)Nodes.size ())
	return Nodes.at ((int)node_index).x ();
return -1;
}


double
Function_Nodes::y
	(
	size_t	node_index
	) const
{
if (node_index < (size_t)Nodes.size ())
	return Nodes.at ((int)node_index).y ();
return -1;
}

QPointF Function_Nodes::sample (size_t node_index) const
{
   return Nodes.at((int)node_index);
}

QwtDoubleRect
Function_Nodes::boundingRect () const
{
return QwtDoubleRect
	(
	0,			//	Left.
	max_y (),	//	Top.
	max_x (),	//	Width.
	max_y ()	//	Height.
	);
}

/*==============================================================================
	Utilities
*/
void
Function_Nodes::linear_interpolation
	(
	int			x0,
	int			y0,
	int			x1,
	int			y1,
	Data_Map*	data_map
	)
{
#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
clog << ">>> Function_Nodes::linear_interpolation: from "
		<< x0 << "x, " << y0 << "y to " << x1 << "x, " << y1 << "y"
		" for Data_Map @ " << (void*)data_map  << endl;
#endif
if (data_map &&
	x0 != x1)
	{
	if (x0 > x1)
		{
		//	Must be monotonically increasing from x0 to x1.
		int
			value;
		value = x0;
		x0 = x1;
		x1 = value;
		value = y0;
		y0 = y1;
		y1 = value;
		}

	//	Slope of the line.
	double
		slope = double (y1 - y0) / (x1 - x0);
	#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
	clog << "    slope = " << slope << endl;
	#endif

	//	Clip to the data range.
	int
		x = data_map->size () - 1;
	if (x0 < 0)
		x0 = 0;
	else
	if (x0 > x)
		x0 = x;
	if (x1 < 0)
		x1 = 0;
	else
	if (x1 > x)
		x1 = x;

	#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
	clog << "    data range " << x0 << " - " << x1 << endl;
	#endif
	if (x0 != x1)
		{
		Data_Map::value_type
			datum;
		#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
		clog << "    interpolated values - " << endl
			 << "    " << setw (4) << x0 << ':';
		#endif
		for (x  = x0;
			 x <= x1;
		   ++x)
			{
			datum = static_cast<Data_Map::value_type>
					(y0 + ((x - x0) * slope) + 0.5);
			(*data_map)[x] = datum;
			#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
			clog << ' ' << setw (3) << ((int)datum & 0xFF);
			if (x &&
				! ((x + 1) % 16))
				{
				clog << endl;
				if (x != x1)
					clog << "    " << setw (4) << (x + 1) << ':';
				}
			#endif
			}
		#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
		if (x1 % 16)
			clog << endl;
		#endif
		}
	}
#if ((DEBUG_SECTION) & DEBUG_INTERPOLATORS)
clog << "<<< Function_Nodes::linear_interpolation" << endl;
#endif
}


}	//	namespace HiRISE
}	//	namespace UA
