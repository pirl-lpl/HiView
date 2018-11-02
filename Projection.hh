/*	Projection

HiROC CVS ID: $Id: Projection.hh,v 1.13 2014/11/04 18:15:45 guym Exp $

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

#ifndef UA_HiRISE_Projection_hh
#define UA_HiRISE_Projection_hh

//	Forward references.
namespace idaeim {
namespace PVL {
class Parameter;
class Aggregate;
}}

#include	<QString>

#include	<string>
#include	<iosfwd>


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Coordinate;

/**	A <i>Projection</i> is used to algorithmically map, or "project",
	from one image coordinate system to another while maintaining the
	spatial relationships of all coordinates.

	Spatial projections are used in many contexts. The typical example is
	when a camera image of curved surface is to be mapped as if the
	surface were flat. The commonly known Mercator projection of a map of
	the Earth is such a projection. There are many map projections. See
	Snyder, John P., "Map Projections - A Working Manual", U.S Geological
	Survey Professional Paper 1395, 1987 for a discussion of the map
	projection algorithm used in these classes.

	The core projection characteristic is an algorithm that maps
	coordinates from one coordinate system to another. The specific
	projection algorithms used - the implementation of a set of equations
	- will, of course, vary from one type of projection to another to
	achieve the desired mapping effect. In addition, the choice of
	projection is likey to depend on the range of coordinate values to be
	mapped - projections are likely to have increasing visual distortion
	effects as boundary conditions are approached - or the accuracy to be
	achieved - the implementing equations are like to make various
	simplifying assumptions. Nevertheless, all projections are expected
	to map coordinates between image and world coordinate systems. The
	former is in image pixel units of sample - horizontal distance from
	the left-most pixel - and and line - vertical distance from the
	top-most line - of a rectangular array of pixels. The latter is
	typically in planetary longitude - angular angular distance about the
	center of the planet relative to a zero-reference longitude - and
	latitude - angular angular distance about the center of the planet
	relative to the equator - usually measured in decimal degrees.
	However, other "world" coordinate systems are quite possible. In
	order to accommodate images of arbitrary location, an intermediate
	projeciton coordinate system is typically used by the projection
	equations, with a simple translational equation used to map between
	the projection coordinate system and the location of the image raster
	in that system.

	A specific projection, implemented as a subclass of Projection, will
	require various parameters need by the projection equations. These
	are supplied by a PVL Parameter Aggregate. Because this Projection
	implementation assumes that planetary projections are to be
	implemented by subclasses, the Parameter group used to construct a
	Projection collects the common set of parameter values required by
	the specific projection algorithms. If all the required parameters
	are not provided the default identity projection - in which
	coordinate values are not changed - is used.

	Note: This class is based on the Java Projection class of the
	Image_Tools package of the PIRL Java Packages
	(http://pirlwww.lpl.arizona.edu/software/PIRL_Java_Packages.shtml).
	
	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.13 $
*/
class Projection
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

//!	The {@link projection_name() projection name for the identity projection.
static const char* const
	IDENTITY_PROJECTION_NAME;

//!	Names by which this projection is known.
static const char* const
	PROJECTION_NAMES[];

//!	Names of required parameters for this projection.
static const char* const
	REQUIRED_PARAMETERS[];

//!	Names of optional parameters for this projection.
static const char* const
	OPTIONAL_PARAMETERS[];


//!	Default use_polar_spherical_sterographic value.
static const bool
	DEFAULT_USE_SPHERICAL;

/**	Value of an invalid datum.

	<b>WARNING</b>: The invalid value is the special quiet NaN value
	that can not be meaningfully compared with other values, including
	itself; use the {@link is_invalid(double) is_invalid} function to
	test a value for the special invalid value.

	Note: The invalid value always produces an invalid value in math
	operations.
*/
static const double
	INVALID_VALUE;

//!	Default not-applicable constant.
static const double
	DEFAULT_NA;

