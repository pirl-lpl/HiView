/*	Projection

HiROC CVS ID: $Id: Projection.cc,v 1.13 2014/11/04 21:01:33 guym Exp $

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

#include	"Projection.hh"

//	Projection subclasses that can be created. 
#include	"Equirectangular_Projection.hh"
#include	"Polar_Stereographic_Elliptical_Projection.hh"

#include	"PDS_Metadata.hh"
#include	"Coordinate.hh"
#include	"HiView_Utilities.hh"

#include	"PVL.hh"
using namespace idaeim;
using namespace idaeim::PVL;

#include	<QString>
#include	<QStringList>
#include	<QtCore/qmath.h>
#include	<QtCore/qnumeric.h>

#include	<string>
using std::string;
#include	<cctype>
using std::toupper;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
#include	<stdexcept>
using std::invalid_argument;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_UTILTIIES			(1 << 2)
#define	DEBUG_CONVERTERS		(1 << 3)
#define DEBUG_METADATA_PARAMETERS (1 << 4)

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
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
	Projection::ID = "UA::HiRISE::Projection ($Revision: 1.13 $ $Date: 2014/11/04 21:01:33 $)";


const char* const
	Projection::CANONICAL_PROJECTION_NAME
		= Projection::IDENTITY_PROJECTION_NAME;

const char* const
	Projection::IDENTITY_PROJECTION_NAME	= "Identity";

const char* const
	Projection::PROJECTION_NAMES[] =
		{
		Projection::IDENTITY_PROJECTION_NAME,
		NULL
		};

const char* const
	Projection::REQUIRED_PARAMETERS[] =	{NULL};

const char* const
	Projection::OPTIONAL_PARAMETERS[] =	{NULL};


const double
	Projection::INVALID_VALUE			= qQNaN ();

#ifndef DEFAULT_PROJECTION_NA_CONSTANT
#define DEFAULT_PROJECTION_NA_CONSTANT	-9998.0
#endif
const double
	Projection::DEFAULT_NA				= DEFAULT_PROJECTION_NA_CONSTANT;


#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif
const double
	Projection::PI						= M_PI,
	Projection::PI_OVER_2				= M_PI / 2.0,
	Projection::PI_OVER_180				= M_PI / 180.0,
	Projection::PI_UNDER_180			= 180.0 / M_PI
#ifndef DBL_EPSILON
	,Projection::DBL_EPSILON				= 2.2204460492503131E-16
#endif
;

/*==============================================================================
	Constructors
*/
Projection::Projection ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Projection" << endl;
#endif
reset ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Projection" << endl;
#endif
}


Projection::Projection
	(
	const Projection&	projection
	)
	:	Projection_Name (projection.Projection_Name),
		NA (projection.NA),

		Positive_West (projection.Positive_West),
		Planetocentric (projection.Planetocentric),
		Pixel_Size (projection.Pixel_Size),
		Horizontal_Offset (projection.Horizontal_Offset),
		Vertical_Offset (projection.Vertical_Offset),
		Equitorial_Radius (projection.Equitorial_Radius),
		Polar_Radius (projection.Polar_Radius),
		Center_Longitude (projection.Center_Longitude),
		Center_Latitude (projection.Center_Latitude),
		Rotation (projection.Rotation),

		Not_Identity (projection.Not_Identity),
		Eccentricity (projection.Eccentricity),
		Coefficient_PC_to_PG (projection.Coefficient_PC_to_PG),
		Coefficient_PG_to_PC (projection.Coefficient_PG_to_PC),
		Rotation_cos (projection.Rotation_cos),
		Rotation_sin (projection.Rotation_sin)
{}


Projection&
Projection::operator=
	(
	const Projection&	projection
	)
{
if (this != &projection)
	{
	Projection_Name			= projection.Projection_Name;

	NA						= projection.NA;

	Not_Identity			= projection.Not_Identity;
	Positive_West			= projection.Positive_West;
	Planetocentric			= projection.Planetocentric;

	Pixel_Size				= projection.Pixel_Size;
	Equitorial_Radius		= projection.Equitorial_Radius;
	Polar_Radius			= projection.Polar_Radius;
	Eccentricity			= projection.Eccentricity;
	Center_Latitude			= projection.Center_Latitude;
	Center_Longitude		= projection.Center_Longitude;
	Horizontal_Offset		= projection.Horizontal_Offset;
	Vertical_Offset			= projection.Vertical_Offset;
	Rotation				= projection.Rotation;

	Coefficient_PC_to_PG	= projection.Coefficient_PC_to_PG;
	Coefficient_PG_to_PC	= projection.Coefficient_PG_to_PC;
	Rotation_cos			= projection.Rotation_cos;
	Rotation_sin			= projection.Rotation_sin;
	}
return *this;
}


