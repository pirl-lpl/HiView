/*	JP2_Image

HiROC CVS ID: $Id: JP2_Image.cc,v 2.33 2014/05/27 17:13:51 guym Exp $

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

#include	"JP2_Image.hh"

#include	"HiView_Utilities.hh"

//	UA::HiRISE::JP2_Reader.
#include	"JP2.hh"
using UA::HiRISE::JP2;
#include	"JP2_Reader.hh"
using UA::HiRISE::JP2_Reader;
#include	"JP2_Exception.hh"
using UA::HiRISE::JP2_Exception;

//	PIRL++ Dimensions.
#include	"Dimensions.hh"

#include	"PVL.hh"
using idaeim::PVL::Aggregate;

#include	<QMutexLocker>

#include	<string>
using std::string;
#include	<sstream>
using std::ostringstream;
#include	<iomanip>
using std::endl;
#include	<stdexcept>
using std::exception;
using std::invalid_argument;
using std::out_of_range;
using std::bad_alloc;


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_TRANSFORMS		(1 << 1)
#define DEBUG_BAND_MAP			(1 << 2)
#define DEBUG_DATA_MAPPING		(1 << 3)
#define DEBUG_ACCESSORS			(DEBUG_TRANSFORMS | \
								 DEBUG_DATA_MAPS | \
								 DEBUG_BAND_MAP)
#define DEBUG_DATA_BUFFERS		(1 << 4)
#define DEBUG_RENDER			(1 << 6)
#define DEBUG_NOTIFY			(1 << 7)
#define DEBUG_PIXEL_DATUM		(1 << 8)
#define DEBUG_PIXEL_MAPPING		(1 << 9)
#define	DEBUG_ONE_LINE			(1 << 10)
#define DEBUG_PIXEL_DATA		(1 << 11)
#define DEBUG_HISTOGRAMS		(1 << 12)
#define DEBUG_PRINT_HISTOGRAMS	(1 << 13)
#define DEBUG_IMAGE_TO_SOURCE	(1 << 14)
#define DEBUG_OVERVIEW			(1 << 15)
#define DEBUG_PROMPT			(1 << 16)
#define DEBUG_LOCATION			(1 << 17)
#define DEBUG_METADATA			(1 << 18)

#define DEBUG_TILE_MARKINGS		(1 << 20)
#define TILE_MARKINGS_Y					36
#define TILE_MARKINGS_COLOR				Qt::green

#define DEBUG_DEFAULT	(DEBUG_ALL & \
						~DEBUG_PIXEL_DATUM & \
						~DEBUG_PIXEL_MAPPING & \
						~DEBUG_PIXEL_DATA)

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#else

#include	<iostream>
using std::cin;
using std::cout;
using std::clog;
using std::flush;
using std::boolalpha;
using std::hex;
using std::dec;
using std::setw;
using std::setfill;

#endif

#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	JP2_Image::ID =
		"UA::HiRISE::JP2_Image ($Revision: 2.33 $ $Date: 2014/05/27 17:13:51 $)";


#ifndef JP2_METADATA_GROUP_NAME
#define JP2_METADATA_GROUP_NAME			"JP2"
#endif
Plastic_Image::Name_String
	JP2_Image::JP2_METADATA_GROUP		= JP2_METADATA_GROUP_NAME;

#ifndef MAX_ALLOCATION
#define MAX_ALLOCATION		((unsigned long long)-1)
#endif

#ifndef JP2_RENDERING_INCREMENT_LINES
#define JP2_RENDERING_INCREMENT_LINES	400
#endif


#ifndef	DOXYGEN_PROCESSING
/*==============================================================================
	Image Rendering Monitor
*/
/**	Rendering Process Monitor

	<b>N.B.</b>: The rendering monitor must be registered with the Source
	JP2_Reader to receive its rendering notices. It should be
	deregistered when notices are no longer to be received and forwarded.

	A JP2_Image_Rendering_Monitor object is registered with a JP2_Reader to
	recieve notices from the JP2_Reader at increments during image
	rendering. The notice specifies the image region that was rendered in
	the rendering increment just completed. The notice is {@link
	Plastic_Image::notify_rendering_monitors(Rendering_Monitor::Status,
	const QString&, const QRect&) forwarded} to the owner of the monitor
	- the Plastic_Image base class of the JP2_Image bound to the
	JP2_Image_Rendering_Monitor object when it was created - which
	multiplexes the notice to all listeners that have {@link
	Plastic_Image::add_rendering_monitor(Plastic_Image::Rendering_Monitor*)
	registered with it. The return value of the forwarded notice is
	returned to the JP2_Reader; if the return value is false any
	additional rendering that is pending is canceled.
*/
class JP2_Image_Rendering_Monitor
:	public JP2_Reader::Rendering_Monitor
{
private:

JP2_Image
	*Owner;

public:

JP2_Image_Rendering_Monitor
	(
	JP2_Image*	owner = NULL
	)
{Owner = owner;}

/**	Callback method that receives rendering notices from the JP2_Reader.

	A JP2_Image_Rendering_Monitor object is registered with a JP2_Reader
	to recieve notices from the JP2_Reader at increments during image
	rendering. The notice specifies the image region that was rendered in
	the rendering increment just completed.

	Nothing is done if this JP2_Image_Rendering_Monitor does not have a
	JP2_Image Owner.

	If the region being rendered has a single band Depth, the source
	{@link JP2_Reader::is_rendered_band(unsigned int) band being rendered}
	is found from the reader. Otherwise band zero is used as the reference
	band.

	The image region being rendered is {@link
	map_source_to_display(const PIRL::Cube&, band) mapped} from the full
	resolution source image grid to the JP2_Image Owner image display
	coordinates.

	If the rendering status has a {@link
	#JP2_Reader::Rendering_Monitor::RENDERED_DATA_MASK rendering bit}
	set {@link map_source_data_to_display_data(const QRect&) the source
	image data is mapped to the display image data}.

	If the rendering status indicates that {@link
	#JP2_Reader::Rendering_Monitor::DONE rendering is done} for the
	JP2_Image Owner its {@link source_data_rendered(bool, int) source
	data is marked having completed rendering}.

	The notice is {@link
	Plastic_Image::notify_rendering_monitors(Rendering_Monitor::Status,
	const QString&, const QRect&) forwarded} to the owner of the monitor
	- the Plastic_Image base class of the JP2_Image bound to the
	JP2_Image_Rendering_Monitor object when it was created - which
	multiplexes the notice to all listeners that have {@link
	Plastic_Image::add_rendering_monitor(Plastic_Image::Rendering_Monitor*)
	registered with it. The return value of the forwarded notice is
	returned to the JP2_Reader; if the return value is false any
	additional rendering that is pending is canceled.

	@param	reader	A reference to the JP2_Reader that sent the notice.
	@param	status	A JP2_Reader::Rendering_Monitor::Status value that
		indicates the rendering status.
	@param	message	A message string that provides a brief description
		associated with the status.
	@param	rendered_region	A reference to a Cube that specifies the
		region of the image that has been rendered.
	@param	source_region	A reference to a Cube that specifies the
		region of the source image that is being rendered.
*/
bool
notify
	(
	JP2_Reader&
        #if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
        reader
        #endif
	,JP2_Reader::Rendering_Monitor::Status	status,
	const std::string&						message,
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
	const Cube&								rendered_region,
	#else
	const Cube&,
	#endif
	const Cube&								source_region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
clog << "++> JP2_Image::JP2_Image_Rendering_Monitor::notify:" << endl
	 << "    status " << status << " \"" << message << '"' << endl
	 << "    " << *Owner << endl
	 << "    rendered region " << rendered_region << endl
	 << "                 of " << reader.rendered_region () << endl
	 << "      source region " << source_region << endl
	 << "                 of " << reader.image_region () << endl;
#endif
bool
	continue_rendering = true;
if (Owner)
	{
	/*
		Map the region rendered from the full resolution image grid
		to the Owner's image display coordinates.
	*/
	QRect
		display_region (Owner->map_source_to_display (source_region));

	if (status & JP2_Reader::Rendering_Monitor::RENDERED_DATA_MASK)
		{
		//	Data rendering notification.
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
		clog << "     display region " << display_region << endl
			 << "                 of " << Owner->rect () << endl
			 << "    map_source_data_to_display_data ..." << endl;
		#endif
		if (Owner->source_precision_bytes () == 1)
			Owner->map_source_data_to_display_data<quint8>  (display_region);
		else
			Owner->map_source_data_to_display_data<quint16> (display_region);
		}
	else
	if (status == JP2_Reader::Rendering_Monitor::DONE)
		{
		//	Mark the data buffers as rendered.
		int
			band = 0;
		if (source_region.Depth != 1)
			band = -1;
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
		clog << "    source_data_rendered (true, " << band << ')' << endl;
		#endif
		Owner->source_data_rendered (true, band);
		}

	continue_rendering = Owner->notify_rendering_monitors
		(
		static_cast<Plastic_Image::Rendering_Monitor::Status>(status),
		QString::fromStdString (message),
		display_region
		);
	}
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_NOTIFY | DEBUG_LOCATION))
clog << "<++ JP2_Image::JP2_Image_Rendering_Monitor::notify: "
		<< boolalpha << continue_rendering << endl;
#endif
return continue_rendering;
}
};	//	JP2_Image_Rendering_Monitor
#endif