/*==============================================================================
	Constructors
*/
/**	Creates a projection-specific Projection subclass implementation.

	The choice of specific Projection subclass implementation is based on
	the type of projection specified by the {@link
	#PROJECTION_TYPE_PARAMETER_NAME} parameter. The selected subclass
	will be constructed using the specified Aggregate of parameters.

	As a special case, if the projection type name, with all embedded
	spaces removed, is {@link #POLARSTEREOGRAPHIC_PROJECTION_NAME} the
	spherical form of this projection algorithm is selected when {@link
	use_polar_spherical_sterographic(bool) enabled}; otherwise the
	elliptical form of the projection algorithm is selected. The
	spherical form of the projection algorithm is faster but slightly
	less accurate than the elliptical form.

	@param	parameters	A pointer to an idaeim::PVL::Aggregate containing
		the projection definition parameter values. These parameters will
		be passed to the specific projection class to be constructed. If
		NULL, or there is no {@link #PROJECTION_TYPE_PARAMETER_NAME}
		parameter or it specifies an unsupported parameter type, a
		default Projection is constructed which will have the {@link
		#is_identity() identity} Projection will be provided.
	@param	throw_on_failure	If true and the parameters are not NULL
		but there is no PROJECTION_TYPE_PARAMETER_NAME parameter or it
		names a projection type that is not supported an
		idaeim::PVL::Invalid_Argument exception will be thrown with a
		message; otherwise a default identity Projection will be provided
		when an appropriate projection-specific subclass can not be
		identified. <b>N.B.</b>: If the parameters are NULL this argument
		is ignored and a default identity Projection will be provided.
	@return	A pointer to a Projection (identity projection) or
		Projection specific subclass object. This will never be NULL.
		<b>N.B.</b>: Ownership of the object is transferred to the
		caller.
	@throws	idaeim::PVL::Invalid_Argument	If throw_on_failure is true
		and the {@link #PROJECTION_TYPE_PARAMETER_NAME} parameter could
		not be found or specifies an unsupported projection type.
	@see	Equirectangular_Projection
	@see	Polar_Stereographic_Elliptical_Projection
	@see	Polar_Stereographic_Spherical_Projection
*/
static Projection* create (const idaeim::PVL::Aggregate* parameters = 0,
	bool throw_on_failure = false);

/**	Assigns another Projection to this Projection.

	@param	projection	The Projection to be assigned.
	@return	This Projection.
*/
virtual Projection& operator= (const Projection& projection);

/**	Clone the Projection.

	@return	A pointer to a Projection that is a copy of this Projection
		or the appropriate Projection subclass.
*/
virtual Projection* clone () const;

//!	Destroys the Projection.
virtual ~Projection ();


protected:

/**	Constructs a default Projection.

	An identity Projection is constructed with all state data {@link reset()
	reset} to their default values.
*/
Projection ();

/**	Copies a Projection.

	@param	projection	The Projection to be copied.
*/
Projection (const Projection& projection);

/*==============================================================================
	Accessors
*/
public:

/**	Canonical projection algorithm name.

	@return	The canonical name of the projection algorithm.
	@see	projection_name()
*/
virtual const char* canonical_projection_name () const;

/**	Projection type name.

	<b>N.B.</b>: The projection type name is obtained from the
	{@link parameters(const idaeim::PVL::Aggregate*) parameters'}
	{@link projection_name_parameter() projection name parameter}.
	This may be different than the {@link canonical_projection_name()
	cononical projection name}.

	@return The name of the projection being used.
*/
inline QString projection_name () const
{return Projection_Name;}

/*	Get the list of names by which a projection is known.

	@return	A NULL terminated list of names for the projection
		implementation. Any of the names in the list will qualify for a
		match when the {@link projection_name_parameter() projection name
		parameter is examined when a Projection is to be {@link
		create(const idaeim::PVL::Aggregate*, bool) created}.
*/
static const char* const* projection_names ();

/**	The name of the parameter associated with the {@link name() proection
	name}.

	@return	The name of the parameter used to obtain the projection name.
*/
static const char* projection_name_parameter ();

/**	Test if an identity proction is in effect.

	An identity proejction will be {@link
	create(const idaeim::PVL::Aggregate*, bool) created} if a recognized
	{@link name() projection name} was not found in the {@link
	parameters(const idaeim::PVL::Aggregate*) parameters} or some other
	problem was encountered while processing the parameters.

	@return	true if an identity projection is in effect; false otherwise.
*/
inline bool is_identity () const
{return ! Not_Identity;}

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

