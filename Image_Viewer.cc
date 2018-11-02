/*	Image_Viewer

HiROC CVS ID: $Id: Image_Viewer.cc,v 1.137 2014/05/27 17:32:25 guym Exp $

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

#include	"Image_Viewer.hh"

#include	"HiView_Config.hh"
#include	"Plastic_Image.hh"
#include	"Plastic_QImage.hh"
#include	"JP2_Image.hh"
#include	"Tiled_Image_Display.hh"
#include	"HiView_Utilities.hh"
#include    "Projection.hh"
#include    "Coordinate.hh"

//	UA::HiRISE::JP2_Reader.
#include	"JP2.hh"
using UA::HiRISE::JP2;
#include	"JP2_Reader.hh"
using UA::HiRISE::JP2_Reader;
#include	"JP2_Exception.hh"
using UA::HiRISE::JP2_Exception;

#include	<QApplication>
#include	<QDesktopWidget>
#include	<QScrollBar>
#include	<QSlider>
#include	<QLabel>
#include	<QFont>
#include	<QFile>
#include	<QResizeEvent>
#include	<QAction>
#include	<QMenu>
#include	<QContextMenuEvent>
#include	<QErrorMessage>
#include	<QCursor>
#include	<QBitmap>
#include    <QClipboard>

#include	<algorithm>
using std::min;
using std::max;
#include	<cmath>
#include	<sstream>
using std::ostringstream;
#include	<stdexcept>
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
#define DEBUG_LAYOUT			(1 << 2)
#define DEBUG_SCALE				(1 << 3)
#define DEBUG_SLOTS				(1 << 4)
#define DEBUG_SIGNALS			(1 << 5)
#define DEBUG_MENUS				(1 << 6)
#define DEBUG_SCROLLBARS		(1 << 7)
#define DEBUG_EVENTS			(1 << 8)
#define DEBUG_MOUSE_EVENTS		(1 << 9)
#define DEBUG_KEY_EVENTS		(1 << 10)
#define DEBUG_MOVE				(1 << 11)
#define DEBUG_SIZE_HINT			(1 << 12)
#define DEBUG_MAP_BANDS			(1 << 13)
#define DEBUG_MAP_DATA			(1 << 14)

#define DEBUG_DEFAULT	(DEBUG_ALL & \
						~DEBUG_EVENTS & \
						~DEBUG_MOUSE_EVENTS & \
						~DEBUG_SIGNALS)

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)		"" #__VA_ARGS__ ""
#define AS_STRING(...)			STRINGIFIED(__VA_ARGS__)
#endif

#ifndef WIN32
#define _DEBUG_OBJECT_ AS_STRING(DEBUG_OBJECT)
#define OBJECT_CONDITIONAL(expression) \
	if (QString (_DEBUG_OBJECT_).isEmpty () || \
		QString (_DEBUG_OBJECT_) == objectName ()) \
	{ \
	expression \
	}
#else
#define OBJECT_CONDITIONAL(expression) expression
#endif

#include	<iostream>
#include	<bitset>
#include	<limits>
using std::clog;
using std::boolalpha;
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
	Image_Viewer::ID =
		"UA::HiRISE::Image_Viewer ($Revision: 1.137 $ $Date: 2014/05/27 17:32:25 $)";


#ifndef DEFAULT_IMAGE_DISPLAY_WIDTH
#define DEFAULT_IMAGE_DISPLAY_WIDTH			512
#endif
#ifndef DEFAULT_IMAGE_DISPLAY_HEIGHT
#define DEFAULT_IMAGE_DISPLAY_HEIGHT		316
#endif
const QSize
	Image_Viewer::DEFAULT_IMAGE_DISPLAY_SIZE
		(DEFAULT_IMAGE_DISPLAY_WIDTH, DEFAULT_IMAGE_DISPLAY_HEIGHT);

/*==============================================================================
	Application configuration parameters
*/
#define Panel_Frame_Style \
	HiView_Config::Panel_Frame_Style
#define Panel_Frame_Width \
	HiView_Config::Panel_Frame_Width

#define Reticule_Cursor \
	HiView_Config::Reticule_Cursor
#define Shift_Cursor \
	HiView_Config::Shift_Cursor
#define Scale_Cursor \
	HiView_Config::Scale_Cursor

/*==============================================================================
	Class data members
*/
#ifndef SCALING_MINOR_INCREMENT
#define SCALING_MINOR_INCREMENT				.01
#endif
double
	Image_Viewer::Scaling_Minor_Increment	= SCALING_MINOR_INCREMENT;

#ifndef SCALING_MAJOR_INCREMENT
#define SCALING_MAJOR_INCREMENT				.1
#endif
double
	Image_Viewer::Scaling_Major_Increment	= SCALING_MAJOR_INCREMENT;

int
	Image_Viewer::Horizontal_Scrollbar_Height,
	Image_Viewer::Vertical_Scrollbar_Width,
	Image_Viewer::Sliding_Scale_Width;

QErrorMessage
	*Image_Viewer::Error_Message		= NULL;

#ifndef DEFAULT_SCALING_IMMEDIATE
#define DEFAULT_SCALING_IMMEDIATE			true;
#endif
bool
	Image_Viewer::Default_Scaling_Immediate	= DEFAULT_SCALING_IMMEDIATE;

#ifndef DEFAULT_SCROLLBARS_ENABLED
#define DEFAULT_SCROLLBARS_ENABLED			true
#endif

/*==============================================================================
	Constructors
*/
Image_Viewer::Image_Viewer
	(
	QWidget*		parent
	)
	:	QFrame (parent),
		Source_Name (),
		Image_Display (NULL),
		Horizontal_Scrollbar (NULL),
		Vertical_Scrollbar (NULL),
		Scrollbars_Enabled (DEFAULT_SCROLLBARS_ENABLED),
		Menu_Position (-1, -1),
		Control_Mode (NO_CONTROL_MODE),
		Mouse_Drag_Image_Position (-1, -1),
		Default_Cursor (NULL),
        Projector(NULL),
		Block_Image_Updates (true)
{
setObjectName ("Image_Viewer");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer @ " << (void*)this
		<< ": " << object_pathname (this) << endl;)
#endif

setFrameStyle (Panel_Frame_Style);
setLineWidth (Panel_Frame_Width);

create_menus ();

//	Vertical_Scrollbar - right side.
Vertical_Scrollbar = new QScrollBar (Qt::Vertical, this);
Vertical_Scrollbar->setToolTip (tr ("Vertical scrolling"));
Vertical_Scrollbar_Width = (Vertical_Scrollbar->sizeHint ()).rwidth ();
Vertical_Scrollbar->setMinimum (0);

//	Horizontal_Scrollbar - bottom.
Horizontal_Scrollbar = new QScrollBar (Qt::Horizontal, this);
Horizontal_Scrollbar->setToolTip (tr ("Horizontal scrolling"));
Horizontal_Scrollbar_Height = (Horizontal_Scrollbar->sizeHint ()).rheight ();
Horizontal_Scrollbar->setMinimum (0);

//	Lower Right Corner (LRC) widget.
LRC_Widget = new QWidget (this);
LRC_Widget->setFixedSize
	(Vertical_Scrollbar_Width, Horizontal_Scrollbar_Height);
LRC_Widget->setVisible (false);

//	Scrollbars movement connections.
connect (Horizontal_Scrollbar,
	SIGNAL	(valueChanged (int)),
	SLOT (scrollbar_value_changed ()));
connect (Vertical_Scrollbar,
	SIGNAL	(valueChanged (int)),
	SLOT (scrollbar_value_changed ()));

//	Sliding_Scale.
Sliding_Scale = new QSlider (Qt::Vertical, this);
Sliding_Scale->setToolTip (tr ("Image scaling"));
Sliding_Scale->setTickPosition (QSlider::NoTicks);
Sliding_Scale->setRange
	(scale_to_slider (Tiled_Image_Display::min_scale ()),
	 scale_to_slider (Tiled_Image_Display::max_scale ()));
Sliding_Scale->setSingleStep
	(Sliding_Scale->maximum () * Scaling_Minor_Increment);
if (! Sliding_Scale->singleStep ())
	Sliding_Scale->setSingleStep (1);	//	Minimum increment.
Sliding_Scale->setPageStep
	(Sliding_Scale->maximum () * Scaling_Major_Increment);
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "    Sliding_Scale range: "
		<< slider_to_scale (Sliding_Scale->minimum ())
		<< '/' << Sliding_Scale->minimum () << " - "
		<< slider_to_scale (Sliding_Scale->maximum ())
		<< '/' << Sliding_Scale->maximum () << endl
	 << "    Sliding_Scale increments: "
		<< "minor - " << Scaling_Minor_Increment
		<< '/' << Sliding_Scale->singleStep ()
		<< ", major - " << Scaling_Major_Increment
		<< '/' << Sliding_Scale->pageStep () << endl;)
#endif
Sliding_Scale->setTracking (default_scaling_immediate ());
Sliding_Scale->setValue (scale_to_slider (1.0));
Sliding_Scale_Width = (Sliding_Scale->sizeHint ()).rwidth ();

connect (Sliding_Scale,
	SIGNAL (sliderMoved (int)),
	SLOT (sliding_scale_value (int)));
connect (Sliding_Scale,
	SIGNAL (valueChanged (int)),
	SLOT (sliding_scale_value_changed (int)));

Sliding_Scale_Value = new QLabel (this);
QFont
	scale_font (Sliding_Scale_Value->font ());
scale_font.setStyleHint (QFont::SansSerif);
scale_font.setStretch (QFont::Condensed);
scale_font.setPointSize (9);
Sliding_Scale_Value->setFont (scale_font);
sliding_scale_value (1.0);
Sliding_Scale_Width = qMax (Sliding_Scale_Width,
	(Sliding_Scale_Value->sizeHint ()).width ());
Sliding_Scale_Value->setFixedSize
	(Sliding_Scale_Width, Horizontal_Scrollbar_Height);

//	Enable keyboard events.
setFocusPolicy (Qt::StrongFocus);

//	Image_Display
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
OBJECT_CONDITIONAL (
clog << "    new Tiled_Image_Display ..." << endl;)
#endif
Image_Display =  new Tiled_Image_Display (this);

//		Default cursor.
Default_Cursor = Reticule_Cursor;
Image_Display->setCursor (*Default_Cursor);