#ifndef	DOXYGEN_PROCESSING
/*==============================================================================
	Source Data Management
*/
class Source_Data
{
public:

//	The source image band from which data is rendered.
int
	Band;

//	Flags if the buffer content is current with respect to size and resolution.
bool
	Rendered;

/*	Size of each rendered source data area in each buffer.

	The rendered source data is written to the assigned buffer as a
	rectangular area starting at the beginning of the buffer. The
	width of the rendered area is the length of a line of rendered
	pixels in this area; the height is the number of rendered lines.
*/
QSize
	Rendered_Size;

/*	JPEG2000 resolution level of rendered source data in each buffer.

	The JPEG2000 codestream is rendered at the resolution level that
	results in a scaling factor at or above the intended scaling factor.
	The difference between the resolution level scaling factor and the
	intended scaling factor is set in the Differential_Transforms.
*/
unsigned int
	Rendered_Resolution;

//!	Storage for pixel data rendered from the source image.
unsigned char
	*Buffer;


Source_Data
	(
	unsigned long long	amount = 0
	)
	:	Band (-1),
		Rendered (false),
		Rendered_Size (0, 0),
		Rendered_Resolution (0),
		Buffer (NULL)
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_CONSTRUCTORS | DEBUG_BAND_MAP))
clog << ">>> JP2_Image::Source_Data @ " << (void*)this
		<< ": " << amount << endl;
#endif
if (amount)
	{
	try {Buffer = new unsigned char[amount];}
	catch (bad_alloc&)
		{
		ostringstream
			message;
		message
			<< JP2_Image::ID << endl
			<< "Couldn't allocate a " << magnitude_of (amount)
				<< " (" << amount << ") byte source data buffer.";
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | \
						DEBUG_CONSTRUCTORS | \
						DEBUG_BAND_MAP))
		clog << message.str () << endl;
		#endif
		throw out_of_range (message.str ());
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_CONSTRUCTORS | DEBUG_BAND_MAP))
clog << "    buffer @ " << (void*)Buffer << endl
	 << "<<< JP2_Image::Source_Data" << endl;
#endif
}


Source_Data&
operator=
	(
	const Source_Data&	source_data
	)
{
if (this != &source_data)
	{
	Band = source_data.Band;
	Rendered = source_data.Rendered;
	Rendered_Size = source_data.Rendered_Size;
	Rendered_Resolution = source_data.Rendered_Resolution;
	//	N.B.: Shared data buffer.
	Buffer = source_data.Buffer;
	}
return *this;
}


~Source_Data ()
{
if (Buffer)
	delete[] Buffer;
}


Source_Data&
operator|=
	(
	const Source_Data&	source_data
	)
{
if (Band   == source_data.Band &&
	Buffer == source_data.Buffer)
	{
	Rendered = source_data.Rendered;
	Rendered_Size = source_data.Rendered_Size;
	Rendered_Resolution = source_data.Rendered_Resolution;
	}
return *this;
}

};	//	Class Source_Data


std::ostream&
operator<<
	(
	std::ostream&					stream,
	const UA::HiRISE::Source_Data&	source_data
	)
{
return stream
	<< "source band " << source_data.Band
	<< " buffer @ " << (void*)source_data.Buffer
	<< " is" << (source_data.Rendered ? "" : " not")
	<< " rendered with size " << source_data.Rendered_Size
	<< " at resolution level " << source_data.Rendered_Resolution;
}
#endif


void
JP2_Image::assign_source_data_buffers ()
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::assign_source_data_buffers" << endl
	 << "    Band_Map = " << Band_Map << endl));
#endif
if (! Data_Buffers[0] ||	//	No buffer pool.
	closed ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
	LOCKED_LOGGING ((
	clog << "    closed" << endl
		 << "<<< JP2_Image::assign_source_data_buffers" << endl));
	#endif
	return;
	}

#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
LOCKED_LOGGING ((
clog << "    Data_Buffers pool -" << endl));
#endif
Source_Data
	*data_buffers[3];
int
	band = -1;
while (++band < 3)
	{
	//	Pool of buffers.
	#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
	if (Data_Buffers[band])
		{
		LOCKED_LOGGING ((
		clog << "      display band " << band << " from "
				<< *Data_Buffers[band] << endl));
		}
	#endif
	data_buffers[band] = Data_Buffers[band];
	Display_Data_Buffers[band] = NULL;
	}

#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
LOCKED_LOGGING ((
clog << "    reused -" << endl));
#endif
int
	index;
band = -1;
while (++band < 3)
	{
	if (Display_Data_Buffers[band])
		//	Already assigned (duplicate).
		continue;

	index = -1;
	//	Search the pool for a buffer for the mapped source band.
	while (++index < 3)
		{
		if (data_buffers[index] &&
			data_buffers[index]->Band == (int)Band_Map[band])
			{
			Display_Data_Buffers[band] = data_buffers[index];
			data_buffers[index] = NULL;
			#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
			LOCKED_LOGGING ((
			clog << "      display band " << band << " from "
					<< *Display_Data_Buffers[band] << endl));
			#endif
			break;
			}
		}
	if (Display_Data_Buffers[band])
		{
		//	Possible duplicate band mapping.
		index = band;
		while (++index < 3)
			{
			if (Band_Map[index] == Band_Map[band])
				{
				Display_Data_Buffers[index] = Display_Data_Buffers[band];
				#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
				LOCKED_LOGGING ((
				clog << "     +display band " << index
						<< " from " << *Display_Data_Buffers[index] << endl));
				#endif
				}
			}
		}
	}

#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
LOCKED_LOGGING ((
clog << "    reassigned -" << endl));
#endif
band = -1;
while (++band < 3)
	{
	if (Display_Data_Buffers[band])
		//	Already assigned.
		continue;

	//	Take the next available buffer.
	index = -1;
	while (++index < 3)
		{
		if (data_buffers[index])
			{
			//	Reassign the buffer to a different source band.
			#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
			LOCKED_LOGGING ((
			clog << "               reassigned "
				<< *data_buffers[index] << endl));
			#endif
			Display_Data_Buffers[band] = data_buffers[index];
			Display_Data_Buffers[band]->Band = Band_Map[band];
			Display_Data_Buffers[band]->Rendered = false;
			Display_Data_Buffers[band]->Rendered_Size = QSize ();
			Display_Data_Buffers[band]->Rendered_Resolution = 0;
			data_buffers[index] = NULL;
			#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
			LOCKED_LOGGING ((
			clog << "      display band " << index << " from "
					<< *Display_Data_Buffers[band] << endl));
			#endif
			break;
			}
		}
	if (! Display_Data_Buffers[band])
		{
		//	Insufficient buffers in the pool!
		ostringstream
			message;
		message
			<< ID << endl
			<< "Couldn't assign a source data buffer for display band "
				<< band << endl
			<< "because there are insufficient data buffers available!";
		throw out_of_range (message.str ());
		}

	//	Possible duplicate band mapping.
	index = band;
	while (++index < 3)
		{
		if (Band_Map[index] == Band_Map[band])
			{
			Display_Data_Buffers[index] = Display_Data_Buffers[band];
			#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
			LOCKED_LOGGING ((
			clog << "     +display band " << index
					<< " from " << *Display_Data_Buffers[index] << endl));
			#endif
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
LOCK_LOG;
clog << "    remaining in the buffer pool -" << endl;
band = -1;
while (++band < 3)
	if (data_buffers[band])
		clog << "      " << *data_buffers[band] << endl;
clog << "<<< JP2_Image::assign_source_data_buffers" << endl;
UNLOCK_LOG;
#endif
}


void**
JP2_Image::source_data_buffers ()
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::source_data_buffers" << endl
	 << "    " << *this << endl));
#endif
int
	band = source_bands ();
while (band--)
	Source_Data_Buffers[band] = NULL;

band = -1;
while (++band < 3)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
	LOCKED_LOGGING ((
	clog << "    display band " << band
			<< " from " << *Rendering_Display_Data_Buffers[band] << endl));
	#endif
	Source_Data_Buffers[Rendering->Band_Map[band]]
		= Rendering_Display_Data_Buffers[band]->Buffer;
	}
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << "<<< JP2_Image::source_data_buffers" << endl));
#endif
return Source_Data_Buffers;
}


bool
JP2_Image::rendered_source_data
	(
	int		band
	)
	const
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::rendered_source_data: " << band << endl
	 << "    " << *this << endl));
#endif
bool
	rendered = true;
if (band < 0)
	{
	band = -1;
	while (++band < 3)
		{
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Display_Data_Buffers[" << band << "] = "
				<< (void*)Display_Data_Buffers[band] << endl));
		if (Display_Data_Buffers[band])
			{
			LOCKED_LOGGING ((
				clog << "      Rendered = " << boolalpha
						<< Display_Data_Buffers[band]->Rendered << endl));
			}
		#endif
		if (! Display_Data_Buffers[band] ||
			! Display_Data_Buffers[band]->Rendered)
			{
			rendered = false;
			break;
			}
		}
	}
else
if (band < 3)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
	LOCKED_LOGGING ((
	clog << "    Display_Data_Buffers[" << band << "] = "
			<< (void*)Display_Data_Buffers[band] << endl));
	if (Display_Data_Buffers[band])
		{
		LOCKED_LOGGING ((
			clog << "      Rendered = " << boolalpha
					<< Display_Data_Buffers[band]->Rendered << endl));
		}
	#endif
	rendered =
		Display_Data_Buffers[band] &&
		Display_Data_Buffers[band]->Rendered;
	}