/**	Get the Not-Applicable value.

	The Not-Applicable value is initially {@link reset() reset} to
	the {@link #DEFAULT_NA} value. When the {@link
	parameters(const idaeim::PVL::Aggregate*) parameters} are
	processed the Not-Applicable value will be set to the value of the
	{@link not_applicable_parameter() not-applicable parameter} if
	present.

	Parameters obtained that have the Not-Applicable value are set to
	the {@link #INVALID_VALUE}.

	@return	The Not-Applicable value.
	@see	not_applicable_parameter()
*/
inline double not_applicable () const
{return NA;}

/**	The name of the parameter associated with the {@link not_applicable()
	not-applicable} value.

	@return	The name of the parameter used to obtain the not-applicable
		value.
*/
static const char* not_applicable_parameter ();

/**	Pixel size.

	The PDS Data Dictionary specifies that the units for the pixel size
	parameter are KM/PIX. However, for some data products (e.g. HiRISE)
	the pixel size is measured in meters. The only way to distinguish
	which units apply is to examine the units associated with the value:
	If the units string starts with 'K' (case insensitive) the value is
	taken to be in kilometers. If the value has no units it is assumed
	that the PDS Data Dictionary units are applicable.

	@return	The size of an image pixel, in meters.
	@see	pixel_size_parameter()
*/
inline double pixel_size () const
{return Pixel_Size;}

/**	The name of the parameter associated with the {@link pixel_size()
	pixel size} value.

	@return	The name of the parameter used to obtain the pixel size value.
*/
static const char* pixel_size_parameter ();

/**	Equitorial radius.

	@return	The planet radius at its equator, in meters.
	@see	equatorial_radius_parameter()
*/
inline double equatorial_radius () const
{return Equitorial_Radius;}

/**	The name of the parameter associated with the {@link equatorial_radius()
	equatorial radius} value.

	@return	The name of the parameter used to obtain the equatorial radius
		value.
*/
static const char* equatorial_radius_parameter ();

/**	Polar radius.

	@return	The planet radius at its poles, in meters.
	@see	polar_radius_parameter()
*/
inline double polar_radius () const
{return Polar_Radius;}

/**	The name of the parameter associated with the {@link polar_radius()
	polar radius} value.

	@return	The name of the parameter used to obtain the polar radius
		value.
*/
static const char* polar_radius_parameter ();

/**	Horizontal image offset.

	@return	The image horizontal offset from the projection center, in
		pixels.
	@see	horizontal_offset_parameter()
*/
inline double horizontal_offset () const
{return Horizontal_Offset;}

/**	The name of the parameter associated with the {@link
	horizontal_offset() horizontal image offset} value.

	@return	The name of the parameter used to obtain the image horizontal
		offset value.
*/
static const char* horizontal_offset_parameter ();

/**	Vertical image offset.

	@return	The image vertical offset from the projection center, in
		pixels.
	@see	vertical_offset_parameter()
*/
inline double vertical_offset () const
{return Vertical_Offset;}

/**	The name of the parameter associated with the {@link
	vertical_offset() vertical image offset} value.

	@return	The name of the parameter used to obtain the image vertical
		offset value.
*/
static const char* vertical_offset_parameter ();

/**	Planetocentric projection.

	The projection is planetocentric if the {latitude_type_parameter()
	latitude type parameter} has the value {@link
	#PDS_Metadata::PLANETOCENTRIC_PROJECTION_NAME}.

	@return	true if the projection is planetocentric; false if the
		projection is planetographic or has not been determined from
		the {@link parameters(const idaeim::PVL::Aggregate*) parameters}.
	@see	latitude_type_parameter()
*/
inline bool planetocentric () const
{return Planetocentric;}

/**	The name of the parameter associated with a {@link planetocentric()
	planetocentric} projection.

	@return	The name of the parameter used to obtain the planetocentric
		value.
*/
static const char* latitude_type_parameter ();

/**	Center latitude.

	The value of the {@link center_latitude_parameter() center latitude
	parameter} is converted from degrees to [0 - 2PI) radians.

	<b>N.B.</b>: If the {@link latitude_type_parameter() latitude type}
	is {@link planetocentric() planetocentric} the value of the center
	latitude parameter will be {@link
	planetographic_to_planetocentric(double) converted to planetocentric}
	to provide a consistent latitude value.

	@return	The projection planetocentric center latitude, in radians.
		This will be the {@link #INVALID_VALUE} if the value was not
		obtained from the parameters.
	@see	center_latitude_parameter()
*/
inline double center_latitude () const
{return Center_Latitude;}

