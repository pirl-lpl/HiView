/*	Tiled_Image_Display

HiROC CVS ID: $Id: Tiled_Image_Display.cc,v 1.160 2014/05/23 00:49:35 guym Exp $

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

#include	"Tiled_Image_Display.hh"

#if (SYNCHRONOUS_RENDERING +0) == 1
#warning
#warning	>>> Synchronous image rendering will be used.
#warning
#include	"Image_Renderer.hh"
#else
#include	"Image_Renderer_Thread.hh"
#endif

#include	"Image_Tile.hh"
#include	"Plastic_Image.hh"
#include	"Plastic_QImage.hh"
#include	"HiView_Utilities.hh"

#include	<QPainter>
#include	<QPaintEvent>
#include	<QMouseEvent>
#include	<QPoint>
#include	<QPointF>
#include	<QErrorMessage>
#include	<QApplication>
#include        <QDebug>

#include	<algorithm>
using std::min;
using std::max;
#include	<sstream>
using std::ostringstream;
#include	<stdexcept>
using std::exception;
using std::invalid_argument;
#include	<iomanip>
using std::endl;


#if defined (DEBUG_SECTION)
/*******************************************************************************
	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_LOAD_IMAGE		(1 << 1)
#define DEBUG_SLOTS				(1 << 3)
#define DEBUG_SIGNALS			(1 << 4)
#define DEBUG_EVENTS			(1 << 5)
#define DEBUG_MOUSE_MOVE_EVENTS	(1 << 6)
#define	DEBUG_SAVE				(1 << 7)
#define	DEBUG_MAP_POINTS		(1 << 8)
#define	DEBUG_PIXEL_DATUM		(1 << 9)
#define DEBUG_ORIGIN			(1 << 10)
#define DEBUG_MOVE				(1 << 11)
#define DEBUG_SCALING			(1 << 12)
#define DEBUG_IMAGE_GEOMETRY	(DEBUG_ORIGIN | DEBUG_SCALING)
#define DEBUG_PAINT				(1 << 13)
#define DEBUG_PAINT_ONE			(1 << 14)
#define DEBUG_PIXEL_DATA		(1 << 15)
#define DEBUG_TILE_GRID			(1 << 16)
#define DEBUG_RESET_TILES		(1 << 17)
#define	DEBUG_REGION			(1 << 18)
#define DEBUG_MAP_BANDS			(1 << 19)
#define DEBUG_MAP_DATA			(1 << 20)
#define DEBUG_ACCESSORS			(1 << 21)
#define DEBUG_HISTOGRAMS		(1 << 22)
#define DEBUG_STATE				(1 << 23)
#define DEBUG_PROMPT			(1 << 24)
#define DEBUG_OVERVIEW			(1 << 25)
#define DEBUG_LOCATION			(1 << 26)
#define DEBUG_RENDERED			(1 << 27)
#define DEBUG_METADATA			(1 << 28)

#define DEBUG_TILE_MARKINGS		(1 << 30)
#define TILE_MARKINGS_RESET_Y			0
#define TILE_MARKINGS_RESET_COLOR		Qt::red
#define TILE_MARKINGS_PAINT_Y			60
#define TILE_MARKINGS_PAINT_COLOR		Qt::blue
#define TILE_MARKINGS_BACKGROUND_COLOR	Qt::cyan

#define DEBUG_DEFAULT	(DEBUG_ALL & \
						~DEBUG_PIXEL_DATUM & \
						~DEBUG_MOUSE_MOVE_EVENTS & \
						~DEBUG_MAP_POINTS & \
						~DEBUG_PIXEL_DATA & \
						~DEBUG_PROMPT & \
						~DEBUG_SIGNALS & \
						~DEBUG_ACCESSORS)

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#else
#include	<QCoreApplication>

#include	<iostream>
#include	<bitset>
#include	<limits>
using std::cin;
using std::clog;
using std::boolalpha;
using std::hex;
using std::dec;
using std::setfill;
using std::setw;
#endif

#endif	//	DEBUG_SECTION


namespace UA::HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Tiled_Image_Display::ID =
		"UA::HiRISE::Tiled_Image_Display ($Revision: 1.160 $ $Date: 2014/05/23 00:49:35 $)";


#ifndef	MINIMUM_IMAGE_TILE_DIMENSION
#define MINIMUM_IMAGE_TILE_DIMENSION	256
#endif
const int
	Tiled_Image_Display::MINIMUM_TILE_DIMENSION	=
		MINIMUM_IMAGE_TILE_DIMENSION;

/*------------------------------------------------------------------------------
	Defaults
*/
#ifndef DEFAULT_IMAGE_TILE_WIDTH
#define	DEFAULT_IMAGE_TILE_WIDTH		(MINIMUM_IMAGE_TILE_DIMENSION << 2)
#endif
#if (DEFAULT_IMAGE_TILE_WIDTH) < MINIMUM_IMAGE_TILE_DIMENSION
#error "DEFAULT_IMAGE_TILE_WIDTH must be at least 256."
#endif
#ifndef DEFAULT_IMAGE_TILE_HEIGHT
#define	DEFAULT_IMAGE_TILE_HEIGHT		(MINIMUM_IMAGE_TILE_DIMENSION << 2)
#endif
#if (DEFAULT_IMAGE_TILE_HEIGHT) < MINIMUM_IMAGE_TILE_DIMENSION
#error "DEFAULT_IMAGE_TILE_HEIGHT must be at least 256."
#endif
#if ((DEBUG_SECTION) && (((DEBUG_SECTION) & DEBUG_OVERVIEW) == 0))
	#undef DEFAULT_IMAGE_TILE_WIDTH
	#undef DEFAULT_IMAGE_TILE_HEIGHT
	#define	DEFAULT_IMAGE_TILE_WIDTH	512
	#define	DEFAULT_IMAGE_TILE_HEIGHT	512
#endif
QSize
	Tiled_Image_Display::Default_Tile_Display_Size
		(DEFAULT_IMAGE_TILE_WIDTH, DEFAULT_IMAGE_TILE_HEIGHT);

#ifndef IMAGE_VIEWER_MIN_SCALE
#define IMAGE_VIEWER_MIN_SCALE			0.01
#endif
double
	Tiled_Image_Display::Min_Scale		= IMAGE_VIEWER_MIN_SCALE;
#ifndef IMAGE_VIEWER_MAX_SCALE
#define IMAGE_VIEWER_MAX_SCALE			10.0
#endif
double
	Tiled_Image_Display::Max_Scale		= IMAGE_VIEWER_MAX_SCALE;

#ifndef DEFAULT_SOURCE_IMAGE_RENDERING
#define DEFAULT_SOURCE_IMAGE_RENDERING	true
#endif
bool
	Tiled_Image_Display::Default_Source_Image_Rendering =
		DEFAULT_SOURCE_IMAGE_RENDERING;

/*------------------------------------------------------------------------------
	Local
*/
#ifndef DOXYGEN_PROCESSING
namespace
{
enum
	{
	BACKGROUND_TILES_RESET	= -1,
	NO_TILES_RESET			= 0,
	VISIBLE_TILES_RESET		= 1
	};
}
#endif

/*==============================================================================
	Class data members
*/
QErrorMessage
	*Tiled_Image_Display::Error_Message	= NULL;

/*==============================================================================
	Constructors
*/
Tiled_Image_Display::Tiled_Image_Display
	(
	QWidget*				parent
	)
	:	QWidget (parent),
		Source_Image (NULL),
		Source_Image_Rendering (Default_Source_Image_Rendering),
		Reference_Image (NULL),
		Image_Loading (false),
		Pending_State_Change (NO_STATE_CHANGE),
		Pending_State_Change_Enabled (false), // VALGRIND
		Tile_Grid_Images (new QList<QList<Plastic_Image*>*>),
		Tile_Image_Pool_Max (0),
		Tile_Grid_Size (0, 0),
		Tile_Display_Size (Default_Tile_Display_Size),
		Tile_Image_Size (Tile_Display_Size)
{
setObjectName ("Tiled_Image_Display");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Tiled_Image_Display @ " << (void*)this
		<< ": " << object_pathname (this) << endl;
#endif
setAttribute (Qt::WA_OpaquePaintEvent, true);
setAttribute (Qt::WA_NoSystemBackground, true);

//	Disable auto-update.
Plastic_Image::default_auto_update (false);

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "    new Image_Renderer ..." << endl));
#endif
//	Renderer.
#if (SYNCHRONOUS_RENDERING +0) == 1
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
LOCKED_LOGGING ((
clog << "    >>> SYNCHRONOUS_RENDERING <<<" << endl));
#endif
Renderer = new Image_Renderer;
#else
Renderer = new Image_Renderer_Thread;
#endif
//	Only update when all displayable tiles have been rendered.
Renderer->immediate_mode (false);
//	Hold rendering in abeyance until started.
Renderer->reset ();
//	Connect to the image loaded signal.
connect (Renderer,
			SIGNAL (image_loaded (bool)),
			SLOT (loaded (bool)));
//	Connect to the rendered tile signal.
connect (Renderer,
			SIGNAL (rendered (const QPoint&, const QRect&)),
			SLOT (rendered (const QPoint&, const QRect&)));
//	Filtered Renderer status signal.
connect (Renderer,
			SIGNAL (status (int)),
			SLOT (renderer_status (int)));
//	Propogate the Renderer status notice signal.
connect (Renderer,
			SIGNAL (status_notice (const QString&)),
			SIGNAL (rendering_status_notice (const QString&)));
//	Connect to the rendering error signal.
connect (Renderer,
			SIGNAL (error (const QString&)),
			SLOT (rendering_error (const QString&)));

//	Enable mouseMoveEvent tracking.
setMouseTracking (true);

//	Provide an empty image display.
loaded (true);
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display: "<< object_pathname (this) << endl));
#endif
}


Tiled_Image_Display::~Tiled_Image_Display ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << ">>> ~Tiled_Image_Display: @ " << (void*)this << endl));
#endif
//	Stop all rendering.
Renderer->finish
	(Image_Renderer::FORCE_CANCEL |
	 Image_Renderer::WAIT_UNTIL_DONE);
//	Delete all of the tiles.
clear_tiles ();

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "    delete Tile_Grid_Images @ " << (void*)Tile_Grid_Images << endl));
#endif
delete Tile_Grid_Images;

#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "    delete Renderer @ " << (void*)Renderer << endl));
#endif
delete Renderer;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
LOCKED_LOGGING ((
clog << "<<< ~Tiled_Image_Display" << endl));
#endif
}

/*==============================================================================
	Image
*/
bool
Tiled_Image_Display::image
	(
	const QString&	source_name,
	const QSizeF&	scaling
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::image: \"" << source_name << '"' << endl
	 << "    scaling = " << scaling << endl
	 << "    in " << pathname << endl));
#endif
Initial_Scaling = scaling;
bool
	registered = false;
if (! source_name.isEmpty ())
	{
	Image_Loading = true;

	//	Cancel and clear any rendering in progress and stop rendering.
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Tiled_Image_Display::image: force rendering reset" << endl));
	#endif
	Renderer->reset
		(Image_Renderer::DO_NOT_WAIT |
		 Image_Renderer::FORCE_CANCEL);

	//	Register the source with the Renderer for loading.
	if ((registered = Renderer->image (source_name)))
		{
		#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
		LOCKED_LOGGING ((
		clog << "    Tiled_Image_Display::image: state_change_start" << endl));
		#endif
		Pending_State_Change = NO_STATE_CHANGE;	//	Reset all state changes;
		state_change_start (IMAGE_LOAD_STATE);
		}
	else
		Image_Loading = false;
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    " << pathname << endl
	 << "<<< Tiled_Image_Display::image: "
		<< boolalpha << registered << endl));
#endif
return registered;
}


bool
Tiled_Image_Display::image
	(
	const QString&	source_name,
	const QSize&	display_size
	)
{
//	Flag as display size values.
return image (source_name, QSizeF (display_size) *= -1);
}


bool
Tiled_Image_Display::image
	(
	const Shared_Image&	source_image,
	const QSizeF&		scaling
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << ">>> Tiled_Image_Display::image: Shared_Image ";
if (source_image)
	clog << *source_image << endl;
else
	clog << "NULL" << endl;
clog << "    scaling = " << scaling << endl
	 << "    in " << pathname << endl;
UNLOCK_LOG;
#endif
Image_Loading = true;

//	Cancel and clear any rendering in progress and stop rendering.
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    Tiled_Image_Display::image: force rendering reset" << endl));
#endif
Renderer->reset
		(Image_Renderer::DO_NOT_WAIT |
		 Image_Renderer::FORCE_CANCEL);
Initial_Scaling = scaling;

bool
	registered = false;
if (! source_image)
	{
	//	Provide an empty image.
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Providing an empty image" << endl));
	#endif
	registered = Renderer->image (Shared_Image (new Plastic_QImage ()));
	}
else
	registered = Renderer->image (source_image);
if (registered)
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Tiled_Image_Display::image: state_change_start" << endl));
	#endif
	Pending_State_Change = NO_STATE_CHANGE;	//	Reset all state changes;
	state_change_start (IMAGE_LOAD_STATE);
	}
else
	Image_Loading = false;

#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Tiled_Image_Display::image: "
		<< boolalpha << registered << endl));
#endif
return registered;
}


bool
Tiled_Image_Display::image
	(
	const Shared_Image&	source_image,
	const QSize&		display_size
	)
{
//	Flag as display size values.
return image (source_image, QSizeF (display_size) *= -1);
}


void
Tiled_Image_Display::loaded
	(
	bool	successful
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::loaded: " << boolalpha << successful << endl
	 << "    in " << pathname << endl));
#endif
int
	state = NO_STATE_CHANGE;

if (successful)
	{
	Pending_State_Change &= ~IMAGE_LOAD_STATE;
	if (Image_Loading)
		Pending_State_Change_Enabled = false;

	//	Stop any rendering in progress (should have been done at load request).
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE))
	LOCKED_LOGGING ((
	clog << "    Tiled_Image_Display::loaded: stop_rendering" << endl));
	#endif
	Renderer->stop_rendering ();

	/*	Clear the tile grid.

		>>> WARNING <<< The tile images must be deleted before the source image.
	*/
	clear_tiles ();

	//	Switch over to the new Source and Reference images.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
	LOCK_LOG;
	clog << "    switching Source_Image from -" << endl;
	if (Source_Image)
		clog << "    " << *Source_Image << endl;
	else
		clog
		 << "    NULL" << endl;
	UNLOCK_LOG;
	#endif

	Source_Image = Renderer->source_image ();
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
	LOCK_LOG;
	clog << "    switched Source_Image to -" << endl;
	if (Source_Image)
		clog << "    " << *Source_Image << endl;
	else
		clog
		 << "!!! NULL" << endl;
	clog << "    switching Reference_Image from -" << endl;
	if (Reference_Image)
		clog << "    " << *Reference_Image << endl;
	else
		clog
		 << "    NULL" << endl;
	UNLOCK_LOG;
	#endif

	//	Register for metadata change notifications.
	Source_Image->add_metadata_monitor (this);

	Renderer->delete_image (Reference_Image);
	Reference_Image = Renderer->reference_image ();
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
	LOCK_LOG;
	clog << "    switched Reference_Image to -" << endl;
	if (Reference_Image)
		clog << "    " << *Reference_Image << endl;
	else
		clog
		 << "!!! NULL" << endl;
	UNLOCK_LOG;
	#endif

	//	Renderer cleanup (dispose of the old Reference_Image).
	Renderer->clean_up ();

	if (Image_Loading)
		//	Initialize the new image state.
		state |=
			IMAGE_MOVE_STATE   |
			BAND_MAPPING_STATE |
			DATA_MAPPING_STATE;

	//	Reset the tile grid origin.
	displayed_tile_grid_origin (QPointF ());

	//	Set the initial image scaling in the reference image.
	if (Initial_Scaling.isEmpty ())
		{
		//	Determine the scaling from the image/display size ratios.
		QSize
			//	Assume the display size was specified.
			display_size (Initial_Scaling.toSize () *= -1);
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE))
		LOCKED_LOGGING ((
		clog << "    preferred display size = " << display_size << endl));
		#endif
		if (display_size.rwidth ()  == 0 ||
			display_size.rheight () == 0)
			{
			//	Use the size of the widget.
			display_size = size ();
			if (display_size.isEmpty ())
				//	Use the display size hint.
				display_size = sizeHint ();
			}
		Initial_Scaling.rheight () =
		Initial_Scaling.rwidth () =
			scale_to_size (image_size (), display_size);
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE))
		LOCKED_LOGGING ((
		clog << "    tentative scaling = "
				<< Initial_Scaling.rheight () << endl));
		#endif
		}
	if (Initial_Scaling.rwidth ()  < Min_Scale)
		Initial_Scaling.rwidth ()  = Min_Scale;
	else
	if (Initial_Scaling.rwidth ()  > Max_Scale)
		Initial_Scaling.rwidth ()  = Max_Scale;
	if (Initial_Scaling.rheight () < Min_Scale)
		Initial_Scaling.rheight () = Min_Scale;
	else
	if (Initial_Scaling.rheight () > Max_Scale)
		Initial_Scaling.rheight () = Max_Scale;

	//	Set the initial scaling in the Reference_Image.
	if (Reference_Image->source_scaling (Initial_Scaling) &&
		Image_Loading)
		state |= IMAGE_SCALE_STATE;

	QSizeF
		displayed_size (Initial_Scaling);
	displayed_size.rwidth ()  *= image_width ();
	displayed_size.rheight () *= image_height ();
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "    Initial_Scaling = " << Initial_Scaling << endl
		 << "     displayed size = " << displayed_size << endl));
	#endif

	/*	>>> CAUTION <<< The order of operations is important.

		Determining the values of data members concerned with tile
		management may depend on the values of other data members. So it
		is important to avoid a dependency loop.

		The Reference image origin (origin on the display viewport in
		image space) and scaling are first reset to 0,0 and the
		Initial_Scaling, respectively.

		* Tile_Display_Size - Size of a tile in display space.

			Depends on the source image size. Must not be zero (divisor).

		* Tile_Image_Size - Size of a tile in image space.

			Reset by tile_image_size where it depends on the
			Tile_Display_Size and the scaling of the reference image.

		* Tile_Grid_Size - Size of the tile grid in tile units.

			Initialized to zero by clear_tiles, then reset by
			resize_tile_grid where it depends on the current display
			viewport size and the Tile_Display_Size.

		* Tiled_Image_Region - Tiled region in image space.

			Initialized to zero by clear_tiles, then reset by
			resize_tile_grid where its size depends on the current
			display viewport size, Tile_Display_Size, and
			Tile_Image_Size; its origin depends on the Tile_Image_Size.

		* Tile_Display_Offset - Offset of the display viewport origin in
			the first display tile.

			Reset to zero.

		* Displayed_Image_Region - The region of the image, in image space,
			within the display viewport.

			Reset by displayed_image_region where it depends on mapping
			to image space the display viewport extent (depends on the
			current display viewport size) - using map_display_to_image
			which depends on Tile_Display_Offset, scaling of the
			reference image and the origin of the display viewport in
			image space (initialized to 0,0) - which is clipped to the
			image size.

		* Lower_Right_Origin_Limit - The lower-right limit, in image space,
			of the image display origin.

			Reset by calculate_lower_right_origin_limit which depends on
			the origin of the display viewport in image space
			(initialized to 0,0), the scaling of the reference image and
			the current size of the display viewport.
	*/
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE))
	LOCKED_LOGGING ((
	clog << "    reset the Tiled_Image_Display data members" << endl));
	#endif
	//	Set the tile size.
	Tile_Display_Size = image_size ();	//	Single tile display.
	if (! dynamic_cast<Plastic_QImage*>(Reference_Image) &&
		(Tile_Display_Size.rwidth ()  > Default_Tile_Display_Size.rwidth () ||
		 Tile_Display_Size.rheight () > Default_Tile_Display_Size.rheight ()))
		 Tile_Display_Size = Default_Tile_Display_Size;	//	Multi-tile display.
	reset_tile_image_size ();

	//	Reset the origin, in image space, of the entire tile grid region.
	Tiled_Image_Region.setX (-Tile_Image_Size.width ());
	Tiled_Image_Region.setY (-Tile_Image_Size.height ());

	//	Reset the tile display offset.
	Tile_Display_Offset.rx () =
	Tile_Display_Offset.ry () = 0;

	//	Reset the displayed image region.
	reset_displayed_image_region ();

	if (Image_Loading)
		{
		Image_Loading = false;

		//	Assemble the tile grid.
		resize_tile_grid ();

		//	Initialize the tile images.
		if (! reset_tiles ())
			state = NO_STATE_CHANGE;

		if (Source_Image_Rendering)
			//	Queue the Source_Image for uncancelable background rendering.
			Renderer->queue (Source_Image,
				Image_Renderer::LOW_PRIORITY_RENDERING,
				! Image_Renderer::CANCELABLE);

		Image_Loading = true;
		}

	//	Reset the lower-right image display origin limit.
	Lower_Right_Origin_Limit = calculate_lower_right_origin_limit ();

	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE))
	QPointF
		tile_grid_origin (displayed_tile_grid_origin ());
	LOCKED_LOGGING ((
	clog << "    Tiled_Image_Display::loaded" << endl
		 << "    in " << pathname << endl
		 << "             Tile_Display_Size = " << Tile_Display_Size << endl
		 << "               Tile_Image_Size = " << Tile_Image_Size << endl
		 << "    displayed_tile_grid_origin = " << tile_grid_origin << endl
		 << "           Tile_Display_Offset = " << Tile_Display_Offset << endl
		 << "                Tile_Grid_Size = " << Tile_Grid_Size << endl
		 << "            Tiled_Image_Region = " << Tiled_Image_Region << endl
		 << "        Displayed_Image_Region = "
			<< Displayed_Image_Region << endl
		 << "      Lower_Right_Origin_Limit = "
			<< Lower_Right_Origin_Limit << endl));
	#endif
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCK_LOG;
	print_tile_grid ();
	print_tile_pool ();
	UNLOCK_LOG;
	Renderer->print_render_queue ();
	Renderer->print_delete_queue ();
	LOCK_LOG;
	Image_Tile::tile_accounting ();
	Renderer->image_accounting ();
	UNLOCK_LOG;
	#endif
	}

