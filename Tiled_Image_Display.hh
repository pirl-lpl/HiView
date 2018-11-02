/*	Tiled_Image_Display

HiROC CVS ID: $Id: Tiled_Image_Display.hh,v 1.72 2013/09/09 22:11:55 stephens Exp $

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

#ifndef HiView_Tiled_Image_Display_hh
#define HiView_Tiled_Image_Display_hh

#include	"Plastic_Image.hh"

//	PIRL++
#include	"Reference_Counted_Pointer.hh"

#include	<QWidget>

//	Forward references.
template<typename T> class QVector;
template<typename T> class QList;
class QPaintEvent;
class QMouseEvent;
class QErrorMessage;

namespace idaeim {
namespace PVL {
class Aggregate;
}}


namespace UA
{
namespace HiRISE
{
//	Forward references.
class	Image_Renderer;

/**	A <i>Tiled_Image_Display</i> provides a QWidget for the display of
	a grid of image tiles rendered from a Plastic_Image.

	The image displayed in the QWidget viewport is provided by rendering
	a grid of image tiles based on a single Plastic_Image. In the
	degenerate case where the Plastic_Image fully encapsulates a single,
	in-memory image (a Plastic_QImage) a single tile the size of the
	source image is used for the display. When the source image can be
	dynamically rendered from its data source with a selectable rendering
	region and/or scale (e.g. a JP2_Image) multiple tiles will be used
	for the display if the source image is larger than a single tile.

	Each tile grid has a fixed {@link tile_display_size() display size} -
	i.e. all tiles have the same size - that is set when the source image
	is first encountered, with an {@link tile_image_size(int) image
	size} that depends on the current {@link image_scaling(int) image
	scaling}. The tile size may be changed at any time. The origin of the
	logical tile grid is set at the origin of image band zero.

	To accommodate very large images only a limited region of the logical
	tile grid may be instantiated for rendering at any time. The
	effective tile grid is larger than necessary for the display viewport
	to provide for look-ahead rendering of the image in anticipation of
	local region panning. Thus the effective tile grid will always
	include a margin of tiles around the displayed tile grid; the displayed
	tile grid includes those tiles that contain the image appearing on
	the display viewport. The tile grid margin is one tile above and to
	the left of the displayed tile grid, and one or two tiles below and to
	the right of the displayed tile grid. The possible additional margin
	tiles below and to the right of the displayed tile grid are provided
	when the size of the display is such that moving the display viewport
	origin to the lower right corner of the viewport origin tile would
	result in the lower right corner of the viewport falling in the first
	margin tile; having the extra tile margin prevents "thrashing" of
	tile instantiation -  construction and destruction - if the display
	boundary moves back and forth across the single tile margin leading
	edge. The {@link displayed_tile_grid_origin(int) origin of the displayed
	tile grid} in image space does not include the margin tiles; i.e. the
	tile grid at location 1,1 has its initial origin (upper left corner)
	at image location 0,0; the tile grid at location 0,0 has its initial
	origin at -Tile_Image_Size.width,-Tile_Image_Size.height. 

	The {@link image() source image}, whether generated locally or
	provided from an external source, can be shared elsewhere (e.g. with
	other Tiled_Image_Display objects) without the need for a copy; it
	will not be modified. The source image is {@link
	Plastic_Image::clone(const QSize&, Plastic_Image::Mapping_Type)
	cloned} to provide a master reference image that has zero size. Each
	tile grid image is instantialed as a clone clone} of the reference
	image, sharing the {@link Plastic_Image::source_band_map() band map}
	and {@link Plastic_Image::source_data_maps() data maps}, but having
	its own {@link Plastic_Image::source_transforms() geometric
	transforms} set according to the tile's image scaling and location in
	the image tile grid. Some of the tiles in the tile grid margin may
	not be instantiated. For example, if the display origin is located at
	the image origin then the first tile row - the top margin - and the
	first tile column - the left margin - will not be instatiated since
	there is no image there to render. Similarly, the bottom and right
	margins may also not be instantiated if the corresponding edges of
	the image are within the display viewport. Tile rows that are not
	instantiated are NULL; individual tiles that are not instantiated in
	a partitially instatiated tile row are NULL. When the displayed tile
	grid is {@link reset_tiles() reset} uninstatiated tile rows and tiles
	that overlap the image area are instantiated; those tiles and tiles
	rows that are instantiated but no longer overlap the image area are
	deleted and NULL pointers left in their place.

	The Tile_Image_Display manages the effective tile grid. It is resized
	as needed when the display viewport is resized. The tile locations
	and scaling are adjusted when the image is to be relocated or scaled
	in the display viewport. Tiles are instantiated or deleted as needed.
	The QWidget is painted by painting the tiles from the displayed tile
	grid onto their relative locations in the display viewport. When the
	viewport is painted, if a tile grid image has not yet been rendered
	to its expected display state, the source image is used to paint the
	corresponding grid area until the rendered tile grid image becomes
	available.

	Alignment of the source image with the display image: Scaling of the
	source image into the display image can result in display image
	pixels not being an integer multiple, or divisor, of the source image
	pixels. Scaling is done by the Plastic_Image class implementation
	for the type of source image which will interpolate source data into
	the display data as seems appropriate (a nearest-neighbor technique
	is typical). However, the image edge alignment issue remains. The
	Tiled_Image_Display will always align the top and left edges of the
	source image with the display image; i.e. the top and left display
	viewport image edges will always be at a source image pixel boundary.
	When the source image extends beyond the bottom or right display
	viewport boundary the edge of the display viewport may, or may not,
	occur at a source image pixel boundary depending on the image
	scaling. Note that the tile image grid is aligned to the unscaled
	source image and so tile edges always occur at pixel boundaries.
	However, when a tile image is rendered into a display image partial
	source pixels are included at the top and left edges and excluded
	at the bottom and right edges. This ensures that a source pixel
	occurs only once in all the tile grid images.

	Qt signals are provided for {@link image_loaded(bool) image load
	completion}, {@link image_cursor_moved(const QPoint&, const QPoint&)
	the position of the mouse cursor}, {@link image_moved(const QPoint&,
	int) image movement}, {@link displayed_image_region_resized(const
	QSize&) visible image area change}, {@link
	display_viewport_resized(const QSize&) viewport resizing}, {@link
	image_scaled(const QSizeF&, int) image scaling}, {@link
	rendering_status(int) tile rendering status} and {@link
	rendering_status_notice(const QString&) rendering status notice}
	reporting, and {@link state_change(int) display state changes}.

	@author	Bradford Castalia, UA/HiROC
	@version $Revision: 1.72 $
*/
class Tiled_Image_Display
	//	N.B.: the QWidget must be listed first to be noticed by Q_OBJECT.
:	public QWidget,
	//	The Metadata_Monitor receives metadata_changed notifications.
	public Plastic_Image::Metadata_Monitor
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Types:
*/
typedef PIRL::Reference_Counted_Pointer<Plastic_Image>	Shared_Image;

typedef Plastic_Image::Mapping_Type						Mapping_Type;
typedef Plastic_Image::Data_Map							Data_Map;
typedef Plastic_Image::Histogram						Histogram;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Minimum allowable tile size dimension.
static const int
	MINIMUM_TILE_DIMENSION;

/*	>>> CAUTION <<< These values must be the same as the corresponding
	symbols in the Image_Renderer.
*/
//!	The {@link rendering_status(int) rendering status} values.
enum
	{
	NOT_RENDERING				= 0,
	RENDERING_BACKGROUND		= (1 << 0),
	RENDERING_VISIBLE_TILES		= (1 << 1),
	LOADING_IMAGE				= RENDERING_BACKGROUND |
								  RENDERING_VISIBLE_TILES,
	RENDERING_CANCELED			= (1 << 2)
	};

//!	{@link state_change(int) state change} bit flags.
enum
	{
	NO_STATE_CHANGE						= 0,
	IMAGE_LOAD_STATE					= (1 << 0),
	DISPLAY_SIZE_STATE					= (1 << 1),
	TILE_SIZE_STATE						= (1 << 2),
	IMAGE_MOVE_STATE					= (1 << 3),
	IMAGE_SCALE_STATE					= (1 << 4),
	BAND_MAPPING_STATE					= (1 << 5),
	DATA_MAPPING_STATE					= (1 << 6),
	STATE_TYPE_MASK	=
		IMAGE_LOAD_STATE   |
		DISPLAY_SIZE_STATE |
		TILE_SIZE_STATE    |
		IMAGE_MOVE_STATE   |
		IMAGE_SCALE_STATE  |
		BAND_MAPPING_STATE |
		DATA_MAPPING_STATE,

	RENDERING_VISIBLE_TILES_STATE		= (1 << 7),
	RENDERING_BACKGROUND_STATE			= (1 << 8),
	RENDERING_CANCELED_STATE			= (1 << 9),
	RENDERING_COMPLETED_STATE			= (1 << 10),
	RENDERING_VISIBLE_TILES_COMPLETED_STATE =
		RENDERING_VISIBLE_TILES_STATE |
		RENDERING_BACKGROUND_STATE    |
		RENDERING_COMPLETED_STATE,
	COMPLETED_WITHOUT_RENDERING_STATE	= (1 << 11),
	STATE_QUALIFIER_MASK =
		RENDERING_VISIBLE_TILES_STATE |
		RENDERING_BACKGROUND_STATE    |
		RENDERING_CANCELED_STATE      |
		RENDERING_COMPLETED_STATE     |
		COMPLETED_WITHOUT_RENDERING_STATE
	};

/*------------------------------------------------------------------------------
	Defaults
*/
protected:

//!	Default size of an image rendering tile in the display space.
static QSize
	Default_Tile_Display_Size;

//!	Image scaling limits.
static double
	Min_Scale,
	Max_Scale;

//!	Render Source_Image here default.
static bool
	Default_Source_Image_Rendering;

/*==============================================================================
	Constructors
*/
public:

/**	Construct a Tiled_Image_Display with no image.

	@param	parent	A pointer to the parent QWidget for this widget.
		May be NULL.
*/
Tiled_Image_Display (QWidget* parent = NULL);

virtual ~Tiled_Image_Display ();