//	Instantiate an empty Image_Display.
loaded (true);

//		Image load connection.
connect (Image_Display,
	SIGNAL (image_loaded (bool)),
	SLOT (loaded (bool)));
//		Image cursor move connection.
connect (Image_Display,
	SIGNAL (image_cursor_moved (const QPoint&, const QPoint&)),
	SLOT (cursor_moved (const QPoint&, const QPoint&)));
//		Image display move propagation.
connect (Image_Display,
	SIGNAL (image_moved (const QPoint&, int)),
	SIGNAL (image_moved (const QPoint&, int)));
//		Image display resize propagation.
connect (Image_Display,
	SIGNAL (displayed_image_region_resized (const QSize&)),
	SIGNAL (displayed_image_region_resized (const QSize&)));
connect (Image_Display,
	SIGNAL (display_viewport_resized (const QSize&)),
	SIGNAL (display_viewport_resized (const QSize&)));
//		Image scaling propagation.
connect (Image_Display,
	SIGNAL (image_scaled (const QSizeF&, int)),
	SIGNAL (image_scaled (const QSizeF&, int)));
//		Image display rendering status propagation.
connect (Image_Display,
	SIGNAL (rendering_status (int)),
	SIGNAL (rendering_status (int)));
//		Image display status notice propagation.
connect (Image_Display,
	SIGNAL (rendering_status_notice (const QString&)),
	SIGNAL (rendering_status_notice (const QString&)));
//		Image display status change notice propagation.
connect (Image_Display,
	SIGNAL (state_change (int)),
	SIGNAL (state_change (int)));

Block_Image_Updates = false;
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer "<< object_pathname (this) << endl;)
#endif
}


Image_Viewer::~Image_Viewer ()
{
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
OBJECT_CONDITIONAL (
clog << ">-< ~Image_Viewer: @ " << (void*)this << endl;)
#endif
}

/*==============================================================================
	Image
*/
bool
Image_Viewer::image
	(
	const QString&	source_name,
	const QSize&	display_size
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::image (QString, QSize) "
		<< object_pathname (this) << endl
	 << "    source_name = \"" << source_name << '"' << endl
	 << "    display_size = " << display_size << endl;)
#endif
bool
	loaded;
if (display_size.isEmpty ())
	loaded = Image_Display->image (source_name, viewport_size ());
else
	loaded = Image_Display->image (source_name, display_size);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::image (QString, QSize) "
		<< object_pathname (this) << ": " << boolalpha << loaded << endl;)
#endif
return loaded;
}


bool
Image_Viewer::image
	(
	const QString&	source_name,
	const QSizeF&	scaling
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::image (QString, QSizeF) "
		<< object_pathname (this) << endl
	 << "    source_name = \"" << source_name << '"' << endl
	 << "    scaling = " << scaling << endl;)
#endif
bool
	loaded;
if (scaling.isEmpty ())
	loaded = image (source_name, QSize ());
else
	loaded = Image_Display->image (source_name, scaling);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::image (QString, QSizeF) "
		<< object_pathname (this) << ": " << boolalpha << loaded << endl;)
#endif
return loaded;
}


bool
Image_Viewer::image
	(
	const Shared_Image&	source_image,
	const QSize&		display_size
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::image (Shared_Image, QSize) "
		<< object_pathname (this) << endl
	 << "    source_" << *source_image << endl
	 << "    display_size = " << display_size << endl;)
#endif
bool
	loaded;
if (display_size.isEmpty ())
	loaded = Image_Display->image (source_image, viewport_size ());
else
	loaded = Image_Display->image (source_image, display_size);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::image (Shared_Image, QSize) "
		<< object_pathname (this) << ": " << boolalpha << loaded << endl;)
#endif
return loaded;
}


bool
Image_Viewer::image
	(
	const Shared_Image&	source_image,
	const QSizeF&		scaling
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::image (Shared_Image, QSizeF) "
		<< object_pathname (this) << endl
	 << "    source_" << *source_image << endl
	 << "    scaling = " << scaling << endl;)
#endif
bool
	loaded;
if (scaling.isEmpty ())
	loaded = image (source_image, QSize ());
else
	loaded = Image_Display->image (source_image, scaling);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::image (Shared_Image, QSizeF) "
		<< object_pathname (this) << ": " << boolalpha << loaded << endl;)
#endif
return loaded;
}


bool
Image_Viewer::image
	(
	const QImage&	source_image,
	const QSize&	display_size
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
LOCKED_LOGGING ((
clog << ">>> Image_Viewer::image (QImage, QSize) "
		<< object_pathname (this) << endl
	 << "    source_image @ " << (void*)&source_image << endl
	 << "    display_size = " << display_size << endl));)
#endif
//	Copy the source image.
const Plastic_Image
	*source;
Plastic_Image
	*plastic_image;
if ((source = dynamic_cast<const Plastic_Image*>(&source_image)))
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	OBJECT_CONDITIONAL (
	clog << "    cloning the Plastic_Image" << endl;)
	#endif
	plastic_image = source->clone ();
	}
else
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	OBJECT_CONDITIONAL (
	clog << "    wrapping the QImage in a Plastic_Image" << endl;)
	#endif
	plastic_image = new Plastic_QImage (source_image);
	}
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "    new " << *plastic_image << endl;)
#endif

QSize
	fit_to_size (display_size);
if (fit_to_size.isEmpty ())
	fit_to_size = viewport_size ();
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "    fit_to_size = " << fit_to_size << endl;)
#endif
bool
	registered = image (Shared_Image (plastic_image), fit_to_size);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
LOCKED_LOGGING ((
clog << "<<< Image_Viewer::image "
		<< object_pathname (this) << ": " << boolalpha << registered << endl));)
#endif
return registered;
}


bool
Image_Viewer::image
	(
	const QImage&	source_image,
	const QSizeF&	scaling
	)
{
if (scaling.isEmpty ())
	return image (source_image, QSize ());

#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
LOCKED_LOGGING ((
clog << ">>> Image_Viewer::image (QImage, QSizeF) "
		<< object_pathname (this) << endl
	 << "    source_image @ " << (void*)&source_image << endl
	 << "    scaling = " << scaling << endl));)
#endif
//	Copy the source image.
const Plastic_Image
	*source;
Plastic_Image
	*plastic_image;
if ((source = dynamic_cast<const Plastic_Image*>(&source_image)))
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	OBJECT_CONDITIONAL (
	clog << "    cloning the Plastic_Image" << endl;)
	#endif
	plastic_image = source->clone ();
	}
else
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	OBJECT_CONDITIONAL (
	clog << "    wrapping the QImage in a Plastic_Image" << endl;)
	#endif
	plastic_image = new Plastic_QImage (source_image);
	}
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
clog << "    new " << *plastic_image << endl;)
#endif

bool
	registered = image (Shared_Image (plastic_image), scaling);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
LOCKED_LOGGING ((
clog << "<<< Image_Viewer::image "
		<< object_pathname (this) << ": " << boolalpha << registered << endl));)
#endif
return registered;
}


void
Image_Viewer::loaded
	(
	bool	successful
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
OBJECT_CONDITIONAL (
LOCKED_LOGGING ((
clog << ">>> Image_Viewer::loaded " << object_pathname (this)
		<< ": " << boolalpha << successful << endl));)
#endif
Source_Name = Image_Display->image_name ();

//	Reset the image display conditions.
QSize
	display_size (size ());
if (! display_size.isValid ())
	display_size = sizeHint ();
layout_display (display_size);
update_sliding_scale ();
update_actions ();

if (! Block_Image_Updates)
	{
	//	>>> SIGNAL <<<
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SIGNALS))
	OBJECT_CONDITIONAL (
	LOCKED_LOGGING ((
	clog << "^^^ Image_Viewer::loaded: "
			  "emit image_loaded " << boolalpha << successful << endl));)
	#endif
	emit image_loaded (successful);
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_SIGNALS))
else
	{
	OBJECT_CONDITIONAL (
	LOCKED_LOGGING ((
	clog << "    Image_Viewer::loaded: emit image_loaded "
			<< boolalpha << successful << " blocked" << endl));)
	}
OBJECT_CONDITIONAL (
LOCKED_LOGGING ((
clog << "<<< Image_Viewer::loaded " << object_pathname (this) << endl));)
#endif
}


void
Image_Viewer::cancel_rendering ()
{Image_Display->cancel_rendering ();}

/*==============================================================================
	Layout
*/
void
Image_Viewer::layout_display
	(
	QSize	display_size
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::layout_display " << object_pathname (this) << endl
	 << "     display_size = " << display_size << endl;)
#endif
QSize
	scaled_image_size (Image_Display->scaled_image_size ());
if (display_size.isEmpty ())
	display_size = scaled_image_size;
QSize
	widget_size (display_size);
int
	frame_margin = frameWidth ();
//	Exclude the frame margins from the image display size.
display_size.rwidth ()  -= (frame_margin << 1);
display_size.rheight () -= (frame_margin << 1);
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
OBJECT_CONDITIONAL (
clog << "             viewport size = " << size () << endl
	 << "               widget size = " << widget_size << endl
	 << "               frame width = " << frame_margin << endl
	 << "        image display size = " << display_size << endl
	 << "    displayed_image_region = " << displayed_image_region () << endl
	 << "         scaled_image_size = " << scaled_image_size << endl
	 << "        scrollbars enabled = " << boolalpha << Scrollbars_Enabled
	 	<< endl;)
#endif

bool
	vertical_scrollbar_visible = false,
	horizontal_scrollbar_visible = false,
	sliding_scale_visible = false;
int
	sliding_scale_x = 0,
	sliding_scale_height = 0;