/**	The name of the parameter associated with the {@link center_latitude()
	center latitude} value.

	The center latitude parameter is expected to provide a value in
	degrees.

	@return The name of the parameter used to obtain the center latitude
		value.
*/
static const char* center_latitude_parameter ();

/**	Positive west longitude coordinate system.

	Positive west longitude values are used if the {@link
	positive_longitude_parameter() positive longitude parameter name} has the
	value {@link #PDS_Metadata::POSITIVE_LONGITUDE_WEST_NAME}. The return
	value will be false if the parameter name has the value {@link
	#PDS_Metadata::POSITIVE_LONGITUDE_EAST_NAME} or the parameter was
	not found.

	@return	true if the projection coordinate system uses positive west
		longitude values; false if positive east longitutude values are
		used.
*/
inline bool positive_west () const
{return Positive_West;}

/**	The name of the parameter associated with the {@link positive_west()
	positive west longitude} value.

	@return The name of the parameter used to obtain the positive west
		longitude value.
*/
static const char* positive_longitude_parameter ();

/**	Center longitude.

	The value of the {@link center_longitude_parameter() center longitude
	parameter} is converted from degrees to [0 - 2PI) radians.

	<b>N.B.</b>: If the {@link positive_longitude_parameter() longitude
	direction} is {@link positive_west() positive west} the value of the
	center longitude parameter will have its sign reversed to provide a
	consistent positive east longitude value.

	@return	The projection center easterly longitude, in [0 - 2PI)
		radians. This will be the {@link #INVALID_VALUE} if the value was
		not obtained from the parameters.
	@see	center_longitude_parameter()
*/
inline double center_longitude () const
{return Center_Longitude;}

/**	The name of the parameter associated with the {@link center_longitude()
	center longitude} value.

	The center longitude parameter is expected to provide a value in
	degrees.

	@return The name of the parameter used to obtain the center longitude
		value.
*/
static const char* center_longitude_parameter ();

/**	Rotation.

	The value of the {@link rotation_parameter_name() rotation parameter}
	is converted from degrees to [0 - 2PI) radians.

	@return	The rotation, in radians. This will be the {@link
		#INVALID_VALUE} if the value was not obtained from the
		parameters.
	@see	rotation_parameter()
*/
inline double rotation () const
{return Rotation;}

/**	The name of the parameter associated with the {@link rotation()
	rotation} value.

	The rotation parameter is expected to provide a value in degrees.

	@return The name of the parameter used to obtain the rotation value.
*/
static const char* rotation_parameter ();

/*------------------------------------------------------------------------------
	Derived values
*/
/**	Eccentricity of the planet.

	E = sqrt (1 - (Rp**2 / Re**2))

	where:

	Re = {@link equatorial_radius() Equatorial Radius}<br>
	Rp = {@link polar_radius() Polar Radius}<br>

	@return	The planet eccentricity. This will be the INVALID_VALUE if a
		required parameter was not obtained.
	@see	eccentricity(double, double)
*/
inline double eccentricity () const
{return Eccentricity;}

/**	Eccentricity.

	E = sqrt (1 - (polar_radius**2 / equatorial_radius**2))

	@param	polar_radius	The body polar radius.
	@param	equatorial_radius	The body equatorial radius.
	@return	The calculated eccentricity value.
*/
static double eccentricity (double polar_radius, double equatorial_radius);

/**	Calculate the radius of the planet at some latitude.

	R = Re; if latitude = 0.0<br>
	R = Rp; if latitude = PI_OVER_2<br>
	R = Re * Rp / sqrt (a**2 + b**2)<br>

	where:

	a = Re * sin (latitude)<br>
	b = Rp * cos (latitude)<br>

	@param	latitude	The latitude, in radians, at which the local
		radius is to be calculated.
	@return	The radius of the planet at the specified latitude. This
		will be the INVALID_VALUE if a required parameter was not
		obtained.
*/
double local_radius (double latitude) const;

/*==============================================================================
	Manipulators
*/
/**	Reset to the default initial state.

	The {@link projection_name() projection name} is reset to the
	{@link #IDENTITY_PROJECTION_NAME}.

	The {@link not_applicable () not-applicable} value is reset to the
	{@link #DEFAULT_NA} value.

	The {@link is_identity() identity} state is set to false.

	The {@link planetocentric() planetocentric} and {@link positive_west()
	positive west} flags are set to false.

	All other parameter values, and derived values, are set to the {@link
	#INVALID_VALUE}.

	@return	This Projection.
*/
virtual Projection& reset ();