/*==============================================================================
	Accessors
*/
/** Get the Saved Coordinate from the last click on the image
    @return A QPoint containing the last valid coordate clicked.
*/
    const QPoint Get_Saved_Coordinate();
    
/**	Get the name of the image source.

	@return	A QString providing the name of the image source. This may be
		a pathname to a local file, a URL for a remote JPIP server
		source, or an arbitrary name. It may be empty if the source of
		the image being displayed in not known.
*/
inline QString image_name () const
	{return Source_Image->source_name ();}

/**	Request that an image be loaded for display.

	<b>N.B.</b>: The image is not loaded immediately; the source name is
	registered with the Image_Renderer which will asynchronously load the
	source and reference image rendering components for display. When
	image loading is complete the {@link image_loaded(bool) image_loaded}
	signal will be emitted.

	@param	source_name	A QString naming the source of the image. This
		may be a local image file pathname or a remote JPIP server URL.
		If an emtpy string is specified nothing is done.
	@param	scaling	A QSizeF that specifies how the image is to be scaled
		when first loaded. The scaling values are clipped to be in the
		(@link min_scale() minimum scaling} and {@link max_scale() maximum
		scaling} range, inclusive.
	@return	true if the image was registered for display; false otherwise.
		A source will fail to be registered if a source is currently
		registered (because it has not completed loading) or it is
		identical to the current {@link image() image} being displayed.
	@see	image(const QString&, const QSize&)
*/
bool image (const QString& source_name,
	const QSizeF& scaling = QSizeF (1.0, 1.0));

/**	Request that an image be loaded for display.

	<b>N.B.</b>: The image is not loaded immediately; the source name is
	registered with the Image_Renderer which will asynchronously load the
	source and reference image rendering components for display. When
	image loading is complete the {@link image_loaded(bool) image_loaded}
	signal will be emitted.

	@param	source_name	A QString naming the source of the image. This
		may be a local image file pathname or a remote JPIP server URL.
		If an emtpy string is specified nothing is done.
	@param	display_size	A QSize specifying the desired display size of
		the image to which it will be scaled when loaded. If the size is
		empty the current size of the display viewport will be used; if
		this is empty the widget {@link sizeHint() size hint} will be used.
	@return	true if the image was registered for display; false otherwise.
		A source will fail to be registered if a source is currently
		registered (because it has not completed loading) or it is
		identical to the current {@link image() image} being displayed.
	@see	image(const QString&, const QSizeF&)
*/
bool image (const QString& source_name,
	const QSize& display_size);

/**	Request that an image be loaded for display.

	<b>N.B.</b>: The image is not loaded immediately; the source image is
	registered with the Image_Renderer which will asynchronously load the
	reference image rendering component for display. When image loading
	is complete the {@link image_loaded(bool) image_loaded} signal will
	be emitted.

	@param	source_image	A pointer to a Plastic_Image held in a
		Shared_Image reference counted pointer object.
	@param	scaling	A QSizeF that specifies how the image is to be scaled
		when first loaded. The scaling values are clipped to be in the
		(@link min_scale() minimum scaling} and {@link max_scale() maximum
		scaling} range, inclusive.
	@return	true if the image was registered for display; false otherwise.
		A source will fail to be registered if a source is currently
		registered (because it has not completed loading) or it is
		identical to the current {@link image() image} being displayed.
	@see	image(const QString&, const QSizeF&)
*/
bool image (const Shared_Image& source_image,
	const QSizeF& scaling = QSizeF (1.0, 1.0));

/**	Request that an image be loaded for display.

	<b>N.B.</b>: The image is not loaded immediately; the source image is
	registered with the Image_Renderer which will asynchronously load the
	reference image rendering component for display. When image loading
	is complete the {@link image_loaded(bool) image_loaded} signal will
	be emitted.

	@param	source_name	A QString naming the source of the image. This
		may be a local image file pathname or a remote JPIP server URL.
		If an emtpy string is specified nothing is done.
	@param	display_size	A QSize specifying the desired display size of
		the image to which it will be scaled when loaded. If the size is
		empty the current size of the display viewport will be used; if
		this is empty the widget {@link sizeHint() size hint} will be used.
	@return	true if the image was registered for display; false otherwise.
		A source will fail to be registered if a source is currently
		registered (because it has not completed loading) or it is
		identical to the current {@link image() image} being displayed.
	@see	image(const QString&, const QSize&)
*/
bool image (const Shared_Image& source_image,
	const QSize& display_size);

/**	Get the source image for the currently displayed image.

	@return A Shared_Image reference counted pointer object holding
		the pointer to the Plastic_Image used as the source image
		for the display.
*/
inline Shared_Image image () const
	{return Source_Image;}

inline static void default_source_image_rendering (bool enabled)
	{Default_Source_Image_Rendering = enabled;}
inline static bool default_source_image_rendering ()
	{return Default_Source_Image_Rendering;}
inline void source_image_rendering (bool enabled)
	{Source_Image_Rendering = enabled;}
inline bool source_image_rendering ()
	{return Source_Image_Rendering;}

/**	Set the suggested rendering increment.

	<b>N.B.</b>: The rendering increment of all curent and future tiles
	is changed.

	@param	rendering_increment	The number of display image lines to
		incrementally render that will be suggested to the rendering
		implementation.
	@see	rendering_increment_lines()
*/
void rendering_increment_lines (int rendering_increment);
inline static int rendering_increment_lines ()
	{return (int)Plastic_Image::default_rendering_increment_lines ();}

void background_color (QRgb color);
inline static QRgb background_color ()
	{return Plastic_Image::default_background_color ();}

/**	Set the maximum image area to use when loading a JP2 source image.

	When a JP2 {@link image() source image} is loaded from a {@link
	image(const QString&) named image source} it may be exceedingly
	large, possibly too large to fit into memory. So the image will be
	rendered into a limited area. A scaling factor for the image will be
	determined by fitting the image size to the maximum image amount and
	limiting the scaling factor to be less then 1.0.

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
void max_source_image_area (unsigned long area);
unsigned long max_source_image_area () const;

static void default_max_source_image_area (unsigned long area);
static unsigned long default_max_source_image_area ();

/**	Get the image metadata.

	The metadata parameters will include at least a group of {@link
	#Plastic_Image::IMAGE_METADATA_GROUP} parameters with basic image
	information. For a JP2_Image a group of {@link
	#JP2_Image::JP2_METADATA_GROUP} parameters that provides details
	about the JP2 source data will also be included. Additional metadata
	may be provided at the discretion of the source image implementation.

	<b>N.B.</b>: Changes to the metadata parameters may occur after the
	initial metadata has been obtained. In these cases a {@link
	metadata_changed(Plastic_Image&) metadata changed} notification will
	be produced by the source image which should then result in an {@link
	image_metadata_changed(idaeim::PVL::Aggregate*) image metadata
	changed signal} being emitted.

	@return	A pointer to the metadata parameters.
*/
inline idaeim::PVL::Aggregate* image_metadata () const
	{return Source_Image->metadata ();}

/**	Receives notifications of changes to image metadata.

	<b>For internal use only.</b>

	This method implements the metadata_changed pure virtual method of
	the Plastic_Image::Metadata_Monitor, of which a Tiled_Image_Display
	is a subclass. This Tiled_Image_Display object is expected to be
	{@link Plastic_Image::add_metadata_monitor(Metadata_Monitor*)
	registered} with the source image. If changes to the metadata occur
	after the initial {@link image_metadata() image metadata} has been
	obtained this method will be called as notification that a change has
	occured in the parameters. This method then emits the {@link
	image_metadata_changed(idaeim::PVL::Aggregate*) image metadata
	changed signal}.

	@param	image	A reference to the Plastic_Image for which the
		metadata changed. This should be the {@link image() source
		image}.
*/
virtual void metadata_changed (Plastic_Image& image);

inline QSize image_size () const
	{return Source_Image->source_size ();}
inline int image_width () const
	{return Source_Image->source_width ();}
inline int image_height () const
	{return Source_Image->source_height ();}

/**	Get the size of the scaled image.

	@param	band	The image band for which the image size is desired.
		If negative, the size inclusive of all image bands is obtained.
	@return	A QSize specifying the size of the image after the {@link
		image_scaling(int) image scaling} has been applied. The
		dimensions are rounded up to the nearest integer; i.e. a scaled
		partial pixel at an image boundary occupies a whole pixel
		location. <b>N.B.</b>: If the band is negative then the scaled
		image size encloses all displayed bands, which includes band
		origin offsets and different band scalings.
*/
QSize scaled_image_size (int band = 0) const;

/**	Get the image coordinate at the display origin.

	@param	band	The image band for which to get the display origin.
		If less than zero the reference band (0) will be used.
	@return	A QPointF coordinate in unscaled image space corresponding
		to the display origin (0,0; the upper-left corner). <b>N.B.</b>:
		Floating point image coordinate values are provided for accuracy
		when accuracy is needed (e.g. scaling). The values should be
		truncated (excess partial pixels clipped off) if integer values
		are needed, NOT rounded to the nearest integer.
	@see displayed_image_region(int, bool)
*/
QPointF displayed_image_origin (int band = 0) const;

/**	Get the image origin lower right limit point.

	When setting the {@link move_image (const QPoint&, int) image display
	origin} the origin point is contrainted to be within the image
	boundaries minus the viewport size in scaled image space on the
	bottom and right. This prevents placing the image display origin above
	and to the left of the image, and so far down and to the right that
	the display viewport would be beyond the image boundaries. However,
	when the scaled image size is less than the size of the display
	viewport lower right boundaries must necessarily be beyond the image
	boundaries. In this case any limit value that would be negative for
	the dimension in which the vieport size exceeds the scaled image size
	will be clipped to zero.

	@return	A QPoint coordinate specifing the lower right image display
		origin limit.
	@see	calculate_lower_right_origin_limit()
*/
QPoint lower_right_origin_limit () const
	{return Lower_Right_Origin_Limit;}

