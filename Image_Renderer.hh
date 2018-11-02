/*	Image_Renderer

HiROC CVS ID: $Id: Image_Renderer.hh,v 1.35 2012/09/17 04:52:22 castalia Exp $

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

#ifndef HiView_Image_Renderer_hh
#define HiView_Image_Renderer_hh

#include	"Plastic_Image.hh"
#include	"Image_Tile.hh"

//	PIRL++
#include	"Reference_Counted_Pointer.hh"

#include	<QObject>
#include	<QMutex>
#include	<QList>
#include	<QPoint>
#include	<QSize>


namespace UA
{
namespace HiRISE
{
//	Forward reference.
class JP2_Image;
class Image_Renderer_Rendering_Monitor;


/**	A <i>Image_Renderer</i> provides thread safe image rendering and
	deletion.

	An Image_Renderer manages a queue of images, encapsulated in image
	tiles, that are to be rendered. Images may be asynchronously added to
	or canceled from the queue in a thread safe manner. However, if an
	image is actively being rendered it will not be canceled from the
	queue, though in this case the rendering will be aborted which will
	cause it to stop at the first safe opportunity.

	The queue is processed in a rendering loop that may be in an active,
	runnable or suspended state. The rendering loop is active when the
	image queue is being consumed. This generally means that the first
	entry in the image queue is being rendered, though the loop also
	includes destroying images that have been queued for deletion and
	checking for rendering state changes signaled by the user that are
	interleaved with the image rendering phase of the loop. When the
	rendering state is set to be not runnable processing of the image
	queue will stop as soon as the state change is detected and will not
	begin again until the rendering has been manually started which will
	set the rendering state to runnable. When the rendering state is set
	to be suspended processing of the image queue will stop as soon as
	the state change is detected but will automatically begin again if an
	entry is added to the image queue or rendering is manually started.
	When rendering is active and the image queue becomes empty rendering
	is automatically suspended. There is also a special finished state
	that, when set, will set the rendering loop to be not runnable, clear
	the image queue, abort any rendering in progress, and cause the
	rendering loop to exit. For the Image_Renderer_Thread subclass,
	exiting the rendering loop cause the thread to stop running.

	Image rendering is based on a source image that is loaded from either
	an image source name reference (which may be a file pathname or a
	JPIP URL for a JP2 source) or an externally shared image.

	The source image is used to generate a zero-sized reference image
	from which image clones are generated. The image clones share the
	band and data mapping tables with the reference image. Image clones
	are typically, but not necessarily, the images queued for rendering.

	Any image may be queued for rendering. The queued image is associated
	with a location coordinate when encapsulated in an image tile. Image
	tiles with non-null coordinates have high priority for rendering:
	they are added to the queue at an entry before any low priority image
	tiles (with null coordinates); the exception is that first queue
	entry, if being actively rendered, can not be pre-empted. An image
	may be re-queued; i.e. an image may be added to the queue when there
	is already an entry for the image in the queue. Re-queued images may
	have a different priority than the current entry, in which case they
	may be relocated in the queue, or have changed handling flags; when
	an image actively being rendered is re-queued a new queue entry is
	added. When an image is queued for rendering it may be flagged for
	special handling: A non-cancelable image will remain queued when the
	queue is cleared of all other images. An image may also be flagged
	for deletion when its encapsulating tile is destroyed.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.35 $
*/
class Image_Renderer
:	public QObject
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Types:
*/
typedef PIRL::Reference_Counted_Pointer<Plastic_Image>	Shared_Image;
typedef QList<Image_Tile*>								Tile_Queue;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


/*	>>> CAUTION <<< The Image_Tile values are relied on to be the
	the first two bits of the status code.
*/
/**	Rendering {@link status(int) status} codes.

	<b>N.B.</b>: The RENDERING_CANCELED value is used as a bit flag
	that may qualify the other values.
*/
enum
	{
	NOT_RENDERING				= 0,
	RENDERING_LOW_PRIORITY		= Image_Tile::LOW_PRIORITY,
	RENDERING_HIGH_PRIORITY		= Image_Tile::HIGH_PRIORITY,
	LOADING_IMAGE =
		RENDERING_LOW_PRIORITY |
		RENDERING_HIGH_PRIORITY,
	RENDERING_CANCELED			= (1 << 2)
	};

//!	Options when canceling rendering.
static const int
	DO_NOT_WAIT,
	WAIT_UNTIL_DONE,
	DELETE_WHEN_DONE,
	FORCE_CANCEL;

static const QPoint
	LOW_PRIORITY_RENDERING;

static const bool
	CANCELABLE;

/*------------------------------------------------------------------------------
	Defaults
*/
protected:

//!	Default area, in pixels, of a JP2 source image loaded from a named source.
static unsigned long
	Default_Min_Source_Image_Area,
	Default_Max_Source_Image_Area;

//!	Maximum time, in seconds, to wait for an event to occur.
static int
	Wait_Seconds;

/*==============================================================================
	Constructors
*/
public:

explicit Image_Renderer (QObject* parent = NULL);

virtual ~Image_Renderer ();

/*==============================================================================
	Source image
*/
/**	Load new source and reference images into this Image_Renderer.

	This method is asynchronous: It will return immediately. When the
	{link image_loaded(bool) image loaded signal} has been emitted the
	{@link source_image() source} and {@link reference_image() reference}
	images may be accessed; </b>but not until then</b>. If the image
	can not be loaded the {@link error(QString&) error signal} will
	also be emitted.

	@param	source_name	A QString specifying the image source (a local
		file pathname or JPIP server URL) for rendering.
	@return	true if the source name was registered for loading; false
		if a {@link image(const QImage&) source image} has already
		been registered.
*/
bool image (const QString& source_name);