/*==============================================================================
	Converters
*/
/**	Get the world longitude,latitude coordinate for an image sample,line
	coordinate.

	This method simply returns a copy of the coordinate argument. A
	projection specific subclass will override this method.

	@param	image_coordinate	An image sample,line Coordinate.
	@return	A Coordinate containing a copy of the image_coordinate.
*/
virtual Coordinate to_world (const Coordinate& image_coordinate) const;

/**	Get the image sample,line coordinate for a world longitude,latitude
	coordinate.

	This method simply returns a copy of the coordinate argument. A
	projection specific subclass will override this method.

	@param	world_coordinate	The world longitude,latitude Coordinate.
	@return	A Coordinate containing a copy of the world_coordinate.
*/
virtual Coordinate to_image (const Coordinate& world_coordinate) const;

/** Converts an angle in decimal degrees, to a degrees, minutes, seconds
	representation.

	The format of the degrees, minutes, seconds String representation is:

	DDDd MMm SS.SSSs

	where DDD is integer degrees, MM is integer minutes in the range 0 to
	59, and SS.SSS is seconds with fractional seconds in the range 0.0 to
	59.999 For example, 206.291 degrees is represented as "206d 17m
	27.600s". <b>N.B.</b>: A fixed field format is used in which leading
	spaces and trailing zeros are used to each value occurs at a specific
	location in the String representation.

	@param	angle	The angle in degrees to be represented.
	@return	The String representation.
*/
static QString degrees_minutes_seconds (double angle);

/** Converts an angle in decimal degrees, to an hours, minutes, seconds
	representation.

	The format of the hours, minutes, seconds String representation is:

	HHh MMm SS.SSSs

	where HH is integer hours in the range 0 to 24, MM is integer minutes
	in the range 0 to 59, and SS.SSS is seconds with fractional seconds
	in the range 0.0 to 59.999 For example, 206.291 degrees is
	represented as "13h 45m  9.840s". <b>N.B.</b>: A fixed field format
	is used in which leading spaces and trailing zeros are used to each
	value occurs at a specific location in the String representation.

	@param	angle	The angle in degrees to be represented.
	@return	The String representation.
*/
static QString hours_minutes_seconds (double angle);

/** Converts a degrees representation string to its decimal value.

	The format of the degrees representation string is:

	HH[h] [MM[m] [SS[.SSS][s]]]

	or

	DDDd [MM[m] [SS[.SSS][s]]]

	where HH is integer hours, MM is integer minutes, and SS.SSS is
	seconds with fractional seconds; DDD is integer degrees. <b>N.B.</b>:
	The first value must have a "d" suffix to be taken as a degrees
	value; the second and third values, if present, are always taken
	to be minutes and seconds, respectively.

	@param	representation	The degrees representation string.
	@return	The degrees value of the representation.
	@throws	invalid_argument	If the representation is empty or
		could not be coverted due to a syntax problem.
*/
static double decimal_degrees (const QString& representation);

/**	Convert a degrees value to a radians value.

	radians = degrees * PI / 180

	@param	degrees	A degrees value.
	@return	The radians value.
	@see	to_degrees(double)
*/
inline static double to_radians (double degrees)
{return degrees * PI_OVER_180;}

/**	Convert a radians value to a degrees value.

	degrees = radians * 180 / PI

	@param	radians	A radians value.
	@return	The degrees value.
	@see	to_radians(double)
*/
inline static double to_degrees (double radians)
{return radians * PI_UNDER_180;}

/**	Ensure that a longitude value is in the range 0 to 360 degrees.

	A negative longitude value is repeatedly increased by 360 until it is
	no longer negative. A longitude value greater than or equal to 360 is
	repeatedly decreased by 360 until it is less than 360.

	@param	longitude	The longitude value to map to the 0 inclusive to
		360 exclusive degree domain.
	@return	A longitude value in the [0-360) degree domain.
*/
static double to_360 (double longitude)
{
while (longitude <    0.0) longitude += 360.0;
while (longitude >= 360.0) longitude -= 360.0;
return longitude;
}

