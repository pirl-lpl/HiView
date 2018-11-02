/*	Image_Renderer

HiROC CVS ID: $Id: Image_Renderer.cc,v 1.58 2013/05/22 23:27:23 guym Exp $

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

#include	"Image_Renderer.hh"

#include	"Plastic_Image.hh"
#include	"Plastic_QImage.hh"
#include	"JP2_Image.hh"
#include	"HiView_Utilities.hh"

//	UA::HiRISE::JP2_Reader.
#include	"JP2.hh"
using UA::HiRISE::JP2;
#include	"JP2_Reader.hh"
using UA::HiRISE::JP2_Reader;
#include	"JP2_Exception.hh"
using UA::HiRISE::JP2_Exception;

#include	<QFileInfo>

#include	<cmath>
#include	<stdexcept>
using std::exception;
#include	<cassert>


#if defined (DEBUG_SECTION)
/*******************************************************************************
	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_ACCESSORS			(1 << 1)
#define DEBUG_SLOTS				(1 << 2)
#define DEBUG_SIGNALS			(1 << 3)
#define DEBUG_STATUS			(1 << 4)
#define DEBUG_STATUS_NOTICE		(1 << 5)
#define DEBUG_RENDER			(1 << 6)
#define DEBUG_NOTIFY			(1 << 7)
#define DEBUG_NOTIFY_VISIBLE	(1 << 8)
#define DEBUG_LOAD_IMAGE		(1 << 9)
#define DEBUG_CLONE_IMAGE		(1 << 10)
#define DEBUG_QUEUE				(1 << 11)
#define DEBUG_DELETE_TILES		(1 << 12)
#define DEBUG_HELPERS			(1 << 13)
#define DEBUG_PROMPT			(1 << 14)
#define DEBUG_OVERVIEW			(1 << 15)
#define DEBUG_LOCATION			(1 << 16)
#define DEBUG_IMAGE_ACCOUNTING	(DEBUG_CLONE_IMAGE | \
								 DEBUG_DELETE_TILES)

#define DEBUG_TILE_MARKINGS		(1 << 20)
#define TILE_MARKINGS_COLOR				Qt::green
#define TILE_MARKINGS_ANTE_Y			12
#define TILE_MARKINGS_CANCELED_COLOR	Qt::yellow
#define TILE_MARKINGS_CANCELED_Y		24
#define TILE_MARKINGS_POST_Y			48

#define DEBUG_DEFAULT			DEBUG_ALL

#if (DEBUG_SECTION+0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF

#else
#include	<QThread>

#include	<iostream>
using std::clog;
using std::cin;
#include	<iomanip>
using std::endl;
using std::boolalpha;
using std::flush;
using std::dec;

#if ((DEBUG_SECTION) & (DEBUG_CLONE_IMAGE | DEBUG_QUEUE | DEBUG_DELETE_TILES))
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
	Image_Renderer::ID =
		"UA::HiRISE::Image_Renderer ($Revision: 1.58 $ $Date: 2013/05/22 23:27:23 $)";


const int
	Image_Renderer::DO_NOT_WAIT				= 0,
	Image_Renderer::WAIT_UNTIL_DONE			= (1 << 0),
	Image_Renderer::DELETE_WHEN_DONE		= (1 << 1),
	Image_Renderer::FORCE_CANCEL			= (1 << 2);

const QPoint
	Image_Renderer::LOW_PRIORITY_RENDERING;

const bool
	Image_Renderer::CANCELABLE				= true;

/*==============================================================================
	Class data
*/
QMutex
	Image_Renderer::Rendering_Lock;

/*------------------------------------------------------------------------------
	Defaults
*/
#ifndef DEFAULT_MIN_SOURCE_IMAGE_AREA
#define DEFAULT_MIN_SOURCE_IMAGE_AREA		(1 << 20)
#endif
#ifndef DEFAULT_MAX_SOURCE_IMAGE_AREA
#define DEFAULT_MAX_SOURCE_IMAGE_AREA		(2 << 20)
#endif
unsigned long
	Image_Renderer::Default_Min_Source_Image_Area
		= DEFAULT_MIN_SOURCE_IMAGE_AREA,
	Image_Renderer::Default_Max_Source_Image_Area
		= DEFAULT_MAX_SOURCE_IMAGE_AREA;

#ifndef DEFAULT_WAIT_SECONDS
#define DEFAULT_WAIT_SECONDS				20
#endif
int
	Image_Renderer::Wait_Seconds			= DEFAULT_WAIT_SECONDS;


#ifndef	DOXYGEN_PROCESSING
/*------------------------------------------------------------------------------
	Image rendering monitor.
*/
/**	Rendering Process Monitor

	<b>N.B.</b>: The rendering monitor must be {@link
	add_rendering_monitor(Plastic_Image::Rendering_Monitor*) registered
	with a Plastic_Image to receive its rendering notices. It should be
	(@link remove_rendering_monitor(Plastic_Image::Rendering_Monitor*)
	removed} when notices are no longer to be received and forwarded.

	A Image_Renderer_Rendering_Monitor object registered with a
	Plastic_Image recieves notices at increments during image rendering.
	The notice specifies the status of the rendering process, a status
	message, and the image region that was rendered in the rendering
	increment just completed.
*/
class Image_Renderer_Rendering_Monitor
:	public Plastic_Image::Rendering_Monitor
{
private:

Image_Renderer
	*Owner;

public:

Image_Renderer_Rendering_Monitor
	(
	Image_Renderer*	owner = NULL
	)
{Owner = owner;}


bool
notification
	(
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
					DEBUG_NOTIFY | \
					DEBUG_NOTIFY_VISIBLE | \
					DEBUG_LOCATION))
	Plastic_Image&								image,
	#else
	Plastic_Image&,
	#endif
	Plastic_Image::Rendering_Monitor::Status	status,
	const QString&								message,
	const QRect&								tile_region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
				DEBUG_NOTIFY | \
				DEBUG_NOTIFY_VISIBLE | \
				DEBUG_LOCATION))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (Owner));
#if ((DEBUG_SECTION) & DEBUG_NOTIFY_VISIBLE)
if (Owner->Active_Tile->is_high_priority ())
#endif
clog << "++> Image_Renderer::Image_Renderer_Rendering_Monitor::notification "
		<< thread_ID << endl
	 << "    status " << status << " \"" << message << '"' << endl
	 << "    Owner Image_Renderer @ " << (void*)Owner << ' ' << pathname << endl
	 << "    image @ " << (void*)&image << " - " << image.source_name () << endl
	 << "    tile_region = " << tile_region << endl
	 << "    Active_Tile " << *(Owner->Active_Tile) << endl;
#endif
if (Owner)
	{
	//	>>> SIGNAL <<<
	Owner->send_status_notice (message);

	if (! Owner->Active_Tile->Tile_Region.isEmpty () &&	//	Visible in display?
		(status & Plastic_Image::Rendering_Monitor::RENDERED_DATA_MASK))
		{
		//	>>> SIGNAL <<<
		Owner->send_rendered
			(Owner->Active_Tile->Tile_Coordinate, tile_region);
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
						DEBUG_NOTIFY | \
						DEBUG_NOTIFY_VISIBLE | \
						DEBUG_LOCATION))
		clog << "      notification sent rendered signal" << endl;
		#if ((DEBUG_SECTION) & DEBUG_NOTIFY_VISIBLE)
		if (Owner->Active_Tile->is_high_priority ())
			{
		#endif
			#if ((DEBUG_SECTION) & DEBUG_PROMPT)
			char
				input[4];
			clog << "vis. tile " << Owner->Active_Tile->Tile_Coordinate
				<< " region " << tile_region << " > ";
			cin.getline (input, 2);
			if (input[0] == 'q')
				exit (7);
			#endif
		#if ((DEBUG_SECTION) & DEBUG_NOTIFY_VISIBLE)
			}
		#endif
		#endif
		}
	}
#if ((DEBUG_SECTION) & (DEBUG_RENDER | \
				DEBUG_NOTIFY | \
				DEBUG_NOTIFY_VISIBLE | \
				DEBUG_LOCATION))
clog << "<++ Image_Renderer::Image_Renderer_Rendering_Monitor::notification "
		<< thread_ID << ": true" << endl;
#endif
return true;
}
};	//	Image_Renderer_Rendering_Monitor
#endif


//	Signal emitter proxies for the rendering_montitor:
void
Image_Renderer::send_status_notice
	(
	const QString&	message
	)
{
#if ((DEBUG_SECTION) & (DEBUG_NOTIFY | DEBUG_SIGNALS | DEBUG_STATUS_NOTICE))
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCKED_LOGGING ((
clog << "    Image_Renderer::Image_Renderer_Rendering_Monitor::notification "
		<< thread_ID << ": emit status_notice \"" << message << '"' << endl));
#endif
//	>>> SIGNAL <<<
emit status_notice (message);
}


