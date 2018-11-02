/*	Image_Tile

HiROC CVS ID: $Id: Image_Tile.cc,v 1.20 2012/09/16 07:40:07 castalia Exp $

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

#include	"Image_Tile.hh"

#include	"Plastic_Image.hh"
#include	"HiView_Utilities.hh"

#include	<iostream>


#if defined (DEBUG_SECTION)
/*******************************************************************************
	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)

#define DEBUG_DEFAULT		DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
using std::clog;
#include	<iomanip>
using std::endl;
using std::boolalpha;

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
#include	<QList>
#endif
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
	Image_Tile::ID =
		"UA::HiRISE::Image_Tile ($Revision: 1.20 $ $Date: 2012/09/16 07:40:07 $)";


#ifndef DOXYGEN_PROCESSING
namespace
{
//	Descriptions of the tile status values.
static const QString
	Status_Description[] =
		{
		"Finished",
		"Low Priority",
		"High Priority",
		"Image Load"
		};
}
#endif

/*==============================================================================
	Tile Accounting
*/
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
namespace
{
QList<Image_Tile*>
	Tile_Accounting;
}

void
Image_Tile::tile_accounting ()
{
Plastic_Image
	*image;
int
	list_size = Tile_Accounting.size (),
	index,
	rindex;
clog << "    " << Tile_Accounting.size () << " extant Image_Tiles";
if (Tile_Accounting.size ())
	{
	clog << " -" << endl
		 << "    -------------------------------------" << endl;
	for (index = 0;
		 index < list_size;
		 index++)
		{
		image = Tile_Accounting[index]->Image;
		rindex = list_size;
		while (--rindex >= 0)
			if (rindex != index &&
				Tile_Accounting[rindex]->Image == image)
				break;
		if (rindex < 0)
			clog << "    ";
		else
			clog << "+!+ ";
		clog << index << ": " << *Tile_Accounting[index] << endl;
		}
	clog << "    -------------------------------------" << endl;
	}
else
	clog << endl;
}

#else
void
Image_Tile::tile_accounting ()
{}
#endif

/*==============================================================================
	Constructors
*/
Image_Tile::Image_Tile ()
	:	Image (NULL),
		Cancelable (true),
		Delete_Image_When_Done (false)
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
Tile_Accounting.append (this);
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< Image_Tile @ " << (void*)this << endl
	 << "    default " << *this << endl;
#endif
}


Image_Tile::Image_Tile
	(
	Plastic_Image*	image,
	const QPoint&	tile_coordinate,
	const QRect&	tile_region,
	bool			cancelable,
	bool			delete_when_done
	)
	:	Image (image),
		Tile_Coordinate (tile_coordinate),
		Tile_Region (tile_region),
		Cancelable (cancelable),
		Delete_Image_When_Done (delete_when_done)
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
Tile_Accounting.append (this);
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCK_LOG;
clog << ">-< Image_Tile @ " << (void*)this << endl
	 << "    from Plastic_Image " << *image << endl
	 << "    " << *this << endl;
tile_accounting ();
UNLOCK_LOG;
#endif
}


Image_Tile::Image_Tile
	(
	Plastic_Image*	image,
	const QPoint&	tile_coordinate,
	bool			cancelable,
	bool			delete_when_done
	)
	:	Image (image),
		Tile_Coordinate (tile_coordinate),
		Tile_Region (),
		Cancelable (cancelable),
		Delete_Image_When_Done (delete_when_done)
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
Tile_Accounting.append (this);
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCK_LOG;
clog << ">-< Image_Tile @ " << (void*)this << endl
	 << "    from Plastic_Image " << *image << endl
	 << "    " << *this << endl;
tile_accounting ();
UNLOCK_LOG;
#endif
}


Image_Tile::Image_Tile
	(
	const Image_Tile&	image_tile
	)
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
Tile_Accounting.append (this);
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">-< Image_Tile @ " << (void*)this << endl
	 << "    copy " << image_tile << endl;
#endif
*this = image_tile;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCK_LOG;
tile_accounting ();
UNLOCK_LOG;
#endif
}


Image_Tile&
Image_Tile::operator=
	(
	const Image_Tile&	image_tile
	)
{
if (this != &image_tile)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << ">-< Image_Tile:operator= @ " << (void*)this << endl
		 << "    assign " << image_tile << endl;
	#endif
	Image           = image_tile.Image;
	Tile_Coordinate = image_tile.Tile_Coordinate;
	Tile_Region     = image_tile.Tile_Region;
	Cancelable      = image_tile.Cancelable;
	Delete_Image_When_Done = image_tile.Delete_Image_When_Done;
	}
return *this;
}


Image_Tile::~Image_Tile ()
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
Tile_Accounting.removeAll (this);
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << ">>> Image_Tile::~Image_Tile @ " << (void*)this << endl
	 << "    " << *this << endl));
#endif
if (Delete_Image_When_Done &&
	Image)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	LOCKED_LOGGING ((
	clog << "--- delete " << *Image << endl));
	#endif
	delete Image;
	Image = NULL;
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
else
	{
	LOCKED_LOGGING ((
	clog << "... not deleted " << *Image << endl));
	}
LOCK_LOG;
tile_accounting ();
clog << "<<< Image_Tile::~Image_Tile" << endl;
UNLOCK_LOG;
#endif
}

/*==============================================================================
	Utility Functions
*/
std::ostream&
operator<<
	(
	std::ostream&		stream,
	const Image_Tile&	image_tile
	)
{
return stream
	<< "Image_Tile @ " << (void*)&image_tile
		<< " Image @ " << (void*)(image_tile.Image)
		<< " at " << image_tile.Tile_Coordinate
		<< " displays " << image_tile.Tile_Region
		<< " is " << (image_tile.Cancelable ?
			"" : "not ") << "cancelable"
		<< (image_tile.Delete_Image_When_Done ?
			"; delete image when done" : "");
}



}	//	namespace HiRISE
}	//	namespace UA
