/*	Polar_Stereographic_Elliptical_Projection

HiROC CVS ID: $Id: Polar_Stereographic_Elliptical_Projection.cc,v 1.7 2012/09/27 22:31:39 castalia Exp $

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

#include	"Polar_Stereographic_Elliptical_Projection.hh"

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
#include	<stdexcept>
using std::out_of_range;


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
	Polar_Stereographic_Elliptical_Projection::ID
		= "UA::HiRISE::Polar_Stereographic_Elliptical_Projection"
			" ($Revision: 1.7 $ $Date: 2012/09/27 22:31:39 $)";


const char* const
	Polar_Stereographic_Elliptical_Projection::CANONICAL_PROJECTION_NAME
		= "Polar Stereographic Elliptical";

const char* const
	Polar_Stereographic_Elliptical_Projection::PROJECTION_NAMES[] =
		{
		"POLAR STEREOGRAPHIC",
		"POLAR_STEREOGRAPHIC",
		"POLARSTEREOGRAPHIC",
		NULL
		};

const char* const
	Polar_Stereographic_Elliptical_Projection::REQUIRED_PARAMETERS[] =
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
	Polar_Stereographic_Elliptical_Projection::OPTIONAL_PARAMETERS[] =
		{
		positive_longitude_parameter (),
		latitude_type_parameter (),
		NULL
		};

/*==============================================================================
	Constructors
*/
Polar_Stereographic_Elliptical_Projection::
	Polar_Stereographic_Elliptical_Projection
	(
	const idaeim::PVL::Aggregate*	params
	)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Polar_Stereographic_Elliptical_Projection" << endl;
#endif
parameters (params, ID);

if (Not_Identity)
	{
	//	Precalculated coefficients.

	E_over_2 = eccentricity () * 0.5;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    E_over_2 = " << E_over_2 << endl;
	#endif

	if (center_latitude () == 0.0)
		{
		reset ();

		ostringstream
			message;
		message
			<< "The " << CANONICAL_PROJECTION_NAME
				<< " projection can't be done"
			<< "when the " << center_latitude_parameter ()
				<< " parameter value is 0.";
		throw Out_of_Range (message.str (), ID);
		}
	if (center_latitude () < 0.0)
		Center_Latitude_Sign = -1.0;
	else
		Center_Latitude_Sign = 1.0;
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Center_Latitude_Sign = " << Center_Latitude_Sign << endl;
	#endif

	bool
		at_pole = true;
	double
		coefficient_t = 0.0;
	if ((PI_OVER_2 - qAbs (center_latitude ())) > DBL_EPSILON)
		{
		coefficient_t = coefficient_T (Center_Latitude_Sign * center_latitude ());
		if (qAbs (coefficient_t) > DBL_EPSILON)
			at_pole = false;
		}
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    at_pole = " << at_pole << endl
		 << "      coefficient_t = " << coefficient_t << endl;
	#endif

	if (at_pole)
		Distance_Coefficient =
			qSqrt
				(qPow (1.0 + eccentricity (), 1.0 + eccentricity ()) *
				 qPow (1.0 - eccentricity (), 1.0 - eccentricity ()))
			/ (2.0 * Equitorial_Radius);
	else
		{
		double
			phi = Center_Latitude_Sign * center_latitude (),
			Esinphi = eccentricity () * qSin (phi);
		Distance_Coefficient = coefficient_t /
			(Equitorial_Radius * qCos (phi) / qSqrt (1.0 - Esinphi * Esinphi));
		}
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Distance_Coefficient = " << Distance_Coefficient << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Polar_Stereographic_Elliptical_Projection" << endl;
#endif
}


Polar_Stereographic_Elliptical_Projection::
	Polar_Stereographic_Elliptical_Projection
	(
	const Polar_Stereographic_Elliptical_Projection&	projection
	)
	:	Projection (projection),
		E_over_2 (projection.E_over_2),
		Center_Latitude_Sign (projection.Center_Latitude_Sign),
		Distance_Coefficient (projection.Distance_Coefficient)
{}