void
Image_Renderer::send_rendered
	(
	const QPoint&	tile_coordinate,
	const QRect&	tile_region
	)
{
#if ((DEBUG_SECTION) & (DEBUG_NOTIFY | DEBUG_SIGNALS))
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCKED_LOGGING ((
clog << "    Image_Renderer::Image_Renderer_Rendering_Monitor::notification "
		<< thread_ID << ": emit rendered -" << endl
	 << "    tile_coordinate = " << tile_coordinate << endl
	 << "        tile_region = " << tile_region << endl));
#endif
//	>>> SIGNAL <<<
emit rendered (tile_coordinate, tile_region);
}

/*------------------------------------------------------------------------------
	Accounting
*/
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
namespace
{
void
print_queue
	(
	const Image_Renderer::Tile_Queue&	queue
	)
{
clog << "    " << queue.size () << " queue entries";
if (queue.size ())
	{
	clog << " -" << endl
		 << "    -------------------------------------" << endl;
	for (int index = 0;
			 index < queue.size ();
			 index++)
		clog << "      " << index << ": " << *queue[index] << endl;
	clog << "    -------------------------------------" << endl;
	}
else
	clog << endl;
}
}


void
Image_Renderer::print_render_queue () const
{
QString
	pathname (object_pathname (this));
bool
	locked = Queue_Lock.tryLock ();
LOCK_LOG;
if (Active_Tile)
	clog << "    Image_Renderer::Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Image_Renderer::Render_Queue" << endl
	 << "    in " << pathname << endl;
print_queue (Render_Queue);
UNLOCK_LOG;
if (locked)
	Queue_Lock.unlock ();
}

void
Image_Renderer::print_delete_queue () const
{
QString
	pathname (object_pathname (this));
bool
	locked = Queue_Lock.tryLock ();
LOCK_LOG;
clog << "    Image_Renderer::Delete_Queue" << endl
	 << "    in " << pathname << endl;
print_queue (Delete_Queue);
UNLOCK_LOG;
if (locked)
	Queue_Lock.unlock ();
}

#else
void
Image_Renderer::print_render_queue () const
{}

void
Image_Renderer::print_delete_queue () const
{}
#endif


#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
namespace
{
QList<Plastic_Image*>
	Image_Accounting;
}

void
Image_Renderer::image_accounting ()
{
clog << "    " << Image_Accounting.size () << " extant Plastic_Images";
if (Image_Accounting.size ())
	{
	clog << " -" << endl
		 << "    -------------------------------------" << endl;
	for (int
			index = 0;
			index < Image_Accounting.size ();
			index++)
		{
		if (Image_Accounting[index] == Source_Image)
			clog << "SSS ";
		else
		if (Image_Accounting[index] == Reference_Image)
			clog << "RRR ";
		else
			clog << "    ";
		clog << index << ": " << *Image_Accounting[index] << endl;
		}
	clog << "    -------------------------------------" << endl;
	}
else
	clog << endl;
}

#else
void
Image_Renderer::image_accounting ()
{}
#endif

/*==============================================================================
	Constructors
*/
Image_Renderer::Image_Renderer
	(
	QObject*	parent
	)
	:
	QObject (parent),
	Suspended (true),
	Runnable (false),
	Finish (false),
	Immediate_Mode (false),
	Source_Image (new Plastic_QImage ()),
	Max_Source_Image_Area (Default_Max_Source_Image_Area),
	Reference_Image (Source_Image->clone (QSize (0, 0))),
	Active_Tile (NULL),
	Cancel (false),
	Image_Rendering_Monitor (new Image_Renderer_Rendering_Monitor (this))
{
setObjectName ("Image_Renderer");
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_IMAGE_ACCOUNTING))
LOCKED_LOGGING ((
clog << ">-< Image_Renderer @ " << (void*)this
		<< ": " << object_pathname (this) << endl
	 << "       initial Source_Image " << *Source_Image << endl
	 << "    initial Reference_Image " << *Reference_Image << endl));
#endif
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
if (Reference_Image)
	Image_Accounting.append (Reference_Image);
#endif
}


Image_Renderer::~Image_Renderer()
{
#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
QString
	pathname (object_pathname (this));
#endif
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_IMAGE_ACCOUNTING))
LOCKED_LOGGING ((
clog << ">>> ~Image_Renderer @ " << (void*)this
		<< ": " << pathname << endl));
#endif
/*	>>> WARNING <<< This Image_Renderer is destroyed AFTER any derived
	classes. This creates complications for the order in which finish
	operations are done. It is assumed that derived class destructors
	will call this base class finish method. After the base class is
	destroyed it may not be possible to call the virtual finish method,
	and the local finish method will call the virtual start_rendering
	method. So, to avoid this snafu the finish mehtod is only called if
	it has not yet been called (Finish is not set). This has the
	additional benefit of avoiding a potential redundant finish.
*/
if (! Finish)
	finish ();

if (Reference_Image)
	{
	#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_IMAGE_ACCOUNTING))
	LOCKED_LOGGING ((
	clog << "    delete the Reference_Image -" << endl
		 << "    " << *Reference_Image << endl));
	#endif
	delete Reference_Image;
	#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
	if (! Image_Accounting.removeAll (Reference_Image))
		{
		LOCKED_LOGGING ((
		clog << "!!! Reference_Image not accounted for" << endl
			 << "    in " << pathname << endl));
		}
	#endif
	}

if (Image_Rendering_Monitor)
	delete Image_Rendering_Monitor;
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_IMAGE_ACCOUNTING))
print_render_queue ();
print_delete_queue ();
LOCK_LOG;
image_accounting ();
clog << "    in " << pathname << endl
	 << "<<< ~Image_Renderer @ " << (void*)this << endl;
UNLOCK_LOG;
#endif
}

/*==============================================================================
	Accessors
*/
const Image_Renderer::Shared_Image
Image_Renderer::source_image () const
{
QMutexLocker
	queue_lock (&Queue_Lock);
return Source_Image;
}


Plastic_Image*
Image_Renderer::reference_image () const
{
QMutexLocker
	queue_lock (&Queue_Lock);
return Reference_Image;
}


void
Image_Renderer::immediate_mode
	(
	bool	enable
	)
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << ">-< Image_Renderer::immediate_mode "
		<< (void*)QThread::currentThreadId ()
		<< ": " << boolalpha << enable << endl));
#endif
QMutexLocker
	mode_lock (&Mode_Lock);
Immediate_Mode = enable;
}


bool
Image_Renderer::immediate_mode () const
{
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::immediate_mode " << thread_ID << endl));
#endif
QMutexLocker
	mode_lock (&Mode_Lock);
bool
	immediate_mode = Immediate_Mode;
#if ((DEBUG_SECTION) & DEBUG_ACCESSORS)
LOCKED_LOGGING ((
clog << "<<< Image_Renderer::immediate_mode " << thread_ID
	 	<< ": " << boolalpha << immediate_mode << endl));
#endif
return immediate_mode;
}

/*==============================================================================
	Queue management
*/
void
Image_Renderer::queue
	(
	Plastic_Image*	image,
	const QPoint&	tile_coordinate,
	const QRect&	tile_region,
	bool			cancelable
	)
{
if (! image)
	return;

Image_Tile
	*image_tile =
		new Image_Tile (image, tile_coordinate, tile_region, cancelable);
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::queue " << thread_ID << ": image -" << endl
	 << "    " << *image << endl
	 << "    " << *image_tile << endl
	 << "    lock Queue_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Queue_Lock.lock ();

int
	index = find_tile (image, Render_Queue);
if (index < 0)
	{
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::queue " << thread_ID
			<< ": adding new tile" << endl
		 << "    in " << pathname << endl));
	#endif
	add_tile (image_tile);
	}
else
	{
	//	The image is already in the queue.
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::queue " << thread_ID
			<< ": already in Render_Queue[" << index << "] -" << endl
		 << "    " << *Render_Queue[index] << endl
		 << "    in " << pathname << endl));
	#endif
	if (image_tile->status () == Render_Queue[index]->status () &&
		image_tile->area ()   == Render_Queue[index]->area ())
		{
		//	No priority change; replace the current entry.
		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::queue " << thread_ID
				<< ": replace Render_Queue[" << index << "] -" << endl
			 << "    " << *Render_Queue[index] << endl
			 << "    in " << pathname << endl));
		#endif
		//	Delete the existing tile.
		Render_Queue[index]->Delete_Image_When_Done = false;
		delete Render_Queue[index];
		//	Replace the queue entry with the new tile.
		Render_Queue[index] = image_tile;
		}
	else
		{
		//	Change of priority; remove current entry and add new tile.
		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::queue " << thread_ID
				<< ": remove and delete Render_Queue[" << index << "] -" << endl
			 << "    " << *Render_Queue[index] << endl
			 << "    in " << pathname << endl));
		#endif
		//	Delete the existing tile.
		Render_Queue[index]->Delete_Image_When_Done = false;
		delete Render_Queue.takeAt (index);
		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::queue " << thread_ID
				<< ": add new tile" << endl
			 << "    in " << pathname << endl));
		#endif
		add_tile (image_tile);
		}
	}
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCK_LOG;
clog << "    Image_Renderer::queue " << thread_ID << endl
	 << "    in " << pathname << endl;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Render_Queue -" << endl;
print_queue (Render_Queue);
clog << "    Image_Renderer::queue " << thread_ID
		<< ": unlock Queue_Lock" << endl
	 << "    in " << pathname << endl;