Projection*
Projection::clone () const
{return new Projection (*this);}


Projection*
Projection::create
	(
	const idaeim::PVL::Aggregate*	params,
	bool							throw_on_failure
	)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Projection.create" << endl;
#endif
Projection*
	projection (NULL);
QString
	projection_name;
if (params)
	{
	try {projection_name = QString::fromStdString
			(PDS_Metadata::string_value (*params,
				projection_name_parameter ()));}
	catch (Invalid_Argument) {}
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    projection name: " << projection_name << endl;
#endif
if (in_list (projection_name,
		Equirectangular_Projection::projection_names ()))
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    constructing Equirectangular_Projection" << endl;
	#endif
	projection = new Equirectangular_Projection (params);
	}
else
if (in_list (projection_name,
		Polar_Stereographic_Elliptical_Projection::projection_names ()))
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    constructing Polar_Stereographic_Elliptical_Projection"
			<< endl;
	#endif
	projection =
		new Polar_Stereographic_Elliptical_Projection (params);
	}
else
	{
	//	Default identity projection.
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    constructing default Projection" << endl;
	#endif
	projection = new Projection ();
	if (params &&
		throw_on_failure)
		{
		ostringstream
			message;
		message
			<< "Can't create a Projection: The "
				<< projection_name_parameter ()
				<< " parameter" << endl;
		if (projection_name.isEmpty ())
			message << "was not found.";
		else
			message 
				<< "specifies the unsupported " << projection_name
					<< " projection type.";
		throw Invalid_Argument (message.str (), ID);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Projection.Create: @ " << (void*)projection << endl;
#endif
return projection;
}

Projection::~Projection () {}

/*==============================================================================
	Accessors
*/
const char*
Projection::canonical_projection_name () const
{return CANONICAL_PROJECTION_NAME;}

const char* const*
Projection::projection_names ()
{return PROJECTION_NAMES;}

const char*
Projection::projection_name_parameter ()
{return PDS_Metadata::PROJECTION_TYPE_PARAMETER_NAME;}

const char* const*
Projection::required_parameters () const
{return REQUIRED_PARAMETERS;}

const char* const*
Projection::optional_parameters () const
{return OPTIONAL_PARAMETERS;}

const char*
Projection::not_applicable_parameter ()
{return PDS_Metadata::NOT_APPLICABLE_CONSTANT_PARAMETER_NAME;}

const char*
Projection::pixel_size_parameter ()
{return PDS_Metadata::PIXEL_SIZE_PARAMETER_NAME;}

const char*
Projection::latitude_type_parameter ()
{return PDS_Metadata::LATITUDE_TYPE_PARAMETER_NAME;}

const char*
Projection::equatorial_radius_parameter ()
{return PDS_Metadata::EQUITORIAL_RADIUS_PARAMETER_NAME;}

const char*
Projection::polar_radius_parameter ()
{return PDS_Metadata::POLAR_RADIUS_PARAMETER_NAME;}

const char*
Projection::horizontal_offset_parameter ()
{return PDS_Metadata::HORIZONATAL_OFFSET_PARAMETER_NAME;}

const char*
Projection::vertical_offset_parameter ()
{return PDS_Metadata::VERTICAL_OFFSET_PARAMETER_NAME;}

const char*
Projection::positive_longitude_parameter ()
{return PDS_Metadata::POSITIVE_LONGITUDE_PARAMETER_NAME;}

const char*
Projection::center_latitude_parameter ()
{return PDS_Metadata::CENTER_LATITUDE_PARAMETER_NAME;}

const char*
Projection::center_longitude_parameter ()
{return PDS_Metadata::CENTER_LONGITUDE_PARAMETER_NAME;}

const char*
Projection::rotation_parameter ()
{return PDS_Metadata::PROJECTION_ROTATION_PARAMETER_NAME;}

/*==============================================================================
	Manipulators
*/
/*------------------------------------------------------------------------------
	Helpers for parsing parameters.
*/
#ifndef DOXYGEN_PROCESSING
namespace
{
QString
requested_name
	(
	int		requested
	)
{
QString
	name (requested ?
	((requested > 0) ? "REQUIRED" : "OPTIONAL") : "UNREQUESTED");
name += " (";
name += QString::number (requested);
name += ')';
return name;
}


void
throw_out_of_range
	(
	const char* const	parameter,
	double				value,
	string				comment = string ()
	)
{
ostringstream
	message;
message << "Numeric value out of range, "
<< "Invalid " << parameter << " value: " << value;
if (! comment.empty ())
	message
	<< endl
	<< comment;
throw Out_of_Range (message.str ());
}


void
throw_out_of_range
	(
	const char* const	parameter,
	string				value,
	string				comment = string ()
	)
{
ostringstream
	message;
message << "String value out of range, "
	<< "Invalid " << parameter << " value: " << value;
if (! comment.empty ())
	message
	<< endl
	<< comment;
throw Out_of_Range (message.str ());
}

}
#endif


double
Projection::positive_meters
	(
	const Aggregate&	metadata,
	const char* const	parameter
	) const
{
double
	value = NA_checked (metadata, parameter);
if (! is_invalid (value))
	{
	if (value <= 0.0)
		throw_out_of_range (parameter, value,
			"The value must be non-zero positive.");
	string
		units = PDS_Metadata::find_parameter (metadata, parameter)
			->value ().units ();
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "      units = " << units << endl;
	#endif
	if (units.empty () ||
		toupper (units[0]) == 'K')
		value *= 1000.0;		//	Kilometers to meters.
	}
return value;
}


double
Projection::NA_checked
	(
	const Aggregate&	metadata,
	const char* const	parameter
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << parameter << endl;
#endif
double
	value = INVALID_VALUE;
int
	requested = is_requested (parameter,
		required_parameters (), optional_parameters ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "      " << requested_name (requested) << endl;
#endif
if (requested)
	{
	try {value = PDS_Metadata::numeric_value (metadata, parameter);}
	catch (Invalid_Argument&) {if (requested == REQUIRED) throw;}
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    = " << value << endl;
	#endif
	if (value == NA)
		{
		if (requested == Projection::REQUIRED)
			throw_out_of_range (parameter, value,
				"Required value has not-applicable value.");
		else
			{
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "      Optional value has NA value." << endl;
			#endif
			value = INVALID_VALUE;
			}
		}
	}
return value;
}


QString
Projection::string_value
	(
	const Aggregate&	metadata,
	const char* const	parameter
	) const
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    " << parameter << endl;
#endif
string
	value;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "      "
		<< requested_name (is_requested (parameter,
			required_parameters (), optional_parameters ()))
		<< endl;
#endif
try {value = PDS_Metadata::string_value (metadata, parameter);}
catch (Invalid_Argument&)
	{
	if (is_requested (parameter,
			required_parameters (), optional_parameters ())
		== REQUIRED)
		throw;
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    = " << value << endl;
#endif
return QString::fromStdString (value);
}


bool
Projection::parameters
	(
	const idaeim::PVL::Aggregate*	params,
	const char* const				projection_ID
	)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Projection::parameters: @ " << (void*)params << endl;
#endif
bool
	OK = true;
Q_UNUSED(projection_ID);
reset ();

if (params)
	{
	OK = false;

	//	Projection parameters group.
	const Aggregate*
		mapping_parameters;
	Parameter*
		parameter (PDS_Metadata::find_parameter (*params,
			PDS_Metadata::IMAGE_MAP_PROJECTION_GROUP_NAME,
			PDS_Metadata::CASE_INSENSITIVE, 0,
			PDS_Metadata::AGGREGATE_PARAMETER));
	if (parameter)
		{
		mapping_parameters = dynamic_cast<Aggregate*>(parameter);
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "    getting parameters from " << parameter->pathname ()
				<< endl;
		#endif
		}
	else
		mapping_parameters = params;
	#if ((DEBUG_SECTION) & DEBUG_METADATA_PARAMETERS)
	clog << "    mapping parameters -" << endl
		 << *mapping_parameters << endl;
	#endif

	QString
		name;
	try
		{
		//	Common parameters:

		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "    " << projection_name_parameter () << endl;
		#endif
		try {Projection_Name = QString::fromStdString
				(PDS_Metadata::string_value (*mapping_parameters,
				 projection_name_parameter ()));}
		catch (Invalid_Argument&) {}
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Projection_Name = " << Projection_Name << endl;
		#endif

		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "    " << not_applicable_parameter () << endl;
		#endif
		try {NA = PDS_Metadata::numeric_value (*mapping_parameters,
				not_applicable_parameter ());}
		catch (Invalid_Argument&)
			{NA = DEFAULT_NA;}
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      NA = " << NA << endl;
		#endif

		//	Conditional parameters:

		Pixel_Size = positive_meters (*mapping_parameters,
			pixel_size_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Pixel_Size = "
			 	<< Pixel_Size << endl;
		#endif

		Equitorial_Radius = positive_meters (*mapping_parameters,
			equatorial_radius_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Equitorial_Radius = "
				<< Equitorial_Radius << endl;
		#endif

		Polar_Radius = positive_meters (*mapping_parameters,
			polar_radius_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Polar_Radius = "
				<< Polar_Radius << endl;
		#endif

		//	Derived value.
		if (! is_invalid (Equitorial_Radius) &&
			! is_invalid (Polar_Radius))
			{
			//	Planetocentric_to_Planetographic conversion coefficient.
			Coefficient_PC_to_PG = Equitorial_Radius / Polar_Radius;
			Coefficient_PC_to_PG *= Coefficient_PC_to_PG;
			//	Planetographic_to_Planetocentric conversion coefficient.
			Coefficient_PG_to_PC = Polar_Radius / Equitorial_Radius;
			Coefficient_PG_to_PC *= Coefficient_PG_to_PC;
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "      (Re / Rp)**2 = " << Coefficient_PC_to_PG << endl
				 << "      (Rp / Re)**2 = " << Coefficient_PG_to_PC << endl;
			#endif

			Eccentricity = eccentricity (Polar_Radius, Equitorial_Radius);
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "      Eccentricity = " << Eccentricity << endl;
			#endif
			}
		
		Horizontal_Offset = NA_checked (*mapping_parameters,
			horizontal_offset_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Horizontal_Offset = " << Horizontal_Offset << endl;
		#endif

		Vertical_Offset = NA_checked (*mapping_parameters,
			vertical_offset_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Vertical_Offset = " << Vertical_Offset << endl;
		#endif

		name = string_value (*mapping_parameters,
			positive_longitude_parameter ()).toUpper ();
		if (name == PDS_Metadata::POSITIVE_LONGITUDE_WEST_NAME)
			Positive_West = true;
		else
		if (name == PDS_Metadata::POSITIVE_LONGITUDE_EAST_NAME)
			Positive_West = false;
		else
		if (! name.isEmpty ())
			throw_out_of_range
				(positive_longitude_parameter (),
				 name.toStdString (),
				 string ("The value must be \"")
					+ PDS_Metadata::POSITIVE_LONGITUDE_WEST_NAME
					+ "\" or \""
					+ PDS_Metadata::POSITIVE_LONGITUDE_EAST_NAME + "\".");
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Positive_West = " << Positive_West << endl;
		#endif

		Center_Longitude = NA_checked (*mapping_parameters,
			center_longitude_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Center_Longitude = "
				<< Center_Longitude << " deg." << endl;
		#endif
		if (! is_invalid (Center_Longitude))
			{
			if (Positive_West)
				{
				Center_Longitude *= -1.0;
				#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
				clog << "                         westing to easting" << endl;
				#endif
				}
			Center_Longitude = to_radians (to_360 (Center_Longitude));
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "                       = "
					<< Center_Longitude << " rad." << endl;
			#endif
			}

		name = string_value (*mapping_parameters,
			latitude_type_parameter ()).toUpper ();
		if (name == PDS_Metadata::PLANETOCENTRIC_PROJECTION_NAME)
			Planetocentric = true;
		else
		if (name == PDS_Metadata::PLANETOGRAPHIC_PROJECTION_NAME)
			Planetocentric = false;
		else
		if (! name.isEmpty ())
			throw_out_of_range
				(latitude_type_parameter (),
				 name.toStdString (),
				 string ("The only allowed value is \"")
					+ PDS_Metadata::PLANETOCENTRIC_PROJECTION_NAME
					+ "\" or \""
					+ PDS_Metadata::PLANETOGRAPHIC_PROJECTION_NAME + "\".");
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Planetocentric = " << Planetocentric << endl;
		#endif

		Center_Latitude = NA_checked (*mapping_parameters,
			center_latitude_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Center_Latitude = "
				<< Center_Latitude << " deg." << endl;
		#endif
		if (! is_invalid (Center_Latitude))
			{
			if (Center_Latitude > 90 ||
				Center_Latitude < -90)
				throw_out_of_range
					(
					center_latitude_parameter (),
					Center_Latitude,
					"The latitude value must be in the range -90 to +90.");
			Center_Latitude = to_radians (Center_Latitude);
			if (! Planetocentric)
				{
				Center_Latitude =
					planetographic_to_planetocentric (Center_Latitude);
				#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
				clog << "                        ographic to ocentric"
						<< endl;
				#endif
				}
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "                      = "
					<< Center_Latitude << " rad. planetocentric" << endl;
			#endif
			}

		Rotation = NA_checked (*mapping_parameters,
			rotation_parameter ());
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "      Rotation = " << Rotation << " deg." << endl;
		#endif
		if (is_invalid (Rotation) ||
			(Rotation = to_radians (to_360 (Rotation))) == 0.0)
			{
			Rotation_cos = 1.0;
			Rotation_sin = 0.0;
			}
		else
			{
			Rotation_cos = qCos (Rotation);
			Rotation_sin = qSin (Rotation);
			}

		Not_Identity = true;
		}
	catch (idaeim::Exception& except)
		{
		reset ();
/*
		ostringstream
			message;
		if (projection_ID)
			message << projection_ID << endl;
		message
			<< ID << endl
			<< except.message () << endl
			<< "While seeting the "
				<< canonical_projection_name () << " projection parameters.";
		except.message (message.str ());*/
		throw;
		}
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Projection::parameters" << endl;
#endif
return OK;
}


Projection&
Projection::reset ()
{
Projection_Name			= IDENTITY_PROJECTION_NAME;

NA						= DEFAULT_NA;

Not_Identity			=
Positive_West			= false;
Planetocentric			= true;

Pixel_Size				=
Equitorial_Radius		=
Polar_Radius			=
Eccentricity			=
Center_Latitude			=
Center_Longitude		=
Horizontal_Offset		=
Vertical_Offset			=
Rotation				=
Coefficient_PC_to_PG	=
Coefficient_PG_to_PC	= INVALID_VALUE;

Rotation_cos			= 1.0;
Rotation_sin			= 0.0;

return *this;
}

/*==============================================================================
	Converters
*/
Coordinate
Projection::to_world
	(
	const Coordinate&	image_coordinate
	) const
{return Coordinate (image_coordinate);}


Coordinate
Projection::to_image
	(
	const Coordinate&	world_coordinate
	) const
{return Coordinate (world_coordinate);}


QString
Projection::degrees_minutes_seconds
	(
	double	angle
	)
{
int
	degrees = (int)angle;
double
	fraction = qAbs (angle - degrees) * 60.0;
int
	minutes = (int)fraction;
fraction = (fraction - minutes) * 60.0;
int
	seconds = (int)fraction;
int
	msecs = (int)((fraction - seconds) * 1000.0 + 0.5);
if (msecs >= 1000)
	{
	msecs -= 1000;
	seconds++;
	}
if (seconds >= 60)
	{
	seconds -= 60;
	minutes++;
	}
if (minutes >= 60)
	{
	minutes -= 60;
	degrees++;
	}
return
	QString ("%1d %2m %3.%4s")
	.arg (degrees, 3)
	.arg (minutes, 2)
	.arg (seconds, 2)
	.arg (msecs, 3, 10, QChar ('0'));
}


QString
Projection::hours_minutes_seconds
	(
	double	angle
	)
{
angle = to_360 (angle) / 15.0;
int
	hours = (int)angle;
double
	fraction = qAbs (angle - hours) * 60.0;
int
	minutes = (int)fraction;
fraction = (fraction - minutes) * 60.0;
int
	seconds = (int)fraction;
int
	msecs = (int)((fraction - seconds) * 1000.0 + 0.5);
if (msecs >= 1000)
	{
	msecs -= 1000;
	seconds++;
	}
if (seconds >= 60)
	{
	seconds -= 60;
	minutes++;
	}
if (minutes >= 60)
	{
	minutes -= 60;
	hours++;
	}
return
	QString ("%1h %2m %3.%4s")
	.arg (hours, 2)
	.arg (minutes, 2)
	.arg (seconds, 2)
	.arg (msecs, 3, 10, QChar ('0'));
}


double
Projection::decimal_degrees
	(
	const QString&	representation
	)
{
double
	degrees (0.0);
QStringList
	words (representation.split (QRegExp ("\\s+"), QString::SkipEmptyParts));
if (words.isEmpty ())
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Unable to convert an empty string to decimal degrees.";
	throw invalid_argument (message.str ());
	}

bool
	OK (false);
int
	index;
if ((index = words[0].indexOf ('D', 0, Qt::CaseInsensitive)) >= 0)
	{
	//	Degrees.
	words[0] = words[0].left (index);
	degrees = words[0].toInt (&OK);
	if (! OK)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to convert to decimal degrees: \"" << representation
				<< '"' << endl
			<< "Degrees (the first value; \"" << words[0]
				<< "\") must be an integer.";
		throw invalid_argument (message.str ());
		}
	}
else
	{
	//	Hours.
	if ((index = words[0].indexOf ('H', 0, Qt::CaseInsensitive)) >= 0)
		words[0] = words[0].left (index);
	degrees = words[0].toInt (&OK) * 15;	//	Convert to degrees.
	if (! OK)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to convert to decimal degrees: \"" << representation
				<< '"' << endl
			<< "Hours (the first value; \"" << words[0]
				<< "\") must be an integer.";
		throw invalid_argument (message.str ());
		}
	}
degrees = to_360 (degrees);

if (words.size () > 1)
	{
	//	Minutes.
	if ((index = words[0].indexOf ('M', 0, Qt::CaseInsensitive)) >= 0)
		words[1] = words[1].left (index);
	int
		minutes (words[1].toInt (&OK));
	if (! OK)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to convert to decimal degrees: \"" << representation
				<< '"' << endl
			<< "Minutes (the second value; \"" << words[1]
				<< "\") must be an integer.";
		throw invalid_argument (message.str ());
		}
	if (minutes < 0 ||
		minutes > 59)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to convert to decimal degrees: \"" << representation
				<< '"' << endl
			<< "Minutes (the second value; " << words[1]
				<< ") must be in the range [0-60).";
		throw invalid_argument (message.str ());
		}
	degrees += minutes * (15.0 / 60.0);		//	Convert to degrees.
	}

if (words.size () > 2)
	{
	//	Seconds.
	if ((index = words[0].indexOf ('S', 0, Qt::CaseInsensitive)) >= 0)
		words[2] = words[2].left (index);
	double
		seconds (words[2].toDouble (&OK));
	if (! OK)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to convert to decimal degrees: \"" << representation
				<< '"' << endl
			<< "Seconds (the third value; \"" << words[2]
				<< "\") must be a real number.";
		throw invalid_argument (message.str ());
		}
	if (seconds <   0.0 ||
		seconds >= 60.0)
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to convert to decimal degrees: \"" << representation
				<< '"' << endl
			<< "Seconds (the third value; " << words[2]
				<< ") must be in the range [0-60).";
		throw invalid_argument (message.str ());
		}
	degrees += seconds * (15.0 / 3600.0);	//	Convert to degrees.
	}

if (words.size () > 3)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Unable to convert to decimal degrees: \"" << representation
			<< '"' << endl
		<< "Seconds (the third value) should be the last part.";
	throw invalid_argument (message.str ());
	}

return degrees;
}


double
Projection::planetocentric_to_planetographic
	(
	double	latitude
	) const
{
if (! is_invalid (Coefficient_PC_to_PG) &&
	qAbs (latitude) < PI_OVER_2)
	latitude = qAtan (qTan (latitude) * Coefficient_PC_to_PG);
return latitude;
}


double
Projection::planetographic_to_planetocentric
	(
	double	latitude
	) const
{
if (! is_invalid (Coefficient_PG_to_PC) &&
	qAbs (latitude) < PI_OVER_2)
	latitude = qAtan (qTan (latitude) * Coefficient_PG_to_PC);
return latitude;
}

/*==============================================================================
	Derived values
*/
double
Projection::local_radius
	(
	double	latitude
	) const
{
double
	radius = INVALID_VALUE;
if (latitude == PI_OVER_2)
	radius = Polar_Radius;
else
if (latitude == 0.0)
	radius = Equitorial_Radius;
else
if (! is_invalid (Equitorial_Radius) &&
	! is_invalid (Polar_Radius))
	{
	double
		coefficient_e = Equitorial_Radius * qSin (latitude),
		coefficient_p = Polar_Radius      * qCos (latitude);
	radius = Equitorial_Radius * Polar_Radius / qSqrt
		((coefficient_e * coefficient_e) + (coefficient_p * coefficient_p));
	}
return radius;
}


double
Projection::eccentricity
	(
	double	polar_radius,
	double	equatorial_radius
	)
{return qSqrt (1.0 -
	((polar_radius * polar_radius) / (equatorial_radius * equatorial_radius)));}

/*==============================================================================
	Helpers
*/
void
Projection::rotate_from_image
	(
	Coordinate& image_coordinate
	) const
{
if (is_invalid (Rotation) ||
	Rotation == 0.0)
	return;

image_coordinate.X =
	image_coordinate.X * Rotation_cos - image_coordinate.Y * Rotation_sin;
image_coordinate.Y =
	image_coordinate.Y * Rotation_cos + image_coordinate.X * Rotation_sin;
}


void
Projection::rotate_to_image
	(
	Coordinate& image_coordinate
	) const
{
if (is_invalid (Rotation) ||
	Rotation == 0.0)
	return;

image_coordinate.X =
	image_coordinate.X * Rotation_cos + image_coordinate.Y * Rotation_sin;
image_coordinate.Y =
	image_coordinate.Y * Rotation_cos - image_coordinate.X * Rotation_sin;
}

/*==============================================================================
	Utilities
*/
bool
Projection::is_invalid
	(
	double	value
	)
{return qIsNaN (value);}


int
Projection::is_requested
	(
	const char* const	parameter_name,
	const char* const*	required_list,
	const char* const*	optional_list
	)
{
#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
clog << ">>> Projection::is_requested: " << parameter_name << endl;
#endif
const char**
	list;
#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
clog << "    required_list -" << endl;
#endif
if ((list = const_cast<const char**>(required_list)))
	{
	while (*list)
		{
		#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
		clog << "      " << *list << endl;
		#endif
		if (compare (*list++, parameter_name))
			{
			#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
			clog << "<<< Projection::is_requested: "
					<< requested_name (REQUIRED) << endl;
			#endif
			return REQUIRED;
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
clog << "    optional_list -" << endl;
#endif
if ((list = const_cast<const char**>(optional_list)))
	{
	while (*list)
		{
		#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
		clog << "      " << *list << endl;
		#endif
		if (compare (*list++, parameter_name))
			{
			#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
			clog << "<<< Projection::is_requested: "
					<< requested_name (OPTIONAL) << endl;
			#endif
			return OPTIONAL;
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_UTILTIIES)
clog << "<<< Projection::is_requested: "
		<< requested_name (UNREQUESTED) << endl;
#endif
return UNREQUESTED;
}


bool
Projection::in_list
	(
	const QString&		name,
	const char* const*	list
	)
{
const char**
	entry;
if ((entry = const_cast<const char**>(list)))
	while (*entry)
		if (name.compare (*entry++, Qt::CaseInsensitive) == 0)
			return true;
return false;
}



}	//	namespace HiRISE
}	//	namespace UA