if (Image_Loading)
	{
	Image_Loading = false;

	/*	>>> WARNING <<< The order of image load completion signaling
		is important: The image_loaded signal must occur before the
		state change signal.
	*/
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::loaded: "
			"emit image_loaded " << boolalpha << successful << endl));
	#endif
	emit image_loaded (successful);

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_STATE | \
					DEBUG_LOAD_IMAGE | \
					DEBUG_SLOTS | \
					DEBUG_SIGNALS))
	int
		value = IMAGE_LOAD_STATE |
			(successful ?
		 	RENDERING_COMPLETED_STATE :
			RENDERING_CANCELED_STATE);
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::loaded: "
			"emit state_change: " << value << " - "
			<< state_change_description (value) << endl
		 << "    in " << object_pathname (this) << endl));
	#endif
	emit state_change
		(IMAGE_LOAD_STATE |
			(successful ?
		 	RENDERING_COMPLETED_STATE :
			RENDERING_CANCELED_STATE));
	Pending_State_Change &= ~IMAGE_LOAD_STATE;

	//	Restart rendering.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE))
	LOCKED_LOGGING ((
	clog << "    start Renderer" << endl));
	#endif
	Pending_State_Change_Enabled = true;
	state_change_start (state);

	if (successful)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_SIGNALS))
		LOCKED_LOGGING ((
		clog << "^^^ Tiled_Image_Display::loaded: "
				"emit image_scaled " << Initial_Scaling << endl));
		#endif
		emit image_scaled (Initial_Scaling, -1);

		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_SIGNALS))
		LOCKED_LOGGING ((
		clog << "^^^ Tiled_Image_Display::loaded: "
				"emit displayed_image_region_resized: "
				<< displayed_image_region_size () << endl));
		#endif
		emit displayed_image_region_resized (displayed_image_region_size ());
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Tiled_Image_Display::loaded" << endl));
#endif
}


void
Tiled_Image_Display::rendering_increment_lines
	(
	int	rendering_increment
	)
{
if (rendering_increment < 0)
	rendering_increment = 0;

if ((unsigned int)rendering_increment
		!= Plastic_Image::default_rendering_increment_lines ())
	{
	Plastic_Image::default_rendering_increment_lines
		((unsigned int)rendering_increment);

	QList<Plastic_Image*>
		*tiles;
	Plastic_Image
		*image;
	int
		tile_cols,
		tile_rows = Tile_Grid_Size.rheight ();
	while (tile_rows--)
		{
		if ((tiles = Tile_Grid_Images->at (tile_rows)))
			{
			tile_cols = Tile_Grid_Size.rwidth ();
			while (tile_cols--)
				if ((image = tiles->at (tile_cols)))
					image->rendering_increment_lines
						((unsigned int)rendering_increment);
			}
		}
	}
}


void
Tiled_Image_Display::background_color
	(
	QRgb	color
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << ">>> Tiled_Image_Display::background_color: 0x"
		<< hex << setfill ('0') << setw (8) << color << endl;
#endif
if (! color)
	{
	color = palette ().window ().color ().rgba ();
	#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
	clog << "    using palette window color = 0x" << setw (8) << color << endl;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << "    current color = 0x" << setw (8)
		<< Plastic_Image::default_background_color ()
		<< setfill (' ') << dec << endl;
#endif
if (color != Plastic_Image::default_background_color ())
	{
	Plastic_Image::default_background_color (color);

	int
		tile_rows = Tile_Grid_Size.rheight (),
		tile_cols = Tile_Grid_Size.rwidth ();
	if (! tile_rows ||
		! tile_cols)
		{
		#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
		LOCKED_LOGGING ((
		clog << "    no tiles" << endl
			 << "<<< Tiled_Image_Display::background_color" << endl));
		#endif
		return;
		}

	//	Stop all rendering.
	Pending_State_Change_Enabled = false;
	Renderer->stop_rendering (true);

	QList<Plastic_Image*>
		*tiles;
	Plastic_Image
		*image;
	QPainter
		painter;
	QRect
		paint_region;
	QSize
		displayed_size;
	while (tile_rows--)
		{
		if ((tiles = Tile_Grid_Images->at (tile_rows)))
			{
			tile_cols = Tile_Grid_Size.rwidth ();
			while (tile_cols--)
				{
				if ((image = tiles->at (tile_cols)))
					{
					image->background_color (color);

					//	Only touch-up images with background showing.
					displayed_size = image->displayed_size ();
					if (displayed_size != Tile_Display_Size)
						{
						painter.begin (image);
						if (displayed_size.rwidth ()
								< Tile_Display_Size.rwidth ())
							{
							//	Right area.
							paint_region.setRect
								(displayed_size.rwidth (), 0,
								 Tile_Display_Size.rwidth ()
							 		- displayed_size.rwidth (),
								 displayed_size.rheight ());
							painter.fillRect (paint_region, color);
							}
						if (displayed_size.rheight ()
								< Tile_Display_Size.rheight ())
							{
							//	Bottom area.
							paint_region.setRect
								(0, displayed_size.rheight (),
								 Tile_Display_Size.rwidth (),
								 Tile_Display_Size.rheight ()
							 		- displayed_size.rheight ());
							painter.fillRect (paint_region, color);
							}
						painter.end ();
						}
					else
						//	All from here to start of row are fully covered.
						break;
					}
				}
			}
		}

	update ();

	//	Restart rendering.
	Pending_State_Change_Enabled = true;
	Renderer->start_rendering ();
	}
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
clog << "<<< Tiled_Image_Display::background_color" << endl;
#endif
}


void
Tiled_Image_Display::max_source_image_area
	(
	unsigned long	area
	)
{Renderer->max_source_image_area (area);}


unsigned long
Tiled_Image_Display::max_source_image_area () const
{return Renderer->max_source_image_area ();}


unsigned long
Tiled_Image_Display::default_max_source_image_area ()
{return Image_Renderer::default_max_source_image_area ();}


void
Tiled_Image_Display::default_max_source_image_area
	(
	unsigned long	area
	)
{Image_Renderer::default_max_source_image_area (area);}

/*------------------------------------------------------------------------------
	Metadata
*/
void
Tiled_Image_Display::metadata_changed
	(
	Plastic_Image&	image
	)
{
//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_METADATA | DEBUG_SIGNALS))
LOCKED_LOGGING ((
clog << "^^^ Tiled_Image_Display::metadata_changed: "
		"emit image_metadata_changed (metadata @ "
		<< (void*)image.metadata () << ')' << endl));
#endif
emit image_metadata_changed (image.metadata ());
}

/*------------------------------------------------------------------------------
	Histograms
*/
bool
Tiled_Image_Display::source_data_histograms
	(
	QVector<Histogram*>	histograms,
	const QRect&		image_region
	) const
{
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << ">>> Tiled_Image_Display::source_data_histograms:" << endl
	 << "    image_region = " << image_region << endl;
#endif
bool
	completed = false;

if (! round_down (tiled_image_region ()).contains (image_region))
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	clog << "    tiled_image_region = "
			<< round_down (tiled_image_region ()) << endl
		 << "    does not encompass the selected image region" << endl
		 << "<<< Tiled_Image_Display::source_data_histograms: false" << endl;
	#endif
	return false;
	}

//	Initialize the histograms.
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << "    histograms @ ";
#endif
for (int
		index = 0,
			entries = qMin (histograms.size (), 3);
		index < entries;
		index++)
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	if (index)
		clog << ", ";
	#endif
	if (histograms[index])
		{
		#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
		clog << (void*)histograms[index];
		#endif
		histograms[index]->fill (0);
		}
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	else
		clog << "NULL";
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << endl;
#endif

if (image_region.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	clog << "    empty image selection region" << endl
		 << "<<< Tiled_Image_Display::source_data_histograms: true" << endl;
	#endif
	return true;
	}
unsigned long long
	area = image_region.width () * image_region.height (),
	counted,
	count = 0;
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << "    image_region area = " << area << endl;
#endif
QList<Plastic_Image*>
	*tiles;
Plastic_Image
	*tile_image;
int
	tile_row = Tile_Grid_Size.height (),
	tile_col,
	tile_cols = Tile_Grid_Size.width ();
while (tile_row--)
	{
	if ((tiles = Tile_Grid_Images->at (tile_row)))
		{
		tile_col = tile_cols;
		while (tile_col--)
			{
			if ((tile_image = tiles->at (tile_col)) &&
				image_region.intersects (tile_image->image_region ()))
				{
				counted =
					tile_image->source_histograms (histograms, image_region);
				#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
				clog << "    tile " << tile_col << ',' << tile_row << " - "
						<< tile_image->image_region () << ": "
						<< counted << " counted" << endl;
				#endif
				if (counted == 0)
					{
					#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
					clog << "    no histogram" << endl;
					#endif
					goto Done;
					}
				if ((count += counted) >= area)
					goto Completed;
				}
			}
		}
	}

Completed:
completed = true;

Done:
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << "<<< Tiled_Image_Display::source_data_histograms: "
		<< boolalpha << completed << endl;
#endif
return completed;
}


bool
Tiled_Image_Display::display_data_histograms
	(
	QVector<Histogram*>	histograms,
	const QRect&		display_region
	) const
{
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << ">>> Tiled_Image_Display::display_data_histograms: "
		<< display_region << endl
	 << "    histograms @ ";
#endif
if (! tiled_image_region ().contains (map_display_to_image (display_region)))
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	clog << "    tiled_image_region = "
			<< round_down (tiled_image_region ()) << endl
		 << "    does not encompass the selected image region" << endl
		 << "<<< Tiled_Image_Display::display_data_histograms: false" << endl;
	#endif
	return false;
	}

//	Initialize the histograms.
for (int
		index = 0,
			entries = qMin (histograms.size (), 3);
		index < entries;
		index++)
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	if (index)
		clog << ", ";
	#endif
	if (histograms[index])
		{
		#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
		clog << (void*)histograms[index];
		#endif
		histograms[index]->fill (0);
		}
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	else
		clog << "NULL";
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << endl;
#endif

if (display_region.isEmpty () ||
	Tile_Display_Size.isEmpty () ||
	Tile_Grid_Size.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	clog << "    empty " << (display_region.isEmpty () ?
			"display region" : "tile grid") << endl
		 << "<<< Tiled_Image_Display::display_data_histograms" << endl;
	#endif
	return true;
	}

QPoint
	tile_grid (map_display_to_tile (display_region.topLeft ())),
	tile_grid_limit (map_display_to_tile (QPoint
		(display_region.left () + display_region.width (),
		 display_region.top ()  + display_region.height ())));
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << "    selected tile grid = "
		<< tile_grid << " to " << tile_grid_limit << endl;
#endif
if (tile_grid.rx () < 0 ||
	tile_grid_limit.rx () < 0)
	{
	#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
	clog << "!!! selected region outside the tile grid!?!" << endl
		 << "<<< Tiled_Image_Display::display_data_histograms" << endl;
	#endif
	return false;
	}

QRect
	tile_region (tile_display_region (tile_grid));
int
	tile_col_start = tile_grid.rx (),
	x_origin = tile_region.x ();
unsigned long long
	total = display_region.width () * display_region.height (),
	count = 0;
QList<Plastic_Image*>
	*tiles;
Plastic_Image
	*tile_image;

while (true)
	{
	if ((tiles = Tile_Grid_Images->at (tile_grid.ry ())))
		{
		tile_grid.rx () = tile_col_start;
		while (true)
			{
			#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
			clog << "      " << tile_grid
					<< " tile display region = " << tile_region
					<< ", selected = " << (tile_region & display_region)
					<< endl;
			#endif
			if ((tile_image = tiles->at (tile_grid.rx ())))
				{
				count += tile_image->display_histograms (histograms,
					tile_region & display_region);
				if (count >= total)
					goto Done;
				}
			if (++tile_grid.rx () > tile_grid_limit.rx ())
				break;
			tile_region.translate (Tile_Display_Size.width (), 0);
			}
		}
	if (++tile_grid.ry () > tile_grid_limit.ry ())
		break;
	tile_region.moveTo
		(x_origin, tile_region.y () + Tile_Display_Size.height ());
	}
Done:
#if ((DEBUG_SECTION) & DEBUG_HISTOGRAMS)
clog << "<<< Tiled_Image_Display::display_data_histograms: true" << endl;
#endif
return true;
}

/*==============================================================================
	Accesors
*/
Plastic_Image::Pixel_Datum
Tiled_Image_Display::image_pixel_datum
	(
	unsigned int	x,
	unsigned int	y,
	unsigned int	band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::image_pixel_datum: "
		<< x << "x, " << y << "y, " << band << 'b' << endl
	 << "    Tile_Grid_Size = " << Tile_Grid_Size << endl));
#endif
QPoint
	point (map_image_to_tile (QPoint (x, y), band));
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
LOCKED_LOGGING ((
clog << "      tile grid = " << point << endl));
#endif
if (point.rx () >= 0 &&
	point.rx () < Tile_Grid_Size.width () &&
	point.ry () >= 0 &&
	point.ry () < Tile_Grid_Size.height ())
	{
	QList<Plastic_Image*>
		*tiles = Tile_Grid_Images->at (point.ry ());
	if (tiles)
		{
		Plastic_Image
			*tile_image = tiles->at (point.rx ());
		if (tile_image &&
			! tile_image->needs_update ())
			{
			#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
			LOCK_LOG;
			clog << "    get the source_pixel_value" << endl;
			Plastic_Image::Pixel_Datum
				datum = tile_image->source_pixel_value (x, y, band);
			clog << "<<< Tiled_Image_Display::image_pixel_datum: "
					<< datum << endl;
			UNLOCK_LOG;
			return datum;
			#else
			return tile_image->source_pixel_value (x, y, band);
			#endif
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::image_pixel_datum: "
		<< Plastic_Image::UNDEFINED_PIXEL_VALUE << endl));
#endif
return Plastic_Image::UNDEFINED_PIXEL_VALUE;
}


