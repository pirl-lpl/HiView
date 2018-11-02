/*	Equirectangular_Projection

HiROC CVS ID: $Id: Equirectangular_Projection.hh,v 1.9 2014/11/04 18:15:45 guym Exp $

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

#ifndef HiView_Equirectangular_Projection_hh
#define HiView_Equirectangular_Projection_hh

#include	"Projection.hh"

class QString;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Coordinate;

class Equirectangular_Projection
:	public Projection
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	The canonical name of the projection algorithm implemented by the class.
static const char* const
	CANONICAL_PROJECTION_NAME;

//!	Names by which this projection is known.
static const char* const
	PROJECTION_NAMES[];

//!	Names of required parameters for this projection.
static const char* const
	REQUIRED_PARAMETERS[];

//!	Names of optional parameters for this projection.
static const char* const
	OPTIONAL_PARAMETERS[];

/*==============================================================================
	Constructors
*/
/**	Constructs a Equirectangular_Projection from a Parameter Aggregate.

	@param	parameters	A pointer to an idaeim::PVL::Aggregate containing
		(at least) the minimally required parameter values. If all
		required parameters are not provided - including if the argument
		is null - the {@link is_identity() identity projection flag} is
		set.
	@throws	idaeim::Exception	If there is a problem reading the parameters
		or a parameter is found to have an invalid value.
	@see	parameters(const idaeim::PVL::Aggregate*)
*/
explicit Equirectangular_Projection
	(const idaeim::PVL::Aggregate* parameters = 0);

/**	Copies an Equirectangular_Projection.

	@param	projection	The Equirectangular_Projection to be copied.
*/
Equirectangular_Projection (const Equirectangular_Projection& projection);

/**	Assigns another Equirectangular_Projection to this
	Equirectangular_Projection.

	@param	projection	The Equirectangular_Projection to be assigned.
	@return	This Equirectangular_Projection.
*/
virtual Equirectangular_Projection& operator=
	(const Equirectangular_Projection& projection);

/**	Clone the Equirectangular_Projection.

	@return	A pointer to an Equirectangular_Projection that is a copy of
		this Equirectangular_Projection or the appropriate subclass.
*/
virtual Equirectangular_Projection* clone () const;

//!	Destroys the Equirectangular_Projection.
virtual ~Equirectangular_Projection ();

/*==============================================================================
	Accessors
*/
/**	Canonical projection algorithm name.

	@return	The canonical name of the projection algorithm.
	@see	projection_name()
*/
virtual const char* canonical_projection_name () const;

/*	Get the list of names by which a projection is known.

	@return	A NULL terminated list of names for the projection
		implementation. Any of the names in the list will qualify for a
		match when the {@link projection_name_parameter() projection name
		parameter is examined when a Projection is to be {@link
		create(const idaeim::PVL::Aggregate*, bool) created}.
*/
static const char* const* projection_names ();

/**	Get the list of parameters required by the projection implementation.

	The list of required parameters is used when {@link
	parameters(const idaeim::PVL::Aggregate* parameters, const char* const*,
	const char* const*) parameter values are obtained}.

	@return	A NULL terminated list of parameter name strings. May be NULL
		if no parameters are required.
*/
virtual const char* const* required_parameters () const;

/**	Get the list of optional parameters that may be used by the
	projection implementation.

	The list of optional parameters is used when {@link
	parameters(const idaeim::PVL::Aggregate* parameters, const char* const*,
	const char* const*) parameter values are obtained}.

	@return	A NULL terminated list of parameter name strings. May be NULL
		if no optional parameters are used.
*/
virtual const char* const* optional_parameters () const;

/*==============================================================================
	Converters
*/
/**	Get the world longitude,latitude coordinate for an image sample,line
	coordinate.

	The conversion algorithm is:

	Wx = to_degrees ({@link center_longitude() Center_Longitude}
		 + WOx / (Rl * cos ({@link center_latitude() Center_Latitude})))<br>
	Wy = to_degrees (WOy / Rl)<br>

	where:

	Rl is the {@link Projection::local_radius(double) radius} of the
	planet at the Center_Latitude.

	WOx is the {@link to_world_X_offset(double) horizontal offset from
	the projection center of the image coordinate} in meters.

	WOy is the {@link to_world_Y_offset(double) vertical offset from
	the projection center of the image coordinate} in meters.

	<b>N.B.</b>: The image sample,line coordinate is {@link
	rotate_from_image(Coordinate&) rotated to the world coordinate
	space}.

	@param	image_coordinate	The image sample,line coordinate.
		<b>N.B.</b>: The image coordinate system is left-handed
		cartesian; i.e. the x axis corresponds to pixel samples
		increasing rightwards, the y axis corresponds to image lines
		increasing downwards, the origin (0,0) is the upper-left pixel.
	@return	A world longitude,latitude coordinate. The X value of the
		Coordinate is the easting longitude in the [0-360) degree range;
		the Y value is the planetocentric latitude in the +/90 degreee
		range.
*/
virtual Coordinate to_world (const Coordinate& image_coordinate) const;

/**	Get the image sample,line coordinate for a world longitude,latitude
	coordinate.

	The conversion algorithm is:

	Ix = IOx ( (Rl * cos ({@link center_latitude() Center_Latitude}))
		* (Wx - {@link center_latitude() Center_Latitude}) )

	Iy = IOy ( Rl * Wy )

	where:

	Wx is the world coordinate longitude converted to radians.<br>
	Wy is the world coordinate latitude converted to radians.<br>

	Rl is the {@link Projection::local_radius(double) radius} of the
	planet at the Center_Latitude.

	IOx is the function that produces the {@link
	to_image_X_offset(double) horizontal image offset} derived from the
	projection center borizontal offset of the world coordinate.
	
	IOy is the function that produces the {@link
	to_image_Y_offset(double) vertical image offset} derived from the
	projection center vertical offset of the world coordinate.

	<b.N.B.</b>: The Ix,Iy coordinate is rounded to the nearest pixel and
	{@link rotate_to_image(Coordinate&) rotated to image sample,line
	values} before being returned.

	@param	world_coordinate	A world longitude,latitude Coordinate.
		The X value of the Coordinate is the easting longitude; the Y
		value is the planetocentric latitude. Values are in degrees.
	@return	The image sample,line coordinate.
		<b>N.B.</b>: The image coordinate system is left-handed
		cartesian; i.e. the x axis corresponds to pixel samples
		increasing rightwards, the y axis corresponds to image lines
		increasing downwards, the origin (0,0) is the upper-left pixel.
*/
virtual Coordinate to_image (const Coordinate& world_coordinate) const;

/*==============================================================================
	Data
*/
protected:

//	Pre-computed values.
double
	Local_Radius,
	Local_Radius_Coefficient;

};	//	End of Equirectangular_Projection class.


}	//	namespace HiRISE
}	//	namespace UA
#endif
