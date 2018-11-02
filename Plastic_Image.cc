/*	Plastic_Image

HiROC CVS ID: $Id: Plastic_Image.cc,v 2.38 2014/05/27 17:13:51 guym Exp $

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

#include	"Plastic_Image.hh"

#include	"HiView_Utilities.hh"

#include	<QString>
#include	<QSize>
#include	<QSizeF>
#include	<QPoint>
#include	<QPointF>
#include	<QVector>
#include	<QTransform>

#include	<QMutex>
#include	<QMutexLocker>
#include	<QThread>

//	For the mark_image function.
#include	<QPainter>
#include	<QRect>
#include	<QPen>

#include	"PVL.hh"
using idaeim::PVL::Parser;
using idaeim::PVL::Parameter;
using idaeim::PVL::Aggregate;
using idaeim::PVL::Assignment;
using idaeim::PVL::Value;
using idaeim::PVL::Integer;
using idaeim::PVL::String;

#include	<cmath>
#include	<iostream>
using std::clog;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::dec;
using std::hex;
using std::setw;
using std::setfill;
using std::endl;
#include	<stdexcept>
using std::invalid_argument;
using std::out_of_range;
using std::logic_error;
using std::runtime_error;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_TRANSFORMS		(1 << 1)
#define DEBUG_DATA_MAPPING		(1 << 2)
#define DEBUG_DATA_MAP			(1 << 3)
#define DEBUG_BAND_MAP			(1 << 4)
#define DEBUG_ACCESSORS			(DEBUG_TRANSFORMS | \
								 DEBUG_DATA_MAPS | \
								 DEBUG_BAND_MAP)
#define DEBUG_MANIPULATORS		(1 << 5)
#define DEBUG_HELPERS			(1 << 6)
#define DEBUG_RENDER			(1 << 7)
#define DEBUG_NOTIFY			(1 << 8)
#define DEBUG_PROMPT			(1 << 18)
#define DEBUG_PIXEL_DATA		(1 << 9)
#define	DEBUG_ONE_LINE			(1 << 10)
#define DEBUG_UPDATE			(1 << 11)
#define DEBUG_HISTOGRAMS		(1 << 12)
#define DEBUG_PRINT_HISTOGRAMS	(1 << 13)
#define DEBUG_LOCATION			(1 << 14)
#define DEBUG_METADATA			(1 << 15)

#define DEBUG_TILE_MARKINGS		(1 << 20)
#define TILE_MARKINGS_Y				36
#define TILE_MARKINGS_COLOR			Qt::green

#define DEBUG_DEFAULT		(DEBUG_ALL & ~DEBUG_PIXEL_DATA & ~DEBUG_DATA_MAP)

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#else

#include	<string>
using std::string;
using std::cin;
using std::boolalpha;
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
	Plastic_Image::ID =
		"UA::HiRISE::Plastic_Image ($Revision: 2.38 $ $Date: 2014/05/27 17:13:51 $)";


/*	>>> WARNING <<< A 32-bit image pixel data format is required.
	The QImage::Format_RGB32 also ensures that the MSB (alpha band)
	will always be FF so that QRgb values of zero can be distinguished
	as an invalid pixel datum.
*/
#ifndef PLASTIC_IMAGE_FORMAT
#define PLASTIC_IMAGE_FORMAT			QImage::Format_RGB32
#endif
const QImage::Format
	Plastic_Image::IMAGE_FORMAT			= PLASTIC_IMAGE_FORMAT;

#ifndef IMAGE_METADATA_GROUP_NAME
#define IMAGE_METADATA_GROUP_NAME			"Image"
#endif
Plastic_Image::Name_String
	Plastic_Image::IMAGE_METADATA_GROUP		= IMAGE_METADATA_GROUP_NAME,
	Plastic_Image::IMAGE_SOURCE_PARAMETER	= "Source",
	/*
		These parameter names should be the same as those for JP2_Metadata.
	*/
	Plastic_Image::IMAGE_WIDTH_PARAMETER	= "Width",
	Plastic_Image::IMAGE_HEIGHT_PARAMETER	= "Height",
	Plastic_Image::IMAGE_BANDS_PARAMETER	= "Image_Bands",
	Plastic_Image::VALUE_BITS_PARAMETER		= "Value_Bits";

const Plastic_Image::Pixel_Datum
	Plastic_Image::UNDEFINED_PIXEL_VALUE	= (Plastic_Image::Pixel_Datum)-1;

const unsigned int
	Plastic_Image::MAXIMUM_PIXEL_DATUM_PRECISION	= 16;

const Plastic_Image::Mapping_Type
	Plastic_Image::NO_MAPPINGS			= 0,
	Plastic_Image::IDENTICAL_MAPPINGS	= NO_MAPPINGS,
	Plastic_Image::BAND_MAP				= (1 << 0),
	Plastic_Image::TRANSFORMS			= (1 << 1),
	Plastic_Image::DATA_MAPS			= (1 << 2),
	Plastic_Image::ALL_MAPPINGS			= (BAND_MAP | TRANSFORMS | DATA_MAPS);


QString
Plastic_Image::mapping_type_names
	(
	Mapping_Type	mapping_type
	)
{
QString
	names;
if (mapping_type)
	{
	if (mapping_type & BAND_MAP)
		names += "BAND_MAP";
	if (mapping_type & TRANSFORMS)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "TRANSFORMS";
		}
	if (mapping_type & DATA_MAPS)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "DATA_MAPS";
		}
	}
else
	names = "NO_MAPPINGS";
return names;
}


const QString
	Plastic_Image::Rendering_Monitor::Status_Message[] =
		{
		//	0: INFO_ONLY
		"Information only.",
		//	1: LOW_QUALITY_DATA
		"Rendering low quality pixel data.",
		//	2: TOP_QUALITY_DATA
		"Rendering top quality pixel data.",
		//	3: RENDERED_DATA_MASK
		"Rendering pixel data.",
		//	4: CANCELED
		"Data rendering was canceled.",
		//	unused
		"Low quality rendering canceled.",
		"Top quality rendering canceled.",
		"Rendering canceled.",
		//	8: DONE
		"Data rendering is done.",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		//	DATA_MAPPING
		"Mapping source data to display data.",
		"",
		"Mapping source data to display data."
		};

/*------------------------------------------------------------------------------
	Defaults
*/
#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)				"" #__VA_ARGS__ ""
#define AS_STRING(...)					STRINGIFIED(__VA_ARGS__)
#endif

#ifndef DEFAULT_BACKGROUND_COLOR
#ifndef DEBUG_SECTION
#define DEFAULT_BACKGROUND_COLOR		transparent
#else
#define DEFAULT_BACKGROUND_COLOR		green
#endif
#endif
#define _DEFAULT_BACKGROUND_COLOR_		AS_STRING(DEFAULT_BACKGROUND_COLOR)
QRgb
	Plastic_Image::Default_Background_Color
		= QColor (_DEFAULT_BACKGROUND_COLOR_).rgba ();

#ifndef DEFAULT_AUTO_UPDATE
#define DEFAULT_AUTO_UPDATE				false
#endif
bool
	Plastic_Image::Default_Auto_Update	= DEFAULT_AUTO_UPDATE;

#ifndef DEFAULT_RENDERING_INCREMENT_LINES
#define DEFAULT_RENDERING_INCREMENT_LINES	100
#endif
unsigned int
	Plastic_Image::Default_Rendering_Increment_Lines
		= DEFAULT_RENDERING_INCREMENT_LINES;

/*==============================================================================
	Constructors
*/
Plastic_Image::Plastic_Image
	(
	const QSize&		image_size,
	const unsigned int*	band_map,
	const QTransform**	transforms,
	const Data_Map**	data_maps
	)
	:	QImage ((image_size.isValid () ?
			image_size : QSize (0, 0)), IMAGE_FORMAT),
		Object_Lock (QMutex::Recursive),
		Metadata (NULL),
		Image_Metadata (NULL),
		Update (*this),
		Source_Name (),
		Auto_Update (Default_Auto_Update),

		//	Rendering variables:
		Band_Map (const_cast<unsigned int*>(band_map)),
		Geo_Transforms (const_cast<QTransform**>(transforms)),
		Data_Maps (const_cast<Data_Map**>(data_maps)),
		Locally_Owned_and_Operated (NO_MAPPINGS),
		Mapping_Differences (IDENTICAL_MAPPINGS),
		Needs_Update (ALL_MAPPINGS),
		Needs_Update_Shadow (NO_MAPPINGS),
		Rendering_Increment_Lines (Default_Rendering_Increment_Lines),
		Background_Color (Default_Background_Color),
		Closed (false),
		Rendering (NULL),
		Cancel_Update (false)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Plastic_Image @ " << (void*)this << endl
	 << "          size = " << size () << endl
	 << "      band_map @ " << (void*)band_map << endl
	 << "    transforms @ " << (void*)transforms << endl
	 << "     data_maps @ " << (void*)data_maps << endl;
#endif
#if (DEBUG_SECTION)
clog << boolalpha;
#endif

//	N.B.: The initialization resets will provide the local mappings.
if (! Band_Map)
	Locally_Owned_and_Operated |= BAND_MAP;
if (Geo_Transforms)
	Mapping_Differences |= different_transforms ();
else
	Locally_Owned_and_Operated |= TRANSFORMS;
if (Data_Maps)
	Mapping_Differences |= different_data_maps ();
else
	Locally_Owned_and_Operated |= DATA_MAPS;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    Locally_Owned_and_Operated = "
		<< mapping_type_names (Locally_Owned_and_Operated)
		<< " (" << Locally_Owned_and_Operated << ')' << endl
	 << "<<< Plastic_Image" << endl;
#endif
}


void
Plastic_Image::initialize ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Plastic_Image::initialize" << endl
	 << "    Locally_Owned_and_Operated = "
		<< mapping_type_names (Locally_Owned_and_Operated)
		<< " (" << Locally_Owned_and_Operated << ')' << endl;
#endif

/*	Detach any shared image data.

	A QImage uses an implicitly shared buffer for its pixel data.
	Normally this means that a QImage that is a copy of another QImage
	will share the same pixel data buffer until a method is called that
	will write new pixel values (e.g. setPixel) or anticipates that new
	pixel data will be written (e.g. scanLine), at which point a copy of
	the shared data buffer is made for local use.

	However, deferring the provision of a local pixel data buffer to the
	point at which pixel data will be written (by a scanLine call)
	creates the possibility that the shared data buffer of the QImage is
	in use at that point in another thread. This, of couse, will cause a
	crash. This QImage is detached from any shared data buffer - a local
	data buffer is provided - here to prevent this possibility.

	N.B.: The QImage isDetached and detach methods are undocumented!
*/
if (! size ().isEmpty () &&
	! isDetached ())
	{
	detach ();
	if (! scanLine (0))
		{
		//	Failed to obtain a local pixel data buffer; out of memory.
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to obtain the required pixel data buffer" << endl
			<< "while initializing a new image object.";
		throw runtime_error (message.str ());
		}
	}