else
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't determine if source data is rendered for band "
			<< band << endl
		<< "because there are only 3 display bands.";
	throw invalid_argument (message.str ());
	}
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::rendered_source_data: "
	<< boolalpha << rendered << endl));
#endif
return rendered;
}


void
JP2_Image::source_data_rendered
	(
	bool	rendered,
	int		band
	)
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << ">-< JP2_Image::source_data_rendered: "
		<< boolalpha << rendered << ", band " << band << endl));
#endif
if (Rendering_Display_Data_Buffers[0])
	{
	if (band < 0)
		{
		Rendering_Display_Data_Buffers[0]->Rendered =
		Rendering_Display_Data_Buffers[1]->Rendered =
		Rendering_Display_Data_Buffers[2]->Rendered = rendered;
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Rendering_Display_Data_Buffers[0] "
				<< *Rendering_Display_Data_Buffers[0] << endl
			 << "    Rendering_Display_Data_Buffers[1] "
				<< *Rendering_Display_Data_Buffers[1] << endl
			 << "    Rendering_Display_Data_Buffers[2] "
				<< *Rendering_Display_Data_Buffers[2] << endl));
		#endif
		}
	else
	if (band < 3)
		{
		Rendering_Display_Data_Buffers[band]->Rendered = rendered;
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Rendering_Display_Data_Buffers[" << band << "] "
				<< *Rendering_Display_Data_Buffers[band] << endl));
		#endif
		}
	else
		{
		ostringstream
			message;
		message
			<< ID << endl
			<< "Can't set source data rendered for band " << band << endl
			<< "because there are only 3 display bands.";
		throw invalid_argument (message.str ());
		}
	}
}


void
JP2_Image::source_buffer_rendered
	(
	bool	rendered,
	void*	buffer
	)
{
#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << ">-< JP2_Image::source_buffer_rendered: "
		<< boolalpha << rendered << ", buffer @ " << buffer << endl));
#endif
if (Rendering_Data_Buffers[0])
	{
	if (Rendering_Data_Buffers[0]->Buffer == buffer)
		{
		Rendering_Data_Buffers[0]->Rendered = rendered;
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Rendering_Display_Data_Buffers[0] "
				<< *Rendering_Display_Data_Buffers[0] << endl));
		#endif
		}
	else
	if (Rendering_Data_Buffers[1]->Buffer == buffer)
		{
		Rendering_Data_Buffers[1]->Rendered = rendered;
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Rendering_Display_Data_Buffers[1] "
				<< *Rendering_Display_Data_Buffers[1] << endl));
		#endif
		}
	else
	if (Rendering_Data_Buffers[2]->Buffer == buffer)
		{
		Rendering_Data_Buffers[2]->Rendered = rendered;
		#if ((DEBUG_SECTION) & (DEBUG_DATA_BUFFERS | DEBUG_RENDER))
		LOCKED_LOGGING ((
		clog << "    Rendering_Display_Data_Buffers[2] "
				<< *Rendering_Display_Data_Buffers[2] << endl));
		#endif
		}
	}
}

/*==============================================================================
	Constructors
*/
JP2_Image::JP2_Image
	(
	JP2_Reader*				JP2_reader,
	const QSize&			size,
	const unsigned int*		band_map,
	const QTransform**		transforms,
	const Data_Map**		data_maps
	)
	:
	Plastic_Image (! size.isEmpty () ? size
		: (JP2_reader ?
			(QSize (JP2_reader->rendered_width (),
					JP2_reader->rendered_height ())
				*= (1.0 / JP2_reader->resolution_level ()))
			: size),
		band_map, transforms, data_maps),
	Source (JP2_reader),
	Source_Rendering_Monitor (NULL),
	Data_Buffer_Size (0)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2_Reader: From JP2_Reader* @ " << (void*)JP2_reader << endl
	 << "     specified size = " << size << endl
	 << "    rendered region = ";
if (JP2_reader)
	clog << JP2_reader->rendered_region ();
else
	clog << "NULL";
clog << endl;
#endif

initialize ();

if (! Auto_Update ||
	! JP2_reader)
	fill (background_color ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< JP2_Image" << endl;
#endif
}


JP2_Image::JP2_Image
	(
	const JP2_Reader&		JP2_reader,
	const QSize&			size,
	const unsigned int*		band_map,
	const QTransform**		transforms,
	const Data_Map**		data_maps
	)
	:
	Plastic_Image (! size.isEmpty () ? size
		: (QSize (JP2_reader.rendered_width (),
				  JP2_reader.rendered_height ())
				*= (1.0 / JP2_reader.resolution_level ())),
		band_map, transforms, data_maps),
	Source (JP2_reader.clone ()),
	Source_Rendering_Monitor (NULL),
	Data_Buffer_Size (0)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2_Reader: From JP2_Reader& @ " << (void*)&JP2_reader << endl
	 << "     specified size = " << size << endl
	 << "    rendered region = " << JP2_reader.rendered_region () << endl;
#endif

initialize ();

if (! Auto_Update ||
	! Source)
	fill (background_color ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< JP2_Image" << endl;
#endif
}


JP2_Image::JP2_Image
	(
	const JP2_Image&	image,
	const QSize&		size,
	Mapping_Type		shared_mappings
	)
	:
	Plastic_Image (! size.isEmpty () ? size : image.size (),
		(shared_mappings & BAND_MAP)   ? image.source_band_map () : NULL,
		(shared_mappings & TRANSFORMS) ?
			const_cast<const QTransform**>(image.source_transforms ()) : NULL,
		(shared_mappings & DATA_MAPS)  ?
			const_cast<const Data_Map**>(image.source_data_maps ()) : NULL),
	Source (image.Source),	//	Share the JP2_Reader.
	Source_Rendering_Monitor (NULL),
	Data_Buffer_Size (0)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2_Image: Copy " << image.source_name () << endl
	 << "     specified size = " << size << endl
	 << "         image size = " << image.size () << endl
	 << "    shared_mappings = " << shared_mappings
	 	<< ": " << mapping_type_names (shared_mappings) << endl
	 << "    Source: ";
if (Source)
	clog << Source->image_size () << ", "
			<< Source->image_bands () << 'b' << endl;
else
	clog << "NULL";
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

#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
background_color (qRgb (0, 255, 0));
#else
background_color (image.background_color ());
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    background_color = #" << setw (8) << setfill ('0') << hex
		<< background_color () << dec << setfill (' ') << endl;
#endif

if ((Auto_Update = do_update))
	update ();
else
	fill (background_color ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< JP2_Image" << endl;
#endif
}


JP2_Image::JP2_Image
	(
	const QString&	name,
	const QSize&	size
	)
	:
	Plastic_Image (size),
	Source (NULL),
	Source_Rendering_Monitor (NULL),
	Data_Buffer_Size (0)
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> JP2_Image: " << name << endl
	 << "    specified size = " << size << endl;
#endif
try {Source = JP2::reader (name.toStdString ());}
catch (exception&
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		except
		#endif
		)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    Failed to construct a JP2_Reader for the source -" << endl
		 << except.what () << endl;
	#endif
	}

initialize ();

source_name (name);

if (! Auto_Update ||
	! Source)
	fill (background_color ());
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< JP2_Image" << endl;
#endif
}


void
JP2_Image::initialize ()
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
clog << ">>> JP2_Image::initialize @ " << (void*)this << endl;
#endif
fill (background_color ());

bool
	do_update = auto_update (false);

Data_Buffers[0] =
Data_Buffers[1] =
Data_Buffers[2] =
Display_Data_Buffers[0] =
Display_Data_Buffers[1] =
Display_Data_Buffers[2] = NULL;

//	Initialize the base class object.
Plastic_Image::initialize ();

if (! Source)
	{
	close ();
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
	clog << "    no Source" << endl
		 << "<<< JP2_Image::initialize" << endl;
	#endif
	return;
	}

//	Determine the size of a source data buffer.
int
	image_width = Source->image_width (),
	image_height = Source->image_height (),
	bands = Source->image_bands ();
Source_Data_Buffers = new void*[bands];
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
clog << "      image size = "
		<< image_width << "w, " << image_height << "h, " << bands << "b, "
		<< Source->rendered_pixel_bytes () << " byte"
		<< ((Source->rendered_pixel_bytes () == 1) ? ", " : "s, ")
		<< Source->rendered_pixel_bits () << " bits" << endl
	 << "    display size = " << size () << endl;
#endif
if (! image_width ||
	! image_height ||
	! bands ||
	! Source->rendered_pixel_bytes ())
	{
	close ();
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
	clog << "    no rendered image content" << endl
		 << "<<< JP2_Image::initialize" << endl;
	#endif
	return;
	}
/*
	The source image band data buffer will be sized to the smaller
	of the full source size or twice the image display size (to
	allow for rendering size to the next higher resolution level
	for the current scaling).
*/
if (image_width  > (width ()  << 1) ||
	image_height > (height () << 1))
	{
	image_width  = (width ()  << 1) + 1;
	image_height = (height () << 1) + 1;
	}

bool
	overflow = false;
unsigned long long
	value = image_width,
	amount = image_width;
amount *= image_height;
if ((amount / image_height) != value)
	overflow = true;
if (! overflow)
	{
	value = amount;
	amount *= Source->rendered_pixel_bytes ();
	if ((amount / Source->rendered_pixel_bytes ()) != value)
		overflow = true;
	}