/**	Load new source and reference images into this Image_Renderer.

	This method is asynchronous: It will return immediately. When the
	{link image_loaded(bool) image loaded signal} has been emitted the
	{@link source_image() source} and {@link reference_image() reference}
	images may be accessed; </b>but not until then</b>. If the image
	can not be loaded the {@link error(QString&) error signal} will
	also be emitted.

	@param	source_image	A Shared_Image specifying the image source to
		use for rendering.
	@return	true if the source image was registered for loading; false
		if a {@link image(const QString&) source name} has already
		been registered.
*/
bool image (const Shared_Image& source_image);

/**	Get the source image.

	The source image, if it is loaded from a {@link image(const QString&)
	named source}, is pre-rendered to a size that will be no more than
	the {@link default_source_image_MB() default source image MB} in total
	data volume; unless a {@link image(const QImage&) rendered image} is
	used as its source, in which case a copy of the source image is used.
	The source image is used to fill-in for a tile image until the latter
	has been rendered and data mapped.
	
	<b>WARNING</b>: Do not use this method, or the object pointed to
	by its return value, after a source {@link image(const QString&)
	name} or {@link image{const QImage&) image} has been specified and
	before the {@link image_loaded(bool) image loaded signal} has been
	received. During that time the source and reference image objects will
	be in the process of being replaced.

	@return	A Shared_Image pointer to the current source image. This will
		never be NULL; at worst an empty source image will be provided.
*/
const Shared_Image source_image () const;

/**	Get the reference image.

	The reference image is a zero-sized clone of the source image used as
	the reference for all other {@link image_clone(const QSize,
	Plastic_Image::Mapping_Type) cloned images}. <b>N.B.</b>: The
	reference image does not share any image mapping characteristics -
	bands map, geometric transformations, source-to-display data mapping -
	with the source image from which it is cloned, only the source image
	itself.
	
	<b>WARNING</b>: Do not use this method, or the object pointed to
	by its return value, after a source {@link image(const QString&)
	name} or {@link image{const QImage&) image} has been specified and
	before the {@link image_loaded() image loaded signal} has been
	received. During that time the source and reference image objects will
	be in the process of being replaced.

	@return	A pointer to a Plastic_Image that is the current reference
		image. This will never be NULL.
*/
Plastic_Image* reference_image () const;

/**	Set the maximum image area to use when loading a JP2 source image.

	When a JP2 {@link source_image() source image} is loaded from a
	{@link image(const QString&) named image source} it may be
	exceedingly large, possibly too large to fit into memory. So the
	image will be rendered into a limited area. A scaling factor for the
	image will be determined by {@link scale_to_area(const QSize&,
	unsigned long) fitting the image size to the maximum image area} and
	limiting the scaling factor to be less thean or equal to 1.0.

	The closer the image scaling factor is to 1.0 the better the source
	image will appear when being used to fill the display viewport until
	the corresponding image tiles are rendered and become available for
	display. However, the smaller the source image is the faster it will
	load and the less system memory it will use. The {@link
	default_max_source_image_area(unsigned long) default max source
	image area} can be used to set an appropriate default value.

	@param	area	The maximum image area to be allowed when loading
		a JP2 image.
*/
inline void max_source_image_area (unsigned long area)
	{
	if (area < Default_Min_Source_Image_Area)
		area = Default_Min_Source_Image_Area;
	Max_Source_Image_Area = area;
	}

inline unsigned long max_source_image_area () const
	{return Max_Source_Image_Area;}

inline static unsigned long default_max_source_image_area ()
	{return Default_Max_Source_Image_Area;}

inline static void default_max_source_image_area (unsigned long area)
	{
	if (area < Default_Min_Source_Image_Area)
		area = Default_Min_Source_Image_Area;
	Default_Max_Source_Image_Area = area;
	}

/**	Get a clone of the {@link reference_image() reference image.

	This method is synchronous: it will not return until the image clone
	as been created.

	@param	size	A QSize specifying the image size of the clone.
		If the size is not valid the clone will be the same size
		as the reference image. By default the clone has zero size.
	@param	shared_mappings	A Plastic_Image::Mapping_Type bit map
		specifying the data mappings of the reference image to
		be shared with the clone. By default no data mappings are
		shared.
	@return	A Plastic_Image pointer to the image clone.
*/
Plastic_Image* image_clone (const QSize& size = QSize (0, 0),
	Plastic_Image::Mapping_Type shared_mappings = 0);

/*------------------------------------------------------------------------------
	Helpers
*/
protected:

/**	Load any posted new source image.

	<b>N.B.</b>: The Queue_Lock is held by this method.

	If neither an {@link image(const Shared_Image&) Image_Source} nor a
	@link image(const QString&) Source_Name} has been posted nothing
	is done. If a non-NULL Image_Source is found it is used as-is.
	If a non-empty Source_Name is found it is used to {@link
	load_image(const QString&) load an image}. <b>N.B.</b>: The
	Rendering_Lock is held while the image is being loaded.

	The {@link #LOADING_IMAGE} {@link status(int) status signal} is
	emitted <b>before</b> image loading begins.

	When a new source image is available it is used to {@link
	clone_image(Plastic_Image*, const QSize&, Plastic_Image::Mapping_Type)
	clone an image} to be used as the new Reference_Image. If this
	succeeds the new source image is set as the Source_Image.

	When image loading completes the Image_Source is nullified and the
	Source_Name is cleared. Then the {@link image_loaded(bool) image
	loaded signal} is emitted. All signals are emitted while the
	Queue_Lock is unlocked. <b>N.B.</b>: Image loading always
	<i>starts</i> with a LOADING_IMAGE status signal and <i>ends</i> with
	an image_loaded signal indicating if the load succeeded or not.

	Disposition of the previous Source_Image and Reference_Image is left
	to the user: The user is expected to be holding pointers to the
	Source_Image and Reference_Image. The Source_Image pointer is held by
	a Shared_Image, so it will automatically dispose of the previous
	object when the last reference to it is removed, typically by
	assigning the new {@link source_image() source image} from the
	Renderer in response to a successful {@link image_loaded(bool) image
	loaded signal}. However, the Reference_Image is held by a native
	pointer value, so the user needs to dispose of the previous object
	before updating its pointer to the new object. This should be done by
	using the Renderer to {@link delete_image(Plastic_Image*) queue the
	image object for deletion}.

	<b>N.B.</b>: If a source image can not be loaded or the reference
	image can not be generated the existing Source_Image and
	Reference_Image remain unaffected and a false image loaded signal
	will be emitted.
*/
void load_image ();