if (Scrollbars_Enabled &&
	! scaled_image_size.isEmpty ())
	{
	sliding_scale_visible = true;
	display_size.rwidth () -= Sliding_Scale_Width;
	sliding_scale_x =
		frame_margin + display_size.rwidth ();
	sliding_scale_height =
		display_size.rheight () - Horizontal_Scrollbar_Height;
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "    Sliding_Scale visible" << endl
		 << "      image display width reduced by "
		 	<< Sliding_Scale_Width << endl;)
	#endif

	if (scaled_image_size.rheight () > display_size.rheight ())
		{
		vertical_scrollbar_visible = true;
		display_size.rwidth () -= Vertical_Scrollbar_Width;
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "    Vertical_Scrollbar visible" << endl
			 << "      image display width reduced by "
		 		<< Vertical_Scrollbar_Width << endl;)
		#endif
		}
	if (scaled_image_size.rwidth () > display_size.rwidth ())
		{
		horizontal_scrollbar_visible = true;
		display_size.rheight () -= Horizontal_Scrollbar_Height;
		#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "    Horizontal_Scrollbar visible" << endl
			 << "      image display height reduced by "
		 		<< Horizontal_Scrollbar_Height << endl;)
		#endif
		if (! vertical_scrollbar_visible &&
			scaled_image_size.rheight () > display_size.rheight ())
			{
			vertical_scrollbar_visible = true;
			display_size.rwidth () -= Vertical_Scrollbar_Width;
			#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
			OBJECT_CONDITIONAL (
			clog << "    Vertical_Scrollbar visible" << endl
				 << "      image display width reduced by "
		 			<< Vertical_Scrollbar_Width << endl;)
			#endif
			}
		}
	}

#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
OBJECT_CONDITIONAL (
clog << "    Image_Display geometry, request = "
		<< frame_margin << "x, "
		<< frame_margin << "y, "
		<< display_size.rwidth () << "w, "
		<< display_size.rheight () << 'h' << endl;)
#endif
Image_Display->setGeometry
	(frame_margin, frame_margin,
	display_size.rwidth (), display_size.rheight ());
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
OBJECT_CONDITIONAL (
clog << "    Image_Display geometry,  actual = "
		<< Image_Display->geometry () << endl;)
#endif

Vertical_Scrollbar->setVisible (vertical_scrollbar_visible);
if (vertical_scrollbar_visible)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "      Vertical_Scrollbar geometry = "
			<< (frame_margin + display_size.rwidth ()) << "x, "
			<< frame_margin << "y, "
			<< Vertical_Scrollbar_Width << "w, "
			<< display_size.rheight () << 'h' << endl;)
	#endif
	Vertical_Scrollbar->setGeometry
		(frame_margin + display_size.rwidth (), frame_margin,
		Vertical_Scrollbar_Width, display_size.rheight ());
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "                                    "
			<< Vertical_Scrollbar->geometry () << endl;)
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
else
OBJECT_CONDITIONAL (
	clog << "    Vertical_Scrollbar not visible" << endl;)
#endif

Horizontal_Scrollbar->setVisible (horizontal_scrollbar_visible);
if (horizontal_scrollbar_visible)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "    Horizontal_Scrollbar geometry = "
			<< frame_margin << "x, "
			<< (frame_margin + display_size.rheight ()) << "y, "
			<< display_size.rwidth () << "w, "
			<< Horizontal_Scrollbar_Height << 'h' << endl;)
	#endif
	Horizontal_Scrollbar->setGeometry
		(frame_margin, frame_margin + display_size.rheight (),
		display_size.rwidth (), Horizontal_Scrollbar_Height);
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "                                    "
			<< Horizontal_Scrollbar->geometry () << endl;)
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
else
OBJECT_CONDITIONAL (
	clog << "    Horizontal_Scrollbar not visible" << endl;)
#endif

Sliding_Scale->setVisible (sliding_scale_visible);
Sliding_Scale_Value->setVisible (sliding_scale_visible);
if (sliding_scale_visible)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "           Sliding_Scale geometry = "
			<< sliding_scale_x << "x, "
			<< frame_margin << "y, "
			<< Sliding_Scale_Width << "w, "
			<< sliding_scale_height << 'h' << endl;)
	#endif
	Sliding_Scale->setGeometry
		(sliding_scale_x, frame_margin,
		Sliding_Scale_Width, sliding_scale_height);
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "                                    "
			<< Sliding_Scale->geometry () << endl;)
	#endif

	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "     Sliding_Scale_Value location = "
			<< sliding_scale_x << "x, "
			<< (frame_margin + sliding_scale_height) << 'y' << endl;)
	#endif
	Sliding_Scale_Value->move
		(sliding_scale_x, frame_margin + sliding_scale_height);
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "                                    "
			<< Sliding_Scale_Value->geometry () << endl;)
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
else
OBJECT_CONDITIONAL (
	clog << "    Sliding_Scale not visible" << endl;)
#endif

adjust_scrollbars_range ();

if (vertical_scrollbar_visible &&
	horizontal_scrollbar_visible)
	{
	LRC_Widget->move
		(frame_margin + display_size.rwidth (),
		 frame_margin + display_size.rheight ());
	LRC_Widget->setVisible (true);
	}
else
	LRC_Widget->setVisible (false);

if (widget_size != size ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "    Image_Viewer::layout_display: adjusting ..." << endl;)
	#endif
	updateGeometry ();
	}
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_EVENTS))
OBJECT_CONDITIONAL (
clog << "    displayed_image_region = " << displayed_image_region () << endl
	 << "<<< Image_Viewer::layout_display " << object_pathname (this) << endl;)
#endif
}


QSize
Image_Viewer::sizeHint () const
{
#if ((DEBUG_SECTION) & DEBUG_SIZE_HINT)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::sizeHint" << endl
	 << "    Image_Display sizeHint = " << Image_Display->sizeHint () << endl;)
#endif
QSize
	display_size (Image_Display->sizeHint ());
if (display_size.isEmpty ())
	display_size = DEFAULT_IMAGE_DISPLAY_SIZE;
display_size.rwidth ()  += (frameWidth () << 1);
display_size.rheight () += (frameWidth () << 1);
if (Vertical_Scrollbar &&
	Vertical_Scrollbar->isVisible ())
	display_size.rwidth () += Vertical_Scrollbar_Width;
if (Sliding_Scale &&
	Sliding_Scale->isVisible ())
	display_size.rwidth () += Sliding_Scale_Width;
if (Horizontal_Scrollbar &&
	Horizontal_Scrollbar->isVisible ())
	display_size.rheight () += Horizontal_Scrollbar_Height;
#if ((DEBUG_SECTION) & DEBUG_SIZE_HINT)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::sizeHint: " << display_size << endl;)
#endif
return display_size;
}


QSize
Image_Viewer::viewport_size () const
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_LOAD_IMAGE))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::viewport_size" << endl;)
#endif
QSize
	preferred_size (Image_Display->size ());
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_LOAD_IMAGE))
OBJECT_CONDITIONAL (
clog << "    Image_Display size = " << preferred_size << endl;)
#endif
if (preferred_size.isEmpty ())
	preferred_size = DEFAULT_IMAGE_DISPLAY_SIZE;
else
	{
	//	Use the display size sans scrollbars.
	#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_LOAD_IMAGE))
	OBJECT_CONDITIONAL (
	clog << "    adjusting for scrollbars" << endl;)
	#endif
	if (Vertical_Scrollbar &&
		Vertical_Scrollbar->isVisible ())
		preferred_size.rwidth () += Vertical_Scrollbar_Width;
	if (Horizontal_Scrollbar &&
		Horizontal_Scrollbar->isVisible ())
		preferred_size.rheight () += Horizontal_Scrollbar_Height;

	//	!!! Hack to ensure that the image will fit without scrollbars.
	preferred_size.rwidth () -= 1;
	preferred_size.rheight () -= 1;
	}
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_LOAD_IMAGE))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::viewport_size: " << preferred_size << endl;)
#endif
return preferred_size;
}


QSize
Image_Viewer::image_display_size () const
{
#if ((DEBUG_SECTION) & DEBUG_LAYOUT)
OBJECT_CONDITIONAL (
clog << ">-< Image_Viewer::image_display_size: "
		<< Image_Display->size () << endl;)
#endif
return Image_Display->size ();
}


void
Image_Viewer::max_source_image_area
	(
	int		area
	)
{
if (area < 1)
	area = 1;
Image_Display->max_source_image_area ((unsigned long)area << 20);
}


void
Image_Viewer::rendering_increment_lines
	(
	int		rendering_increment
	)
{Image_Display->rendering_increment_lines (rendering_increment);}


void
Image_Viewer::background_color
	(
	QRgb	color
	)
{Image_Display->background_color (color);}


void
Image_Viewer::tile_size
	(
	int		size
	)
{tile_size (QSize (size, size));}


void
Image_Viewer::tile_size
	(
	const QSize&	size
	)
{
QSize
	new_size (size);
if (new_size.rwidth ()  < Tiled_Image_Display::minimum_tile_dimension ())
	new_size.rwidth ()  = Tiled_Image_Display::minimum_tile_dimension ();
if (new_size.rheight () < Tiled_Image_Display::minimum_tile_dimension ())
	new_size.rheight () = Tiled_Image_Display::minimum_tile_dimension ();
Tiled_Image_Display::default_tile_display_size (new_size);
Image_Display->tile_display_size (new_size);
}

/*------------------------------------------------------------------------------
	JP2 specific settings.
*/
void
Image_Viewer::JPIP_request_timeout
	(
	int		seconds
	)
{
if (seconds < 0)
	seconds = 0;
JP2_Reader::default_JPIP_request_timeout ((unsigned int)seconds);
}


int
Image_Viewer::JPIP_request_timeout ()
{return JP2_Reader::default_JPIP_request_timeout ();}


void
Image_Viewer::JPIP_proxy
	(
	const QString&	proxy
	)
{JP2_Reader::default_jpip_proxy (proxy.toStdString ());}


QString
Image_Viewer::JPIP_proxy ()
{return QString::fromStdString (JP2_Reader::default_jpip_proxy ());}


void
Image_Viewer::JPIP_cache_directory
	(
	const QString&	pathname
	)
{JP2_Reader::default_jpip_cache_directory (pathname.toStdString ());}


QString
Image_Viewer::JPIP_cache_directory ()
{return QString::fromStdString (JP2_Reader::default_jpip_cache_directory ());}

/*------------------------------------------------------------------------------
	Scrolling
*/
bool
Image_Viewer::move_image
	(
	const QPoint&	origin,
	int				band
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::move_image: "
		<< origin << ", " << band << 'b' << endl;)
#endif
bool
	changed = Image_Display->move_image (origin, band);
if (changed)
	layout_display (size ());
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MOVE))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::move_image: " << boolalpha << changed << endl;)
#endif
return changed;
}


bool
Image_Viewer::shift_image
	(
	const QSize&	offsets,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOVE)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::shift_image: "
		<< offsets << ", " << band << 'b' << endl;)
#endif
bool
	moved = false;
