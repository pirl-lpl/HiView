/*	JP2_Image

HiROC CVS ID: $Id: JP2_Image.hh,v 2.18 2012/09/19 01:04:29 castalia Exp $

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

#ifndef HiView_JP2_Image_hh
#define HiView_JP2_Image_hh

#include	"Plastic_Image.hh"

//	PIRL++
#include	"Reference_Counted_Pointer.hh"
#include	"Dimensions.hh"

//	Forward references.
template<typename T> class QVector;

namespace idaeim {
namespace PVL {
class Aggregate;
}}


namespace UA
{
namespace HiRISE
{
//	Forward references.
class JP2_Reader;
class JP2_Image_Rendering_Monitor;
class Source_Data;


class JP2_Image
:	public Plastic_Image
{
public:
/*==============================================================================
	Types:
*/
typedef PIRL::Reference_Counted_Pointer<JP2_Reader>	Shared_JP2_Reader;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	JP2 metadata parameters group.
static Name_String
	JP2_METADATA_GROUP;

/*==============================================================================
	Constructors
*/
/*	Construct a JP2_Image on a JP2_Reader.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image will not have been rendered, but will be filled with
	the {@link background_color() background color}; in this case the
	image should be  either {@link render_image() rendered} or {@link
	update() updated}.

	@param	reader	A pointer to a JP2_Reader. <b>N.B.</b>: The ownership
		of the JP2_Reader is transfered to the JP2_Image. If NULL the
		image will have no image data source.
	@param	size	The size of the new JP2_Image. If the size is not
		valid the rendered size from the JP2_Reader dividied by its
		resolution level will be used, unless the JP2_Reader is NULL in
		which case an empty image will be constructed.
	@param	band_map	A pointer to unsigned int array of three values
		that will be used as the shared band map for this JP2_Image. If
		NULL band map sharing is not enabled and a default band map will
		be provided.
	@param	transforms	An array of QTransform pointers, one for each
		band of the image, that will be used as the shared geometric
		transforms for this JP2_Image. If NULL geometric transforms
		sharing is not enabled and default identity transforms will be
		provided.
	@param	data_maps	An array of data map arrays, one for each band of
		the image, that will be used as the shared data maps for this
		JP2_Image. If NULL data map sharing is not enabled and default
		identity data maps will be provided.
*/
explicit JP2_Image (JP2_Reader* reader = NULL,
	const QSize&		size = QSize (),
	const unsigned int*	band_map = NULL,
	const QTransform**	transforms = NULL,
	const Data_Map**	data_maps = NULL);

/*	Construct a JP2_Image on a JP2_Reader.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image will not have been rendered, but will be filled with
	the {@link background_color() background color}; in this case the
	image should be  either {@link render_image() rendered} or {@link
	update() updated}.

	@param	reader	A JP2_Reader. The JP2_Reader is cloned to become
		the {@link source() source} of image data. 
	@param	size	The size of the new JP2_Image. If the size is not
		valid the rendered size from the JP2_Reader dividied by its
		resolution level will be used.
	@param	band_map	A pointer to unsigned int array of three values
		that will be used as the shared band map for this JP2_Image. If
		NULL band map sharing is not enabled and a default band map will
		be provided.
	@param	transforms	An array of QTransform pointers, one for each
		band of the image, that will be used as the shared geometric
		transforms for this JP2_Image. If NULL geometric transforms
		sharing is not enabled and default identity transforms will be
		provided.
	@param	data_maps	An array of data map arrays, one for each band of
		the image, that will be used as the shared data maps for this
		JP2_Image. If NULL data map sharing is not enabled and default
		identity data maps will be provided.
*/
explicit JP2_Image (const JP2_Reader& JP2_reader,
	const QSize&		size = QSize (),
	const unsigned int*	band_map = NULL,
	const QTransform**	transforms = NULL,
	const Data_Map**	data_maps = NULL);