/**	Load an image from a named source.

	If the source name is for a {@link
	HiView_Utilities::is_JP2_file(const QString&) JP2 file} or a {@link
	HiView_Utilities::is_URL(const QString&) JP2 URL} a attempt is made
	to {@link load_JP2_image(const QString&) load a JP2_Image); otherwise
	if the name is for a file that is accessible from the local
	filesystem an attempt is made to construct a QImage from the file and
	wrap it in a Plastic_QImage. If the source can not be loaded an
	{@link error(const QString&) error signal} will be emitted containing
	a message describing the problem.

	@param	source_name	A QString naming the image source. This may by
		a pathname to a local file or a URL for a remote JP2 file served
		by a JPIP server. If the name is empty nothing is done.
	@return	A pointer to a Plastic_Image containing the image that was
		loaded. This will be NULL if an image could not be loaded from
		the source for any reason.
	@see	load_image()
*/
Plastic_Image* load_image (const QString& source_name);

/**	Load a JP2 image from a named source.

	A JP2_Reader is constructed for the named source. The size of the
	image is obtained from the reader and used to determine a scaling
	factor that will {@link scale_to_area(const QSize&, unsigned
	long) fit the size} to the {@link max_source_image_area() maximum
	image area}. If the scaling factor is less than 0.95 the image size
	is scaled down when the reader is used to contstruct a JP2_Image and
	its scaling factor is set accordingly.

	@param	source_name	The name of the image source, which may be a
		pathname to a local file or a URL for a remote JPIP server.
	@return	A pointer to a JP2_Image. This will be NULL if an image
		could not be loaded from the source.
	@see	load_image()
*/
JP2_Image* load_JP2_image (const QString& source_name);

/**	Clone an image.

	<b>N.B.</b>: The Rendering_Lock is held while the image is being
	cloned.

	If an exception is thrown from the source image's clone method
	an {@link error(const QString&) error signal} will be emitted
	containing a message describing the problem.

	@param	image	A pointer to the Plastic_Image to be cloned.
		If NULL, nothing is done.
	@param	size	A QSize specifying the image size of the clone.
		If the size is not valid the clone will be the same size
		as the reference image. By default the clone has zero size.
	@param	shared_mappings	A Plastic_Image::Mapping_Type bit map
		specifying the data mappings of the reference image to
		be shared with the clone. By default no data mappings are
		shared.
	@return	A Plastic_Image pointer to the image clone. If no clone
		was created, either because the source image is NULL or an
		exception occured during cloning, NULL will be returned.
*/
Plastic_Image* clone_image (Plastic_Image* image,
	const QSize& size = QSize (0, 0),
	Plastic_Image::Mapping_Type shared_mappings = Plastic_Image::NO_MAPPINGS);

/*==============================================================================
	Queue
*/
public:

/**	Queue an image for rendering.

	An image with a non-null tile coordinate is associated with a "high
	priority" tile - the image for a high priority tile appears in the
	display viewport - which is given priority in the rendering queue
	over tiles that have a null tile coordinate that are considered to be
	"low priority" tiles - a low priority tile image does not appear in
	the display viewport. In addition, high priority tiles are maintained
	in visible tile region area order from larger to smaller tile areas.

	If the image to be queued for rendering is found to already be in the
	rendering queue and its rendering priority - {@link
	Image_Tile::status() priority status} and {@link Image_Tile::area()
	visible image region area} - is the same then the new tile replaces
	the existing tile at the same entry the queue. Otherwise the existing
	tile entry is removed and the new tile for the image added to the
	queue. <b>N.B.</b>: When an existing queue entry is changed the existing
	tile entry {@link Image_Tile::Delete_Image_When_Done} flag is cleared
	before the tile is deleted.

	<b>N.B.</b>: An image can be queued for rendering even if it is
	currently being rendered (is associated with the Active_Tile).

	<b>N.B.</b>: Queuing an image will cause rendering to be {@link
	start_rendering() started} if the renering loop is {@link runnable()
	runnable}.

	<b>N.B.</b>: The queue is locked during the queuing operation.

	@param	image	A pointer to Plastic_Image to be queued for rendering.
	@param	tile_coordinate	A QPoint that specifies the coordinate of the
		tile image in the tile grid. If the coordinate is null the image
		is for a low priority (non-display) tile; otherwise it is for a
		high priority (display) tile.
	@param	tile_region	The part of the tile image, in tile-relative
		display coordinates, that intersects with the display viewport.
	@param	cancelable	If true the queued entry may be {@link cancel(bool)
		canceled; otherwise {@link clear()
	@see	add_tile(Image_Tile*)
*/
void queue (Plastic_Image* image,
	const QPoint& tile_coordinate = QPoint (),
	const QRect& tile_region = QRect (),
	bool cancelable = true);

/**	Queue an image for rendering.

	This is a convenience method to queue an image for rendering that has
	no visible tile region.

	@param	image	A pointer to Plastic_Image to be queued for rendering.
	@param	tile_coordinate	A QPoint that specifies the coordinate of the
		tile image in the tile grid. If the coordinate is null the image
		is for a low priority (non-display) tile; otherwise it is for a
		high priority (display) tile.
	@param	cancelable	If true the queued entry may be {@link cancel(bool)
		canceled; otherwise {@link clear()
	@see	queue(Plastic_Image*, const QPoint&, const QRect&, bool)
*/
inline void queue (Plastic_Image* image,
	const QPoint& tile_coordinate,
	bool cancelable)
	{queue (image, tile_coordinate, QRect (), cancelable);}

/**	Get the tile image rendering status.

	@return	The {@link Image_Tile::status() priority status} of the
		Active_Tile is returned. If there is no Active_Tile the status of
		the first tile in the rendering queue (which is high-to-low
		priority ordered) is returned. If the queue is empty zero is
		returned.
	@see	status(int)
*/
int rendering_status () const;

