/*	Save_Image_Dialog

HiROC CVS ID: $Id: Save_Image_Dialog.cc,v 1.11 2012/06/15 01:16:07 castalia Exp $

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

#include	"Save_Image_Dialog.hh"

#include	"HiView_Config.hh"
#include	"HiView_Utilities.hh"
#include	"Image_Viewer.hh"
#include	"Plastic_Image_Factory.hh"
#include	"Drawn_Line.hh"

#include	<QFrame>
#include	<QGridLayout>
#include	<QLabel>
#include	<QSpinBox>
#include	<QDoubleSpinBox>
#include	<QFileInfo>
#include	<QDir>
#include	<QMessageBox>

//	For INT_MAX.
#include	<limits.h>


#if defined (DEBUG_SECTION)
/*	DEBUG_SECTION controls

	DEBUG_SECTION report selection options.
	Define any of the following options to obtain the desired debug reports:
*/
#define DEBUG_OFF				0
#define DEBUG_ALL				-1
#define DEBUG_CONSTRUCTORS		(1 << 0)
#define DEBUG_RESET				(1 << 1)
#define DEBUG_SLOTS				(1 << 2)
#define	DEBUG_SIZE				(1 << 3)

#define DEBUG_DEFAULT	DEBUG_ALL

#if (DEBUG_SECTION +0) == 0
#undef  DEBUG_SECTION
#define DEBUG_SECTION DEBUG_OFF
#endif

#include	<iostream>
#include	<iomanip>
using std::clog;
using std::endl;
using std::boolalpha;
#endif	//	DEBUG_SECTION