if (! offsets.isNull ())
	{
	QPoint
		origin (round_down (displayed_image_origin (band)));
	#if ((DEBUG_SECTION) & DEBUG_MOVE)
	OBJECT_CONDITIONAL (
	clog << "    from origin = " << origin << endl;)
	#endif
	origin.rx () -= offsets.width ();
	origin.ry () -= offsets.height ();
	#if ((DEBUG_SECTION) & DEBUG_MOVE)
	OBJECT_CONDITIONAL (
	clog << "      to origin = " << origin << endl;)
	#endif
	moved = move_image (origin, band);
	}
#if ((DEBUG_SECTION) & DEBUG_MOVE)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::shift_image: " << boolalpha << moved << endl;)
#endif
return moved;
}


bool
Image_Viewer::shift_display
	(
	const QSize&	offsets,
	int				band
	)
{
#if ((DEBUG_SECTION) & DEBUG_MOVE)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::shift_display: "
		<< offsets << ", " << band << 'b' << endl;)
#endif
bool
	moved = false;
if (! offsets.isNull ())
	{
	QSizeF
		scaled_offsets (image_scaling (band));
	#if ((DEBUG_SECTION) & DEBUG_MOVE)
	OBJECT_CONDITIONAL (
	clog << "       image_scaling = " << scaled_offsets << endl;)
	#endif
	if (scaled_offsets.rwidth () >= 1.0)
		scaled_offsets.rwidth () *= offsets.width ();
	else
		scaled_offsets.rwidth () =
			offsets.width () / scaled_offsets.rwidth ();
	if (scaled_offsets.rheight () >= 1.0)
		scaled_offsets.rheight () *= offsets.height ();
	else
		scaled_offsets.rheight () =
			offsets.height () / scaled_offsets.rheight ();
	#if ((DEBUG_SECTION) & DEBUG_MOVE)
	OBJECT_CONDITIONAL (
	clog << "      scaled offsets = " << scaled_offsets
			<< " (" << round_up (scaled_offsets) << ')' << endl;)
	#endif
	moved = shift_image (round_up (scaled_offsets), band);
	}
#if ((DEBUG_SECTION) & DEBUG_MOVE)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::shift_image: " << boolalpha << moved << endl;)
#endif
return moved;
}


void
Image_Viewer::adjust_scrollbars_range ()
{
if (! Scrollbars_Enabled)
	return;

#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << ">>> adjust_scrollbars_range" << endl
	 << "      Vertical_Scrollbar values: "
		<< Vertical_Scrollbar->value () << " of range "
		<< Vertical_Scrollbar->minimum () << " - "
		<< Vertical_Scrollbar->maximum () << ", pageStep "
		<< Vertical_Scrollbar->pageStep () << endl
	 << "    Horizontal_Scrollbar values: "
		<< Horizontal_Scrollbar->value () << " of range "
		<< Horizontal_Scrollbar->minimum () << " - "
		<< Horizontal_Scrollbar->maximum () << ", pageStep "
		<< Horizontal_Scrollbar->pageStep () << endl;)
#endif
/*	The scrollbar page step (thumb) size is logically equivalent to
	the size of the display scaled to image space.
*/
QSize
	display_size (Image_Display->size ());
QSizeF
	scaling (image_scaling ());
#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "              display size = " << display_size << endl
	 << "             image_scaling = " << scaling << endl;)
#endif
display_size.rwidth () =
	display_size.rwidth () / scaling.rwidth ();
//	static_cast<int>(ceil (display_size.rwidth () / scaling.rwidth ()));
display_size.rheight () =
	display_size.rheight () / scaling.rheight ();
//	static_cast<int>(ceil (display_size.rheight () / scaling.rheight ()));
#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "    scaled display size = " << display_size
		<< " (new pageStep)" << endl;)
#endif
Horizontal_Scrollbar->setPageStep (display_size.rwidth ());
Vertical_Scrollbar->setPageStep (display_size.rheight ());

/*	The maximum range corresponds to that part of the scroll bar
	not including the thumb, which is logically equivalent to 
	the image size minus the scaled display size.
	The maximum range is never less than zero.
*/
QSize
	size_of_image (image_size ());
#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "                image_size = " << size_of_image << endl;)
#endif
size_of_image.rwidth () -= display_size.rwidth ();
if (size_of_image.rwidth () < 0)
	size_of_image.rwidth () = 0;
size_of_image.rheight () -= display_size.rheight ();
if (size_of_image.rheight () < 0)
	size_of_image.rheight () = 0;
Horizontal_Scrollbar->setMaximum (size_of_image.rwidth ());
Vertical_Scrollbar->setMaximum (size_of_image.rheight ());

/*	The scrollbar value (location of the thumb) is logically equivalent to
	the image display origin.
*/
QPoint
	origin (round_down (displayed_image_origin ()));
#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "    displayed_image_origin = " << origin << endl;)
#endif
if (Horizontal_Scrollbar->value () != origin.rx ())
	Horizontal_Scrollbar->setValue (origin.rx ());
if (Vertical_Scrollbar->value () != origin.ry ())
	Vertical_Scrollbar->setValue (origin.ry ());

#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "      Vertical_Scrollbar values: "
		<< Vertical_Scrollbar->value () << " of range "
		<< Vertical_Scrollbar->minimum () << " - "
		<< Vertical_Scrollbar->maximum () << ", pageStep "
		<< Vertical_Scrollbar->pageStep () << endl
	 << "    Horizontal_Scrollbar values: "
		<< Horizontal_Scrollbar->value () << " of range "
		<< Horizontal_Scrollbar->minimum () << " - "
		<< Horizontal_Scrollbar->maximum () << ", pageStep "
		<< Horizontal_Scrollbar->pageStep () << endl
	 << "    displayed_image_origin = " << displayed_image_origin () << endl
	 << "<<< adjust_scrollbars_range" << endl;)
#endif
}


void
Image_Viewer::scrollbar_value_changed ()
{
if (Image_Display &&
	! Block_Image_Updates)
	{
	#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT | DEBUG_MOVE))
	OBJECT_CONDITIONAL (
	clog << ">>> Image_Viewer::scrollbar_value_changed" << endl;)
	#endif
	QPoint
		origin (round_down (displayed_image_origin ()));
	#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT | DEBUG_MOVE))
	OBJECT_CONDITIONAL (
	clog << "    displayed_image_origin = " << origin << endl
		 << "          scrollbar values = "
			<< Horizontal_Scrollbar->value () << "x, "
			<< Vertical_Scrollbar->value () << 'y' << endl;)
	#endif
	if (origin.rx () != Horizontal_Scrollbar->value () ||
		origin.ry () != Vertical_Scrollbar->value ())
		{
		#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT | DEBUG_MOVE))
		OBJECT_CONDITIONAL (
		clog << "    move_image to scrollbar values ..." << endl;)
		#endif
		Image_Display->move_image (QPoint
			(Horizontal_Scrollbar->value (), Vertical_Scrollbar->value ()));
		}
	#if ((DEBUG_SECTION) & (DEBUG_SCROLLBARS | DEBUG_LAYOUT | DEBUG_MOVE))
	OBJECT_CONDITIONAL (
	clog << "<<< Image_Viewer::scrollbar_value_changed" << endl;)
	#endif
	}
}


void
Image_Viewer::scrollbars
	(
	bool	enabled
	)
{
if (Scrollbars_Enabled != enabled)
	{
	Scrollbars_Enabled = enabled;
	layout_display (size ());
	update ();
	}
}

/*------------------------------------------------------------------------------
	Scaling
*/
bool
Image_Viewer::scale_image
	(
	const QSizeF&	scaling,
	const QPoint&	center,
	int				band
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::scale_image: " << scaling << endl
	 << "    center = " << center << endl
	 << "      band = " << band << endl;)
#endif
bool
	scaled = Image_Display->scale_image (scaling, center, band);
if (scaled)
	{
	/*	Block image updates.

		The layout update changes the scrollbar positions which
		would ordinarly cause one image move operation for each
		scrollbar change. These unnecessary and expensive effects
		are prevented during this layout update.
	*/
	Block_Image_Updates = true;
	layout_display (size ());

	double
		scale = qMax (scaling.width (), scaling.height ());
	sliding_scale_value (scale);
	int
		slider_value = scale_to_slider (scale);
	if (Sliding_Scale->value () != slider_value)
		Sliding_Scale->setValue (slider_value);

	update_actions ();
	Block_Image_Updates = false;
	}
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::scale_image: "
		<< boolalpha << scaled << endl;)
#endif
return scaled;
}


bool
Image_Viewer::scale_by
	(
	const QSizeF&	scaling_factors
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::scale_by: " << scaling_factors << endl;)
#endif
QSizeF
	scaling (image_scaling ());
scaling += scaling_factors;
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "    scaling = " << scaling << endl;)
#endif

bool
	fixed_position = true;
QPoint
	display_position;
if (Menu_Position.rx () >= 0)
	{
	display_position = Image_Display->mapFromGlobal (Menu_Position);
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "      menu display position = " << display_position << endl;)
	#endif
	}
else
	{
	display_position = Image_Display->mapFromGlobal (QCursor::pos ());
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "    cursor display position = " << display_position << endl;)
	#endif
	}

QRect
	display_region (image_display_region ());
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "       image display region = " << display_region << endl;)
#endif
if (! display_region.contains (display_position))
	{
	fixed_position = false;
	//	Scale about the center of the image display region.
	display_position.rx () = display_region.width () >> 1;
	display_position.ry () = display_region.height () >> 1;
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "    using the display center = " << display_position << endl;)
	#endif
	}
QPointF
	image_position (Image_Display->map_display_to_image (display_position));
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "      image center position = "
		<< round_down (image_position) << " (" << image_position << ')' << endl;)
#endif
bool
	scaled = scale_image (scaling, round_down (image_position));