if (overflow ||
	amount > MAX_ALLOCATION)
	{
	close ();
	ostringstream
		message;
	message
		<< ID << endl
		<< "The required pixel data buffer size for a "
			<< Source->image_width () << "w, "
			<< Source->image_height () << "h, "
			<< Source->rendered_pixel_bits () << " byte image band" << endl
		<< "is greater than the maximum "
			<< magnitude_of (MAX_ALLOCATION) << " byte allocation size.";
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
	clog << message.str () << endl;
	#endif
	throw out_of_range (message.str ());
	}
Data_Buffer_Size = amount;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
clog << "    Data_Buffer_Size = " << magnitude_of (Data_Buffer_Size)
		<< " (" << Data_Buffer_Size << ") bytes" << endl;
#endif

//	Source data buffers allocations.
if (bands > 3)
	bands = 3;
int
	band = -1;
while (++band < bands)
	{
	try {Data_Buffers[band] = new Source_Data (Data_Buffer_Size);}
	catch (out_of_range&)
		{
		close ();
		throw;
		}
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
	clog << "    Data_Buffers[" << band << "]: " << *Data_Buffers[band] << endl;
	#endif
	//	Used during rendering.
	Rendering_Data_Buffers[band] = new Source_Data;
	}
//	Fill all Rendering_Data_Buffers.
while (band < 3)
	Rendering_Data_Buffers[band++] = new Source_Data;

//	Initial assignment of data buffers to bands.
assign_source_data_buffers ();

//	Use band-sequential format.
Source->image_data_format (JP2_Reader::FORMAT_BSQ);

//	Set the source origin and scale consistent with the rendering region.
Cube
	rendered_region (Source->rendered_region ());
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
clog << "    Source rendered_region = " << rendered_region << endl
	 << "          resolution level = " << Source->resolution_level () << endl;
#endif
source_origin (QPointF (rendered_region.X, rendered_region.Y));
source_scale (1.0 / Source->resolution_level ());

//	Construct the Source JP2 Rendering_Monitor.
Source_Rendering_Monitor = new JP2_Image_Rendering_Monitor (this);

//	Reset auto-update.
if ((Auto_Update = do_update))
	update ();
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_DATA_BUFFERS | DEBUG_BAND_MAP))
clog << "<<< JP2_Image::initialize" << endl;
#endif
}


JP2_Image::~JP2_Image ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> ~JP2_Image @ " << (void*)this << endl;
#endif
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "    deleting Display_Data_Buffers" << endl;
#endif
for (int
		band = 0;
		band < 3;
		band++)
	{
	if (Data_Buffers[band])
		{
		#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
		clog << "    delete Data_Buffers[" << band << "] - "
				<< *Data_Buffers[band] << endl;
		#endif
		delete Data_Buffers[band];
		}
	if (Rendering_Data_Buffers[band])
		{
		//	Buffer is shared with the Data_Buffers.
		Rendering_Data_Buffers[band]->Buffer = NULL;
		delete Rendering_Data_Buffers[band];
		}
	}

if (Source_Data_Buffers)
	{
	#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
	clog << "    delete Source_Data_Buffers array @ "
			<< (void*)Source_Data_Buffers << endl;
	#endif
	delete[] Source_Data_Buffers;
	}

if (Source_Rendering_Monitor)
	delete Source_Rendering_Monitor;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< ~JP2_Image" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
JP2_Image*
JP2_Image::clone
	(
	const QSize&	size,
	Mapping_Type	shared_mappings
	) const
{return new JP2_Image (*this, size, shared_mappings);}


const void*
JP2_Image::source () const
{return Source;}


void
JP2_Image::close ()
{
QMutexLocker
	object_lock (&Object_Lock);
if (Source &&
	! is_rendering ())
	Source->close ();
Plastic_Image::close ();
}

/*------------------------------------------------------------------------------
	Metadata
*/
idaeim::PVL::Aggregate*
JP2_Image::metadata ()
{
QMutexLocker
	object_lock (&Object_Lock);
#if ((DEBUG_SECTION) & DEBUG_METADATA)
clog << ">>> JP2_Image::metadata" << endl;
#endif
Aggregate
	*metadata = NULL;
if (Metadata)
	metadata = Plastic_Image::metadata ();
else
if (Source)
	{
	//	Initialize Metadata.
	metadata = Plastic_Image::metadata ();
	if (metadata)
		{
		//	Copy out the JP2 metadata from the Source.
		Aggregate
			*JP2_metadata (new Aggregate
				(*(static_cast<JP2_Metadata*>(Source)
				->metadata_parameters ())));
		JP2_metadata->name (JP2_METADATA_GROUP);
		//	Put the JP2 metadata into the root metadata.
		metadata->add (JP2_metadata);
		#if ((DEBUG_SECTION) & DEBUG_METADATA)
		clog << "    basic metadata appended with JP2 metadata" << endl;
		#endif
		}
	}
#if ((DEBUG_SECTION) & DEBUG_METADATA)
clog << "<< JP2_Image::metadata" << endl;
#endif
return metadata;
}

/*------------------------------------------------------------------------------
	Image Characterization
*/
QSize
JP2_Image::source_size () const
{return Source ?
	QSize (Source->image_width (), Source->image_height ()) : QSize (0, 0);}


unsigned int
JP2_Image::source_bands () const
{
if (Source)
	return Source->image_bands ();
return 0;
}


unsigned int
JP2_Image::source_precision_bits () const
{
if (Source)
	return Source->rendered_pixel_bits ();
return 0;
}


Plastic_Image::Pixel_Datum
JP2_Image::source_pixel_value
	(
	unsigned int	x,
	unsigned int	y,
	unsigned int	band
	)
	const
{
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
clog << ">>> JP2_Image::source_pixel_value: "
		<< x << "x, " << y << "y, " << band << 'b' << endl;
#endif
if (Source &&
	Display_Data_Buffers[band]->Rendered &&
	band < 3)
	{
	//	Map the image coordinate to the source data location.
	QPoint
		origin (round_down (source_origin (band)));
	#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
	clog << "       origin = " << origin << endl;
	#endif
	unsigned int
		resolution = Display_Data_Buffers[band]->Rendered_Resolution;
	if (resolution &&
		x >= static_cast<unsigned int>(origin.rx ()) &&
		y >= static_cast<unsigned int>(origin.ry ()))
		{
		//	Offset into the band data.
		--resolution;
		x -= origin.rx ();
		x >>= resolution;
		y -= origin.ry ();
		y >>= resolution;
		#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
		clog << "    Rendered_Resolution = " << (resolution + 1) << endl
			 << "                 offset = "
			 	<< x << "x, " << y << 'y' << endl
			 << "          Rendered_Size = "
			 	<< Display_Data_Buffers[band]->Rendered_Size << endl;
		#endif
		if (x < static_cast<unsigned int>
				((Display_Data_Buffers[band]->Rendered_Size).rwidth ()) &&
			y < static_cast<unsigned int>
				((Display_Data_Buffers[band]->Rendered_Size).rheight ()))
			{
			Pixel_Datum
				value;
			//	N.B.: Presumes BSQ data organization.
			y *= (Display_Data_Buffers[band]->Rendered_Size).rwidth ();
			y += x;
			if (Source->rendered_pixel_bytes () == 1)
				value = *(reinterpret_cast<quint8*>
					(Display_Data_Buffers[band]->Buffer) + y);
			else
				value = *(reinterpret_cast<quint16*>
					(Display_Data_Buffers[band]->Buffer) + y);
			#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
			clog << "<<< JP2_Image::source_pixel_value: " << value << endl;
			#endif
			return value;
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
clog << "<<< JP2_Image::source_pixel_value: " << UNDEFINED_PIXEL_VALUE << endl;
#endif
return UNDEFINED_PIXEL_VALUE;
}


QPoint
JP2_Image::map_image_to_source
	(
	const QPoint&	image_coordinate,
	int				band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_IMAGE_TO_SOURCE | DEBUG_HISTOGRAMS))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::map_image_to_source:" << endl
		<< "        image_coordinate = " << image_coordinate
		<< ", " << band << 'b' << endl));
#endif
if (band < 0)
	band = 0;
if (Source &&
	band < 3)
	{
	QPoint
		source_coordinate (image_coordinate),
		origin (round_down (source_origin (band)));
	unsigned int
		resolution = Display_Data_Buffers[band]->Rendered_Resolution;
	#if ((DEBUG_SECTION) & (DEBUG_IMAGE_TO_SOURCE | DEBUG_HISTOGRAMS))
	clog << "                  origin = " << origin << endl
		 << "     Rendered_Resolution = " << resolution << endl;
	#endif
	if (resolution &&
		source_coordinate.rx () >= origin.rx () &&
		source_coordinate.ry () >= origin.ry ())
		{
		//	Offset into the band data.
		source_coordinate -= origin;
		#if ((DEBUG_SECTION) & (DEBUG_IMAGE_TO_SOURCE | DEBUG_HISTOGRAMS))
		clog << "                  offset = " << source_coordinate << endl;
		#endif
		//	Scale to the rendering resolution.
		--resolution;
		source_coordinate.rx () >>= resolution;
		source_coordinate.ry () >>= resolution;
		#if ((DEBUG_SECTION) & (DEBUG_IMAGE_TO_SOURCE | DEBUG_HISTOGRAMS))
		clog << "       source coordinate = " << source_coordinate << endl
			 << "     Rendered_Size = "
			 	<< Display_Data_Buffers[band]->Rendered_Size << endl;
		#endif
		if (source_coordinate.rx () < static_cast<int>
				((Display_Data_Buffers[band]->Rendered_Size).rwidth ()) &&
			source_coordinate.ry () < static_cast<int>
				((Display_Data_Buffers[band]->Rendered_Size).rheight ()))
			{
			#if ((DEBUG_SECTION) & (DEBUG_IMAGE_TO_SOURCE | DEBUG_HISTOGRAMS))
			clog << "<<< JP2_Image::map_image_to_source: "
				<< source_coordinate << endl;
			#endif
			return source_coordinate;
			}
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_IMAGE_TO_SOURCE | DEBUG_HISTOGRAMS))
clog << "<<< JP2_Image::map_image_to_source: " << QPoint (-1, -1) << endl;
#endif
return QPoint (-1, -1);
}

