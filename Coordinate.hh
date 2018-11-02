/*	Coordinate

HiROC CVS ID: $Id: Coordinate.hh,v 1.2 2012/09/18 01:06:18 castalia Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*******************************************************************************/

#ifndef UA_HiRISE_Coordinate_hh
#define UA_HiRISE_Coordinate_hh

class QString;

#include	<iosfwd>


namespace UA
{
namespace HiRISE
{
/**	A <i>Coordinate</i> holds the X,Y values of a location in a
	two-dimensional coordinate system.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.2 $
*/
class Coordinate
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Constructors
*/
//!	Constructs a default Coordiante with zero values.
Coordinate ();

/**	Constructs a Coordinate with the specified values.

	@param	x	The X value.
	@param	y	The Y value.
*/
Coordinate (double x, double y);

/**	Constructs a coordinate from a string representation.

	The string representation of a Coordinate has the form:

	X[x],Y[y]

	where X and Y are the coordinate values with optional 'x' and 'y'
	annotations. The comma (',') separator is required. Whitespace is
	ignored.

	@param	coordinate	A string representation of a coordinate.
	@throws	invalid_argument	If the string is not a valid coordinate
		representation. In this case the coordinate will be left with
		zero values.
*/
explicit Coordinate (const QString& coordinate);

/**	Copies a Coordinate.

	@param	coordinate	The Coordinate to be copied.
*/
Coordinate (const Coordinate& coordinate);

/**	Assigns another Coordinate to this Coordinate.

	@param	coordinate	The Coordinate to be assigned.
*/
Coordinate& operator= (const Coordinate& coordinate);

/*==============================================================================
	Data
*/
//!	Coordinate values.
double
	X, Y;

};	//	End of Coordinate class.

/*==============================================================================
	Utilities
*/
/**	Coordinate output operator.

	The coordinate text representation has the form:

	Xx, Yy

	where X and Y are the coordinate values and 'x' and 'y' are
	value annotations.

	@param	stream	A std::ostream reference where the represetation
		will be written.
	@param	coordinate	A Coordinate reference.
	@return	The stream reference.
*/
std::ostream& operator<< (std::ostream& stream, const Coordinate& coordinate);


}	//	namespace HiRISE
}	//	namespace UA
#endif