if (fixed_position &&
	scaled)
	{
	//	Current cursor position.
	display_position = Image_Display->mapFromGlobal (QCursor::pos ());
	QPoint
		position (Image_Display->map_image_to_display (image_position));
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "    current cursor display position = "
			<< display_position << endl
		 << "      image center display position = "
			<< position << endl;)
	#endif
	if (display_position != position)
		{
		if (position.rx () < 0)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
			OBJECT_CONDITIONAL (
			clog << "     set to left edge" << endl;)
			#endif
			position.rx () = 0;
			}
		else
		if (position.rx () >= Image_Display->width ())
			{
			#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
			OBJECT_CONDITIONAL (
			clog << "     set to right edge" << endl;)
			#endif
			position.rx () = Image_Display->width () - 1;
			}
		if (position.ry () < 0)
			{
			#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
			OBJECT_CONDITIONAL (
			clog << "     set to top edge" << endl;)
			#endif
			position.ry () = 0;
			}
		else
		if (position.ry () >= Image_Display->height ())
			{
			#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
			OBJECT_CONDITIONAL (
			clog << "     set to bottom edge" << endl;)
			#endif
			position.rx () = Image_Display->height () - 1;
			}

		#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
		OBJECT_CONDITIONAL (
		clog << "                     move cursor to = " << position << endl;)
		#endif
		QCursor::setPos (Image_Display->mapToGlobal (position));
		cursor_moved (position, round_down (image_position));
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::scale_by: " << boolalpha << scaled << endl;)
#endif
return scaled;
}


bool
Image_Viewer::scale_up
	(
	double	factor
	)
{
if (factor <= 0.0)
	{
	if (factor < 0.0)
		{
		//	Change the scaled image size by at most one pixel.
		QSizeF
			image_region (displayed_image_region ().size ());
		factor = 1.0 - qMin
			((image_region.rwidth ()  / (image_region.rwidth ()  + 1)),
			 (image_region.rheight () / (image_region.rheight () + 1)));
		}
	else
		{
		factor = scaling_major_increment ();
/*
		//	Find the factor that produces the next highest pixel multiple scale.
		QSizeF
			scaling (image_scaling ());
		double
			scale = qMax (scaling.rwidth (), scaling.rheight ());
		factor = scale;
		if (scale < 1.0)
			factor = 1 / scale;
		int
			level = static_cast<int>(ceil (factor));
		if (level == factor)
			(scale < 1.0) ?  --level : ++level;
		else
		if (scale < 1.0)
			--level;
		factor = level;
		if (scale < 1.0)
			factor = 1.0 / factor;
		factor -= scale;
*/
		}
	}
return scale_by (QSizeF (factor, factor));
}


bool
Image_Viewer::scale_down
	(
	double	factor
	)
{
if (factor <= 0.0)
	{
	if (factor < 0.0)
		{
		//	Change the scaled image size by at most one pixel.
		QSizeF
			image_region (displayed_image_region ().size ());
		factor = 1.0 - qMin
			(((image_region.rwidth ()  - 1) / image_region.rwidth ()),
			 ((image_region.rheight () - 1) / image_region.rheight ()));
		}
	else
		{
		factor = scaling_major_increment ();
/*
		//	Find the factor that produces the next lowest pixel multiple scale.
		QSizeF
			scaling (image_scaling ());
		double
			scale = qMax (scaling.rwidth (), scaling.rheight ());
		factor = scale;
		if (scale < 1.0)
			factor = 1.0 / scale;
		int
			level = static_cast<int>(floor (factor));
		if (level == factor)
			(scale <= 1.0) ?  ++level : --level;
		else
		if (scale < 1.0)
			++level;
		factor = level;
		if (scale <= 1.0)
			factor = 1.0 / factor;
		factor = scale - factor;
*/
		}
	}
return scale_by (QSizeF (-factor, -factor));
}


bool
Image_Viewer::actual_size ()
{
QSizeF
	scaling_factors (1, 1);
return scale_by (scaling_factors -= image_scaling ());
}


bool
Image_Viewer::fit_image_to_window ()
{
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::fit_image_to_window" << endl;)
#endif
bool
	scaled = false;
QSize
	display_viewport (viewport_size ());
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "    display viewport_size = " << display_viewport << endl;)
#endif
if (! display_viewport.isEmpty ())
	{
	double
		scale = qMin ((double)display_viewport.width () / image_width (),
					  (double)display_viewport.height () / image_height ());
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "               image_size = " << image_size () << endl
		 << "                    scale = " << scale << endl;)
	#endif
	if ((scaled = scale_image (QSizeF (scale, scale), QPoint (0, 0))))
		{
		#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
		OBJECT_CONDITIONAL (
		clog << "        scaled image size = " << scaled_image_size () << endl;)
		#endif
		layout_display (size ());
		update_actions ();
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::fit_image_to_window: "
		<< boolalpha << scaled << endl;)
#endif
return scaled;
}


bool
Image_Viewer::fit_to_width ()
{
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::fit_image_width_to_window" << endl;)
#endif
bool
	scaled = false;
QSize
	size_of_display (viewport_size ()),
	size_of_image (image_size ());
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "    display viewport_size = " << size_of_display << endl
	 << "               image size = " << size_of_image << endl;)
#endif
if (! size_of_display.isEmpty () &&
	! size_of_image.isEmpty ())
	{
	double
		scale = (double)size_of_display.width () / size_of_image.rwidth ();
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "                    scale = " << scale << endl;)
	#endif
	if (Vertical_Scrollbar &&
		(int)(scale * size_of_image.rheight ()) > size_of_display.rheight ())
		{
		//	Tall image; the vertical scrollbar will be displayed.
		size_of_display.rwidth () -= Vertical_Scrollbar_Width;
		scale = (double)size_of_display.width () / size_of_image.rwidth ();
		}
	while (size_of_display.rwidth () > 1 &&
			(int)(scale * size_of_image.rwidth ()) > size_of_display.rwidth ())
		{
		--size_of_display.rwidth ();
		scale = (double)size_of_display.rwidth () / size_of_image.rwidth ();
		}

	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "            applied scale = " << scale << endl;)
	#endif
	if ((scaled = scale_image (QSizeF (scale, scale), QPoint (0, 0))))
		{
		#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
		OBJECT_CONDITIONAL (
		clog << "        scaled image size = " << scaled_image_size () << endl;)
		#endif
		layout_display (size ());
		update_actions ();
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::fit_image_width_to_window: "
		<< boolalpha << scaled << endl;)
#endif
return scaled;
}


bool
Image_Viewer::fit_to_height ()
{
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::fit_image_height_to_window" << endl;)
#endif
bool
	scaled = false;
QSize
	size_of_display (viewport_size ()),
	size_of_image (image_size ());
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "    display viewport_size = " << size_of_display << endl
	 << "               image size = " << size_of_image << endl;)
#endif
if (! size_of_display.isEmpty () &&
	! size_of_image.isEmpty ())
	{
	double
		scale = (double)size_of_display.height () / size_of_image.height ();
	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "                    scale = " << scale << endl;)
	#endif
	if (Horizontal_Scrollbar &&
		(int)(scale * size_of_image.rwidth ()) > size_of_display.rwidth ())
		{
		//	Tall image; the horizontal scrollbar will be displayed.
		size_of_display.rheight () -= Horizontal_Scrollbar_Height;
		scale = (double)size_of_display.rheight () / size_of_image.rheight ();
		}
	while (size_of_display.rheight () > 1 &&
			(int)(scale * size_of_image.rheight ()) > size_of_display.rheight ())
		{
		--size_of_display.rheight ();
		scale = (double)size_of_display.rheight () / size_of_image.rheight ();
		}

	#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
	OBJECT_CONDITIONAL (
	clog << "            applied scale = " << scale << endl;)
	#endif
	if ((scaled = scale_image (QSizeF (scale, scale), QPoint (0, 0))))
		{
		#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
		OBJECT_CONDITIONAL (
		clog << "        scaled image size = " << scaled_image_size () << endl;)
		#endif
		layout_display (size ());
		update_actions ();
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_SCALE | DEBUG_LAYOUT | DEBUG_SLOTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::fit_image_height_to_window: "
		<< boolalpha << scaled << endl;)
#endif
return scaled;
}

bool Image_Viewer::copy_coordinates()
{
    if(Image_Display == NULL) 
    {
        return false;
    }
    
    QClipboard *cb = QApplication::clipboard();
    
    const QPoint coord = Image_Display->Get_Saved_Coordinate();

    if (Projector && !Projector->is_identity()) 
    { //this check will ensure that there is a latitude/longitude coordinate, if not then only x,y can be used.
        char str[90]; //string is going to take up at least ~53 characters
        Coordinate coordinate_XY(coord.x(), coord.y());
        Coordinate coordinate_degree = Projector->to_world(coordinate_XY);
        if (!Times_Copied)
        {
            sprintf
            (
                str, 
                "x,y,longitude,latitude\n%.0f,%.0f,%.11f,%.11f", 
                coordinate_XY.X, 
                coordinate_XY.Y, 
                coordinate_degree.X, 
                coordinate_degree.Y
            );
        }
        else
        {
            sprintf
            (
                str, 
                "%.0f,%.0f,%.11f,%.11f", 
                coordinate_XY.X, 
                coordinate_XY.Y, 
                coordinate_degree.X, 
                coordinate_degree.Y
            );
        }
        
        cb->setText(str, QClipboard::Clipboard);
        ++Times_Copied;
    }
    else 
    {
        char str[30]; //string is going to take up at least ~8 characters
        
        if(!Times_Copied)
            sprintf(str, "x\t%d,y\t%d", coord.x(), coord.y());
        else
            sprintf(str, "%d,%d", coord.x(), coord.y());
        cb->setText(str, QClipboard::Clipboard);
        ++Times_Copied;
    }
    return true;
}


void
Image_Viewer::sliding_scale_value_changed
	(
	int		value
	)
{
QSizeF
	scaling (image_scaling ());
if (value != scale_to_slider (qMax (scaling.width (), scaling.height ())))
	{
	scaling.rwidth () = scaling.rheight () = slider_to_scale (value);
	scale_image (scaling, round_down (displayed_image_region ().center ()));
	}
}


void
Image_Viewer::sliding_scale_value
	(
	int		value
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SCALE))
OBJECT_CONDITIONAL (
clog << ">-< Image_Viewer::sliding_scale_value: " << value << " -> "
		<< slider_to_scale (value) << endl;)
#endif
sliding_scale_value (slider_to_scale (value));
}


void
Image_Viewer::sliding_scale_value
	(
	double	value
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SCALE))
OBJECT_CONDITIONAL (
clog << ">-< Image_Viewer::sliding_scale_value: " << value << " -> "
		<< QString ("%1").arg (value, 5, 'f', 2) << endl;)
#endif
Sliding_Scale_Value->setText (QString ("%1").arg (value, 5, 'f', 2));
}