#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_TILE_MARKINGS))
//	Green background for debbugging.
background_color (qRgb (0, 255, 0));
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    background_color = #" << setw (8) << setfill ('0') << hex
		<< background_color () << dec << setfill (' ') << endl;
#endif

bool
	do_update = auto_update (false);
if (Locally_Owned_and_Operated & TRANSFORMS ||
	! Geo_Transforms)
	source_transform_reset ();
if (Locally_Owned_and_Operated & BAND_MAP ||
	! Band_Map)
	source_band_map_reset ();
if (Locally_Owned_and_Operated & DATA_MAPS ||
	! Data_Maps)
	source_data_map_reset ();
if (size ().isEmpty ())
	Needs_Update = NO_MAPPINGS;

if ((Auto_Update = do_update))
	update ();
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Plastic_Image:initialize" << endl;
#endif
}


Plastic_Image::~Plastic_Image ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> ~Plastic_Image @ " << (void*)this << endl;
#endif
QMutexLocker
	object_lock (&Object_Lock);

if (Metadata)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    delete Metadata @ " << (void*)Metadata << endl;
	#endif
	delete Metadata;
	Metadata = NULL;
	}

int
	band;
if (Locally_Owned_and_Operated & BAND_MAP)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    delete Band_Map @ " << (void*)Band_Map << endl;
	#endif
	delete[] Band_Map;
	}
if (Locally_Owned_and_Operated & TRANSFORMS &&
	Geo_Transforms)
	{
	band = -1;
	while (++band < 3)
		{
		if (Geo_Transforms[band])
			{
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "    delete Geo_Transforms[" << band << "] @ "
					<< (void*)Geo_Transforms[band] << endl;
			#endif
			delete Geo_Transforms[band];
			}
		}
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    delete Geo_Transforms @ " << (void*)Geo_Transforms << endl;
	#endif
	delete[] Geo_Transforms;
	}
if (Locally_Owned_and_Operated & DATA_MAPS &&
	Data_Maps)
	{
	band = -1;
	while (++band < 3)
		{
		if (Data_Maps[band])
			{
			#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
			clog << "    delete Data_Maps[" << band << "] @ "
					<< (void*)Data_Maps[band] << endl;
			#endif
			delete Data_Maps[band];
			}
		}
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    delete Data_Maps @ " << (void*)Data_Maps << endl;
	#endif
	delete[] Data_Maps;
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< ~Plastic_Image" << endl;
#endif
}

/*==============================================================================
	Metadata
*/
bool
Plastic_Image::source_name
	(
	const QString&	name
	)
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_METADATA)
clog << ">>> Plastic_Image::source_name: " << name << endl;
#endif
bool
	changed = false;
if (! closed ())
	{
	if ((changed = Source_Name != name))
		{
		#if ((DEBUG_SECTION) & DEBUG_METADATA)
		clog << "    changed from " << Source_Name << endl;
		#endif
		Source_Name = name;

		if (Image_Metadata)
			{
			Parameter*
				parameter (Image_Metadata->find (IMAGE_SOURCE_PARAMETER));
			#if ((DEBUG_SECTION) & DEBUG_METADATA)
			clog << "    Metadata @ " << (void*)Metadata << endl
				 << "    " << IMAGE_SOURCE_PARAMETER << " parameter "
				 	<< (parameter ? "updated" : "added") << endl;
			#endif
			if (parameter)
				*parameter = String (Source_Name.toStdString (), String::TEXT);
			else
				{
				//	Insert missing Source parameter (shouldn't happen).
				parameter = new Assignment (IMAGE_SOURCE_PARAMETER);
				*parameter = String (Source_Name.toStdString (), String::TEXT);
				Image_Metadata->poke (Metadata->begin (), parameter);
				}

			//	Send change notification.
			notify_metadata_monitors ();
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_METADATA)
clog << "<<< Plastic_Image::source_name: " << changed << endl;
#endif
return changed;
}

idaeim::PVL::Aggregate*
Plastic_Image::metadata ()
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_METADATA)
clog << ">>> Plastic_Image::metadata" << endl;
#endif

if (! Metadata)
	{
	Metadata = new Aggregate ("Metadata");
	Image_Metadata = new Aggregate (IMAGE_METADATA_GROUP);
	#if ((DEBUG_SECTION) & DEBUG_METADATA)
	clog << "          Metadata @ " << (void*)Metadata << endl
		 << "    Image_Metadata @ " << (void*)Image_Metadata << endl;
	#endif
	QSize
		image_size (source_size ());
	(*Image_Metadata)
		.add (Assignment (IMAGE_SOURCE_PARAMETER)
			= String (source_name ().toStdString (), String::TEXT))
		.add (Assignment (IMAGE_WIDTH_PARAMETER)
			= Integer (image_size.width (), Value::UNSIGNED).units ("columns"))
		.add (Assignment (IMAGE_HEIGHT_PARAMETER)
			= Integer (image_size.height (), Value::UNSIGNED).units ("rows"))
		.add (Assignment (IMAGE_BANDS_PARAMETER)
			= Integer (source_bands (), Value::UNSIGNED))
		.add (Assignment (VALUE_BITS_PARAMETER)
			= Integer (source_precision_bits (), Value::UNSIGNED).units ("bits"));
	Metadata->poke_back (Image_Metadata);
	#if ((DEBUG_SECTION) & DEBUG_METADATA)
	clog << "          Metadata -" << endl
		 << *Metadata;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_METADATA)
clog << "<<< Plastic_Image::metadata @ " << (void*)Metadata << endl;
#endif
return Metadata;
}


bool
Plastic_Image::add_metadata_monitor
	(
	Metadata_Monitor*	monitor
	)
{
bool
	added = false;
if (monitor)
	{
	QMutexLocker
		object_lock (&Object_Lock);
	if (! Closed &&
		! Metadata_Monitors.contains (monitor))
		{
		Metadata_Monitors.append (monitor);
		added = true;
		}
	}
return added;
}


bool
Plastic_Image::remove_metadata_monitor
	(
	Metadata_Monitor*	monitor
	)
{
QMutexLocker
	object_lock (&Object_Lock);
return Metadata_Monitors.removeOne (monitor);
}


void
Plastic_Image::notify_metadata_monitors ()
{
QMutexLocker
	object_lock (&Object_Lock);
if (! Closed)
	{
	int
		entry = Metadata_Monitors.size ();
	while (entry-- > 0)
		Metadata_Monitors[entry]->metadata_changed (*this);
	}
}

/*------------------------------------------------------------------------------
	Close
*/
void
Plastic_Image::close ()
{
QMutexLocker
	object_lock (&Object_Lock);
Closed = true;
}


bool
Plastic_Image::closed () const
{
QMutexLocker
	object_lock (&Object_Lock);
return Closed;
}

/*------------------------------------------------------------------------------
	Source pixel
*/
//	Default inplementation if not implemented more efficiently by subclasses.
Plastic_Image::Triplet
Plastic_Image::source_pixel
	(
	const QPoint&	point
	) const
{
unsigned int
	x = static_cast<unsigned int>(point.x ()),
	y = static_cast<unsigned int>(point.y ());
return Triplet
	(source_pixel_value (x, y, 0),
	 source_pixel_value (x, y, 1),
	 source_pixel_value (x, y, 2));
}

/*------------------------------------------------------------------------------
	Source band by display band mapping
*/
bool
Plastic_Image::source_band_map
	(
	const unsigned int*	band_map,
	bool				shared
	)
{
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << ">>> Plastic_Image::source_band_map:" << band_map << endl
	 << "    shared = " << shared << endl
	 << "    Band_Map" << Band_Map << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "<<< Plastic_Image::source_band_map: false" << endl;
	#endif
	return false;
	}

bool
	changed = false;
unsigned int
	band = -1,
	bands = source_bands ();

if (! band_map)
	{
	if (! (Locally_Owned_and_Operated & BAND_MAP))
		{
		//	Revert to local band map.
		Locally_Owned_and_Operated |= BAND_MAP;
		if (! (band_map = const_cast<const unsigned int*>(Band_Map)))
			{
			//	No shared map; initialize the local map.
			#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
			clog << "    initialize new Band_Map" << endl;
			#endif
			Band_Map = new unsigned int[3];
			reset_band_map (Band_Map);
			changed = true;
			}
		else
			{
			//	Replace the shared map with a local map.
			Band_Map = new unsigned int[3];
			#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
			clog << "    replace shared band map" << band_map << endl;
			#endif
			while (++band < 3)
				Band_Map[band] = band_map[band];
			}
		}
	changed |= needs_update (changed ? BAND_MAP : NO_MAPPINGS);
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    Band_Map" << Band_Map << endl
		 << "<<< Plastic_Image::source_band_map: " << changed << endl;
	#endif
	return changed;
	}

//	Check map validity.
while (++band < 3)
	{
	if (band_map[band] >= bands)
		{
		Update.end (Update.SEQUENCE_END);
		ostringstream
			message;
		message
			<< ID << endl
			<< "Invalid source band map element "
				<< band << " - " << band_map[band] << endl
			<< "The image source only has " << bands
				<< " band" << plural (bands) << '.';
		throw invalid_argument (message.str ());
		}
	if (Band_Map[band] != band_map[band])
		{
		changed = true;
		if (! shared)
			Band_Map[band] = band_map[band];
		}
	}

if (shared)
	{
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    delete Band_Map @ " << (void*)Band_Map << endl;
	#endif
	if (band_map != Band_Map &&
		(Locally_Owned_and_Operated & BAND_MAP))
		{
		#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
		clog << "    delete Band_Map @ " << (void*)Band_Map << endl;
		#endif
		delete[] Band_Map;
		Locally_Owned_and_Operated &= ~BAND_MAP;
		}
	Band_Map = const_cast<unsigned int*>(band_map);
	}