/**	Map a display coordinate to its image coordinate.

	The display origin in image space is held by the {@link
	source_image() source image} origin. The display coordinate is
	relative to this origin (i.e. it is 0,0 at the display origin). It is
	scaled by the scaling factors for the band and offset by the source
	origin to produce the image space coordinate.

	@param	coordinate	A QPoint coordinate in display space.
	@param	band	The image band to use as a location and scaling
		reference.
	@return	A QPointF coordinate in image space corresponding the display
		space coordinate. <b>N.B.</b>: The returned coordinate may be
		located outside the {@link image_size() image bounds} if the
		specified display coordinate falls on an area of the display
		viewport that is not within the {@link displayed_image_region(int)
		displayed image region} for the band.
*/
QPointF map_display_to_image (const QPoint& coordinate, int band = 0) const;

/**	Map an image display viewport rectangle to its source image region.

	This is a convenience overload method that maps the defining corner
	coordinates of the specified image display region to their source
	image coordinates.

	@param	display_region	A QRect in image display viewport coordinates.
	@param	band	The image band for which the image position is
		requested.
	@return	A QRectF in source image space cooresponding to the image
		display viewport coordinate. <b>N.B.</b>: The returned image
		coordinate may be located outside the {@link image_size() image
		bounds} if the specified display coordinate falls on an area of
		the display viewport that is not within the {@link
		displayed_image_region(int) displayed image region} for the band.
	@see	map_display_to_image(const QPoint&, int) const
*/
inline QRectF map_display_to_image
	(const QRect& display_region, int band = 0) const
	{return QRectF
		(map_display_to_image (display_region.topLeft (), band),
		 map_display_to_image (display_region.bottomRight ()
		 	+= QPoint (1, 1) /* QRect BR adjust */, band));}

/**	Map an image coordinate to its display viewport coordinate.

	@param	coordinate	A QPointF coordinate in image space.
	@param	band	The image band to use as a location and scaling
		reference.
	@return	A QPoint coordinate relative to the display viewport
		corresponding the image coordinate. <b>N.B.</b>: The coordinate
		may lie outside the bounds of the display viewport.
*/
QPoint map_image_to_display (const QPointF& coordinate, int band = 0) const;

/**	Map an image coordinate to its display viewport coordinate.

	This is a convenience overload method that maps the defining corner
	coordinates of the specified image region to their image display
	coordinates.

	@param	image_region	A QRectF in source image coordinates.
	@param	band	The image band to use as a location and scaling
		reference.
	@return	A QRect relative to the image display viewport corresponding
		the source image region. <b>N.B.</b>: The rectangle may lie
		outside the bounds of the display viewport.
	@see	map_image_to_display(const QPointF&, int) const
*/
inline QRect map_image_to_display
	(const QRectF& image_region, int band = 0) const
	{return QRect
		(map_image_to_display (image_region.topLeft (),     band),
		 map_image_to_display (image_region.bottomRight (), band)
		 	-= QPoint (1, 1) /* QRect BR adjust */);}

/**	Get the region of the image, in image space, that is contained within
	the display viewport.

	Only that portion of the image that is being displayed is returned.
	The display viewport may extend beyond the boundaries of the
	displayed image.

	@param	band	The image band for which to obtain the region of the
		image appearing in the display.
	@return	A QRectF describing the region of the image that appears in
		the display viewport. <b>N.B.</b>: The image display region is
		provided as floating point values for accuracy in other contexts
		(e.g. scaled image dimensions). The dimensions of the region
		rectangle should be truncated if integer values are needed,  not
		rounded to the nearest integer.
	@see	displayed_image_region_size(int)
	@see	image_display_region(int)
*/
QRectF displayed_image_region (int band = 0) const;

/**	Get the size the image, in image space, that is contained within
	the display viewport.

	<b>N.B.</b>: Partial source image pixels at the edges of the image
	display region, that may result from image scaling greater than 1.0,
	are excluded; i.e. the size returned only includes whole source image
	pixels.

	@param	band	The image band for which to obtain the region of the
		image appearing in the display.
	@return	A QSize that describes the size of the image region that
		appears in the display viewport, excluding partial pixels at
		the edges of the region.
	@see	displayed_image_region(int) const
*/
QSize displayed_image_region_size (int band = 0) const;

/**	Get the region of the display viewport, in display space, that contains
	the displayed image region.

	The origin of the display space (0,0) is at the top-left corner of
	the display viewport and the coordinate values increase to the right
	and down.

	The returned rectangle will always have its top-left coordinate at
	the display origin. The returned rectangle will be different from the
	display viewport rectangle only when the right and/or bottom edge of
	the image appears in the display.

	@param	band	The image band for which the display region is desired.
	@return	A QRect specifying the region of the display viewport, in
		display space, that contains a region of the selected image band.
	@see	displayed_image_region(int) const
*/
QRect image_display_region (int band = 0) const;

inline QSizeF image_scaling (int band = 0) const
	{return Reference_Image->source_scaling ((band < 0) ? 0 : band);}
inline void image_scaling
	(double* horizontal_scaling, double* vertical_scaling, int band) const
	{return Reference_Image->source_scaling
		(horizontal_scaling, vertical_scaling, (band < 0) ? 0 : band);}

static void min_scale (double scale_factor);
inline static double min_scale ()
	{return Min_Scale;}

static void max_scale (double scale_factor);
inline static double max_scale ()
	{return Max_Scale;}

inline int image_bands () const
	{return Reference_Image->source_bands ();}

inline unsigned int* band_map () const
	{return Reference_Image->source_band_map ();}

inline int image_data_precision () const
	{return Reference_Image->source_precision_bits ();}

/**	Get the image pixel datum at an image band coordinate.

	The pixel datum is for the source image value before data mapping
	has been applied.

	@param	x	The horizontal offset (to the right) from the image
		origin (left edge, zero) of the pixel datum.
	@param	y	The vertical offset (down) from the image origin (top
		edge, zero) of the pixel datum.
	@param	band	The image band of the pixel datum, where zero is
		the reference band.
	@return	The {@link #Plastic_Image::Pixel_Datum} value at the
		specified image location. This will be the
		{@link #Plastic_Image::UNDEFINED_PIXEL_VALUE} if the location
		is not within an active image tile or the image does not
		contain the specified band.
*/
Plastic_Image::Pixel_Datum image_pixel_datum
	(unsigned int x, unsigned int y, unsigned int band) const;

/**	Get the image pixel value at an image coordinate.

	@param	coordinate	An image coordinate.
	@return	A {@link #Plastic_Image::Triplet} containing the three
		{@link image_pixel_datum(unsigned int, unsigined int, unsigned
		int) image pixel datum} values at the image coordinate.
		<b>N.B.</b>: Any of the pixel datum values may be the {@link
		#Plastic_Image::UNDEFINED_PIXEL_VALUE} if the mapping of the
		image coordinate for the corresponding band does not fall within
		an active image tile or the image does not contain the
		corresponding band.
*/
Plastic_Image::Triplet image_pixel (const QPoint& coordinate) const;

/**	Get the display pixel value at a viewport display coordinate.

	The pixel value has been mapped from the image source pixel value
	to the display value.

	@param	coordinate	A QPoint display viewport coordinate.
	@return	A QRgb (32-bit integer) pixel value in which the bytes,
		from most to least significant, correspond to the alpha, red,
		green and blue pixel datum values. <b>N.B></b>: If the coordinate
		does not map to an active image tile a value of zero will be
		returned, but this can only be a hint that the value may be
		invalid because the default alpha value is 0xFF.
*/
QRgb display_value (const QPoint& coordinate) const;

/**	Get the display pixel value at a viewport display coordinate.

	@param	coordinate	A QPoint display viewport coordinate.
	@return	A {@link #Plastic_Image::Triplet} containing the three red,
		green blue {@link display_value(const QPoint&) display value}
		datum components at the display coordinate.
*/
Plastic_Image::Triplet display_pixel (const QPoint& coordinate) const;

/**	Preferred size of the display.

	The preferred size is the {@link scaled_image_size(int) scaled image
	size}.

	@return	A QSize containing the preferred size of the display.
*/
virtual QSize sizeHint () const;

/**	Get the Data_Maps to be used for mapping source image data to
	display image data.

	Three Data_Maps are provided: one for mapping to the red (0) display
	data image band, one for the green (1) band and one for the blue (2)
	band. Each Data_Map is QVector LUT with one entry for each possible
	source image data value, in data value order. Each entry is an 8-bit
	value that provides the display data value for the source image data
	value corresponding to the entry index.

	The Data_Maps that are provided are contained in the reference image
	from which all tile images are cloned and that share the same
	Data_Maps with the reference image.

	@return	A pointer to an array of three Data_Map vector pointers.
*/
inline Data_Map** data_maps () const
	{return Reference_Image->source_data_maps ();}

/**	Get the pending state change of the image display.

	When a pending state change has been completed the {@link
	state_change(int) state change} signal will be emitted.

	@return A state change code with bit flags indicating the pending
		state change.
*/
inline int pending_state_change () const
	{return Pending_State_Change;}

//	Histograms:

/**	Produce image data histograms.

	An image data histogram is a count, in pixel datum value order, of
	the number of occurances of pixels with the corresponding value -
	i.e. a frequency distribution - for a specified region of the  source
	image data band. A histogram is produced for each of the source bands
	mapped to the three display bands. <b>N.B.</b>: Original source image
	data values - before any data mapping - are counted.

	@param	histograms	A QVector of {@link #Histogram} pointers. Three
		Histograms will be produced. If any of the three pointers are
		NULL a new Histogram will be created (its ownership is
		transferred to the caller), or assigned another non-NULL
		Histogram pointer for same source data band that is being {@link
		map_bands(const unsigned int*) mapped}. If a Histogram is not
		large enough for the {@link image_data_precision() source image
		data precision} it is enlarged. Each Histogram is cleared (filled
		with zero count values) before data scanning begins.
	@param	image_region	A QRect that specifies the region of the
		source image for which to produce histograms. An empty region
		produces empty histograms.
*/
bool source_data_histograms (QVector<Histogram*> histograms,
	const QRect& image_region) const;
bool display_data_histograms (QVector<Histogram*> histograms,
	const QRect& display_region) const;