Plastic_Image::Triplet
Tiled_Image_Display::image_pixel
	(
	const QPoint&	coordinate
	) const
{
unsigned int
	x = static_cast<unsigned int>(coordinate.x ()),
	y = static_cast<unsigned int>(coordinate.y ());
return Plastic_Image::Triplet
	(image_pixel_datum (x, y, 0),
	 image_pixel_datum (x, y, 1),
	 image_pixel_datum (x, y, 2));
}


QRgb
Tiled_Image_Display::display_value
	(
	const QPoint&	coordinate
	) const
{
if (coordinate.x () >= 0 &&
	coordinate.y () >= 0 &&
	coordinate.x () < width () &&
	coordinate.y () < height ())
	{
	QPoint
		point (map_display_to_tile (coordinate));
	if (point.rx () >= 0 &&
		point.rx () < Tile_Grid_Size.width () &&
		point.ry () >= 0 &&
		point.ry () < Tile_Grid_Size.height ())
		{
		QList<Plastic_Image*>
			*tiles = Tile_Grid_Images->at (point.ry ());
		if (tiles)
			{
			Plastic_Image
				*tile_image = tiles->at (point.rx ());
			if (tile_image &&
				! tile_image->needs_update ())
				return tile_image->pixel (map_display_to_tile_offset (coordinate));
			}
		}
	}
/*
	N.B.: Returning zero flags an invalid value only on the assumption
	that the QImage pixel data format always sets the MSB (alpha band)
	value to FF.
*/
return 0;
}


Plastic_Image::Triplet
Tiled_Image_Display::display_pixel
	(
	const QPoint&	coordinate
	) const
{
QRgb
	value = display_value (coordinate);
#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATUM)
LOCKED_LOGGING ((
clog << ">-< Tiled_Image_Display::display_pixel: "
		<< coordinate << " = " << hex << value << dec << endl));
#endif
if (value)
	return Plastic_Image::Triplet (value);
return Plastic_Image::Triplet ();
}


QPointF
Tiled_Image_Display::displayed_image_origin
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::displayed_image_origin: band "
		<< band << endl));
#endif
if (band <= 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "<<< Tiled_Image_Display::displayed_image_origin: "
			<< Displayed_Image_Region.topLeft () << endl));
	#endif
	return Displayed_Image_Region.topLeft ();
	}
QPointF
	point (Displayed_Image_Region.topLeft ());
point += displayed_tile_grid_origin (band);
point -= displayed_tile_grid_origin (0);
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::displayed_image_origin: " << point << endl));
#endif
return point;
}


QPoint
Tiled_Image_Display::calculate_lower_right_origin_limit () const
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::calculate_lower_right_origin_limit" << endl));
#endif
QPointF
	limit (image_width () - 1, image_height () - 1);
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "       image LR limit = " << limit << endl));
#endif

//	Maximum positive origin offset.
QPointF
	offset  (displayed_tile_grid_origin (1) - displayed_tile_grid_origin (0)),
	offsets (displayed_tile_grid_origin (2) - displayed_tile_grid_origin (0));
if (offset.rx () < offsets.rx ())
	offset.rx () = offsets.rx ();
if (offset.ry () < offsets.ry ())
	offset.ry () = offsets.ry ();
if (offset.rx () < 0)
	offset.rx () = 0;
if (offset.ry () < 0)
	offset.ry () = 0;
limit += offset;
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "     max image offset = " << offset << endl
	 << "    adjusted LR limit = " << limit << endl));
#endif

//	Maximum image scaling.
double
	scale_width,
	scaling_width,
	scale_height,
	scaling_height;
image_scaling (&scale_width,   &scale_height,   0);
image_scaling (&scaling_width, &scaling_height, 1);
if (scale_width < scaling_width)
	scale_width = scaling_width;
if (scale_height < scaling_height)
	scale_height = scaling_height;
image_scaling (&scaling_width, &scaling_height, 2);
if (scale_width < scaling_width)
	scale_width = scaling_width;
if (scale_height < scaling_height)
	scale_height = scaling_height;

//	Offset back by viewport size in scaled image space.
limit.rx () -= width ()  / scale_width;
limit.ry () -= height () / scale_height;
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "    max image scaling = "
		<< scale_width << "w, " << scale_height << 'h' << endl
	 << "        viewport size = " << size () << endl
	 << "          scaled size = "
	 	<< (width ()  / scale_width) << "w, "
		<< (height () / scale_height) << 'h' << endl));
#endif
if (limit.rx () < 0)
	limit.rx () = 0;
if (limit.ry () < 0)
	limit.ry () = 0;
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::calculate_lower_right_origin_limit: "
		<< limit << endl));
#endif
return round_down (limit);
}


QRectF
Tiled_Image_Display::displayed_image_region
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::displayed_image_region: band "
		<< band << endl));
#endif
if (band <= 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_REGION))
	LOCKED_LOGGING ((
	clog << "    using cached Displayed_Image_Region" << endl
		 << "<<< Tiled_Image_Display::displayed_image_region: "
			<< Displayed_Image_Region << endl));
	#endif
	return Displayed_Image_Region;
	}
QRectF
	region (calculate_displayed_image_region (band));
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::displayed_image_region: " << region << endl));
#endif
return region;
}


QSize
Tiled_Image_Display::displayed_image_region_size
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::displayed_image_region_size: band "
		<< band << endl));
#endif
QRectF
	region;
if (band <= 0)
	region = Displayed_Image_Region;
else
	region = calculate_displayed_image_region (band);
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "    displayed image region = " << region << endl));
#endif
QSize
	region_size
		(round_down (region.right ())  - round_up (region.left ()),
		 round_down (region.bottom ()) - round_up (region.top ()));
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::displayed_image_region_size: "
		<< region_size << endl));
#endif
return region_size;
}


bool
Tiled_Image_Display::reset_displayed_image_region ()
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::reset_displayed_image_region:" << endl
	 << "        Displayed_Image_Region = " << Displayed_Image_Region << endl));
#endif
QRectF
	region (calculate_displayed_image_region (0));
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "    calculated displayed_image_region = " << region << endl));
#endif
bool
	changed = false;
if (region != Displayed_Image_Region)
	{
	QSize
		region_size (round_down (region.size ()));
	if (region_size.rwidth ()
			!= round_down (Displayed_Image_Region.width ()) ||
		region_size.rheight ()
			!= round_down (Displayed_Image_Region.height ()))
		changed = true;

	Displayed_Image_Region = region;
	#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
	LOCKED_LOGGING ((
	clog << "    new Displayed_Image_Region = "
			<< Displayed_Image_Region << endl
		 << "    displayed image region size = " << region_size << endl));
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::reset_displayed_image_region: "
		<< boolalpha << changed << endl));
#endif
return changed;
}


QRectF
Tiled_Image_Display::calculate_displayed_image_region
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::calculate_displayed_image_region: band "
		<< band << endl));
#endif
//	Clip the lower right corner to the image boundary.
QPointF
	lower_right (map_display_to_image (QPoint (width (), height ()), band));
QSizeF
	size_of_image (image_size ());
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "                   image size = " << size_of_image << endl
	 << "    LR image display position = " << lower_right << endl));
#endif
if (lower_right.rx () >= size_of_image.rwidth ())
	lower_right.rx ()  = size_of_image.rwidth ();
if (lower_right.ry () >= size_of_image.rheight ())
	lower_right.ry ()  = size_of_image.rheight ();
QRectF
	region (map_display_to_image (QPoint (), band), lower_right);
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "          LR clipped position = " << lower_right << endl
	 << "<<< Tiled_Image_Display::calculate_displayed_image_region: "
	 	<< region << endl));
#endif
return region;
}



QRect
Tiled_Image_Display::image_display_region
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::image_display_region: band "
		<< band << endl));
#endif
QRectF
	image_region (displayed_image_region (band));
QRect
	display_region
		(map_image_to_display (image_region.topLeft ()),
		 map_image_to_display (image_region.bottomRight () - QPointF (1, 1)));
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::image_display_region: "
		<< display_region << endl));
#endif
return display_region;
}

/*------------------------------------------------------------------------------
	Coordinate mappers
*/
QPointF
Tiled_Image_Display::map_display_to_image
	(
	const QPoint&	coordinate,
	int				band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::map_display_to_image: " << coordinate
		<< ", " << band << 'b' << endl));
#endif
//	Add the display offset in the viewport origin tile.
QPointF
	point (coordinate + Tile_Display_Offset);
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "           Tile_Display_Offset = " << Tile_Display_Offset << endl
	 << "     display coordinate offset = " << point << endl));
#endif

//	Scale the display point to image space.
if (band < 0)
	band = 0;
double
	scale_width,
	scale_height;
image_scaling (&scale_width, &scale_height, band);
point.rx () /= scale_width;
point.ry () /= scale_height;
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
QPointF
	tile_grid_origin (displayed_tile_grid_origin (band));
LOCKED_LOGGING ((
clog << "                       scaling = "
		<< scale_width << "w, " << scale_height << 'h' << endl
	 << "       image coordinate offset = " << point << endl
	 << "    displayed_tile_grid_origin = " << tile_grid_origin << endl));
#endif

//	Add the origin of the active tile grid in image space.
point += displayed_tile_grid_origin (band);
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_display_to_image: " << point << endl));
#endif
return point;
}


QPoint
Tiled_Image_Display::map_image_to_display
	(
	const QPointF&	coordinate,
	int				band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::map_image_to_display: " << coordinate
		<< ", " << band << 'b' << endl));
#endif
if (band < 0)
	band = 0;
//	Offset of the image point from the origin of the active tile grid.
QPointF
	point (coordinate - displayed_tile_grid_origin (band));
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
QPointF
	tile_grid_origin (displayed_tile_grid_origin (band));
LOCKED_LOGGING ((
clog << "    displayed_tile_grid_origin = " << tile_grid_origin << endl
	 << "       image coordinate offset = " << point << endl));
#endif

//	Scale the image point to display space.
double
	scale_width,
	scale_height;
image_scaling (&scale_width, &scale_height, band);
point.rx () *= scale_width;
point.ry () *= scale_height;
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "                       scaling = "
		<< scale_width << "w, " << scale_height << 'h' << endl
	 << "     display coordinate offset = " << point << endl
	 << "           Tile_Display_Offset = "
	 	<< Tile_Display_Offset << endl));
#endif

//	Subtract the display offset in the viewport origin tile.
point -= Tile_Display_Offset;
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_image_to_display: "
		<< round_down (point) << " (" << point << ')' << endl));
#endif
return round_down (point);
}


QPointF
Tiled_Image_Display::map_tile_to_image
	(
	const QPoint&	coordinate,
	int				band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::map_tile_to_image: " << coordinate
		<< ", " << band << 'b' << endl));
#endif
QPointF
	point (displayed_tile_grid_origin (band));
QSizeF
	extent (tile_image_size (band));

#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "    displayed_tile_grid_origin = " << point << endl
	 << "               tile_image_size = " << extent << endl));
#endif
point.rx () += extent.rwidth ()  * (coordinate.x () - 1);
point.ry () += extent.rheight () * (coordinate.y () - 1);
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_tile_to_image: " << point << endl));
#endif
return point;
}


QPoint
Tiled_Image_Display::map_image_to_tile
	(
	const QPointF&	coordinate,
	int				band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::map_image_to_tile: " << coordinate
		<< ", " << band << 'b' << endl));
#endif
QRectF
	region (tiled_image_region (band));
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "    tiled_image_region = " << region << endl));
#endif
if (region.contains (coordinate))
	{
	QPointF
		point (coordinate);
	point -= region.topLeft ();
	#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
	LOCKED_LOGGING ((
	clog << "       Tile_Image_Size = " << Tile_Image_Size << endl));
	#endif
	if (band == 0)
		{
		point.rx () /= Tile_Image_Size.width ();
		point.ry () /= Tile_Image_Size.height ();
		}
	else
		{
		QSizeF
			tile_size (tile_image_size (band));
		point.rx () /= tile_size.rwidth ();
		point.ry () /= tile_size.rheight ();
		}
	#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
	LOCKED_LOGGING ((
	clog << "<<< Tiled_Image_Display::map_image_to_tile: "
			<< round_down (point) << " (" << point << ')' << endl));
	#endif
	return round_down (point);
	}
//	Out of bounds.
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_image_to_tile: "
		<< QPoint (-1, -1) << endl));
#endif
return QPoint (-1, -1);
}


QPoint
Tiled_Image_Display::map_image_to_tile_offset
	(
	const QPointF&	coordinate,
	int				band
	) const
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::map_image_to_tile_offset: " << coordinate
		<< ", " << band << 'b' << endl));
#endif
/*
	>>> CAUTION <<< The tiled_image_region will return the cached
	Tiled_Image_Region value, unless band is positive in which case
	calculate_tiled_image_region will be called. Thus Tiled_Image_Region
	must be current and correct for this method to be reliable.
*/
QRectF
	region (tiled_image_region (band));
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "    tiled_image_region = " << region << endl));
#endif
if (region.contains (coordinate))
	{
	//	Tile region offset in image units.
	QPointF
		point (coordinate);
	point -= region.topLeft ();
	QSizeF
		tile_size (tile_image_size (band));
	#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "     tile grid offset coordinate = " << point << endl
		 << "                 tile_image_size = " << tile_size << endl));
	#endif

	//	Tile offset in image space.
	point.rx () -=
		static_cast<int>(point.rx () / tile_size.rwidth ())
		* tile_size.rwidth ();
	point.ry () -=
		static_cast<int>(point.ry () / tile_size.rheight ())
		* tile_size.rheight ();
	#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "    tile image offset coordinate = " << point << endl));
	#endif

	//	Convert to offset in display space.
	if (band < 0)
		band = 0;
	double
		scale_width,
		scale_height;
	image_scaling (&scale_width, &scale_height, band);
	point.rx () *= scale_width;
	point.ry () *= scale_height;
	#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "                         scaling = "
			<< scale_width << "w, " << scale_height << 'h' << endl
		 << "<<< Tiled_Image_Display::map_image_to_tile_offset: "
			<< round_down (point) << " (" << point << ')' << endl));
	#endif
	return round_down (point);
	}
//	Out of bounds.
#if ((DEBUG_SECTION) & (DEBUG_MAP_POINTS | DEBUG_ORIGIN))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_image_to_tile_offset: "
		<< QPoint (-1, -1) << endl));
#endif
return QPoint (-1, -1);
}


QPoint
Tiled_Image_Display::map_display_to_tile
	(
	const QPoint&	coordinate
	) const
{
if (Tile_Display_Size.width ()  == 0 ||
	Tile_Display_Size.height () == 0)
	return QPoint (0, 0);

//	Display viewport coordinate.
QPoint
	point (coordinate);
//	Adjust to display tile grid origin.
point += Tile_Display_Offset;	//	Display viewport offset.
//	Convert to tile grid offset.
point.rx () /= Tile_Display_Size.width ();
point.ry () /= Tile_Display_Size.height ();
//	Adjust for tile margin.
point.rx ()++;
point.ry ()++;
if (point.rx () < 0 ||
	point.ry () < 0 ||
	point.rx () >= Tile_Grid_Size.width () ||
	point.ry () >= Tile_Grid_Size.height ())
	//	Outside the effective tile grid.
	point.rx () =
	point.ry () = -1;
return point;
}


QPoint
Tiled_Image_Display::map_display_to_tile_offset
	(
	const QPoint&	coordinate
	) const
{
//	Display viewport coordinate.
QPoint
	point (coordinate);
//	Adjust to display tile grid origin.
point += Tile_Display_Offset;	//	Display viewport offset.
//	Convert to tile offset.
point.rx () %= Tile_Display_Size.width ();
point.ry () %= Tile_Display_Size.height ();
return point;
}


QRect
Tiled_Image_Display::tile_relative_region
	(
	const QRect&	tile_region,
	QPoint			tile_origin
	)
{
QRect
	region (tile_region);
if (region.isEmpty ())
	tile_origin.rx () =
	tile_origin.ry () = 0;
else
	{
	//	Move the viewport-relative tile origin to the tile-relative coordinate.
	if (tile_origin.rx () < 0)
		tile_origin.rx () = -tile_origin.rx ();
	else
	if (tile_region.left () > tile_origin.rx ())
		tile_origin.rx () = tile_region.left () - tile_origin.rx ();
	else
		tile_origin.rx () = 0;
	if (tile_origin.ry () < 0)
		tile_origin.ry () =
			-tile_origin.ry ();
	else
	if (tile_region.top () > tile_origin.ry ())
		tile_origin.ry () = tile_region.top () - tile_origin.ry ();
	else
		tile_origin.ry () = 0;
	}
//	Move the display region to the tile_relative origin.
region.moveTo (tile_origin);
return region;
}


void
Tiled_Image_Display::viewport_relative_region
	(
	const QPoint&	tile_coordinate,
	QRect&			tile_region
	) const
{
QPoint
	origin (tile_display_origin (tile_coordinate));
//	Tile-relative origin offset.
origin += tile_region.topLeft ();
tile_region.moveTo (origin);
}

/*==============================================================================
	Tiles
*/
QSizeF
Tiled_Image_Display::tile_image_size
	(
	int		band
	) const
{
if (band <= 0)
	return Tile_Image_Size;
return calculate_tile_image_size (band);
}


bool
Tiled_Image_Display::reset_tile_image_size ()
{
QSizeF
	size (calculate_tile_image_size ());
bool
	changed = (size != Tile_Image_Size);
if (changed)
	Tile_Image_Size = size;
return changed;
}


QSizeF
Tiled_Image_Display::calculate_tile_image_size
	(
	int		band
	) const
{
if (band < 0)
	band = 0;
double
	scale_width,
	scale_height;
image_scaling (&scale_width, &scale_height, band);
QSizeF
	tile_size (tile_display_size ());
tile_size.rwidth ()  /= scale_width;
tile_size.rheight () /= scale_height;
return tile_size;
}