/**	Test if an image is queued for rendering.

	<b>N.B.</b>: An image that {@link is_rendering(Plastic_Image*) is
	rendering} is not in the rendering queue.

	If the image is NULL the test is if any image is queued.

	@param	image	A pointer to the Plastic_Image to be sought in the
		Render_Queue. If NULL the test is if any image is in the
		Render_Queue.
	@return	true if the image is queued for rendering, or if the image
		is NULL if any image is queued for rendering; false otherwise.
*/
bool is_queued (Plastic_Image* image = NULL) const;

/**	Test if an image is currently being rendered.

	If the image is NULL the test is if any image is being rendered.

	@param	image	A pointer to the Plastic_Image to be compared with
		the image associated with the Active_Tile. If NULL, only the test
		for an Active_Tile is made.
	@return	true if an image is being rendered and, if the image
		specified is non-NULL, the specified image is being rendered; 
		false if there is no Active_Tile or, if a non-NULL image was
		specified, the Active_Tile is not the specified image.
*/
bool is_rendering (Plastic_Image* image = NULL) const;

/**	Test if the image is queued for deletion.

	@param	image	A pointer to the Plastic_Image to be sought in the
		Delete_Queue.
	@return	true if the image is queued for deletion; false otherwise.
*/
bool will_delete (Plastic_Image* image) const;

/**	Enable or disable immediate mode.

	In immediate mode a {@link rendered(const QPoint&) rendered signal}
	is sent immedately after the completion of image rendering for each
	{@link Image_Tile::is_high_priority() high priority} tile. When immediate
	mode is disabled a rendering signal is sent only after the completion
	of image rendering for all high priority tiles.

	@param	enable	If true immediate mode is enabled; otherwise immediate
		mode is disabled.
*/
void immediate_mode (bool enable);

/**	Test if immediate mode is enabled

	@return	true if immediate mode is enabled; false otherwise.
*/
bool immediate_mode () const;

/**	Cancel processing of an image.

	The Render_Queue is searched for the Image_Tile containing the
	image. If no match is found nothing is done. If the entry found is
	for the first, active, image tile processing of the image is
	aborted; in this case the method will not return until processing of
	the image has stopped. Otherwise the matching entry is simply
	removed from the Render_Queue.

	<b>N.B.</b>: Even queued entries marked as non-cancelable will be
	canceled by this method.

	<b>WARNING</b>: The image of the active tile must not be immediately
	deleted by the caller if the return value is false; any tile image
	queued for rendering should only be deleted after the image rendering
	has been successfully canceled or this Image_Renderer is not running.

	@param	image	A pointer to the Plastic_Image to be removed from
		the Render_Queue and aborted if it is currently being rendered and is
		cancelable. If NULL, nothing is done.
	@param	cancel_options	One or more cancel options bit flags. The
		{@link #DELETE_WHEN_DONE} flag marks the tile image to be deleted
		when the tile is deleted from the queue. The {#WAIT_UNTIL_DONE}
		flag will block the method from returning until the cancellation
		is complete. Cancellation is complete if the Render_Queue is empty or
		this Image_Renderer is not, or no longer, processing the active
		tile. Cancel of rendering will be signaled but rendering may
		still be in progress when the method returns.
	@return	true if the image was found in the queue and canceled, or it was
		not found in the queue; false if the image was found to be the
		currently rendering image and it did not abort.
	@see	cancel(int)
	@see	abort(bool)
*/
bool cancel
	(Plastic_Image* image, int cancel_options = WAIT_UNTIL_DONE);

/**	{@link stop_rendering(bool) Stop rendering} and {@link clear(int)
	clear} the rendering queue.

	@param	cancel_options	One or more cancel options bit flags. The
		{@link #DELETE_WHEN_DONE} flag marks tile images to be deleted
		when the tile is deleted from the queue. The {#WAIT_UNTIL_DONE}
		flag will block the method from returning until the cancellation
		is complete. Cancellation is complete if the Render_Queue is empty or
		this Image_Renderer is not, or no longer, processing the active
		tile. Cancellation of rendering will be signaled but rendering may
		still be in progress when the method returns.
	@return	true if the thread finished before returning; false otherwise.
	@see	clear(int)
	@see	start_rendering()
*/
bool reset (int cancel_options = WAIT_UNTIL_DONE);

/**	Delete an image.

	If the image is found in a tile on the Rendering_Queue the tile is
	marked for {@link #DELETE_WHEN_DONE} and moved to the Delete_Queue
	for {@link delete_tiles() eventual deletion}. If the image is
	associated with the Active_Tile it is marked for {@link
	#DELETE_WHEN_DONE} and left to complete rendering after which it will
	be deleted. <b>N.B.</b>: The Active_Tile being rendered is not {@link
	abort(int) aborted}; {@link cancel(Plastic_Image*, int) cancel}
	image rendering to abort the Active_Tile.

	If the image is not found in a tile on the Rendering_Queue it is
	wrapped in an image tile set for DELETE_WHEN_DONE and placed on the
	Delete_Queue for eventual deletion.

	@param	image	A pointer to the Plastic_Image object to be deleted.
	@return	true if the image was found on the Rendering_Queue or
		associated with the Active_Tile; false otherwise. In any case the
		image is either queued for deletion at the next opportunity or
		marked for deletion when its rendering is completed.
*/
bool delete_image (Plastic_Image* image);

/**	Clean the queue of tiles pending deletion.

	All tiles that have been queued for deletion are deleted leaving the
	Delete_Queue empty.

	<b>>>> CAUTION <<<</b> This method will attempt to lock the
	Queue_Lock.

	@see	delete_image(Plastic_Image*)
	@see	delete_tile(Image_Tile*)
	@see	delete_tiles()
*/
void clean_up ();

/*------------------------------------------------------------------------------
	Queue management
*/
protected:

/*
	N.B.: Most of the queue management methods assume that the Queue_Lock
	as been acquired so the Render or Delete Queue can be safely
	manipulated. The exception is entry zero of the Render_Queue which
	holds the tile currently being rendered if the Active_Tile is
	non-NULL; The Active_Tile entry is the responsibility of the render
	method.
*/

/**	Search a Tile_Queue for the Image_Tile containing a specific image.

	<b>>>> WARNING <<<</b> The specified queue is expected to be locked
	before this function is used.

	@param	image	A pointer to the Plastic_Image that an Image_Tile
		must contain to produce a match.
	@param	queue	A Tile_Queue reference that is to be searched.
	@return	The index in the queue where the matching Image_Tile was
		found, or -1 if no match was found.
*/
static int find_tile (Plastic_Image* image, const Tile_Queue& queue);

/**	Add an Image_Tile to the Render_Queue.

	<b>>>> WARNING <<<</b> The Queue_Lock is expected to be locked before
	this method is entered.

	The Render_Queue is maintained such that it has two sections: a high
	priority section and a low priority seciton. The high priority
	section preceeds the low priority section (in indexing order). Since
	tiles are drawn from the front of the queue for rendering, the high
	priority tiles will be rendered first.

	An image tile that has a null tile coordinate is defined as a low
	priority tile. It is always appended to the end of the Render_Queue.
	If the Render_Queue is empty the tile is simply appended.

	An image tile that has a non-null tile coordinate is defined as a
	high priority tile. It is inserted in the high priority section of
	the Render_Queue (i.e. before the first entry that has a null tile
	coordinate) in order of the area of the tile's visible image region
	from larger to smaller image regions.

	Finally, if this Image_Renderer is marked as {@link runnable()
	runnable} and it is currently not running the processing thread is
	started.

	As a safety check, the tile image is found in the Delete_Queue that
	tile has its {@link Image_Tile::Delete_Image_When_Done} flag cleared,
	it is removed from the queue and deleted. Also, if the image is
	associated with the Active_Tile its Delete_Image_When_Done flag is
	cleared.

	@param	image_tile	The Image tile to be added to the Render_Queue.
*/
void add_tile (Image_Tile* image_tile);

/**	Clear the Render_Queue.

	<b>>>> WARNING <<<</b> The Queue_Lock is expected to be locked before
	this method is entered.

	An active rendering is {@link abort(int) aborted}.

	All Render_Queue tile entries that are marked as Cancelable are moved
	to the Delete_Queue. If {@link #FORCE_CANCEL} is set in the cancel
	options the Cancelable setting is ignored.

	If the {@link #DELETE_WHEN_DONE} flag is set in the cancel options
	each tile entry moved to the Delete_Queue is marked
	Delete_Image_When_Done to delete its image when the tile is deleted.

	The entries in the Delete_Queue are then {@link delete_tiles()
	deleted}.

	@param	cancel_options	One or more cancel options bit flags. The
		{@link #DELETE_WHEN_DONE} flag marks the tile image to be deleted
		when the tile is deleted from the queue. The {@link
		#WAIT_UNTIL_DONE} flag will block the method from returning until
		the cancellation is complete. Cancellation is complete if the
		Render_Queue has been emptied of all {@link cancellable tiles and
		the active tile, if any, has been aborted. or only the first,
		active, tile remains this Image_Renderer is not, or no longer,
		processing the active tile. Cancel of rendering will be signaled
		but rendering may still be in progress when the method returns.
	@return	true if the thread finished all rendering; false otherwise.
	@see	cancel(int)
	@see	reset(int)
	@see	abort(int)
*/
bool clear (int cancel_options = WAIT_UNTIL_DONE);

/**	Abort rendering.

	<b>>>> WARNING <<<</b> The Queue_Lock is expected to be locked before
	this method is entered.

	If there is no Active_Tile nothing is done.

	The Cancel flag is raised and the image of the Active_Tile is
	notified to {@link Plastic_Image::cancel_update() cancel rendering}.
	Note that when rendering is being done asynchronously (by an
	Image_Renderer_Thread implementation of this class) cancellation does
	not occur immediately: If rendering is in progress the Plastic_Image
	will respond to the next rendering notification that an increment of
	rendered data is available according to the canelation state of the
	image; i.e. once rendering is started - it will not be started if the
	cancel state was already set - it will continue until the completion
	of the current rendering increment.

	If the {@link #DELETE_WHEN_DONE} flag is set in the cancel options
	the Active_Tile is marked to delete its image when the tile is
	destroyed.

	If the {@link #WAIT_UNTIL_DONE} flag is set in the cancel options the
	Rendering_Lock is acquired - which waits for the canceled rendering
	of the current tile image to complete - before the Rendering_Lock is
	released again. However, the attempt to acquire the lock will timeout
	after 20 seconds, in which case the method will return false. If the
	cancel options has the {@link #DO_NOT_WAIT} value then waiting for
	rendering to complete will not be done and the method returns false
	immediately.

	<b>WARNING</b>: The image of the active tile must not be immediately
	deleted by the caller if the return value is false; any tile image
	queued for rendering should only be deleted after the image rendering
	has been successfully {@link cancel(Plastic_Image*, bool) canceled},
	or the rendering loop is not active. This is best done by {@link
	delete_image(Plastic_Image*) queueing the image for deletion} by the
	Image_Renderer when it is not rendering.
	
	@param	cancel_options	One or more cancel options bit flags. The
		{@link #DELETE_WHEN_DONE} flag marks the tile image to be deleted
		when the tile is deleted from the queue. The {@link #WAIT_UNTIL_DONE}
		flag will block the method from returning until the cancellation
		is complete. Cancellation is complete if the Render_Queue is empty or
		rendering of the active tile, if there is one, has been completed.
	@return	true if their was no Active_Tile being rendered or
		WAIT_UNTIL_DONE was set in the cancel options and rendering
		completed within the allowed wait time; false if there is an
		Active_Tile with rendering still be in progress when the method
		returns.
*/
bool abort (int cancel_options = WAIT_UNTIL_DONE);

