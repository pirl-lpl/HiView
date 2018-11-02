/*	Coordinate

HiROC CVS ID: $Id: Coordinate.cc,v 1.1 2012/09/16 07:50:51 castalia Exp $

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

#include	"Coordinate.hh"

#include	<QString>

#include	<string>
using std::string;
#include	<ostream>
using std::ostream;
#include	<sstream>
using std::ostringstream;
#include	<stdexcept>
using std::invalid_argument;
#include	<iomanip>
using std::endl;


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Coordinate::ID =
		"UA::HiRISE::Coordinate ($Revision: 1.1 $ $Date: 2012/09/16 07:50:51 $)";

/*==============================================================================
	Constructors
*/
Coordinate::Coordinate ()
	:	X (0.0),
		Y (0.0)
{}


Coordinate::Coordinate
	(
	double	x,
	double	y
	)
	:	X (x),
		Y (y)
{}


Coordinate::Coordinate
	(
	const Coordinate&	coordinate
	)
	:	X (coordinate.X),
		Y (coordinate.Y)
{}


Coordinate::Coordinate
	(
	const QString&	coordinate
	)
	:	X (0.0),
		Y (0.0)
{
double
	x (0.0), y (0.0);
bool
	OK (false);
QString
	value;
int
	index,
	separator_index = coordinate.indexOf (',');
if (separator_index > 0)
	{
	value = coordinate.left (separator_index);
	if ((index = value.indexOf ('x', Qt::CaseInsensitive) >= 0))
		value = value.left (index);
	value = value.trimmed ();
	x = value.toDouble (&OK);
	if (OK)
		{
		value = coordinate.right (coordinate.size () - separator_index - 1);
		if ((index = value.indexOf ('y', Qt::CaseInsensitive) >= 0))
			value = value.left (index);
		value = value.trimmed ();
		y = value.toDouble (&OK);
		}
	}
if (OK)
	{
	X = x;
	Y = y;
	}
else
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Not a valid coordinate: " << qPrintable (coordinate);
	throw invalid_argument (message.str ());
	}
}


Coordinate&
Coordinate::operator=
	(
	const Coordinate&	coordinate
	)
{
if (&coordinate != this)
	{
	X = coordinate.X;
	Y = coordinate.Y;
	}
return *this;
}

/*==============================================================================
	Utilities
*/
std::ostream&
operator<<
	(
	std::ostream&		stream,
	const Coordinate&	coordinate
	)
{
return stream
	<< coordinate.X << "x, "
	<< coordinate.Y << 'y';
}


}	//	namespace HiRISE
}	//	namespace UA