void
Tiled_Image_Display::tile_display_size
	(
	const QSize&	size
	)
{
if (Image_Loading)
	return;

QSize
	tile_size (size);
if (tile_size.isEmpty ())
	tile_size = Default_Tile_Display_Size;
if (tile_size.rwidth ()  < MINIMUM_TILE_DIMENSION)
	tile_size.rwidth ()  = MINIMUM_TILE_DIMENSION;
if (tile_size.rheight () < MINIMUM_TILE_DIMENSION)
	tile_size.rheight () = MINIMUM_TILE_DIMENSION;
if (tile_size != Tile_Display_Size)
	{
	Pending_State_Change_Enabled = false;

	//	Cancel all rendering.
	Renderer->reset (Image_Renderer::WAIT_UNTIL_DONE);

	//	Reset the tile grid.
	clear_tiles ();
	Tile_Display_Size = tile_size;
	reset_tile_image_size ();
	resize_tile_grid ();

	//	Initialize the tile images.
	reset_tiles ();

	//	Restart rendering.
	Pending_State_Change_Enabled = true;
	state_change_start (TILE_SIZE_STATE);
	}
}


void
Tiled_Image_Display::default_tile_display_size
	(
	const QSize&	size
	)
{
QSize
	tile_size (size);
if (tile_size.isEmpty ())
	{
	tile_size.rwidth ()  = DEFAULT_IMAGE_TILE_WIDTH;
	tile_size.rheight () = DEFAULT_IMAGE_TILE_HEIGHT;
	}
else
	{
	if (tile_size.rwidth ()  < MINIMUM_TILE_DIMENSION)
		tile_size.rwidth ()  = MINIMUM_TILE_DIMENSION;
	if (tile_size.rheight () < MINIMUM_TILE_DIMENSION)
		tile_size.rheight () = MINIMUM_TILE_DIMENSION;
	}
Default_Tile_Display_Size = tile_size;
}


#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
void
Tiled_Image_Display::print_tile_grid () const
{
QList<Plastic_Image*>
	*tiles;
Plastic_Image
	*image;
QRect
	viewport (rect ()),
	tile_region (-Tile_Display_Offset, Tile_Display_Size);
//	Move to the tile grid origin.
tile_region.translate
	(-Tile_Display_Size.width (), -Tile_Display_Size.height ());
int
	tile_region_left_edge = tile_region.x ();
int
	rows = Tile_Grid_Images->size (),
	row = -1,
	cols,
	col;
clog << "    Tiled_Image_Display::Tile_Grid_Images " << Tile_Grid_Size;
if (rows)
	{
	clog << " -" << endl
		 << "    -------------------------------------" << endl;
	while (++row < rows)
		{
		clog << "    row " << row << " -" << endl;
		if ((tiles = Tile_Grid_Images->at (row)))
			{
			cols = tiles->size ();
			col = -1;
			while (++col < cols)
				{
				clog << "     ";
				if (tile_region.intersects (viewport))
					clog << '*';
				else
					clog << ' ';
				clog << "col " << col << ' ';
				image = tiles->at (col);
				if (image)
					clog << *image << endl;
				else
					clog << "NULL" << endl;
				tile_region.translate (Tile_Display_Size.width (), 0);
				}
			}
		else
			clog << "      empty" << endl;
		tile_region.moveTo
			(tile_region_left_edge,
			 tile_region.y () + Tile_Display_Size.height ());
		}
	clog << "    -------------------------------------" << endl;
	}
else
	clog << endl;
}


void
Tiled_Image_Display::print_tile_pool () const
{
int
	entries = Tile_Image_Pool.size ();
clog << "    Tiled_Image_Display::Tile_Image_Pool ";
if (entries)
	{
	clog << " -" << endl
		 << "    -------------------------------------" << endl;
	for (int
			index = 0;
			index < entries;
			index++)
		clog << "    " << index << ": " << *Tile_Image_Pool[index] << endl;
	}
else
	clog << "empty" << endl;
}

#else
void
Tiled_Image_Display::print_tile_grid () const
{}

void
Tiled_Image_Display::print_tile_pool () const
{}
#endif


bool
Tiled_Image_Display::resize_tile_grid ()
{
#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << ">>> Tiled_Image_Display::resize_tile_grid" << endl
	 << "    in " << pathname << endl
	 << "     Tile_Display_Size = " << Tile_Display_Size << endl
	 << "    Tiled_Image_Region = " << Tiled_Image_Region << endl
	 << "    pre-resize -" << endl;
print_tile_grid ();
UNLOCK_LOG;
#endif
bool
	resized = false;

if (Image_Loading ||
	Tile_Display_Size.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
	LOCKED_LOGGING ((
	clog << "    " << (Image_Loading ?
			"Image_Loading in progress" :
			"empty Tile_Display_Size") << endl
		 << "<<< Tiled_Image_Display::resize_tile_grid: "
			<< boolalpha << resized << endl));
	#endif
	return resized;
	}

//	Effective tile grid size.
QSize
	display_size (size ());
int
	old_cols = Tile_Grid_Size.rwidth (),
	old_rows = Tile_Grid_Size.rheight (),
	new_cols =
		(display_size.rwidth () + Tile_Display_Size.rwidth ())
		/ Tile_Display_Size.rwidth (),
	new_rows =
		(display_size.rheight () + Tile_Display_Size.rheight ())
		/ Tile_Display_Size.rheight ();
if (display_size.rwidth () % Tile_Display_Size.rwidth ())
	++new_cols;
if (display_size.rheight () % Tile_Display_Size.rheight ())
	++new_rows;
//	Pre-render margins.
new_cols += 2;
new_rows += 2;
#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
LOCKED_LOGGING ((
clog << "            image size = " << image_size () << endl
	 << "     scaled image size = " << display_size << endl
	 << "          display size = " << size () << endl
	 << "     tile display size = " << Tile_Display_Size << endl
	 << "    old tile grid size = " << Tile_Grid_Size << endl
	 << "    new tile grid size = "
		<< new_cols << "w, " << new_rows << 'h' << endl));
#endif
if (new_rows != old_rows ||
	new_cols != old_cols)
	{
	//	Resize the tile grid.
	resized = true;

	//	Increase the maximum tile image pool size if needed.
	if (Tile_Image_Pool_Max < (new_rows * new_cols))
		Tile_Image_Pool_Max =  new_rows * new_cols;

	QList<Plastic_Image*>
		*tiles;
	Plastic_Image
		*tile_image;
	int
		tile_row = -1,
		tile_col;
	while (++tile_row < new_rows)
		{
		if (tile_row < old_rows)
			tiles = Tile_Grid_Images->at (tile_row);
		else
			{
			//	Add empty tile row.
			#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
			LOCKED_LOGGING ((
			clog << "      adding row " << tile_row << endl));
			#endif
			Tile_Grid_Images->append (tiles = NULL);
			}
		if (tiles)
			{
			if ((tile_col = tiles->size ()) < new_cols)
				{
				//	Add tile column.
				#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
				LOCKED_LOGGING ((
				clog << "      adding " << (new_cols - tile_col)
						<< " columns to row " << tile_row << endl));
				#endif
				while (tile_col++ < new_cols)
					tiles->append (NULL);
				}
			else
				{
				//	Remove unused extra tile columns.
				while (tile_col-- > new_cols)
					{
					#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
					LOCKED_LOGGING ((
					clog << "      removing tile "
							<< tile_col << ',' << tile_row
							<< " @ " << (void*)(tiles->at (tile_col)) << endl));
					#endif
					if ((tile_image = tiles->takeLast ()))
						{
						if (Tile_Image_Pool.size () < Tile_Image_Pool_Max)
							{
							//	Move the unused image to the image pool.
							#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
							LOCKED_LOGGING ((
							clog << "        move image to pool -" << endl
								 << "        " << *tile_image << endl));
							#endif
							Tile_Image_Pool.append (tile_image);
							}
						else
							{
							//	Delete the unused image.
							#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
							LOCKED_LOGGING ((
							clog << "        queue for deletion "
									<< *tile_image << endl));
							#endif
							Renderer->delete_image (tile_image);
							}
						}
					}
				}
			}
		}

	//	Remove unused extra tile rows.
	tile_row = old_rows;
	while (tile_row-- > new_rows)
		{
		if ((tiles = Tile_Grid_Images->takeLast ()))
			{
			tile_col = tiles->size ();
			while (tile_col--)
				{
				#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
				LOCKED_LOGGING ((
				clog << "      removing tile "
						<< tile_col << ',' << tile_row
						<< " @ " << (void*)(tiles->at (tile_col)) << endl));
				#endif
				if ((tile_image = tiles->takeLast ()))
					{
					if (Tile_Image_Pool.size () < Tile_Image_Pool_Max)
						{
						//	Move the unused image to the image pool.
						#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
						LOCKED_LOGGING ((
						clog << "        move image to pool -" << endl
							 << "        " << *tile_image << endl));
						#endif
						Tile_Image_Pool.append (tile_image);
						}
					else
						{
						#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
						LOCKED_LOGGING ((
						clog << "        queue for deletion "
								<< *tile_image << endl));
						#endif
						Renderer->delete_image (tile_image);
						}
					}
				}
			delete tiles;
			}
		}

	//	Tile grid size.
	Tile_Grid_Size.rwidth ()  = new_cols;
	Tile_Grid_Size.rheight () = new_rows;

	//	Tiled image region.
	Tiled_Image_Region.setWidth  (new_rows * Tile_Image_Size.rwidth ());
	Tiled_Image_Region.setHeight (new_cols * Tile_Image_Size.rheight ());
	#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
	LOCK_LOG;
	clog << "    Tiled_Image_Display::resize_tile_grid" << endl
		 << "    in " << pathname << endl
		 << "       Tile_Image_Size = " << Tile_Image_Size << endl
		 << "    Tiled_Image_Region = " << Tiled_Image_Region << endl
		 << "    post-resize -" << endl;
	print_tile_grid ();
	UNLOCK_LOG;
	Renderer->print_render_queue ();
	Renderer->print_delete_queue ();
	LOCK_LOG;
	Image_Tile::tile_accounting ();
	Renderer->image_accounting ();
	UNLOCK_LOG;
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_TILE_GRID)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::resize_tile_grid: "
		<< boolalpha << resized << endl));
#endif
return resized;
}


QRect
Tiled_Image_Display::tile_display_region
	(
	const QPoint&	coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::tile_display_region: "
		<< coordinate << endl));
#endif
QRect
	region (tile_display_origin (coordinate), Tile_Display_Size);
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::tile_display_region: " << region << endl));
#endif
return region;
}


QPoint
Tiled_Image_Display::tile_display_origin
	(
	const QPoint&	coordinate
	) const
{
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::tile_display_origin: "
		<< coordinate << endl));
#endif
//	Tile grid coordinate.
QPoint
	origin (coordinate);
//	Adjust for tile grid margin relative to Tile_Display_Offset tile.
origin.rx ()--;
origin.ry ()--;
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "            active grid = " << origin << endl));
#endif
//	Offsets in display units from display tile grid origin.
origin.rx () *= Tile_Display_Size.width ();
origin.ry () *= Tile_Display_Size.height ();
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "      Tile_Display_Size = " << Tile_Display_Size << endl
	 << "          display units = " << origin << endl));
#endif
//	Display viewport offset in the first (UL) visible tile.
origin -= Tile_Display_Offset;
#if ((DEBUG_SECTION) & DEBUG_MAP_POINTS)
LOCKED_LOGGING ((
clog << "    Tile_Display_Offset = " << Tile_Display_Offset << endl
	 << "<<< Tiled_Image_Display::tile_display_origin: " << origin << endl));
#endif
return origin;
}


QRectF
Tiled_Image_Display::tiled_image_region
	(
	int		band
	) const
{
if (band <= 0)
	return Tiled_Image_Region;
return calculate_tiled_image_region (band);
}


bool
Tiled_Image_Display::reset_tiled_image_region ()
{
reset_tile_image_size ();
QRectF
	region (calculate_tiled_image_region ());
bool
	changed = (region != Tiled_Image_Region);
if (changed)
	Tiled_Image_Region = region;
return changed;
}


QRectF
Tiled_Image_Display::calculate_tiled_image_region
	(
	int		band
	) const
{
QPointF
	origin (displayed_tile_grid_origin (band));
QSizeF
	extent (calculate_tile_image_size (band));
origin.rx () -= extent.rwidth ();
origin.ry () -= extent.rheight ();
extent.rheight () *= Tile_Grid_Size.width ();
extent.rwidth ()  *= Tile_Grid_Size.height ();
return QRectF (origin, extent);
}


#ifndef DOXYGEN_PROCESSING
namespace
{
enum
	{
	RENDER_TILES	= -1,
	NO_CHANGE		= 0,
	UPDATE_DISPLAY	= 1
	};


QString
move_change_description
	(
	int		change
	)
{
switch (change)
	{
	case NO_CHANGE:
		return "NO_CHANGE";
	case UPDATE_DISPLAY:
		return "UPDATE_DISPLAY";
	case RENDER_TILES:
		return "RENDER_TILES";
	}
return "unknown";
}
}
#endif


bool
Tiled_Image_Display::move_image
	(
	const QPoint&	origin,
	int				band
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN | DEBUG_OVERVIEW))
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::move_image: " << origin
		<< ", " << band << 'b' << endl
	 << "    in " << pathname << endl));
#endif
int
	change = NO_CHANGE,
	tiles_reset = NO_TILES_RESET;
bool
	image_region_resized = false;

QPoint
	clipped_origin (origin);
//	Clip the origin of the reference band to within the image bounds.
if (clipped_origin.rx () > Lower_Right_Origin_Limit.rx ())
	clipped_origin.rx () = Lower_Right_Origin_Limit.rx ();
if (clipped_origin.ry () > Lower_Right_Origin_Limit.ry ())
	clipped_origin.ry () = Lower_Right_Origin_Limit.ry ();
if (clipped_origin.rx () < 0)
	clipped_origin.rx () = 0;
if (clipped_origin.ry () < 0)
	clipped_origin.ry () = 0;

QPoint
	current_origin (round_down (displayed_image_origin (band)));
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
QPointF
	real_origin (displayed_image_origin (band)),
	mapped_origin (map_display_to_image (QPoint ()));
LOCKED_LOGGING ((
clog << "      Lower_Right_Origin_Limit = " << Lower_Right_Origin_Limit << endl
	 << "                clipped origin = " << clipped_origin << endl
	 << "        displayed_image_origin = " << current_origin
	 	<< " (" << real_origin << ')' << endl
	 << "         mapped display origin = " << round_down (mapped_origin)
	 	<< " (" << mapped_origin << ')' << endl));