UNLOCK_LOG;
#endif
Queue_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::queue " << thread_ID << endl));
#endif
}


bool
Image_Renderer::is_queued
	(
	Plastic_Image* image
	) const
{
QMutexLocker
	queue_lock (&Queue_Lock);
bool
	queued;
if (image)
	queued = find_tile (image, Render_Queue) >= 0;
else
	queued = Render_Queue.count () != 0;
return queued;
}


int
Image_Renderer::rendering_status () const
{
QMutexLocker
	queue_lock (&Queue_Lock);
int
	priority_status = 0;
if (Active_Tile)
	priority_status = Active_Tile->status ();
else
if (! Render_Queue.isEmpty ())
	priority_status = Render_Queue.first ()->status ();
return priority_status;
}


bool
Image_Renderer::is_rendering
	(
	Plastic_Image*	image
	) const
{
QMutexLocker
	queue_lock (&Queue_Lock);
bool
	rendering;
if (image)
	rendering =
		Active_Tile &&
		Active_Tile->Image == image;
else
	rendering = Active_Tile != NULL;
return rendering;
}


bool
Image_Renderer::will_delete
	(
	Plastic_Image*	image
	) const
{
QMutexLocker
	queue_lock (&Queue_Lock);
return find_tile (image, Delete_Queue) >= 0;
}


bool
Image_Renderer::cancel
	(
	Plastic_Image* image,
	int				cancel_options
	)
{
if (! image)
	return false;

#if ((DEBUG_SECTION) & DEBUG_QUEUE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCK_LOG;
clog << ">>> Image_Renderer::cancel " << thread_ID << ": image -" << endl
	 << "    " << *image << endl
	 << "    cancel_options = " << cancel_options
		<< " - " << cancel_options_descriptions (cancel_options) << endl
	 << "    lock Queue_Lock" << endl;
UNLOCK_LOG;
#endif
Queue_Lock.lock ();
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << "    Image_Renderer::cancel " << thread_ID << endl
	 << "    in " << pathname << endl;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Render_Queue -" << endl;
print_queue (Render_Queue);
UNLOCK_LOG;
#endif
bool
	canceled = true;

if (Active_Tile &&
	Active_Tile->Image == image)
	{
	//	Cancel rendering of the active tile.
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::cancel " << thread_ID
			<< ": abort" << endl));
	#endif
	canceled = abort (cancel_options);
	}

int
	index = Render_Queue.size ();
while (index--)
	{
	if (Render_Queue[index]->Image == image)
		{
		if (cancel_options & DELETE_WHEN_DONE)
			//	Mark the tile image for deletion when the tile is destroyed.
			Render_Queue[index]->Delete_Image_When_Done = true;

		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::cancel " << thread_ID
				<< ": queue for deletion Render_Queue[" << index << "] -"
				<< endl
			 << "    " << *Render_Queue[index] << endl));
		#endif
		delete_tile (Render_Queue.takeAt (index));
		}
	}
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCK_LOG;
clog << "    Image_Renderer::cancel " << thread_ID << endl;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Render_Queue after canceling image @ "
		<< (void*)image << " -" << endl;
print_queue (Render_Queue);
clog << "    Image_Renderer::cancel " << thread_ID
		<< ": unlock Queue_Lock" << endl;
UNLOCK_LOG;
#endif
Queue_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCKED_LOGGING ((
clog << "<<< Image_Renderer::cancel " << thread_ID
		<< ": " << boolalpha << canceled << endl));
#endif
return canceled;
}


bool
Image_Renderer::cancel
	(
	int		cancel_options
	)
{
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCK_LOG;
clog << ">>> Image_Renderer::cancel " << thread_ID
		<< ": " << cancel_options
		<< " - " << cancel_options_descriptions (cancel_options) << endl
	 << "    lock Queue_Lock" << endl;
UNLOCK_LOG;
#endif
Queue_Lock.lock ();
bool
	done = clear (cancel_options);
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "    Image_Renderer::cancel " << thread_ID
		<< ": unlock Queue_Lock" << endl));
#endif
Queue_Lock.unlock ();
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "<<< Image_Renderer::cancel " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


bool
Image_Renderer::reset
	(
	int		cancel_options
	)
{
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this)),
	description (cancel_options_descriptions (cancel_options));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::reset " << thread_ID
		<< ": " << cancel_options << " - " << description << endl
	 << "    in " << pathname << endl
	 << "    lock Queue_Lock" << endl));
#endif
Queue_Lock.lock ();
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::reset " << thread_ID << ": stop rendering" << endl
	 << "    in " << pathname << endl));
#endif
stop_rendering ();
bool
	done = clear (cancel_options);
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::reset " << thread_ID
		<< ": unlock Queue_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Queue_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::reset " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


int
Image_Renderer::find_tile
	(
	Plastic_Image*		image,
	const Tile_Queue&	queue
	)
{
int
	index = queue.size ();
while (index)
	if (queue[--index]->Image == image)
		return index;
return -1;
}


void
Image_Renderer::add_tile
	(
	Image_Tile*		image_tile
	)
{
//	>>> CAUTION: The Queue_Lock is expected to be locked.
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << ">>> Image_Renderer::add_tile " << thread_ID
		<< ": " << *image_tile << endl
	 << "    in " << pathname << endl;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Render_Queue -" << endl;
print_queue (Render_Queue);
UNLOCK_LOG;
#endif
//	Safety check for the image in the Delete_Queue.
int
	index = find_tile (image_tile->Image, Delete_Queue);
if (index >= 0)
	{
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "!!! remove image of tile to add from the Delete_Queue -" << endl
		 << "    " << *Delete_Queue[index] << endl));
	#endif
	//	Retain the Delete_Image_When_Done if set in either tile.
	image_tile->Delete_Image_When_Done
		|= Delete_Queue[index]->Delete_Image_When_Done;
	//	Remove the tile from the Delete_Queue.
	Delete_Queue[index]->Delete_Image_When_Done = false;
	delete Delete_Queue.takeAt (index);
	}

//	Safety check for the image in the Active_Tile.
if (Active_Tile &&
	Active_Tile->Image == image_tile->Image)
	{
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    add tile with image of Active_Tile -" << endl
		 << "    " << *Active_Tile << endl));
	#endif
	image_tile->Delete_Image_When_Done
		|= Active_Tile->Delete_Image_When_Done;
	//	Image will be in the Render_Queue.
	Active_Tile->Delete_Image_When_Done = false;
	}

if (Render_Queue.isEmpty () ||
	image_tile->is_low_priority () ||
	image_tile->Tile_Region.isEmpty ())
	{
	//	Empty queue or low priority tile.
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::add_tile " << thread_ID
			<< ": append " << (image_tile->is_low_priority () ?
				"low priority " : "")
			<< "tile to " << (Render_Queue.isEmpty () ? "empty " : "")
			<< "Render_Queue" << endl
		 << "    in " << pathname << endl));
	#endif
	Render_Queue.append (image_tile);
	}
else
	{
	/*
		Insert the high priority tile before the first tile that is not high
		priority or has a visible region area less that or equal to that
		of the tile to be added to the queue; i.e. the high priority
		section of the queue is to be ordered from high to low visible
		tile region area.
	*/
	unsigned long long
		tile_area = image_tile->area ();
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    tile_area = " << tile_area << endl));
	#endif
	int
		tiles = Render_Queue.size (),
		index = -1;
	while (++index < tiles &&
		 	Render_Queue.at (index)->is_high_priority () &&
			Render_Queue.at (index)->area () > tile_area)
		{
		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    " << index << ": " << *(Render_Queue.at (index))
				<< ", area = " << Render_Queue.at (index)->area () << endl));
		#endif
		}
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::add_tile " << thread_ID
			<< ": insert tile at Render_Queue index " << index << endl
		 << "    in " << pathname << endl));
	#endif
	Render_Queue.insert (index, image_tile);
	}

if (runnable ())
	{
	//	Start rendering.
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::add_tile " << thread_ID
			<< ": start rendering" << endl
		 << "    in " << pathname << endl));
	#endif
	start_rendering ();
	}
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCK_LOG;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Render_Queue -" << endl;
print_queue (Render_Queue);
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::add_tile " << thread_ID << endl;
UNLOCK_LOG;
#endif
}


bool
Image_Renderer::clear
	(
	int		cancel_options
	)
{
//	>>> CAUTION: The Queue_Lock is expected to be locked.
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this)),
	description (cancel_options_descriptions (cancel_options));
LOCK_LOG;
clog << ">>> Image_Renderer::clear " << thread_ID
		<< ": " << cancel_options << " - " << description << endl
	 << "    in " << pathname << endl;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    Render_Queue -" << endl;
print_queue (Render_Queue);
UNLOCK_LOG;
#endif
bool
	//	Cancel rendering of the active tile.
	done = abort (cancel_options);

int
	index = Render_Queue.size ();
while (index--)
	{
	if (Render_Queue[index]->Cancelable ||
		(cancel_options & FORCE_CANCEL))
		{
		if (cancel_options & DELETE_WHEN_DONE)
			//	Mark the tile image for deletion when the tile is destroyed.
			Render_Queue[index]->Delete_Image_When_Done = true;

		#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::clear " << thread_ID
				<< ": queue for deletion Render_Queue[" << index << "] -"
				<< endl
			 << "    " << *Render_Queue[index] << endl
			 << "    in " << pathname << endl));
		#endif
		delete_tile (Render_Queue.takeAt (index));
		}
	}