/*------------------------------------------------------------------------------
	Histograms
*/
unsigned long long
JP2_Image::source_histograms
	(
	QVector<Histogram*>	histograms,
	const QRect&		source_region
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> JP2_Image::source_histograms:" << endl
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
if (! rendered_source_data () ||
	source_region.isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
	clog << "    nothing to render" << endl
		 << "<<< JP2_Image::source_histograms: 0" << endl;
	#endif
	return 0;
	}

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

QRect
	source_data_region;
QPoint
	source_data_BR;
unsigned long long
	count,
	max_count = 0;
for (int
		band = 0;
		band < 3;
	  ++band)
	{
	if (refresh_source_histogram (histograms, band))
		{
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "       image region UL = "
				<< selected_region.topLeft () << endl;
		#endif
		source_data_region.setTopLeft
			(map_image_to_source (selected_region.topLeft (), band));
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "        source data UL = "
				<< source_data_region.topLeft () << endl
			 << "       image region BR = "
		 		<< selected_region.bottomRight () << endl;
		#endif
		source_data_region.setBottomRight
			(map_image_to_source (selected_region.bottomRight (), band));
		#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
		clog << "        source data BR = "
				<< source_data_region.bottomRight () << endl
			 << "    source data region = "
		 		<< source_data_region << endl;
		#endif
		if (source_data_region.left ()  < 0 ||
			source_data_region.right () < 0)
			{
			//	This can happen if rendering is in progress.
			#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
			clog << "!!! Invalid image to source coordinate mapping!" << endl;
			#endif
			continue;
			}

		if (Source->rendered_pixel_bytes () == 1)
			source_data_histogram<quint8>
				(histograms[band], source_data_region, band);
		else
			source_data_histogram<quint16>
				(histograms[band], source_data_region, band);

		count =
			(unsigned long long)source_data_region.width ()
				* source_data_region.height ();
		if (max_count < count)
			max_count = count;
		#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
		clog << "    band " << band << " -" << endl;
		clog << histograms[band];
		#endif
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "<<< JP2_Image::source_histograms: " << max_count << endl;
#endif
return max_count;
}


template<typename Pixel_Data_Type>
void
JP2_Image::source_data_histogram
	(
	Histogram*		histogram,
	const QRect&	source_data_region,
	int				band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << ">>> JP2_Image::source_data_histogram:" << endl
	 << "    histogram @ " << (void*)histogram << endl
	 << "    source_data_region = " << source_data_region << endl
	 << "    band = " << band << endl;
#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
int
	line = source_data_region.top (),
	line_number_digits = decimal_digits
		(source_data_region.y () + source_data_region.height ()),
	mask = (1 << source_precision_bits ()) - 1,
	pixel_value_digits = hex_digits (mask);
#endif
#endif
unsigned int
	source_data_width = (Display_Data_Buffers[band]->Rendered_Size).width (),
	line_samples = source_data_region.width (),
	max_value = histogram->size ();
Pixel_Data_Type
	*data =
		reinterpret_cast<Pixel_Data_Type*>(Display_Data_Buffers[band]->Buffer)
		+ source_data_region.left ()
		+ (source_data_region.top () * source_data_width),
	*end_data = data + (source_data_region.height () * source_data_width),
	*datum,
	value,
	*end_datum;
while (data < end_data)
	{
	datum = data;
	end_datum = datum + line_samples;
	#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
	clog << "    " << setw (line_number_digits) << line++ << ':' << hex;
	#endif
	while (datum < end_datum)
		{
		if ((value = *(datum++)) < max_value)
			{
			(*histogram)[value]++;
			#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
			clog << ' ' << setw (pixel_value_digits) << ((int)value & mask);
			#endif
			}
		#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
		else
			clog << setw (pixel_value_digits + 1) << ' ';
		#endif
		}
	data += source_data_width;
	#if ((DEBUG_SECTION) & DEBUG_PRINT_HISTOGRAMS)
	clog << dec << endl;
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_HISTOGRAMS | DEBUG_PRINT_HISTOGRAMS))
clog << "<<< JP2_Image::source_data_histogram" << endl;
#endif
}

/*------------------------------------------------------------------------------
	Band mapping
*/
bool
JP2_Image::source_band_map
	(
	const unsigned int*	band_map,
	bool				shared
	)
{
Update.start ();

if (Closed)
	{
	Update.end ();
	return false;
	}

#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << ">>> JP2_Image::source_band_map:" << band_map << endl
	 << "    shared = " << boolalpha << shared << endl;
#endif
bool
	do_update = auto_update (false),
	changed = Plastic_Image::source_band_map (band_map, shared);
if (changed)
	//	Re-assign the the source data buffers.
	assign_source_data_buffers ();

auto_update (do_update);
changed |= needs_update (changed ? BAND_MAP : NO_MAPPINGS);
Update.end ();
#if ((DEBUG_SECTION) & DEBUG_BAND_MAP)
clog << "<<< JP2_Image::source_band_map: " << boolalpha << changed << endl;
#endif
return changed;
}

/*==============================================================================
	Rendering
*/
bool
JP2_Image::needs_update
	(
	Mapping_Type	changed
	)
	throw (Render_Exception, std::bad_exception)
{
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
clog << ">>> JP2_Image::needs_update: " << changed
		<< ": " << mapping_type_names (changed) << endl
	 << "    " << *this << endl;
#endif
Update.start ();

if (changed & BAND_MAP)
	//	Re-assign the the source data buffers.
	assign_source_data_buffers ();

bool
	updated = Plastic_Image::needs_update (changed);

Update.end ();
#if ((DEBUG_SECTION) & (DEBUG_UPDATE | DEBUG_MANIPULATORS))
clog << "<<< JP2_Image::needs_update: " << updated << endl;
#endif
return updated;
}


bool
JP2_Image::render_image ()
	throw (Render_Exception, std::bad_exception)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::render_image" << endl
	 << "    " << *this << endl));
#endif
Update.start ();

if (Closed)
	{
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "    Closed" << endl));
	#endif
	Update.end (Update.SEQUENCE_END);
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "<<< JP2_Image::render_image: false" << endl));
	#endif
	return false;
	}

if (update_canceled ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "    update canceled before rendering" << endl));
	#endif
	cancel_update (false);
	Update.end (Update.SEQUENCE_END);
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "<<< JP2_Image::render_image: false" << endl));
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

//	Render the source data.
bool
	rendered = false;
string
	exception_report;
try
	{
	/*	Render the Source data.

		The Source pixel data required for the display image at the
		current source image location and scaling will be rendered from
		the source JPEG2000 codestream into the Data_Buffers as neeed.

		In addition, the Source data will be mapped into the Display
		data via the JP2_Image_Rendering_Monitor call-back notify from
		the source rendering procedure.
	*/
	rendered = render_source ();
	}
catch (JP2_Exception& except)
	{exception_report = except.message ();}
catch (exception& except)
	{exception_report = except.what ();}
catch (...)
	{exception_report = "Unexpected exception!";}
if (! exception_report.empty ())
	{
	if (! Source->is_open ())
		close ();
	is_rendering (false);
	ostringstream
		message;
	message
		<< ID << endl
		<< "Failed to render the image source -" << endl
		<< source_name () << endl
		/*<< TODO exception_report*/;
	throw Render_Exception (message.str ());
	}

/*	Check for canceled rendering.

	This is  as a test-and-reset operation to avoid a possible race.
*/
bool
	canceled = cancel_update (false);

#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
//	Tile markings.
double
	origin_x = Rendering->Geo_Transforms[0]->m31 (),
	origin_y = Rendering->Geo_Transforms[0]->m32 (),
	scaling  = Rendering->Geo_Transforms[0]->m11 ();
QString
	label = QString (" JP2 image @ %1: so %2, %3; s %4")
	.arg ((ulong)this, 0, 16)
	.arg (origin_x)
	.arg (origin_y)
	.arg (scaling)
	.append (canceled ? " CANCELED" : "")
	.append (rendered ? "" : " INCOMPLETE");
mark_image (this, label,
	TILE_MARKINGS_Y,
	TILE_MARKINGS_COLOR);
mark_image (this, label,
	height () - TILE_MARKINGS_Y - 12,
	TILE_MARKINGS_COLOR);
#endif

if (canceled)
	rendered = false;
if (rendered)
	Rendering->Needs_Update = NO_MAPPINGS;

cancel_update (false);	//	Always clear the cancel status.

//	End rendering.
is_rendering (false);
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "<<< JP2_Image::render_image: "
		<< boolalpha << rendered << endl));
#endif
return rendered;
}