/**	Ensure that a longitude value is in the range -180 to 180 degrees.

	A longitude value less than -180 is repeatedly increased by 360 until
	it is greater than or equal to -180. A longitude value greater than
	or equal to 180 is repeatedly decreased by 360 until it is less than
	180.

	@param	longitude	The longitude value to map to the -180 inclusive
		to 180 exclusive degree domain.
	@return	A longitude value in the [-180 to 180) degree domain.
*/
static double to_180 (double longitude)
{
while (longitude < -180.0) longitude += 360.0;
while (longitude >= 180.0) longitude -= 360.0;
return longitude;
}

/**	Convert planetocentric latitude to planetographic latitude.

	Lpg = atan (tan (Lpc) * (Re / Rp)**2)

	where:

	Re = {@link equatorial_radius() equatorial radius} in meters<br>
	Rp = {@link polar_radius() polar radius} in meters<br>

	Note: (Re / Rp)**2 is precalculated as Coefficient_PC_to_PG when the
	{@link parameters(const idaeim::PVL::Aggregate*) parameters} are
	processed. <b>WARNING</b>: If the parameter values provided for the
	equatorial and polar radius are not the true planet radii (as is the
	case for HiRISE data products %-[) then planetocentric-planetographic
	conversions are not possible.

	<b>N.B.</b>: If the absolute value of the latitude is >= PI_OVER_2,
	or the Coefficient_PC_to_PG is the {@link #INVALID_VALUE},
	then the latitude value is returned unchanged.

	@param	latitude	The planetocentric latitude (Lpc), in radians, to
		convert.
	@return	The converted planetographic latitude (Lpg) value in radians.
*/
double planetocentric_to_planetographic (double latitude) const;

/**	Convert planetographic latitude to planetocentric latitude.

	Lpc = atan (tan (Lpg) * (Rp / Re)**2);

	where:

	Re = {@link equatorial_radius() equatorial radius} in meters<br>
	Rp = {@link polar_radius() polar radius} in meters<br>

	Note: (Rp / Re)**2 is precalculated as Coefficient_PG_to_PC when the
	{@link parameters(const idaeim::PVL::Aggregate*) parameters} are
	processed. <b>WARNING</b>: If the parameter values provided for the
	equatorial and polar radius are not the true planet radii (as is the
	case for HiRISE data products %-[) then planetocentric-planetographic
	conversions are not possible.

	<b>N.B.</b>: If the absolute value of the latitude is PI_OVER_2 (or
	greater), or the Coefficient_PG_to_PC is the {@link #INVALID_VALUE},
	then the latitude value is returned unchanged.

	@param	latitude	The planetographic latitude (Lpg), in radians, to
		convert.
	@return	The converted planetocentric latitude (Lpc) value in radians.
*/
double planetographic_to_planetocentric (double latitude) const;

/*==============================================================================
	Utilities
*/
/**	Test if a value is the special {@link #INVALID_VALUE}.

	@param	value	The value to be tested.
	@return	true if the value is the INVALID_VALUE; false otherwise.
*/
static bool is_invalid (double value);

/**	Possible return values for the {@link
	is_requested(const char* const, const char* const*, const char* const*)
	is_requested} function.
*/
enum
	{
	OPTIONAL	= -1,
	UNREQUESTED	= 0,
	REQUIRED	= 1
	};

/**	Check a parameter name for being in a required or optional name list.

	The required list is first checked for the presences of the parameter
	name. If the name is found in the list REQUIRED (1) is returned.
	
	The optional list is then checked for the presences of the parameter
	name. If the name is found in the list OPTIONAL (-1) is returned.

	If the name is not found in either list UNREQUESTED (0) is returned.

	<b>N.B.</b>: The name comparisons are case insensitive.

	@param	parameter_name	The parameter name to be checked.
	@param	required_list	A NULL-terminated list of names that
		constitute the required list. May be NULL to skip the required
		list check.
	@param	optional_list	A NULL-terminated list of names that
		constitute the optional list. May be NULL to skip the optional
		list check.
	@return	REQUIRED (1) if the parameter name was found in the required list,
		OPTIONAL (-1) if the parameter name was found in the optional list,
		UNREQUESTED (0) if the parameter name was not found in either list.
*/
static int is_requested (const char* const parameter_name,
	const char* const* required_list = NULL,
	const char* const* optional_list = NULL);

