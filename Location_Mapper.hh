/*	Location_Mapper

HiROC CVS ID: $Id: Location_Mapper.hh,v 1.5 2012/09/26 03:23:27 castalia Exp $

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

#ifndef HiView_Location_Mapper_hh
#define HiView_Location_Mapper_hh

//	Forward references.
namespace idaeim {
namespace PVL {
class Aggregate;
}}

class QString;


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Coordinate;
class Projection;

/**	A <i>Location_Mapper</i> manages the mapping between pixel coordinates
	and real world coordinates.

	The projection information is defined by a set of PDS metadata
	parameters. Parameters of known names are sought to provide the
	required values to assemble the projection definition. As usual, this
	dependency on parameter names can be a source of misunderstanding. If
	a required parameter is not found this will be reported in an
	expception that names the missing parameter.

	At the very least, an affine geo-transform array for the identity
	projection - in which image coordinates and world coordinates are
	identical - is available. PDS parameters may be provided that enable
	the identification and assembly of other projections.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.5 $
*/
class Location_Mapper
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
/**	Construct a Location_Mapper.

	<b/>N.B.</b>: Ownership of the parameters is not transferred to
	the Location_Mapper. The {@link parameters(const idaeim::PVL::Aggregate*)
	parameters} are only used to construct the
	tranformation matrix and spatial reference projection objects of
	the Location_Mapper.

	@param	parameters	A pointer to an idaeim::PVL::Aggregate that
		contains the PDS parameters defining the image pixel coordinate to
		real world coordinate projection. If NULL a {@link reset() default}
		Location_Mapper is constructed.
*/
explicit Location_Mapper (idaeim::PVL::Aggregate* parameters = 0);

/**	Copy a Location_Mapper object.

	@param	location_mapper	The Location_Mapper to be copied.
*/
Location_Mapper (const Location_Mapper& location_mapper);

/**	Assign another Location_Mapper object to this Location_Mapper object.

	@param	location_mapper	A Location_Mapper object to be assigned to
		this Location_Mapper object.
*/
Location_Mapper& operator= (const Location_Mapper& location_mapper);

/**	Destroy the Location_Mapper object.
*/
virtual ~Location_Mapper ();

/*==============================================================================
	Accessors
*/
/**	Get the name of the projection type.

	Any given type of projection may have several aliases for its name.
	The name returned by this method is the one that is specified by
	the PDS PROJECTION_TYPE_PARAMETER_NAME parameter.

	@return	A QString with the name of the projection type. If a
		projection was not initialized this will be an empty string.
*/
QString projection_name () const;

/**	Get the Projection object.

	@return	A pointer to the Projection object being used. This will be
		NULL if a Projection could not be created.
*/
inline Projection* projection () const
	{return Projector;}

/*==============================================================================
	Manipulators
*/
/**	Apply new PDS metadata parameters to redefine the location mapping
	{@link geo_transform() geo-transform} and {@link spatial_reference ()
	projection}.

	@param	parameters	A pointer to an idaeim::PVL::Aggregate that
		contains the parameters used to re-initialize the transform and
		spatial reference. If NULL the Location_Mapper is {@link reset()
		reset}.
	@return	This Location_Mapper object.
*/
Location_Mapper& parameters (const idaeim::PVL::Aggregate* parameters);

/**	Reset to the default initial state.

	The {@link projection() projection} is reset to the identity
	projection.
*/
void reset ();

/*==============================================================================
	Converters
*/
/**	Get the world longitude,latitude coordinate for an image sample,line
	coordinate.

	The conversion algorithm is:

	longitude = Center_Longitude + x / (R * cos (Center_Latitude))<br>
	latitude = y / R<br>

	where R is the {@link Projection::local_radius(double) radius} of the
	body at the Center_Latitude.

	@param	image_coordinate	The image sample,line coordinate.
	@return	The world longitude,latitude coordinate. The {@link
		Point2D#getX() x} value of the coordinate Point2D is the
		longitude; the {@link Point2D#getY() y} value is the latitude.
		Values are in degrees within the range 0-360.
*/
Coordinate project_to_world (const Coordinate& image_coordinate) const;

/**	Get the image sample,line coordinate for a world longitude,latitude
	coordinate.

	The conversion algorithm is:


	@param	world_coordinate	A world longitude,latitude Coordinate.
		The X value of the Coordinate is the longitude; the Y value is
		the latitude. Values are in degrees.
	@return	The image sample,line coordinate.
*/
Coordinate project_to_image (const Coordinate& world_coordinate) const;

/*==============================================================================
	Data
*/
private:

Projection
	*Projector;

};	//	End of Location_Mapper class.


}	//	namespace HiRISE
}	//	namespace UA
#endif