/**	Copy a JP2_Image.

	The image {@link source() source} - a Shared_JP2_Reader - is copied.
	<b>N.B.</b>: The Reference_Counted_Pointer holding the JP2_Reader
	pointer is copied; i.e. the image source is shared with the JP2_Image
	being copied.

	The image {@link source_transforms() geometric transforms}, {@link
	source_band_map() band map} and {@link source_data_maps() data maps}
	are copied unless the corresponding shared mappings flag inidicates
	they are to be shared with the image being copied. The {@link
	background_color() background color} is also copied.

	<b>Warning</b>: If any data mappings are shared the JP2_Image being
	copied must remain valid as long as this JP2_Image is in use.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image clone will not have been rendered, but will be filled
	with the {@link background_color() background color}; in this case
	the image should be  either {@link render_image() rendered} or {@link
	update() updated}.

	@param	image	The JP2_Image to be copied.
	@param	size	The size of the new JP2_Image. If the size is not
		valid the size of the JP2_Image being copied (not its source
		image) will be used.
	@param	shared_mappings	A Mapping_Type that specifies any combination
		of {@link #BAND_MAP}, {@link #TRANSFORMS} or {@link #DATA_MAPS} -
		or {@link #NO_MAPPINGS} - data mappings that are to be shared with
		the JP2_Image being copied.
*/
explicit JP2_Image (const JP2_Image& image,
	const QSize&	size = QSize (),
	Mapping_Type	shared_mappings = NO_MAPPINGS);

/**	Construct a JP2_Image from a named source.

	A JP2_Reader will be constructed from the named source as the
	source of image data for the JP2_Image. If this fails no exception
	is thrown; the {@link source() data source} will be NULL.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image clone will not have been rendered, but will be filled
	with the {@link default_background_color() default background color};
	in this case the image should be  either {@link render_image()
	rendered} or {@link update() updated}.

	@param	source_name	The name of the image data source. This may be
		a filesystem pathname or a JPIP URL.
	@param	size	The size of the new JP2_Image.
*/
JP2_Image (const QString& source_name, const QSize& size);

/**	Destroy this JP2_Image.

	All data buffers and the JP2_Image_Rendering_Monitor are deleted. 
*/
virtual ~JP2_Image ();

/*==============================================================================
	Accessors
*/
/**	Clone this JP2_Image.

	A {@link JP2_Image(const JP2_Image&, const QSize& Mapping_Type) copy}
	of this JP2_Image is constructed.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image clone will not have been rendered, but will be filled
	with the {@link background_color() background color}; in this case
	the image should be  either {@link render_image() rendered} or {@link
	update() updated}.

	@param	size	The size of the new JP2_Image. If the size is not
		valid the size of this JP2_Image being copied (not its source
		image) will be used.
	@param	shared_mappings	A Mapping_Type that specifies any combination
		of {@link #BAND_MAP}, {@link #TRANSFORMS} or {@link #DATA_MAPS} -
		or {@link #NO_MAPPINGS} - data mappings that are to be shared with
		this JP2_Image being copied.
	@return	A pointer to a JP2_Image.
*/
virtual JP2_Image* clone (const QSize& size = QSize (),
	Mapping_Type shared_mappings = NO_MAPPINGS) const;

/**	Get the image metadata.

	If the metadata has already been assembled it is immediately returned.

	Metadata assembly is deferred until the first time the metadata is
	requested. Then the base {@link Plastic_Image::metadata() image metadata}
	is generated, and the {@link #JP2_METADATA_GROUP) parameters from the
	{@link source() JP2_Reader source} are appended.

	If {@link get_pds_label() getting a PDS label file has been enabled}

	@return	A pointer to the metadata parameters.
*/
virtual idaeim::PVL::Aggregate* metadata ();

/**	Get the image source.

	<b>WARNING</b>: The return value is a pointer to a JP2_Reader.
	However, the source of image data for a JP2_Image is a
	Shared_JP2_Reader - a pointer to a JP2_Reader held by a
	Reference_Counted_Pointer - which is shared by all copies/clones of
	the JP2_Image (and their copies/clones).

	@return	A pointer to a JP2_Reader.
*/
virtual const void* source () const;

virtual QSize source_size () const;

virtual unsigned int source_bands () const;

virtual unsigned int source_precision_bits () const;

virtual Pixel_Datum source_pixel_value
	(unsigned int x, unsigned int y, unsigned int band) const;

/**	Set the source image to display image band mapping.

	If the {@link Plastic_Image::source_band_map(const unsigned int*, bool)
	band mapping} is changed the {@link assign_source_data_buffers()
	source data buffers are re-assigned}.

	@param	band_map	An array of three source image band index values
		in display image band order.
	@param	shared	true if the specified band map is to be shared
		(externally owned); false if a band map is to be provided
		interally.
	@return	true if the existing band map was changed; false otherwise.
*/
virtual bool source_band_map
	(const unsigned int* band_map, bool shared = false);

