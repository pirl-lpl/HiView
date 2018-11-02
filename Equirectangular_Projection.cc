/*	Equirectangular_Projection

HiROC CVS ID: $Id: Equirectangular_Projection.cc,v 1.8 2012/09/27 22:24:56 castalia Exp $

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

#include	"Equirectangular_Projection.hh"

#include	"PDS_Metadata.hh"
#include	"Coordinate.hh"
#include	"HiView_Utilities.hh"

#include	"PVL.hh"
using idaeim::PVL::Aggregate;
using idaeim::Out_of_Range;

#include	<QString>
#include	<QtCore/qmath.h>

#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
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
#define	DEBUG_CONVERTERS		(1 << 3)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
using std::showpoint;
#endif

#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Equirectangular_Projection::ID
		= "UA::HiRISE::Equirectangular_Projection"
			" ($Revision: 1.8 $ $Date: 2012/09/27 22:24:56 $)";


const char* const
	Equirectangular_Projection::CANONICAL_PROJECTION_NAME
		= "Equirectangular";

const char* const
	Equirectangular_Projection::PROJECTION_NAMES[] =
		{
		"EQUIRECTANGULAR",
		"EQUIRECTANGULAR_CYLINDRICAL",
		"EQUIDISTANT",
		"EQUIDISTANT_CYLINDRICAL",
		"SIMPLE_CYLINDRICAL",
		NULL
		};

const char* const
	Equirectangular_Projection::REQUIRED_PARAMETERS[] =
		{
		Projection::pixel_size_parameter (),
		Projection::horizontal_offset_parameter (),
		Projection::vertical_offset_parameter (),
		Projection::equatorial_radius_parameter (),
		Projection::polar_radius_parameter (),
		Projection::center_longitude_parameter (),
		Projection::center_latitude_parameter (),
		NULL
		};

const char* const
	Equirectangular_Projection::OPTIONAL_PARAMETERS[] =
		{
		positive_longitude_parameter (),
		latitude_type_parameter (),
		NULL
		};

/*==============================================================================
	Constructors
*/
Equirectangular_Projection::Equirectangular_Projection
	(
	const idaeim::PVL::Aggregate*	params
	)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Equirectangular_Projection: parameters @ "
		<< (void*)params << endl;
#endif
parameters (params, ID);

if (Not_Identity)
	{
	if (center_latitude () == PI_OVER_2)
		{
		reset ();

		ostringstream
			message;
		message
			<< "The " << CANONICAL_PROJECTION_NAME
				<< " projection can't be done" << endl
			<< "when the " << center_latitude_parameter ()
				<< " parameter value is "
				<< to_degrees (Center_Latitude) << " degrees.";
		throw Out_of_Range (message.str (), ID);
		}
	Local_Radius = local_radius (center_latitude ());
	Local_Radius_Coefficient = Local_Radius * qCos (center_latitude ());
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Equirectangular_Projection" << endl;
#endif
}


Equirectangular_Projection::Equirectangular_Projection
	(
	const Equirectangular_Projection&	projection
	)
	:	Projection (projection),
		Local_Radius (projection.Local_Radius),
		Local_Radius_Coefficient (projection.Local_Radius_Coefficient)
{}


Equirectangular_Projection&
Equirectangular_Projection::operator=
	(
	const Equirectangular_Projection&	projection
	)
{
if (this != &projection)
	{
	Projection::operator= (*this);
	Local_Radius = projection.Local_Radius;
	Local_Radius_Coefficient = projection.Local_Radius_Coefficient;
	}
return *this;
}


Equirectangular_Projection*
Equirectangular_Projection::clone () const
{return new Equirectangular_Projection (*this);}


Equirectangular_Projection::~Equirectangular_Projection ()
{}

/*==============================================================================
	Accessors
*/
const char*
Equirectangular_Projection::canonical_projection_name () const
{return CANONICAL_PROJECTION_NAME;}

const char* const*
Equirectangular_Projection::projection_names ()
{return PROJECTION_NAMES;}

const char* const*
Equirectangular_Projection::required_parameters () const
{return REQUIRED_PARAMETERS;}

const char* const*
Equirectangular_Projection::optional_parameters () const
{return OPTIONAL_PARAMETERS;}

/*==============================================================================
	Converters
*/
Coordinate
Equirectangular_Projection::to_world
	(
	const Coordinate& image_coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << ">>> Equirectangular_Projection.to_world: " << image_coordinate << endl;
#endif
Coordinate
	world_coordinate (image_coordinate);
if (Not_Identity)
	{
	rotate_from_image (world_coordinate);

	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "    longitude -" << endl
		 << "      X = center_longitude + to_world_X_offset ("
			<< image_coordinate.X << ") / Local_Radius_Coefficient" << endl
		 << "        center_longitude = " << center_longitude () << endl
		 << "        to_world_X_offset (Xi) = "
		 	<< to_world_X_offset (image_coordinate.X) << endl
		 << "        Local_Radius_Coefficient = "
		 	<< Local_Radius_Coefficient << endl;
	#endif
	world_coordinate.X =
		center_longitude ()
		+ to_world_X_offset (image_coordinate.X) / Local_Radius_Coefficient;
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "      X = " << world_coordinate.X << " East rad." << endl;
	#endif
	world_coordinate.X = to_360 (to_degrees (world_coordinate.X));
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "          " << world_coordinate.X << " deg. 360" << endl;
	#endif

	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "    latitude -" << endl
		 << "      Y = to_world_Y_offset ("
			<< image_coordinate.Y << ") / Local_Radius" << endl
		 << "        to_world_Y_offset (Yi) = "
		 	<< to_world_Y_offset (image_coordinate.Y) << endl
		 << "        Local_Radius = " << Local_Radius << endl;
	#endif
	world_coordinate.Y =
		to_world_Y_offset (image_coordinate.Y) / Local_Radius;
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "      Y = " << world_coordinate.Y
			<< " rad. planetocentric" << endl
		 << "          " << to_degrees (world_coordinate.Y)
			<< " deg." << endl;
	clog << "          "
			<< planetocentric_to_planetographic (world_coordinate.Y)
			<< " rad. planetographic" << endl
		 << "          "
		 	<< to_degrees (planetocentric_to_planetographic (world_coordinate.Y))
			<< " deg." << endl;
	#endif
	world_coordinate.Y = to_degrees (world_coordinate.Y);
	}
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << "<<< Equirectangular_Projection.to_world: " << world_coordinate << endl;
#endif
return world_coordinate;
}


Coordinate 
Equirectangular_Projection::to_image
	(
	const Coordinate& world_coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << ">>> Equirectangular_Projection.to_image: " << world_coordinate << endl;
#endif
Coordinate
	image_coordinate (world_coordinate);
if (Not_Identity)
	{
	image_coordinate.X =
		(int)(to_image_X_offset (Local_Radius_Coefficient
			* (to_radians (to_360 (world_coordinate.X))
				- center_longitude ()))
			+ 0.5);		//	Round to nearest pixel.
	image_coordinate.Y =
		(int)(to_image_Y_offset (Local_Radius
			* to_radians (world_coordinate.Y))
			+ 0.5);		//	Round to nearest pixel.

	rotate_to_image (image_coordinate);
	}
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << "<<< Equirectangular_Projection.to_image: " << image_coordinate << endl;
#endif
return image_coordinate;
}


}	//	namespace HiRISE
}	//	namespace UA