#endif
if (clipped_origin != current_origin)
	{
	Pending_State_Change_Enabled = false;
	change = UPDATE_DISPLAY;

	QPointF
		location (clipped_origin);
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "            Tiled_Image_Region = "
			<< Tiled_Image_Region << endl
		 << "           Tile_Display_Offset = "
			<< Tile_Display_Offset << endl));
	QPoint
		tile_display_offset (map_image_to_tile_offset (current_origin));
	LOCKED_LOGGING ((
	clog << "    mapped tile display offset = "
			<< tile_display_offset << endl
		 << "        Displayed_Image_Region = "
			<< Displayed_Image_Region << endl
		 << "               Tile_Image_Size = "
			<< Tile_Image_Size << endl));
	#endif
	if (band <= 0)
		{
		//	Move the reference band.
		QPoint
			offset (map_image_to_tile (location, band));
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		LOCKED_LOGGING ((
		clog << "    map_image_to_tile = " << offset << endl));
		#endif
		if (offset.rx () < 0 ||
			offset.ry () < 0)
			{
			//	Now for someplace completely different.
			change = RENDER_TILES;
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
			LOCKED_LOGGING ((
			clog << "    reset tile grid" << endl));
			#endif
			if (! Image_Loading)
				//	Cancel all rendering.
				Renderer->reset (Image_Renderer::DO_NOT_WAIT);

			//	Tile offset from image origin.
			offset.rx () = location.rx () / Tile_Image_Size.rwidth ();
			offset.ry () = location.ry () / Tile_Image_Size.rheight ();
			//	Allow for top/left tile margin.
			offset.rx ()--;
			offset.ry ()--;
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
			LOCKED_LOGGING ((
			clog << "    reset tile grid offset = " << offset << endl));
			#endif
			//	Tile grid offset in image space.
			location.rx () = offset.rx () * Tile_Image_Size.rwidth ();
			location.ry () = offset.ry () * Tile_Image_Size.rheight ();
			Tiled_Image_Region.moveTo (location);
			}
		else
			{
			//	Tile grid shift.
			offset.rx ()--;
			offset.ry ()--;
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
			LOCKED_LOGGING ((
			clog << "    shift tile grid offset = " << offset << endl));
			#endif
			if (! offset.isNull ())
				{
				change = RENDER_TILES;
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
				LOCK_LOG;
				clog << "    Tiled_Image_Display::move_image" << endl
					 << "    in " << pathname << endl
					 << "    pre-shift -" << endl;
				print_tile_grid ();
				UNLOCK_LOG;
				#endif

				if (offset.ry ())
					{
					//	Shift tile rows.
					Tiled_Image_Region.moveTop
						(Tiled_Image_Region.top ()
							+ (offset.ry () * Tile_Image_Size.rheight ()));
					#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
					LOCKED_LOGGING ((
					clog << "    rotate rows " << offset.ry ()
							<< "; new top = "
							<< Tiled_Image_Region.top () << endl));
					#endif
					if (offset.ry () > 0)
						//	Rotate up.
						while (offset.ry ()--)
							Tile_Grid_Images->append
								(Tile_Grid_Images->takeFirst ());
					else
						//	Rotate down.
						while (offset.ry ()++)
							Tile_Grid_Images->prepend
								(Tile_Grid_Images->takeLast ());
					}
				if (offset.rx ())
					{
					//	Shift tile columns.
					Tiled_Image_Region.moveLeft
						(Tiled_Image_Region.left ()
							+ (offset.rx () * Tile_Image_Size.rwidth ()));
					#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
					LOCKED_LOGGING ((
					clog << "    rotate cols " << offset.rx ()
							<< "; new left = "
							<< Tiled_Image_Region.left () << endl));
					#endif
					QList<Plastic_Image*>
						*tiles;
					int
						shift,
						tile_row = Tile_Grid_Size.rheight ();
					while (tile_row)
						{
						if ((tiles = Tile_Grid_Images->at (--tile_row)))
							{
							if ((shift = offset.rx ()) > 0)
								//	Rotate left.
								while (shift--)
									tiles->append (tiles->takeFirst ());
							else
								//	Rotate right.
								while (shift++)
									tiles->prepend (tiles->takeLast ());
							}
						}
					}
				#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
				LOCK_LOG;
				clog << "    Tiled_Image_Display::move_image" << endl
					 << "    in " << pathname << endl
					 << "    post-shift -" << endl;
				print_tile_grid ();
				UNLOCK_LOG;
				#endif
				}
			}

		if (change == RENDER_TILES)
			{
			/*	Reset the tile grid origin.

				>>> CAUTION <<< This must be done before other operations
				that depend on the position of the tile grid origin.
			*/
			location = Tiled_Image_Region.topLeft ();
			location.rx () += Tile_Image_Size.rwidth ();
			location.ry () += Tile_Image_Size.rheight ();
			#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
			LOCKED_LOGGING ((
			clog << "    relocated Tiled_Image_Region = "
					<< Tiled_Image_Region << endl
				 << "      displayed_tile_grid_origin = " << location << endl));
			#endif
			displayed_tile_grid_origin (location, band);
			}

		/*	Reset the display tile offset.

			>>> CAUTION <<< This must be done before other operations
			that depend on this value; e.g. reset_displayed_image_region
			depends on calculate_displayed_image_region which depends on
			map_display_to_image which depends on Tile_Display_Offset.
		*/
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		real_origin = displayed_image_origin (band);
		mapped_origin = map_display_to_image (QPoint ());
		LOCKED_LOGGING ((
		clog << "    before map_image_to_tile_offset -" << endl
			 << "                clipped_origin = " << clipped_origin << endl
			 << "        displayed_image_origin = "
		 		<< round_down (real_origin) << " (" << real_origin << ')'
				<< endl
			 << "         mapped display origin = "
		 		<< round_down (mapped_origin) << " (" << mapped_origin << ')'
				<< endl));
		#endif
		Tile_Display_Offset = map_image_to_tile_offset (clipped_origin);
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		real_origin = displayed_image_origin (band);
		mapped_origin = map_display_to_image (QPoint ());
		LOCKED_LOGGING ((
		clog << "    after map_image_to_tile_offset -" << endl
			 << "       new Tile_Display_Offset = "
			 << Tile_Display_Offset << endl
			 << "        displayed_image_origin = "
		 		<< round_down (real_origin) << " (" << real_origin << ')'
				<< endl
			 << "         mapped display origin = "
		 		<< round_down (mapped_origin) << " (" << mapped_origin << ')'
				<< endl));
		#endif

		//	Reset the displayed image region.
		image_region_resized = reset_displayed_image_region ();
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		real_origin = displayed_image_origin (band);
		mapped_origin = map_display_to_image (QPoint ());
		LOCKED_LOGGING ((
		clog << "    reset_displayed_image_region = "
				<< image_region_resized << endl
			 << "        Displayed_Image_Region = "
			 	<< Displayed_Image_Region << endl
			 << "        displayed_image_origin = "
		 		<< round_down (real_origin) << " (" << real_origin << ')'
				<< endl
			 << "         mapped display origin = "
		 		<< round_down (mapped_origin) << " (" << mapped_origin << ')'
				<< endl));
		#endif
		}
	else
		{
		/*	Reset the tile grid origin.

			>>> CAUTION <<< This must be done before other operations
			that depend on this value; e.g. reset_displayed_image_region
			depends on calculate_displayed_image_region which depends on
			map_display_to_image which depends on Tile_Display_Offset.
		*/
		change = RENDER_TILES;
		location -= current_origin;	//	Origin offset.
		location += displayed_tile_grid_origin (band);
		displayed_tile_grid_origin (location, band);
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		LOCKED_LOGGING ((
		clog << "    change to non-reference band" << endl
			 << "    displayed_tile_grid_origin = " << location << endl));
		#endif
		}

	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "    change = " << change << " - "
			<< move_change_description (change) << endl));
	#endif
	if (change == RENDER_TILES)
		{
		//	Reset the tile locations.
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		LOCKED_LOGGING ((
		clog << "    reset_tiles ..." << endl));
		#endif
		if ((tiles_reset = reset_tiles ()) != VISIBLE_TILES_RESET)
			change = UPDATE_DISPLAY;
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		if (tiles_reset)
			{
			LOCK_LOG;
			clog << "    Tiled_Image_Display::move_image" << endl
				 << "    in " << pathname << endl
				 << "    post-reset -" << endl;
			print_tile_grid ();
			UNLOCK_LOG;
			}
		#endif
		}

	//	Restart rendering.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
	LOCKED_LOGGING ((
	clog << "    start Renderer" << endl));
	#endif
	Pending_State_Change_Enabled = true;
	state_change_start (IMAGE_MOVE_STATE);
	}

if (change ||
	clipped_origin != origin)	//	Report if requested origin was clipped.
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
	real_origin = displayed_image_origin (band);
	mapped_origin = map_display_to_image (QPoint ());
	LOCKED_LOGGING ((
	clog << "    image " << (change ? "moved" : "requested origin clipped")
			<< " to " << clipped_origin << endl
		 << "            Tiled_Image_Region = "
			<< Tiled_Image_Region << endl
		 << "           Tile_Display_Offset = "
			<< Tile_Display_Offset << endl
		 << "        Displayed_Image_Region = "
			<< Displayed_Image_Region << endl
		 << "        displayed_image_origin = "
		 	<< round_down (real_origin) << " (" << real_origin << ')' << endl
		 << "         mapped display origin = "
		 	<< round_down (mapped_origin) << " (" << mapped_origin << ')'
			<< endl));
	#endif
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::move_image: "
			"emit image_moved" << endl
		 << "    origin = " << clipped_origin << endl
		 << "      band = " << band << endl));
	#endif
	emit image_moved (clipped_origin, band);
	}

if (image_region_resized)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::move_image: "
			"emit displayed_image_region_resized: "
			<< displayed_image_region_size () << endl));
	#endif
	emit displayed_image_region_resized (displayed_image_region_size ());
	}

if (change == UPDATE_DISPLAY)
	{
	//	No visible tiles changed but origin moved; schedule a display repaint.
	update ();

	if (! tiles_reset)
		{
		//	Report the completion of the state change.
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN))
		LOCKED_LOGGING ((
		clog << "    state_change_completed COMPLETED_WITHOUT_RENDERING_STATE"
				<< endl));
		#endif
		state_change_completed (COMPLETED_WITHOUT_RENDERING_STATE);
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE | DEBUG_ORIGIN | DEBUG_OVERVIEW))
real_origin = displayed_image_origin ();
LOCKED_LOGGING ((
clog << "    new origin = " << real_origin << endl
	 << "<<< Tiled_Image_Display::move_image: " << boolalpha
	 	<< (change != NO_CHANGE) << endl));
#endif
return change;
}


int
Tiled_Image_Display::reset_tiles ()
{
#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::reset_tiles" << endl
	 << "    in " << pathname << endl
	 << "    Tile_Grid_Size = " << Tile_Grid_Size << endl));
#endif
int
	changed = NO_TILES_RESET;
bool
	origin_changed,
	scaling_changed;
int
	tile_rows = Tile_Grid_Size.rheight (),
	tile_cols = Tile_Grid_Size.rwidth ();
if (! tile_rows ||
	! tile_cols)
	{
	#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
	LOCKED_LOGGING ((
	clog << "    no tiles" << endl
		 << "<<< Tiled_Image_Display::reset_tiles: " << changed << endl));
	#endif
	return changed;
	}

if (! Image_Loading)
	//	Stop rendering while tiles are being reset.
	Renderer->stop_rendering ();

//	Tile display geometry.
QPoint
	tile_grid,
	displayed_tile_origin;
QRect
	viewport (rect ()),
	//	Equivalent to tile_display_region for the first visible tile.
	tile_region (-Tile_Display_Offset, Tile_Display_Size),
	//	Region of the tile visible in the display.
	displayed_tile_region;
//	Move to the tile grid origin.
tile_region.translate
	(-Tile_Display_Size.width (), -Tile_Display_Size.height ());
int
	tile_region_left_edge = tile_region.x ();
#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
LOCKED_LOGGING ((
clog << "    Display geometry -" << endl
	 << "    Tile_Display_Offset = " << Tile_Display_Offset << endl
	 << "      Tile_Display_Size = " << Tile_Display_Size << endl
	 << "               viewport = " << rect () << endl));
#endif

//	Tile image geometry.
QPointF
	tile_locations[3];
double
	tile_locations_left_edge[3];
QSizeF
	size_of_image (image_size ()),
	tile_sizes[3];
#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
LOCKED_LOGGING ((
clog << "    Image geometry -" << endl
	 << "    image size = " << size_of_image << endl));
#endif
int
	band = 3;
while (band--)
	{
	tile_sizes[band] = tile_image_size (band);
	tile_locations[band] = displayed_tile_grid_origin (band);
	//	Adjust to top-left outer margin.
	tile_locations_left_edge[band] =
	tile_locations[band].rx () -= tile_sizes[band].rwidth ();
	tile_locations[band].ry () -= tile_sizes[band].rheight ();
	#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
	LOCKED_LOGGING ((
	clog << "      band " << band
			<< " tile size = " << tile_sizes[band] << endl));
	#endif
	}

#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
LOCK_LOG;
clog << "    Tiled_Image_Display::reset_tiles"  << endl
	 << "    in " << pathname << endl
	 << "    pre-reset -" << endl;
print_tile_grid ();
UNLOCK_LOG;
#endif
QList<Plastic_Image*>
	*tiles;
Plastic_Image
	*tile_image;
while (true)
	{
	#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
	LOCKED_LOGGING ((
	clog << "    tile row " << tile_grid.ry ()
			<< " location = " << tile_locations[0] << endl));
	#endif
	if (! (tiles = Tile_Grid_Images->at (tile_grid.ry ())))
		{
		if (tile_locations[0].ry () >= 0 &&
			tile_locations[0].ry () < size_of_image.rheight ())
			{
			//	Activate the tile row.
			#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
			LOCKED_LOGGING ((
			clog << "+++ activating row -" << endl));
			#endif
			Tile_Grid_Images->replace
				(tile_grid.ry (), tiles = new QList<Plastic_Image*>);
			tile_grid.rx () = tile_cols;
			while (tile_grid.rx ()--)
				tiles->append (NULL);
			}
		}
	else
	if (tile_locations[0].ry () < 0 ||
		tile_locations[0].ry () >= size_of_image.rheight ())
		{
		//	Deactivate the tile row.
		#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
		LOCKED_LOGGING ((
		clog << "--- deactivating row -" << endl));
		#endif
		tile_grid.rx () = tile_cols;
		while (tile_grid.rx ()--)
			{
			if ((tile_image = tiles->takeLast ()))
				{
				//	Deactivate the tile.
				#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
				LOCKED_LOGGING ((
				clog << "----- deactivating column " << tile_grid.rx () << endl
					 << "        cancel rendering " << endl));
				#endif
				Renderer->cancel (tile_image, Image_Renderer::DO_NOT_WAIT);
				if (Tile_Image_Pool.size () < Tile_Image_Pool_Max)
					{
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					LOCKED_LOGGING ((
					clog << "      move to pool " << *tile_image << endl));
					#endif
					Tile_Image_Pool.append (tile_image);
					}
				else
					{
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					LOCKED_LOGGING ((
					clog << "      queue for deletion "
							<< *tile_image << endl));
					#endif
					Renderer->delete_image (tile_image);
					}
				}
			}
		delete tiles;
		Tile_Grid_Images->replace (tile_grid.ry (), tiles = NULL);
		}

	tile_grid.rx () = 0;
	if (tiles)
		{
		while (true)
			{
			#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
			LOCKED_LOGGING ((
			clog << "      tile column " << tile_grid.rx ()
					<< " location = " << tile_locations[0] << endl));
			#endif
			if (! (tile_image = tiles->at (tile_grid.rx ())))
				{
				if (tile_locations[0].rx () >= 0 &&
					tile_locations[0].rx () < size_of_image.rwidth () &&
					! Image_Loading)
					{
					//	Activate the tile.
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					LOCKED_LOGGING ((
					clog << "+++++ activating column -" << endl));
					#endif
					if (Tile_Image_Pool.isEmpty ())
						{
						tile_image =
							Renderer->image_clone
								(Tile_Display_Size,
								//	Mappings shared with the Reference_Image.
								Plastic_Image::BAND_MAP |
								Plastic_Image::DATA_MAPS);
						#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | \
										DEBUG_IMAGE_GEOMETRY))
						LOCK_LOG;
						if (tile_image)
							clog << "      cloned " << *tile_image << endl;
						else
							clog << "!!!!! image_clone failed!" << endl;
						UNLOCK_LOG;
						#endif
						if (! tile_image)
							goto Done;
						}
					else
						{
						tile_image = Tile_Image_Pool.takeLast ();
						#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | \
										DEBUG_IMAGE_GEOMETRY))
						LOCKED_LOGGING ((
						clog << "      move from pool "
								<< *tile_image << endl));
						#endif
						}
					tiles->replace (tile_grid.rx (), tile_image);
					}
				}
			else
			if ((tile_locations[0].rx () < 0 ||
				 tile_locations[0].rx () >= size_of_image.rwidth ()) &&
				! Image_Loading)
				{
				//	Deactivate the tile.
				#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
				LOCKED_LOGGING ((
				clog << "----- deactivating column" << endl
					 << "        cancel rendering " << endl));
				#endif
				Renderer->cancel (tile_image, Image_Renderer::DO_NOT_WAIT);
				if (Tile_Image_Pool.size () < Tile_Image_Pool_Max)
					{
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					LOCKED_LOGGING ((
					clog << "      move to pool " << *tile_image << endl));
					#endif
					Tile_Image_Pool.append (tile_image);
					}
				else
					{
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					LOCKED_LOGGING ((
					clog << "      queue for deletion "
							<< *tile_image << endl));
					#endif
					Renderer->delete_image (tile_image);
					}
				tiles->replace (tile_grid.rx (), tile_image = NULL);
				}

			if (tile_image)
				{
				//	Reset the tile image.
				#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
				LOCKED_LOGGING ((
				clog << "        ante " << *tile_image << endl
					 << "        display region " << tile_region
					 	<< ", visible " << boolalpha
					 	<< tile_region.intersects (viewport) << endl));
				#endif

				origin_changed =
				scaling_changed = false;
				band = 3;
				while (band--)
					{
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					QSizeF
						scaling (image_scaling (band));
					LOCKED_LOGGING ((
					clog << "        band " << band
							<< " - origin " << tile_locations[band]
							<< ", scaling " << scaling << endl));
					#endif
					scaling_changed |= tile_image->source_scaling
						(image_scaling (band));
					origin_changed |= tile_image->source_origin
						(tile_locations[band], band);
					}

				if (! changed &&
					(origin_changed ||
					 scaling_changed))
					//	The change is intially presumed to be for a margin tile.
					changed = BACKGROUND_TILES_RESET;
				#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
				LOCKED_LOGGING ((
				clog << "        post " << *tile_image << endl
					 << boolalpha
					 << "           origin_changed = " << origin_changed << endl
					 << "          scaling_changed = " << scaling_changed
					 << endl));
				#endif
				#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
				//	Tile markings.
				QString
					label = QString (" reset @ %1: g %2, %3; o %4, %5, s %6")
					.arg ((ulong)tile_image, 0, 16)
					.arg (tile_grid.rx ())
					.arg (tile_grid.ry ())
					.arg (tile_image->source_origin ().x ())
					.arg (tile_image->source_origin ().y ())
					.arg (tile_image->source_scaling ().width());
				mark_image (tile_image, label,
					TILE_MARKINGS_RESET_Y,
					TILE_MARKINGS_RESET_COLOR);
				mark_image (tile_image, label,
					tile_image->height () - TILE_MARKINGS_RESET_Y,
					TILE_MARKINGS_RESET_COLOR);
				#endif

				if (tile_image->needs_update () &&
					! Image_Loading)
					{
					#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
					LOCKED_LOGGING ((
					clog << "    Tiled_Image_Display::reset_tiles: "
							"cancel and queue tile image for rendering -"
							<< endl
						 << "    " << *tile_image << endl
						 << "    tile grid " << tile_grid
						 	<< ", display region " << (tile_region & viewport)
							<< endl));
					#endif
					Renderer->cancel (tile_image, Image_Renderer::DO_NOT_WAIT);

					/*	Move the tile-viewport intersection region origin
						to its tile-relative coordinate.
					*/
					displayed_tile_region = tile_relative_region
						(tile_region & viewport, tile_region.topLeft ());
					if (! displayed_tile_region.isEmpty ())
						changed = VISIBLE_TILES_RESET;

					//	Queue the tile image for rendering.
					Renderer->queue (tile_image,
						(displayed_tile_region.isEmpty () ?
							Image_Renderer::LOW_PRIORITY_RENDERING : tile_grid),
						 displayed_tile_region);
					}
				}
			#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
			else
				{
				LOCKED_LOGGING ((
				clog << "..... inactive column" << endl));
				}
			#endif
			if (++tile_grid.rx () == tile_cols)
				break;

			//	Move the tile locations to the next column.
			tile_region.translate (Tile_Display_Size.rwidth (), 0);

			band = 3;
			while (band--)
				tile_locations[band].rx () += tile_sizes[band].rwidth ();
			}
		}
	#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
	else
		{
		LOCKED_LOGGING ((
		clog << "... inactive row" << endl));
		}
	#endif
	if (++tile_grid.ry () == tile_rows)
		break;

	//	Move the tile locations to the beginning of the next row.
	tile_region.moveTo
		(tile_region_left_edge,
		 tile_region.y () + Tile_Display_Size.rheight ());

	band = 3;
	while (band--)
		{
		tile_locations[band].rx () = tile_locations_left_edge[band];
		tile_locations[band].ry () += tile_sizes[band].rheight ();
		}
	}