changed |= needs_update (changed ? BAND_MAP : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << "    Band_Map" << Band_Map << endl
	 << "<<< Plastic_Image::source_band_map: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::source_band_map_reset ()
{
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << ">>> Plastic_Image::source_band_map_reset" << endl;
#endif
Update.start ();
if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "<<< Plastic_Image::source_band_map_reset: false" << endl;
	#endif
	return false;
	}

if (! Band_Map)
	{
	//	Create a new local band map.
	Band_Map = new unsigned int[3];
	Band_Map[0] = (unsigned int)-1;	//	Ensure changed will be true;
	Band_Map[1] = (unsigned int)-1;	//	VALGRIND
	Band_Map[2] = (unsigned int)-1;	//	VALGRIND
	Locally_Owned_and_Operated |= BAND_MAP;
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    initialize Band_Map" << endl;
	#endif
	}
//	Reset the band map.
bool
	changed = reset_band_map (Band_Map);

changed |= needs_update (changed ? BAND_MAP : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << "<<< Plastic_Image::source_band_map_reset: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::reset_band_map
	(
	unsigned int*	band_map
	)
{
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << ">>> Plastic_Image::reset_band_map:" << band_map << endl;
#endif
bool
	changed = false;
if (band_map)
	{
	unsigned int
		band = 0,
		bands = source_bands ();
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    source_bands = " << bands << endl;
	#endif
	if (bands > 3)
		//	Use up to the first three source bands.
		bands = 3;

	while (band < bands)
		{
		if (band_map[band] != band)
			{
			changed = true;
			band_map[band] = band;
			}
		++band;
		}

	if (bands < 3)
		{
		if (bands)	//	If no source bands just use zero.
			bands--;
		//	Map the last band onto the remaining image bands.
		#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
		clog << "    Mapping bands " << band << "-2 to " << bands << endl;
		#endif
		while (band < 3)
			{
			if (band_map[band] != bands)
				{
				changed = true;
				band_map[band] = bands;
				}
			++band;
			}
		}
	#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
	clog << "    band_map @ " << (void*)band_map
			<< ": " << band_map[0]
			<< ", " << band_map[1]
			<< ", " << band_map[2] << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << "    band map" << band_map << endl
	 << "<<< Plastic_Image::reset_band_map: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::different_band_map
	(
	const unsigned int*	band_map
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
if (! band_map)
	return Band_Map != NULL;
if (! Band_Map)
	return true;
int
	band = source_bands ();
while (band--)
	if (Band_Map[band] != band_map[band])
		return true;
return false;
}


int
Plastic_Image::source_band
	(
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
if (! Band_Map)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain a source band for band " << band << endl
		<< "because no band map has been allocated!";
	throw logic_error (message.str ());
	}
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain a source band for band " << band << endl
		<< "because there are only image bands 0-2.";
	throw invalid_argument (message.str ());
	}
return Band_Map[band];
}

/*------------------------------------------------------------------------------
	Geometric transformation
*/
bool
Plastic_Image::set_source_transform
	(
	const QTransform&	transform,
	int					band
	)
{
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::set_source_transform:" << endl
	 << transform
	 << "    band = " << band << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "<<< Plastic_Image::set_source_transform: false" << endl;
	#endif
	return false;
	}

bool
	changed = false;
if (band < 0)
	{
	for (band = 0;
		 band < 3;
		 band++)
		{
		if (*Geo_Transforms[band] != transform)
			{
			changed = true;
			*Geo_Transforms[band] = transform;
			}
		}
	Mapping_Differences &= ~TRANSFORMS;
	}
else
if (band < 3)
	{
	if (*Geo_Transforms[band] != transform)
		{
		changed = true;
		*Geo_Transforms[band] = transform;
		Mapping_Differences &= ~TRANSFORMS;
		Mapping_Differences |= different_transforms ();
		}
	}
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "    Mapping_Differences = " << Mapping_Differences << endl;
#endif

changed |= needs_update (changed ? TRANSFORMS : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::set_source_transform: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::source_transforms
	(
	const QTransform**	transforms,
	bool				shared
	)
{
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_transforms: @ "
		<< (void*)transforms << endl
	 << "    shared = " << shared << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "<<< Plastic_Image::source_transforms: false" << endl;
	#endif
	return false;
	}

bool
	changed = false,
	locally_owned = Locally_Owned_and_Operated & TRANSFORMS;
int
	band = -1;

if (transforms)
	{
	if (shared)
		{
		//	Check for changed transforms.
		while (++band < 3)
			{
			if (transforms[band] != Geo_Transforms[band])
				changed |= (*transforms[band] != *Geo_Transforms[band]);
			if (locally_owned &&
				Geo_Transforms[band])
				delete Geo_Transforms[band];
			}

		//	Set the shared transforms.
		if (locally_owned)
			delete[] Geo_Transforms;
		Geo_Transforms = const_cast<QTransform**>(transforms);
		Locally_Owned_and_Operated &= ~TRANSFORMS;

		//	Check for different transforms.
		Mapping_Differences &= ~TRANSFORMS;
		Mapping_Differences |= different_transforms ();
		}
	else
		{
		//	Apply each transform.
		bool
			do_update = auto_update (false);
		while (++band < 3)
			{
			if (transforms[band])
				changed |= set_source_transform (*transforms[band], band);
			else
				changed |= set_source_transform (QTransform (), band);
			}
		auto_update (do_update);
		}
	}
else
if (! locally_owned)
	{
	//	Revert to local transforms.
	if (! (transforms = const_cast<const QTransform**>(Geo_Transforms)))
		changed = source_transform_reset ();
	else
		{
		if (! Geo_Transforms)
			Geo_Transforms = new QTransform*[3];
		#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
		clog << "    local Geo_Transforms @ " << (void*)Geo_Transforms << endl
			 << "          replace shared @ " << (void*)transforms << endl;
		#endif
		Locally_Owned_and_Operated |= TRANSFORMS;
		while (++band < 3)
			Geo_Transforms[band] = new QTransform (*transforms[band]);
		}
	}

changed |= needs_update (changed ? TRANSFORMS : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_transforms: " << changed << endl;
#endif
return changed;
}


QTransform*
Plastic_Image::source_transform
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_transform: " << band << endl;
#endif
QMutexLocker
	object_lock (&Object_Lock);
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain a source transform for band " << band << endl
		<< "because there are only transforms for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_transform" << endl;
#endif
return Geo_Transforms[band];
}


QTransform
Plastic_Image::invert_keeping_origin
	(
	const QTransform&	transform
	)
{
QTransform
	xform
		(transform.m11 (), transform.m12 (), transform.m13 (),
		 transform.m21 (), transform.m22 (), transform.m23 (),
		//	Remove the origin offset.
		 0,             0,                   transform.m33 ());
//	Invert the transform matrix.
bool
	invertible;
xform = xform.inverted (&invertible);
if (! invertible)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Unable to invert the geometric transformation -"
		<< transform;
	throw runtime_error (message.str ());
	}
//	Restore the origin offset.
xform.setMatrix
	(xform.m11 (),     xform.m12 (),     xform.m13 (),
	 xform.m21 (),     xform.m22 (),     xform.m23 (),
	 transform.m31 (), transform.m32 (), xform.m33 ());
return xform;
}


QTransform
Plastic_Image::invert_clipping_origin
	(
	const QTransform&	transform
	)
{
QTransform
	xform
		(transform.m11 (), transform.m12 (), transform.m13 (),
		 transform.m21 (), transform.m22 (), transform.m23 (),
		//	Remove the origin offset.
		 0,             0,                   transform.m33 ());
//	Invert the transform matrix.
bool
	invertible;
xform = xform.inverted (&invertible);
if (! invertible)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Unable to invert the geometric transformation -"
		<< transform;
	throw runtime_error (message.str ());
	}
//	Clip the origin offset to its fractional part.
xform.setMatrix
	(xform.m11 (),     xform.m12 (),     xform.m13 (),
	 xform.m21 (),     xform.m22 (),     xform.m23 (),
	((transform.m31 () < 0.0) ?
	 	(floor (transform.m31 ()) - transform.m31 ()) :
		(transform.m31 () - floor (transform.m31 ()))),
	((transform.m32 () < 0.0) ?
	 	(floor (transform.m32 ()) - transform.m32 ()) :
		(transform.m32 () - floor (transform.m32 ()))),
	 xform.m33 ());
return xform;
}


bool
Plastic_Image::source_transform_reset
	(
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_transform_reset: " << band << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "<<< Plastic_Image::source_transform_reset: false" << endl;
	#endif
	return false;
	}

bool
	changed = true;
if (Geo_Transforms)
	changed = set_source_transform (QTransform (), band);
else
	{
	Locally_Owned_and_Operated |= TRANSFORMS;
	Geo_Transforms = new QTransform*[3];
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    local Geo_Transforms @ " << (void*)Geo_Transforms << endl;
	#endif
	for (int
			band = 0;
			band < 3;
			band++)
		Geo_Transforms[band] = new QTransform;
	}

Update.end ();
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_transform_reset: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::source_origin
	(
	const QPointF&	origin,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_origin: "
		<< origin.x () << "x, " << origin.y () << 'y'
		<< "; " << band << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "<<< Plastic_Image::source_origin: false" << endl;
	#endif
	return false;
	}