void
Image_Viewer::update_sliding_scale ()
{
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SCALE))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::update_sliding_scale" << endl;)
#endif
QSizeF
	scaling (image_scaling ());
scaling.rwidth () = qMax (scaling.width (), scaling.height ());
int
	value = scale_to_slider (scaling.rwidth ());
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SCALE))
OBJECT_CONDITIONAL (
clog << "     slider = " << Sliding_Scale->value ()
	 	<< " of " << Sliding_Scale->minimum ()
		<< " - "  << Sliding_Scale->maximum () << endl
	 << "    scaling = " << scaling.rwidth () << endl
	 << "     slider = " << value << endl;)
#endif
if (Sliding_Scale->value () != value)
	Sliding_Scale->setValue (value);
sliding_scale_value (scaling.rwidth ());
#if ((DEBUG_SECTION) & (DEBUG_LAYOUT | DEBUG_SCALE))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::update_sliding_scale" << endl;)
#endif
}


double
Image_Viewer::slider_to_scale
	(
	int		value
	)
{return pow (10, static_cast<double>(value) / 100.0);}


int
Image_Viewer::scale_to_slider
	(
	double	value
	)
{return static_cast<int>(log10 (value) * 100.0);}


void
Image_Viewer::min_scale
	(
	double	scale_factor
	)
{
if (scale_factor > 0.0)
	Tiled_Image_Display::min_scale (scale_factor);
}


void
Image_Viewer::max_scale
	(
	double	scale_factor
	)
{
if (scale_factor >= 1.0)
	Tiled_Image_Display::max_scale (scale_factor);
}


void
Image_Viewer::scaling_minor_increment
	(
	double	increment
	)
{
if (increment > 0.0)
	Scaling_Minor_Increment = increment;
}


void
Image_Viewer::scaling_major_increment
	(
	double	increment
	)
{
if (increment > 0.0)
	Scaling_Major_Increment = increment;
}

/*------------------------------------------------------------------------------
	Band and Data Mapping
*/
bool
Image_Viewer::map_bands
	(
	const unsigned int*	band_map
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::map_bands: ";
if (band_map)
	clog << band_map[0] << ", " << band_map[1] << ", " << band_map[2] << endl;
else
	clog << "NULL" << endl;
)
#endif
bool
	changed = Image_Display->map_bands (band_map);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_BANDS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::map_bands: " << boolalpha << changed << endl;)
#endif
return changed;
}


bool
Image_Viewer::map_data
	(
	Data_Map**	maps
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_DATA))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::map_data" << endl;)
#endif
bool
	changed = Image_Display->map_data (maps);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_MAP_DATA))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::map_data: " << boolalpha << changed << endl;)
#endif
return changed;
}

/*==============================================================================
	Menus
*/
void
Image_Viewer::create_menus ()
{
#if ((DEBUG_SECTION) & DEBUG_MENUS)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::create_menus" << endl;)
#endif
QKeySequence
	key_sequence;

View_Menu = new QMenu (tr ("&View"));

Scale_Up_Action = new QAction (tr ("Scale &Up"), this);
key_sequence = QKeySequence (QKeySequence::ZoomIn);
if (key_sequence.isEmpty ())
	key_sequence = QKeySequence (tr ("Ctrl++"));
Scale_Up_Action->setShortcut (key_sequence);
Scale_Up_Action->setEnabled(false);
connect (Scale_Up_Action,
	SIGNAL (triggered ()),
	SLOT (scale_up ()));
View_Menu->addAction (Scale_Up_Action);

Scale_Down_Action = new QAction (tr ("Scale &Down"), this);
key_sequence = QKeySequence (QKeySequence::ZoomOut);
if (key_sequence.isEmpty ())
	key_sequence = QKeySequence (tr ("Ctrl+-"));
Scale_Down_Action->setShortcut (key_sequence);
Scale_Down_Action->setEnabled (false);
connect (Scale_Down_Action,
	SIGNAL (triggered ()),
	SLOT (scale_down ()));
View_Menu->addAction (Scale_Down_Action);

Normal_Size_Action = new QAction (tr ("&Actual Size"), this);
Normal_Size_Action->setShortcut (tr ("Ctrl+1"));
Normal_Size_Action->setEnabled (false);
connect (Normal_Size_Action,
	SIGNAL (triggered ()),
	SLOT (actual_size ()));
View_Menu->addAction (Normal_Size_Action);

Fit_to_Window_Action = new QAction (tr ("&Fit Image to Window"), this);
Fit_to_Window_Action->setEnabled (false);
Fit_to_Window_Action->setShortcut(tr ("Ctrl+Shift+F"));
connect (Fit_to_Window_Action,
	SIGNAL (triggered ()),
	SLOT (fit_image_to_window ()));
View_Menu->addAction (Fit_to_Window_Action);

Fit_to_Width_Action = new QAction (tr ("Fit to &Width"), this);
Fit_to_Width_Action->setEnabled (false);
Fit_to_Width_Action->setShortcut(tr ("Ctrl+Shift+W"));
connect (Fit_to_Width_Action,
	SIGNAL (triggered ()),
	SLOT (fit_to_width ()));
View_Menu->addAction (Fit_to_Width_Action);

Fit_to_Height_Action = new QAction (tr ("Fit to &Height"), this);
Fit_to_Height_Action->setEnabled (false);
Fit_to_Height_Action->setShortcut(tr ("Ctrl+Shift+H"));
connect (Fit_to_Height_Action,
	SIGNAL (triggered ()),
	SLOT (fit_to_height ()));
View_Menu->addAction (Fit_to_Height_Action);
    
    Copy_Action = new QAction (tr ("&Copy Coordinates"), this);
    Copy_Action->setEnabled(false);
    Copy_Action->setShortcut (tr ("Ctrl+C"));
    connect (Copy_Action, SIGNAL (triggered()), SLOT (copy_coordinates()));
    View_Menu->addAction (Copy_Action);
    
    #if ((DEBUG_SECTION) & DEBUG_MENUS)
        OBJECT_CONDITIONAL (clog << "<<< Image_Viewer::create_menus" << endl;)
    #endif
}


QList<QAction*>
Image_Viewer::scale_menu_actions () const
{
#if ((DEBUG_SECTION) & DEBUG_MENUS)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::scale_menu_actions" << endl;)
#endif
QList<QAction*>
	actions;
actions
	<< Scale_Up_Action
	<< Scale_Down_Action
	<< Normal_Size_Action
	<< Fit_to_Window_Action
	<< Fit_to_Width_Action
	<< Fit_to_Height_Action
	<< Copy_Action;
#if ((DEBUG_SECTION) & DEBUG_MENUS)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::scale_menu_actions" << endl;)
#endif
return actions;
}

QAction* Image_Viewer::copy_coordinates_action() const {
	return Copy_Action;
}

void
Image_Viewer::update_actions ()
{
    update_scaling_actions ();
    update_window_fit_actions ();
    update_copy_action();
}


void
Image_Viewer::update_scaling_actions ()
{
if (! Image_Display ||
	image_size ().isEmpty ())
	{
	Scale_Up_Action->setEnabled (false);
	Scale_Down_Action->setEnabled (false);
	Normal_Size_Action->setEnabled (false);
	Fit_to_Window_Action->setEnabled (false);
	}
else
	{
	QSizeF
		scaling (image_scaling ());
	Scale_Up_Action->setEnabled
		(max (scaling.rwidth (), scaling.rheight ())
		< Image_Display->max_scale ());
	Scale_Down_Action->setEnabled
		(min (scaling.rwidth (), scaling.rheight ())
		> Image_Display->min_scale ());
	Normal_Size_Action->setEnabled
		(scaling.rwidth ()  != 1.0 ||
		 scaling.rheight () != 1.0);
	}
}


void
Image_Viewer::update_window_fit_actions ()
{
Fit_to_Window_Action->setEnabled (! display_fit_to_image ());
QSize
	display_size (image_display_size ()),
	scaled_size (scaled_image_size ());
Fit_to_Width_Action->setEnabled
	(display_size.rwidth () != scaled_size.rwidth () &&
	 display_size.rwidth () != (scaled_size.rwidth () - 1) &&
	 display_size.rwidth () != (scaled_size.rwidth () + 1));
Fit_to_Height_Action->setEnabled
	(display_size.height () != scaled_size.height () &&
	 display_size.height () != (scaled_size.height () - 1) &&
	 display_size.height () != (scaled_size.height () + 1));
}

void Image_Viewer::update_copy_action()
{
    if (! Image_Display || image_size ().isEmpty ()) {
        Copy_Action->setEnabled(false);
    }
    else {
        Copy_Action->setEnabled(true);
    }
}

bool
Image_Viewer::display_fit_to_image () const
{
#if ((DEBUG_SECTION) & (DEBUG_MENUS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::display_fit_to_image" << endl;)
#endif
bool
	fit = true;
if (Image_Display)
	{
	QSize
		display_size (image_display_size ()),
		scaled_size (scaled_image_size ()),
		max_size
			(qApp->desktop ()->availableGeometry ().size ()
				- window ()->size () + size ());
	fit =
		(scaled_size.rwidth ()  == display_size.rwidth () &&
		 scaled_size.rheight () <= display_size.rheight ()) ||
		(scaled_size.rheight () == display_size.rheight () &&
		 scaled_size.rwidth ()  <= display_size.rwidth ()) ||
		 display_size == max_size;
	if (! fit)
		{
		QSizeF
			scaling (image_scaling ());
		if (qMin (scaling.rwidth (), scaling.rheight ()) == min_scale () &&
			(scaled_size.rwidth ()  > display_size.rwidth () ||
			 scaled_size.rheight () > display_size.rheight ()))
			fit = true;
		}
	#if ((DEBUG_SECTION) & (DEBUG_MENUS | DEBUG_LAYOUT))
	OBJECT_CONDITIONAL (
	clog << "    image display size = " << display_size << endl
		 << "     scaled_image_size = " << scaled_size << endl
		 << "              max_size = " << max_size << endl;)
	#endif
	}
#if ((DEBUG_SECTION) & (DEBUG_MENUS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::display_fit_to_image: "
	<< boolalpha << fit << endl;)
#endif
return fit;
}

/*==============================================================================
	Events
*/
void
Image_Viewer::resizeEvent
	(
	QResizeEvent*	event
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::resizeEvent " << object_pathname (this) << endl
	 << "    from " << event->oldSize () << endl
	 << "      to " << event->size () << endl;)