/*------------------------------------------------------------------------------
	Display tiling
*/
/**	Set the default size, in display space, of a rendering tile.

	<b>N.B.</b>: The effective tile size will depend on the image being
	displayed: For a QImage based source image the entire image size is
	the effective tile size. Otherwise the effective size is the lesser
	of the image size or the default size. 

	@param	size	The default display size of an image rendering tile.
		When no size is specified, or an invalid size (dimensions less
		than zero) is specified, the built-in default size specified by
		the DEFAULT_IMAGE_TILE_WIDTH and DEFAULT_IMAGE_TILE_HEIGHT
		compile-time macros will be used. <b>N.B.</b>: A positive size
		dimension less than the {@link minimum_tile_dimension() minimum
		tile dimension} will be set to the minimum.
	@see	default_tile_display_size()
	@see	tile_display_size(const QSize&)
*/
static void default_tile_display_size (const QSize& size = QSize ());

/**	Get the default size of an image rendering tile in display space.

	@return	A QSize specifying the default display size of a tile.
		<b>N.B.</b>: Tile dimensions less than the {@link
		minimum_tile_dimension() minimum tile dimension} will be
		set to the minimum.
	@see	default_tile_display_size(const QSize&)
*/
inline static QSize default_tile_display_size ()
	{return Default_Tile_Display_Size;}

/**	Set the size, in display space, of an image rendering tile.

	@param	size	The display size of an image rendering tile. If the
		size is empty (either dimension less than or equal to zero), the
		{@link default_tile_display_size() default tile size} will be
		used. <b>N.B.</b>: Tile dimensions less than the
		{@link #MINIMUM_TILE_DIMENSION} will be increased to the minimum.
	@see	default_tile_display_size(const QSize&)
	@see	tile_display_size()
*/
void tile_display_size (const QSize& size);

/**	Get the size of an image rendering tile in display space.

	@return	A QSize specifying the display size of a tile.
	@see	tile_display_size(const QSize&)
*/
inline QSize tile_display_size () const
	{return Tile_Display_Size;}

inline static int minimum_tile_dimension ()
	{return MINIMUM_TILE_DIMENSION;}


protected:

/**	Get the origin of the displayed tile grid in image space.

	The origin of the displayed tile grid is held by the source image.
	The displayed tile grid origin is the origin of tile 1,1 in the
	effective tile grid.

	<b>N.B.</b>: The display viewport may be offset from the origin of
	the displayed tile grid origin, but the origin of the display
	viewport is expected to always be located within the bounds of the
	displayed tile grid origin tile. The origin tile is expected to
	always be at tile grid coordinate 1,1 in the effective tile grid.

	@param	band	The image band for which the tile grid origin is
		requested.
	@return	A QPointF providing the origin of the displayed tile grid
		in image pixel units. <b>N.B.</b>: Sub-pixel accuracy is used
		to specify the displayed tile grid origin to ensure accurate
		placement of tiles relative to the grid origin.
	@see	displayed_tile_grid_origin(const QPointF&, int)
*/
inline QPointF displayed_tile_grid_origin (int band = 0) const
	{return Reference_Image->source_origin ((band < 0) ? 0 : band);}

/**	Set the origin of the displayed tile grid in image space.

	<b>N.B.</b>: The tile grid origin is set as the {@link
	Plastic_Image::source_origin(const QPointF&, int) source origin}
	of the Reference_Image.

	@param	A QPointF specifying the origin of the displayed tile grid
		in image pixel units. <b>N.B.</b>: Sub-pixel accuracy is used
		to specify the displayed tile grid origin to ensure accurate
		placement of tiles relative to the grid origin.
	@param	band	The image band to which the tile grid origin
		applies. If negative the origin of all image bands are set.
	@see	displayed_tile_grid_origin(int)
*/
void displayed_tile_grid_origin (const QPointF& origin, int band = -1)
	{Reference_Image->source_origin (origin, band);}

/**	Get the size of a tile in scaled image space.

	Each tile is an area with a fixed {@link tile_display_size() tile
	size} in the display space; i.e. the display size of each tile image
	is the same size. The image area rendered by each tile depends on the
	scaling, both horizontal and vertical, which may vary for each band.

	The display tile size is scaled by the scaling factors for the band.
	A scaling factor less than one increases the tile image size relative
	to a normal 1:1 scaling factor. The dimensions include any partial
	pixel at the boundaries which are used for accurate image tiling.

	@param	band	The image band to use as a scaling reference.
	@return	A QSizeF size in image space.
*/
QSizeF tile_image_size (int band = 0) const;

/**	Get the origin of a tile in display viewport coordinates.

	@param	coordinate	A QPoint tile grid coordinate. Coordinate (0,0)
		is for the upper-left tile in the grid. Coordinate values increase
		to the right (+x) and downward (+y).
	@return	A QPoint specifying the display viewport coordinate of the
		tile origin. <b>N.B.</b>: The tile origin may lie outside, the
		boundaries of the display viewport.
	@see	tile_display_region(const QPoint&)
*/
QPoint tile_display_origin (const QPoint& coordinate) const;

/**	Get the region of a tile in display viewport coordinates.

	The tile region is display viewport coordinates is the region located
	at the {@link tile_display_origin(const QPoint&) tile display origin}
	having the {@link tile_display_size() tile display size}.

	@param	coordinate	A QPoint tile grid coordinate. Coordinate (0,0)
		is for the upper-left tile in the grid. Coordinate values increase
		to the right (+x) and downward (+y).
	@return	A QRect specifying the display region of the tile. <b>N.B.</b>:
		The tile display region may extend beyond, or be totally outside,
		the boundaries of the display viewport.
	@see	tile_display_origin(const QPoint&)
*/
QRect tile_display_region (const QPoint& coordinate) const;

/**	Get the effective tile grid region in image space.

	<b>N.B.</b>: The effective tile grid may extend beyond the bounaries
	of the image.

	if the band number is zero the cached Tiled_Image_Region is returned.
	Otherwise {@link calculate_tiled_image_region(int)} is called.

	Determining tiled region depends on the {@link
	displayed_tile_grid_origin(int)}, the {@link tile_image_size(int)}
	and the {@link tile_grid_size()}.

	@param	band	The image band for which the tiled region is
		requested.
	@return	A QRectF describing the image region of the selected image
		band contained within the effective tile grid. Sub-pixel accuracy
		is used for the region values.
*/
QRectF tiled_image_region (int band = 0) const;

/**	Map a tile grid coordinate to the image coordinate of the tile origin.

	<b>N.B.</b>: The mapped image coordinate may be outside the bounds
	of the image.

	@param	coordinate	A QPoint coordinate of a tile in the effective tile
		grid.
	@param	band	The band of the image coordinate to be mapped.
	@return	A QPointF coordinate in image space corresponding to the
		origin (upper-left corner) of the specified tile.
	@see	map_image_to_tile (const QPointF&, int)
*/
QPointF map_tile_to_image (const QPoint& coordinate, int band = 0) const;

/**	Map an image coordinate to a tile grid coordinate.

	A tile coordinate references an effective tile in the tile grid, not
	a location within the tile. An image coordinate that falls outside of
	the effective tile grid will have negative tile coordinates.
	<b>N.B></b>: A tile coordinate in the effective tile grid may
	reference a tile or tile row that has not been instantiated.

	@param	coordinate	A QPointF coordinate in image space.
	@param	band		The band of the image coordinate being mapped.
	@return	A QPoint coordinate of a tile in the effective tile grid. If
		the coordinate values are negative the image coordinates
		do not fall on an effective tile.
	@see	map_tile_to_image (const QPointF&, int)
*/
QPoint map_image_to_tile (const QPointF& coordinate, int band = 0) const;

/**	Map an image coordinate to a tile display coordinate.

	The image coordinate system is relative to the full resolution image
	with its origin (0,0) at the upper left pixel in the image with
	coordinates increasing to the right (+x) and downward (+y) in pixel
	units.

	The tile grid coordinate system has its origin (0,0) at the upper
	left tile in the grid with coordinates increasing to the right (+x)
	and downward (+y) in whole tile units.

	@param	coordinate	A QPointF coordinate in image space.
	@param	band	The image band to which the image coordinate applies.
	@return	A QPoint specifying the tile grid coordinate of the tile that
		contains the image coordinate. This will be (-1,-1) if the image
		coordinate is outside the boundaries of the {@link
		tiled_image_region(int) tiled image region}.
*/
QPoint map_image_to_tile_offset (const QPointF& coordinate, int band = 0) const;

/**	Map a viewport display coordinate to a tile grid coordinate.

	The viewport display coordinate system has its origin (0,0) at the
	upper left pixel in the display with coordinates increasing to the
	right (+x) and downward (+y) in pixel units.

	The tile grid coordinate system has its origin (0,0) at the upper
	left tile in the grid with coordinates increasing to the right (+x)
	and downward (+y) in whole tile units.

	@param	coordinate	A QPoint coordinate in the display viewport.
	@return	A QPoint of a tile coordinate in the effective tile grid.
		This will be (-1,-1) if the display coordinate is outside the
		boundaries of the effective tile grid.
	@see	map_image_to_tile(const QPointF&, int)
*/
QPoint map_display_to_tile (const QPoint& coordinate) const;

/**	Map a viewport display coordinate to a tile display coordinate.

	@param	coordinate	A QPoint coordinate in the display viewport.
	@return	A QPoint providing the coordinate in tile display units,
		relative to the upper-left (0,0) datum, of the image coordinate.
*/
QPoint map_display_to_tile_offset (const QPoint& coordinate) const;

/**	Get the size of the effective tile grid in tile grid units.

	@return A QSize providing the size of the effiective tile grid in
		terms of the number of tiles wide and high.
*/
inline QSize tile_grid_size () const
	{return Tile_Grid_Size;}
inline int tile_grid_width () const
	{return Tile_Grid_Size.width ();}
inline int tile_grid_height () const
	{return Tile_Grid_Size.height ();}

/**	Resize the tile grid to the size of the display viewport.

	The size of the effective tile grid is recalculated as the dimensions of
	the display viewport plus the {@link tile_display_size() dimensions of
	a tile} (to allow for the display viewport origin to occur anywhere in
	the viewport origin tile) divided by the dimensions of tile.
	If the calculated size of the effective tile grid is the same s the
	current {@link tile_grid_size() tile grid size} then nothing is done.

	When the size of the tile grid changes NULL tile rows and/or NULL
	NULL tiles are added as need when a tile grid dimension has
	increased. When a tile grid dimension has decreased instantiated
	tiles in the extra columns are deleted and removed from the effective
	tile grid, and extra tile rows are removed after being emptied of
	instantiated tiles.

	The current {@link tile_grid_size() tile grid size} and {@link
	tiled_image_region(int) effective tiled image region} are updated.

	@return true if the size of the effective tile grid changed; false
		otherwise.
*/
bool resize_tile_grid ();