if (band > 2)
	{
	Update.end (Update.SEQUENCE_END);
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't set the source origin for band " << band
			<< " to " << origin << endl
		<< "because there are only transforms for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
bool
	changed = false,
	do_update = auto_update (false);
QTransform
	*transform;
int
	bands = 3;
if (band < 0)
	band = 0;
else
	bands = band + 1;
while (band < bands)
	{
	transform = Geo_Transforms[band];
	if (origin.x () != transform->m31 ()||
		origin.y () != transform->m32 ())
		{
		transform->setMatrix
			(transform->m11 (), transform->m12 (), transform->m13 (),
			 transform->m21 (), transform->m22 (), transform->m23 (),
			 origin.x (),       origin.y (),       transform->m33 ()),
		changed = true;
		}
	++band;
	}

auto_update (do_update);
changed |= needs_update (changed ? TRANSFORMS : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_origin: " << changed << endl;
#endif
return changed;
}


QPointF
Plastic_Image::source_origin
	(
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_origin: " << band << endl;
#endif
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain the source origin for band " << band << endl
		<< "because there are only transforms for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_origin: "
		<< static_cast<int>(Geo_Transforms[band]->m31 ()) << "x, "
		<< static_cast<int>(Geo_Transforms[band]->m32 ()) << 'y' << endl;
#endif
return QPointF (Geo_Transforms[band]->m31 (), Geo_Transforms[band]->m32 ());
}


bool
Plastic_Image::source_scaling
	(
	double	scale_horizontal,
	double	scale_vertical,
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_scaling: "
		<< scale_horizontal << "x, "
		<< scale_vertical << "y; "
		<< band << 'b' << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "<<< Plastic_Image::source_scaling: false" << endl;
	#endif
	return false;
	}

if (band > 2)
	{
	Update.end (Update.SEQUENCE_END);
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't set the source scaling for band " << band << " to "
			<< scale_horizontal << "x, " << scale_vertical << 'y' << endl
		<< "because there are only transforms for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
bool
	changed = false,
	do_update = auto_update (false);
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "    current scaling -" << endl;
#endif
QTransform
	*transform;
int
	bands = 3;
if (band < 0)
	band = 0;
else
	bands = band + 1;
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
if (band < bands)
	clog << "    current scaling -" << endl;
#endif
while (band < bands)
	{
	transform = Geo_Transforms[band];
	#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
	clog << "    " << band << ": "
			<< transform->m11 () << "x, "
			<< transform->m22 () << 'y' << endl;
	#endif
	if (scale_horizontal != transform->m11 () ||
		scale_vertical   != transform->m22 ())
		{
		transform->setMatrix
			(scale_horizontal,  transform->m12 (), transform->m13 (),
			 transform->m21 (), scale_vertical,    transform->m23 (),
			 transform->m31 (), transform->m32 (), transform->m33 ());
		changed = true;
		}
	++band;
	}

auto_update (do_update);
changed |= needs_update (changed ? TRANSFORMS : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_scaling: " << changed << endl;
#endif
return changed;
}


void
Plastic_Image::source_scaling
	(
	double*	scale_horizontal,
	double*	scale_vertical,
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_scaling: get x,y for band "<< band << endl;
#endif
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain the source scaling for band " << band << endl
		<< "because there are only transforms for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_scaling: "
		<< Geo_Transforms[band]->m11 () << "x, "
		<< Geo_Transforms[band]->m22 () << 'y' << endl;
#endif
if (scale_horizontal)
	*scale_horizontal = Geo_Transforms[band]->m11 ();
if (scale_vertical)
	*scale_vertical = Geo_Transforms[band]->m22 ();
}


QSizeF
Plastic_Image::source_scaling
	(
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << ">>> Plastic_Image::source_scaling: get QSizeF for band "
		<< band << endl;
#endif
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain the source scaling for band " << band << endl
		<< "because there are only transforms for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_TRANSFORMS)
clog << "<<< Plastic_Image::source_scaling: "
		<< Geo_Transforms[band]->m11 () << "x, "
		<< Geo_Transforms[band]->m22 () << 'y' << endl;
#endif
return QSizeF (Geo_Transforms[band]->m11 (), Geo_Transforms[band]->m22 ());
}


QRect
Plastic_Image::image_region
	(
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
QPoint
	origin (round_down (source_origin ()));
double
	scale_width,
	scale_height;
source_scaling (&scale_width, &scale_height, band);
QSize
	image_size (source_size ()),
	region_size
		(static_cast<int>(width ()  / scale_width),
		 static_cast<int>(height () / scale_height));
if ((region_size.rwidth () + origin.rx ()) > image_size.rwidth ())
	 region_size.rwidth () = image_size.rwidth () - origin.rx ();
if ((region_size.rheight () + origin.ry ()) > image_size.rheight ())
	 region_size.rheight () = image_size.rheight () - origin.ry ();
return QRect (origin, region_size);
}


QSize
Plastic_Image::displayed_size
	(
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
double
	scale_width,
	scale_height;
source_scaling (&scale_width, &scale_height, band);
QPoint
	origin (round_down (source_origin ()));
QSize
	image_size (source_size ());
image_size.rwidth () -= origin.rx ();
image_size.rwidth () = static_cast<int>(image_size.rwidth () * scale_width);
if (image_size.rwidth () > width ())
	image_size.rwidth () = width ();
image_size.rheight () -= origin.ry ();
image_size.rheight () = static_cast<int>(image_size.rheight () * scale_height);
if (image_size.rheight () > height ())
	image_size.rheight () = height ();
return image_size;
}

/*------------------------------------------------------------------------------
	Source to image data mapping
*/
bool
Plastic_Image::source_data_map
	(
	const Data_Map&	data_map,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::source_data_map: @ "
		<< (void*)&data_map << "; " << band << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "<<< Plastic_Image::source_data_map: false" << endl;
	#endif
	return false;
	}

if (band > 2)
	{
	Update.end (Update.SEQUENCE_END);
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't set the source data map for band " << band << endl
		<< "because there are only data maps for image bands 0-2.";
	throw invalid_argument (message.str ());
	}

bool
	changed = false,
	single_band = true;
int
	bands = band + 1;
if (band < 0)
	{
	single_band = false;
	band = 0;
	bands = 3;
	}
while (band < bands)
	{
	//	Implicitly shared data storage.
	if (! Data_Maps[band])
		{
		Data_Maps[band] = new Data_Map (data_map);
		changed = true;
		}
	else
	if (*Data_Maps[band] != data_map)
		{
		*Data_Maps[band] = data_map;
		changed = true;
		}
	if (Data_Maps[band]->size () < (int)source_data_map_size ())
		Data_Maps[band]->resize (source_data_map_size ());
	++band;
	}
Mapping_Differences &= ~DATA_MAPS;
if (single_band)
	Mapping_Differences |= different_data_maps ();

#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << "    Mapping_Differences = " << Mapping_Differences << endl;
#endif
changed |= needs_update (changed ? DATA_MAPS : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << "<<< Plastic_Image::source_data_map: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::source_data_maps
	(
	const Data_Map**	data_maps,
	bool				shared 
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::source_data_maps: @ " << (void*)data_maps << endl
	 << "    shared = " << shared << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "<<< Plastic_Image::source_data_maps: false" << endl;
	#endif
	return false;
	}

if (data_maps &&
	data_maps == Data_Maps)
	{
	//	Identical maps.
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "    identical maps; redundant self-assignment" << endl
		 << "<<< Plastic_Image::source_data_maps: false" << endl;
	#endif
	return false;
	}

bool
	changed = false,
	locally_owned = Locally_Owned_and_Operated & DATA_MAPS;
unsigned int
	band = -1;

if (data_maps)
	{
	if (! data_maps[0] ||
		! data_maps[1] ||
		! data_maps[2])
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to assign source data maps from a NULL map.";
		throw invalid_argument (message.str ());
		}

	if (shared)
		{
		//	Check for changed data maps.
		while (++band < 3)
			{
			changed |= (*data_maps[band] != *Data_Maps[band]);
			if (locally_owned &&
				Data_Maps[band])
				delete Data_Maps[band];
			}

		//	Set the shared data maps.
		if (locally_owned)
			delete[] Data_Maps;
		Data_Maps = const_cast<Data_Map**>(data_maps);
		Locally_Owned_and_Operated &= ~DATA_MAPS;

		//	Check for different data maps.
		Mapping_Differences &= ~DATA_MAPS;
		Mapping_Differences |= different_data_maps ();
		}
	else
		{
		//	Apply each data map.
		bool
			do_update = auto_update (false);
		while (++band < 3)
			changed |= source_data_map (*data_maps[band], band);
		auto_update (do_update);
		}
	}
else
if (! locally_owned)
	{
	//	Revert to local data maps.
	Locally_Owned_and_Operated |= DATA_MAPS;
	if (! (data_maps = const_cast<const Data_Map**>(Data_Maps)))
		//	No Data_Maps; reset.
		changed = source_data_map_reset ();
	else
		{
		//	Replaced shared Data_Maps with locally supplied.
		Data_Maps = new Data_Map*[3];
		#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
		clog << "    local Data_Maps @ " << (void*)Data_Maps << endl
			 << "     replace shared @ " << (void*)data_maps << endl;
		#endif
		while (++band < 3)
			{
			Data_Maps[band] = new Data_Map (*data_maps[band]);
			#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
			clog << "    Data_Maps[" << band << "] @ "
					<< (void*)Data_Maps[band] << endl;
			#endif
			}
		}
	}

changed |= needs_update (changed ? DATA_MAPS : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << "<<< Plastic_Image::source_data_maps: " << changed << endl;
#endif
return changed;
}


Plastic_Image::Data_Map*
Plastic_Image::source_data_map
	(
	int		band
	) const
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::source_data_map: " << band << endl;
#endif
if (band < 0 ||
	band > 2)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't obtain a source data map for band " << band << endl
		<< "because there are only maps for image bands 0-2.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << "<<< Plastic_Image::source_data_map" << endl;
#endif
return Data_Maps[band];
}


bool
Plastic_Image::source_data_map_reset
	(
	int		band
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::source_data_map_reset: " << band << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "    Closed" << endl;
	#endif
	Update.end ();
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "<<< Plastic_Image::source_data_map_reset: false" << endl;
	#endif
	return false;
	}

if (MAXIMUM_PIXEL_DATUM_PRECISION < source_precision_bits ())
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't allocate a data map for "
			<< source_precision_bits () << "-bit image data;" << endl
		<< "the maximum supported is "
			<< MAXIMUM_PIXEL_DATUM_PRECISION << "-bit image data.";
	throw out_of_range (message.str ());
	}

bool
	changed = true;
Data_Map
	data_map (source_data_map_size ());
reset_data_map (data_map);

if (Data_Maps)
	changed = source_data_map (data_map, band);
else
	{
	Locally_Owned_and_Operated |= DATA_MAPS;
	Data_Maps = new Data_Map*[3];
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "    local Data_Maps @ " << (void*)Data_Maps << endl;
	#endif
	for (int
			index = 0;
			index < 3;
			index++)
		{
		Data_Maps[index] = new Data_Map (source_data_map_size ());
		assign_data_map (*Data_Maps[index], data_map);
		}
	}

Update.end ();
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << "<<< Plastic_Image::source_data_map_reset: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::reset_data_map
	(
	Data_Map&	data_map
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::reset_data_map: @ " << (void*)&data_map << endl;
#endif
bool
	changed = false;
double
	counter = 0,
	increment = 0;
int
	map_size = data_map.size ();
if (map_size > 1)
	{
	increment =
		static_cast<double>(1 << (sizeof (Data_Map::value_type) << 3))
		/ (map_size - 1);
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
	clog << "    value bits = " << (sizeof (Data_Map::value_type) << 3) << endl
		 << "      map_size = " << map_size << endl
		 << "     increment = " << increment << endl;
	#endif
	}

#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING & DEBUG_DATA_MAP)
clog << hex << setfill ('0');
#endif
/*	Only write to the Data_Map storage if necessary.

	Keeping in mind that the Data_Map storage is implicitly shared,
	only activate the copy-on-write condition if the data content
	must be changed.
*/
Data_Map::value_type
	value = 0;
for (int
		index = 0;
		index < map_size;
	  ++index)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING & DEBUG_DATA_MAP)
	if (! (index % 16))
		{
		if (index)
			clog << endl;
		clog << setw (4) << index;
		}
	clog << ' ' << setw (2) << ((unsigned int)(value) & 0xFF);
	#endif
	if (data_map.at (index) != value)
		{
		/*	Data copy.

			If the Data_Map storage is currently being shared it
			will now be detached and a deep copy will be made.
		*/
		data_map.replace (index, value);
		changed = true;
		}
	for (counter += increment;
		 counter >= 1;
		 counter -= 1)
		++value;
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << dec << setfill (' ')
	 << "<<< Plastic_Image::reset_data_map: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::assign_data_map
	(
	Data_Map&		to_map,
	const Data_Map&	from_map
	)
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::assign_data_map: @ "
		<< (void*)&to_map << " = @ " << (void*)&from_map << endl;
#endif
if (from_map.size () > to_map.size ())
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't assign a data map of size " << from_map.size ()
			<< " to a data map of size " << to_map.size () << '.';
	throw invalid_argument (message.str ());
	}

#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING & DEBUG_DATA_MAP)
clog << hex << setfill ('0');
#endif
/*	Only write to the Data_Map storage if necessary.

	Keeping in mind that the Data_Map storage is implicitly shared,
	only activate the copy-on-write condition if the data content
	must be changed.
*/
bool
	changed = false;