Done:
#if ((DEBUG_SECTION) & (DEBUG_RESET_TILES | DEBUG_IMAGE_GEOMETRY))
LOCK_LOG;
clog << "    Tiled_Image_Display::reset_tiles"  << endl
	 << "    in " << pathname << endl
	 << "    post-reset tiles -" << endl;
print_tile_grid ();

UNLOCK_LOG;
Renderer->print_render_queue ();
Renderer->print_delete_queue ();

LOCK_LOG;
Image_Tile::tile_accounting ();
Renderer->image_accounting ();

clog << "    in " << pathname << endl
	 << "<<< Tiled_Image_Display::reset_tiles: " << changed << endl;
UNLOCK_LOG;
#endif
return changed;
}


void
Tiled_Image_Display::clear_tiles ()
{
#if ((DEBUG_SECTION) & (DEBUG_TILE_GRID | DEBUG_CONSTRUCTORS))
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << ">>> Tiled_Image_Display::clear_tiles" << endl
	 << "    in " << pathname << endl
	 << "    tile grid size = " << Tile_Grid_Size << endl;
print_tile_grid ();
UNLOCK_LOG;
#endif
//	Cancel all rendering.
Renderer->reset (Image_Renderer::WAIT_UNTIL_DONE);
QList<Plastic_Image*>
	*tiles;
QPoint
	tile_grid (0, Tile_Grid_Size.rheight ());
int
	tile_cols = Tile_Grid_Size.rwidth ();
while (tile_grid.ry ()--)
	{
	if ((tiles = Tile_Grid_Images->takeAt (tile_grid.ry ())))
		{
		tile_grid.rx () = tile_cols;
		while (tile_grid.rx ()--)
			{
			#if ((DEBUG_SECTION) & (DEBUG_TILE_GRID | DEBUG_CONSTRUCTORS))
			LOCK_LOG;
			clog << "    removing tile grid " << tile_grid << ' ';
			if (tiles->at (tile_grid.rx ()))
				clog << *(tiles->at (tile_grid.rx ()));
			else
				clog << "NULL";
			clog << endl;
			UNLOCK_LOG;
			#endif
			Renderer->delete_image (tiles->takeLast ());
			}
		delete tiles;
		}
	}
Tiled_Image_Region.setWidth (0);
Tiled_Image_Region.setHeight (0);
Tile_Grid_Size.rwidth () =
Tile_Grid_Size.rheight () = 0;

//	Clear the tile image pool.
#if ((DEBUG_SECTION) & (DEBUG_TILE_GRID | DEBUG_CONSTRUCTORS))
LOCKED_LOGGING ((
clog << "    clearing " << Tile_Image_Pool.size ()
		<< " images from the the Tile_Image_Pool" << endl));
#endif
tile_cols = Tile_Image_Pool.size ();
while (tile_cols--)
	{
	#if ((DEBUG_SECTION) & (DEBUG_TILE_GRID | DEBUG_CONSTRUCTORS))
	LOCKED_LOGGING ((
	clog << "    " << tile_cols << ": " << *Tile_Image_Pool[tile_cols]
			<< endl));
	#endif
	Renderer->delete_image (Tile_Image_Pool.takeLast ());
	}
Tile_Image_Pool_Max = 0;

//	Force the images queued for deletion to be deleted.
Renderer->clean_up ();

#if ((DEBUG_SECTION) & (DEBUG_TILE_GRID | DEBUG_CONSTRUCTORS))
LOCK_LOG;
print_tile_grid ();
print_tile_pool ();
UNLOCK_LOG;
Renderer->print_render_queue ();
Renderer->print_delete_queue ();
LOCK_LOG;
Image_Tile::tile_accounting ();
Renderer->image_accounting ();
clog << "    in " << pathname << endl
	 << "<<< Tiled_Image_Display::clear_tiles" << endl;
UNLOCK_LOG;
#endif
}

/*==============================================================================
	Scaling
*/
bool
Tiled_Image_Display::scale_image
	(
	const QSizeF&	scale,
	const QPoint&	center,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_SCALING)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::scale_image:" << endl
	 << "    scaling = " << scale << endl
	 << "     center = " << center << endl
	 << "       band = " << band << endl));
#endif
if (Image_Loading ||
	Tile_Display_Size.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	LOCKED_LOGGING ((
	clog << "    " << (Image_Loading ?
			"Image_Loading in progress" : "no tiles") << endl
		 << "<<< Tiled_Image_Display::scale_image: false" << endl));
	#endif
	return false;
	}

bool
	changed = false;
QSizeF
	current_scaling (image_scaling (band)),
	scaling (scale);
if (scaling.rwidth () < Min_Scale)
	scaling.rwidth () = Min_Scale;
else
if (scaling.rwidth () > Max_Scale)
	scaling.rwidth () = Max_Scale;
if (scaling.rheight () < Min_Scale)
	scaling.rheight () = Min_Scale;
else
if (scaling.rheight () > Max_Scale)
	scaling.rheight () = Max_Scale;
#if ((DEBUG_SECTION) & DEBUG_SCALING)
LOCKED_LOGGING ((
clog << "    current scaling = " << current_scaling << endl
	 << "    limited scaling = " << scaling << endl));
#endif

if ((changed = (scaling != current_scaling)))
	{
	Pending_State_Change_Enabled = false;

	//	Cancel all rendering.
	Renderer->reset (Image_Renderer::DO_NOT_WAIT);

	//	Current image origin.
	QPointF
		old_origin (displayed_image_origin (band)),
		origin (old_origin);
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	QPoint
		display_center (map_image_to_display (center));
	LOCKED_LOGGING ((
	clog << "    center display point = " << display_center << endl));
	#endif

	//	Change the scaling of the reference image.
	Reference_Image->source_scaling (scaling, band);

	//	Reset the lower-right image display origin limit.
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	LOCKED_LOGGING ((
	clog << "      old Lower_Right_Origin_Limit = "
		<< Lower_Right_Origin_Limit << endl));
	#endif
	Lower_Right_Origin_Limit = calculate_lower_right_origin_limit ();
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	LOCKED_LOGGING ((
	clog << "      new Lower_Right_Origin_Limit = "
		<< Lower_Right_Origin_Limit << endl));
	#endif

	if (band <= 0)
		{
		//	Adjust the cached tile location information:

		//	Tile image size.
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "               old Tile_Image_Size = "
				<< Tile_Image_Size << endl));
		#endif
		reset_tile_image_size ();
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "               new Tile_Image_Size = "
				<< Tile_Image_Size << endl));
		#endif

		//	New image origin.
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "        old displayed_image_origin = " << origin << endl));
		#endif
		if (origin != center)
			{
			//	Offset of the old origin from the center (positive: right/down).
			origin = center - origin;
			//	Scaled offset.
			origin.rx () *= (current_scaling.rwidth ()  / scaling.rwidth ());
			origin.ry () *= (current_scaling.rheight () / scaling.rheight ());
			//	New origin location.
			origin = QPointF (center) - origin;
			}
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "                        new origin = " << origin << endl));
		#endif
		//	Clip the origin to within the display limits.
		if (origin.rx () < 0)
			origin.rx () = 0;
		if (origin.rx () > Lower_Right_Origin_Limit.rx ())
			origin.rx () = Lower_Right_Origin_Limit.rx ();
		if (origin.ry () < 0)
			origin.ry () = 0;
		if (origin.ry () > Lower_Right_Origin_Limit.ry ())
			origin.ry () = Lower_Right_Origin_Limit.ry ();
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "                    clipped origin = " << origin << endl));
		#endif
		}

	/*	Reset the tile grid origin.

		>>> CAUTION <<< This must be done before other operations that
		depend on the position of the tile grid origin.
	*/
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	QPointF
		tile_grid_origin (displayed_tile_grid_origin (band));
	LOCKED_LOGGING ((
	clog << "    old displayed_tile_grid_origin = "
			<< tile_grid_origin << endl));
	#endif
	QSizeF
		size (tile_image_size (band));
	QPointF
		grid_origin
			(static_cast<int>(origin.rx () / size.rwidth ())
				* size.rwidth (),
			 static_cast<int>(origin.ry () / size.rheight ())
			 	* size.rheight ());
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	LOCKED_LOGGING ((
	clog << "    new displayed_tile_grid_origin = "
			<< grid_origin << endl));
	#endif
	displayed_tile_grid_origin (grid_origin, band);

	bool
		image_region_resized = false;

	if (band <= 0)
		{
		/*	Adjust the cached tile location information:

			>>> CAUTION <<< The order of operations is important
			as there are various dependencies of later operations
			on former operations.
		*/
		//	Tiled image region.
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "            old Tiled_Image_Region = "
				<< Tiled_Image_Region << endl));
		#endif
		reset_tiled_image_region ();
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "            new Tiled_Image_Region = "
				<< Tiled_Image_Region << endl));
		#endif

		/*	Reset the display tile offset.

			>>> CAUTION <<< This must be done before other operations
			that depend on this value; e.g. reset_displayed_image_region
			depends on calculate_displayed_image_region which depends on
			map_display_to_image which depends on Tile_Display_Offset.
		*/
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "           old Tile_Display_Offset = "
				<< Tile_Display_Offset << endl));
		#endif
		Tile_Display_Offset = map_image_to_tile_offset (origin);
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "           new Tile_Display_Offset = "
				<< Tile_Display_Offset << endl));
		#endif

		//	Reset the displayed image region.
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "        old Displayed_Image_Region = "
				<< Displayed_Image_Region << endl));
		#endif
		image_region_resized = reset_displayed_image_region ();
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		LOCKED_LOGGING ((
		clog << "        new Displayed_Image_Region = "
				<< Displayed_Image_Region << endl));
		#endif
		#if ((DEBUG_SECTION) & DEBUG_SCALING)
		QPoint
			point;
		QPointF
			pointF;
		point = map_image_to_display (center);
		LOCK_LOG;
		clog << "    Tiled_Image_Display - map_image_to_display ("
				<< center << ") = " << point << endl;
		point = map_image_to_display (origin);
		clog << "    Tiled_Image_Display - map_image_to_display ("
			 	<< origin << ") = " << point << endl;
		pointF = map_display_to_image (QPoint ());
		clog << "    Tiled_Image_Display - map_display_to_image (0, 0) = "
			 	<< pointF << endl;
		UNLOCK_LOG;
		#endif
		}

	//	Reset the tile geometries.
	reset_tiles ();
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	display_center = map_image_to_display (center);
	LOCKED_LOGGING ((
	clog << "    center display point after reset_tiles = "
			<< display_center << endl));
	#endif

	//	Restart rendering.
	#if ((DEBUG_SECTION) & DEBUG_SCALING)
	LOCKED_LOGGING ((
	clog << "    start Renderer" << endl));
	#endif
	int
		change = IMAGE_SCALE_STATE;
	if (old_origin != origin)
		change |= IMAGE_MOVE_STATE;
	Pending_State_Change_Enabled = true;
	state_change_start (change);

	/*	Update the display.

		N.B.: Without this the display update will be too slow.
	*/
	update ();

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SCALING | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::scale_image: "
			"emit image_scaled" << endl
		 << "    scaling = " << scaling << endl
		 << "       band = " << endl));
	#endif
	emit image_scaled (scaling, band);

	if (old_origin != origin)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SCALING | DEBUG_SIGNALS))
		LOCKED_LOGGING ((
		clog << "^^^ Tiled_Image_Display::scale_image: "
				"emit image_moved" << endl
			 << "    origin = " << round_down (origin) << endl
			 << "      band = " << band << endl));
		#endif
		emit image_moved (round_down (origin), band);
		}

	if (image_region_resized)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SCALING | DEBUG_SIGNALS))
		LOCKED_LOGGING ((
		clog << "^^^ Tiled_Image_Display::scale_image: "
				"emit displayed_image_region_resized: "
				<< displayed_image_region_size () << endl));
		#endif
		emit displayed_image_region_resized (displayed_image_region_size ());
		}
	}
#if ((DEBUG_SECTION) & DEBUG_SCALING)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::scale_image: "
		<< boolalpha << changed << endl));
#endif
return changed;
}


QSize
Tiled_Image_Display::scaled_image_size
	(
	int		band
	) const
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::scaled_image_size:" << band << endl
	 << "    Source_Image @ " << (void*)Source_Image << endl));
#endif
if (band < 0)
	{
	QRect
		inclusive
			(round_down (displayed_tile_grid_origin (0)), scaled_image_size (0)),
		band_rect;
	for (band = 1;
		 band < 3;
		 band++)
		{
		band_rect.setTopLeft (round_down (displayed_tile_grid_origin (band)));
		band_rect.setSize (scaled_image_size (band));
		inclusive |= band_rect;
		}
	#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
	LOCKED_LOGGING ((
	clog << "<<< Tiled_Image_Display::scaled_image_size: "
			<< inclusive.size () << endl));
	#endif
	return inclusive.size ();
	}
else
	{
	QSize
		scaled_size (image_size ());
	QSizeF
		scaling (image_scaling (band));
	scaled_size.rwidth () =
		round_up (scaled_size.rwidth ()  * scaling.rwidth ());
	scaled_size.rheight () =
		round_up (scaled_size.rheight () * scaling.rheight ());
	#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
	LOCKED_LOGGING ((
	clog << "<<< Tiled_Image_Display::scaled_image_size: "
			<< scaled_size << endl));
	#endif
	return scaled_size;
	}
}


void
Tiled_Image_Display::min_scale
	(
	double	scale_factor
	)
{
if (scale_factor <= 0.0)
	{
	ostringstream
		message;
	message << ID << endl
			<< "Invalid minimum scale factor - " << scale_factor;
	throw invalid_argument (message.str ());
	}
Min_Scale = scale_factor;
}


void
Tiled_Image_Display::max_scale
	(
	double	scale_factor
	)
{
if (scale_factor < 1.0)
	{
	ostringstream
		message;
	message << ID << endl
			<< "Invalid maximum scale factor - " << scale_factor;
	throw invalid_argument (message.str ());
	}
Max_Scale = scale_factor;
}


double
Tiled_Image_Display::scale_to_size
	(
	const QSize&	source_size,
	const QSize&	destination_size
	)
{
double
	scale = 1.0;
QSize
	from_size (source_size),
	to_size (destination_size);
if (! destination_size.isEmpty () &&
	! from_size.isEmpty () &&
	(from_size.rwidth ()  > to_size.width () ||
	 from_size.rheight () > to_size.height ()))
	{
	scale = qMin (((double)to_size.rwidth ()  / from_size.rwidth ()),
				  ((double)to_size.rheight () / from_size.rheight ()));
	while (to_size.rwidth () > 1 &&
			(int)(scale * from_size.rwidth ()) > destination_size.width ())
		{
		--to_size.rwidth ();
		scale = (double)to_size.rwidth () / from_size.rwidth ();
		}
	while (to_size.rheight () > 1 &&
			(int)(scale * from_size.rheight ()) > destination_size.height ())
		{
		--to_size.rheight ();
		scale = (double)to_size.rheight () / from_size.rheight ();
		}
	}
#if ((DEBUG_SECTION) & DEBUG_UTILITIES)
clog << ">-< Tiled_Image_Display::scale_to_size:" << endl
	 << "         source size = " << source_size << endl
	 << "    destination size = " << destination_size << endl;
	 << "               scale = " << scale << endl
	 << "         scaled size = " << (source_size * scale) << endl;
#endif
return scale;
}

/*==============================================================================
	Band and Data Mappings
*/
bool
Tiled_Image_Display::map_bands
	(
	const unsigned int*		band_map
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
LOCK_LOG;
clog << ">>> Tiled_Image_Display::map_bands: ";
if (band_map)
	clog << band_map[0] << ", " << band_map[1] << ", " << band_map[2] << endl;
else
	clog << "NULL" << endl;
clog << "    in " << object_pathname (this) << endl
	 << "       tile grid size = " << Tile_Grid_Size << endl;
UNLOCK_LOG;
#endif
if (! band_map ||
	Tile_Grid_Size.isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
	LOCKED_LOGGING ((
	clog << "    no " << (band_map ? "tiles" : "band_map") << endl
		 << "<<< Tiled_Image_Display::map_bands: false" << endl));
	#endif
	return false;
	}

//	Test if the band map has changed.
bool
	changed = Reference_Image->different_band_map (band_map);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
LOCKED_LOGGING ((
clog << "    Reference_Image different_band_map = "
		<< boolalpha << changed << endl));
#endif
if (changed)
	{
	Pending_State_Change_Enabled = false;

	//	Cancel all rendering.
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    reset Renderer" << endl));
	#endif
	Renderer->reset ();

	//	Reset the band map in the Reference_Image.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
	LOCKED_LOGGING ((
	clog << "    set band map for Reference_Image @ "
			<< (void*)Reference_Image << endl));
	#endif
	Reference_Image->source_band_map (band_map);

	//	Apply the update to the tile images.
	image_update_needed (Plastic_Image::BAND_MAP);

	//	Test if the band map has changed in the Source_Image.
	if (Source_Image->different_band_map (band_map))
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
		LOCKED_LOGGING ((
		clog << "    set band map for Source_Image @ "
				<< (void*)Source_Image << endl));
		#endif
		//	Reset the band map in the Source_Image.
		Source_Image->source_band_map (band_map);

		if (Source_Image_Rendering)
			//	Queue the Source_Image for low priority uncancelable rendering.
			Renderer->queue (Source_Image,
				Image_Renderer::LOW_PRIORITY_RENDERING,
				! Image_Renderer::CANCELABLE);
		}

	//	Start rendering tiles.
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    start Renderer" << endl));
	#endif
	Pending_State_Change_Enabled = true;
	state_change_start (BAND_MAPPING_STATE);
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_bands: "
		<< boolalpha << changed << endl));