bool
JP2_Image::render_source ()
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::render_source:" << endl
	 << "    " << *this << endl));
#endif
bool
	rendered = true;
Cube
	expected,
	actual;
int
	bands = source_bands ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    source_bands = " << bands << endl));
#endif
if (bands > 3)
	bands = 3;

#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    mapping_differences = " << mapping_differences () << endl));
#endif
if (Rendering->Mapping_Differences & TRANSFORMS)
	{
	//	Geometric differences between the different bands.

	/*	Register the JP2_Reader::Rendering_Monitor.

		The Rendering_Monitor will receive incremental rendering
		notices from the Source render method. Those notices that
		indicate that new pixel data is available will result in
		the map_source_data_to_display_data method being called
		to produce the final Display data.
	*/
	Source->rendering_monitor (Source_Rendering_Monitor);

	/*	Register the pixel data storage with the JP2_Reader.

		N.B.: This will automatically select the corresponding bands
		for rendering.
	*/
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    registering Source_Data_Buffers as Source image_data"
			<< endl));
	#endif
	Source->image_data (source_data_buffers (), Data_Buffer_Size);

	//	Render each band separately; start by turning off all bands.
	Source->render_band (JP2_Reader::ALL_BANDS, false);

	int
		band = -1;
	while (rendered &&
			++band < bands)
		{
		if (rendering_resolution_and_region (band) ||
			Rendering->Needs_Update & (TRANSFORMS & BAND_MAP))
			{
			//	Reset the display image band to background.
			fill (Rendering->Background_Color, band);

			#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
			LOCKED_LOGGING ((
			clog << "    rendering display band " << band << " -> "
					<< *Rendering_Display_Data_Buffers[band] << endl));
			#endif
			Source->render_band (Rendering->Band_Map[band], true);
			if (band)
				Source->render_band (Rendering->Band_Map[band - 1], false);

			//	Resolution and region determines rendered region.
			expected = Source->rendered_region ();

			/*	Render the source.

				The JPEG2000 codestream is fetched and decompressed
				into rendered Source pixel data. The rendering
				process is expected to be done incrementally with
				notifications being sent after each increment to the
				registered Rendering_Monitor.
			*/
			try {actual = Source->render ();}
			catch (JP2_Exception&)
				{
				//	Deregister the JP2_Reader::Rendering_Monitor.
				Source->rendering_monitor (NULL);
				throw;
				}
			catch (...)
				{
				Source->rendering_monitor (NULL);
				throw;
				}

			#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
			LOCKED_LOGGING ((
			clog << "      rendered region " << actual << ' '
					<< ((actual == expected) ? '=' : '!')
					<< "= " << expected << " expected region" << endl));
			#endif
			if (actual != expected)
				rendered = false;
			else
				source_buffer_rendered (true,
					Source_Data_Buffers[Rendering->Band_Map[band]]);
			}
		}

	//	Deregister the JP2_Reader::Rendering_Monitor.
	Source->rendering_monitor (NULL);
	}
else
	{
	//	All bands rendered with the same geometry.
	if (rendering_resolution_and_region ())
		{
		//	Source data rendering needed.

		//	Reset the display image to background.
		fill (Rendering->Background_Color);

		/*	Register the JP2_Reader::Rendering_Monitor.

			The Rendering_Monitor will receive incremental rendering
			notices from the Source render method. Those notices that
			indicate that new pixel data is available will result in
			the map_source_data_to_display_data method being called
			to produce the final Display data.
		*/
		Source->rendering_monitor (Source_Rendering_Monitor);

		/*	Register the pixel data storage with the JP2_Reader.

			N.B.: This will automatically select the corresponding bands
			for rendering.
		*/
		#if ((DEBUG_SECTION) & DEBUG_RENDER)
		LOCKED_LOGGING ((
		clog << "    registering Source_Data_Buffers as Source image_data"
				<< endl));
		#endif
		Source->image_data (source_data_buffers (), Data_Buffer_Size);

		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    rendering all bands -" << endl
			 << "    display band 0 -> "
			 	<< *Rendering_Display_Data_Buffers[0] << endl
			 << "    display band 1 -> "
			 	<< *Rendering_Display_Data_Buffers[1] << endl
			 << "    display band 2 -> "
			 	<< *Rendering_Display_Data_Buffers[2] << endl));
		#endif

		//	Resolution and region determines rendered region.
		expected = Source->rendered_region ();

		/*	Render the source.

			The JPEG2000 codestream is fetched and decompressed into
			rendered Source pixel data. The rendering process is expected
			to be done incrementally with notifications being sent after
			each increment to the registered Rendering_Monitor.
		*/
		try {actual = Source->render ();}
		catch (JP2_Exception&)
			{
			//	Deregister the JP2_Reader::Rendering_Monitor.
			Source->rendering_monitor (NULL);
			throw;
			}
		catch (...)
			{
			Source->rendering_monitor (NULL);
			throw;
			}
		Source->rendering_monitor (NULL);
		source_data_rendered (rendered = (actual == expected));
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW | DEBUG_LOCATION))
		LOCKED_LOGGING ((
		clog << "      rendered region " << expected << ' '
				<< ((actual == expected) ? '=' : '!')
				<< "= " << expected << " expected region" << endl));
		#endif
		}
	else
	if (Rendering->Needs_Update)
		rendered = map_source ();
	}
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "<<< JP2_Image::render_source: " << boolalpha << rendered << endl));
#endif
return rendered;
}


bool
JP2_Image::map_source ()
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::map_source:" << endl
	 << "    " << *this << endl));
#endif
bool
	mapped = true;
Rendering_Monitor::Status
	status = Rendering_Monitor::REMAPPING_DATA;
int
	data_precision = source_precision_bytes (),
	display_height = height (),
	increment = Source->effective_rendering_increment_lines ();
if (increment == 0)
	increment = display_height;
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "    increment = " << increment << endl));
#endif
QRect
	display_region (0, 0, width (), increment);
while (display_region.top () < display_height)
	{
	if ((display_region.top () + display_region.height ()) >= display_height)
		display_region.setHeight (display_height - display_region.top ());

	if (data_precision == 1)
		mapped = map_source_data_to_display_data<quint8>  (display_region);
	else
		mapped = map_source_data_to_display_data<quint16> (display_region);

	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCK_LOG;
	clog << "    JP2_Image::map_source: notify_rendering_monitors"
			" status " << status << " \""
			<< Rendering_Monitor::Status_Message[status] << '"' << endl
		 << "    display_region = " << display_region << endl;
	UNLOCK_LOG;
	#endif
	if (! mapped ||
		! (mapped = notify_rendering_monitors
			(status, Rendering_Monitor::Status_Message[status],
			display_region)))
		break;

	display_region.moveTop (display_region.top () + increment);
	}

if (mapped)
	{
	status = Rendering_Monitor::DONE;
	display_region = rect ();
	}
else
	status = Rendering_Monitor::CANCELED;
notify_rendering_monitors
	(status, Rendering_Monitor::Status_Message[status],
	display_region);
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "<<< JP2_Image::map_source: " << boolalpha << mapped << endl));
#endif
return mapped;
}


template<typename Pixel_Data_Type>
bool
JP2_Image::map_source_data_to_display_data
	(
	const QRect&	region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
				DEBUG_PIXEL_MAPPING | \
				DEBUG_PIXEL_DATA | \
				DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::map_source_data_to_display_data: " << region << endl
	 << "    " << *this << endl));
#endif
Pixel_Datum
	datum;
QRgb
	pixel_value,
	*display_data,
	*image_display_data =
		image_data () + (region.top () * width ()) + region.left ();
int
	source_line = region.height ();		//	display lines, temp.
if (source_line == 0 &&
	region.top () == 0)
	source_line = height ();
int
	source_sample = region.width ();	//	display samples, temp.
if (source_sample == 0 &&
	region.left () == 0)
	source_sample = width ();
int
	display_line = region.top () - 1,
	display_sample,
	display_line_end   = region.top ()  + source_line,
	display_sample_end = region.left () + source_sample;
double
	line,
	sample;
#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
				DEBUG_PIXEL_MAPPING | \
				DEBUG_PIXEL_DATA | \
				DEBUG_LOCATION))
LOCK_LOG;
clog << "      display image size = " << size () << endl
	 << "      first display line = " << region.top () << endl
	 << "           display lines = " << region.height () << endl
	 << "        display_line_end = " << display_line_end << endl
	 << "    first display sample = " << region.left () << endl
	 << "         display samples = " << region.width () << endl
	 <<	"      display_sample_end = " << display_sample_end << endl
	 << "      image display data @ " << (void*)image_data () << endl
	 << "     region display_data @ " << (void*)image_display_data
	 	<< " at " << region.left () << "x, " << region.top () << 'y' << endl
	 << "      " << setw (2) << (sizeof (Pixel_Data_Type) << 3)
	 	<< "-bit source_data -" << endl
	 << "         background_color #"
		<< setw (sizeof (QRgb)) << setfill ('0') << hex
		<< Rendering->Background_Color << setfill (' ') << dec << endl;
	UNLOCK_LOG;
#endif

//	Pre-map the band-map sensitive values.
QTransform
	*transforms[3];
Pixel_Data_Type
	*source_data[3];
unsigned int
	source_data_width[3],
	source_data_height[3];
Data_Map
	*data_maps[3];
QRgb
	background_value[3];
int
	band = -1;
