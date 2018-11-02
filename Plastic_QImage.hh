/*	Plastic_QImage

HiROC CVS ID: $Id: Plastic_QImage.hh,v 1.13 2012/06/15 01:16:07 castalia Exp $

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

#ifndef HiView_Plastic_QImage_hh
#define HiView_Plastic_QImage_hh

#include	"Plastic_Image.hh"


namespace UA
{
namespace HiRISE
{
class Plastic_QImage
:	public Plastic_Image
{
public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Constructors
*/
/**	Construct a Plastic_QImage from a pointer to a QImage.

	<b>N.B.</b>: Ownership of the QImage is transfered to the
	Plastic_QImage; i.e. the QImage must have been constructed on the
	heap.

	@param	image	A pointer to a QImage that will provide the source
		image for this Plastic_Image. If NULL an empty QImage will be
		provided that will have zero-valued parameters and no image
		data.
	@param	size	The size of the new Plastic_QImage. If the size is
		not valid the size of the QImage, if provided, will be used.
	@param	band_map	A pointer to unsigned int array of three values
		that will be used as the shared band map for this
		Plastic_Image. If NULL band map sharing is not enabled and a
		default band map will be provided.
	@param	transforms	An array of QTransform pointers, one for each
		band of the image, that will be used as the shared geometric
		transforms for this Plastic_Image. If NULL geometric transforms
		sharing is not enabled and default identity transforms will be
		provided.
	@param	data_maps	An array of data map arrays, one for each band
		of the image, that will be used as the shared data maps for this
		Plastic_Image. If NULL data map sharing is not enabled and
		default identity data maps will be provided.
*/
explicit Plastic_QImage (QImage* image = NULL,
	const QSize&		size = QSize (),
	const unsigned int*	band_map = NULL,
	const QTransform**	transforms = NULL,
	const Data_Map**	data_maps = NULL);

/**	Construct a Plastic_QImage from a QImage.

	@param	image	A QImage that will be copied to provide the source
		image for this Plastic_Image. If the image is null a new
		empty QImage will be provided.
	@param	size	The size of the new Plastic_QImage. If the size is
		not valid the size of the QImage will be used.
	@param	band_map	A pointer to unsigned int array of three values
		that will be used as the shared band map for this
		Plastic_Image. If NULL band map sharing is not enabled and a
		default band map will be provided.
	@param	transforms	An array of QTransform pointers, one for each
		band of the image, that will be used as the shared geometric
		transforms for this Plastic_Image. If NULL geometric transforms
		sharing is not enabled and default identity transforms will be
		provided.
	@param	data_maps	An array of data map arrays, one for each band
		of the image, that will be used as the shared data maps for this
		Plastic_Image. If NULL data map sharing is not enabled and
		default identity data maps will be provided.
*/
explicit Plastic_QImage (const QImage& image,
	const QSize&		size = QSize (),
	const unsigned int*	band_map = NULL,
	const QTransform**	transforms = NULL,
	const Data_Map**	data_maps = NULL);

/**	Copy a Plastic_QImage.

	The image {@link source() source} - a QImage - is copied. However, if
	the image has no source an empty source is provided. The image {@link
	source_transforms() geometric transforms}, {@link source_band_map()
	band map} and {@link source_data_maps() data maps} are copied unless
	the corresponding shared mappings flag inidicates they are to be
	shared with the image being copied. The {@link background_color()
	background color} is also copied.

	<b>Warning</b>: If any data mappings are shared the Plastic_QImage
	being copied must remain valid as long as this Plastic_QImage is
	in use.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image clone will not have been rendered, but will be filled
	with the {@link background_color() background color}; in this case
	the image should be  either {@link render_image() rendered} or {@link
	update() updated}.

	@param	image	The Plastic_QImage to be copied. If the image does
		not have a {@link source() source} image an empty QImage will be
		provided for the source of this Plastic_QImage.
	@param	size	The size of the new Plastic_QImage. If the size is
		not valid the size of the Plastic_QImage being copied (not its
		source image) will be used.
	@param	shared_mappings	A Mapping_Type that specifies any combination
		of {@link #BAND_MAP}, {@link #TRANSFORMS} or {@link #DATA_MAPS} -
		or {@link #NO_MAPPINGS} - data mappings that are to be shared with
		the Plastic_QImage being copied.
*/
explicit Plastic_QImage (const Plastic_QImage& image,
	const QSize&	size = QSize (),
	Mapping_Type	shared_mappings = NO_MAPPINGS);

/**	Destroy this Plastic_QImage.

	The {@link source() source} QImage is deleted.
*/
virtual ~Plastic_QImage ();

/*==============================================================================
	Accessors
*/
/**	Clone this Plastic_QImage.

	A {@link
	Plastic_QImage(const Plastic_QImage&, const QSize& Mapping_Type)
	copy} of this Plastic_QImage is constructed.

	<b>N.B.</b>: If {@link default_auto_update() default auto-update} is
	false the image clone will not have been rendered, but will be filled
	with the {@link background_color() background color}; in this case
	the image should be  either {@link render_image() rendered} or {@link
	update() updated}.

	@param	size	The size of the new Plastic_QImage. If the size is
		not valid the size of this Plastic_QImage being copied (not its
		source image) will be used.
	@param	shared_mappings	A Mapping_Type that specifies any combination
		of {@link #BAND_MAP}, {@link #TRANSFORMS} or {@link #DATA_MAPS} -
		or {@link #NO_MAPPINGS} - data mappings that are to be shared with
		this Plastic_QImage being copied.
	@return	A pointer to a Plastic_QImage.
*/
virtual Plastic_QImage* clone (const QSize& size = QSize (),
	Mapping_Type shared_mappings = NO_MAPPINGS) const;

virtual const void* source () const;

virtual QSize source_size () const;

virtual unsigned int source_bands () const;

virtual unsigned int source_precision_bits () const;

virtual Pixel_Datum source_pixel_value
	(unsigned int x, unsigned int y, unsigned int band) const;
virtual Plastic_Image::Triplet source_pixel (const QPoint& point) const;

/*==============================================================================
	Data
*/
protected:

QImage
	*Source;

};	//	Class Plastic_QImage


}	//	namespace HiRISE
}	//	namespace UA
#endif