#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
LOCK_LOG;
clog << "    Image_Renderer::clear " << thread_ID << endl
	 << "    in " << pathname << endl;
if (Active_Tile)
	clog << "    Active_Tile -" << endl
		 << "    " << *Active_Tile << endl;
clog << "    cleared Render_Queue -" << endl;
print_queue (Render_Queue);
UNLOCK_LOG;
#endif

//	Clear the Delete_Queue.
delete_tiles ();

if (! done &&
	(cancel_options & WAIT_UNTIL_DONE))
	{
	#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE | DEBUG_SIGNALS))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::clear " << thread_ID << ": emit error - "
			"\"Image rendering did not complete!\"" << endl
		 << "    in " << pathname << endl));
	#endif
	/*	>>> SIGNAL <<<
	emit error ("Image rendering did not complete when canceled!");
   */
	}
#if ((DEBUG_SECTION) & (DEBUG_SLOTS | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::clear " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


bool
Image_Renderer::abort
	(
	int		cancel_options
	)
{
//	>>> CAUTION: The Queue_Lock is expected to be locked.
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this)),
	description (cancel_options_descriptions (cancel_options));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::abort " << thread_ID
		<< ": " << cancel_options << " - " << description << endl
	 << "    in " << pathname << endl));
#endif
bool
	done = true;
if (Active_Tile)
	{
	#if ((DEBUG_SECTION) & DEBUG_QUEUE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::abort " << thread_ID
			<< ": cancel rendering for " << *Active_Tile << endl
		 << "    in " << pathname << endl));
	#endif
	//	Raise the canceled flag for rendered tile disposition.
	Cancel = true;
	//	Notify the image rendering machinery to cancel operations.
	Active_Tile->Image->cancel_update ();

	if (cancel_options & DELETE_WHEN_DONE)
		//	Mark the tile image for deletion when the tile is destroyed.
		Active_Tile->Delete_Image_When_Done = true;

	if (cancel_options & WAIT_UNTIL_DONE)
		{
		//	Wait for any rendering to complete.
		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::abort " << thread_ID
				<< ": try lock Rendering_Lock" << endl
			 << "    in " << pathname << endl
			 << "    wait up to " << Wait_Seconds << " seconds" << endl));
		#endif
		if (Rendering_Lock.tryLock (Wait_Seconds * 1000))
			{
			#if ((DEBUG_SECTION) & DEBUG_QUEUE)
			LOCKED_LOGGING ((
			clog << "    Image_Renderer::abort " << thread_ID
					<< ": unlock Rendering_Lock" << endl
				 << "    in " << pathname << endl));
			#endif
			Rendering_Lock.unlock ();
			}
		else
			{
			#if ((DEBUG_SECTION) & DEBUG_QUEUE)
			LOCKED_LOGGING ((
			clog << "!!! Image_Renderer::abort " << thread_ID
					<< ": failed to obtain Rendering_Lock after "
					<< Wait_Seconds << " seconds!" << endl
				 << "    in " << pathname << endl));
			#endif
			done = false;
			}
		}
	else
		{
		//	Didn't wait and rendering is in progress.
		#if ((DEBUG_SECTION) & DEBUG_QUEUE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::abort " << thread_ID
				<< ": didn't wait" << endl
			 << "    in " << pathname << endl));
		#endif
		done = false;
		}
	}
#if ((DEBUG_SECTION) & DEBUG_QUEUE)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::abort " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


bool
Image_Renderer::delete_image
	(
	Plastic_Image*	image
	)
{
if (! image)
	return false;

#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
void*
	thread_ID = (void*)QThread::currentThreadId ();
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::delete_image " << thread_ID << ": image -" << endl
	 << "    " << *image << endl
	 << "    lock Queue_Lock" << endl));
#endif
Queue_Lock.lock ();
bool
	deleted = false;
int
	index = find_tile (image, Render_Queue);
if (index >= 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::delete_image " << thread_ID
			<< ": queue for deletion Render_Queue[" << index << "] -" << endl
		 << "    " << *Render_Queue[index] << endl));
	#endif
	Render_Queue[index]->Delete_Image_When_Done = true;
	delete_tile (Render_Queue.takeAt (index));
	deleted = true;
	}

if (Active_Tile &&
	Active_Tile->Image == image)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::delete_image " << thread_ID
			<< ": Active_Tile -" << endl
		 << "    " << *Active_Tile << endl));
	#endif
	//	Delete when done if not in the Render_Queue.
	Active_Tile->Delete_Image_When_Done = ! deleted;
	//	The Active_Tile is put on the Delete_Queue when rendering is done.
	deleted = true;
	}
if (! deleted)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::delete_image " << thread_ID
			<< ": tile image and queue for deletion" << endl));
	#endif
	delete_tile (new Image_Tile (image, QPoint (), true, true));
	}
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "    Image_Renderer::delete_image " << thread_ID
		<< ": unlock Queue_Lock" << endl));
#endif
Queue_Lock.unlock ();
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "<<< Image_Renderer::delete_image " << thread_ID
		<< ": " << boolalpha << deleted << endl));
#endif
return deleted;
}


void
Image_Renderer::delete_tile
	(
	Image_Tile*		image_tile
	)
{
//	>>> CAUTION: The Queue_Lock is expected to be locked.
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::delete_tile " << thread_ID
		<< ": " << *image_tile << endl
	 << "    in " << pathname << endl));
#endif

//	Safety check for the image in the Render_Queue.
int
	index = find_tile (image_tile->Image, Render_Queue);
if (index >= 0 &&
	image_tile->Delete_Image_When_Done)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "!!! don't delete image in Render_Queue[" << index << "] -" << endl
		 << "    " << *Render_Queue[index] << endl));
	#endif
	image_tile->Delete_Image_When_Done = false;
	}

index = Delete_Queue.size ();
while (index--)
	{
	if (Delete_Queue[index]->Image == image_tile->Image)
		{
		//	The tile Image is already in the Delete_Queue.
		#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::delete_tile " << thread_ID
				<< ": tile image already queued for deletion -" << endl
			 << "    " << *(image_tile->Image) << endl));
		#endif
		if (Delete_Queue[index] != image_tile)
			{
			//	Different tiles.
			#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
			LOCKED_LOGGING ((
			clog << "    Image_Renderer::delete_tile " << thread_ID
					<< ": different image tile -" << endl
				 << "    " << *Delete_Queue[index] << endl));
			#endif
			if (image_tile->Delete_Image_When_Done)
				{
				Delete_Queue[index]->Delete_Image_When_Done = true;
				image_tile->Delete_Image_When_Done = false;
				}
			#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
			LOCKED_LOGGING ((
			clog << "    Image_Renderer::delete_tile " << thread_ID
					<< ": delete image tile" << endl));
			#endif
			delete image_tile;
			}
		#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
		else
			{
			LOCKED_LOGGING ((
			clog << "    Image_Renderer::delete_tile " << thread_ID
					<< ": tile already queued for deletion" << endl));
			}
		#endif
		return;
		}
	}
//	Queue for deletion of the tile and its image.
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "    Image_Renderer::delete_tile " << thread_ID
		<< ": queue for deletion" << endl));
#endif
Delete_Queue.append (image_tile);
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCK_LOG;
clog << "    Delete_Queue -" << endl;
print_queue (Delete_Queue);
clog << "<<< Image_Renderer::delete_tile " << thread_ID << endl;
UNLOCK_LOG;
#endif
}


void
Image_Renderer::delete_tiles ()
{
//	>>> CAUTION: The Queue_Lock is expected to be locked by the Renderer thread.
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::delete_tiles " << thread_ID << endl
	 << "    in " << pathname << endl));
#endif
bool
	locked = Queue_Lock.tryLock ();
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCK_LOG;
clog << "    Image_Renderer::delete_tiles " << thread_ID
		<< ": Queue_Lock was " << (locked ? "not " : "") << "locked" << endl
	 << "    in " << pathname << endl
	 << "    Delete_Queue -" << endl;
print_queue (Delete_Queue);
UNLOCK_LOG;
#endif
int
	index = Delete_Queue.size ();
while (--index >= 0)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "    delete " << *Delete_Queue[index] << endl));
	#endif
	#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
	if (Delete_Queue[index]->Delete_Image_When_Done &&
		! Image_Accounting.removeAll (Delete_Queue[index]->Image))
		{
		LOCKED_LOGGING ((
		clog << "!!! image to be deleted not accounted for!" << endl));
		}
	#endif
	delete Delete_Queue.takeAt (index);
	}
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCK_LOG;
image_accounting ();
UNLOCK_LOG;
#endif
if (locked)
	{
	#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::delete_tiles " << thread_ID
		 	<< ": unlock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.unlock ();
	}
#if ((DEBUG_SECTION) & (DEBUG_DELETE_TILES | DEBUG_QUEUE))
LOCKED_LOGGING ((
clog << "<<< Image_Renderer::delete_tiles " << thread_ID << endl));
#endif
}