while (++band < 3)
	{
	//	Geo-transform.
	if (Differential_Transforms[band].isIdentity ())
		transforms[band] = NULL;
	else
		transforms[band] = &Differential_Transforms[band];

	//	Source data buffer.
	source_data[band] = reinterpret_cast<Pixel_Data_Type*>
		(Rendering_Display_Data_Buffers[band]->Buffer);

	//	Source data buffer size.
	source_data_width[band] =
		(Rendering_Display_Data_Buffers[band]->Rendered_Size).rwidth ();
	source_data_height[band] =
		(Rendering_Display_Data_Buffers[band]->Rendered_Size).rheight ();

	//	Source-to-Display data LUT.
	data_maps[band] = Rendering->Data_Maps[band];

	//	Background (no image) value.
	background_value[band] =
		Rendering->Background_Color & (0xFF << (16 - (band << 3)));

	#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
					DEBUG_PIXEL_MAPPING | \
					DEBUG_PIXEL_DATA | \
					DEBUG_LOCATION))
	LOCK_LOG;
	if (transforms[band])
		{
		transforms[band]->map
			(region.left(), region.top (),
			 &sample,       &line);
		//	Round down to integer coordinates.
		source_sample = round_down (sample);
		source_line   = round_down (line);
		}
	else
		{
		source_sample = region.left ();
		source_line   = region.top ();
		}
	clog << "    display band " << band
			<< " from " << *Display_Data_Buffers[band] << endl
		 << "      display "
			<< region.left() << "x, " << region.top () << 'y'
			<< " from source "
			<< source_sample << "x, " << source_line << 'y'
			<< " @ " << (void*)(source_data[band]
				+ (source_line * source_data_width[band])) << endl;
	UNLOCK_LOG;
	#endif
	}

while (++display_line < display_line_end)
	{
	display_data = image_display_data;
	display_sample = region.left () - 1;
	while (++display_sample < display_sample_end)
		{
		pixel_value = 0xFF000000;
		band = -1;
		#if ((DEBUG_SECTION) & DEBUG_PIXEL_MAPPING)
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
				{
				transforms[band]->map
					(display_sample, display_line,
					        &sample,        &line);
				//	Round down to integer coordinates.
				source_sample = round_down (sample);
				source_line   = round_down (line);
				}
			else
				{
				source_sample = display_sample;
				source_line   = display_line;
				}
			#if ((DEBUG_SECTION) & DEBUG_PIXEL_MAPPING)
			#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
			if (! display_line)
			#endif
				clog << "      " << band << "->" << Rendering->Band_Map[band]
					 << ": display "
					 << setw (3) << display_sample << "x, "
					 << setw (3) << display_line << "y <- source "
					 << setw (3) << source_sample << "x, "
					 << setw (3) << source_line << "y ";
			#endif
			if (source_sample < 0 ||
				source_sample >= (int)source_data_width[band] ||
				source_line   < 0 ||
				source_line   >= (int)source_data_height[band])
				{
				#if ((DEBUG_SECTION) & DEBUG_PIXEL_MAPPING)
				#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
				if (! display_line)
				#endif
					clog << "= BG " << endl;
				#endif
				pixel_value |= background_value[band];
				}
			else
				{
				/*
					Use the source pixel datum obtained from the
					source_data buffer at the line,sample offset as an
					index into the Data_Maps LUT which provides an 8-bit
					image pixel band datum that is shifted into the band
					location of the QRgb (unsigned int) full pixel value
					being assembled.
				*/
				datum =
					*(source_data[band]
					+ (source_line * source_data_width[band])
					+ source_sample);
				if (datum > (unsigned int)(data_maps[band]->size ()))
					{
					#if ((DEBUG_SECTION) & DEBUG_PIXEL_MAPPING)
					clog
						<< "= BG" << endl
						<< "Invalid source data mapping while rendering" << endl
						<< "at line " << source_line
							<< ", sample " << source_sample
							<< ", band " << band
							<< " with datum " << datum
							<< " of data map size " << data_maps[band]->size ();
					#endif
					pixel_value |= background_value[band];
					}
				else
					{
					#if ((DEBUG_SECTION) & DEBUG_PIXEL_MAPPING)
					#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
					if (! display_line)
						{
					#endif
						clog
							<< "@ " << setfill (' ') << setw (sizeof (void*) << 1)
							<< (void*)(source_data[band]
							   + (source_line * source_data_width[band])
							   + source_sample)
							<< hex << setfill ('0')
							<< setw (sizeof (Pixel_Data_Type) << 1)
							<< datum << " -> " << setw (2)
							<< static_cast<Pixel_Datum>(data_maps[band]->at (datum))
							<< dec << setfill (' ') << endl;
					#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
						}
					#endif
					#endif
					//	Merge the mapped pixel datum into the image pixel value.
					pixel_value |= static_cast<QRgb>(
						data_maps[band]->at (datum) << (16 - (band << 3)));
					}
				}
			}
		#if ((DEBUG_SECTION) & DEBUG_PIXEL_MAPPING)
		#if ((DEBUG_SECTION) & DEBUG_ONE_LINE)
		if (! display_line)
		#endif
			clog << "        display pixel @ "
					<< setfill (' ') << setw (sizeof (display_data) << 1)
					<< (void*)display_data
					<< " = " << hex << setw (8) << pixel_value
					<< dec << endl;
		#endif
		*display_data++ = pixel_value;
		}

	//	Move to the beginning of the next line of the region.
	image_display_data += width ();
	}
#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
				DEBUG_PIXEL_MAPPING | \
				DEBUG_PIXEL_DATA | \
				DEBUG_LOCATION))
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
LOCK_LOG;
display_image_data (*this);
UNLOCK_LOG;
#endif
#if ((DEBUG_SECTION) & DEBUG_PROMPT)
char
	input[4];
cout << "Mapped region " << region << " > " << flush;
cin.getline (input, 2);
if (input[0] == 'q')
	exit (7);
#endif
LOCKED_LOGGING ((
clog << "<<< JP2_Image::map_source_data_to_display_data: "
		<< boolalpha << (display_line == display_line_end) << endl));
#endif
return (display_line == display_line_end);
}


bool
JP2_Image::rendering_resolution_and_region
	(
	int		band
	)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << ">>> JP2_Image::rendering_resolution_and_region: " << band << endl;
#endif
int
	image_band = (band < 0) ? 0 : band;
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << "    effective band " << image_band << " from "
		<< *Rendering_Display_Data_Buffers[image_band] << endl;
#endif
bool
	rendering_needed = ! rendered_source_data (band);

//	Determine the resolution level to be used with the JP2_Reader.
QSizeF
	scaling (source_scaling (image_band));
double
	scale = qMax (scaling.rwidth (), scaling.rheight ());
unsigned int
resolution = nearest_resolution_level (scale);

Rendering_Display_Data_Buffers[image_band]->Rendered_Resolution = resolution;

//	Determine the image region to be obtained from the JP2_Reader.
QPoint
	origin (round_down (source_origin (image_band)));

Rectangle
	region (origin.x (), origin.y (),
		//	The region is relative to the full resolution grid.
		width ()  << (resolution - 1),
		height () << (resolution - 1));

if (scale < 1.0) {
	//	Get double region to allow for scale down to the next resolution level.
	region *= 2; }

//	Set the resolution and region of the JP2_Reader.
rendering_needed |=
	Source->resolution_and_region (resolution, region);
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << "             scaling = " << scaling << endl
	 << "               scale = " << scale << endl
	 << "    resolution level = " << resolution << endl
	 << "       source_origin = " << source_origin (image_band) << endl
	 << "              origin = " << origin << endl
	 << "     selected region = " << region << endl
	 << "     rendered_region = " << Source->rendered_region () << endl;
#endif

//	Size of the rendered band.
(Rendering_Display_Data_Buffers[image_band]->Rendered_Size).rwidth () =
	Source->rendered_width ();
(Rendering_Display_Data_Buffers[image_band]->Rendered_Size).rheight () =
	Source->rendered_height ();

//	Set the differential transform to be applied to the rendered data.
Differential_Transforms[image_band] = *source_transform (image_band);
#if ((DEBUG_SECTION) & DEBUG_RENDER)
clog << "    Differential_Transforms -"
	 << Differential_Transforms[image_band];
#endif
//	Set the residual, after resolution level, scaling.
scale = 1 << (resolution - 1);
Differential_Transforms[image_band].scale (scale, scale);
#if ((DEBUG_SECTION) & DEBUG_RENDER)
clog << "    at scale " << scale << " -"
	 << Differential_Transforms[image_band];
#endif
//	Invert, clipping the origin offset to its fractional part.
Differential_Transforms[image_band]
	= invert_clipping_origin (Differential_Transforms[image_band]);
#if ((DEBUG_SECTION) & DEBUG_RENDER)
clog << "    inverted and origin clipped - "
	 << Differential_Transforms[image_band]
	 << "      map 0,0 -> "
	 	<< Differential_Transforms[image_band].map (QPointF (0, 0)) << endl
	 << "      map 1,1 -> "
	 	<< Differential_Transforms[image_band].map (QPointF (1, 1)) << endl
	 << "      map " << (width () - 2) << ',' << (height () - 2) << " -> "
	 	<< Differential_Transforms[image_band].map
			(QPointF (width () - 2, height () - 2)) << endl
	 << "      map " << (width () - 1) << ',' << (height () - 1) << " -> "
	 	<< Differential_Transforms[image_band].map
			(QPointF (width () - 1, height () - 1)) << endl
	 << "      map "
	 	<< Rendering_Display_Data_Buffers[image_band]->Rendered_Size
	 	<< " -> " << Differential_Transforms[image_band].map (QPointF
			(Source->rendered_width (), Source->rendered_height ()))
			<< endl;