inline const unsigned int* source_band_map () const
	{return Plastic_Image::source_band_map ();}

virtual void close ();

/**	Produce a histogram of source image data.

	Overrides the
	{@link Plastic_Image::source_histograms(QVector<Histogram*>, const QRect&)
	default source histogram} method by scanning the source data buffer
	contents corresponding the specified source region. This enables very
	large source images that have been read into the source data buffer
	at a low rendering resolution to be quickly scanned, rather than
	scanning the entire source image area and mapping each coordinate to
	a source buffer datum, with the huge amount of redundancy that would
	involve. <b>N.B.</b>: A consequence of this optimization is that
	fewer data values will be sampled for the histograms than the
	specified image region suggests.

	@param	histograms	A vector of three Histogram data vector pointers.
		Any of the pointers may be NULL in which case a Histogram data
		vector will be allocated, and its ownership transferred to the
		caller, as needed.
	@param	source_region	A QRect specifying the region of the source
		image, in unscaled source image pixel units, to be scanned in the
		source data buffer. Pixels on the edges of the rectangle are
		included.
	@return	The maximum number of pixels sampled for each histogram.
		This will be zero if no histogramming was done; e.g. if there is
		no JP2_Reader source, the source has not yet been rendered, or the
		source region does not intersect with the {@link image_region()
		image region}.
*/
virtual unsigned long long source_histograms (QVector<Histogram*> histograms,
	const QRect& source_region) const;

/*==============================================================================
	Image Rendering
*/
virtual bool needs_update (Mapping_Type changed)
	throw (Render_Exception, std::bad_exception);

virtual bool render_image ()
	throw (Render_Exception, std::bad_exception);

/*==============================================================================
	Helpers
*/
/**	Tests if the source names a JP2 formatted file.

	To be a valid JP2 formatted file the pathname must exist, be a
	normal readable file, and the first 12 bytes of the file must contain
	the {@link #JP2_SIGNATURE}.

	This function requires a pathname to a local file; it does not work
	with remote JPIP sources. It is intended to provide a quick check that a
	pathname refers to a JP2 file as might be used by a file selection
	filter.

	@param	pathname	A file pathname.
	@return	true if the file appears to contain JP2 formatted content;
		false otherwise.
*/
static bool is_JP2_file (const QString& pathname);

/**	Get the nearest JP2 resolution level for a scaling factor.

	The source image data is to be rendered at a resolution that provides
	the best fidelity, but not more than what is required for the
	specifed image scale. Thus the nearest resolution level for a given
	scale factor is the one that results in a rendering scale factor at
	or above the specifed image scale.

	For any scale factor greater than or equal to 1.0 the nearest
	resolution level is 1. In this case scaling the rendered JP2 region
	will either not be needed or a scaling up to greater than full
	resolution will be needed.

	For any other scale factor the resolution level at or above
	the level that produces the specified scale factor is determined. In
	this case scaling the rendered JP2 region down may be needed to
	achieve the specified image scale. The amount to scale down will be
	the residual scaling.

	@param	scale	A positive image scaling factor.
	@return	The nearest JP2 resolution level for the scale factor. A value
		zero will be returned if there is no data {@link source() source}.
	@throws	invalid_argument	If the scale factor is not positive.
*/
unsigned int nearest_resolution_level (double scale) const;


protected:

/**	Initialize the Source JP2_Reader and the pixel data buffers.

	The base Plastic_Image class is first intialized.

	A source image band data buffer is a dynamically allocated (on the
	heap) as an array of unsigned long long integers. A data buffer is
	allocated for the lesser of the number of display image bands and
	source image bands. These data buffers are {@link
	assign_source_data_buffers() assigned to source image bands} that
	have been {@link source_band_map() mapped} to display image bands.

	The size of a source data buffer for a single image band -
	Data_Buffer_Size bytes, is determined from the lesser of the source
	image area times the number of bytes per pizel sample or twice the
	image display area (to allow for a rendering size to the next higher
	resolution level for the current scaling). The Source_Data_Rendered
	array - that maps all source image bands to a buffer pointer - and
	the Display_Data_Buffers array - that maps each of the three display
	image bands to a buffer pointer - are given their initial assignments
	of source data buffers.

	<b>N.B.</b>: Each source data buffer is assumed to remained aligned
	with the source rendered region; i.e. the first rendered pixel sample
	for each band, at the source image location set when the {@link
	rendering_resolution_and_region() rendering resolution and region} is
	determined, will be written to the first source data buffer location.

	The source is configured to use the band sequential (BSQ) data
	organization for rendered image pixel sample values.

	The display image {@link source_origin(const QPointF&, int) origin}
	is initialized to the current source image rendered image origin, and
	the display image {@link source_scale(double, int) scaling} is
	initialized according to the current source image rendering
	resolution.

	A rendering monitor is constructed to receive notifications of source
	rendering events.

	@throws	out_of_range	If the calculated size of the source data
		buffer is larger than the maximum size_t value for the host
		system.
*/
virtual void initialize ();