namespace UA
{
namespace HiRISE
{
/*==============================================================================
	Constants
*/
const char* const
	Save_Image_Dialog::ID =
		"UA::HiRISE::Save_Image_Dialog ($Revision: 1.11 $ $Date: 2012/06/15 01:16:07 $)";

/*==============================================================================
	Defaults
*/
#ifndef AS_STRING
/*	Provides stringification of #defined names.

	Note: The extra double quotes are for MSVC which fails to stringify
	__VA_ARGS__ if its value is empty (STRINGIFIED has no argument).
	In this case the double quotes coalesce into the intended empty
	string constant; otherwise they have no effect on the string generated.
*/
#define STRINGIFIED(...)			"" #__VA_ARGS__ ""
#define AS_STRING(...)				STRINGIFIED(__VA_ARGS__)
#endif

#ifndef DEFAULT_SAVE_IMAGE_FORMAT
#define DEFAULT_SAVE_IMAGE_FORMAT	PNG
#endif
#define _DEFAULT_SAVE_IMAGE_FORMAT_	AS_STRING(DEFAULT_SAVE_IMAGE_FORMAT)
QString
	Save_Image_Dialog::Default_Image_Format (_DEFAULT_SAVE_IMAGE_FORMAT_);

QString
	Save_Image_Dialog::Default_Directory;

/*==============================================================================
	Application configuration parameters
*/
#define Heading_Line_Weight \
	HiView_Config::Heading_Line_Weight
#define Label_Frame_Style \
	HiView_Config::Label_Frame_Style
#define Label_Frame_Width \
	HiView_Config::Label_Frame_Width

/*==============================================================================
	Local variables.
*/
namespace
{
QSize
	Thumbnail_Max_Size;
}

/*==============================================================================
	Constructor
*/
Save_Image_Dialog::Save_Image_Dialog
	(
	const Image_Viewer&	image_viewer,
	QWidget*			parent
	)
	:	QFileDialog (parent, tr ("Save Image"), Default_Directory,
			HiView_Utilities::image_writer_formats_file_filters ()),
		Image_View (&image_viewer),
		Thumbnail (NULL)
{
setObjectName ("Save_Image_Dialog");
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << ">>> Save_Image_Dialog" << endl;
#endif
setAcceptMode (QFileDialog::AcceptSave);
setFileMode (QFileDialog::AnyFile);
setOptions
	(QFileDialog::DontUseNativeDialog |
	 QFileDialog::DontResolveSymlinks);
setConfirmOverwrite (true);

if (! image_format (Default_Image_Format))
	image_format (HiView_Utilities::image_writer_formats ().first ());

connect (this,
	SIGNAL (filterSelected (const QString&)),
	SLOT (filter_selected (const QString&)));
connect (this,
	SIGNAL (fileSelected (const QString&)),
	SLOT (file_selected (const QString&)));

/*==============================================================================
	Add-on panel
*/
QLayout
	*layout = QFileDialog::layout ();
QGridLayout
	*dialog_grid_layout = dynamic_cast<QGridLayout*>(layout);
if (dialog_grid_layout)
	{
	Displayed_Image_Size = Image_View->displayed_image_region ().size ();
	QSize
		dialog_size (QFileDialog::sizeHint ());
	Thumbnail_Max_Size = dialog_size / 2;

	Drawn_Line
		*line;
	QWidget
		*panel = new QWidget;
	QGridLayout
		*grid_layout = new QGridLayout (panel);
	int
		row = -1;

	//	Heading.
	dialog_grid_layout->addWidget (line = new Drawn_Line (Heading_Line_Weight),
		dialog_grid_layout->rowCount (), 0, 1, -1, Qt::AlignBottom);
	line->alignment (Qt::AlignBottom);
	dialog_size.rheight () += Heading_Line_Weight;
	++row;
	grid_layout->addWidget (new QLabel (tr ("<b>Image Size</b>")),
		row, 0, Qt::AlignLeft | Qt::AlignVCenter);

	//	Width.
	++row;
	grid_layout->addWidget (new QLabel (tr ("Width:")),
		row, 1, Qt::AlignRight | Qt::AlignVCenter);
	grid_layout->addWidget (Image_Width = new QSpinBox,
		row, 2, Qt::AlignLeft | Qt::AlignVCenter);

	//	Height.
	++row;
	grid_layout->addWidget (new QLabel (tr ("Height:")),
		row, 0, -1, 1, Qt::AlignRight | Qt::AlignVCenter);
	grid_layout->addWidget (Image_Height = new QSpinBox,
		row, 1, -1, 1, Qt::AlignLeft | Qt::AlignVCenter);

	//	Scale.
	grid_layout->addWidget (new QLabel (tr ("Scale:")),
		row, 3, Qt::AlignRight | Qt::AlignVCenter);
	grid_layout->addWidget (Image_Scale = new QDoubleSpinBox,
		row, 4, Qt::AlignRight | Qt::AlignVCenter);
	Image_Scale->setDecimals (4);
	Image_Scale->setRange (0.0001, 100.0);
	Image_Scale->setSingleStep (0.01);
	grid_layout->addWidget (new QLabel (tr ("Max:")),
		row + 1, 3, Qt::AlignRight | Qt::AlignVCenter);
	grid_layout->addWidget (Max_Scale = new QLabel ("    "),
		row + 1, 4, Qt::AlignLeft | Qt::AlignVCenter);
	Max_Scale->setToolTip
		(tr ("Max scale at which the image will fit in memory."));
	//	Padding.
	grid_layout->setRowStretch (row + 2, 100);

	//	Reset the size control values, which sets the Thumbnail.
	reset ();

	//	Thumbnail.
	Thumbnail->setFrameStyle (Label_Frame_Style);
	Thumbnail->setLineWidth (Label_Frame_Width);
	Thumbnail->setMargin (0);
	grid_layout->addWidget (Thumbnail,
		row, 2, -1, 1, Qt::AlignCenter);

	//	Padding.
	grid_layout->setColumnMinimumWidth (5, 10);
	grid_layout->setColumnStretch (5, 100);

	dialog_grid_layout->addWidget (panel,
		dialog_grid_layout->rowCount (), 0, 1, -1,
			Qt::AlignLeft | Qt::AlignVCenter);

	//	Reset the dialog size.
	dialog_size.rheight () += panel->sizeHint ().height ();
	resize (dialog_size);

	//	Connect the controls to the slots.
	connect (Image_Width,
		SIGNAL (valueChanged (int)),
		SLOT (image_width (int)));
	connect (Image_Height,
		SIGNAL (valueChanged (int)),
		SLOT (image_height (int)));
	connect (Image_Scale,
		SIGNAL (valueChanged (double)),
		SLOT (image_scale (double)));
	}
#if ((DEBUG_SECTION) & DEBUG_CONSTRUCTORS)
clog << "<<< Save_Image_Dialog" << endl;
#endif
}

/*==============================================================================
	Accessors
*/
bool
Save_Image_Dialog::image_format
	(
	const QString&	format
	)
{
bool
	selected = false;
if (! format.isEmpty () &&
	HiView_Utilities::image_writer_formats ()
		.contains (format, Qt::CaseInsensitive))
	{
	Image_Format = format;
	selectNameFilter (HiView_Utilities::file_filter_for (Image_Format));
	setDefaultSuffix (Image_Format);
	selected = true;
	}
return selected;
}


QSize
Save_Image_Dialog::image_size () const
{return QSize (Image_Width->value (), Image_Height->value ());}


double
Save_Image_Dialog::image_scale () const
{return Image_Scale->value ();}

/*==============================================================================
	Manipulators
*/
bool
Save_Image_Dialog::reset ()
{
#if ((DEBUG_SECTION) & DEBUG_RESET)
clog << ">>> Save_Image_Dialog::reset" << endl;
#endif
Displayed_Image_Size = Image_View->displayed_image_region ().size ();
QSize
	image_display_size (Image_View->image_display_region ().size ());

/*	QImage size limitation.

	A QImage size is limited by the precision of an int.
	If this changes (if QImage is enhanced to use size_t) this calculation
	of the maximum scale factor will need to change accordingly.
*/

if (Displayed_Image_Size.isEmpty ())
	Image_Scale->setMaximum (1.0);
else
	Image_Scale->setMaximum
		(INT_MAX
		/ (Displayed_Image_Size.rwidth () * Displayed_Image_Size.rheight ()
			* sizeof (int)));
Max_Scale->setNum (round_to (Image_Scale->maximum (), 4));
#if ((DEBUG_SECTION) & DEBUG_RESET)
clog << "    Displayed_Image_Size = " << Displayed_Image_Size << endl
	 << "      image_display_size = " << image_display_size << endl
	 << "               max scale = " << Image_Scale->maximum () << endl;
#endif
bool
	enabled;
enabled = Image_Width->blockSignals (true);
Image_Width->setMaximum ((int)
	((Image_Scale->maximum () * Displayed_Image_Size.rwidth ()) + 0.5));
Image_Width->setMinimum ((int)
	((Image_Scale->minimum () * Displayed_Image_Size.rwidth ()) - 0.5));
Image_Width->setValue (image_display_size.rwidth ());
Image_Width->blockSignals (enabled);

enabled = Image_Height->blockSignals (true);
Image_Height->setMaximum ((int)
	((Image_Scale->maximum () * Displayed_Image_Size.rheight ()) + 0.5));
Image_Height->setMinimum ((int)
	((Image_Scale->minimum () * Displayed_Image_Size.rheight ()) - 0.5));
Image_Height->setValue (image_display_size.rheight ());
Image_Height->blockSignals (enabled);

QSizeF
	scaling (Image_View->image_scaling ());
#if ((DEBUG_SECTION) & DEBUG_RESET)
clog << "                 scaling = " << scaling << endl;
#endif
enabled = Image_Scale->blockSignals (true);
Image_Scale->setValue (qMax (scaling.width (), scaling.height ()));
Image_Scale->blockSignals (enabled);

QSize
	thumbnail_size (image_display_size);
thumbnail_size.scale (Thumbnail_Max_Size, Qt::KeepAspectRatio);
#if ((DEBUG_SECTION) & DEBUG_RESET)
clog << "      Thumbnail_Max_Size = " << Thumbnail_Max_Size << endl
	 << "          thumbnail_size = " << thumbnail_size << endl;
#endif

enabled = true;

if (! Thumbnail)
	{
	Thumbnail = new QLabel;
	enabled = false;
	}

Plastic_Image
	*thumbnail_image = NULL;
if (enabled)
	{
	QString
		message;
	try
		{
		if (Image_View->image_name ().isEmpty ())
			thumbnail_image = Image_View->image ()->clone (thumbnail_size);
		else
		if (! (thumbnail_image = Plastic_Image_Factory::create
				(Image_View->image_name (), thumbnail_size)))
			{
			message =
				tr ("Could not create a %1x%2 thumbnail of the image.\n")
					.arg (thumbnail_size.width ())
					.arg (thumbnail_size.height ());
			message += Plastic_Image_Factory::error_message ();
			}
		if (thumbnail_image)
			{
			thumbnail_image->source_band_map
				(Image_View->image ()->source_band_map ());
			thumbnail_image->source_data_maps
				(const_cast<const Plastic_Image::Data_Map**>
				(Image_View->image ()->source_data_maps ()));
			thumbnail_image->source_origin
				(Image_View->displayed_image_origin ());
			thumbnail_image->source_scale
				((double)thumbnail_size.rwidth ()
					/ Displayed_Image_Size.rwidth ());
			thumbnail_image->update ();
			}
		}
	catch (Plastic_Image::Render_Exception& except)
		{
		message =
			tr ("Could not render a %1x%2 thumbnail of the image")
				.arg (thumbnail_size.width ())
				.arg (thumbnail_size.height ());
		if (Image_View->image_name ().isEmpty ())
			message += '.';
		else
			{
			message += " -\n";
			message += Image_View->image_name ();
			}
		message += "\n\n";
		message += QString::fromStdString (except.message ());
		}
	catch (...)
		{
		message =
			tr ("Unexpected exception while generating thumbnail image!");
		}
	if (! thumbnail_image)
		{
		QMessageBox::warning (this, windowTitle (), message);

		if (thumbnail_image)
			delete thumbnail_image;
		thumbnail_image = NULL;
		}
	}
if (thumbnail_image)
	{
	Thumbnail->setPixmap (QPixmap::fromImage (*thumbnail_image));
	delete thumbnail_image;
	}
else
	{
	QPixmap
		pixmap (thumbnail_size);
	pixmap.fill ();
	Thumbnail->setPixmap (pixmap);
	}
#if ((DEBUG_SECTION) & DEBUG_RESET)
clog << "<<< Save_Image_Dialog::reset: " << boolalpha << enabled << endl;
#endif
return enabled;
}

/*==============================================================================
	Slots
*/
void
Save_Image_Dialog::file_selected
	(
	const QString&	pathname
	)
{
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">-< Save_Image_Dialog::file_selected: \"" << pathname << '"' << endl;
#endif
if (! pathname.isEmpty ())
	{
	QString
		name (pathname);
	if (name.length () > 2 &&
		name.at (0) == '~' &&
		name.at (1) == QDir::separator ())
		//	Substitute '~' with the user's home directory pathname.
		name.replace (0, 1, QDir::homePath ());
	Pathname = QFileInfo (name).absoluteFilePath ();
	}
}

void
Save_Image_Dialog::filter_selected
	(
	const QString&	format
	)
{
Image_Format = format.left (format.indexOf (' ')).toLower ();
#if ((DEBUG_SECTION) & DEBUG_SLOTS)
clog << ">-< Save_Image_Dialog::filter_selected: \"" << format
		<< "\" - " << Image_Format << ')' << endl;
#endif
setDefaultSuffix (Image_Format);
}


void
Save_Image_Dialog::image_width
	(
	int		width
	)
{
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << ">>> Save_Image_Dialog::image_width: " << width << endl;
#endif
bool
	enabled;
double
	scale = (double)width / Displayed_Image_Size.rwidth ();
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "    scale = " << scale << endl;
#endif
if (scale > Image_Scale->maximum ())
	{
	scale = Image_Scale->maximum ();
	width = scale * Displayed_Image_Size.rwidth ();
	#if ((DEBUG_SECTION) & DEBUG_SIZE)
	clog << "    scale reset to " << scale << endl
		 << "    width reset to " << width << endl;
	#endif
	enabled = Image_Width->blockSignals (true);
	Image_Width->setValue (width);
	Image_Width->blockSignals (enabled);
	}

int
	height = scale * Displayed_Image_Size.rheight ();
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "    height = " << height << endl;
#endif
enabled = Image_Height->blockSignals (true);
Image_Height->setValue (height);
Image_Height->blockSignals (enabled);

enabled = Image_Scale->blockSignals (true);
Image_Scale->setValue (scale);
Image_Scale->blockSignals (enabled);
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "<<< Save_Image_Dialog::image_width" << endl;
#endif
}


void
Save_Image_Dialog::image_height
	(
	int		height
	)
{
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << ">>> Save_Image_Dialog::image_height: " << height << endl;
#endif
bool
	enabled;
double
	scale = (double)height / Displayed_Image_Size.rheight ();
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "    scale = " << scale << endl;
#endif
if (scale > Image_Scale->maximum ())
	{
	scale = Image_Scale->maximum ();
	height = scale * Displayed_Image_Size.rheight ();
	#if ((DEBUG_SECTION) & DEBUG_SIZE)
	clog << "    scale reset to " << scale << endl
		 << "    height reset to " << height << endl;
	#endif
	enabled = Image_Width->blockSignals (true);
	Image_Height->setValue (height);
	Image_Height->blockSignals (enabled);
	}

int
	width = scale * Displayed_Image_Size.rwidth ();
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "    width = " << width << endl;
#endif
enabled = Image_Width->blockSignals (true);
Image_Width->setValue (width);
Image_Width->blockSignals (enabled);

enabled = Image_Scale->blockSignals (true);
Image_Scale->setValue (scale);
Image_Scale->blockSignals (enabled);
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "<<< Save_Image_Dialog::image_height" << endl;
#endif
}


void
Save_Image_Dialog::image_scale
	(
	double	scale
	)
{
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << ">>> Save_Image_Dialog::image_scale: " << scale << endl;
#endif
bool
	enabled;
int
	length;
length = scale * Displayed_Image_Size.rwidth ();
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "    width = " << length << endl;
#endif
enabled = Image_Width->blockSignals (true);
Image_Width->setValue (length);
Image_Width->blockSignals (enabled);

length = scale * Displayed_Image_Size.rheight ();
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "    height = " << length << endl;
#endif
enabled = Image_Width->blockSignals (true);
Image_Height->setValue (length);
Image_Height->blockSignals (enabled);
#if ((DEBUG_SECTION) & DEBUG_SIZE)
clog << "<<< Save_Image_Dialog::image_scale" << endl;
#endif
}


void
Save_Image_Dialog::min_scale
	(
	double	scale
	)
{
if (scale > 0.0)
	Image_Scale->setMinimum (scale);
}


void
Save_Image_Dialog::max_scale
	(
	double	scale
	)
{
if (scale > 0.0)
	Image_Scale->setMaximum (scale);
}


}	//	namespace HiRISE
}	//	namespace UA