int
	map_size = from_map.size ();
for (int
		index = 0;
		index < map_size;
	  ++index)
	{
	#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING & DEBUG_DATA_MAP)
	if (! (index % 16))
		{
		if (index)
			clog << endl;
		clog << setw (4) << index;
		}
	clog << ' ' << setw (2) << ((unsigned int)(from_map.at (index)) & 0xFF);
	#endif
	if (to_map.at (index) != from_map.at (index))
		{
		/*	Data copy.

			If the Data_Map storage is currently being shared it
			will now be detached and a deep copy will be made.
		*/
		to_map.replace (index, from_map.at (index));
		changed = true;
		}
	}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << dec << setfill (' ')
	 << "<<< Plastic_Image::assign_data_map: " << changed << endl;
#endif
return changed;
}


bool
Plastic_Image::apply_data_map
	(
	const Data_Map&	to_map,
	const Data_Map&	from_map
	)
{
if (from_map.size () > to_map.size ())
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't apply a data map of size " << from_map.size ()
			<< " to a data map of size " << to_map.size () << '.';
	throw invalid_argument (message.str ());
	}

bool
	changed = false;
int
	map_size = to_map.size ();
Data_Map::value_type
	*data = const_cast<Data_Map::value_type*>(to_map.data ());
for (int
		index = 0;
		index < map_size;
	  ++index,
	  ++data)
	{
	if (*data != from_map.at (index))
		{
		*data = from_map.at (index);
		changed = true;
		}
	}
return changed;
}


bool
Plastic_Image::different_data_maps
	(
	const Data_Map**	data_maps
	) const
{
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << ">>> Plastic_Image::different_data_maps" << endl
	 << "    Data_Maps @ " << (void*)Data_Maps << endl
	 << "    data_maps @ " << (void*)data_maps << endl;
#endif
QMutexLocker
	object_lock (&Object_Lock);
int
	band = 3;
if (Data_Maps &&
	data_maps &&
	Data_Maps != data_maps)
	while (band--)
		{
		#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
		clog << "    Data_Maps[" << band << "] @ "
				<< (void*)Data_Maps[band] << endl
			 << "    data_maps[" << band << "] @ "
				<< (void*)data_maps[band] << endl;
		#endif
		if ( Data_Maps[band] !=  data_maps[band] &&
			*Data_Maps[band] != *data_maps[band])
			{
			#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
			clog << "<<< Plastic_Image::different_data_maps: true" << endl;
			#endif
			return true;
			}
		}
#if ((DEBUG_SECTION) & DEBUG_DATA_MAPPING)
clog << "<<< Plastic_Image::different_data_maps: false" << endl;
#endif
return false;
}


unsigned int
Plastic_Image::source_data_map_size () const
{
unsigned int
	map_size = source_precision_bits ();
if (map_size)
	map_size = 1 << map_size;
return map_size;
}

