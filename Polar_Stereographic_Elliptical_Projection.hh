/*	Polar_Stereographic_Elliptical_Projection

HiROC CVS ID: $Id: Polar_Stereographic_Elliptical_Projection.hh,v 1.8 2014/11/04 18:15:45 guym Exp $

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

#ifndef HiView_Polar_Stereographic_Elliptical_Projection_hh
#define HiView_Polar_Stereographic_Elliptical_Projection_hh

#include	"Projection.hh"

class QString;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Coordinate;

class Polar_Stereographic_Elliptical_Projection
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
/**	Constructs a Polar_Stereographic_Elliptical_Projection from a
	Parameter Aggregate.

	@param	parameters	A pointer to an idaeim::PVL::Aggregate containing
		(at least) the minimally required parameter values. If all
		required parameters are not provided - including if the argument
		is null - the {@link is_identity() identity projection flag} is
		set.
	@throws	idaeim::Exception	If there is a problem reading the parameters
		or a parameter is found to have an invalid value.
	@see	parameters(const idaeim::PVL::Aggregate*)
*/
explicit Polar_Stereographic_Elliptical_Projection
	(const idaeim::PVL::Aggregate* parameters = 0);

/**	Copies a Polar_Stereographic_Elliptical_Projection.

	@param	projection	The Polar_Stereographic_Elliptical_Projection to
		be copied.
*/
Polar_Stereographic_Elliptical_Projection
	(const Polar_Stereographic_Elliptical_Projection& projection);

/**	Assigns another Polar_Stereographic_Elliptical_Projection to this
	Polar_Stereographic_Elliptical_Projection.

	@param	projection	The Polar_Stereographic_Elliptical_Projection to
		be assigned.
	@return	This Polar_Stereographic_Elliptical_Projection.
*/
virtual Polar_Stereographic_Elliptical_Projection& operator=
	(const Polar_Stereographic_Elliptical_Projection& projection);

/**	Clone the Polar_Stereographic_Elliptical_Projection.

	@return	A pointer to an Polar_Stereographic_Elliptical_Projection
		that is a copy of this Polar_Stereographic_Elliptical_Projection
		or the appropriate subclass.
*/
virtual Polar_Stereographic_Elliptical_Projection* clone () const;

//!	Destroys the Polar_Stereographic_Elliptical_Projection.
virtual ~Polar_Stereographic_Elliptical_Projection ();

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

<dl>
<dt>Wx =
<dd>CLS * {@link center_longitude() Center_Longitude}<br>

	if Distance == 0<br>

	CLS * atan2 (E, -N) + {@link center_longitude() Center_Longitude}<br>

	if Distance != 0<br>