void
Image_Renderer::clean_up ()
{
Queue_Lock.lock ();
delete_tiles ();
Queue_Lock.unlock ();
}

/*==============================================================================
	Rendering
*/
void
Image_Renderer::run_rendering ()
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::run_rendering " << thread_ID << endl
	 << "    in " << pathname << endl));
#endif
Finish = false;

/*	In order to prevent a deadlock in the render method due to an
	attempt to lock the Queue_Lock which may have already been locked
	in the calling, synchronous, context, the Queue_Lock needs to be
	unlocked here, if locked, before render is entered and relocked
	afterwards so the caller's expectation of needing to unlock it
	can be met.
*/
bool
	locked = Queue_Lock.tryLock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer::run_rendering " << thread_ID
		<< ": Queue_Lock was " << (locked ? "not " : "") << "locked" << endl
	 << "    in " << pathname << endl));
#endif
Queue_Lock.unlock ();	

#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer::run_rendering " << thread_ID
		<< ": enter the rendering loop ..." << endl
	 << "    in " << pathname << endl));
#endif
render ();

if (! locked)
	{
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::run_rendering " << thread_ID
			<< ": relock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.lock ();
	}
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::run_rendering " << thread_ID << endl));
#endif
}


bool
Image_Renderer::is_ready ()
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS | DEBUG_STATUS))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
#endif
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::is_ready " << thread_ID << endl
	 << "    lock Ready_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Ready_Lock.lock ();
if (Suspended ||
	Finish)
	{
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS | DEBUG_STATUS))
	LOCKED_LOGGING ((
	clog << "^^^ Image_Renderer::is_ready " << thread_ID
			<< ": emit status " << NOT_RENDERING << " - "
			<< status_description (NOT_RENDERING) << endl
		 << "    in " << pathname << endl));
	#endif
	//	>>> SIGNAL <<<
	emit status (NOT_RENDERING);
	}
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer::is_ready " << thread_ID
		<< ": unlock Ready_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Ready_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::is_ready " << thread_ID
	 	<< ": " << boolalpha << (Runnable && ! Finish) << endl));
#endif
return (Runnable && ! Finish);
}


void
Image_Renderer::render ()
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW | DEBUG_QUEUE | DEBUG_STATUS))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this)),
	name;
#endif
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::render " << thread_ID << endl
	 << "    in " << pathname << endl));
#endif
#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
QString
	label;
#endif
int
	tile_status;
bool
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_TILE_MARKINGS))
	complete,
	#endif
	canceled;

while (true)
	{
	if (! is_ready ())
		//	Finish.
		break;

	//	Check for source image loading.
	load_image ();

	if (Finish)	//	In case finish was called during image_load.
		break;

	//	------------------------------------------------------------------------
	//	Acquire the Queue_Lock.
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": lock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.lock ();

	if (Render_Queue.isEmpty ())
		{
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::render " << thread_ID
				<< ": empty Render_Queue" << endl
			 << "    suspend_rendering" << endl
			 << "    in " << pathname << endl));
		#endif
		suspend_rendering ();
		#if ((DEBUG_SECTION) & DEBUG_RENDER)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::render " << thread_ID
				<< ": unlock Queue_Lock" << endl
			 << "    in " << pathname << endl));
		#endif
		Queue_Lock.unlock ();
		continue;
		}
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_QUEUE | DEBUG_OVERVIEW))
	LOCK_LOG;
	clog << "--> Image_Renderer::render " << thread_ID << endl
		 << "    in " << pathname << endl;
	if (Active_Tile)
		clog << "    Active_Tile -" << endl
			 << "    " << *Active_Tile << endl;
	clog << "    Render_Queue -" << endl;
	print_queue (Render_Queue);
	UNLOCK_LOG;
	#endif

	/*	Acquire the Active_Tile from the front of the Render_Queue.

		A non-NULL Active_Tile is used as a flag that an image tile has
		been acquired for rendering. When rendering of the Active_Tile
		is complete it will be deleted and the Active_Tile reset to NULL.
	*/
	Active_Tile = Render_Queue.takeAt (0);
	tile_status = Active_Tile->status ();
	canceled = Cancel;
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_TILE_MARKINGS))
	complete = false;
	#endif
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_QUEUE | DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": Active_Tile " << *Active_Tile << endl
		 << "    " << *(Active_Tile->Image) << endl
		 << "    canceled = " << boolalpha << canceled << endl
		 << "    in " << pathname << endl));
	#endif
	#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
	//	Tile markings.
	label = QString (" ante render @ %1: gc %2, %3; so %4, %5; s %6")
		.arg ((ulong)(Active_Tile->Image), 0, 16)
		.arg (Active_Tile->Tile_Coordinate.rx ())
		.arg (Active_Tile->Tile_Coordinate.ry ())
		.arg (Active_Tile->Image->source_origin ().x ())
		.arg (Active_Tile->Image->source_origin ().y ())
		.arg (Active_Tile->Image->source_scaling ().width ());
	mark_image (Active_Tile->Image, label,
		TILE_MARKINGS_ANTE_Y,
		TILE_MARKINGS_COLOR);
	mark_image (Active_Tile->Image, label,
		Active_Tile->Image->height () - TILE_MARKINGS_ANTE_Y - 12,
		TILE_MARKINGS_COLOR);
	#endif

	//	Rendering begins .......................................................
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": lock Rendering_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Rendering_Lock.lock ();

	//	Register the Image_Rendering_Monitor with the tile's image.
	Active_Tile->Image->add_rendering_monitor (Image_Rendering_Monitor);

	/*	Release the Queue_Lock during rendering.

		>>> WARNING <<< The Queue_Lock must be released AFTER the
		Rendering_Lock is acquired to avoid a race condition in abort.
	*/
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": unlock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.unlock ();

	if (! canceled)
		{
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS | DEBUG_STATUS))
		LOCKED_LOGGING ((
		clog << "^^^ Image_Renderer::render " << thread_ID
				<< ": emit status " <<tile_status << " - "
				<< status_description (tile_status) << endl
			 << "    in " << pathname << endl));
		#endif
		//	>>> SIGNAL <<<
		emit status (tile_status);

		//	Render the tile.
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "==> Image_Renderer::render " << thread_ID
				<< ": rendering -" << endl
			 << "    update " << *Active_Tile << endl
			 << "      " << *(Active_Tile->Image) << endl
			 << "    in " << pathname << endl));
		#endif
		try {
			#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_TILE_MARKINGS))
			complete =
			#endif
			Active_Tile->Image->update ();
			}
		catch (Plastic_Image::Render_Exception& except)
			{
			#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS))
			LOCKED_LOGGING ((
			clog << "^^^ Image_Renderer::render " << thread_ID
					<< ": emit error -" << endl
				 << except.message () << endl
				 << "    in " << pathname << endl));
			#endif
			//	>>> SIGNAL<<<
			emit error (QString::fromStdString (except.message ()));
			reset (DO_NOT_WAIT);
			canceled = true;
			}
		#if ((DEBUG_SECTION) & DEBUG_RENDER)
		LOCKED_LOGGING ((
		clog << "<== Image_Renderer::render " << thread_ID
				<< ": rendering complete "
				<< boolalpha << complete << endl
			 << "    for " << *Active_Tile << endl
			 << "    in " << pathname << endl));
		#endif
		#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
		//	Tile markings.
		label.replace (0, 5, " post");
		mark_image (Active_Tile->Image, label,
			TILE_MARKINGS_POST_Y,
			TILE_MARKINGS_COLOR);
		mark_image (Active_Tile->Image, label,
			Active_Tile->Image->height () - TILE_MARKINGS_POST_Y - 12,
			TILE_MARKINGS_COLOR);
		#endif
		}

	//	Remove the Image_Rendering_Monitor from the image.
	Active_Tile->Image->remove_rendering_monitor (Image_Rendering_Monitor);

	Rendering_Lock.unlock ();
	//	Rendering done .........................................................

	/*	Reacquire the Queue_Lock.

		>>> WARNING << The Queue_Lock must be acquired AFTER the
		Rendering_Lock is released to avoid a deadlock condition in
		abort.

		N.B.: The Active_Tile may continue to be used under the
		presumption that it will only be deleted, once acquired, here.
	*/
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": relock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.lock ();

	//	Tile disposition:

	//	The Cancel flag may have been set while the Queue_Lock was unlocked.
	canceled |= Cancel;
	#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "<-- Image_Renderer::render " << thread_ID
			<< ": rendering "
			<< (canceled ? "canceled " : "")
			<< (complete ? "complete" : "incomplete") << " -" << endl
		 << "    " << *(Active_Tile->Image) << endl
		 << "    in " << pathname << endl));
	#endif
	#if ((DEBUG_SECTION) & DEBUG_TILE_MARKINGS)
	if (canceled ||
		! complete)
		{
		//	Tile markings.
		label = QString (" ")
			+= QString (canceled ? "CANCELED " : "")
			+= QString (complete ? "COMPLETE " : "INCOMPLETE ")
			+= QString ("- ")
			+= Plastic_Image::mapping_type_names
				(Active_Tile->Image->needs_update ());
		mark_image (Active_Tile->Image, label,
			TILE_MARKINGS_CANCELED_Y,
			TILE_MARKINGS_CANCELED_COLOR, Qt::transparent);
		mark_image (Active_Tile->Image, label,
			Active_Tile->Image->height () - TILE_MARKINGS_CANCELED_Y - 12,
			TILE_MARKINGS_CANCELED_COLOR, Qt::transparent);
		}
	#endif
	if (canceled)
		{
		//	Clear any lingering rendering cancellation status.
		Active_Tile->Image->cancel_update (false);
		Cancel = false;

		tile_status |= RENDERING_CANCELED;
		#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS | DEBUG_STATUS))
		LOCKED_LOGGING ((
		clog << "^^^ Image_Renderer::render " << thread_ID
				<< ": emit status " << tile_status << " - "
				<< status_description (tile_status) << endl
			 << "    in " << pathname << endl));
		#endif
		//	>>> SIGNAL <<<
		emit status (tile_status);
		}
	else
		{
		if (tile_status == RENDERING_HIGH_PRIORITY ||
			! Active_Tile->Cancelable)
			{
			if (Immediate_Mode)
				{
				#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS))
				LOCKED_LOGGING ((
				clog << "^^^ Image_Renderer::render " << thread_ID
						<< ": emit rendered " << *Active_Tile << endl
					 << "    in " << pathname << endl));
				#endif
				//	>>> SIGNAL <<<
				emit rendered (Active_Tile->Tile_Coordinate);
				}
			else
			if (Render_Queue.isEmpty () ||
				//	Check the next Render_Queue entry.
				Render_Queue.at (0)->is_low_priority ())
				{
				#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_SIGNALS))
				LOCKED_LOGGING ((
				clog << "^^^ Image_Renderer::render " << thread_ID
						<< ": emit rendered all central tiles (0,0)" << endl
					 << "    in " << pathname << endl));
				#endif
				//	>>> SIGNAL <<<
				emit rendered (QPoint ());
				}
			}
		}

	//	Dispose of the Active_Tile.
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": queue for deletion Active_Tile " << *Active_Tile << endl
		 << "    in " << pathname << endl));
	#endif
	delete_tile (Active_Tile);
	Active_Tile = NULL;

	//	Release the Queue_Lock.
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": unlock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.unlock ();
	}
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::render " << thread_ID << endl));
#endif
}