#endif
if (Image_Display)
	layout_display (event->size ());

update_window_fit_actions ();
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_LAYOUT))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::resizeEvent " << object_pathname (this) << endl;)
#endif
}


void
Image_Viewer::contextMenuEvent
	(
	QContextMenuEvent*	event
	)
{
if ((Control_Mode & CONTROL_MODE))
	//	No context menu while a control mode is in effect.
	return;
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::contextMenuEvent:" << endl
	 << "    global position = " << event->globalPos () << endl
	 << "     local position = " << event->pos () << endl;)
#endif
Menu_Position = event->globalPos ();
View_Menu->exec (event->globalPos ());
Menu_Position.rx () =
Menu_Position.ry () = -1;
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::contextMenuEvent" << endl;)
#endif
}

/*------------------------------------------------------------------------------
	Mouse events
*/
void
Image_Viewer::mousePressEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::mousePressEvent:" << endl
	 << "    widget position = " << event->pos () << endl;)
#endif
bool
	accepted = false;
if (Control_Mode == SHIFT_MODE &&
	event->buttons () == Qt::LeftButton)
	{
	QPoint
		display_position
			(Image_Display->mapFromGlobal (event->globalPos ()));
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	OBJECT_CONDITIONAL (
	clog << "     image display position = " << display_position << endl;)
	#endif
	if (image_display_region ().contains (display_position))
		{
		Mouse_Drag_Image_Position =
			(round_down (map_display_to_image (display_position)));
		accepted = true;
		}
	}
else
	{
	Mouse_Drag_Image_Position.rx () =
	Mouse_Drag_Image_Position.ry () = -1;
	}
event->setAccepted (accepted);
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::mousePressEvent" << endl;)
#endif
}


void
Image_Viewer::mouseMoveEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::mouseMoveEvent:" << endl
	 << "    mouse position = " << event->pos () << endl;)
#endif
bool
	accepted = false;
if (Control_Mode == SHIFT_MODE &&
	event->buttons () == Qt::LeftButton)
	{
	accepted = true;
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	OBJECT_CONDITIONAL (
	clog << "    SHIFT_MODE -" << endl;)
	#endif
	QPoint
		position (round_down (map_display_to_image
			(Image_Display->mapFromGlobal (event->globalPos ()))));
	#if ((DEBUG_SECTION) & DEBUG_EVENTS)
	OBJECT_CONDITIONAL (
	clog << "      image position = " << position << endl;)
	#endif
	if (Mouse_Drag_Image_Position.rx () == -1 &&
		Mouse_Drag_Image_Position.ry () == -1)
		{
		//	SHIFT_MODE entered after mousePressEvent.
		mousePressEvent (event);	//	Pretend this was a mousePressEvent.
		accepted = event->isAccepted ();
		}
	else
	if (position != Mouse_Drag_Image_Position &&
		! shift_image (QSize
			(position.rx () - Mouse_Drag_Image_Position.rx (),
			 position.ry () - Mouse_Drag_Image_Position.ry ())) &&
		displayed_image_region ().contains (position))
		Mouse_Drag_Image_Position = position;

	}
else
	{
	Mouse_Drag_Image_Position.rx () =
	Mouse_Drag_Image_Position.ry () = -1;
	}
event->setAccepted (accepted);
#if ((DEBUG_SECTION) & DEBUG_EVENTS)
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::mouseMoveEvent" << endl;)
#endif
}


void
Image_Viewer::mouseReleaseEvent
	(
	QMouseEvent*	event
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::mouseReleaseEvent" << endl
	 << "     button = " << event-> button () << endl
	 << "    buttons = " << event->buttons () << endl;)
#endif
bool
	accepted = false;
if (Control_Mode == SCALE_MODE)
	{
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "    SCALE_MODE" << endl;)
	#endif
	accepted = true;
	if (event->button () == Qt::LeftButton)
		{
		if (Scale_Down_Action->isEnabled ())
			scale_down ();
		}
	else
	if (event->button () == Qt::MidButton)
		{
		if (Scale_Up_Action->isEnabled ())
			scale_up ();
		}
	}
else
	{
	Mouse_Drag_Image_Position.rx () =
	Mouse_Drag_Image_Position.ry () = -1;
	}
event->setAccepted (accepted);
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::mouseReleaseEvent" << endl;)
#endif
}


void
Image_Viewer::wheelEvent
	(
	QWheelEvent*	event
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::wheelEvent:" << endl
	 << "    position = " << event->pos () << endl
	 << "       delta = " << event->delta () << endl;)
#endif
bool
	accepted = false;
int
	delta = event->delta () / 8;
if (delta)
	{
	if (Control_Mode == SCALE_MODE)
		{
		delta /= 4;
		if (delta)
			{
			accepted = true;
			QSizeF
				scaling (image_scaling ());
			double
				max_scaling = qMax (scaling.rwidth (), scaling.rheight ()),
				scale = slider_to_scale (scale_to_slider (max_scaling) + delta);
			#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
			OBJECT_CONDITIONAL (
			clog << "    SCALE_MODE - scaling = " << scaling << endl
				 << "         tentative scale = " << scale << endl
				 << "    scaling_minor_increment = "
		 			<< scaling_minor_increment () << endl;)
			#endif
			if (qAbs (max_scaling  - scale) < scaling_minor_increment ())
				{
				scale = (max_scaling > scale) ?
					(max_scaling - scaling_minor_increment ()) :
					(max_scaling + scaling_minor_increment ());
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "        min change scale = " << scale << endl;)
				#endif
				}
			scaling.rwidth () =
			scaling.rheight () = scale;

			QPoint
				center (round_down (map_display_to_image
					(Image_Display->mapFromGlobal (event->globalPos ()))));
			#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
			OBJECT_CONDITIONAL (
			clog << "      mouse image position = " << center << endl;)
			#endif
			scale_image (scaling, center);
			}
		}
	else
	if (Control_Mode == SHIFT_MODE)
		{
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "    SHIFT_MODE - offset = " << delta << endl;)
		#endif
		accepted = true;
		if (event->orientation () == Qt::Horizontal)
//			shift_display (QSize (delta, 0));
			shift_image (QSize (delta, 0));
		else
//			shift_display (QSize (0, delta));
			shift_image (QSize (0, delta));
		}
	}
event->setAccepted (accepted);
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_MOUSE_EVENTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::wheelEvent" << endl;)
#endif
}


void
Image_Viewer::cursor_moved
	(
	const QPoint&	display_position,
	const QPoint&	image_position
	)
{
//	>>> SIGNAL <<
#if ((DEBUG_SECTION) & (DEBUG_MOUSE_EVENTS | DEBUG_SIGNALS))
OBJECT_CONDITIONAL (
clog << "^^^ Image_Viewer::cursor_moved: emit image_cursor_moved" << endl
	 << "    display_position = " << display_position << endl
	 << "      image_position = " << image_position << endl;)
#endif
emit image_cursor_moved (display_position, image_position);

if (display_position.x () < 0)
	{
	//	Cursor is out-of-bounds.
	//	>>> SIGNAL <<
	#if ((DEBUG_SECTION) & (DEBUG_MOUSE_EVENTS | DEBUG_SIGNALS))
	OBJECT_CONDITIONAL (
	clog << "^^^ Image_Viewer::cursor_moved: emit image_pixel_value" << endl
		 << "    display_pixel = " << Plastic_Image::Triplet () << endl
		 << "      image_pixel = " << Plastic_Image::Triplet () << endl;)
	#endif
	emit image_pixel_value
		(Plastic_Image::Triplet (), Plastic_Image::Triplet ());
	}
else
	{
	//	>>> SIGNAL <<
	#if ((DEBUG_SECTION) & (DEBUG_MOUSE_EVENTS | DEBUG_SIGNALS))
	OBJECT_CONDITIONAL (
	clog << "^^^ Image_Viewer::cursor_moved: emit image_pixel_value" << endl
		 << "    display_pixel = " << display_pixel (display_position) << endl
		 << "      image_pixel = " << image_pixel (image_position) << endl;)
	#endif
	emit image_pixel_value
		(display_pixel (display_position), image_pixel (image_position));
	}
}

/*------------------------------------------------------------------------------
	Key press events
*/
void
Image_Viewer::keyPressEvent
	(
	QKeyEvent* event
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::keyPressEvent:" << endl
	 << "          key = 0x" << hex << event->key () << endl
	 << "    modifiers = 0x" << event->modifiers () << dec << endl
	 << "       repeat = " << boolalpha << event->isAutoRepeat () << endl;)
#endif
bool
	accepted = false;
Qt::KeyboardModifiers
	modifiers = event->modifiers ();
int
	key = event->key ();