/**	Queue an image tile for deletion.

	<b>>>> WARNING <<<</b> The Queue_Lock is expected to be locked before
	this method is entered.

	The image tile is placed on the Delete_Queue, unless it or a tile
	with its image is already queued.

	Image tiles queued for deletion are guaranteed to only be placed on
	the queue once and will never have the same image as any other tile
	on the queue. Whenever an existing queued tile is found to have the
	same image as the specified tile, and the tiles are not the same
	tile, the queued tile will have its Delete_When_Done flag set if, and
	only if, the specified tile has the flag set; in this case the
	specified tile flag will be reset before it is deleted.

	>>> CAUTION <<< The image tile queued for deletion must be removed
	from the Render_Queue. Never queue the Active_Tile for deletion; it
	is the responsibility of the rendering loop.

	@param image_tile	A pointer to the Image_Tile to be deleted.
	@see	delete_tiles()
*/
void delete_tile (Image_Tile* image_tile);

/**	Delete all entries in the Delete_Queue.

	Each Image_Tile pointer entry in the Delete_Queue is removed and the
	object referred to is deleted. On return the queue will be empty.

	<b>N.B.</b>: The image associated with the tile is only deleted if it
	had been marked {@link #DELETE_WHEN_DONE}.

	<b>WARNING</b>: The Queue_Lock may be locked before this method is
	entered. The method will try to lock the Queue_Lock which will lock
	the mutex if it has not already been locked. However, if the mutex
	is already locked the method proceeds. Therefor this method is only
	for use within the thread where this Image_Renderer is running; for
	any other thread access to the method would not be blocked while
	the Image_Renderer thread is manipulating the queue.

	@see	delete_tile(Image_Tile*)
	@see	delete_image(Plastic_Image*)
*/
void delete_tiles ();

/*==============================================================================
	Rendering
*/
public:

/**	Start the rendering loop.

	The rendering loop is set to be runnable and not suspended. Then the
	rendering loop is {@link run_rendering() run}.

	@see	render()
	@see	suspend_rendering(bool)
	@see	stop_rendering(bool)
*/
virtual void start_rendering ();

/**	Suspend the rendering loop.

	Any rendering in progress will continue to completion after which the
	rendering loop will stop; any images that are still in the {@link
	queue(Plastic_Image*, const QPoint&, bool) rendering queue} will
	remain queued. The rendering loop remains runnable: if an image is
	{@link add_tile(Image_Tile*) added} to the queue rendering will
	automatically begin again.

	@param	wait	true if the thread should try to wait until any
		redering in progress has completed before returning; false
		otherwise.
	@return	true if rendering is suspended when the method returns;
		false if wait was false and rendering was not suspended when
		this method was called, or if wait was true and waiting for
		rendering to complete timed out.
	@see	suspended()
	@see	stop_rendering(bool)
	@see	start_rendering()
*/
virtual bool suspend_rendering (bool wait = false);

/**	Test if the rendering loop has been {@link suspend_rendering(bool)
	suspended}.

	<b>N.B.</b>: A suspended Image_Renderer may still be in the processes
	of completing a rendering operation.

	@return	true if this Image_Renderer has been suspended; false
		otherwise.
	@see	suspend_rendering(bool)
*/
bool suspended () const;

/**	Stop the rendering loop.

	Rendering is {@link suspend_rendering(bool) suspended} and the
	rendering loop is set to no longer be {@link runnable() runnable};
	the rendering loop will not be automatically restarted when another
	image is {@link queue(Plastic_Image*, const QPoint&, bool) queued}
	for rendering. The rendering loop must be explicitly {@link
	start_rendering() started} to begin rendering again.

	<b>N.B.</b>: If this Image_Renderer is an Image_Renderer_Thread -
	i.e. it is running in its own thread - the thread remains running but
	its execution will be blocked until rendering is started again.

	@param	wait	true if the thread should try to wait until any
		redering in progress has completed before returning; false
		otherwise.
	@return	true if rendering is stopped when the method returns;
		false if wait was false and rendering was not stopped when
		this method was called, or if wait was true and waiting for
		rendering to complete timed out.
	@see	start_rendering()
	@see	finish(int)
*/
bool stop_rendering (bool wait = false);

/**	Test if the rendering loop is runnable when a source is registered
	for loading or an image is queued for rendering.

	A runnable Image_Renderer will be {@link start_rendering() started},
	if it is {@link runnable() runnable}, when a source {@link
	image(const QString&) name} or {@link image(const QImage&) image} is
	registered or an image is {@link queue(Plastic_Image*, const
	QPoint&, bool) queued} for rendering. If the Image_Renderer is not
	runnable registering a source or queuing an image will not start
	rendering.

	@return	true if this Image_Renderer is runnable; false otherwise.
	@see	start_rendering()
	@see	stop_rendering(bool)
*/
bool runnable () const;

/**	Finish running the rendering loop.

	The rendering loop is flagged to finish running. The rendering loop
	is {@link reset(int) reset} which canels any rendering in progress
	and clears the rendering queue using the specified cancel options -
	the FORCE_CANCEL option is always applied - and any tiles in the
	delete queue are {@link delete_tiles() deleted}.

	@param	cancel_options	One or more cancel option flags to be applied
		when rendering is {@link cancel(int) canceled}.
	@return	true
*/
virtual bool finish (int cancel_options = WAIT_UNTIL_DONE);

//------------------------------------------------------------------------------
protected:

/**	Load any pending new source image and render all queued image tiles.

	A loop is entered that checks the Render_Queue for images to be rendered.
	The render loop continues until all rendeing is {@link finish(int)
	finished.

	At the beginning of the render loop a check is made that {@link
	is_ready() rendering may proceed}.

	If a {@link image(const QString&) source name} or {@link image(const
	QImage&) source image} has been registered it is {@link load_image()
	loaded}.

	If the Render_Queue is empty {@link suspend_rendering(bool) rendering
	is suspended} and the loop is restarted.

	If an image tile is in the Render_Queue its image {@link status(int)
	rendering status is signaled} and the image is rendered. If the
	rendering is not {@link cancel(int) canceled} and the tile is a
	{@link Image_Tile::is_high_priority() high priority} or {@link
	queue(Plastic_Image*, const QPoint&, bool) queued as not cancelable}
	then the {@link rendered(const QPoint&) rendered signal} will be
	emitted. However, if {@link immediate_mode(bool) immediate mode} as
	not been enabled then the signal will only be emitted when all high
	priority tiles in the queue have completed rendering.
*/
void render ();