Polar_Stereographic_Elliptical_Projection&
Polar_Stereographic_Elliptical_Projection::operator=
	(
	const Polar_Stereographic_Elliptical_Projection&	projection
	)
{
if (this != &projection)
	{
	Projection::operator= (*this);
	E_over_2 = projection.E_over_2;
	Center_Latitude_Sign = projection.Center_Latitude_Sign;
	Distance_Coefficient = projection.Distance_Coefficient;
	}
return *this;
}


Polar_Stereographic_Elliptical_Projection*
Polar_Stereographic_Elliptical_Projection::clone () const
{return new Polar_Stereographic_Elliptical_Projection (*this);}


Polar_Stereographic_Elliptical_Projection::
	~Polar_Stereographic_Elliptical_Projection ()
{}

/*==============================================================================
	Accessors
*/
const char*
Polar_Stereographic_Elliptical_Projection::canonical_projection_name () const
{return CANONICAL_PROJECTION_NAME;}

const char* const*
Polar_Stereographic_Elliptical_Projection::projection_names ()
{return PROJECTION_NAMES;}

const char* const*
Polar_Stereographic_Elliptical_Projection::required_parameters () const
{return REQUIRED_PARAMETERS;}

const char* const*
Polar_Stereographic_Elliptical_Projection::optional_parameters () const
{return OPTIONAL_PARAMETERS;}

/*==============================================================================
	Converters
*/
Coordinate
Polar_Stereographic_Elliptical_Projection::to_world
	(
	const Coordinate& image_coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << ">>> Polar_Stereographic_Elliptical_Projection::to_world: "
		<< image_coordinate << endl;
#endif
Coordinate
	world_coordinate (image_coordinate);
if (Not_Identity)
	{
	rotate_from_image (world_coordinate);

	double
		x = Center_Latitude_Sign
			* to_world_X_offset (image_coordinate.X),
		y = Center_Latitude_Sign
			* to_world_Y_offset (image_coordinate.Y),
		distance = qSqrt ((x * x) + (y * y)),
		T = distance * Distance_Coefficient;
		#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
		clog << "      x = Center_Latitude_Sign * to_world_X_offset("
				<< image_coordinate.X << ") = "
				<< Center_Latitude_Sign << " * "
				<< to_world_X_offset (image_coordinate.X) << endl
			 << "      y = Center_Latitude_Sign * to_world_Y_offset("
				<< image_coordinate.Y << ") = "
				<< Center_Latitude_Sign << " * "
				<< to_world_Y_offset (image_coordinate.Y) << endl
			 << "      distance = sqrt((x * x) + (y * y)) = "
			 	<< distance << endl
			 << "      T = distance * Distance_Coefficient = "
			 	<< T << endl;
		#endif

	//	Longitude.
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "    longitude -" << endl;
	#endif
	world_coordinate.X = ((distance == 0.0) ?
		(Center_Latitude_Sign * center_longitude ()) :
		(Center_Latitude_Sign * qAtan2 (x, -y) + center_longitude ()));
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "      X = " << world_coordinate.X << " East rad." << endl;
	#endif
	world_coordinate.X = to_360 (to_degrees (world_coordinate.X));
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "          " << world_coordinate.X << " deg. 360" << endl;
	#endif

	//	Latitude.
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "    latitude -" << endl;
	#endif
	try {world_coordinate.Y = Center_Latitude_Sign * coefficient_Phi2 (T);}
	catch (out_of_range& except)
		{
		world_coordinate.X =
		world_coordinate.Y = INVALID_VALUE;

		ostringstream
			message;
		message
			<< except.what () << endl
			<< "Unable to convert image coordinates "
				<< image_coordinate << " to world coordinates.";
		throw out_of_range (message.str ());
		}
	if (qAbs (world_coordinate.Y) > PI_OVER_2)
		{
		world_coordinate.X =
		world_coordinate.Y = INVALID_VALUE;

		ostringstream
			message;
		message
			<< ID << endl
			<< "The " << CANONICAL_PROJECTION_NAME
				<< " projection of image coordinates "
				<< image_coordinate << " to world coordinates" << endl
			<< "resulted in an invalid latitude of "
				<< to_degrees (world_coordinate.Y) << " degrees.";
		throw out_of_range (message.str ());
		}
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "      Y = " << world_coordinate.Y<< " rad. planetographic" << endl
		 << "          " << to_degrees (world_coordinate.Y) << " deg." << endl
		 << "          "
		 	<< planetographic_to_planetocentric (world_coordinate.Y)
			<< " rad. planetocentric" << endl
		 << "          "
		 	<< to_degrees (planetographic_to_planetocentric (world_coordinate.Y))
			<< " deg." << endl;
	#endif
	world_coordinate.Y = to_degrees
		(planetographic_to_planetocentric (world_coordinate.Y));
	}
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << "<<< Polar_Stereographic_Elliptical_Projection::to_world: "
		<< world_coordinate << endl;
#endif
return world_coordinate;
}