void
Image_Renderer::start_rendering ()
{
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
#endif
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::start_rendering " << thread_ID << endl
	 << "    in " << pathname << endl));
#endif
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer::start_rendering " << thread_ID
		<< ": lock Ready_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Ready_Lock.lock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer::start_rendering " << thread_ID << endl
	 << "    in " << pathname << endl
	 << "    Suspended = false" << endl
	 << "     Runnable = true" << endl
	 << "    unlock Ready_Lock" << endl
	 << "    run_rendering" << endl));
#endif
Suspended = false;
Runnable = true;
Ready_Lock.unlock ();

//	Render whatever is in the rendering queue.
run_rendering ();
#if ((DEBUG_SECTION) & (DEBUG_RENDER | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::start_rendering " << thread_ID << endl));
#endif
}


bool
Image_Renderer::runnable () const
{
QMutexLocker
	ready_lock (&Ready_Lock);
return Runnable;
}


bool
Image_Renderer::suspend_rendering
	(
	bool	wait
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::suspend_rendering " << thread_ID
		<< ": wait = " << boolalpha << wait << endl
	 << "    in " << pathname << endl));
#endif
bool
	ready_locked = Ready_Lock.tryLock (),
	done = Suspended;
Suspended = true;

if (wait &&
	! done)
	{
	bool
		queue_locked = Queue_Lock.tryLock ();

	//	Wait for any rendering to complete.
	#if ((DEBUG_SECTION) & DEBUG_RENDER)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::suspend_rendering " << thread_ID
			<< ": try lock Rendering_Lock" << endl
		 << "    in " << pathname << endl
		 << "    wait up to " << Wait_Seconds << " seconds" << endl));
	#endif
	if ((done = Rendering_Lock.tryLock (Wait_Seconds * 1000)))
		{
		#if ((DEBUG_SECTION) & DEBUG_RENDER)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::suspend_rendering " << thread_ID
				<< ": unlock Rendering_Lock" << endl
			 << "    in " << pathname << endl));
		#endif
		Rendering_Lock.unlock ();
		}

	if (queue_locked)
		Queue_Lock.unlock ();
	}

if (ready_locked)
	Ready_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::suspend_rendering " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


bool
Image_Renderer::suspended () const
{
QMutexLocker
	ready_lock (&Ready_Lock);
return Suspended;
}


bool
Image_Renderer::stop_rendering
	(
	bool	wait
	)
{
#if ((DEBUG_SECTION) & DEBUG_RENDER)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::stop_rendering " << thread_ID
		<< ": wait = " << boolalpha << wait << endl
	 << "    in " << pathname << endl
	 << "    Image_Renderer::stop_rendering " << thread_ID
	 	<< ": unlock Ready_Lock" << endl));
#endif
Ready_Lock.lock ();
//	Suspend the rendering loop.
bool
	done = suspend_rendering (wait);
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    Image_Renderer::stop_rendering " << thread_ID << endl
	 << "    in " << pathname << endl
	 << "    Runnable = false" << endl
	 << "    unlock Ready_Lock" << endl));
#endif
Runnable = false;
Ready_Lock.unlock ();
#if ((DEBUG_SECTION) & DEBUG_RENDER)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::stop_rendering " << thread_ID
		<< ": " << boolalpha << done << endl));
#endif
return done;
}


bool
Image_Renderer::finish
	(
	int		cancel_options
	)
{
#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this)),
	description (cancel_options_descriptions (cancel_options));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::finish " << thread_ID
		<< ": " << cancel_options << " - " << description << endl
	 << "    in " << pathname << endl));
#endif
//	Flag the run loop to finish.
Finish = true;

//	Cancel all rendering; this will also clear the Delete_Queue.
reset (cancel_options | FORCE_CANCEL);

#if ((DEBUG_SECTION) & (DEBUG_CONSTRUCTORS | DEBUG_RENDER))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::finish " << thread_ID << ": true" << endl));
#endif
return true;
}

/*==============================================================================
	Source image
*/
bool
Image_Renderer::image
	(
	const QString&		source_name
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::image " << thread_ID
		<< ": \"" << source_name << '"' << endl
	 << "    in " << pathname << endl));
#endif
bool
	registered = false;
if (! source_name.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::image " << thread_ID
			<< ": lock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.lock ();
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCK_LOG;
	clog << "    Image_Renderer::image " << thread_ID << endl
		 << "    in " << pathname << endl
		 << "    Source_Name = \"" << Source_Name << '"' << endl
		 << "    Source_Image " << *Source_Image << endl;
	if (Image_Source)
		clog << "    Image_Source " << *Image_Source << endl;
	UNLOCK_LOG;
	#endif
	if (Source_Name != source_name)
		{
		Source_Name = source_name;
		Image_Source = NULL;
		registered = true;
		}
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::image " << thread_ID
			<< ": unlock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.unlock ();
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::image " << thread_ID
	 	<< ": " << boolalpha << registered << endl));
#endif
return registered;
}


bool
Image_Renderer::image
	(
	const Shared_Image&	source_image
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << ">>> Image_Renderer::image " << thread_ID << ": source image -" << endl
	 << "    ";
if (source_image)
	clog << *source_image << endl;
else
	clog << "NULL" << endl;
clog << "    in " << pathname << endl;
UNLOCK_LOG;
#endif
bool
	registered = false;
if (source_image)
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << ">>> Image_Renderer::image " << thread_ID
			<< ": lock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.lock ();
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCK_LOG;
	clog << "    Image_Renderer::image " << thread_ID << endl
		 << "    in " << pathname << endl
		 << "    Source_Name = \"" << Source_Name << '"' << endl
		 << "    Source_Image " << *Source_Image << endl;
	if (Image_Source)
		clog << "    Image_Source " << *Image_Source << endl;
	UNLOCK_LOG;
	#endif
	if (Source_Image != source_image)
		{
		Image_Source = source_image;
		Source_Name.clear ();
		registered = true;
		}
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::image " << thread_ID
			<< ": unlock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.unlock ();
	}
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::image " << thread_ID
	 	<< ": " << boolalpha << registered << endl));
#endif
return registered;
}

//------------------------------------------------------------------------------

void
Image_Renderer::load_image ()
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_OVERVIEW | \
				DEBUG_IMAGE_ACCOUNTING | \
				DEBUG_STATUS))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
#endif
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::load_image " << thread_ID << endl
	 << "    in " << pathname << endl
	 << "    Image_Renderer::load_image " << thread_ID
		<< ": tryLock Queue_Lock" << endl));
#endif
bool
	locked = Queue_Lock.tryLock ();
if (! Image_Source &&
	Source_Name.isEmpty ())
	{
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
		 	<< ": no image to load" << endl
		 << "    in " << pathname << endl
		 << "<<< Image_Renderer::load_image " << thread_ID << endl));
	#endif
	if (locked)
		Queue_Lock.unlock ();
	return;
	}