/**	Test if a name is in a list of names.

	@param	name	The name to be tested.
	@param	list	A NULL-terminated list of names. May be NULL.
	@return	true if the name was found in the list; false otherwise.
*/
static bool in_list (const QString& name, const char* const* list);

/*==============================================================================
	Helpers
*/
protected:
/*------------------------------------------------------------------------------
	Parameters
*/
/**	Initialize the Projection parameters.

	If neither {@link required_parameters() required} nor {@link
	optional_parameters() optional} parameters are available only the
	{@link parameter_name() parameter name} and {@link not_applicable()
	not-applicable} values will be obtained, if present.

	<b>N.B.</b>: If an exception is thrown the projection state
	will have been {@link reset() reset} to its default identity
	values.

	@param	params	A pointer to an idaeim::PVL::Aggregate that contains
		the parameters used to set data values to be used by the
		projection algorithms. If NULL the Projection is {@link reset()
		reset}.
	@param	projection_ID	The ID of the Projection subclass.
	@return	true if all requred parameters were found; false otherwise.
	@throws idaeim::PVL::Invalid_Argument	If a required parameter
		could not be found.
	@throws	idaeim::PVL::Out_of_Range	If a required parameter, or a
		variable derived from a required parameter, has a value that is
		invalid.
*/
bool parameters (const idaeim::PVL::Aggregate* params = NULL,
	const char* const projection_ID = NULL);

virtual double positive_meters (const idaeim::PVL::Aggregate& params,
	const char* const parameter) const;

virtual double NA_checked (const idaeim::PVL::Aggregate& params,
	const char* const parameter) const;

virtual QString string_value (const idaeim::PVL::Aggregate& params,
	const char* const parameter) const;

/*------------------------------------------------------------------------------
	Converters
*/
/**	Convert a longitude value to an image x value.

	sample = (longitude / Ps) + Ox

	where:

	Ps = {@link pixel_size() pixel size} in meters<br>
	Ox = image {@link horizontal_offset() horizontal offset} in pixels<br>

	@param	longitude	A longitude value in radians.
	@return	An image sample (x) value in pixels.
*/
inline double to_image_X_offset (double longitude) const
{return (longitude / Pixel_Size) + Horizontal_Offset;}

/**	Convert a latitude value to an image y value.

	line = Oy - (latitude / Ps)

	where:

	Ps = {@link pixel_size() pixel size} in meters<br>
	Oy = image {@link vertical_offset() vertical offset} in pixels<br>

	@param	latitude	A latitude value in radians.
	@return	An image line (y) value in pixels.
*/
inline double to_image_Y_offset (double latitude) const
{return Vertical_Offset - (latitude / Pixel_Size);}

/**	Convert an image sample (x) value to horizontal offset in world
	units.

	longitude = (sample - Ox) * Ps

	where:

	Ps = {@link pixel_size() pixel size} in meters<br>
	Ox = image {@link horizontal_offset() horizontal offset} in pixels<br>

	@param	sample	An image sample (x) value.
	@return	A longitude value in radians.
*/
inline double to_world_X_offset (const double sample) const
{return (sample - Horizontal_Offset) * Pixel_Size;}

/**	Convert an image line (y) value to vertical offset in world
	units.

	latitude = (Oy - line) * Ps

	where:

	Ps = {@link pixel_size() pixel size} in meters<br>
	Ox = image {@link horizontal_offset() horizontal offset} in pixels<br>

	@param	sample	An image line (y) value.
	@return	A latitude value in radians.
*/
inline double to_world_Y_offset (const double line) const
{return (Vertical_Offset - line) * Pixel_Size;}

/**	Rotate an image coordinate from sample,line values to the rotated
	world coordinate space.

	Xr = Xi * cos (Rotation) - Yi * sin (Rotation)<br>
	Yr = Yi * cos (Rotation) + Xi * sin (Rotation)<br>

	where Xi,Yi are the image sample,line coordinate values, and Xr,Yr
	are the rotated world image values.

	Note: The cos (Rotation) and sin (Rotation) values have been
	pre-computed at the time the {@link rotation() Rotation} value
	was obtained from the {@link parameters(const idaeim::PVL::Aggregate*)
	pararameters}.

	@param	image_coordinate	A Coordinate to be rotated. The rotation
		is done in place.
*/
void rotate_from_image (Coordinate& image_coordinate) const;