/*------------------------------------------------------------------------------
	Histograms
*/
unsigned long long
Plastic_Image::source_histograms
	(
	QVector<Histogram*>	histograms,
	const QRect&		source_region
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> Plastic_Image::source_histograms:" << endl
	 << "    source_region = " << source_region << endl;
#endif
if (histograms.size () < 3)
	{
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    resize histograms set from " << histograms.size ()
			<< " to 3" << endl;
	#endif
	histograms.resize (3);
	}
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "    histograms @ "
	 	<< (void*)histograms[0] << ", "
	 	<< (void*)histograms[1] << ", "
	 	<< (void*)histograms[2] << endl;
#endif

if (source_region.isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    empty source_region" << endl
		 << "<<< Plastic_Image::source_histograms: 0" << endl;
	#endif
	return 0;
	}

QMutexLocker
	object_lock (&Object_Lock);
QRect
	selected_region (image_region ());
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "    image region = " << selected_region << endl;
#endif
if (source_region.left () >
		(selected_region.left () + selected_region.width ()) ||
	(source_region.left () + source_region.width ()) <
		selected_region.left () ||
	source_region.top () >
		(selected_region.top () + selected_region.height ()) ||
	(source_region.top () + source_region.height ()) <
		selected_region.top ())
	{
	//	The histogram region does not intersect with the image region.
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    selection region outside image region" << endl
		 << "<<< Plastic_Image::source_histograms: 0" << endl;
	#endif
	return 0;
	}

//	Intersection of the histogram region with the image region.
selected_region &= source_region;
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "    selected image region = " << selected_region << endl;
#endif

Histogram
	*histogram;
Pixel_Datum
	pixel_value;
unsigned long long
	count,
	max_count = 0;
int
	start_line = selected_region.top () - 1,
	line = 0,
	end_line = start_line + selected_region.height () + 1,
	start_sample = selected_region.left () - 1,
	sample = 0,
	end_sample = start_sample + selected_region.width () + 1,
	source_band;
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "      first = "
		<< (start_line + 1) << "x, " << (start_sample + 1) << 'y' << endl
	 << "       last = "
	 	<< (end_sample - 1) << "x, " << (end_line - 1) << 'y' << endl;
#endif
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (refresh_source_histogram (histograms, band))
		{
		histogram = histograms[band];
		count = 0;
		source_band = Band_Map[band];
		line = start_line;

		while (++line < end_line)
			{
			sample = start_sample;
			while (++sample < end_sample)
				{
				pixel_value = source_pixel_value (sample, line, source_band);
				if (pixel_value != UNDEFINED_PIXEL_VALUE)
					{
					(*histogram)[pixel_value]++;
					++count;
					}
				}
			}

		if (max_count < count)
			max_count = count;
		#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
		clog << "    band " << band << " -" << endl;
		clog << histogram;
		#endif
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "<<< Plastic_Image::source_histograms: " << max_count << endl;
#endif
return max_count;
}


bool
Plastic_Image::refresh_source_histogram
	(
	QVector<Histogram*>&	histograms,
	int						band
	) const
{	
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> Plastic_Image::refresh_source_histogram: " << band << endl;
#endif
bool
	refresh = true;
int
	entries = 1 << source_precision_bits (),
	other_band = band;

//	Check for a display band mapped from the same source band.
//	CAUTION: It is assumed that the band number is valid (0-2).
while (other_band--)
	if (Band_Map[band] == Band_Map[other_band])
		break;

if (other_band >= 0)
	{
	//	Found an equivalent band; use its histogram data.
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    band " << band << " and band " << other_band
			<< " are both mapped to band " << Band_Map[band] << endl;
	#endif
	if (! histograms[band])
		{
		//	Share the histogram.
		histograms[band] = histograms[other_band];
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "    NULL histogram[" << band << "] set to histogram["
				<< other_band << "] @ "
				<< (void*)histograms[other_band] << endl;
		#endif
		}
	else
	if (histograms[band] != histograms[other_band])
		{
		if (histograms[band]->size () < histograms[other_band]->size ())
			{
			#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
			clog << "    histogram[ " << band << "] resized from "
					<< histograms[band]->size () << " to "
					<< histograms[other_band] << " entries" << endl;
			#endif
			histograms[band]->resize (entries);
			}
		//	Copy the histogram data.
		*histograms[band] = *histograms[other_band];
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "    histogram[" << band << "] copied from histogram["
				<< other_band << ']' << endl;
		#endif
		}
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	else
		clog << "    histogram[" << band << "] is histogram["
				<< other_band << "] @ "
				<< (void*)histograms[other_band] << endl;
	#endif
	refresh = false;
	}
else
	{
	if (! histograms[band])
		{
		//	Allocate a Histogram initialized with zeros.
		histograms[band] = new Histogram (entries);
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "    new histogram[ " << band << "] @ "
				<< (void*)histograms[band] << " with "
				<< entries << " entries" << endl;
		#endif
		}
	else
	if (histograms[band]->size () < entries)
		{
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "    histogram[ " << band << "] resized from "
				<< histograms[band]->size () << " to "
				<< entries << " entries" << endl;
		#endif
		histograms[band]->resize (entries);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> Plastic_Image::refresh_source_histogram: " << refresh << endl;
#endif
return refresh;
}


unsigned long long
Plastic_Image::display_histograms
	(
	QVector<Histogram*>	histograms,
	const QRect&		display_region
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> Plastic_Image::display_histograms:" << endl
	 << "    display_region = " << display_region << endl;
#endif
if (histograms.size () < 3)
	{
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    resize histograms set from " << histograms.size ()
			<< " to 3" << endl;
	#endif
	histograms.resize (3);
	}
for (int
		band = 0;
		band < 3;
		band++)
	{
	if (! histograms[band])
		{
		//	Allocate a Histogram initialized with NULLs.
		#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
		clog << "      new band " << band
				<< " Histogram allocated with 256 entries" << endl;
		#endif
		histograms[band] = new Histogram (256);
		}
	else
	if (histograms[band]->size () < 256)
		{
		#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
		clog << "      band " << band << " histogram resized to 256 entries"
				<< endl;
		#endif
		histograms[band]->resize (256);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> Plastic_Image::display_histograms: " << display_region << endl
	 << "    histograms @ "
	 	<< (void*)histograms[0] << ", "
	 	<< (void*)histograms[1] << ", "
	 	<< (void*)histograms[2] << endl;
#endif

if (display_region.isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    empty display_region" << endl
		 << "<<< Plastic_Image::display_histograms: 0" << endl;
	#endif
	return 0;
	}

QMutexLocker
	object_lock (&Object_Lock);
QRect
	selected_region (rect ());	//	Selected display region.
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "    display region = " << selected_region << endl;
#endif
if (display_region.left () >
		(selected_region.left () + selected_region.width ()) ||
	(display_region.left () + display_region.width ()) <
		selected_region.left () ||
	display_region.top () >
		(selected_region.top () + selected_region.height ()) ||
	(display_region.top () + display_region.height ()) <
		selected_region.top ())
	{
	//	The histogram region does not intersect with the display region.
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	clog << "    selection region outside display image" << endl
		 << "<<< Plastic_Image::display_histograms: 0" << endl;
	#endif
	return 0;
	}
//	Intersection of the histogram region with the display region.
selected_region &= display_region;
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "    selected display region = " << selected_region << endl;
#endif

/*	>>> CAUTION <<< Implicit data sharing.

	The bits method does NOT cause shared image display data to be
	detached if it is in use elsewhere as long as the call is from a
	const QImage (this) context. Thus it is important that this method be
	declared const.
*/
const QRgb
	*image_display_data =
		reinterpret_cast<const QRgb*>(bits ())
		+ (selected_region.top () * width ()) + selected_region.left (),
	*display_data;
int
	line = selected_region.top () - 1,
	end_line = line + selected_region.height () + 1,
	start_sample = selected_region.left () - 1,
	sample = 0,
	end_sample = start_sample + selected_region.width () + 1;
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "      first = "
		<< (line + 1) << "x, " << (start_sample + 1) << 'y' << endl
	 << "       last = "
	 	<< (end_sample - 1) << "x, " << (end_line - 1) << 'y' << endl;
#endif
while (++line < end_line)
	{
	display_data = image_display_data;
	sample = start_sample;
	while (++sample < end_sample)
		{
		++((*histograms[0])[qRed   (*display_data)]);
		++((*histograms[1])[qGreen (*display_data)]);
		++((*histograms[2])[qBlue  (*display_data)]);
		++display_data;
		}

	//	Move to the beginning of the next line of the region.
	image_display_data += width ();
	}
#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
clog << histograms[0]
	 << histograms[1]
	 << histograms[2];
#endif
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "<<< Plastic_Image::display_histograms: "
		<< ((unsigned long long)selected_region.width ()
			* selected_region.height ()) << endl;
#endif
return ((unsigned long long)selected_region.width ()
			* selected_region.height ());
}

/*------------------------------------------------------------------------------
	Band fill
*/
void
Plastic_Image::fill
	(
	QRgb	color,
	int		band
	)
{
if (band < 0 ||
	band > 2)
	return;

QRgb
	band_mask = 0xFF << (16 - (band << 3)),
	fill_value = color & band_mask;
for (QRgb
		/*
			N.B.: The image_data method returns a pointer to writable
			(non-const) image display data. This avoids having the
			implicitly shared image data detatched if it is currently
			being used elsewhere.
		*/
		*display_data = image_data (),
		*end_data = display_data + (width () * height ());
		 display_data < end_data;
	   ++display_data)
	*display_data = (*display_data & band_mask) | fill_value;
}

/*------------------------------------------------------------------------------
	Data update and rendering
*/
bool
Plastic_Image::update ()
	throw (Render_Exception, std::bad_exception)
{
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
clog << ">>> Plastic_Image::update:" << endl;
#endif
Update.start ();
bool
	up_to_date = false;
if (! Closed)
	{
	#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
	clog << "       Rendering = " << Rendering << endl
		 << "    Needs_Update = " << mapping_type_names (Needs_Update)
		 	<< " (" << Needs_Update << ')' << endl;
	#endif
	if (Rendering)
		up_to_date = (Needs_Update_Shadow == NO_MAPPINGS);
	else
	if (Needs_Update)
		up_to_date = render_image ();
	}
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
else
	clog << "    Closed" << endl;
#endif
Update.end ();
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
clog << "<<< Plastic_Image::update: " << up_to_date << endl;
#endif
return up_to_date;
}


bool
Plastic_Image::needs_update
	(
	Mapping_Type	changed
	)
	throw (Render_Exception, std::bad_exception)
{
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
clog << ">>> Plastic_Image::needs_update: " << mapping_type_names (changed)
		<< " (" << changed << ')' << endl
	 << "    " << *this << endl;
#endif
Update.start ();
bool
	updated = false;
if (! Closed)
	{
	#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
	clog << "       Rendering = " << Rendering << endl;
	#endif
	if (Rendering)
		{
		Needs_Update_Shadow |= (changed & ALL_MAPPINGS);
		#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
		clog << "    Needs_Update_Shadow = "
				<< mapping_type_names (Needs_Update_Shadow)
				<< " (" << Needs_Update_Shadow << ')' << endl;
		#endif
		}
	else
		{
		Needs_Update |= (changed & ALL_MAPPINGS);
		#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
		clog << "     Auto_Update = " << Auto_Update << endl
			 << "    Needs_Update = "
				<< mapping_type_names (Needs_Update)
				<< " (" << Needs_Update << ')' << endl;
		#endif
		if (Auto_Update &&
			Needs_Update)
			updated = render_image ();
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
else
	clog << "    Closed" << endl;
#endif
Update.end ();
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
clog << "<<< Plastic_Image::needs_update: " << updated << endl;
#endif
return updated;
}


Plastic_Image::Mapping_Type
Plastic_Image::needs_update () const
{
QMutexLocker
	object_lock (&Object_Lock);
//	Return the cummulative value.
return Needs_Update | Needs_Update_Shadow;
}


bool
Plastic_Image::render_image ()
	throw (Render_Exception, std::bad_exception)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << ">>> Plastic_Image::render_image" << endl
	 << "    " << *this << endl;
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	clog << "    Closed" << endl;
	#endif
	Update.end (Update.SEQUENCE_END);
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	clog << "<<< Plastic_Image::render_image: false" << endl;
	#endif
	return false;
	}

if (Rendering)
	{
	ostringstream
		message;
	message
		<< "Request to render \"" << source_name () << '"' << endl
		<< "while rendering is in progress.";
	#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER | DEBUG_LOCATION))
	clog << message.str () << endl;
	#endif
	Update.end (Update.SEQUENCE_END);
	throw Render_Exception (message.str ());
	}

if (update_canceled ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	clog << "    update canceled before rendering" << endl;
	#endif
	cancel_update (false);
	Update.end (Update.SEQUENCE_END);
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	clog << "<<< Plastic_Image::render_image: false" << endl;
	#endif
	return false;
	}

if (! source () ||
	source_size ().isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
	clog << "    No data source or empty source" << endl;
	#endif
	Needs_Update = NO_MAPPINGS;
	fill (background_color ());
	Update.end (Update.SEQUENCE_END);
	#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
	clog << "<<< Plastic_Image::render_image" << endl;
	#endif
	return false;
	}

//	Beginning rendering.
is_rendering (true);

//	End of the update sequence; release the Object_Lock during redering.
Update.end (Update.SEQUENCE_END);
/*
	>>> CAUTION <<< From this point until rendering ends (is_rendering (false))
	only Rendering configuration variables should be used.
*/

QTransform
	transform,
	*transforms[3] = {NULL, NULL, NULL};
QRgb
	pixel_value,
	/*
		N.B.: The image_data method returns a pointer to writable
		(non-const) image display data. This avoids having the
		implicitly shared image data detatched if it is currently
		being used elsewhere.
	*/
	*display_data = image_data (),
	background_value[3];
unsigned int
	band_map[3];
Data_Map
	*data_maps[3];
int
	band = -1;
while (++band < 3)
	{
	//	Use the destination to source inverse transform.
	try {transform = invert_keeping_origin
			(*(Rendering->Geo_Transforms[band]));}
	catch (runtime_error& except)
		{
		is_rendering (false);
		ostringstream
			message;
		message	//	invert_keeping_origin provides the ID.
			<< except.what ()
			<< "While preparing to render image source -" << endl
			<< source_name ();
		throw Render_Exception (message.str ());
		}
	#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
	clog << "    band " << band << " -" << endl
		 << "    transform -" << *source_transform (band);
	clog << "    inverted -" << transform;
	#endif
	if (! transform.isIdentity ())
		transforms[band] = new QTransform (transform);
	#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER))
	else
		clog << "    set to NULL" << endl;
	#endif

	//	Display-to-Source band map.
	band_map[band] = Rendering->Band_Map[band];
	if (band_map[band] > 3)
		{
		is_rendering (false);
		ostringstream
			message;
		message
			<< ID << endl
			<< "Unable to render image source -" << endl
			<< source_name () << endl
			<< "Band map entry " << band << " refers to non-display band "
				<< band_map[band] << '.';
		throw Render_Exception (message.str ());
		}

	//	Source-to-Display data LUT.
	data_maps[band] = Rendering->Data_Maps[band];

	//	Background (no image) value.
	background_value[band] =
		Rendering->Background_Color & (0xFF << (16 - (band << 3)));
	}

Pixel_Datum
	datum,
	max_value = source_data_map_size () - 1;
int
	source_line,
	source_sample,
	display_line = -1,
	display_lines = height (),
	display_sample,
	display_samples = width ();
unsigned int
	rendered_lines = 0,
	rendering_increment = rendering_increment_lines ();
if (rendering_increment &&
	//	"Close enough" margin.
	((rendering_increment >> 3) == 0 ||
	(rendering_increment + (rendering_increment >> 3))
		>= (unsigned int)display_lines))
	rendering_increment = (unsigned int)display_lines;

#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER | DEBUG_LOCATION))
clog << "    display_samples = " << display_samples << endl
	 << "      display_lines = " << display_lines << endl
	 << "    rendering incr. = " << rendering_increment << endl
	 << "       display_data @ " << (void*)display_data << endl
	 << "     source samples = " << source_size ().width () << endl
	 << "       source lines = " << source_size ().height () << endl
	 << "          max_value = " << hex << max_value << dec << endl;
#endif

Rendering_Monitor::Status
	status = Rendering_Monitor::TOP_QUALITY_DATA;
QRect
	rendered_region (0, 0, display_samples, 0);

while (++display_line < display_lines)
	{
	display_sample = -1;
	while (++display_sample < display_samples)
		{
		pixel_value = 0xFF000000;
		band = -1;
		#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
		if (! display_sample)
			{
			#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
			if (! display_line)
			#endif
				clog << "    line " << display_line << " -" << endl;
			}
		#endif
		while (++band < 3)
			{
			if (transforms[band])
				transforms[band]->map
					(display_sample, display_line,
					&source_sample, &source_line);
			else
				{
				source_sample = display_sample;
				source_line = display_line;
				}
			datum =
				source_pixel_value
					(source_sample, source_line, band_map[band]);
			#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
			#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
			if (! display_line)
			#endif
				clog << "      " << band << "->" << band_map[band]
						<< ": image "
						<< setw (3) << display_sample << "x, "
						<< setw (3) << display_line << "y <- source "
						<< setw (3) << source_sample << "x, "
						<< setw (3) << source_line << "y = "
						<< setfill ('0') << hex << setw (2)
						<< datum << " -> ";
			#endif
			if (datum == UNDEFINED_PIXEL_VALUE ||
				datum > max_value)
				{
				#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
				#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
				if (! display_line)
				#endif
					clog << "BG" << dec << setfill (' ') << endl;
				#endif
				pixel_value |= background_value[band];
				}
			else
				{
				#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
				#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
				if (! display_line)
				#endif
					clog
						<< setw (2)
						<< static_cast<Pixel_Datum>(data_maps[band]->at (datum))
						<< dec << setfill (' ') << endl;
				#endif
				//	Merge the mapped pixel datum into the image pixel value.
				pixel_value |=
					static_cast<QRgb>(data_maps[band]->at (datum))
					<< (16 - (band << 3));
				}
			}
		#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
		#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
		if (! display_line)
		#endif
			clog << "      display_data @ "
					<< setfill ('0') << hex << setw (sizeof (display_data) << 1)
					<< (void*)display_data
					<< " = " << setw (8) << pixel_value
					<< dec << setfill (' ') << endl;
		#endif
		*display_data++ = pixel_value;
		}
	if (rendering_increment &&
		++rendered_lines == rendering_increment)
		{
		rendered_region.setHeight (static_cast<int>(rendered_lines));
		if (display_line == display_lines)
			status = Rendering_Monitor::DONE;
		#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | \
						DEBUG_RENDER | \
						DEBUG_NOTIFY | \
						DEBUG_LOCATION))
		clog << "    Plastic_Image::render_image: notify_rendering_monitors"
				" status " << status << " \""
				<< Rendering_Monitor::Status_Message[status] << '"' << endl
			 << "    region = " << rendered_region << endl;
		#endif
		if (! notify_rendering_monitors
				(status, Rendering_Monitor::Status_Message[status],
				rendered_region))
			break;

		//	Move the top of the rendered region to the next line.
		rendered_region.setTop (display_line + 1);
		rendered_lines = 0;
		}
	}

/*	Check for canceled rendering.
	This is done as a test-and-reset operation to avoid a possible race.
*/
bool
	canceled = cancel_update (false),
	completed = (display_line == display_lines);

if (rendered_lines)
	{
	rendered_region.setHeight (static_cast<int>(rendered_lines));
	status = (display_line == display_lines) ?
		Rendering_Monitor::DONE :
		Rendering_Monitor::CANCELED;
	#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | \
					DEBUG_RENDER | \
					DEBUG_NOTIFY | \
					DEBUG_LOCATION))
	clog << "    Plastic_Image::render_image: notify_rendering_monitors"
			" status " << status
			<< " \"" << Rendering_Monitor::Status_Message[status] << '"' << endl
		 << "    region = " << rendered_region << endl;
	#endif
	notify_rendering_monitors
		(status, Rendering_Monitor::Status_Message[status], rendered_region);
	}

//	Cleanup.
for (band = 0;
	 band < 3;
	 band++)
	if (transforms[band])
		delete transforms[band];

#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
//	Tile markings.
QString
	label = QString (" image @ %1: so %2, %3; s %4")
	.arg ((ulong)this, 0, 16)
	.arg (source_origin ().x ())
	.arg (source_origin ().y ())
	.arg (source_scaling ().width ())
	.append (canceled ? " CANCELED" : "")
	.append (completed ? "" : " INCOMPLETE");
mark_image (this, label,
	TILE_MARKINGS_Y,
	TILE_MARKINGS_COLOR);
mark_image (this, label,
	height () - TILE_MARKINGS_Y - 12,
	TILE_MARKINGS_COLOR);
#endif

completed = (! canceled) && completed;
if (completed)
	Rendering->Needs_Update = NO_MAPPINGS;

cancel_update (false);	//	Always clear the cancel status.

//	End rendering.
is_rendering (false);
#if ((DEBUG_SECTION) & (DEBUG_MANIPULATORS | DEBUG_RENDER | DEBUG_LOCATION))
clog << "<<< Plastic_Image::render_image: " << completed << endl;
#endif
return completed;
}


bool
Plastic_Image::cancel_update
	(
	bool	cancel
	)
{
QMutexLocker
	rendering_monitors_lock (&Rendering_Monitors_Lock);
#if ((DEBUG_SECTION) & (DEBUG_NOTIFY | DEBUG_RENDER))
clog << ">-< Plastic_Image::cancel_update: " << cancel << endl;
#endif
bool
	canceled = Cancel_Update;
if (! cancel ||
	! Closed)
	Cancel_Update = cancel;
return canceled;
}


bool
Plastic_Image::update_canceled () const
{
QMutexLocker
	rendering_monitors_lock (&Rendering_Monitors_Lock);
return Cancel_Update;
}


bool
Plastic_Image::add_rendering_monitor
	(
	Rendering_Monitor*	monitor
	)
{
bool
	added = false;
if (monitor)
	{
	QMutexLocker
		object_lock (&Object_Lock);
	if (! Closed &&
		! Rendering_Monitors.contains (monitor))
		{
		Rendering_Monitors.append (monitor);
		added = true;
		}
	}
return added;
}


bool
Plastic_Image::remove_rendering_monitor
	(
	Rendering_Monitor*	monitor
	)
{
QMutexLocker
	rendering_monitors_lock (&Rendering_Monitors_Lock);
return Rendering_Monitors.removeOne (monitor);
}


bool
Plastic_Image::notify_rendering_monitors
	(
	Rendering_Monitor::Status	status,
	const QString&				message,
	const QRect&				region
	)
{
QMutexLocker
	rendering_monitors_lock (&Rendering_Monitors_Lock);
#if ((DEBUG_SECTION) & (DEBUG_NOTIFY | DEBUG_RENDER | DEBUG_LOCATION))
clog << ">>> Plastic_Image::notify_rendering_monitors:" << endl
	 << "    status " << status << " \"" << message << '"' << endl
	 << "    rendered region = " << region << endl
	 << "     display region = " << rect () << endl
	 << "      Cancel_Update = " << Cancel_Update << endl;
#endif
if (Closed)
	{
	#if ((DEBUG_SECTION) & (DEBUG_NOTIFY | DEBUG_RENDER))
	clog << "    Closed" << endl;
	#endif
	return false;
	}

#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
//	Tile markings.
ostringstream
	label;
label << " incr. region @ " << (void*)this << ": " << region
		<< "; st " << status;
mark_image (this, QString::fromStdString (label.str ()),
	region.y (),
	TILE_MARKINGS_COLOR);
#endif

bool
	cancel = Cancel_Update;

for (int entry = 0 ; entry < Rendering_Monitors.size () ; entry++) 
    if (Rendering_Monitors[entry] != NULL)
	cancel |= ! Rendering_Monitors[entry]->notification
					(*this, status, message, region);
#if ((DEBUG_SECTION) & DEBUG_PROMPT)
char
	input[4];
clog << "Notified " << region << " > ";
cin.getline (input, 2);
if (input[0] == 'q')
	exit (7);
#endif
#if ((DEBUG_SECTION) & (DEBUG_NOTIFY | DEBUG_RENDER | DEBUG_LOCATION))
clog << "<<< Plastic_Image::notify_rendering_monitors: " << (! cancel) << endl;
#endif
return ! cancel;
}


void
Plastic_Image::rendering_increment_lines
	(
	unsigned int	rendering_increment
	)
{
QMutexLocker
	object_lock (&Object_Lock);
if (! Closed)
	Rendering_Increment_Lines = rendering_increment;
}


unsigned int
Plastic_Image::rendering_increment_lines () const
{
QMutexLocker
	object_lock (&Object_Lock);
return Rendering_Increment_Lines;
}

/*==============================================================================
	Rendering configuration
*/
Plastic_Image::Rendering_Data::Rendering_Data
	(
	const Plastic_Image&	image
	)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> Plastic_Image::Rendering_Data @ " << (void*)this << endl
	 << "    owner - " << image << endl));