/**	Reset the location and scaling of the tiles.

	If the tile grid is empty nothing is done.

	If the image loading is not in progress {@link
	Image_Renderer::stop_rendering() image rendering is stopped}.

	Each tile in the effective tile grid is examined:

	A tile row that is inactive (NULL) but its expected location is found
	to overlap the image area (of the reference band) is activated by
	providing a tile row populated with inactive (NULL) tiles. Active
	tile rows that no longer overlap the image area are deactivated by
	deleting each active tile in the row and then deleting the row and
	replacing it in the grid with an inactive row.

	A tile that is inactive but it's expected location is found to
	overlap the image area is activated by setting the tile to a {@link
	Plastic_Image::clone(const QSize&, Plastic_Image::Mapping_Type)
	clone} of the source image. The tile clone has the {@link
	tile_display_size() tile size} and shares the {@link
	Plastic_Image::source_band_map() band map} and {@link
	Plastic_Image::source_data_maps() data maps} with the source image.
	Active tiles that no longer overlap the image area are deactivated by
	deleting them and replacing them in the grid with an inactive tile.

	Each tile that remains active has its image location and scaling set
	to the tile's expected location relative to the current {@link
	displayed_tile_grid_origin(int) displayed tile grid origin} and
	{@link image_scaling(int) image scaling} for each band. If this
	results in the image {@link Plastic_Image::needs_update() needing to
	be updated}, and image loading is not in progress, rendering of the
	image is {@link Image_Renderer::cancel(Plastic_Image*, int)
	canceled}, without waiting, and then {@link
	Image_Renderer::queue(Plastic_Image*, const QPoint&, const QRect&,
	bool) queue} the tile for rendering with its rendering priority and
	display viewport region determined by the tile's location in the tile
	grid.

	If the image loading is not in progress {@link
	Image_Renderer::start_rendering() image rendering is started}.

	@return	Zero (false) if there was no change to the location or
		scaling of any active tile; +1 if any tile image visible in
		the display viewport was changed and queued for rendering;
		-1 if only background (margin) tiles were changed.
*/
int reset_tiles ();

/**	Clear the effective tile grid of all tiles.

	Every active (non-NULL) tile in the effective tile grid is deleted,
	removed from its tile row and the tile row deleted and removed from
	the grid. The resulting effective tile grid is empty.
*/
void clear_tiles ();

/*==============================================================================
	Utilities
*/
public:

/**	Get the next image tile rendering status.

	Image tiles for the display grid are queued for rendering in high-to-low
	priority order. A high priority image tile is visible in the display
	viewport; a low priority tile is in the display grid margin outside the
	viewport or is for the background fill image.

	@return	The returned status is the tile being actively rendered
		or the first tile in the queue. This will be either {@link
		#RENDERING_VISIBLE_TILES} or {@link #RENDERING_BACKGROUND}. If no
		tile is being rendered or is queued {@link #NOT_RENDERING}
		is returned.
	@see	rendering_status(int)
*/
int rendering_status () const;

/**	Get the scaling factor that will fit a source size within a
	destination size.

	If the source size is not empty and either of its dimensions is
	greater than the destination size, the minimum of the destination to
	source dimension ratios is the returned scale factor. Otherwise the
	returned scale factor will be 1.0. If the destination size is empty the
	returned scale factor will be 1.0.

	@param	source_size	A QSize specifying the source size.
	@param	destination_size	A QSize specifying the destination size.
	@return	A double scale factor less than or equal to 1.0 and greater
		than 0.0.
*/
static double scale_to_size
	(const QSize& source_size, const QSize& destination_size);

/**	Provide a brief description of a {@link rendering_status(int)
	rendering status} value;

	@param	status	A rendering status value.
	@return	A QString that very briefly describes the status value.
*/
static QString rendering_status_description (int status);

/**	Provide a brief description of a {@link state_change(int) state
	change} code.

	@param	state	A state change code.
	@return	A QString that very briefly describes the state change
		indicated by the bit flags of the code value.
*/
static QString state_change_description (int state);

inline static QErrorMessage* error_message ()
	{return Error_Message;}

//	Ownership of the QErrorMesage is NOT transferred.
inline static void error_message (QErrorMessage* dialog)
	{Error_Message = dialog;}

//	DEBUG only methods.
void print_tile_grid () const;
void print_tile_pool () const;

/*==============================================================================
	Qt signals
*/
signals:

/**	Signals the result of an image load request.

	This signal is emitted from the {@link loaded(bool) image loaded} slot
	<b>after</b> the image loading procedure has been completed and the
	Renderer has been restarted.

	@param	successful	true if the image load completed successfully;
		false otherwise.
	@see	image(const QString&, const QSizeF&)
	@see	image(const QString&, const QSize&)
	@see	image(const Shared_Image&, const QSizeF&)
	@see	image(const Shared_Image&, const QSize&)
	@see	Image_Renderer::image_loaded(bool)
*/
void image_loaded (bool successful);

/**	Signals a change to the {@link image_metadata() image metadata}.

	@param	metadata	An idaeim::PVL::Aggregate pointer to the changed
		metadata. This should be the same as the idaeim::PVL::Aggregate
		pointer returned from the most recent image metadata
*/
void image_metadata_changed (idaeim::PVL::Aggregate* metadata);

/**	Signals the position of the image cursor.

	This signal is emitted from the {@link mouseMoveEvent(QMouseEvent*)
	mouse move event handler} to report the current image cursor
	position in image coordinates.

	@param	display_position	A QPoint specifying the cursor position
		in display image coordinates, which is the same as the position
		from the mouse move event.
	@param	image_position	A QPoint specifying the cursor position
		in source image coordinates. This will be -1,-1 if the cursor
		is not within the {@link displayed_image_region(int) displayed
		image region} of the reference band.
*/
void image_cursor_moved
	(const QPoint& display_position, const QPoint& image_position);

/**	Signals a change to the image location in the display viewport.

	This signal is emitted as a result of an {@link move_image(const
	QPoint&, int) image move} request after adjusting the image
	tiles to the origin, but only if the image position moved in the
	display viewport. The signal will be emitted <b>after</b> any {@link
	reset_tiles() tile resets} that may have been needed.

	This signal is emitted from the {@link scale_image(const QSizeF&,
	const QPoint&, int) image scaling} procedure immediately <b>after</b>
	the {@link image_scaled(const QSizeF&, int) image scaled signal} was
	emitted, but only if the image position at the display viewport origin
	changed.

	@param	origin	A QPoint specifying the position of the image, in
		source image coordinates, at the display viewport origin.
	@param	band	The image band that moved. This will be -1 if all
		image bands moved.
*/
void image_moved (const QPoint& origin, int band);

/**	Signals that the size of the displayed image region changed.

	This signal is emitted from the {@link loaded(bool) image loaded}
	slot, if the image was successfully loaded, <b>after</b> the image
	loading procedure has been completed and {@link
	Image_Renderer::start_rendering() rendering has been started}. This
	signal is used to report the initial image region appearing in the
	display viewport.

	This signal is emitted as a result of an {@link move_image(const
	QPoint&, int) image move} request after adjusting the image
	tiles to the origin}, but only if the {@link
	reset_displayed_image_region() reset of the displayed image region}
	found that the image region size if the display viewport changed. The
	signal will be emitted <b>after</b> any {@link reset_tiles() tile
	resets} that may have been needed.

	This signal is emitted from the {@link scale_image(const QSizeF&,
	const QPoint&, int) image scaling} procedure, but only if the image
	scaling changed and the {@link reset_displayed_image_region() reset
	of the displayed image region} found that the image region size if
	the display viewport changed. The signal will be emitted <b>after</b>
	the tile images have been {@link reset_tiles() reset}.

	This signal is emitted from the display {@link
	resizeEvent(QResizeEvent*) resize event}, but only if the {@link
	reset_displayed_image_region() reset of the displayed image region}
	found that the image region size if the display viewport changed. The
	signal will be emitted <b>after</b> the tile images have been {@link
	reset_tiles() reset}.

	@param	region_size	A QSize specifying the size of the {@link
		displayed_image_region() displayed image region} truncated to
		remove partial pixels.
*/
void displayed_image_region_resized (const QSize& region_size);

/**	Signals that the size of the image display viewport changed.

	This signal is emitted from the display {@link
	resizeEvent(QResizeEvent*) resize event} <b>after</b> the
	tile images have been {@link reset_tiles() reset}.

	@param	viewport_size	A QSize specifying the new display viewport
		size.
*/
void display_viewport_resized (const QSize& viewport_size);

/**	Signals that image scaling has changed.

	This signal is emitted from the {@link scale_image(const QSizeF&,
	const QPoint&, int) image scaling} procedure <b>after</b> the new
	scaling has been applied and the tile images have been {@link
	reset_tiles() reset}.

	This signal is emitted from the {@link loaded(bool) image loaded}
	slot, if the image was successfully loaded, <b>after</b> the image
	loading procedure has been completed and {@link
	Image_Renderer::start_rendering() rendering has been started}. This
	signal is used to report the initial scaling factor applied to the
	loaded image.

	@param	scaling	A QSizeF with the horizontal and vertical scaling
		factors that have been applied to the displayed image.
	@param	band	The image band that was scaled. This will be -1
		if all image bands were scaled.
*/
void image_scaled (const QSizeF& scaling, int band);

/**	Signals the status of tile image rendering.

	This signal is emitted from the {@link renderer_status(int) renderer
	status slot} that filters the status signals from the Renderer.

	@param	status	A rendering status value.
	@see	renderer_status(int)
	@see	rendering_status_description(int)
*/
void rendering_status (int status);

/**	Conveys a tile image rendering progress status notice message.

	This signal is propagated from the {@link
	Image_Renderer::status_notice(const QString&) image renderer
	status notice signal}.

	@param	message	A QString forwarded from the rendering progress status
		notice message.
*/
void rendering_status_notice (const QString& message);

