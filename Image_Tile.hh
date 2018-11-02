/*	Image_Tile

HiROC CVS ID: $Id: Image_Tile.hh,v 1.19 2012/06/17 00:34:08 castalia Exp $

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

#ifndef HiView_Image_Tile_hh
#define HiView_Image_Tile_hh

#include	<QMetaType>
#include	<QPoint>
#include	<QRect>
#include	<QString>

#include	<iosfwd>


namespace UA
{
namespace HiRISE
{
//	Forward reference.
class Plastic_Image;

/**	An <i>Image_Tile</i> encapsulates a Plastic_Image and the
	characteristics associated with rendering the image.

	@author		Bradford Castaia, UA/HiROC
	@version	$Revision: 1.19 $
*/
class Image_Tile
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


/**	The {@link status() status of an Image_Tile.

	The LOW_PRIORITY status refers to a tile that contains a null tile
	coordinate. Such a tile is expected to be located beyond the bounds
	of the display viewport.

	The HIGH_PRIORITY status refers to a tile that contains a positive
	tile coordinate. Such a tile is expected to intersect the display
	viewport. The Image_Renderer will  place a high priority tile before
	low priority tiles in the Image_Rendering Queue of Image_Tiles.

	<b>N.B.</b>: Corresponding Image_Renderer status values depend on
	these values.
*/
enum
	{
	LOW_PRIORITY		= 1,
	HIGH_PRIORITY		= 2
	};
/*	>>> CAUTION <<< These values must be 1 and 2 because they are
	used by the Image_Renderer which relies on them.
*/

/*==============================================================================
	Constructors
*/
/**	Construct an empty Image_Tile.
*/
Image_Tile ();

/**	Construct an Image_Tile with a Plastic_Image and optional
	rendering characteristics.

	@param	image	A pointer to a Plastic_Image. This should not be
		null, but it is not an error if it is.
	@param	tile_coordinate	A QPoint specifying the location of the
		tile image in the display tile grid. The tile coordinate
		determines the {@link status() tile status}: A tile with a
		null coordinate is deemed low priority; otherwise the tile
		is high priority.
	@param	tile_region A QRect specifying the region of the tile image,
		in tile-relative coordinates, that is visible in the display
		viewport.
	@param	cancelable	If true rendering of the tile image may be
		canceled; otherwise the tile should be considered persistent in
		the rendering queue when rendering is cancelled.
	@param	delete_when_done	If true the tile's Plastic_Image is to
		be deleted when the tile is destroyed.
*/
Image_Tile (Plastic_Image* image,
	const QPoint& tile_coordinate = QPoint (),
	const QRect& tile_region = QRect (),
	bool cancelable = true, bool delete_when_done = false);

/**	Construct an Image_Tile with a Plastic_Image and
	rendering characteristics.

	The tile will have a null Tile_Region which make it low priority.

	@param	image	A pointer to a Plastic_Image. This should not be
		null, but it is not an error if it is.
	@param	tile_coordinate	A QPoint specifying the location of the
		tile image in the display tile grid. The tile coordinate
		determines the {@link status() tile status}: A tile with a
		null coordinate is deemed low priority; otherwise the tile
		is high priority.
	@param	cancelable	If true rendering of the tile image may be
		canceled; otherwise the tile should be considered persistent in
		the rendering queue when rendering is cancelled.
	@param	delete_when_done	If true the tile's Plastic_Image is to
		be deleted when the tile is destroyed.
*/
Image_Tile (Plastic_Image* image,
	const QPoint& tile_coordinate,
	bool cancelable = true, bool delete_when_done = false);

/**	Construct an Image_Tile from a copy of another Image_Tile.

	The Image pointer and all rendering characteristics are copied.

	@param	image_tile	A reference to the Image_Tile to be copied.
*/
Image_Tile (const Image_Tile& image_tile);

/**	Assign the content of another Image_Tile to this Image_Tile.

	The Image pointer and all rendering characteristics are copied.
	The previous Image is never deleted.

	@param	image_tile	A reference to the Image_Tile to be assigned.
*/
Image_Tile& operator= (const Image_Tile& image_tile);

/**	Delete the Image_Tile.

	If Delete_Image_When_Done is true the Plastic_Image pointed to by
	Image is deleted.
*/
~Image_Tile ();

/*==============================================================================
	Accessors
*/
/**	Get the priority status of the tile.

	@return	LOW_PRIORITY if the tile {@link is_low_priority() is low
		priority}; HIGH_PRIORITY otherwise.
*/
inline int status () const
	{return (is_low_priority () ? LOW_PRIORITY : HIGH_PRIORITY);}

/**	Test if tile has low priority status.

	@return	true if the Tile_Coordinate x value is zero; false otherwise.
	@see	status()
*/
inline bool is_low_priority () const
	{return Tile_Coordinate.x () == 0;}

/**	Test if tile has high priority status.

	@return	true if the Tile_Coordinate x value greater than zero;
		false otherwise.
	@see	status()
*/
inline bool is_high_priority () const
	{return Tile_Coordinate.x () > 0;}

/**	Get the area of the tile visible in the display viewport.

	@return	The area of the tile visible in the display viewport; i.e.
		the Tile_Region width times height. Note that a {@link
		is_low_priority() low priority} tile will always have a zero
		area.
*/
inline unsigned long long area () const
	{return (unsigned long long)Tile_Region.width () * Tile_Region.height ();}

/*==============================================================================
	Tile Accounting
*/
/*	Print a list of extant Image_Tiles.

	<b>N.B.</b>: This method is a no-op unless the Image_Tile class has
	been compiled with DEBUG defined.

	When an Image_Tile is constructed it is appended to the list of
	extant Image_Tiles and when it is destroyed it is removed from
	the list.
*/
static void tile_accounting ();

/*==============================================================================
	Data Members
*/
//	The image that renders this tile.
Plastic_Image*
	Image;

//	The coordinate of the tile in the tile grid.
QPoint
	Tile_Coordinate;

//	The tile region (tile-relative) that is visible in the display viewport.
QRect
	Tile_Region;

//	Flag to indicate if the Renderer can cancel rendering the tile image.
bool
	Cancelable;

//	Flag to indicate if the Image is to be deleted when the tile is destroyed.
bool
	Delete_Image_When_Done;
};

/*==============================================================================
	Utility Functions
*/
/**	Image_Tile output operator.

	The description printed to the output stream is contained on a single
	line of the form:

	Image_Tile @ <tile address>
		Image @ <Image address>
		at <Tile_Coordinate>
		displays <Tile_Region>
		is [not] <Cancelable> [; <Delete_Image_When_Done>]

	@param	stream	A std::ostream reference.
	@param	image_tile	An Image_Tile reference.
	@return	The stream reference.
*/
std::ostream& operator<< (std::ostream& stream,
	const Image_Tile& image_tile);

}	//	namespace HiRISE
}	//	namespace UA

//!	Qt meta-type declaration.
Q_DECLARE_METATYPE(UA::HiRISE::Image_Tile);

#endif