/**	Assign a source data buffer for each source image band being rendered.

	A source data buffer is provided for each source image band that is
	{@link source_band_map() mapped} to a display image band. Each source
	data buffer that is needed is dynamically {@link
	allocate_source_data_buffer() allocated}. When the same source image
	band is mapped to different display image bands, only one source data
	buffer will be allocated for the source image band; i.e. source data
	buffers are allocated to source image bands, not display image bands,
	but a maximum of three source data buffers will be allocated because
	no more than three source image bands, regardless of how many bands a
	source image may have, will be mapped to the three display image
	bands.

	The source data buffers are reused, to minimize memory reallocation,
	when {@link source_band_map(const unsigned int*, bool) remapping the
	source image bands}. If a source data buffer becomes unused it is
	deleted.
*/
void assign_source_data_buffers ();

/**	Map a source image coordinate to the source data buffer coordinate
	where the source image pixel datum can be found.

	@param	image_coordinate	A QPoint in the full resolution source
		image space.
	@param	band	The display image band from which to obtain the pixel
		coordinate. <b>N.B.</b>: This is not a source image band number
		but a display image band number in the range 0-2 (corresponding
		to the red, green and blue display bands). Each display image
		band is {@link source_band(int) mapped} from only one source
		image band; each source image band may be mapped to any, or none,
		of the display image bands.
	@return	A QPoint specifying the coordinate in the source data band
		buffer (cast to the data type of the source image pixel values)
		where the source image pixel datum can be found. The coordinate
		will be (-1, -1) if the specified image coordinate does not
		fall within the bounds of the source data buffer.
*/
QPoint map_image_to_source (const QPoint& image_coordinate, int band) const;

/**	Set the image area and resolution level that will be used when
	{@link render_source(unsigned long long*, unsigned long long*)
	rendering the source data.

	The maximum current {@link source_scaling(int) source scaling} value
	is used to find the {@link nearest_resolution_level(double) nearest
	JPEG2000 resolution level} to be used when rendering the source data.
	The current {@link source_origin(int) source origin} and rendered
	image size is used to select the source image data region to be
	rendered.  <b>N.B.</b>:	When the scaling factor is less than 1.0 the
	size of the selected source image region is doubled from what is
	required for the selected source resolution level to provide source
	data for scaling factors down to the next lower resolution level. The
	image region and resolution level are set in the JP2_Reader {@link
	source() source data} object. If this results in no change to the
	source data object configuration this method will return false to
	indicate that re-rendering of the source data is not needed.

	The internal differential geometric transformation matrices - between
	the resolution level scale that is rendered from the source data and
	the actual image scaling that is rendered to the image data - are
	updated, along with the cached values of source image resolution
	level and region extent.

	@param	band	The image band to configure. If the value is negative
		the reference band (0) is used. The value is mapped to the {@link
		source_band(int) source data band} that has been assigned to the
		image band.
	@return	true if the JP2_Reader {@link source() source data} object
		configuration was changed such that source data rendering is
		needed; false if there was no change to the source data object,
		so the current contents of the source data buffer are correct.
*/
bool rendering_resolution_and_region (int band = -1);

/**	Test if the source data has been rendered.

	@param	band	The display band number that will have its
		Source_Data Rendered condition tested. If the band number is
		negative the Source_Data of all display bands will be tested.
	@return	true if the Source_Data for the display band, or all display
		bands, has been rendered.
*/
bool rendered_source_data (int band = -1) const;