if (locked)
	{
	//	Emit signals without Queue_Lock locked.
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
		 	<< ": unlock Queue_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Queue_Lock.unlock ();
	}

#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_OVERVIEW | \
				DEBUG_SIGNALS | \
				DEBUG_STATUS))
LOCKED_LOGGING ((
clog << "^^^ Image_Renderer::load_image " << thread_ID
		<< ": emit status " << LOADING_IMAGE << " - "
		<< status_description (LOADING_IMAGE) << endl
	 << "    in " << pathname << endl));
#endif
//	>>> SIGNAL <<<
emit status (LOADING_IMAGE);

//	Lock Queue_Lock during image loading.
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_image " << thread_ID
		<< ": lock Queue_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Queue_Lock.lock ();

bool
	loaded = false;
/*
	>>> WARNING <<< A Shared_Image must be used for the loaded image
	pointer because the source may be a Shared_Image and it is important
	that the Source_Image in this case remain connected to the shared
	source. Using a Plastic_Image pointer here to convey the shared
	Image_Source to the Source_Image would be distinct from the original
	shared Image_Source (i.e. it would have the same pointer value but a
	different counter), which will result in premature destruction of the
	image object.
*/
Shared_Image
	source_image;
if (Image_Source)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
			<< ": Image_Source -" << endl
		 << "    " << *Image_Source << endl
		 << "    in " << pathname << endl));
	#endif
	source_image = Image_Source;
	}
else
if (! Source_Name.isEmpty ())
	{
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
			<< ": Source_Name \"" << Source_Name << '"' << endl
		 << "    in " << pathname << endl));
	#endif
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
	 		<< ": lock Rendering_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Rendering_Lock.lock ();

	//	Load the source image from the named source.
	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
			<< ": load image \"" << Source_Name << '"' << endl
		 << "    in " << pathname << endl));
	#endif
	source_image = load_image (Source_Name);

	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::load_image " << thread_ID
	 		<< ": unlock Rendering_Lock" << endl
		 << "    in " << pathname << endl));
	#endif
	Rendering_Lock.unlock ();
	}

//	Clear the source image load request.
Image_Source = NULL;
Source_Name.clear ();

if (source_image)
	{
	#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
					DEBUG_IMAGE_ACCOUNTING | \
					DEBUG_OVERVIEW))
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::render " << thread_ID
			<< ": new Source_Image -" << endl
		 << "    " << *source_image << endl
		 << "    in " << pathname << endl));
	#endif
	//	Clone the Reference_Image from the source image; nothing shared.
	Plastic_Image
		*reference_image = clone_image (source_image);
	if (reference_image)
		{
		//	No Reference_Image disposition here.
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::render " << thread_ID
				<< ": old Reference_Image -" << endl
			 << "    " << *Reference_Image << endl
			 << "    in " << pathname << endl));
		#endif
		//	Reset the Reference_Image data mapping.
		Reference_Image = reference_image;
		Reference_Image->auto_update (false);
		Reference_Image->source_band_map_reset ();
		Reference_Image->source_data_map_reset ();
		Reference_Image->source_transform_reset ();
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
						DEBUG_IMAGE_ACCOUNTING | \
						DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::render " << thread_ID
				<< ": new Reference_Image -" << endl
			 << "    " << *Reference_Image << endl
			 << "    in " << pathname << endl));
		#endif

		//	Transfer the new source_image to the Source_Image.
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::render " << thread_ID
				<< ": old Source_Image -" << endl
			 << "    " << *Source_Image << endl));
		#endif
		Source_Image = source_image;
		loaded = true;
		}
	else
		{
		#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::load_image " << thread_ID
				<< ": source image clone failed." << endl));
		#endif
		delete source_image;
		}
	}

#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_image " << thread_ID
	 	<< ": unlock Queue_Lock" << endl
	 << "    in " << pathname << endl));
#endif
Queue_Lock.unlock ();

//	Emit signals without Queue_Lock locked.
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | \
				DEBUG_OVERVIEW | \
				DEBUG_SIGNALS | \
				DEBUG_STATUS))
LOCKED_LOGGING ((
clog << "^^^ Image_Renderer::load_image " << thread_ID
		<< ": emit image_loaded " << boolalpha << loaded << endl
	 << "    in " << pathname << endl));
#endif
//	>>> SIGNAL <<<
emit image_loaded (loaded);
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::load_image " << thread_ID
		<< ": loaded = " << boolalpha << loaded << endl));
#endif
}


Plastic_Image*
Image_Renderer::load_image
	(
	const QString&	source_name
	)
{
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::load_image " << thread_ID
	 	<< ": " << source_name << endl
	 << "    in " << pathname << endl));
#endif
Plastic_Image
	*plastic_image = NULL;
if (! source_name.isEmpty ())
	{
	if (JP2_Image::is_JP2_file (source_name) ||
		HiView_Utilities::is_URL (source_name))
		plastic_image = load_JP2_image (source_name);
	else
		{
		const char
			*report = NULL;
		QFileInfo
			file (source_name);
		if (! file.exists ())
			report = "The file could not be found.";
		else
		if (file.isDir ())
			report = "The file is a directory.";
		else
		if (! file.isReadable ())
			report = "The file is not readable.";
		else
		if (file.size () == 0)
			report = "The file is empty.";
		else
			{
			QImage
				*source_image = new QImage (source_name);
			if (! source_image->isNull ())
				{
				plastic_image = new Plastic_QImage (source_image);
				plastic_image->source_name (source_name);
				#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
				LOCKED_LOGGING ((
				clog << "    Image_Renderer::load_image " << thread_ID
	 					<< ": loaded QImage @ " << (void*)source_image << endl
					 << "    " << *plastic_image << endl
					 << "    in " << pathname << endl));
				#endif
				}
			else
				{
				delete source_image;
				#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
				LOCKED_LOGGING ((
				clog << "   Image_Renderer::load_image " << thread_ID
						<< ": unable to load source - " << Source_Name << endl
					 << "    in " << pathname << endl));
				#endif
				report = "An image could not be obtained from the file.";
				}
			}
		if (report)
			{
			QString
				error_message (ID);
			error_message += '\n';
			error_message += tr ("Failed to load an image from");
			error_message += " \"";
			error_message += source_name;
			error_message += "\".\n";
			error_message += tr (report);
			#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
			LOCKED_LOGGING ((
			clog << "^^^ Image_Renderer::load_image " << thread_ID
					<< ": emit error -" << endl
				 << error_message << endl
				 << "    in " << pathname << endl));
			#endif
			//	>>> SIGNAL <<<
			emit error (error_message.replace ("\n", "<br>"));
			}
		}
	}
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::load_image "  << thread_ID
		<< ": Plastic_Image @ " << (void*)plastic_image << endl));
#endif
return plastic_image;
}


JP2_Image*
Image_Renderer::load_JP2_image
	(
	const QString&	source_name
	)
{
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::load_JP2_image " << thread_ID
	 	<< ": " << source_name << endl
	 << "    in " << pathname << endl));
#endif
QString
	report;
JP2_Reader
	*JP2_reader = NULL;
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_JP2_image " << thread_ID
		<< ": constructing a JP2::reader on \""
			<< source_name.toStdString () << '"' << endl
	 << "    in " << pathname << endl));
#endif
try {JP2_reader = JP2::reader (source_name.toStdString ());}
catch (JP2_Exception& except)
	{report = except.what ();}
catch (std::exception& except)
	{report = except.what ();}
catch (...)
	{report = "Unknown exception!";}
if (! report.isEmpty ())
	{
	Error_Report:
   QStringList lines = report.split("\n",  QString::SkipEmptyParts);
	QString
		error_message (lines.size() > 1 ? lines.at(lines.size()-1) : report);
	error_message += '\n';
   error_message += ID;
   error_message += '\n';
	error_message += tr ("Failed to load a JP2 image from");
	error_message += " \"";
	error_message += source_name;
   error_message += '\n';
   error_message += report;
	error_message += "\".\n";
	#if ((DEBUG_SECTION) & DEBUG_SIGNALS)
	LOCKED_LOGGING ((
	clog << "^^^ Image_Renderer::load_JP2_image " << thread_ID
			<< ": emit error -" << endl
		 << error_message << endl
		 << "    in " << pathname << endl));
	#endif
	//	>>> SIGNAL <<<
	emit error (error_message.replace ("\n", "<br>"));

	if (JP2_reader)
		{
		#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
		LOCKED_LOGGING ((
		clog << "    Image_Renderer::load_JP2_image " << thread_ID
				<< ": delete JP2_Reader @ " << (void*)JP2_reader << endl
			 << "    in " << pathname << endl));
		#endif
		delete JP2_reader;
		}

	#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
	LOCKED_LOGGING ((
	clog << "    in " << pathname << endl
		 << "<<< Image_Renderer::load_JP2_image: NULL" << endl));
	#endif
	return NULL;
	}
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_JP2_image " << thread_ID
		<< ": JP2_Reader @ " << (void*)JP2_reader << " -" << endl
	 << "     image_width = " << JP2_reader->image_width () << endl
	 << "    image_height = " << JP2_reader->image_height () << endl
	 << "     image_bands = " << JP2_reader->image_bands () << endl
	 << "    in " << pathname << endl));