if (key == Qt::Key_Up ||
	key == Qt::Key_Down ||
	key == Qt::Key_Left ||
	key == Qt::Key_Right)
	{
	//	Arrow key actions
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "    arrow key" << endl;)
	#endif
	if (Control_Mode == SCALE_MODE)
		{
		//	Image scaling mode in effect.
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "      SCALE_MODE" << endl;)
		#endif
		switch (key)
			{
			case Qt::Key_Up:
				if (Scale_Up_Action->isEnabled ())
					{
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
					OBJECT_CONDITIONAL (
					clog << "        scale_up" << endl;)
					#endif
					scale_up (-1);
					accepted = true;
					}
				break;
			case Qt::Key_Down:
				if (Scale_Down_Action->isEnabled ())
					{
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
					OBJECT_CONDITIONAL (
					clog << "        scale_down" << endl;)
					#endif
					scale_down (-1);
					accepted = true;
					}
				break;
			}
		}
	else
	if (Control_Mode == SHIFT_MODE)
		{
		//	Image scrolling mode in effect.
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "      SHIFT_MODE" << endl;)
		#endif
		accepted = true;
		QSize
			offsets (0, 0);
		switch (key)
			{
			case Qt::Key_Up:	offsets.rheight () = +1;	break;
			case Qt::Key_Down:	offsets.rheight () = -1;	break;
			case Qt::Key_Left:	offsets.rwidth ()  = +1;	break;
			case Qt::Key_Right:	offsets.rwidth ()  = -1;
			}
		if (! offsets.isNull ())
			{
			#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
			OBJECT_CONDITIONAL (
			clog << "      offsets = " << offsets << endl;)
			#endif
			modifiers &= ~Qt::KeypadModifier;
			if (modifiers == (Qt::ShiftModifier | Qt::AltModifier) ||
				modifiers == Qt::AltModifier ||
				/*
					Special case: With X11 the Shift-Alt key combination,
					pressed in that order, sets Qt::ShiftModifier and
					Qt::MetaModifier instead of Qt::AltModifier.
				*/
				modifiers == (Qt::ShiftModifier | Qt::MetaModifier))
				{
				//	Page shift.
				offsets.rwidth ()  *= Image_Display->width ();
				offsets.rheight () *= Image_Display->height ();
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "      Shift/Alt Modifier" << endl
					 << "      offsets = " << offsets << endl;)
				#endif
				}
			shift_display (offsets);
			}
		}
	else
		{
		//	Cursor position.
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "      NO_CONTROL_MODE" << endl;)
		#endif
		bool
			nudge = true;
		QPoint
			position (Image_Display->mapFromGlobal (QCursor::pos ()));
		if (Image_Display->rect ().contains (position))
			{
			accepted = true;
			switch (key)
				{
				case Qt::Key_Up:
					if (position.ry () > 0)
						--position.ry ();
					else
						nudge = false;
					break;
				case Qt::Key_Down:
					if (position.ry () < (Image_Display->height () - 1))
						++position.ry ();
					else
						nudge = false;
					break;
				case Qt::Key_Left:
					if (position.rx () > 0)
						--position.rx ();
					else
						nudge = false;
					break;
				case Qt::Key_Right:
					if (position.rx () < (Image_Display->width () - 1))
						++position.rx ();
					else
						nudge = false;
				}
			if (nudge)
				{
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "        set cursor position to " << position << endl;)
				#endif
				QCursor::setPos (Image_Display->mapToGlobal (position));
				accepted = true;
				}
			}
		}
	}
else
if (key == Qt::Key_PageUp ||
	key == Qt::Key_PageDown)
	{
	if (Control_Mode == SHIFT_MODE ||
		Control_Mode == NO_CONTROL_MODE)
		{
		shift_display (QSize (0, ((key == Qt::Key_PageUp) ?
			Image_Display->height () : -Image_Display->height ())));
		accepted = true;
		}
	}
else
//	Ignore (propagate) any other auto-repeat keys.
if (! event->isAutoRepeat ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
	OBJECT_CONDITIONAL (
	clog << "    non-autorepeat" << endl;)
	#endif
	if (modifiers == Qt::ControlModifier &&
		//	No Control/Command operations while control mode is in effect.
		Control_Mode == NO_CONTROL_MODE)
		{
		//	Control/Command key combinations.
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "    control/command key combination" << endl;)
		#endif
		switch (key)
			{
			case Qt::Key_Plus:
			case Qt::Key_Equal:
				if (Scale_Up_Action->isEnabled ())
					{
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
					OBJECT_CONDITIONAL (
					clog << "    key initiated scale_up" << endl;)
					#endif
					Image_Display->setCursor (*Scale_Cursor);
					scale_up ();
					Image_Display->setCursor (*Default_Cursor);
					accepted = true;
					}
				break;
			case Qt::Key_Minus:
				if (Scale_Down_Action->isEnabled ())
					{
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
					OBJECT_CONDITIONAL (
					clog << "    key initiated scale_down" << endl;)
					#endif
					Image_Display->setCursor (*Scale_Cursor);
					scale_down ();
					Image_Display->setCursor (*Default_Cursor);
					accepted = true;
					}
				break;
			case Qt::Key_Home:
			case Qt::Key_1:
				if (Normal_Size_Action->isEnabled ())
					{
					#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
					OBJECT_CONDITIONAL (
					clog << "    key initiated actual_size" << endl;)
					#endif
					Image_Display->setCursor (*Scale_Cursor);
					actual_size ();
					Image_Display->setCursor (*Default_Cursor);
					accepted = true;
					}
				break;
			}
		}
	else
		{
		switch (key)
			{
			//	Check for Control_Mode change.
			case Qt::Key_Shift:
			case Qt::Key_Space:
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "    " << ((key == Qt::Key_Shift) ? "Shift" : "Space")
						<< endl;)
				#endif
				if (modifiers == 0 ||
					(modifiers & (Qt::ShiftModifier | Qt::AltModifier)))
					{
					if (Control_Mode == NO_CONTROL_MODE)
						{
						#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
						OBJECT_CONDITIONAL (
						clog << "    Control_Mode = SHIFT_MODE" << endl;)
						#endif
						Control_Mode = SHIFT_MODE;
						Image_Display->setCursor (*Shift_Cursor);
						accepted = true;
						}
					if (Control_Mode == SCALE_MODE_PENDING)
						{
						#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
						OBJECT_CONDITIONAL (
						clog << "    Control_Mode = SCALE_MODE" << endl;)
						#endif
						Control_Mode = SCALE_MODE;
						Image_Display->setCursor (*Scale_Cursor);
						accepted = true;
						}
					}
				break;
			case Qt::Key_Z:
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "    Z" << endl;)
				#endif
				if (modifiers == Qt::ShiftModifier ||
					modifiers == 0)
					{
					if (Control_Mode == SHIFT_MODE)
						{
						#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
						OBJECT_CONDITIONAL (
						clog << "    Control_Mode = SCALE_MODE" << endl;)
						#endif
						Control_Mode = SCALE_MODE;
						Image_Display->setCursor (*Scale_Cursor);
						}
					else
						{
						#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
						OBJECT_CONDITIONAL (
						clog << "    Control_Mode = SCALE_MODE_PENDING" << endl;)
						#endif
						Control_Mode = SCALE_MODE_PENDING;
						}
					accepted = true;
					}
				break;
			case Qt::Key_Alt:
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "    Alt" << endl;)
				#endif
				if (Control_Mode == SHIFT_MODE)
					//	Shift-Alt page scrolling.
					accepted = true;
				break;
			}
		if (! accepted &&
			Control_Mode != NO_CONTROL_MODE)
			{
			if (Control_Mode == SHIFT_MODE &&
				/*
					Special case: With X11 the Shift-Alt key combination,
					pressed in that order, sets Qt::ShiftModifier and
					Qt::MetaModifier instead of Qt::AltModifier.
				*/
				modifiers == (Qt::ShiftModifier | Qt::MetaModifier))
				//	Alt added to Shift.
				accepted = true;
			else
				{
				//	Now for something completely different.
				#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
				OBJECT_CONDITIONAL (
				clog << "    Control_Mode = NO_CONTROL_MODE" << endl;)
				#endif
				Control_Mode = NO_CONTROL_MODE;
				Image_Display->setCursor (*Default_Cursor);
				releaseKeyboard ();
				}
			}
		else
		if (accepted)
			grabKeyboard ();
		}
	}
event->setAccepted (accepted);
if (! accepted)
	QWidget::keyPressEvent (event);
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::keyPressEvent" << endl;)
#endif
}


void
Image_Viewer::keyReleaseEvent
	(
	QKeyEvent*	event
	)
{
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
OBJECT_CONDITIONAL (
clog << ">>> Image_Viewer::keyReleaseEvent:" << endl
	 << "          key = 0x" << hex << event->key () << endl
	 << "    modifiers = 0x" << event->modifiers () << dec << endl
	 << "       repeat = " << boolalpha << event->isAutoRepeat () << endl;)
#endif
if (! event->isAutoRepeat ())
	{
	int
		mode = -1;
	switch (event->key ())
		{
		case Qt::Key_Shift:
		case Qt::Key_Space:
			if (Control_Mode == SCALE_MODE)
				mode = SCALE_MODE_PENDING;
			else
			if (Control_Mode == SHIFT_MODE)
				mode = NO_CONTROL_MODE;
			break;
		case Qt::Key_Z:
			if (Control_Mode == SCALE_MODE)
				mode = SHIFT_MODE;
			else
			if (Control_Mode != NO_CONTROL_MODE)
				mode = NO_CONTROL_MODE;
		}
	if (mode >= 0)
		{
		#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
		OBJECT_CONDITIONAL (
		clog << "    Control_Mode = "
				<< control_mode_description (mode) << endl;)
		#endif
		Control_Mode = mode;
		change_cursor ();
		if (mode == NO_CONTROL_MODE)
			releaseKeyboard ();
		}
	}
QWidget::keyReleaseEvent (event);
#if ((DEBUG_SECTION) & (DEBUG_EVENTS | DEBUG_KEY_EVENTS))
OBJECT_CONDITIONAL (
clog << "<<< Image_Viewer::keyReleaseEvent" << endl;)
#endif
}


void
Image_Viewer::default_cursor
	(
	QCursor*	cursor
	)
{
if (! cursor)
	cursor = Reticule_Cursor;
if (Default_Cursor != cursor)
	{
	Default_Cursor = cursor;
	if (Control_Mode == NO_CONTROL_MODE)
		Image_Display->setCursor (*Default_Cursor);
	}
}


void
Image_Viewer::change_cursor ()
{
QCursor
	*cursor;
switch (Control_Mode)
	{
	case SHIFT_MODE:	cursor = Shift_Cursor; break;
	case SCALE_MODE:	cursor = Scale_Cursor; break;
	default:			cursor = Default_Cursor;
	}
Image_Display->setCursor (*cursor);
}


QString
Image_Viewer::control_mode_description
	(
	int		control_mode
	)
{
QString
	description ("unknown");
switch (control_mode)
	{
	case NO_CONTROL_MODE:		description = "NO_CONTROL_MODE"; break;
	case SHIFT_MODE:			description = "SHIFT_MODE"; break;
	case SCALE_MODE:			description = "SCALE_MODE"; break;
	case SCALE_MODE_PENDING:	description = "SCALE_MODE_PENDING";
	}
description += " (";
description += QString::number (control_mode);
description += ')';
return description;
}


/*==============================================================================
	Utilities
*/
void
Image_Viewer::error_message
	(
	QErrorMessage*	dialog
	)
{
//	Share the Error_Message dialog with the Tiled_Image_Display.
Tiled_Image_Display::error_message (Error_Message = dialog);
}
    
/*==============================================================================
    World Information
 */
    void Image_Viewer::projection( Projection* projector)
    {
        if(Projector != projector) {
            Projector = projector;
            Times_Copied = 0;
        }
    }


}	//	namespace HiRISE
}	//	namespace UA