#endif
return changed;
}


bool
Tiled_Image_Display::map_data
	(
	Data_Map**	maps
	)
{
#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::map_data: @ " << (void*)maps << endl));
#endif
if (! maps)
	{
	maps = data_maps ();
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    using Reference_Image data_maps @ " << (void*)maps << endl));
	#endif
	}
#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
else
	{
	LOCKED_LOGGING ((
	clog << "    Reference_Image data_maps @ " << (void*)data_maps () << endl));
	}
#endif

/*	Detecting changes to the data maps.

	The Data_Maps that are modified are expected to be contained in the
	Reference_Image; a separate set may be modified, but operating on a
	copy seems to be an unnecessary expense. For this method to detect
	that the data maps have changed in this case they are compared
	against the Source_Image Data_Maps, which are only updated by this
	method. Note that the cost of the extra comparison - the
	source_data_maps setter does a comparison as the data is assigned -
	may be greater than the savings, and potential complications, of
	using the Reference_Image Data_Maps in place.
*/
#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
LOCKED_LOGGING ((
clog << "    Comparing Source_Image data maps with proffered maps" << endl));
#endif
bool
	changed = Source_Image->different_data_maps ((const Data_Map**)maps);
#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
LOCKED_LOGGING ((
clog << "    Source_Image different_data_maps = "
		<< boolalpha << changed << endl));
#endif
if (changed)
	{
	Pending_State_Change_Enabled = false;

	//	Cancel all rendering.
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    reset Renderer" << endl));
	#endif
	Renderer->reset ();

	//	Apply the data maps to the reference image.
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    apply data maps to the Reference_Image" << endl));
	#endif
	Reference_Image->source_data_maps ((const Data_Map**)maps);

	//	Apply the update to the tile images.
	image_update_needed (Plastic_Image::DATA_MAPS);

	//	Apply the data maps to the source image.
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    apply data maps to the Source_Image" << endl));
	#endif
	if (Source_Image->source_data_maps ((const Data_Map**)maps) &&
		Source_Image_Rendering)
		{
		#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
		LOCKED_LOGGING ((
		clog << "    queue the Source_Image for rendering" << endl));
		#endif
		//	Queue the Source_Image for low priority uncancelable rendering.
		Renderer->queue (Source_Image,
			Image_Renderer::LOW_PRIORITY_RENDERING,
			! Image_Renderer::CANCELABLE);
		}

	//	Start rendering tiles.
	#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
	LOCKED_LOGGING ((
	clog << "    start Renderer" << endl));
	#endif
	Pending_State_Change_Enabled = true;
	state_change_start (DATA_MAPPING_STATE);
	}
#if ((DEBUG_SECTION) & DEBUG_MAP_DATA)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::map_data: " << boolalpha << changed << endl));
#endif
return changed;
}


void
Tiled_Image_Display::image_update_needed
	(
	Mapping_Type	update_type
	)
{
#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::image_update_needed: " << update_type
		<< " - " << Plastic_Image::mapping_type_names (update_type) << endl
	 << "    in " << object_pathname (this) << endl));
#endif
int
	tile_rows = Tile_Grid_Size.rheight (),
	tile_cols = Tile_Grid_Size.rwidth ();
if (! tile_rows ||
	! tile_cols)
	{
	#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
	LOCKED_LOGGING ((
	clog << "    no tiles" << endl
		 << "<<< Tiled_Image_Display::image_update_needed" << endl));
	#endif
	return;
	}

//	Tile display geometry.
QPoint
	tile_grid;
QRect
	viewport (rect ()),
	//	Equivalent to tile_display_region for the first visible tile.
	tile_region (-Tile_Display_Offset, Tile_Display_Size),
	//	Region of the tile visible in the display.
	displayed_tile_region;
tile_region.translate
	(-Tile_Display_Size.width (), -Tile_Display_Size.height ());
int
	tile_region_left_edge = tile_region.x ();
#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
LOCKED_LOGGING ((
clog << "    Tile_Display_Size = " << Tile_Display_Size << endl
	 << "             viewport = " << rect () << endl));
#endif
QList<Plastic_Image*>
	*tiles;
Plastic_Image
	*image;
while (true)
	{
	#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
	LOCKED_LOGGING ((
	clog << "    tile row " << tile_grid.ry () << endl));
	#endif
	if ((tiles = Tile_Grid_Images->at (tile_grid.ry ())))
		{
		tile_grid.rx () = 0;
		while (true)
			{
			#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
			LOCKED_LOGGING ((
			clog << "      tile column " << tile_grid.rx () << endl));
			#endif
			if ((image = tiles->at (tile_grid.rx ())))
				{
				image->needs_update (update_type);

				/*	Move the tile-viewport intersection region origin
					to its tile-relative coordinate.
				*/
				displayed_tile_region = tile_relative_region
					(tile_region & viewport, tile_region.topLeft ());
				#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
				LOCKED_LOGGING ((
				clog << "                  tile_region = "
					 	<< tile_region << endl
					 << "        displayed_tile_region = "
					 	<< displayed_tile_region << endl
					 << "        queue for rendering" << endl));
				#endif

				//	Queue the tile image for rendering.
				Renderer->queue (image,
					(displayed_tile_region.isEmpty () ?
						Image_Renderer::LOW_PRIORITY_RENDERING : tile_grid),
					 displayed_tile_region);
				}
			#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
			else
				{
				LOCKED_LOGGING ((
				clog << "      inactive column" << endl));
				}
			#endif
			if (++tile_grid.rx () == tile_cols)
				break;

			//	Move the tile region to the next column.
			tile_region.translate (Tile_Display_Size.rwidth (), 0);
			}
		}
	#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
	else
		{
		LOCKED_LOGGING ((
		clog << "    inactive row" << endl));
		}
	#endif
	if (++tile_grid.ry () == tile_rows)
		break;

	//	Move the tile region to the beginning of the next row.
	tile_region.moveTo
		(tile_region_left_edge,
		 tile_region.y () + Tile_Display_Size.rheight ());
	}
#if ((DEBUG_SECTION) & (DEBUG_MAP_BANDS | DEBUG_MAP_DATA))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::image_update_needed" << endl));
#endif
}

/*------------------------------------------------------------------------------
	State
*/
int
Tiled_Image_Display::rendering_status () const
{return Renderer->rendering_status ();}


void
Tiled_Image_Display::state_change_start
	(
	int		state
	)
{
#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::state_change_start: " << state
		<< " - " << state_change_description (state) << endl
	 << "    in " << object_pathname (this) << endl
	 << "    Pending_State_Change = " << Pending_State_Change << " - "
	 	<< state_change_description (Pending_State_Change) << endl));
#endif
int
	state_type = state & STATE_TYPE_MASK;
if (state_type)
	{
	//	Pending state is changing.
	Pending_State_Change |= state_type;
	state |= Pending_State_Change;

	if (state & IMAGE_LOAD_STATE)
		state |= RENDERING_VISIBLE_TILES_STATE;
	else
		{
		int
			status = rendering_status ();
		#if ((DEBUG_SECTION) & DEBUG_STATE)
		LOCKED_LOGGING ((
		clog << "    rendering_status = " << status << " - "
				<< rendering_status_description (status) << endl));
		#endif
		if (status)
			state |= ((status == RENDERING_VISIBLE_TILES) ?
					RENDERING_VISIBLE_TILES_STATE :
					RENDERING_BACKGROUND_STATE);
		}

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SIGNALS | DEBUG_STATE))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::state_change_start: "
			"emit state_change: " << state << " - "
			<< state_change_description (state) << endl
		 << "    in " << object_pathname (this) << endl));
	#endif
	emit state_change (state);
	}

#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << "    Renderer::start_rendering ..." << endl));
#endif
Renderer->start_rendering ();

#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::state_change_start" << endl));
#endif
}


void
Tiled_Image_Display::state_change_completed
	(
	int		qualifier
	)
{
/*
	Only signal state change completed if all rendering has completed.
	The rendered method will signal when visible tile rendering
	completes.
*/
int
	status = rendering_status ();
#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::state_change_completed: "
		<< qualifier << " - "
		<< state_change_description (qualifier) << endl
	 << "    in " << object_pathname (this) << endl
	 << "    Pending_State_Change = " << Pending_State_Change << " - "
	 	<< state_change_description (Pending_State_Change) << endl
	 << "    rendering_status = " << status << " - "
	 	<< rendering_status_description (status) << endl));
#endif
if (! status &&
	Pending_State_Change)
	{
	qualifier |= Pending_State_Change;
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::state_change_completed: "
			"emit state_change: " << qualifier << " - "
			<< state_change_description (qualifier) << endl
		 << "    in " << object_pathname (this) << endl));
	#endif
	emit state_change (qualifier);
	Pending_State_Change = NO_STATE_CHANGE;
	}
#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::state_change_completed" << endl));
#endif
}


QString
Tiled_Image_Display::state_change_description
	(
	int		state
	)
{
QString
	names;
if (state)
	{
	//	State change types:
	bool
		has_type = false;

	if (state & IMAGE_LOAD_STATE)
		{
		names = "Loading";
		has_type = true;
		}
	if (state & DISPLAY_SIZE_STATE)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "Display Resizing";
		has_type = true;
		}
	if (state & TILE_SIZE_STATE)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "Tiles Resizing";
		has_type = true;
		}
	if (state & IMAGE_MOVE_STATE)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "Moving";
		has_type = true;
		}
	if (state & IMAGE_SCALE_STATE)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "Scaling";
		has_type = true;
		}
	if (state & BAND_MAPPING_STATE)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "Band Mapping";
		has_type = true;
		}
	if (state & DATA_MAPPING_STATE)
		{
		if (! names.isEmpty ())
			names += ", ";
		names += "Data Mapping";
		has_type = true;
		}

	//	State change qualifiers:
	bool
		has_qualifier = false;

	if (state & RENDERING_VISIBLE_TILES_STATE)
		{
		if (has_type)
			names += ';';
		if (! names.isEmpty ())
			names += ' ';
		names += "Rendering Visible Tiles";
		has_type = false;
		has_qualifier = true;
		}
	if (state & RENDERING_BACKGROUND_STATE)
		{
		if (has_type)
			names += ';';
		else
		if (has_qualifier)
			names += ',';
		if (! names.isEmpty ())
			names += ' ';
		names += "Rendering Background";
		has_type = false;
		has_qualifier = true;
		}
	if (state & RENDERING_CANCELED_STATE)
		{
		if (has_type)
			names += ';';
		else
		if (has_qualifier)
			names += ',';
		if (! names.isEmpty ())
			names += ' ';
		names += "Canceled";
		has_type = false;
		has_qualifier = true;
		}
	if (state & RENDERING_COMPLETED_STATE)
		{
		if (has_type)
			names += ';';
		else
		if (has_qualifier)
			names += ',';
		if (! names.isEmpty ())
			names += ' ';
		names += "Completed";
		has_type = false;
		has_qualifier = true;
		}
	if (state & COMPLETED_WITHOUT_RENDERING_STATE)
		{
		if (has_type)
			names += ';';
		else
		if (has_qualifier)
			names += ',';
		if (! names.isEmpty ())
			names += ' ';
		names += "Completed w/o Rendering";
		has_type = false;
		has_qualifier = true;
		}
	}
else
	names = "No State Change";
return names;
}


void
Tiled_Image_Display::renderer_status
	(
	int		status
	)
{
#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::renderer_status: " << status << ' '
		<< Image_Renderer::status_description (status) << endl
	 << "    in " << object_pathname (this) << endl));
#endif
//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SIGNALS))
LOCKED_LOGGING ((
clog << "^^^ Tiled_Image_Display::renderer_status: "
		"emit rendering_status " << status << " - "
		<< rendering_status_description (status) << endl));
#endif
emit rendering_status (status);

if (status == NOT_RENDERING ||
	(status & Image_Renderer::RENDERING_CANCELED))
	{
	Renderer->clean_up ();

	#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "    Pending_State_Change_Enabled = "
			<< Pending_State_Change_Enabled << endl
		 << "    Pending_State_Change = " << Pending_State_Change << " - "
		 	<< state_change_description (Pending_State_Change) << endl));
	#endif
	if (Pending_State_Change_Enabled &&
		/*
			>>> CAUTION <<<
			Image load completion state signalling is a special case
			that is done in the loaded method.
		*/
		! (Pending_State_Change & IMAGE_LOAD_STATE))
		state_change_completed ((status == NOT_RENDERING) ?
			RENDERING_COMPLETED_STATE :
			RENDERING_CANCELED_STATE);
	}
#if ((DEBUG_SECTION) & DEBUG_STATE)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::renderer_status" << endl));
#endif
}


QString
Tiled_Image_Display::rendering_status_description
	(
	int		status
	)
{
QString
	description;
switch (status & ~RENDERING_CANCELED)
	{
	case NOT_RENDERING:
		description = tr ("Not Rendering"); break;
	case RENDERING_BACKGROUND:
		description = tr ("Rendering Background"); break;
	case RENDERING_VISIBLE_TILES:
		description = tr ("Rendering Visible Tiles"); break;
	case LOADING_IMAGE:
		description = tr ("Loading Image"); break;
	default:
		description = tr ("Unknown Status %1").arg (status);
	}
if (status & RENDERING_CANCELED)
	description += tr (" Canceled");
return description;
}


void
Tiled_Image_Display::cancel_rendering ()
{
#if ((DEBUG_SECTION) & (DEBUG_SIGNALS | DEBUG_STATE))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::cancel_rendering" << endl));
#endif
//	Cancel all rendering.
#if ((DEBUG_SECTION) & (DEBUG_SIGNALS | DEBUG_STATE))
LOCKED_LOGGING ((
clog << "    reset Renderer without waiting" << endl));
#endif
Renderer->reset
	(Image_Renderer::WAIT_UNTIL_DONE |
	 Image_Renderer::FORCE_CANCEL);

//	Signal rendering canceled status (in case nothing was being rendered).
renderer_status (RENDERING_CANCELED);

if (Pending_State_Change & IMAGE_LOAD_STATE)
	{
	/*
		If an IMAGE_LOAD_STATE is pending the renderer_status would not
		call state_change_completed, and state_change_completed will not
		emit the state_change signal if any rendering is in progress. So
		this bypasses all that and does the state_change_completed.
	*/
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_STATE | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::cancel_rendering: "
			"emit state_change: "
			<< (Pending_State_Change | RENDERING_CANCELED_STATE) << " - "
			<< state_change_description
				(Pending_State_Change | RENDERING_CANCELED_STATE) << endl
		 << "    in " << object_pathname (this) << endl));
	#endif
	emit state_change (Pending_State_Change | RENDERING_CANCELED_STATE);
	Pending_State_Change = NO_STATE_CHANGE;
	}
}


void
Tiled_Image_Display::rendering_error
	(
	const QString&	message
	)
{
QString
	report (message);
if (Error_Message)
	Error_Message->showMessage (report.replace ("\n", "<br>"));
}

/*==============================================================================
	Event Handlers
*/
void
Tiled_Image_Display::rendered
	(
	const QPoint&	tile_coordinate,
	const QRect&	tile_region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
				DEBUG_RENDERED | \
				DEBUG_PAINT | \
				DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::rendered:" << endl
	 << "    tile_coordinate = " << tile_coordinate << endl
	 << "        tile_region = " << tile_region << endl));
#endif
if (tile_coordinate.isNull ())
	{
	//	Visible tile image rendering completed. Update the entire viewport.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_RENDERED | \
					DEBUG_PAINT))
	LOCKED_LOGGING ((
	clog << "    update ..." << endl));
	#endif
	update ();

	if (Pending_State_Change)
		{
		//	>>> SIGNAL <<<
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_RENDERED | \
						DEBUG_PAINT | \
						DEBUG_STATE | \
						DEBUG_SIGNALS))
		LOCKED_LOGGING ((
		clog << "^^^ Tiled_Image_Display::rendered: emit state_change: "
				<< (Pending_State_Change |
					RENDERING_VISIBLE_TILES_COMPLETED_STATE) << " - "
				<< state_change_description
				 	(Pending_State_Change |
					RENDERING_VISIBLE_TILES_COMPLETED_STATE) << endl
			 << "    in " << object_pathname (this) << endl));
		#endif
		emit state_change
			(Pending_State_Change |
			/*
				Transition state: Visible tile rendering has completed
				and background rendering may be starting. Regardless of
				whether any background tiles are to be rendered, when
				rendering stops a final completed status will be signalled.
			*/
			RENDERING_VISIBLE_TILES_COMPLETED_STATE);
		}
	}
else
	{
	//	Incremental tile image rendering. Single tile region repaint.
	QRect
		display_region (tile_region);
	if (display_region.isEmpty ())
		display_region = tile_display_region (tile_coordinate);
	else
		//	Convert tile_region to viewport relative display_region.
		viewport_relative_region (tile_coordinate, display_region);
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_RENDERED | \
					DEBUG_PAINT | \
					DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "     display region = " << display_region << endl));
	#endif
	display_region &= rect ();	//	Only the portion within the viewport.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
					DEBUG_RENDERED | \
					DEBUG_PAINT | \
					DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "           viewport = " << rect () << endl
		 << "     clipped region = " << display_region << endl));
	#endif
	if (! display_region.isEmpty ())
		{
		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
						DEBUG_RENDERED | \
						DEBUG_PAINT | \
						DEBUG_LOCATION))
		clog << "    repaint single tile ..." << endl;
		#endif
		repaint (display_region);
		//qApp->sendPostedEvents ();
		#if ((DEBUG_SECTION) & DEBUG_PROMPT)
		char
			input[4];
		clog << "Repainted " << display_region << " > ";
		cin.getline (input, 2);
		if (input[0] == 'q')
			exit (7);
		#endif
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | \
				DEBUG_RENDERED | \
				DEBUG_PAINT | \
				DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::rendered" << endl));
#endif
}


void
Tiled_Image_Display::paintEvent
	(
	QPaintEvent*	event
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | \
				DEBUG_PAINT | \
				DEBUG_PAINT_ONE | \
				DEBUG_LOCATION))