/**	Run the rendering loop.
*/
virtual void run_rendering ();

/**	Test if the rendering loop is to continue;

	<b>N.B.</b>: The Ready_Lock is locked during this method.

	This method is called in each cycle of the rendering loop. <b>N.B.</b>:
	This method is only used by a synchronous Image_Renderer; the
	Image_Renderer_Thread subclass overrides this method to provide
	thread blocking control.

	If this Image_Renderer is not runnable or is in the the process of
	finishing the {@link status(int) status} signal is emitted with the
	{@link #NOT_RENDERING} condition.

	@return true if this Image_Renderer is runnable and is not in the
		the process of finishing.
*/
virtual bool is_ready ();

/**	Send the {@link rendered(const QPoint&, const QRect&) rendered signal}.

	This method is used as a proxy for the {@link
	Plastic_Image::Rendering_Monitor::notification(Plastic_Image&,
	Status, const QString&, const QRect&) rendering monitor notification}
	call back handler to send the rendered signal for the owner of the
	Rendering_Monitor.

	@param	tile_coordinate	A QPoint specifying the display tile grid
		coordinate of the tile being rendered. May be null (have zero
		values).
	@param	tile_region	A QRect specifying the region of the tile image,
		in tile-relative coordinates, that is visible in the display
		viewport. May be empty.
*/
void send_rendered
	(const QPoint& tile_coordinate, const QRect& tile_region);

/**	Send the {@link status_notice(const QString&) status notice signal}.

	This method is used as a proxy for the {@link
	Plastic_Image::Rendering_Monitor::notification(Plastic_Image&,
	Status, const QString&, const QRect&) rendering monitor notification}
	call back handler to send the status notice signal for the owner of the
	Rendering_Monitor.

	@param	message	A QString forwarded from the rendering progress status
		notice message.
*/
void send_status_notice (const QString& message);

/*==============================================================================
	Utilities
*/
public:

/**	Fit a size to an area.

	The scaling factor for the given size that will maximally fill, but
	not exceed, the given area is determined.

	@param	size	A QSize to be fitted to the area.
	@param	area	The area within which to fit the size.
	@return	The scaling factor that when applied to the specified size
		will produce a new size that will be as large as possible within
		the specified area.
*/
static double scale_to_area (const QSize& size, unsigned long area);

/**	Get a brief description of a status condition value.

	@param	condition	A {@link status(int) status} condition value.
	@return	A QString that briefly describes, in a single line, the
		status condition value.
*/
static QString status_description (int condition);

/**	Get a brief description of a cancel option code.

	@param	cancel_options	A cancel option code.
	@return	A QString that briefly describes, in a single line, the
		cancel options.
*/
static QString cancel_options_descriptions (int cancel_options);

/*------------------------------------------------------------------------------
	Accounting
*/
//	DEBUG only methods.
void print_render_queue () const;
void print_delete_queue () const;

/*	Print a list of extant Plastic_Images constructed by an Image_Renderer.

	<b>N.B.</b>: This method is a no-op unless the Image_Renderer class has
	been compiled with DEBUG defined.

	When an image is {@link clone_image(Plastic_Image*, const QSize&,
	Plastic_Image::Mapping_Type) cloned} it is appended to the
	list of extant Image_Tiles and when it is {@link delete_tiles()
	deleted} it is removed from the list. The {@link source_image()
	source image} is not included in the list.
*/
void image_accounting ();

/*==============================================================================
	Qt signals
*/
signals:

/**	Signal the completion of {@link load_image() image loading}.

	This signal is always emitted <b>after</b> image loading has been
	attempted.

	@param	successful	true if the an image was successfully loaded;
		false otherwise.
*/
void image_loaded (bool successful);

/**	Signal tile image rendering progress.

	This signal is emitted from the {@link
	Plastic_Image::Rendering_Monitor} when the Active_Tile does not
	have an empty Tile_Region - i.e. some portion of the tile image
	appears in the display - and the {@link
	Plastic_Image::Rendering_Monitor::notification(Plastic_Image&,
	Status, const QString&, const QRect&) rendering notification} {@link
	Plastic_Image::Rendering_Monitor::Status status} has an active
	rendering bit set. Note that non-rendering status nofications do not
	result in this signal being emitted from the Rendering_Monitor.

	This signal is emitted from the main {@link render() render}
	loop <b>after</b> a tile has completed rendering without being
	canceled, if the tile has high priority status (i.e. it is visible in
	the display viewport) or is marked as not cancelable. When {@link
	immediate_mode(bool) immediate mode} is in effect the signal includes
	the tile's display tile grid coordinate. Otherwise the signal is only
	emitted when the next tile in the queue has low priority status or
	the rendering queue is empty, and then the signal always has a null
	tile coordinate. The signal emitted from the render loop always has
	an empty tile region.

	@param	tile_coordinate	A QPoint specifying the display tile grid
		coordinate of the tile being rendered. May be null (have zero
		values).
	@param	tile_region	A QRect specifying the region of the tile image,
		in tile-relative coordinates, that is visible in the display
		viewport. May be empty.
*/
void rendered
	(const QPoint& tile_coordinate = QPoint (),
	 const QRect& tile_region = QRect ());

/**	Provide a tile image rendering progress status notice message.

	This signal is emitted from the {@link
	Plastic_Image::Rendering_Monitor::notification(Plastic_Image&,
	Status, const QString&, const QRect&) rendering monitor notification}
	to forward the rendering progress status notice message.

	@param	message	A QString forwarded from the rendering progress status
		notice message.
*/
void status_notice (const QString& message);