/**	Rotate an image coordinate to sample,line values from the rotated
	world coordinate space.

	Xi = Xr * cos (Rotation) + Yr * sin (Rotation)<br>
	Yi = Yr * cos (Rotation) - Xr * sin (Rotation)<br>

	where Xr,Yr are the rotated world image values, and Xi,Yi are the
	image sample,line coordinate values.

	Note: The cos (Rotation) and sin (Rotation) values have been
	pre-computed at the time the {@link rotation() Rotation} value
	was obtained from the {@link parameters(const idaeim::PVL::Aggregate*)
	pararameters}.

	@param	image_coordinate	A Coordinate to be rotated. The rotation
		is done in place.
*/
void rotate_to_image (Coordinate& image_coordinate) const;

/*==============================================================================
	Data
*/
protected:

/**	PROJECTION_TYPE_PARAMETER_NAME

	Empty if no parameter found.

	@see	parameter_name()
*/
QString
	Projection_Name;

/**	NOT_APPLICABLE_CONSTANT_PARAMETER_NAME

	Set to DEFAULT_NA if no parameter found.
*/
double
	NA;

/**	POSITIVE_LONGITUDE_PARAMETER_NAME

	true if the value is POSITIVE_LONGITUDE_WEST_NAME.
	false if the value is POSITIVE_LONGITUDE_EAST_NAME.
		error if neither.
*/
bool
	Positive_West;

/**	LATITUDE_TYPE_PARAMETER_NAME

	true if the value is PLANETOCENTRIC_PROJECTION_NAME.
	false otherwise.
*/
bool
	Planetocentric;

//!	PIXEL_SIZE_PARAMETER_NAME (meters)
double
	Pixel_Size;

//!	HORIZONATAL_OFFSET_PARAMETER_NAME (pixels)
double
	Horizontal_Offset;
//!	VERTICAL_OFFSET_PARAMETER_NAME (pixels)
double
	Vertical_Offset;

//!	EQUITORIAL_RADIUS_PARAMETER_NAME (meters )
double
	Equitorial_Radius;
//!	POLAR_RADIUS_PARAMETER_NAME (meters)
double
	Polar_Radius;

//!	CENTER_LONGITUDE_PARAMETER_NAME (radians from degress)
double
	Center_Longitude;
//!	CENTER_LATITUDE_PARAMETER_NAME (radians from degress)
double
	Center_Latitude;

//!	PROJECTION_ROTATION_PARAMETER_NAME (radians from degress)
double
	Rotation;

//	Derived values:

/**	Flags that an identity Projection is not in effect.

	Set when {@link parameters(const idaeim::PVL::Aggregate*) parameters}
	are set and the PROJECTION_TYPE_PARAMETER_NAME and all required
	parameters are found. May also be set by a subclass as appropriate.
*/
bool
	Not_Identity;

/**	Body eccentricity.

	Requires EQUITORIAL_RADIUS_PARAMETER_NAME and POLAR_RADIUS_PARAMETER_NAME.
*/
double
	Eccentricity;

/**	Planetocentric to Planetographic conversion coefficient.

	Requires EQUITORIAL_RADIUS_PARAMETER_NAME and POLAR_RADIUS_PARAMETER_NAME.

	(Equitorial_Radius / Polar_Radius) * (Equitorial_Radius / Polar_Radius)
*/
double
	Coefficient_PC_to_PG;

/**	Planetographic to Planetocentric conversion coefficient.

	Requires EQUITORIAL_RADIUS_PARAMETER_NAME and POLAR_RADIUS_PARAMETER_NAME.

	(Polar_Radius / Equitorial_Radius) * (Polar_Radius / Equitorial_Radius)
*/
double
	Coefficient_PG_to_PC;

/**	Rotation cosine and sine values.

	If Rotation is zero or the INVALID_VALUE,
	Rotation_cos is set to 1 and ROTATION_SIN is set to 0.
*/
double
	Rotation_cos,
	Rotation_sin;

//!	Constants available for use in various calculations.
static const double
	PI,
	PI_OVER_2,
	PI_OVER_180,
	PI_UNDER_180
#ifndef DBL_EPSILON
	,DBL_EPSILON
#endif
;

};	//	End of Projection class.



}	//	namespace HiRISE
}	//	namespace UA
#endif