#endif
//	Copy the rendering variables.
Band_Map = new unsigned int[3];
Geo_Transforms = new QTransform*[3];
Data_Maps = new Plastic_Image::Data_Map*[3];
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "          Band_Map @ " << (void*)(image.Band_Map) << endl
	 << "    Geo_Transforms @ " << (void*)(image.Geo_Transforms) << endl
	 << "         Data_Maps @ " << (void*)(image.Data_Maps) << endl));
#endif
for (int
		band = 0;
		band < 3;
		band++)
	{
	Band_Map[band] =
		image.Band_Map[band];
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "    Band_Map[" << band << "] = " << image.Band_Map[band] << endl));
	#endif
	Geo_Transforms[band] =
		new QTransform (*(image.Geo_Transforms[band]));
	/*
		N.B.: The data of the copied Data_Map is implicitly shared.
		The data will only be copied if it is modifed (copy-on-write;
		i.e. accessed with a non-const method).

		!!! Could this be the source of a race condition?
	*/
	Data_Maps[band] =
		new Plastic_Image::Data_Map (*image.Data_Maps[band]);
	}
Locally_Owned_and_Operated = image.Locally_Owned_and_Operated;
Mapping_Differences        = image.Mapping_Differences;
Background_Color           = image.Background_Color;
Needs_Update               = image.Needs_Update;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "    Locally_Owned_and_Operated = 0x" << hex
		<< Locally_Owned_and_Operated << endl
	 << "           Mapping_Differences = 0x"
	 	<< Mapping_Differences << endl
	 << "                  Needs_Update = 0x"
	 	<< Needs_Update << dec << endl
	 << "<<< Plastic_Image::Rendering_Data" << endl));
#endif
}


Plastic_Image::Rendering_Data::~Rendering_Data ()
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> Plastic_Image::~Rendering_Data @ " << (void*)this << endl));
#endif
if (Geo_Transforms)
	{
	for (int
			band = 0;
			band < 3;
		  ++band)
		if (Geo_Transforms[band])
			delete Geo_Transforms[band];
	delete[] Geo_Transforms;
	}
if (Data_Maps)
	{
	for (int
			band = 0;
			band < 3;
		  ++band)
		if (Data_Maps[band])
			delete Data_Maps[band];
	delete[] Data_Maps;
	}
if (Band_Map)
	delete[] Band_Map;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "<<< Plastic_Image::~Rendering_Data @ " << (void*)this << endl));
#endif
}


QPointF
Plastic_Image::Rendering_Data::origin
	(
	int		band
	) const
{return QPointF (Geo_Transforms[band]->m31 (), Geo_Transforms[band]->m32 ());}


QSizeF
Plastic_Image::Rendering_Data::scaling
	(
	int		band
	) const
{return QSizeF (Geo_Transforms[band]->m11 (), Geo_Transforms[band]->m22 ());}


bool
Plastic_Image::is_rendering
	(
	bool	rendering
	)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> Plastic_Image::is_rendering: " << rendering << endl));
#endif
QMutexLocker
	object_lock (&Object_Lock);
bool
	was_rendering = false;
if (rendering)
	{
	if (Rendering)
		was_rendering = true;
	else
		{
		//	Start rendering with a copy of the rendering data values.
		Rendering = new Rendering_Data (*this);
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
		LOCKED_LOGGING ((
		clog << "    new Rendering_Data @ " << (void*)Rendering << endl
			 << "    for " << *this << endl));
		#endif

		//	Reset the shadow value.
		Needs_Update_Shadow = NO_MAPPINGS;
		}
	}
else
if (Rendering)
	{
	was_rendering = true;

	//	Sync the Needs_Update values.
	Needs_Update = Needs_Update_Shadow | Rendering->Needs_Update;
	Needs_Update_Shadow = NO_MAPPINGS;

	//	Rendering has ended.
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "    delete Rendering_Data @ " << (void*)Rendering << endl
		 << "    for " << *this << endl));
	#endif
	delete Rendering;
	Rendering = NULL;

	if (Closed)
		{
		//	The image was closed while it was being rendered.
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
		LOCKED_LOGGING ((
		clog << "    the image was Closed" << endl));
		#endif
		close ();
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "<<< Plastic_Image::is_rendering: " << was_rendering << endl));
#endif
return was_rendering;
}


bool
Plastic_Image::is_rendering () const
{
QMutexLocker
	object_lock (&Object_Lock);
return (Rendering != NULL);
}