#endif

//	Size the image appropriately for a source image.
QSize
	source_size (JP2_reader->image_width (), JP2_reader->image_height ());
double
	scale = scale_to_area (source_size, max_source_image_area ());
#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_JP2_image " << thread_ID
		<< ": scale " << scale << " to fit " << max_source_image_area ()
		<< " amount " << endl
	 << "    in " << pathname << endl));
#endif
if (scale < .95)
	source_size *= scale;
#if ((DEBUG_SECTION) & DEBUG_LOAD_IMAGE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_JP2_image " << thread_ID
		<< ": construct a JP2_Image with source size "
		<< source_size << endl
	 << "    in " << pathname << endl));
#endif
//	Construct the JP2_Image. Ownership of the JP2_Reader is transferred.
JP2_Image
	*JP2_image = NULL;
try {JP2_image = new JP2_Image (JP2_reader, source_size);}
catch (JP2_Exception& except)
	{report = except.what ();}
catch (std::exception& except)
	{report = except.what ();}
catch (...)
	{report = "Unknown exception!";}
if (! report.isEmpty ())
	goto Error_Report;

if (scale < .95)
	//	Set the image scaling to fit.
	JP2_image->source_scale (scale);

//	Apply the source name.
JP2_image->source_name (source_name);

#if ((DEBUG_SECTION) & (DEBUG_LOAD_IMAGE | DEBUG_OVERVIEW))
LOCKED_LOGGING ((
clog << "    Image_Renderer::load_JP2_image " << thread_ID
		<< ": JP2_Image -" << endl
	 << "    " << *JP2_image << endl
	 << "    in " << pathname << endl
	 << "<<< Image_Renderer::load_JP2_image " << thread_ID << endl));
#endif
return JP2_image;
}

/*==============================================================================
	Cloned image
*/
Plastic_Image*
Image_Renderer::image_clone
	(
	const QSize&					size,
	Plastic_Image::Mapping_Type	shared_mappings
	)
{
#if ((DEBUG_SECTION) & DEBUG_CLONE_IMAGE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this)),
	names (Plastic_Image::mapping_type_names (shared_mappings));
LOCKED_LOGGING ((
clog << ">>> Image_Renderer::image_clone " << thread_ID << ":" << endl
	 << "               size = " << size << endl
	 << "    shared_mappings = " << shared_mappings << " - " << names << endl));
#endif
#if ((DEBUG_SECTION) & DEBUG_CLONE_IMAGE)
LOCKED_LOGGING ((
clog << "    Image_Renderer::image_clone " << thread_ID
		<< ": cloning Reference image -" << endl
	 << "    " << *Reference_Image << endl
	 << "    in " << pathname << endl));
#endif
/*	>>> WARNING <<<

	The Reference_Image must not be changed when clone_image is called
	(possible race condition), nor the object deleted while the cloning
	is in progress.
*/
Plastic_Image
	*cloned_image = clone_image
		(Reference_Image, size, shared_mappings);
#if ((DEBUG_SECTION) & DEBUG_CLONE_IMAGE)
LOCK_LOG;
clog << "    Image_Renderer::image_clone " << thread_ID
		<< ": cloned_image -" << endl;
if (cloned_image)
	clog << "    " << *cloned_image << endl;
else
	clog << "    NULL" << endl;
clog << "    in " << pathname << endl
	 << "<<< Image_Renderer::image_clone " << thread_ID
	 	<< ": " << (void*)cloned_image << endl;
UNLOCK_LOG;
#endif
return cloned_image;
}


Plastic_Image*
Image_Renderer::clone_image
	(
	Plastic_Image*					source_image,
	const QSize&					image_size,
	Plastic_Image::Mapping_Type	shared_mappings
	)
{
#if ((DEBUG_SECTION) & DEBUG_CLONE_IMAGE)
void*
	thread_ID = (void*)QThread::currentThreadId ();
QString
	pathname (object_pathname (this));
LOCK_LOG;
clog << ">>> Image_Renderer::clone_image " << thread_ID
	 	<< ": source_image -" << endl;
if (source_image)
	clog << "    " << *source_image << endl;
else
	clog << "    NULL" << endl;
clog << "    in " << pathname << endl;
UNLOCK_LOG;
#endif
Plastic_Image
	*cloned_image = NULL;
if (source_image)
	{
	QString
		error_message;
	#if ((DEBUG_SECTION) & DEBUG_CLONE_IMAGE)
	LOCKED_LOGGING ((
	clog << "    Image_Renderer::clone_image " << thread_ID
			<< ": clone the source image" << endl
		 << "    in " << pathname << endl));
	#endif
	try {cloned_image = source_image->clone (image_size, shared_mappings);}
	catch (JP2_Exception& except)
		{error_message = except.what ();}
	catch (std::exception& except)
		{error_message = except.what ();}
	catch (...)
		{error_message = "Unknown exception!";}
	if (! error_message.isEmpty ())
		{
		QString
			message (ID);
		message += "\n";
		message += "Failed to clone image";
		if (source_image->source_name ().isEmpty ())
			message += ".\n";
		else
			message +=
				QString (" \"") + source_image->source_name () + "\".\n";
		error_message = message + error_message;
		#if ((DEBUG_SECTION) & (DEBUG_CLONE_IMAGE | DEBUG_SIGNALS))
		LOCKED_LOGGING ((
		clog << "^^^ Image_Renderer::clone_image " << thread_ID
				<< ": emit error -" << endl
			 << error_message << endl
			 << "    in " << pathname << endl));
		#endif
		//	>>> SIGNAL <<<
		emit error (error_message.replace ("\n", "<br>"));
		}
	}

#if defined (DEBUG_SECTION) && DEBUG_SECTION != 0
if (cloned_image)
	Image_Accounting.append (cloned_image);
#endif
#if ((DEBUG_SECTION) & DEBUG_CLONE_IMAGE)
LOCK_LOG;
clog << "    Image_Renderer::clone_image: " << thread_ID << endl
	 << "    in " << pathname << endl
	 << "    cloned ";
if (cloned_image)
	clog << *cloned_image << endl;
else
	clog << "image NULL" << endl;
image_accounting ();
clog << "<<< Image_Renderer::clone_image " << thread_ID << endl;
UNLOCK_LOG;
#endif
return cloned_image;
}

/*==============================================================================
	Utilities
*/
double
Image_Renderer::scale_to_area
	(
	const QSize&	size,
	unsigned long 	area
	)
{
double
	scaling = sqrt ((double)area / ((double)size.width () * size.height ()));
QSize
	scaled_size (size * scaling);
unsigned long long
	amount = (unsigned long long)scaled_size.rwidth () * scaled_size.rheight ();
if (amount > area)
	{
	/*	The scaling factor is too large, due to rounding.

		Reduce the scaled area by the smallest dimension that will bring
		the area under the limit. Then recalculate the scaling based on
		the adjusted dimension.
	*/
	amount -= area;
	if (scaled_size.rwidth () < scaled_size.rheight ())
		{
		/*	If reducing the scaled area by a row width is sufficient
			to bring the area under the limit, then decrement the
			column height. Otherwise decrement the row width to reduce
			the scaled area by a column height.
		*/
		if ((unsigned long long)scaled_size.rwidth () >= amount)
			{
			//	Reduce the area by a row width.
			scaled_size.rheight ()--;
			scaling = (double)scaled_size.rheight () / size.height ();
			}
		else
			{
			//	Reduce the area by a column height.
			scaled_size.rwidth ()--;
			scaling = (double)scaled_size.rwidth () / size.width ();
			}
		}
	else
		{
		if ((unsigned long long)scaled_size.rheight () >= amount)
			{
			scaled_size.rwidth ()--;
			scaling = (double)scaled_size.rwidth () / size.width ();
			}
		else
			{
			scaled_size.rheight ()--;
			scaling = (double)scaled_size.rheight () / size.height ();
			}
		}
	}
return scaling;
}


QString
Image_Renderer::status_description
	(
	int		condition
	)
{
QString
	description;
switch (condition & ~RENDERING_CANCELED)
	{
	case NOT_RENDERING:
		description = tr ("Not Rendering"); break;
	case RENDERING_LOW_PRIORITY:
		description = tr ("Rendering Low Priority"); break;
	case RENDERING_HIGH_PRIORITY:
		description = tr ("Rendering High Priority"); break;
	case LOADING_IMAGE:
		description = tr ("Loading Image"); break;
	default:
		description = tr ("Unknown Status %1").arg (condition);
	}
if (condition & RENDERING_CANCELED)
	description += tr (" Canceled");
return description;
}


QString
Image_Renderer::cancel_options_descriptions
	(
	int		cancel_options
	)
{
QString
	descriptions ((cancel_options & WAIT_UNTIL_DONE) ?
		"WAIT_UNTIL_DONE" : "DO_NOT_WAIT");
if (cancel_options & DELETE_WHEN_DONE)
	descriptions += ", DELETE_WHEN_DONE";
if (cancel_options & FORCE_CANCEL)
	descriptions += ", FORCE_CANCEL";
return descriptions;
}


}	//	namespace HiRISE
}	//	namespace UA