/**	Set the rendered state of the Source_Data.

	<b>N.B.</b>: The Rendering_Display_Data_Buffers are used, not the
	Display_Data_Buffers. This method is only to be used during rendering
	operations, after which the Rendered state is transferred to the
	Display_Data_Buffers if they have not changed during rendering.

	@param	rendered	The Rendered condition to be applied.
	@param	band	The display band number that will have its
		Source_Data Rendered condition affected. If the band number is
		negative the Source_Data of all display bands will be affected.
*/
void source_data_rendered (bool rendered, int band = -1);

/**	Set the rendered state of the Source_Data.

	<b>N.B.</b>: The Rendering_Display_Data_Buffers are used, not the
	Display_Data_Buffers. This method is only to be used during rendering
	operations, after which the Rendered state is transferred to the
	Display_Data_Buffers if they have not changed during rendering.

	@param	rendered	The Rendered condition to be applied.
	@param	buffer	The Buffer of the Source_Data to have its Rendered
		condition affected.
*/
void source_buffer_rendered (bool rendered, void* buffer);

/**	Set or reset the rendering state.

	This method is expected to be called before rendering starts and
	after rendering completes. Data used during rendering operations that
	might be changed by another thread is copied.

	The base class {@link Plastic_Image::is_rendering(bool)} method
	is called first.

	If rendering was not active and has started the Data_Buffers are
	copied to the Rendering_Data_Buffers and the
	Rendering_Display_Data_Buffers are reset with pointers to the
	Rendering_Data_Buffers corresponding to the Display_Data_Buffers
	pointer relationship to the Data_Buffers.

	If rendering was active and has ended the Data_Buffers Rendering
	conditions are updated from the Rendering_Data_Buffers if the
	corresponding Source_Data state has not changed.

	@param	rendering	true if rendering is starting; false if
		rendering has been completed.
	@return	true if rendering was in progress when the method was called.
		If the rendering argument is true and the return value is true
		then rendering is already in progress and should not be restarted
		again until the method returns false. If the rendering argument is
		false and the return value is false, then nothing was done.
*/
virtual bool is_rendering (bool rendering);

inline bool is_rendering () const
	{return Plastic_Image::is_rendering ();}

/**	Render the JPEG2000 codestream into the source data buffers and
	map the source data to the display data.

	Rendering of the JPEG2000 codestream is done by the JP2_Reader {@link
	source() source data} object. It obtains from data storage the
	sections of the codestream that correspond to this JP2_Image {@link
	rendering_resolution_and_region(int) region and resolution level},
	incrementally decodes the codestream and deposits the rendered raw
	pixel values in the {@link assign_source_data_buffers() source data
	buffers} that have been allocated.

	<b>N.B.</b>: Each source data buffer is assumed to remained aligned
	with the source rendered region; i.e. the first rendered pixel sample
	for each band, at the source image location set when the {@link
	rendering_resolution_and_region() rendering resolution and region} is
	determined, will be written to the first source data buffer location
	of the buffer assigned to the band.

	If {@link mapping_differences() data mapping differences} has the
	{@link #TRANSFORMS} flags set - these flags indicate that the
	geometric transforms are not the same in all for the rendered bands -
	then a band-by-band rendering of the source data is done in which the
	source region and resolution are set individually for each band.
	Otherwise a single rendering is done with the common source region
	and resolution. <b>N.B.</b>: If setting the source region and
	resolution does not change the source data object configuration then
	the rendering for the band, or bands, is not done (the source data
	for the region has already been rendered).

	The amount of data that is expected to be rendered from the source is
	determined before each rendering operation that is done, and the
	amount of rendered pixel data actually rendered from the source is
	obtained. Only if the total amount of data actually rendered is equal
	to the amount expected is a true status returned. <b>N.B.</b>: If no
	data is expected from a rendering, because the source data expected
	has already been rendered into the raw pixel buffer, then no actual
	amount is obtained which results in a true rendered status.

	@return	true if all expected source data was actually obtained as
		rendered pixel data; false otherwise.
*/
bool render_source ();

/**	Map the source data to the display data.

	<b>N.B.</b>: The source data is not rendered. The current contents
	of the Source Data buffers are used.

	@return	true if the entire display was mapped from the source data;
		false if the mapping was canceled before completion.
*/
bool map_source ();


private:

