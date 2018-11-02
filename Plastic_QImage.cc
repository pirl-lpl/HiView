/*	Plastic_QImage

HiROC CVS ID: $Id: Plastic_QImage.cc,v 1.30 2012/06/15 01:16:07 castalia Exp $

Copyright (C) 2009-2011  Arizona Board of Regents on behalf of the
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

#include	"Plastic_QImage.hh"

#include	<QPoint>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF			0
#define DEBUG_ALL			-1
#define DEBUG_CONSTRUCTORS	(1 << 0)
#define DEBUG_ACCESSORS		(1 << 1)
#define DEBUG_MANIPULATORS	(1 << 2)
#define DEBUG_HELPERS		(1 << 3)
#define DEBUG_UTILITIES		(1 << 4)
#define DEBUG_RENDER		(1 << 5)
#define DEBUG_PIXEL_DATA	(1 << 6)

#define DEBUG_DEFAULT		(DEBUG_ALL & ~DEBUG_PIXEL_DATA)

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	"HiView_Utilities.hh"

#include	<iostream>
using std::clog;
#include	<iomanip>
using std::endl;
using std::hex;
using std::dec;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Plastic_QImage::ID =
		"UA::HiRISE::Plastic_QImage ($Revision: 1.30 $ $Date: 2012/06/15 01:16:07 $)";

/*==============================================================================
	Constructors
*/
Plastic_QImage::Plastic_QImage
	(
	QImage*				image,
	const QSize&		size,
	const unsigned int*	band_map,
	const QTransform**	transforms,
	const Data_Map**	data_maps
	)
	:
	Plastic_Image (! size.isEmpty () ? size
		: (image ? image->size () : size),
		band_map, transforms, data_maps),
	Source (image ? image : new QImage (0, 0, IMAGE_FORMAT))
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Plastic_QImage @ " << (void*)this
		<< ": From QImage* @ " << (void*)image << endl;
if (image)
	clog << "        image size = " << image->size ()
	 	<< ", " << image->depth () << 'd' << endl;
clog << "    specified size = " << size
		<< (size.isEmpty () ? " (empty)" : "") << endl;
#endif

initialize ();

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Plastic_QImage" << endl;
#endif
}


Plastic_QImage::Plastic_QImage
	(
	const QImage&		image,
	const QSize&		size,
	const unsigned int*	band_map,
	const QTransform**	transforms,
	const Data_Map**	data_maps
	)
	:
	Plastic_Image (! size.isEmpty () ? size : image.size (),
		band_map, transforms, data_maps),
	Source (image.isNull () ?
		new QImage (0, 0, IMAGE_FORMAT) : new QImage (image))
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Plastic_QImage @ " << (void*)this
		<< ": From QImage& @ " << (void*)(&image) << endl
	 << "    new Source image @ " << (void*)Source << endl
	 << "        image size = " << image.size ()
	 	<< ", " << image.depth () << 'd' << endl
	 << "    specified size = " << size
		<< (size.isEmpty () ? " (empty)" : "") << endl;
#endif
initialize ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Plastic_QImage" << endl;
#endif
}


Plastic_QImage::Plastic_QImage
	(
	const Plastic_QImage&	image,
	const QSize&			size,
	Mapping_Type			shared_mappings
	)
	:
	Plastic_Image (size.isValid () ? size : image.size (),
		(shared_mappings & BAND_MAP)   ?
			image.source_band_map () : NULL,
		(shared_mappings & TRANSFORMS) ?
			const_cast<const QTransform**>(image.source_transforms ()) : NULL,
		(shared_mappings & DATA_MAPS)  ?
			const_cast<const Data_Map**>(image.source_data_maps ()) : NULL),
	Source (NULL)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Plastic_QImage @ " << (void*)this
		<< ": Copy @ " << (void*)(&image) << endl
	 << "    source image name - " << image.source_name () << endl
	 << "         image size = " << image.size ()
	 	<< ", " << image.depth () << 'd' << endl
	 << "     specified size = " << size
		<< (size.isEmpty () ? " (empty)" : "") << endl
	 << "    shared_mappings = " << shared_mappings
	 	<< ": " << mapping_type_names (shared_mappings) << endl;
#endif
if (image.Source)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "       source size = " << image.Source->size () << endl
		 << "      image source @ " << (void*)image.Source<< endl;
	#endif
	Source = new QImage (*image.Source);
	}