#endif

if (band < 0)
	{
	//	All bands rendered the same.
	if (Rendering_Display_Data_Buffers[2]->Rendered_Resolution != resolution)
		{
		rendering_needed = true;
		Rendering_Display_Data_Buffers[2]->Rendered = false;
		Rendering_Display_Data_Buffers[2]->Rendered_Resolution = resolution;
		}
	if (Rendering_Display_Data_Buffers[1]->Rendered_Resolution != resolution)
		{
		rendering_needed = true;
		Rendering_Display_Data_Buffers[1]->Rendered = false;
		Rendering_Display_Data_Buffers[1]->Rendered_Resolution = resolution;
		}

	if (Rendering_Display_Data_Buffers[2]->Rendered_Size !=
		Rendering_Display_Data_Buffers[0]->Rendered_Size)
		{
		rendering_needed = true;
		Rendering_Display_Data_Buffers[2]->Rendered = false;
		Rendering_Display_Data_Buffers[2]->Rendered_Size =
		Rendering_Display_Data_Buffers[0]->Rendered_Size;
		}
	if (Rendering_Display_Data_Buffers[1]->Rendered_Size !=
		Rendering_Display_Data_Buffers[0]->Rendered_Size)
		{
		rendering_needed = true;
		Rendering_Display_Data_Buffers[1]->Rendered = false;
		Rendering_Display_Data_Buffers[1]->Rendered_Size =
		Rendering_Display_Data_Buffers[0]->Rendered_Size;
		}

	Differential_Transforms[2] =
	Differential_Transforms[1] =
	Differential_Transforms[0];
	}

//	Set the rendering increment.
unsigned int
	height =
		(Rendering_Display_Data_Buffers[image_band]->Rendered_Size).rheight (),
	rendering_increment = rendering_increment_lines ();
if (rendering_increment &&
	//	"Close enough" margin.
	((rendering_increment >> 3) == 0 ||
	(rendering_increment + (rendering_increment >> 3)) > height))
	rendering_increment = height;
Source->rendering_increment_lines (rendering_increment);
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << "    source rendering_increment_lines = "
		<< rendering_increment << endl;
#endif

#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
clog << "<<< JP2_Image::rendering_resolution_and_region: "
		<< boolalpha << rendering_needed << endl;
#endif
return rendering_needed;
}


/*==============================================================================
	Helpers
*/
bool
JP2_Image::is_JP2_file
	(
	const QString&	pathname
	)
{return JP2_Utilities::is_JP2_file (pathname.toStdString (), 0, 0);}


unsigned int
JP2_Image::nearest_resolution_level
	(
	double	scale
	) const
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
clog << ">>> JP2_Image::nearest_resolution_level: " << scale << endl;
#endif
if (scale <= 0)
	{
	ostringstream
		message;
	message
		<< ID << endl
		<< "Can't use image scale factor " << scale << '.';
	throw invalid_argument (message.str ());
	}

unsigned int
	resolution_level = 1;
if (! Source)
	resolution_level = 0;
else
if (scale <= 0.5)
	{
	unsigned int
		resolution_levels = Source->resolution_levels (),
		scale_level = static_cast<unsigned int>(1.0 / scale);
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	clog << "    resolution_levels = " << resolution_levels << endl
		 << "          scale_level = " << scale_level << endl;
	#endif
	/*	Get the most significant bit number of the scale_level

		This bit number is the smallest resolution level corresponding
		to an image scale at least as large as the specified scale.
	*/
	while ((scale_level >>= 1) &&
			++resolution_level < resolution_levels) ;
	}
#if ((DEBUG_SECTION) & DEBUG_RENDER)
clog << "<<< JP2_Image::nearest_resolution_level: " << resolution_level << endl;
#endif
return resolution_level;
}


QRect
JP2_Image::map_source_to_display
	(
	const PIRL::Cube&	source_region
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> JP2_Image::map_source_to_display: " << source_region << endl
	 << "    " << *this << endl));
#endif
QRect
	region;
if (Source &&
	! size ().isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "    Rendering @ " << (void*)Rendering << endl
		 << "    for " << *this << endl));
	#endif
	QPointF
		origin (Rendering->origin (0));
	QSizeF
		scaling (Rendering->scaling (0));
	if (source_region.Depth != 1)
		{
		//	Find the largest display region that encompasses the source region.
		QPointF
			origins (Rendering->origin (1));
		if (origin.rx () > origins.rx ())
			origin.rx () = origins.rx ();
		if (origin.ry () > origins.ry ())
			origin.ry () = origins.ry ();
		origins = Rendering->origin (2);
		if (origin.rx () > origins.rx ())
			origin.rx () = origins.rx ();
		if (origin.ry () > origins.ry ())
			origin.ry () = origins.ry ();

		QSizeF
			scalings (Rendering->scaling (1));
		if (scaling.rwidth () < scalings.rwidth ())
			scaling.rwidth () = scalings.rwidth ();
		if (scaling.rheight () < scalings.rheight ())
			scaling.rheight () = scalings.rheight ();
		scalings = Rendering->scaling (2);
		if (scaling.rwidth () < scalings.rwidth ())
			scaling.rwidth () = scalings.rwidth ();
		if (scaling.rheight () < scalings.rheight ())
			scaling.rheight () = scalings.rheight ();
		}

	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "      display origin = " << origin << endl
		 << "             scaling = " << scaling << endl));
	#endif
	QRectF
		rectangle
		(
		/*
			The origin of the region in the display is the image region
			offset from the image origin and scaled by the image scaling
			factors.
		*/
		(source_region.X - origin.rx ()) * scaling.rwidth (),
		(source_region.Y - origin.ry ()) * scaling.rheight (),
		/*
			The size of the region in the display is the size of the
			image region scaled by the image scaling factors.
		*/
		source_region.Width  * scaling.rwidth (),
		source_region.Height * scaling.rheight ()
		);

	//	The display region contains the floating point display rectangle.
	region = rectangle.toAlignedRect ();
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	LOCKED_LOGGING ((
		clog << "    display region = " << region
			<< " (" << rectangle << ')' << endl));
	#endif

	//	Clip to the display image boundaries.
	if (region.left () < 0)
		region.setLeft (0);
	else
	if (region.left () > width ())
		region.setLeft (width () - 1);

	if (region.top () < 0)
		region.setTop (0);
	else
	if (region.top () > height ())
		region.setTop (height () - 1);

	if ((region.left () + region.width ()) > width ())
		 region.setWidth (width () - region.left ());
	if ((region.top () + region.height ()) > height ())
		 region.setHeight (height () - region.top ());
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
	if (region.height () == 0)
		{
		LOCKED_LOGGING ((
		clog << "!!! zero height region" << endl));
		}
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "<<< JP2_Image::map_source_to_display: "
		<< region << endl));
#endif
return region;
}


bool
JP2_Image::is_rendering
	(
	bool	rendering
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << ">>> JP2_Image::is_rendering: " << boolalpha << rendering << endl
	 << "    " << *this << endl));
#endif
/*	Lock the object while rendering data is being copied.

	<b>N.B.</b>: The Object_Lock is expected to be a recursive mutex
	so the base class is_rendering method, when called, can also lock
	the mutex.
*/
Object_Lock.lock ();
int
	band = -1;
bool
	was_rendering = Plastic_Image::is_rendering (rendering);
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    was rendering - " << was_rendering << endl));
#endif
if (was_rendering)
	{
	if (! rendering)
		{
		//	Reset Rendering if Source_Data state unchanged.
		while (++band < 3)
			{
			if (Data_Buffers[band])
				{
				#if ((DEBUG_SECTION) & DEBUG_RENDER)
				LOCKED_LOGGING ((
				clog << "    " << band << " previous "
						<< *Data_Buffers[band] << endl
					 << "       current "
			 			<< *Rendering_Data_Buffers[band] << endl));
				#endif
				*Data_Buffers[band] |= *Rendering_Data_Buffers[band];
				#if ((DEBUG_SECTION) & DEBUG_RENDER)
				LOCKED_LOGGING ((
				clog << "       updated "
			 			<< *Data_Buffers[band] << endl));
				#endif
				}
			else
				break;
			}
		}
	}
else
if (rendering)
	{
	//	Copy the Source_Data states and display-from-source associations.
	int
		index;
	while (++band < 3)
		{
		if (! Data_Buffers[band])
			//	No more data buffers.
			break;

		*Rendering_Data_Buffers[band] = *Data_Buffers[band];
		index = -1;
		while (++index < 3)
			if (Display_Data_Buffers[index] == Data_Buffers[band])
				Rendering_Display_Data_Buffers[index]
					= Rendering_Data_Buffers[band];
		}
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCK_LOG;
	clog << "    Rendering_Display_Data_Buffers -" << endl;
	band = -1;
	while (++band < 3)
		clog << "    display band " << band << " from "
				<< * Rendering_Display_Data_Buffers[band] << endl;
	UNLOCK_LOG;
	#endif
	}
Object_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "<<< JP2_Image::is_rendering: " << was_rendering << endl));
#endif
return was_rendering;
}


}	//	namespace UA::HiRISE