/**	Signal the status of the renderer.

	This signal is emitted <b>before</b> {@link load_image() image
	loading} commences with the {@link #LOADING_IMAGE} value.
	<b>N.B></b>: No status signal is emitted at the completion of image
	loading; instead an {@link image_loaded(bool) image loaded} signal
	is emitted.

	This signal is emitted from <b>before</b> {@link start_rendering()
	rendering is started} with the {@link #RENDERING_HIGH_PRIORITY}
	value if rendering is {@link suspended() suspended} and the {@link
	finish(int) finish} state is not in effect.

	This signal is emitted from the {@link is_ready() is ready} test
	with the {@link #NOT_RENDERING} value when rendering has been
	{@link suspend_rendering(bool) suspended}. For the {@link
	Image_Renderer_Thread::is_ready() asynchronous renderer} the signal
	is emitted immediately <b>before</b> waiting for the ready event
	latch to clear.

	This signal is emitted from the main {@link render() render} loop
	with the Active_Tile priority status (either {@link
	#RENDERING_LOW_PRIORITY} or {@link #RENDERING_HIGH_PRIORITY})
	immediately <b>before</b> the Active_Tile Image is updated (i.e.
	rendered if any updates are pending}.

	This signal is emitted from the main {@link render() render} loop
	with the Active_Tile priority status (either {@link
	#RENDERING_LOW_PRIORITY} or {@link #RENDERING_HIGH_PRIORITY}) and
	the {@link #RENDERING_CANCELED} qualifier bit set if rendering was
	canceled for any reason. The signal is emitted <b>after</b> any
	rendering in progress was canceled. Note that only rendering of
	the Active_Tile may have been canceled rather than all queued
	rendering; check if any image {@link is_queued(Plastic_Image*)
	is queued} to make the distinction.

	@param	An Image_Tile status value.
	@see status_description(int)
*/
void status (int condition);

/**	Signal an error condition.

	This signal is emitted when rendering did not complete during the
	wait interval when the rendering queue was {@link clear(int) cleared}.

	This signal is emitted from the main {@link render() render} loop
	to forward a {@link Plastic_Image::Render_Exception} caught from
	the rendering of the Active_Tile Image. The signal is emitted
	immediately <b>before</b> the rendering queue is {@link reset(int)
	reset}.

	This signal is emitted from the {@link load_image(const QString&)
	load image from source name} procedure if the source was found to
	be invalid in some way.

	This signal is emitted from the {@link load_JP2_image(const QString&)
	JP2 image loader} if an exception was thrown from the JP2 reader.

	This signal is emitted from the {@link clone_image(Plastic_Image*,
	const QSize&, Plastic_Image::Mapping_Type) image cloning} procedure
	to forward any exception that is thrown.

	@param	message	A QString describing the error condition.
*/
void error (const QString& message);

/*==============================================================================
	Qt slots
*/
public slots:

/*	Cancel rendering and {@link clear(int) clear} the queue.

	Any rendering in progress is {@link abort(int) aborted} and the
	rendering queue is {@link clear(int) cleared}.

	@param	cancel_options	One or more cancel options bit flags. The
		{@link #DELETE_WHEN_DONE} flag marks the tile image to be deleted
		when the tile is deleted from the queue. The {#WAIT_UNTIL_DONE}
		flag will block the method from returning until the cancellation
		is complete. Cancellation is complete if the Render_Queue is empty or
		this Image_Renderer is not, or no longer, processing the active
		tile. Cancel of rendering will be signaled but rendering may
		still be in progress when the method returns.
	@return	true if the thread finished before returning; false otherwise.
	@see	clear(int)
	@see	cancel(Plastic_Image*, int)
*/
bool cancel (int cancel_options = WAIT_UNTIL_DONE);

/*==============================================================================
	Data
*/
protected:

//	Ready_Lock access controlled -----------------------------------------------
mutable QMutex
	Ready_Lock;

//!	The rendering loop has been suspended.
bool
	Suspended;
//!	The rendering loop may be run.
bool
	Runnable;
//!	Flag the rendering loop to finish.
bool
	Finish;


private:

//	Mode_Lock access controlled ------------------------------------------------
mutable QMutex
	Mode_Lock;
//!	Provide immediate mode rendered signal.
bool
	Immediate_Mode;


//	Queue_Lock access controlled -----------------------------------------------
mutable QMutex
	Queue_Lock;

//!	Rendered source image from which all others are derived.
Shared_Image
	Source_Image;

//!	Maximum amount of space to use when loading a Source_Image.
unsigned long
	Max_Source_Image_Area;

/**	The reference image (zero size) from which clones are copied
	and with which band and data mapping is shared.
*/
Plastic_Image
	*Reference_Image;

//!	Source name (pathname or URL) of the source image to load.
QString
	Source_Name;
//!	Image to use for the Source_Image.
Shared_Image
	Image_Source;


//!	The queue of image tiles to be rendered.
Tile_Queue
	Render_Queue;

//!	The queue of image tiles to be deleted.
Tile_Queue
	Delete_Queue;

/**	The active tile has been acquired for rendering.

	The active tile is always acquired from the first Render_Queue entry
	and will only be non-NULL when the renderer is running and has acquired
	an active tile.

	The Active_Tile is guarded by the Queue_Lock.
*/
Image_Tile
	*Active_Tile;

//!	The current rendering operation has been canceled.
bool
	Cancel;


//	Rendering_Lock access control ----------------------------------------------
/*	Render only one image at a time.

	Because it is likely that there will be more than one Image_Renderer
	in use by an application, the source image being rendered may be
	shared amongst them, and image rendering may be done asynchronously
	in a multi-threaded context, it is important to ensure that image
	rendering is done synchrously, one at a time, across all
	Image_Renderer objects. Therefor the rendering lock is a class data
	member rather than an object data member.
*/
static QMutex
	Rendering_Lock;

friend class Image_Renderer_Rendering_Monitor;
Image_Renderer_Rendering_Monitor
	*Image_Rendering_Monitor;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