Coordinate 
Polar_Stereographic_Elliptical_Projection::to_image
	(
	const Coordinate& world_coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << ">>> Polar_Stereographic_Elliptical_Projection::to_image: "
		<< world_coordinate << endl;
#endif
Coordinate
	image_coordinate (world_coordinate);
if (Not_Identity)
	{
	image_coordinate.X = to_radians (to_360
		(world_coordinate.X)),
	image_coordinate.Y = planetocentric_to_planetographic (to_radians
		(world_coordinate.Y));
	double
		L = Center_Latitude_Sign * (image_coordinate.X - center_longitude ()),
		T = coefficient_T (image_coordinate.Y) / Distance_Coefficient;

	image_coordinate.X =
		(int)(to_image_X_offset (
			 Center_Latitude_Sign * T * qSin (L))
			+ 0.5);		//	Round to nearest pixel.
	image_coordinate.Y =
		(int)(to_image_Y_offset (
			-Center_Latitude_Sign * T * qCos (L))
			+ 0.5);		//	Round to nearest pixel.

	rotate_to_image (image_coordinate);
	}
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << "<<< Polar_Stereographic_Elliptical_Projection::to_image: "
		<< image_coordinate << endl;
#endif
return image_coordinate;
}

/*==============================================================================
	Derived values
*/
double
Polar_Stereographic_Elliptical_Projection::coefficient_T
	(
	double	latitude
	) const
{
double
	coef = 0.0;
if ((PI_OVER_2 - qAbs (latitude)) > DBL_EPSILON)
	{
	coef = eccentricity () * qSin (latitude);
	coef =
		qTan (0.5 * (PI_OVER_2 - latitude)) /
		qPow ((1.0 - coef) / (1.0 + coef), E_over_2);
	}
return coef;
}


double
Polar_Stereographic_Elliptical_Projection::coefficient_Phi2
	(
	double	T
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << ">>> Polar_Stereographic_Elliptical_Projection::coefficient_Phi2: "
		<< T << endl;
#endif
double
	phi = PI_OVER_2 - 2.0 * qAtan (T),
	new_phi,
	eccentricity_sine_phi,
	difference = 1.0;
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << "      0: " << phi << endl;
#endif
int
	iteration = 0;
while (iteration < 15 &&
		difference > 0.0000000001)
	{
	eccentricity_sine_phi = eccentricity () * qSin (phi);
	new_phi =
		PI_OVER_2 - 2.0
		* qAtan (T
			* qPow
				(
				(1.0 - eccentricity_sine_phi) /
				(1.0 + eccentricity_sine_phi),
				E_over_2
				));
	difference = qAbs (new_phi - phi);
	phi = new_phi;
	++iteration;
	#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
	clog << "      " << iteration << ": " << phi
			<< " (dif = " << difference << ')' << endl;
	#endif
	}
if (iteration == 15)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "The " << CANONICAL_PROJECTION_NAME
			<< " projection computation of coefficient phi2" << endl
		<< "failed to converge using coefficient T of " << T << '.';
	throw out_of_range (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_CONVERTERS)
clog << "    iteration = " << iteration << endl
	 << "<<< Polar_Stereographic_Elliptical_Projection::coefficient_Phi2: "
		<< phi << endl;
#endif
return phi;
}


}	//	namespace HiRISE
}	//	namespace UA