else
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    no image source" << endl;
	#endif
	Source = new QImage (0, 0, IMAGE_FORMAT);
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Source @ " << (void*)Source << endl;
#endif

Auto_Update = image.Auto_Update;
bool
	do_update = auto_update (false);

initialize ();

source_name (image.source_name ());

if (! (shared_mappings & BAND_MAP))
	source_band_map (image.source_band_map ());
if (! (shared_mappings & TRANSFORMS))
	source_transforms
		(const_cast<const QTransform**>(image.source_transforms ()));
if (! (shared_mappings & DATA_MAPS))
	source_data_maps
		(const_cast<const Data_Map**>(image.source_data_maps ()));

#if (DEBUG_SECTION)
background_color (qRgb (255, 255, 255));
#else
background_color (image.background_color ());
#endif

if ((Auto_Update = do_update))
	update ();
else
	fill (background_color ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Plastic_QImage" << endl;
#endif
}


Plastic_QImage::~Plastic_QImage ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> ~Plastic_QImage @ " << (void*)this << endl;
#endif
if (Source)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    delete Source @ " << (void*)Source << endl;
	#endif
	delete Source;
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< ~Plastic_QImage" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
Plastic_QImage*
Plastic_QImage::clone
	(
	const QSize&	size,
	Mapping_Type	shared_mappings
	) const
{return new Plastic_QImage (*this, size, shared_mappings);}


const void*
Plastic_QImage::source () const
{return Source;}


QSize
Plastic_QImage::source_size () const
{return Source ? Source->size () : QSize (0, 0);}


unsigned int
Plastic_QImage::source_bands () const
{
unsigned int
	bands = 0;
if (Source)
	{
	if (Source->depth () > 8)
		bands = 3;
	else
		bands = 1;
	}
return bands;
}


unsigned int
Plastic_QImage::source_precision_bits () const
{
unsigned int
	precision = 0;
if (Source)
	{
	switch (Source->format ())
		{
		case QImage::Format_Mono:
		case QImage::Format_MonoLSB:
			precision = 1;
			break;
		case QImage::Format_Indexed8:
		case QImage::Format_RGB32:
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
		case QImage::Format_RGB888:
			precision = 8;
			break;
		case QImage::Format_RGB16:
			precision = 16;
			break;
		case QImage::Format_RGB666:
		case QImage::Format_ARGB6666_Premultiplied:
			precision = 6;
			break;
		case QImage::Format_RGB555:
		case QImage::Format_ARGB8555_Premultiplied:
			precision = 5;
			break;
		case QImage::Format_RGB444:
		case QImage::Format_ARGB4444_Premultiplied:
			precision = 4;
		case QImage::Format_Invalid:
		case QImage::Format_ARGB8565_Premultiplied:
		default:
			precision = 0;
			break;
		}
	}
return precision;
}


Plastic_Image::Pixel_Datum
Plastic_QImage::source_pixel_value
	(
	unsigned int	x,
	unsigned int	y,
	unsigned int	band
	)
	const
{
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
clog << ">-< Plastic_QImage::source_pixel_value: "
		<< x << "x, " << y << "y, " << band << "b = ";
#endif
Pixel_Datum
	value;
if (Source &&
	x < static_cast<unsigned int>(Source->width ()) &&
	y < static_cast<unsigned int>(Source->height ()) &&
	band < 3)
	{
	#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
	clog << hex << Source->pixel (x, y) << dec << " -> ";
	#endif
	band = 16 - (band << 3);
	value = (Source->pixel (x, y) & (0xFF << band)) >> band;
	}
else
	value = UNDEFINED_PIXEL_VALUE;
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
clog << value << endl;
#endif
return value;
}


Plastic_Image::Triplet
Plastic_QImage::source_pixel
	(
	const QPoint&	point
	) const
{
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
clog << ">-< Plastic_QImage::source_pixel: " << point << " = ";
#endif
Triplet
	triplet;
if (Source &&
	point.x () < Source->width () &&
	point.y () < Source->height ())
	{
	QRgb
		datum = Source->pixel (point);
	#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
	clog << hex << datum << dec << " -> ";
	#endif
	triplet.Datum[0] = (datum & 0xFF0000) >> 16;
	triplet.Datum[1] = (datum & 0x00FF00) >> 8;
	triplet.Datum[2] =  datum & 0x0000FF;
	}
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
clog << triplet << endl;
#endif
return triplet;
}


}	//	namespace HiRISE
}	//	namespace UA