/**	Map a region on the full resolution source image canvas to a display
	region on this image.

	This method is expected to be used during {@link is_rendering()
	source data rendering} to map a rendered source image region reported
	to the JP2_Image_Rendering_Monitor to the region of the display image
	to be scanned when {@link map_source_data_to_display_data (const
	QRect&) mapping source data to display data}.

	The specified source image region is relative to the full resolution
	image grid. The offset of this region from the full resolution image
	origin being used for Rendering, and its size, is scaled by the
	scaling factors being used for Rendering. This region is inclusively
	aligned to the display image coordinates - partial pixels at the
	boundaries are included - and then clipped to the extent of the
	display image size.

	@param	source_region	A Cube specifying a region of the source
		image relative to the full resolution grid. The Depth of this
		Cube is the number of bands that were rendered.
	@return	A QRect specifying a region of this image in display
		coordinates that encompasses the image region falling within the
		display image area. This may be a null rectangle if there is no
		cooresponding display image region.
*/
QRect map_source_to_display (const PIRL::Cube& source_region) const;

/**	Reset the array of source data buffer pointers by source band.

	The Source_Data_Buffers array is reset from
	Rendering_Display_Data_Buffers using the Rendering Band_Map.

	@return	The Source_Data_Buffers.
*/
void** source_data_buffers ();

/**	Map source data values into the image display data.

	@param	region	A QRect specifying a region of the display image for
		which source data buffer values are to be mapped to the display
		image data. <b>N.B.</b>: The origin of the region is relative
		to the display image origin at (0,0). If an empty region is
		specified the entire display area will be updated from the
		source data.
	@return	true if the mapping was completed; false if a {@link
		notify_rendering_monitors(Rendering_Monitor::Status, const
		QString&, const QRect&) rendering monitor notififcation} returned
		false to cancel the rendering.
*/
template<typename Pixel_Data_Type> bool map_source_data_to_display_data
	(const QRect& region = QRect ());

/**	Produce a histogram of source image data.

	@param	histogram	A pointer to a Histogram data vector.
	@param	source_region	A QRect specifying the region of the source
		image, in unscaled source image pixel units, to be scanned.
		Pixels on the edges of the rectangle are included.
	@param	band	A display image band. The band will be used to select
		the Source_Data buffer containing the rendered pixel data.
*/
template<typename Pixel_Data_Type> void source_data_histogram
	(Histogram* histogram, const QRect& source_region, int band) const;

/*==============================================================================
	Data
*/
private:

Shared_JP2_Reader
	Source;

//	The asynchronous JP2_Reader::Rendering_Monitor event notifier.
friend class JP2_Image_Rendering_Monitor;
JP2_Image_Rendering_Monitor
	*Source_Rendering_Monitor;

//!	Source data buffer management structure.

/*	Size of each source data buffer.

	Each buffer is the same size measured in bytes. The size is the
	smaller of the entire source image size of a single image band or
	twice the image display size (to allow for rendering size to the next
	higher resolution level for the current scaling).
*/
unsigned long long
	Data_Buffer_Size;

/**	Source data pointers.

	A pool of Source_Data buffers is provided for the storage of rendered
	pixel data from individual source image bands. There will be up to
	three buffers, one per display band, but no more than the number of
	source image bands (unallocated buffer pointer entries are NULL). If
	there are fewer source image bands than display bands a buffer will
	always be used as the source for more than one display band.
*/
Source_Data
	*Data_Buffers[3];

/**	Source data pointers, by display band index.

	Each display image band is assiged a source image data buffer. The
	band map determines which source band is associated with each display
	band. The same buffer may be assigned to more than one display band.
*/
Source_Data
	*Display_Data_Buffers[3];

/*------------------------------------------------------------------------------
	Rendering data. Only used during rendering.
*/

//	Copies of the Data_Buffers and corresponding Display_Data_Buffers.
Source_Data
	*Rendering_Data_Buffers[3],
	*Rendering_Display_Data_Buffers[3];

//	Differential transformations from rendered source to display.
QTransform
	Differential_Transforms[3];

/**	Source image band data buffer pointers, by source band index.

	Each source image band that is mapped to a display image band is
	associated with one buffer.

	<b>N.B.</b>: This is an array of pixel data storage addresses as used
	by the {@link JP2_Reader::image_data(void**, unsigned long long)}
	method to set source image bands to be rendered.
*/
void
	**Source_Data_Buffers;

};	//	Class JP2_Image


}	//	namespace HiRISE
}	//	namespace UA
#endif