/**	Signals a state change of the image display.

	A state change is signaled whenever the state of the displayed image
	is changing. A state change signal includes a state change code value
	that indicates what is changing.

	The following state change flags indicate the <i>type</i> of display
	change:
<dl>
<dt>{@link #NO_STATE_CHANGE}
<dd>No change to the display state. This state is not signaled.

<dt>{@link #IMAGE_LOAD_STATE}
<dd>A request for a new {@link image(const QString&, const QSize&)
	image} to be loaded has been started. This state change will only be
	signaled if the request for a new image was successfully registered
	with the Renderer. This state change is always signaled
	synchronously.

	An {@link loaded(bool) image load has completed}. If the image load
	completed successfully the {@link #RENDERING_COMPLETED_STATE} flag
	is also set. If the image load failed the {@link
	#RENDERING_CANCELED_STATE} is set instead. This state change is
	always signaled asynchronously, and is always associated with an
	{@link image_loaded(bool) image loaded} signal that occurs
	<b>before</b> the state change signal.

<dt>{@link #DISPLAY_SIZE_STATE}
<dd>A {@link resizeEvent(QResizeEvent*) display resize event} has
	occurred. If the display resize was completed without needing to
	{@link reset_tiles() reset any tiles} completion is signaled
	synchronously; otherwise it is signaled asynchronously.

<dt>{@link #TILE_SIZE_STATE}
<dd>A {@link tile_display_size(const QSize&) tile size} change was
	requested. The completion of the change is signalled asynchnronously
	after the new tiles have been rendered.

<dt>{@link IMAGE_MOVE_STATE}
<dd>The {@link displayed_image_origin(int) origin} of the image in the
	display viewport} has changed. This may be the result of {@link
	move_image(const QPoint&, int) moving} the image. <b>N.B.</b>: No
	change is signaled if the image was not moved. If the image moved
	but no tile images needed to be rendered the change is signaled
	synchronously; otherwise it is signaled asynchronously.

	This change may also be due to {@link scale_image(const QSizeF&,
	const QPoint&, int) image scaling}. <b>N.B.</b>: This change is
	only  signaled if image scaling is successful. In this case the
	change is always signaled asynchronously.

	Also, whenever resetting tiles results in any change to a tile
	origin for any reason this change will be included in the
	asychnronous state change signal.

<dt>{@link #IMAGE_SCALE_STATE}
<dd>An {@link scale_image(const QSizeF&, const QPoint&, int) image
	scaling} request has completed. This change is always signaled
	asynchronously.

<dt>{@link #BAND_MAPPING_STATE}
<dd>A {@link map_bands(const unsigned int*) band mapping} request has
	resulted in a change to the image display. This change is always
	signaled asynchronously. <b.N.B.</b>: This change will only be
	signaled if band mapping was successful.

<dt>{@link #DATA_MAPPING_STATE}
<dd>A {@link map_data(Data_Map**) data mapping} request has resulted in
	a change to the image display. This change is always signaled
	asynchronously. <b.N.B.</b>: This change will only be signaled if
	data mapping was successful.
</dl>

	The following state change flags are <i>qualifiers</i> for the type
	of display change:
<dl>
<dt>{@link #RENDERING_VISIBLE_TILES_STATE}
<dd>This flag indicates a change involving the rendering of tile images
	that are visible in the display viewport. These are the tiles in the
	display grid that have high priority {@link rendering_status()
	rendering status} and are rendered before margin tiles or the
	background image.

	This flag will be set when a {@link state_change_start(int) state
	change is starting} with at least one tile visible in the display
	viewport in the rendering queue or activlely being rendered.
	<b>N.B.</b>: The signal is emitted synchronously <b>before</b>
	rendering is started. In this case neither the
	RENDERING_CANCELED_STATE nor RENDERING_COMPLETED_STATE flags will be
	set.

	This flag will be set when all visible tiles are finished being
	{@link rendered(const QPoint&, const QRect&) rendered} and more
	background tiles may remain to be rendered. In this case the {@link
	#RENDERING_COMPLETED_STATE} flag will also be set. <b>N.B.</b>: The
	signal is emitted asynchronously <b>after</b> a display update has
	been queued.

<dt>{@link #RENDERING_BACKGROUND_STATE}
<dd>This flag indicates a change involving the rendering of tile images
	that are not visible in the display viewport, or the background
	image tile; i.e. margin tiles. These are the tiles that have low
	priority (@link rendering_status() rendering status} and are
	rendered after visible display tiles.

	This flag will be set when a {@link state_change_start(int) state
	change is starting} with at least one tile to be rendered but no
	high priority tiles. <b>N.B.</b>: The signal is emitted
	synchronously <b>before</b> rendering is started. In this case
	neither the RENDERING_CANCELED_STATE nor RENDERING_COMPLETED_STATE
	flags will be set.

<dt>{@link #RENDERING_CANCELED_STATE}
<dd>This flag indicates that image rendering has been {@link
	cancel_rendering() canceled}. <b>N.B.</b>: The state is signalled
	synchronously <b>after</b> any rendering in progress has been
	canceled, but some rendering may still not yet have finished. Other
	state change flags will indicate what display changes were canceled;
	no signal is emitted if no display change was pending when the
	cancellation was requested.

	This flag will be set if the Renderer {@link renderer_status(int)
	signals} a {@link #RENDERING_CANCELED} status due to some rendering
	failure.

<dt>{@link #RENDERING_COMPLETED_STATE}
<dd>This flag indicates that rendering associated with the change
	specified by display type change state flags has completed. This
	state is signalled asynchronously on completion of an image load,
	when visible tile image rendering has completed, and when all
	rendering has been done. In this case no other qualifier flag will
	be set.

	This flag will be set along with RENDERING_VISIBLE_TILES_STATE and
	RENDERING_BACKGROUND_STATE (i.e. {@link
	#RENDERING_VISIBLE_TILES_COMPLETED_STATE) when rendering of visible
	tiles has completed for the change specified by display type change
	state flags. There may or may not be additional rendering of
	background/margin tile images yet to be completed, but in either
	case a final state change will be signalled without the
	RENDERING_VISIBLE_TILES_STATE but with either
	RENDERING_COMPLETED_STATE or RENDERING_CANCELED_STATE.

<dt>{@link #COMPLETED_WITHOUT_RENDERING_STATE}
<dd>This flag indicates that a state change was completed without any
	tile image rendering being required. This state is signalled
	synchronously when the image region is {@link move_image(const
	QPoint&, int) moved}, or a {@link {@link resizeEvent(QResizeEvent*)
	display resize event} has been accomplished, using already rendered
	tile images.
</dl>

	Some changes to the Tiled_Image_Display are synchronous responses to
	requests for changes. However, when a change request results in
	queuing image tiles for rendering  the {@link pending_state_change()
	Pending_State_Change} is set with the bit flag(s) for the state
	change type that is in progress, and a state change signal is
	signalled asynchronously when Tiled_Image_Display's Image_Renderer
	(which is expected to operate asynchronously on a separate thread)
	signals that it has completed rendering.

	A state change is signalled at the <b>start</b> of each type of
	change. In this case neither the RENDERING_COMPLETED_STATE nor the
	RENDERING_CANCELED_STATE qualifiers will be set. The
	RENDERING_VISIBLE_TILES_STATE or the RENDERING_BACKGROUND_STATE, but
	not both, may be set if any tile image rendering was queued as a
	result of the requested image display change; neither of these flags
	will be set if no rendering was required to accomplish the display
	change. When a state change start is signalled a {@link
	pending_state_change() pending state change} is always in effect. If
	a pending state change was in effect when a new type of change was
	requested the pending state change value is updated with the new type
	bit set before the start of the change is signalled. However, if the
	requested display change was for a type of change that was already
	pending but not yet completed, then a new state change start is not
	signalled; i.e. a state change start is only signalled once for each
	type of change that is requested until the pending change(s) have
	been completed.

	A state change is signalled at the <b>transition</b> from rendering
	of tile images visible in the display viewport to possibly rendering
	background tiles. In this case the RENDERING_COMPLETED_STATE
	qualifier will be set and both the RENDERING_VISIBLE_TILES_STATE and
	RENDERING_BACKGROUND_STATE flags are also set (this is the only case
	in which both of these flags will be set). <b>N.B.</b>: The
	RENDERING_BACKGROUND_STATE flag is always set even if no backgroud
	tile rendering is queued; check the {@link rendering_state()
	rendering state} to determine if any more rendering is queued. The
	display change type flags will have the value of the pending state
	change.

	A state change is signalled at the <b>completion</b> all rendering.
	In this case the RENDERING_COMPLETED_STATE qualifier will be set but
	neither the RENDERING_VISIBLE_TILES_STATE nor
	RENDERING_BACKGROUND_STATE flags will be set. The display change type
	flags will have the value of the pending state change(s) that
	completed; the pending state change value will be reset to
	NO_STATE_CHANGE immediately after the state change completion is
	signalled. At this point rendering has stopped and nothing is queued.
	A completion state change will be signalled even if no rendering was
	required to complete the change, in which case the
	COMPLETED_WITHOUT_RENDERING_STATE qualifier will be set instead of
	the RENDERING_COMPLETED_STATE. A completion state change is also
	signalled if all rendering was cancelled, in which case the
	RENDERING_CANCELED_STATE qualifier will be set instead of
	RENDERING_COMPLETED_STATE.

	@param	state	A state change bit flags code value.
*/
void state_change (int state);

/*==============================================================================
	Qt slots
*/
public slots:

/**	Move the position of the image in the display viewport.

	The display origin is the upper-left corner of the viewport. The
	image is moved in the display such that the image origin coordinate
	is located at the display origin. However, the specified origin is
	clipped to be within the bounds of the source image and the {@link
	calculate_lower_right_origin_limit() lower right origin limit}.

	If the origin is the same as the current image origin for the
	specified band nothing is done. Otherwise the origin of the specified
	band is changed in the source image and an {@link image_moved (const
	QPoint&, int) image moved signal} will be emitted after the tile grid
	has been adjusted as needed.

	The tile grid is adjusted only if the reference band is affected. The
	display of tiles is always relative to the reference band. Changes in
	the origin of any other band are reflected in how the source image
	data is mapped into the display image of each tile. If the new origin
	is within the bounds of the current tile grid (as determined by
	obtaining the {@link map_image_to_tile(const QPointF&, int) tile grid
	coordinate of the origin point}), and the origin has moved outside of
	the viewport origin tile (at effective tile grid coordinate 1,1) then
	the tiles are shifted left or right and/or up or down to bring the
	tile containing the origin into the position of the viewport origin
	tile. However, if the origin falls outside the tile grid then the
	tile grid origin is simply reset to the upper left corner of the tile
	one tile left and one tile above (the tile margin) the tile in which
	the image origin is now located.

	After adjusting the tile grid the offset of the display viewport
	origin in the viewport origin tile is reset.

	@param	origin	A QPoint specifying the image coordinate at the
		display viewport origin in source image pixel units.
	@param	band	The image band to be moved. If negative all image
		bands will be moved.
	@return	true if the image was moved; false otherwise.
*/
bool move_image (const QPoint& origin, int band = -1);

/**	Scale the image by horizontal and vertical scaling factors about a
	center (invariant) point.

	Nothing is done if the image is in the process of being loaded or
	there are no display tiles.

	The scaling factors are clipped to be in the (@link min_scale()
	minimum scaling} and {@link max_scale() maximum scaling} range,
	inclusive. If the resulting scaling is the same as the current
	image scaling nothing is done.

	The Renderer is {@link Image_Renderer::reset(int) reset} without
	waiting. The Reference_Image scaling is set to the new scaling.

	If all bands, or the reference (zero) band, is being scaled the
	{@link reset_tile_image_size() tile image size is reset} and the new
	image display origin is calculated and clipped to the display limits.
	The {@link displayed_tile_grid_origin(const QPointF&, int) tile grid
	origin} is recalculated and unconditionally reset, which is required
	for adjusting the cache of tile location information.

	The {@link pending_state_change() pending state change} has the
	{@link #IMAGE_SCALE_STATE} flag set, and the {@link
	#IMAGE_MOVE_STATE} flag is also set if the image origin changed.
	The display image tiles are {@link reset_tiles() reset} - which will
	initiate tile image rendering - and the image display is updated.
	Finally the {@link image_scaled(const QSizeF&, int) image scaled
	signal} is emitted, and the {@link image_moved(const QPoint&, int)
	image moved signal} is emitted if the image origin changed.

	@param	scaling	A QSizeF specifying the horizontal and vertical scaling
		factors to be applied after clipping to the allowed scaling range.
	@param	center	A QPoint specifying the invariant point for the scaling
		in source image coordinates.
	@param	band	The image band to be scaled. If negative all bands will
		be scaled.
	@return	true if the image scaling changed and the {@link
		image_scaled(const QSizeF&, int) image scaled signal} was emitted;
		false otherwise.
*/
bool scale_image (const QSizeF& scaling, const QPoint& center, int band = -1);

bool map_bands (const unsigned int* band_map);

/**	Map the source image data to the display image data.

	The Data_Maps to be used for mapping the source image data to the
	display image data may be obtained as pointers to the reference image
	{@link data_maps() data maps}. If different data maps are specified
	their contents will be copied into the reference image data maps
	before the data mapping is applied.

	@param	maps	An array of three Data_Map pointers. If NULL
		the reference image data maps are used.
	@return	true if the data mapping changed and tile image updates
		have been (or will be) applied.
*/
bool map_data (Data_Map** maps = NULL);

/**	Cancel any and all image rendering.

	Any tile images that are queued for, or being, rendered are {@link
	Image_Renderer::reset(int) canceled} and deleted from the rendering
	queue.

	The rendering cancellation is done without waiting for any tile
	rendering in progress to be completed. A {@link
	rendering_status(int) rendering status signal} will be emitted with
	the {@link #RENDERING_CANCELED} status value when all rendering
	activity has been completed. If the renderer was not active - no
	image tile was being rendered and none was queued for rendering -
	then the {@link #RENDERING_CANCELED} rendering status signal will be
	emitted immediately.

	If a {@link pending_state_change() display state change is pending}
	a {@link state_change(int) state change is signaled}.
*/
void cancel_rendering ();


private slots:

/**	Handle the {@link Image_Renderer::image_loaded(bool) image loaded}
	signal from the Image_Renderer.

	The {@link image_loaded(bool)} signal is always emitted at the end
	of this procedure.

	If the successful is true, the {@link image_scaled(const QSizeF, int)}
	signal and {@link displayed_image_region_resized(const QSize&)}
	signals are emitted to report the initial image scaling and displayed
	image region in the viewport.

	@param	successful	true if the image was successfully loaded; false
		otherwise.
*/
void loaded (bool successful);

/**	Filter the status signals from the Renderer.

	The Renderer status value corresponds to a local status value:

	- {@link #Image_Renderer::NOT_RENDERING} is
		{@link #NOT_RENDERING}.
	- {@link #Image_Renderer::RENDERING_LOW_PRIORITY} is
		{@link #RENDERING_BACKGROUND}.
	- {@link #Image_Renderer::RENDERING_HIGH_PRIORITY} is
		{@link #RENDERING_VISIBLE_TILES}.
	- {@link #Image_Renderer::LOADING_IMAGE} is
		{@link #LOADING_IMAGE).
	- The {@link #Image_Renderer::RENDERING_CANCELED} bit flag
		is the {@link #RENDERING_CANCELED} flag.

	A {@link rendering_status(int)} signal is emitted with the status
	value.

	If the status is NOT_RENDERING or the RENDERING_CANCELED bit flag
	is set a {@link Image_Renderer::clean_up() Renderer clean up} is
	done. If a display {@link pending_state_change() state change is
	pending}, that is not the IMAGE_LOAD_STATE, the {@link
	state_change_completed(int)} is signalled - <b>after</b> the
	rendering_status signal - with the RENDERING_COMPLETED_STATE or
	RENDERING_CANCELED_STATE flag set corresponding to the state
	value.

	@param	status	A status signal condition from the Renderer.
*/
void renderer_status (int status);

/**	Handle the {@link Image_Renderer::rendered(const QPoint&, const QRect&)
	rendered} signal from the Image_Renderer.

	This slot is connected to the Image_Renderer signal of the same name.
	The Image_Renderer is expected to emit the signal when it has
	completed a rendering operation. The type of rendering operation is
	indicated by the tile_coordinate argument:
	
	When the tile_coordinate isNull all tiles queued for rendering that
	overlap the display viewport have been rendered. In this case the
	entire Tiled_Image_Display is queued for an update which will result
	in a {@link paintEvent(QPaintEvent*)} being invoked later from the
	event queue for the entire region of the viewport. <b>After</b> the
	update has been queued the {@link state_change(int)} signal is
	emitted with the value of the {@link pending_state_change()
	Pending_State_Change} qualified with the
	RENDERING_VISIBLE_TILES_STATE and RENDERING_COMPLETED_STATE flags
	set; and the Pending_State_Change is then reset to NO_STATE_CHANGE.

	When the tile_coordinate is valid only the tile at the specified
	coordinate has been rendered. In this case if the tile_region is
	valid it is converted from a tile-relative region to a {@link
	viewport_relative_region(const QPoint&, QRect&)} that specifies the
	region of the display viewport that was rendered. If the tile_region
	isEmpty the entire {@link tile_display_region(const QPoint&)} for the
	tile_coordinate has completed rendering. If any portion of the
	viewport-relative region overlaps the display viewport that portion
	of the display is immediately repainted. This incremental rendering
	does not result in a state change signal.

	@param	tile_coordinate	A QPoint specifying the tile that was rendered.
		If the value is not valid the entire viewport is ready for an update.
	@param	tile_region	a QRect Specifiying the tile-relative region of the
		tile for the tile_coordinate that has completed rendering. If the
		value is not valid the entire tile has been rendered.
*/
void rendered
	(const QPoint& tile_coordinate, const QRect& tile_region = QRect ());

void rendering_error (const QString& message);

/*==============================================================================
	Event Handlers
*/
public:

virtual void paintEvent (QPaintEvent* event);
virtual void resizeEvent (QResizeEvent* event);
virtual void mouseMoveEvent (QMouseEvent* event);
virtual void mousePressEvent (QMouseEvent* event);
virtual void leaveEvent (QEvent* event);

/*==============================================================================
	Helpers
*/
protected:

/**	Calculate the image origin lower right limit point.

	When setting the {@link move_image (const QPoint&, int) image display
	origin} the origin point is contrainted to be within the image
	boundaries minus the viewport size in scaled image space on the
	bottom and right. This prevents placing the image display origin above
	and to the left of the image, and so far down and to the right that
	the display viewport would be beyond the image boundaries. However,
	when the scaled image size is less than the size of the display
	viewport lower right boundaries must necessarily be beyond the image
	boundaries. In this case any limit value that would be negative for
	the dimension in which the vieport size exceeds the scaled image size
	will be clipped to zero.

	<b>N.B.</b>: When calculating the image extent the maximum positive
	image offests and maximum image scaling factors for all bands are
	used.

	@return	A QPoint specifing the maximum image display origin location
		that will keep the display viewport boundaries from exceeding
		the boundaries of the image; except the lower and right boundaries
		of the viewport may exceed the image boundaries if, and only if,
		the corresponding viewport dimension is larger that the image
		dimension, in which case the origin limit coordinate will always
		be 0,0.
*/
QPoint calculate_lower_right_origin_limit () const;

/**	Calculate the region of the image, in image space, that is contained
	within the display viewport.

	Only that portion of the image that is being displayed is returned.
	The display viewport may extend beyond the boundaries of the
	displayed image.

	Determining the image region depends on {@link
	map_display_to_image(const QPoint&, int) mapping} the upper-left and
	lower-right corners of the display viewport to their image
	coordinates and clipping the lower-right coordinate to the limits of
	the {@link image_size()}.

	@param	band	The image band for which to obtain the region of the
		image appearing in the display.
	@return	A QRectF describing the region of the image that appears in
		the display viewport. <b>N.B.</b>: The image display region is
		provided as floating point values for accuracy in other contexts
		(e.g. scaled image dimensions). The dimensions of the region
		rectangle should be truncated if integer values are needed,  not
		rounded the the nearest integer.
	@see	displayed_image_region(int) const
	@see	image_display_region(int)
*/
QRectF calculate_displayed_image_region (int band = 0) const;

