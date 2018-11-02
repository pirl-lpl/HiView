/*	Save_Image_Dialog

HiROC CVS ID: $Id: Save_Image_Dialog.hh,v 1.4 2011/12/24 00:48:26 castalia Exp $

Copyright (C) 2011  Arizona Board of Regents on behalf of the
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

#ifndef HiView_Save_Image_Dialog_hh
#define HiView_Save_Image_Dialog_hh

#include	<QFileDialog>

class QLabel;
class QSpinBox;
class QDoubleSpinBox;


namespace UA
{
namespace HiRISE
{
//	Forward reference.
class Image_Viewer;

/**	A <i>Save_Image_Dialog</i> is a QFileDialog subclass that adds image
	size controls.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.4 $
*/
class Save_Image_Dialog
:	public QFileDialog
{
//	Qt Object declaration.
Q_OBJECT

public:
/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;

/*==============================================================================
	Defaults
*/
//!	The default directory pathname.
static QString
	Default_Directory;

//!	The default image file format.
static QString
	Default_Image_Format;

/*==============================================================================
	Constructor
*/
/**	Construct a Save_Image_Dialog for the display of an Image_Viewer.

	<b>N.B.</b>: The dialog should be {@link reset() reset} before
	each use to ensure that its Image Size panel is synchronized with
	the current Image_Viewer state.

	@param	image_viewer	The Image_Viewer from which the currently
		displayed image region is to be saved.
	@param	parent	The parent widget over which the dialog should be
		shown. May be NULL.
*/
explicit Save_Image_Dialog (const Image_Viewer& image_viewer,
	QWidget* parent = NULL);

/*==============================================================================
	Accessors
*/
/**	Get the pathname for the saved image.

	@return	A QString specifying the pathname for the saved image.
*/
inline QString pathname () const
	{return Pathname;}

/**	Get the name of the file format for the saved image.

	The name of the file format is the same as the default filename
	extension to be used in the {@link pathname() saved file pathname}.

	@return A QString specifying the name of the file format for the
		saved image.
*/
inline QString image_format () const
	{return Image_Format;}

/**	Set the name of the file format for the saved image.

	@param	format	A QString specifying the name of the file format for the
		saved image.
	@return	true if the specified format was selected; false if the format
		name is empty or was not found in a case insensitive search of the
		{@link HiView_Utilities::image_writer_formats() list of all known
		file formats that can be written}.
*/
bool image_format (const QString& format);

/**	Get the size of the saved image.

	@return	The size of the saved image.
	@see	image_scale()
*/
QSize image_size () const;

/**	Get the source image scaling factor for the saved image.

	@return	The scaling factor to be applied to the source image
		to produce the saved image at the appropriate (@link image_size()
		image size}.
*/
double image_scale () const;

/*==============================================================================
	Manipulators
*/
/**	Reset the Image Size controls.

	The current state of the Image_Viewer is used to reset the image
	width, height and scale controls. A thumbnail of the currently
	displayed image region is also included.

	<b>N.B.</b>: If the rendering is in progress for the Image_Viewer
	no thumbnail will be generated - a white rectangle will be provided
	in its place - and an information message will be shown noting this
	condition. If a thumbnail image could not be generated an information
	message indicating the problem is shown and the empty thumbnail will
	be used.

	@return	true if the Image Size controls were reset; false if the
		Image_Viewer is found to have rendering in progress or the
		thumbnail image could not be generated.
*/
bool reset ();

/*==============================================================================
	Slots
*/
public slots:

/**	Set the minimum allowed image scaling factor.

	@param	scale	The minimum allowed image scaling factor.
*/
void min_scale (double scale);

/**	Set the maximum allowed image scaling factor.

	@param	scale	The maximum allowed image scaling factor.
*/
void max_scale (double scale);


private slots:

void file_selected (const QString& pathname);
void filter_selected (const QString& format);
void image_width (int width);
void image_height (int height);
void image_scale (double scale);

/*==============================================================================
	Data
*/
private:

const Image_Viewer
	*Image_View;

QString
	Pathname,
	Image_Format;

QSizeF
	Displayed_Image_Size;

QLabel
	*Thumbnail;
QSpinBox
	*Image_Width,
	*Image_Height;
QDoubleSpinBox
	*Image_Scale;
QLabel
	*Max_Scale;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
