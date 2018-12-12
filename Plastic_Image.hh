/*	Plastic_Image

HiROC CVS ID: $Id: Plastic_Image.hh,v 2.33 2013/09/11 20:11:29 guym Exp $

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

#ifndef HiView_Plastic_Image_hh
#define HiView_Plastic_Image_hh

#include	<QImage>
#include	<QColor>
#include	<QMutex>

//	Forward references.
template<typename T> class QVector;
template<typename T> class QList;
class QString;
class QSize;
class QSizeF;
class QPoint;
class QPointF;
class QTransform;

namespace idaeim::PVL {
class Aggregate;
}

#include	<iosfwd>
#include	<string>

#ifdef _WIN32
/*
	The MSVC compiler, as of this writing, does not support throw
	specifications for method/function declarations. This pragma disables
	the warning messages (4290) that would otherwise be produced at compile
	time as a result of the ignored throw specifications.
*/
#pragma warning (disable : 4290)
#endif


namespace UA::HiRISE
{
/**	A <i>Plastic_Image</i> is a QImage that appears to be geometrically
	and photometrically plastic.

	Plastic_Image is an abstract class that must be implemented by
	concrete classes that provide the image data source.

	Each subclass is expected to initialize the Plastic_Image during
	construction.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 2.33 $
*/
class Plastic_Image
:	public QImage
{
public:
/*==============================================================================
	Types
*/
//!	A constant name string.
typedef const char* const	Name_String;

//!	Data type of a generic pixel sample as used in a {@link #Triplet}.
typedef unsigned int		Pixel_Datum;

//!	A lookup table that maps source data values to display image values.
typedef QVector<quint8>		Data_Map;

//!	A histogram table of pixel value counts by pixel value.
typedef QVector<quint64>	Histogram;

//!	A set of bit flags indicating possible data mapping changes.
typedef unsigned int		Mapping_Type;

/**	A <i>Triplet</i> holds pixel values for up to three bands.

	A Triplet has three Pixel_Datum values.
	The values are expected to be for red, green and blue image bands,
	but the color names are conventially descriptive rather than
	necessarily prescriptive.

	A Triplet is used to provide a {@link
	Plastic_Image::source_pixel(const QPoint&) source image pixel value}.
*/
struct Triplet
	{
	//!	The Triplet's Datum values.
	Pixel_Datum
		Datum[3];

	/**	Construct an undefined Triplet.

		All Datum entries will have the {@link #UNDEFINED_PIXEL_VALUE}.
	*/
	Triplet ()
	{
	Datum[0] =
	Datum[1] =
	Datum[2] = UNDEFINED_PIXEL_VALUE;
	}

	/**	Construct a Triplet from three Pixel_Datum values.

		@param	red		The Pixel_Datum value for Datum[0].
		@param	green	The Pixel_Datum value for Datum[1].
		@param	blue	The Pixel_Datum value for Datum[2].
	*/
	Triplet (Pixel_Datum red, Pixel_Datum green, Pixel_Datum blue)
	{
	Datum[0] = red;
	Datum[1] = green;
	Datum[2] = blue;
	}

	/**	Construct a Triplet from a QRgb value.

		@param	value	A QRgb value. Datum[0] is assigned from
			qRed(value), Datum[1] from aGreen(value), and Datum{2]
			from qBlue(value).
	*/
	explicit Triplet (QRgb value)
	{
	Datum[0] = qRed (value);
	Datum[1] = qGreen (value);
	Datum[2] = qBlue (value);
	}

	/**	Construct a Triplet from an array of Pixel_Datum values.

		@param	data	A Pixel_Datum array. If NULL all Datum entries
			will have the {@link #UNDEFINED_PIXEL_VALUE}.
		@param	amount	The number of data array values. Up to the first
			three values will be assigned to the corresponding Datum
			entries; any remaining Datum entries will have the
			{@link #UNDEFINED_PIXEL_VALUE}.
	*/
	explicit Triplet (const Pixel_Datum* const data, int amount = 3)
	{
	for (int index = 0;
			 index < 3;
		   ++index)
		{
		if (data &&
			amount > index)
			Datum[index] = data[index];
		else
			Datum[index] = UNDEFINED_PIXEL_VALUE;
		}
	}

	/**	Copy constructor.

		@param	triplet	The Triplet that will have its Datum values
			copied to the Datum of the new Triplet.
	*/
	Triplet (const Triplet& triplet)
	{
	Datum[0] = triplet.Datum[0];
	Datum[1] = triplet.Datum[1];
	Datum[2] = triplet.Datum[2];
	}

	/**	Assignment operator.

		@param	triplet	The Triplet that will have its Datum values
			assigned to the Datum of this Triplet.
		@return	This Triplet.
	*/
	Triplet& operator= (const Triplet& triplet)
	{
	if (this != &triplet)
		{
		Datum[0] = triplet.Datum[0];
		Datum[1] = triplet.Datum[1];
		Datum[2] = triplet.Datum[2];
		}
	return *this;
	}
	};


/**	A <i>Metadata_Monitor</i> is a virtual interface used to monitor
	changes to the image metadata.

	A Metadata_Monitor is a pure virtual interface class. An
	implementation of this class may be {@link
	Plastic_Image::add_metadata_monitor(Metadata_Monitor*) registered}
	with a Plastic_Image to receive notifications whenever the {@link
	metadata() image metadata} has changed.
*/
struct Metadata_Monitor
	{
	/**	Notification of metadata change.

	@param	image	A reference to the Plastic_Image for which its
		{@link Plastic_Image::metadata() metadata} has changed.
	*/
	virtual void metadata_changed (Plastic_Image& image) = 0;

	virtual ~Metadata_Monitor () {}
	};


/**	A <i>Rendering_Monitor</i> is a virtual interface used to monitor
	image source rendering progress.

	A Rendering_Monitor is a pure virtual interface class. An
	implementation of this class may be {@link
	Plastic_Image::add_rendering_monitor(Rendering_Monitor*) registered}
	with a Plastic_Image to receive notifications of each image source
	rendering increment.
*/
struct Rendering_Monitor
	{
	/**	Rendering status bit flags.

		A rendering status value is a bit map of possible
		rendering status conditions that may reported.
	*/
	enum Status
		{
		INFO_ONLY			= 0,
		LOW_QUALITY_DATA	= (1 << 0),
		TOP_QUALITY_DATA	= (1 << 1),
		RENDERED_DATA_MASK	= (1 << 0) |
							  (1 << 1),
		CANCELED			= (1 << 2),
		DONE				= (1 << 3),
		MAPPING_DATA		= (1 << 4),
		REMAPPING_DATA		= MAPPING_DATA |
							  TOP_QUALITY_DATA
		};

	static const QString
		Status_Message[];

	/**	Get a brief description of a rendering status value.
	*/
	inline static const QString& status_message (Status status)
		{return Status_Message[status];}

	/**	Notification of rendering progress.

		<b>N.B.</b>: A Plastic_Image may not implement incremental
		rendering, in which case notification will only occur when
		the rendering is done.

		@param	image	A reference to the Plastic_Image that is doing
			the image rendering.
		@param	status	A rendering {@link #Status} code that indicates
			the context in which the notification is being sent.
		@param	message	A message associated with the notification event.
			This may be empty.
		@param	display_region	A QRect describing the region of the
			display image, which may be different than the source image
			data region that was {@link render_image() mapped} to
			the display image, that has just been rendered. This may
			be empty if no new data is available.
		@return	true, if the rendering process is to continue; false if
			the rendering process is to be discontinued.
	*/
	virtual bool notification (Plastic_Image& image,
		Status status, const QString& message, const QRect& display_region) = 0;

	virtual ~Rendering_Monitor () {}
	};


/**	Rendering exception.

	This exception is to be thrown by the {@link render_image()} method,
	and all subclasses that implement this method, if any exception
	occurs during rendering. No other type of exception is to be thrown.
	The message associated with the exception should fully describe the
	cause of the exception in the render_image method.

	N.B.: The {@link update()} and {@link needs_update(Mapping_Type)}
	methods, and only these methods, call render_image. Thus all three
	methods may throw this exception, but only this exception.
*/
struct Render_Exception
{
std::string
	Message;

explicit Render_Exception (const std::string& message)
	:	Message (std::string (Plastic_Image::ID) + '\n' + message)
	{}

std::string message () const
	{return Message;}
};

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Format used for all Plastic_Image base QImage class objects.
static const QImage::Format
	IMAGE_FORMAT;

//!	Image metadata parameters group.
static Name_String
	IMAGE_METADATA_GROUP,
	IMAGE_SOURCE_PARAMETER,
	IMAGE_WIDTH_PARAMETER,
	IMAGE_HEIGHT_PARAMETER,
	IMAGE_BANDS_PARAMETER,
	VALUE_BITS_PARAMETER;

//!	The source_pixel_value for a coordinate outside the source boundaries.
static const Pixel_Datum
	UNDEFINED_PIXEL_VALUE;

//!	Maximum supported image data pixel sample (band) datum precision in bits.
static const unsigned int
	MAXIMUM_PIXEL_DATUM_PRECISION;

//!	A set of mappings (geotransform or data) are all the same.
static const Mapping_Type
	IDENTICAL_MAPPINGS;

//!	No data mapping changes are pending.
static const Mapping_Type
	NO_MAPPINGS;

//!	The band map change bit flag of a Mapping_Type value.
static const Mapping_Type
	BAND_MAP;

//!	The geotransforms change bit flag of a Mapping_Type value.
static const Mapping_Type
	TRANSFORMS;

//!	The data maps change bit flag of a Mapping_Type value.
static const Mapping_Type
	DATA_MAPS;

/**	Bit mask that selects all mapping bit flags.

	This is equivalent to BAND_MAP | TRANSFORMS | DATA_MAPS.
*/
static const Mapping_Type
	ALL_MAPPINGS;

/*------------------------------------------------------------------------------
	Defaults
*/
//!	The default background color of the image.
static QRgb
	Default_Background_Color;

//!	Default auto-update mode.
static bool
	Default_Auto_Update;

//!	Default number of output image lines in each rendering increment.
static unsigned int
	Default_Rendering_Increment_Lines;

/*==============================================================================
	Constructors
*/
protected:
explicit Plastic_Image (const QSize& size,
	const unsigned int*	band_map = NULL,
	const QTransform**	transforms = NULL,
	const Data_Map**	data_maps = NULL);

private:
Plastic_Image () {};
Plastic_Image (const Plastic_Image&) : QImage () {};

public:
//!	Destructor.
virtual ~Plastic_Image ();

/*==============================================================================
	Accessors
*/
/**	Set the image source name.

	The image source name is expected to be a local filesystem pathname
	or a URL for a remote file (though "file" URLs are also acceptable).

	<b>N.B.</b>: If this Plastic_Image has been {@link closed() closed}
	nothing is done.

	@param	name	A QString containing the image source name.
	@return	true if the name was changed; false otherwise.
*/
virtual bool source_name (const QString& name);

/**	Get the image source name.

	@return	A QString containing the image source name.
	@see	source_name(const QString&)
*/
inline QString source_name () const
	{return Source_Name;}

//	Methods that require, or are releated to, subclass implementations ---------

/**	Construct a clone of this Plastic_Image.

	This method is implemented by a subclass which will invoke its copy
	constructor.

	@param	size	A QSize specifying the size of the cloned image.
	@param	shared_mappings	A Mapping_Type bit map indicating which
		mappings of this image are to be shared with the cloned image.
	@return	A pointer to a Plastic_Image that is a copy of this image.
*/
virtual Plastic_Image* clone (const QSize& size = QSize (),
	Mapping_Type shared_mappings = NO_MAPPINGS) const = 0;

/**	Get the image metadata.

	@return	A pointer to the metadata parameters.
*/
virtual idaeim::PVL::Aggregate* metadata ();

/**	Add a Metadata_Monitor for this Plastic_Image.

	<b>N.B.</b>: A monitor can not be added from within the
	metadata_changed method of a monitor for this image; this would cause
	a dead lock condition since the list of registered monitors is
	protected by a mutual exclusion lock to ensure thread safety.

	@param	monitor	A pointer to the Metadata_Monitor to add. A NULL
		value is not added.
	@return	true if the monitor was added; false if it was not added
		because it was already registered, the value is NULL, or the
		image is {@link closed() closed}.
	@see	remove_metadata_monitor(Metadata_Monitor*)
*/
bool add_metadata_monitor (Metadata_Monitor* monitor);

/**	Remove a Metadata_Monitor for this Plastic_Image.

	<b>N.B.</b>: A monitor can not be removed from within the metadata_changed
	method of a monitor for this image; this would cause a dead lock
	condition since the list of registered monitors is protected by
	a mutual exclusion lock to ensure thread safety.

	@param	monitor	A pointer to the Metadata_Monitor to remove.
	@return	true if the monitor was removed; false if it was not
		registered.
	@see	add_metadata_monitor(Metadata_Monitor*)
*/
bool remove_metadata_monitor (Metadata_Monitor* monitor);

protected:
/**	Notify all {@link add_metadata_monitor (Metadata_Monitor*)
	registered} metadata monitors.

	If the image is {@link closed() closed} nothing is done.
*/
void notify_metadata_monitors ();

public:

/**	Get the data source for this Plastic_Image.

	The data source is used by an implementing subclass to obtain
	image data for rendering to this image's display data.

	@return	A pointer to the data source for this image. The type of the
		data source depends on the implementing subclass; users of this
		method will have to cast the pointer to the appropriate type.
*/
virtual const void* source () const = 0;

/**	Get the size of the source image.

	@return	A QSize containing the size of the source image in unscaled
		pixel units.
*/
virtual QSize source_size () const = 0;

/**	Get the width of the source image.

	@return The width of the source image in unscaled pixel units.
	@see	source_size()
*/
inline unsigned int source_width () const
	{return source_size ().rwidth ();}

/**	Get the height of the source image.

	@return The height of the source image in unscaled pixel units.
	@see	source_size()
*/
inline unsigned int source_height () const
	{return source_size ().rheight ();}

/**	Get the number of bands in the source image.

	@return The number of bands in the source image.
*/
virtual unsigned int source_bands () const = 0;

/**	Get the source image pixel value precision bits.

	@return	The pixel datum value precision bits. <b>N.B.</b>: The
		precision is for a pixel datum of a single band; all source image
		bands will have the same precision.
	@see	source_precision_bytes()
*/
virtual unsigned int source_precision_bits () const = 0;

/**	Get the source image pixel value precision bytes.

	A pixel datum will occupy a multiple of an 8-bit byte.

	@return	The pixel datum value precision bytes. <b>N.B.</b>: The
		precision is for a pixel datum of a single band; all source image
		bands will have the same precision.
	@see	source_precision_bits()
	@see	bits_bytes(unsigned int)
*/
inline unsigned int source_precision_bytes () const
	{return bits_bytes (source_precision_bits ());}

/**	Get the value of a source image pixel datum.

	@param	x	The horizontal location of the pixel in the source image
		before any {@link source_transforms() geometric transforms} are
		applied. Location 0 is the first pixel at the left edge of the
		image with location values increasing to the right.
	@param	y	The vertical location of the pixel in the source image
		before any {@link source_transforms() geometric transforms} are
		applied. Location 0 is the first pixel at the top edge of the
		image with location values increasing downwards.
	@param	band	The display image band from which to obtain the pixel
		datum. <b>N.B.</b>This is not a source image band number, which
		are mapped into display image bands, but a display image band
		number in the range 0-2 (corresponding to the red, green and blue
		display bands). Each display image band is {@link
		source_band(int) mapped} from only one source image band; each
		source image band may be mapped to any, or none, of the display
		image bands.
	@return	A Pixel_Datum value. This will be the {@link
		#UNDEFINED_PIXEL_VALUE} (equivalent to -1) if the x,y coordinate
		does not fall within the source image {@link image_region(int)
		region} currently available, or the display band number is
		invalid.
*/
virtual Pixel_Datum source_pixel_value
	(unsigned int x, unsigned int y, unsigned int band) const = 0;

/**	Get the source image pixel value at a source image location.

	The {@link source_pixel_value(unsigned int, unsigned int, unsigned int)
	source pixel datum} for each display band is collected into an RGB
	Triplet.

	@param	point	A QPoint specifying a source image location.
	@return	A Triplet containing the pixel datum values in Red, Green,
		Blue order. <b>N.B.</b>: Any and all of the Triplet datum values
		may be the {@link #UNDEFINED_PIXEL_VALUE} if the point does not
		fall within the source image {@link image_region(int) region}
		currently available for the corresponding band.
*/
virtual Triplet source_pixel (const QPoint& point) const;

/**	Close the source image.

	The source image will no longer be accessible. The display image
	will remain unchanged after this point.

	@see	closed()
*/
virtual void close ();

/**	Test if the source image has been {@link closed() closed}.

	@return true if the source image is closed; false if the image remains
		open for further changes.
*/
bool closed () const;

//	Source band by image band mapping ------------------------------------------

/**	Set the image band map.

	An image band map is an array of three source image band index values
	in display image band order. Display image bands are red (0), green (1)
	and blue (2) and never change. There are an indefinate number of source
	image bands. The band map specifies which source image bands are
	used for each of the three display image bands. The same source image
	band may be used for more than one display image band.

	<b>N.B.</b>: If the image is {@link closed() closed} nothing is done.

	If no band map is specified (band_map is NULL) and the current band
	map is {@link local_mappings() locally owned} nothing is done.
	However, if the band map is currently shared, the external band map
	is copied to a locally owned band map. <b>N.B.</b>: This is how a
	shared band map can be "acquired" locally. In the case where the
	shared band map is NULL, a {@link source_band_map_reset() default
	band map} will be provided locally.

	<b>N.B.</b>: A change to the band map will only cause the display
	image to be {@link update() updated} if {@link auto_update(bool)
	auto-update} has been enabled.

	If the shared argument is false the changes are applied to the band
	map regardless of whether it is locally owned or not; i.e. when the
	shared argument is false the specified band map is not to be shared
	as an object but its values are to be copied into the band map being
	used. If the shared argument is true the current band map is deleted,
	if it is not the same object as the specified band map and is also
	locally owned, and the specified band map is set as the current band
	map and flagged as externally owned.

	@param	band_map	An array of three source image band index values
		in display image band order. If NULL and the band map is
		currently being shared it will revert to a locally owned map.
	@param	shared	true if the specified band map is to be shared
		(externally owned); false if a band map is to be provided
		interally.
	@return	true if the existing band map was changed; false otherwise.
	@throws	invalid_argument	If the new band map contains a source image
		band value that does not exist in the source image.
	@see	source_band_map()
*/
virtual bool source_band_map
	(const unsigned int* band_map, bool shared = false);

/**	Get the image band map.

	@return	A pointer to the effective band map.
	@see	source_band_map(const unsigned int*, bool)
*/
inline unsigned int* source_band_map () const
	{return Band_Map;}

/**	Get the source image band mapped to a display image band.

	A source image may have zero (in the degenerate case of an empty
	image) or more image bands. The limit to the number of possible
	source image bands is set by the maximum integer value. Each source
	image band may be mapped to any, or none, of the display image bands.

	The display image always has three bands, so display image band
	numbers are in the range 0-2 (corresponding to the red, green and
	blue display bands). Each display image band is mapped from one, and
	only one, source image band.

	Note that the same source image band may be mapped to more than one
	display image band. For example, a single band (monchrome) source
	image will have its band 0 mapped to all three display bands.

	@param	band	A display image band number.
	@return	The source image band number that is mapped to the display
		image band.
	@throws	logic_error	if no band map has been allocated.
	@throws invalid_argument	if the specifed band is not in the display
		image band number range of 0-2.
	@see	source_band_map()
*/
int source_band (int band) const;

/**	Reset the image band map to the default state.

	If this Plastic_Image does not have a band map, neither locally
	owned nor shared, a locally owned band map is allocated for it.

	The first three source image bands will be used for the display image
	bands. However, if there are less than three source image bands the
	last source image band will be used for all remaining display image
	bands. For a monochrome source image this means that the first and
	only band will be used for all display image bands, as would be
	expected. In the degenerate case where the source image has no bands
	a band map with all entries set to zero will be still be supplied.

	@return	true if there was any change to the band map; false otherwise.
*/
bool source_band_map_reset ();

/**	Reset a band map to the default state.

	@param	band_map	An array of three source image band index values
		in display image band order. If NULL nothing is done.
	@return	true if the map was changed; false otherwise.
	@see	source_band_map_reset()
*/
bool reset_band_map (unsigned int* band_map);

/**	Test if a band map is different from the current band map.

	@param	band_map	An array of three source image band index values
		in display image band order. May be NULL.
	@return true if the specified band map and the current {@link
		source_band_map() band map} contain any corresponding entries
		that are different; false if all entries are identical. If both
		the specified band map and the current band map are NULL false is
		returned; if one or the other is NULL true is returned.
*/
bool different_band_map (const unsigned int* band_map) const;

//	Geometric transformation ---------------------------------------------------

//		Strange compiler glitch requires disambiguation of source_transform?!?
/**	Set the source-to-display geometric transformation for a display
	image band.

	A source-to-display geometric transformation controls the placement
	of source image pixels in the display image. A 2 dimensional
	transformation is used from the source image coordinate system to the
	display image coordinate system. The transformation is specified by a
	3x3 matrix of polynomial coefficients that implement a combination of
	{@link source_origin(const QPointF&, int) translation}, {@link
	source_scaling(const QSizeF&, int) scaling}, rotation, shearing, and
	perspective coordinate transformations.

	If the image is {@link closed() closed} nothing is done.

	<b>N.B.</b>: A change to the source-to-display geometric
	transformation will only cause the display image to be {@link
	update() updated} if {@link auto_update(bool) auto-update} has been
	enabled.

	@param	transform	A QTransform specifying the source-to-display
		image transformation to be applied.
	@param	band	The display band to which the transformation applies.
		If negative the transformation is applied to all display bands. If
		greater than 2 (the maximum display band index) nothing is done.
	@return	true if the source-to-display geometric transformation was
		changed; false otherwise.
	@see	source_transform(int)
	@see	source_transform_reset(int)
	@see	source_transforms(const QTransform**, bool)
*/
virtual bool set_source_transform
	(const QTransform& transform, int band = -1);

/**	Get the source-to-display geometric transformation for a display
	image band.

	@param	band	The display band for which to get the transformation.
	@return	A pointer to the QTransform for the band.
	@throws	invalid_argument	If the band number is not in the range 0-2.
	@see	set_source_transform(const QTransform&, int)
*/
QTransform* source_transform (int band = 0) const;

/**	Set the source-to-display geometric transformations for all bands.

	If the image is {@link closed() closed} nothing is done.

	<b>N.B.</b>: A change to the source-to-display geometric
	transformations will only cause the display image to be {@link
	update() updated} if {@link auto_update(bool) auto-update} has been
	enabled.

	@param	transforms	An array of three QTransform pointers, in display
		band order. If NULL and the transforms are currently not {@link
		local_mappings() locally owned} local owned copies of the current
		transforms are made; if the transforms are already locally owned
		nothing is done.
	@param	shared	If true locally owned transforms are deleted and the
		{@link source_transforms() transforms array} (a pointer to the
		QTransform pointers) is replaced with the new transforms array.
		<b>N.B.</b>: In this case each of the QTransform pointers must be
		valid and, along with the array pointer itself, must remain valid
		as long as the transforms are in use. If shared is false a local
		copy is made of each QTransform in the new array. In this case a
		NULL entry in the new transform array results in an identity
		transform as if the transform for the corresponding band was
		{@link source_transform_reset(int) reset}.
	@return	true if any source-to-display geometric transformation was
		changed; false otherwise.
	@see	source_transforms()
	@see	set_source_transform(const QTransform&, int)
	@see	source_transform_reset(int)
*/
virtual bool source_transforms (const QTransform** transforms,
	bool shared = false);

/**	Get the array of source-to-display geometric transformations.

	@return	An array of (pointer to) three QTransform pointers, in
		display band order.
	@see	source_transforms(const QTransform**, bool)
*/
inline QTransform** source_transforms () const
	{return Geo_Transforms;}

/**	Reset a source-to-display geometric transformation to identity.

	The source-to-display geometric transformation for the band is {@link
	set_source_transform(const QTransform&, int) set} to the identity
	transform. The identity transform causes no source-to-display
	geometric change.

	<b>N.B.</b>: A change to the source-to-display geometric
	transformation will only cause the display image to be {@link
	update() updated} if {@link auto_update(bool) auto-update} has been
	enabled.

	@param	band	The display band to which the transformation applies.
		If negative the transformation is applied to all display bands. If
		greater than 2 (the maximum display band index) nothing is done.
	@return	true if the source-to-display geometric transformation was
		changed; false otherwise.
*/
bool source_transform_reset (int band = -1);

bool source_origin (const QPointF& origin, int band = -1);
QPointF source_origin (int band = 0) const;

/**	Set the source image scaling factors.

	If the image is {@link closed() closed} nothing is done.

	<b>N.B.</b>: A change to the scaling will only cause the display
	image to be {@link update() updated} if {@link auto_update(bool)
	auto-update} has been enabled.

	@param	scale_horizontal	The horizontal scaling factor.
	@param	scale_vertical		The vertical scaling factor.
	@param	band	The display band to which the scaling applies.
		If negative the scaling is applied to all display bands.
	@throws	invalid_argument	If the band number is not in the range 0-2.
	@return	true if the scaling changed; false otherwise.
	@see	source_scaling(const QSizeF&, int)
*/
bool source_scaling
	(double scale_horizontal, double scale_vertical, int band = -1);

/**	Set the source image scaling factors.

	If the image is {@link closed() closed} nothing is done.

	<b>N.B.</b>: A change to the scaling will only cause the display
	image to be {@link update() updated} if {@link auto_update(bool)
	auto-update} has been enabled.

	@param	scaling	A QSizeF containing the horizontal (width) and
		vertical (height) scaling factors.
	@param	band	The display band to which the scaling applies.
		If negative the scaling is applied to all display bands.
	@throws	invalid_argument	If the band number is not in the range 0-2.
	@return	true if the scaling changed; false otherwise.
	@see	source_scaling(double, double, int)
*/
inline bool source_scaling (const QSizeF& scaling, int band = -1)
	{return source_scaling (scaling.width (), scaling.height (), band);}

/**	Set the source image scaling factors.

	If the image is {@link closed() closed} nothing is done.

	<b>N.B.</b>: A change to the scaling will only cause the display
	image to be {@link update() updated} if {@link auto_update(bool)
	auto-update} has been enabled.

	@param	scale	The scaling factor for both the horizontal and
		vertical dimensions.
	@param	band	The display band to which the scaling applies.
		If negative the scaling is applied to all display bands.
	@throws	invalid_argument	If the band number is not in the range 0-2.
	@return	true if the scaling changed; false otherwise.
	@see	source_scaling(double, double, int)
*/
inline bool source_scale (double scale, int band = -1)
	{return source_scaling (scale, scale, band);}

/**	Get the source image scaling factors.

	@param	scale_horizontal	A pointer to a double variable that will
		receive the horizontal scaling factor.
	@param	scale_vertical	A pointer to a double variable that will
		receive the scale_vertical scaling factor.
	@param	band	The display band for which to get the scaling factors.
*/
void source_scaling
	(double* scale_horizontal, double* scale_vertical, int band = 0) const;

/**	Get the source image scaling factors.

	@param	band	The display band for which to get the scaling factors.
	@return	A QSizeF containing the horizontal (width) and vertical
		(height) scaling factors.
	@see	source_scaling (const QSizeF&, int)
	@see	source_scaling(double*, double*, int)
*/
QSizeF source_scaling (int band = 0) const;

/**	Test if the geometric transformations for each image band are
	different.

	@return	The {@link #TRANSFORMS} Mapping_Type if the geometric
		transformations for each image band are different; {@link
		#IDENTICAL_MAPPINGS} (0) if the transformations for all bands are
		the same.
*/
inline Mapping_Type different_transforms () const
	{
	return
		 (Geo_Transforms &&
		(*Geo_Transforms[0] != *Geo_Transforms[1] ||
		 *Geo_Transforms[0] != *Geo_Transforms[2])) ?
		TRANSFORMS : IDENTICAL_MAPPINGS;
	}

/**	Get the region of the source image contained within the display image.

	@param	band	The image band for which to obtain the region.
	@return	A QRect that specifies that part of the image, in unscaled
		source image pixel units, that is contained within the display
		image area. <b>N.B.</b>: The source image region extent may not
		reach to the right and/or bottom of the display image, but the
		{@link source_origin(int) source image region origin} will always
		be cooincident with the origin of the display image upper-left
		corner.
*/
QRect image_region (int band = 0) const;

/**	Get the size of the display image area containing the scaled source
	image region.

	@param	band	The image band for which to obtain the image area.
	@return	A QSize that specifies that part of the display image, in
		display pixel units, that contains the scaled source image
		region. <b>N.B.</b>: If the scaled source image region does not
		cover the entire display image the extent of the area will not
		reach to the right and/or bottom of the display image, but the
		{@link source_origin(int) source image region origin} will always
		be cooincident with the origin of the display image upper-left
		corner.
*/
QSize displayed_size (int band = 0) const;

//	Source to image data mapping -----------------------------------------------

/**	Set one or all source image data maps to another data map.

	A Data_Map is a vector of pixel sample values for one band of the
	display image in source data order that act as a lookup table (LUT)
	mapping source data values to display image values.

	The new Data_Map values will be copied into the existing Data_Map(s).
	If a Data_Map does not exist for a band to be assigned a new map, one
	will be created as a copy of the provided Data_Map.

	<b>N.B.</b>: If the image is {@link closed() closed} nothing is done
	and false is returned.

	@param	data_map	A Data_Map reference. If the data map does not
		have at least {@link source_data_map_size()} entries additional
		zero-valued entries will be added to make up the difference. The
		new values are copied into the existing Data_Map(s).
	@param	band	The display image band to which the Data_Map is to be
		assigned. If the value is negative the map will be assigned to
		all three display image bands.
	@return	true if the any existing data map value was changed.
	@throws invalid_argument if the band number is greater than 2.
*/
virtual bool source_data_map (const Data_Map& data_map, int band = -1);

/**	Get the data map for a display band.

	@param	band	The display image band for which the Data_Map is to be
		obtained.
	@return	A pointer to the Data_Map for the specified display band.
	@throws	invalid_argument if the band number is not valid; there are
		three display bands numbered 0-2.
*/
Data_Map* source_data_map (int band = 0) const;

/**	Set all the source image data maps.

	All current data maps will be replaced with new maps.

	<b>N.B.</b>: If the image is {@link closed() closed} nothing is done
	and false is returned. Also, if the specified data_maps are identical
	to the current data maps, nothing is done.

	If no data maps are specified (data_maps is NULL), and the current
	data maps are {@link local_mappings() locally owned) nothing is done.
	However, if the data maps are currently shared, the shared maps are
	copied to locally owned maps. <b>N.B.</b>: This is how shared data
	maps can be "acquired" locally. In the case where the shared data
	maps are NULL, {@link source_data_map_reset(int) default data maps}
	will be provided locally.

	<b>N.B.</b>: If the new maps are to be shared they will be used in
	place rather than copied.

	@param	data_maps	An array of three Data_Map pointers in Red, Green,
		Blue order. If NULL and the data maps are currently being shared
		they will revert to locally owned maps.
	@param	shared	If true and the data maps are {@link local_mappings()
		locally owned} the local data maps will be deleted and the new
		data maps will be used in place without being copied. In this
		case the data maps will not be marked as locally owned. If false
		each map is {@link source_data_map(const Data_Map&, int) assigned
		to its corresponding band. If the data_maps pointer is NULL, this
		argument is ignored.
	@throws invalid_argument if any of the specified data maps are NULL.
*/
virtual bool source_data_maps (const Data_Map** data_maps, bool shared = false);

/**	Get the current data maps.

	@return	A pointer to the array of three effective data maps in Red,
		Green, Blue order. <b>N.B.</b>: The returned pointer will become
		invalid if new {@link source_data_maps(const Data_Map**, bool)
		shared data maps are assigned} or shared maps are reverted to
		locally owned maps.
*/
inline Data_Map** source_data_maps () const
	{return Data_Maps;}

/**	Reset one or all source image data to display image data maps.

	<b>N.B.</b>: If the image is {@link closed() closed} nothing is done.

	@param	band	The display image band to have its
		{@link source_data_map(int) Data_Map} {@link
		reset_data_map(Data_Map&) reset}. If negative the Data_Maps
		for all display bands will be reset.
*/
bool source_data_map_reset (int band = -1);

/**	Test if the Data_Maps for this image are different than another
	set of Data_Maps.

	Each Data_Map for this image is compared with the corresonding
	Data_Map of the specifed set. The Data_Map corresponding
	entry values of each Data_Map pair are compared.

	@param	data_maps	An array of three Data_Map pointers.
	@return	true if the two sets of Data_Maps have any differences;
		false if both sets contain the same values.
*/
bool different_data_maps (const Data_Map** data_maps) const;

/**	Reset a Data_Map to a linear mapping.

	The linear mapping sets entry 0 to 0 and the last entry (size - 1)
	to 255; all other entry values are proportional to the entry index
	in the map range: value = 255 * (index / size - 1). Thus for 8-bit
	precision source image data each entry value equals its entry index
	(i.e. an identity mapping).

	The values of the Data_Map are changed if, and only if, the existing
	value is not the same as the value to which it would be reset. The
	comparison of values is done with read-only access and the assignment
	is conditional on the values being different to prevent an
	unnecessary copy-on-write; i.e. if the data storage of the Data_Map
	is being implicitly shared with another Data_Map its data storage is
	detached and a deep copy is made only if the values need to be
	changed.

	@return true if any values in the map were changed; false otherwise.
*/
static bool reset_data_map (Data_Map& map);

/**	Assign the values of a Data_Map to another Data_Map.

	Each entry of the from_map is assigned to the corresponding
	entry of the to_map if, and only if, the corresponding values
	differ. The comparison of values is done with read-only access
	and the assignment is conditional on the values being different
	to prevent an unnecessary copy-on-write; i.e. if the data storage
	of the to_map Data_Map is being implicitly shared with another
	Data_Map its data storage is detached and a deep copy is made only
	if the values need to be changed.

	<b>N.B.</b>: A Data_Map may be assigned to another Data_Map using the
	assignment operator (operator=()) which will do an unconditional
	replacement of the data storage reference (a shallow copy) contained
	in the Data_Map which provides fast, implicitly shared data storage
	assignement. This is may be more desirable than conditional
	assignment with a possible deep copy.

	@param	to_map	A reference to the Data_Map to be assigned the
		values from the from_map.
	@param	from_map	A reference to the Data_Map from which values
		will be assigned to the to_map.
	@return	true	If any value of the to_map was changed; false otherwise.
	@throws	invalid_argument	If both the to_map and from_map are not
		the same size.
	@see	apply_data_map(const Data_Map&, const Data_Map&)
*/
static bool assign_data_map (Data_Map& to_map, const Data_Map& from_map);

/**	Apply the values of a Data_Map to another Data_Map.

	The data values from one Data_Map are applied to the data values of
	another Data_Map without affecting the implicitly shared data state.

	<b>N.B.</b>: Any other Data_Map sharing the data storage of the
	to_map will also see its values changed; i.e. if the data storage of
	the of the to_map is being implicitly shared with the data storage of
	another Data_Map the data will remain shared even if any data values
	are changed.

	@param	to_map	A reference to the Data_Map to be assigned the
		values from the from_map. <b>N.B.</b>: This Data_Map is typed
		as const to prevent a deep copy if has shared data, but the
		data content will still be changed to the values of the from_map.
	@param	from_map	A reference to the Data_Map from which values
		will be assigned to the to_map.
	@return	true	If any value of the to_map was changed; false otherwise.
	@throws	invalid_argument	If both the to_map and from_map are not
		the same size.
	@see	assign_data_map(Data_Map&, const Data_Map&)
*/
static bool apply_data_map (const Data_Map& to_map, const Data_Map& from_map);

/**	Get the required size of a Data_Map for this image.

	The required size of a Data_Map is determined by the image's {@link
	source_precision_bits() data precision}: size = 1 << precision.

	@return	The required size of a Data_Map for this image. This will be
		zero if, and only if, the image has no data.
*/
unsigned int source_data_map_size () const;

/**	Test if the {@link source_data_maps() data maps} have any differences
	between them.

	@return A Mapping_Type value that will be {@link #IDENTICAL_MAPPINGS}
		if all of the data maps are equivalent (have the same contents);
		otherwise the value will be {@link #DATA_MAPS} if there any
		differences between the contents of any of the data maps.
*/
inline Mapping_Type different_data_maps () const
	{
	return
		 (Data_Maps &&
		(*Data_Maps[0] != *Data_Maps[1] ||
		 *Data_Maps[0] != *Data_Maps[2])) ?
		DATA_MAPS : IDENTICAL_MAPPINGS;
	}

/**	Get the source-to-display image mapping differences.

	The {@link #Mapping_Dfferences} code identifies differences in the
	geometric transforms and/or data mapping for the image bands that are
	being displayed.

	@return	A Mapping_Type bit field that identifies which mappings sets
		have differences.
*/
inline Mapping_Type mapping_differences () const
	{return Mapping_Differences;}

/**	Identify which mappings are locally owned.

	@return	A Mapping_Type bit field that identifies which mappings are
		locally owned. Mappings that are not locally owned are shared
		with an external entity that owns the corresponding mapping
		data structures.
*/
inline Mapping_Type local_mappings () const
	{return Locally_Owned_and_Operated;}

static QString mapping_type_names (Mapping_Type mapping_type);

//	Background color -----------------------------------------------------------

inline static void default_background_color (QRgb color)
	{Default_Background_Color = color;}
inline static QRgb default_background_color ()
	{return Default_Background_Color;}
inline bool background_color (QRgb color)
	{
	bool
		changed = Background_Color != color;
	Background_Color = color;
	return changed;
	}
inline QRgb background_color () const
	{return Background_Color;}

/**	Fill a single band of the display image from a color value.

	Each display image pixel value has its band index byte replaced with
	the corresponding byte of the specified color (typically the
	{@link background_color() background color}).

	@param	color	A QRgb value from which the band value will be obtained.
	@param	band	A display image band index. If the value is not in
		the allowed range of 0-2 nothing is done.
*/
void fill (QRgb color, int band);

inline void fill (QRgb color)
	{QImage::fill ((uint)color);}

//	Data histograms ------------------------------------------------------------

/**	Produce histograms of source image data.

	A histogram is generated for each source data band {@link
	source_band_map() mapped} into this Plastic_Image. If a source data
	band is mapped into the image more than once the first histogram
	vector generated for the band is simply copied to the histogram
	vector corresponding to the same band, unless the corresponding
	histogram vectors are identical.

	A histogram is produced by scanning the source band pixel data that
	is within the specified image region. Each histogram data vector is
	used to accumulate the count of pixels by pixel value. Thus each
	vector must have at least as many entries as there are possible pixel
	values in the image as determined by the {@link
	source_precision_bits() precision} of the image data. The data
	vectors will be resized if necessary to provide the required number
	of entries. <b>N.B.</b>: The histogram data vector is not initialized
	before being used to accumulate pixel counts; all entries should be
	reset to zero before this method is called if pixel counting is
	expected to start from zero.

	@param	histograms	A vector of three Histogram data vector pointers.
		Any of the pointers may be NULL in which case a Histogram data
		vector will be allocated, and its ownership transferred to the
		caller, as needed.
	@param	source_region	A QRect specifying the region of the source
		image, in unscaled source image pixel units, to be scanned.
		Pixels on the edges of the rectangle are included.
	@return	The maximum number of pixels sampled for each histogram.
		This will be zero if no histogramming was done.
*/
virtual unsigned long long source_histograms (QVector<Histogram*> histograms,
	const QRect& source_region) const;

/**	Produce histograms of display image data.

	@param	histograms	A vector of three Histogram data vector pointers.
		Any of the pointers may be NULL in which case a Histogram data
		vector will be allocated, and its ownership transferred to the
		caller, as needed.
	@param	display_region	A QRect specifying the region of the display
		image, in dispaly image pixel units, to be scanned.
	@return	The number of pixels sampled for each histogram.
*/
virtual unsigned long long display_histograms (QVector<Histogram*> histograms,
	const QRect& display_region) const;

//	Data rendering -------------------------------------------------------------

/**	Render the source image data into the destination display image.

	All {@link needs_update() needed updates} are applied in mapping the
	source data to the destination image. Depending on the requirements
	of the subclass implementation managing the source image data, a
	local cache of source data may need to be refreshed before the
	destination image can be rendered. In the base class implementation
	the rendering is done by mapping the coordinate of each destination
	image pixel back to its source image data coordinate using the {@link
	source_transform(int) geometric transformation matrix} that has been
	invert_keeping_origin(const QTransform&) inverted while keeping the
	orign offset unchanged}. This is used to obtain the {@link
	source_pixel_value(unsigned int, unsigned int, unsigned int) source
	pixel value} for the {@link source_data_map(int) mapped image band}.
	The pixel value is then {@link source_data_map(int) data mapped} to
	its destination image display pixel value. This is done independently
	for each band of the destination image.

	When rendering begins a copy is made of the image configuration
	variables that affect rendering; this includes the needs update
	state. While the image {@link is_rendering() is rendering} the
	configuration of the image may be changed. These changes are
	accumulated in a shadow needs update state that is combined with the
	needs update state that results from the rendering - which is
	expected to be that no update is needed if rendering was successful,
	or the original needs update state if rendering was {@link
	cancel_update(bool) canceled} or failed - to produce a new needs
	update state when rendering ends.

	@return	true if the source data was completely mapped; false if the
		rendering was interrupted for any reason. For example, if the
		Plastic_Image implementation does incremental rendering and the
		response to a {@link add_rendering_monitor(Rendering_Monitor*)
		registered} Rendering_Monitor {@link
		Rendering_Monitor::notification(Fungile_Image&, const QRect&)
		notification} is false then rendering will be stopped at that
		point. Rendering may also be interrupted by a failure condition
		in the rendering engine.
	@throws	Render_Exception	If rendering the image threw this exception.
	@throws	std::bad_exception	If rendering the image threw an exception
		other than Render_Exception.
	@see	update()
*/
virtual bool render_image ()
	throw (Render_Exception, std::bad_exception);

/**	Update the image from its source data if {@link needs_update() needed}.

	The image will be updated by {@link render_image() rendering} the
	image from the source data through the data mapping structures into
	the display data. This will not be done if the image is {@link
	closed() closed}, it currently {@link is_rendering() is rendering},
	or its {@link needs_update() needs update state} is false.

	While rendering is in progress the state of the shadow needs update
	varriable is returned; i.e. true is returned if no changes to the
	configuration have been made since rendering was started; false
	otherwise. <b>N.B></b>: This method may return true while rendering
	is in progess but an update may still be needed after rendering
	ends depending on the outcome of the rendering operation.

	@return	true if the image is updated, either because it does not
		currently need updating or because rendering was successfully
		accomplished, or because rendering is in progress and no new
		changes have been made to the image configuration. Otherwise
		false is returned if the image is closed, rendering is in
		progress and changes have been made to the image configuration,
		or image rendering ended without completing all the updates
		that were needed.
	@throws	Render_Exception	If rendering the image threw this exception.
	@throws	std::bad_exception	If rendering the image threw an exception
		other than Render_Exception.
	@see	render_image()
	@see	needs_update(Mapping_Type)
*/
virtual bool update ()
	throw (Render_Exception, std::bad_exception);

/**	Set the needs update condition.

	Two state flags control when the image data is updated from its
	source data: {@link auto_update() auto update} and {@link
	needs_update() needs update}. Both must be true for this method to
	cause the {@link render_image() image update} to occur.

	First the needs update state is logically OR-ed with the changed
	argument. This argument is true if it has any combination of the
	{@link #BAND_MAP}, {@link #TRANSFORMS} or {@link #DATA_MAPS} flags
	set; any other bits will be ignored. The argument is false if the
	{@link #NO_MAPPINGS} value is used. The needs update becomes true if
	the changed argument is true, and remains true if it was already
	true; it does not become false if the changed argument is false and
	needs update was already true. Then, if both the needs update and
	auto update states are true the data update is done. <b>N.B.</b>: The
	data update will not be done unless the auto update state is true.

	N.B.: Normally a Plastic_Image detects when its {@link
	source_band_map() band mapping}, {@link source_transforms() geometric
	transforms} or {@link source_data_maps() data mapping} have changed
	and sets its needs update condition itself. However, when any of
	these mapping definitions are shared from an external source the
	Plastic_Image sharing them can not detect when they have changed and
	so it must be notified of the change. This method is used to provide
	that notification. Disable {@link auto_update(bool) auto update} when
	the notification is provided, and restore it's previous state after
	notification, if the intent is only to update the needs update state
	without possibly causing an image update operation.

	@param	changed	A Mapping_Type value indicating which, if any,
		mapping definitions have changed. Any combination of the
		{@link #BAND_MAP}, {@link #TRANSFORMS} or {@link #DATA_MAPS} flags
		may be used for a true value; use {@link #NO_MAPPINGS} (or zero or
		false) otherwise.
	@return	true if the image was updated; false otherwise. N.B.: false
		may be returned even when both the needs update and auto update
		states are true if the {@link render_image() image update}
		operation returned false. If the image is {@link closed() closed}
		false will be returned.
	@throws	Render_Exception	If rendering the image threw this exception.
	@throws	std::bad_exception	If rendering the image threw an exception
		other than Render_Exception.
	@see	needs_update()
*/
virtual bool needs_update (Mapping_Type changed)
	throw (Render_Exception, std::bad_exception);

/**	Get the needs update condition.

	@return	A non-zero Mapping_Type value if the image is in need of
		updating from its source data. In this case the value may be
		any combination of the {@link #BAND_MAP}, {@link #TRANSFORMS}
		or {@link #DATA_MAPS} flags. The value will be zero
		({@link #NO_MAPPINGS}) if the image does not seem to be in
		need of updating.
	@see	needs_update(Mapping_Type)
*/
Mapping_Type needs_update () const;

inline static void default_auto_update (bool enable)
	{Default_Auto_Update = enable;}
inline static bool default_auto_update ()
	{return Default_Auto_Update;}

/**	Enable or disable automatic image update.

	When auto-update is enabled and a change is made to any {@link
	mapping_differences() source-to-display image mapping} the
	display image is automatically {@link update() updated}.

	When auto-update is disabled image mapping changes are
	{@link needs_update() accumulated} and will only be applied when
	an image update is requested.

	@param	enable	If true automatic image update is enabled; if false
		automatic image update is disabled. <b>N.B.</b>: Enabling
		automatic image update does not cause an update to occur.
	@return	The previous automatic image update condition.
	@see	auto_update()
*/
inline bool auto_update (bool enable)
	{
	bool
		auto_update = Auto_Update;
	Auto_Update = enable;
	return auto_update;
	}

/**	Get the automatic image update condition.

	@return	The current automatic image update condition.
	@see	auto_update(bool)
*/
inline bool auto_update () const
	{return Auto_Update;}

/**	Cancel an update in progress.

	If an update is in progress it is canelled when the next {@link
	notify_rendering_monitors(Rendering_Monitor::Status, const QString&,
	const QRect&) rendering monitor notification} occurs. This method
	returns immediately without waiting for the canellation to occur.

	This is a thread safe method. <b>N.B.</b>: Using this method to
	test-and-reset the cancel status will avoid any possible race
	condition.

	@param	cancel	If true an update in progress will be canceled; if
		false an update cancellation is cleared so an update in progress
		may proceed (this is expected to be used by subclass
		implementations to clear any canceled update). <b>N.B.</b>: If the
		image is {@link closed() closed} a cancel status can only be
		cleared, not set; i.e. only a false cancel status will be
		accepted if the image is closed.
	@return	The previous cancel status.
	@see	update_canceled ()
*/
bool cancel_update (bool cancel = true);

/**	Test if the last update was canceled.

	This is a thread safe method.

	@return	true if the last update was canceled during the
		{@link render_image() image rendering} process; false
		otherwise.
	@see	cancel_update(bool)
*/
bool update_canceled () const;

/**	Add a Rendering_Monitor for this Plastic_Image.

	<b>N.B.</b>: A monitor can not be added from within the notification
	method of a monitor for this image; this would cause a dead lock
	condition since the list of registered monitors is protected by
	a mutual exclusion lock to ensure thread safety.

	@param	monitor	A pointer to the Rendering_Monitor to add. A NULL
		value is not added.
	@return	true if the monitor was added; false if it was not added
		because it was already registered, the monitor is NULL, or the
		image is {@link closed() closed}.
	@see	remove_rendering_monitor(Rendering_Monitor*)
*/
bool add_rendering_monitor (Rendering_Monitor* monitor);

/**	Remove a Rendering_Monitor for this Plastic_Image.

	<b>N.B.</b>: A monitor can not be removed from within the notification
	method of a monitor for this image; this would cause a dead lock
	condition since the list of registered monitors is protected by
	a mutual exclusion lock to ensure thread safety.

	@param	monitor	A pointer to the Rendering_Monitor to remove.
	@return	true if the monitor was removed; false if it was not
		registered.
	@see	add_rendering_monitor(Rendering_Monitor*)
*/
bool remove_rendering_monitor (Rendering_Monitor* monitor);

protected:
/**	Notify all {@link add_rendering_monitor(Rendering_Monitor*)
	registered} rendering monitors.

	If the image is {@link closed() closed} nothing is done.

	<b>N.B.</b>: This method is only for use by implementation subclasses.

	@param	status	A rendering {@link #Rendering_Monitor::Status} code
		that indicates the context in which the notification is being
		sent.
	@param	message	A message associated with the notification event.
		This may be empty.
	@param	display_region	A QRect describing the region of the display
		image, <b>not the source image</b>, that was just rendered.
		<b>N.B.</b>: The region may be empty if no new rendered data
		is available, but may yet become available with future
		notifications.
	@return	true if rendering is to continue; false if rendering is to be
		stopped. <b>N.B.</b>: true will be return if, only if, all
		Rendering_Monitor {@link
		Rendering_Monitor::notification(Plastic_Image&, const QRect&)
		notifications} respond true and rendering has not been {@link
		cancel_update(bool) canceled} during the rendering process.
	@see	add_rendering_monitor(Rendering_Monitor*)
*/
bool notify_rendering_monitors (Rendering_Monitor::Status status,
	const QString& message, const QRect& display_region);

public:

/**	Set the default rendering increment.

	@param	rendering_increment	The default incremental rendering lines
		for new Plastic_Images.
	@see	rendering_increment_lines(unsigned int)
*/
inline static void default_rendering_increment_lines
	(unsigned int rendering_increment)
	{Default_Rendering_Increment_Lines = rendering_increment;}

/**	Get the default rendering increment.

	@return	The default incremental rendering lines for new
		Plastic_Images.
	@see	default_rendering_increment_lines(unsigned int)
*/
inline static unsigned int default_rendering_increment_lines ()
	{return Default_Rendering_Increment_Lines;}

/**	Set the suggested rendering increment.

	The specified number of rendering increment lines is a suggestion to
	the Plastic_Image implementation of how many lines of the display
	image (not the source data image} to render before providing a {@link
	Rendering_Monitor::notification(Plastic_Image&, const QRect&)
	notification} to all {@link add_rendering_monitor(Rendering_Monitor*)
	registered} rendering monitors. If the value is zero the underlying
	source data rendering implementation may still incrementally render
	the pixel data (this is particularly true for JP2 rendering), but
	rendering notifications will only be sent at the completion of
	rendering. Having nofications sent often increases the opportunities
	for the implmentation to notice that rendering has been canceled and
	thus prevent a highly interactive application from having to wait on
	cancellation longer than might be desired.

	<b>N.B.</b>: The rendering increment is a <b>suggestion</b> to the
	implementation, which may render more or less lines - but
	always whole lines - for each rendering increment.

	@param	rendering_increment	The number of display image lines to
		incrementally render that will be suggested to the rendering
		implementation.
	@see	rendering_increment_lines()
*/
virtual void rendering_increment_lines (unsigned int rendering_increment);

/**	Get the suggested rendering increment.

	@return	The number of display image lines to incrementally render.
		This may be zero if there is no suggested rendering increment.
	@see	rendering_increment_lines(unsigned int)
*/
virtual unsigned int rendering_increment_lines () const;

/**	Test if the image source data is currently being rendered.

	@return	true if {@link render_image() rendering} is in progress;
		false otherwise.
	@see	is_rendering(bool)
*/
bool is_rendering () const;

/*==============================================================================
	Helpers
*/
protected:

/**	Initialize the object data structures.

	Each implementing subclass is expected to initialize, during object
	construction, the object data structures of the Plastic_Image. Any
	data structures of the subclass may also be initialized at this time.

	When the Plastic_Image is initialized the band map, source data maps
	and geometric transforms are reset to their default states if the
	corresponding data structure is {@link local_mappings() locally
	owned} or does not yet exist. This will set the {@link needs_update{}
	update needed state}. If, however, the display image is empty then
	the update needed state is set to NO_MAPPINGS.

	<b>N.B.</b>: {@link auto_update(bool) auto-update} is disabled while
	the data structures are being initialized; then the auto-update mode
	is restored and, if it was enabled, the image is {@link update()
	updated}.
*/
virtual void initialize ();

/**	Set or reset the rendering state.

	If rendering is true a new Rendering_Data structure, Rendering, is
	constructed (if one does not already exist). This will make a
	copy of all the data members of this Plastic_Image that are used in
	source data rendering.

	If rendering is false and Rendering is non-NULL the {@link
	needs_update() needs update} state is set to the shadow needs update
	state variable used during rendering ORed with the needs update state
	that resulted from the rendering in the Rendering variables. Then the
	Rendering configuration variables are deleted and Rendering is set to
	NULL.

	<b>N.B.</b>: Rendering is used as the indication that {@link
	is_rendering() rendering is in progress}; i.e. when Rendering is NULL
	rendering is not in progress.

	@param	rendering	true if rendering is starting and a copy of the
		Plastic_Image rendering parameters (Rendering) is to be made;
		false if rendering is ending and any current Rendering_Data is to
		be deleted and Rendering reset to NULL.
	@return	true if rendering was in progress when the method was called.
		If the rendering argument is true and the return value is true
		then rendering is already in progress and should not be restarted
		again until the method returns false. If the rendering argument is
		false and the return value is false, then nothing was done.
*/
virtual bool is_rendering (bool rendering);

/*	Get a pointer to the image data.

	The QImage class employs implicit sharing of its data storage. This
	can result in a data race condition when the shared data is being
	used in one thread when the image data is to be written in a separate
	thread.

	When a QImage is copied, via the copy constructor or assignment
	operator, a shallow copy is made that only increments the shared data
	reference counter (the reference counter is decremented when a QImage
	is destroyed). Normally, when the shared image data storage is to be
	changed the the non-const image data address is obtained using a
	QImage method  (scanLine or bits) that will detatch the shared data
	storage if its reference counter is greater than 1. Detatching the
	shared data storage involves making a copy of the data storage that
	is then assigned to the object having its storage detached. The
	assignment (operator=) will decrement the reference counter of the
	shared data storage and delete the current data storage if its
	reference counter has dropped to zero. This is the point at which the
	race condition occurs: Between the point at which the reference
	counter is decremented and the deletion of the data starts another
	thread may acquire the address of the data for its own use, but the
	data will be deleted while the data is in use.

	One way to avoid this situation is to use a lock accessible to both
	threads that will ensure the data is only deleted while the data lock
	is being held. This can be rather awkward.

	However, incrementally rendering source data into the sections of the
	display data owned by the QImage is expected to occur while other,
	already rendered, sections of the data are being used by the GUI
	thread for painting to the display screen (in a paintEvent). In this
	case the shared data storage is not to be detached.

	This method obtains the address of the shared image data storage, as
	a non-const pointer, without triggering a storage detach operation.
	This enables sections of the data to be read in one thread while,
	probably other sections, are simultaneously being written by
	Plastic_Image subclasses that are incrementally rendering or mapping
	source data.

	@return	A QRgb pointer to the base QImage display data storage that
		can be written.
*/
QRgb* image_data () const;

/**	Check one histogram of a set to see if it is to be refreshed.

	This method is expected to be used inside a by-display band loop to
	check that the histogram associated with the display band needs to be
	refreshed before the refresh is done.

	The {@link source_band_map() source band map is checked for a display
	band less than the specified display band that is mapped from the
	same source band. If a match is found the histogram does not need to
	be refreshed; i.e. the histogram has already been refreshed from the
	same source data. In this case, if the histogram pointer for the
	display band is NULL it assigned the histogram pointer for the
	matching band; a null-NULL matching histogram has its data values
	replaced with values (after being resized, if needed to accommodate
	all the required entries) copied from the matching band histogram.

	When the no matching source band mapping is found for the specified
	band (note that this will always be the case for band zero), the
	histogram is expected to need a refresh. The histogram is validated:
	A NULL histogram is provided with a new Histogram sized to the {@link
	source_precision_bits() source data precision}; other wise it is
	resized to accommodate all the required entries for the source data
	precision.

	@param	histograms	A reference to the histogram set: a QVector of
		three Histogram pointers any of which may be NULL.
	@param	band	The display band number of the histogram in the set
		to be checked.
	@return	true if the histogram is to be refreshed; false if
		the histogram band number is mapped to a lower numbered band and
		thus contains the identical data.
*/
bool refresh_source_histogram (QVector<Histogram*>& histograms, int band) const;

/*==============================================================================
	Utilities
*/
public:

inline static unsigned int bits_bytes (unsigned int bits)
	{return (bits >> 3) + ((bits % 8) ? 1 : 0);}

inline static unsigned int bits_mask (unsigned int bits)
	{
	unsigned int
		mask = 0;
	while (bits--)
		(mask <<= 1) |= 1;
	return mask;
	}

/**	Invert a transformation matrix while retaining the origin offset.

	A copy of the transform with the origin offset removed is inverted
	and then the origin offset is restored.

	The {@link source_transform(int) transformation matrix for a source
	image band} is a mapping from source image data space to destination
	image display space. When {@link render_data() rendering} the source
	image data to the destination display image each destination pixel is
	mapped back to its source image data coordinate. This is done by
	using the source-to-destination transformation matrix inverted to a
	destination-to-source transformation matrix but with the image origin
	offset first set to zero and then finally restored unchanged.

	@param	transform	The QTransform to be inverted.
	@return	A QTransform that is the transform inverted without any origin
		offset translation and then the original orgin offset restored.
	@throws runtime_error	If the transform matrix could not be inverted.
	@see invert_clipping_origin(const QTransform&)
*/
static QTransform invert_keeping_origin (const QTransform& transfrom);

/**	Invert a transformation matrix while clipping the origin offset
	to its fractional part.

	The geometric transformation matrix is inverted in the same way as
	when this is done while retaining the original origin offset.
	However, in this case instead of restoring the original origin offset
	it is clipped to only the fraction part. This is used by subclass
	implementations that dynamically obtain source data from a file for
	only that portion covering the region starting at the image origin
	offset (e.g. dynamic tile data acquisistion). In this case the origin
	offset is only that part carried over from any previous (left or
	above) region.

	@param	transform	The QTransform to be inverted.
	@return	A QTransform that is the transform inverted without any origin
		offset translation and then the original orgin offset restored.
	@throws runtime_error	If the transform matrix could not be inverted.
	@see invert_keeping_origin(const QTransform&)
*/
static QTransform invert_clipping_origin (const QTransform& transfrom);

private:

inline static const char* plural (unsigned int amount)
	{return (amount == 1) ? "" : "s";}

/*==============================================================================
	Data
*/
protected:

//!	Lock for all object data. Constructed as a recursive mutex.
mutable QMutex
	Object_Lock;

//!	The image metadata parameters.
mutable idaeim::PVL::Aggregate
	*Metadata;
//!	The {@link #IMAGE_METADATA_GROUP} in the Metadata.
mutable idaeim::PVL::Aggregate
	*Image_Metadata;
//!	Registered Metadata_Monitors.
QList<Metadata_Monitor*>
	Metadata_Monitors;


/**	The <i>Update_Locker</i> locks the Object_Lock when an update
	sequence starts and does not unlock the Object_Lock until the update
	sequence ends.
*/
class Update_Locker
{
public:

Update_Locker (const Plastic_Image& image);
Update_Locker ();
~Update_Locker ();

/**	Start an update sequence.
*/
void start ();

enum Update_End_Condition
	{
	SEQUENCE_END,
	LEVEL_DEPENDENT
	};

/**	Consider ending an update sequence.
*/
void end (Update_End_Condition condition = LEVEL_DEPENDENT);

private:

/*	Guard lock for the Update_Locker settings.
	Not to be confused with locking the update sequence.
*/
QMutex
	Update_Lock;

//!	Pointer to the Plastic_Image Object_Lock to be managed.
QMutex
	*Object_Lock;

/**	Flag that an update sequence is in progress.

	To avoid race conditions the Object_Lock needs to remain locked from
	before rendering variables are changed through (or starting with)
	update_needed (or starting with update) and possibly into
	render_image. But the lock is to be released while rendering is
	occuring in render_image, or when the update sequence is completed in
	the method where it started. This flag is used to indicate in an
	update sequence that the Object_Lock is already locked, as is not to
	be (recursively) locked, so it can be finally unlocked either in
	render_image when rendering begins or back at the point where the
	update sequence started if the sequence does not result in rendering.

	Each time start is called Updating is incremented. Each time end is
	called Updating is decremented. The update sequence ends when
	Updating becomes zero.
*/
int
	Updating;

//	The ID of the QThread in which the update sequence started.
void
	*Initiator_Thread;
};	//	class Update_Locker

Update_Locker
	Update;


//!	The name (typically a pathname or URL) of the source file.
QString
	Source_Name;

//!	Automatically map source to image when mapping changes.
mutable bool
	Auto_Update;

/*------------------------------------------------------------------------------
	Rendering configuration variables.
*/
//!	Source data band by display image band.
unsigned int
	*Band_Map;

//!	Geometric transformation of source coordinates to image coordinates.
QTransform
	**Geo_Transforms;

/**	Maps source pixel sample data values to display image pixel byte values.

	Each Data_Map is a LUT with one entry for each possible source image
	data value, determined by the {@link source_data_map_size()}. Each
	entry is an unsigned 8-bit value that specifies the display data value
	for the source data value corresponding to the entry index. Three
	Data_Maps are provided that map to the red (0), green (1) and blue (2)
	image display bands.
*/
Data_Map
	**Data_Maps;

//!	Which maps are locally allocated.
Mapping_Type
	Locally_Owned_and_Operated;

//!	Optimization flags for when mapping is different from band to band.
Mapping_Type
	Mapping_Differences;

//!	Mapping has changed and an image update is needed.
Mapping_Type
	Needs_Update,
	Needs_Update_Shadow;

//!	Suggested number of output image lines in each rendering increment.
unsigned int
	Rendering_Increment_Lines;

//!	The image value used when no source data is applied.
QRgb
	Background_Color;

//!	Image source data access has been closed flag.
bool
	Closed;

// Data variables used in rendering copied from the Plastic_Image.
struct Rendering_Data
{
Rendering_Data (const Plastic_Image& image);
virtual ~Rendering_Data ();

QPointF origin (int band = 0) const;
QSizeF scaling (int band = 0) const;

unsigned int
	*Band_Map;
QTransform
	**Geo_Transforms;
Data_Map
	**Data_Maps;
Mapping_Type
	Locally_Owned_and_Operated,
	Mapping_Differences;
Mapping_Type
	Needs_Update;
QRgb
	Background_Color;
};

Rendering_Data
	*Rendering;


private:

//!	Registered Rendering_Monitors.
QList<Rendering_Monitor*>
	Rendering_Monitors;
mutable QMutex
	Rendering_Monitors_Lock;
bool
	Cancel_Update;

};	//	Class Plastic_Image

/*==============================================================================
	Utility Functions
*/
std::ostream& operator<< (std::ostream& stream,
	const Plastic_Image& image);

std::ostream& operator<< (std::ostream& stream,
	const Plastic_Image::Triplet& triplet);

void display_image_data (const Plastic_Image& image);
void display_line_data (const Plastic_Image& image, int line);

std::ostream& operator<< (std::ostream& stream,
	const QTransform& transform);

std::ostream& operator<< (std::ostream& stream,
	const Plastic_Image::Histogram* const histogram);

std::ostream& operator<< (std::ostream& stream,
	const unsigned int*	band_map);

void mark_image (Plastic_Image* image, const QString& label,
	int top, const QColor& text_color, const QColor& field_color = QColor ());


}	//	namespace UA::HiRISE
#endif