/**	Reset the Displayed_Image_Region cache.

	The {@link calculate_displayed_image_region(int) displayed image
	region is calculated} for the reference band (0). If the new region
	is different from the cached region the Displayed_Image_Region cache
	is updated. The truncated size - excess partial pixels are clipped
	off - of the previous region is compared to the truncated size of the
	new region to determine the return value.

	@return	true if the size of the displayed image region changed; false
		otherwise.
	@see	displayed_image_region(int) const
*/
bool reset_displayed_image_region ();

/**	Calculate the size of a tile in scaled image space.

	Each tile is an area with a fixed {@link tile_display_size() tile
	size} in the display space; i.e. the display size of each tile image
	is the same size. The image area rendered by each tile depends on the
	scaling, both horizontal and vertical, which may vary for each band.

	The display tile size is scaled by the scaling factors for the band.
	A scaling factor less than one increases the tile image size relative
	to a normal 1:1 scaling factor. The dimensions include any partial
	pixel at the boundaries which are used for accurate image tiling.

	@param	band	The image band to use as a scaling reference.
	@return	A QSizeF size in image space.
	@see	tile_image_size(int) const
*/
QSizeF calculate_tile_image_size (int band = 0) const;

/**	Reset the Tile_Image_Size cache.

	The {@link calculate_tile_image_size(int) tile image size is
	calculated} for the reference band (0) and used to update the
	Tile_Image_Size cache.

	@return true if the tile image size changed; false otherwise.
	@see tile_image_size(int) const
*/
bool reset_tile_image_size ();

/**	Calculate the effective tile grid region in image space.

	<b>N.B.</b>: The effective tile grid may extend beyond the bounaries
	of the image.

	Determining tiled region depends on the {@link
	displayed_tile_grid_origin(int)}, {@link
	calculate_tile_image_size(int)} and the Tile_Grid_Size.

	@param	band	The image band for which the tiled region is
		requested.
	@return	A QRectF describing the image region of the selected image
		band contained within the effective tile grid. Sub-pixel accuracy
		is used for the region values.
	@see	tiled_image_region(int) const
*/
QRectF calculate_tiled_image_region (int band = 0) const;

/**	Reset the Tiled_Image_Region cache.

	The {@link reset_tile_image_size() tile image size is reset} first.

	The {@link calculate_tiled_image_region(int) tiled image region is
	calculated} for the reference band (0) and used to update the
	Tiled_Image_Region cache.

	@return	true if the tiled image region changed; false otherwise;
	@see	tiled_image_region(int) const
*/
bool reset_tiled_image_region ();

/**	Convert a viewport-relative region to a tile-relative region.

	The tile region is assumed to be the intersection of the display
	viewport and a tile's entire region in display viewport coordinates.
	The origin of the resulting intersection - the tile_region - will be
	within the display viewport boundaries. The tile_region origin will
	be changed, without changing the region size, to a coordinate
	relative to the tile origin.

	When the tile origin relative to the viewport has a negative
	coordinate, then the area of the tile overlaps the top and/or left of
	the viewport, in which case the cooresponding tile_region origin
	coordinate will be moved to the negative of the (negative) tile
	viewport origin offset. For positive tile origin coordinates the
	tile_region origin coordinate will be moved to zero; i.e. there is no
	offset of the tile_region origin coordinate and the tile origin.
	<b>N.B.</b>: If the tile_region is empty its origin is
	unconditionally set to 0,0.

	@param	tile_region	A QRect specifying a tile region with 
		viewport-relative origin.
	@param	tile_origin	A QPoint giving the viewport-relative origin of
		the entirie tile of which tile_region is a part.
	@return	The tile_region with its origin (top-left) changed to be
		tile-relative.
*/
static QRect tile_relative_region
	(const QRect& tile_region, QPoint tile_origin);

/**	Convert a tile-relative region to a viewport-relative region.

	The specified tile region is assumed to have an origin relative to
	the tile specified by the tile coordinate. The origin of the region
	is changed, without changing the size of the region, to be relative
	to the viewport origin.

	<b>N.B.</b>: The conversion of the tile_region origin is done
	in-place.

	@param	tile_coordinate	A QPoint that specifies the tile grid
		coordinates of a tile.
	@param	tile_region	A QRect that specifies the region, with a
		tile-relative origin, of the tile referenced by the
		tile_coordinate. The origin (top-left) will be changed to bw
		viewport-relative.
*/
void viewport_relative_region
	(const QPoint& tile_coordinate, QRect& tile_region) const;

/**	Set each tile image as needing the specified rendering update
	and queue it for rendering.

	If the tile grid is empty nothing is done.

	Each valid tile in the grid has its image {@link
	Plastic_Image::needs_update(Mapping_Type) marked as needing the
	specified rendering update}. Then the image is {@link
	Image_Renderer::queue(Plastic_Image*, const QPoint&, const QRect&,
	bool) queued for rendering} with the appropriate image display
	region.

	@param	update_type	A Mapping_Type that specifies the rendering update
		that should be applied.
*/
void image_update_needed (Mapping_Type update_type);

/**	Start a change to the state of the image display.

	If the specified state type value (without qualifier flags) is
	non-zero and the current Pending_State_Change already includes the
	specified state no state change signal is emitted; but the Renderer
	is always started.

	The Pending_State_Change is updated with the new state type. The
	specified state is or-ed with the updated Pending_State_Change value
	and qualified based on the current {@link rendering_status()
	rendering status}: If the status is {@link #NOT_RENDERING} (no
	image tiles queued for rendering nor being rendered) the state value
	is unchanged, {@link #RENDERING_VISIBLE_TILES} sets the {@link
	#RENDERING_VISIBLE_TILES_STATE} flag, otherwise the {@link
	#RENDERING_BACKGROUND_STATE} flag is set. <b>Special case</b>: If the
	Pending_State_Change has {@link #IMAGE_LOAD_STATE} set the
	RENDERING_VISIBLE_TILES_STATE is set instead of using the rendering
	status. Then the {@link state_change(int)} signal is emitted with the
	resulting state value.

	Finally {@link Image_Renderer::start_rendering() rendering is started}.

	@param	state	The state change value indicating what display
		state is changing. <b>N.B.</b>: The value may contain qualifier
		flags.
	@see	state_change_completed(int)
*/
void state_change_start (int state = NO_STATE_CHANGE);

/**	A change to the image display has completed.

	If there is no {@link pending_state_change() pending state change}
	or any {@link rendering_status() rendering} is in progress or
	queued nothing is done; i.e. a state change is completed only
	when a state change is pending and all rendering has completed.

	The {@link state_change(int) state change} signal is emitted with the
	value of the Pending_State_Change and qualifier bit flag(s) set. Then
	the Pending_State_Change is reset to NO_STATE_CHANGE.

	@param	qualifiers	A state change value. This is expected, but not
		required, to contain only state change qualifier flags.
*/
void state_change_completed (int qualifiers = RENDERING_COMPLETED_STATE);

/*==============================================================================
	Data
*/
private:
    
QPoint Last_Clicked_Coord;

//!	Tile image rendering.
Image_Renderer
	*Renderer;

/**	The source image.

	The source image is copied to provide the reference image from which
	all tile grid images are cloned. The source image is used to paint a
	tile in the viewport display when the tile image has not yet been
	rendered.

	<b>N.B.</b>: The pointer to the source image is held by a
	Shared_Image reference counted pointer so it can be safely shared.
*/
Shared_Image
	Source_Image;

//!	Flag that the Source_Image is to be rendered here.
bool
	Source_Image_Rendering;

/**	The reference image.

	The reference image provides the image from which all rendering tiles
	are cloned. Its origin is the origin of the displayed tile grid in
	image space and its scale values are applied to all image tiles. Its
	band and data mapping structures are shared with all the tiles.
*/
Plastic_Image
	*Reference_Image;

//!	Flag that image loading is in progress.
bool
	Image_Loading;
	
//!	The scaling to apply to an image when first loaded.
QSizeF
	Initial_Scaling;

//!	Display state changes that are pending rendering completion.
int
	Pending_State_Change;
/**
	Guard against premature pending state change completion.

	When a state change is setup while a previous change is being
	rendered at the point that rendering is stopped or canceled (reset)
	{@link renderer_status(int) handling the renderer status signal} that
	occurs would inappropriately produce a pending state change
	completion if the new state change was not yet setup. This flag is a
	guard against that possibility: The flag is set false before the
	setup stops or cancels rendering and restored to true when setup is
	done; the signal handler checks the flag to determine if a pending
	state change should be signalled as completed.
*/
bool
	Pending_State_Change_Enabled;


//	The tile grid:

//!	The grid of image rendering tiles covering the Tiled_Image_Region.
QList<QList<Plastic_Image*>*>
	*Tile_Grid_Images;

//!	Pool of unused tile grid images.
QList<Plastic_Image*>
	Tile_Image_Pool;
int
	Tile_Image_Pool_Max;

//!	Size of the tile grid in tile units.
QSize
	Tile_Grid_Size;

//!	Size of a tile in display space.
QSize
	Tile_Display_Size;

/*------------------------------------------------------------------------------
	Cached tile location information.

	Cached values are all relative to the reference band (0).
*/
/**	Size of a tile in image space.

	N.B.: The tile size in image space is the size of the tile in
	display space divided by the image scaling. To ensure accurate
	tile placement in scaled image space the scaled tile size is
	measured to sub-pixel accuracy.
*/
mutable QSizeF
	Tile_Image_Size;

//!	Tiled region, including margin tiles, in image space.
mutable QRectF
	Tiled_Image_Region;

//	The display viewport:

//!	The region of the image, in image space, within the display viewport.
mutable QRectF
	Displayed_Image_Region;

//!	The lower-right limit, in image space, of the image display origin.
QPoint
	Lower_Right_Origin_Limit;

//!	Offset of the display viewport origin in the first (UL) visible tile.
QPoint
	Tile_Display_Offset;

/*------------------------------------------------------------------------------
*/
//!	Shared error message dialog.
static QErrorMessage
	*Error_Message;
};


}	//	namespace HiRISE
}	//	namespace UA
#endif