</dl>

	where:

	CLS is the sign of the {@link center_latitude() Center_Latitude}; i.e.
	CLS = -1 if the Center_Latitude is negative, 1 otherwise.

	Distance = sqrt (E**2 + N**2)<br>
	E = CLS * WOx<br>
	N = CLS * WOy<br>

	WOx is the {@link to_world_X_offset(double) horizontal offset from
	the projection center of the image coordinate} in meters.

	WOy is the {@link to_world_Y_offset(double) vertical offset from
	the projection center of the image coordinate} in meters.


	Wy = CLS * P

	where:

	P = PI_OVER_2 - 2 * atan (T)

	where:

	P is iteratively converged such that abs (P - P') <= 0.0000000001;

<dl>
<dt>P' =
<dd>PI_OVER_2 - 2 * atan
		(T * ( (1 - ECC * sin (P)) / (1 + ECC * sin (P)) )**(ECC / 2) )<br>
</dl>
	ECC = {@link eccentricity() Eccentricity}

	and:

	T = Distance * Distance_Coefficient

	where:

<dl>
<dt>Distance_Coefficient =
<dd>sqrt ( (1 + ECC)**(1 + ECC) * (1 - ECC)**(1 - ECC) ) / (2 * Re)<br>

	if the {@link center_latitude() Center_Latitude} is at a pole.<br>

	coefficient_T (CLA) / ( Re * cos (CLA)
		/ sqrt ( 1 - ECC * sin (CLA)**2 ) )<br>

	if the Center_Latitude is not at a pole.<br>

<dt>coefficient_T (latitude) =
<dd>tan ( PI - (latitude / 2) )
		/ ( (1 - (ECC * sin (latitude)))
		  / (1 + (ECC * sin (latitude))) )**(ECC / 2)<br>
</dl>
	CLA = absolute value of the Center_Latitude; i.e. CLS * Center_Latitude.<br>
	Re = {@link equatorial_radius() Equitorial_Radius}<br>

	The {@link center_latitude() Center_Latitude} is at a pole if its
	absolute value is (or is vanishingly near) PI_OVER_2, or the absolute
	value of coefficient_T (CLA) is (or is vanishingly near) zero.

	<b>N.B.</b>: The image sample,line coordinate is {@link
	rotate_from_image(Coordinate&) rotated to the world coordinate
	space}.

	@param	image_coordinate	An image sample,line Coordinate.
		<b>N.B.</b>: The image coordinate system is left-handed
		cartesian; i.e. the x axis corresponds to pixel samples
		increasing rightwards, the y axis corresponds to image lines
		increasing downwards, the origin (0,0) is the upper-left pixel.
	@return	A world longitude,latitude coordinate. The X value of the
		Coordinate is the easting longitude in the [0-360) degree range;
		the Y value is the planetocentric latitude in the +/90 degreee
		range.
	@throws std::out_of_range	If the projection of the coordinate
		resulted in an invalid latitude.
*/
virtual Coordinate to_world (const Coordinate& image_coordinate) const;

/**	Get the image sample,line coordinate for a world longitude,latitude
	coordinate.

	The conversion algorithm is:

	Ix = IOx ( CLS * T * sin (L) )<br>
	Iy = IOy ( -CLS * T * cos (L)) )<br>

	where:

	IOx is the function that produces the {@link
	to_image_X_offset(double) horizontal image offset} derived from the
	projection center borizontal offset of the world coordinate.
	
	IOy is the function that produces the {@link
	to_image_Y_offset(double) vertical image offset} derived from the
	projection center vertical offset of the world coordinate.

	CLS is the sign of the {@link center_latitude() Center_Latitude}; i.e.
	CLS = -1 if the Center_Latitude is negative, 1 otherwise.

	L = CLS * (Wx - {@link center_longitude() Center_Longitude})

	Wx is the world coordinate longitude converted to radians.

	T = coefficient_T (Wy) / Distance_Coefficient

	where:

	Wy is the world coordinate latitude converted to planetographic radians.<br>

<dl>
<dt>coefficient_T (latitude) =
<dd>tan ( PI - (latitude / 2) )
		/ ( (1 - (ECC * sin (latitude)))
		  / (1 + (ECC * sin (latitude))) )**(ECC / 2)<br>
<dt>Distance_Coefficient =
<dd>sqrt ( (1 + ECC)**(1 + ECC) * (1 - ECC)**(1 - ECC) ) / (2 * Re)<br>

	if the {@link center_latitude() Center_Latitude} is at a pole.<br>

	coefficient_T (CLA) / ( Re * cos (CLA)
		/ sqrt ( 1 - ECC * sin (CLA)**2 ) )<br>

	if the Center_Latitude is not at a pole.<br>
</dl>

	CLA = absolute value of the Center_Latitude; i.e. CLS * Center_Latitude.<br>
	ECC = {@link eccentricity() Eccentricity}<br>
	Re = {@link equatorial_radius() Equitorial_Radius}<br>

	The {@link center_latitude() Center_Latitude} is at a pole if its
	absolute value is (or is vanishingly near) PI_OVER_2, or the absolute
	value of coefficient_T (CLA) is (or is vanishingly near) zero.

	<b.N.B.</b>: The Ix,Iy coordinate is rounded to the nearest pixel and
	{@link rotate_to_image(Coordinate&) rotated to image sample,line
	values} before being returned.

	@param	world_coordinate	A world longitude,latitude Coordinate.
		The X value of the Coordinate is the easting longitude; the Y
		value is the planetocentric latitude. Values are in degrees.
	@return	An image sample,line Coordinate.
		<b>N.B.</b>: The image coordinate system is left-handed
		cartesian; i.e. the x axis corresponds to pixel samples
		increasing rightwards, the y axis corresponds to image lines
		increasing downwards, the origin (0,0) is the upper-left pixel.
*/
virtual Coordinate to_image (const Coordinate& world_coordinate) const;

/*==============================================================================
	Derived values
*/
protected:

/**	Computes a T coefficient.

	T = tan ((PI_OVER_2 - latitude) / 2) /
		( (1 - ({@link eccentricity() Eccentricity} * sin (latitude)))
		/ (1 + ({@link eccentricity() Eccentricity} * sin (latitude))) )
			** ({@link eccentricity() Eccentricity} / 2)

	However, T = 0 if PI_OVER_2 - |latitude| < DBL_EPSILON

	where DBL_EPSILON is a very small value.

	<b>N.B.</b>: {@link eccentricity() Eccentricity} / 2 is assumed to
	have been pre-computed as E_over_2.

	@param	latitude	A panetographic latitude value in radians.
	@return	A T coefficient value.
*/
double coefficient_T (double latitude) const;

/**	Calculate the Phi2 coefficient.

	The Phi2 coefficient is used in the coversion of an image coordinate
	{@link to_world{const Coordinate&) to world} coordinate. An
	iterative converging algorithm is used.

	@param	coefficient_t	The coefficient t value calculated based on
		the image coordinate values.
	@return	The Phi2 coefficient used to calculate a world coordinate
		latitude (Y) value.
	@throws	std::out_of_range	If the algorithm failed to converge
		after 15 iterations.
*/
double coefficient_Phi2 (double coefficient_t) const;

/*==============================================================================
	Data
*/
protected:

double
	E_over_2,
	Center_Latitude_Sign,
	Distance_Coefficient;

};	//	End of Polar_Stereographic_Elliptical_Projection class.


}	//	namespace HiRISE
}	//	namespace UA
#endif