QString
	pathname (object_pathname (this));
QSizeF
	scaled_size (scaled_image_size ());
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::paintEvent: " << event->rect () << endl
	 << "    in " << pathname << endl
	 << "              display size = " << size () << endl
	 << "         Tile_Display_Size = " << Tile_Display_Size << endl
	 << "         scaled image size = " << scaled_size << endl
	 << "            Tile_Grid_Size = " << Tile_Grid_Size << endl
	 << "    displayed_image_region = " << displayed_image_region () << endl));
#endif
QRect
	paint_region (event->rect ());
QPainter
	painter (this);
#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
painter.setPen (QPen (TILE_MARKINGS_PAINT_COLOR));
QString
	label;
#endif

if (Tile_Display_Size.isEmpty () ||
	Tile_Grid_Size.isEmpty ())
	{
	painter.eraseRect (rect ());
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | \
					DEBUG_PAINT | \
					DEBUG_PAINT_ONE | \
					DEBUG_LOCATION))
	LOCKED_LOGGING ((
	clog << "    no tiles to paint" << endl
		 << "<<< Tiled_Image_Display::paintEvent" << endl));
	#endif
	return;
	}

QPoint
	tile_grid (map_display_to_tile (paint_region.topLeft ())),
	//	N.B.: bottomRight is (left + width - 1, top + height - 1)
	tile_grid_limit (map_display_to_tile (paint_region.bottomRight ())),
	painted_limit (-1, -1);

/*	Note: The paint_region should always be within the positive tile grid range.
	This is just a safety precaution that will fall back to selecting the
	entire visible tile grid.
*/
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
LOCKED_LOGGING ((
clog << "        grid coverage = " << tile_grid
		<< " to " << tile_grid_limit << " inclusive" << endl));
#endif
if (tile_grid.rx () < 1)
	tile_grid.rx () = 1;
if (tile_grid.ry () < 1)
	tile_grid.ry () = 1;

QRect
	//	Display region of the first visible tile.
	tile_region (tile_display_region (tile_grid)),
	//	Region of the tile visible in the display.
	displayed_tile_region;

QList<Plastic_Image*>
	*tiles = NULL;
Plastic_Image
	*tile_image = NULL;

if (tile_grid == tile_grid_limit &&		//	Only one tile.
	paint_region != rect ())			//	Not the entire viewport.
	{
	//	Single tile update.
	if ((tiles = Tile_Grid_Images->at (tile_grid.ry ())) &&
		(tile_image = tiles->at (tile_grid.rx ())))
		{
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | \
						DEBUG_PAINT | \
						DEBUG_PAINT_ONE | \
						DEBUG_LOCATION))
		LOCKED_LOGGING ((
		clog << "======> tile " << tile_grid << ' ' << *tile_image << endl
			 << "------> painting single tile " << tile_grid << endl
			 << "        paint region = " << paint_region << endl
			 << "         tile region = " << tile_region << endl));
		#endif
		//	Extant tile image containing the display region.
		painter.drawImage (tile_region, *tile_image);

		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT | DEBUG_PAINT_ONE))
		#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
		LOCK_LOG;
		display_image_data (*tile_image);
		clog << "<-- image data for tile " << tile_grid
				<< " -------------------------------------------------" << endl;
		UNLOCK_LOG;
		#endif
		#endif
		#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
		//	Tile markings; incremental region rendering.
		painter.drawLine
			(paint_region.topLeft (),
			 paint_region.bottomRight ());
		painter.drawLine
			(paint_region.bottomLeft (),
			 paint_region.topRight ());
		ostringstream
			report;
		report << " incr. paint @ " << (void*)tile_image
				<< ": gc " << tile_grid
				<< "; tr " << tile_region
				<< "; pr " << paint_region;
		QRect
			rect (paint_region.x (), paint_region.y (),
				  paint_region.width (), 12);
		painter.drawText
			(rect, Qt::AlignLeft | Qt::AlignVCenter,
			QString::fromStdString (report.str ()));
		#endif

		//	Flush painting to the display.
		//qApp->flush ();
		}
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT | DEBUG_PAINT_ONE))
	else
		{
		LOCKED_LOGGING ((
		clog << "    no tile to paint" << endl
			 << "    tiles @ " << (void*)tiles
				<< ", tile_image @ " << (void*)tile_image << endl
			 << "    tile_region (" << tile_region
			 	<< ") contains paint_region (" << paint_region << ") = "
				<< boolalpha << tile_region.contains (paint_region) << endl));
		}
	LOCKED_LOGGING ((
	clog << "    in " << pathname << endl
		 << "<<< Tiled_Image_Display::paintEvent" << endl));
	#endif
	return;
	}

//	Source image region used for background painting.
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
LOCKED_LOGGING ((
clog << "    source " << *Source_Image << endl));
#endif
if (Source_Image_Rendering &&
	Source_Image->needs_update () &&
	! Renderer->is_queued (Source_Image))
	{
	//	The Source image has not yet been rendered.
	#if ((DEBUG_SECTION) & (DEBUG_PAINT | DEBUG_LOAD_IMAGE))
	LOCKED_LOGGING ((
	clog << "    Tiled_Image_Display::paintEvent: "
			"queue new Source_Image for rendering" << endl));
	#endif
	Renderer->queue (Source_Image,
		Image_Renderer::LOW_PRIORITY_RENDERING,
		! Image_Renderer::CANCELABLE);
	}

QSizeF
	source_size (tile_image_size ()),
	source_scaling (Source_Image->source_scaling ());
source_size.rwidth ()  *= source_scaling.rwidth ();
source_size.rheight () *= source_scaling.rheight ();
QRectF
	source_region
		(map_tile_to_image (tile_grid) *= source_scaling.rwidth (),
		 source_size);
source_region.translate (1, 1);

double
	source_x_origin = source_region.x ();
//	Source image size.
source_size = Source_Image->source_size ();
source_size *= source_scaling.rwidth ();

//	Background fill color.
QColor
	background_fill_color
		(
		#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
		TILE_MARKINGS_BACKGROUND_COLOR
		#else
		Plastic_Image::default_background_color ()
		#endif
		);
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
clog << "    background_fill_color = 0x" << hex << setfill ('0') << setw (8)
		<< background_fill_color.rgba () << setfill (' ') << dec << endl;
#endif

if (++tile_grid_limit.rx () == 0)
	  tile_grid_limit.rx () = Tile_Grid_Size.rwidth ();
if (++tile_grid_limit.ry () == 0)
	  tile_grid_limit.ry () = Tile_Grid_Size.rheight ();
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT | DEBUG_PAINT_ONE))
LOCKED_LOGGING ((
clog << "++> multi-tile coverage = " << tile_grid
		<< " to " << tile_grid_limit << " exclusive" << endl));
#endif
int
	tile_col_start = tile_grid.rx (),
	x_origin = tile_region.x ();

while (true)
	{
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
	LOCKED_LOGGING ((
	clog << "    row " << tile_grid.ry () << endl));
	#endif
	if ((tiles = Tile_Grid_Images->at (tile_grid.ry ())))
		{
		tile_grid.rx () = tile_col_start;
		while (true)
			{
			#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
			LOCKED_LOGGING ((
			clog << "      col " << tile_grid.rx ()
					<< " - tile region " << tile_region << endl));
			#endif
			if ((tile_image = tiles->at (tile_grid.rx ())))
				{
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
				LOCKED_LOGGING ((
				clog << "======> tile " << tile_grid << ' '
						<< *tile_image << endl));
				#endif
				if (tile_image->needs_update ())
					{
					//	Re-queue the tile for rendering.
					displayed_tile_region =
						tile_relative_region
							(tile_region & rect (), tile_region.topLeft ());
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
					LOCKED_LOGGING ((
					clog << "        queue tile image for rendering -"
							<< endl
						 << *tile_image << endl
						 << "    tile grid " << tile_grid
						 	<< ", tile region " << displayed_tile_region
							<< endl));
					#endif
					Renderer->queue
						(tile_image, tile_grid, displayed_tile_region);

					//	Paint the source image region on the display.
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
					LOCKED_LOGGING ((
					clog << "------> source region = "
							<< round_down (source_region)
							<< " (" << source_region << ')' << endl));
					#endif
					#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
					label = QString (" fill: ");
					#endif
					if (Source_Image->needs_update () ||
						source_region.right ()  >= source_size.rwidth () ||
						source_region.bottom () >= source_size.rheight ())
						{
						#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
						LOCKED_LOGGING ((
						clog << "------> background fill" << endl));
						#endif
						//	Fill the tile region with background.
						painter.fillRect (tile_region, background_fill_color);
						#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
						label = QString (" background: ");
						#endif
						}
					if (! Source_Image->needs_update ())
						{
						//	Paint the tile region from the source image.
						#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
						LOCKED_LOGGING ((
						clog << "------> source image fill" << endl));
						#endif
						painter.drawImage (tile_region, *Source_Image,
							round_down (source_region));
						#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
						label = QString (" source fill: ");
						#endif
						}
					}
				else
					{
					//	Paint the tile image region on the display viewport.
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT | DEBUG_LOCATION))
					LOCK_LOG;
					clog << "------> painting tile " << tile_grid << endl
						 << "        paint region = " << paint_region << endl
						 << "         tile region = " << tile_region << endl;
					#if ((DEBUG_SECTION) & DEBUG_PIXEL_DATA)
					display_image_data (*tile_image);
					clog << "<------ image data for tile " << tile_grid
							<< " -------------------------------------------------" << endl;
					#endif
					UNLOCK_LOG;
					#endif
					#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
					label = QString (" paint @ %1: ")
						.arg ((ulong)tile_image, 0, 16);
					#endif
					painter.drawImage (tile_region, *tile_image);
					}
				#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
				//	Tile markings.
				painter.drawLine
					(tile_region.topLeft (),
					 tile_region.bottomRight ());
				painter.drawLine
					(tile_region.bottomLeft (),
					 tile_region.topRight ());
				label +=
					QString ("gc %1, %2; so %3, %4; to %5, %6")
					.arg (tile_grid.rx ())
					.arg (tile_grid.ry ())
					.arg (tile_image->source_origin ().x ())
					.arg (tile_image->source_origin ().y ())
					.arg (tile_region.x ())
					.arg (tile_region.y ());
				QRect
					rect (tile_region.x (),
						  tile_region.y () + TILE_MARKINGS_PAINT_Y,
						  tile_region.width (), 12);
				painter.fillRect (rect, Qt::white);
				painter.drawText
					(rect, Qt::AlignLeft | Qt::AlignVCenter, label);
				if (tile_region.height ()
						> (TILE_MARKINGS_PAINT_Y << 1))
					{
					rect.moveTop
						(tile_region.bottom () - TILE_MARKINGS_PAINT_Y);
					painter.fillRect (rect, Qt::white);
					painter.drawText
						(rect, Qt::AlignLeft | Qt::AlignVCenter, label);
					}
				painter.drawRect (tile_region);
				#endif

				if (painted_limit.rx () < tile_region.right ())
					painted_limit.rx () = tile_region.right ();
				}
			#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
			else
				{
				LOCKED_LOGGING ((
				clog << "        empty" << endl));
				}
			#endif

			if (++tile_grid.rx () == tile_grid_limit.rx ())
				break;
			//	Move to the next tile column.
			tile_region.translate (Tile_Display_Size.rwidth (), 0);
			source_region.translate (source_region.width (), 0);
			}
		painted_limit.ry () = tile_region.bottom ();
		}
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
	else
		{
		LOCKED_LOGGING ((
		clog << "      empty" << endl));
		}
	#endif

	//	Move to the beginning of the next tile row.
	tile_region.moveTo
		(x_origin, tile_region.y () + Tile_Display_Size.rheight ());
	if (++tile_grid.ry () == tile_grid_limit.ry ())
		break;
	source_region.moveTo
		(source_x_origin, source_region.y () + source_region.height ());
	}

//	Paint any unpainted update area with background.
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
LOCKED_LOGGING ((
clog << "       painted region bottom = "
		<< painted_limit.ry () << endl
	 << "         paint region bottom = "
		<< paint_region.bottom () << endl));
#endif
if (painted_limit.ry () < paint_region.bottom ())
	{
	tile_region.setTop (painted_limit.ry () + 1);
	tile_region.setLeft (event->rect().left ());
	tile_region.setBottom (paint_region.bottom ());
	tile_region.setRight (paint_region.right ());
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
	LOCKED_LOGGING ((
	clog << "    bottom background region = " << tile_region << endl));
	#endif
	painter.fillRect (tile_region, background_fill_color);
	}
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
LOCKED_LOGGING ((
clog << "        painted region right = "
		<< painted_limit.rx () << endl
	 << "          paint region right = "
		<< paint_region.right () << endl));
#endif
if (painted_limit.rx () < paint_region.right ())
	{
	tile_region.setTop (event->rect().top ());
	tile_region.setLeft (painted_limit.rx () + 1);
	tile_region.setBottom (painted_limit.ry ());
	tile_region.setRight (paint_region.right ());
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_PAINT))
	LOCKED_LOGGING ((
	clog << "      side background region = " << tile_region << endl));
	#endif
	painter.fillRect (tile_region, background_fill_color);
	}
#if ((DEBUG_SECTION) & DEBUG_PROMPT)
char
	input[4];
clog << "Painted " << paint_region << " > ";
cin.getline (input, 2);
if (input[0] == 'q')
	exit (7);
#endif
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | \
				DEBUG_PAINT | \
				DEBUG_PAINT_ONE | \
				DEBUG_LOCATION))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Tiled_Image_Display::paintEvent" << endl));
#endif
}


void
Tiled_Image_Display::mouseMoveEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOUSE_MOVE_EVENTS)
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::mouseMoveEvent: " << endl
	 << "     global position = " << event->globalPos () << endl
	 << "    display position = " << event->pos () << endl));
#endif
if (rect ().contains (event->pos ()))
	{

	QPoint
		point (round_down (map_display_to_image (event->pos ())));
	#if ((DEBUG_SECTION) & DEBUG_MOUSE_MOVE_EVENTS)
	LOCKED_LOGGING ((
	clog << "      image position = " << point << endl
		 << "    Displayed_Image_Region = " << Displayed_Image_Region << endl));
	#endif
	if (! Displayed_Image_Region.contains (point))
		point.rx () =
		point.ry () = -1;

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_MOUSE_MOVE_EVENTS | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::mouseMoveEvent: "
			"emit image_cursor_moved" << endl
		 << "    display position = " << event->pos () << endl
		 << "      image position = " << point << endl));
	#endif
	emit image_cursor_moved (event->pos (), point);

	QWidget::mouseMoveEvent (event);
	}
#if ((DEBUG_SECTION) & DEBUG_MOUSE_MOVE_EVENTS)
LOCKED_LOGGING ((
clog << "<<< Tiled_Image_Display::mouseMoveEvent" << endl));
#endif

}

    void Tiled_Image_Display::mousePressEvent (QMouseEvent* event) {
        if(event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton) {
            QPoint image_position(round_down(map_display_to_image(event->pos())));
                if(Displayed_Image_Region.contains(image_position)) {
                    Last_Clicked_Coord = image_position;
                }
        }
        QWidget::mousePressEvent(event);
    }


void
Tiled_Image_Display::resizeEvent
	(
	QResizeEvent*
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_REGION | DEBUG_OVERVIEW))
		event
	#endif
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_REGION | DEBUG_OVERVIEW))
QString
	pathname (object_pathname (this));
QRectF
	display_region (displayed_image_region ());
LOCKED_LOGGING ((
clog << ">>> Tiled_Image_Display::resizeEvent:" << endl
	 << "    in " << pathname << endl
	 << "    from " << event->oldSize () << endl
	 << "      to " << event->size () << endl
	 << "    displayed_image_region = " << display_region << endl));
#endif
Pending_State_Change_Enabled = false;
int
	tiles_reset = NO_TILES_RESET;
if (resize_tile_grid ())
	tiles_reset = reset_tiles ();

//	>>> SIGNAL <<<
#if ((DEBUG_SECTION) & (DEBUG_SIGNALS | DEBUG_EVENTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "^^^ Tiled_Image_Display::resizeEvent: "
		"emit display_viewport_resized " << size () << endl));
#endif
emit display_viewport_resized (size ());

//	Reset the displayed image region.
if (reset_displayed_image_region ())
	{
	//	Reset the lower-right image display origin limit.
	Lower_Right_Origin_Limit = calculate_lower_right_origin_limit ();

	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_SIGNALS | \
					DEBUG_EVENTS | \
					DEBUG_REGION | \
					DEBUG_STATE))
	LOCKED_LOGGING ((
	clog << "^^^ Tiled_Image_Display::displayed_image_region: "
			"emit displayed_image_region_resized: "
			<< displayed_image_region_size () << endl));
	#endif
	emit displayed_image_region_resized (displayed_image_region_size ());
	}

//	Restart rendering.
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_REGION))
LOCKED_LOGGING ((
clog << "    start Renderer" << endl));
#endif
Pending_State_Change_Enabled = true;
state_change_start (DISPLAY_SIZE_STATE);

if (! tiles_reset)
	//	Report the completion of the state change.
	state_change_completed (COMPLETED_WITHOUT_RENDERING_STATE);

//	Note: A display update is done automatically for a resizeEvent.
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_REGION | DEBUG_OVERVIEW))
display_region = displayed_image_region ();
LOCKED_LOGGING ((
clog << "    displayed_image_region = " << display_region << endl
	 << "    in " << pathname << endl
	 << "<<< Tiled_Image_Display::resizeEvent" << endl));
#endif
}

    const QPoint Tiled_Image_Display::Get_Saved_Coordinate() {
        return Last_Clicked_Coord;
    }

void
Tiled_Image_Display::leaveEvent
	(
	QEvent*
	)
{emit image_cursor_moved (QPoint (-1, -1), QPoint (-1, -1));}


QSize
Tiled_Image_Display::sizeHint () const
{return const_cast<Tiled_Image_Display*>(this)->scaled_image_size ();}


}	//	namespace UA::HiRISE