QRgb*
Plastic_Image::image_data () const
{return reinterpret_cast<QRgb*>(const_cast<uchar*>(bits ()));}

/*==============================================================================
	Update sequence locker
*/
Plastic_Image::Update_Locker::Update_Locker
	(
	const Plastic_Image&	image
	)
	:
	Object_Lock (&(image.Object_Lock)),
	Updating (false),
	Initiator_Thread (NULL)
{}


Plastic_Image::Update_Locker::Update_Locker ()
	:
	Object_Lock (NULL),
	Updating (false),
	Initiator_Thread (NULL)
{}


Plastic_Image::Update_Locker::~Update_Locker ()
{end ();}


void
Plastic_Image::Update_Locker::start ()
{
QMutexLocker
	update_lock (&Update_Lock);
void*
	current_thread = (void*)QThread::currentThreadId ();
#if ((DEBUG_SECTION) & DEBUG_UPDATE)
LOCKED_LOGGING ((
clog << ">>> Plastic_Image::Update_Locker::start " << current_thread << endl
	 << "    Initiator_Thread " << Initiator_Thread << endl
	 << "    Updating = " << Updating << endl));
#endif

if (! Updating ||
	Initiator_Thread != current_thread)
	{
	/*
		Either an update sequence is not in progress, or an update
		sequence is in progress but it was started by a different thread
		from the current thread. In the latter case - where the current
		thread is different from the initiator thread - the attempt to
		acquire the Object_Lock will block the thread until the update
		sequence in progress completes.
	*/
	if (Updating)
		{
		#if ((DEBUG_SECTION) & DEBUG_UPDATE)
		LOCKED_LOGGING ((
		clog << "    Update_Lock.unlock in anticipation of blocking"
				" Object_Lock.lock" << endl));
		#endif
		/*
			>>> WARNING <<<
			To avoid a deadlock condition over access to the Update_Lock
			by the initiator thread - where the blocked current thread
			holds the Update_Lock thus preventing the initiator thread
			from acquiring the lock when calling Update_Locker methods -
			the Update_Lock must be released by the current thread that
			will block when acquiring the Object_Lock.
		*/
		update_lock.unlock ();
		}

	#if ((DEBUG_SECTION) & DEBUG_UPDATE)
	LOCKED_LOGGING ((
	clog << "    Object_Lock.lock" << endl));
	#endif
	Object_Lock->lock ();
	//	The current thread now has exclusive control of the update sequence.

	/*
		In the case of a different thread from the one having started an
		update sequence in progress having acquired the Object_Lock after
		the previous update sequence completed, the Update_Lock would now
		be unlocked and needs to be relocked. Relocking is safe even if
		the Update_Lock was not unlocked - i.e. there was no thread
		change - because the QMutexLocker will only call the lock method
		on the lock if it had been unlocked here.
	*/
	update_lock.relock ();
	Initiator_Thread = current_thread;
	#if ((DEBUG_SECTION) & DEBUG_UPDATE)
	LOCKED_LOGGING ((
	clog << "    Initiator_Thread " << Initiator_Thread << endl));
	#endif
	}
++Updating;
#if ((DEBUG_SECTION) & DEBUG_UPDATE)
LOCKED_LOGGING ((
clog << "    Updating = " << Updating << endl
	 << "<<< Plastic_Image::Update_Locker::start " << current_thread << endl));
#endif
}


void
Plastic_Image::Update_Locker::end
	(
	Update_End_Condition	condition
	)
{
QMutexLocker
	update_lock (&Update_Lock);
#if ((DEBUG_SECTION) & DEBUG_UPDATE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCKED_LOGGING ((
clog << ">>> Plastic_Image::Update_Locker::end " << thread_ID
		<< ": " << condition << endl
	 << "    Initiator_Thread " << Initiator_Thread << endl
	 << "    Updating = " << Updating << endl));
#endif
if (Updating)
	{
	if (condition == SEQUENCE_END)
		//	Unconditional end of the update sequence.
		Updating = 0;
	else
		Updating--;
	if (Updating == 0)
		{
		Initiator_Thread = NULL;

		#if ((DEBUG_SECTION) & DEBUG_UPDATE)
		LOCKED_LOGGING ((
		clog << "    Object_Lock.unlock" << endl));
		#endif
		//	Unlock the object last; another thread might be waiting to pounce.
		Object_Lock->unlock ();
		}
	#if ((DEBUG_SECTION) & DEBUG_UPDATE)
	else
		{
		LOCKED_LOGGING ((
		clog << "    still Updating = " << Updating << endl));
		}
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_UPDATE)
LOCKED_LOGGING ((
clog << "<<< Plastic_Image::Update_Locker::end " << thread_ID << endl));
#endif
}

/*==============================================================================
	Utility Functions
*/
std::ostream&
operator<<
	(
	std::ostream&			stream,
	const Plastic_Image&	image
	)
{
return stream
	<< "image @ " << (void*)(&image)
		<< " source name \"" << image.source_name () << '"' << std::endl
	<< "    size " << image.size () << '/' << image.source_size ()
		<< ", region " << image.image_region ()
		<< ", scaling " << image.source_scaling ()
		<< ", needs update "
		<< image.mapping_type_names (image.needs_update ())
		<< " (" << image.needs_update () << ')'
		<< (image.update_canceled () ? " canceled" : "");
}


std::ostream&
operator<<
	(
	std::ostream&					stream,
	const Plastic_Image::Triplet&	triplet
	)
{
return stream
	<< triplet.Datum[0] << "r, "
	<< triplet.Datum[1] << "g, "
	<< triplet.Datum[2] << 'b';
}


void
display_line_data
	(
	const Plastic_Image&	image,
	int						line
	)
{
QSize
	image_size (image.size ());
if (line < image_size.rheight ())
	{
	/*	>>> CAUTION <<< Implicit data sharing.

		The scanLine method does NOT cause shared image display data to
		be detached if it is in use elsewhere as long as the call is from
		a const QImage (this) context. Thus it is important that the
		image argument be declared const.
	*/
	const QRgb
		*pixels =
			reinterpret_cast<const QRgb*>(image.scanLine (line));
	clog << "line " << line << " @ " << (void*)pixels << " -" << endl
		 << hex << setfill ('0');
	for (int
			index = 0;
			index < image_size.rwidth ();
			index++,
			pixels++)
		{
		if (! (index % 16))
			{
			if (index)
				clog << endl;
			clog << dec << setfill (' ')
				 << setw (5) << index << ": "
				 << hex << setfill ('0');
			}
		clog << "  "
				<< setw (2) << qRed (*pixels) << ','
				<< setw (2) << qGreen (*pixels) << ','
				<< setw (2) << qBlue (*pixels);
		}
	clog << dec << setfill (' ') << endl;
	}
}


void
display_image_data
	(
	const Plastic_Image&	image
	)
{
QSize
	image_size (image.size ());
if (image_size.rheight () &&
	image_size.rwidth ())
	{
	int
		line = -1,
		matches = 0;
	const QRgb
		*last_line_pixels,
		*this_line_pixels;

	clog << ">>> display_image_data" << endl
		 << "    " << image << endl;
	while (++line < image_size.rheight ())
		{
		if (line)
			{
			/*	>>> CAUTION <<< Implicit data sharing.

				The scanLine method does NOT cause shared image display
				data to be detached if it is in use elsewhere as long as
				the call is from a const QImage (this) context. Thus it
				is important that the image argument be declared const.
			*/
			last_line_pixels =
				reinterpret_cast<const QRgb*>(image.scanLine (line - 1));
			this_line_pixels =
				reinterpret_cast<const QRgb*>(image.scanLine (line));
			int
				index;
			for (index = 0;
				index < image_size.rwidth ();
				index++)
				if (*last_line_pixels++ != *this_line_pixels++)
					break;
			if (index == image_size.rwidth ())
				++matches;
			else
				matches = -matches;
			}
		if (matches <= 0)
			{
			if (matches < 0)
				{
				clog << "line " << (line + matches);
				if (matches != -1)
					clog << " - " << (line - 1);
				clog << " matches the previous line." << endl;
				matches = 0;
				}
			display_line_data (image, line);
			}
		}
	if (matches)
		{
		clog << "line " << (line - matches);
		if (matches != 1)
			clog << " - " << (line - 1);
		clog << " matches the previous line." << endl;
		}
	clog << "<<< display_image_data" << endl;
	}
}


std::ostream&
operator<<
	(
	std::ostream&		stream,
	const QTransform&	transform
	)
{
return  stream
	<< std::endl
	<< "    m11 = " << std::setw (6) << transform.m11 ()
		<< ", m12 = " << std::setw (6) << transform.m12 ()
		<< ", m13 = " << std::setw (6) << transform.m13 () << std::endl
	<< "    m21 = " << std::setw (6) << transform.m21 ()
		<< ", m22 = " << std::setw (6) << transform.m22 ()
		<< ", m23 = " << std::setw (6) << transform.m23 () << std::endl
	<< "    m31 = " << std::setw (6) << transform.m31 ()
		<< ", m32 = "  << std::setw (6)<< transform.m32 ()
		<< ", m33 = " << std::setw (6) << transform.m33 () << std::endl;
}


std::ostream&
operator<<
	(
	std::ostream&							stream,
	const Plastic_Image::Histogram* const	histogram
	)
{
stream << "    Histogram data -";
if (histogram)
	{
	for (int
			entry = 0;
			entry < histogram->size ();
			entry++)
		{
		if (! (entry % 16))
			stream << endl
				 << dec << setw (9) << entry << ":" << hex;
		stream << ' ' << setw (8) << histogram->at (entry);
		}
	stream << dec << endl;
	}
else
	stream << " NULL" << endl;
return stream;
}


std::ostream&
operator<<
	(
	std::ostream&		stream,
	const unsigned int*	band_map
	)
{
stream << " @ ";
if (band_map)
	stream << (void*)band_map << " - "
		 << band_map[0] << ", "
		 << band_map[1] << ", "
		 << band_map[2];
else
	stream << "NULL";
return stream;
}


void
mark_image
	(
	Plastic_Image*	image,
	const QString&	label,
	int				top,
	const QColor&	text_color,
	const QColor&	field_color
	)
{
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << ">>> mark_image" << endl;
#endif
static QMutex
	Helping;
if (image->isNull ())
	return;

Helping.lock ();
QPainter
	painter;
painter.begin (image);
QRect
	rect (0, top, image->width (), 12);
QColor
	rect_color (field_color);
if (! rect_color.isValid ())
	rect_color = Qt::white;
painter.fillRect (rect, rect_color);
painter.setPen (QPen (text_color));
painter.drawText (rect, Qt::AlignLeft | Qt::AlignVCenter, label);
painter.end ();
Helping.unlock ();
#if ((DEBUG_SECTION) & DEBUG_HELPERS)
clog << "<<< mark_image" << endl;
#endif
}

}	//	namespace HiRISE
}	//	namespace UA
